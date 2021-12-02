#ifndef CS_DB_DEMUX_H
#define CS_DB_DEMUX_H

#include "linuxos.h"

#ifdef __cplusplus
extern "C" {
#endif


#define  CS_DB_MAX_NUMBER_FILTERS      (28)
#define  CS_DB_INVALID_FILTER          CS_DB_MAX_NUMBER_FILTERS
#define     kDB_DEMUX_INVAILD_PID        0x1FFF

#define CS_DEMUX_TIMEOUT_STEP               100
#define CS_DEMUX_TASK_RELEASE_TIME      100

//typedef struct DB_Filter* DB_FilterHandle; 
typedef void *  DB_FilterHandle; 

typedef void (*DB_DemuxCallBack)(void* userparam,
                                DB_FilterHandle  handle,
                                BOOL timedout,
                                U8 const* data,
                                U32 size);



typedef enum DB_DemuxFilterMode{
	DB_DemuxFilterMode_Default = 0X0, //Repeat Version
	DB_DemuxFilterMode_OneShot = 0X01,
	DB_DemuxFilterMode_NotVersion = 0X02
} DB_DemuxFilterMode;




typedef enum
{
	CS_DEMUX_BUFFER_MODE_1K = 0,
	CS_DEMUX_BUFFER_MODE_4K
}DB_Demux_Buffer_Mode;

typedef enum
{
        DB_DEMUX_SLOT_VID  = 0,
        DB_DEMUX_SLOT_AUD,
        DB_DEMUX_SLOT_SECTION,
        DB_DEMUX_SLOT_PES_DATA
}DB_Demux_Slot_Type;



enum{
	DB_DMX_OK=0,
	DB_DMX_FAILURE=-1,	
};

typedef enum CS_DemuxFilterMode{
	CS_DemuxFilterMode_Default = 0X0, //Repeat Version
	CS_DemuxFilterMode_OneShot = 0X01,
	CS_DemuxFilterMode_NotVersion = 0X02
} CS_DemuxFilterMode;


typedef struct
{
	U8 *FilterValues_p;
	U8 *FilterMasks_p;

	U8 *ModePattern_p;			/* SectionFilter only : not for the other 2 */
//	CSXPORT_FilterRepeatMode_t RepeatMode;
}DB_DemuxFilterParams;

typedef struct
{
				CSDEMUX_HANDLE		 Slot_Handle;
				CSDEMUX_HANDLE		 Filter_Handle;
                                    DB_Demux_Slot_Type              Slot_Type;
				CSDEMUX_FILTER_ID	 Filter_ID;
				CSDEMUX_PIDFT_ID	 Slot_ID;		
				CSDEMUX_CHL_ID		 Channel_ID;
                                   DB_DemuxFilterParams FilterSet;
				CS_DemuxFilterMode	 Mode;
				CSOS_Semaphore_t*	 DmxSemHandle;
				DB_DemuxCallBack 	 callback;
                                    DB_Demux_Buffer_Mode buff_mode;
				U8					 Nomatch[8];
				U8					 SWFilterMatch[8];
				U8					 SWFilterMask[8];
				U32 				 DestinationSize0;
				U8 *				 Destination0_p;
				U32 				 BufferSize;
				U32                              Timeout;
				U32				 StartTime;
				U16				 Pid;
				U8                   Inuse;
				U8                   Running;
                                    BOOL             IfGetData_inCurrentCircle;
                                    BOOL            crc_check_enable;
				
}CS_DMX_INFO;

/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
CSDEMUX_HANDLE CS_GetPesDemuxInfo(U8 slotNumber);

INT32 DB_DemuxCreateFilter(
                                                                                    DB_FilterHandle* handle,
                                                                                    U16 pid,
                                                                                    DB_Demux_Buffer_Mode bufsize,
                                                                                    U8 const match[8],
                                                                                    U8 const mask[8],
                                                                                    U8 const Nomatch[8],
                                                                                    DB_DemuxFilterMode mode,
                                                                                    BOOL    crc_enable,
                                                                                    U32 timeout, //ms, 0代表无穷
                                                                                    DB_DemuxCallBack callback, //每次收到符合条件的数据包都会调用一次
                                                                                    void* userparam
                                                                                    );



INT32 DB_DemuxFlushBuffer(DB_FilterHandle handle);
INT32 DB_DemuxStartFilter(DB_FilterHandle handle);
INT32 DB_DemuxStopFilter(DB_FilterHandle handle);
INT32 DB_DemuxDeleteFilter(DB_FilterHandle handle);
INT32 DB_DemuxInit(void);
INT32 DB_DemuxResetChannel(void);
INT32 DB_DemuxTerm(void);
INT32 DB_DemuxSetFilterParameter(DB_FilterHandle handle, U16 pid, U8 const match[8], U8 const mask[8], U8 const Nomatch[8], DB_DemuxFilterMode mode, U32 timeout, BOOL crc_enable);

INT32 DB_DemuxCreatePesSlot( DB_FilterHandle* handle, U16 pid, DB_DemuxCallBack callback );
INT32 DB_DemuxDeletePesSlot(DB_FilterHandle handle);
INT32 DB_DemuxGetPes(DB_FilterHandle handle,U8 **DataBuffer,U32 *BufferLength,U32 Timeout);
INT32 DB_DemuxSetVidPid(CSDEMUX_HANDLE vid_slot_handle, U16 pid);
INT32 DB_DemuxSetAudPid(CSDEMUX_HANDLE aud_slot_handle, U16 pid);
CSDEMUX_HANDLE DB_DemuxGetHandlebyPid(U16 pid);


#ifdef __cplusplus
}
#endif

#endif

