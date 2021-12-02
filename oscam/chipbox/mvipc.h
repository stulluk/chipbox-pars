/****************************************************************
*
* FILENAME
*	mvipc.h
*
* PURPOSE 
*	MV API Header For Inter-Process Communication
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
#ifndef __MVAPI_IPC_H__
#define __MVAPI_IPC_H__

/****************************************************************
 *                       include files                          *
 ****************************************************************/

/****************************************************************
 *                          define                              *
 ****************************************************************/
#define MAIN_TO_PLUGIN_KEY        1004
#define PLUGIN_TO_MAIN_KEY        2004
#define TO_MAIN_MESSAGE_KEY       3004
#define FROM_MAIN_MESSAGE_KEY     4004

#define MAX_IPC_DATA_LENGTH       1024

#define DATA_TYPE_OSCAM           0x0100
#define DATA_TYPE_RCU             0x0200
#define DATA_TYPE_TEST            0xFF00

#define DATA_CHANNEL              0x00
#define DATA_PMT                  0x01
#define DATA_CAT                  0x02
#define DATA_ECM                  0x03
#define DATA_EMM                  0x04
#define DATA_ECM_FILTER           0x05
#define DATA_EMM_FILTER           0x06
#define DATA_ECM_RESULT           0x07
#define DATA_EMM_RESULT           0x08
#define DATA_SERVER_STATUS        0x09
#define DATA_CONTROL              0x10

#define DATA_NULL                 0xFF

#define CAM_DISABLE               0
#define CAM_READY                 1
#define CAM_ENABLE                2
#define CAM_PING                  3

/****************************************************************
 *	                      Type Define                           *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
int MvApiCreatIpcMessage(unsigned key);
int MvApiGetIpcMessage(unsigned char mode);
int MvSendMessage(int msgId, long dataType, int length, unsigned char *data);
int MvReceiveMessage(int msgId, long *dataType, int *length, unsigned char *data);

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                         Functions                            *
 ****************************************************************/

#endif /* __MVAPI_IPC_H__ */
