/*
 * merih_tuner_int.h - internal definitions of the EABI re-implementation of the Merih
 * tuner library (libsystem_merih.a: Tuner.o, TunerDrv.o, Avl2108.o, Ix2564.o).
 *
 * The original library only exists as OABI objects. This port was written from their
 * disassembly (2026-10-07) and keeps the same public API (dvbtuner.h), the same
 * register sequences and the same task / message behaviour. Only the AVL2108 demodulator
 * and the IX2564 PLL (the Chipbox hardware) are implemented; STV0903 / STV6110 are not.
 */
#ifndef MERIH_TUNER_INT_H
#define MERIH_TUNER_INT_H

#include <stdio.h>

#include "mvosapi.h"
#include "mvmiscapi.h"
#include "dvbtuner.h"

/* Maximum number of transponders kept from one blind-scan step. */
#define TUNER_MAX_BLIND_TP        256

/* Messages of the tuner main task (first byte of a queue message). */
#define TUNER_MSG_RESET           0
#define TUNER_MSG_SEARCH          1
#define TUNER_MSG_STOP            2
#define TUNER_MSG_REPORT          4
#define TUNER_MSG_LNB_OFF         6

/* Size of one main-task message: type word + TunerSearchParam_t. */
#define TUNER_MSG_SIZE            (4 + sizeof(TunerSearchParam_t))

struct TunerObject_s;

/* PLL (RF tuner) driver function table (Ix2564PllFunctions). */
typedef struct
{
	TunerError (*Init)(struct TunerObject_s *obj, U32 *ifType);
	TunerError (*SetFrequency)(struct TunerObject_s *obj, U32 freqMhz, U8 bandWidth);
	TunerError (*GetFrequency)(struct TunerObject_s *obj, U32 *freqMhz);
	TunerError (*SetBandWidth)(struct TunerObject_s *obj, U8 bandWidth);
} TunerPllFunctions_t;

/* Demodulator description filled by the demodulator Init() (Avl2108Init). */
typedef struct
{
	U16                        ChipId;      /* 0x2108 */
	U8                         Reserved;
	U8                         Pad0;
	U8                         MpegMode;    /* TunerOutputMode of the open parameter */
	U8                         Pad1[3];
	U32                        RefClock;    /* PlConf key (4000 for IX2564) */
	const TunerPllFunctions_t *Pll;
} TunerDemodInfo_t;

/* Parameter block given to the demodulator Init(). */
typedef struct
{
	U32                        RefClock;
	U32                        DemodId;     /* 0x86 / 0x87 */
	U8                         MpegMode;
	U8                         Reserved0;
	U8                         Reserved1;
	U8                         Reserved2;
	U8                         PllAddress;
	U8                         Pad[3];
	const TunerPllFunctions_t *Pll;
} TunerDemodInitParam_t;

/* Demodulator driver function table (Avl2108DemodFunctions). */
typedef struct
{
	U16        (*LockAlgo)(struct TunerObject_s *obj, TunerTuneParam_t *tuneIn,
	                       TunerTuneParam_t *tuneOut, TunerDemodInfo_t *info, U8 searchMode);
	TunerError (*RepeatRead)(struct TunerObject_s *obj, U32 subAddr, U8 *data, U32 len);
	TunerError (*RepeatWrite)(struct TunerObject_s *obj, U32 subAddr, U8 *data, U32 len);
	TunerError (*SendDiseqc)(struct TunerObject_s *obj, U8 *data, U32 len);
	TunerError (*Set22K)(struct TunerObject_s *obj, U8 on);
	TunerError (*SetToneBurst)(struct TunerObject_s *obj, U8 toneBurst);
	TunerError (*GetSignalInfo)(struct TunerObject_s *obj, StreamType dvbType,
	                            TunerSignalState_t *state);
	TunerError (*ChangeSearchMode)(struct TunerObject_s *obj, TunerDemodInfo_t *info, U8 normal);
	TunerError (*Init)(struct TunerObject_s *obj, TunerDemodInitParam_t *param,
	                   TunerDemodInfo_t *info);
} TunerDemodFunctions_t;

/* One tuner (the 108-byte Merih tuner handle). */
typedef struct TunerObject_s
{
	TunerType                     Type;
	U8                            SearchEnable;     /* 0 while a stop / off is in progress */
	U8                            MotorMoving;
	U8                            MotorWaitCount;
	U8                            MotorRetry;
	U8                            BlindProcess;     /* blind scan progress 0..100 */
	U8                            InUse;
	U8                            TunerNumber;
	U8                            I2cNumber;
	U8                            DemodAddress;
	U8                            PllAddress;
	U8                            DemodSubAddrLen;
	U8                            PllSubAddrLen;
	U8                            PllRepeat;        /* PLL behind the demodulator repeater */
	U32                           MainQueue;
	U32                           SkewQueue;
	U32                           AccessSem;        /* demodulator register access */
	U32                           ConfirmSem;       /* stop / off confirmation */
	U32                           ControlSem;       /* LNB / DiSEqC control */
	U32                           DemodI2c;
	U32                           PllI2c;
	U32                           ResetGpio;
	U32                           LnbGpio;
	U32                           HorVerGpio;
	U32                           V12Gpio;
	U32                           SkewGpio;
	const TunerDemodFunctions_t  *Demod;
	TunerSearchParam_t           *PrevParam;        /* parameters applied last */
	TunerSignalState_t            SignalState;      /* read by TunerReadSignalState() */
	TunerTuneParam_t              TuneResult[TUNER_MAX_BLIND_TP];
} TunerObject_t;

/* Per tuner task bookkeeping (TunerMainObject). */
typedef struct
{
	const TunerDemodFunctions_t  *Demod;
	const TunerPllFunctions_t    *Pll;
	U32                           MainTask;
	U32                           SkewTask;
} TunerMainObject_t;

/* Argument handed to tunerMainTask(). */
typedef struct
{
	U8                            TunerIndex;
	TunerDemodInfo_t             *DemodInfo;
	const TunerDemodFunctions_t  *Demod;
	BOOL                        (*CallBack)(TunerResult_t *result);
} TunerTaskArg_t;

extern TunerObject_t              *TunerHandle;
extern TunerMainObject_t          *TunerMainObject;
extern U8                          MaxNumberOfTuner;
extern const TunerDemodFunctions_t Avl2108DemodFunctions;
extern const TunerPllFunctions_t   Ix2564PllFunctions;
extern const U8                    DemodPatchData[];
extern const U8                    BlindScanPatchData[];

/* TunerDrv.c */
U32        tunerGetBandWidth(U32 symbolRate, U8 rollOff);
U8         tunerAllocHandle(TunerOpenParam_t *openParam);
TunerError tunerI2cOpen(U32 *deviceId, I2cOpenParam_t *param);
TunerError tunerGpioOpen(U32 *deviceId, GpioOpenParam_t *param);
TunerError tunerReset(U32 gpio);
TunerError tunerLnbPower(U32 gpio, U8 on);
TunerError tunerHorVer(U32 gpio, U8 hor);
TunerError tuner12v(U32 gpio, U8 on);
TunerError tunerGetDemodResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len);
TunerError tunerSetDemodResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len);
TunerError tunerGetPllResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len);
TunerError tunerSetPllResters(TunerObject_t *obj, U32 subAddr, U8 *data, U32 len);
TunerError tunerGetPllOneResters(TunerObject_t *obj, U32 subAddr, U8 *data);
TunerError tunerSendMainMessage(U32 queue, U8 type, TunerSearchParam_t *param);
TunerError tunerControlLock(TunerObject_t *obj);
TunerError tunerControlRelease(TunerObject_t *obj);
TunerError tunerAccessLock(TunerObject_t *obj);
TunerError tunerAccessRelease(TunerObject_t *obj);
TunerError tunerConfirmReceive(TunerObject_t *obj);
U8         tunerGetDiseqcMotor(DistqcMotorCommand command, S16 value, U8 *cmd);
void       tunerMainTask(void *param);
void       tunerSkewTask(void *param);

/* Tuner.c */
U8         TunerGetSearchStop(U32 tunerHandleId);
int        tunerDebugEnabled(void);

/* Diagnostics on stdout when the environment has MERIH_TUNER_DEBUG=1. */
#define TUNER_DEBUG(...) \
	do { if (tunerDebugEnabled()) { printf("[tuner] " __VA_ARGS__); fflush(stdout); } } while (0)

#endif /* MERIH_TUNER_INT_H */
