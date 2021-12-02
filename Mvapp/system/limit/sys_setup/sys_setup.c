
#include "linuxos.h"

#include "crc.h"
#include "database.h"
#include "demux.h"
#include "db_builder.h"
#include "av_zapping.h"
#include "date_time.h"
#include "timer.h"
#include "date_time.h"
//#include "dvbt_tuner.h"
#include "sys_setup.h"
#include "fe_mngr.h"
#include "ch_install.h"
#include "eit_engine.h"
#include "csmpr_player.h"
#include "csmpr_recorder.h"
#include "e2p.h"
//#include "mv_popup.h"
#include "csmpr_usb.h"
/* 2010.09.04 By KB KIm For New SI */
#include "tunerinit.h"
#include "tableApi.h"

extern int Cat6611_init(void);
extern BOOL MV_DB_Init(BOOL bReset);
extern void MV_DB_Output_File_Test(void);
extern BOOL HeartBitInit(void);

U32 g_screen_width;
U32	g_screen_height;

//int   	   display_fd;
//int      audio_fd ;
//CSTVOUT_HANDLE    tve_handle = NULL;
//CSDEMUX_HANDLE    xport_chl_handle = CSDEMUX_UNVALID_HANDLE;
//CSDEMUX_HANDLE    xport_pid_filter0_handle = CSDEMUX_UNVALID_HANDLE;
//CSDEMUX_HANDLE    xport_pid_filter1_handle = CSDEMUX_UNVALID_HANDLE;
#if 0
#ifdef  test_e2p
static void e2p_Test(void *pram)
{
	CS_ErrorCode_t  ErrorCode = CS_NO_ERROR;

	E2P_DeviceId_t		DevId = 0;
	E2P_OpenParams_t	E2POpenParams;
	E2P_Handle_t			E2pHandle;

	EVT_SubscribeParams_t		UserParams;
	CSEVT_SubscriberID_t 		SubscriberID;

	EVT_Setup();

	I2C_Setup();

	E2P_Setup();

//	E2POpenParams.AddressType = CSI2C_ADDRESS_7_BITS;
//	E2POpenParams.I2cAddress = 0x50;
//	E2POpenParams.BaudRate = CSI2C_RATE_FASTMODE;
	E2P_Open(DevId, &E2POpenParams, &E2pHandle);
#ifdef	EVT_READWRITE

	UserParams.Callback = E2P_Callback;
	//UserParams.SubscriberData_p = NULL;
	E2P_SubscribeEvent(UserParams, &SubscriberID);
#endif

/**************************    Write & Read  Test    **********************************/

	/*   --------- the whole chip  write & read    test ---------  */

	U8	*ReadBuff;
        U8	*WriteBuff;
        U32	icount,jcount;
	/* Memory allocation */
    ReadBuff = (U8 *)CSOS_AllocateMemory(0, 8*1024);
    WriteBuff = (U8 *)CSOS_AllocateMemory(0, 8*1024);
    /* Initialize test data */
    for( icount = 0; icount < 7; icount++ )
        memset( WriteBuff+icount*1024, icount+170+17, 1024 );   /* 170: 0x10101010 */

	memset( WriteBuff+7*1024, 85+17, 1024 );					/* 85: 0x01010101 */

		memset( ReadBuff, 9, 8*1024 );
	/* Write test */
    #ifndef	EVT_READWRITE
	printf("please wait for Writing data .....\n\n");
	ErrorCode = E2P_Write( E2pHandle, 0x0000, WriteBuff, 8*1024 );
	if(ErrorCode != CS_NO_ERROR)
        printf(" e2p_Test(): Write the E2P error ! %x \n\n", ErrorCode);
#else
	ErrorCode = E2P_WRITE( E2pHandle, 0x0000, WriteBuff, 8*1024, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_WRITE error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Write Message OK...\n");
#endif
	/* Read test */#ifndef	EVT_READWRITE
	printf("please wait for Reading data .....\n\n");
	ErrorCode = E2P_Read( E2pHandle, 0x0000, ReadBuff,8*1024 );
	if(ErrorCode != CS_NO_ERROR)
        printf(" e2p_Test(): Read the E2P error ! %x \n\n", ErrorCode);

	printf("\n----write read over----\n");
	boolean	cmpresult=true;
	for( jcount = 0; jcount < 8*1024; jcount++)
	{
		/*printf("%d. 0x%x\n", jcount, ReadBuff[jcount] );	if( ((jcount %29) == 0) )
		{	printf( "please press Enter key....." );		getchar();		}*/
		if( ReadBuff[jcount] != WriteBuff[jcount] )
		{
			printf("data compare wrong: ReadBuff[%d]=0x%x WriteBuff[%d]=0x%x\n",jcount,ReadBuff[jcount],jcount,WriteBuff[jcount]);
			cmpresult = false;
		}
	}
	if(cmpresult == true)
		printf("\n------read data ringht!------\n\n");
#else
	ErrorCode = E2P_READ( E2pHandle, 0x0000, ReadBuff, 8*1024, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_READ error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Read Message OK...\n");
#endif


	/*   --------- Random  write & read    test ---------  */	U8	ReBuf[32];	U8	WrBuf[32];	U32	kcount;
	/* Initialize test data */
	WrBuf[0] = 0x01;WrBuf[1] = 0x12;WrBuf[2] = 0x23;WrBuf[3] = 0x34;WrBuf[4] = 0x45;WrBuf[5] = 0x56;	WrBuf[6] = 0x67;WrBuf[7] = 0x78;WrBuf[8] = 0x89;WrBuf[9] = 0x9a;WrBuf[10] = 0xab;WrBuf[11] = 0xbc;	WrBuf[12] = 0xcd;WrBuf[13] = 0xde;WrBuf[14] = 0xef;WrBuf[15] = 0xff;WrBuf[16] = 0x12;WrBuf[17] = 0x34;	WrBuf[18] = 0x56;WrBuf[19] = 0x78;WrBuf[20] = 0x90;WrBuf[21] = 0x13;WrBuf[22] = 0x57;WrBuf[23] = 0x90;	WrBuf[24] = 0x00;
	/* Write test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Write( E2pHandle, 0x1000, WrBuf, 24 );
	if(ErrorCode != CS_NO_ERROR)		printf(" e2p_Test(): Write the E2P error ! %x \n\n", ErrorCode);
#else
	ErrorCode = E2P_WRITE( E2pHandle, 0x1000, WrBuf, 24, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_WRITE error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Write Message OK...\n");
#endif

	/* Read test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Read(E2pHandle, 0x1000, ReBuf, 24 );
	if(ErrorCode != CS_NO_ERROR)		printf(" e2p_Test(): Read the E2P error ! %x \n\n", ErrorCode);

	for( kcount = 0; kcount < 24; kcount++)		printf("%d. 0x%x\n", kcount, ReBuf[kcount] );
#else
	ErrorCode = E2P_READ( E2pHandle, 0x1000, ReBuf, 24, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_READ error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Read Message OK...\n");
#endif

	/* Write test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Write(E2pHandle, 0x0501, WrBuf+16, 3 );
	if(ErrorCode != CS_NO_ERROR)
        printf(" e2p_Test(): Write the E2P error ! %x \n\n", ErrorCode);
#else
	ErrorCode = E2P_WRITE( E2pHandle, 0x0501, WrBuf+16, 3, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_WRITE error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Write Message OK...\n");
#endif

	/* Read test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Read(E2pHandle, 0x0501, ReBuf, 3 );
	if(ErrorCode != CS_NO_ERROR)
        printf(" e2p_Test(): Read the E2P error ! %x \n\n", ErrorCode);

	for( kcount = 0; kcount < 3; kcount++)
        printf("%d. 0x%x\n", kcount, ReBuf[kcount] );
#else
	ErrorCode = E2P_READ( E2pHandle, 0x0501, ReBuf, 3, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_READ error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Read Message OK...\n");
#endif

	/* Write test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Write(E2pHandle, 0x0000, WrBuf, 25 );
	if(ErrorCode != CS_NO_ERROR)		printf(" e2p_Test(): Write the E2P error ! %x \n\n", ErrorCode);
#else
	ErrorCode = E2P_WRITE( E2pHandle, 0x0000, WrBuf, 25, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_WRITE error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Write Message OK...\n");
#endif

	/* Read test */
#ifndef	EVT_READWRITE
	ErrorCode = E2P_Read(E2pHandle, 0x0000, ReBuf, 25 );
	if(ErrorCode != CS_NO_ERROR)		printf(" e2p_Test(): Read the E2P error ! %x \n\n", ErrorCode);

	for( kcount = 0; kcount < 25; kcount++)		printf("%d. 0x%x\n", kcount, ReBuf[kcount] );
#else
	ErrorCode = E2P_READ( E2pHandle, 0x0000, ReBuf, 25, SubscriberID );
	if(ErrorCode != CS_NO_ERROR)
		printf(" e2p_Test(): E2P_READ error ! %x \n\n", ErrorCode);
	else
		printf("\nSend Read Message OK...\n");
#endif

/**************************    Write & Read  Test  Over  **********************************/

	while(1)
	{
		CSOS_DelayTaskMs(15);
	}

	return;
}
#endif

#endif

#if 0
void init_display_mode(void)
{
   tve_handle = CSTVOUT_Open();
   if( NULL == tve_handle )  return;
   CSTVOUT_SetMode( tve_handle, TVOUT_MODE_576I );

   CSTVOUT_SetOSDMode( tve_handle, TVOUT_OSD_LAYER0, TVOUT_OSD_MODE_576 );

   CSTVOUT_EnableOSDLayer( tve_handle, TVOUT_OSD_LAYER0 );
}
#endif
int CS_DRV_Init(void)
{

    printf("CS_DRIVER_INIT Started...\n");
	BOOL	status;

	TunerInitial();
	// printf("tunerHandleId = %d\n", Tuner_HandleId[0]);

	printf("Tuner Driver init succesfully!!...\n");

#ifdef USE_HDMI_CAT6611
	if( Cat6611_init() != 0 )
	{
		dprintf(("Orion Cat6611_init Failed ...\n"));
		return FALSE;
	}

	printf("HDMI Driver init succesfully!!...\n");
#else
	if( CSHDMI_Init() != HDMI_SUCCESS )
	{
		dprintf(("Orion HDMI Init Failed ...\n"));
		return FALSE;
	}

	printf("HDMI Driver init succesfully!!...\n");
#endif

	dprintf(("CSHDMI_Init =  time[%d]\n",  CS_OS_time_now()));

	if( CSDEMUX_Init() != CSAPI_SUCCEED )
	{
		dprintf(("Orion Xport Init Failed ...\n"));
		return FALSE;
	}

	printf("DEMUX Driver init succesfully!!...\n");

	//E2P_Init(0xa0);

	status = CS_CRC_Init();

	printf("CRC Driver init succesfully!!...\n");

	status = MV_DB_Init(TRUE);
	printf("Database Driver init succesfully!!...\n");
	//MV_DB_Output_File_Test();
	//printf("Database output file test succesfully!!...\n");

	if ( status == FALSE )
		dprintf(("MV_DB_Init error = %d, time[%d]\n", status, CS_OS_time_now()));

	status = CS_AV_Init();

	printf("CSAV Driver init succesfully!!...\n");
	if ( status == FALSE )
		dprintf(("CS_AV_Init error = %d, time[%d]\n", status, CS_OS_time_now()));

	ScartInit();       /* By KB Kim : 2010_08_31 for Scart Control */

	printf("SCART Driver init succesfully!!...\n");

	/* For Heart bit control */
	HeartBitInit();

	printf("Heartbit Driver init succesfully!!...\n");

	/* By KB Kim 2010.08.31 for RGB Control */
	CS_AV_SetTVOutput();

	printf("CS_DRIVER_INIT Finished...\n");

	return CS_NO_ERROR;

}

#if 0
int CS_DRV_Open(void)
{
    //1 line sync 0 host sync
    CSDEMUX_WriteReg(0x41400000+(0x0009<<2), 0xf);

    xport_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
    if(xport_chl_handle == CSDEMUX_UNVALID_HANDLE)
    {
        printf("Orion Xport Channel0 Open Failed ...\n");
        CSDEMUX_Terminate();
        return CSAPI_FAILED;
    }

    //CHL Config
    CSDEMUX_CHL_SetInputMode( xport_chl_handle, DEMUX_INPUT_MOD_TUNER );

    CSDEMUX_CHL_Enable( xport_chl_handle );


    return CSAPI_SUCCEED;


}
#endif

U32 CS_SYS_GetHandle(eSysHandleType type)
{
	U32 		SysHandle = 0;

	switch(type)
	{
		case CS_SYS_XPORT_HANDLE:
		//	SysHandle=(U32)CSDEMUX_HANDLE;
			break;
		case CS_SYS_BLIT_HANDLE:
		//	SysHandle=(U32)BLIT_handle;
			break;
		case CS_SYS_DISP_HANDLE:
		//	SysHandle=(U32)DISP_handle;
			break;

		case CS_SYS_FLASH_HANDLE:
		//  SysHandle=(U32)Flash_handle;
			break;
		case CS_SYS_E2P_HANDLE:
		//  SysHandle=(U32)E2P_handle;
			break;

		default:

			break;

	}

	return SysHandle;
}

BOOL CS_SYS_Init(void)
{
	BOOL	status;
	tCS_DT_Date  start_date;
	tCS_DT_Time	start_time;

	status = DB_DemuxInit();
	// printf("DB_DemuxInit error = %d, time[%d]\n", status, CS_OS_time_now());
	status = SiInitTable();
	if (status)
	{
		printf("SiInitTable error = %d, time[%d]\n", status, CS_OS_time_now());
	}

	start_date.year = 2010;
	start_date.month = 1;
	start_date.day = 1;

	start_time.hour = 0;
	start_time.minute = 0;

	status = CS_DT_Init(start_date, start_time);

	printf("CS_DT_Init error = %d, time[%d]\n", status, CS_OS_time_now());

#if 1
	status = CS_FE_Init();
	printf("CS_FE_Init error = %d, time[%d]\n", status, CS_OS_time_now());

    status = CS_INSTALL_Init();
	printf("CS_INSTALL_Init error = %d, time[%d]\n", status, CS_OS_time_now());
#endif
    status = CS_EIT_Init();
    printf("CS_EIT_Init error = %d, time[%d]\n", status, CS_OS_time_now());

    status = CS_TIMER_Init();
    printf("CS_TIMER_Init error = %d, time[%d]\n", status, CS_OS_time_now());

    /*status = CS_GFX_Init();
    printf("CS_GFX_Init error = %d\n", status);*/
    printf("CS_DT_Start\n");

	PopUp_Init(NULL);
	CAS_PopUp_Init(NULL);

	UsbCon_Init(NULL);
	printf("USB Connect Init Success ...\n");

	CSMPR_Player_Init();
	CSMPR_Record_Init();
/*
	if( CS_PVR_Init(CS_PVR_STATUS_IDLE) != TRUE )
	{
		printf("PVR Init Failed ...\n");
	 	return FALSE;
	}
*/
	printf("PVR Init Success ...\n");
	printf("=================================================\n\n\n\n");
    /*if( CSHDMI_Init() != HDMI_SUCCESS )
    {
        printf("Orion HDMI Init Failed ...\n");
        return FALSE;
    }*/

    return(TRUE);

}



