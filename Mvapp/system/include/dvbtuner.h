/****************************************************************
*
* FILENAME
*	dvbtuner.h
*
* PURPOSE 
*	Common Header file for Tuner Driver Function and APIs
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/
#ifndef DVB_TUNER_H
#define DVB_TUNER_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include "mvosapi.h"
#include "mvmiscapi.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define DVB_FEC_AUTO       0

#define DVBS2_QPSK_14      1
#define DVBS2_QPSK_13      2
#define DVBS2_QPSK_25      3
#define DVBS2_QPSK_12      4
#define DVBS2_QPSK_35      5
#define DVBS2_QPSK_23      6
#define DVBS2_QPSK_34      7
#define DVBS2_QPSK_45      8
#define DVBS2_QPSK_56      9
#define DVBS2_QPSK_89     10
#define DVBS2_QPSK_910    11
#define DVBS2_8PSK_35     12
#define DVBS2_8PSK_23     13
#define DVBS2_8PSK_34     14
#define DVBS2_8PSK_56     15
#define DVBS2_8PSK_89     16
#define DVBS2_8PSK_910    17
#define DVBS2_16APSK_23   18
#define DVBS2_16APSK_34   19
#define DVBS2_16APSK_45   20
#define DVBS2_16APSK_56   21
#define DVBS2_16APSK_89   22
#define DVBS2_16APSK_910  23
#define DVBS2_32APSK_34   24
#define DVBS2_32APSK_45   25
#define DVBS2_32APSK_56   26
#define DVBS2_32APSK_89   27
#define DVBS2_32APSK_910  28

#define DVBS1_PR_1_2       1
#define DVBS1_PR_2_3       2
#define DVBS1_PR_3_4       3
#define DVBS1_PR_4_5       4		/*for turbo code only*/
#define DVBS1_PR_5_6       5
#define DVBS1_PR_6_7       6		/*for DSS only		 */
#define DVBS1_PR_7_8       7
#define DVBS1_PR_8_9       8		/*for turbo code only*/

#define DVB_FEC_UNKNOWN    0xFF

#define ROLL_OFF_35             0
#define ROLL_OFF_25             1
#define ROLL_OFF_20             2

#define TONEBURST_NONE          0
#define TONEBURST_SAT_A         1
#define TONEBURST_SAT_B         2

#define TUNER_DRIVER_NAME  20

#define TUNER_OUT_PARALLEL       0
#define TUNER_OUT_PARALLEL_CI    1
#define TUNER_OUT_SERIAL         2
#define TUNER_OUT_SERIAL_CONT    3

#define TUNER_OUT_FALLING        0
#define TUNER_OUT_RISING         4

#define TUNER_CONECTION_NONE     0
#define TUNER_CONECTION_DISEQC   1
#define TUNER_CONECTION_MOTOR    2
#define TUNER_CONECTION_USALS    3
#define TUNER_CONECTION_UNICABLE 4

#define LNB_UNIVERSAL            0
#define LNB_SINGLE               1
#define LNB_WIDE                 2
#define LNB_DIGITURK1            3
#define LNB_DIGITURK2            4
#define LNB_DIGITURK3            5
#define LNB_DIGITURK4            6

#ifdef USA_SUPPORT /* USA Support Version */
#define LNB_USA_DP21             7
#define LNB_USA_DP22             8
#define LNB_USA_DP42A            9
#define LNB_USA_DP42B            10
#define LNB_USA_DP44A            11
#define LNB_USA_DP44N            12
#endif /* #ifdef USA_SUPPORT */

/* For Blind Scan By KB Kim 2011.02.26 */
#define TUNER_MIN_FREQ           950
#define TUNER_MAX_FREQ           2150
/****************************************************************
 *                       Type define                            *
 ****************************************************************/
/* For Motor Control By KB Kim 2011.05.22 */
typedef enum
{
	MOTOR_CMD_HALT,			    /**< Stop Positioner movement , igrone input parameter*/
	MOTOR_CMD_LIMIT_OFF,		/**< Disable Limits , igrone input parameter*/
	MOTOR_CMD_LIMIT_EAST,		/**< Set East Limit , igrone input parameter*/
	MOTOR_CMD_LIMIT_WEST,		/**< Set West Limit , igrone input parameter*/
	MOTOR_CMD_DRIVE_EAST,		/**< Drive Motor East , input parameter is Steps , move step and move continue both use this command */ 
	MOTOR_CMD_DRIVE_WEST,		/**< Drive Motor West , input parameter is Steps , move step and move continue both use this command */ 
	MOTOR_CMD_STORE_POSITION,   /**< Store Satellite Position XX, input parameter is Position Number */
	MOTOR_CMD_GOTO_POSITION,	/**< Drive Motor to Satellite Position XX,, input parameter is Position Number */
	MOTOR_CMD_GOTO_REF,		    /**< Drive Motor to Reference Position , igrone input parameter*/
	MOTOR_CMD_GOTO_X,			/**< Drive Motor to a specific angle, input parameter range: -900(90.0 degree) < x <+ 900(90.0 degree) */
	MOTOR_CMD_RECALCULATION
} DistqcMotorCommand;

typedef enum{
	TUNER_SATELLITE = 0,  /* Satellite(DVB-S, S2 Tuner) */
	TUNER_CABLE,          /* Cable(DVB-C Tuner) */
	TUNER_TERRESTRIAL     /* Terrestrial(DVB-T Tuner) */
}TunerType;

typedef enum{
	TUNER_NO_ERROR = 0,  /* No error found */
	TUNER_NOT_FOUND,     /* Can not find tuner (Tuner handle error) */
	TUNER_MODE_ERROR,    /* Mode are not matched with Tuner(DVB-S, C, T ...) */
	TUNER_DATA_ERROR,    /* Tuner error found on the Parameter */
	TUNER_INIT_ERROR,    /* Can not initialize tuner */
	TUNER_OPEN_ERROR,    /* Can not open tuner driver */
	TUNER_BUSY_ERROR,    /* Tuner  part busy for previous Op */
	TUNER_SET_ERROR      /* Tuner Control error */
}TunerError;

typedef enum{
	STV0903 = 0,   /* STV0903 Demodulator */
	AVL2108,
	UNKNOWN_DEMOD 
}DemodType;

typedef enum{
	STV6110 = 0,   /* STV6110 Tuner PllDevice */
	IX2564,
	UNKNOWN_PLL 
}TunerPllType;

typedef enum{
	STREAM_AUTO = 0,
	STREAM_DVBS2,
	STREAM_DVBS1,
	STREAM_DSS,
	STREAM_S_TURBOCODE,
	STREAM_64_QAM,
	STREAM_128_QAM,
	STREAM_256_QAM,
	STREAM_OFDM,
	STREAM_UNKNOWN 
} StreamType;

typedef enum{
	SIGNAL_UNLOCK = 0,  /* Signal Unlocked */
	SIGNAL_LOCK,        /* Signal Locked and get Data */
	SIGNAL_CARRIER      /* Found Carrier but no Data (Reserved for Unicable) */
} SiginalLock;

typedef enum
{
	DVBT_PARITY_AUTO=0,
	DVBT_PARITY_HIGH,
	DVBT_PARITY_LOW
} DvbT_FECParity;

typedef struct
{
	U8              TunerNumber;
	U8              SearchMode;          /* 0 : No Auto Re-Tune Mode, 1 : Auto Retune-Mode */
	U16             TpNumber;            /* TP Number to Identify TP */
	U16             TpFrequency;         /* MHz */
	/* For Blind Scan By KB Kim 2011.02.26 */
	U16             StartFrequency;      /* MHz */
	U16             StopFrequency;       /* MHz */
	U16             Symbolrate;          /* KHz, Blind Mode : 0 ( < 1000 KHz) */
	StreamType      SignalType;          /* Auto : STREAM_AUTO, DVBS2, S1, DSS, TurboCode */
	U8              RollOff;             /* NYQUIST Roll Off Filter value */
	U8              FecCode;             /* Auto : DVB_FEC_AUTO */
	U8              Power;               /* LNB Power : DVB-S, 5V Control : Other system */
	U8              HorVer;              /* Ver : 0, Hor : 1 */
	U8              On22khz;             /* 22KHz Off : 0, 22KHz ON : 1 */
	U8              DiseqcPort;          /* None : 0, A : 1, B : 2, C : 3, D : 4, 1/16 : 5, ... 16/16 : 20 */
	U8              ToneBurst;           /* None : 0, SatA : 1, SatB : 2 */
	U8              Sw12V;               /* 0/12V Switch, 0V : 0, 12V : 1 */
	U8              SkewValue;           /* 65 <= value <= 235 : Skew Value, 0xFF : Skew OFF */

	/* LNB Type */
	U8              LnbType;             /* Universal : 0, Signle : 1, Wide : 2, DigiTurk 1 : 3, DigiTurk 2 : 4, DigiTurk 3 : 5, DigiTurk 4 : 6, */
	                                     /* USA DishPro DP21 : 7, DP22 : 8, DP42A : 9, DP42B : 10, DP44A : 11, DP44B : 12 */
	U16             LnbLocalHi;          /* LNB Local High Frequency */
	U16             LnbLocalLow;         /* LNB Local Low Frequency */

	U8              ConnectionMode;      /* None : 0, Diseqc Port : 1, Diseqc Motor : 2, USALs : 3, Unicable : 4 */
	U8              MotorPosition;       /* Diseqc Motor saved Position */
	S16             MotorAngle;          /* For USALs : Angle * 10 (ex: 10.1 -> 101) */
	/* Uni Cable Mode */
#if 0
	U8              UniCableMode;        /* None : 0, Normal mode : 0x5A, Control Mode : 0x5B */
#endif
	U8              UnicableSatPosition; /* SatA : 0, SatB : 1 */
	U8              UniCableUBand;       /* User Selected Band Number (1 ~ 8) */
	U16             UniCableBandFreq;    /* Unicable Band Frequency */	
#ifdef USA_SUPPORT /* USA Support Version */
	U8              LagacySwitch;        /* DishNet Lagacy Switch, None : 0, Twin A	: 1, Twin B	: 2, Quad A	: 3, Quad B	: 4 */
#endif /* #ifdef USA_SUPPORT */
}TunerSearchParam_t;

#if 0
typedef struct
{
	U16             Frequency;    /* MHz */
	U16             Symbole;      /* KHz */
	StreamType      SignalType;
	U8              FecCode; 
} TunerDvbCSearchParam_t;

typedef struct
{	
	U16             Frequency;    /* MHz */
	U16             Symbole;      /* KHz */
	StreamType      SignalType;
	U8              FecCode; 
}TunerDvbTSearchParam_t;

typedef union
{
	TunerDvbSSearchParam_t *DvbS;
	TunerDvbCSearchParam_t *DvbC;
	TunerDvbTSearchParam_t *DvbT;
}TunerSearchParam_t;
#endif

typedef struct
{	
	SiginalLock     LockState;    /* */
	/* For Blind Scan By KB Kim 2011.02.26 */
	U16             BlindTpFound;
	U8              Strength;     /* % */
	U8              Quality;      /* % */
}TunerSignalState_t;

/* For Blind Scan By KB Kim 2011.02.26 */
typedef struct TunerBlindTpData_s
{	
	U32				Frequency ;
	U32				SymbolRate;
	U8				Polarization;
}TunerBlindTpData_t;

typedef struct
{
	U16             TpNumber;            /* TP Number to Identify TP */
	U32             TpFrequency;         /* MHz */
	/* For Blind Scan By KB Kim 2011.02.26 */
	U32             FrequencyMin;        /* Minimum Frequency For Blind Scan : MHz */
	U32             FrequencyMax;        /* Maximum Frequency For Blind Scan : MHz */
	U32             Symbolrate;          /* KHz, Blind Mode : 0 ( < 1000 KHz) */
	StreamType      DvbType;             /* Auto : STREAM_AUTO, DVBS2, S1, DSS, TurboCode */
	U8              PunctureRate;        /* Puncture Rate , Auto : DVB_FEC_AUTO */
	U8              ModCode;             /* DVB-S2 Mode Code */
	U8              RollOff;             /* NYQUIST Roll Off Filter value */
	/* For Blind Scan By KB Kim 2011.02.26 */
	U8              SearchMode;          /* Search Mode (SEARCH_MODE_BLIND : 0 , SEARCH_MODE_COLD : 1 ,SEARCH_MODE_WARM : 2 */
} TunerTuneParam_t;

typedef struct
{	
	TunerSignalState_t   *SiganlState;    /* Signal tune status */
	TunerTuneParam_t     *TuneResult;     /* Signal tune result */
}TunerResult_t;

typedef struct
{
	U8                NumberOfTuner; /* Number Of Tuner in this system */
}TunerInitParam_t;

typedef struct
{
	U8                TunerNumber;     /* Tuner Number for multi-Tuner(first tuner : 0, ...) */
	U8                PllI2CRepeate;   /* 1 : Use Demodulator Repeate function for Pll Setting */
	U8                TunerOutputMode; /* 0 : Parallel, 1 : Parallel CI, 2 : Serial, 3 : Serial Cont */
	TunerType         Type;            /* Tuner Type (DVB-S, -C or -T) */
	DemodType         Demodulator;     /* Tuner Demodulator Type (STV0903 for current version) */
	TunerPllType      TunerPll;        /* Tuner Pll Device Type (STV6110 for current Version) */
	I2cOpenParam_t    DemodI2cParam;   /* Demodulator Device I2C Parameter */
	I2cOpenParam_t    PllI2cParam;     /* Pll Device I2C Parameter */
	GpioOpenParam_t   TunerReset;      /* GPIO Port for Tuner Reset */
	GpioOpenParam_t   LnbPower;        /* GPIO Port for LNB Power control */
	GpioOpenParam_t   HorVer;          /* GPIO Port for H/V Control */
	GpioOpenParam_t   V12;             /* GPIO Port for 0/12V Control    (Option) */
	GpioOpenParam_t   Skew;            /* GPIO Port for Skew control     (Option) */
	BOOL             (*CallBackNotify)(TunerResult_t *result);
}TunerOpenParam_t;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
TunerError TunerInit(TunerInitParam_t initParam);
TunerError TunerOpen(U32 *tunerHandleId, TunerOpenParam_t *openParam);
TunerError TunerClose(U32 tunerHandleId);
TunerError TunerTerm(void);
TunerError TunerSearchStart(U32 tunerHandleId, TunerSearchParam_t *searchData);
TunerError TunerReadSignalState(U32 tunerHandleId, TunerSignalState_t *siganlState);
/* For Blind Scan By KB Kim 2011.02.26 */
U16 TunerGetBlindTpData(U32 tunerHandleId, U8 currentPol, TunerBlindTpData_t *tpData);
TunerError TunerSendDiseqcData(U32 tunerHandleId, U8 *data, U32 len);
/* For Motor Control By KB Kim 2011.05.22 */
TunerError TunerControlMotor(U32 tunerHandleId, DistqcMotorCommand command, S16 value);
S16 TunerGetMotorAngle(int satLongitude, int myLongitude, int myLatitude);

/* For Blind Scan By KB Kim 2011.02.26 */
U8 TunerGetSearchStop(U32 tunerHandleId);
U8 TunerGetBlindProcess(U32 tunerHandleId);
TunerError TunerSetBlindProcess(U32 tunerHandleId, U8 procss);

TunerError TunerSearchStop(U32 tunerHandleId);

/* For Loop throuth contron in Sleep By KB Kim 2011.10.11 */
TunerError TunerOff(U32 tunerHandleId);

#endif /* #ifndef DVB_TUNER_H */
