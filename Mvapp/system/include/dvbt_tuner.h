#ifndef __DVBT_TUNER_H
#define __DVBT_TUNER_H

#ifdef __cplusplus
extern "C" {
#endif

 typedef enum
{
	CS_TUNER_NO_ERROR = 0,
	CS_TUNER_ERROR_BAD_PARAMETER,			  
	CS_TUNER_ERROR_NO_MEMORY,				 
	CS_TUNER_ERROR_I2C, 		
	CS_TUNER_ERROR_TIMEOUT,  
	CS_TUNER_ERROR_OTHER
}tCS_DVBT_TUNER_Error_t;

 typedef enum
 {
 	CS_TUNER_EV_NO_OPERATION = 0,
	CS_TUNER_EV_LOCKED,
	CS_TUNER_EV_UNLOCKED,
	CS_TUNER_EV_WAITLOCKED
}tCS_DVBT_TUNER_EventType_t;


typedef enum
{
	CS_DVBT_TUNER_STATUS_SCANNING,
	CS_DVBT_TUNER_STATUS_LOCKED,
	CS_DVBT_TUNER_STATUS_UNLOCKED,	
	CS_DVBT_TUNER_STATUS_NOT_FOUND,
	CS_DVBT_TUNER_STATUS_IDLE
}tCS_DVBT_TUNER_Status;


typedef enum
{
	CS_DVBT_TUNER_FEC_NONE = 0,    
	CS_DVBT_TUNER_FEC_1_2,       
	CS_DVBT_TUNER_FEC_2_3,
	CS_DVBT_TUNER_FEC_3_4,
	CS_DVBT_TUNER_FEC_5_6,
	CS_DVBT_TUNER_FEC_7_8
}tCS_DVBT_TUNER_FECRate;

typedef enum
{
	CS_DVBT_TUNER_MOD_NONE = 0,
	CS_DVBT_TUNER_MOD_QPSK,
	CS_DVBT_TUNER_MOD_16QAM,
	CS_DVBT_TUNER_MOD_64QAM
}tCS_DVBT_TUNER_Modulation;

typedef enum
{
	CS_DVBT_TUNER_MODE_2K = 0,
	CS_DVBT_TUNER_MODE_8K
}tCS_DVBT_TUNER_Mode;

typedef enum
{
	CS_DVBT_TUNER_GUARD_1_32 = 0,
	CS_DVBT_TUNER_GUARD_1_16,
	CS_DVBT_TUNER_GUARD_1_8,
	CS_DVBT_TUNER_GUARD_1_4
}tCS_DVBT_TUNER_Guard;

typedef enum
{
	CS_DVBT_FORCE_NONE = 0,
	CS_DVBT_FORCE = 1
}tCS_DVBT_TUNER_Force;

typedef enum
{
	CS_DVBT_TUNER_HIER_NONE = 0,
	CS_DVBT_TUNER_HIER_1,
	CS_DVBT_TUNER_HIER_2,
	CS_DVBT_TUNER_HIER_4
}tCS_DVBT_TUNER_Hierarchy;

typedef enum 
{
	CS_DVBT_TUNER_INVERSION_NONE = 0,
	CS_DVBT_TUNER_INVERSION = 1
}tCS_DVBT_TUNER_Spectrum;

typedef enum
{
	CS_DVBT_OFFSET_NONE = 0,
	CS_DVBT_OFFSET = 1
}tCS_DVBT_TUNER_FreqOff;

typedef enum
{
       	CS_DVBT_PARITY_AUTO=0,
	CS_DVBT_PARITY_HIGH,
	CS_DVBT_PARITY_LOW
}tCS_DVBT_TUNER_FECParity;

typedef enum{
	TUNER_SAT = 0,  
	TUNER_TER 
}tCS_TUNER_Type_t;


typedef struct
{	
	U16 	ChanBW;
	U32 	FrequencyKHz;
	tCS_DVBT_TUNER_FECParity	Priority;
}tCS_DVBT_TUNER_ScanParams;


typedef struct
{
	tCS_DVBT_TUNER_Status 			Status;
	tCS_DVBT_TUNER_ScanParams		ScanData;

	tCS_DVBT_TUNER_FECRate	FECHP;
	tCS_DVBT_TUNER_FECRate	FECLP;	
	tCS_DVBT_TUNER_Mode		Mode;
	tCS_DVBT_TUNER_Modulation	Constellation;
	tCS_DVBT_TUNER_Guard		GuardInterval;
	tCS_DVBT_TUNER_Force		Force;
	tCS_DVBT_TUNER_Hierarchy	HierarchicalInfo;	
	BOOL	Signal_HPLock;
	BOOL	Signal_LPLock;
	U8		Signal_HPQuality;
	U8		Signal_HPLevel;
	U8		Signal_LPQuality;
	U8		Signal_LPLevel;
	S16		Freq_Offset;
} tCS_DVBT_TUNER_TunerInfo;

typedef void (*tCS_Tuner_NotificationFunction)(tCS_DVBT_TUNER_EventType_t event, tCS_TUNER_Type_t tunertype);

typedef struct{
	tCS_Tuner_NotificationFunction 	NotifyFunction;
}tCS_TUNER_InitParams;

tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_Init ( tCS_TUNER_InitParams params);
tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_GetTunerInfo ( tCS_DVBT_TUNER_TunerInfo *TunerInfo);
//tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_SetFrequency( MV_ScanParams pScanParams );
tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_AbortScan(void);
BOOL  CS_DVBT_TUNER_IsLocked(void);


#ifdef __cplusplus
}
#endif

#endif 

