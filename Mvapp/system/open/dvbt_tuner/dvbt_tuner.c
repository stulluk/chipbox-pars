#include "linuxos.h"

#include "tuner_drv.h"
#include "dvbt_tuner.h"

#define	TUNER_WAIT_LOCK_MAX_RETRY	5

#define DVBT_TUNER_TASK_PRIORITY		15
#define DVBT_TUNER_STACK_SIZE 		1024*4
CSOS_TaskFlag_t DVBT_TUNER_TASK_FLAG;
U8	DVBT_TUNER_TASK_STACK[DVBT_TUNER_STACK_SIZE];
CSOS_Task_Handle_t 	DVBT_TUNER_TASK_HANDLE;
CSOS_TaskDesc_t 		*DVBT_TUNER_TASK_DESC;
BOOL TUNER_Start_Tracking = TRUE;

static  CSOS_Semaphore_t 		*sem_TurnerAccess = NULL;
static  CSOS_Semaphore_t 		*sem_TurnerTimoutAccess = NULL;

U8 wait_lock_retry = 0;
tCS_Tuner_NotificationFunction	DVBT_TUNER_Notify;

tCS_DVBT_TUNER_TunerInfo		Tuner_Info;

void DVBT_TUNER_TrackTask(void * arg)
{
    	static tCS_DVBT_TUNER_EventType_t	Event = CS_TUNER_EV_NO_OPERATION;
    	static tCS_DVBT_TUNER_Status		CurrentStatus;
 
    while(TUNER_Start_Tracking)
    {
       CSOS_WaitSemaphoreTimeOut(sem_TurnerTimoutAccess, 400);
       //CSOS_DelayTaskMs(600);
       CSOS_WaitSemaphore(sem_TurnerAccess);

    	switch(Tuner_Info.Status)
    	{
              case CS_DVBT_TUNER_STATUS_SCANNING:
              	if(ZL10353_IsLocked())
                     {
                                Event=CS_TUNER_EV_LOCKED;           
                      }
                      else
	     	        {
	                        Event=CS_TUNER_EV_WAITLOCKED;
	     	        }
              	break;
              case CS_DVBT_TUNER_STATUS_LOCKED:
              	if(ZL10353_IsLocked())
                   {
                                 Event=CS_TUNER_EV_NO_OPERATION;         
                    }
                    else
                    {
                                 Event=CS_TUNER_EV_WAITLOCKED;
             	  }
              	break;
              case CS_DVBT_TUNER_STATUS_UNLOCKED:
              	if(!ZL10353_IsLocked())
                     {
                                 Event=CS_TUNER_EV_WAITLOCKED;         
                      }
                      else
                    	{
                                 Event=CS_TUNER_EV_LOCKED;
             	       } 
              	break;
              case CS_DVBT_TUNER_STATUS_NOT_FOUND:
              	if(!ZL10353_IsLocked())
                     {
                                 Event=CS_TUNER_EV_NO_OPERATION;         
                      }
                      else
                    	{
                                 Event=CS_TUNER_EV_LOCKED;
             	       } 
              	break;
              case CS_DVBT_TUNER_STATUS_IDLE:
              default:
              	Event = CS_TUNER_EV_NO_OPERATION;
              	break;
              	
    	}

    	switch (Event)
        {
        
            case CS_TUNER_EV_LOCKED: 
            	  wait_lock_retry = 0;
                CurrentStatus = CS_DVBT_TUNER_STATUS_LOCKED;
                break;

            case CS_TUNER_EV_WAITLOCKED:
            	   if(wait_lock_retry<TUNER_WAIT_LOCK_MAX_RETRY)
            	   {
                           wait_lock_retry++;
                           Event=CS_TUNER_EV_NO_OPERATION;
                           CurrentStatus = CS_DVBT_TUNER_STATUS_UNLOCKED;
            	   }
            	   else
            	   {
                            wait_lock_retry=0;
                            Event=CS_TUNER_EV_UNLOCKED;
                            CurrentStatus = CS_DVBT_TUNER_STATUS_NOT_FOUND; 
            	   }
                 break;
            case CS_TUNER_EV_UNLOCKED: 
			CurrentStatus = CS_DVBT_TUNER_STATUS_NOT_FOUND; 
            	     break;

	     case CS_TUNER_EV_NO_OPERATION:
            default:
                break;

        }   /* switch(Event) */

       Tuner_Info.Status=CurrentStatus;

	//printf("Check times is %d,   Tuner status is %d,   event is 0x%x\n",wait_lock_retry,CurrentStatus,Event);
      	/*{
      		uint8_t	tempbu;
		tempbu=ChipGetOneRegister(sZL10353.hChip, ZL10353_SNR);
		printf("ZL10353_SNR = 0x%x\n", tempbu);
      }*/
	      
       if(Event!=CS_TUNER_EV_NO_OPERATION)
       {		
             DVBT_TUNER_Notify(Event, TUNER_TER);
       }

    	CSOS_SignalSemaphore(sem_TurnerAccess);
    }
    
    return;


}


tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_Init ( tCS_TUNER_InitParams params)
{	
	memset(&Tuner_Info, 0, sizeof(tCS_DVBT_TUNER_TunerInfo));

	sem_TurnerAccess = CSOS_CreateSemaphoreFifo (NULL, 1);
	sem_TurnerTimoutAccess = CSOS_CreateSemaphoreFifo (NULL, 1);

	TUNER_Start_Tracking = TRUE;

	DVBT_TUNER_Notify = params.NotifyFunction;

         ZL10353_Init();

	if (CSOS_CreateTask(DVBT_TUNER_TrackTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						DVBT_TUNER_STACK_SIZE,				/* size of stack in bytes */
						DVBT_TUNER_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&DVBT_TUNER_TASK_HANDLE,			/* return thread handle */
						&DVBT_TUNER_TASK_DESC, 			/* space to store thread data */ 
						DVBT_TUNER_TASK_PRIORITY,
						"dvbt_tuner_monitor", 				/* name of thread */
						DVBT_TUNER_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the DVBT_TUNER_TrackTask \n" );
		return(CS_TUNER_ERROR_OTHER);
	}

	CSOS_StartTask(DVBT_TUNER_TASK_HANDLE);
	
        return(CS_TUNER_NO_ERROR);
}

tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_GetTunerInfo ( tCS_DVBT_TUNER_TunerInfo *TunerInfo)
{
	
	U16 Received_TPS = 0;

	U32 TPSFECLP;
	U32 TPSFECHP;
	U32 TPSModulation;
	U32 TPSMode;
	U32 TPSGuard;
	U32 TPSHierarchy;

	CSOS_WaitSemaphore(sem_TurnerAccess);

	if(ZL10353_IsLocked())
	{
		Received_TPS = ZL10353_GetReceivedTPS();

		if((Received_TPS & 0x8000)!= 0)
		{
			TPSMode                  = Received_TPS&0x0003;
			TPSGuard                	= (Received_TPS&0x000c)>>2;
			TPSFECLP 		= (Received_TPS&0x0070)>>4;     	
			TPSFECHP		= (Received_TPS&0x0380)>>7;
			TPSHierarchy		= (Received_TPS&0x1c00)>>10;   
			TPSModulation		= (Received_TPS&0x6000)>>13;		   

			switch(TPSModulation)
			{
			        case 0:    Tuner_Info.Constellation	= CS_DVBT_TUNER_MOD_QPSK; break;
			        case 1:    Tuner_Info.Constellation	= CS_DVBT_TUNER_MOD_16QAM; break;
			        case 2:    Tuner_Info.Constellation	= CS_DVBT_TUNER_MOD_64QAM; break;
				default: 	Tuner_Info.Constellation = CS_DVBT_TUNER_MOD_NONE; break;
			}

			switch(TPSFECHP)
			{
			       case 0:     Tuner_Info.FECHP 	= CS_DVBT_TUNER_FEC_1_2; break;
			       case 1:     Tuner_Info.FECHP 	= CS_DVBT_TUNER_FEC_2_3; break;
			       case 2:     Tuner_Info.FECHP 	= CS_DVBT_TUNER_FEC_3_4; break;
			       case 3:     Tuner_Info.FECHP 	= CS_DVBT_TUNER_FEC_5_6; break;
			       case 4:     Tuner_Info.FECHP 	= CS_DVBT_TUNER_FEC_7_8; break;
				default:    Tuner_Info.FECHP = CS_DVBT_TUNER_FEC_NONE; break;
			}
			switch(TPSFECLP)
			{
			       case 0:     Tuner_Info.FECLP 	= CS_DVBT_TUNER_FEC_1_2; break;
			       case 1:     Tuner_Info.FECLP 	= CS_DVBT_TUNER_FEC_2_3; break;
			       case 2:     Tuner_Info.FECLP 	= CS_DVBT_TUNER_FEC_3_4; break;
			       case 3:     Tuner_Info.FECLP 	= CS_DVBT_TUNER_FEC_5_6; break;
			       case 4:     Tuner_Info.FECLP 	= CS_DVBT_TUNER_FEC_7_8; break;
				default:    Tuner_Info.FECLP = CS_DVBT_TUNER_FEC_NONE; break;

			}

			Tuner_Info.Mode            		= TPSMode;
			Tuner_Info.GuardInterval		= TPSGuard;
			Tuner_Info.Force 			= CS_DVBT_FORCE_NONE;
			Tuner_Info.HierarchicalInfo 	= TPSHierarchy;
			Tuner_Info.Freq_Offset 		= 0;
		}

		if(Tuner_Info.ScanData.Priority == CS_DVBT_PARITY_LOW)
		{
			Tuner_Info.Signal_LPLock = 1;
			Tuner_Info.Signal_LPQuality = ZL10353_GetSNR();
              		Tuner_Info.Signal_LPLevel = Tuner_Info.Signal_LPQuality - sqrt(Tuner_Info.Signal_LPQuality*2);  
		}
		else
		{
			Tuner_Info.Signal_HPLock = 1;
			Tuner_Info.Signal_HPQuality = ZL10353_GetSNR();
              		Tuner_Info.Signal_HPLevel = Tuner_Info.Signal_HPQuality - sqrt(Tuner_Info.Signal_HPQuality*2); 
		}
	}
	else
	{
		if(Tuner_Info.ScanData.Priority == CS_DVBT_PARITY_LOW)
	   	{
			Tuner_Info.Signal_LPLock = 0;
			Tuner_Info.Signal_LPQuality = 0;
              		Tuner_Info.Signal_LPLevel=0;
	   	}
		else
		{
			Tuner_Info.Signal_HPLock = 0;
			Tuner_Info.Signal_HPQuality = 0;
              		Tuner_Info.Signal_HPLevel=0;
		}
	}
	
	memcpy(TunerInfo, &Tuner_Info, sizeof(tCS_DVBT_TUNER_TunerInfo));

	CSOS_SignalSemaphore(sem_TurnerAccess);

	return(CS_TUNER_NO_ERROR);
}

tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_SetFrequency( MV_ScanParams pScanParams )
{

	CSOS_WaitSemaphore(sem_TurnerAccess);
	CSOS_SignalSemaphore(sem_TurnerTimoutAccess);
	      
	 ZL10353_PartReset();
	      
	Tuner_Info.ScanData.FrequencyKHz	=	pScanParams.u16TPFrequency;
	Tuner_Info.ScanData.ChanBW		=	pScanParams.u16SymbolRate;
	Tuner_Info.ScanData.Priority		=	pScanParams.u8Polar_H;

	if(pScanParams.Priority == CS_DVBT_PARITY_LOW)
	{
		Tuner_Info.Signal_LPLock = 0;
		Tuner_Info.Signal_LPQuality = 0;
		Tuner_Info.Signal_LPLevel=0;
		ZL10353_SetTSPriority(0xc0);
	}
	else
	{
		Tuner_Info.Signal_HPLock = 0;
		Tuner_Info.Signal_HPQuality = 0;
		Tuner_Info.Signal_HPLevel =0;
		ZL10353_SetTSPriority(0x40);
	}

	//printf("CenterFrequency = %d, ChanBW = %d\n", Tuner_Info.ScanData.CenterFrequency, Tuner_Info.ScanData.ChanBW);
	if(Tuner_Info.ScanData.ChanBW == 6) 
		ZL10353_Set_CaptRange(6);
	else
		ZL10353_Set_CaptRange(2);

	ZL10353_SetBandWidth(pScanParams.ChanBW);
	ZL10353_ProgramPLL(pScanParams.FrequencyKHz, pScanParams.ChanBW);

	Tuner_Info.Status = CS_DVBT_TUNER_STATUS_SCANNING;
	wait_lock_retry = 0; 

	CSOS_SignalSemaphore(sem_TurnerAccess);

	return(CS_TUNER_NO_ERROR);
}

tCS_DVBT_TUNER_Error_t CS_DVBT_TUNER_AbortScan(void)
{
	CSOS_WaitSemaphore(sem_TurnerAccess);
       	//CSOS_SignalSemaphore(sem_TurnerTimoutAccess);
         Tuner_Info.Status = CS_DVBT_TUNER_STATUS_IDLE;
        	wait_lock_retry = 0; 
        	CSOS_SignalSemaphore(sem_TurnerAccess);
	return(CS_TUNER_NO_ERROR);
}

BOOL  CS_DVBT_TUNER_IsLocked(void)
{
         return (ZL10353_IsLocked());
}

