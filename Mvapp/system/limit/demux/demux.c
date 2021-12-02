

#include "linuxos.h"

#include "crc.h"
#include "demux.h"
#include "av_zapping.h"

#define DEMUX_STACK_SIZE	   1024*8
#define DMX_TASK_PRI		   10

CSOS_Semaphore_t*	sem_DemuxAccess = NULL;

//static variable
static CS_DMX_INFO 				DMX_Info[CS_DB_MAX_NUMBER_FILTERS];

//task
static U8							NCS_demuxStack[DEMUX_STACK_SIZE];
static CSOS_Task_Handle_t 		         NCS_DmxTaskHandle;
static CSOS_TaskDesc_t*			NCS_DmxTaskDesc;
static CSOS_TaskFlag_t				TaskFlag=0;

#define     kCS_DEMUX_MAX_MSG			20
static  CSOS_MessageQueue_t                 *Demux_MsgQid = NULL;	

typedef struct
{
	U8          filter_num;
    U8          reserve1;
    U16         reserve2;
}tCS_DEMUX_Msg_t;

static void DMXEnterCriticalSection(CSOS_Semaphore_t *Semp);
static void DMXLeaveCriticalSection(CSOS_Semaphore_t *Semp);

static CS_ErrorCode_t CS_GetFreeDemuxInfo(CS_DMX_INFO **FreeInfo, DB_Demux_Slot_Type slot_type)
{
	U8  i=0;

	for(i=0;i<CS_DB_MAX_NUMBER_FILTERS;i++)
	{
		if((DMX_Info[i].Inuse==0)&&(DMX_Info[i].Slot_Type == slot_type))
		{
			*FreeInfo=&DMX_Info[i];
			break;
		}
	}
    
	if(i >= CS_DB_MAX_NUMBER_FILTERS)
			return 2;
	
	return CS_NO_ERROR;
}

/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
CSDEMUX_HANDLE CS_GetPesDemuxInfo(U8 slotNumber)
{
	CSDEMUX_HANDLE handle;
	
	handle = CSDEMUX_UNVALID_HANDLE;
	
	if (DMX_Info[slotNumber].Slot_Type == DB_DEMUX_SLOT_PES_DATA)
	{
		if (DMX_Info[slotNumber].Inuse)
		{
			handle = DMX_Info[slotNumber].Slot_Handle;
		}
	}

	return handle;
}

static CS_ErrorCode_t CS_InitSectionFilter(void)
{
	U8  i=0;

	for(i=0;i<CS_DB_MAX_NUMBER_FILTERS;i++)
	{
		DMX_Info[i].Inuse=0;
		DMX_Info[i].Running=0;
		DMX_Info[i].StartTime=0;
		DMX_Info[i].IfGetData_inCurrentCircle = FALSE;
		DMX_Info[i].crc_check_enable = FALSE;
		DMX_Info[i].Filter_ID = i;
		DMX_Info[i].Channel_ID= DEMUX_CHL_ID0;
		DMX_Info[i].Slot_ID = i;
		DMX_Info[i].Slot_Type = DB_DEMUX_SLOT_SECTION;
		DMX_Info[i].Pid = kDB_DEMUX_INVAILD_PID;
		DMX_Info[i].callback = NULL;
		DMX_Info[i].Slot_Handle = NULL;
		DMX_Info[i].Filter_Handle = NULL;
	}
	
    DMX_Info[0].Slot_Type = DB_DEMUX_SLOT_VID;
    DMX_Info[0].Inuse = 1;
    DMX_Info[1].Slot_Type = DB_DEMUX_SLOT_AUD;
    DMX_Info[1].Inuse = 1;
    DMX_Info[8].Slot_Type = DB_DEMUX_SLOT_PES_DATA;
    DMX_Info[9].Slot_Type = DB_DEMUX_SLOT_PES_DATA;
                
	return CS_NO_ERROR;
}

static INT32 DMXCheckHandle(DB_FilterHandle handle)
{
	U8	i=0;

	for(i=0;i<CS_DB_MAX_NUMBER_FILTERS;i++)
	{
		if(handle==&DMX_Info[i])
		{
			if(DMX_Info[i].Inuse==1)
			return DB_DMX_OK;
		}
	}

	return DB_DMX_FAILURE;
}

CSDEMUX_HANDLE xport_chl_handle = CSDEMUX_UNVALID_HANDLE;
INT32 DB_DemuxResetChannel(void)
{
    DMXEnterCriticalSection(sem_DemuxAccess);
    
    if(xport_chl_handle != CSDEMUX_UNVALID_HANDLE)
    {
        CSDEMUX_CHL_Disable( xport_chl_handle );
        CSDEMUX_CHL_Close(xport_chl_handle);
    }

    xport_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
    if(xport_chl_handle == CSDEMUX_UNVALID_HANDLE)
    {
        // printf("!!!Error: cs_middlware open Channel0 error.\n");
        DMXLeaveCriticalSection(sem_DemuxAccess);
        return DB_DMX_FAILURE;
    }
    CSDEMUX_CHL_SetInputMode( xport_chl_handle, DEMUX_INPUT_MOD_TUNER );
    CSDEMUX_CHL_Enable( xport_chl_handle );

    DMXLeaveCriticalSection(sem_DemuxAccess);
    return DB_DMX_OK;
}

INT32 DB_DemuxInit(void)
{
	CS_ErrorCode_t	ErrorCode = CS_NO_ERROR;

	sem_DemuxAccess = CSOS_CreateSemaphoreFifo(NULL,1);

	CS_InitSectionFilter();	

    //DB_DemuxResetChannel();
    xport_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
    if(xport_chl_handle == CSDEMUX_UNVALID_HANDLE)
    {
        printf("!!!Error: cs_middlware open Channel0 error.\n");
        return DB_DMX_FAILURE;
    }

    //CHL Config
    CSDEMUX_CHL_SetInputMode( xport_chl_handle, DEMUX_INPUT_MOD_TUNER );
    CSDEMUX_CHL_Enable( xport_chl_handle );

    if((Demux_MsgQid = CSOS_CreateMessageQueue("/Demux_MsgQid",sizeof(tCS_DEMUX_Msg_t), kCS_DEMUX_MAX_MSG )) == NULL)
	{
	        printf("create Demux_MsgQid error\n");
		return(DB_DMX_FAILURE);
	}

	return DB_DMX_OK;
}

INT32 DB_DemuxTerm(void)
{
	//Rtime do not supply this function

	CSOS_DeleteTask(NCS_DmxTaskHandle,NULL);
			
	return DB_DMX_OK;
}

U8  GetFilterNumbyHandle(CSDEMUX_HANDLE handle)
{
        U8  i;
        
        for(i=0;i<CS_DB_MAX_NUMBER_FILTERS;i++)
	{
		if(DMX_Info[i].Filter_Handle==handle)
		{
			return(i);
		}
	}

        return(CS_DB_INVALID_FILTER);
}

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
            )
{
	return DB_DMX_OK;
}

INT32 DB_DemuxSetFilterParameter(DB_FilterHandle handle, U16 pid, U8 const match[8], U8 const mask[8], U8 const Nomatch[8], DB_DemuxFilterMode mode, U32 timeout, BOOL crc_enable)

{
	return DB_DMX_OK;
}

INT32 DB_DemuxDeleteFilter(DB_FilterHandle handle)
{
	return DB_DMX_OK;
}

INT32 DB_DemuxStartFilter(DB_FilterHandle handle)
{
	return DB_DMX_OK;
}

INT32 DB_DemuxStopFilter(DB_FilterHandle handle)
{
	return DB_DMX_OK;
}

INT32 DB_DemuxFlushBuffer(DB_FilterHandle handle)
{
	return DB_DMX_OK;
}

INT32 DB_DemuxCreatePesSlot( DB_FilterHandle* handle, U16 pid, DB_DemuxCallBack callback )
{

	CSAPI_RESULT	  Result;
	CS_ErrorCode_t	  ErrorCode = CS_NO_ERROR;
	CS_DMX_INFO *	  PDmx_SFilter;

	DMXEnterCriticalSection(sem_DemuxAccess);
	
	ErrorCode=CS_GetFreeDemuxInfo(&PDmx_SFilter, DB_DEMUX_SLOT_PES_DATA);
	if( CS_NO_ERROR != ErrorCode ) 
	{
	    DMXLeaveCriticalSection(sem_DemuxAccess);
	    return DB_DMX_FAILURE;
	}
   
	PDmx_SFilter->Filter_Handle = CSDEMUX_Filter_Open(PDmx_SFilter->Filter_ID);
    //printf("PES Filter_Handle = 0x%x\n",PDmx_SFilter->Filter_Handle);
	PDmx_SFilter->Slot_Handle = CSDEMUX_PIDFT_Open(PDmx_SFilter->Slot_ID);
	// printf("PES Slot[%d] Slot_Handle = 0x%x\n", PDmx_SFilter->Slot_ID, PDmx_SFilter->Slot_Handle);

	/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
	if (PDmx_SFilter->Slot_Handle != NULL)
	{
		if (CSDEMUX_PIDFT_MallocDES(PDmx_SFilter->Slot_Handle) == CSAPI_FAILED)
		{
			printf("CSDEMUX_PIDFT_MallocDES for Slot[%d] error\n", PDmx_SFilter->Slot_ID);
		}
	}

	Result=CSDEMUX_PIDFT_SetChannel(PDmx_SFilter->Slot_Handle, DEMUX_CHL_ID0);
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_PIDFT_SetChannel Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}
	Result=CSDEMUX_PIDFT_SetPID(PDmx_SFilter->Slot_Handle, pid); 
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_PIDFT_SetPID Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}
	Result=CSDEMUX_PIDFT_Enable(PDmx_SFilter->Slot_Handle);
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_PIDFT_Enable Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}

	Result=CSDEMUX_Filter_AddPID(PDmx_SFilter->Filter_Handle, pid);
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_Filter_AddPID Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}
	Result=CSDEMUX_Filter_SetFilterType(PDmx_SFilter->Filter_Handle, DEMUX_FILTER_TYPE_PES);
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_Filter_SetFilterType Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}
	Result=CSDEMUX_Filter_Enable(PDmx_SFilter->Filter_Handle);
	if(Result!=CSAPI_SUCCEED)
	{
		printf("~_~:CSXPORT_Filter_Enable Fail\n");
		return CS_ERROR_UNKNOWN_DEVICE;
	}

	PDmx_SFilter->Pid = pid;	
	PDmx_SFilter->callback = callback;
	PDmx_SFilter->StartTime = CS_OS_time_now();
	PDmx_SFilter->Inuse = 1;

    *handle=(DB_FilterHandle *)PDmx_SFilter;

    DMXLeaveCriticalSection(sem_DemuxAccess);

	return CS_NO_ERROR;
}


INT32 DB_DemuxDeletePesSlot(DB_FilterHandle handle)
{
        CS_DMX_INFO *				PDmx_SFilter;

	DMXEnterCriticalSection(sem_DemuxAccess);
	
	PDmx_SFilter=(CS_DMX_INFO *)handle;
	if(DMXCheckHandle(handle)!=DB_DMX_OK)
	{
		printf("Invalid Handle\n");
		DMXLeaveCriticalSection(sem_DemuxAccess);
		return DB_DMX_FAILURE;
	}

	PDmx_SFilter->Inuse=0;
	PDmx_SFilter->Running=0;

	/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
	if (PDmx_SFilter->Slot_Handle != NULL)
	{
		CSDEMUX_PIDFT_FreeDES(PDmx_SFilter->Slot_Handle);
	}
	
	CSDEMUX_PIDFT_Disable(PDmx_SFilter->Slot_Handle);
	CSDEMUX_Filter_Disable(PDmx_SFilter->Filter_Handle);
	CSDEMUX_PIDFT_Close(PDmx_SFilter->Slot_Handle);
	CSDEMUX_Filter_Close(PDmx_SFilter->Filter_Handle);

	PDmx_SFilter->Pid = kDB_DEMUX_INVAILD_PID;
	PDmx_SFilter->callback = NULL;
	PDmx_SFilter->Slot_Handle = NULL;
	PDmx_SFilter->Filter_Handle = NULL;

	DMXLeaveCriticalSection(sem_DemuxAccess);

	return DB_DMX_OK;
}

INT32 DB_DemuxGetPes(DB_FilterHandle handle,U8 **DataBuffer,U32 *BufferLength,U32 Timeout)
{
    CS_DMX_INFO * PDmx_SFilter;
    U32           PackSize = 0;

	
	DMXEnterCriticalSection(sem_DemuxAccess);
    
	PDmx_SFilter=(CS_DMX_INFO *)handle;

    if(PDmx_SFilter->Filter_Handle == NULL)
     {
        DMXLeaveCriticalSection(sem_DemuxAccess);
        return(DB_DMX_FAILURE);
     }
        
    
    if(CSDEMUX_Filter_ReadWait(PDmx_SFilter->Filter_Handle,Timeout)!=CSAPI_SUCCEED)
	{
	    DMXLeaveCriticalSection(sem_DemuxAccess);
		return(-2);
	}
	if(CSDEMUX_Filter_CheckDataSize(PDmx_SFilter->Filter_Handle,&PackSize)!=CSAPI_SUCCEED)
	{
	
	}
//	printf("Get Data... ...PackSize=%d\n",PackSize);
    

	if(PackSize==0)
	{
		printf("Check Subtitle Size error\n");
        DMXLeaveCriticalSection(sem_DemuxAccess);
		return(-3);
	}

    *DataBuffer = malloc(PackSize);
	// printf("dmx malloc ptr:0x%x\n",*DataBuffer);

	if(CSDEMUX_Filter_ReadData(PDmx_SFilter->Filter_Handle, *DataBuffer, &PackSize)==CSAPI_SUCCEED)
	{
		*BufferLength = PackSize;
	}
	else
	{
	    DMXLeaveCriticalSection(sem_DemuxAccess);

	    return(-4);
	}
	
	// printf("====>>    DB_DemuxGetPes  Slot[%d] : PackSize=%d\n", PDmx_SFilter->Slot_ID, PackSize);
	DMXLeaveCriticalSection(sem_DemuxAccess);
	return(DB_DMX_OK);
}


INT32 DB_DemuxSetVidPid(CSDEMUX_HANDLE vid_slot_handle, U16 pid)
{
	DMX_Info[0].Slot_Handle = vid_slot_handle;
	DMX_Info[0].Pid = pid;

	 return CS_NO_ERROR;
}

INT32 DB_DemuxSetAudPid(CSDEMUX_HANDLE aud_slot_handle, U16 pid)
{
	DMX_Info[1].Slot_Handle = aud_slot_handle;
	DMX_Info[1].Pid = pid;

	return CS_NO_ERROR;
}

CSDEMUX_HANDLE DB_DemuxGetHandlebyPid(U16 pid)
{
    U8  i=0;
    
    for(i=0;i<CS_DB_MAX_NUMBER_FILTERS;i++)
	{
		if(DMX_Info[i].Pid == pid)
		{
			return(DMX_Info[i].Slot_Handle);
		}
	}

	return NULL;  
}

static void DMXEnterCriticalSection(CSOS_Semaphore_t *Semp)
{
	CSOS_WaitSemaphore(Semp);
}

static void DMXLeaveCriticalSection(CSOS_Semaphore_t *Semp)
{
	CSOS_SignalSemaphore(Semp);
}


