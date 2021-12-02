#include "linuxos.h"
#include "sys_setup.h"

#include "hdmitx.h"
#include "cat6611api.h"

#define CAT6611_TASK_NAME "Cat6611"
#define CAT6611_TASK_PRIORITY 7 
#define CAT6611_TASK_STACKSIZE 65536


static U8							CAT6611Stack[CAT6611_TASK_STACKSIZE];
static CSOS_Task_Handle_t 		       CAT6611TaskHandle;
static CSOS_TaskDesc_t*			CAT6611TaskDesc;
static CSOS_TaskFlag_t				CAT6611TaskFlag=0;

static HDMI_Video_Type SetVideoType=HDMI_576i50, CurrentVideoType=HDMI_576i50;

/* For HDMI Aspec Control By KB Kim 20101225 */
static HDMI_Aspec SetAspecType = HDMI_16x9, CurrentAspecType = HDMI_16x9;

CSOS_Semaphore_t 		*sem_Cat6611Access = NULL;

void HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_Aspec asPecR, HDMI_OutputColorMode OutputColorMode);
static void Cat6611_task(void *Param);

void Cat6611ClearVideoState(void)
{
	CurrentVideoType = HDMI_Unkown;
	CurrentAspecType = HDMI_4x3;
}

int Cat6611_init(void)
{
	rt_error_t	ErrorCode = CS_NO_ERROR;

	ErrorCode=CSOS_CreateTask((void *)(Cat6611_task),
								NULL,
								NULL,
								CAT6611_TASK_STACKSIZE,
								CAT6611Stack,
								NULL,
								&CAT6611TaskHandle,
								&CAT6611TaskDesc,
								CAT6611_TASK_PRIORITY,
								CAT6611_TASK_NAME,
								CAT6611TaskFlag);
	if(ErrorCode!=CS_NO_ERROR)
	{
		return 1;
	}
    
	ErrorCode=CSOS_StartTask(CAT6611TaskHandle);
	if(ErrorCode!=CS_NO_ERROR)
	{
		return 1;
	}

	sem_Cat6611Access  = CSOS_CreateSemaphoreFifo ( NULL, 1 );
    if( sem_Cat6611Access == NULL )
	{
		return 1;
	}
    
    return 0;

}

extern _IDATA BYTE bInputSignalType;

/* For HDMI Aspec Control By KB Kim 20101225 */
void Cat6611_SetAspecRatio(U8 on16_9)
{
	if(on16_9)
	{
		SetAspecType = HDMI_16x9;
		/*
		switch (CurrentVideoType)
		{
			case HDMI_480i60 :
				SetVideoType = HDMI_480i60_16x9;
				break;
			case HDMI_576i50 :
				SetVideoType = HDMI_576i50_16x9;
				break;
			case HDMI_576p50 :
				SetVideoType = HDMI_576p50_16x9;
				break;
			default :
				break;
		}
		*/
	}
	else
	{
		SetAspecType = HDMI_4x3;
		/*
		switch (CurrentVideoType)
		{
			case HDMI_480i60_16x9 :
				SetVideoType = HDMI_480i60;
				break;
			case HDMI_576i50_16x9 :
				SetVideoType = HDMI_576i50;
				break;
			case HDMI_576p50_16x9 :
				SetVideoType = HDMI_576p50;
				break;
			default :
				break;
		}
		*/
	}
}
	
int Cat6611_SetOutputMode(eSYSHDMIType Mode)
{
	CSOS_WaitSemaphore(sem_Cat6611Access);

	// printf("Mode=%d  %d\n",Mode,SYS_HDMI_480I);
	switch(Mode)
	{
		case SYS_HDMI_480I:
	        SetVideoType=HDMI_480i60;
			bInputSignalType=T_MODE_SYNCEMB|T_MODE_CCIR656;			
			break;
    
		case SYS_HDMI_576I:
			SetVideoType=HDMI_576i50;
			bInputSignalType=T_MODE_SYNCEMB|T_MODE_CCIR656;
			break;

		case SYS_HDMI_576P:
			SetVideoType=HDMI_576p50;
			bInputSignalType=T_MODE_SYNCEMB;
			break;

		case SYS_HDMI_720P50:
			SetVideoType=HDMI_720p50;            
			bInputSignalType=T_MODE_SYNCEMB;
			break;

		case SYS_HDMI_720P60:
			SetVideoType=HDMI_720p60;
			bInputSignalType=T_MODE_SYNCEMB;
			break;

		case SYS_HDMI_1080I25:
			SetVideoType=HDMI_1080i50;
			bInputSignalType=T_MODE_SYNCEMB;
			break;

		case SYS_HDMI_1080I30:
			SetVideoType=HDMI_1080i60;
			bInputSignalType=T_MODE_SYNCEMB;
			break;

		default:
			SetVideoType=HDMI_576i50;
			bInputSignalType=T_MODE_SYNCEMB|T_MODE_CCIR656;
			break;
	}
	CSOS_SignalSemaphore(sem_Cat6611Access);

	return 0;
}


static void Cat6611_reset(void)
{
    CSGPIO_HANDLE   gpio_handle = NULL;
    CSAPI_RESULT    result;

	gpio_handle = CSGPIO_Open(15);
    if( gpio_handle == NULL )
    {
        // printf("\n open gpio 15 erro\n");
        return;
    }
    
	result= CSGPIO_SetDirection( gpio_handle, GPIO_DIRECTION_WRITE );   // 1: write  0:read
    if(result != CSAPI_SUCCEED)
    {
        // printf("\n set gpio direction error!!!\n");
        return;
    }
    
	result= CSGPIO_Write( gpio_handle, 0 );	/* only 0 and 1 are valid */
    if( result != CSAPI_SUCCEED )
    {
        // printf("\n write gpio error\n");
        return;
    }

    CSOS_DelayTaskMs(100);

	result= CSGPIO_Write( gpio_handle, 1 );	/* only 0 and 1 are valid */
    if( result != CSAPI_SUCCEED )
    {
        // printf("\n write gpio error\n");
        return;
    }

    // printf("HDMI Chip Cat6611 Reseted!\n");
    CSGPIO_Close(gpio_handle);
	
}

void Cat6611Pause(void)
{
	if (sem_Cat6611Access != NULL)
	{
		CSOS_WaitSemaphore(sem_Cat6611Access);
	}
}

void Cat6611Resume(void)
{
	if (sem_Cat6611Access != NULL)
	{
		CSOS_SignalSemaphore(sem_Cat6611Access);
	}
}

static void Cat6611_task(void *Param)
{

    Cat6611_reset();

    CSOS_WaitSemaphore(sem_Cat6611Access);

	/* For HDMI Aspec Control By KB Kim 20101225 */
    HDMITX_ChangeDisplayOption(SetVideoType, SetAspecType, HDMI_YUV444) ; // set initial video mode and initial output color mode
    CurrentVideoType = SetVideoType;	
    CSOS_SignalSemaphore(sem_Cat6611Access);
	
    InitCAT6611();

    //printf("HDMITX_ReadI2C_Byte 0x01 = 0x%x\n",HDMITX_ReadI2C_Byte(0x13));
    //printf("HDMITX_WriteI2C_Byte 0x01 = 0x%x\n",HDMITX_WriteI2C_Byte(0x13,0xAA));
    //printf("HDMITX_ReadI2C_Byte 0x01 = 0x%x\n",HDMITX_ReadI2C_Byte(0x13));

    // printf("InitCAT6611 end,task star...\n");
    while(1)
    {
    	CSOS_WaitSemaphore(sem_Cat6611Access);

		/* For HDMI Aspec Control By KB Kim 20101225 */
        if ((CurrentVideoType != SetVideoType) || (CurrentAspecType != SetAspecType))
    	{
    		// printf("SetVideoType:%d\n",SetVideoType);
    		/* For HDMI Aspec Control By KB Kim 20101225 */
    		HDMITX_ChangeDisplayOption(SetVideoType, SetAspecType, HDMI_YUV444) ;
		    //InitCAT6611();
		    CurrentVideoType = SetVideoType;
			CurrentAspecType = SetAspecType;
    	}

        HDMITX_DevLoopProc() ;
	    CSOS_SignalSemaphore(sem_Cat6611Access);
	 
	    CSOS_DelayTaskMs(100);
    }

}



