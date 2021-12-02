/*******************************************************************************

File name   : linuxos.c

Description : xport HAL (Hardware Abstraction xport) access to hardware source file

Copyright (C) 2007 Celestial

References  :

create 	2007.09.26			: Pinanhai - weiyu.Luo  :
 
*******************************************************************************/
#include "linuxos.h"




/*******************************************************************************
Name        : CSOS_CreateMessageQueue
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : pointer to the allocated Message queue or NULL (if some errors are occued)
*******************************************************************************/
CSOS_MessageQueue_t *CSOS_CreateMessageQueue(const char *Qname,unsigned int ElementSize, unsigned int NoElements)
{


	CSOS_MessageQueue_t * MessQ_handle;
	struct mq_attr mqstate; 
	
	mqstate.mq_maxmsg = NoElements;
	mqstate.mq_msgsize = ElementSize;
	mqstate.mq_flags =O_RDWR|O_CREAT;
	mqstate.mq_curmsgs = 0;
//	printf("CSOS_CreateMessageQueue-------------------------IN\n");
	MessQ_handle = (CSOS_MessageQueue_t *) mq_open(Qname,O_RDWR |O_CREAT ,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, &mqstate);
//	printf("MessQ_handle = 0x%x",MessQ_handle);
	if ((mqd_t) MessQ_handle ==  (mqd_t)(-1)) 
	{	
		printf("CSOS_CreateMessageQueue--------MessQ_handle == NULL11111111\n");
		return NULL;
	}
	else 
	{
//		printf("CSOS_CreateMessageQueue-------------------------OUT\n");//
		return MessQ_handle;
	}

}

/*******************************************************************************
Name        : CSOS_CreateMessageQueue
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_NO_ERROR,  CS_ERROR_MESSAGE_CLOSE, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
int CSOS_DeleteMessageQueue(CSOS_MessageQueue_t * MsgQueue_p)
{

	if (MsgQueue_p == NULL) 	
	{
		return 0;
	}

	if (mq_close((mqd_t)MsgQueue_p) == 0)	
		return 1;
	else return 0;
	

}


/*******************************************************************************
Name        : CSOS_ReleaseMessagbuffer
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_NO_ERROR
*******************************************************************************/
int CSOS_ReleaseMessagebuffer(CSOS_MessageQueue_t * MsgQueue_p, void * Msg_p)
{

	free(Msg_p);
	return 1;

}



/*******************************************************************************
Name        : CSOS_ReceiveMessage
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : pointer to the allocated memory
*******************************************************************************/
 void *CSOS_ReceiveMessage(CSOS_MessageQueue_t * MsgQueue_p)
{

	void * Msg_buffer;
	struct mq_attr mqstat;

//	printf("CSOS_ReceiveMessage-------------------> IN\n");
	if (mq_getattr((mqd_t)MsgQueue_p, &mqstat) == -1)
	{
//		printf("CSOS_ReceiveMessage----------mq_getattr((mqd_t)MsgQueue_p, &mqstat)--------> IN\n");
		return NULL;
	}

	
	Msg_buffer = malloc(mqstat.mq_msgsize );
	if(Msg_buffer != NULL)
        {   
	
            	if(mq_receive((mqd_t)MsgQueue_p,Msg_buffer, mqstat.mq_msgsize, NULL) == -1)
            	{
                            free(Msg_buffer);
            		return NULL;
            	}

       }
//	printf("CSOS_ReceiveMessage-------------------> OUT\n");
	
	return Msg_buffer;
	

}

 void *CSOS_ReceiveMessageTimeOut(CSOS_MessageQueue_t * MsgQueue_p, unsigned int  TimeOutMs)
{

	void * Msg_buffer;
	struct mq_attr mqstat;
         struct timespec  time_value;

//	printf("CSOS_ReceiveMessage-------------------> IN\n");
	if (mq_getattr((mqd_t)MsgQueue_p, &mqstat) == -1)
	{
//		printf("CSOS_ReceiveMessage----------mq_getattr((mqd_t)MsgQueue_p, &mqstat)--------> IN\n");
		return NULL;
	}

	
	Msg_buffer = malloc(mqstat.mq_msgsize );
        if(Msg_buffer != NULL)
            {
                clock_gettime(CLOCK_REALTIME, &time_value);
                time_value.tv_sec += TimeOutMs/1000;
                if((time_value.tv_nsec/1000000 + TimeOutMs%1000)>=1000)
                    {
                        time_value.tv_sec += 1;
                        time_value.tv_nsec -= (1000-(TimeOutMs%1000))*1000000;
                    }
                else
                    {
                        time_value.tv_nsec += (TimeOutMs%1000)*1000000;
                    }
        	
            	if(mq_timedreceive((mqd_t)MsgQueue_p,Msg_buffer, mqstat.mq_msgsize, NULL, &time_value) == -1)
            	{
                              free(Msg_buffer);
            		return NULL;
            	}
            }
//	printf("CSOS_ReceiveMessage-------------------> OUT\n");
	
	return Msg_buffer;
	

}


/*******************************************************************************
Name        : CSOS_SendMessage
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
 int CSOS_SendMessage(CSOS_MessageQueue_t * MsgQueue_p,void * Msg_p,unsigned int Msg_Size, int Priority)
{


     struct mq_attr mqstat;
          
     if (mq_getattr((mqd_t)MsgQueue_p, &mqstat) == -1) 
     {
	  return 0;
     }

     if (mq_send((mqd_t)MsgQueue_p,(const char *) Msg_p,Msg_Size,Priority) != 0) 
     {
     #if 0
            int err;

            err = errno;

            switch(err)
                {
                    case EAGAIN:
                        printf("The queue was empty, and the O_NONBLOCK flag was set for the message queue description referred to by mqdes.\n");
                        break;

                    case EBADF:
                        printf("The descriptor specified in mqdes was invalid. \n");
                        break;
                        
                    case EMSGSIZE:
                        printf("msg_len was greater than the mq_msgsize attribute of the message queue. \n");
                        break;

                    case EINTR:
                        printf("The call was interrupted by a signal handler. \n");
                        break;

                    case EINVAL:
                        printf("The call would have blocked, and abs_timeout was invalid, either because tv_sec was less than zero, or because tv_nsec was less than zero or greater than 1000 million. \n");
                        break;

                    case ETIMEDOUT:
                        printf("The call timed out before a message could be transferred.\n");
                        break;

                    default:
                        printf("other error!\n");
                        break;
                }
            #endif
	  return 0;
            
     }
     
	return 1;
	
}


/*******************************************************************************
Name        : CSOS_CreateSemaphoreFifo
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/

CSOS_Semaphore_t * CSOS_CreateSemaphoreFifo(CSOS_Partition_t * Partition_p, const int InitialValue)
{
	CSOS_Semaphore_t * Semaphore_p;
        int     err;


	Semaphore_p = (CSOS_Semaphore_t *) CSOS_AllocateMemory(NULL,sizeof(CSOS_Semaphore_t));

	if (Semaphore_p != NULL)
	{
		err = sem_init(Semaphore_p, 0,InitialValue);

	}

	return Semaphore_p;
}



/*******************************************************************************
Name        : CSOS_DeleteSemaphore
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t  CSOS_DeleteSemaphore(CSOS_Partition_t * Partition_p, CSOS_Semaphore_t * Semaphore_p)
{
	if (Semaphore_p != NULL)
	{
		sem_destroy(Semaphore_p);
		
	}
	else
	{

		return CS_ERROR_BAD_PARAMETER;
	}

	return CS_NO_ERROR;
	
}

/*******************************************************************************
Name        : CSOS_SignalSemaphore
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_SignalSemaphore(CSOS_Semaphore_t * Semaphore_p)
{

	if(Semaphore_p != NULL)
	{
		sem_post(Semaphore_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;


}

/*******************************************************************************
Name        : CSOS_WaitSemaphore
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t  CSOS_WaitSemaphore(CSOS_Semaphore_t * Semaphore_p)
{
	if(Semaphore_p != NULL)
	{
		return ( sem_wait(Semaphore_p) == 0 )? CS_NO_ERROR:CS_ERROR_FAILD_GET_SEMAPHORE;
	}
	return CS_ERROR_FAILD_GET_SEMAPHORE;
}

/*******************************************************************************
Name        : CSOS_WaitSemaphoreTimeOut
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/

CS_ErrorCode_t  CSOS_WaitSemaphoreTimeOut(CSOS_Semaphore_t * Semaphore_p, unsigned int TimeOutMs)
{
         struct timespec  time_value;

	if(Semaphore_p != NULL)
	{
		clock_gettime(CLOCK_REALTIME, &time_value);
                    time_value.tv_sec += TimeOutMs/1000;
                    if((time_value.tv_nsec/1000000 + TimeOutMs%1000)>=1000)
                        {
                            time_value.tv_sec += 1;
                            time_value.tv_nsec -= (1000-(TimeOutMs%1000))*1000000;
                        }
                    else
                        {
                            time_value.tv_nsec += (TimeOutMs%1000)*1000000;
                        }
                    //CSOS_WaitSemaphoreTimeOut(sem_test_Access, 2000);
                    //printf("%d.%d\n", time_value.tv_sec,  time_value.tv_nsec/1000000);
                    if(sem_timedwait (Semaphore_p, &time_value)==0)
                        {
                            return CS_NO_ERROR;
                        }
		
	}
	return CS_ERROR_FAILD_GET_SEMAPHORE;


}

/*******************************************************************************
Name        : CSOS_GetSemaphoreValue
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_GetSemaphoreValue(CSOS_Semaphore_t * Semaphore_p, int *sval)
{
	if(Semaphore_p != NULL)
	{
		sem_getvalue(Semaphore_p,sval);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;

}

/*******************************************************************************
Name        : CSOS_EnterCriticalSection
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_EnterCriticalSection(CSOS_Semaphore_t *Lock_p)
{
	if(Lock_p != NULL)
	{
		sem_wait(Lock_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;


}

/*******************************************************************************
Name        : CSOS_ExitCriticalSection
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_ExitCriticalSection(CSOS_Semaphore_t *Lock_p)
{

	if(Lock_p != NULL)
	{
		sem_post(Lock_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;

}




/*******************************************************************************
Name        : CSOS_CreateMutexFifo
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CSOS_Mutex_t * CSOS_CreateMutexFifo(void)
{
	CSOS_Mutex_t* Mutex_p;

	Mutex_p = (CSOS_Mutex_t *) CSOS_AllocateMemory(NULL,sizeof(CSOS_Mutex_t));

	if (Mutex_p != NULL)
	{
		pthread_mutex_init(Mutex_p, NULL);
	}

	return Mutex_p;


}

/*******************************************************************************
Name        : CSOS_DeleteMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_DeleteMutex(CSOS_Mutex_t * Mutex_p)
{

	if(Mutex_p != NULL)
	{
		pthread_mutex_destroy(Mutex_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;
	
}

/*******************************************************************************
Name        : CSOS_LockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_LockMutex(CSOS_Mutex_t * Mutex_p)
{
	
	if(Mutex_p != NULL)
	{
		pthread_mutex_lock(Mutex_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;

}

/*******************************************************************************
Name        : CSOS_UnlockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t CSOS_UnlockMutex(CSOS_Mutex_t * Mutex_p)
{

	if(Mutex_p != NULL)
	{
		pthread_mutex_unlock(Mutex_p);
		return CS_NO_ERROR;
	}
	
	return CS_ERROR_FAILD_GET_SEMAPHORE;
}



/*******************************************************************************
Name        : CSOS_UnlockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t  CSOS_CreateTask (void *Function,
                                       void *Param,
                                       CSOS_Partition_t *StackPartition,
                                       U32 StackSize,
                                       void *Stack,
                                       CSOS_Partition_t* TaskPartition,
                                       CSOS_Task_Handle_t* Task,
                                       CSOS_TaskDesc_t** Tdesc,
                                       U32 Priority,
                                       char* Name,
                                       CSOS_TaskFlag_t Flags )
{
		int error ;

                
                pthread_attr_t attr;
                struct sched_param param;
                
                pthread_attr_init(&attr);
                pthread_attr_getschedparam(&attr, &param);
                param.sched_priority = Priority+70;
                pthread_attr_setschedparam(&attr, &param);
                pthread_attr_setschedpolicy(&attr, SCHED_RR);
	
		error = pthread_create(Task,&attr,(void*)Function,Param);
		if(error != 0)
		{
			return CS_ERROR_FAILD_GET_SEMAPHORE;
		}
		return CS_NO_ERROR;

}

/*******************************************************************************
Name        : CSOS_UnlockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t  CSOS_StartTask  ( CSOS_Task_Handle_t  Task)
{

	return CS_NO_ERROR;

}

/*******************************************************************************
Name        : CSOS_UnlockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
CS_ErrorCode_t  CSOS_DeleteTask (CSOS_Task_Handle_t Task,CSOS_Partition_t* TaskPartition)
{

	pthread_cancel(Task);
	return CS_NO_ERROR;
//


}

/*******************************************************************************
Name        : CSOS_UnlockMutex
Description : 
Parameters  : 
Assumptions : 
Limitations :
Returns     : CS_ERROR_MESSAGE_SEND, CS_ERROR_BAD_PARAMETER
*******************************************************************************/
void CSOS_DelayTaskMs(int millisecon)
{
	usleep(1000*millisecon);
}

U32 CS_OS_time_now(void)
{
    struct timespec  time_value;
    U32     retval;

    clock_gettime(CLOCK_REALTIME, &time_value);

    retval = time_value.tv_sec*1000+ time_value.tv_nsec/1000000;

    return(retval);
}

U32 CS_OS_time_Get_Sec(void)
{
    struct timespec  time_value;

    clock_gettime(CLOCK_REALTIME, &time_value);

    return(time_value.tv_sec);
}

void CS_OS_Time_Offset_Setting(int int_hour, int int_minite, BOOL PlusMinus)
{
	struct timespec  time_value;

    clock_gettime(CLOCK_REALTIME, &time_value);

	if ( PlusMinus == FALSE )
	{
		//printf("\n ===== MINUS : %ld", time_value.tv_sec);
	    time_value.tv_sec = time_value.tv_sec - (time_t)((int_hour * 3600) + (int_minite * 60));
		//printf(" : %ld : %d\n", time_value.tv_sec, ((int_hour * 3600) + (int_minite * 60)));
	}
	else
	{
		//printf("\n ===== PLUS : %ld", time_value.tv_sec);
		time_value.tv_sec = time_value.tv_sec + (time_t)((int_hour * 3600) + (int_minite * 60));
		//printf(" : %ld : %d\n", time_value.tv_sec, ((int_hour * 3600) + (int_minite * 60)));
	}

	clock_settime(CLOCK_REALTIME, &time_value);
}


