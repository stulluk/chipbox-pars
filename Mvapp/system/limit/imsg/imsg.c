/****************************************************************
*
* FILENAME
*	scart.c
*
* PURPOSE 
*	Scart control driver source
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.31           KB
*
****************************************************************/

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linuxdefs.h"
#include "database.h"
#include "mvosapi.h"
#include "mvmiscapi.h"
#include "mvapi.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define IMSG_DEBUG_ON
#define DebugPrint(x)                    printf x
#define DebugPrt(x)                      printf x

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/
int  HostReceiveMsgId;
U32  HostReceiveTaskId;

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern void MvSendRcuKeyData(U32 keyValue);

/****************************************************************
 *                         Functions                            *
 ****************************************************************/
/*
 *  Name : IMessageReceiveTask
 *  Description
 *     Receive Task Inter-Processor message.
 *  INPUT Arg
 *     U8 onFB                    : FB control (1 : FB ON(RGB mode), 0 : FB Off)
 *     void *param                : Message Key Value
 *  OUTPUT Arg
 *     NONE
 *  RETURN : NONE
 */
void IMessageReceiveTask(void *param)
{
	int  messageId;
	int  result;
	int  msgLength;
	long dataType;
	U8   data[MAX_IPC_DATA_LENGTH];

	messageId = *(int*)param;
	dataType = 0;
	
	if (messageId == (-1))
	{
#ifdef IMSG_DEBUG_ON
		DebugPrint(("============= IMessageReceiveTask : Can not get ID\n"));
#endif // #ifdef CASDRV_DEBUG_ON
		return;
	}

	while(1)
	{
		result = MvReceiveMessage(messageId, &dataType, &msgLength, data);

		if (result != (-1))
		{
			if ((dataType == DATA_TYPE_RCU) && (msgLength == 1))
			{
				MvSendRcuKeyData((U32)data[0]);
			}
		}
	}
}

/*
 *  Name : IMessageInit
 *  Description
 *     Init Inter processor message.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Init Error
 */
BOOL IMessageInit(void)
{
	BOOL result;
	
	HostReceiveMsgId = MvApiCreatIpcMessage(TO_MAIN_MESSAGE_KEY);
	if (HostReceiveMsgId == (-1))
	{
#ifdef IMSG_DEBUG_ON
		DebugPrint(("============= IMessageInit Error : Can not open Host Message Receive ID\n"));
#endif // #ifdef CASDRV_DEBUG_ON
		return TRUE;
	}
	
	result = OsCreateTask(&HostReceiveTaskId,
		                  "HotMsgReceiveTask",
		                  IMessageReceiveTask,
		                  (void *)&HostReceiveMsgId,
		                  10,
		                  1024);
	if ((result != OS_NO_ERROR) || (HostReceiveTaskId == 0))
	{
		/* We want to continue without Skew */
#ifdef IMSG_DEBUG_ON
		DebugPrint(("============= IMessageInit Error : Cannot Create HotMsgReceiveTask\n"));
#endif // #ifdef HEART_DEBUG_ON
		return TRUE;
	}

	return FALSE;
}

/*
 *  Name : HeartBitTerm
 *  Description
 *     Terminate Heart Bit Driver.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void
 *     NONE
 */
void IMessageTerm(void)
{
	if (HostReceiveTaskId)
	{
		OsDeleteTask(HostReceiveTaskId);
	}
}

