/****************************************************************
*
* FILENAME
*	sc_def.h
*
* PURPOSE 
*	SC Driver Common Define Header
*
* AUTHOR
*	Jacob
*
* HISTORY
*  Status                            Date              Author
*  Create                         05.05.2005           Jacob
*
****************************************************************/
#ifndef _SC_DEF_H_
#define _SC_DEF_H_

#define NEW_PK0_USE 	1

// #define NEW_B1_EMU_USED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mvosapi.h"
#include "mvutil.h"
#include "casdrv_def.h"
#include "casapi.h"
#include "linuxdefs.h"

#if defined(SC_USED) || defined (SC_CARD_USED) || defined (SC_SHARE_USED)

/****************************************************************
 *                          define                              *
 ****************************************************************/
#define OS_MEM_ALLOCATE(x)  OsMemoryAllocate (x)
#define OS_MEM_FREE(x)      OsMemoryFree (x)

/* Added By Jacob For Better Smart Function */
#define SC_SMART_DEFAULT_IO_TIMEOUT   1000  /* 1000 msec */

/* For B1 Autoroll By Jacob 12 Oct 2006 */
// #define USA_VERSION

/* For PIP By Jacob 28 June 2008 */
#define MESSAGE_SUBDATA_SIZE            5

/* For Map3B By Jacob */
#define NAGRA2_CW_DELAY                 0x08

/* For EMM 9F Update By Jacob 20 Nov 2007 */
#define NAGRA2_DN_FOUND                 0x10
#define NAGRA2_BEV_FOUND                0x20

#define SC_CONTROL_MESSAGE_QUE_NUMBER   50
#define SC_MESSAGE_QUE_NUMBER           25
#define TABLE_BUFFER_SIZE               1024 /* Table buffer size for PMT, CAT, ECM and EMM */
#define MIN_TABLE_SIZE                  8
#define TABLE_CRC_SIZE                  4
#define MIN_CAT_SIZE                    12 /* MIN_TABLE_SIZE + TABLE_CRC_SIZE */
#define CAT_PID                         0x0001
#define CAT_TABLE_ID                    0x01
#define PMT_TABLE_ID                    0x02
#define ECM_TABLE_ID_0                  0x80
#define ECM_TABLE_ID_1                  0x81
#define EMM_TABLE_ID                    0x82
#define EMM_CRYPTOWROKS_TABLE_ID        0x80
#define INVALID_TABLE_ID                0xFF
#define MAX_MESSAGE_WAIT                0xFFFFFFFF

#define MAX_SUB_DATA                    4

#define CW_PROCESS_CONFIRM              0x01
#define CW_EVEN_VALID                   0x10
#define CW_ODD_VALID                    0x20
 
#define TPS_TAG                         0xD2

#define MAX_IR_ECM_VERSION              16
#define INVALID_ECM_VERSION             0xFF

#ifdef SC_CARD_USED
#define NO_OF_CARD_SLOT                 MAX_CARD_SLOT
#define CARD_INFO_BUFFER_SIZE           512 /* Smart Card Info Buffer size */
#define MAX_CARD_LABEL_LENGTH	        16
#define MAX_CARD_PROVIDER_NUMBER	    16

#define MAX_EMM_FILTER_BUFFER           256

#define EMM_TIMEOUT             90 /* sec -> 1 min 30 sec */
#endif /* #ifdef SC_CARD_USED */
/****************************************************************
 *                       type define                            *
 ****************************************************************/
// typedef  unsigned char                 U8;

// typedef  unsigned short int            U16;

// typedef  unsigned int                  U32;
#ifndef byte
typedef  unsigned char                 byte;
#endif

typedef enum{									/* CA Table Message Command */
	SC_TABLE_PROCESS_STOP = 0,
	SC_TABLE_PROCESS_START,
	/* For Tps AES Key Autoroll by Jacob 29 Dec. 2005 */
	SC_TABLE_PROCESS_TPS,
	SC_TABLE_PROCESS_CW
}SC_MsgCommand;

typedef enum{									/* CA Control Message Group */
	SC_CONTROL_MAIN = 0,
	SC_CONTROL_PSI_NOTIFY,
	SC_CONTROL_TABLE_NOTIFY,
	SC_CONTROL_RESULT_NOTIFY,
	SC_CONTROL_CARD_NOTIFY,
	SC_CONTROL_STOP_NOTIFY
}SC_MsgControlGroup;

typedef enum{									/* CA Control Message Command */
	SC_STOP_ALL = 0,
	SC_STOP_SC_ECM,
	SC_STOP_SMART_ECM,
	SC_STOP_SC_EMM,
	SC_STOP_SMART_EMM,
	SC_START_ALL,
	SC_START_NEW_AUDIO,
	SC_START_SC_ECM,
	SC_START_SMART_ECM,
	SC_START_SC_EMM,
	SC_START_SMART_EMM,
	SC_START_NEW_CHANNEL,
	SC_GET_PMT,
	SC_GET_CAT,
	SC_GET_ECM,
	SC_GET_EMM,
	SC_SMART_INSERTED,
	SC_SMART_REMOVED,
	SC_SMART_REINIT,
	SC_SMART_INITIALED,
	SC_SMART_DETECT_OK,
	SC_SMART_DETECT_FAIL,
	SC_SHARE_ENABLED,
	SC_SHARE_REMOVED,
	SC_SHARE_REINIT,
	ECM_SC_OK,
	ECM_SMART_OK,
	EMM_SC_OK,
	EMM_SMART_OK,
	ECM_SC_FAIL,
	ECM_SMART_FAIL,
	EMM_SC_FAIL,
	EMM_SMART_FAIL,
	SHARE_CW_OK,
	SHARE_ECM_OK,
	SHARE_ECM_FAIL,
	SHARE_REQUEST_ECM,
	SC_STOP_SHARE_ECM
} SC_MsgControlCommand;

typedef enum
{
	MESSAGE_SC = 0,
	MESSAGE_CARD,
	MESSAGE_SHARE,
	MESSAGE_NONE
} MessageType;

typedef struct {												/* CA Message Type */
	SC_MsgCommand	        Command;
	/* For PIP By Jacob 28 June 2008 */
	U8                      SourceId;
	U8                      BankNumber;
	U8                      SlotNumber;
	U16                     ChannelId;
	U16                     DataSize;
	U8                      DataBuffer[TABLE_BUFFER_SIZE];
} Sc_Message_t;

typedef struct {												/* CA Control Message Type */
	SC_MsgControlGroup	    CmdGroup;
	SC_MsgControlCommand	Command;
	/* For PIP By Jacob 28 June 2008 */
	U16                     SubData[MESSAGE_SUBDATA_SIZE];
	U16                     DataSize;
	U8                     *DataBuffer;
} Sc_ControlMessage_t;

#ifdef SC_CARD_USED
typedef enum{									                /* CA Control Message Group */
	SC_SMART_CARD_NONE = 0,
	SC_SMART_CARD_REMOVE,
	SC_SMART_CARD_DETECT,
	SC_SMART_CARD_RE_INIT
}SC_TaskCommand;

typedef struct {												/* Smart Card Task Message */
	SC_TaskCommand	        Command;
	U8                      CardSlot;
} Sc_Smart_Message_t;

typedef struct {
	U8  ProviderId[3];
	U8  ProviderLabel[MAX_CARD_LABEL_LENGTH];
} CardProvider_t;

typedef struct {
	U16         	        CasId;
	U8                      CardStatus;
	U8                      CardProviderNumber;
	CardProvider_t          CardProvider[MAX_CARD_PROVIDER_NUMBER];
} Sc_CardInfo_t;

#endif // #ifdef SC_CARD_USED

#ifdef SC_SHARE_USED

typedef void (*NetChanCallBacl_t)(ChannelInfo_t *chInfo);
typedef void (*NetEcmCallBacl_t)(U8 *ecmData, U32 size);

/*
typedef enum{
	SC_SHARE_PROCESS_STOP = 0,
	SC_SHARE_PROCESS_ECM,
	SC_SHARE_PROCESS_CW
}SC_ShareMsgCommand;
*/

typedef struct {												/* CA Message Type */
	U8                      SourceId;
	U8                      BankNumber;
	U8                      SlotNumber;
	U8                      IredetoVersion;
	U16                     ChannelId;
	U16                     ServiceId;
	U16                     CasId;
	U16                     EcmPid;
	U32                     ProviderId;
	U16                     DataSize;
	U8                      Data[TABLE_BUFFER_SIZE];
	U8                      Cw[16];
} Sc_ShareEcmCwData_t;

typedef struct {												/* CA Message Type */
	SC_MsgCommand           Command;
	U8                      SourceId;
	U8                      BankNumber;
	U8                      SlotNumber;
	U16                     DataSize;
	U8                      DataBuffer[];
} Sc_ShareMsg_t;

typedef struct {												/* CA Message Type */
	U8                      Result;
	U16                     DataSize;
	U8                      DataBuffer[16];
} Sc_ShareResult_t;
#endif // #ifdef SC_SHARE_USED

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/
extern U8    SmartReadBuffer[256];

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

U32 Sc_GetTimeNow(void);
U32 Sc_DiffTimeMilli(U32 currentTime, U32 beforeTime);
U32 ScGetTimeLapseMilli(U32 pastTime);

U16 GetInfoLength(U8 *data);
U16 GetPid(U8 *data);

/* 9F Data move to Key Db Jacob 26 Oct. 2008 */
U8 *GetUsa9FData(void);

boolean GetUserKey(U16 casId, U32 providerId, U8 keyIndex, U8 *length, U8 *data, U8 skip);

boolean SC_StartEcm(U8 sourceId, U16 pid, U8 tableId, U8 versionNumber, U8 *slot);
boolean SC_StartEmm(U8 sourceId, U16 pid, U8 numberOfFilter, U8 *filterData, U8 *filterMask, U8 *slot);
boolean SC_StartCat(U8 sourceId, U8 *slot);
boolean SC_StopTable(U8 sourceId, U8 slot);
/* For Viaccess Key Autoroll by Jacob 22 June 2006 */
void ScUpdateKey (U16 casId, U32 providerId, U8 keyNumber, U8 *keyData, U8 mode);
void ScSetCw (U8 sourceId, U8 slotNumber, U8* key, U8 confirmFlag);
boolean ScGetBissKey(U16 ChannelId, U8 *cw);

boolean SendScControlMessage (SC_MsgControlGroup msgGroup,
							  SC_MsgControlCommand command,
							  U16 *subData,
							  U16  size,
							  U8  *buffer);
boolean Sc_ReceiveControlMessage (SC_MsgControlGroup    *group,
								  SC_MsgControlCommand  *command,
								  U16                   *SubData,
								  U16                    maxSize,
								  U16                   *actualSize,
								  U8                   **buffer,
								  U32                    Timeout /* mSec */);
void SC_DrvCriticalSessionIn(void);
void SC_DrvCriticalSessionOut(void);
boolean SendEcmMessage(SC_MsgCommand command,
                       U8  sourceId,
                       U8  bank,
                       U8  slot,
                       U16 channelId,
                       U16 size,
                       U8 *buffer,
                       MessageType  dest);
#ifdef SC_USED
boolean ReceiveScEcmMessage (SC_MsgCommand *command,
							 /* For PIP By Jacob 28 June 2008 */
							 U8 *sourceId,
							 U8 *bank,
							 U8 *slot,
							 U16 maxSize,
							 U16 *actualSize,
							 U8 *buffer);
boolean SendScEmmMessage (SC_MsgCommand command,
						  /* For PIP By Jacob 28 June 2008 */
						  U8  sourceId,
						  U8  slot,
						  U16 size,
						  U8 *buffer);
boolean ReceiveScEmmMessage (SC_MsgCommand *command,
							 /* For PIP By Jacob 28 June 2008 */
							 U8 *sourceId,
							 U8 *bank,
							 U16 maxSize,
							 U16 *actualSize,
							 U8 *buffer);
#endif /* #ifdef SC_USED */

#ifdef SC_CARD_USED
boolean ReceiveCardEcmMessage (SC_MsgCommand *command,
							   /* For PIP By Jacob 28 June 2008 */
							   U8 *sourceId,
							   U8 *bank,
							   U8 *slot,
							   U16 maxSize,
							   U16 *actualSize,
							   U8 *buffer);
boolean SendCardEmmMessage (SC_MsgCommand command,
							/* For PIP By Jacob 28 June 2008 */
							U8  sourceId,
							U8  slot,
							U16 size,
							U8 *buffer);
boolean ReceiveCardEmmMessage (SC_MsgCommand *command,
							   /* For PIP By Jacob 28 June 2008 */
							   U8 *sourceId,
							   U8 *bank,
							   U16 maxSize,
							   U16 *actualSize,
							   U8 *buffer);

void CardReboot(U8 cardSlot);
#endif /* #ifdef SC_CARD_USED */

#ifdef SC_SHARE_USED
boolean ScSendEcmToShare(Sc_ShareEcmCwData_t ecmData);

boolean ReceiveShareEcmMessage (SC_MsgCommand *command,
							   U8 *sourceId,
							   U8 *bank,
							   U8 *slot,
							   U16 *channelId,
							   U16 maxSize,
							   U16 *actualSize,
							   U8 *buffer,
							   U32 timeout);

boolean SendShareCwMessage (U8  result,
							U16 cwSize,
							U8 *cwData);
boolean ReceiveShareCwMessage (U8  *result,
							   U16 *cwSize,
							   U8  *cwData,
							   U32  timeout);

U8 ScGetCwFromShare(U8 *data);

#endif // #ifdef boolean ScCheckShare(void)

#endif /* #if defined(SC_USED) || defined (SC_CARD_USED) */

#endif /* #ifndef _SC_DEF_H_ */
