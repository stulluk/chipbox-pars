
#include "linuxos.h"

#include "lgs8913.h"
#include "dmbt_tuner.h"

//#define         test_dmbt_tuner

#define	TUNER_CHECK_GI_MAX_RETRY	                2
#define	TUNER_WAIT_MAN_LOCK_MAX_RETRY	        (5-1)
#define	TUNER_WAIT_AUTO_DONE_MAX_RETRY	3
#define	SIGNAL_PARAM_CHANGE_MAX_RETRY	        (2-1)
#define     kCS_TUNER_MAX_MSG			20


#define DMBT_TUNER_TASK_PRIORITY		14
#define DMBT_TUNER_STACK_SIZE 		1024*8
CSOS_TaskFlag_t DMBT_TUNER_TASK_FLAG;
U8	DMBT_TUNER_TASK_STACK[DMBT_TUNER_STACK_SIZE];
CSOS_Task_Handle_t 	DMBT_TUNER_TASK_HANDLE;
CSOS_TaskDesc_t 		*DMBT_TUNER_TASK_DESC;
BOOL TUNER_Start_Tracking = TRUE;

static  CSOS_Semaphore_t 		*sem_TurnerAccess = NULL;

static CSOS_MessageQueue_t			*CS_TUNER_MsgQid = NULL;

static  U8   check_gi_retry = 0;
static U8 wait_auto_lock_retry = 0;

static tCS_Tuner_NotificationFunction	DMBT_TUNER_Notify;

static tCS_DMBT_TUNER_TunerInfo		Tuner_Info;

static BOOL    Is_scan_msg_sent = FALSE;

static tCS_DMBT_TUNER_Status  current_set_status = CS_DMBT_TUNER_STATUS_NONE;


void DMBT_TUNER_TrackTask(void * arg)
{
    	static tCS_DMBT_TUNER_EventType_t	Event = CS_TUNER_EV_NO_OPERATION;
    	static tCS_DMBT_TUNER_Status		CurrentStatus;
        U8  locked, registerData;
        S8 err = LGS_NO_ERROR;
        tCS_TUNER_Msg_t			*msgReceived;

        static  U8 pcontrolFrame;
        static  U8 pmode;
        static  U8 prate;
        static  U8 pinterleaverLength;
        static U8 pguardInterval;
        static  U32     freq = 474000;
        static   tCS_DMBT_TUNER_EventType_t pre_event = CS_TUNER_EV_NO_OPERATION;
        
        static  U32 check_lock_count = 0;
        static  U32 unlock_count = 0;
        
 
    while(TUNER_Start_Tracking)
    {

        CSOS_DelayTaskMs(300);

       CSOS_WaitSemaphore(sem_TurnerAccess);

        if(current_set_status != CS_DMBT_TUNER_STATUS_NONE)
         {
            Tuner_Info.Status = current_set_status;
            CurrentStatus = current_set_status;
            current_set_status = CS_DMBT_TUNER_STATUS_NONE;
            freq = Tuner_Info.ScanData.FrequencyKHz;
            switch(Tuner_Info.ScanData.FEC)
                {
                    case CS_DMBT_TUNER_FEC_0_4:
                            prate = RX_FEC_0_4;
                            break;
                    
                    case CS_DMBT_TUNER_FEC_0_6:
                            prate = RX_FEC_0_6;
                            break;
                    
                    case CS_DMBT_TUNER_FEC_0_8:
                            prate = RX_FEC_0_8;
                            break;
                            
                    default:
                            prate = RX_FEC_0_6;
                            break;  
                }

              switch(Tuner_Info.ScanData.Constellation)
                {
                    case CS_DMBT_TUNER_MOD_64QAM:
                            pmode = RX_SC_QAM64;
                            break;
                    
                    case CS_DMBT_TUNER_MOD_16QAM:
                            pmode = RX_SC_QAM16;
                            break;
                    
                    case CS_DMBT_TUNER_MOD_QPSK:
                            pmode = RX_SC_QAM4;
                            break;
                            
                    default:
                            pmode = RX_SC_QAM64;
                            break;  
                }

            switch(Tuner_Info.ScanData.GuardInterval)
                {
                    case CS_DMBT_TUNER_GUARD_420:
                            pguardInterval = RX_GI_420;
                            break;
                    
                    case CS_DMBT_TUNER_GUARD_945:
                            pguardInterval = RX_GI_945;
                            break;
                            
                    default:
                            pguardInterval = RX_GI_945;
                            break;  
                }

            switch(Tuner_Info.ScanData.IL_length)
                {
                    case CS_DMBT_TUNER_IL_720:
                            pinterleaverLength = RX_TIM_LONG;
                            break;
                    
                    case CS_DMBT_TUNER_IL_240:
                            pinterleaverLength = RX_TIM_MIDDLE;
                            break;
                            
                    default:
                            pinterleaverLength = RX_TIM_LONG;
                            break;  
                }
            }

       CSOS_SignalSemaphore(sem_TurnerAccess);

       //printf("Tuner_Info.Status = %d, freq = %d\n", Tuner_Info.Status, freq);
       
    	switch(Tuner_Info.Status)
    	{
              case CS_DMBT_TUNER_STATUS_SCANNING:
                    printf("CS_DMBT_TUNER_STATUS_SCANNING, frequency = %d\n", freq);
                    LGS_StartTuner( freq/1000 );

	     	        {
	     	            check_gi_retry = 0;
                            wait_auto_lock_retry = TUNER_WAIT_AUTO_DONE_MAX_RETRY;
                            pre_event = CS_TUNER_EV_NO_OPERATION;
                                CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK;//CS_DMBT_TUNER_STATUS_CHECK_LOCK;
                                Event=CS_TUNER_EV_WAITLOCKED;
	     	        }
              	break;
                
              case  CS_DMBT_TUNER_STATUS_PLAYING:
                    printf("CS_DMBT_TUNER_STATUS_PLAYING, frequency = %d, param[%x, %x, %x, %x]\n", freq, pmode, prate, pinterleaverLength, pguardInterval);
                    LGS_StartTuner( freq/1000 );
                    {
                            LGS_WriteRegister (0x03, 00);
                            LGS_WriteRegister(0x04, pguardInterval);
                            LGS_SetManualMode( pmode, prate, pinterleaverLength, pguardInterval );
                            //LGS_SoftReset();
                            
	     	            check_gi_retry = 0;
                            wait_auto_lock_retry = 0;
                            pre_event = CS_TUNER_EV_NO_OPERATION;
                                CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK;//CS_DMBT_TUNER_STATUS_CHECK_LOCK;
                                Event=CS_TUNER_EV_NO_OPERATION;
                                check_lock_count = 0;
                                unlock_count = 0;
                        }
                    break;
                
              case  CS_DMBT_TUNER_STATUS_SET_GI_420:
                {
                        LGS_WriteRegister (0x03, 00);
                        LGS_WriteRegister(0x04, RX_GI_420);
		    LGS_SetAutoMode();
                        LGS_SoftReset();
                        pguardInterval = RX_GI_420;
                        CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK;
                        Event=CS_TUNER_EV_NO_OPERATION;
                        wait_auto_lock_retry = 0;

                }
              break;

              case  CS_DMBT_TUNER_STATUS_SET_GI_945:
                {
                    LGS_WriteRegister (0x03, 00);
                    LGS_WriteRegister(0x04, RX_GI_945);
		  LGS_SetAutoMode();
                    LGS_SoftReset();
                    pguardInterval = RX_GI_945;
                     CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK;
                     Event=CS_TUNER_EV_NO_OPERATION;
                     wait_auto_lock_retry = 0;
                }
              break;

              case  CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK:
                {
                              err = LGS_AutoDetectDone(&locked);
                            if ((err == LGS_NO_ERROR) && (locked == 1))
                                {
                    		//printf("LGS_AutoDetectDone() done\n");
                    		err = LGS_CheckLocked(&locked);
                    		if ((err == LGS_NO_ERROR) && (locked == 1))
                    		{	
                    		        LGS_GetAutoParameters(&pcontrolFrame, &pmode, &prate, &pinterleaverLength);
                                            LGS_SetManualMode( pmode, prate, pinterleaverLength, pguardInterval );
                                            //LGS_SoftReset();
                                            CSOS_WaitSemaphore(sem_TurnerAccess);
                                            switch(prate)
                                                {
                                                    case RX_FEC_0_4:
                                                            Tuner_Info.ScanData.FEC = CS_DMBT_TUNER_FEC_0_4;
                                                            break;
                                                    
                                                    case RX_FEC_0_6:
                                                            Tuner_Info.ScanData.FEC = CS_DMBT_TUNER_FEC_0_6;
                                                            break;
                                                    
                                                    case RX_FEC_0_8:
                                                            Tuner_Info.ScanData.FEC = CS_DMBT_TUNER_FEC_0_8;
                                                            break;
                                                            
                                                    default:
                                                            Tuner_Info.ScanData.FEC = CS_DMBT_TUNER_FEC_NONE;
                                                            break;  
                                                }

                                            switch(pmode)
                                                {
                                                    case RX_SC_QAM64:
                                                            Tuner_Info.ScanData.Constellation= CS_DMBT_TUNER_MOD_64QAM;
                                                            break;
                                                    
                                                    case RX_SC_QAM16:
                                                            Tuner_Info.ScanData.Constellation = CS_DMBT_TUNER_MOD_16QAM;
                                                            break;
                                                    
                                                    case RX_SC_QAM4:
                                                            Tuner_Info.ScanData.Constellation = CS_DMBT_TUNER_MOD_QPSK;
                                                            break;
                                                            
                                                    default:
                                                            Tuner_Info.ScanData.Constellation = CS_DMBT_TUNER_FEC_NONE;
                                                            break;  
                                                }

                                            switch(pguardInterval)
                                                {
                                                    case RX_GI_420:
                                                            Tuner_Info.ScanData.GuardInterval= CS_DMBT_TUNER_GUARD_420;
                                                            break;
                                                    
                                                    case RX_GI_945:
                                                            Tuner_Info.ScanData.GuardInterval = CS_DMBT_TUNER_GUARD_945;
                                                            break;
                                                            
                                                    default:
                                                            Tuner_Info.ScanData.GuardInterval = CS_DMBT_TUNER_GUARD_NONE;
                                                            break;  
                                                }

                                            switch(pinterleaverLength)
                                                {
                                                    case RX_TIM_LONG:
                                                            Tuner_Info.ScanData.IL_length= CS_DMBT_TUNER_IL_720;
                                                            break;
                                                    
                                                    case RX_TIM_MIDDLE:
                                                            Tuner_Info.ScanData.IL_length = CS_DMBT_TUNER_IL_240;
                                                            break;
                                                            
                                                    default:
                                                            Tuner_Info.ScanData.IL_length = CS_DMBT_TUNER_IL_720;
                                                            break;  
                                                }
                                            
                                            CSOS_SignalSemaphore(sem_TurnerAccess);
                                            
                    			Event=CS_TUNER_EV_NO_OPERATION;
                                            CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK; 
                                            check_lock_count = 0;
                                            unlock_count = 0;
                    		}
                                    else
                                    {
                                            //printf("LGS_AutoDetectDone not lock\n");
                                            Event=CS_TUNER_EV_WAITLOCKED;
                                            CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK; 
                                    }
                                }
                            else
                            {
                                    //printf("LGS_AutoDetectDone not done\n");
                                    Event=CS_TUNER_EV_WAITLOCKED;
                                    CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK; 
                            }
		}
              break;
              
              case CS_DMBT_TUNER_STATUS_LOCKED:
              	if(CS_DMBT_TUNER_IsLocked())
                   {
                                 Event=CS_TUNER_EV_NO_OPERATION;
                    }
                    else
                    {
                                 Event=CS_TUNER_EV_NO_OPERATION;
                                 CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK; 
                                 check_lock_count = 0;
                                 unlock_count = 0;
             	  }
              	break;
                
              case CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK:
              case CS_DMBT_TUNER_STATUS_UNLOCKED:
              	if(!CS_DMBT_TUNER_IsLocked())
                     {
                                 Event=CS_TUNER_EV_NO_OPERATION;
                                 CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_MANUAL_LOCK; 
                                 unlock_count++;
                      }
                    check_lock_count++;

                    if(check_lock_count>=TUNER_WAIT_MAN_LOCK_MAX_RETRY)
                        {
                            
                            if(unlock_count >=SIGNAL_PARAM_CHANGE_MAX_RETRY)
                            {
                                Event=CS_TUNER_EV_WAITLOCKED;
                                 CurrentStatus = CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK;
                                 wait_auto_lock_retry = TUNER_WAIT_AUTO_DONE_MAX_RETRY;
                            }
                            else
                            {
                                Event=CS_TUNER_EV_LOCKED;
                            }

                            check_lock_count = 0;
                            unlock_count = 0;
                        }
                     
              	break;
                
              case CS_DMBT_TUNER_STATUS_IDLE:
              default:
              	Event = CS_TUNER_EV_NO_OPERATION;
              	break;
              	
    	}

    	switch (Event)
        {
        
            case CS_TUNER_EV_LOCKED: 
                  wait_auto_lock_retry = 0;
                  check_gi_retry = 0;
                  CurrentStatus = CS_DMBT_TUNER_STATUS_LOCKED;
                break;

            case CS_TUNER_EV_WAITLOCKED:
                    if(CS_DMBT_TUNER_STATUS_CHECK_AUTO_LOCK == CurrentStatus)
                        {
                                if(wait_auto_lock_retry < TUNER_WAIT_AUTO_DONE_MAX_RETRY)
                                    {
                                        wait_auto_lock_retry++;
                                        Event=CS_TUNER_EV_NO_OPERATION;
                                    }
                                else
                                    {
                                        wait_auto_lock_retry = 0;

                                        if(check_gi_retry%2==0)
                                                CurrentStatus = CS_DMBT_TUNER_STATUS_SET_GI_945;
                                            else
                                                CurrentStatus = CS_DMBT_TUNER_STATUS_SET_GI_420;
                                            
                                        if(check_gi_retry<TUNER_CHECK_GI_MAX_RETRY)
                                        {
                                            
                                           check_gi_retry++;
                                            Event=CS_TUNER_EV_NO_OPERATION;
                                        }
                                     else
                            	   {
                                            check_gi_retry = 0;
                                            Event=CS_TUNER_EV_UNLOCKED;
                            	   }
                                    }
                        }
            	   
                 break;
            case CS_TUNER_EV_UNLOCKED: 
			CurrentStatus = CS_DMBT_TUNER_STATUS_UNLOCKED; 
            	     break;

	     case CS_TUNER_EV_NO_OPERATION:
            default:
                break;

        }   /* switch(Event) */

       Tuner_Info.Status=CurrentStatus;

	//printf("Check times is %d,   Tuner status is %d,   event is 0x%x\n",wait_auto_lock_retry,CurrentStatus,Event);
      	/*{
      		uint8_t	tempbu;
		tempbu=ChipGetOneRegister(sZL10353.hChip, ZL10353_SNR);
		printf("ZL10353_SNR = 0x%x\n", tempbu);
      }*/
	      
       if(Event!=CS_TUNER_EV_NO_OPERATION)
       {		
            //printf("DMBT_TUNER_Notify, Event = %d\n", Event);
             DMBT_TUNER_Notify(Event, TUNER_TER);
       }

    	//CSOS_SignalSemaphore(sem_TurnerAccess);

        //printf("222222222222222222222222\n");
    }
    
    return;


}


static int _gpio_write(char *devname, char* buf, int len )
{
	int gpio_fd;
	int retval;
	char cmd='O';

	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Error: Open %s.\n",devname);
		return -1;
	}
	
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}

	retval = write(gpio_fd, buf , len);
	if (retval != len)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}
/*
	cmd= 'o';
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}
*/

    retval = close(gpio_fd);

	return len;
    
}


tCS_DMBT_TUNER_Error_t CS_TUNER_HWReset(void)
{
#if defined(USE_SHARP_TUNER)
	int  retval = 0;
	char value = '0';
	// retval = _gpio_write("/dev/gpio/6",&value,1);
	usleep(40*1000);
	value = '1';
	// retval = _gpio_write("/dev/gpio/6", &value,1);
	usleep(40*1000);
#endif
	return 0;	
}


tCS_DMBT_TUNER_Error_t CS_TUNER_LockLED( BOOL Locked )
{
	int  retval = 0;	
	char value;

	if( Locked == TRUE )
	{
		value = '1';		
	}
	else
	{		
		value = '0';
	}
	retval = _gpio_write("/dev/gpio/0",&value,1);
	return 0;	
}



tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_Init ( tCS_TUNER_InitParams params)
{	
	memset(&Tuner_Info, 0, sizeof(tCS_DMBT_TUNER_TunerInfo));

	sem_TurnerAccess = CSOS_CreateSemaphoreFifo (NULL, 1);
	//sem_TurnerTimoutAccess = CSOS_CreateSemaphoreFifo (NULL, 0);

	TUNER_Start_Tracking = TRUE;

	DMBT_TUNER_Notify = params.NotifyFunction;

    

    /*该函数由机顶盒厂商实现*/
    CS_TUNER_HWReset();

    /*设置LGS芯片的I2C基地址*/
    LGS_DemodulatorBaseAddress(0, 0, 0);
    
    /* 初始化I2C 总线*/
    tuner_i2c_init();
    
    /*设置LGS芯片的MPEG输出格式*/
    LGS_SetMpegMode(0, 2, 0) ;

    /*软件重启*/
    LGS_SoftReset();

#if 1
         if((CS_TUNER_MsgQid = CSOS_CreateMessageQueue("/TUNER_MsgQid",sizeof(tCS_TUNER_Msg_t), kCS_TUNER_MAX_MSG )) == NULL)
	{
	        printf("create TUNER_MsgQid error\n");
		return(CS_TUNER_ERROR_OTHER);
	}
         
	if (CSOS_CreateTask(DMBT_TUNER_TrackTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						DMBT_TUNER_STACK_SIZE,				/* size of stack in bytes */
						DMBT_TUNER_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&DMBT_TUNER_TASK_HANDLE,			/* return thread handle */
						&DMBT_TUNER_TASK_DESC, 			/* space to store thread data */ 
						DMBT_TUNER_TASK_PRIORITY,
						"dvbt_tuner_monitor", 				/* name of thread */
						DMBT_TUNER_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the DMBT_TUNER_TrackTask \n" );
		return(CS_TUNER_ERROR_OTHER);
	}

	CSOS_StartTask(DMBT_TUNER_TASK_HANDLE);
#endif

#ifdef  test_dmbt_tuner
    while(1)
        {
                U8 controlFrame; 
                U8 mode; 
                U8 rate; 
                U8 interleaverLength;
                U8 guardInterval;
                U8  locked;
                U8      k = 0;
        
                mode = RX_SC_QAM16;
                rate = RX_FEC_0_6;
                interleaverLength = RX_TIM_MIDDLE;
                guardInterval = RX_GI_945;

                printf("---------------set freq 666MHz------------------\n");
                
            LGS_StartTuner( 666 );
                LGS_WriteRegister (0x03, 00);
                LGS_WriteRegister(0x04, RX_GI_945);
                LGS_SetManualMode( mode, rate, interleaverLength, guardInterval );
                //LGS_SoftReset();
                
                for(k = 0; k<10; k++)
                    {
        
                        CSOS_DelayTaskMs(300);
                        //LGS_AutoDetectDone(&locked);
                            //printf("LGS_AutoDetectDone = %d\n", locked);
                        if(CS_DMBT_TUNER_IsLocked())
                            {
                                printf("TUNER LOCKED!!\n");
                            }
                        else
                            {
                                
                                printf("TUNER NOT LOCKED!!\n");
                            }
                        
                     }
               
                
                mode = RX_SC_QAM16;
                rate = RX_FEC_0_8;
                interleaverLength = RX_TIM_LONG;
                guardInterval = RX_GI_420;

                printf("---------------set freq 474MHz------------------\n");
                
            LGS_StartTuner( 474 );
                LGS_WriteRegister (0x03, 00);
                //LGS_WriteRegister(0x04, RX_GI_420);
                //LGS_SetAutoMode();
                //LGS_SoftReset();

                k = 0;
        
                while(1)
                    {
                        
                        S8 err = LGS_NO_ERROR;
                    U8 locked, registerData;
                        
                        {
                                    
                            LGS_WriteRegister(0x04, RX_GI_420);
                            LGS_SetAutoMode();
                                    LGS_SoftReset();
                            LGS_Wait(300);
                            err = LGS_AutoDetectDone(&locked);
                            if ((err == LGS_NO_ERROR) && (locked == 1))
                            {   
                                
                                err = LGS_CheckLocked(&locked);
                                if ((err == LGS_NO_ERROR) && (locked == 1))
                                {   
                                    printf("LGS_CheckLocked()hhhaa  420 locked\n");
                                    guardInterval = RX_GI_420;
                                    break;
                                }
                            }   
                            
                            LGS_WriteRegister(0x04, RX_GI_945);
                            LGS_SetAutoMode();
                                    LGS_SoftReset();
                            LGS_Wait(300);  
                            err = LGS_AutoDetectDone(&locked);
                            if ((err == LGS_NO_ERROR) && (locked == 1))
                            {                           
                                err = LGS_CheckLocked(&locked);
                                if ((err == LGS_NO_ERROR) && (locked == 1))
                                {   
                                    printf("LGS_CheckLocked()hhhaa  945 locked\n");
                                    guardInterval = RX_GI_945;
                                    break;
                                }
                                
                            }
        
                                    k++;
                        }
                    }
        
                LGS_GetAutoParameters(&controlFrame, &mode, &rate, &interleaverLength);
                LGS_SetManualMode( mode, rate, interleaverLength, guardInterval );
                //LGS_SoftReset();
                    printf("loop = %d, pmode = %d, prate = %d, pinterleaverLength = %d, pguardInterval = %d\n",k, mode, rate, interleaverLength, guardInterval);
                 
        
                //while(1)
                for(k = 0; k<10; k++)
                    {
        
                        CSOS_DelayTaskMs(300);
                        //LGS_AutoDetectDone(&locked);
                            //printf("LGS_AutoDetectDone = %d\n", locked);
                        if(CS_DMBT_TUNER_IsLocked())
                            {
                                printf("TUNER LOCKED!!\n");
                            }
                        else
                            {
                                
                                printf("TUNER NOT LOCKED!!\n");
                            }
                        
                     }
        }
#endif

        return(CS_TUNER_NO_ERROR);
}

tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_GetTunerInfo ( tCS_DMBT_TUNER_TunerInfo *TunerInfo)
{
	
	U16 Received_TPS = 0;

	U32 TPSFECLP;
	U32 TPSFECHP;
	U32 TPSModulation;
	U32 TPSMode;
	U32 TPSGuard;
	U32 TPSHierarchy;

	CSOS_WaitSemaphore(sem_TurnerAccess);

	if(CS_DMBT_TUNER_IsLocked())
	{
		Tuner_Info.Signal_Lock = 1;
		Tuner_Info.Signal_Quality = 60;//LGS_ReadBER();//ZL10353_GetSNR();
                  Tuner_Info.Signal_Level = Tuner_Info.Signal_Quality - sqrt(Tuner_Info.Signal_Quality*2);
	   	}
		else
		{
        
		Tuner_Info.Signal_Lock = 0;
		Tuner_Info.Signal_Quality = 0;//LGS_ReadBER();//ZL10353_GetSNR();
                  Tuner_Info.Signal_Level = 0;

                  Tuner_Info.ScanData.FEC = CS_DMBT_TUNER_FEC_NONE;
                  Tuner_Info.ScanData.IL_length = CS_DMBT_TUNER_IL_720;
                  Tuner_Info.ScanData.Constellation = CS_DMBT_TUNER_MOD_NONE;
                  Tuner_Info.ScanData.GuardInterval = CS_DMBT_TUNER_GUARD_NONE;
	}

	memcpy(TunerInfo, &Tuner_Info, sizeof(tCS_DMBT_TUNER_TunerInfo));

	CSOS_SignalSemaphore(sem_TurnerAccess);

	return(CS_TUNER_NO_ERROR);
}

tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_SetFrequency( tCS_DMBT_TUNER_ScanParams pScanParams )
{

    U8 controlFrame; 
    U8 mode; 
    U8 rate; 
    U8 interleaverLength;
    U8 guardInterval;
    static tCS_TUNER_Msg_t tuner_msg;

    //printf("CS_DMBT_TUNER_SetFrequency %d\n", CS_OS_time_now());

	CSOS_WaitSemaphore(sem_TurnerAccess);
	      
	Tuner_Info.ScanData.FrequencyKHz	=	pScanParams.FrequencyKHz;
	Tuner_Info.ScanData.ChanBW		    =	pScanParams.ChanBW;
         Tuner_Info.ScanData.Force                      =       pScanParams.Force;

         Tuner_Info.ScanData.FEC                        =       pScanParams.FEC;
         Tuner_Info.ScanData.IL_length                  =       pScanParams.IL_length;
         Tuner_Info.ScanData.Constellation          =       pScanParams.Constellation;
         Tuner_Info.ScanData.GuardInterval          =       pScanParams.GuardInterval;

         if(Tuner_Info.ScanData.Force  == TRUE)
                current_set_status = CS_DMBT_TUNER_STATUS_PLAYING;
         else
                current_set_status = CS_DMBT_TUNER_STATUS_SCANNING;


        CSOS_SignalSemaphore(sem_TurnerAccess);
	return(CS_TUNER_NO_ERROR);
}

tCS_DMBT_TUNER_Error_t CS_DMBT_TUNER_AbortScan(void)
{
#if 0
        tCS_TUNER_Msg_t tuner_msg;

	CSOS_WaitSemaphore(sem_TurnerAccess);
        wait_manual_lock_retry = 0; 
        
        tuner_msg.Scan_Params.ChanBW = Tuner_Info.ScanData.ChanBW;
        tuner_msg.Scan_Params.FrequencyKHz = Tuner_Info.ScanData.FrequencyKHz;
        tuner_msg.Scan_Params.Priority = Tuner_Info.ScanData.Priority;
        tuner_msg.Set_Status = CS_DMBT_TUNER_STATUS_IDLE;
        
        CSOS_SendMessage(CS_TUNER_MsgQid, &tuner_msg, sizeof(tCS_TUNER_Msg_t), 0);

        CSOS_SignalSemaphore(sem_TurnerAccess);
 #endif   
	return(CS_TUNER_NO_ERROR);
}

BOOL  CS_DMBT_TUNER_IsLocked(void)
{
    U8 Locked = 0;
    
    if( LGS_CheckLocked( &Locked ) == 0 )
    {
        //printf("Locked = %d\n", Locked);
        if( Locked )
        {
            return(TRUE);
        }
        else
        {
            return(FALSE);
        }
            
    }
    
    return (FALSE);
}

#if 0
int SetTunerForTest()
{
        U8 controlFrame; 
        U8 mode; 
        U8 rate; 
        U8 interleaverLength;
        U8 guardInterval;
        
        LGS_StartTuner( 666 );
        LGS_WriteRegister (0x03, 00);
        LGS_WriteRegister(0x04, RX_GI_420);
        LGS_SetManualMode( RX_SC_QAM16, RX_FEC_0_6, RX_TIM_LONG, RX_GI_420 );

        LGS_Wait(200);
        
        LGS_StartTuner( 482 );
	LGS_LoopAutoDetect( &controlFrame, &mode, &rate, &interleaverLength, &guardInterval );
	LGS_SetManualMode( mode, rate, interleaverLength, guardInterval );
        //LGS_WriteRegister (0x03, 00);
        //LGS_WriteRegister(0x04, RX_GI_945);
        //LGS_SetManualMode( RX_SC_QAM64, RX_FEC_0_6, RX_TIM_LONG, RX_GI_945 );
        
        LGS_SoftReset();
        LGS_Wait(200);

        while(1)
            {

                CSOS_DelayTaskMs(200);
                //LGS_AutoDetectDone(&locked);
                    //printf("LGS_AutoDetectDone = %d\n", locked);
                if(CS_DMBT_TUNER_IsLocked())
                    {
                        printf("TUNER LOCKED!!\n");
                        break;
                    }
                else
                    {
                        
                        printf("TUNER NOT LOCKED!!\n");
                    }
                
             }
        
}
#endif

