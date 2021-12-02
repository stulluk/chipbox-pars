#include "linuxos.h"
#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csmpr_common.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"
#include "date_time.h"
#include "eit_engine.h"
#include "scart.h"
#include "csdesktop.h"
#include "sattp.h"
#include "ui_common.h"

#define		MV_KEY_RIGHT				TRUE
#define		MV_KEY_LEFT					FALSE

#define		Banner_timer_Max			( CS_DBU_GetBannerKeepTime()*1000 )
#define		Volume_timer_Max			( 2*1000 )
#define		ONE_HOUR					3600
#define		TWO_HOUR					( ONE_HOUR * 2 )

#define 	MUTE_ICON_X					1080
#define		MUTE_ICON_Y					100
#define 	PVR_ICON_X					MUTE_ICON_X
#define		PVR_ICON_Y					( MUTE_ICON_Y - MV_BMP[MVBMP_PVR_REC].bmHeight - 10 )
#define 	PVR_ICON_REW_X				( PVR_ICON_X - MV_BMP[MVBMP_PVR_REC].bmWidth - 8 )
#define 	PVR_ICON_FF_X				( PVR_ICON_X + MV_BMP[MVBMP_PVR_REC].bmWidth + 8 )

#define 	VOLUME_X					390
#define		VOLUME_Y					560
#define		VOLUME_DX					500
#define		VOLUME_DY					40
#define		VOLUME_TEXT_X				890
#define		VOLUME_TEXT_DX				50
#define		VOLUME_FULL_DX 				( MV_BMP[MVBMP_VOLUME_ICON].bmWidth + VOLUME_DX + VOLUME_TEXT_DX )

/**************************** Record Info Banner **********************************/

#define		BANNER_STAR_Y				572
#define		BANNER_CIRCLE_Y				(BANNER_STAR_Y - (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2))
#define 	PLAY_STAR_X					100
#define		PLAY_STAR_Y					BANNER_CIRCLE_Y
#define 	PLAY_STAR_W					1080
#define		PLAY_STAR_H					200
#define		PLAY_TIME_Y					( PLAY_STAR_Y + 56 )
#define		PLAY_TIME_DY				( PLAY_STAR_Y - PLAY_TIME_Y )
#define		PLAY_ITEM_X					100
#define		PLAY_ITEM_Y1				( PLAY_STAR_Y + 56 )
#define		PLAY_ITEM_Y2				( PLAY_ITEM_Y1 + 30 )
#define		PLAY_ITEM_Y3				( PLAY_ITEM_Y2 + 34 )
#define		PLAY_ITEM_H					28
#define		PLAY_NAME_X					( PLAY_ITEM_X + MV_BMP[MVBMP_PVR_PLAY].bmWidth + 20 )
#define		PLAY_NAME_Y					( PLAY_STAR_Y + 56 )
#define 	PLAY_NAME_DX				240
#define		PLAY_DATE_X					826
#define		PLAY_DATE_Y					PLAY_NAME_Y
#define		PLAY_TIME_X					( PLAY_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 100 )
#define		PLAY_STORAGE_X				( PLAY_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 100 )
#define		PLAY_PROGRESS_X				250
#define 	PLAY_PROGRESS_DX			470

/**********************************************************************************/

#define 	PVR_CH_WINDOW_X				160
#define 	PVR_CH_WINDOW_Y				130
#define		PVR_CH_WINDOW_DX			470
#define 	PVR_CH_WINDOW_DY			430
#define 	PVR_CH_WINDOW_NAME_X		170
#define 	PVR_CH_WINDOW_NAME_DX		310
#define 	PVR_CH_WINDOW_SIZE_X		( PVR_CH_WINDOW_NAME_X + PVR_CH_WINDOW_NAME_DX )
#define 	PVR_CH_WINDOW_SIZE_DX		120
#define 	PVR_CH_WINDOW_ITEM_DX		( PVR_CH_WINDOW_NAME_DX + PVR_CH_WINDOW_SIZE_DX )
#define		PVR_CH_WINDOW_TITLE_Y		140
#define		PVR_CH_WINDOW_LIST_Y		180
#define 	PVR_CH_WINDOW_SCROLL_X		( PVR_CH_WINDOW_NAME_X + PVR_CH_WINDOW_ITEM_DX )
#define 	PVR_CH_WINDOW_SCROLL_DX		20
#define 	PVR_CH_WINDOW_SCROLL_Y		PVR_CH_WINDOW_LIST_Y
#define 	PVR_CH_WINDOW_SCROLL_DY		300
#define		PVR_CH_WINDOW_DATA_Y		490

#define		PVR_LIST_ITEM				10

/**********************************************************************************/

static CSAPP_Applet_t					CSApp_PVR_Player_Applets;
static tCSDesktopVolume 				PVR_Volume;
static stPvrBanner						PVR_Play_Banner;
static stPvrBanner						PVR_Play_Pause;
static time_t							StartTime;
static time_t							EndTime;
static time_t							DurationTime;
static time_t							NowStartTime;
static time_t							NowTime;
static char								GetFileName[256];
static st_pvr_data						Get_PVR_Rec_Data;
static st_pvr_data						Get_TmpPVR_Rec_Data;
static BOOL								b8Position_Move = FALSE;
static stFile_db						stFileDB;
static 	BOOL							IsThereFile = FALSE;
static 	BOOL							b8List_Status = FALSE;
static 	U16								u16PVR_index;
static 	U16								u16PVR_Focus;
static 	U16								u16PVR_Cur_Page;
static 	U16								u16PVR_Pre_Page;

static BITMAP							btpDR_Capture;
static BITMAP							btpTime_Capture;

static int PVR_Player_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);
extern void MV_Calculate_Size(long long llong, char *temp);

void PVR_Player_Draw_Icon(HDC hdc)
{
	SetBrushColor(hdc, COLOR_transparent);
	FillBox(hdc,ScalerWidthPixel(PVR_ICON_REW_X),ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel((PVR_ICON_FF_X + MV_BMP[MVBMP_FORWARD_2X].bmWidth) - PVR_ICON_REW_X),ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight));
	
	switch(CSMPR_Player_GetStatus())
	{
		case CSMPR_PLAY_RUN:
			if ( CS_MW_GetCurrentMenuLanguage() == CS_APP_TURCK )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_RUN_TR]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_RUN]);
			break;
		case CSMPR_PLAY_PAUSE:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_PVR_RUN]);
			break;
		case CSMPR_PLAY_FF_2X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_FF_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_FORWARD_2X]);
			break;
		case CSMPR_PLAY_FF_4X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_FF_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_FORWARD_4X]);
			break;
		case CSMPR_PLAY_FF_8X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_FF_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_FORWARD_8X]);
			break;
		case CSMPR_PLAY_FF_16X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_FF_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_FORWARD_16X]);
			break;
		case CSMPR_PLAY_REW_2X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_REW_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_BACKWARD_2X]);
			break;
		case CSMPR_PLAY_REW_4X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_REW_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_BACKWARD_4X]);
			break;
		case CSMPR_PLAY_REW_8X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_REW_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_BACKWARD_8X]);
			break;
		case CSMPR_PLAY_REW_16X:
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_REW_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RUN].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RUN].bmHeight), &MV_BMP[MVBMP_BACKWARD_16X]);
			break;
		default:
			break;
	}
}

void PVR_Play_Draw_NowPlaying(HWND hwnd)
{
	HDC hdc;
	
	hdc=BeginPaint(hwnd);
	MV_Draw_Msg_Window(hdc, CSAPP_STR_NOW_PLAY);
	EndPaint(hwnd,hdc);

	usleep( 2000*1000 );

	hdc=BeginPaint(hwnd);
	Close_Msg_Window(hdc);
	EndPaint(hwnd,hdc);
}

void PVR_Play_Draw_Duration(HDC hdc)
{
	RECT				Usage_Rect;
	U16 				LevelValue = 0;
	U16					MoveValue = 0;
	char				acStart[16];
	char				acEnd[16];
	char				acNow[16];
	char				acMove[16];
	char				Temp_Str[30];
	struct tm			tm_time;
	time_t				Temp_Time;
	time_t				Move_Time;
	struct timespec		time_value;

	memset( acStart, 0x00, 16);
	memset( acEnd, 0x00, 16);
	memset( acNow, 0x00, 16);
	memset( Temp_Str, 0x00, 30);

	Usage_Rect.top = PLAY_ITEM_Y3;
	Usage_Rect.bottom = Usage_Rect.top + MV_INSTALL_MENU_HEIGHT + 10;
	Usage_Rect.left = PLAY_PROGRESS_X;
	Usage_Rect.right = Usage_Rect.left + PLAY_PROGRESS_DX;

	if ( btpDR_Capture.bmHeight == 0 )
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(Usage_Rect.left - 20), ScalerHeigthPixel(Usage_Rect.top - 50), ScalerWidthPixel(PLAY_PROGRESS_DX + 380), ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT + 70), &btpDR_Capture);
	else
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(Usage_Rect.left - 20), ScalerHeigthPixel(Usage_Rect.top - 50), ScalerWidthPixel(PLAY_PROGRESS_DX + 380), ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT + 70), &btpDR_Capture);

	CSMPR_Player_GetDuration(&DurationTime);

	clock_gettime(CLOCK_REALTIME, &time_value);
	NowTime = time_value.tv_sec;

//	Temp_Time = NowTime - NowStartTime;
//	Temp_Time = StartTime + Temp_Time;

	CSMPR_Player_GetElapsedTime( &Temp_Time );
	CSMPR_Player_GetMoveTime( &Move_Time );

	LevelValue = (U16)( ( Temp_Time * 100 ) / DurationTime );
	MoveValue = (U16)( ( Move_Time * 100 ) / DurationTime );

	memcpy(&tm_time, localtime(&StartTime), sizeof(tm_time));
	sprintf(acStart, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);

	if ( EndTime == 0 )
	{
		memcpy(&tm_time, localtime(&DurationTime), sizeof(tm_time));
		sprintf(acEnd, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
	} else {
		memcpy(&tm_time, localtime(&EndTime), sizeof(tm_time));
		sprintf(acEnd, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
	}

	memcpy(&tm_time, localtime(&Temp_Time), sizeof(tm_time));
	sprintf(acNow, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);

	memcpy(&tm_time, localtime(&Move_Time), sizeof(tm_time));
	sprintf(acMove, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_Draw_Time_Progress_Bar(hdc, &Usage_Rect, LevelValue, MoveValue, acStart, acEnd, Temp_Time );
	//memcpy(&tm_time, localtime(&DurationTime), sizeof(tm_time));
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(PLAY_DATE_X + 50),ScalerHeigthPixel(PLAY_ITEM_Y2 + 8), Get_PVR_Rec_Data.PVR_Start_UTC);

	if ( b8Position_Move == TRUE )
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(PLAY_DATE_X - 96),ScalerHeigthPixel(PLAY_ITEM_Y2 + 8), acMove);
	
	memcpy(&tm_time, localtime(&DurationTime), sizeof(tm_time));
	sprintf(Temp_Str, "%s / %02d:%02d", acNow, tm_time.tm_hour, tm_time.tm_min);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(PLAY_DATE_X + 50),ScalerHeigthPixel(PLAY_ITEM_Y3), Temp_Str);
}

static void PVR_Play_VolumeScroll(BOOL right)
{
	PVR_Volume.Volume=CS_DBU_GetVolume();

	if(CS_AV_Audio_GetMuteStatus() == TRUE)
		CS_AV_Audio_SetMuteStatus(FALSE);
	
	if(right == FALSE)
	{
		if(PVR_Volume.Volume<=0)
			PVR_Volume.Volume = 0;
		else
			PVR_Volume.Volume--;
	}
	else
	{
		PVR_Volume.Volume++;
		if(PVR_Volume.Volume > kCS_DBU_MAX_VOLUME)
			PVR_Volume.Volume = kCS_DBU_MAX_VOLUME;
	}
	
	if(PVR_Volume.Volume == 0)
	{
		CS_DBU_SetMuteStatus(eCS_DBU_ON);
		CS_AV_Audio_SetMuteStatus(TRUE);
		CS_DBU_SaveMuteStatus();
	}
	else
	{
		if(CS_AV_Audio_GetMuteStatus()==TRUE)
		{
			CS_DBU_SetMuteStatus(eCS_DBU_OFF);
			CS_AV_Audio_SetMuteStatus(FALSE);
			CS_DBU_SaveMuteStatus();
		}
	}
	
	PVR_Volume.Draw=TRUE;
	CS_DBU_SetVolume(PVR_Volume.Volume);

	CS_AV_AudioSetVolume(PVR_Volume.Volume);
	
	CS_DBU_SaveVolume();
	CS_DBU_SaveMuteStatus();
    
}

static void PVR_Play_PaintVolume(HDC hdc)
{
	int      	num;
	char		acText[5];

	if( PVR_Volume.Draw != TRUE && PVR_Volume.OnScreen == TRUE )
	{
		PVR_Volume.OnScreen=FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY));
		return;
	}
	
	PVR_Volume.OnScreen = TRUE;
	num = PVR_Volume.Volume;

	FillBoxWithBitmap (hdc, ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth), ScalerHeigthPixel(VOLUME_Y), ScalerWidthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_VOLUME_ICON].bmHeight), &MV_BMP[MVBMP_VOLUME_ICON]);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y),ScalerWidthPixel(VOLUME_DX + VOLUME_TEXT_DX),ScalerHeigthPixel(VOLUME_DY));
	MV_SetBrushColor(hdc,MVAPP_YELLOW_COLOR);
	FillBox(hdc,ScalerWidthPixel(VOLUME_X),ScalerHeigthPixel(VOLUME_Y+5),ScalerWidthPixel((VOLUME_DX * ((num*100)/kCS_DBU_MAX_VOLUME))/100),ScalerHeigthPixel(VOLUME_DY-10));

	sprintf(acText, "%d", num);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(VOLUME_TEXT_X + 10),ScalerHeigthPixel(VOLUME_Y + 7), acText);
}

static void PVR_Player_PaintMute(HDC hdc)
{
	if(CS_AV_Audio_GetMuteStatus() == FALSE)
	{
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(MUTE_ICON_X), ScalerHeigthPixel(MUTE_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight));
	}
	else
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MUTE_ICON_X), ScalerHeigthPixel(MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_MUTE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUTE_ICON].bmHeight), &MV_BMP[MVBMP_MUTE_ICON]);
	}
}

static void PVR_Player_PaintPause(HDC hdc)
{
	if( PVR_Play_Pause.Draw != TRUE && PVR_Play_Pause.OnScreen == TRUE )
	{
		PVR_Play_Pause.OnScreen = FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(MUTE_ICON_X - MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MUTE_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmHeight));
	}
	else
	{
		PVR_Play_Pause.OnScreen = TRUE;
		FillBoxWithBitmap (hdc, ScalerWidthPixel(MUTE_ICON_X - MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MUTE_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PAUSE_ICON].bmHeight), &MV_BMP[MVBMP_PAUSE_ICON]);
	}
}

static void PVR_Play_Draw_Time(HDC hdc)
{
	U16 			local_utc;
	U16 			local_mjd;
	tCS_DT_Time     local_time;
	tCS_DT_Date		local_date;
	tCS_DT_Date  	Current_Date;
	tCS_DT_Time		Current_Time;
	char 			acDate[100];
	char 			acTime[100];

	  /* Recording Information Banner */
	memset( acTime, 0x00, 100 );
	memset( acDate, 0x00, 100 );
	
	if ( CS_MW_GetTimeMode() == CS_APP_TIME_MODE_NET )
	{
#if 0
		local_utc = CS_DT_GetLocalUTC();
		local_mjd = CS_DT_GetLocalMJD();
		local_time = CS_DT_UTCtoHM(local_utc);
		local_date = CS_DT_MJDtoYMD(local_mjd);
#else
		MV_OS_Get_Time_to_MJD_UTC_Date_Time(&local_mjd, &local_utc, &local_date, &local_time);
#endif

		sprintf(acDate, "%02d/%02d", local_date.day, local_date.month);
		sprintf(acTime, "%02d:%02d", local_time.hour, local_time.minute);
	} else if ( CS_MW_GetTimeMode() == CS_APP_TIME_MODE_LOCAL ) {
#if 0
		Current_Date = CS_DT_MJDtoYMD(CS_DT_GetLocalMJD());
		Current_Time = CS_DT_UTCtoHM(CS_DT_GetLocalUTC());
#else
		MV_OS_Get_Time_to_MJD_UTC_Date_Time(&local_mjd, &local_utc, &Current_Date, &Current_Time);
#endif
		sprintf(acDate, "%02d/%02d", Current_Date.day, Current_Date.month);
		sprintf(acTime, "%02d:%02d", Current_Time.hour, Current_Time.minute);
	} else {
		MV_OS_Get_Time_Offset_Splite(acDate, acTime, TRUE);
	}	

	if ( btpTime_Capture.bmHeight == 0 )
	{
		// MV_GetBitmapFromDC (hdc, ScalerWidthPixel(PLAY_DATE_X), ScalerHeigthPixel(PLAY_DATE_Y), ScalerWidthPixel(PLAY_STORAGE_X - PLAY_DATE_X), ScalerHeigthPixel(btpTime_Capture.bmHeight - 28), &btpTime_Capture);
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(PLAY_DATE_X), ScalerHeigthPixel(PLAY_DATE_Y), ScalerWidthPixel(PLAY_STORAGE_X - PLAY_DATE_X), ScalerHeigthPixel(28), &btpTime_Capture);
	}
	else
	{
		// MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(PLAY_DATE_X), ScalerHeigthPixel(PLAY_DATE_Y), ScalerWidthPixel(PLAY_STORAGE_X - PLAY_DATE_X), ScalerHeigthPixel(btpTime_Capture.bmHeight - 28), &btpTime_Capture);
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(PLAY_DATE_X), ScalerHeigthPixel(PLAY_DATE_Y), ScalerWidthPixel(PLAY_STORAGE_X - PLAY_DATE_X), ScalerHeigthPixel(28), &btpTime_Capture);
	}

	FillBoxWithBitmap(hdc, ScalerWidthPixel(PLAY_DATE_X), ScalerHeigthPixel(PLAY_DATE_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_DATE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_DATE]);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(PLAY_TIME_X), ScalerHeigthPixel(PLAY_DATE_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_TIME].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_TIME].bmHeight), &MV_BMP[MVBMP_PVR_TIME]);

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	
	CS_MW_TextOut(hdc,ScalerWidthPixel(PLAY_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 6),ScalerHeigthPixel(PLAY_DATE_Y), acDate);
	CS_MW_TextOut(hdc,ScalerWidthPixel(PLAY_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 6),ScalerHeigthPixel(PLAY_DATE_Y), acTime);

/*    Check between System Tiem and TDT time (MVAPP Time) */
#if 0
	MV_OS_Get_Time_Offset_Splite(acDate, acTime, TRUE);

	CS_MW_TextOut(hdc,ScalerWidthPixel(PLAY_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(PLAY_DATE_Y + 36), acDate);
	CS_MW_TextOut(hdc,ScalerWidthPixel(PLAY_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 10),ScalerHeigthPixel(PLAY_DATE_Y + 36), acTime);
#endif
/**********************************************************/
}

static void PVR_Player_PaintBanner(HDC hdc)
{
	RECT			rc;
	char    		Text[256];

	if(PVR_Play_Banner.Draw == TRUE)
	{
		PVR_Volume.Draw = FALSE;
	}
	
	if( PVR_Play_Banner.Draw != TRUE && PVR_Play_Banner.OnScreen == TRUE )
	{
		printf("PVR_Player_PaintBanner : Remove\n");
		PVR_Play_Banner.OnScreen = FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(PLAY_STAR_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(PLAY_STAR_H));
		CSMPR_Player_Clear_TempPosition();
		b8Position_Move = FALSE;
		UnloadBitmap (&btpDR_Capture);
		UnloadBitmap (&btpTime_Capture);

		memset(&btpDR_Capture, 0x00, sizeof(BITMAP));
		memset(&btpTime_Capture, 0x00, sizeof(BITMAP));
		return;
	}
    
	if( PVR_Play_Banner.Draw != TRUE )
	{
		return;
	}

	PVR_Play_Banner.OnScreen=TRUE;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(0), ScalerHeigthPixel(PLAY_STAR_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_INFOBAR].bmHeight), &MV_BMP[MVBMP_PVR_INFOBAR]);

	FillBoxWithBitmap(hdc, ScalerWidthPixel(PLAY_ITEM_X), ScalerHeigthPixel(PLAY_STAR_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_PLAY].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_PLAY].bmHeight), &MV_BMP[MVBMP_PVR_PLAY]);
	
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);

	rc.left = ScalerWidthPixel(PLAY_NAME_X);
	rc.top = ScalerHeigthPixel(PLAY_NAME_Y + 6);
	rc.right = ScalerWidthPixel(PLAY_NAME_X + PLAY_NAME_DX + 300);
	rc.bottom = ScalerHeigthPixel(PLAY_NAME_Y + PLAY_ITEM_H);

	if ( Get_PVR_Rec_Data.PVR_Ch_Index[0] == 0x00 )
	{
		CSMPR_Player_Get_Title(Text);
	} else {
		sprintf(Text, "%s : %s", Get_PVR_Rec_Data.PVR_Ch_Index, Get_PVR_Rec_Data.PVR_Title);
	}
	CS_MW_DrawText(hdc, Text, -1, &rc, DT_LEFT);

	PVR_Play_Draw_Time(hdc);
	PVR_Play_Draw_Duration(hdc);
}

void PVR_Draw_FileBoard(HDC hdc)
{
	RECT	Text_Rect;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_CH_WINDOW_X), ScalerHeigthPixel(PVR_CH_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_CH_WINDOW_X + PVR_CH_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(PVR_CH_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_CH_WINDOW_X), ScalerHeigthPixel(PVR_CH_WINDOW_Y + PVR_CH_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_CH_WINDOW_X + PVR_CH_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(PVR_CH_WINDOW_Y + PVR_CH_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(PVR_CH_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(PVR_CH_WINDOW_Y),ScalerWidthPixel(PVR_CH_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(PVR_CH_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(PVR_CH_WINDOW_X), ScalerHeigthPixel(PVR_CH_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(PVR_CH_WINDOW_DX),ScalerHeigthPixel(PVR_CH_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	
	FillBox(hdc,ScalerWidthPixel(PVR_CH_WINDOW_NAME_X), ScalerHeigthPixel(PVR_CH_WINDOW_TITLE_Y),ScalerWidthPixel(PVR_CH_WINDOW_ITEM_DX + 20),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H));	
	Text_Rect.top = ScalerWidthPixel(PVR_CH_WINDOW_TITLE_Y + 4);
	Text_Rect.bottom = ScalerWidthPixel(Text_Rect.top + MV_INSTALL_MENU_BAR_H - 4);
	Text_Rect.left = ScalerWidthPixel(PVR_CH_WINDOW_NAME_X);
	Text_Rect.right = ScalerWidthPixel(Text_Rect.left + PVR_CH_WINDOW_ITEM_DX + 20);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_REC_FILE), -1, &Text_Rect, DT_CENTER);
}

void PVR_Draw_FileList_Item(HDC hdc, int esItem, U8 u8Kind)
{
	RECT				TmpRect;
	struct stat			statbuffer;
	char				TmpStr[256];
	char				FileName[256];
	CSMPR_FILEINFO  	fileInfo;
	struct tm			tm_time;
	time_t				Temp_Duration = 0;
	
	if ( esItem == 0xFFFF )
	{
		int 		y_gap = PVR_CH_WINDOW_LIST_Y + MV_INSTALL_MENU_BAR_H * ( 4 + 1 );

		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		TmpRect.left	= ScalerWidthPixel(PVR_CH_WINDOW_NAME_X);
		TmpRect.right	= ScalerHeigthPixel(TmpRect.left + PVR_CH_WINDOW_ITEM_DX);
		TmpRect.top		= ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	= ScalerHeigthPixel(TmpRect.top + MV_INSTALL_MENU_BAR_H);

		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NO_FILE), -1, &TmpRect, DT_CENTER);
	} else {
		int 			y_gap = PVR_CH_WINDOW_LIST_Y + MV_INSTALL_MENU_BAR_H * esItem ;

		sprintf(FileName, "%s/%s", RECfile, stFileDB.acFileName[esItem + ( u16PVR_Cur_Page * PVR_LIST_ITEM)]);
		get_file_info_simple( FileName, &fileInfo );
		
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		if( u8Kind == FOCUS ) {
			FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_CH_WINDOW_NAME_X), ScalerHeigthPixel(y_gap), ScalerWidthPixel(PVR_CH_WINDOW_ITEM_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H), &MV_BMP[MVBMP_CHLIST_SELBAR]);
			
			memset(&Get_TmpPVR_Rec_Data, 0x00, sizeof(st_pvr_data));
			MV_PVR_CFG_File_Parser(&Get_TmpPVR_Rec_Data, FileName);
#if 0			
			printf("\n\nName : %s\n", Get_TmpPVR_Rec_Data.PVR_Ch_Index);
			printf("Title : %s\n", Get_TmpPVR_Rec_Data.PVR_Title);
			printf("Start : %s\n", Get_TmpPVR_Rec_Data.PVR_Start_UTC);
			printf("End : %s\n", Get_TmpPVR_Rec_Data.PVR_End_UTC);
			printf("Position : %lld\n\n\n", Get_TmpPVR_Rec_Data.PVR_Last_Posion);
#endif
			MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(PVR_CH_WINDOW_NAME_X),ScalerHeigthPixel( PVR_CH_WINDOW_DATA_Y ), ScalerWidthPixel(PVR_CH_WINDOW_ITEM_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H * 2) );

			TmpRect.left	= ScalerWidthPixel(PVR_CH_WINDOW_NAME_X);
			TmpRect.right	= ScalerHeigthPixel(TmpRect.left + PVR_CH_WINDOW_ITEM_DX);
			TmpRect.top		= ScalerHeigthPixel(PVR_CH_WINDOW_DATA_Y);
			TmpRect.bottom	= ScalerHeigthPixel(TmpRect.top + MV_INSTALL_MENU_BAR_H);
			
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			CS_MW_DrawText(hdc, Get_TmpPVR_Rec_Data.PVR_Title, -1, &TmpRect, DT_LEFT);
			
			TmpRect.top = ScalerHeigthPixel(TmpRect.bottom);
			TmpRect.bottom = ScalerHeigthPixel(TmpRect.top + MV_INSTALL_MENU_BAR_H);

			Temp_Duration = Get_TmpPVR_Rec_Data.PVR_End_OS - Get_TmpPVR_Rec_Data.PVR_Start_OS;
			memcpy(&tm_time, localtime(&Temp_Duration), sizeof(tm_time));
			sprintf(TmpStr, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);
			CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_LEFT);
		} else {
			if ( esItem % 2 == 0 )
			{
				MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
				MV_FillBox( hdc, ScalerWidthPixel(PVR_CH_WINDOW_NAME_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(PVR_CH_WINDOW_ITEM_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			} else {
				MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(PVR_CH_WINDOW_NAME_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(PVR_CH_WINDOW_ITEM_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			}
		}

		if ( ( esItem + ( u16PVR_Cur_Page * PVR_LIST_ITEM) ) < stFileDB.u16file_Count )
		{
			TmpRect.left	= ScalerWidthPixel(PVR_CH_WINDOW_NAME_X + 4);
			TmpRect.right	= ScalerWidthPixel(TmpRect.left + PVR_CH_WINDOW_NAME_DX - 4);
			TmpRect.top		= ScalerHeigthPixel(y_gap+4);
			TmpRect.bottom	= ScalerWidthPixel(TmpRect.top + MV_INSTALL_MENU_BAR_H);
			
			if( stat(FileName, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");

			if( S_ISDIR(statbuffer.st_mode) )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_OK_ICON].bmWidth ), ScalerHeigthPixel(y_gap), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);

			CS_MW_DrawText(hdc, stFileDB.acFileName[esItem + ( u16PVR_Cur_Page * PVR_LIST_ITEM)], -1, &TmpRect, DT_LEFT);

			memset(TmpStr, 0x00, 256);
			if( !(S_ISDIR(statbuffer.st_mode)) )
			{
				TmpRect.left	= ScalerWidthPixel(PVR_CH_WINDOW_SIZE_X);
				TmpRect.right	= ScalerWidthPixel(TmpRect.left + PVR_CH_WINDOW_SIZE_DX);
				MV_Calculate_Size( ( fileInfo.total_size / 1024 ), TmpStr);
				CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_RIGHT);
			}
		}
		
		if ( u8Kind == FOCUS )
		{
			TmpRect.top = PVR_CH_WINDOW_SCROLL_Y;
			TmpRect.left = PVR_CH_WINDOW_SCROLL_X;
			TmpRect.right = TmpRect.left + PVR_CH_WINDOW_SCROLL_DX;
			TmpRect.bottom = TmpRect.top + PVR_CH_WINDOW_SCROLL_DY;
			MV_Draw_ScrollBar(hdc, TmpRect, u16PVR_index, stFileDB.u16file_Count - 1, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
		}
	}
}

void PVR_Draw_File_List(HDC hdc)
{
	U16 	i = 0;

	if ( IsThereFile == TRUE )
	{
		for( i = 0 ; i < PVR_LIST_ITEM ; i++ )
		{
			if ( i == u16PVR_Focus )
				PVR_Draw_FileList_Item(hdc, i, FOCUS);
			else
				PVR_Draw_FileList_Item(hdc, i, UNFOCUS);
		}
	} else {
		PVR_Draw_FileList_Item(hdc, 0xFFFF, UNFOCUS);
	}
}

void PVR_Draw_File_List_Window(HWND hwnd)
{
	HDC 		hdc;
	
	u16PVR_index = 0;
	u16PVR_Focus = 0;
	u16PVR_Cur_Page = 0;
	u16PVR_Pre_Page = 0;
	b8List_Status = TRUE;
	
	memset(&stFileDB, 0x00, sizeof(stFile_db));
	
	if ( MV_Load_TS_fileData(&stFileDB) != FILE_OK )
		IsThereFile = FALSE;
	else
		IsThereFile = TRUE;

	hdc=MV_BeginPaint(hwnd);
	PVR_Draw_FileBoard(hdc);
	PVR_Draw_File_List(hdc);
	MV_EndPaint(hwnd,hdc);
}

void PVR_Clear_File_List_Window(HWND hwnd)
{
	HDC 		hdc;
	
	b8List_Status = FALSE;

	hdc=MV_BeginPaint(hwnd);
	SetBrushColor(hdc, COLOR_transparent);
	FillBox(hdc,ScalerWidthPixel(PVR_CH_WINDOW_X),ScalerHeigthPixel(PVR_CH_WINDOW_Y),ScalerWidthPixel(PVR_CH_WINDOW_DX), ScalerHeigthPixel(PVR_CH_WINDOW_DY));
	MV_EndPaint(hwnd,hdc);
}

void PVR_File_List_Proc(HWND hwnd, U8 u8Key)
{
	HDC 		hdc;
	char		TmpStr[256];

	if ( MV_Check_Confirm_Window() == TRUE )
	{
		MV_Confirm_Proc(hwnd, u8Key);

		if ( u8Key == CSAPP_KEY_ESC || u8Key == CSAPP_KEY_MENU || u8Key == CSAPP_KEY_ENTER )
		{
			if ( u8Key == CSAPP_KEY_ENTER )
			{
				char 		acPlay_File[100];
				long long	Temp_Position = 0;

				sprintf(acPlay_File, "%s/%s", RECfile, stFileDB.acFileName[u16PVR_index]);
					
				if ( MV_Check_YesNo() == TRUE )
				{	
					Temp_Position = Get_TmpPVR_Rec_Data.PVR_Last_Posion;
				} else {
					Temp_Position = 0;
				}

				CSMPR_Player_Stop();
				
				hdc = BeginPaint(hwnd);
				Restore_Confirm_Window(hdc);
				EndPaint(hwnd,hdc);

				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
				EndPaint(hwnd,hdc);

				if ( CSMPR_Player_Start(acPlay_File, Temp_Position) == CSMPR_ERROR )
				{
					CSMPR_Player_Resume();
					
					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
					
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_PLAY_ERROR);
					EndPaint(hwnd,hdc);

					usleep( 1000 * 1000 );

					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
				} else {
					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);

					SendMessage(hwnd,MSG_CREATE,0,0);
					
					PVR_Clear_File_List_Window(hwnd);
				}
			} else {
				hdc = BeginPaint(hwnd);
				Restore_Confirm_Window(hdc);
				EndPaint(hwnd,hdc);
			}
		}
		
		if (u8Key == CSAPP_KEY_IDLE)
		{
			PVR_Clear_File_List_Window(hwnd);
		}
	}else {

		switch(u8Key)
		{
			case CSAPP_KEY_UP:
				hdc = BeginPaint(hwnd);
				u16PVR_Pre_Page = u16PVR_Cur_Page;
				PVR_Draw_FileList_Item(hdc, u16PVR_Focus, UNFOCUS);

				if ( u16PVR_index <= 0 )
					u16PVR_index = stFileDB.u16file_Count - 1;
				else
					u16PVR_index--;

				u16PVR_Focus = get_focus_line(&u16PVR_Cur_Page, u16PVR_index, PVR_LIST_ITEM);

				if ( u16PVR_Pre_Page != u16PVR_Cur_Page )
					PVR_Draw_File_List(hdc);
				else
					PVR_Draw_FileList_Item(hdc, u16PVR_Focus, FOCUS);

				EndPaint(hwnd,hdc);
				break;

			case CSAPP_KEY_DOWN:
				hdc = BeginPaint(hwnd);
				u16PVR_Pre_Page = u16PVR_Cur_Page;
				PVR_Draw_FileList_Item(hdc, u16PVR_Focus, UNFOCUS);

				if ( u16PVR_index >= stFileDB.u16file_Count - 1 )
					u16PVR_index = 0;
				else
					u16PVR_index++;

				u16PVR_Focus = get_focus_line(&u16PVR_Cur_Page, u16PVR_index, PVR_LIST_ITEM);

				if ( u16PVR_Pre_Page != u16PVR_Cur_Page )
					PVR_Draw_File_List(hdc);
				else
					PVR_Draw_FileList_Item(hdc, u16PVR_Focus, FOCUS);

				EndPaint(hwnd,hdc);
				break;

			case CSAPP_KEY_LEFT:
				break;

			case CSAPP_KEY_RIGHT:
				break;

			case CSAPP_KEY_ENTER:
				if ( Get_TmpPVR_Rec_Data.PVR_Last_Posion != 0 )
				{
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_CONTINUE);
				} else {
					CSMPR_Player_Stop();
					memset ( TmpStr, 0x00, 256 );
					sprintf(TmpStr, "%s/%s", RECfile, stFileDB.acFileName[u16PVR_index]);
					if ( CSMPR_Player_Start(TmpStr, 0) == CSMPR_ERROR )
					{
						CSMPR_Player_Resume();
						
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_PLAY_ERROR);
						EndPaint(hwnd,hdc);

						usleep( 1000 * 1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
						break;
					} else {
						PVR_Clear_File_List_Window(hwnd);
						SendMessage(hwnd,MSG_CREATE,0,0);
					}
				}
				break;

			case CSAPP_KEY_MENU:
			case CSAPP_KEY_ESC:
				PVR_Clear_File_List_Window(hwnd);
				break;
					
			default:
				break;
		}
	}
}

CSAPP_Applet_t CSApp_PVR_Player(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_PVR_Player_Applets = CSApp_Applet_Error;

#ifdef  Screen_1080
	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = 1920;
	HEIGHT = 1080;
#else
	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif

	CreateInfo.dwStyle	 = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "pvr_player";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = PVR_Player_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = COLOR_transparent;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_PVR_Player_Applets;
}

static int PVR_Player_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{
	HDC 				hdc;
	struct timespec		time_value;

	switch(message)
	{
		case MSG_CREATE:
			PVR_Play_Banner.Draw = FALSE;
			PVR_Play_Banner.OnScreen = FALSE;
			PVR_Volume.Draw = FALSE;
			PVR_Volume.OnScreen = FALSE;
			PVR_Play_Pause.Draw = FALSE;
			PVR_Play_Pause.OnScreen = FALSE;
			b8Position_Move = FALSE;

			memset(&btpDR_Capture, 0x00, sizeof(BITMAP));
			memset(&btpTime_Capture, 0x00, sizeof(BITMAP));
			memset(GetFileName, 0x00, 256);
			memset(&Get_PVR_Rec_Data, 0x00, sizeof(st_pvr_data));
			CSMPR_Player_GetFileName(GetFileName);
			MV_PVR_CFG_File_Parser(&Get_PVR_Rec_Data, GetFileName);
			
			//CSMPR_Player_Get_JumpOffset(&DurationTime);

			StartTime = Get_PVR_Rec_Data.PVR_Start_OS;
			EndTime = Get_PVR_Rec_Data.PVR_End_OS;
			DurationTime = EndTime - StartTime;

			clock_gettime(CLOCK_REALTIME, &time_value);
			NowStartTime = time_value.tv_sec;

			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

			SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
			break;
			
		case MSG_PAINT:
			hdc=MV_BeginPaint(hwnd);

			PVR_Player_Draw_Icon(hdc);

			PVR_Play_Banner.Draw = TRUE;
			PVR_Player_PaintMute(hdc);
			PVR_Player_PaintBanner(hdc);
			MV_EndPaint(hwnd,hdc);
			return 0;

		case MSG_TIMER:
			if(wparam == DESKTOP_VOLUME_TIMER_ID)
			{
				PVR_Volume.Draw = FALSE;
				
				hdc=MV_BeginPaint(hwnd);
				PVR_Play_PaintVolume(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
					KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
			} 
			else if(wparam == DESKTOP_BANNER_TIMER_ID)
			{	
				PVR_Play_Banner.Draw = FALSE;
				
				hdc=MV_BeginPaint(hwnd);
				PVR_Player_PaintBanner(hdc);
				MV_EndPaint(hwnd,hdc);
				
				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
			}
			break;

		case MSG_PVR_PLAY_END:
			CSApp_PVR_Player_Applets=CSApp_Applet_Desktop;
			SendMessage(hwnd,MSG_CLOSE,0,0);
			break;

		case MSG_KEYDOWN:
			if ( b8List_Status == TRUE )
			{
				PVR_File_List_Proc(hwnd, wparam);
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_VOL_DOWN:
					{
						if ( PVR_Play_Banner.OnScreen == TRUE )
						{
							PVR_Play_Banner.Draw = FALSE;
				
							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintBanner(hdc);
							MV_EndPaint(hwnd,hdc);
							
							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}
						
						PVR_Play_VolumeScroll(FALSE);
						
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						PVR_Player_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;
					
				case CSAPP_KEY_VOL_UP:
					{
						if ( PVR_Play_Banner.OnScreen == TRUE )
						{
							PVR_Play_Banner.Draw = FALSE;
				
							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintBanner(hdc);
							MV_EndPaint(hwnd,hdc);
							
							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}
						
						PVR_Play_VolumeScroll(TRUE);
						
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						PVR_Player_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;
					
				case CSAPP_KEY_LEFT:
					if ( PVR_Volume.OnScreen == TRUE )
					{
						PVR_Volume.Draw = FALSE;
			
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
					}
					
					if(PVR_Play_Banner.OnScreen == FALSE)
					{
						PVR_Play_Banner.Draw = TRUE;
						
						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					
						hdc=MV_BeginPaint(hwnd);
						PVR_Player_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					
					if ( PVR_Play_Banner.OnScreen == TRUE )
					{
						b8Position_Move = TRUE;
						CSMPR_Player_SetPoint( MV_KEY_LEFT );

						hdc=MV_BeginPaint(hwnd);
						PVR_Play_Draw_Duration(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					} 
					break;
/*
				case CSAPP_KEY_VOL_DOWN:	
					{
						PVR_Play_VolumeScroll(FALSE);
						
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						PVR_Player_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;
*/					
				case CSAPP_KEY_RIGHT:
					if ( PVR_Volume.OnScreen == TRUE )
					{
						PVR_Volume.Draw = FALSE;
			
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
					}
					
					if(PVR_Play_Banner.OnScreen == FALSE)
					{
						PVR_Play_Banner.Draw = TRUE;
						
						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					
						hdc=MV_BeginPaint(hwnd);
						PVR_Player_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					
					if ( PVR_Play_Banner.OnScreen == TRUE )
					{
						b8Position_Move = TRUE;
						CSMPR_Player_SetPoint( MV_KEY_RIGHT );

						hdc=MV_BeginPaint(hwnd);
						PVR_Play_Draw_Duration(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					} 
					break;
/*
				case CSAPP_KEY_VOL_UP:
					{
						PVR_Play_VolumeScroll(TRUE);
						
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						PVR_Player_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd, DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;
*/
				case CSAPP_KEY_MUTE:
					if(CS_AV_Audio_GetMuteStatus() == FALSE)
						CS_AV_Audio_SetMuteStatus(TRUE);
					else
						CS_AV_Audio_SetMuteStatus(FALSE);
					
					CS_DBU_SaveMuteStatus();
					
					hdc=MV_BeginPaint(hwnd);
					PVR_Player_PaintMute(hdc);
					MV_EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

				case CSAPP_KEY_INFO:
					if ( PVR_Volume.OnScreen == TRUE )
					{
						PVR_Volume.Draw = FALSE;
			
						hdc=MV_BeginPaint(hwnd);
						PVR_Play_PaintVolume(hdc);
						MV_EndPaint(hwnd,hdc);
						
						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
					}
					
					if(PVR_Play_Banner.OnScreen == FALSE)
					{
						PVR_Play_Banner.Draw = TRUE;
						
						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					} else {
						PVR_Play_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
					}
					
					hdc=MV_BeginPaint(hwnd);
					PVR_Player_PaintBanner(hdc);
					MV_EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_REW:
					if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_STOP )
					{
						if ( CSMPR_Player_GetStatus() == CSMPR_PLAY_REW_16X )
							CSMPR_Player_Resume();
						else
							CSMPR_Player_Rewind();

						PVR_Play_Banner.Draw = TRUE;
						
						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
						
						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

						hdc=MV_BeginPaint(hwnd);
						PVR_Player_PaintBanner(hdc);
						PVR_Player_Draw_Icon(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_FF:
					if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_STOP )
					{
						if ( CSMPR_Player_GetStatus() == CSMPR_PLAY_FF_16X )
							CSMPR_Player_Resume();
						else
							CSMPR_Player_Forward();

						PVR_Play_Banner.Draw = TRUE;
						
						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);
						
						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

						hdc=MV_BeginPaint(hwnd);
						PVR_Player_PaintBanner(hdc);
						PVR_Player_Draw_Icon(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_PLAY:
					if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_RUN )
					{
						CSMPR_Player_Resume();
						if ( PVR_Play_Pause.Draw == TRUE )
						{
							PVR_Play_Pause.Draw = FALSE;

							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintPause(hdc);
							PVR_Player_Draw_Icon(hdc);
							MV_EndPaint(hwnd,hdc);
						} else {
							hdc=MV_BeginPaint(hwnd);
							PVR_Player_Draw_Icon(hdc);
							MV_EndPaint(hwnd,hdc);
						}
					} 
					else if ( CSMPR_Player_GetStatus() == CSMPR_PLAY_RUN )
					{
						if ( PVR_Play_Banner.OnScreen == TRUE )
						{
							PVR_Play_Banner.Draw = FALSE;
				
							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintBanner(hdc);
							PVR_Player_Draw_Icon(hdc);
							MV_EndPaint(hwnd,hdc);
							
							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}
						
						PVR_Draw_File_List_Window(hwnd);
					}
					break;
					
				case CSAPP_KEY_LIST:
					if ( CSMPR_Player_GetStatus() >= CSMPR_PLAY_RUN )
					{
						if ( PVR_Play_Banner.OnScreen == TRUE )
						{
							PVR_Play_Banner.Draw = FALSE;
				
							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintBanner(hdc);
							MV_EndPaint(hwnd,hdc);
							
							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}
						
						PVR_Draw_File_List_Window(hwnd);
					}
					break;

				case CSAPP_KEY_PAUSE:
					if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_STOP )
					{
						if( PVR_Play_Pause.Draw == FALSE )
						{
							PVR_Play_Pause.Draw = TRUE;
							CSMPR_Player_Pause();

							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintPause(hdc);
							PVR_Player_Draw_Icon(hdc);
							MV_EndPaint(hwnd,hdc);
						}
						else
						{
							PVR_Play_Pause.Draw = FALSE;
							CSMPR_Player_Resume();

							hdc=MV_BeginPaint(hwnd);
							PVR_Player_PaintPause(hdc);
							PVR_Player_Draw_Icon(hdc);
							MV_EndPaint(hwnd,hdc);
						}
					}
					break;
					
				case CSAPP_KEY_STOP:
					if ( CSMPR_Player_GetStatus() > CSMPR_PLAY_STOP )
					{
						MV_stServiceInfo 			ServiceData_Temp;
						tCS_DB_ServiceManageData 	service_index;

						CS_DB_GetCurrentList_ServiceData( &service_index, CS_DB_GetCurrentService_OrderIndex());
						MV_DB_GetServiceDataByIndex( &ServiceData_Temp, service_index.Service_Index );

						CSMPR_Player_Stop();
						CS_MW_PlayServiceByIdx(ServiceData_Temp.u16ChIndex, RE_TUNNING);						
					
						CSApp_PVR_Player_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;

				case CSAPP_KEY_ENTER:
					if ( PVR_Play_Banner.OnScreen == TRUE && b8Position_Move == TRUE )
					{
						b8Position_Move = FALSE;
						CSMPR_Player_Set_Position();

						hdc=MV_BeginPaint(hwnd);
						PVR_Play_Draw_Duration(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd, DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);						
					} else {
						PVR_Play_Draw_NowPlaying(hwnd);
					}
					break;
					
				case CSAPP_KEY_ESC:
					if(PVR_Play_Banner.OnScreen == TRUE)
					{
						PVR_Play_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
					
						hdc=MV_BeginPaint(hwnd);
						PVR_Player_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					} else {
						PVR_Play_Draw_NowPlaying(hwnd);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_PVR_Player_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
						
				default:
					PVR_Play_Draw_NowPlaying(hwnd);
					break;
			}

			break;
		case MSG_CLOSE:
			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
				KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

			if ( btpDR_Capture.bmHeight != 0 )
			{
				UnloadBitmap (&btpDR_Capture);
				UnloadBitmap (&btpTime_Capture);
			}
			
			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;
		default:
			break;
	}

	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


