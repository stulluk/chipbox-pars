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
#include "demux.h" /* By KB Kim : 2011.06.13 */



#define		SUBTITLE_OPEN_NONE	0 /* By KB Kim : 2011.06.13 */

#define		Banner_timer_Max			( CS_DBU_GetBannerKeepTime()*1000 )
#define		Pvr_Banner_timer_Max		( 10 * 1000 )
#define		Volume_timer_Max			( 2*1000 )
#define		pvr_timer_Max				1000
#define		ONE_HOUR					3600
#define		ONE_MINITE					60
#define		HALF_HOUR					1800
#define		TWO_HOUR					( ONE_HOUR * 2 )

#define 	MUTE_ICON_X					1080
#define		MUTE_ICON_Y					100
#define 	PVR_ICON_X					MUTE_ICON_X
#define		PVR_ICON_Y					( MUTE_ICON_Y - MV_BMP[MVBMP_PVR_REC].bmHeight - 10 )

#define 	VOLUME_X					390
#define		VOLUME_Y					560
#define		VOLUME_Y2					650 /* By KB Kim : 2011.06.13 */
#define		VOLUME_DX					500
#define		VOLUME_DY					40
#define		VOLUME_TEXT_X				890
#define		VOLUME_TEXT_DX				50
#define		VOLUME_FULL_DX 				( MV_BMP[MVBMP_VOLUME_ICON].bmWidth + VOLUME_DX + VOLUME_TEXT_DX )

/**************************** Info Banner *****************************************/

#define 	BANNER_STAR_X				100
#define		BANNER_STAR_Y				572
#define 	BANNER_STAR_W				1080
#define		BANNER_STAR_H				148
#define		BANNER_CLEAR_H				(BANNER_STAR_H + (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2))
#define		BANNER_CIRCLE_X				(BANNER_STAR_X + (BANNER_STAR_W/2) - (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth/2))
#define		BANNER_CIRCLE_Y				(BANNER_STAR_Y - (MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2))
#define		BANNER_ITEM_X				122
#define		BANNER_ITEM_Y1				572
#define		BANNER_ITEM_Y2				602
#define		BANNER_ITEM_Y3				640
#define		BANNER_ITEM_H				28
#define		BANNER_NAME_X				BANNER_ITEM_X
#define 	BANNER_NAME_DX				240
#define		BANNER_ICON_X				(BANNER_ITEM_X + BANNER_NAME_DX)
#define		BANNER_ICON_DX				30
#define		BANNER_TIME_X 				( BANNER_ITEM_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 100 )

#define 	BANNER_EPG_X				(BANNER_CIRCLE_X + MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth + 10)

/**********************************************************************************/
/**************************** Record Info Banner **********************************/

#define 	RECORD_STAR_X				100
#define		RECORD_STAR_Y				BANNER_CIRCLE_Y
#define 	RECORD_STAR_W				1080
#define		RECORD_STAR_H				200
#define		RECORD_TIME_Y				( RECORD_STAR_Y + 56 )
#define		RECORD_TIME_DY				( RECORD_STAR_Y - RECORD_TIME_Y )
#define		RECORD_ITEM_X				100
#define		RECORD_ITEM_Y1				( RECORD_STAR_Y + 56 )
#define		RECORD_ITEM_Y2				( RECORD_ITEM_Y1 + 30 )
#define		RECORD_ITEM_Y3				( RECORD_ITEM_Y2 + 34 )
#define		RECORD_ITEM_H				28
#define		RECORD_NAME_X				( RECORD_ITEM_X + MV_BMP[MVBMP_PVR_RECORD].bmWidth + 40 )
#define		RECORD_NAME_Y				( RECORD_STAR_Y + 56 )
#define 	RECORD_NAME_DX				240
#define		RECORD_DATE_X				820
#define		RECORD_DATE_Y				RECORD_NAME_Y
#define		RECORD_TIME_X				( RECORD_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 100 )
#define		RECORD_STORAGE_X			( RECORD_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 100 )
#define 	RECORD_STORAGE_Y			( RECORD_NAME_Y + 8 )
#define		RECORD_PROGRESS_X			250
#define 	RECORD_PROGRESS_DX			470

/**********************************************************************************/
/**********************************************************************************/

static char 							CSApp_PVR_filename[128];
static CSAPP_Applet_t					CSApp_PVR_Record_Applets;
static CSAPP_Applet_t					CSApp_PVR_Streaming_Applets;
static tCSDesktopVolume 				PVR_Volume;
static stPvrBanner						PVR_Banner;
static stPvrBanner						PVR_Rec_Banner;
static tCS_DB_Error						DBError;
static tCS_DB_ServiceManageData 		service_index;
static MV_stServiceInfo 				ServiceData;
static tCS_EIT_Event_t					present;
static tCS_EIT_Event_t					follow;
static time_t							StartTime;
static time_t							DurationTime;
static time_t							NowTime;
static U8								u8Hour_value;
static U8								u8Min_value;
static char								acHour[3];
static char								acMin[3];
static U8								u8InputCount = 0;
static char								acTemp_duration[10];

static struct f_size					stfile_size;
static BITMAP							btTime_Capture;

static int PVR_Record_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);
/************v38.........**************************/
static int PVR_Streaming_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);
/***************************************************/

extern U16 Get_Width_Number(char* text);
extern void Draw_Number_Icon(HDC hdc, int x, int y, char* text);

void Duration_to_Time(void)
{
	u8Hour_value = DurationTime/3600;
	u8Min_value = ( DurationTime%3600 ) / 60;

	sprintf(acHour, "%02d", u8Hour_value);
	sprintf(acMin, "%02d", u8Min_value);

	sprintf(acTemp_duration, "%02d%02d", u8Hour_value, u8Min_value);
}

void PVR_Update_Duration(WPARAM wparam)
{
	char	Temp;

	switch(wparam)
	{
		case CSAPP_KEY_0:
			Temp = '0';
			break;

		case CSAPP_KEY_1:
			Temp = '1';
			break;

		case CSAPP_KEY_2:
			Temp = '2';
			break;

		case CSAPP_KEY_3:
			Temp = '3';
			break;

		case CSAPP_KEY_4:
			Temp = '4';
			break;

		case CSAPP_KEY_5:
			Temp = '5';
			break;

		case CSAPP_KEY_6:
			Temp = '6';
			break;

		case CSAPP_KEY_7:
			Temp = '7';
			break;

		case CSAPP_KEY_8:
			Temp = '8';
			break;

		case CSAPP_KEY_9:
			Temp = '9';
			break;

		default:
			Temp = '0';
			break;
	}

	switch (u8InputCount)
	{
		case 0:
			acHour[0] = Temp;
			break;

		case 1:
			acHour[1] = Temp;
			break;

		case 2:
			acMin[0] = Temp;
			break;

		case 3:
			acMin[1] = Temp;
			break;

		default:
			break;
	}

	acTemp_duration[u8InputCount] = Temp;
}

void PVR_Duration_Time(HDC hdc)
{
	//PVR_Rec_Banner.OnScreen = TRUE;
	RECT		acRect;
	char		acTemp_Time[20];
	char		acTemp_Str[20];
	int			i;

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	acRect.top = RECORD_STAR_Y + 6;
	acRect.left = WARNING_LEFT + (WARNING_DX - 200)/2;
	acRect.bottom = acRect.top + 30;
	acRect.right = acRect.left + 200;

	memset( acTemp_Time, 0x00, 20 );
	memset( acTemp_Str, 0x00, 20 );

	sprintf(acTemp_Time, "%s%s", acHour, acMin);
	FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));

	acRect.top += 4;
	acRect.left = WARNING_LEFT + (WARNING_DX/2) - 80;
	acRect.bottom = acRect.top + 22;
	acRect.right = acRect.left + 25;

	for ( i = 0 ; i < 4 ; i++ )
	{
		if ( i == 2 )
			acRect.left += 35;
		else
			acRect.left += 25;

		acRect.right = acRect.left + 25;
		sprintf(acTemp_Str, "%c", acTemp_Time[i]);

		if ( i == u8InputCount )
		{
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
		else
		{
			SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
	}
//	CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
}

void PVR_Draw_Duration_Time(HDC hdc)
{
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(RECORD_STAR_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(RECORD_STAR_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(RECORD_STAR_Y + 44 - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(RECORD_STAR_Y + 44 - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(RECORD_STAR_Y),ScalerWidthPixel(WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(44));
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(RECORD_STAR_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_DX),ScalerHeigthPixel(44 - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Duration_to_Time();
	PVR_Duration_Time(hdc);
}

void PVR_Draw_Duration(HDC hdc)
{
	RECT				Usage_Rect;
	U16 				LevelValue = 0;
	char				acStart[16];
	struct tm			tm_time;
	char				acEnd[16];
	char				acNow[16];
	char				Temp_Str[30];
	long				TempTime;
	struct timespec		time_value;

	memset( acStart, 0x00, 16);
	memset( acEnd, 0x00, 16);
	memset( acNow, 0x00, 16);

	Usage_Rect.top = RECORD_ITEM_Y3;
	Usage_Rect.bottom = Usage_Rect.top + MV_INSTALL_MENU_HEIGHT;
	Usage_Rect.left = RECORD_PROGRESS_X;
	Usage_Rect.right = Usage_Rect.left + RECORD_PROGRESS_DX;

	clock_gettime(CLOCK_REALTIME, &time_value);
	NowTime = time_value.tv_sec;

	LevelValue = (U16)( (( NowTime - StartTime) * 100 ) / DurationTime );

	memcpy(&tm_time, localtime(&StartTime), sizeof(tm_time));
	sprintf(acStart, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);

	TempTime = StartTime + DurationTime;
	memcpy(&tm_time, localtime(&TempTime), sizeof(tm_time));
	sprintf(acEnd, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);

	TempTime = NowTime - StartTime;
	memcpy(&tm_time, localtime(&TempTime), sizeof(tm_time));
	sprintf(acNow, "%02d:%02d", tm_time.tm_hour, tm_time.tm_min);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_Draw_Time_Progress_Bar(hdc, &Usage_Rect, LevelValue, 0, acStart, acEnd, ( NowTime - StartTime ) );

	memcpy(&tm_time, localtime(&DurationTime), sizeof(tm_time));
	sprintf(Temp_Str, "%s / %02d:%02d", acNow, tm_time.tm_hour, tm_time.tm_min);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(RECORD_DATE_X + 50),ScalerHeigthPixel(RECORD_ITEM_Y3), Temp_Str);
}

static void PVR_Volume_Setting_With_Offset(void)
{
	if ( PVR_Volume.Volume != 0 )
	{
		if (ServiceData.u8AudioVolume > 32 )
		{
			if ( PVR_Volume.Volume + ( ServiceData.u8AudioVolume - 32 ) > 50 )
				CS_AV_AudioSetVolume(kCS_DBU_MAX_VOLUME);
			else
				CS_AV_AudioSetVolume(PVR_Volume.Volume + ( ServiceData.u8AudioVolume - 32 ) );
		}
		else if ( ServiceData.u8AudioVolume < 32 )
		{
			if ( PVR_Volume.Volume - ( 32 - ServiceData.u8AudioVolume ) < 0 )
				CS_AV_AudioSetVolume(0);
			else
				CS_AV_AudioSetVolume(PVR_Volume.Volume - ( 32 - ServiceData.u8AudioVolume ));
		}
		else
			CS_AV_AudioSetVolume(PVR_Volume.Volume);
	}
}

static void PVR_VolumeScroll(BOOL right)
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

	PVR_Volume_Setting_With_Offset();

	CS_DBU_SaveVolume();
	CS_DBU_SaveMuteStatus();

}

static void PVR_PaintVolume(HDC hdc)
{
	int      	num;
	char		acText[5];

	/* By KB Kim : 2011.06.13 */
	if ( CS_MW_Get_Subtitle_Status() == SUBTITLE_OPEN_NONE )
	{
		// printf("PVR_PaintVolume : SubTitle OFF !\n");
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
	else
	{
		// printf("PVR_PaintVolume : SubTitle ON !\n");
		if( PVR_Volume.Draw != TRUE && PVR_Volume.OnScreen == TRUE )
		{
			PVR_Volume.OnScreen=FALSE;
			SetBrushColor(hdc, COLOR_transparent);
			FillBox(hdc,ScalerWidthPixel(VOLUME_X - MV_BMP[MVBMP_VOLUME_ICON].bmWidth),ScalerHeigthPixel(VOLUME_Y2),ScalerWidthPixel(VOLUME_FULL_DX),ScalerHeigthPixel(VOLUME_DY));
			return;
		}

		PVR_Volume.OnScreen = TRUE;
		num = PVR_Volume.Volume;

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

static void PVR_Record_PaintMute(HDC hdc)
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

static void PVR_Draw_Time(HDC hdc, BOOL b8Kind)
{
	U16 			local_utc;
	U16 			local_mjd;
	tCS_DT_Time     local_time;
	tCS_DT_Date		local_date;
	tCS_DT_Date  	Current_Date;
	tCS_DT_Time		Current_Time;
	char 			acDate[100];
	char 			acTime[100];

	if ( b8Kind == TRUE ) /*  Normal Information Banner */
	{
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

		if ( btTime_Capture.bmHeight == 0 )
			MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2), ScalerWidthPixel(BANNER_NAME_DX+ 120), ScalerHeigthPixel(BANNER_ITEM_H), &btTime_Capture);
		else
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2), ScalerWidthPixel(BANNER_NAME_DX+ 120), ScalerHeigthPixel(BANNER_ITEM_H), &btTime_Capture);

		FillBoxWithBitmap(hdc, ScalerWidthPixel(BANNER_ITEM_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4),ScalerWidthPixel(MV_BMP[MVBMP_PVR_DATE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_DATE]);
		FillBoxWithBitmap(hdc, ScalerWidthPixel(BANNER_TIME_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4),ScalerWidthPixel(MV_BMP[MVBMP_PVR_TIME].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_TIME].bmHeight), &MV_BMP[MVBMP_PVR_TIME]);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);

		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_ITEM_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), acDate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_TIME_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), acTime);
	} else {  /* Recording Information Banner */
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

		if ( MV_BMP[MVBMP_PVR_TIME_CAP].bmHeight == 0 )
			MV_GetBitmapFromDC (hdc, ScalerWidthPixel(RECORD_DATE_X), ScalerHeigthPixel(RECORD_DATE_Y), ScalerWidthPixel(RECORD_STORAGE_X - RECORD_DATE_X), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_TIME_CAP]);
		else
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(RECORD_DATE_X), ScalerHeigthPixel(RECORD_DATE_Y), ScalerWidthPixel(RECORD_STORAGE_X - RECORD_DATE_X), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_TIME_CAP]);

		FillBoxWithBitmap(hdc, ScalerWidthPixel(RECORD_DATE_X), ScalerHeigthPixel(RECORD_DATE_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_DATE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_DATE].bmHeight), &MV_BMP[MVBMP_PVR_DATE]);
		FillBoxWithBitmap(hdc, ScalerWidthPixel(RECORD_TIME_X), ScalerHeigthPixel(RECORD_DATE_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_TIME].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_TIME].bmHeight), &MV_BMP[MVBMP_PVR_TIME]);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);

		CS_MW_TextOut(hdc,ScalerWidthPixel(RECORD_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 6),ScalerHeigthPixel(RECORD_DATE_Y), acDate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(RECORD_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 6),ScalerHeigthPixel(RECORD_DATE_Y), acTime);

/*    Check between System Tiem and TDT time (MVAPP Time) */
#if 0
		MV_OS_Get_Time_Offset_Splite(acDate, acTime, TRUE);

		CS_MW_TextOut(hdc,ScalerWidthPixel(RECORD_DATE_X + MV_BMP[MVBMP_PVR_DATE].bmWidth + 10),ScalerHeigthPixel(RECORD_DATE_Y + 36), acDate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(RECORD_TIME_X + MV_BMP[MVBMP_PVR_TIME].bmWidth + 10),ScalerHeigthPixel(RECORD_DATE_Y + 36), acTime);
#endif
/**********************************************************/
	}
}

static void PVR_Draw_EPG(HDC hdc)
{
	U16						Utc;
	U8						StarHour,StarMinute,EndHour,EndMinute;
	char					Text[45];
	tCS_EIT_Event_t			present;
	tCS_EIT_Event_t			follow;

	CS_EIT_Get_PF_Event(ServiceData.u16TransponderIndex , ServiceData.u16ServiceId, &present, &follow);

	memset(Text, 0x00, 45);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Info_top_Color.MV_R, CFG_Info_top_Color.MV_G, CFG_Info_top_Color.MV_B, 0xFF));

	if ( MV_BMP[MVBMP_INFO_EPG1_CAP].bmHeight == 0 )
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG1_CAP]);
	else
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG1_CAP]);

	if( present.start_date_mjd != 0 || follow.start_date_mjd != 0 )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*6),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_EPG_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*6),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_EPG_UNFO_ICON]);

	if(present.start_date_mjd != 0)
	{
		Utc = present.start_time_utc;
		StarHour = Utc>>8;
		StarMinute = (Utc&0x00FF);
		Utc = present.end_time_utc;
		EndHour = Utc>>8;
		EndMinute = (Utc&0x00FF);
		snprintf(Text, 20, "%.2x:%.2x-%.2x:%.2x", StarHour, StarMinute, EndHour, EndMinute);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);
		strncpy(Text, present.event_name, 20);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+180), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);
	}
	else
	{
		sprintf(Text, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_EPG_CURRENT), CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y1 + 4), Text);
	}

	memset(Text, 0x00, 45);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Info_top_Color.MV_R, CFG_Info_top_Color.MV_G, CFG_Info_top_Color.MV_B, 0xFF));

	if ( MV_BMP[MVBMP_INFO_EPG2_CAP].bmHeight == 0 )
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y2),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG2_CAP]);
	else
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_EPG_X),ScalerHeigthPixel(BANNER_ITEM_Y2),ScalerWidthPixel( ( BANNER_STAR_X + BANNER_STAR_W ) - BANNER_EPG_X - 10 ), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_EPG2_CAP]);

	if(follow.start_date_mjd != 0)
	{
		Utc = follow.start_time_utc;
		StarHour = Utc>>8;
		StarMinute = (Utc&0x00FF);
		Utc = follow.end_time_utc;
		EndHour = Utc>>8;
		EndMinute = (Utc&0x00FF);
		snprintf(Text, 20, "%.2x:%.2x-%.2x:%.2x", StarHour, StarMinute, EndHour, EndMinute);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), Text);
		strncpy(Text, follow.event_name, 20);
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X+180), ScalerHeigthPixel(BANNER_ITEM_Y2 + 6), Text);
	}
	else
	{
		sprintf(Text, "%s : %s", CS_MW_LoadStringByIdx(CSAPP_STR_EPG_NEXT), CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
		CS_MW_TextOut(hdc,ScalerWidthPixel(BANNER_EPG_X), ScalerHeigthPixel(BANNER_ITEM_Y2 + 4), Text);
	}
}

static void PVR_PaintBanner(HDC hdc)
{
	RECT			rc;
	char    		Text[30];
	MV_stSatInfo 	Temp_SatInfo;
	MV_stTPInfo 	Temp_TPInfo;
	tMWStream		SUBTTXstream;
	/* By KB Kim 2011.01.20 */
	U8              tvRadio;

	memset(&btTime_Capture, 0x00, sizeof(BITMAP));

	if(PVR_Banner.Draw == TRUE)
	{
		PVR_Volume.Draw = FALSE;
	}

	if( PVR_Banner.Draw != TRUE && PVR_Banner.OnScreen == TRUE )
	{
		PVR_Banner.OnScreen = FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(BANNER_CLEAR_H));
		UnloadBitmap (&btTime_Capture);
		return;
	}

	if( PVR_Banner.Draw != TRUE )
	{
		return;
	}

	PVR_Banner.OnScreen=TRUE;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(0), ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(MV_BMP[MVBMP_INFOBAR].bmHeight), &MV_BMP[MVBMP_INFOBAR]);

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);

	MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, ServiceData.u16ChIndex);

	MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, ServiceData.u16ChIndex);
	if ( Temp_TPInfo.u8Polar_H == P_H)
		sprintf(Text, "%s   %d/%s/%d", Temp_SatInfo.acSatelliteName, Temp_TPInfo.u16TPFrequency, "H", Temp_TPInfo.u16SymbolRate);
	else
		sprintf(Text, "%s   %d/%s/%d", Temp_SatInfo.acSatelliteName, Temp_TPInfo.u16TPFrequency, "V", Temp_TPInfo.u16SymbolRate);

	rc.left = ScalerWidthPixel(BANNER_NAME_X);
	rc.top = ScalerHeigthPixel(BANNER_ITEM_Y3);
	rc.right = ScalerWidthPixel((rc.left + BANNER_NAME_DX * 2));
	rc.bottom = ScalerHeigthPixel(rc.top + BANNER_ITEM_H);

	if ( MV_BMP[MVBMP_INFO_SAT_INFO_CAP].bmHeight == 0 )
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_SAT_INFO_CAP]);
	else
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(rc.left), ScalerHeigthPixel(rc.top), ScalerWidthPixel(rc.right - rc.left), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_SAT_INFO_CAP]);


	CS_MW_DrawText(hdc, Text, -1, &rc, DT_LEFT);

	sprintf(Text, "%04d", CS_DB_GetCurrentService_OrderIndex() + 1);

	rc.left = ScalerWidthPixel(BANNER_CIRCLE_X + ( MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth - Get_Width_Number(Text) )/2);
	rc.top = ScalerHeigthPixel(BANNER_CIRCLE_Y + ((MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight/2) - (MV_BMP[MVBMP_0_ICON].bmHeight/2)));
	rc.right = ScalerWidthPixel(BANNER_CIRCLE_X + MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth);
	rc.bottom = ScalerHeigthPixel(rc.top + BANNER_ITEM_H);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_CIRCLE_X),ScalerHeigthPixel(BANNER_CIRCLE_Y),ScalerWidthPixel(MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_INFO_BANNER_CIRCLE].bmHeight), &MV_BMP[MVBMP_INFO_BANNER_CIRCLE]);

	Draw_Number_Icon(hdc, rc.left, rc.top, Text);

	if ( MV_BMP[MVBMP_INFO_CHNAME_CAP].bmHeight == 0 )
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(BANNER_NAME_X), ScalerHeigthPixel(BANNER_ITEM_Y1), ScalerWidthPixel(BANNER_NAME_DX), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_CHNAME_CAP]);
	else
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(BANNER_NAME_X), ScalerHeigthPixel(BANNER_ITEM_Y1), ScalerWidthPixel(BANNER_NAME_DX), ScalerHeigthPixel(BANNER_ITEM_H), &MV_BMP[MVBMP_INFO_CHNAME_CAP]);

	rc.left = ScalerWidthPixel(BANNER_NAME_X);
	rc.top = ScalerHeigthPixel(BANNER_ITEM_Y1 + 6);
	rc.right = ScalerWidthPixel(BANNER_NAME_X + BANNER_NAME_DX);
	rc.bottom = ScalerHeigthPixel(BANNER_ITEM_Y1 + BANNER_ITEM_H);

	sprintf(Text, "%s", ServiceData.acServiceName);
	CS_MW_DrawText(hdc, Text, -1, &rc, DT_LEFT);

	if(ServiceData.u8AC3Flag == TRUE)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_DOLBY_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_DOLBY_UNFO_ICON]);

	/* By KB Kim 2011.01.20 */
	if(ServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*1),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_HD_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*1),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_HD_UNFO_ICON]);

	if(ServiceData.u8Scramble == TRUE)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*2),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SCRAM_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*2),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SCRAM_UNFO_ICON]);

	CS_MW_GetTeletextStream(&SUBTTXstream);

	if(SUBTTXstream.Number == 0)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*3),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_TTX_UNFO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*3),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_TTX_FO_ICON]);

	CS_MW_GetSubtitleStream(&SUBTTXstream);

	if(SUBTTXstream.Number==0)
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_UNFO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*4),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_SUBT_FO_ICON]);

	/* By KB Kim 2011.01.20 */
	if (ServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE)
	{
		tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
	}
	else
	{
		tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	}
	if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, ServiceData.u16ChIndex) < MV_MAX_FAV_KIND )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*5),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_FAV_FO_ICON]);
	else
		FillBoxWithBitmap(hdc,ScalerWidthPixel(BANNER_ICON_X + BANNER_ICON_DX*5),ScalerHeigthPixel(BANNER_ITEM_Y1),ScalerWidthPixel(BANNER_ICON_DX-2), ScalerHeigthPixel(28), &MV_BMP[MVBMP_INFO_FAV_UNFO_ICON]);

	PVR_Draw_Time(hdc, TRUE);
	PVR_Draw_EPG(hdc);
}

void PVR_Draw_HDD_Avail(HDC hdc)
{
	U16		Level_Value = 0;
	U16		loca_Y = RECORD_STORAGE_Y;
	char	Text[10];

	MV_Get_USB_Info(0, &stfile_size);
	Level_Value = (U16)(( stfile_size.blocks - stfile_size.avail ) / ( stfile_size.blocks / 100 ));

	if( Level_Value < 90 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_WHITE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_WHITE].bmHeight), &MV_BMP[MVBMP_HDD_WHITE]);
	} else if ( Level_Value < 100 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_BLUE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_BLUE].bmHeight), &MV_BMP[MVBMP_HDD_BLUE]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_RED].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_RED].bmHeight), &MV_BMP[MVBMP_HDD_RED]);
	}

	loca_Y += MV_BMP[MVBMP_HDD_WHITE].bmHeight + 2;

	if( Level_Value < 70 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_WHITE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_WHITE].bmHeight), &MV_BMP[MVBMP_HDD_WHITE]);
	} else if ( Level_Value < 90 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_BLUE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_BLUE].bmHeight), &MV_BMP[MVBMP_HDD_BLUE]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_RED].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_RED].bmHeight), &MV_BMP[MVBMP_HDD_RED]);
	}

	loca_Y += MV_BMP[MVBMP_HDD_WHITE].bmHeight + 2;

	if( Level_Value < 50 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_WHITE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_WHITE].bmHeight), &MV_BMP[MVBMP_HDD_WHITE]);
	} else if ( Level_Value < 70 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_BLUE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_BLUE].bmHeight), &MV_BMP[MVBMP_HDD_BLUE]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_RED].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_RED].bmHeight), &MV_BMP[MVBMP_HDD_RED]);
	}

	loca_Y += MV_BMP[MVBMP_HDD_WHITE].bmHeight + 2;

	if( Level_Value < 30 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_WHITE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_WHITE].bmHeight), &MV_BMP[MVBMP_HDD_WHITE]);
	} else if ( Level_Value < 50 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_BLUE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_BLUE].bmHeight), &MV_BMP[MVBMP_HDD_BLUE]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_RED].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_RED].bmHeight), &MV_BMP[MVBMP_HDD_RED]);
	}

	loca_Y += MV_BMP[MVBMP_HDD_WHITE].bmHeight + 2;

	if( Level_Value < 10 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_WHITE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_WHITE].bmHeight), &MV_BMP[MVBMP_HDD_WHITE]);
	} else if ( Level_Value < 30 ) {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_BLUE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_BLUE].bmHeight), &MV_BMP[MVBMP_HDD_BLUE]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(RECORD_STORAGE_X),ScalerHeigthPixel(loca_Y),ScalerWidthPixel(MV_BMP[MVBMP_HDD_RED].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_HDD_RED].bmHeight), &MV_BMP[MVBMP_HDD_RED]);
	}

	loca_Y += MV_BMP[MVBMP_HDD_WHITE].bmHeight + 6;
	sprintf(Text, "%d%%", Level_Value);
	CS_MW_TextOut(hdc,ScalerWidthPixel(RECORD_STORAGE_X), ScalerHeigthPixel(loca_Y), Text);
}

static void PVR_Record_PaintBanner(HDC hdc)
{
	RECT			rc;
	char    		Text[256];

	if(PVR_Rec_Banner.Draw == TRUE)
	{
		PVR_Volume.Draw = FALSE;
	}

	if( PVR_Rec_Banner.Draw != TRUE && PVR_Rec_Banner.OnScreen == TRUE )
	{
		PVR_Rec_Banner.OnScreen = FALSE;
		SetBrushColor(hdc, COLOR_transparent);
		FillBox(hdc,ScalerWidthPixel(0),ScalerHeigthPixel(RECORD_STAR_Y),ScalerWidthPixel(1280), ScalerHeigthPixel(RECORD_STAR_H));
		return;
	}

	if( PVR_Rec_Banner.Draw != TRUE )
	{
		return;
	}

	PVR_Rec_Banner.OnScreen=TRUE;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(0), ScalerHeigthPixel(RECORD_STAR_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_INFOBAR].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_INFOBAR].bmHeight), &MV_BMP[MVBMP_PVR_INFOBAR]);

	FillBoxWithBitmap(hdc, ScalerWidthPixel(RECORD_ITEM_X), ScalerHeigthPixel(RECORD_STAR_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_RECORD].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_RECORD].bmHeight), &MV_BMP[MVBMP_PVR_RECORD]);

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);

	rc.left = ScalerWidthPixel(RECORD_NAME_X);
	rc.top = ScalerHeigthPixel(RECORD_NAME_Y + 6);
	rc.right = ScalerWidthPixel(RECORD_NAME_X + RECORD_NAME_DX + 300);
	rc.bottom = ScalerHeigthPixel(RECORD_NAME_Y + RECORD_ITEM_H);

	if(present.start_date_mjd != 0)
		sprintf(Text, "%s : %s", ServiceData.acServiceName, present.event_name);
	else
		sprintf(Text, "%s : %s", ServiceData.acServiceName, CS_MW_LoadStringByIdx(CSAPP_STR_NONE));

	CS_MW_DrawText(hdc, Text, -1, &rc, DT_LEFT);

	PVR_Draw_Time(hdc, FALSE);
	PVR_Draw_Duration(hdc);
	PVR_Draw_Duration_Time(hdc);
	PVR_Draw_HDD_Avail(hdc);
}

void PVR_Set_Duration_Time(U16 Duration_UTC)
{
	tCS_DT_Time			tTemp_HM;

	if ( Duration_UTC > 0 && Duration_UTC != 0xFFFF )
	{
		tTemp_HM = CS_DT_UTCtoHM(Duration_UTC);
		DurationTime = (tTemp_HM.hour * 3600) + (tTemp_HM.minute * 60);
		printf("==== DURATION : %ld ======\n", DurationTime);
	}
}

CSAPP_Applet_t CSApp_PVR_Record(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_PVR_Record_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption = "pvr_record";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = PVR_Record_Msg_cb;
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

	return CSApp_PVR_Record_Applets;
}

static int PVR_Record_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{
    //printf("PVR_Record_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)");

	HDC 				hdc;
	struct timespec		time_value;
	U8 					SubtitleNumber = 0;

	switch(message)
	{
		case MSG_CREATE:
			PVR_Rec_Banner.Draw = FALSE;
			PVR_Rec_Banner.OnScreen = FALSE;
			PVR_Banner.Draw = FALSE;
			PVR_Banner.OnScreen = FALSE;
			PVR_Volume.Draw = FALSE;
			PVR_Volume.OnScreen = FALSE;
			u8InputCount = 0;

#if 0 /* By KB Kim : 2011.06.13 */
			if (CS_MW_GetSubtitlePid() >= /*kDB_DEMUX_INVAILD_PID*/0x1FFF)
			{
				SubtitleNumber = MvGetCurrentSubtitle();
				// printf("PVR_Record_Msg_cb : Start Subtitle %d/%d\n", SubtitleNumber, MvGetTotalSubtitleNumber());
				if (SubtitleNumber < MvGetTotalSubtitleNumber())
				{
					CS_MW_OpenSubtitle();
				}
			}
#endif

			DBError = CS_DB_GetCurrentList_ServiceData( &service_index, CS_DB_GetCurrentService_OrderIndex());

			if( DBError == eCS_DB_OK )
			{
				DBError = MV_DB_GetServiceDataByIndex( &ServiceData, service_index.Service_Index );
				CS_EIT_Get_PF_Event(ServiceData.u16TransponderIndex , ServiceData.u16ServiceId, &present, &follow);
			}

			if ( CSMPR_Record_Start() != 0 || DBError != eCS_DB_OK )
			{
				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_REC_ERROR);
				EndPaint(hwnd,hdc);

				usleep( 2000*1000 );

				hdc=BeginPaint(hwnd);
				Close_Msg_Window(hdc);
				EndPaint(hwnd,hdc);

				CSApp_PVR_Record_Applets=CSApp_Applet_Desktop;
				SendMessage(hwnd,MSG_CLOSE,0,0);
				break;
			}

			if ( DurationTime == 0 )
				DurationTime = TWO_HOUR;

			clock_gettime(CLOCK_REALTIME, &time_value);
			StartTime = time_value.tv_sec;

			CSMPR_Record_GetFileName(CSApp_PVR_filename);
			MV_PVR_FileWrite_Time(CSApp_PVR_filename, present.event_name, service_index.Service_Index, CS_PVR_REC);
			//printf("\nFile Name : %s ===== \n", CSApp_PVR_filename);

			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

			if(IsTimerInstalled(hwnd, PVR_CHECK_TIMER_ID))
				KillTimer(hwnd,PVR_CHECK_TIMER_ID);

			SetTimer(hwnd, PVR_CHECK_TIMER_ID, pvr_timer_Max);

			break;

		case MSG_PAINT:
			hdc=MV_BeginPaint(hwnd);
			if ( CS_MW_GetCurrentMenuLanguage() == CS_APP_TURCK )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_REC_TR]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_REC]);
			PVR_Banner.Draw = TRUE;
			PVR_PaintBanner(hdc);
			PVR_Record_PaintMute(hdc);
			MV_EndPaint(hwnd,hdc);
			return 0;

		case MSG_TIMER:
			if(wparam == PVR_CHECK_TIMER_ID )
			{
				clock_gettime(CLOCK_REALTIME, &time_value);
				NowTime = time_value.tv_sec;

				if ( NowTime > StartTime + DurationTime )
				{
					if ( CSMPR_Record_GetStatus() > CSMPR_REC_IDLE )
					{
						tCS_DB_ServiceManageData 	service_index_Temp;

						CS_DB_GetCurrentList_ServiceData( &service_index_Temp, CS_DB_GetCurrentService_OrderIndex());

						if ( CSMPR_Record_GetStatus() == CSMPR_REC_RUN )
						{
							MV_PVR_FileWrite_Time(CSApp_PVR_filename, NULL, service_index_Temp.Service_Index, CS_PVR_STOP);
							CSMPR_Record_Stop();
						}
					}

					CSApp_PVR_Record_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
				} else if ( PVR_Banner.OnScreen == TRUE ) {
					hdc=MV_BeginPaint(hwnd);
					PVR_Draw_Time(hdc, TRUE);
					MV_EndPaint(hwnd,hdc);
				} else if ( PVR_Rec_Banner.OnScreen == TRUE ) {
					hdc=MV_BeginPaint(hwnd);
					PVR_Draw_Time(hdc, FALSE);
					MV_EndPaint(hwnd,hdc);
				}
			}
			else if(wparam == DESKTOP_BANNER_TIMER_ID)
			{
				PVR_Banner.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_PaintBanner(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
			}
			else if(wparam == DESKTOP_VOLUME_TIMER_ID)
			{
				PVR_Volume.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_PaintVolume(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
					KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
			}
			else if(wparam == PVR_REC_BANNER_TIMER_ID)
			{
				PVR_Rec_Banner.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_Record_PaintBanner(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
					KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);
			}

			/* By KB Kim : 2011.06.13 */
			if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
			{
				if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
				{
					SubtitleNumber = MvGetCurrentSubtitle();
					if (SubtitleNumber < MvGetTotalSubtitleNumber())
					{
						CS_MW_OpenSubtitle();
					}
				}
			}

			break;

		case MSG_KEYDOWN:

			switch(wparam)
			{
				case CSAPP_KEY_VOL_DOWN:
				case CSAPP_KEY_LEFT:
					if ( PVR_Rec_Banner.OnScreen == TRUE && wparam == CSAPP_KEY_LEFT )
					{
						if ( u8InputCount > 0 )
						{
							u8InputCount--;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
								KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

							SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
						}
					} else {
						hdc=MV_BeginPaint(hwnd);

						if ( PVR_Banner.OnScreen == TRUE )
						{
							PVR_Banner.Draw = FALSE;
							PVR_PaintBanner(hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}

						/* By KB Kim : 2011.06.13 */
						if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}

						PVR_VolumeScroll(FALSE);

						PVR_PaintVolume(hdc);
						PVR_Record_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;

				case CSAPP_KEY_VOL_UP:
				case CSAPP_KEY_RIGHT:
					if ( PVR_Rec_Banner.OnScreen == TRUE && wparam == CSAPP_KEY_RIGHT )
					{
						if ( u8InputCount < 3 )
						{
							u8InputCount++;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
								KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

							SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
						}
					} else {
						hdc=MV_BeginPaint(hwnd);

						if ( PVR_Banner.OnScreen == TRUE )
						{
							PVR_Banner.Draw = FALSE;
							PVR_PaintBanner(hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}

						/* By KB Kim : 2011.06.13 */
						if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}

						PVR_VolumeScroll(TRUE);

						PVR_PaintVolume(hdc);
						PVR_Record_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;

				case CSAPP_KEY_MUTE:
					if(CS_AV_Audio_GetMuteStatus() == FALSE)
						CS_AV_Audio_SetMuteStatus(TRUE);
					else
						CS_AV_Audio_SetMuteStatus(FALSE);

					CS_DBU_SaveMuteStatus();

					hdc=MV_BeginPaint(hwnd);
					PVR_Record_PaintMute(hdc);
					MV_EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

				case CSAPP_KEY_INFO:
					if(PVR_Rec_Banner.OnScreen == TRUE)
					{
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}

					if(PVR_Banner.OnScreen == FALSE)
					{
						CS_MW_CloseSubtitle(); /* By KB Kim : 2011.06.13 */
						PVR_Banner.Draw = TRUE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					} else {
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
					}

					hdc=MV_BeginPaint(hwnd);
					PVR_PaintBanner(hdc);
					MV_EndPaint(hwnd,hdc);

					/* By KB Kim : 2011.06.13 */
					if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
					{
						if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}
					}

					break;

				case CSAPP_KEY_REC:
					if(PVR_Banner.OnScreen == TRUE)
					{
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}

					if(PVR_Rec_Banner.OnScreen == FALSE)
					{
						CS_MW_CloseSubtitle(); /* By KB Kim : 2011.06.13 */
						PVR_Rec_Banner.Draw = TRUE;
						u8InputCount = 0;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
					} else {
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);
					}

					hdc=MV_BeginPaint(hwnd);
					PVR_Record_PaintBanner(hdc);
					MV_EndPaint(hwnd,hdc);

					/* By KB Kim : 2011.06.13 */
					if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
					{
						if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}
					}

					break;

				case CSAPP_KEY_ESC:
					if(PVR_Rec_Banner.OnScreen == TRUE)
					{
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					} else if(PVR_Banner.OnScreen == TRUE)
					{
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);

					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_NOW_REC);
						EndPaint(hwnd,hdc);

						usleep( 2000*1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_STOP:
					if ( CSMPR_Record_GetStatus() > CSMPR_REC_IDLE )
					{
						tCS_DB_ServiceManageData 	service_index_Temp;

						CS_DB_GetCurrentList_ServiceData( &service_index_Temp, CS_DB_GetCurrentService_OrderIndex());

						if ( CSMPR_Record_GetStatus() == CSMPR_REC_RUN )
						{
							printf("CSAPP_KEY_STOP CS_PVR_GetExecStatus %d\n", CSMPR_Player_GetStatus());
							MV_PVR_FileWrite_Time(CSApp_PVR_filename, NULL, service_index_Temp.Service_Index, CS_PVR_STOP);
							CSMPR_Record_Stop();
							printf("CSAPP_KEY_STOP CS_PVR_GetExecStatus %d\n", CSMPR_Player_GetStatus());
						}
					}

					CSApp_PVR_Record_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
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
				case CSAPP_KEY_RECALL:
					if ( PVR_Rec_Banner.OnScreen == TRUE )
					{
						PVR_Update_Duration(wparam);

						if ( u8InputCount < 3 )
						{
							u8InputCount++;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);
						} else {
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);
						}

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_PVR_Record_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_ENTER:
					if ( PVR_Rec_Banner.OnScreen == TRUE )
					{
						char		Temp_str[4];

						u8InputCount = 0;

						Temp_str[0] = acTemp_duration[0];
						Temp_str[1] = acTemp_duration[1];
						Temp_str[3] = 0x00;

						u8Hour_value = atoi(Temp_str);

						Temp_str[0] = acTemp_duration[2];
						Temp_str[1] = acTemp_duration[3];
						Temp_str[3] = 0x00;

						u8Min_value = atoi(Temp_str);

						DurationTime = ( u8Hour_value * 3600 ) + ( u8Min_value * 60 );

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					break;

				default:
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_NOW_REC);
					EndPaint(hwnd,hdc);

					usleep( 2000*1000 );

					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
					break;
			}

			break;
		case MSG_CLOSE:
			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
				KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

			if(IsTimerInstalled(hwnd, PVR_CHECK_TIMER_ID))
				KillTimer(hwnd,PVR_CHECK_TIMER_ID);

			if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
				KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

			CS_MW_CloseSubtitle();

			DurationTime = 0;

			if ( btTime_Capture.bmHeight != 0 )
			{
				UnloadBitmap (&btTime_Capture);
			}

			if ( MV_Get_BootMode() == BOOT_TIMER )
			{
				CSApp_PVR_Record_Applets = CSApp_Applet_Sleep;
				MV_Set_BootMode(BOOT_NORMAL);
			}

			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;
		default:
			break;
	}

	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

/************************v38den itibaren.....*********************************////

CSAPP_Applet_t CSApp_PVR_Streaming(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_PVR_Streaming_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption = "pvr_streaming";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = PVR_Streaming_Msg_cb;
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

	return CSApp_PVR_Streaming_Applets;
}

static int PVR_Streaming_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{
    //printf("PVR_Streaming_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)");

	HDC 				hdc;
	struct timespec		time_value;
	U8 					SubtitleNumber = 0;

	switch(message)
	{
		case MSG_CREATE:
			PVR_Rec_Banner.Draw = FALSE;
			PVR_Rec_Banner.OnScreen = FALSE;
			PVR_Banner.Draw = FALSE;
			PVR_Banner.OnScreen = FALSE;
			PVR_Volume.Draw = FALSE;
			PVR_Volume.OnScreen = FALSE;
			u8InputCount = 0;

#if 0 /* By KB Kim : 2011.06.13 */
			if (CS_MW_GetSubtitlePid() >= /*kDB_DEMUX_INVAILD_PID*/0x1FFF)
			{
				SubtitleNumber = MvGetCurrentSubtitle();
				// printf("PVR_Record_Msg_cb : Start Subtitle %d/%d\n", SubtitleNumber, MvGetTotalSubtitleNumber());
				if (SubtitleNumber < MvGetTotalSubtitleNumber())
				{
					CS_MW_OpenSubtitle();
				}
			}
#endif

			DBError = CS_DB_GetCurrentList_ServiceData( &service_index, CS_DB_GetCurrentService_OrderIndex());

			if( DBError == eCS_DB_OK )
			{
				DBError = MV_DB_GetServiceDataByIndex( &ServiceData, service_index.Service_Index );
				CS_EIT_Get_PF_Event(ServiceData.u16TransponderIndex , ServiceData.u16ServiceId, &present, &follow);
			}

			if ( CSMPR_Streaming_Start() != 0 || DBError != eCS_DB_OK )
			{
				hdc=BeginPaint(hwnd);
				MV_Draw_Msg_Window(hdc, CSAPP_STR_REC_ERROR);
				EndPaint(hwnd,hdc);

				usleep( 2000*1000 );

				hdc=BeginPaint(hwnd);
				Close_Msg_Window(hdc);
				EndPaint(hwnd,hdc);

				CSApp_PVR_Record_Applets=CSApp_Applet_Desktop;
				SendMessage(hwnd,MSG_CLOSE,0,0);
				break;
			}

			if ( DurationTime == 0 )
				DurationTime = TWO_HOUR;

			clock_gettime(CLOCK_REALTIME, &time_value);
			StartTime = time_value.tv_sec;







            //hwnd = GetActiveWindow();

            hdc=BeginPaint(hwnd);

            MV_Warning_Report_Window_Open( hwnd, MV_WINDOW_STREAMING_STARTED);
            usleep( 2000*1000 );
            MV_Warning_Report_Window_Close( hwnd );


			EndPaint(hwnd,hdc);

			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);

			if(IsTimerInstalled(hwnd, PVR_CHECK_TIMER_ID))
				KillTimer(hwnd,PVR_CHECK_TIMER_ID);

			SetTimer(hwnd, PVR_CHECK_TIMER_ID, pvr_timer_Max);

			break;

		case MSG_PAINT:
			hdc=MV_BeginPaint(hwnd);
			/*if ( CS_MW_GetCurrentMenuLanguage() == CS_APP_TURCK )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_REC_TR]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(PVR_ICON_X), ScalerHeigthPixel(PVR_ICON_Y),ScalerWidthPixel(MV_BMP[MVBMP_PVR_REC].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_PVR_REC].bmHeight), &MV_BMP[MVBMP_PVR_REC]);
			*/

			PVR_Banner.Draw = TRUE;
			PVR_PaintBanner(hdc);
			PVR_Record_PaintMute(hdc);
			MV_EndPaint(hwnd,hdc);
			return 0;

		/*case MSG_TIMER:
			if(wparam == PVR_CHECK_TIMER_ID )
			{
				clock_gettime(CLOCK_REALTIME, &time_value);
				NowTime = time_value.tv_sec;

				if ( NowTime > StartTime + DurationTime )
				{
					if ( CSMPR_Record_GetStatus() > CSMPR_REC_IDLE )
					{
						tCS_DB_ServiceManageData 	service_index_Temp;

						CS_DB_GetCurrentList_ServiceData( &service_index_Temp, CS_DB_GetCurrentService_OrderIndex());

						if ( CSMPR_Record_GetStatus() == CSMPR_REC_RUN )
						{
							//MV_PVR_FileWrite_Time(CSApp_PVR_filename, NULL, service_index_Temp.Service_Index, CS_PVR_STOP);
							CSMPR_Record_Stop();
						}
					}

					CSApp_PVR_Streaming_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
				} else if ( PVR_Banner.OnScreen == TRUE ) {
					hdc=MV_BeginPaint(hwnd);
					PVR_Draw_Time(hdc, TRUE);
					MV_EndPaint(hwnd,hdc);
				} else if ( PVR_Rec_Banner.OnScreen == TRUE ) {
					hdc=MV_BeginPaint(hwnd);
					PVR_Draw_Time(hdc, FALSE);
					MV_EndPaint(hwnd,hdc);
				}
			}
			else if(wparam == DESKTOP_BANNER_TIMER_ID)
			{
				PVR_Banner.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_PaintBanner(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
					KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
			}
			else if(wparam == DESKTOP_VOLUME_TIMER_ID)
			{
				PVR_Volume.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_PaintVolume(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
					KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);
			}
			else if(wparam == PVR_REC_BANNER_TIMER_ID)
			{
				PVR_Rec_Banner.Draw = FALSE;

				hdc=MV_BeginPaint(hwnd);
				PVR_Record_PaintBanner(hdc);
				MV_EndPaint(hwnd,hdc);

				if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
					KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);
			}

			// By KB Kim : 2011.06.13
			if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
			{
				if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
				{
					SubtitleNumber = MvGetCurrentSubtitle();
					if (SubtitleNumber < MvGetTotalSubtitleNumber())
					{
						CS_MW_OpenSubtitle();
					}
				}
			}

			break;*/

		case MSG_KEYDOWN:

			switch(wparam)
			{
				case CSAPP_KEY_VOL_DOWN:
				case CSAPP_KEY_LEFT:
					if ( PVR_Rec_Banner.OnScreen == TRUE && wparam == CSAPP_KEY_LEFT )
					{
						if ( u8InputCount > 0 )
						{
							u8InputCount--;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
								KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

							SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
						}
					} else {
						hdc=MV_BeginPaint(hwnd);

						if ( PVR_Banner.OnScreen == TRUE )
						{
							PVR_Banner.Draw = FALSE;
							PVR_PaintBanner(hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}

						/* By KB Kim : 2011.06.13 */
						if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}

						PVR_VolumeScroll(FALSE);

						PVR_PaintVolume(hdc);
						PVR_Record_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;

				case CSAPP_KEY_VOL_UP:
				case CSAPP_KEY_RIGHT:
					if ( PVR_Rec_Banner.OnScreen == TRUE && wparam == CSAPP_KEY_RIGHT )
					{
						if ( u8InputCount < 3 )
						{
							u8InputCount++;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);

							if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
								KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

							SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
						}
					} else {
						hdc=MV_BeginPaint(hwnd);

						if ( PVR_Banner.OnScreen == TRUE )
						{
							PVR_Banner.Draw = FALSE;
							PVR_PaintBanner(hdc);

							if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
								KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
						}

						/* By KB Kim : 2011.06.13 */
						if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}

						PVR_VolumeScroll(TRUE);

						PVR_PaintVolume(hdc);
						PVR_Record_PaintMute(hdc);
						MV_EndPaint(hwnd,hdc);

						if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
							KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

						SetTimer(hwnd, DESKTOP_VOLUME_TIMER_ID, Volume_timer_Max);
					}
					break;

				case CSAPP_KEY_MUTE:
					if(CS_AV_Audio_GetMuteStatus() == FALSE)
						CS_AV_Audio_SetMuteStatus(TRUE);
					else
						CS_AV_Audio_SetMuteStatus(FALSE);

					CS_DBU_SaveMuteStatus();

					hdc=MV_BeginPaint(hwnd);
					PVR_Record_PaintMute(hdc);
					MV_EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

				case CSAPP_KEY_INFO:
					if(PVR_Rec_Banner.OnScreen == TRUE)
					{
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}

					if(PVR_Banner.OnScreen == FALSE)
					{
						CS_MW_CloseSubtitle(); /* By KB Kim : 2011.06.13 */
						PVR_Banner.Draw = TRUE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						SetTimer(hwnd, DESKTOP_BANNER_TIMER_ID, Banner_timer_Max);
					} else {
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);
					}

					hdc=MV_BeginPaint(hwnd);
					PVR_PaintBanner(hdc);
					MV_EndPaint(hwnd,hdc);

					/* By KB Kim : 2011.06.13 */
					if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
					{
						if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}
					}

					break;

				/*case CSAPP_KEY_REC:
					if(PVR_Banner.OnScreen == TRUE)
					{
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}

					if(PVR_Rec_Banner.OnScreen == FALSE)
					{
						CS_MW_CloseSubtitle(); /* By KB Kim : 2011.06.13
						PVR_Rec_Banner.Draw = TRUE;
						u8InputCount = 0;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
					} else {
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);
					}

					hdc=MV_BeginPaint(hwnd);
					PVR_Record_PaintBanner(hdc);
					MV_EndPaint(hwnd,hdc);

					/* By KB Kim : 2011.06.13
					if (CS_MW_GetSubtitlePid() >= kDB_DEMUX_INVAILD_PID)
					{
						if( PVR_Rec_Banner.Draw == FALSE && PVR_Banner.Draw == FALSE)
						{
							SubtitleNumber = MvGetCurrentSubtitle();
							if (SubtitleNumber < MvGetTotalSubtitleNumber())
							{
								CS_MW_OpenSubtitle();
							}
						}
					}

					break;*/

				case CSAPP_KEY_ESC:
					if(PVR_Rec_Banner.OnScreen == TRUE)
					{
						PVR_Rec_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					} else if(PVR_Banner.OnScreen == TRUE)
					{
						PVR_Banner.Draw = FALSE;

						if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
							KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

						hdc=MV_BeginPaint(hwnd);
						PVR_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);

					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_NOW_REC);
						EndPaint(hwnd,hdc);

						usleep( 2000*1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_STOP:
					if ( CSMPR_Record_GetStatus() > CSMPR_REC_IDLE )
					{
						tCS_DB_ServiceManageData 	service_index_Temp;

						CS_DB_GetCurrentList_ServiceData( &service_index_Temp, CS_DB_GetCurrentService_OrderIndex());

						if ( CSMPR_Record_GetStatus() == CSMPR_REC_RUN )
						{
							printf("CSAPP_KEY_STOP CS_PVR_GetExecStatus %d\n", CSMPR_Player_GetStatus());
							//MV_PVR_FileWrite_Time(CSApp_PVR_filename, NULL, service_index_Temp.Service_Index, CS_PVR_STOP);
							CSMPR_Record_Stop();
							printf("CSAPP_KEY_STOP CS_PVR_GetExecStatus %d\n", CSMPR_Player_GetStatus());
						}
					}

					CSApp_PVR_Streaming_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
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
				case CSAPP_KEY_RECALL:
					if ( PVR_Rec_Banner.OnScreen == TRUE )
					{
						PVR_Update_Duration(wparam);

						if ( u8InputCount < 3 )
						{
							u8InputCount++;
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);
						} else {
							hdc=MV_BeginPaint(hwnd);
							PVR_Duration_Time(hdc);
							MV_EndPaint(hwnd,hdc);
						}

						if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
							KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

						SetTimer(hwnd, PVR_REC_BANNER_TIMER_ID, Pvr_Banner_timer_Max);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_PVR_Streaming_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_ENTER:
					if ( PVR_Rec_Banner.OnScreen == TRUE )
					{
						char		Temp_str[4];

						u8InputCount = 0;

						Temp_str[0] = acTemp_duration[0];
						Temp_str[1] = acTemp_duration[1];
						Temp_str[3] = 0x00;

						u8Hour_value = atoi(Temp_str);

						Temp_str[0] = acTemp_duration[2];
						Temp_str[1] = acTemp_duration[3];
						Temp_str[3] = 0x00;

						u8Min_value = atoi(Temp_str);

						DurationTime = ( u8Hour_value * 3600 ) + ( u8Min_value * 60 );

						hdc=MV_BeginPaint(hwnd);
						PVR_Record_PaintBanner(hdc);
						MV_EndPaint(hwnd,hdc);
					}
					break;

				default:
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_STREAMING_NOW);
					EndPaint(hwnd,hdc);

					usleep( 2000*1000 );

					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
					break;
			}

			break;
		case MSG_CLOSE:
			if(IsTimerInstalled(hwnd, DESKTOP_BANNER_TIMER_ID))
				KillTimer(hwnd,DESKTOP_BANNER_TIMER_ID);

			if(IsTimerInstalled(hwnd, DESKTOP_VOLUME_TIMER_ID))
				KillTimer(hwnd,DESKTOP_VOLUME_TIMER_ID);

			if(IsTimerInstalled(hwnd, PVR_CHECK_TIMER_ID))
				KillTimer(hwnd,PVR_CHECK_TIMER_ID);

			if(IsTimerInstalled(hwnd, PVR_REC_BANNER_TIMER_ID))
				KillTimer(hwnd,PVR_REC_BANNER_TIMER_ID);

			CS_MW_CloseSubtitle();

			DurationTime = 0;

			if ( btTime_Capture.bmHeight != 0 )
			{
				UnloadBitmap (&btTime_Capture);
			}

			if ( MV_Get_BootMode() == BOOT_TIMER )
			{
				CSApp_PVR_Streaming_Applets = CSApp_Applet_Sleep;
				MV_Set_BootMode(BOOT_NORMAL);
			}

			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;
		default:
			break;
	}

	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


