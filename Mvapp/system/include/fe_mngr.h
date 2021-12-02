#ifndef __FE_MNGR_H
#define __FE_MNGR_H

#include "database.h"
/* For Blind Scan By KB Kim 2011.02.26 */
#include "dvbtuner.h"

//#define     USE_DMBT
#define     USE_DVBT
//#define 	      USE_ISDB

#if defined(USE_DVBT)
#include "dvbt_tuner.h"

typedef struct
{
	tCS_DVBT_TUNER_EventType_t 	NotifyEvent;
	tCS_TUNER_Type_t 			TunerType;
}tCS_FE_Msg_t;

#elif defined(USE_DMBT)
#include "dmbt_tuner.h"

typedef struct
{
	tCS_DMBT_TUNER_EventType_t 	NotifyEvent;
	tCS_TUNER_Type_t 			TunerType;
}tCS_FE_Msg_t;

#elif defined(USE_ISDB)
#include "isdb_api.h"

typedef struct
{
	tCS_ISDB_TUNER_EventType_t 	NotifyEvent;
	tCS_ISDB_TUNER_Type_t 		TunerType;
}tCS_FE_Msg_t;
#endif


#define	kCS_FE_MODULATION_NONE   		0
#define 	kCS_FE_MODULATION_QPSK    		1
#define 	kCS_FE_MODULATION_16QAM   	2
#define 	kCS_FE_MODULATION_64QAM   	3

#define 	kCS_FE_FEC_NONE	0
#define 	kCS_FE_FEC_1_2		1
#define 	kCS_FE_FEC_2_3        	2
#define 	kCS_FE_FEC_3_4        	3
#define 	kCS_FE_FEC_5_6        	4
#define 	kCS_FE_FEC_7_8        	5


#define	kCS_FE_BW_6MHZ		6
#define	kCS_FE_BW_7MHZ		7
#define	kCS_FE_BW_8MHZ		8


#define	kCS_FE_HI_NONE		0
#define	kCS_FE_HI_ALPHA1	1
#define	kCS_FE_HI_ALPHA2	2
#define	kCS_FE_HI_ALPHA4	3

#define	kCS_FE_TM_2K		0
#define	kCS_FE_TM_8K		1

#define	kCS_FE_GI_32		0
#define	kCS_FE_GI_16		1
#define	kCS_FE_GI_8			2
#define	kCS_FE_GI_4			3

#define	kCS_FE_AUTO_PARITY		0
#define	kCS_FE_HIGH_PARITY			1
#define	kCS_FE_LOW_PARITY			2


#define kCS_FE_MAX_NO_OF_NOTIFY_CLIENTS              6

#define Ter_CH_Freq_Max_Offset						2000
#define mTerCHFreqInRange(x,y) (((x == y) || ((x >= y - Ter_CH_Freq_Max_Offset) && (x <= y + Ter_CH_Freq_Max_Offset))) ? 1:0) 

typedef  enum
{
	eCS_FE_NO_ERROR = 0,
	eCS_FE_ERROR
}tCS_FE_Error;


typedef enum 
{
	eCS_FE_LOCKED = 0,
	eCS_FE_SIGNAL_LOST,
	eCS_FE_UNLOCKED,
	eCS_FE_NONE
}tCS_FE_Notification;

typedef enum 
{
	eCS_FE_AUTO_PRIORITY = 0,
	eCS_FE_HIGH_PRIORITY,
	eCS_FE_LOW_PRIORITY
}tCS_FE_Priority;


typedef enum
{
	eCS_FE_STATE_SCANNING = 0,
	eCS_FE_STATE_LOCKED,
	eCS_FE_STATE_UNLOCKED,
	eCS_FE_STATE_NONE
}tCS_FE_State;

typedef struct
{		
	U16 		Bandwidth;
	U32		FrequencyKHz;
	tCS_FE_Priority	Priority;
#if defined(USE_DMBT)
        U8      Ter_FEC;    
        U8      Ter_Mode;
        U8      Ter_Constellation;
        U8      Ter_GuardInterval;
        U8      Ter_Force;
#endif

}tCS_FE_TerScanParams;

typedef struct
{
	U16		u16Tpnumber;
	U16		u16TPFrequency;
	U16		u16SymbolRate;
	U8		u8Polar_H;
}MV_ScanParams;

/* For Blind Scan By KB Kim 2011.02.26 */
typedef struct
{
	U8      SatIndex;
	U8      PolarH;
	U8      LnbHi;
}MV_BlindScanParams;

typedef struct
{	
	U32 		Ter_FrequencyKHz;
	U8 		Ter_BW;
	U8		Ter_FECParity;
	U8		Ter_FECHP;
	U8		Ter_FECLP;	
	U8		Ter_Mode;
	U8		Ter_Constellation;
	U8		Ter_GuardInterval;
	U8		Ter_Force;
	U8		Ter_Hierarchical;			
	U8		Ter_Signal_HPquality;	
	U8		Ter_Signal_HPlevel;
	U8		Ter_Signal_LPquality;	
	U8		Ter_Signal_LPlevel;
	U8 		Ter_HPLockStatus;
	U8 		Ter_LPLockStatus;
	S8		Ter_Offset;
} tCS_FE_TerTunerInfo;

typedef void (*tCS_FE_NotificationFunction)(tCS_FE_Notification notification);
void FE_Notify(tCS_FE_Notification notification);
tCS_FE_Error CS_FE_Register_Tuner_Notify(U8 *ClientId, tCS_FE_NotificationFunction NotifyFunction);
tCS_FE_Error CS_FE_Unregister_Tuner_Notify(U8 ClientId);
BOOL CS_FE_Init (void);

/* For Blind Scan By KB Kim 2011.02.26 */
U16 GetBlindTpResult (U8 currentPol, TunerBlindTpData_t *tpData);
U8 MV_FE_GetBlindProcess(void);
tCS_FE_Error MV_FE_SetBlindProcess (U8 process);
tCS_FE_Error CS_FE_StartBlindScan (MV_BlindScanParams blindParam);
tCS_FE_Error CS_FE_StartScan ( MV_ScanParams scanparam, unsigned char mode);
tCS_FE_Error CS_FE_StopScan (void);
tCS_FE_Error CS_FE_StopSearch (void);
tCS_FE_Error CS_FE_GetInfo (tCS_FE_TerTunerInfo *FEInfo);
void MV_FE_SetTuning_TP(MV_stSatInfo *Temp_SatData, U8 u8SatIndex, U8 u8TPIndex);

#endif 

