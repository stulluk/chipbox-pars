/****************************************************************
*
* FILENAME
*	mvipc.c
*
* PURPOSE 
*	MV API For Inter-Process Communication
*	
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                        14 Feb. 2011          KB Kim
*
****************************************************************/
/****************************************************************
 *                       include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mvipc.h"

/****************************************************************
 *                          define                              *
 ****************************************************************/
// #define MVAPI_IPC_DEBUG_ON

/****************************************************************
 *	                      Type Define                           *
 ****************************************************************/
typedef struct CommData_s
{
   long  Data_type;
   int   Data_Length;
   char  Data_buff[MAX_IPC_DATA_LENGTH];
} CommData_t;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                         Functions                            *
 ****************************************************************/

/*
 *  Name : MvApiCreatIpcMessage
 *  Description
 *     Creat IPC Message and return Id.
 *  INPUT Arg
 *     unsigned key                   : Communication Mode Key
 *  OUTPUT Arg
 *     NONE
 *  RETURN : int
 *     -1     : Error to get message ID
 *     others : Message ID
 */
int MvApiCreatIpcMessage(unsigned key)
{
	int msgId;

	msgId = msgget((key_t)key, IPC_CREAT | 0666);

	return msgId;
}

/*
 *  Name : MvApiGetIpcMessage
 *  Description
 *     Get IPC Message Id.
 *  INPUT Arg
 *     unsigned char mode             : Communication Mode (0 : Host to Plugin, 1 : Plugin to Host)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : int
 *     -1     : Error to get message ID
 *     others : Message ID
 */
int MvApiGetIpcMessage(unsigned char mode)
{
	int msgId;

	if (mode)
	{
		msgId = msgget((key_t)PLUGIN_TO_MAIN_KEY, IPC_CREAT | 0666);
	}
	else
	{
		msgId = msgget((key_t)MAIN_TO_PLUGIN_KEY, IPC_CREAT | 0666);
	}

	return msgId;
}

/*
 *  Name : MvSendMessage
 *  Description
 *     Send IPC Message via message ID.
 *  INPUT Arg
 *     int msgId                      : Message Id for communcation
 *     long dataType                  : Data Type
 *     int length                     : Sub-data length
 *     unsigned char *data            : Sub-Data buffer pointer
 *  OUTPUT Arg
 *     NONE
 *  RETURN : int
 *     -1     : Error to send message
 *     others : Message send success
 */
int MvSendMessage(int msgId, long dataType, int length, unsigned char *data)
{
	CommData_t sendData;
	int        result;

	if (msgId == (-1))
	{
#ifdef MVAPI_IPC_DEBUG_ON
		printf ("MvSendMessage : Wrong Message Id\n");
#endif

		return (-1);
	}

	if (length > MAX_IPC_DATA_LENGTH)
	{
#ifdef MVAPI_IPC_DEBUG_ON
		printf ("MvSendMessage : Message Length Error [%d]\n", length);
#endif

		return (-1);
	}

	sendData.Data_type   = dataType;
	sendData.Data_Length = length;
	memset(sendData.Data_buff, 0x00, MAX_IPC_DATA_LENGTH);
	if (length > 0)
	{
		memcpy(sendData.Data_buff, (char *)data, length);
	}

	result = msgsnd(msgId, &sendData, sizeof(CommData_t) - sizeof( long), 0);
#ifdef MVAPI_IPC_DEBUG_ON
	printf ("MvSendMessage : Message Send Result [%d]\n", result);
#endif

	return result;
}

/*
 *  Name : MvSendMessage
 *  Description
 *     Send IPC Message via message ID.
 *  INPUT Arg
 *     int msgId                      : Message Id for communcation
 *  OUTPUT Arg
 *     long *dataType                 : Received Data Type
 *     int  *length                   : Received Sub-data length
 *     unsigned char *data            : Received Sub-Data buffer pointer
 *  RETURN : int
 *     -1     : Error to Receive message
 *     others : Message Receive success
 */
int MvReceiveMessage(int msgId, long *dataType, int *length, unsigned char *data)
{
	CommData_t receiveData;
	long       receivedType;
	int        result;

	receivedType = *dataType;

	*dataType = DATA_NULL;
	*length   = 0;
	memset(data, 0x00, MAX_IPC_DATA_LENGTH);

	if (msgId == (-1))
	{
#ifdef MVAPI_IPC_DEBUG_ON
		printf ("MvSendMessage : Wrong Message Id\n");
#endif

		return (-1);
	}

	result = msgrcv(msgId, &receiveData, sizeof(CommData_t) - sizeof( long), receivedType, 0);
	if (result != (-1))
	{
		*dataType = receiveData.Data_type;
		*length   = receiveData.Data_Length;
		if (receiveData.Data_Length > 0)
		{
			memcpy((char *)data, receiveData.Data_buff, receiveData.Data_Length);
		}
	}

#ifdef MVAPI_IPC_DEBUG_ON
	printf ("MvSendMessage : Message Receive Result [%d]\n", result);
#endif

	return result;
}
