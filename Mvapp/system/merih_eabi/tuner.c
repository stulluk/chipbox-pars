/*
 * tuner.c - public tuner API of the Merih library (port of Merih Tuner.o, dvbtuner.h).
 *
 * Written from the disassembly of the OABI object in libsystem_merih.a. A tuner handle is
 * the address of its TunerObject_t. Only the AVL2108 demodulator with the IX2564 PLL is
 * supported (the Chipbox hardware); STV0903 / STV6110 configurations are rejected.
 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "merih_tuner_int.h"

#define TUNER_TASK_PRIORITY   10
#define TUNER_TASK_STACK      1024
#define SKEW_TASK_STACK       500
#define TUNER_QUEUE_DEPTH     20
#define IX2564_REF_CLOCK      4000

TunerObject_t     *TunerHandle;
TunerMainObject_t *TunerMainObject;
U8                 MaxNumberOfTuner;

/* MERIH_TUNER_DEBUG=1 in the environment enables the TUNER_DEBUG() diagnostics. */
int tunerDebugEnabled(void)
{
	static int enabled = -1;
	const char *value;

	if (enabled < 0)
	{
		value   = getenv("MERIH_TUNER_DEBUG");
		enabled = ((value != NULL) && (value[0] == '1')) ? 1 : 0;
	}
	return enabled;
}

/* Allocate the handle tables for initParam.NumberOfTuner tuners. */
TunerError TunerInit(TunerInitParam_t initParam)
{
	U8 number = initParam.NumberOfTuner;

	if (number == 0)
		return TUNER_DATA_ERROR;
	TunerHandle = (TunerObject_t *)OsMemoryAllocate(number * sizeof(TunerObject_t));
	if (TunerHandle == NULL)
		return TUNER_INIT_ERROR;
	memset(TunerHandle, 0, number * sizeof(TunerObject_t));
	TunerMainObject = (TunerMainObject_t *)OsMemoryAllocate(number * sizeof(TunerMainObject_t));
	if (TunerMainObject == NULL)
	{
		OsMemoryFree(TunerHandle);
		TunerHandle = NULL;
		return TUNER_INIT_ERROR;
	}
	memset(TunerMainObject, 0, number * sizeof(TunerMainObject_t));
	MaxNumberOfTuner = number;
	return TUNER_NO_ERROR;
}

/*
 * Open one tuner: I2C devices, GPIO lines, demodulator / PLL bring-up (firmware download)
 * and the main task. *tunerHandleId gets the handle, 0 on error.
 */
TunerError TunerOpen(U32 *tunerHandleId, TunerOpenParam_t *openParam)
{
	TunerObject_t               *obj;
	TunerMainObject_t           *main;
	TunerDemodInfo_t            *info;
	TunerDemodInitParam_t        initParam;
	TunerTaskArg_t              *arg;
	const TunerDemodFunctions_t *demod;
	const TunerPllFunctions_t   *pll;
	U32                          resetGpio;
	U32                          ifType;
	U32                          skewTask = 0;
	U32                          mainTask = 0;
	char                         name[20];
	TunerError                   err;
	U8                           index;

	*tunerHandleId = 0;
	index = tunerAllocHandle(openParam);
	if (index == 0xff)
		return TUNER_INIT_ERROR;
	obj  = &TunerHandle[index];
	main = &TunerMainObject[index];
	memset(obj, 0, sizeof(*obj));

	if (openParam->Demodulator != AVL2108)
	{
		printf("TunerOpen : demodulator %d not supported (AVL2108 only)\n", openParam->Demodulator);
		return TUNER_INIT_ERROR;
	}
	if (openParam->TunerPll != IX2564)
	{
		printf("TunerOpen : PLL %d not supported (IX2564 only)\n", openParam->TunerPll);
		return TUNER_INIT_ERROR;
	}
	demod = &Avl2108DemodFunctions;
	pll   = &Ix2564PllFunctions;

	info = (TunerDemodInfo_t *)OsMemoryAllocate(sizeof(TunerDemodInfo_t));
	if (info == NULL)
		return TUNER_OPEN_ERROR;
	memset(info, 0, sizeof(*info));

	if ((tunerI2cOpen(&obj->DemodI2c, &openParam->DemodI2cParam) != TUNER_NO_ERROR) ||
	    (tunerI2cOpen(&obj->PllI2c, &openParam->PllI2cParam) != TUNER_NO_ERROR))
		return TUNER_OPEN_ERROR;
	if (tunerGpioOpen(&resetGpio, &openParam->TunerReset) != TUNER_NO_ERROR)
		return TUNER_OPEN_ERROR;
	obj->ResetGpio = resetGpio;
	tunerReset(resetGpio);

	if (openParam->Type == TUNER_SATELLITE)
	{
		if (tunerGpioOpen(&obj->LnbGpio, &openParam->LnbPower) != TUNER_NO_ERROR)
			return TUNER_OPEN_ERROR;
		tunerLnbPower(obj->LnbGpio, 0);
		if (tunerGpioOpen(&obj->HorVerGpio, &openParam->HorVer) != TUNER_NO_ERROR)
			return TUNER_OPEN_ERROR;
		tunerGpioOpen(&obj->V12Gpio, &openParam->V12);
		tunerGpioOpen(&obj->SkewGpio, &openParam->Skew);
	}

	if (OsCreateSemaphore(&obj->ControlSem, 1) != OS_NO_ERROR)
		obj->ControlSem = 0;
	if (OsCreateSemaphore(&obj->AccessSem, 1) != OS_NO_ERROR)
		obj->AccessSem = 0;
	if (OsCreateSemaphore(&obj->ConfirmSem, 0) != OS_NO_ERROR)
		obj->ConfirmSem = 0;

	sprintf(name, "TunerMQueue%d", openParam->TunerNumber);
	if ((OsCreateMessageQueue(&obj->MainQueue, name, TUNER_MSG_SIZE, TUNER_QUEUE_DEPTH) !=
	     OS_NO_ERROR) || (obj->MainQueue == 0))
		return TUNER_OPEN_ERROR;

	obj->PrevParam = (TunerSearchParam_t *)OsMemoryAllocate(sizeof(TunerSearchParam_t));
	if (obj->PrevParam == NULL)
		return TUNER_OPEN_ERROR;
	memset(obj->PrevParam, 0, sizeof(TunerSearchParam_t));

	obj->Type            = openParam->Type;
	obj->TunerNumber     = openParam->TunerNumber;
	obj->I2cNumber       = openParam->DemodI2cParam.DevideNo;
	obj->DemodAddress    = openParam->DemodI2cParam.Address;
	obj->PllAddress      = openParam->PllI2cParam.Address;
	obj->DemodSubAddrLen = openParam->DemodI2cParam.SubAddrLen;
	obj->PllSubAddrLen   = openParam->PllI2cParam.SubAddrLen;
	obj->PllRepeat       = openParam->PllI2CRepeate;
	obj->Demod           = demod;
	obj->MotorMoving     = 0;
	obj->SearchEnable    = 1;
	obj->InUse           = 1;

	/* Demodulator (firmware download) and PLL power-up; errors are not fatal (as before). */
	memset(&initParam, 0, sizeof(initParam));
	initParam.RefClock   = IX2564_REF_CLOCK;
	initParam.DemodId    = 0x87;
	initParam.MpegMode   = openParam->TunerOutputMode;
	initParam.PllAddress = openParam->PllI2cParam.Address;
	initParam.Pll        = pll;
	err = demod->Init(obj, &initParam, info);
	if (err != TUNER_NO_ERROR)
		printf("TunerOpen : AVL2108 init failed (%d)\n", err);
	err = pll->Init(obj, &ifType);
	if (err != TUNER_NO_ERROR)
		printf("TunerOpen : IX2564 init failed (%d)\n", err);
	TUNER_DEBUG("open: demod i2c %u addr 0x%02x, pll addr 0x%02x repeat %u, mpeg %u\n",
	            (unsigned)obj->DemodI2c, obj->DemodAddress, obj->PllAddress, obj->PllRepeat,
	            openParam->TunerOutputMode);

	if (obj->SkewGpio)
	{
		sprintf(name, "TunerSQueue%d", openParam->TunerNumber);
		if ((OsCreateMessageQueue(&obj->SkewQueue, name, 1, TUNER_QUEUE_DEPTH) == OS_NO_ERROR) &&
		    (obj->SkewQueue != 0))
		{
			sprintf(name, "TunerSTask%d", openParam->TunerNumber);
			OsCreateTask(&skewTask, name, tunerSkewTask, &obj->TunerNumber,
			             TUNER_TASK_PRIORITY, SKEW_TASK_STACK);
		}
	}

	/* The task gets its own copy of the arguments (freed by the task). */
	arg = (TunerTaskArg_t *)OsMemoryAllocate(sizeof(TunerTaskArg_t));
	if (arg == NULL)
	{
		memset(obj, 0, sizeof(*obj));
		return TUNER_OPEN_ERROR;
	}
	arg->TunerIndex = index;
	arg->DemodInfo  = info;
	arg->Demod      = demod;
	arg->CallBack   = openParam->CallBackNotify;
	sprintf(name, "TunerMTask%d", openParam->TunerNumber);
	if (OsCreateTask(&mainTask, name, tunerMainTask, arg, TUNER_TASK_PRIORITY,
	                 TUNER_TASK_STACK) != OS_NO_ERROR)
	{
		OsMemoryFree(arg);
		memset(obj, 0, sizeof(*obj));
		return TUNER_OPEN_ERROR;
	}

	main->Demod    = demod;
	main->Pll      = pll;
	main->MainTask = mainTask;
	main->SkewTask = skewTask;
	*tunerHandleId = (U32)obj;
	return TUNER_NO_ERROR;
}

/* Stop the tasks and release every resource of a tuner. */
TunerError TunerClose(U32 tunerHandleId)
{
	TunerObject_t     *obj = (TunerObject_t *)tunerHandleId;
	TunerMainObject_t *main;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	main = &TunerMainObject[obj->TunerNumber];
	if (main->MainTask)
		OsDeleteTask(main->MainTask);
	if (main->SkewTask)
		OsDeleteTask(main->SkewTask);
	if (obj->SkewQueue)
		OsDeleteMessageQueue(obj->SkewQueue);
	if (obj->MainQueue)
		OsDeleteMessageQueue(obj->MainQueue);
	if (obj->AccessSem)
		OsDeleteSemaphore(obj->AccessSem);
	if (obj->SkewGpio)
		GpioPortClose(obj->SkewGpio);
	if (obj->V12Gpio)
		GpioPortClose(obj->V12Gpio);
	if (obj->HorVerGpio)
		GpioPortClose(obj->HorVerGpio);
	if (obj->LnbGpio)
		GpioPortClose(obj->LnbGpio);
	if (obj->ResetGpio)
		GpioPortClose(obj->ResetGpio);
	if (obj->PllI2c)
		I2cClose(obj->PllI2c);
	if (obj->DemodI2c)
		I2cClose(obj->DemodI2c);
	obj->InUse = 0;
	return TUNER_NO_ERROR;
}

/* Free the handle tables. */
TunerError TunerTerm(void)
{
	OsMemoryFree(TunerMainObject);
	TunerMainObject = NULL;
	if (TunerHandle == NULL)
		return TUNER_INIT_ERROR;
	OsMemoryFree(TunerHandle);
	TunerHandle = NULL;
	return TUNER_NO_ERROR;
}

/* Ask the main task to tune (searchData->Symbolrate < 1000 starts a blind scan). */
TunerError TunerSearchStart(U32 tunerHandleId, TunerSearchParam_t *searchData)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	if (obj->TunerNumber != searchData->TunerNumber)
		return TUNER_DATA_ERROR;
	return tunerSendMainMessage(obj->MainQueue, TUNER_MSG_SEARCH, searchData);
}

/* Lock state, strength and quality as last measured by the main task. */
TunerError TunerReadSignalState(U32 tunerHandleId, TunerSignalState_t *siganlState)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	*siganlState = obj->SignalState;
	return TUNER_NO_ERROR;
}

/* Copy the transponders found by the last blind scan; returns their number. */
U16 TunerGetBlindTpData(U32 tunerHandleId, U8 currentPol, TunerBlindTpData_t *tpData)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;
	U16            number;
	U16            index;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	number = obj->SignalState.BlindTpFound;
	if ((obj->SignalState.LockState != SIGNAL_LOCK) || (number == 0))
		return 0;
	for (index = 0; index < number; index++)
	{
		tpData[index].Frequency    = obj->TuneResult[index].TpFrequency;
		tpData[index].SymbolRate   = obj->TuneResult[index].Symbolrate;
		tpData[index].Polarization = currentPol;
	}
	return number;
}

/* Send a raw DiSEqC message (22 kHz is paused around it). */
TunerError TunerSendDiseqcData(U32 tunerHandleId, U8 *data, U32 len)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;
	TunerError     err;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	if ((obj->Demod == NULL) || (obj->Demod->SendDiseqc == NULL) || (len > 8))
		return TUNER_DATA_ERROR;

	tunerControlLock(obj);
	obj->Demod->Set22K(obj, 0);
	OsWaitMillisecond(25);
	err = obj->Demod->SendDiseqc(obj, data, len);
	OsWaitMillisecond(25);
	obj->Demod->Set22K(obj, obj->PrevParam->On22khz);
	printf("TunerSendDiseqcData : 22KHz = %d\n", obj->PrevParam->On22khz);
	OsWaitMillisecond(25);
	tunerControlRelease(obj);
	return err;
}

/* DiSEqC 1.2 positioner command. */
TunerError TunerControlMotor(U32 tunerHandleId, DistqcMotorCommand command, S16 value)
{
	U8         cmd[8];
	U8         len;
	TunerError err;

	len = tunerGetDiseqcMotor(command, value, cmd);
	if (len == 0)
		return TUNER_DATA_ERROR;
	err = TunerSendDiseqcData(tunerHandleId, cmd, len);
	OsWaitMillisecond(25);
	return err;
}

/*
 * USALS motor angle in 0.1 degree for a satellite and receiver position. Arguments in
 * 0.1 degree; longitudes above 180.0 are west (minus 360.0), latitudes above 100.0 south.
 * Returns 0 when the satellite is out of reach.
 */
S16 TunerGetMotorAngle(int satLongitude, int myLongitude, int myLatitude)
{
	const double earth    = 6378.0;
	const double orbit    = 42164.2;
	const double pi       = 3.14159265358979323846;
	const double toRad    = pi / 180.0;
	int          satLon   = (satLongitude > 1800) ? satLongitude - 3600 : satLongitude;
	int          myLon    = (myLongitude > 1800) ? myLongitude - 3600 : myLongitude;
	int          latitude = (myLatitude > 1000) ? -(myLatitude % 1000) : myLatitude;
	int          delta;
	double       latDeg;
	double       z;
	double       z2;
	double       rLat;
	double       y;
	double       y2;
	double       x;
	double       a;
	double       b;
	double       c;
	double       angle;
	int          tenths;

	if ((abs(satLon) > 1800) || (abs(myLon) > 1800) || (abs(latitude) > 900))
		return 0;
	delta = abs(satLon - myLon);
	if (delta > 1800)
		delta -= 3600;
	if (abs(delta) > 650)
		return 0;

	latDeg = latitude / 10.0;
	z      = sin(latDeg * toRad) * earth;          /* height above the equator plane */
	z2     = z * z;
	rLat   = sqrt(earth * earth - z2);            /* radius of the latitude circle */
	y      = sin((delta / 10.0) * toRad) * rLat;
	y2     = sqrt(y * y + z2);
	y2     = y2 * y2;
	x      = sqrt(earth * earth - y2);
	a      = sqrt((orbit - rLat) * (orbit - rLat) + z2);
	b      = sqrt((orbit - x) * (orbit - x) + y2);
	c      = sqrt(2.0 * orbit * orbit - 2.0 * orbit * orbit * cos((delta / 10.0) * toRad));
	angle  = acos((b * b + a * a - c * c) / (2.0 * b * a)) * 180.0 / pi;
	if (fabs(angle) > 65.0)
		return 0;

	if (!(((satLon < myLon) && (latDeg > 0.0)) || ((satLon > myLon) && (latDeg < 0.0))))
		angle = -angle;
	tenths = (int)(angle * 100.0);
	if (tenths <= 0)
		return (S16)(-((-tenths + 5) / 10));
	return (S16)((tenths + 5) / 10);
}

/* 0 while a stop / off is in progress (aborts lock polling and blind scan). */
U8 TunerGetSearchStop(U32 tunerHandleId)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;

	return (obj != NULL) ? obj->SearchEnable : 0;
}

/* Blind scan progress in percent. */
U8 TunerGetBlindProcess(U32 tunerHandleId)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;

	return (obj != NULL) ? obj->BlindProcess : 0;
}

TunerError TunerSetBlindProcess(U32 tunerHandleId, U8 procss)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;

	if (obj == NULL)
		return TUNER_INIT_ERROR;
	obj->BlindProcess = (procss >= 100) ? 100 : procss;
	return TUNER_NO_ERROR;
}

/* Send a stop / LNB-off request and wait until the main task confirmed it. */
static TunerError tunerStopRequest(U32 tunerHandleId, U8 type)
{
	TunerObject_t *obj = (TunerObject_t *)tunerHandleId;
	TunerError     err;

	if (obj == NULL)
		return TUNER_DATA_ERROR;
	obj->SearchEnable = 0;
	err = tunerSendMainMessage(obj->MainQueue, type, NULL);
	if (err == TUNER_NO_ERROR)
		tunerConfirmReceive(obj);
	obj->SearchEnable = 1;
	return err;
}

/* Stop searching / monitoring. */
TunerError TunerSearchStop(U32 tunerHandleId)
{
	return tunerStopRequest(tunerHandleId, TUNER_MSG_STOP);
}

/* Stop and switch the LNB supply off (standby). */
TunerError TunerOff(U32 tunerHandleId)
{
	return tunerStopRequest(tunerHandleId, TUNER_MSG_LNB_OFF);
}
