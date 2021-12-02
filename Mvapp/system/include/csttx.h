#ifndef _CS_TTX_H_
#define _CS_TTX_H_

#include "demux.h"

#ifdef __cplusplus
extern "C" {
#endif



//#define   BT_TTX
			




#define CSTTX_MAX_DEVICE       		1  /* Max number of Init() allowed            */
#define CSTTX_MAX_OPEN         		1  /* Max num of Open() allowed per Init()    */
#define CSTTX_MAX_UNIT         		(CSTTX_MAX_OPEN * CSTTX_MAX_DEVICE)
#define CSTTX_VALID_UNIT       	   	0x13063062
#define MAX_PAGE_NUMBER_STORE  	800

#define INVALID_DEVICE_INDEX 		(-1)

#define CSTTX_DECODER_PESDATA_EVT     ((((('X')<<4)|0x0)<<16)+1)


#define  TTXLOWORD(V)        ((WORD)(V&0x0FFFF))
#define  TTXHIWORD(V)        ((WORD)((V>>16)&0x0FFFF))
#define  TTXMAKELONG(V,P)    (((DWORD)(P&0x0FFFF)<<16 ) | ((DWORD)(V&0x0FFFF)))


#define TTX_REPORTERROR(x)\
{\
	printf("__TTX_REPORTERROR__ %d\n",x);\
	return x;\
}

#define  DWORD	unsigned int
#define  BYTE	unsigned char
#define  WORD	unsigned short
//typedef unsigned char       BYTE;
//typedef unsigned short      WORD;

typedef U32 CSTTX_Handle_t;

typedef struct CSTTX_InitParams_s
{
   CSOS_MessageQueue_t*	QueueHandle;
   CS_DeviceName_t             EvtHandlerName;
   CSOS_Partition_t * 		MemoryPartition;
   CS_DeviceName_t		XportName;
   CSDEMUX_HANDLE		XportHandle;
  // U16					TtxPid;
   U32					MaxOpen;
} CSTTX_InitParams_t;

typedef struct CSTTX_OpenParams_s
{
    U32                 			TtxPid;
} CSTTX_OpenParams_t;

typedef struct CSttx_Task_s
{
    CSOS_Partition_t *		StackPartition;
    void *					Stack;
    CSOS_Task_Handle_t 	TaskHandle;
    CSOS_TaskDesc_t* 		Taskdesc;
    CSOS_Semaphore_t*	SemProcessTTX;
    BOOL					IsRunning;
    BOOL					ToBeDeleted;
} CSttx_Task_t;

typedef struct CSttx_DeviceData_s
{
    CSTTX_InitParams_t   	InitParams;
    //CSEVT_Handle_t      		EVTHandle;
    CSttx_Task_t			DemuxTask;
    CSttx_Task_t			DecoderTask;
   // CSEVT_EventID_t     		Notify_id;
    CSOS_Semaphore_t*	SemAccess;
    CS_DMX_INFO  			*PESDmxHandle;
   // CSDEMUX_HANDLE  		hpidfilter;
} CSttx_DeviceData_t;

typedef struct CSttx_Device_S
{
    CS_DeviceName_t     	DeviceName;
    CSttx_DeviceData_t * 	DeviceData_p;
} CSttx_Device_t;

typedef struct
{
    CSttx_Device_t* 		Device_p;
   // STCC_Handle_t Handle;
    U32 					UnitValidity;
} CSttx_Unit_t;

typedef struct CSTTX_TermParams_s
{
    BOOL                 		ForceTerminate;
} CSTTX_TermParams_t;

typedef struct VT_Page{
    DWORD   dwPageCode;
    WORD    wControlBits;
    BYTE    Frame[26][40];
    BYTE    LineState[26];
    DWORD   EditorialLink[6];
    BYTE     DesinationCode;
    BOOL    bBufferReserved;
    BOOL    bReceived;
    BOOL    bShowRow24;    
    struct VT_Page* pNextPage;
} TVTPage;

enum
{
    TTXEVENT_HEADERUPDATE   = 0,    // A new rolling header is ready
    TTXEVENT_PAGEBEGIN      = 1,    // New page reception has began  --reserve for future
    TTXEVENT_PAGEUPDATE     = 2,    // Page update have been received
    TTXEVENT_PAGEREFRESH    = 3,    // Page received with no update
    TTXEVENT_PDCUPDATE      = 4,    // Program Delivery Control changed  --reserve for future
    TTXEVENT_COMMENTUPDATE  = 5,    // Row 24 commentary changed  --reserve for future
    TTXEVENT_SUBTITLE  = 6,	//subtitle coming
};


/// Control bits in a page header
enum
{
    VTCONTROL_MAGAZINE      = 7 << 0,
    VTCONTROL_ERASEPAGE     = 1 << 3,
    VTCONTROL_NEWSFLASH     = 1 << 4,
    VTCONTROL_SUBTITLE      = 1 << 5,
    VTCONTROL_SUPRESSHEADER = 1 << 6,
    VTCONTROL_UPDATE        = 1 << 7,
    VTCONTROL_INTERRUPTED   = 1 << 8,
    VTCONTROL_INHIBITDISP   = 1 << 9,
    VTCONTROL_MAGSERIAL     = 1 << 10,
    VTCONTROL_CHARSUBSET    = 7 << 11,
};


/// Bit vector for LineState
enum
{
    CACHESTATE_HASDATA       = 1 << 0,
    CACHESTATE_UPDATED       = 1 << 1,
    CACHESTATE_HASERROR      = 1 << 2,
};


/// Special page and subpage values
enum
{
    /*
     * These values are local use and not from
     * any TT standards.  Where these are used
     * need to be under strict control because
     * not all path look out for these special
     * values.
     */

    VTPAGE_ERROR                = 0x000,
    
    VTPAGE_FLOFRED              = 0x010,
    VTPAGE_FLOFGREEN            = 0x011,
    VTPAGE_FLOFYELLOW           = 0x012,
    VTPAGE_FLOFBLUE             = 0x013,
    VTPAGE_PREVIOUS             = 0x020,

    VTPAGE_NULLMASK             = 0x0FF,

    VTSUBPAGE_NULL              = 0x3F7F,
    VTSUBPAGE_UNSET             = 0xFFFF,
};


typedef struct _UpdateEventPara
{
	BYTE     			uMsg;
	DWORD     		dwParam;	
}UpdateEventPara;

CS_Revision_t CSttx_GetRevision(void);
CS_ErrorCode_t CSttx_Init(const CS_DeviceName_t DeviceName,const CSTTX_InitParams_t * const InitParams_p);
CS_ErrorCode_t CSttx_Open(const CS_DeviceName_t DeviceName,const CSTTX_OpenParams_t * const OpenParams_p, CSTTX_Handle_t *Handle_p);
CS_ErrorCode_t CSttx_Close(const CSTTX_Handle_t Handle);
CS_ErrorCode_t CSttx_Term(const CS_DeviceName_t DeviceName,const CSTTX_TermParams_t *const TermParams_p);
U32 CSTTXGetDislayPage(DWORD dwPageCode, TVTPage* pBuffer);
CS_ErrorCode_t CSTTXGetDisplayHeader(TVTPage* pBuffer, BOOL bClockOnly);
CS_ErrorCode_t CSTTXGetNextDisplaySubPage(DWORD dwPageCode, TVTPage* pBuffer,BOOL RiseOrder);

#ifdef __cplusplus
}
#endif
#endif
