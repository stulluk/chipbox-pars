#ifndef ISDB_ALPUS_API_H_
#define ISDB_ALPUS_API_H_

#ifdef __cplusplus
extern "C" {
#endif

 typedef enum
 {
 	CS_TUNER_EV_NO_OPERATION = 0,
	CS_TUNER_EV_LOCKED,
	CS_TUNER_EV_UNLOCKED,
	CS_TUNER_EV_WAITLOCKED
}tCS_ISDB_TUNER_EventType_t;

typedef enum
{
	CS_ISDB_TUNER_STATUS_SCANNING,
	CS_ISDB_TUNER_STATUS_LOCKED,
	CS_ISDB_TUNER_STATUS_UNLOCKED,	
	CS_ISDB_TUNER_STATUS_NOT_FOUND,
	CS_ISDB_TUNER_STATUS_IDLE
}tCS_ISDB_TUNER_Status;

typedef enum{
	TUNER_SAT = 0,  
	TUNER_TER 
}tCS_ISDB_TUNER_Type_t;

 typedef enum
{
	CS_TUNER_NO_ERROR = 0,
	CS_TUNER_ERROR_BAD_PARAMETER,			  
	CS_TUNER_ERROR_NO_MEMORY,				 
	CS_TUNER_ERROR_I2C, 		
	CS_TUNER_ERROR_TIMEOUT,  
	CS_TUNER_ERROR_OTHER
}tCS_ISDB_TUNER_Error_t;

typedef struct
{	
	U32 	FrequencyKHz;
	//tCS_DVBT_TUNER_FECParity	Priority;
}tCS_ISDB_TUNER_ScanParams;

typedef struct
{
	tCS_ISDB_TUNER_Status 			Status;
	tCS_ISDB_TUNER_ScanParams		ScanData;

	//tCS_ISDB_TUNER_FECRate	FECHP;
	//tCS_ISDB_TUNER_FECRate	FECLP;	
	//tCS_ISDB_TUNER_Mode		Mode;
	//tCS_ISDB_TUNER_Modulation	Constellation;
	//tCS_ISDB_TUNER_Guard		GuardInterval;
	//tCS_ISDB_TUNER_Force		Force;
	//tCS_ISDB_TUNER_Hierarchy	HierarchicalInfo;	
	/*BOOL	Signal_HPLock;
	BOOL	Signal_LPLock;
	U8		Signal_HPQuality;
	U8		Signal_HPLevel;
	U8		Signal_LPQuality;
	U8		Signal_LPLevel;
	S16		Freq_Offset;*/
} tCS_ISDB_TUNER_TunerInfo;


typedef void (*tCS_ISDB_NotificationFunction)(tCS_ISDB_TUNER_EventType_t event, tCS_ISDB_TUNER_Type_t tunertype);

typedef struct{
	tCS_ISDB_NotificationFunction 	NotifyFunction;
}tCS_ISDB_InitParams;


tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_Init ( tCS_ISDB_InitParams params);
tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_GetTunerInfo ( tCS_ISDB_TUNER_TunerInfo *TunerInfo);
tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_SetFrequency( tCS_ISDB_TUNER_ScanParams pScanParams );
tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_AbortScan(void);
BOOL  CS_ISDB_TUNER_IsLocked(void);
#ifdef __cplusplus
}
#endif

#endif
