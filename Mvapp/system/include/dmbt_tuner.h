#ifndef __DMBT_TUNER_H
#define __DMBT_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

//#define     original_mode

 typedef enum
{
	CS_TUNER_NO_ERROR = 0,
	CS_TUNER_ERROR_BAD_PARAMETER,			  
	CS_TUNER_ERROR_NO_MEMORY,				 
	CS_TUNER_ERROR_I2C, 		
	CS_TUNER_ERROR_TIMEOUT,  
	CS_TUNER_ERROR_OTHER
}tCS_DMBT_TUNER_Error_t;

 typedef enum
 {
 	CS_TUNER_EV_NO_OPERATION = 0,
	CS_TUNER_EV_LOCKED,
	CS_TUNER_EV_UNLOCKED,
	CS_TUNER_EV_WAITLOCKED
}tCS_DMBT_TUNER_EventType_t;


typedef enum
{
	CS_DMBT_TUNER_STATUS_SCANNING = 0,
         CS_DMBT_TUNER_STATUS_PLAYING,
	CS_DMBT_TUNER_STATUS_LOCKED,
	CS_DMBT_TUNER_STATUS_UNLOCKED,	
	CS_DMBT_TUNER_STATUS_SET_GI_420,
	CS_DMBT_TUNER_STATUS_SET_GI_945,
#ifdef  original_mode
	CS_DMBT_TUNER_STATUS_CHECK_LOCK,
#else
	CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK,
	CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK,
#endif
	CS_DMBT_TUNER_STATUS_IDLE,
	CS_DMBT_TUNER_STATUS_NONE
}tCS_DMBT_TUNER_Status;


typedef enum
{
	CS_DMBT_TUNER_FEC_NONE = 0,    
	CS_DMBT_TUNER_FEC_0_4,       
	CS_DMBT_TUNER_FEC_0_6,
	CS_DMBT_TUNER_FEC_0_8
}tCS_DMBT_TUNER_FECRate;

typedef enum
{
	CS_DMBT_TUNER_MOD_NONE = 0,
	CS_DMBT_TUNER_MOD_QPSK,
	CS_DMBT_TUNER_MOD_16QAM,
	CS_DMBT_TUNER_MOD_64QAM
}tCS_DMBT_TUNER_Modulation;

typedef enum
{
	CS_DMBT_TUNER_IL_720 = 0,
	CS_DMBT_TUNER_IL_240
}tCS_DMBT_TUNER_InterLeave;

typedef enum
{
        CS_DMBT_TUNER_GUARD_NONE = 0,
	CS_DMBT_TUNER_GUARD_420,
	CS_DMBT_TUNER_GUARD_945
}tCS_DMBT_TUNER_Guard;

typedef enum
{
	CS_DMBT_FORCE_NONE = 0,
	CS_DMBT_FORCE = 1
}tCS_DMBT_TUNER_Force;

typedef enum
{
	CS_DMBT_TUNER_HIER_NONE = 0,
	CS_DMBT_TUNER_HIER_1,
	CS_DMBT_TUNER_HIER_2,
	CS_DMBT_TUNER_HIER_4
}tCS_DMBT_TUNER_Hierarchy;

typedef enum 
{
	CS_DMBT_TUNER_INVERSION_NONE = 0,
	CS_DMBT_TUNER_INVERSION = 1
}tCS_DMBT_TUNER_Spectrum;

typedef enum
{
	CS_DMBT_OFFSET_NONE = 0,
	CS_DMBT_OFFSET = 1
}tCS_DMBT_TUNER_FreqOff;

typedef enum
{
       	CS_DMBT_PARITY_AUTO=0,
	CS_DMBT_PARITY_HIGH,
	CS_DMBT_PARITY_LOW
}tCS_DMBT_TUNER_FECParity;

typedef enum{
	TUNER_SAT = 0,  
	TUNER_TER 
}tCS_TUNER_Type_t;


typedef struct
{	
	U16 	ChanBW;
	U32 	FrequencyKHz;
    
	tCS_DMBT_TUNER_FECRate	FEC;
	tCS_DMBT_TUNER_InterLeave	IL_length;
	tCS_DMBT_TUNER_Modulation	Constellation;
	tCS_DMBT_TUNER_Guard		GuardInterval;
	tCS_DMBT_TUNER_Force		Force;
}tCS_DMBT_TUNER_ScanParams;

typedef struct
{
	//tCS_DMBT_TUNER_ScanParams   Scan_Params;
         tCS_DMBT_TUNER_Status               Set_Status;
}tCS_TUNER_Msg_t;



typedef struct
{
	tCS_DMBT_TUNER_Status 			Status;
	tCS_DMBT_TUNER_ScanParams		ScanData;

	BOOL	Signal_Lock;
	U8		Signal_Quality;
	U8		Signal_Level;
	S16		Freq_Offset;
} tCS_DMBT_TUNER_TunerInfo;

typedef void (*tCS_Tuner_NotificationFunction)(tCS_DMBT_TUNER_EventType_t event, tCS_TUNER_Type_t tunertype);

typedef struct{
	tCS_Tuner_NotificationFunction 	NotifyFunction;
}tCS_TUNER_InitParams;

tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_Init ( tCS_TUNER_InitParams params);
tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_GetTunerInfo ( tCS_DMBT_TUNER_TunerInfo *TunerInfo);
tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_SetFrequency( tCS_DMBT_TUNER_ScanParams pScanParams );
tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_AbortScan(void);
BOOL  CS_DMBT_TUNER_IsLocked(void);

tCS_DMBT_TUNER_Error_t CS_TUNER_LockLED( BOOL Locked );

#ifdef __cplusplus
}
#endif

#endif 

