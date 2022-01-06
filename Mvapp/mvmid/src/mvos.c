/****************************************************************
*
* FILENAME
*	mvos.c
*
* PURPOSE 
*	Middle ware for OS functions
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#include "mvos.h"
#include "database.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#ifndef TIMER_MAX
#define TIMER_MAX              0xFFFFFFFF
#endif

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
typedef struct
{
	char   Name[MAX_MESSAGE_QUEUE_NAME];
	mqd_t  QueueId;
}QueueHandle_t;

typedef struct 
{
	U16 year;
	U8  month;
	U8  day;
}tCS_DT_Date;

typedef struct
{
	U8 hour;  	/* 0~23 */
	U8 minute;
}tCS_DT_Time;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern ssize_t mq_timedreceive (mqd_t __mqdes,
                                     char *__restrict __msg_ptr,
                                     size_t __msg_len,
                                     unsigned int *__restrict __msg_prio,
                                     const struct timespec *__restrict __abs_timeout);
extern int sem_timedwait (sem_t *__restrict __sem, __const struct timespec *__restrict __abstime);
extern U16 CS_MW_GetTimeRegion(void);
extern U16 CS_MW_GetTimeMode(void);
extern void CS_DT_ManualSetDateAndTime(U16 mjd, U16 utc);
extern U16 CS_DT_YMDtoMJD(tCS_DT_Date ymd);
extern U16 CS_DT_HMtoUTC(tCS_DT_Time hm);

/****************************************************************
 *                         Functions                            *
 ****************************************************************/
// #ifndef DEBUG_ON
/*
 *  Name : OsDebugPrintf
 *  Description
 *     Print Debug message.
 *  INPUT Arg
 *     char *Format_p : print format
 *  OUTPUT Arg
 *     NONE
 *  RETURN : NONE
 */
void OsDebugPrintf(__const char *__restrict __format, ...)
{
	/* NULL function */
}
// #endif

/*
 *  Name : DebugPrintf
 *  Description
 *     Print Debug message.
 *  INPUT Arg
 *     char *Format_p : print format
 *  OUTPUT Arg
 *     NONE
 *  RETURN : NONE
 */
void OsDumpData(char *title, U8 *data, U32 size)
{
//#ifdef DEBUG_ON
#if 1
	U32 count;
	U32 count16;
	
	printf("********************************************************\n");
	printf("\t%s\n", title);
	printf("********************************************************");

	count16 = 0;
	
	for (count =0; count < size; count++)
	{
		if (count16 == 0)
		{
			printf("\n\t");
		}
		printf("%02X", data[count]);
		count16++;
		
		if (count16 >= 16)
		{
			count16 = 0;
		}
		else
		{
			printf(" ");
		}
	}
	printf("\n");
#endif
}

/*
 *  Name : DebugPrintf
 *  Description
 *     Print Debug message.
 *  INPUT Arg
 *     char *Format_p : print format
 *  OUTPUT Arg
 *     NONE
 *  RETURN : NONE
 */
void OsDumpDataNoTitle(U8 *data, U32 size)
{
	U32 count;
	U32 count16;
	
	count16 = 0;
	for (count =0; count < size; count++)
	{
		if (count16 == 0)
		{
			printf("\t");
		}
		printf("%02X", data[count]);
		count16++;
		
		if (count16 >= 16)
		{
			printf("\n");
			count16 = 0;
		}
		else
		{
			printf(" ");
		}
	}
	if (count16 != 0)
	{
		printf("\n");
	}
}

/*
 *  Name         : OsWaitMicrosecond
 *  Description
 *     Task Wait as Micro second unit.
 *  INPUT Arg
 *     U32 time : wait time (Micro Second unit)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void
 */
void OsWaitMicrosecond(U32 time)
{
	usleep(time);
}

/*
 *  Name         : OsWaitMillisecond
 *  Description
 *     Task Wait as Milli Second unit.
 *  INPUT Arg
 *     U32 time : wait time (Milli Second unit)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void
 */
void OsWaitMillisecond(U32 time)
{
	usleep(1000 * time);
}

/*
 *  Name         : OsTimeNow
 *  Description
 *     Get current OS Time and return (microsecond unit).
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : U32 (OS Time value)
 */
U32 OsTimeNow(void)
{
	U32   currentTime;
    struct timespec  time_value;

    clock_gettime(CLOCK_REALTIME, &time_value);

    currentTime = time_value.tv_sec*1000000 + time_value.tv_nsec/1000;

	return currentTime;
}

/*
 *  Name         : OsTimeNowMilli
 *  Description
 *     Get current OS Time and return (millisecond unit).
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : U32 (OS Time value)
 */
U32 OsTimeNowMilli(void)
{
	U32   currentTime = 0;
    struct timespec  systemTime;

    clock_gettime(CLOCK_REALTIME, &systemTime);

    currentTime = systemTime.tv_sec*1000 + systemTime.tv_nsec/1000000;

	return currentTime;
}

/*
 *  Name         : OsTimeDiff
 *  Description
 *     Calculate Time Difference and return value.
 *  INPUT Arg
 *     U32 now    : Current OS Time
 *     U32 before : Previous Time
 *  OUTPUT Arg
 *     NONE
 *  RETURN : U32 (OS Time value)
 */
U32 OsTimeDiff(U32 now, U32 before)
{
	U32   diffTime = 0;

	if(before <= now)
	{
		diffTime = now - before; // 
	}
	else
	{
		diffTime = TIMER_MAX - before;
		diffTime = diffTime + now + 1;
	}
	
	return diffTime;
}

/*
 *  Name         : OsMemoryAllocate
 *  Description
 *     Memory allocate by given size.
 *  INPUT Arg
 *     U32 size   : Memory size in byte to be allocated
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void * (allocated memory pointer, NULL : if fail to allocate)
 */
void *OsMemoryAllocate(int size)
{
	return malloc((size_t)size);
}

/*
 *  Name         : OsMemoryFree
 *  Description
 *     Memory free for given point.
 *  INPUT Arg
 *     void *memP : Memory pointer to be freed
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsMemoryFree(void *memP)
{
	if (memP == NULL)
	{
		OsDebugPrintf("OsMemoryFree Error : NULL pointer\n");
		return OS_RETURN_ERROR;
	}

	free(memP);

	return OS_NO_ERROR;
}

/*
 *  Name         : OsCreateTask
 *  Description
 *     Create Task.
 *  INPUT Arg
 *     char *taskName             : name of Task 
 *     void (*task)(void* param)  : Task function
 *     void *param                : Parameter for Task function
 *     U32   stackSize            : Stack size for task
 *  OUTPUT Arg
 *     U32  *taskId               : Created Task Id
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsCreateTask(U32  *taskId,
				char *taskName,
				void (*task)(void *),
				void *param,
				U8    prio,
				U32   stackSize)
{
	pthread_t          *threadId;
	pthread_attr_t      taskAttr;
	struct sched_param  sParam;

	*taskId = 0;

	OsDebugPrintf("OsCreateTask : %s task, size = %d\n", taskName, stackSize);
	if (pthread_attr_init(&taskAttr) != 0)
	{
		OsDebugPrintf("OsCreateTask Error : pthread_attr_init error\n");
		return OS_RETURN_ERROR;
	}
 	if (pthread_attr_getschedparam(&taskAttr, &sParam) != 0)
	{
		OsDebugPrintf("OsCreateTask Error : pthread_attr_getschedparam error\n");
		return OS_RETURN_ERROR;
	}
	
	// pthread_attr_setinheritsched(&taskAttr, PTHREAD_EXPLICIT_SCHED);

	sParam.sched_priority = prio + 70;
	
	if (pthread_attr_setschedpolicy(&taskAttr, SCHED_RR) != 0)
	{
		OsDebugPrintf("OsCreateTask Error : pthread_attr_setschedpolicy error\n");
		return OS_RETURN_ERROR;
	}

	if (pthread_attr_setschedparam(&taskAttr, &sParam) != 0)
	{
		OsDebugPrintf("OsCreateTask Error : pthread_attr_setschedparam error\n");
		return OS_RETURN_ERROR;
	}


	threadId = (pthread_t *)OsMemoryAllocate(sizeof(pthread_t));
	
	if (pthread_create(threadId, &taskAttr, (void *)task, param) != 0)
	{
		OsDebugPrintf("OsCreateTask Error : pthread_create error\n");
		return OS_RETURN_ERROR;
	}

	*taskId = (U32)threadId;

	return OS_NO_ERROR;
}

/*
 *  Name         : OsDeleteTask
 *  Description
 *     Delete Task.
 *  INPUT Arg
 *     U32 taskId                 : Task Id which want to delete 
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsDeleteTask(U32 taskId)
{
	pthread_t          *threadId;

	if(taskId == 0)
	{
		OsDebugPrintf("OsDeleteTask Error : wrong Task Id\n");
		return OS_RETURN_ERROR;
	}

	threadId = (pthread_t *)taskId;

	pthread_cancel(*threadId);
	OsMemoryFree((void *) threadId);

	return OS_NO_ERROR;
}

/*
 *  Name         : OsCreateSemaphore
 *  Description
 *     Create Semaphore.
 *  INPUT Arg
 *     int initVal                : Semaphore initial value 
 *  OUTPUT Arg
 *     U32  *semId                : Created Semaphore Id
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsCreateSemaphore(U32 *semId, U32 initVal)
{
	sem_t  *semPointer;
	int     error;

	semPointer = (sem_t *) malloc(sizeof(sem_t));

	if (semPointer == NULL)
	{
		OsDebugPrintf("OsCreateSemaphore Error : Semaphore Memory allocation error\n");
		*semId = 0;
		return OS_RETURN_ERROR;
	}

	error = sem_init(semPointer, 0, initVal);
	if (error != 0)
	{
		OsDebugPrintf("OsCreateSemaphore Error : Semaphore Init Error\n");
		*semId = 0;
		OsMemoryFree((void *) semPointer);
		return OS_RETURN_ERROR;
	}
	
	*semId = (U32)semPointer;

	return OS_NO_ERROR;
}

/*
 *  Name         : OsDeleteSemaphore
 *  Description
 *     Delete Semaphore.
 *  INPUT Arg
 *     U32   semId                : Semaphore Id which want to delete
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsDeleteSemaphore(U32 semId)
{
	sem_t  *semPointer;

	if(semId == 0)
	{
		OsDebugPrintf("OsDeleteSemaphore Error : Wrong Semaphore Id\n");
		return OS_RETURN_ERROR;
	}

	semPointer = (sem_t *)semId;
	sem_destroy(semPointer);
	OsMemoryFree((void *) semPointer);

	return OS_NO_ERROR;
}

/*
 *  Name         : OsSemaphoreWait
 *  Description
 *     Semaphore wait with given timeOut.
 *  INPUT Arg
 *     U32 semId                  : Semaphore Id
 *     U32 timeOut                : Timeout value(millisec), 0 : no wait, 0xFFFFFFFF : Timeout forever
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsSemaphoreWait(U32 semId, U32 timeOut)
{
	sem_t           *semPointer;
	struct timespec  systemTime;
	int              semValue;
	long int         msecTime;

	if(semId == 0)
	{
		OsDebugPrintf("OsSemaphoreWait Error : Wrong Semaphore Id\n");
		return OS_RETURN_ERROR;
	}

	semPointer = (sem_t *)semId;

	switch(timeOut)
	{
	case TIMEOUT_IMMEDIATE : /* No wait. Just check count */
		sem_getvalue (semPointer, &semValue);
		if (semValue > 0)
		{
			 clock_gettime(CLOCK_REALTIME, &systemTime);
			 if(sem_timedwait (semPointer, &systemTime) != 0)
			 {
				 OsDebugPrintf("OsSemaphoreWait Error : TIMEOUT_IMMEDIATE sem_timedwait error\n");
				 return OS_RETURN_ERROR;
			 }
		}
		else
		{
			return OS_RETURN_ERROR;
		}
		break;
	case TIMEOUT_FOREVER   :  /* wait semaphore signal without timeOut */
		if (sem_wait(semPointer) != 0)
		{
			OsDebugPrintf("OsSemaphoreWait Error : TIMEOUT_FOREVER sem_wait error\n");
			return OS_RETURN_ERROR;
		}
		break;
	default                : /* wait semaphore signal but wait only during timeOut(msec) */
		clock_gettime(CLOCK_REALTIME, &systemTime);
		systemTime.tv_sec += timeOut/1000;
		msecTime = timeOut%1000;
		if((systemTime.tv_nsec/1000000 + msecTime)>=1000)
		{
			/* Result bigger than 1 second*/
			systemTime.tv_sec += 1;
			systemTime.tv_nsec -= ((1000 - msecTime) * 1000000);
		}
		else
		{
			systemTime.tv_nsec += (msecTime * 1000000); 
		}

		if(sem_timedwait (semPointer, &systemTime) != 0)
		{
			OsDebugPrintf("OsSemaphoreWait Error : sem_timedwait error\n");
			return OS_RETURN_ERROR;
		}
		break;
	}

	return OS_NO_ERROR;
}

/*
 *  Name         : OsSemaphoreSignal
 *  Description
 *     Semaphore signal.
 *  INPUT Arg
 *     U32 semId                  : Semaphore Id
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsSemaphoreSignal(U32 semId)
{
	sem_t    *semPointer;

	if(semId == 0)
	{
		OsDebugPrintf("OsSemaphoreSignal Error : Wrong Semaphore Id\n");
		return OS_RETURN_ERROR;
	}

	semPointer = (sem_t *)semId;
	sem_post(semPointer);

	return OS_NO_ERROR;
}

/*
 *  Name         : OsCreateMessageQueue
 *  Description
 *     Create Message Queue.
 *  INPUT Arg
 *     char *qName                : Name of Message Queue to identify
 *     U32 queueSize              : Size of one Message buffer
 *     U32 numberOfQueue          : Number of Message buffer in the Queue
 *  OUTPUT Arg
 *     U32 *queueId               : Created Queue ID
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsCreateMessageQueue(U32 *queueId, char *qName, U32 messageSize, U32 numberOfMessage)
{
	mqd_t           messageQid;
	struct mq_attr  mqAttrib;
	U32             nameSize;
	QueueHandle_t  *qHandle;

	qHandle = (QueueHandle_t *)OsMemoryAllocate(sizeof(QueueHandle_t));
	if (qHandle == (QueueHandle_t *)NULL)
	{
		/* Error to Allocate Handle */
		OsDebugPrintf("OsCreateMessageQueue Error : Error to Allocate Handle\n");
		*queueId = 0;
		return OS_RETURN_ERROR;
	}
	memset((char *)qHandle, 0x00, sizeof(QueueHandle_t));
	
	mqAttrib.mq_maxmsg = numberOfMessage;
	mqAttrib.mq_msgsize = messageSize;
	mqAttrib.mq_flags = O_RDWR|O_CREAT;
	mqAttrib.mq_curmsgs = 0;

	nameSize = (U32)strlen(qName);
	if (nameSize > (MAX_MESSAGE_QUEUE_NAME - 2))
	{
		nameSize = MAX_MESSAGE_QUEUE_NAME - 2;
	}
	
	if (*qName == '/')
	{
		memcpy(qHandle->Name, qName, nameSize);
	}
	else
	{
		/* First Byte must start with '/' */
		*qHandle->Name = '/';
		memcpy(qHandle->Name + 1, qName, nameSize);
	}

	OsDebugPrintf("OsCreateMessageQueue Q Name : %s\n", qHandle->Name);
	messageQid = mq_open(qHandle->Name, O_RDWR|O_CREAT ,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH, &mqAttrib);
	if (messageQid == (mqd_t)(-1))
	{
		/* Error to open Queue */
		OsDebugPrintf("OsCreateMessageQueue Error : Error to open queue\n");
		*queueId = 0;
		OsMemoryFree(qHandle);
		return OS_RETURN_ERROR;
	}
	qHandle->QueueId = messageQid;
	*queueId = (U32)qHandle;

	return OS_NO_ERROR;
}

/*
 *  Name         : OsDeleteMessageQueue
 *  Description
 *     Delete Message Queue.
 *  INPUT Arg
 *     U32   queueId              : Message Queue Id which want to delete
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsDeleteMessageQueue(U32 queueId)
{
	mqd_t           messageQid;
	QueueHandle_t  *qHandle;

	if (queueId == 0)
	{
		OsDebugPrintf("OsDeleteMessageQueue Error : Wrong Queue Id\n");

		return OS_RETURN_ERROR;
	}

	qHandle = (QueueHandle_t *)queueId;

	messageQid = qHandle->QueueId;
	OsMemoryFree(qHandle);
	
	if (messageQid == (mqd_t)NULL_ID)
	{
		OsDebugPrintf("OsDeleteMessageQueue Error : Message Queue not created\n");
		return OS_RETURN_ERROR;
	}

	mq_close(messageQid);

	return OS_NO_ERROR;
}

/*
 *  Name         : OsClaimMessage
 *  Description
 *     Claim Queue buffer from given queue Id.
 *  INPUT Arg
 *     U32 queueId                : Queue ID
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void * (Queue buffer pointer)
 */
void *OsClaimMessage(U32 queueId)
{
	mqd_t           messageQid;
	struct mq_attr  mqAttrib;
	void           *message;
	QueueHandle_t  *qHandle;

	if (queueId == 0)
	{
		OsDebugPrintf("OsClaimMessage Error : Wrong Queue Id\n");

		return (void *)NULL;
	}

	qHandle = (QueueHandle_t *)queueId;

	messageQid = qHandle->QueueId;
	
	if (messageQid == (mqd_t)NULL_ID)
	{
		OsDebugPrintf("OsClaimMessage Error : Message Queue not created\n");
		return (void *)NULL;
	}


	if (mq_getattr(messageQid, &mqAttrib) == -1)
	{
		OsDebugPrintf("OsClaimMessage Error : Error to get Attribute\n");

		return (void *)NULL;
	}

	if (mqAttrib.mq_curmsgs >= mqAttrib.mq_maxmsg)
	{
		OsDebugPrintf("OsClaimMessage Error : Message Queue Full\n");

		return (void *)NULL;
	}

	message = (void *)malloc(mqAttrib.mq_msgsize);
	if (message == NULL)
	{
		OsDebugPrintf("OsClaimMessage Error : Can not allocate message\n");
	}

	return message;
}

/*
 *  Name         : OsSendMessage
 *  Description
 *     Send message by given Queue ID.
 *  INPUT Arg
 *     U32 queueId                : Queue ID
 *     void *message              : Message buffer pointer
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsSendMessage(U32 queueId, void *message)
{
	mqd_t           messageQid;
	struct mq_attr  mqAttrib;
	QueueHandle_t  *qHandle;

	if (message == NULL)
	{
		OsDebugPrintf("SendMessage Error : No Message\n");

		return OS_RETURN_ERROR;
	}

	if (queueId == 0)
	{
		OsDebugPrintf("SendMessage Error : Wrong Queue Id\n");

		return OS_RETURN_ERROR;
	}

	qHandle = (QueueHandle_t *)queueId;

	messageQid = qHandle->QueueId;
	
	if (messageQid == (mqd_t)NULL_ID)
	{
		OsDebugPrintf("SendMessage Error : Message Queue not created\n");
		return OS_RETURN_ERROR;
	}

	if (mq_getattr(messageQid, &mqAttrib) == -1)
	{
		OsDebugPrintf("SendMessage Error : Error to get Attribute\n");
		OsMemoryFree((void *)message);
		return OS_RETURN_ERROR;
	}

	if (mqAttrib.mq_curmsgs >= mqAttrib.mq_maxmsg)
	{
		OsDebugPrintf("SendMessage Error : Message Queue Full\n");
		OsMemoryFree((void *)message);
		return OS_RETURN_ERROR;
	}

	if (mq_send(messageQid, (const char *)message, mqAttrib.mq_msgsize, 0) != 0)		
	{
		OsDebugPrintf("SendMessage Error : Message Send Error\n");
		OsMemoryFree((void *)message);
		return OS_RETURN_ERROR;
	}

	OsMemoryFree((void *)message);
	return OS_NO_ERROR;
}

/*
 *  Name         : OsReceiveMessage
 *  Description
 *     Receive Message from given Queue Id buffer with timeOut.
 *  INPUT Arg
 *     U32 queueId                : Queue ID
 *     U32 timeOut                : Timeout value, 0 : no wait, 0xFFFFFFFF : Timeout forever
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void * (Received message pointer)
 */
void *OsReceiveMessage(U32 queueId, U32 timeOut)
{
	mqd_t            messageQid;
	struct mq_attr   mqAttrib;
	char            *message;
	struct timespec  systemTime;
	long int         msecTime;
	QueueHandle_t  *qHandle;

	if (queueId == 0)
	{
		OsDebugPrintf("OsReceiveMessage Error : Wrong Queue Id\n");

		return (void *)NULL;
	}

	qHandle = (QueueHandle_t *)queueId;

	messageQid = qHandle->QueueId;
	
	if (messageQid == (mqd_t)NULL_ID)
	{
		OsDebugPrintf("OsReceiveMessage Error : Message Queue not created\n");
		return (void *)NULL;
	}

	if (mq_getattr(messageQid, &mqAttrib) == -1)
	{
		OsDebugPrintf("OsReceiveMessage Error : Error to get Attribute\n");
		return (void *)NULL;
	}
	message = (char *)malloc(mqAttrib.mq_msgsize);

	switch(timeOut)
	{
	case TIMEOUT_IMMEDIATE : /* No wait. Just check count */
		if (mqAttrib.mq_curmsgs > 0)
		{
			clock_gettime(CLOCK_REALTIME, &systemTime);
			if(mq_timedreceive(messageQid, message, mqAttrib.mq_msgsize, NULL, &systemTime) <= 0)
			{
				/* Message receive error */
				// OsDebugPrintf("OsReceiveMessage Error : TIMEOUT_IMMEDIATE mq_timedreceive error\n");
				OsMemoryFree((void *)message);
				return (void *)NULL;
			}
		}
		else
		{
			/* No Message waiting */
			// OsDebugPrintf("OsReceiveMessage Error : TIMEOUT_IMMEDIATE No message\n");
			OsMemoryFree((void *)message);
			return (void *)NULL;
		}
		break;
	case TIMEOUT_FOREVER   :  /* wait Message without timeOut */
		if(mq_receive(messageQid, message, mqAttrib.mq_msgsize, NULL) <= 0)
		{
			/* Message Receive error  */
			// OsDebugPrintf("OsReceiveMessage Error : TIMEOUT_FOREVER mq_receive error\n");
			OsMemoryFree((void *)message);
			return (void *)NULL;
		}
		break;
	default                : /* wait Message but wait only during timeOut(msec) */
		clock_gettime(CLOCK_REALTIME, &systemTime);
		systemTime.tv_sec += timeOut/1000;
		msecTime = timeOut%1000;
		if((systemTime.tv_nsec/1000000 + msecTime)>=1000)
		{
			/* Result bigger than 1 second*/
			systemTime.tv_sec += 1;
			systemTime.tv_nsec -= ((1000 - msecTime) * 1000000);
		}
		else
		{
			systemTime.tv_nsec += (msecTime * 1000000); 
		}

		if(mq_timedreceive(messageQid, message, mqAttrib.mq_msgsize, NULL, &systemTime) <= 0)
		{
			/* Message receive error */
			// OsDebugPrintf("OsReceiveMessage Error : mq_timedreceive error\n");
			OsMemoryFree((void *)message);
			return (void *)NULL;
		}

		break;
	}

	return (void *)message;
}

/*
 *  Name         : OsReleaseMessage
 *  Description
 *     Release Message to Queue Id buffer.
 *  INPUT Arg
 *     U32 queueId                : Queue ID
 *     void *message              : Message buffer pointer
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL OsReleaseMessage(U32 queueId, void *message)
{
	OsMemoryFree((void *)message);

	if (queueId == 0)
	{
		OsDebugPrintf("OsReleaseMessage Error : Wrong Queue Id\n");

		return OS_RETURN_ERROR;
	}

	return OS_NO_ERROR;
}

extern U16 CS_MW_GetTimeZone(void);

void MV_OS_Get_Time_Offset(char *acBuffer, BOOL b8Kind)
{
	struct tm 		tm_time;
	struct timespec time_value;

	clock_gettime(CLOCK_REALTIME, &time_value);

	memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

	if ( b8Kind == TRUE )
		sprintf(acBuffer, "%02d:%02d:%02d   %02d/%02d/%04d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year+1900);
	else
		sprintf(acBuffer, "%02d/%02d/%04d;%02d:%02d:%02d", tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year+1900, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
}

void MV_OS_Get_Time_Offset_Splite(char *acDate, char *acTime, BOOL b8Kind)
{
	struct tm 		tm_time;
	struct timespec time_value;

	clock_gettime(CLOCK_REALTIME, &time_value);

	memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

	if ( b8Kind == TRUE )
	{
		sprintf(acDate, "%02d/%02d", tm_time.tm_mday, tm_time.tm_mon + 1);
		sprintf(acTime, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
	}
	else
	{
		sprintf(acDate, "%02d/%02d/%04d", tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_year+1900);
		sprintf(acTime, "%02d:%02d:%02d", tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	}
}

struct timespec MV_OS_Time_Trans(void)
{
	int				int_hour = 0;
	int				int_minite = 0;
	BOOL			PlusMinus = FALSE;
	U16				Current_TimeZone = 0;
	struct timespec time_value;

	if ( CS_MW_GetTimeMode() != eCS_DBU_TIME_MANUAL )
	{
		Current_TimeZone = CS_MW_GetTimeZone();
		
		if ( CS_MW_GetTimeRegion() == 1 )
			Current_TimeZone += 2;
		
		if (Current_TimeZone < 24)
		{
			PlusMinus = FALSE;
			int_hour = (12-(Current_TimeZone+1)/2);
			int_minite = (Current_TimeZone%2)*30;
		}
		else
		{
			PlusMinus = TRUE;
			int_hour = (Current_TimeZone/2-12);
			int_minite = (Current_TimeZone%2)*30;
		}
	}

	clock_gettime(CLOCK_REALTIME, &time_value);
	
	if ( PlusMinus )
		time_value.tv_sec += (( int_hour * 3600 ) + ( int_minite * 60 ));
	else
		time_value.tv_sec -= (( int_hour * 3600 ) + ( int_minite * 60 ));

	return time_value;
}

int MV_OS_Get_Time_From_NTP(void)
{
	int 			Result = 0;
	struct tm		tm_time;
	tCS_DT_Time		time_HM;
	tCS_DT_Date 	date_ymd;
	struct timespec time_value;

	//Result = system( "ntpclient -c 1 -h pool.ntp.org -s &" );
	Result = system( "rdate -s utcnist.colorado.edu" );

	if ( Result != 0 )
		Result = system( "rdate -s time-b.nist.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s time-a.timefreq.bldrdoc.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s tick.ucla.edu" );

	usleep( 500*1000 );
	
	if ( Result == 0 )
	{
		time_value = MV_OS_Time_Trans();

		memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

		date_ymd.year = tm_time.tm_year + 1900;
		date_ymd.month = tm_time.tm_mon + 1;
		date_ymd.day = tm_time.tm_mday;
		time_HM.hour = tm_time.tm_hour;
		time_HM.minute = tm_time.tm_min;
		
		if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
			// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
			printf("clock_settime :: Error\n");
		}

		CS_DT_ManualSetDateAndTime(CS_DT_YMDtoMJD(date_ymd), CS_DT_HMtoUTC(time_HM));
		CS_DBU_SaveUserSettingDataInHW();
		//printf("=== %04d / %02d / %02d ==> %02d : %02d   :  %ld\n", date_ymd.year, date_ymd.month, date_ymd.day, time_HM.hour, time_HM.minute, time_value.tv_sec);
	} else {
		printf("Get NTP Time Error ======\n");
	}
	
	return Result;
}

int MV_OS_Get_Time_From_NTP_LongTime(void)
{
	int 			Result = 0;
	struct tm		tm_time;
	tCS_DT_Time		time_HM;
	tCS_DT_Date 	date_ymd;
	struct timespec time_value;

	//Result = system( "ntpclient -c 1 -h pool.ntp.org -s &" );
	Result = system( "rdate -s utcnist.colorado.edu" );

	if ( Result != 0 )
		Result = system( "rdate -s time-b.nist.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s time-a.timefreq.bldrdoc.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s tick.ucla.edu" );

	usleep( 2000*1000 );
	
	if ( Result == 0 )
	{
		time_value = MV_OS_Time_Trans();

		memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

		date_ymd.year = tm_time.tm_year + 1900;
		date_ymd.month = tm_time.tm_mon + 1;
		date_ymd.day = tm_time.tm_mday;
		time_HM.hour = tm_time.tm_hour;
		time_HM.minute = tm_time.tm_min;
		
		if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
			// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
			printf("clock_settime :: Error\n");
		}

		CS_DT_ManualSetDateAndTime(CS_DT_YMDtoMJD(date_ymd), CS_DT_HMtoUTC(time_HM));
		CS_DBU_SaveUserSettingDataInHW();
		//printf("=== %04d / %02d / %02d ==> %02d : %02d   :  %ld\n", date_ymd.year, date_ymd.month, date_ymd.day, time_HM.hour, time_HM.minute, time_value.tv_sec);
	} else {
		printf("Get NTP Time Error ======\n");
	}
	
	return Result;
}

