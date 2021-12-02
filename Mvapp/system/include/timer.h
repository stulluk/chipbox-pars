#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "database.h"

#define kCS_TIMER_MAX_NO_OF_JOB		9

typedef enum
{
	eCS_TIMER_NO_ERROR = 0,
	eCS_TIMER_ERROR
} tCS_TIMER_Error;

typedef enum
{
	eCS_TIMER_Disable=0,
	eCS_TIMER_Enable
} tCS_TIMER_Status;


typedef enum
{
	eCS_TIMER_Wakeup=0,
	eCS_TIMER_Sleep,
	eCS_TIMER_Record,
	eCS_TIMER_Duration	
} tCS_TIMER_Type;


typedef enum
{
	eCS_TIMER_Onetime=0,
	eCS_TIMER_Everyday,
	eCS_TIMER_Everyweek	
}tCS_TIMER_Cycle;

typedef enum
{	
	eCS_TIMER_SERVICE_TV =0,							
	eCS_TIMER_SERVICE_RADIO,						
	eCS_TIMER_SERVICE_TV_FAV,
	eCS_TIMER_SERVICE_RD_FAV,
	eCS_TIMER_SERVICE_TV_SAT,
	eCS_TIMER_SERVICE_RD_SAT,
	eCS_TIMER_SERVICE_INVALID
} tCS_TIMER_ServiceListType;

typedef enum
{
	eCS_TIMER_NOTIFY_WAKEUP =0,							
	eCS_TIMER_NOTIFY_POWEROFF,
	eCS_TIMER_NOTIFY_RECORD
} tCS_TIMER_Notify_Type;

typedef struct
{
	tCS_DB_ServiceListType	SList_Type;
	U16						SList_Value;
	U16						Service_Index;
} tCS_TIMER_Service;

typedef struct
{
	tCS_TIMER_Status	CS_Timer_Status; 
	tCS_TIMER_Cycle		CS_Timer_Cycle;
	tCS_TIMER_Type		CS_Timer_Type;
	U16					CS_Begin_Weekday;
	U16					CS_Begin_MDJ;
	U16					CS_Begin_UTC;
	U16					CS_Duration_UTC; 		
	tCS_TIMER_Service	CS_Wakeup_Service;
} tCS_TIMER_JobInfo;

typedef void ( * tCS_TIMER_NotifyFunction)(tCS_TIMER_Notify_Type report, tCS_TIMER_Service service, U16 Timer_Duration);

tCS_TIMER_Error CS_DT_Register_Timer_Notify(tCS_TIMER_NotifyFunction NotifyFunction);
tCS_TIMER_Error CS_DT_Unregister_Timer_Notify(void);
tCS_TIMER_Error CS_TIMER_Reset(void);
BOOL CS_TIMER_Init(void);
tCS_TIMER_Error CS_TIMER_GetJobInfo(U8 JobIndex, tCS_TIMER_JobInfo * job);
tCS_TIMER_Error CS_TIMER_CheckandSaveJobInfo( tCS_TIMER_JobInfo  *job, U8 JobIndex);
tCS_TIMER_Error CS_TIMER_GetFreeJob(U8 * JobIndex);
void MV_Timer_Notify(tCS_TIMER_Notify_Type report, tCS_TIMER_Service service, U16 Timer_Duration);

#ifdef __cplusplus
}
#endif

#endif 


