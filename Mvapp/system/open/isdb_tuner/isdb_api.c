#include "linuxos.h"

#include "isdb_i2capi.h"
#include "isdb_drv.h"
#include "isdb_api.h"

#define	ISDB_WAIT_LOCK_MAX_RETRY	3

#define ISDB_TUNER_TASK_PRIORITY		15
#define ISDB_TUNER_STACK_SIZE 		1024*4
CSOS_TaskFlag_t ISDB_TUNER_TASK_FLAG;
U8	ISDB_TUNER_TASK_STACK[ISDB_TUNER_STACK_SIZE];
CSOS_Task_Handle_t 	ISDB_TUNER_TASK_HANDLE;
CSOS_TaskDesc_t 		*ISDB_TUNER_TASK_DESC;

static BOOL ISDB_Start_Tracking = TRUE;

static  CSOS_Semaphore_t 		*sem_ISDBAccess = NULL;
static  CSOS_Semaphore_t 		*sem_ISDBTimoutAccess = NULL;

tCS_ISDB_TUNER_TunerInfo		ISDB_Info;

U8 Isdb_wait_lock_retry = 0;
tCS_ISDB_NotificationFunction	ISDB_TUNER_Notify;

static void ISDB_Init(void);
static BOOL ISDB_IsLocked(void);

void ISDB_TUNER_TrackTask(void * arg)
{
    	static tCS_ISDB_TUNER_EventType_t	Event = CS_TUNER_EV_NO_OPERATION;
    	static tCS_ISDB_TUNER_Status		CurrentStatus;
 
    while(ISDB_Start_Tracking)
    {
       CSOS_WaitSemaphoreTimeOut(sem_ISDBTimoutAccess, 400);
       //CSOS_DelayTaskMs(600);
       CSOS_WaitSemaphore(sem_ISDBAccess);

    	switch(ISDB_Info.Status)
    	{
              case CS_ISDB_TUNER_STATUS_SCANNING:
              	if(ISDB_IsLocked())
                     {
                                Event=CS_TUNER_EV_LOCKED;           
                      }
                      else
	     	        {
	                        Event=CS_TUNER_EV_WAITLOCKED;
	     	        }
              	break;
              case CS_ISDB_TUNER_STATUS_LOCKED:
              	if(ISDB_IsLocked())
                   {
                                 Event=CS_TUNER_EV_NO_OPERATION;         
                    }
                    else
                    {
                                 Event=CS_TUNER_EV_WAITLOCKED;
             	  }
              	break;
              case CS_ISDB_TUNER_STATUS_UNLOCKED:
              	if(!ISDB_IsLocked())
                     {
                                 Event=CS_TUNER_EV_WAITLOCKED;         
                      }
                      else
                    	{
                                 Event=CS_TUNER_EV_LOCKED;
             	       } 
              	break;
              case CS_ISDB_TUNER_STATUS_NOT_FOUND:
              	if(!ISDB_IsLocked())
                     {
                                 Event=CS_TUNER_EV_NO_OPERATION;         
                      }
                      else
                    	{
                                 Event=CS_TUNER_EV_LOCKED;
             	       } 
              	break;
              case CS_ISDB_TUNER_STATUS_IDLE:
              default:
              	Event = CS_TUNER_EV_NO_OPERATION;
              	break;
              	
    	}

    	switch (Event)
        {
        
            case CS_TUNER_EV_LOCKED: 
            	  Isdb_wait_lock_retry = 0;
                CurrentStatus = CS_ISDB_TUNER_STATUS_LOCKED;
                break;

            case CS_TUNER_EV_WAITLOCKED:
            	   if(Isdb_wait_lock_retry<ISDB_WAIT_LOCK_MAX_RETRY)
            	   {
                           Isdb_wait_lock_retry++;
                           Event=CS_TUNER_EV_NO_OPERATION;
                           CurrentStatus = CS_ISDB_TUNER_STATUS_UNLOCKED;
            	   }
            	   else
            	   {
                            Isdb_wait_lock_retry=0;
                            Event=CS_TUNER_EV_UNLOCKED;
                            CurrentStatus = CS_ISDB_TUNER_STATUS_NOT_FOUND; 
            	   }
                 break;
            case CS_TUNER_EV_UNLOCKED: 
			CurrentStatus = CS_ISDB_TUNER_STATUS_NOT_FOUND; 
            	     break;

	     case CS_TUNER_EV_NO_OPERATION:
            default:
                break;

        }   /* switch(Event) */

       ISDB_Info.Status=CurrentStatus;

	//printf("Check times is %d,   Tuner status is %d,   event is 0x%x\n",wait_lock_retry,CurrentStatus,Event);
      	/*{
      		uint8_t	tempbu;
		tempbu=ChipGetOneRegister(sZL10353.hChip, ZL10353_SNR);
		printf("ZL10353_SNR = 0x%x\n", tempbu);
      }*/
	      
       if(Event!=CS_TUNER_EV_NO_OPERATION)
       {		
             ISDB_TUNER_Notify(Event, TUNER_TER);
       }

    	CSOS_SignalSemaphore(sem_ISDBAccess);
    }
    
    return;


}



tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_Init ( tCS_ISDB_InitParams params)
{	
	memset(&ISDB_Info, 0, sizeof(tCS_ISDB_TUNER_TunerInfo));

	sem_ISDBAccess = CSOS_CreateSemaphoreFifo (NULL, 1);
	sem_ISDBTimoutAccess = CSOS_CreateSemaphoreFifo (NULL, 1);

	ISDB_Start_Tracking = TRUE;

	ISDB_TUNER_Notify = params.NotifyFunction;

         ISDB_Init();

	if (CSOS_CreateTask(ISDB_TUNER_TrackTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						ISDB_TUNER_STACK_SIZE,				/* size of stack in bytes */
						ISDB_TUNER_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&ISDB_TUNER_TASK_HANDLE,			/* return thread handle */
						&ISDB_TUNER_TASK_DESC, 			/* space to store thread data */ 
						ISDB_TUNER_TASK_PRIORITY,
						"dvbt_tuner_monitor", 				/* name of thread */
						ISDB_TUNER_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the DVBT_TUNER_TrackTask \n" );
		return(CS_TUNER_ERROR_OTHER);
	}

	CSOS_StartTask(ISDB_TUNER_TASK_HANDLE);
	
        return(CS_TUNER_NO_ERROR);
}


tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_GetTunerInfo ( tCS_ISDB_TUNER_TunerInfo *TunerInfo)
{
	
/*	U16 Received_TPS = 0;

	U32 TPSFECLP;
	U32 TPSFECHP;
	U32 TPSModulation;
	U32 TPSMode;
	U32 TPSGuard;
	U32 TPSHierarchy;*/

	CSOS_WaitSemaphore(sem_ISDBAccess);

       //

	CSOS_SignalSemaphore(sem_ISDBAccess);

	return(CS_TUNER_ERROR_TIMEOUT);
}


tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_SetFrequency( tCS_ISDB_TUNER_ScanParams pScanParams )
{

	CSOS_WaitSemaphore(sem_ISDBAccess);
	CSOS_SignalSemaphore(sem_ISDBTimoutAccess);
	      
	      
	ISDB_Info.ScanData.FrequencyKHz	=	pScanParams.FrequencyKHz;
	


	MOPIC_SendData(ISDB_Info.ScanData.FrequencyKHz,TC90507_CONFIG_DIGITAL);

	ISDB_Info.Status = CS_ISDB_TUNER_STATUS_SCANNING;
        	Isdb_wait_lock_retry = 0; 

	CSOS_SignalSemaphore(sem_ISDBAccess);

	return(CS_TUNER_NO_ERROR);
}



tCS_ISDB_TUNER_Error_t CS_ISDB_TUNER_AbortScan(void)
{
	CSOS_WaitSemaphore(sem_ISDBAccess);
       	//CSOS_SignalSemaphore(sem_TurnerTimoutAccess);
         ISDB_Info.Status = CS_ISDB_TUNER_STATUS_IDLE;
        	Isdb_wait_lock_retry = 0; 
        	CSOS_SignalSemaphore(sem_ISDBAccess);
	return(CS_TUNER_NO_ERROR);
}

BOOL  CS_ISDB_TUNER_IsLocked(void)
{
         return (ISDB_IsLocked());
}

static BOOL ISDB_IsLocked(void)
{
	TC90507_ucRSERROR_t pStatus;
	int iRetVal=ALPS_I2C_ERR_NON;
	
	iRetVal=TC90507_Get_RSErrorStatus(&pStatus);
	if(iRetVal!=ALPS_I2C_ERR_NON) return FALSE;
	
	if(pStatus.ucStatus_LayerA==TC90507_RS_NONERROR||pStatus.ucStatus_LayerB==TC90507_RS_NONERROR\
		||pStatus.ucStatus_LayerB==TC90507_RS_NONERROR)
		{
			return TRUE;
		}

	//printf("ucStatus_LayerA=%d ucStatus_LayerB=%d ucStatus_LayerC=%d\n",pStatus.ucStatus_LayerA,pStatus.ucStatus_LayerB,pStatus.ucStatus_LayerC);
	return FALSE;
}

static void ISDB_Init(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	printf("ISDB_Init....\n");
	TC90507_gpio_reset();
	iRetVal=TC90507_I2C_OpenPort();

	if(ALPS_i2c_init()==0)
		{
			printf("ALPS_i2c_init success...\n");
		}
	else
		{
			printf("ALPS_i2c_init fail\n");
		}

	//TC90507_Set_MOPICType(TC90507_MOP_TDA6651);
	//TC90507_Set_IFType(TC90507_IF_44M);

	//TC90507_ucBERType=TC90507_BER_AFTER_VITERBI;
	//TC90507_ucTSOutputLayer=TC90507_TS_OUTPUT_LAYER_ALL;

	
	iRetVal|=TC90507_Set_RegInit();
	TC90507_Set_SleepMode(TC90507_DISABLE);
	TC90507_Set_BERMornitorType(TC90507_CONFIG_DIGITAL);
	Set_TSOutputLayer(TC90507_TS_OUTPUT_LAYER_ALL);
	
	iRetVal|=TC90507_Set_ChannelMode(TC90507_CH_SELECTION_MODE);

	TC90507_Set_SFQBERMeasurement(TC90507_ENABLE);

	//TC90507_ucActiveMode=TC90507_CONFIG_DIGITAL;	
	//TC90507_uiBypassFreqkHz=509143;

	//iRetVal|=MOPIC_SendData();


	if (iRetVal==ALPS_I2C_ERR_NON)
		printf("\n - I2C Status : OK");
	else
		printf("\n - I2C Status : ERROR");
}

