/****************************************************************
*
* FILENAME
*	mvosapi.h
*
* PURPOSE 
*	Header for OS Function APIs
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/
#ifndef MV_OS_API_H
#define MV_OS_API_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include "mvos.h"
#include "mvutil.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define GOLDBOX
#define	NEW_INSTALL
/****************************************************************
 *                       Type define                            *
 ****************************************************************/
 
/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
// #ifndef DEBUG_ON
extern void OsDebugPrintf(char *Format_p, ...);
// #endif
extern void OsDumpData(char *title, U8 *data, U32 size);
extern void OsDumpToFile(FILE* fileP, U8 *data, U32 size);
extern void OsDumpDataNoTitle(U8 *data, U32 size);
extern void OsWaitMicrosecond(U32 time);
extern void OsWaitMillisecond(U32 time);
extern U32 OsTimeNow(void);
extern U32 OsTimeNowMilli(void);
extern U32 OsTimeDiff(U32 now, U32 before);
extern void *OsMemoryAllocate(int size);
extern BOOL OsMemoryFree(void *memP);
extern BOOL OsCreateTask(U32  *taskId,
					   char *taskName,
					   void (*task)(void *),
					   void *param,
					   U8    prio,
					   U32   stackSize);
extern BOOL OsDeleteTask(U32 taskId);
extern BOOL OsCreateSemaphore(U32 *semId, U32 initVal);
extern BOOL OsDeleteSemaphore(U32 semId);
extern BOOL OsSemaphoreWait(U32 semId, U32 timeout);
extern BOOL OsSemaphoreSignal(U32 semId);
extern BOOL OsCreateMessageQueue(U32 *queueId, char *qName, U32 messageSize, U32 numberOfMessage);
extern BOOL OsDeleteMessageQueue(U32 queueId);
extern void *OsClaimMessage(U32 queueId);
extern BOOL OsSendMessage(U32 queueId, void *message);
extern void *OsReceiveMessage(U32 queueId, U32 timeOut);
extern BOOL OsReleaseMessage(U32 queueId, void *message);
extern void MV_OS_Get_Time_From_NTP(void);
extern int MV_OS_Get_Time_From_NTP_LongTime(void);
extern void MV_OS_Get_Time_Offset(char *acBuffer, BOOL b8Kind);
extern void MV_OS_Get_Time_Offset_Splite(char *acDate, char *acTime, BOOL b8Kind);
extern struct timespec MV_OS_Time_Trans(void);
#endif // #ifndef MV_OS_API_H

