/****************************************************************************
** Copyright (c) 2006-2007
** All Right Reserved
**
** author:
**
** Description:
**    Csdesktop.c
**        This file contains all functions and routines which implemented desktop
**
** History:
**		version		date		name		desc
**		1.0.0             2007.09.13 	lsq		create
**
**     I do not like changelogs,so eliminate them.
**
****************************************************************************/
#include "linuxos.h"

#include "demux.h"
#include "database.h"
#include "date_time.h"
#include "eit_engine.h"
#include "av_zapping.h"
#include "timer.h"
#include "csttxdraw.h"

#include "mwsetting.h"
#include "mwsvc.h"
#include "mwpublic.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"
#include "csmpr_usb.h"

#include "mv_gui_interface.h"
#include "sattp.h"
#include "timer.h"
#include "ui_common.h"
#include "mvosapi.h"
#include "csinstall.h"
#include "cstimesetting.h"
#include "dvbtuner.h"

#define		DESK_EPG
#define		FOR_HW_TEST

#define		Banner_timer_Max		( CS_DBU_GetBannerKeepTime()*1000 )
#define		Volume_timer_Max		2*1000
#define		VMode_timer_Max			6*1000
#define		Input_timer_Max			3*1000
#define		Info_txt_timer_Max		1000

#define     DESKTOP_CHANNEL_CHANGE_GAP 		1000
#define     DESKTOP_CHANNEL_CHANGE_TIMER 	500
#define     DESKTOP_CHANNEL_CHANGE_COUNT 	2

#define 	kNbSecondPerMinute 		60
#define 	kNbSecondPerHour 		3600
#define 	kNbMinutePerHour 		60
#define 	kNbHourPerDay			24

/**************************** Info Banner *****************************************/

#define		BANNER_CAPTURE_X		0
#define		BANNER_CAPTURE_DX		1280
#define 	BANNER_STAR_X			100
#define		BANNER_STAR_Y			572
#define 	BANNER_STAR_W			1080
#define		BANNER_STAR_H			148
#define		BANNER_CLEAR_H			(BANNER_STAR_H + (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2))
#define		BANNER_CIRCLE_X			(BANNER_STAR_X + (BANNER_STAR_W/2) - (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth/2))
#define		BANNER_CIRCLE_Y			(BANNER_STAR_Y - (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2))
#define		BANNER_ITEM_X			122
#define		BANNER_ITEM_Y1			572
#define		BANNER_ITEM_Y2			602
#define		BANNER_ITEM_Y3			640
#define		BANNER_ITEM_H			28
#define		BANNER_NAME_X			BANNER_ITEM_X
#define 	BANNER_NAME_DX			240
#define		BANNER_ICON_X			(BANNER_ITEM_X + BANNER_NAME_DX)
#define		BANNER_ICON_DX			30
#define		BANNER_TIME_X 			( BANNER_ITEM_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 100 )
#define 	BANNER_EPG_X			(BANNER_CIRCLE_X + MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth + 10)

/**********************************************************************************/

static CSAPP_Applet_t		CSApp_Desktop_Applets;
static tCSDesktopInfo		DeskData;
static BOOL					b8NoSignal_Status = FALSE; // by dreamcom .. For not display Scaramble at time No Signal
static BOOL					b8Scramble_Status = FALSE; // by dreamcom .. For Not Redraw Scaramble
static BOOL					b8Inforbar_Check_Flag = FALSE;
static U8					u8Sleep_Time;
static BOOL					u8Sleep_Time_Flag = FALSE;

static BITMAP				bt_banner;

extern CSVID_HANDLE			vid_handle;
extern CSAUD_HANDLE			aud_handle;

/* By KB Kim 2011.06.02 */
static U8  DestTopUnderFlow;

static int Desktop_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
static void DesktopPaint(HWND hwnd,tCSDesktopInfo* DrawData);
static void UpdateBannerText(tCSDesktopInfo* DrawData);
static void InitDesktopData(void);
static void DesktopBannerScroll(tCSDesktopInfo* DrawData);
static void DesktopVolumeScroll(tCSDesktopInfo* DrawData,BOOL right);
static void GetBannerData(tCSDesktopInfo* DrawData);
#ifdef DESK_EPG
static void UpdateEPGData(tCSDesktopInfo* DrawData);
#endif
static void DesktopNotify(tMWNotifyData NotifyData);
static void DesktopPaintInputBack(HDC hdc, BOOL b8Kind, char *Temmp_Str);

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U8 SubtitleNumber = 0;



char Video_Mode[6][10] = {
	"480I",
	"576I",
	"576P",
	"720P",
	"1080I",
	"AUTO"
};

//static void VideoFormatNotify(eCS_MW_PIPE_TYPE PipeType,tCS_MW_MSG_INFO MsgInfo);

U16 Get_Width_Number(char* text)
{
	U8		i;
	U16		width = 0;

	for ( i = 0 ; i < PROGRAM_MAX_NUM ; i++ )
	{
		switch(text[i])
		{
			case '0':
				width += MV_BMP[MVBMP_0_ICON].bmWidth + 2;
				break;
			case '1':
				width += MV_BMP[MVBMP_1_ICON].bmWidth + 2;
				break;
			case '2':
				width += MV_BMP[MVBMP_2_ICON].bmWidth + 2;
				break;
			case '3':
				width += MV_BMP[MVBMP_3_ICON].bmWidth + 2;
				break;
			case '4':
				width += MV_BMP[MVBMP_4_ICON].bmWidth + 2;
				break;
			case '5':
				width += MV_BMP[MVBMP_5_ICON].bmWidth + 2;
				break;
			case '6':
				width += MV_BMP[MVBMP_6_ICON].bmWidth + 2;
				break;
			case '7':
				width += MV_BMP[MVBMP_7_ICON].bmWidth + 2;
				break;
			case '8':
				width += MV_BMP[MVBMP_8_ICON].bmWidth + 2;
				break;
			case '9':
				width += MV_BMP[MVBMP_9_ICON].bmWidth + 2;
				break;
			default:
				break;
		}
	}
	return width;
}

void Draw_Number_Icon(HDC hdc, int x, int y, char* text)
{
	U8		i;

	for ( i = 0 ; i < PROGRAM_MAX_NUM ; i++ )
	{
		switch(text[i])
		{
			case '0':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_0_ICON].bmHeight), &MV_BMP[MVBMP_0_ICON]);
				x += MV_BMP[MVBMP_0_ICON].bmWidth + 2;
				break;
			case '1':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_1_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_1_ICON].bmHeight), &MV_BMP[MVBMP_1_ICON]);
				x += MV_BMP[MVBMP_1_ICON].bmWidth + 2;
				break;
			case '2':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_2_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_2_ICON].bmHeight), &MV_BMP[MVBMP_2_ICON]);
				x += MV_BMP[MVBMP_2_ICON].bmWidth + 2;
				break;
			case '3':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_3_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_3_ICON].bmHeight), &MV_BMP[MVBMP_3_ICON]);
				x += MV_BMP[MVBMP_3_ICON].bmWidth + 2;
				break;
			case '4':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_4_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_4_ICON].bmHeight), &MV_BMP[MVBMP_4_ICON]);
				x += MV_BMP[MVBMP_4_ICON].bmWidth + 2;
				break;
			case '5':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_5_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_5_ICON].bmHeight), &MV_BMP[MVBMP_5_ICON]);
				x += MV_BMP[MVBMP_5_ICON].bmWidth + 2;
				break;
			case '6':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_6_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_6_ICON].bmHeight), &MV_BMP[MVBMP_6_ICON]);
				x += MV_BMP[MVBMP_6_ICON].bmWidth + 2;
				break;
			case '7':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_7_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_7_ICON].bmHeight), &MV_BMP[MVBMP_7_ICON]);
				x += MV_BMP[MVBMP_7_ICON].bmWidth + 2;
				break;
			case '8':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_8_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_8_ICON].bmHeight), &MV_BMP[MVBMP_8_ICON]);
				x += MV_BMP[MVBMP_8_ICON].bmWidth + 2;
				break;
			case '9':
				FillBoxWithBitmap (hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y),ScalerWidthPixel(MV_BMP[MVBMP_9_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_9_ICON].bmHeight), &MV_BMP[MVBMP_9_ICON]);
				x += MV_BMP[MVBMP_9_ICON].bmWidth + 2;
				break;
			default:
				break;
		}
	}
}

/* by kb : 20100418 */

void DestTopSetInfo(HWND hwnd, U8 timerSet)
{
	if (timerSet)
	{
		DeskData.Banner.Draw = TRUE;
		DesktopBannerScroll(&DeskData);

		if ( b8NoSignal_Status == TRUE )
		{
			DeskData.Warning.Draw=TRUE;
		}
		else if( b8Scramble_Status == TRUE )
		{
			DeskData.Warning.Draw=FALSE;
		}

		b8Scramble_Status = FALSE;

		SendMessage(hwnd, MSG_PAINT,0,0);
		if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
			KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

		SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
	}
}

void DeskTopChangeChannel(HWND hwnd, U8 playOn, U8 timerSet)
{
	tCS_DBU_Service 		ServiceTriplet;
	tCS_DBU_Service 		Last_ServiceTriplet;
	tCS_DB_ServiceListType  Current_Service_Type;
	tCS_DB_ServiceListType  Last_Service_Type;

	if ((!playOn)
		&& (CS_MW_GetServicesLockStatus())
		&& (DeskData.ServiceData.u8Lock == eCS_DB_LOCKED )
		&& (DeskData.Current_Service != CS_APP_GetLastUnlockServiceIndex()))
	{
		CS_MW_StopService(TRUE);
	}

	// printf("================= Play On : %d =================\n", playOn);
#if 0 /* By KB Kim 2011.08.11 */
	if (playOn)
		CS_MW_PlayServiceByIdx(DeskData.Service_Index, RE_TUNNING);
	else
		CS_MW_PlayServiceByIdx(DeskData.Service_Index, NOT_TUNNING);
#else
	CS_MW_PlayServiceByIdx(DeskData.Service_Index, NOT_TUNNING);
#endif

	DestTopSetInfo(hwnd, timerSet);

	ServiceTriplet.sCS_DBU_ServiceIndex = CS_DB_GetCurrentService_OrderIndex();
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
	Last_ServiceTriplet = CS_DB_GetLastServiceTriplet();
	CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));

	Current_Service_Type = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
	Last_Service_Type = Last_ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;

	/* By KB Kim for Radio Back : 2011.06.20 */
	if ( Current_Service_Type == eCS_DB_RADIO_LIST
		|| Current_Service_Type == eCS_DB_FAV_RADIO_LIST
		|| Current_Service_Type == eCS_DB_SAT_RADIO_LIST )
	{
		if ( Last_Service_Type == eCS_DB_TV_LIST
			|| Last_Service_Type == eCS_DB_FAV_TV_LIST
			|| Last_Service_Type == eCS_DB_SAT_TV_LIST )
		{
			CS_AV_VideoBlank();
			CS_AV_Play_IFrame(RADIO_BACK);
		}
	}
	else if ( Current_Service_Type == eCS_DB_TV_LIST
			|| Current_Service_Type == eCS_DB_FAV_TV_LIST
			|| Current_Service_Type == eCS_DB_SAT_TV_LIST )
	{
		if ( Last_Service_Type == eCS_DB_RADIO_LIST
			|| Last_Service_Type == eCS_DB_FAV_RADIO_LIST
			|| Last_Service_Type == eCS_DB_SAT_RADIO_LIST )
		{
			printf("=========== DeskTopChangeChannel : Video Blank ===============\n");
			CS_AV_VideoBlank();
		}
	}

	CS_DBU_SaveCurrentService(ServiceTriplet);
}
/******************************/

/* By KB Kim : 2011.06.30 */
void DeskTopCheckChannelInList(tCS_DB_ServiceListTriplet *listTriplet)
{
	U16                       numberOfService;
	tCS_DB_ServiceListTriplet currentTriplet;
	tCS_DB_ServiceListType    currentServiceType;

	currentTriplet     = *listTriplet;
	currentServiceType = currentTriplet.sCS_DB_ServiceListType;
	numberOfService = CS_DB_GetListServiceNumber(currentTriplet);
/*
	printf("DeskTopCheckChannelInList : currentTriplet.sCS_DB_ServiceListType[%d] numberOfService[%d]\n",
		(U32)currentTriplet.sCS_DB_ServiceListType, numberOfService);
*/
	if (numberOfService == 0)
	{
		currentTriplet.sCS_DB_ServiceListTypeValue = 0;
		switch (currentServiceType)
		{
			case eCS_DB_TV_LIST :
				currentTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				break;
			case eCS_DB_RADIO_LIST :
				currentTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				break;
			case eCS_DB_FAV_TV_LIST :
			case eCS_DB_SAT_TV_LIST :
				currentTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				numberOfService = CS_DB_GetListServiceNumber(currentTriplet);
				if (numberOfService == 0)
				{
					currentTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				}
				break;
			case eCS_DB_FAV_RADIO_LIST :
			case eCS_DB_SAT_RADIO_LIST :
				currentTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				numberOfService = CS_DB_GetListServiceNumber(currentTriplet);
				if (numberOfService == 0)
				{
					currentTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				}
				break;
			default :
				break;
		}

		*listTriplet = currentTriplet;
		// printf("DeskTopCheckChannelInList : listTriplet->sCS_DB_ServiceListType[%d]\n", listTriplet->sCS_DB_ServiceListType);

		CS_DB_SetCurrentList(currentTriplet, FALSE);
	}
}

CSAPP_Applet_t	CSApp_Desktop(void)
{
	int   			BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   			msg;
	HWND  			hwndMain;
	MAINWINCREATE	CreateInfo;

	printf(" ###### START DESKTOP ============================\n");

	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);

	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "csdesktop window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Desktop_Msg_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_transparent;
	CreateInfo.dwAddData 		= 0;
	CreateInfo.hHosting 		= HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MainWindowThreadCleanup (hwndMain);

	return CSApp_Desktop_Applets;
}

static BOOL	BKPinDraw=FALSE;//avoid redraw

static int Desktop_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC				hdc = 0;
	U16             numberOfService;
	static U16 		Prev_Service = 0;
	static U8       bennerTimerSet = 0;
	static U32      currentMilliTime;
	static U32      channelChangeCounter;
	static U32      channelChangeTime;
	static U8       channelChanged;
	static U8       channelChangeRequest;
	U8              channelChangeOn = 0;

	switch(message)
	{
		case MSG_CREATE:
			{
				tCS_DB_ServiceListTriplet	ListTriplet;

				//printf(" ###### START MSG_CREATE ============================\n");

				memset(&bt_banner, 0x00, sizeof(BITMAP));

				if ( MV_DB_GetALLServiceNumber() == 0 )
				{
					CSApp_Desktop_Applets = CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
				} else {
					CSApp_Desktop_Applets = CSApp_Applet_Desktop;
				}

				CS_MW_SetNormalWindow();

				CS_DB_GetCurrentListTriplet(&ListTriplet);
#if 1 /* By KB Kim : 2011.06.30 */
				DeskTopCheckChannelInList(&ListTriplet);
				numberOfService = CS_DB_GetListServiceNumber(ListTriplet);
				if (numberOfService == 0)
				{
					/* Error on the List */
					CSApp_Desktop_Applets = CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
				}
#else
				if ( ListTriplet.sCS_DB_ServiceListType >= eCS_DB_FAV_TV_LIST )
				{
					switch(ListTriplet.sCS_DB_ServiceListType)
					{
						case eCS_DB_FAV_TV_LIST:
							if ( MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_TV_LIST_ID, ListTriplet.sCS_DB_ServiceListTypeValue) == 0 )
							{
								ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
								ListTriplet.sCS_DB_ServiceListTypeValue = 0;
							}
							break;

						case eCS_DB_FAV_RADIO_LIST:
							if ( MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_RADIO_LIST_ID, ListTriplet.sCS_DB_ServiceListTypeValue) == 0 )
							{
								ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
								ListTriplet.sCS_DB_ServiceListTypeValue = 0;
							}
							break;

						case eCS_DB_SAT_TV_LIST:
							if ( MV_Get_TVServiceCount_at_Sat(ListTriplet.sCS_DB_ServiceListTypeValue) == 0 )
							{
								ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
								ListTriplet.sCS_DB_ServiceListTypeValue = 0;
							}
							break;

						case eCS_DB_SAT_RADIO_LIST:
							if ( MV_Get_RDServiceCount_at_Sat(ListTriplet.sCS_DB_ServiceListTypeValue) == 0 )
							{
								ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
								ListTriplet.sCS_DB_ServiceListTypeValue = 0;
							}
							break;

						default:
							break;
					}

					//CS_APP_SetLastUnlockServiceIndex(0xffff);
					CS_DB_SetCurrentList(ListTriplet, FALSE);
				}
#endif

				InitDesktopData();

				b8NoSignal_Status = FALSE;
				b8Scramble_Status = FALSE;
				APP_SetMainMenuStatus(FALSE);
				DeskData.Service_Index = CS_DB_GetCurrentServiceIndex();
				//DeskData.Service_Index = CS_DB_GetCurrentList_ServiceNum();
				DeskData.Input.Active=FALSE;
				DeskData.Input.CreateTimer=FALSE;

				GetBannerData(&DeskData);

				//printf("\n #############################  Start Desk Top Service : %d, %d, %s\n", DeskData.Service_Index, DeskData.ServiceData.u16ChIndex, DeskData.ServiceData.acServiceName);

				//UpdateBannerText(&DeskData);

				DesktopBannerScroll(&DeskData);
				Prev_Service = DeskData.Current_Service;

				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
				SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

				if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
					KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);
				SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);

				if(IsTimerInstalled(hwnd, DESKTOP_VMODE_TIMER_ID))
					KillTimer(hwnd, DESKTOP_VMODE_TIMER_ID);
				SetTimer(hwnd, DESKTOP_VMODE_TIMER_ID, VMode_timer_Max);

				if(IsTimerInstalled(hwnd, INFO_TXT_TIMER_ID))
					KillTimer(hwnd, INFO_TXT_TIMER_ID);
				SetTimer(hwnd, INFO_TXT_TIMER_ID, Info_txt_timer_Max);

				u8Sleep_Time = CS_DBU_Get_Slepp();
				u8Sleep_Time_Flag = FALSE;

				MW_FE_EIT_Start();
				CS_MW_SVC_Open(DesktopNotify);
				channelChangeCounter = 0;
				channelChangeTime    = 0;
				channelChanged       = 0;
				channelChangeRequest = 0;
				currentMilliTime     = 0;
				// printf("0 begin service , time = %d : Serv_index : %d\n", CS_OS_time_now(), DeskData.Service_Index);

				SendMessage(hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
				//printf(" ###### END MSG_CREATE ============================\n");
			}
			break;

		case MSG_PAINT:
			//printf(" ###### START MSG_PAINT ============================\n");
			DesktopPaint(hwnd,&DeskData);
/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
#if 0
			if( DeskData.Banner.Draw==FALSE && DeskData.Volume.Draw==FALSE && DeskData.ParentPin.Draw==FALSE )
			{
				DeskData.Subtitle.Pid=CS_MW_GetSubtitlePid();

				if(DeskData.Subtitle.Pid>=kDB_DEMUX_INVAILD_PID)
					DeskData.Subtitle.Open=FALSE;
				else
					DeskData.Subtitle.Open=TRUE;

				if(DeskData.Subtitle.Open==TRUE)
					CS_MW_OpenSubtitle(DeskData.Subtitle.Pid);
			}
#endif
			//printf(" ###### END MSG_PAINT ============================\n");
			return 0;
			break;

		case MSG_NOW_NEXT_UPDATE:
			/* By KB Kim 2011.05.28 */
			if (DeskData.ServiceData.u16ServiceId == (U16)wparam)
			{
#ifdef DESK_EPG
			UpdateEPGData(&DeskData);
#endif
			SendMessage(hwnd,MSG_PAINT,0,0);
			}
			break;

		case MSG_CHECK_SERVICE_LOCK:
			/* by kb : 20100418 */
			//printf("=== Current : %d , Unlock : %d \n", DeskData.Current_Service, CS_APP_GetLastUnlockServiceIndex() );
			if ( CS_MW_GetServicesLockStatus() && ( DeskData.ServiceData.u8Lock == eCS_DB_LOCKED ) && ( DeskData.Current_Service != CS_APP_GetLastUnlockServiceIndex() ) )
			{
				MV_Draw_Password_Window(hwnd);
				CS_MW_StopService(TRUE);
			} else {
				if (channelChangeRequest)
				{
					/* Re requested  Channel change : Reset Counter */
					channelChangeCounter = 0;
					DestTopSetInfo(hwnd, bennerTimerSet);
				}
				else
				{
					if (channelChanged)
					{
						currentMilliTime = OsTimeNowMilli();
						//printf("\n === SetTimer(hwnd, INFO_TXT_TIMER_ID, Info_txt_timer_Max) ===\n");
						if(IsTimerInstalled(hwnd, INFO_TXT_TIMER_ID))
							KillTimer(hwnd, INFO_TXT_TIMER_ID);
						SetTimer(hwnd, INFO_TXT_TIMER_ID, Info_txt_timer_Max);

						if (OsTimeDiff(currentMilliTime, channelChangeTime) < DESKTOP_CHANNEL_CHANGE_GAP)
						{
							if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
								KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
							SetTimer(hwnd, CHANNELCHANGE_TIMER_ID, DESKTOP_CHANNEL_CHANGE_TIMER);

							DestTopSetInfo(hwnd, bennerTimerSet);
							channelChangeRequest = 1;
							channelChangeCounter = 0;
							channelChanged       = 0;

							break;
						}
					}

					//CS_DB_SetLastServiceTriplet();

					// printf("=== MSG_CHECK_SERVICE_LOCK : DeskTopChangeChannel[%d]\n", wparam);

					if ( wparam == 1 )
						DeskTopChangeChannel(hwnd, 1, bennerTimerSet);
					else
						DeskTopChangeChannel(hwnd, 0, bennerTimerSet);

					channelChangeRequest = 0;
					channelChanged       = 1;
					channelChangeTime    = OsTimeNowMilli();
				}

				bennerTimerSet = 0;
			}

			break;

		case MSG_VIDEO_FORFMAT_UPDATE:
			{

			}
			break;

		case MSG_TIMER:
			if (wparam == CHANNELCHANGE_TIMER_ID)
			{
				if (channelChangeRequest)
				{
					channelChangeCounter++;
					if (channelChangeCounter >= DESKTOP_CHANNEL_CHANGE_COUNT)
					{
						if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
						{
							KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
						}
						channelChangeCounter = 0;
						channelChangeRequest = 0;
						channelChanged       = 0;
						SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
					}
				}
				else
				{
					if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
					{
						KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
					}
					channelChangeCounter = 0;
					channelChangeRequest = 0;
					channelChanged       = 0;
				}
			}
			else if(wparam == DESKTOP_BANNER_TIMER_ID)
			{
				DeskData.Banner.Draw=FALSE;
				SendMessage(hwnd,MSG_PAINT,0,0);
				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
			}
			else if(wparam == DESKTOP_VOLUME_TIMER_ID)
			{
				DeskData.Volume.Draw=FALSE;

				SendMessage(hwnd,MSG_PAINT,0,0);
				if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
					KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);
			}
			else if(wparam == DESKTOP_VMODE_TIMER_ID)
			{
				DeskData.VMode.Draw=FALSE;

				SendMessage(hwnd,MSG_PAINT,0,0);
				if(IsTimerInstalled(hwnd, DESKTOP_VMODE_TIMER_ID))
					KillTimer(hwnd, DESKTOP_VMODE_TIMER_ID);
			}
			else if (wparam == DESKTOP_SLEEP_TIMER_ID)
			{
				hdc = BeginPaint(hwnd);
				DesktopPaintInputBack(hdc, FALSE, NULL);
				EndPaint(hwnd,hdc);

				CS_DBU_Set_Sleep(u8Sleep_Time);
				u8Sleep_Time_Flag = FALSE;

				CS_DBU_SaveUserSettingDataInHW();

				if(IsTimerInstalled(hwnd, DESKTOP_SLEEP_TIMER_ID))
					KillTimer(hwnd, DESKTOP_SLEEP_TIMER_ID);
			}
			else if(wparam == DESKTOP_INPUT_TIMER_ID)
			{
				U16	   	Svcidx=0;
				BOOL	Status;

				CS_MW_CloseSubtitle();

				{
					Status=CS_MW_GetServiceIdxByItemIdx(DeskData.Input.Input-1,&Svcidx);
					if(Status==TRUE)
					{
						CS_DB_SetLastServiceTriplet();
						CS_DB_SetCurrentService_OrderIndex(DeskData.Input.Input-1);
						DesktopBannerScroll(&DeskData);
						// check whether service changed
						if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
						{
							CS_APP_SetLastUnlockServiceIndex(0xFFFF);
						}
						Prev_Service = DeskData.Current_Service;

						SendMessage(hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
					}
				}
				DeskData.Input.Cursor=0;
				memset(DeskData.Input.Text,0,PROGRAM_MAX_NUM);
				DeskData.Input.Draw=FALSE;
				DeskData.Input.Active=FALSE;
				DeskData.Input.CreateTimer=FALSE;

				if(IsTimerInstalled(hwnd, DESKTOP_INPUT_TIMER_ID))
					KillTimer(hwnd,DESKTOP_INPUT_TIMER_ID);

				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
				SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

				SendMessage(hwnd,MSG_PAINT,0,0);
			}
			else if (wparam == INFO_TXT_TIMER_ID )
			{
				char				acTemp_Str[1024];
				char				actemp_char[256];
				char				acTemp_TP[256];
				char				acTemp_EPG[256];
				FILE*				fp;
				MV_stSatInfo 		Temp_SatInfo;
				MV_stTPInfo 		Temp_TPInfo;
				tCS_EIT_Event_t		present;
				tCS_EIT_Event_t		follow;
				TunerSignalState_t 	Siganl_State;
				extern U32			*Tuner_HandleId;

#ifdef CHECK_CH_WATCH
				U32					Current_Ch_Count;

				Current_Ch_Count = MV_Get_Current_Channel_Time();
#endif // #ifdef CHECK_CH_WATCH

				memset( acTemp_Str, 0x00, 1024 );
				memset( actemp_char, 0x00, 256 );
				memset( acTemp_TP, 0x00, 256 );
				memset( acTemp_EPG, 0x00, 256 );
				memset( &present, 0x00, sizeof(tCS_EIT_Event_t));
				memset( &follow, 0x00, sizeof(tCS_EIT_Event_t));

				MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, DeskData.Banner.BannerInfo.ServiceName.u16Index);
				MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, DeskData.Banner.BannerInfo.ServiceName.u16Index);
#ifdef CHECK_CH_WATCH
/************************/
				if ( Current_Ch_Count == 30 )
				{
					MV_Set_Current_Channel_Data(DeskData.Banner.BannerInfo.ServiceName.u16Index, DeskData.Banner.BannerInfo.ServiceName.CH_Num);
				}
				else if ( Current_Ch_Count > 30 && MV_Check_Current_Ch_Data(DeskData.Banner.BannerInfo.ServiceName.u16Index, Temp_SatInfo, Temp_TPInfo) == TRUE )
				{
					//MV_Send_Current_Channel_Data();
					Send_ch_data_Init();
				}
				else if ( Current_Ch_Count < 30 && MV_Check_Current_Ch_Data(DeskData.Banner.BannerInfo.ServiceName.u16Index, Temp_SatInfo, Temp_TPInfo) == FALSE )
				{
					MV_ReSet_Current_Channel_Time();
				}

				MV_Set_Current_Ch_Data(DeskData.Banner.BannerInfo.ServiceName.u16Index, Temp_SatInfo, Temp_TPInfo);
/************************/
#endif // #ifdef CHECK_CH_WATCH
				if ( Temp_TPInfo.u8Polar_H == P_H )
					sprintf(acTemp_TP, "%d/H/%d", Temp_TPInfo.u16SymbolRate, Temp_TPInfo.u16TPFrequency);
				else
					sprintf(acTemp_TP, "%d/V/%d", Temp_TPInfo.u16SymbolRate, Temp_TPInfo.u16TPFrequency);

				MV_OS_Get_Time_Offset(actemp_char, FALSE);
				CS_EIT_Get_PF_Event(DeskData.ServiceData.u16TransponderIndex , DeskData.ServiceData.u16ServiceId, &present, &follow);

				if ( present.start_date_mjd == 0 )
				{
					if ( follow.start_date_mjd == 0 )
						sprintf(acTemp_EPG, "%s;%s", CS_MW_LoadStringByIdx(CSAPP_STR_NONE), CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
					else
						sprintf(acTemp_EPG, "%s;%s", CS_MW_LoadStringByIdx(CSAPP_STR_NONE), follow.event_name);
				}
				else
				{
					if ( follow.start_date_mjd == 0 )
						sprintf(acTemp_EPG, "%s;%s", present.event_name, CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
					else
						sprintf(acTemp_EPG, "%s;%s", present.event_name, follow.event_name);
				}

				if ( present.start_date_mjd != 0 || follow.start_date_mjd != 0 )
				{
					DeskData.Banner.BannerInfo.ServiceAttribute.EPG.EPG = TRUE;
				} else {
					DeskData.Banner.BannerInfo.ServiceAttribute.EPG.EPG = FALSE;
				}

				TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);

/* Ch_mun ; Ch_Name ; Date+Time ; Dolby[0/1] ; HD[0/1] ; EPG[0/1] ; Scramble[0/1] ; TTX[0/1] ; SubTitle[0/1] ; Favorite[0/1] ; EPG ; Sat_Name ; TP ; Signal */
				sprintf(acTemp_Str, "%04d;%s;%s;%d;%d;%d;%d;%d;%d;%d;%s;%s;%s;%d",
									DeskData.Banner.BannerInfo.ServiceName.CH_Num, 					/* Channel Num */
									DeskData.Banner.BannerInfo.ServiceName.Name, 					/* Channel Name */
									actemp_char,													/* Date + Time */
									DeskData.Banner.BannerInfo.ServiceAttribute.AC3.AC3,			/* Dolby */
									DeskData.Banner.BannerInfo.ServiceAttribute.HD.HD,				/* HD */
									DeskData.Banner.BannerInfo.ServiceAttribute.EPG.EPG,			/* EPG */
									DeskData.Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE,	/* Scramble */
									DeskData.Banner.BannerInfo.ServiceAttribute.TTX.TTX,			/* TTX */
									DeskData.Banner.BannerInfo.ServiceAttribute.Sub.SUB,			/* SubTitle */
									DeskData.Banner.BannerInfo.ServiceAttribute.Fav.FAV,			/* Favorite */
									acTemp_EPG,														/* EPG Current and Next */
									Temp_SatInfo.acSatelliteName,									/* Satellite Name */
									acTemp_TP,														/* TP Information */
									Siganl_State.Quality);											/* Signal - Quality */

				if (!(fp = fopen("/tmp/info.txt", "w")))
        			printf("\n Info.txt File Create Error\n");
				else
				{
					fprintf(fp, "%s", acTemp_Str);
					fclose (fp);
				}
			}

			/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
			DeskData.Subtitle.Pid = CS_MW_GetSubtitlePid();
			if (DeskData.Subtitle.Pid >= kDB_DEMUX_INVAILD_PID)
			{
				if( DeskData.Banner.Draw==FALSE && DeskData.Volume.Draw==FALSE && DeskData.ParentPin.Draw==FALSE )
				{
					SubtitleNumber = MvGetCurrentSubtitle();
					if (SubtitleNumber < MvGetTotalSubtitleNumber())
					{
						CS_MW_OpenSubtitle();
					}
				}
			}

			break;

		case MSG_MOTOR_MOVING: /* For Motor Control By KB Kim 2011.05.22 */
			if (wparam)
			{
				Mv_MotorMovingDisplay();
			}
			else
			{
				Motor_Moving_Stop();
			}
			break;

		case MSG_PLAYSERVICE:
			{
				#if 0
				tCS_DBU_Service ServiceTriplet;

				CS_MW_PlayServiceByIdx(DeskData.Service_Index, NOT_TUNNING);
				if (bennerTimerSet)
				{
					bennerTimerSet = 0;
					DeskData.Banner.Draw = TRUE;
					DesktopBannerScroll(&DeskData);

					if ( b8NoSignal_Status == TRUE )
						DeskData.Warning.Draw=TRUE;
					else if( b8Scramble_Status == TRUE )
						DeskData.Warning.Draw=FALSE;
					b8Scramble_Status = FALSE;

					SendMessage(hwnd,MSG_PAINT,0,0);
					if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
						KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

					SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
				}

				ServiceTriplet.sCS_DBU_ServiceIndex = CS_DB_GetCurrentService_OrderIndex();
				CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
				// printf("==== %d , %d , %d , %d ====\n", ServiceTriplet.sCS_DBU_ServiceIndex , ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType );
				CS_DBU_SaveCurrentService(ServiceTriplet);
				// printf("DescTop MSG_PLAYSERVICE END\n");
				#else
				CS_DB_SetLastServiceTriplet();
				DeskTopChangeChannel(hwnd, 1, bennerTimerSet);
				bennerTimerSet = 0;
				#endif
			}
			break;

		case MSG_UPDATE_FE:
			if(wparam == FE_LOCK)
			{
				DeskData.Warning.Draw=FALSE;
				SendMessage(hwnd,MSG_PAINT,0,0);
			}
			else if((wparam == FE_UNLOCK)||(wparam == FE_LOST))
			{
				if( CSMPR_Player_GetStatus() != CSMPR_PLAY_RUN )
				{
					if (( MV_Get_Password_Flag() != TRUE ) &&
						( MV_Get_PopUp_Window_Status() != TRUE ) &&
						( MV_Get_Report_Window_Status() != TRUE ))
					{
						DeskData.Warning.Draw=TRUE;
						DeskData.Warning.Warning=CS_SIGNAL_UNLOCK;
						SendMessage(hwnd,MSG_PAINT,0,0);
					}
				}
			}
			break;

		case MSG_NO_VIDEO:
			{
				tCS_DB_ServiceListTriplet  	ListTriplet;

				CS_DB_GetCurrentListTriplet(&ListTriplet);

				if (ListTriplet.sCS_DB_ServiceListType != eCS_DB_RADIO_LIST && ListTriplet.sCS_DB_ServiceListType != eCS_DB_FAV_RADIO_LIST && ListTriplet.sCS_DB_ServiceListType != eCS_DB_SAT_RADIO_LIST )
				{
					// printf("============== LIST TYPE : %d : %d ================\n", ListTriplet.sCS_DB_ServiceListType, wparam);
					if( CSMPR_Player_GetStatus() != CSMPR_PLAY_RUN )
					{
						if ( wparam == TRUE )
						{
							if ( DeskData.Warning.Draw != TRUE )
							{
								if ( DeskData.ServiceData.u8Scramble == TRUE )
								{
									printf("Scambled \n");


									DeskData.Warning.Draw=TRUE;
									DeskData.Warning.Warning=CS_SERVICE_ENCRYPT;
									//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING); //Deneme 07-02-2012 aykiri ile
									/*if (eCS_AV_ERROR == CS_AV_Play_IFrame("/usr/work0/app/black.mpg"))
                                        printf("black.mpg error!!!\n");  //video problem deneme sertac 10-12-2012 --- boyle yapınca gitgel oluyor test2...*/
									CS_AV_VideoBlank();
									CSAUD_Stop(aud_handle);
								} else {
									// printf("No Service \n");
									CS_AV_VideoBlank();
									CSAUD_Play(aud_handle);
								}
								//CS_AV_Play_IFrame(BOOT_LOGO);
								SendMessage(hwnd,MSG_PAINT,0,0);
							}
						} else {
						    printf("Video Started \n");
						    /*if (eCS_AV_ERROR == CS_AV_Play_IFrame2("/usr/work0/app/black.mpg"))
                                    printf("black.mpg error!!!\n");  //video problem deneme sertac 06-12-2012 --- boyle yapınca gitgel olmuyor test3*/
							//printf("Black.mpg played...\n");


							//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
							//printf("Retune edildi!!\n");

							DeskData.Warning.Draw=FALSE;

							CS_AV_VideoUnblank(); //Deneme için aşağıya aldım v37 öncesi...
							//CSVID_Play(vid_handle); //yilmazsn38-deneme
							//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
							CSAUD_Play(aud_handle);
							SendMessage(hwnd,MSG_PAINT,0,0);
							//CS_AV_VideoUnblank();
						}
					}
				}
			}
			break;

		case MSG_VIDEO_UNDERFLOW:
			if (DeskData.Pause)
			{
				DestTopUnderFlow = 1;
			}
			break;

		case MSG_UPDATE_TTX:
			DeskData.Banner.BannerInfo.ServiceAttribute.TTX.TTX=TRUE;
			DeskData.Banner.BannerInfo.ServiceAttribute.TTX.Update=TRUE;
			SendMessage(hwnd,MSG_PAINT,0,0);
			break;

		case MSG_UPDATE_SUB:
			DeskData.Banner.BannerInfo.ServiceAttribute.Sub.SUB=TRUE;
			DeskData.Banner.BannerInfo.ServiceAttribute.Sub.Update=TRUE;

			/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
			DeskData.Subtitle.Pid = CS_MW_GetSubtitlePid();
			if (DeskData.Subtitle.Pid >= kDB_DEMUX_INVAILD_PID)
			{
				if( DeskData.Banner.Draw==FALSE && DeskData.Volume.Draw==FALSE && DeskData.ParentPin.Draw==FALSE )
				{
					SubtitleNumber = MvGetCurrentSubtitle();
					if (SubtitleNumber < MvGetTotalSubtitleNumber())
					{
						CS_MW_OpenSubtitle();
					}
				}
			}

			SendMessage(hwnd,MSG_PAINT,0,0);
			break;

		case MSG_UPDATE_HD:
			DeskData.Banner.BannerInfo.ServiceAttribute.HD.HD = TRUE;
			DeskData.Banner.BannerInfo.ServiceAttribute.HD.Update = TRUE;
			SendMessage(hwnd,MSG_PAINT,0,0);
			break;

		case MSG_TTX_DISPLAY:
			{
				TTXDisplayBitmap		*TtxDisplayBmp;

				if(wparam!=0)
				{
					hdc=BeginPaint(hwnd);
					TtxDisplayBmp=(TTXDisplayBitmap*)wparam;
					FillBoxWithBitmap(hdc,TtxDisplayBmp->x,TtxDisplayBmp->y,TtxDisplayBmp->TtxBitmap.bmWidth,TtxDisplayBmp->TtxBitmap.bmHeight,&TtxDisplayBmp->TtxBitmap);
					EndPaint(hwnd,hdc);

					TtxDisplayBmp->Update=FALSE;

					//	printf("Display end...:%d\n",CS_OS_time_now());
				}
				else
				{
					printf("Bitmap is NULL\n");
				}
			}
			break;

		case MSG_KEYUP:
			switch(wparam)
			{
				case CSAPP_KEY_UP:
				case CSAPP_KEY_DOWN:
				case CSAPP_KEY_TVRADIO:
				case CSAPP_KEY_FAVOLIST:
				case CSAPP_KEY_SWAP:
				{
					printf("2 begin switch service , time = %d : Serv_index : %d\n", CS_OS_time_now(), DeskData.Service_Index);
					channelChangeOn = 1; // by kb : 20100418
					// SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
				}
				break;
			}
			break;

		case MSG_KEYDOWN:
			{
				if (MV_Get_Password_Flag() == TRUE)
				{
					if ( wparam != CSAPP_KEY_UP && wparam != CSAPP_KEY_DOWN && wparam != CSAPP_KEY_CH_UP && wparam != CSAPP_KEY_CH_DOWN )
					{
						MV_Password_Proc(hwnd, wparam);
						switch(wparam)
						{
							case CSAPP_KEY_0:
							case CSAPP_KEY_1:
							case CSAPP_KEY_2:
							case CSAPP_KEY_3:
							case CSAPP_KEY_4:
							case CSAPP_KEY_5:
							case CSAPP_KEY_6:
							case CSAPP_KEY_7:
							case CSAPP_KEY_8:
							case CSAPP_KEY_9:
								if(MV_Password_Retrun_Value() == TRUE)
								{
									MV_Password_Set_Flag(FALSE);
									hdc = BeginPaint(hwnd);
									MV_Restore_PopUp_Window( hdc );
									EndPaint(hwnd,hdc);

									channelChangeOn = 1;
									bennerTimerSet = 1;

									channelChangeOn = 0;
									DeskData.Banner.BannerInfo.DrawBMP = TRUE;
									DestTopSetInfo(hwnd, bennerTimerSet);
									CS_DB_SetLastServiceTriplet();
									CS_APP_SetLastUnlockServiceIndex(DeskData.Current_Service);
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
								}
								break;

							case CSAPP_KEY_ENTER:
								if(MV_Password_Retrun_Value() == TRUE)
								{
									MV_Password_Set_Flag(FALSE);
									hdc = BeginPaint(hwnd);
									MV_Restore_PopUp_Window( hdc );
									EndPaint(hwnd,hdc);

									bennerTimerSet = 1;
									channelChangeOn = 0;
									DeskData.Banner.BannerInfo.DrawBMP = TRUE;
									DestTopSetInfo(hwnd, bennerTimerSet);
									CS_DB_SetLastServiceTriplet();
									CS_APP_SetLastUnlockServiceIndex(DeskData.Current_Service);
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
								}
								else
									MV_Draw_Password_Window(hwnd);
								break;

							case CSAPP_KEY_ESC:
							case CSAPP_KEY_MENU:
								DeskData.Warning.Draw=TRUE;
								DeskData.Warning.Warning=CS_SERVICE_LOCK;
								SendMessage(hwnd,MSG_PAINT,0,0);
								break;
						}

						if ( wparam == CSAPP_KEY_TV_AV )
						{
							ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						}
						break;
					} else {
						MV_Password_Set_Flag(FALSE);
						hdc = BeginPaint(hwnd);
						MV_Restore_PopUp_Window( hdc );
						EndPaint(hwnd,hdc);
					}
				}

				if ( MV_Get_PopUp_Window_Status() == TRUE )
				{
					MV_PopUp_Proc(hwnd, wparam);

					if ( wparam == CSAPP_KEY_ENTER )
					{
						U8	u8Result_Value;

						u8Result_Value = MV_Get_PopUp_Window_Result();

						switch(MV_Get_PopUp_Window_Kind())
						{
							case eMV_TITLE_SORT:
								break;

							case eMV_TITLE_SAT:
								{
									U8							u8TypeValue;
									tCS_DB_ServiceListTriplet  	ListTriplet;

									if ( u8Result_Value == 0 )
										u8TypeValue = 255;
									else
										u8TypeValue = MV_Get_Satindex_by_Seq(u8Result_Value);

									CS_DB_GetCurrentListTriplet(&ListTriplet);

									switch(ListTriplet.sCS_DB_ServiceListType)
									{
										case eCS_DB_TV_LIST:
										case eCS_DB_FAV_TV_LIST:
										case eCS_DB_SAT_TV_LIST:
											if ( u8TypeValue == 255 )
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
											else
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
											break;
										case eCS_DB_RADIO_LIST:
										case eCS_DB_FAV_RADIO_LIST:
										case eCS_DB_SAT_RADIO_LIST:
											if ( u8TypeValue == 255 )
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
											else
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
											break;
										default:
											break;
									}

									if ( u8TypeValue == 255 )
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									else
										ListTriplet.sCS_DB_ServiceListTypeValue = u8TypeValue;
									CS_APP_SetLastUnlockServiceIndex(0xffff);

									CS_DB_SetCurrentList(ListTriplet, FALSE);

									if ( CS_DBU_GetCH_List_Type() == 0 )
									{
										if ( u8TypeValue == 255 )
										{
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_RDList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_TVList;
										} else {
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_RADIOSATList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_TVSATList;
										}
									} else {
										if ( u8TypeValue == 255 )
										{
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_Ext_RDList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_Ext_TVList;
										} else {
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_Ext_RADIOSATList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_Ext_TVSATList;
										}
									}
									APP_SetMainMenuStatus(TRUE);
									SendMessage (hwnd, MSG_CLOSE, 0, 0);
								}
								break;

							case eMV_TITLE_FAV:
								{
									U8							u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
									U8							u8TypeValue;
									tCS_DB_ServiceListTriplet  	ListTriplet;

									if ( u8Result_Value == 0 )
										u8TypeValue = 255;
									else
										u8TypeValue = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - 1);

									CS_DB_GetCurrentListTriplet(&ListTriplet);

									switch(ListTriplet.sCS_DB_ServiceListType)
									{
										case eCS_DB_TV_LIST:
										case eCS_DB_FAV_TV_LIST:
										case eCS_DB_SAT_TV_LIST:
											if ( u8TypeValue == 255 )
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
											else
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
											u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
											break;
										case eCS_DB_RADIO_LIST:
										case eCS_DB_FAV_RADIO_LIST:
										case eCS_DB_SAT_RADIO_LIST:
											if ( u8TypeValue == 255 )
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
											else
												ListTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
											u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
											break;
										default:
											break;
									}

									if ( u8TypeValue == 255 )
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									else
										ListTriplet.sCS_DB_ServiceListTypeValue = u8TypeValue;

									CS_APP_SetLastUnlockServiceIndex(0xffff);

									CS_DB_SetCurrentList(ListTriplet, FALSE);

									if ( CS_DBU_GetCH_List_Type() == 0 )
									{
										if ( u8TypeValue == 255 )
										{
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_RDList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_TVList;
										} else {
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_RADIOFAVList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_TVFAVList;
										}
									} else {
										if ( u8TypeValue == 255 )
										{
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_Ext_RDList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_Ext_TVList;
										} else {
											if (DeskData.ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
												CSApp_Desktop_Applets = CSApp_Applet_Ext_RADIOFAVList;
											else
												CSApp_Desktop_Applets = CSApp_Applet_Ext_TVFAVList;
										}
									}

									APP_SetMainMenuStatus(TRUE);
									SendMessage (hwnd, MSG_CLOSE, 0, 0);
								}
								break;

							case eMV_TITLE_SAT_FAV:
								break;

							default:
								break;
						}
					}
					else if ( wparam == CSAPP_KEY_TV_AV )
					{
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					}
					break;
				}

				switch(wparam)
				{
					case CSAPP_KEY_ESC:
						if(DeskData.Input.Active==TRUE)
						{
							DeskData.Input.Cursor=0;
							memset(DeskData.Input.Text,0,PROGRAM_MAX_NUM);
							DeskData.Input.Draw=FALSE;
							DeskData.Input.Active=FALSE;
							DeskData.Input.CreateTimer=FALSE;

							if(IsTimerInstalled(hwnd, DESKTOP_INPUT_TIMER_ID))
								KillTimer(hwnd,DESKTOP_INPUT_TIMER_ID);
 						} else {
							DeskData.Banner.Draw=FALSE;
							DeskData.Volume.Draw=FALSE;
 						}
						SendMessage(hwnd,MSG_PAINT,0,0);
						break;

					case CSAPP_KEY_MENU:
						/* By KB Kim 2011.01.13 */
						if ( MV_DB_GetALLServiceNumber() == 0 )
						{
							u8Glob_Sat_Focus = 0;
							u8Glob_TP_Focus = 0;
						}
						else
						{
							u8Glob_Sat_Focus = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
							u8Glob_TP_Focus = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Glob_Sat_Focus, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
						}
						CSApp_Desktop_Applets = CSApp_Applet_MainMenu;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_SAT:
						if (channelChangeRequest)
						{
							if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
							{
								KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
							}

							channelChangeCounter = 0;
							channelChangeRequest = 0;
							channelChanged       = 0;
						}
						MV_Draw_Satlist_Window(hwnd);
						break;

					case CSAPP_KEY_FAVOLIST:
						{
							U8							Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
							tCS_DB_ServiceListTriplet  	ListTriplet;

							CS_DB_GetCurrentListTriplet(&ListTriplet);

							if (channelChangeRequest)
							{
								if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
								{
									KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
								}

								channelChangeCounter = 0;
								channelChangeRequest = 0;
								channelChanged       = 0;
							}

							switch(ListTriplet.sCS_DB_ServiceListType)
							{
								case eCS_DB_FAV_RADIO_LIST:
								case eCS_DB_SAT_RADIO_LIST:
								case eCS_DB_RADIO_LIST:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;

								case eCS_DB_TV_LIST:
								case eCS_DB_FAV_TV_LIST:
								case eCS_DB_SAT_TV_LIST:
									Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								default:
									break;
							}
							MV_Draw_Favlist_Window(hwnd, Service_Type, TRUE);
						}
						break;

					case CSAPP_KEY_EPG:
						{
							tCS_DB_ServiceListTriplet  ListTriplet;

							CS_DB_GetCurrentListTriplet(&ListTriplet);

							switch( ListTriplet.sCS_DB_ServiceListType )
							{
								case eCS_DB_TV_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_TV_EPG;
									break;
								case eCS_DB_RADIO_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_Radio_EPG;
									break;
								case eCS_DB_FAV_TV_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_FAV_TV_EPG;
									break;
								case eCS_DB_FAV_RADIO_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_FAV_Radio_EPG;
									break;
								case eCS_DB_SAT_TV_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_SAT_TV_EPG;
									break;
								case eCS_DB_SAT_RADIO_LIST:
									CSApp_Desktop_Applets = CSApp_Applet_SAT_Radio_EPG;
									break;
								default:
									CSApp_Desktop_Applets = CSApp_Applet_TV_EPG;
									break;
							}
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
						break;
					case CSAPP_KEY_ENTER:
						{
							if(DeskData.Input.Active==TRUE)
							{
								U16		Svcidx=0;
								BOOL	Status=FALSE;

								//DeskData.Input.Input--;
								CS_MW_CloseSubtitle();

								{
									Status=CS_MW_GetServiceIdxByItemIdx(DeskData.Input.Input-1,&Svcidx);
									if(Status==TRUE)
									{
										//DeskData.Current_Service=DeskData.Input.Input;
										//DeskData.Service_Index=Svcidx;
										CS_DB_SetLastServiceTriplet();
										CS_DB_SetCurrentService_OrderIndex(DeskData.Input.Input-1);
										DesktopBannerScroll(&DeskData);
										// check whether service changed
										if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
										{
											CS_APP_SetLastUnlockServiceIndex(0xFFFF);
										}
										Prev_Service = DeskData.Current_Service;

										SendMessage(hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
									}
								}

								DeskData.Input.Cursor=0;
								memset(DeskData.Input.Text,0,PROGRAM_MAX_NUM);
								DeskData.Input.Draw=FALSE;
								DeskData.Input.Active=FALSE;
								DeskData.Input.CreateTimer=FALSE;

								if(IsTimerInstalled(hwnd, DESKTOP_INPUT_TIMER_ID))
									KillTimer(hwnd,DESKTOP_INPUT_TIMER_ID);

								if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
									KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
								SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

								SendMessage(hwnd, MSG_PAINT, 0, 0);
							}
							else
							{
								tCS_DB_ServiceListTriplet  ListTriplet;

								CS_DB_GetCurrentListTriplet(&ListTriplet);

								if ( CS_DBU_GetCH_List_Type() == 0 )
								{
									switch( ListTriplet.sCS_DB_ServiceListType )
									{
										case eCS_DB_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_TVList;
											break;
										case eCS_DB_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_RDList;
											break;
										case eCS_DB_FAV_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_TVFAVList;
											break;
										case eCS_DB_FAV_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_RADIOFAVList;
											break;
										case eCS_DB_SAT_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_TVSATList;
											break;
										case eCS_DB_SAT_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_RADIOSATList;
											break;
										default:
											CSApp_Desktop_Applets = CSApp_Applet_TVList;
											break;
									}
								} else {
									switch( ListTriplet.sCS_DB_ServiceListType )
									{
										case eCS_DB_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_TVList;
											break;
										case eCS_DB_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_RDList;
											break;
										case eCS_DB_FAV_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_TVFAVList;
											break;
										case eCS_DB_FAV_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_RADIOFAVList;
											break;
										case eCS_DB_SAT_TV_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_TVSATList;
											break;
										case eCS_DB_SAT_RADIO_LIST:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_RADIOSATList;
											break;
										default:
											CSApp_Desktop_Applets = CSApp_Applet_Ext_TVList;
											break;
									}
								}

								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
							break;
						}
/*
					case CSAPP_KEY_FAVOLIST:
						{
							tCS_DB_ServiceListTriplet  ListTriplet;

							CS_DB_GetCurrentListTriplet(&ListTriplet);

							if(ListTriplet.sCS_DB_ServiceListType!=eCS_DB_FAV_TV_LIST)
							{
								ListTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
								if ( ListTriplet.sCS_DB_ServiceListType == eCS_DB_TV_LIST )
								{
									ListTriplet.sCS_DB_ServiceListTypeValue = 0;
								}
								else if ( ListTriplet.sCS_DB_ServiceListType == eCS_DB_RADIO_LIST )
								{
									ListTriplet.sCS_DB_ServiceListTypeValue = 1;
								}

								CS_DB_SetLastServiceTriplet();
								CS_APP_SetLastUnlockServiceIndex(0xffff);

								CS_DB_SetCurrentList(ListTriplet, FALSE);

								if (DeskData.ServiceData.u8TvRadio== eCS_DB_RADIO_SERVICE)
									CSApp_Desktop_Applets = CSApp_Applet_RADIOFAVList;
								else
									CSApp_Desktop_Applets = CSApp_Applet_TVFAVList;

								APP_SetMainMenuStatus(TRUE);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						}
						break;
*/
					case CSAPP_KEY_TVRADIO:
						{
							tCS_DB_ServiceListTriplet	ListTriplet;
							tCS_DBU_Service				Last_TVRadio_Triplet;

							CS_DB_GetCurrentListTriplet(&ListTriplet);

							switch(ListTriplet.sCS_DB_ServiceListType)
							{
								case eCS_DB_TV_LIST:

									CS_DB_Set_TVLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_RadioLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_RADIO_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								case eCS_DB_FAV_TV_LIST:

									CS_DB_Set_TVLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_RadioLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_FAV_RADIO_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								case eCS_DB_SAT_TV_LIST:

									CS_DB_Set_TVLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_RadioLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_SAT_RADIO_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								case eCS_DB_RADIO_LIST:

									CS_DB_Set_RadioLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_TVLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_TV_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								case eCS_DB_FAV_RADIO_LIST:

									CS_DB_Set_RadioLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_TVLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_FAV_TV_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								case eCS_DB_SAT_RADIO_LIST:

									CS_DB_Set_RadioLastServiceTriplet();

									Last_TVRadio_Triplet = CS_DB_Get_TVLastServiceTriplet();

									if ( Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType != eCS_DB_SAT_TV_LIST )
									{
										ListTriplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
										ListTriplet.sCS_DB_ServiceListTypeValue = 0;
									} else {
										ListTriplet.sCS_DB_ServiceListType = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
										ListTriplet.sCS_DB_ServiceListTypeValue = Last_TVRadio_Triplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
									}
									break;

								default:
									break;
							}
							/* By KB Kim : 2011.06.30 */
							numberOfService = CS_DB_GetListServiceNumber(ListTriplet);
							if (numberOfService == 0)
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_SERVICE);
								EndPaint(hwnd,hdc);

								usleep( 2000*1000 );

								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								break;
							}

							CS_DB_SetLastServiceTriplet();
							CS_APP_SetLastUnlockServiceIndex(0xffff);
							CS_DB_SetCurrentList(ListTriplet, FALSE);
							CS_DB_SetCurrentService_OrderIndex(Last_TVRadio_Triplet.sCS_DBU_ServiceIndex);

							hdc = BeginPaint(hwnd);
							SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
							FillBox(hdc,ScalerWidthPixel(60), ScalerHeigthPixel(200), ScalerWidthPixel(250), ScalerHeigthPixel(150));
							EndPaint(hwnd,hdc);

							PinDlg_SetStatus(FALSE);

							InitDesktopData();
							GetBannerData(&DeskData);
							DesktopBannerScroll(&DeskData);
							channelChangeOn = 1; // by kb : 20100418
							bennerTimerSet = 1; // by kb : 20100403
						}
						break;

					case CSAPP_KEY_SWAP:/*Recall*/
						CS_MW_CloseSubtitle();
						if ( CS_DBU_GetRecall_Type() == RECALL_MULTI )
						{
							CSApp_Desktop_Applets = CSAPP_Applet_Recall;
							SendMessage(hwnd,MSG_CLOSE,DeskData.Current_Service,0);
						}
						else
						{
							tCS_DBU_Service     servicetriplet;

							servicetriplet = CS_DB_GetLastServiceTriplet();

							if(CS_DB_GetListServiceNumber(servicetriplet.sCS_DBU_ServiceList)>0)
							{
								CS_DB_SetLastServiceTriplet();
								CS_DB_SetCurrentList(servicetriplet.sCS_DBU_ServiceList, FALSE);
								CS_DB_SetCurrentService_OrderIndex(servicetriplet.sCS_DBU_ServiceIndex);

								hdc = BeginPaint(hwnd);
								SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
								FillBox(hdc,ScalerWidthPixel(60), ScalerHeigthPixel(200), ScalerWidthPixel(250), ScalerHeigthPixel(150));
								EndPaint(hwnd,hdc);

								DesktopBannerScroll(&DeskData);

								// check whether service changed
								if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
								{
									CS_APP_SetLastUnlockServiceIndex(0xffff);
								}
								Prev_Service = DeskData.Current_Service;

								channelChangeOn = 1; // by kb : 20100418
								bennerTimerSet = 1; // by kb : 20100403

							}
						}
						break;

					case CSAPP_KEY_CH_UP:
					case CSAPP_KEY_UP:
						CS_MW_CloseSubtitle();

						PinDlg_SetStatus(FALSE);

						DeskData.Pause=FALSE;

						CS_DB_SetLastServiceTriplet();

						CS_DB_SetNextService_OrderIndex();
						DesktopBannerScroll(&DeskData);

						if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
						{
							CS_APP_SetLastUnlockServiceIndex(0xffff);
						}

						Prev_Service = DeskData.Current_Service;

						channelChangeOn = 1; // by kb : 20100418
						bennerTimerSet = 1; // by kb : 20100403

						break;

					case CSAPP_KEY_CH_DOWN:
					case CSAPP_KEY_DOWN:
						CS_MW_CloseSubtitle();

						PinDlg_SetStatus(FALSE);

						DeskData.Pause=FALSE;

						CS_DB_SetLastServiceTriplet();

						CS_DB_SetPreviousService_OrderIndex();
						DesktopBannerScroll(&DeskData);

						if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
						{
							CS_APP_SetLastUnlockServiceIndex(0xffff);
						}

						Prev_Service = DeskData.Current_Service;
						channelChangeOn = 1;
						bennerTimerSet = 1; // by kb : 20100403

						break;

					case CSAPP_KEY_VOL_DOWN:
					case CSAPP_KEY_LEFT:
						if( CSMPR_Player_GetStatus() == CSMPR_PLAY_RUN )
							break;

						DesktopVolumeScroll(&DeskData,FALSE);
						SendMessage(hwnd, MSG_PAINT, 0, 0);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);
						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
						break;

					case CSAPP_KEY_VOL_UP:
					case CSAPP_KEY_RIGHT:
						if( CSMPR_Player_GetStatus() == CSMPR_PLAY_RUN )
							break;

						DesktopVolumeScroll(&DeskData,TRUE);
						SendMessage(hwnd, MSG_PAINT, 0, 0);
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);
						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
						break;
					case CSAPP_KEY_MUTE:
						if(DeskData.Mute.Mute==FALSE)
						{
							DeskData.Mute.Mute=TRUE;
							CS_AV_Audio_SetMuteStatus(TRUE);
						}
						else
						{
							DeskData.Mute.Mute=FALSE;
							CS_AV_Audio_SetMuteStatus(FALSE);
						}
						DeskData.Mute.Draw=TRUE;
						DeskData.Volume.Draw = FALSE;
						CS_DBU_SaveMuteStatus();
						SendMessage(hwnd, MSG_PAINT, 0, 0);
						break;

					case CSAPP_KEY_V_MODE:
						{
							tCS_DB_ServiceListTriplet  	ListTriplet;

							CS_DB_GetCurrentListTriplet(&ListTriplet);

							/* By KB Kim : 2011.06.30 */
							// if (ListTriplet.sCS_DB_ServiceListType != eCS_DB_RADIO_LIST && ListTriplet.sCS_DB_ServiceListType != eCS_DB_FAV_RADIO_LIST && ListTriplet.sCS_DB_ServiceListType != eCS_DB_SAT_RADIO_LIST )
							{
								U16		u16Video_definition = 0;

								u16Video_definition = CS_MW_GetVideoDefinition();
								if ( u16Video_definition == eCS_DBU_DEFINITION_AUTOMATIC )
									u16Video_definition = eCS_DBU_DEFINITION_480I;
								else
									u16Video_definition++;
								CS_MW_SetVideoDefinition(u16Video_definition);
								AdjustVideoWindows();

							        /* By KB Kim : 2011.06.30 */
								if (ListTriplet.sCS_DB_ServiceListType == eCS_DB_RADIO_LIST ||
									ListTriplet.sCS_DB_ServiceListType == eCS_DB_FAV_RADIO_LIST ||
									ListTriplet.sCS_DB_ServiceListType == eCS_DB_SAT_RADIO_LIST )
								{
									CS_AV_Play_IFrame(RADIO_BACK);
								}

								if(DeskData.VMode.VMode==FALSE)
								{
									DeskData.VMode.VMode=TRUE;
								}
								else
								{
									DeskData.VMode.VMode=FALSE;
								}
								DeskData.VMode.Draw=TRUE;
								DeskData.VMode.u8VMode = u16Video_definition;

								if(IsTimerInstalled(hwnd, DESKTOP_VMODE_TIMER_ID))
									KillTimer(hwnd, DESKTOP_VMODE_TIMER_ID);
								SetTimer(hwnd, DESKTOP_VMODE_TIMER_ID, VMode_timer_Max);

								SendMessage(hwnd, MSG_PAINT, 0, 0);
							}
						}
						break;

					case CSAPP_KEY_INFO:
						if(b8Inforbar_Check_Flag == FALSE)
						{
							b8Inforbar_Check_Flag = TRUE;
							CS_MW_CloseSubtitle();
							InitDesktopData();
							GetBannerData(&DeskData);
							//DesktopBannerScroll(&DeskData);
							UpdateBannerText(&DeskData);
#ifdef DESK_EPG
							UpdateEPGData(&DeskData);
#endif
							bennerTimerSet = 1; // by kb : 20100403
							DestTopSetInfo(hwnd, bennerTimerSet);
/*
							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
							SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
*/
#if 0
							DeskData.VMode.Draw = TRUE;
							DeskData.VMode.u8VMode = CS_MW_GetVideoDefinition();

							if(IsTimerInstalled(hwnd, DESKTOP_VMODE_TIMER_ID))
								KillTimer(hwnd, DESKTOP_VMODE_TIMER_ID);
							SetTimer(hwnd, DESKTOP_VMODE_TIMER_ID, Banner_timer_Max);
#endif
							SendMessage(hwnd, MSG_PAINT, 0, 0);
						} else {
							CSApp_Desktop_Applets = CSApp_Applet_ExtInfo;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_TEXT:
						CS_MW_CloseSubtitle();
						CSApp_Desktop_Applets = CSAPP_Applet_Teletext;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_AUDIO:
						CSApp_Desktop_Applets = CSApp_Applet_Audio;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					// case CSAPP_KEY_SUBTITLE:
					case CSAPP_KEY_F2:
						if (CS_MW_GetServicesLockStatus() &&
							(DeskData.ServiceData.u8Lock == eCS_DB_LOCKED) &&
							(DeskData.Current_Service != CS_APP_GetLastUnlockServiceIndex()))
						{
						}
						else
						{
							CSApp_Desktop_Applets = CSAPP_Applet_Desk_CH_Edit;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_PAUSE:
						/* By KB Kim 2011.06.02 */
						if (DeskData.Pause)
						{
							DeskData.Pause=FALSE;
							CS_MW_SwitchVideoUnFreeze();
							DestTopUnderFlow = 0;
						}
						else
						{
							DeskData.Pause=TRUE;
							DestTopUnderFlow = 0;
							CS_MW_SwitchVideoFreeze();
						}
						SendMessage(hwnd, MSG_PAINT, 0, 0);
						break;
					case CSAPP_KEY_SLOW_B:
						// FbStartWatchdog(1);
						break;

					case CSAPP_KEY_SUBTITLE:
					case CSAPP_KEY_F1:
						CS_MW_CloseSubtitle();
						CSApp_Desktop_Applets = CSAPP_Applet_Subtitle;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_SD_HD:
					case CSAPP_KEY_F3:
						CSApp_Desktop_Applets = CSAPP_Applet_Video_Control;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_FIND:
						CSApp_Desktop_Applets = CSAPP_Applet_Finder;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_SLEEP:
						{
							char 				acTempStr[16];
							tCS_TIMER_JobInfo 	stTimerJob;
							tCS_DT_Time			time_HM;
							tCS_DT_Date 		date_ymd;
							struct tm			tm_time;
							time_t				tv_sec;
							struct timespec 	time_value;

							memset(acTempStr, 0x00, 16);

							if(IsTimerInstalled(hwnd, DESKTOP_SLEEP_TIMER_ID))
								KillTimer(hwnd, DESKTOP_SLEEP_TIMER_ID);
							SetTimer(hwnd, DESKTOP_SLEEP_TIMER_ID, Input_timer_Max);

							if ( u8Sleep_Time_Flag == FALSE )
							{
								u8Sleep_Time_Flag = TRUE;
							} else {

								if ( u8Sleep_Time > 11 )
									u8Sleep_Time = 0;
								else
									u8Sleep_Time++;

								if ( u8Sleep_Time == 0 )
								{
									stTimerJob.CS_Timer_Status = eCS_TIMER_Disable;
									CS_TIMER_CheckandSaveJobInfo(&stTimerJob, 8);
								} else {
									tv_sec = u8Sleep_Time * 10 * 60;
									clock_gettime(CLOCK_REALTIME, &time_value);
									time_value.tv_sec += tv_sec;

									memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

									time_HM.hour = tm_time.tm_hour;
									time_HM.minute = tm_time.tm_min;
									date_ymd.year = tm_time.tm_year + 1900;
									date_ymd.month = tm_time.tm_mon + 1;
									date_ymd.day = tm_time.tm_mday;

									stTimerJob.CS_Timer_Status = eCS_TIMER_Enable;
									stTimerJob.CS_Timer_Type = eCS_TIMER_Sleep;
									stTimerJob.CS_Begin_UTC = CS_DT_HMtoUTC(time_HM);
									stTimerJob.CS_Begin_MDJ = CS_DT_YMDtoMJD(date_ymd);
									stTimerJob.CS_Duration_UTC = 0;
									CS_TIMER_CheckandSaveJobInfo(&stTimerJob, 8);
								}
							}

							if ( u8Sleep_Time == 0 )
								sprintf(acTempStr, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
							else
								sprintf(acTempStr, "%d Min", u8Sleep_Time*10);

							hdc = BeginPaint(hwnd);
							DesktopPaintInputBack(hdc, TRUE, acTempStr);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_0:
					case CSAPP_KEY_1:
					case CSAPP_KEY_2:
					case CSAPP_KEY_3:
					case CSAPP_KEY_4:
					case CSAPP_KEY_5:
					case CSAPP_KEY_6:
					case CSAPP_KEY_7:
					case CSAPP_KEY_8:
					case CSAPP_KEY_9:
#if 0
						if(PinDlg_GetStatus() == TRUE)
						{
							U8	ii=0;
							DeskData.ParentPin.Draw=TRUE;
							DeskData.ParentPin.Input[DeskData.ParentPin.Cursor]= Key_to_Ascii(wparam);
							++DeskData.ParentPin.Cursor;

							for(ii=0;ii<DeskData.ParentPin.Cursor;ii++)
							{
								strcpy(DeskData.ParentPin.Text+ii*2,"* ");
							}

							for(ii=DeskData.ParentPin.Cursor;ii<PIN_MAX_NUM;ii++)
							{
								strcpy(DeskData.ParentPin.Text+ii*2,"- ");
							}

							SendMessage(hwnd, MSG_PAINT, 0, 0);

							if(DeskData.ParentPin.Cursor >= PIN_MAX_NUM)
							{
							//if((!memcmp(CS_DBU_GetPinCode(), DeskData.ParentPin.Input, 4))||(!memcmp(DeskData.ParentPin.Input, MASTER_PIN, 4)))
								if(Pin_Verify(DeskData.ParentPin.Input))
								{
									PinDlg_SetStatus(FALSE);
									DeskData.ParentPin.Cursor = 0;
									memset(DeskData.ParentPin.Input,0,5);
									DeskData.ParentPin.Draw=FALSE;
									CS_APP_SetLastUnlockServiceIndex(DeskData.Current_Service);
									SendMessage (hwnd, MSG_PLAYSERVICE, 0, 0);
								}
								else
								{
									DeskData.ParentPin.Cursor = 0;
									memset(DeskData.ParentPin.Input,0,5);
									DeskData.ParentPin.Draw=TRUE;
									strcpy(DeskData.ParentPin.Text,"- - - -");
								}
								SendMessage(hwnd, MSG_PAINT, 0, 0);
							}
						}
						else
#endif
						{
							DeskData.Input.Active=TRUE;
							DeskData.Input.Text[DeskData.Input.Cursor]= Key_to_Ascii(wparam);
							DeskData.Input.Cursor++;
							DeskData.Input.Text[DeskData.Input.Cursor]=0;
							DeskData.Input.Input=atoi(DeskData.Input.Text);
							printf("=>>%d\n",DeskData.Input.Input);

							if(DeskData.Input.CreateTimer==FALSE)
							{
								DeskData.Input.CreateTimer=TRUE;
								SetTimer(hwnd, DESKTOP_INPUT_TIMER_ID,Input_timer_Max);
							}
							else
							{
								KillTimer(hwnd, DESKTOP_INPUT_TIMER_ID);
								SetTimer(hwnd, DESKTOP_INPUT_TIMER_ID,Input_timer_Max);
							}

							DeskData.Input.Draw=TRUE;
							SendMessage(hwnd, MSG_PAINT, 0, 0);

							if(DeskData.Input.Cursor>=PROGRAM_MAX_NUM)
							{
								U16		Svcidx=0;
								BOOL	Status=FALSE;

								DeskData.Input.Active=FALSE;
								CS_MW_CloseSubtitle();

								{
									Status=CS_MW_GetServiceIdxByItemIdx(DeskData.Input.Input-1,&Svcidx);
									if(Status==TRUE)
									{
										CS_DB_SetLastServiceTriplet();
										CS_DB_SetCurrentService_OrderIndex(DeskData.Input.Input-1);
										DesktopBannerScroll(&DeskData);
										// check whether service changed
										if ((DeskData.Current_Service == CS_APP_GetLastUnlockServiceIndex()) && (Prev_Service != DeskData.Current_Service))
										{
											CS_APP_SetLastUnlockServiceIndex(0xffff);
										}
										Prev_Service = DeskData.Current_Service;

										SendMessage(hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
									}
								}

								DeskData.Input.Cursor=0;
								memset(DeskData.Input.Text,0,PROGRAM_MAX_NUM);
								DeskData.Input.Draw=FALSE;
								KillTimer(hwnd,DESKTOP_INPUT_TIMER_ID);
								DeskData.Input.Active=FALSE;
								DeskData.Input.CreateTimer=FALSE;
								SendMessage(hwnd, MSG_PAINT, 0, 0);
							}

						}
						break;

					case CSAPP_KEY_REC:
						if ( UsbCon_GetStatus() == USB_STATUS_MOUNTED )
						{
							if( CSMPR_Record_GetStatus() != CSMPR_REC_IDLE )
								break;

							hdc=MV_BeginPaint(hwnd);
							SetBrushColor(hdc, COLOR_transparent);
							FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_CLEAR_H));
							MV_EndPaint(hwnd,hdc);

							CSApp_Desktop_Applets = CSApp_Applet_Pvr_Record;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_PLAY:
						if ( UsbCon_GetStatus() == USB_STATUS_MOUNTED )
						{
							if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_RUN )
							{
								CSMPR_Player_Resume();
							} else if ( CSMPR_Player_GetStatus() < CSMPR_PLAY_RUN ) {
								if ( UsbCon_GetStatus() == USB_STATUS_MOUNTED )
								{
									set_prev_windown_status(CSApp_Applet_Desktop);
									CSApp_Desktop_Applets = CSApp_Applet_Rec_File;
									SendMessage(hwnd,MSG_CLOSE,0,0);
								}
							}
						}
						break;

					case CSAPP_KEY_LIST:
						CSApp_Desktop_Applets = CSApp_Applet_Timer;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_IDLE:
						CSApp_Desktop_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
#ifdef SMART_PHONE
					case CSAPP_KEY_SLOW:
						CSApp_Desktop_Applets = CSApp_Applet_Smart_OSD;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
#endif
#if 0
					case CSAPP_KEY_FF:
						CSMPR_TMS_Start();
						break;

					case CSAPP_KEY_REW:
						CSMPR_TMS_Play();
						break;

					case CSAPP_KEY_STOP:
						CSMPR_TMS_Stop();
						break;
#endif

/**************************v38....***********************************/
                    case CSAPP_KEY_FF:
                            hdc=MV_BeginPaint(hwnd);
							SetBrushColor(hdc, COLOR_transparent);
							FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_CLEAR_H));
							MV_EndPaint(hwnd,hdc);

							CSApp_Desktop_Applets = CSApp_Applet_Pvr_Streaming;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

/*********************************************************************/


					default:
						printf("wparam : %x\n", wparam);
						break;
				}

				if (channelChangeOn)
				{
					channelChangeOn = 0;
					SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
				}
				else
				{
					channelChanged = 0;
				}
			}
			break;

		case MSG_CLOSE:
			// CS_DB_SetLastServiceTriplet();
			if (channelChangeRequest)
			{
				if(IsTimerInstalled(hwnd, CHANNELCHANGE_TIMER_ID))
				{
					KillTimer (hwnd, CHANNELCHANGE_TIMER_ID);
				}

				channelChangeCounter = 0;
				channelChangeRequest = 0;
				channelChanged       = 0;
			}
			//printf("&&&&&&&&&&&&&&&&&&&&&&\n");
			//CS_MW_Unregister_InfoPipe(VideoNotify,CS_MW_PIPE_VIDEO_FORMAT);
			BKPinDraw=FALSE;
			CS_MW_SVC_Close();
			//CS_MW_EPG_Close();
			//CS_Eit_Stop();
			CS_MW_CloseSubtitle();

			if(IsTimerInstalled(hwnd, INFO_TXT_TIMER_ID))
				KillTimer (hwnd, INFO_TXT_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
				KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_INPUT_TIMER_ID))
				KillTimer(hwnd,DESKTOP_INPUT_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_VMODE_TIMER_ID))
				KillTimer(hwnd,DESKTOP_VMODE_TIMER_ID);

			UnloadBitmap (&bt_banner);


			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}

			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			//printf("________dd___________\n");

			break;
		default:
			break;
	}

	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

static void Desktop_Volume_Setting_With_Offset(tCSDesktopInfo* DrawData)
{
	if ( DrawData->Volume.Volume != 0 )
	{
		if ( DrawData->ServiceData.u8AudioVolume > 32 )
		{
			if ( DrawData->Volume.Volume + ( DrawData->ServiceData.u8AudioVolume - 32 ) > 50 )
				CS_AV_AudioSetVolume(kCS_DBU_MAX_VOLUME);
			else
				CS_AV_AudioSetVolume(DrawData->Volume.Volume + ( DrawData->ServiceData.u8AudioVolume - 32 ) );
		}
		else if ( DrawData->ServiceData.u8AudioVolume < 32 )
		{
			if ( DrawData->Volume.Volume - ( 32 - DrawData->ServiceData.u8AudioVolume ) < 0 )
				CS_AV_AudioSetVolume(0);
			else
				CS_AV_AudioSetVolume(DrawData->Volume.Volume - ( 32 - DrawData->ServiceData.u8AudioVolume ));
		}
		else
			CS_AV_AudioSetVolume(DrawData->Volume.Volume);
	}
}

static void DesktopVolumeScroll(tCSDesktopInfo* DrawData,BOOL right)
{
	if(DrawData->Banner.Draw==TRUE)
	{
		DrawData->Banner.Draw=FALSE;
	}

	DrawData->Volume.Volume=CS_DBU_GetVolume();
	if(right==FALSE)
	{
		if(DrawData->Volume.Volume<=0)
			DrawData->Volume.Volume = 0;
		else
			DrawData->Volume.Volume--;
	}
	else
	{
		DrawData->Volume.Volume++;
		if(DrawData->Volume.Volume>kCS_DBU_MAX_VOLUME)
			DrawData->Volume.Volume = kCS_DBU_MAX_VOLUME;
	}

	if(DrawData->Volume.Volume == 0)
	{
		DrawData->Mute.Mute = TRUE;
		CS_DBU_SetMuteStatus(eCS_DBU_ON);
		DrawData->Mute.Draw=TRUE;
		CS_AV_Audio_SetMuteStatus(TRUE);
	}
	else
	{
		if(DrawData->Mute.Mute==TRUE)
		{
			CS_DBU_SetMuteStatus(eCS_DBU_OFF);
			DrawData->Mute.Mute=FALSE;
			DrawData->Mute.Draw=TRUE;
			CS_AV_Audio_SetMuteStatus(FALSE);
		}
	}

	DrawData->Volume.Draw=TRUE;
	CS_DBU_SetVolume(DrawData->Volume.Volume);


#if 0
	if ( DrawData->ServiceData.u8AudioVolume > 32 )
	{
		if ( DrawData->Volume.Volume + DrawData->ServiceData.u8AudioVolume > 50 )
			CS_AV_AudioSetVolume(kCS_DBU_MAX_VOLUME);
		else
			CS_AV_AudioSetVolume(DrawData->Volume.Volume + DrawData->ServiceData.u8AudioVolume);
	}
	else
	{
		if ( DrawData->Volume.Volume - DrawData->ServiceData.u8AudioVolume =< 0 )
			CS_AV_AudioSetVolume(0);
		else
			CS_AV_AudioSetVolume(DrawData->Volume.Volume - DrawData->ServiceData.u8AudioVolume);
	}
#else
	Desktop_Volume_Setting_With_Offset(DrawData);
#endif
	CS_DBU_SaveVolume();
	CS_DBU_SaveMuteStatus();

}

static void DesktopBannerScroll(tCSDesktopInfo* DrawData)
{
	/* By KB Kim 2011.01.20 */
	U8                                  tvRadio;

	tCS_DB_ServiceManageData			item_data;
	tCS_DB_Error						DBError;
#ifdef DESK_EPG
	tCS_EIT_Error						error;
	tCS_EIT_Event_t					    presentInfo;  // by kb : 20100406
	tCS_EIT_Event_t					    followInfo;  // by kb : 20100406
#endif

	if(DrawData->Banner.Draw==TRUE)
	{
		//DeskData.Banner.BannerInfo.DrawBMP=TRUE;
		DrawData->Banner.BannerInfo.ServiceName.Update=TRUE;
		DrawData->Banner.BannerInfo.Follow.Update=TRUE;
		DrawData->Banner.BannerInfo.Present.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.AC3.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.EPG.Update=TRUE;
	}
	else
	{
		DrawData->Banner.Draw=TRUE;
		DrawData->Banner.BannerInfo.DrawBMP=TRUE;
		DrawData->Banner.BannerInfo.ServiceName.Update=TRUE;
		DrawData->Banner.BannerInfo.Follow.Update=TRUE;
		DrawData->Banner.BannerInfo.Present.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.AC3.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.Update=TRUE;
		DrawData->Banner.BannerInfo.ServiceAttribute.EPG.Update=TRUE;
	}

	DrawData->Banner.BannerInfo.Present.StarMjd=0;
	DrawData->Banner.BannerInfo.Present.StartTime=0;
	DrawData->Banner.BannerInfo.Present.Duration=0;
	DrawData->Banner.BannerInfo.Present.Name[0]=0;
	DrawData->Banner.BannerInfo.Follow.StarMjd=0;
	DrawData->Banner.BannerInfo.Follow.StartTime=0;
	DrawData->Banner.BannerInfo.Follow.Duration=0;
	DrawData->Banner.BannerInfo.Follow.Name[0]=0;

	DrawData->ParentPin.Lock=FALSE;

	DrawData->Current_Service=CS_DB_GetCurrentService_OrderIndex();
	DBError=CS_DB_GetCurrentList_ServiceData(&item_data, DrawData->Current_Service);
	if(DBError==eCS_DB_OK)
	{
		DBError=MV_DB_GetServiceDataByIndex(&DrawData->ServiceData, item_data.Service_Index);
	}

	if(DBError!=eCS_DB_OK)
	{
		DrawData->Service_Index=0xFFFF;
		DrawData->TPIndex=0xFFFF;
	#ifdef FOR_USA
		DrawData->ServiceData.sCS_DB_LCN=0;
	#endif
		//strcpy(DrawData->ServiceData.sCS_DB_ServiceName,"No Channel");
		memset(DrawData->ServiceData.acServiceName, 0, MAX_SERVICE_NAME_LENGTH);

		DrawData->Banner.BannerInfo.ServiceAttribute.HD.HD=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.EPG.EPG=FALSE;

		DrawData->Warning.Warning=CS_NO_SERVICE;
		DrawData->Warning.Draw=TRUE;

		UpdateBannerText(DrawData);
		return;
	}

	DrawData->Service_Index=item_data.Service_Index;
	DrawData->TPIndex=DrawData->ServiceData.u16TransponderIndex;

	if (CS_APP_GetFirstInDesktop())
	{
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.HD=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB=FALSE;
	}

	CS_APP_SetFirstInDesktop(TRUE);

	//DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=DrawData->ServiceData.u8Audio_Type;
	DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=FALSE;

	/* By KB Kim 2011.01.20 */
	if (DrawData->ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
	{
		tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
	}
	else
	{
		tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	}

	if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, DrawData->ServiceData.u16ChIndex) < MV_MAX_FAV_KIND )
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=FALSE;

	if ( DrawData->ServiceData.u8Lock == 1 )
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK=TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK=FALSE;

	if ( DrawData->ServiceData.u8Scramble == 1 )
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE=TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE=FALSE;

#ifdef DESK_EPG
	DrawData->Banner.BannerInfo.Present.Name[0] = '\0';
	DrawData->Banner.BannerInfo.Present.Name[1] = '\0';
	DrawData->Banner.BannerInfo.Follow.Name[0] = '\0';
	DrawData->Banner.BannerInfo.Follow.Name[1] = '\0';

	/* 	by kb : 20100406  */
	memset(&presentInfo, 0, sizeof(tCS_EIT_Event_t));
	memset(&followInfo, 0, sizeof(tCS_EIT_Event_t));

	error = CS_EIT_Get_PF_Event(DrawData->ServiceData.u16TransponderIndex, DrawData->ServiceData.u16ServiceId, &presentInfo, &followInfo);

	if(error == eCS_EIT_NO_ERROR)
	{
		DrawData->Banner.BannerInfo.Present.StarMjd   = presentInfo.start_date_mjd;
		DrawData->Banner.BannerInfo.Present.StartTime = presentInfo.start_time_utc;
		DrawData->Banner.BannerInfo.Present.Duration  = presentInfo.duration_utc;
		memcpy(DrawData->Banner.BannerInfo.Present.Name, presentInfo.event_name, kCS_EIT_MAX_EVENT_NAME_LENGTH);
		DrawData->Banner.BannerInfo.Present.Name[kCS_EIT_MAX_EVENT_NAME_LENGTH-1] = '\0';
		DrawData->Banner.BannerInfo.Present.Name[kCS_EIT_MAX_EVENT_NAME_LENGTH-2] = '\0';

		DrawData->Banner.BannerInfo.Follow.StarMjd   = followInfo.start_date_mjd;
		DrawData->Banner.BannerInfo.Follow.StartTime = followInfo.start_time_utc;
		DrawData->Banner.BannerInfo.Follow.Duration  = followInfo.duration_utc;
		memcpy(DrawData->Banner.BannerInfo.Follow.Name, followInfo.event_name,kCS_EIT_MAX_EVENT_NAME_LENGTH);
		DrawData->Banner.BannerInfo.Follow.Name[kCS_EIT_MAX_EVENT_NAME_LENGTH-1] = '\0';
		DrawData->Banner.BannerInfo.Follow.Name[kCS_EIT_MAX_EVENT_NAME_LENGTH-2] = '\0';
	}
	/*****************************/

#endif // #ifdef DESK_EPG
	DrawData->Pause = FALSE;

	UpdateBannerText(DrawData);
}

static void DesktopPaintBanner(HWND hwnd, tCSDesktopInfo* DrawData)
{
	HDC						hdc;
	RECT					rc;
	tCS_EIT_Event_t			present;
	tCS_EIT_Event_t			follow;
	CSVID_SequenceHeader	B_hdr;
	tMWStream				SUBTTXstream;

	if( Banner_timer_Max == 0 )
	{
		DrawData->Banner.Draw = FALSE;
		DrawData->Banner.OnScreen = FALSE;
		b8Inforbar_Check_Flag = FALSE;
		return;
	}

	if(DrawData->Banner.Draw==TRUE)
	{
		DrawData->Volume.Draw=FALSE;
	}

	if ( CS_DBU_GetANI_Type() )
	{
		int						x;
		U16						Start_y = 0;

		if( DrawData->Banner.Draw!=TRUE && DrawData->Banner.OnScreen==TRUE )
		{
			for ( x = 0 ; x < 5 ; x++ )
			{
				Start_y = BANNER_CIRCLE_Y + ((BANNER_CLEAR_H/5) * x);
				hdc = BeginPaint(hwnd);
				SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
				FillBox(hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(BANNER_CIRCLE_Y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT - BANNER_CIRCLE_Y));
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(Start_y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(BANNER_CLEAR_H), &bt_banner);
				EndPaint(hwnd,hdc);
				usleep(10);
		    }
		}
	}

	hdc=MV_BeginPaint(hwnd);

	if( DrawData->Banner.Draw!=TRUE && DrawData->Banner.OnScreen==TRUE )
	{
		b8Inforbar_Check_Flag = FALSE;
		DrawData->Banner.OnScreen=FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		//FillBox(hdc,ScalerWidthPixel(BANNER_STAR_X),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(BANNER_STAR_W), ScalerHeigthPixel(BANNER_CLEAR_H));
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_CLEAR_H));
		EndPaint(hwnd,hdc);
		return;
	}

	if(DrawData->Banner.Draw!=TRUE)
	{
		EndPaint(hwnd,hdc);
		return;
	}

	if(DrawData->Banner.BannerInfo.DrawBMP==TRUE)
	{
#if 0
		SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_STAR_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_STAR_H));
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_STAR_Y + 72),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_CLEAR_H - 72));
#else
		FillBoxWithBitmap(hdc, ScalerWidthPixel(0), ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(MV_BMP[MVBMP_INFOBAR].bmHeight), &MV_BMP[MVBMP_INFOBAR]);
#endif
		DrawData->Banner.BannerInfo.DrawBMP = FALSE;
	}

	if(DrawData->Banner.BannerInfo.ServiceName.Update==TRUE)
	{
		/* By KB Kim 2011.01.16 */
		char    		Text[35];
		MV_stSatInfo 	Temp_SatInfo;
		MV_stTPInfo 	Temp_TPInfo;

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);

		// printf("\n==== %x : %x ====\n", DrawData->Banner.BannerInfo.ServiceName.u16Index, DrawData->Banner.BannerInfo.ServiceName.CH_Num);

		MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, DrawData->Banner.BannerInfo.ServiceName.u16Index);

		MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, DrawData->Banner.BannerInfo.ServiceName.u16Index);
		/* By KB Kim 2011.01.16 */
		memset(Text, 0x00, 35);
		if ( Temp_TPInfo.u8Polar_H == P_H)
			sprintf(Text, "%s   %d/%s/%d", Temp_SatInfo.acSatelliteName, Temp_TPInfo.u16TPFrequency, "H", Temp_TPInfo.u16SymbolRate);
		else
			sprintf(Text, "%s   %d/%s/%d", Temp_SatInfo.acSatelliteName, Temp_TPInfo.u16TPFrequency, "V", Temp_TPInfo.u16SymbolRate);

		rc.left = ScalerWidthPixel(BANNER_NAME_X);
		rc.top = ScalerHeigthPixel(BANNER_ITEM_Y3);
		rc.right = ScalerWidthPixel((rc.left + BANNER_NAME_DX * 2));
		rc.bottom = ScalerHeigthPixel(rc.top + BANNER_ITEM_H);
		SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Info_bot_Color.MV_R, CFG_Info_bot_Color.MV_G, CFG_Info_bot_Color.MV_B, 0xFF));
//		FillBoxWithBitmap(hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_BLACK_GROUND]);
#if 0
		FillBox(hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H));
#else
		if ( MV_BMP[MVBMP_INFO_SAT_INFO_CAP].bmHeight == 0 )
			MV_GetBitmapFromDC (hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_SAT_INFO_CAP]);
		else
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_SAT_INFO_CAP]);
#endif

		CS_MW_DrawText(hdc, Text, -1, &rc, DT_LEFT);

		/* By KB Kim 2011.01.16 */
		memset(Text, 0x00, 35);
		sprintf(Text, "%04d", DrawData->Banner.BannerInfo.ServiceName.CH_Num);

		rc.left = ScalerWidthPixel(BANNER_CIRCLE_X + ( MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth - Get_Width_Number(Text) )/2);
		rc.top = ScalerHeigthPixel(BANNER_CIRCLE_Y + ((MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2) - (MV_BMP[MVBMP_0_ICON].bmHeight/2)));
		rc.right = ScalerWidthPixel(BANNER_CIRCLE_X + MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth);
		rc.bottom = ScalerHeigthPixel(rc.top + BANNER_ITEM_H);

		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_CIRCLE_X),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight), &MV_BMP[MVBMP_INFO_BANNER_CIRCLE]);

		Draw_Number_Icon(hdc, rc.left, rc.top, Text);
		//CS_MW_DrawText(hdc, Text, -1, &rc, DT_CENTER);

		if ( MV_BMP[MVBMP_INFO_CHNAME_CAP].bmHeight == 0 )
			MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_NAME_X), ScalerHeigthPixel(BANNER_ITEM_Y1), ScalerWidthPixel(BANNER_NAME_DX), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_CHNAME_CAP]);
		else
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_NAME_X), ScalerHeigthPixel(BANNER_ITEM_Y1), ScalerWidthPixel(BANNER_NAME_DX), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_CHNAME_CAP]);

		rc.left = ScalerWidthPixel(BANNER_NAME_X);
		rc.top = ScalerHeigthPixel(BANNER_ITEM_Y1);
		rc.right = ScalerWidthPixel(BANNER_NAME_X + BANNER_NAME_DX);
		rc.bottom = ScalerHeigthPixel(BANNER_ITEM_Y1 + BANNER_ITEM_H);

		/* By KB Kim 2011.01.16 */
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,MVAPP_YELLOW_COLOR);

		memset(Text, 0x00, 35);
		sprintf(Text, "%s", DrawData->Banner.BannerInfo.ServiceName.Name);
		MV_MW_DrawText_Title(hdc, Text, -1, &rc, DT_LEFT);
		DrawData->Banner.BannerInfo.ServiceName.Update=FALSE;

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
	}

	if(DrawData->Banner.BannerInfo.ServiceAttribute.AC3.Update==TRUE)
	{
		if(DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3==TRUE)
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_DOLBY_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON]);
		}
		DrawData->Banner.BannerInfo.ServiceAttribute.AC3.Update=FALSE;
	}

	B_hdr = MV_Get_Seq_Header();
	//printf("\n=== Height : %d , Width : %d =\n", B_hdr.h, B_hdr.w);
	if ( B_hdr.h >= 720 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*1),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_HD_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*1),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_HD_UNFO_ICON]);

	if(DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.Update==TRUE)
	{
		if(DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE==TRUE)
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*2),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SCRAM_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*2),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SCRAM_UNFO_ICON]);
		}
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.Update=FALSE;
	}

	if(DrawData->Banner.BannerInfo.ServiceAttribute.TTX.Update==TRUE)
	{
		if(DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX==TRUE)
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*3),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_TTX_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*3),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_TTX_UNFO_ICON]);
		}
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.Update=FALSE;
	}

	CS_MW_GetSubtitleStream(&SUBTTXstream);

	if(SUBTTXstream.Number==0)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_FO_ICON]);
#if 0
	if(DrawData->Banner.BannerInfo.ServiceAttribute.Sub.Update==TRUE)
	{
		if(DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB==TRUE)
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON]);
		}
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.Update=FALSE;
	}
#endif
	if(DrawData->Banner.BannerInfo.ServiceAttribute.Fav.Update==TRUE)
	{
		if(DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV==TRUE)
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*5),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_FAV_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*5),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_FAV_UNFO_ICON]);
		}
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.Update=FALSE;
	}
#ifdef DESK_EPG
	{
		U16						Utc;
		U8						StarHour,StarMinute,EndHour,EndMinute;
		char					Text[45];
		RECT					Tmp_Rect;

		CS_EIT_Get_PF_Event(DrawData->ServiceData.u16TransponderIndex , DrawData->ServiceData.u16ServiceId, &present, &follow);

		if(DrawData->Banner.BannerInfo.Present.Update==TRUE)
		{
			memset(Text, 0x00, 45);
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			if ( MV_BMP[MVBMP_INFO_EPG1_CAP].bmHeight == 0 )
				MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG1_CAP]);
			else
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG1_CAP]);

			if(present.start_date_mjd != 0)
			{
				Utc = present.start_time_utc;
				StarHour = Utc>>8;
				StarMinute = (Utc&0x00FF);
				Utc = present.end_time_utc;
				EndHour = Utc>>8;
				EndMinute = (Utc&0x00FF);

				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				snprintf(Text, 20, "%.2x:%.2x-%.2x:%.2x", StarHour, StarMinute, EndHour, EndMinute);
				//printf("PRESENT : %s\n", present.event_name);
				Tmp_Rect.top = BANNER_ITEM_Y1 + 4;
				Tmp_Rect.bottom = Tmp_Rect.top + BANNER_ITEM_H;
				Tmp_Rect.left = BANNER_EPG_X;
				Tmp_Rect.right = Tmp_Rect.left + (( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 );
				CS_MW_DrawText(hdc, Text, -1, &Tmp_Rect, DT_LEFT);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);

				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				strncpy(Text, present.event_name, 20);
				Tmp_Rect.left = BANNER_EPG_X + 180;
				//snprintf(Text, 40, "%s", present.event_name);
				CS_MW_DrawText(hdc, Text, -1, &Tmp_Rect, DT_LEFT);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+180), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);
			}
			else
			{
				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				sprintf(Text, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_EPG_CURRENT), CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
				// printf("=== EPG Current == > %s ===\n", Text);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+160), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), " ");
			}
			DrawData->Banner.BannerInfo.Present.Update=FALSE;
		}

		if(DrawData->Banner.BannerInfo.Follow.Update==TRUE)
		{
			memset(Text, 0x00, 45);
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			if ( MV_BMP[MVBMP_INFO_EPG2_CAP].bmHeight == 0 )
				MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y2),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG2_CAP]);
			else
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y2),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG2_CAP]);

			if(DrawData->Banner.BannerInfo.Follow.StarMjd != 0)
			{
				Utc = follow.start_time_utc;
				StarHour = Utc>>8;
				StarMinute = (Utc&0x00FF);
				Utc = follow.end_time_utc;
				EndHour = Utc>>8;
				EndMinute = (Utc&0x00FF);

				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				snprintf(Text, 20, "%.2x:%.2x-%.2x:%.2x", StarHour, StarMinute, EndHour, EndMinute);
				//printf("FOLLOW : %s\n", follow.event_name);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4), Text);
				Tmp_Rect.top = BANNER_ITEM_Y2 + 4;
				Tmp_Rect.bottom = Tmp_Rect.top + BANNER_ITEM_H;
				Tmp_Rect.left = BANNER_EPG_X;
				Tmp_Rect.right = Tmp_Rect.left + (( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 );
				CS_MW_DrawText(hdc, Text, -1, &Tmp_Rect, DT_LEFT);

				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				strncpy(Text, follow.event_name, 20);
				Tmp_Rect.left = BANNER_EPG_X + 180;
				//snprintf(Text, 40, "%s", present.event_name);
				CS_MW_DrawText(hdc, Text, -1, &Tmp_Rect, DT_LEFT);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+180), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4), Text);
			}
			else
			{
				/* By KB Kim 2011.01.16 */
				memset(Text, 0x00, 45);
				sprintf(Text, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_EPG_NEXT), CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
				CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4), Text);
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+160), ScalerHeigthPixel(BANNER_ITEM_Y2), " ");
			}
			DrawData->Banner.BannerInfo.Follow.Update=FALSE;
		}

		if( (present.start_date_mjd != 0) || (follow.start_date_mjd != 0) )
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*6),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_EPG_FO_ICON]);
		}
		else
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*6),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_EPG_UNFO_ICON]);
		}
	}
#endif // #ifdef DESK_EPG
	if ( DrawData->Banner.Draw == TRUE && DrawData->Banner.OnScreen == FALSE )
	{
#if 0
		U16 			local_utc;
		U16 			local_mjd;
		tCS_DT_Time     local_time;
		tCS_DT_Date		local_date;
		tCS_DT_Date  	Current_Date;
		tCS_DT_Time		Current_Time;
#endif
		char 			acDate[100];
		char 			acTime[100];

#if 0
		if ( CS_MW_GetTimeMode() == CS_APP_TIME_MODE_NET )
		{
			local_utc = CS_DT_GetLocalUTC();
			local_mjd = CS_DT_GetLocalMJD();
			local_time = CS_DT_UTCtoHM(local_utc);
			local_date = CS_DT_MJDtoYMD(local_mjd);

			sprintf(acDate, "%02d/%02d", local_date.day, local_date.month);
			sprintf(acTime, "%02d:%02d", local_time.hour, local_time.minute);
		} else if ( CS_MW_GetTimeMode() == CS_APP_TIME_MODE_LOCAL ) {
			Current_Date = CS_DT_MJDtoYMD(CS_DT_GetLocalMJD());
			Current_Time = CS_DT_UTCtoHM(CS_DT_GetLocalUTC());

			sprintf(acDate, "%02d/%02d", Current_Date.day, Current_Date.month);
			sprintf(acTime, "%02d:%02d", Current_Time.hour, Current_Time.minute);
		} else {
#endif
			MV_OS_Get_Time_Offset_Splite(acDate, acTime, TRUE);
#if 0
		}
#endif
		if ( MV_BMP[MVBMP_INFO_TIME_CAP].bmHeight == 0 )
			MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2), ScalerWidthPixel(BANNER_NAME_DX+ 120), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_TIME_CAP]);
		else
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2), ScalerWidthPixel(BANNER_NAME_DX+ 120), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_TIME_CAP]);

		FillBoxWithBitmap(hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4),ScalerWidthPixel(MV_BMP[MVBMP_PVR_DATE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_DATE]);
		FillBoxWithBitmap(hdc, ScalerWidthPixel(BANNER_TIME_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4),ScalerWidthPixel(MV_BMP[MVBMP_PVR_TIME].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_TIME].bmHeight), &MV_BMP[MVBMP_PVR_TIME]);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);

		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_ITEM_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), acDate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_TIME_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), acTime);
	}

	{
		time_t					start_time;
		time_t					end_time;
		time_t					duration_time;
		U16						result1, result2;
		tCS_DT_Time				time_HM;
		tCS_DT_Date				date_YMD;
		struct tm				Temp_Time;
		U8						file_Percent;
		struct timespec 		time_value;

		date_YMD = CS_DT_MJDtoYMD(present.start_date_mjd);
		time_HM = CS_DT_UTCtoHM(present.start_time_utc);

		Temp_Time.tm_sec = 0;
		Temp_Time.tm_min = time_HM.minute;
		Temp_Time.tm_hour = time_HM.hour;
		Temp_Time.tm_mday = date_YMD.day;
		Temp_Time.tm_mon = date_YMD.month - 1;
		Temp_Time.tm_year = date_YMD.year - 1900;

		start_time = (time_t)mktime(&Temp_Time);

		date_YMD = CS_DT_MJDtoYMD(present.end_date_mjd);
		time_HM = CS_DT_UTCtoHM(present.end_time_utc);

		Temp_Time.tm_sec = 0;
		Temp_Time.tm_min = time_HM.minute;
		Temp_Time.tm_hour = time_HM.hour;
		Temp_Time.tm_mday = date_YMD.day;
		Temp_Time.tm_mon = date_YMD.month - 1;
		Temp_Time.tm_year = date_YMD.year - 1900;

		end_time = (time_t)mktime(&Temp_Time);

#if 0
		date_YMD = CS_DT_MJDtoYMD(CS_DT_GetLocalMJD());
		time_HM = CS_DT_UTCtoHM(CS_DT_GetLocalUTC());

		Temp_Time.tm_sec = 0;
		Temp_Time.tm_min = time_HM.minute;
		Temp_Time.tm_hour = time_HM.hour;
		Temp_Time.tm_mday = date_YMD.day;
		Temp_Time.tm_mon = date_YMD.month - 1;
		Temp_Time.tm_year = date_YMD.year - 1900;
#endif
		clock_gettime(CLOCK_REALTIME, &time_value);
		memcpy(&Temp_Time, localtime(&time_value.tv_sec), sizeof(Temp_Time));

		duration_time = (time_t)mktime(&Temp_Time);

		result1 = end_time - start_time;
		result2 = duration_time - start_time;
#if 0
		if ( result1 > 0 )
			printf("\n ==== ( %d * 100 ) / %d ==== %d ===== \n", result2, result1, (result2 * 100) / result1);
		else
			printf("\n ==== ( %d * 100 ) / %d \n", result2, result1);
#endif
		if ( result1 > 0 )
		{
			file_Percent = (result2 * 100) / result1;
			if ( file_Percent <= 100 )
			{
				rc.left = ScalerWidthPixel(BANNER_EPG_X - 26);
				rc.right = rc.left + ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X + 40 );
				rc.top = ScalerHeigthPixel(BANNER_ITEM_Y3);
				rc.bottom = rc.top + ScalerHeigthPixel(BANNER_ITEM_H);
				MV_Draw_LevelBar(hdc, &rc, file_Percent, EN_ITEM_PROGRESS_BAR_LEVEL);
			}
		}
	}

	rc.left = ScalerWidthPixel(BANNER_EPG_X - 206);
	rc.right = rc.left + 120;  //// No meaning .. not use
	rc.top = ScalerHeigthPixel(BANNER_ITEM_Y2 + 6);
	rc.bottom = rc.top + 20;  //// No meaning .. not use
	MV_Draw_Menu_Signal(hdc, rc);


	if ( CS_DBU_GetANI_Type() )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(BANNER_CIRCLE_Y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(BANNER_CLEAR_H), &bt_banner);
		if ( DrawData->Banner.Draw == TRUE && DrawData->Banner.OnScreen == FALSE )
		{
			SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
			FillBox(hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(BANNER_CIRCLE_Y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT - BANNER_CIRCLE_Y));
		}
		EndPaint(hwnd,hdc);

		if ( DrawData->Banner.Draw == TRUE && DrawData->Banner.OnScreen == FALSE )
		{
			int			x;
			U16			Start_y = 0;

			for ( x = 0 ; x < 5 ; x++ )
			{
				Start_y = BANNER_CIRCLE_Y + ((BANNER_CLEAR_H/5) * (5 - ( x + 1 )));
				hdc = BeginPaint(hwnd);
				SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
				FillBox(hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(BANNER_CIRCLE_Y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT - BANNER_CIRCLE_Y));
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_CAPTURE_X), ScalerHeigthPixel(Start_y), ScalerWidthPixel(BANNER_CAPTURE_DX), ScalerHeigthPixel(BANNER_CLEAR_H), &bt_banner);
				EndPaint(hwnd,hdc);
				usleep(10);
		    }
		}
	}else {
		EndPaint(hwnd,hdc);
	}

	DrawData->Banner.OnScreen=TRUE;
}

#define 	DESKTOP_INPUTT_W			130
#define		DESKTOP_INPUTT_H			40
#define		DESKTOP_INPUTT_X			150
#define		DESKTOP_INPUTT_Y			100

static void DesktopPaintInputBack(HDC hdc, BOOL b8Kind, char *Temp_Str)
{
	RECT		rrect;

	if ( b8Kind == TRUE )
	{
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10), ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H));
		FillBoxWithBitmap (hdc, ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10),ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H), &MV_BMP[MVBMP_INPUT_BACK]);

		rrect.top = DESKTOP_INPUTT_Y;
		rrect.bottom = rrect.top + 30;
		rrect.left = DESKTOP_INPUTT_X;
		rrect.right = rrect.left + DESKTOP_INPUTT_W;
		CS_MW_DrawText(hdc, Temp_Str, -1, &rrect, DT_CENTER | DT_VCENTER);
	} else {
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10), ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H));
	}
}

static void DesktopPaintInput(HDC hdc,tCSDesktopInfo* DrawData)
{
	static BOOL ClearFlag=FALSE;
	char		Temp_Str[10];

	memset(Temp_Str, 0x00, 10);

	if(DrawData->Input.Draw==TRUE)
	{
		ClearFlag=FALSE;
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10), ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H));
		FillBoxWithBitmap (hdc, ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10),ScalerWidthPixel(MV_BMP[MVBMP_INPUT_BACK].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INPUT_BACK].bmHeight), &MV_BMP[MVBMP_INPUT_BACK]);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		sprintf(Temp_Str, "%04d", atoi(DrawData->Input.Text));
		Draw_Number_Icon(hdc, ScalerWidthPixel(DESKTOP_INPUTT_X+14), ScalerHeigthPixel(DESKTOP_INPUTT_Y), Temp_Str);
		//CS_MW_TextOut(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X+16),ScalerHeigthPixel(DESKTOP_INPUTT_Y+2),DrawData->Input.Text);
	}
	else
	{
		if(ClearFlag!=TRUE)
		{
			ClearFlag=TRUE;
			SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
			FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_INPUT_BACK].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INPUT_BACK].bmHeight));
		}
	}
}


//#define			DESKTOP_WARNING_W			250
//#define			DESKTOP_WARNING_H			100
#define			DESKTOP_WARNING_W			MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmWidth
#define			DESKTOP_WARNING_H			MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmHeight
#define			DESKTOP_WARNING_X			(1280 - DESKTOP_WARNING_W)/2
#define			DESKTOP_WARNING_Y			(720-DESKTOP_WARNING_H)/2-50

#define			DESKTOP_PARENTPIN_W			260
#define			DESKTOP_PARENTPIN_H			150
#define			DESKTOP_PARENTPIN_X			(720-DESKTOP_PARENTPIN_W)/2
#define			DESKTOP_PARENTPIN_Y			(576-DESKTOP_PARENTPIN_H)/2-50

static void DesktopPaintWarning(HDC hdc,tCSDesktopInfo* DrawData)
{
	static BOOL ClearFlag=FALSE;
	RECT		Temp_Rect;

	//printf("DesktopPaintWarning\n");
	if(DrawData->Warning.Draw==TRUE)
	{
		ClearFlag=FALSE;
		//FillBoxWithBitmap(hdc,ScalerWidthPixel(DESKTOP_WARNING_X),ScalerHeigthPixel(DESKTOP_WARNING_Y),ScalerWidthPixel(DESKTOP_WARNING_W), ScalerHeigthPixel(DESKTOP_WARNING_H), &MV_BMP[MVBMP_DESCTOP_MSG_PANEL]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(DESKTOP_WARNING_X),ScalerHeigthPixel(DESKTOP_WARNING_Y),ScalerHeigthPixel(DESKTOP_WARNING_W), ScalerHeigthPixel(DESKTOP_WARNING_H), &MV_BMP[MVBMP_DESCTOP_MSG_PANEL]);

		//if(DrawData->Warning.Warning == CS_SERVICE_LOCK)
		//	FillBoxWithBitmap(hdc,ScalerWidthPixel(DESKTOP_WARNING_X),ScalerHeigthPixel(DESKTOP_WARNING_Y),ScalerHeigthPixel(DESKTOP_WARNING_W), ScalerHeigthPixel(DESKTOP_WARNING_H), &MV_BMP[MVBMP_DESCTOP_MSG_PANEL]);

		if(DrawData->Warning.Warning == CS_SIGNAL_UNLOCK)
		{
			b8NoSignal_Status = TRUE;
			snprintf(DrawData->Warning.Text,24,"%s",CS_MW_LoadStringByIdx(CSAPP_STR_NO_SIGNAL));
		}
		else if(DrawData->Warning.Warning==CS_NO_SERVICE)
		{
			snprintf(DrawData->Warning.Text,24,"%s",CS_MW_LoadStringByIdx(CSAPP_STR_NO_SERVICE));
		}
		else if(DrawData->Warning.Warning==CS_SERVICE_ENCRYPT)
		{
			snprintf(DrawData->Warning.Text,24,"%s",CS_MW_LoadStringByIdx(CSAPP_STR_PROG_ENCRYPT));
		}
		else
		{
			// snprintf(DrawData->Warning.Text,24,"%s",CS_MW_LoadStringByIdx(CSAPP_STR_LOCK));
			snprintf(DrawData->Warning.Text,24,"%s",CS_MW_LoadStringByIdx(CSAPP_STR_LOCKED_CHANNEL));
		}

		Temp_Rect.top = DESKTOP_WARNING_Y+(DESKTOP_WARNING_H-30)/2;
		Temp_Rect.bottom = Temp_Rect.top + 65;
		Temp_Rect.left = DESKTOP_WARNING_X+20;
		Temp_Rect.right = Temp_Rect.left + DESKTOP_WARNING_W - 40;
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		CS_MW_DrawText(hdc, DrawData->Warning.Text, -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
		//CS_MW_TextOut(hdc,ScalerWidthPixel(DESKTOP_WARNING_X+20), ScalerHeigthPixel(DESKTOP_WARNING_Y+(DESKTOP_WARNING_H-30)/2), DrawData->Warning.Text);
	}
	else
	{
		b8NoSignal_Status = FALSE;
		b8Scramble_Status = FALSE;
		if(ClearFlag==TRUE) return;
		ClearFlag=TRUE;
		//printf("DesktopPaintWarning  True\n");
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(DESKTOP_WARNING_X),ScalerHeigthPixel(DESKTOP_WARNING_Y),ScalerWidthPixel(DESKTOP_WARNING_W), ScalerHeigthPixel(DESKTOP_WARNING_H));
	}
}


static void DesktopPaintPinDialog(HDC hdc,tCSDesktopInfo* DrawData)
{
	static BOOL ClearFlag=FALSE;

	if(DrawData->ParentPin.Draw==FALSE)
	{
		if(ClearFlag==TRUE)return;

		ClearFlag=TRUE;
		BKPinDraw=FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(DESKTOP_PARENTPIN_X),ScalerHeigthPixel(DESKTOP_PARENTPIN_Y),ScalerWidthPixel(DESKTOP_PARENTPIN_W), ScalerHeigthPixel(DESKTOP_PARENTPIN_H));

		return;
	}

	ClearFlag=FALSE;
	if(BKPinDraw==FALSE)
	{
		BKPinDraw=TRUE;

		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		CS_MW_TextOut(hdc, ScalerWidthPixel(DESKTOP_PARENTPIN_X+35), ScalerHeigthPixel(DESKTOP_PARENTPIN_Y+4), CS_MW_LoadStringByIdx(CSAPP_STR_INPUT_PIN));
		CS_MW_TextOut(hdc, ScalerWidthPixel(DESKTOP_PARENTPIN_X+40), ScalerHeigthPixel(DESKTOP_PARENTPIN_Y+72), CS_MW_LoadStringByIdx(CSAPP_STR_PASSWORD));
	}

	SetBkMode(hdc, BM_TRANSPARENT);
	SetBrushColor(hdc, CSAPP_BLACK_COLOR);
	FillBox(hdc,ScalerWidthPixel(DESKTOP_PARENTPIN_X+150), ScalerHeigthPixel(DESKTOP_PARENTPIN_Y+70), ScalerWidthPixel(90), ScalerHeigthPixel(20));

	SetTextColor(hdc,COLOR_yellow);
	CS_MW_TextOut(hdc, ScalerWidthPixel(DESKTOP_PARENTPIN_X+156), ScalerHeigthPixel(DESKTOP_PARENTPIN_Y+72), DrawData->ParentPin.Text);


}

#define 	VOLUME_X			390
//#define		VOLUME_Y		( BANNER_CIRCLE_Y - 50 )
#define		VOLUME_Y			560
#define		VOLUME_Y2			650
#define		VOLUME_DX			500
#define		VOLUME_DY			40
#define		VOLUME_TEXT_X		890
#define		VOLUME_TEXT_DX		50
#define		VOLUME_FULL_DX 		( MV_BMP[MVBMP_VOLUME_ICON].bmWidth + VOLUME_DX + VOLUME_TEXT_DX )

#define		SUBTITLE_OPEN_NONE	0

static void DesktopPaintVolume(HDC hdc,tCSDesktopInfo* DrawData)
{
	int      	num;
	char		acText[5];

	if ( CS_MW_Get_Subtitle_Status() == SUBTITLE_OPEN_NONE )
	{
		if( DrawData->Volume.Draw != TRUE && DrawData->Volume.OnScreen==TRUE )
		{
			DrawData->Volume.OnScreen=FALSE;
			SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
			FillBox(hdc,ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY));
			return;
		}

		if(DrawData->Volume.Draw!=TRUE)
		{
			return;
		}

		DrawData->Volume.OnScreen=TRUE;
		num=DrawData->Volume.Volume;

		FillBoxWithBitmap (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth), ScalerHeigthPixel(VOLUME_Y), ScalerWidthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmHeight), &MV_BMP[MVBMP_VOLUME_ICON]);

		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_DX + VOLUME_TEXT_DX),ScalerHeigthPixel(VOLUME_DY));
		MV_SetBrushColor(hdc,MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y+5),ScalerWidthPixel((VOLUME_DX * ((num*100)/kCS_DBU_MAX_VOLUME))/100),ScalerHeigthPixel(VOLUME_DY-10));

		sprintf(acText, "%d", num);
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(VOLUME_TEXT_X + 10),ScalerHeigthPixel(VOLUME_Y + 7), acText);
	} else {
		if( DrawData->Volume.Draw != TRUE && DrawData->Volume.OnScreen==TRUE )
		{
			DrawData->Volume.OnScreen=FALSE;
			SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
			FillBox(hdc,ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y2),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY));
			return;
		}

		if(DrawData->Volume.Draw!=TRUE)
		{
			return;
		}

		DrawData->Volume.OnScreen=TRUE;
		num=DrawData->Volume.Volume;

		FillBoxWithBitmap (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth), ScalerHeigthPixel(VOLUME_Y2), ScalerWidthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmHeight), &MV_BMP[MVBMP_VOLUME_ICON]);

		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y2),ScalerWidthPixel(VOLUME_DX + VOLUME_TEXT_DX),ScalerHeigthPixel(VOLUME_DY));
		MV_SetBrushColor(hdc,MVAPP_YELLOW_COLOR);
		FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y2+5),ScalerWidthPixel((VOLUME_DX * ((num*100)/kCS_DBU_MAX_VOLUME))/100),ScalerHeigthPixel(VOLUME_DY-10));

		sprintf(acText, "%d", num);
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(VOLUME_TEXT_X + 10),ScalerHeigthPixel(VOLUME_Y2 + 7), acText);
	}
}

#define 	MUTE_ICON_X		1080
#define		MUTE_ICON_Y		100

static void DesktopPaintMute(HDC hdc,tCSDesktopInfo* DrawData)
{
	if(DrawData->Mute.Draw==FALSE)
	{
		return;
	}

	if(DrawData->Mute.Mute==FALSE)
	{
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		FillBox(hdc,ScalerWidthPixel(MUTE_ICON_X), ScalerHeigthPixel(MUTE_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight));
	}
	else
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MUTE_ICON_X), ScalerHeigthPixel(MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight), &MV_BMP[MVBMP_MUTE_ICON]);
	}
}

static void DesktopPaintVideoMode(HDC hdc,tCSDesktopInfo* DrawData)
{
	static BOOL ClearFlag=FALSE;

	if(DrawData->VMode.Draw==TRUE)
	{
		ClearFlag=FALSE;
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y + DESKTOP_INPUTT_H), ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H));
		FillBoxWithBitmap (hdc, ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y + DESKTOP_INPUTT_H),ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight), &MV_BMP[MVBMP_CHLIST_SELBAR]);
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		CS_MW_TextOut(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X+16),ScalerHeigthPixel(DESKTOP_INPUTT_Y + DESKTOP_INPUTT_H + 2), Video_Mode[DrawData->VMode.u8VMode]);
	}
	else
	{
		if(ClearFlag!=TRUE)
		{
			SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
			FillBox(hdc,ScalerWidthPixel(DESKTOP_INPUTT_X), ScalerHeigthPixel(DESKTOP_INPUTT_Y + DESKTOP_INPUTT_H), ScalerWidthPixel(DESKTOP_INPUTT_W), ScalerHeigthPixel(DESKTOP_INPUTT_H));
			ClearFlag=TRUE;
		}
	}
}

static void DesktopPaintPause(HDC hdc,tCSDesktopInfo* DrawData)
{
	if (DrawData->Pause==FALSE)
	{
		SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
		FillBox(hdc,ScalerWidthPixel(MUTE_ICON_X - MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth));
	}
	else
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MUTE_ICON_X - MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth), &MV_BMP[MVBMP_PAUSE_ICON]);
	}
}
static void DesktopPaint(HWND hwnd,tCSDesktopInfo* DrawData)
{
	HDC			hdc;

	DesktopPaintBanner(hwnd,DrawData);

	hdc=MV_BeginPaint(hwnd);

	if(!(DrawData->Warning.Draw==FALSE && DrawData->ParentPin.Draw==TRUE))
	{
  		if (MV_Get_Password_Flag() == FALSE && MV_Get_PopUp_Window_Status() == FALSE )
			DesktopPaintWarning(hdc,DrawData);
	}

	if(DrawData->Warning.Draw==FALSE)
	{
		DesktopPaintPinDialog(hdc,DrawData);
	}

	if(DrawData->Banner.Draw==FALSE)
	{
		DesktopPaintVolume(hdc,DrawData);
	}
	DesktopPaintMute(hdc,DrawData);
	DesktopPaintVideoMode(hdc,DrawData);
	DesktopPaintPause(hdc,DrawData);
	DesktopPaintInput(hdc,DrawData);
//printf(" ###### 3 MSG_PAINT ============================\n");
	MV_EndPaint(hwnd,hdc);
}


static void UpdateBannerText(tCSDesktopInfo* DrawData)
{
	tCS_DB_Error				DBError;
	/*U16					Mjd = 0;
	U16                             		Utc = 0;
	U8						StarHour,StarMinute,EndHour,EndMinute;*/
	tCS_DB_ServiceManageData	item_data;

	DBError=CS_DB_GetCurrentList_ServiceData(&item_data, DrawData->Current_Service);

	/* By KB Kim 2011.01.18 */
	memset(DrawData->Banner.BannerInfo.ServiceName.Name, 0x00, 20);
	if(DBError!=eCS_DB_OK)
	{
		snprintf(DrawData->Banner.BannerInfo.ServiceName.Name, 20, "%s", "No channel");
		DrawData->Banner.BannerInfo.ServiceName.CH_Num = 0;
		DrawData->Banner.BannerInfo.ServiceName.u16Index = 0;

		FbSendFndDisplayNum(234);
	}
	else
	{
		// memcpy(DrawData->Banner.BannerInfo.ServiceName.Name, CS_APP_GetServiceNameByLang(&(DrawData->ServiceData)), 20);
		memcpy(DrawData->Banner.BannerInfo.ServiceName.Name, DrawData->ServiceData.acServiceName, MAX_SERVICE_NAME_LENGTH);

		if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
		{
			tCS_DB_ServiceManageData	banner_item_data;

			DrawData->Banner.BannerInfo.ServiceName.CH_Num = DrawData->Current_Service+1;
			CS_DB_GetCurrentList_ServiceData(&banner_item_data, DrawData->Current_Service);
			DrawData->Banner.BannerInfo.ServiceName.u16Index = banner_item_data.Service_Index;
			FbSendFndDisplayNum((unsigned)DrawData->Current_Service+1);
		}
		else
		{
			DrawData->Banner.BannerInfo.ServiceName.CH_Num = item_data.LCN;
			FbSendFndDisplayNum((unsigned)item_data.LCN);
		}
	}
}

#ifdef DESK_EPG
static void UpdateEPGData(tCSDesktopInfo* DrawData)
{
	tCS_EIT_Event_t					presentInfo;  // by kb : 20100406
	tCS_EIT_Event_t					followInfo;  // by kb : 20100406
	//U16			Mjd,Utc;
	//U8			StarHour,StarMinute,EndHour,EndMinute;
	tCS_EIT_Error		error;

	/* 	by kb : 20100406  */
	memset(&presentInfo, 0, sizeof(tCS_EIT_Event_t));
	memset(&followInfo, 0, sizeof(tCS_EIT_Event_t));

	error = CS_EIT_Get_PF_Event(DrawData->ServiceData.u16TransponderIndex, DrawData->ServiceData.u16ServiceId, &presentInfo, &followInfo);
	//if(EventInfo!=NULL)

	if(error == eCS_EIT_NO_ERROR)
	{
		DrawData->Banner.BannerInfo.Present.StarMjd   = presentInfo.start_date_mjd;
		DrawData->Banner.BannerInfo.Present.StartTime = presentInfo.start_time_utc;
		DrawData->Banner.BannerInfo.Present.Duration  = presentInfo.duration_utc;
		memcpy(DrawData->Banner.BannerInfo.Present.Name, presentInfo.event_name, 61);
		DrawData->Banner.BannerInfo.Present.Name[63] = '\0';
		DrawData->Banner.BannerInfo.Present.Name[62] = '\0';
#if 0
		Utc=DrawData->Banner.BannerInfo.Present.StartTime;
		//CS_DT_Caculate_EPG_Localtime(&Mjd,&Utc);
		StarHour=Utc>>8;
		StarMinute=(Utc&0x00FF);
		Utc=DrawData->Banner.BannerInfo.Present.StartTime;
		//CS_DT_Caculate_EPG_Localtime(&Mjd,&Utc);
		Utc=CS_DT_UTC_Add(Utc,DrawData->Banner.BannerInfo.Present.Duration);
		EndHour=Utc>>8;
		EndMinute=(Utc&0x00FF);
		snprintf(DrawData->Banner.BannerInfo.Present.Text,46,"%.2x:%.2x-%.2x:%.2x  %s",StarHour,StarMinute,EndHour,EndMinute,DrawData->Banner.BannerInfo.Present.Name);
#endif
		DrawData->Banner.BannerInfo.Present.Update=TRUE;

		DrawData->Banner.BannerInfo.Follow.StarMjd   = followInfo.start_date_mjd;
		DrawData->Banner.BannerInfo.Follow.StartTime = followInfo.start_time_utc;
		DrawData->Banner.BannerInfo.Follow.Duration  = followInfo.duration_utc;
		memcpy(DrawData->Banner.BannerInfo.Follow.Name, followInfo.event_name,61);
		DrawData->Banner.BannerInfo.Follow.Name[63] = '\0';
		DrawData->Banner.BannerInfo.Follow.Name[62] = '\0';
#if 0
		Utc=DrawData->Banner.BannerInfo.Follow.StartTime;
		//CS_DT_Caculate_EPG_Localtime(&Mjd,&Utc);
		StarHour=Utc>>8;
		StarMinute=(Utc&0x00FF);
		Utc=DrawData->Banner.BannerInfo.Follow.StartTime;
		//CS_DT_Caculate_EPG_Localtime(&Mjd,&Utc);
		Utc=CS_DT_UTC_Add(Utc,DrawData->Banner.BannerInfo.Follow.Duration);
		EndHour=Utc>>8;
		EndMinute=(Utc&0x00FF);
		snprintf(DrawData->Banner.BannerInfo.Follow.Text,46,"%.2x:%.2x-%.2x:%.2x  %s",StarHour,StarMinute,EndHour,EndMinute,DrawData->Banner.BannerInfo.Follow.Name);
#endif
		DrawData->Banner.BannerInfo.Follow.Update=TRUE;
	}
	else
	{
		DrawData->Banner.BannerInfo.Present.StarMjd = 0;
		DrawData->Banner.BannerInfo.Present.StartTime = 0;
		DrawData->Banner.BannerInfo.Present.Duration = 0;
		memset(DrawData->Banner.BannerInfo.Present.Name, 0, 64);
		DrawData->Banner.BannerInfo.Present.Update=TRUE;

		DrawData->Banner.BannerInfo.Follow.StarMjd = 0;
		DrawData->Banner.BannerInfo.Follow.StartTime =0;
		DrawData->Banner.BannerInfo.Follow.Duration = 0;
		memset(DrawData->Banner.BannerInfo.Follow.Name, 0, 64);
		DrawData->Banner.BannerInfo.Follow.Update=TRUE;
	}
}
#endif

static void GetBannerData(tCSDesktopInfo* DrawData)
{
	tCS_DB_ServiceListTriplet  			ListTriplet;
	tCS_DB_ServiceManageData			item_data;
	tMWStream							SUBTTXstream;
//	tCS_AV_VideoOriginalInfo			VideoFormat;
	tCS_DB_Error						DBError;

	DrawData->Banner.BannerInfo.Present.StartTime=0;
	DrawData->Banner.BannerInfo.Present.Duration=0;
	DrawData->Banner.BannerInfo.Present.Name[0]=0;
	DrawData->Banner.BannerInfo.Follow.StartTime=0;
	DrawData->Banner.BannerInfo.Follow.Duration=0;
	DrawData->Banner.BannerInfo.Follow.Name[0]=0;


	CS_DB_GetCurrentListTriplet(&ListTriplet);

	DrawData->Current_Service=CS_DB_GetCurrentService_OrderIndex();
	//printf("\n=========== %d =============\n", DrawData->Current_Service);
	DBError=CS_DB_GetCurrentList_ServiceData(&item_data, DrawData->Current_Service);

	if(DBError==eCS_DB_OK)
	{
		DBError=MV_DB_GetServiceDataByIndex(&DrawData->ServiceData, item_data.Service_Index);
	}

	if(DBError!=eCS_DB_OK)
	{
		DrawData->Service_Index=0xFFFF;
		DrawData->TPIndex=0xFFFF;
#ifdef FOR_USA
		DrawData->ServiceData.sCS_DB_LCN=0;
#endif
		//strcpy(DrawData->ServiceData.sCS_DB_ServiceName,"No Channel");
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.HD=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE=FALSE;
		DrawData->Banner.BannerInfo.ServiceAttribute.EPG.EPG=FALSE;

		DrawData->Warning.Warning=CS_NO_SERVICE;
		DrawData->Warning.Draw=TRUE;

		return;
	}

	DrawData->Service_Index=item_data.Service_Index;
	DrawData->TPIndex=DrawData->ServiceData.u16TransponderIndex;
#if 0
	CS_AV_GetVideoOriginalInfo(&VideoFormat);

	printf( "Current Video Format = %d FrameRate = %d \n",
	             VideoFormat.Video_Definition,
	             VideoFormat.FrameRate );
	printf( "              Width  = %d Height = %d\n",
	             VideoFormat.Width,
	             VideoFormat.Height );

	if(VideoFormat.Height >= 720)
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.HD=TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.HD.HD=FALSE;
#endif

	CS_MW_GetSubtitleStream(&SUBTTXstream);

	if(SUBTTXstream.Number==0)
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB=FALSE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.Sub.SUB=TRUE;

	CS_MW_GetTeletextStream(&SUBTTXstream);

	if(SUBTTXstream.Number==0)
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX=FALSE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.TTX.TTX=TRUE;

	//DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=DrawData->ServiceData.u8Audio_Type;
	DrawData->Banner.BannerInfo.ServiceAttribute.AC3.AC3=0;
#if 0
	if(DrawData->ServiceData.sCS_DB_FavoriteGroup!=0)
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.Fav.FAV=FALSE;
#endif
	if ( DrawData->ServiceData.u8Lock == 1 )
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK = TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.Lock.LOCK = FALSE;

	if ( DrawData->ServiceData.u8Scramble == 1 )
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE = TRUE;
	else
		DrawData->Banner.BannerInfo.ServiceAttribute.SCRAMBLE.SCRAMBLE = FALSE;

}

static void InitDesktopData(void)
{
	memset(&DeskData, 0, sizeof(tCSDesktopInfo));

	DeskData.Banner.Draw=TRUE;
	DeskData.Banner.OnScreen=FALSE;
	DeskData.Banner.BannerInfo.DrawBMP=TRUE;
	DeskData.Banner.BannerInfo.ServiceName.Update=TRUE;
	DeskData.Banner.BannerInfo.Follow.Update=TRUE;
	DeskData.Banner.BannerInfo.Present.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.AC3.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.HD.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.TTX.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.Sub.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.Fav.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.Lock.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.SCRAMBLE.Update=TRUE;
	DeskData.Banner.BannerInfo.ServiceAttribute.EPG.Update=TRUE;

	DeskData.Input.Draw=FALSE;
	DeskData.Warning.Draw=FALSE;
	DeskData.Volume.Draw=FALSE;
	DeskData.Pause=FALSE;

	DeskData.ParentPin.Cursor=0;
	strcpy(DeskData.ParentPin.Text,"- - - -");
	memset(DeskData.ParentPin.Input,0,5);
	//DeskData.ParentPin.Draw=FALSE;//PinDlg_GetStatus();
	//DeskData.ParentPin.Lock=FALSE;//PinDlg_GetStatus();

	CS_DBU_LoadMuteStatus();
	DeskData.Mute.Mute=CS_DBU_GetMuteStatus();
	DeskData.Mute.Draw=TRUE;
	DeskData.VMode.Draw=FALSE;

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
#if 0
	DeskData.Subtitle.Pid=CS_MW_GetSubtitlePid();

	if(DeskData.Subtitle.Pid>=kDB_DEMUX_INVAILD_PID)
		DeskData.Subtitle.Open=FALSE;
	else
		DeskData.Subtitle.Open=TRUE;
#endif
}

static void DesktopNotify(tMWNotifyData NotifyData)
{
	switch(NotifyData.type)
	{
		case UPDATE_PMT:
			break;

		case UPDATE_FE:
			BroadcastMessage (MSG_UPDATE_FE, NotifyData.uData.FEStatus, 0);
			break;

		case UPDATE_TTX:
			BroadcastMessage (MSG_UPDATE_TTX, 0, 0);
			break;

		case UPDATE_SUB:
			BroadcastMessage (MSG_UPDATE_SUB, 0, 0);
			break;

		case UPDATE_HD:
			BroadcastMessage (MSG_UPDATE_HD, 0, 0);
			break;

		default:
			break;
	}
}


