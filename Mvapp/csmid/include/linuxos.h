#ifndef __LINUXOS_H
#define __LINUXOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wait.h>
#include <pthread.h>
#include <sched.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
//#include <minigui/mgext.h>
//#include <minigui/skin.h>

#include <semaphore.h>
#include "csapi.h"
#include <mqueue.h>
#include <linuxdefs.h>
#include <time.h>

typedef unsigned int     		 CSOS_MessageQueue_t;

typedef sem_t 				CSOS_Semaphore_t;
typedef unsigned int 			CSOS_Partition_t;
typedef	 pthread_mutex_t    	 CSOS_Mutex_t;


typedef pthread_t  			CSOS_Task_Handle_t;
typedef unsigned int  				CSOS_TaskDesc_t;        
typedef unsigned int 				CSOS_TaskFlag_t;

#ifdef PRINT_ALL
	#define dprintf(P)  printf P
#else
	#if 1
		#define dprintf(P)  printf P
	#else
		#define dprintf(P)
	#endif
#endif

#define CSOS_AllocateMemory(partition,size)                 malloc(size)
#define CSOS_DeallocateMemory(partition,ptr)                free(ptr)
#define CSOS_InitSemaphore(sem_p,initval)		     sem_init(sem_p,initval)

#define CS_OS_time_minus(t1,t2)             abs(t1-t2)
#define CS_OS_time_plus(t1,t2)                 t1+t2
#define CS_OS_time_after(t1,t2)                ((t1 > t2) ? 1 : 0)
#define CS_OS_ConvertMstoTick(t1)       t1/1
#define CS_OS_ConvertTicktoMs (t1)      t1

#define CS_OS_Clock_t            U32


CSOS_MessageQueue_t *CSOS_CreateMessageQueue(const char *Qname,unsigned int ElementSize, unsigned int NoElements);
int CSOS_DeleteMessageQueue(CSOS_MessageQueue_t * MsgQueue_p);
int CSOS_ReleaseMessagebuffer(CSOS_MessageQueue_t * MsgQueue_p, void * Msg_p);
void *CSOS_ReceiveMessage(CSOS_MessageQueue_t * MsgQueue_p);
void *CSOS_ReceiveMessageTimeOut(CSOS_MessageQueue_t * MsgQueue_p, unsigned int  TimeOutMs);
int CSOS_SendMessage(CSOS_MessageQueue_t * MsgQueue_p,void * Msg_p,unsigned int Msg_Size, int Priority);

CSOS_Semaphore_t * CSOS_CreateSemaphoreFifo           (CSOS_Partition_t * Partition_p, const int InitialValue);


CS_ErrorCode_t  CSOS_DeleteSemaphore(CSOS_Partition_t * Partition_p, CSOS_Semaphore_t * Semaphore_p);

CS_ErrorCode_t CSOS_SignalSemaphore(CSOS_Semaphore_t * Semaphore_p);

CS_ErrorCode_t  CSOS_WaitSemaphore(CSOS_Semaphore_t * Semaphore_p);  /* Nothing to do*/

CS_ErrorCode_t  CSOS_WaitSemaphoreTimeOut(CSOS_Semaphore_t * Semaphore_p, unsigned int TimeOutMs);

CS_ErrorCode_t CSOS_GetSemaphoreValue(CSOS_Semaphore_t * Semaphore_p, int *sval);

CS_ErrorCode_t CSOS_EnterCriticalSection(CSOS_Semaphore_t *Lock_p);

CS_ErrorCode_t CSOS_ExitCriticalSection(CSOS_Semaphore_t *Lock_p);



/*  Mutex Management */
CSOS_Mutex_t * CSOS_CreateMutexFifo(void);

CS_ErrorCode_t CSOS_DeleteMutex(CSOS_Mutex_t * Mutex_p);

CS_ErrorCode_t CSOS_LockMutex(CSOS_Mutex_t * Mutex_p);

CS_ErrorCode_t CSOS_UnlockMutex(CSOS_Mutex_t * Mutex_p);


/*task Management*/
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
                                       CSOS_TaskFlag_t Flags );


CS_ErrorCode_t  CSOS_StartTask  ( CSOS_Task_Handle_t  Task);


CS_ErrorCode_t  CSOS_DeleteTask (CSOS_Task_Handle_t Task,
                                 CSOS_Partition_t* TaskPartition);



void CSOS_DelayTaskMs(int millisecon);

U32 CS_OS_time_now(void);
void CS_OS_Time_Offset_Setting(int int_hour, int int_minite, BOOL PlusMinus);
U32 CS_OS_time_Get_Sec(void);

#ifdef __cplusplus
}
#endif

#endif 


