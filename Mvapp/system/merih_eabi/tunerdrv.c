/*
 * tunerdrv.c - tuner driver core (port of Merih TunerDrv.o): register access, LNB / DiSEqC
 * control and the tuner main task.
 *
 * Written from the disassembly of the OABI object in libsystem_merih.a. The main task is a
 * small state machine driven by queue messages (search / stop / LNB off / report) and by
 * timeouts (retune, lock monitoring every 100 ms).
 */
#include <string.h>

#include "merih_tuner_int.h"
#include "mvutil.h"

/* Provided by mvapp (cat6611api.c / ui_common.c). */
extern void Cat6611Pause(void);
extern void Cat6611Resume(void);
extern void MvSendMotorMove(U8 moveOn);

/* Main task states. */
#define STATE_IDLE      0
#define STATE_SWITCH    1   /* apply LNB power / DiSEqC / 22 kHz */
#define STATE_LOCK      2   /* run the lock algorithm */
#define STATE_RETRY     3   /* not locked: retune or give up */
#define STATE_MONITOR   4   /* locked: watch the lock */

#define TIMEOUT_NONE    0

/* USALS fraction digits -> DiSEqC nibble (0.0 .. 0.9 degrees). */
static const U8 DiseqcHexTable[10] =
{
	0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x0a, 0x0b, 0x0d, 0x0e
};

/* Tuner filter bandwidth (MHz) for a symbol rate (kSps) and roll-off. */
U32 tunerGetBandWidth(U32 symbolRate, U8 rollOff)
{
	U32 factor;

	if (rollOff == ROLL_OFF_25)
		factor = 25;
	else if (rollOff == ROLL_OFF_20)
		factor = 20;
	else
		factor = 35;
	return ((symbolRate * (factor + 100)) / 100) / 2000;
}

/* Index of a free tuner handle for openParam->TunerNumber, or 0xff. */
U8 tunerAllocHandle(TunerOpenParam_t *openParam)
{
	U8 index;

	if ((MaxNumberOfTuner == 0) || (openParam == NULL) || (TunerHandle == NULL))
		return 0xff;
	index = openParam->TunerNumber;
	if (index >= MaxNumberOfTuner)
		return 0xff;
	return TunerHandle[index].InUse ? 0xff : index;
}

/* Open an I2C device (only bus 0 exists). */
TunerError tunerI2cOpen(U32 *deviceId, I2cOpenParam_t *param)
{
	U32 handle = 0;

	if (param->DevideNo != 0)
	{
		*deviceId = 0;
		return TUNER_DATA_ERROR;
	}
	if ((I2cOpen(&handle, param) != OS_NO_ERROR) || (handle == 0))
	{
		*deviceId = 0;
		return TUNER_OPEN_ERROR;
	}
	*deviceId = handle;
	return TUNER_NO_ERROR;
}

/* Open a GPIO line; port 0xff means "not used" (handle 0). */
TunerError tunerGpioOpen(U32 *deviceId, GpioOpenParam_t *param)
{
	U32 handle = 0;

	*deviceId = 0;
	if (param->PortNumber == 0xff)
		return TUNER_NO_ERROR;
	if ((param->PortNumber > 1) || (param->BitNumber >= MaxBitsPerGpio[param->PortNumber]))
		return TUNER_DATA_ERROR;
	if ((GpioPortOpen(&handle, param) != OS_NO_ERROR) || (handle == 0))
		return TUNER_OPEN_ERROR;
	*deviceId = handle;
	return TUNER_NO_ERROR;
}

/* Pulse the demodulator reset line low for 100 ms. */
TunerError tunerReset(U32 gpio)
{
	if (gpio == 0)
		return TUNER_NOT_FOUND;
	OsWaitMillisecond(100);
	if (GpioPortWrite(gpio, 0) != OS_NO_ERROR)
		return TUNER_DATA_ERROR;
	OsWaitMillisecond(100);
	if (GpioPortWrite(gpio, 1) != OS_NO_ERROR)
		return TUNER_DATA_ERROR;
	return TUNER_NO_ERROR;
}

/* Write an active-low control line (LNB power, H/V, 12 V). */
static TunerError tunerWriteInverted(U32 gpio, U8 on)
{
	if (gpio == 0)
		return TUNER_NOT_FOUND;
	return (GpioPortWrite(gpio, (U8)(1 - on)) != OS_NO_ERROR) ? TUNER_DATA_ERROR : TUNER_NO_ERROR;
}

/* LNB supply on/off (line low = on). */
TunerError tunerLnbPower(U32 gpio, U8 on)
{
	return tunerWriteInverted(gpio, on);
}

/* LNB polarisation: hor = 1 -> 18 V (line low), vertical -> 13 V (line high). */
TunerError tunerHorVer(U32 gpio, U8 hor)
{
	return tunerWriteInverted(gpio, hor);
}

/* 0/12 V switch output. */
TunerError tuner12v(U32 gpio, U8 on)
{
	return tunerWriteInverted(gpio, on);
}

/* Read len bytes at a sub address; error unless the whole length was transferred. */
static TunerError tunerGetRegisters(U32 i2c, U32 subAddr, U8 *data, U32 len)
{
	if (i2c == 0)
		return TUNER_NOT_FOUND;
	return (I2cRead(i2c, subAddr, data, len) == len) ? TUNER_NO_ERROR : TUNER_DATA_ERROR;
}

/* Write len bytes at a sub address; error unless the whole length was transferred. */
static TunerError tunerSetRegisters(U32 i2c, U32 subAddr, U8 *data, U32 len)
{
	if (i2c == 0)
		return TUNER_NOT_FOUND;
	return (I2cWrite(i2c, subAddr, data, len) == len) ? TUNER_NO_ERROR : TUNER_DATA_ERROR;
}

/* Demodulator register read/write split in bus-sized chunks under the I2C bus lock. */
static TunerError tunerDemodTransfer(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len,
                                     BOOL write)
{
	U32        chunk;
	U32        size;
	TunerError err = TUNER_NO_ERROR;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	if (obj->DemodI2c == 0)
		return TUNER_NOT_FOUND;

	I2cAccessLock(obj->I2cNumber);
	chunk = (64 - obj->DemodSubAddrLen) & 0xfffe;
	while (len > 0)
	{
		size = (chunk < len) ? chunk : len;
		err  = write ? tunerSetRegisters(obj->DemodI2c, subAddr, data, size)
		             : tunerGetRegisters(obj->DemodI2c, subAddr, data, size);
		if (err != TUNER_NO_ERROR)
		{
			I2cAccessRelease(obj->I2cNumber);
			return TUNER_DATA_ERROR;
		}
		data    += size;
		subAddr += size;
		len     -= size;
	}
	I2cAccessRelease(obj->I2cNumber);
	return TUNER_NO_ERROR;
}

/* Read demodulator registers. */
TunerError tunerGetDemodResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	return tunerDemodTransfer(obj, subAddr, data, len, FALSE);
}

/* Write demodulator registers. */
TunerError tunerSetDemodResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	return tunerDemodTransfer(obj, subAddr, data, len, TRUE);
}

/* Read PLL registers, through the demodulator repeater when configured. */
TunerError tunerGetPllResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	TunerError err;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	I2cAccessLock(obj->I2cNumber);
	if (obj->PllRepeat)
	{
		I2cAccessRelease(obj->I2cNumber);
		if ((obj->Demod == NULL) || (obj->Demod->RepeatRead == NULL))
			return TUNER_SET_ERROR;
		return obj->Demod->RepeatRead(obj, subAddr, data, len);
	}
	err = tunerGetRegisters(obj->PllI2c, subAddr, data, len);
	I2cAccessRelease(obj->I2cNumber);
	return err;
}

/* Write PLL registers, through the demodulator repeater when configured. */
TunerError tunerSetPllResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len)
{
	TunerError err;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	I2cAccessLock(obj->I2cNumber);
	if (obj->PllRepeat)
	{
		I2cAccessRelease(obj->I2cNumber);
		if ((obj->Demod == NULL) || (obj->Demod->RepeatWrite == NULL))
			return TUNER_SET_ERROR;
		return obj->Demod->RepeatWrite(obj, subAddr, data, len);
	}
	err = tunerSetRegisters(obj->PllI2c, subAddr, data, len);
	I2cAccessRelease(obj->I2cNumber);
	return err;
}

/* Read one PLL register. */
TunerError tunerGetPllOneResters(TunerObject_t *obj, U32 subAddr, U8 *data)
{
	return tunerGetPllResters(obj, subAddr, data, 1);
}

/* Receive the newest message (older pending ones are dropped). */
static U8 *tunerMainMessageReceive(U32 queue, U32 timeout)
{
	U8 *message;
	U8 *next;

	message = (U8 *)OsReceiveMessage(queue, timeout);
	if (message == NULL)
		return NULL;
	next = (U8 *)OsReceiveMessage(queue, TIMEOUT_IMMEDIATE);
	while (next != NULL)
	{
		OsReleaseMessage(queue, message);
		message = next;
		next    = (U8 *)OsReceiveMessage(queue, TIMEOUT_IMMEDIATE);
	}
	return message;
}

/* Queue a message for the main task (search parameters only for TUNER_MSG_SEARCH). */
TunerError tunerSendMainMessage(U32 queue, U8 type, TunerSearchParam_t *param)
{
	U8 *message;

	message = (U8 *)OsClaimMessage(queue);
	if (message == NULL)
		return TUNER_SET_ERROR;
	message[0] = type;
	if (type == TUNER_MSG_SEARCH)
		memcpy(&message[4], param, sizeof(TunerSearchParam_t));
	return (OsSendMessage(queue, message) != OS_NO_ERROR) ? TUNER_SET_ERROR : TUNER_NO_ERROR;
}

/* Queue a skew value for the skew task. */
static TunerError tunerSendSkewMessage(U32 queue, U8 value)
{
	U8 *message;

	message = (U8 *)OsClaimMessage(queue);
	if (message == NULL)
		return TUNER_SET_ERROR;
	message[0] = value;
	return (OsSendMessage(queue, message) != OS_NO_ERROR) ? TUNER_SET_ERROR : TUNER_NO_ERROR;
}

/* Take / give one of the tuner semaphores. */
static TunerError tunerSemWait(TunerObject_t *obj, U32 sem)
{
	if ((obj == NULL) || (sem == 0))
		return TUNER_INIT_ERROR;
	OsSemaphoreWait(sem, TIMEOUT_FOREVER);
	return TUNER_NO_ERROR;
}

static TunerError tunerSemSignal(TunerObject_t *obj, U32 sem)
{
	if ((obj == NULL) || (sem == 0))
		return TUNER_INIT_ERROR;
	OsSemaphoreSignal(sem);
	return TUNER_NO_ERROR;
}

/* LNB / DiSEqC control section. */
TunerError tunerControlLock(TunerObject_t *obj)
{
	return tunerSemWait(obj, (obj != NULL) ? obj->ControlSem : 0);
}

TunerError tunerControlRelease(TunerObject_t *obj)
{
	return tunerSemSignal(obj, (obj != NULL) ? obj->ControlSem : 0);
}

/* Demodulator register access section. */
TunerError tunerAccessLock(TunerObject_t *obj)
{
	return tunerSemWait(obj, (obj != NULL) ? obj->AccessSem : 0);
}

TunerError tunerAccessRelease(TunerObject_t *obj)
{
	return tunerSemSignal(obj, (obj != NULL) ? obj->AccessSem : 0);
}

/* Stop / LNB off handshake between the API and the main task. */
TunerError tunerConfirmReceive(TunerObject_t *obj)
{
	return tunerSemWait(obj, (obj != NULL) ? obj->ConfirmSem : 0);
}

static TunerError tunerConfirmSend(TunerObject_t *obj)
{
	return tunerSemSignal(obj, (obj != NULL) ? obj->ConfirmSem : 0);
}

/*
 * Did the switch side (DiSEqC port, motor position, LNB power, 12 V, polarisation, skew,
 * 22 kHz, LNB type) change since the last tune? Also arms the motor wait counters.
 */
static BOOL tunerCheckSwitchParam(TunerObject_t *obj, TunerSearchParam_t *param)
{
	TunerSearchParam_t *prev;
	S32                 delta;
	U32                 wait;

	if (obj == NULL)
		return FALSE;
	obj->MotorMoving    = 0;
	obj->MotorWaitCount = 0;
	obj->MotorRetry     = 0;
	if (obj->Type != TUNER_SATELLITE)
		return FALSE;
	prev = obj->PrevParam;

	if (param->ConnectionMode != prev->ConnectionMode)
	{
		if ((param->ConnectionMode == TUNER_CONECTION_MOTOR) ||
		    (param->ConnectionMode == TUNER_CONECTION_USALS))
		{
			obj->MotorRetry     = 0;
			obj->MotorMoving    = 1;
			obj->MotorWaitCount = 50;
		}
		return TRUE;
	}

	switch (param->ConnectionMode)
	{
		case TUNER_CONECTION_DISEQC:
			if ((prev->DiseqcPort != param->DiseqcPort) || (prev->ToneBurst != param->ToneBurst))
				return TRUE;
			break;
		case TUNER_CONECTION_MOTOR:
		case TUNER_CONECTION_USALS:
			if (param->ConnectionMode == TUNER_CONECTION_MOTOR)
				delta = (S32)prev->MotorPosition - (S32)param->MotorPosition;
			else
				delta = (S32)prev->MotorAngle - (S32)param->MotorAngle;
			if (delta != 0)
			{
				obj->MotorRetry  = 0;
				obj->MotorMoving = 1;
				if (delta < 0)
					delta = -delta;
				/* About 3.5 s per 100 steps / 10 degrees, at most 50 retries of 500 ms. */
				wait = (U32)(delta * 35) / 1000 + 2;
				obj->MotorWaitCount = (U8)((wait > 50) ? 50 : wait);
				return TRUE;
			}
			break;
		case TUNER_CONECTION_UNICABLE:
			if ((prev->UnicableSatPosition != param->UnicableSatPosition) ||
			    (prev->UniCableUBand != param->UniCableUBand) ||
			    (prev->UniCableBandFreq != param->UniCableBandFreq))
				return TRUE;
			break;
		default:
			break;
	}

	if (obj->LnbGpio && (prev->Power != param->Power))
		return TRUE;
	if (obj->V12Gpio && (prev->Sw12V != param->Sw12V))
		return TRUE;
	if (obj->HorVerGpio && (prev->HorVer != param->HorVer))
		return TRUE;
	if (obj->SkewGpio && (prev->SkewValue != param->SkewValue))
		return TRUE;
	if (prev->On22khz != param->On22khz)
		return TRUE;
	return (prev->LnbType != param->LnbType) ? TRUE : FALSE;
}

/*
 * Convert the search request into demodulator tune parameters (IF frequency, symbol rate,
 * search mode) and pick the LNB oscillator / 22 kHz. Returns TRUE when the transponder
 * (frequency beyond the tolerance, symbol rate, LNB setup) changed.
 */
static BOOL tunerCheckTuneParam(TunerObject_t *obj, TunerSearchParam_t *param,
                                TunerTuneParam_t *tune, U32 *lnbLocal)
{
	TunerSearchParam_t *prev;
	BOOL                changed = FALSE;
	U32                 tolerance;
	U32                 local;

	*lnbLocal = 0;
	if (obj == NULL)
		return FALSE;
	prev = obj->PrevParam;

	if (param->Symbolrate < 1000)
	{
		tolerance = 0;
	}
	else
	{
		tolerance = (prev->Symbolrate + 1000) / 2000;
		if (tolerance >= 5)
			tolerance = 5;
	}
	if ((param->TpFrequency > prev->TpFrequency + tolerance) ||
	    (param->TpFrequency < prev->TpFrequency - tolerance))
		changed = TRUE;

	if (param->Symbolrate < 1000)
	{
		tune->SearchMode = 0;                 /* blind */
		tune->Symbolrate = 0;
		changed          = TRUE;
	}
	else
	{
		if ((param->Symbolrate > prev->Symbolrate + 10) ||
		    (param->Symbolrate < prev->Symbolrate - 10))
			changed = TRUE;
		tune->Symbolrate = param->Symbolrate;
		tune->SearchMode = 2;                 /* fixed symbol rate */
	}

	if (obj->Type != TUNER_SATELLITE)
	{
		tune->TpFrequency = param->TpFrequency;
		return changed;
	}

	if (param->LnbType == LNB_SINGLE)
		param->LnbLocalLow = param->LnbLocalHi;
	if ((prev->LnbType != param->LnbType) || (prev->LnbLocalHi != param->LnbLocalHi) ||
	    (prev->LnbLocalLow != param->LnbLocalLow))
		changed = TRUE;

	switch (param->LnbType)
	{
		case LNB_UNIVERSAL:
			/* High band (22 kHz) above the middle of the two oscillators + 3100 MHz. */
			if (param->TpFrequency >= (param->LnbLocalLow + param->LnbLocalHi + 3100) / 2)
			{
				param->On22khz = 1;
				local          = param->LnbLocalHi;
			}
			else
			{
				param->On22khz = 0;
				local          = param->LnbLocalLow;
			}
			break;
		case LNB_WIDE:
			local = param->HorVer ? param->LnbLocalHi : param->LnbLocalLow;
			break;
		default:
			local = param->LnbLocalLow;
			break;
	}

	*lnbLocal = local;
	if (param->TpFrequency >= local)
	{
		tune->TpFrequency  = param->TpFrequency - local;
		tune->FrequencyMin = param->StartFrequency - local;
		tune->FrequencyMax = param->StopFrequency - local;
	}
	else
	{
		/* C band: the IF spectrum is inverted. */
		tune->TpFrequency  = local - param->TpFrequency;
		tune->FrequencyMin = local - param->StopFrequency;
		tune->FrequencyMax = local - param->StartFrequency;
	}
	return changed;
}

/* RF frequency of a locked signal from the requested IF and the measured IF. */
static U32 tunerGetTunedFrequency(TunerSearchParam_t *param, U32 requestIf, U32 lockedIf,
                                  U32 lnbLocal)
{
	BOOL inverted = (param->ConnectionMode == TUNER_CONECTION_UNICABLE) ? TRUE : FALSE;

	if (param->TpFrequency < lnbLocal)
		inverted = !inverted;
	if (inverted)
		return param->TpFrequency + requestIf - lockedIf;
	return param->TpFrequency + lockedIf - requestIf;
}

/* RF frequency of a blind-scan result. */
static U32 tunerGetTpFrequency(TunerSearchParam_t *param, U32 ifFreq, U32 lnbLocal)
{
	if (param->TpFrequency >= lnbLocal)
		return ifFreq + lnbLocal;
	return lnbLocal - ifFreq;
}

/* DiSEqC 1.0 committed (ports 1-4) or 1.1 uncommitted (5-20) switch command. */
static TunerError tunerGetDiseqcSw(TunerSearchParam_t *param, U8 *cmd)
{
	U8 port = param->DiseqcPort;

	cmd[0] = 0xe0;
	cmd[1] = 0x10;
	cmd[2] = 0x38;
	if ((U8)(port - 1) > 19)
		return TUNER_DATA_ERROR;
	if (port <= 4)
		cmd[3] = (U8)(0xf0 | ((((port - 1) << 2) + (param->HorVer << 1) + param->On22khz) & 0x0f));
	else
	{
		cmd[2] = 0x39;
		cmd[3] = (U8)(0xf0 | ((port - 5) & 0x0f));
	}
	return TUNER_NO_ERROR;
}

/* DiSEqC 1.2 / USALS positioner command; returns its length (0 = unknown command). */
U8 tunerGetDiseqcMotor(DistqcMotorCommand command, S16 value, U8 *cmd)
{
	U32 angle;
	U32 degree;

	cmd[0] = 0xe0;
	cmd[1] = 0x31;
	switch (command)
	{
		case MOTOR_CMD_HALT:       cmd[2] = 0x60; return 3;
		case MOTOR_CMD_LIMIT_OFF:  cmd[2] = 0x63; return 3;
		case MOTOR_CMD_LIMIT_EAST: cmd[2] = 0x66; return 3;
		case MOTOR_CMD_LIMIT_WEST: cmd[2] = 0x67; return 3;
		case MOTOR_CMD_DRIVE_EAST:
			cmd[2] = 0x68;
			cmd[3] = (value == 0) ? 0xff : (U8)value;
			return 4;
		case MOTOR_CMD_DRIVE_WEST:
			cmd[2] = 0x69;
			cmd[3] = (value == 0) ? 0xff : (U8)value;
			return 4;
		case MOTOR_CMD_STORE_POSITION:
			cmd[2] = 0x6a;
			cmd[3] = (U8)value;
			return 4;
		case MOTOR_CMD_GOTO_POSITION:
			cmd[2] = 0x6b;
			cmd[3] = (U8)value;
			return 4;
		case MOTOR_CMD_GOTO_REF:
			cmd[2] = 0x6b;
			cmd[3] = 0;
			return 4;
		case MOTOR_CMD_GOTO_X:
			/* Angle in 0.1 degree: 0xD0/0xE0 = east/west, BCD-like degrees, fraction table. */
			cmd[2] = 0x6e;
			if (value >= 0)
			{
				angle  = (U32)value;
				cmd[3] = 0xd0;
			}
			else
			{
				angle  = (U32)(-value);
				cmd[3] = 0xe0;
			}
			cmd[4]  = 0;
			degree  = angle / 10;
			cmd[4] += (U8)((degree & 0x0f) << 4);
			cmd[3] += (U8)((degree & 0xf0) >> 4);
			cmd[4] |= DiseqcHexTable[angle - degree * 10];
			return 5;
		case MOTOR_CMD_RECALCULATION:
			cmd[2] = 0x6f;
			cmd[3] = 0;
			return 4;
		default:
			return 0;
	}
}

/* Unicable (EN 50494) ODU_Channel_change command; also moves the IF to the user band. */
static TunerError tunerGetUniCableData(TunerSearchParam_t *param, TunerTuneParam_t *tune, U8 *cmd)
{
	U32 tuningWord;
	U32 bank;

	cmd[0] = 0xe0;
	cmd[1] = 0x10;
	cmd[2] = 0x5a;
	tuningWord        = ((tune->TpFrequency + param->UniCableBandFreq + 2) >> 2) - 350;
	tune->TpFrequency = (tuningWord << 2) - tune->TpFrequency + 1400;
	bank   = ((param->HorVer << 1) + (param->UnicableSatPosition << 2) + param->On22khz) & 0xff;
	cmd[3] = (U8)((bank << 2) | ((param->UniCableUBand - 1) << 5) | ((tuningWord >> 8) & 3));
	cmd[4] = (U8)tuningWord;
	return TUNER_NO_ERROR;
}

/* Switch the LNB supply off and remember it. */
static TunerError tunerSetLnbOff(TunerObject_t *obj)
{
	if (obj == NULL)
		return TUNER_NOT_FOUND;
	tunerLnbPower(obj->LnbGpio, 0);
	obj->PrevParam->Power = 0;
	return TUNER_NO_ERROR;
}

/* Apply LNB power, 12 V, skew, polarisation, DiSEqC / positioner / Unicable and 22 kHz. */
static TunerError tunerSetSwitch(TunerObject_t *obj, TunerSearchParam_t *param,
                                 TunerTuneParam_t *tune, const TunerDemodFunctions_t *demod)
{
	TunerSearchParam_t *prev;
	U8                  cmd[8];
	U8                  len;

	if (obj == NULL)
		return TUNER_NOT_FOUND;
	if ((demod == NULL) || (demod->SendDiseqc == NULL) || (demod->Set22K == NULL))
		return TUNER_INIT_ERROR;
	prev = obj->PrevParam;

	if (obj->V12Gpio)
	{
		if (prev->Sw12V != param->Sw12V)
		{
			tuner12v(obj->V12Gpio, param->Sw12V);
			prev->Sw12V = param->Sw12V;
		}
	}
	else
	{
		prev->Sw12V = 0xff;
	}

	if (obj->LnbGpio)
	{
		tunerLnbPower(obj->LnbGpio, param->Power);
		if (prev->Power != param->Power)
			prev->Power = param->Power;
	}
	else
	{
		prev->Power = 0xff;
	}

	if (obj->Type != TUNER_SATELLITE)
		return TUNER_NO_ERROR;

	if (obj->SkewGpio)
	{
		if (prev->SkewValue != param->SkewValue)
		{
			tunerSendSkewMessage(obj->SkewQueue, param->SkewValue);
			prev->SkewValue = param->SkewValue;
			OsWaitMillisecond(100);
		}
	}
	else
	{
		prev->SkewValue = 0xff;
	}

	if (param->Power == 0)
		return TUNER_NO_ERROR;

	prev->On22khz             = param->On22khz;
	prev->DiseqcPort          = param->DiseqcPort;
	prev->UniCableBandFreq    = param->UniCableBandFreq;
	prev->UnicableSatPosition = param->UnicableSatPosition;
	prev->ToneBurst           = param->ToneBurst;
	prev->UniCableUBand       = param->UniCableUBand;
	prev->ConnectionMode      = param->ConnectionMode;
	prev->MotorPosition       = param->MotorPosition;
	prev->MotorAngle          = param->MotorAngle;

	tunerControlLock(obj);
	if (param->ConnectionMode == TUNER_CONECTION_UNICABLE)
	{
		demod->Set22K(obj, 0);
		tunerHorVer(obj->HorVerGpio, 1);
		OsWaitMillisecond(20);
		tunerGetUniCableData(param, tune, cmd);
		demod->SendDiseqc(obj, cmd, 5);
		OsWaitMillisecond(15);
		tunerHorVer(obj->HorVerGpio, 0);
		tunerControlRelease(obj);
		return TUNER_NO_ERROR;
	}

	if (prev->HorVer != param->HorVer)
	{
		prev->HorVer = param->HorVer;
		tunerHorVer(obj->HorVerGpio, param->HorVer);
		OsWaitMillisecond(20);
	}

	switch (param->ConnectionMode)
	{
		case TUNER_CONECTION_DISEQC:
			if ((param->DiseqcPort == 0) && (param->ToneBurst == 0))
				break;
			demod->Set22K(obj, 0);
			OsWaitMillisecond(25);
			if (tunerGetDiseqcSw(param, cmd) == TUNER_NO_ERROR)
			{
				demod->SendDiseqc(obj, cmd, 4);
				OsWaitMillisecond(15);
			}
			if ((demod->SetToneBurst != NULL) && (param->ToneBurst != 0))
			{
				demod->SetToneBurst(obj, param->ToneBurst);
				OsWaitMillisecond(20);
			}
			break;
		case TUNER_CONECTION_MOTOR:
		case TUNER_CONECTION_USALS:
			if (obj->MotorMoving == 0)
				break;
			demod->Set22K(obj, 0);
			OsWaitMillisecond(25);
			len = tunerGetDiseqcMotor(MOTOR_CMD_HALT, 0, cmd);
			demod->SendDiseqc(obj, cmd, len);
			OsWaitMillisecond(25);
			if (param->ConnectionMode == TUNER_CONECTION_MOTOR)
				len = tunerGetDiseqcMotor(MOTOR_CMD_GOTO_POSITION, param->MotorPosition, cmd);
			else
				len = tunerGetDiseqcMotor(MOTOR_CMD_GOTO_X, param->MotorAngle, cmd);
			demod->SendDiseqc(obj, cmd, len);
			OsWaitMillisecond(25);
			break;
		default:
			break;
	}

	demod->Set22K(obj, param->On22khz);
	tunerControlRelease(obj);
	OsWaitMillisecond(20);
	return TUNER_NO_ERROR;
}

/* Mark the signal as unlocked (mvapp shows 10 % / 10 % then). */
static void tunerSetUnlocked(TunerObject_t *obj)
{
	obj->SignalState.LockState = SIGNAL_UNLOCK;
	obj->SignalState.Strength  = 10;
	obj->SignalState.Quality   = 10;
}

/* Skew positioner task (no skew line on the Chipbox, so it is never started there). */
void tunerSkewTask(void *param)
{
	U8             index = *(U8 *)param;
	TunerObject_t *obj;
	U8            *message;

	if (index == 0xff)
		return;
	obj = &TunerHandle[index];
	if ((obj->SkewGpio == 0) || (obj->SkewQueue == 0))
		return;
	for (;;)
	{
		message = (U8 *)OsReceiveMessage(obj->SkewQueue, TIMEOUT_FOREVER);
		if (message == NULL)
			continue;
		if (message[0] == 0xff)
			GpioPortWrite(obj->SkewGpio, 1);
		OsReleaseMessage(obj->SkewQueue, message);
	}
}

/* Tuner main task: see the state description at the top of the file. */
void tunerMainTask(void *param)
{
	TunerTaskArg_t               arg = *(TunerTaskArg_t *)param;
	TunerObject_t               *obj;
	const TunerDemodFunctions_t *demod = arg.Demod;
	TunerDemodInfo_t            *info = arg.DemodInfo;
	TunerResult_t                result;
	TunerSearchParam_t           search;
	TunerTuneParam_t             tuneIn;
	U32                          lnbLocal = 0;
	U32                          timeout = TIMEOUT_FOREVER;
	U32                          savedFreq = 0;
	U32                          state = STATE_IDLE;
	U8                           lossCount = 0;
	U8                          *message;
	U16                          found;
	U16                          index;
	BOOL                         changed;
	BOOL                         switchChanged;

	OsMemoryFree(param);
	if (arg.TunerIndex == 0xff)
		return;
	obj = &TunerHandle[arg.TunerIndex];

	memset(&search, 0, sizeof(search));
	memset(&tuneIn, 0, sizeof(tuneIn));
	result.SiganlState = &obj->SignalState;
	result.TuneResult  = &obj->TuneResult[0];
	tunerSetUnlocked(obj);
	memset(obj->PrevParam, 0, sizeof(TunerSearchParam_t));

	OsWaitMillisecond(1000);
	tunerLnbPower(obj->LnbGpio, 1);

	for (;;)
	{
		message = tunerMainMessageReceive(obj->MainQueue, timeout);
		if (message != NULL)
		{
			switch (message[0])
			{
				case TUNER_MSG_RESET:
					tunerReset(obj->ResetGpio);
					break;

				case TUNER_MSG_SEARCH:
					memcpy(&search, &message[4], sizeof(search));
					changed = tunerCheckTuneParam(obj, &search, &tuneIn, &lnbLocal);
					TUNER_DEBUG("search: tp %u %u MHz %c sr %u lnb %u lo %u/%u 22k %u conn %u port %u "
					            "pwr %u mode %u -> if %u lo %u changed %d state %u\n",
					            search.TpNumber, search.TpFrequency, search.HorVer ? 'H' : 'V',
					            search.Symbolrate, search.LnbType, search.LnbLocalLow,
					            search.LnbLocalHi, search.On22khz, search.ConnectionMode,
					            search.DiseqcPort, search.Power, search.SearchMode,
					            (unsigned)tuneIn.TpFrequency, (unsigned)lnbLocal, changed,
					            (unsigned)state);
					if (tuneIn.SearchMode == 0)
					{
						Cat6611Pause();
						demod->ChangeSearchMode(obj, info, 0);
						Cat6611Resume();
						switchChanged = TRUE;
					}
					else
					{
						switchChanged = tunerCheckSwitchParam(obj, &search);
					}

					if (state == STATE_IDLE)
					{
						state = STATE_SWITCH;
					}
					else if (switchChanged)
					{
						state = STATE_SWITCH;
					}
					else if (changed)
					{
						if (search.ConnectionMode == TUNER_CONECTION_UNICABLE)
						{
							state = STATE_SWITCH;
						}
						else
						{
							timeout = TIMEOUT_NONE;
							state   = STATE_LOCK;
						}
					}
					else if (state == STATE_MONITOR)
					{
						/* Same transponder and still locked: just report again. */
						if (obj->SignalState.LockState == SIGNAL_LOCK)
						{
							if (arg.CallBack != NULL)
								arg.CallBack(&result);
						}
						else
						{
							state = STATE_SWITCH;
						}
					}
					else if (state == STATE_RETRY)
					{
						state = STATE_LOCK;
					}
					break;

				case TUNER_MSG_STOP:
					tunerSetUnlocked(obj);
					obj->SignalState.LockState = SIGNAL_UNLOCK;
					state = STATE_IDLE;
					tunerConfirmSend(obj);
					break;

				case TUNER_MSG_REPORT:
					if ((state != STATE_IDLE) && (arg.CallBack != NULL))
						arg.CallBack(&result);
					break;

				case TUNER_MSG_LNB_OFF:
					tunerSetUnlocked(obj);
					tunerSetLnbOff(obj);
					state = STATE_IDLE;
					tunerConfirmSend(obj);
					break;

				default:
					break;
			}
			OsReleaseMessage(obj->MainQueue, message);
		}

		if (state != STATE_MONITOR)
		{
			lossCount = 0;
			tunerSetUnlocked(obj);
		}

		switch (state)
		{
			case STATE_IDLE:
				timeout = TIMEOUT_FOREVER;
				break;

			case STATE_SWITCH:
				timeout   = TIMEOUT_NONE;
				savedFreq = tuneIn.TpFrequency;
				if (obj->MotorMoving)
				{
					MvSendMotorMove(1);
					if (tuneIn.SearchMode == 0)
					{
						timeout             = obj->MotorWaitCount * 1000;
						obj->MotorWaitCount = 0;
					}
				}
				tunerSetSwitch(obj, &search, &tuneIn, demod);
				state = STATE_LOCK;
				break;

			case STATE_LOCK:
				*obj->PrevParam      = search;
				tuneIn.DvbType      = search.SignalType;
				tuneIn.PunctureRate = search.FecCode;
				tuneIn.RollOff      = search.RollOff;
				tuneIn.ModCode      = 0;
				tuneIn.TpNumber     = search.TpNumber;
				found = demod->LockAlgo(obj, &tuneIn, obj->TuneResult, info, tuneIn.SearchMode);
				TUNER_DEBUG("lock: if %u sr %u type %d mode %u -> %u (if %u type %d)\n",
				            (unsigned)tuneIn.TpFrequency, (unsigned)tuneIn.Symbolrate,
				            tuneIn.DvbType, tuneIn.SearchMode, found,
				            (unsigned)obj->TuneResult[0].TpFrequency, obj->TuneResult[0].DvbType);

				if (tuneIn.SearchMode == 0)
				{
					/* Blind scan finished: report the RF frequencies found. */
					if (found == 0)
					{
						obj->SignalState.BlindTpFound = 0;
						obj->SignalState.LockState    = SIGNAL_UNLOCK;
					}
					else
					{
						obj->SignalState.LockState    = SIGNAL_LOCK;
						obj->SignalState.BlindTpFound = found;
						for (index = 0; index < found; index++)
						{
							obj->TuneResult[index].TpFrequency =
								tunerGetTpFrequency(&search, obj->TuneResult[index].TpFrequency,
								                    lnbLocal);
						}
					}
					if (obj->MotorMoving)
					{
						MvSendMotorMove(0);
						obj->MotorMoving    = 0;
						obj->MotorRetry     = 0;
						obj->MotorWaitCount = 0;
					}
					Cat6611Pause();
					demod->ChangeSearchMode(obj, info, 2);
					Cat6611Resume();
					timeout = TIMEOUT_FOREVER;
					state   = STATE_IDLE;
				}
				else if (found != 0)
				{
					if (obj->Type == TUNER_SATELLITE)
					{
						obj->TuneResult[0].TpFrequency =
							tunerGetTunedFrequency(&search, tuneIn.TpFrequency,
							                       obj->TuneResult[0].TpFrequency, lnbLocal);
					}
					demod->GetSignalInfo(obj, obj->TuneResult[0].DvbType, &obj->SignalState);
					if (obj->SignalState.LockState != SIGNAL_LOCK)
					{
						timeout         = TIMEOUT_NONE;
						state           = STATE_RETRY;
						obj->MotorRetry = 0;
					}
					else if (obj->MotorMoving == 0)
					{
						state   = STATE_MONITOR;
						timeout = (search.SearchMode == 1) ? 100 : TIMEOUT_FOREVER;
					}
					else if (++obj->MotorRetry <= 2)
					{
						timeout = 500;
						state   = STATE_LOCK;
					}
					else
					{
						MvSendMotorMove(0);
						obj->MotorMoving    = 0;
						obj->MotorRetry     = 0;
						obj->MotorWaitCount = 0;
						state   = STATE_MONITOR;
						timeout = (search.SearchMode == 1) ? 100 : TIMEOUT_FOREVER;
					}
				}
				else
				{
					obj->MotorRetry = 0;
					tunerSetUnlocked(obj);
					timeout = TIMEOUT_NONE;
					state   = STATE_RETRY;
				}

				/* While the dish is moving keep retrying every 500 ms. */
				if (obj->MotorMoving)
				{
					if (obj->MotorWaitCount != 0)
					{
						obj->MotorWaitCount--;
						timeout = 500;
						state   = STATE_LOCK;
					}
					else
					{
						MvSendMotorMove(0);
						obj->MotorRetry  = 0;
						obj->MotorMoving = 0;
					}
				}
				if ((arg.CallBack != NULL) && obj->SearchEnable && (obj->MotorMoving == 0))
					arg.CallBack(&result);
				break;

			case STATE_RETRY:
				if (search.SearchMode != 1)
				{
					timeout = TIMEOUT_FOREVER;
					state   = STATE_IDLE;
				}
				else
				{
					timeout            = 100;
					tuneIn.TpFrequency = savedFreq;
					state = (search.ConnectionMode == TUNER_CONECTION_DISEQC) ? STATE_SWITCH
					                                                           : STATE_LOCK;
				}
				break;

			case STATE_MONITOR:
				demod->GetSignalInfo(obj, obj->TuneResult[0].DvbType, &obj->SignalState);
				if (obj->SignalState.LockState == SIGNAL_LOCK)
				{
					lossCount = 0;
					break;
				}
				if (++lossCount <= 3)
					break;
				/* Lost for 4 checks: report and retune. */
				lossCount = 0;
				if (arg.CallBack != NULL)
					arg.CallBack(&result);
				timeout = TIMEOUT_NONE;
				state   = STATE_RETRY;
				break;

			default:
				state   = STATE_IDLE;
				timeout = TIMEOUT_NONE;
				break;
		}
	}
}
