#include "linuxos.h"

#include "e2p_data.h"
#include "e2p.h"
#include "sys_setup.h"

#include "date_time.h"
#include "timer.h"
#include "timer_priv.h"
#include "userdefine.h"
#include "database.h"
#include "../../../middleware/include/mwsetting.h"
#include "csmpr_usb.h"

//#define TEST_ON
/*Task params*/
#define TIMER_TASK_PRIORITY		10
#define TIMER_STACK_SIZE 		1024*4
#define kTIMER_FILE_LENGTH 		(kCS_TIMER_MAX_NO_OF_JOB*(sizeof(tCS_TIMER_JobInfo)))

CSOS_Semaphore_t				* sem_Timer_Access = NULL;
CSOS_MessageQueue_t				*CS_TIMER_MsgQid = NULL;
CSOS_TaskFlag_t 				TIMER_TASK_FLAG;
CSOS_Task_Handle_t 				TIMER_TASK_HANDLE;
CSOS_TaskDesc_t 				*TIMER_TASK_DESC;
tCS_TIMER_JobInfo				sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB];
tTIMER_RecentJobAction			TIMER_Recent_JobAction;

U8								TIMER_TASK_STACK[TIMER_STACK_SIZE];
U8								DT_ModifyOffset_Notification;

static tCS_TIMER_NotifyFunction Timer_Notify = NULL;

extern void PVR_Set_Duration_Time(U16 Duration_UTC);
extern void CS_APP_SetLastUnlockServiceIndex(U16 last_service);
extern BOOL CS_MW_StopService(BOOL blank);
extern BOOL CS_MW_PlayServiceByIdx(U16 Index, U8 RetuneKind);
extern void MV_Set_CurrentGuiApplet(BOOL SetType);
extern void MV_Draw_Msg_Window(HDC hdc, U32 u32Str);
extern void Close_Msg_Window(HDC hdc);
extern void MV_Set_BootMode(SystemBootMede Bootmode);

tCS_TIMER_Error TIMER_CheckTimerExistInHW(void)
{
	tCS_TIMER_Error	found = eCS_TIMER_NO_ERROR;
	char			timer_head[5];

	memset(timer_head,0,5);
	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_TIMER_BASE, timer_head, 4);

	if(strncmp((char*)timer_head,"TIME",4) != 0)
	{
		found = eCS_TIMER_ERROR;
	}
	return(found);
}

tCS_TIMER_Error TIMER_GetJobCurrentAction( tCS_TIMER_JobInfo *job, tTIMER_JobAction *result)
{		
	U16 	local_mjd;
	U16 	local_utc;
	U32		local_time = 0;
	U32 	begin_time = 0;
	U32 	end_time = 0;
	U16		temp_utc;
	U16		temp_mjd;
	U32		timeoverflow;
	U8		current_weekday;

	if(job == NULL)
		return(eCS_TIMER_ERROR);

	if(job->CS_Timer_Status == eCS_TIMER_Enable)
	{
		local_mjd = CS_DT_GetLocalMJD();
		local_utc  = CS_DT_GetLocalUTC();

		local_time |= local_mjd;
		local_time <<= 16;
		local_time |= local_utc;

		begin_time |=  job->CS_Begin_MDJ;
		begin_time <<= 16;
		begin_time |= job->CS_Begin_UTC;
		
		if(job->CS_Timer_Type == eCS_TIMER_Duration)
		{
			temp_mjd = job->CS_Begin_MDJ;

			timeoverflow = CS_DT_UTC_Add(job->CS_Begin_UTC, job->CS_Duration_UTC);
			temp_utc = (U16)(timeoverflow & 0x0000ffff);

			timeoverflow = timeoverflow >> 24;
			
			if ( timeoverflow >0) 
			{
				temp_mjd +=  timeoverflow;
			}

			end_time |=  temp_mjd;
			end_time <<= 16;
			end_time |= temp_utc;
		}
		else
		{
			end_time = begin_time;
		}

		switch( job->CS_Timer_Cycle)
		{
			case eCS_TIMER_Onetime:				
				break;

			case eCS_TIMER_Everyday:
				while( local_time > end_time )
				{
					begin_time +=  0x00010000;		/*MJD+1*/
					end_time +=	0x00010000;		/*MJD+1*/
				}
				break;

			case eCS_TIMER_Everyweek:
				{
					current_weekday = CS_DT_CalculateWeekDay(local_mjd);

					if( current_weekday > job->CS_Begin_Weekday )
					{
						begin_time += ((7 -(current_weekday -job->CS_Begin_Weekday))<<16) ;
						end_time += ((7 -(current_weekday -job->CS_Begin_Weekday))<<16);
					}
					else
					{
						begin_time += (( job->CS_Begin_Weekday -current_weekday )<<16);
						end_time += (( job->CS_Begin_Weekday -current_weekday )<<16);
					}

					while( local_time > end_time )
					{
						begin_time += 0x00070000;		/*MJD+7*/
						end_time += 0x00070000;		/*MJD+7*/
					}
				}
				break;

		}

		result->Begin_Time = begin_time;
		result->End_Time = end_time;

		if(local_time < begin_time)
		{
			result->Recent_Action = eCS_TIMER_Begin;
		}
		else if(local_time < end_time)
		{
			result->Recent_Action = eCS_TIMER_End;
		}
		else
			return(eCS_TIMER_ERROR);
	}	
	else
	{
		return(eCS_TIMER_ERROR);
	}

	return(eCS_TIMER_NO_ERROR);
}

BOOL TIMER_CheckTimeOverlay( tCS_TIMER_JobInfo *JobA,  tCS_TIMER_JobInfo *JobB)
{
	tCS_TIMER_Error		eError;
	tTIMER_JobAction	JobA_time;
	tTIMER_JobAction	JobB_time;
	BOOL				Is_Job_Overlay = TRUE;

	if((JobA == NULL)||(JobB == NULL))
		return(TRUE);

	if((JobA->CS_Timer_Status == eCS_TIMER_Enable) && (JobB->CS_Timer_Status == eCS_TIMER_Enable))
	{
		eError = TIMER_GetJobCurrentAction(JobA, &JobA_time);
		if(eError != eCS_TIMER_NO_ERROR)
		{
			return(TRUE);
		}

		eError = TIMER_GetJobCurrentAction(JobB, &JobB_time);
		if(eError != eCS_TIMER_NO_ERROR)
		{
			return(TRUE);
		}

		if((JobB_time.Begin_Time > JobA_time.End_Time) ||(JobA_time.Begin_Time > JobB_time.End_Time))
		{
			Is_Job_Overlay = FALSE;
		}
		else
		{
			Is_Job_Overlay = TRUE;
		}
	}
	else
	{
		Is_Job_Overlay = TRUE;
	}

	return(Is_Job_Overlay);
}

tCS_TIMER_Error TIMER_LoadJobsFile(void)
{
	tCS_TIMER_Error	eError = eCS_TIMER_NO_ERROR;
   	char    		timer_head[5];

	memset(timer_head,0,5);
	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_TIMER_BASE, timer_head, 4);

	if(strncmp((char*)timer_head,"TIME",4) == 0)
	{
		E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_TIMER_BASE + 4, (U8 *)sCS_TIMER_Job, kTIMER_FILE_LENGTH);
	}
	return(eError);
}


tCS_TIMER_Error TIMER_SaveJobsFile(void)
{
	tCS_TIMER_Error	eError = eCS_TIMER_NO_ERROR;

	/* Write data into the file */
	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_TIMER_BASE, (U8 *)"TIME", 4);
	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_TIMER_BASE + 4, (U8 *)sCS_TIMER_Job, kTIMER_FILE_LENGTH);

	return(eError);
}


void TIMER_SendMessage(tTIMER_MsgType msg, U8 job)
{    
   	tTIMER_Msg_t 	*MsgSent;

	if ( (MsgSent = (tTIMER_Msg_t *) CSOS_AllocateMemory(NULL, sizeof(tTIMER_Msg_t))) != NULL )
	{		
		MsgSent->Message 	= msg;
		MsgSent->Job_Index	= job;

		CSOS_SendMessage(CS_TIMER_MsgQid, (tTIMER_Msg_t *)MsgSent, sizeof(tTIMER_Msg_t), 0 );
		CSOS_DeallocateMemory(NULL, MsgSent);
	}
}

void TIMER_DTUpdate_Callback(tCS_DT_UpdateEvent event, U16 local_mjd, U16 local_utc)
{
	U32						local_time = 0;
	U32						recent_time = 0;
	tTIMER_RecentJobAction	job_action;
	tTIMER_MsgType			msg_type;

	local_time |= local_mjd;
	local_time <<= 16;
	local_time |= local_utc;

	CSOS_WaitSemaphore(sem_Timer_Access);	

	job_action = TIMER_Recent_JobAction;

	CSOS_SignalSemaphore(sem_Timer_Access);

	if(job_action.Recent_Job.Recent_Action == eCS_TIMER_Begin)
	{
		recent_time = job_action.Recent_Job.Begin_Time;
		msg_type = eCS_TIMER_EVT_JOB_BEGIN;
	}
	else
	{
		recent_time = job_action.Recent_Job.End_Time;
		msg_type = eCS_TIMER_EVT_JOB_END;
	}

	//printf("TIMER_DTUpdate_Callback local_time = 0x%x, recent_time = 0x%x\n", local_time, recent_time);

	if(local_time == recent_time)
	{		
		/*事件发生*/
		//printf("TIMER_SendMessage msg_type = %d\n", msg_type);
		TIMER_SendMessage( msg_type, job_action.Job_Index );
	}
	else if(event == eCS_DT_MANUAL)
	{
		TIMER_SendMessage( eCS_TIMER_EVT_MANUAL_TIME, 0);
	}	
}


BOOL TIMER_CheckJobValid(U16 JobIndex)
{
	
	U16		current_mjd;
	U16		current_utc;	
	U32		current_time = 0;
	U32		job_valid_time = 0;
	U16		begin_utc;
	U16		stop_mjd;
	U16		stop_utc;
	U16		duration_utc;
	U32		time_overflow;
	BOOL	IsJobValid = TRUE;

	current_mjd = CS_DT_GetLocalMJD();
	current_utc  = CS_DT_GetLocalUTC();

	current_time |= current_mjd;
	current_time <<= 16;
	current_time |= current_utc;

	if(sCS_TIMER_Job[JobIndex].CS_Timer_Status == eCS_TIMER_Enable)
	{
		switch( sCS_TIMER_Job[JobIndex].CS_Timer_Cycle)
		{
			case eCS_TIMER_Onetime:
				{
					if(sCS_TIMER_Job[JobIndex].CS_Timer_Type == eCS_TIMER_Duration)
					{					
						begin_utc = sCS_TIMER_Job[JobIndex].CS_Begin_UTC;
						duration_utc = sCS_TIMER_Job[JobIndex].CS_Duration_UTC;

						time_overflow = CS_DT_UTC_Add(begin_utc,duration_utc);
						stop_mjd = sCS_TIMER_Job[JobIndex].CS_Begin_MDJ;
						stop_utc = (time_overflow & 0x0000FFFF);
						
						if ( time_overflow >> 24)	
						{
							stop_mjd += ( time_overflow >> 24) ;
						}
						
						job_valid_time |=  stop_mjd;
						job_valid_time <<= 16;
						job_valid_time |= stop_utc;
					}
					else
					{
						job_valid_time |= sCS_TIMER_Job[JobIndex].CS_Begin_MDJ;
						job_valid_time <<= 16;
						job_valid_time |= sCS_TIMER_Job[JobIndex].CS_Begin_UTC;
					}

					if(job_valid_time <=  current_time)
					{
						IsJobValid = FALSE;								
					}
				}
				break;
				
			default:
				break;
		}
	}
	return(IsJobValid);
}


tCS_TIMER_Error TIMER_UpdateJobList(void)
{
	U8        		i=0;
	tCS_TIMER_Error	eError = eCS_TIMER_NO_ERROR;

	for (i = 0;i < kCS_TIMER_MAX_NO_OF_JOB; i++) 
	{
		if(sCS_TIMER_Job[i].CS_Timer_Status == eCS_TIMER_Enable)
		{
			if( TIMER_CheckJobValid(i) == FALSE)
			{
#ifdef TEST_ON
				printf("===== TIMER_CheckJobValid : FALSE =========\n");
#endif
				sCS_TIMER_Job[i].CS_Timer_Status = eCS_TIMER_Disable;
				sCS_TIMER_Job[i].CS_Timer_Cycle = eCS_TIMER_Onetime;
				sCS_TIMER_Job[i].CS_Timer_Type = eCS_TIMER_Wakeup; 
				sCS_TIMER_Job[i].CS_Begin_Weekday = 0xFFFF;
				sCS_TIMER_Job[i].CS_Begin_MDJ = 0xFFFF;
				sCS_TIMER_Job[i].CS_Begin_UTC = 0xFFFF;
				sCS_TIMER_Job[i].CS_Duration_UTC= 0xFFFF;	    	
				sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
				sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Value= 0xFFFF;
				sCS_TIMER_Job[i].CS_Wakeup_Service.Service_Index = 0xFFFF;
			}
		}
	}

	eError = TIMER_SaveJobsFile();
	return(eError);    
}

tCS_TIMER_Error TIMER_UpdateRecentJobEvent(void)
{
	U16					i=0;
	U8					RecentJobIndex;
	tTIMER_JobAction	RecentJob_time;
	tTIMER_JobAction	CompJob_time;
	tCS_TIMER_Error		eError = eCS_TIMER_NO_ERROR;

	RecentJobIndex = 0;

	for (i = 1; i < kCS_TIMER_MAX_NO_OF_JOB; i++) 
	{
		if(sCS_TIMER_Job[RecentJobIndex].CS_Timer_Status == eCS_TIMER_Enable)
		{
			eError = TIMER_GetJobCurrentAction(&(sCS_TIMER_Job[RecentJobIndex]), &RecentJob_time);
			if(eError != eCS_TIMER_NO_ERROR)
			{
				RecentJob_time.Begin_Time = 0xffffffff;
			}
		}
		else
		{
			RecentJob_time.Begin_Time = 0xffffffff;
		}

		if(sCS_TIMER_Job[i].CS_Timer_Status == eCS_TIMER_Enable)
		{
			eError = TIMER_GetJobCurrentAction(&(sCS_TIMER_Job[i]), &CompJob_time);
			if(eError != eCS_TIMER_NO_ERROR)
			{
				CompJob_time.Begin_Time = 0xffffffff;
			}
		}
		else
		{
			CompJob_time.Begin_Time = 0xffffffff;
		}

		if(RecentJob_time.Begin_Time > CompJob_time.Begin_Time)
		{
			RecentJobIndex = i;
			RecentJob_time = CompJob_time;
		}
	}

	TIMER_Recent_JobAction.Job_Index = RecentJobIndex;
	TIMER_Recent_JobAction.Recent_Job = RecentJob_time;

	return(eCS_TIMER_NO_ERROR);	
}

void TIMER_EvtHandle(void* param)
{		
	tTIMER_Msg_t* 			MsgReceived;
	U8						job_index = 0;
	tCS_TIMER_JobInfo		jobinfo;
	tCS_TIMER_Notify_Type	report;
	tCS_TIMER_Service		wakeupservice;

	while(TRUE)
	{	
		printf("-----------------------------\n");
		MsgReceived = (tTIMER_Msg_t *)CSOS_ReceiveMessage(CS_TIMER_MsgQid);
		
		if(MsgReceived !=  NULL)
		{
			printf("\nCSOS_ReceiveMessage %d \n", MsgReceived->Message);
#ifdef TEST_ON
				printf("===== nCSOS_ReceiveMessage : %d =========\n", MsgReceived->Message);
#endif
			CSOS_WaitSemaphore(sem_Timer_Access);
			
			switch(MsgReceived->Message)
			{				
				case  eCS_TIMER_EVT_MANUAL_TIME:
					{
						TIMER_UpdateJobList();
						TIMER_UpdateRecentJobEvent();
					}
					break;

				case eCS_TIMER_EVT_JOB_BEGIN:
					{
						job_index = MsgReceived->Job_Index;

						//       jobinfo = sCS_TIMER_Job[0];

						jobinfo = sCS_TIMER_Job[job_index];

						if((jobinfo.CS_Timer_Type == eCS_TIMER_Duration) || (jobinfo.CS_Timer_Type == eCS_TIMER_Wakeup))
						{
							report = eCS_TIMER_NOTIFY_WAKEUP;
						}
						else if (jobinfo.CS_Timer_Type == eCS_TIMER_Record)
						{
							report = eCS_TIMER_NOTIFY_RECORD;
						}
						else
						{
							report = eCS_TIMER_NOTIFY_POWEROFF;
						}

						wakeupservice = jobinfo.CS_Wakeup_Service;

						TIMER_UpdateJobList();
						TIMER_UpdateRecentJobEvent();
#ifdef TEST_ON
						printf("===== Type : %d , Service : %d , Index : %d =========\n", report, wakeupservice.Service_Index, job_index);
#endif
						if(Timer_Notify!=NULL)
							Timer_Notify(report, wakeupservice, jobinfo.CS_Duration_UTC);
					}
					break;	
					
				case eCS_TIMER_EVT_JOB_END:
					{
						TIMER_UpdateJobList();
						TIMER_UpdateRecentJobEvent();

						/*通知上层应用*/
						report = eCS_TIMER_NOTIFY_POWEROFF;
						wakeupservice.SList_Type = eCS_TIMER_SERVICE_INVALID;

						if(Timer_Notify!=NULL)
							Timer_Notify(report, wakeupservice, job_index);
					}
					break;

			}

			CSOS_SignalSemaphore(sem_Timer_Access);
			CSOS_ReleaseMessagebuffer(CS_TIMER_MsgQid,(void *)MsgReceived);
		}

	}		
}

tCS_TIMER_Error CS_DT_Register_Timer_Notify(tCS_TIMER_NotifyFunction NotifyFunction)
{
	CSOS_WaitSemaphore(sem_Timer_Access);

	if(NotifyFunction != NULL)
	    Timer_Notify = NotifyFunction;

	CSOS_SignalSemaphore(sem_Timer_Access);

	return(eCS_TIMER_NO_ERROR);
}

tCS_TIMER_Error CS_DT_Unregister_Timer_Notify(void)
{
	CSOS_WaitSemaphore(sem_Timer_Access);

	Timer_Notify = NULL;

	CSOS_SignalSemaphore(sem_Timer_Access);

	return(eCS_TIMER_NO_ERROR);
}

tCS_TIMER_Error CS_TIMER_Reset(void)
{
#if 0
	char    cmd[40];
	sprintf(cmd, "rm /home/eeprom%d", kCS_E2P_TIMER_BASE);
	system(cmd);

	sprintf(cmd, "rm /home/eeprom%d", kCS_E2P_TIMER_BASE+4);
	system(cmd);
#else
	U8  i;
	for (i = 0; i < kCS_TIMER_MAX_NO_OF_JOB; i++)
	{
		sCS_TIMER_Job[i].CS_Timer_Status = eCS_TIMER_Disable;
		sCS_TIMER_Job[i].CS_Timer_Cycle = eCS_TIMER_Onetime;
		sCS_TIMER_Job[i].CS_Timer_Type = eCS_TIMER_Wakeup; 
		sCS_TIMER_Job[i].CS_Begin_Weekday = 0xFFFF;
		sCS_TIMER_Job[i].CS_Begin_MDJ = 0xFFFF;
		sCS_TIMER_Job[i].CS_Begin_UTC = 0xFFFF;
		sCS_TIMER_Job[i].CS_Duration_UTC= 0xFFFF;	    	
		sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
		sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Value= 0xFFFF;
		sCS_TIMER_Job[i].CS_Wakeup_Service.Service_Index = 0xFFFF;
	}

	TIMER_SaveJobsFile();
#endif
	return(eCS_TIMER_NO_ERROR);
}

BOOL CS_TIMER_Init(void)
{
	tCS_TIMER_Error		eError = eCS_TIMER_NO_ERROR;
	U8   				i=0;

	sem_Timer_Access = CSOS_CreateSemaphoreFifo (NULL, 1 );

	if((CS_TIMER_MsgQid = CSOS_CreateMessageQueue("/TIMER_MsgQid", sizeof(tTIMER_Msg_t), 20 )) == NULL)
	{
		printf("TIMER_MsgQid error!\n");
		return(FALSE);
	}

	if (CSOS_CreateTask(TIMER_EvtHandle,			/* thread entry point */
						NULL,						/* entry point argument */
						NULL,
						TIMER_STACK_SIZE,			/* size of stack in bytes */
						TIMER_TASK_STACK,			/* pointer to stack base */
						NULL,
						&TIMER_TASK_HANDLE,			/* return thread handle */
						&TIMER_TASK_DESC,			/* space to store thread data */ 
						TIMER_TASK_PRIORITY,
						"timer_handle_task",		/* name of thread */
						TIMER_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the si_monitor_task \n" );
		return(FALSE);
	}

	CSOS_StartTask(TIMER_TASK_HANDLE);

	if(TIMER_CheckTimerExistInHW() == eCS_TIMER_ERROR)
	{
		for (i = 0; i < kCS_TIMER_MAX_NO_OF_JOB; i++)
		{
			sCS_TIMER_Job[i].CS_Timer_Status = eCS_TIMER_Disable;
			sCS_TIMER_Job[i].CS_Timer_Cycle = eCS_TIMER_Onetime;
			sCS_TIMER_Job[i].CS_Timer_Type = eCS_TIMER_Wakeup; 
			sCS_TIMER_Job[i].CS_Begin_Weekday = 0xFFFF;
			sCS_TIMER_Job[i].CS_Begin_MDJ = 0xFFFF;
			sCS_TIMER_Job[i].CS_Begin_UTC = 0xFFFF;
			sCS_TIMER_Job[i].CS_Duration_UTC= 0xFFFF;	    	
			sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
			sCS_TIMER_Job[i].CS_Wakeup_Service.SList_Value= 0xFFFF;
			sCS_TIMER_Job[i].CS_Wakeup_Service.Service_Index = 0xFFFF ;
		}
	}
	else
	{   
		eError = TIMER_LoadJobsFile();
		eError = TIMER_UpdateJobList();
		eError = TIMER_UpdateRecentJobEvent();
/************************** Sleep Timer Clean *****************************/
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Timer_Status = eCS_TIMER_Disable;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Timer_Cycle = eCS_TIMER_Onetime;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Timer_Type = eCS_TIMER_Wakeup; 
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Begin_Weekday = 0xFFFF;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Begin_MDJ = 0xFFFF;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Begin_UTC = 0xFFFF;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Duration_UTC= 0xFFFF;	    	
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Wakeup_Service.SList_Value= 0xFFFF;
		sCS_TIMER_Job[kCS_TIMER_MAX_NO_OF_JOB - 1].CS_Wakeup_Service.Service_Index = 0xFFFF ;
	}

	
	eError = CS_DT_Register_DateTime_Notify(&DT_ModifyOffset_Notification, eCS_DT_MINUTE, TIMER_DTUpdate_Callback);
	eError = CS_DT_Register_Timer_Notify(MV_Timer_Notify);
	return(TRUE);
}

tCS_TIMER_Error CS_TIMER_GetJobInfo(U8 JobIndex, tCS_TIMER_JobInfo * job)
{
	if (JobIndex >= kCS_TIMER_MAX_NO_OF_JOB)
	{
		return(eCS_TIMER_ERROR);
	}
	else
	{
		CSOS_WaitSemaphore(sem_Timer_Access);
		memcpy( job, &sCS_TIMER_Job[JobIndex], sizeof (tCS_TIMER_JobInfo));
		CSOS_SignalSemaphore(sem_Timer_Access);
	}

	return(eCS_TIMER_NO_ERROR);
}

tCS_TIMER_Error CS_TIMER_CheckandSaveJobInfo( tCS_TIMER_JobInfo *job, U8 JobIndex)
{
	U16 		local_mjd;
	U16 		local_utc;
	U32		local_time = 0;
	U32		job_time = 0;
	BOOL	IsJobValid = TRUE;

	if( ( JobIndex >= kCS_TIMER_MAX_NO_OF_JOB ) || ( job == NULL ) )
	{
		printf("\n FAIL : 0 ============\n");
		return(eCS_TIMER_ERROR);
	}

	CSOS_WaitSemaphore(sem_Timer_Access);

	if( job->CS_Timer_Status == eCS_TIMER_Enable )
	{
		local_mjd = CS_DT_GetLocalMJD();
		local_utc  = CS_DT_GetLocalUTC();
		local_time |= local_mjd;
		local_time <<= 16;
		local_time |= local_utc;

		job->CS_Begin_Weekday = CS_DT_CalculateWeekDay( job->CS_Begin_MDJ );
		
		switch(job->CS_Timer_Cycle)
		{
			case eCS_TIMER_Onetime:
				local_mjd = job->CS_Begin_MDJ;
				local_utc  = job->CS_Begin_UTC;
				job_time |= local_mjd;
				job_time <<= 16;
				job_time |= local_utc;

				if (job_time <= local_time)
				{
					printf("\n FAIL : 1 ============\n");
					IsJobValid = FALSE;
				}
				break;

			case eCS_TIMER_Everyday:
				if(job->CS_Duration_UTC > 0x2400)
				{
					printf("\n FAIL : 2 ============\n");
					IsJobValid = FALSE;
				}
				break;

			case eCS_TIMER_Everyweek:
				if(job->CS_Duration_UTC > 0xA800)
				{
					printf("\n FAIL : 3 ============\n");
					IsJobValid = FALSE;
				}
				break;

			default:
				printf("\n FAIL : 4 ============\n");
				IsJobValid = FALSE;
				break;
		}


		if(IsJobValid == TRUE)
		{
			U8	index = 0;
			for(index = 0 ; index < kCS_TIMER_MAX_NO_OF_JOB ; index++)
			{
				if(index == JobIndex) continue; // if compare with itself ,pass it ! czm mod here @0800611 !

				if((sCS_TIMER_Job[index].CS_Timer_Status == eCS_TIMER_Enable) && (TIMER_CheckTimeOverlay(job, &(sCS_TIMER_Job[index])) == TRUE))
				{
					printf("\n FAIL : 5 ============\n");
					IsJobValid = FALSE;
					break;
				}
			}
		}		

	}
	else
	{
		IsJobValid = FALSE;
		//==================add by czm here 
		if(sCS_TIMER_Job[JobIndex].CS_Timer_Status == eCS_TIMER_Enable) // if change enable to the disable,we need to init the job,then write it into eeprom.
		{
			printf("****** Clear Timer %d ******************\n", JobIndex);
			job->CS_Timer_Status = eCS_TIMER_Disable;
			job->CS_Timer_Cycle = eCS_TIMER_Onetime;
			job->CS_Timer_Type = eCS_TIMER_Wakeup; 
			job->CS_Begin_Weekday = 0xFFFF;
			job->CS_Begin_MDJ = 0xFFFF;
			job->CS_Begin_UTC = 0xFFFF;
			job->CS_Duration_UTC= 0xFFFF;	    	
			job->CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
			job->CS_Wakeup_Service.SList_Value= 0xFFFF;
			job->CS_Wakeup_Service.Service_Index = 0xFFFF;
			IsJobValid = TRUE;
		}
		//==========================end add by czm here 

	}


	if(IsJobValid == TRUE)
	{		
		memcpy( &sCS_TIMER_Job[JobIndex], job, sizeof (tCS_TIMER_JobInfo));		
		TIMER_SaveJobsFile();	
		TIMER_UpdateRecentJobEvent();
	}

	CSOS_SignalSemaphore(sem_Timer_Access);

	if(IsJobValid == TRUE)
	{
		return(eCS_TIMER_NO_ERROR);
	}
	else
	{
		return(eCS_TIMER_ERROR);
	}

}


tCS_TIMER_Error CS_TIMER_GetFreeJob(U8* JobIndex)
{
	U8  i;

	CSOS_WaitSemaphore(sem_Timer_Access);

	for (i = 0; i < kCS_TIMER_MAX_NO_OF_JOB; i++)
	{
		if(sCS_TIMER_Job[i].CS_Timer_Status == eCS_TIMER_Disable)
		{
			*JobIndex = i;
			CSOS_SignalSemaphore(sem_Timer_Access);
			return(eCS_TIMER_NO_ERROR);
		}
	}

	CSOS_SignalSemaphore(sem_Timer_Access);

	return(eCS_TIMER_ERROR);
}

void MV_TIMER_Get_Last_Schadule(tCS_TIMER_JobInfo *stJob_Info)
{
	U8	i;

	CSOS_WaitSemaphore(sem_Timer_Access);

	for (i = 0; i < kCS_TIMER_MAX_NO_OF_JOB; i++)
	{
		if(sCS_TIMER_Job[i].CS_Timer_Status == eCS_TIMER_Enable && sCS_TIMER_Job[i].CS_Timer_Type != eCS_TIMER_Sleep )
		{
			if ( stJob_Info->CS_Timer_Status == eCS_TIMER_Enable )
			{
				if ( stJob_Info->CS_Begin_MDJ > sCS_TIMER_Job[i].CS_Begin_MDJ )
				{
					*stJob_Info = sCS_TIMER_Job[i];
				} else {
					if ( stJob_Info->CS_Begin_UTC > sCS_TIMER_Job[i].CS_Begin_UTC )
						*stJob_Info = sCS_TIMER_Job[i];
				}
			} else {
				*stJob_Info = sCS_TIMER_Job[i];
			}
		}
	}

	CSOS_SignalSemaphore(sem_Timer_Access);
}

void MV_Timer_Notify(tCS_TIMER_Notify_Type report, tCS_TIMER_Service service, U16 Timer_Duration)
{
	HWND						hwnd;
	HDC 						hdc;
	tCS_DB_ServiceListTriplet	ListTriplet;
	tCS_DB_ServiceManageData	item_data;
	
	hwnd = GetActiveWindow();
	
	switch(report)
	{
		case eCS_TIMER_NOTIFY_WAKEUP:
			if ( strcmp(GetWindowCaption(hwnd), "Sleep") == 0 )
			{
				if ( service.SList_Type == eCS_TIMER_SERVICE_TV || service.SList_Type == eCS_TIMER_SERVICE_TV_FAV || service.SList_Type == eCS_TIMER_SERVICE_TV_SAT )
					ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				else
					ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				
				ListTriplet.sCS_DB_ServiceListTypeValue = 0;
				CS_APP_SetLastUnlockServiceIndex(0xffff);
				CS_DB_SetCurrentList(ListTriplet, FALSE);
				CS_DB_SetCurrentService_OrderIndex(service.Service_Index);
				CS_DB_GetCurrentList_ServiceData(&item_data, service.Service_Index);
				
				BroadcastMessage (MSG_KEYDOWN, CSAPP_KEY_IDLE, 0);
			} else if ( strcmp(GetWindowCaption(hwnd), "csdesktop window") == 0 ) {
				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
				EndPaint(hwnd,hdc);

				usleep( 1000 * 1000 );
				
				ListTriplet.sCS_DB_ServiceListTypeValue = 0;
				CS_APP_SetLastUnlockServiceIndex(0xffff);
				CS_DB_SetCurrentList(ListTriplet, FALSE);
				CS_DB_SetCurrentService_OrderIndex(service.Service_Index);
				CS_DB_GetCurrentList_ServiceData(&item_data, service.Service_Index);

				CS_MW_StopService(TRUE);
				CS_MW_PlayServiceByIdx(item_data.Service_Index, RE_TUNNING);

				hdc=BeginPaint(hwnd);
				Close_Msg_Window(hdc);
				EndPaint(hwnd,hdc);
			} else {
				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
				EndPaint(hwnd,hdc);

				usleep( 1000 * 1000 );
				
				ListTriplet.sCS_DB_ServiceListTypeValue = 0;
				CS_APP_SetLastUnlockServiceIndex(0xffff);
				CS_DB_SetCurrentList(ListTriplet, FALSE);
				CS_DB_SetCurrentService_OrderIndex(service.Service_Index);
				CS_DB_GetCurrentList_ServiceData(&item_data, service.Service_Index);

				CS_MW_StopService(TRUE);
				CS_MW_PlayServiceByIdx(item_data.Service_Index, RE_TUNNING);

				hdc=BeginPaint(hwnd);
				Close_Msg_Window(hdc);
				EndPaint(hwnd,hdc);
				
				MV_Set_CurrentGuiApplet(FALSE);
			}
			MV_Set_BootMode(BOOT_NORMAL);
			break;
			
		case eCS_TIMER_NOTIFY_POWEROFF:
			if ( strcmp(GetWindowCaption(hwnd), "Sleep") != 0 )
				BroadcastMessage (MSG_KEYDOWN, CSAPP_KEY_IDLE, 0);
			break;

		case eCS_TIMER_NOTIFY_RECORD:
			{
				if ( strcmp(GetWindowCaption(hwnd), "pvr_record") != 0 && UsbCon_GetStatus() == USB_STATUS_MOUNTED )
				{
					if ( service.SList_Type == eCS_TIMER_SERVICE_TV || service.SList_Type == eCS_TIMER_SERVICE_TV_FAV || service.SList_Type == eCS_TIMER_SERVICE_TV_SAT )
						ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
					else
						ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
					
					ListTriplet.sCS_DB_ServiceListTypeValue = 0;
					CS_APP_SetLastUnlockServiceIndex(0xffff);
					CS_DB_SetCurrentList(ListTriplet, FALSE);
					CS_DB_SetCurrentService_OrderIndex(service.Service_Index);

					PVR_Set_Duration_Time(Timer_Duration);

#ifdef TEST_ON
					printf("===== CS_Duration_UTC : %d =========\n", Timer_Duration);
					printf("===== Service : %d =========\n\n", service.Service_Index);
#endif

					if ( strcmp(GetWindowCaption(hwnd), "Sleep") == 0 )
					{
						BroadcastMessage (MSG_KEYDOWN, CSAPP_KEY_IDLE, 0);
						usleep( 1000*1000 );
						MV_Set_BootMode(BOOT_TIMER);
						MV_Set_CurrentGuiApplet(TRUE);
					}
					else
					{
						CS_DB_GetCurrentList_ServiceData(&item_data, service.Service_Index);
						
						if ( strcmp(GetWindowCaption(hwnd), "csdesktop window") == 0 )
						{
							CS_MW_StopService(TRUE);
							CS_MW_PlayServiceByIdx(item_data.Service_Index, RE_TUNNING);
#ifdef TEST_ON
							printf("===== THIS IS DESCTOP =========\n", Timer_Duration);
#endif
							MV_Set_CurrentGuiApplet(TRUE);
						}
						else
						{
							CS_MW_StopService(TRUE);
							CS_MW_PlayServiceByIdx(item_data.Service_Index, RE_TUNNING);
#ifdef TEST_ON
							printf("===== THIS IS NOT DESCTOP =========\n", Timer_Duration);
#endif
							MV_Set_CurrentGuiApplet(TRUE);
						}
					}
				}
			}
			break;

		default:
			break;
	}
}


