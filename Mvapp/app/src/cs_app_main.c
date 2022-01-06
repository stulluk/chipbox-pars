
#define  __ECOS
#include "linuxos.h"

#ifdef	CHENPENG_TEST
#include "database.h"
#include "fe_mngr.h"
#include "date_time.h"
#endif

#include "mwlayers.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"
#include "csmpr_usb.h"
#include "mvosapi.h"
#include "userdefine.h"
#include "mv_oscam.h"

#include "casapi.h"  /* By KB Kim : 2010_07_23 */
#include "security.h"  /* By KB Kim : 2010_11_16 */

//#define	CHENPENG_TEST
#define MV_DBG_TRACE         printf( "[%s]\n", __FUNCTION__ )
//static varaible

static CSAPP_Applet_t 	CurrentGuiApplet;
static BOOL				Direct_CurrentGui_State = FALSE;
static BOOL				Direct_CurrentGui_Type = FALSE;
static BOOL				Check_First_Pass = TRUE;

/* For Front Communication By KB Kim 2011.02.01 */
U8 CurrentBootMode = (U8)BOOT_MODE_UNKNOWN;
U8 CurrentPowerMode = 0;

static void CS_Gui_MainHandle(void);
extern CSAPP_Applet_t  CSApp_Factory_Test(void);
extern void Motor_Moving_Stop(void);
extern CSAPP_Applet_t CSApp_Smart_Menu(void);
/* For RCU Emulation */
extern BOOL IMessageInit(void);

U32                   InfoPipe_client;

void MV_Set_BootMode(SystemBootMede Bootmode)
{
	CurrentBootMode = (U8)Bootmode;
}

U8 MV_Get_BootMode(void)
{
	return CurrentBootMode;
}

int MiniGUIMain (int argc, const char *argv[])  				//cs_app_main  (int argc, const char *argv[])
{
	// Sertac just to avoid compiler warnings
	printf("Argument count is %d , my name is %s \n", argc, argv[0]);
	int i=0;

	printf("MiniGUIMain %d\n", CS_OS_time_Get_Sec());

	CasDrvInit();  /* By KB Kim : 2010_07_23 */
	CS_MW_Init();

	/* For Front Communication By KB Kim 2011.02.01 */
	CurrentBootMode  = FbGetBootMode();

	/* For AC Power ON Control By KB Kim 2011.03.11 */
	CurrentPowerMode = GetPowerStatus();
	// printf("MiniGUIMain: Boot mode = %d\n", CurrentBootMode);
	// printf("MiniGUIMain: Power mode = %d\n", CurrentPowerMode);
	if (CurrentPowerMode != 0)
	{
		CurrentPowerMode = 1;
	}

	if ((CurrentBootMode == BOOT_AC_POWER_ON) && (CurrentPowerMode == 0))
	{
		ClearOSD();
		CS_MW_StopService(TRUE);
		CSOS_DelayTaskMs(200);
		CS_AV_DisableTVOut();
		FbSendFndDisplay("----");
		if ( CS_DBU_GetPower_Type() == 1 )
		{
			if ( CS_DBU_GetLED_Type() == 1 )
				FbRequestShoutDowm(1);
			else
				FbRequestShoutDowm(0);
		}
	}
	else
	{
		if (CurrentPowerMode == 0)
		{
			CurrentPowerMode = 1;
			SetPowerStatus(CurrentPowerMode);
		}
	}


	Sc41Init();  /* By KB Kim : 2010_11_16 */

	if ( CFG_Default_Value || CS_MW_GetLcnMode()==eCS_DB_Operator_Defined)
	{
		// printf("\n===>>>>>> RUN CS_DBU_InitDefaultUserData ====>>> %d\n\n", i);
		CS_DBU_InitDefaultUserData();
	}
	//printf("\n\n ========== Loading Images Ready ============\n\n");
	if ( !CFG_Factory_Mode )
	{
		//printf("\n\n ========== Loading Images Start ============\n\n");
		i = MV_LoadBmp(TRUE);

		if ( i > 0 )
			printf("===>>>>>> LoadBmp Error ====>>> %d\n", i);
	}

	SetKeyInputControl(TRUE);

	/* For Start Picture problem */
	CS_AV_VideoBlank();

	/* For 576i Mode booting problem by KB Kim 20101225 */
	CS_MW_Init_VideoDefinition(1);

	/* For RCU Emulation */
	IMessageInit();

	CS_Gui_MainHandle();

	return 0;

}

#include <termios.h>
int 	fd;  /*�����豸�ļ�������*/

int lowpower_open_serial(int k)
{
	if(k==0)       /*����ѡ��*/
	{
		fd = open("/dev/ttyS0",O_RDWR|O_NOCTTY);  /*��д��ʽ�򿪴���*/
		printf("open /dev/ttyS0");
	}
	else
	{
		fd = open("/dev/ttyS1",O_RDWR|O_NOCTTY);
		printf("open /dev/ttyS1");
	}

	if(fd == -1)  /*��ʧ��*/
	{
		printf("Open serial %d fail\n",k);
		return -1;
	}
	else
		printf("Open serial %d success\n",k);

	return 0;

}

int lowpower_send_serial(void)
{
	char 	sbuf[]={"Hello,this is a Serial_Port test!\n"};/*�����͵����ݣ���\nΪ������־*/

	int 		retv;
	int 		flag_close;

	struct termios option;

	int length=sizeof(sbuf);/*���ͻ��������ݿ���*/

	/*******************************************************************/

	lowpower_open_serial(0);    /*�򿪴���1*/

	/*******************************************************************/

	printf("ready for sending data...\n"); /*׼����ʼ��������*/

	tcgetattr(fd,&option);

	cfmakeraw(&option);

	/*****************************************************************/

	cfsetispeed(&option,B115200); /*����������Ϊ9600bps*/

	cfsetospeed(&option,B115200);

	/*******************************************************************/

	tcsetattr(fd,TCSANOW,&option);

	retv=write(fd,sbuf,length);  /*��������*/

	if(retv==-1)
	{
		printf("write");
	}

	//printf("the number of char sent is %d\n",retv);

#if 1

	flag_close =close(fd);

	if(flag_close ==-1)   /*�ж��Ƿ�ɹ��ر��ļ�*/
		printf("Close the Device failur��\n");

#endif

	return 0;

}

//#include "usb_mount.h"

//extern int UsbCon_Umount( void );

/* By KB Kim 2011.01.13 */
extern U8	u8Glob_Sat_Focus;
extern U8	u8Glob_TP_Focus;

static void CS_Gui_MainHandle(void)
{
	//CurrentGuiApplet=CSApp_Applet_Desktop;

	Check_First_Pass = TRUE;
	/* By KB Kim 2011.01.13 */
	u8Glob_Sat_Focus = 0;
	u8Glob_TP_Focus  = 0;
	if (CurrentPowerMode)
	{
		if ( CFG_Factory_Mode )
		{
			//CurrentGuiApplet=CSApp_Applet_MainMenu;
			CurrentGuiApplet=CSApp_Applet_MainMenu;
		}
		else
		{
			if ( MV_DB_GetALLServiceNumber() == 0 ) {
				CurrentGuiApplet=CSApp_Applet_MainMenu;
			} else {
				CurrentGuiApplet=CSApp_Applet_Desktop;
			}
		}
	}
	else
	{
		CurrentGuiApplet = CSApp_Applet_Sleep;
		printf("\n---------->>>>>>    Goto Sleep Mode \n");
	}
	//Mac_Task();
	//Conax_init();

	while(CurrentGuiApplet!=CSApp_Applet_Exit)
	{
		//printf("===== %d ===========================\n", Direct_CurrentGui_State);
		if ( CS_DBU_GetAntenna_Type() == SCAN_MODE_DISECQ_MOTOR || CS_DBU_GetAntenna_Type() == SCAN_MODE_USALS )
			Motor_Moving_Stop();

		if ( Direct_CurrentGui_State == TRUE )
		{
			//printf("=== GOTO =====\n");
			if ( Direct_CurrentGui_Type == TRUE )
			{
				//printf("=== GOTO PVR =====\n");
				CurrentGuiApplet = CSApp_Applet_Pvr_Record;
			}
			else
			{
				//printf("=== GOTO DESCTOP =====\n");
				CurrentGuiApplet = CSApp_Applet_Desktop;
			}

			Direct_CurrentGui_State = FALSE;
		}

		if ( CurrentGuiApplet == CSApp_Applet_Desktop && MV_DB_GetALLServiceNumber() == 0 )
			CurrentGuiApplet = CSApp_Applet_MainMenu;

//		else
		{
			switch(CurrentGuiApplet)
			{
				case CSApp_Applet_MainMenu:
					if ( CFG_Factory_Mode )
						CurrentGuiApplet = CSApp_Factory_Test();
					else
						CurrentGuiApplet = CSApp_MainMenu();

					b8Last_App_Status = CSApp_Applet_MainMenu;
					break;

				case CSApp_Applet_RDList:
				case CSApp_Applet_TVList:
				case CSApp_Applet_TVFAVList:
				case CSApp_Applet_RADIOFAVList:
				case CSApp_Applet_TVSATList:
				case CSApp_Applet_RADIOSATList:
					CurrentGuiApplet=CSApp_SList(CurrentGuiApplet);
					b8Last_App_Status = CSApp_Applet_TVList;
					break;

				case CSApp_Applet_Ext_RDList:
				case CSApp_Applet_Ext_TVList:
				case CSApp_Applet_Ext_TVFAVList:
				case CSApp_Applet_Ext_RADIOFAVList:
				case CSApp_Applet_Ext_TVSATList:
				case CSApp_Applet_Ext_RADIOSATList:
					// printf("\n===================================\n");
					CurrentGuiApplet=CSApp_Ext_SList(CurrentGuiApplet);
					b8Last_App_Status = CSApp_Applet_Ext_TVList;
					break;

				case CSApp_Applet_Install:
					CurrentGuiApplet=CSApp_Install();
					b8Last_App_Status = CSApp_Applet_Install;
					break;

				case CSApp_Applet_Sat_Setting:
					CurrentGuiApplet=CSApp_Sat_Setting();
					b8Last_App_Status = CSApp_Applet_Sat_Setting;
					break;

				case CSApp_Applet_TP_Setting:
					CurrentGuiApplet=CSApp_TP_Setting();
					b8Last_App_Status = CSApp_Applet_TP_Setting;
					break;

				case CSAPP_Applet_D_motor:
					CurrentGuiApplet=CSApp_Disecq_Motor();
					b8Last_App_Status = CSAPP_Applet_D_motor;
					break;

				case CSAPP_Applet_USALS:
					CurrentGuiApplet=CSApp_Usals_Motor();
					b8Last_App_Status = CSAPP_Applet_USALS;
					break;

				case CSAPP_Applet_UniCable:
					CurrentGuiApplet=CSApp_UniCable();
					b8Last_App_Status = CSAPP_Applet_UniCable;
					break;

				case CSapp_Applet_SystemInfo:
					CurrentGuiApplet=CSApp_Sysinfo();
					b8Last_App_Status = CSapp_Applet_SystemInfo;
					break;

				case CSapp_Applet_EditTV:
				case CSapp_Applet_EditRadio:
				case CSApp_Applet_EditTVFAV:
				case CSApp_Applet_EditRADIOFAV:
				case CSApp_Applet_EditTVSAT:
				case CSApp_Applet_EditRADIOSAT:
					CurrentGuiApplet=CSApp_EditSList(CurrentGuiApplet);
					b8Last_App_Status = CSapp_Applet_EditTV;
					break;

				case CSAPP_Applet_Install_Result:
					CurrentGuiApplet=CSApp_SearchResult();
					b8Last_App_Status = CSAPP_Applet_Install_Result;
					break;

				case CSApp_Applet_TV_EPG:
				case CSApp_Applet_Radio_EPG:
				case CSApp_Applet_FAV_TV_EPG:
				case CSApp_Applet_FAV_Radio_EPG:
				case CSApp_Applet_SAT_TV_EPG:
				case CSApp_Applet_SAT_Radio_EPG:
					CurrentGuiApplet=CSApp_Epg(CurrentGuiApplet);
					b8Last_App_Status = CSApp_Applet_TV_EPG;
					break;
#ifdef DAILY_EPG
				case CSApp_Applet_Daily_TV_EPG:
				case CSApp_Applet_Daily_Radio_EPG:
				case CSApp_Applet_Daily_FAV_TV_EPG:
				case CSApp_Applet_Daily_FAV_Radio_EPG:
				case CSApp_Applet_Daily_SAT_TV_EPG:
				case CSApp_Applet_Daily_SAT_Radio_EPG:
					CurrentGuiApplet=CSApp_Daily_Epg(CurrentGuiApplet);
					b8Last_App_Status = CSApp_Applet_Daily_TV_EPG;
					break;
#endif
				case CSApp_Applet_Desktop:
				case CSApp_Applet_Banner:
				case CSApp_Applet_Normal:
				case CSApp_Applet_Error:
					if ( Check_First_Pass == TRUE && CS_DBU_GetBootLockStatus() == eCS_DBU_ON )
					{
						while(1)
						{
							if( CSApp_Check_Pass() == CSApp_Applet_Exit )
								break;
						}
					}

					if ( b8Last_App_Status == CSApp_Applet_Sleep )
						MV_MW_StartService(CS_DB_GetCurrentServiceIndex());

					Check_First_Pass = FALSE;
					CurrentGuiApplet = CSApp_Desktop();
					b8Last_App_Status = CSApp_Applet_Desktop;
					break;

				case CSApp_Applet_Audio:
					CurrentGuiApplet = CSApp_Audio();
					b8Last_App_Status = CSApp_Applet_Audio;
					break;

				case CSAPP_Applet_Desk_CH_Edit:
					CurrentGuiApplet = CSApp_DeskCh_Edit();
					b8Last_App_Status = CSAPP_Applet_Desk_CH_Edit;
					break;

				case CSAPP_Applet_Recall:
					CurrentGuiApplet = CSApp_Recall();
					b8Last_App_Status = CSAPP_Applet_Recall;
					break;

				case CSAPP_Applet_Teletext:
					CurrentGuiApplet = CSApp_Teletext();
					b8Last_App_Status = CSAPP_Applet_Teletext;
					break;

				case CSAPP_Applet_Subtitle:
					CurrentGuiApplet = CSApp_Subtitle();
					b8Last_App_Status = CSAPP_Applet_Subtitle;
					break;

				case CSapp_Applet_SystemSetting:
					CurrentGuiApplet=CSApp_Setting();
					b8Last_App_Status = CSapp_Applet_SystemSetting;
					break;

				case CSApp_Applet_AVSetting:
					CurrentGuiApplet = CSApp_AVSetting();
					b8Last_App_Status = CSApp_Applet_AVSetting;
					break;

				case CSApp_Applet_Language:
					CurrentGuiApplet = CSApp_LangSetting();
					b8Last_App_Status = CSApp_Applet_Language;
					break;

				case CSApp_Applet_TimeSetting:
					CurrentGuiApplet = CSApp_TimeSetting();
					b8Last_App_Status = CSApp_Applet_TimeSetting;
					break;

				case CSApp_Applet_PinSetting:
					CurrentGuiApplet = CSApp_PinSetting();
					b8Last_App_Status = CSApp_Applet_PinSetting;
					break;

				case CSApp_Applet_NetSetting:
					CurrentGuiApplet = CSApp_NetworkSetting();
					b8Last_App_Status = CSApp_Applet_NetSetting;
					break;

				case CSApp_Applet_Rec_File:
					CurrentGuiApplet = CSApp_RecFile();
					b8Last_App_Status = CSApp_Applet_Rec_File;
					break;

#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
				case CSApp_Applet_Rec_Config:
					CurrentGuiApplet = CSApp_RecConfig();
					b8Last_App_Status = CSApp_Applet_Rec_Config;
					break;
#endif

				case CSApp_Applet_File_Tool:
					CurrentGuiApplet = CSApp_FileTools();
					b8Last_App_Status = CSApp_Applet_File_Tool;
					break;

				case CSApp_Applet_USB_Remove:
					UsbCon_Umount();
					usleep( 5000*1000 );
					//CurrentGuiApplet = MVApp_USB_Remove();
					CurrentGuiApplet = CSApp_MainMenu();
					b8Last_App_Status = CSApp_Applet_USB_Remove;
					break;

				case CSApp_Applet_Storage_Info:
					CurrentGuiApplet = MVApp_Storage_Info();
					b8Last_App_Status = CSApp_Applet_Storage_Info;
					break;

				case CSApp_Applet_Media_Player:
					CurrentGuiApplet = CSApp_MPlayer();
					b8Last_App_Status = CSApp_Applet_Media_Player;
					break;

				case CSApp_Applet_Upgrade:
					CurrentGuiApplet = CSApp_Upgreade();
					b8Last_App_Status = CSApp_Applet_Upgrade;
					break;

				case CSApp_Applet_CI:
					CurrentGuiApplet = CSApp_CI();
					b8Last_App_Status = CSApp_Applet_CI;
					break;

				case CSApp_Applet_CAS:
					CurrentGuiApplet = CSApp_CAS();
					b8Last_App_Status = CSApp_Applet_CAS;
					break;

				case CSApp_Applet_Backup:
					CurrentGuiApplet = CSApp_Backup();
					b8Last_App_Status = CSApp_Applet_Backup;
					break;

				case CSApp_Applet_Restore:
					CurrentGuiApplet = CSApp_Restore();
					b8Last_App_Status = CSApp_Applet_Restore;
					break;

				case CSApp_Applet_PlugIn:
					CurrentGuiApplet = CSApp_PlugIn();
					b8Last_App_Status = CSApp_Applet_PlugIn;
					break;

				case CSApp_Applet_Reset:
					CurrentGuiApplet = CSApp_Reset();
					b8Last_App_Status = CSApp_Applet_Reset;
					break;

				case CSAPP_Applet_Video_Control:
					CurrentGuiApplet = CSApp_Video_Set();
					b8Last_App_Status = CSAPP_Applet_Video_Control;
					break;

				case CSAPP_Applet_Finder:
					CurrentGuiApplet = CSApp_Finder();
					b8Last_App_Status = CSAPP_Applet_Finder;
					break;

				case CSAPP_Applet_KeyEdit:
					CurrentGuiApplet = CSApp_KeyEdit();
					b8Last_App_Status = CSAPP_Applet_KeyEdit;
					break;

				case CSApp_Applet_ExtInfo:
					CurrentGuiApplet = CSApp_ExtInfo();
					b8Last_App_Status = CSApp_Applet_ExtInfo;
					break;

				case CSApp_Applet_Sleep:
					Check_First_Pass = TRUE;
					CurrentGuiApplet = CSApp_SysSleep();
					b8Last_App_Status = CSApp_Applet_Sleep;
					break;

				case CSApp_Applet_Pvr_Record:
					CurrentGuiApplet = CSApp_PVR_Record();
					b8Last_App_Status = CSApp_Applet_Pvr_Record;
					break;

/****************v38den itibaren....************************/
                case CSApp_Applet_Pvr_Streaming:
					CurrentGuiApplet = CSApp_PVR_Streaming();
					b8Last_App_Status = CSApp_Applet_Pvr_Streaming;
					break;
/**********************************************************/

				case CSApp_Applet_Pvr_Player:
					CurrentGuiApplet = CSApp_PVR_Player();
					b8Last_App_Status = CSApp_Applet_Pvr_Player;
					break;

				case CSApp_Applet_Timer:
					CurrentGuiApplet = CSApp_Timer();
					b8Last_App_Status = CSApp_Applet_Timer;
					break;

				case CSApp_Applet_Push:
					CurrentGuiApplet = CSApp_Push();
					b8Last_App_Status = CSApp_Applet_Push;
					break;

				case CSApp_Applet_Calendar:
					CurrentGuiApplet = CSApp_Calendar();
					b8Last_App_Status = CSApp_Applet_Calendar;
					break;
#ifdef SMART_PHONE
				case CSApp_Applet_Smart_OSD:
					CurrentGuiApplet = CSApp_Smart_Menu();
					b8Last_App_Status = CSApp_Applet_Smart_OSD;
					break;
#endif
				case CSApp_Applet_Change_Fav:
					CurrentGuiApplet = CSApp_Fav_Edit();
					b8Last_App_Status = CSApp_Applet_Change_Fav;
					break;

				case CSApp_Applet_OSCAM_Setting:
					CurrentGuiApplet = CSApp_OSCAM();
					b8Last_App_Status = CSApp_Applet_OSCAM_Setting;
					break;

				default:
					CurrentGuiApplet = CSApp_Applet_Exit;
					break;
			}
		}
	}
   	//MiniGUIExtCleanUp ();
}

void MV_Set_CurrentGuiApplet(BOOL SetType)
{
	//MV_DBG_TRACE;
	Direct_CurrentGui_State = TRUE;
	Direct_CurrentGui_Type = SetType;
	BroadcastMessage (MSG_CLOSE, 0, 0);
}

//MTAB_ENTRY(romfs_mte1,"/","romfs","",(CYG_ADDRWORD)0x34410000);


