#include "linuxos.h"

#include "database.h"
#include "cs_app_common.h"

#include "userdefine.h"
#include "cs_app_main.h"
#include "mwsetting.h"
#include "cstimesetting.h"
#include "date_time.h"
#include "ui_common.h"
#include "mvosapi.h"

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

#define	FIELDS_PER_LINE		2

static U16					Timer_Focus_Item = 0;
static U16					Current_TimeZone = 0;
static U16					Current_TimeMode = 0;
static U16					Item_Num = 0;
static U8					eSummer_time = 0;
static tTIME_SETTING_YMD	Current_ymd;
static tTIME_SETTING_HM		Current_hm;
static tCS_DT_Date  		Current_Date;
static tCS_DT_Time			Current_Time;

U8	Time_Arrow_Kind[TIME_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	Time_Enter_Kind[TIME_ITEM_MAX] = {
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

U16 Time_Str[TIME_ITEM_MAX] = {
	CSAPP_STR_TIME_MODE,
	CSAPP_STR_TIME_ZONE,
	CSAPP_STR_SUMMER_TIME,
	CSAPP_STR_YEAR,
	CSAPP_STR_MONTH,
	CSAPP_STR_DAY,
	CSAPP_STR_HOUR,
	CSAPP_STR_MINUTE
};

static CSAPP_Applet_t	CSApp_TimeSetting_Applets;

static U16 sTimeMode[CS_APP_TIME_MODE_NUM] =
{
    CSAPP_STR_TIME_MODE_NET,
    CSAPP_STR_TIME_MODE_LOCAL,
    CSAPP_STR_TIME_MODE_UTC
};

void MV_Draw_TimeSelectBar(HDC hdc, int y_gap, eMV_Time_ItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Time_Enter_Kind[esItem])
	{
		case MV_NUMERIC:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
			break;
		case MV_SELECT:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
			break;
		default:
			break;
	}
	
	if ( Time_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_TimeMenuBar(HDC hdc, U8 u8Focuskind, eMV_Time_ItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == MV_FOCUS )
	{
		if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
		{
			if ( esItem == TIME_ITEM_ZONE )
			{
				SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);

				MV_SetBrushColor( hdc, MV_BAR_DISABLE_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
			}
			else
			{
				SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);

				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
				MV_Draw_TimeSelectBar(hdc, y_gap, esItem);
			}
		} else {
			if ( esItem > TIME_ITEM_SUMMER && esItem < TIME_ITEM_MAX )
			{
				SetTextColor(hdc,MV_BAR_DISABLE_CHAR_COLOR);

				MV_SetBrushColor( hdc, MV_BAR_DISABLE_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
			}
			else
			{
				SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);

				MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
				MV_Draw_TimeSelectBar(hdc, y_gap, esItem);
			}
		}
		SetBkMode(hdc,BM_TRANSPARENT);
	} else {
		if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
		{
			if ( esItem == TIME_ITEM_ZONE )
				SetTextColor(hdc,MV_BAR_DISABLE_CHAR_COLOR);
			else
				SetTextColor(hdc,MV_BAR_ENABLE_CHAR_COLOR);
		} else {
			if ( esItem > TIME_ITEM_SUMMER && esItem < TIME_ITEM_MAX )
				SetTextColor(hdc,MV_BAR_DISABLE_CHAR_COLOR);
			else
				SetTextColor(hdc,MV_BAR_ENABLE_CHAR_COLOR);
		}
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Time_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[20];
		
		case TIME_ITEM_MODE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(sTimeMode[Current_TimeMode]), -1, &TmpRect, DT_CENTER);		
			break;
		case TIME_ITEM_ZONE:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%s", "--:--");
			else
			{
				if (Current_TimeZone < 24)
				{
					sprintf(temp_str, "-%d:%02d", (12-(Current_TimeZone+1)/2), (Current_TimeZone%2)*30);
				}
				else
				{
					sprintf(temp_str, "%d:%02d", (Current_TimeZone/2-12), (Current_TimeZone%2)*30);
				}
			}
			CS_MW_SetTimeZone(Current_TimeZone);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_SUMMER:
			if ( eSummer_time == 1 )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_ON), -1, &TmpRect, DT_CENTER);	
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OFF), -1, &TmpRect, DT_CENTER);	

			CS_MW_SetTimeRegion(eSummer_time);
			break;
		case TIME_ITEM_YEAR:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%d", Current_Date.year);
			else
				sprintf(temp_str, "%s", "----");
				
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_MONTH:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%d", Current_Date.month);
			else
				sprintf(temp_str, "%s", "--");
				
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_DAY:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%d", Current_Date.day);
			else
				sprintf(temp_str, "%s", "--");
				
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_HOUR:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%d", Current_Time.hour);
			else
				sprintf(temp_str, "%s", "--");
				
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_MINUTE:
			if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
				sprintf(temp_str, "%d", Current_Time.minute);
			else
				sprintf(temp_str, "%s", "--");
				
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TIME_ITEM_MAX:
		default:
			break;
	}
}

void MV_Draw_TIME_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < TIME_ITEM_MAX ; i++ )
	{
		if( Timer_Focus_Item == i )
		{
			MV_Draw_TimeMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_TimeMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

static U16 TimeSetting_Get_Item_Num(U16 time_mode)
{
    if (time_mode == CS_APP_TIME_MODE_LOCAL)
    {
        return CS_APP_TIME_SETTING_ITEM_NUM_LOCAL;
    }
    else if (time_mode == CS_APP_TIME_MODE_NET)
    {
        return CS_APP_TIME_SETTING_ITEM_NUM_NET;
    }
	else if (time_mode == CS_APP_TIME_MODE_UTC)
    {
        return CS_APP_TIME_SETTING_ITEM_NUM_NET;
    }
    else
        return 0;
}

static BOOL TimeSetting_Check_Leap_Year(U16 year)
{
    if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

#if 0
static tCS_DT_Date TimeSetting_Convert_Date(tTIME_SETTING_YMD ymd)
{
    tCS_DT_Date tmp;
    char str[5];

    str[0] = ymd.ymd[6];
    str[1] = ymd.ymd[7];
    str[2] = ymd.ymd[8];
    str[3] = ymd.ymd[9];
    str[4] = '\0';
    tmp.year = atoi(str);
    str[0] = ymd.ymd[0];
    str[1] = ymd.ymd[1];
    str[2] = '\0';
    tmp.month = atoi(str);
    str[0] = ymd.ymd[3];
    str[1] = ymd.ymd[4];
    str[2] = '\0';
    tmp.day = atoi(str);

    return tmp;
}

static tCS_DT_Time TimeSetting_Convert_Time(tTIME_SETTING_HM hm)
{
    tCS_DT_Time tmp;
    char str[3];

    str[0] = hm.hm[0];
    str[1] = hm.hm[1];
    str[2] = '\0';
    tmp.hour = atoi(str);
    str[0] = hm.hm[3];
    str[1] = hm.hm[4];
    str[2] = '\0';
    tmp.minute = atoi(str);

    return tmp;
}
#endif

static BOOL TimeSetting_Check_Input_Date(char input_num, tTIME_SETTING_YMD *ymd)
{
    U16 year;
    char year_str[5];

    if (ymd == NULL)
        return FALSE;

    year_str[0] = ymd->ymd[6];
    year_str[1] = ymd->ymd[7];
    year_str[2] = ymd->ymd[8];
    year_str[3] = ymd->ymd[9];
    year_str[4] = '\0';
    year = atoi(year_str);

    switch (ymd->current_char)
	{
		case 1:	// month 1
			if ((input_num == '1') || (input_num == '0'))
			{
				ymd->ymd[0] = input_num;
				if (input_num == '0' && ymd->ymd[1] == '2')
				{
					if (ymd->ymd[3] == '3')
					{
						ymd->ymd[3] = '0';
						ymd->ymd[4] = '1';
					}
				}
				return TRUE;
			}
			return FALSE;
			
		case 2:	// month 2
			if (ymd->ymd[0] == '0')
			{
				ymd->ymd[1] = input_num;
				if (ymd->ymd[1] == '2')
				{
					if (ymd->ymd[3] == '3')
					{
						ymd->ymd[3] = '0';
						ymd->ymd[4] = '1';
					}
				}
				return TRUE;
			}
			else if (ymd->ymd[0] == '1')
			{
				if (input_num == '1' || input_num == '2')
				{
					ymd->ymd[1] = input_num;
					return TRUE;
				}
			}
			return FALSE;
			
		case 4:	// day 1
			if (ymd->ymd[0] == '0' && ymd->ymd[1] == '2')	// Feb
			{
				if ((input_num >= '0') && input_num <= '2')
				{
					ymd->ymd[3] = input_num;
					return TRUE;
				}
			}
			else
			{
				if ((input_num >= '0') && (input_num <= '3'))
				{
					ymd->ymd[3] = input_num;
					if (ymd->ymd[3] == '3')
					{
						if (ymd->ymd[4] != '0' && ymd->ymd[4] != '1')
						ymd->ymd[4] = '0';
					}
					return TRUE;
				}
			}
			return FALSE;
			
		case 5:	// day 2
			if (ymd->ymd[0] == '0' && ymd->ymd[1] == '2')	// Feb
			{
				if (ymd->ymd[3] == '3')
					return FALSE;
				else if (ymd->ymd[3] == '0' || ymd->ymd[3] == '1')
				{
					ymd->ymd[4] = input_num;
					return TRUE;
				}
				else if (ymd->ymd[3] == '2')
				{
					if (TimeSetting_Check_Leap_Year(year))
					{
						ymd->ymd[4] = input_num;
						return TRUE;
					}
					else
					{
						if (input_num != '9')
						{
							ymd->ymd[4] = input_num;
							return TRUE;
						}
					}
				}
			}
			else
			{
				if (ymd->ymd[3] == '3')
				{
					if ((input_num == '1') || (input_num == '0'))
					{
						ymd->ymd[4] = input_num;
						return TRUE;
					}
				}
				else if (ymd->ymd[3] == '2' || ymd->ymd[3] == '1' || ymd->ymd[3] == '0')
				{
					ymd->ymd[4] = input_num;
					return TRUE;
				}
			}
			return FALSE;
			
		case 7:	// year 1
			if ((input_num == '1') || (input_num == '2'))
			{
				ymd->ymd[6] = input_num;
				if (ymd->ymd[6] == '1')
				{
					ymd->ymd[7] = '9';
					ymd->current_char++;
				}
				
				if (ymd->ymd[6] == '2')
				{
					ymd->ymd[7] = '0';
					ymd->current_char++;
				}
				return TRUE;
			}
			return FALSE;
		
		case 8:	// year 2
			if ((ymd->ymd[6] == '2') && (input_num == '0'))
			{
				ymd->ymd[7] = input_num;
				return TRUE;
			}
			
			if ((ymd->ymd[6] == '1') && (input_num == '9'))
			{
				ymd->ymd[7] = input_num;
				return TRUE;
			}
			return FALSE;
			
		case 9:	// year 3
			if ((ymd->ymd[7] == '9') && ((input_num == '8') || (input_num == '9')))
			{
				ymd->ymd[8] = input_num;
				return TRUE;
			}
			else
			{
				ymd->ymd[8] = input_num;
				return TRUE;
			}
			return FALSE;

		case 10:	// year 4
			ymd->ymd[9] = input_num;
			return TRUE;

		default:
			return FALSE;
	}
}

static BOOL TimeSetting_Check_Input_Time(char input_num, tTIME_SETTING_HM *hm)
{
    switch (hm->current_char)
    {
    case 1:
        if (input_num == '0' || input_num == '1' || input_num == '2')
        {
            hm->hm[0] = input_num;
            if (input_num == '2')
            {
                if (hm->hm[1] > '4')
                {
                    hm->hm[1] = '0';
                }
            }
            return TRUE;
        }

        return FALSE;
    case 2:
        if (hm->hm[0] == '2')
        {
            if (input_num >= '0' && input_num <= '3')
            {
                hm->hm[1] = input_num;
                return TRUE;
            }
        }
        else
        {
            hm->hm[1] = input_num;
            return TRUE;
        }

        return FALSE;
    case 4:
        if (input_num >= '0' && input_num <= '5')
        {
            hm->hm[3] = input_num;
            return TRUE;
        }

        return FALSE;
    case 5:
        hm->hm[4] = input_num;
        return TRUE;
    default:
        return FALSE;
    }
}

static int Time_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

CSAPP_Applet_t	CSApp_TimeSetting(void)
{
    int   			BASE_X, BASE_Y, WIDTH, HEIGHT;
    MSG   			msg;
    HWND  			hwndMain;
    MAINWINCREATE	CreateInfo;

	CSApp_TimeSetting_Applets = CSApp_Applet_Error;

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

    CreateInfo.dwStyle	 		= WS_VISIBLE;
    CreateInfo.dwExStyle 		= WS_EX_NONE;
    CreateInfo.spCaption 		= "cstimesetting window";
    CreateInfo.hMenu	 		= 0;
    CreateInfo.hCursor	 		= 0;
    CreateInfo.hIcon	 		= 0;
    CreateInfo.MainWindowProc 	= Time_Msg_cb;
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
    return CSApp_TimeSetting_Applets;

}

void MV_Draw_Timer(HWND hwnd)
{
	HDC 						hdc;
	RECT						rtRect;
	char						temp_char[100];

	MV_OS_Get_Time_Offset(temp_char, TRUE);

	rtRect.top 		= MV_HELP_ICON_Y - 100;
	rtRect.bottom	= rtRect.top + 50;
	rtRect.left		= 200;
	rtRect.right	= 1080;

	hdc=BeginPaint(hwnd);
	SetTextColor(hdc,MV_BAR_ENABLE_CHAR_COLOR);	
	MV_SetBrushColor(hdc, MV_BAR_UNFOCUS_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_FillBox(hdc, rtRect.left, rtRect.top, rtRect.right - rtRect.left, rtRect.bottom - rtRect.top);
	MV_MW_DrawBigText(hdc, temp_char, -1, &rtRect, DT_CENTER);
	EndPaint(hwnd,hdc);
/*
	if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
		system("kill mvapp.elf");
*/
}

static int Time_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 						hdc;
	static BOOL 				MainPaintFlag = TRUE;
	static char					Num_Input;
	U16							CurrMjd;
	U16							CurrUtc;

	switch (message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			
			Current_TimeZone = CS_MW_GetTimeZone();
			Current_TimeMode = CS_MW_GetTimeMode();
			//printf("\n===== TIME MODE : %d \n", Current_TimeMode);
			eSummer_time = CS_MW_GetTimeRegion();
/* Must Default Data Setting */  
			Item_Num = TimeSetting_Get_Item_Num(Current_TimeMode);
			Timer_Focus_Item = 0;
#if 0
			Current_Date = CS_DT_MJDtoYMD(CS_DT_GetLocalMJD());
			Current_Time = CS_DT_UTCtoHM(CS_DT_GetLocalUTC());
#else
			MV_OS_Get_Time_to_MJD_UTC_Date_Time(&CurrMjd, &CurrUtc, &Current_Date, &Current_Time);
#endif

			Current_ymd.current_char = 1;
			Current_hm.current_char = 1;
			sprintf(Current_ymd.ymd, "%02d/%02d/%04d", Current_Date.month, Current_Date.day, Current_Date.year);
			sprintf(Current_hm.hm, "%02d:%02d", Current_Time.hour, Current_Time.minute);

			//system("date > /usr/work0/date.txt");
			break;
		case MSG_PAINT:

			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_TIME);

			hdc=BeginPaint(hwnd);
			MV_Draw_TIME_MenuBar(hdc);
			MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_TIME);
			EndPaint(hwnd,hdc);

			MV_Draw_Timer(hwnd);
			return 0;

		case MSG_TIMER:
			MV_Draw_Timer(hwnd);
			break;

	    case MSG_KEYDOWN:
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Timer_Focus_Item)
					{
						case TIME_ITEM_MODE:
							Current_TimeMode = u8Result_Value;
							break;
							
						case TIME_ITEM_ZONE:
							break;
							
						case TIME_ITEM_SUMMER:
							if ( u8Result_Value == 0 )
								eSummer_time = 1;
							else
								eSummer_time = 0;
							break;
							
						case TIME_ITEM_YEAR:
							break;
							
						case TIME_ITEM_MONTH:
							break;
							
						case TIME_ITEM_DAY:
							break;
							
						case TIME_ITEM_HOUR:
							break;
							
						case TIME_ITEM_MINUTE:
							break;

						default:
							break;
					}
					hdc=BeginPaint(hwnd);
					if ( Timer_Focus_Item == TIME_ITEM_MODE )
					{
						MV_Draw_TIME_MenuBar(hdc);
						MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_TIME);
					}
					else
						MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);
					EndPaint(hwnd,hdc);

					if ( Timer_Focus_Item == TIME_ITEM_SUMMER )
						MV_Draw_Timer(hwnd);
					
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}
				break;
			}
			
	        switch (wparam)
	        {
/* Just Test For Default value setting */
#if 0
				case CSAPP_KEY_YELLOW:
					CS_DBU_SetTimeOffset(kCS_DBU_DEFAULT_TimeOffset);
					Current_TimeZone = CS_MW_GetTimeZone();
					
					hdc=BeginPaint(hwnd);
					if ( Timer_Focus_Item == TIME_ITEM_ZONE )
						MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);
					else
						MV_Draw_TimeMenuBar(hdc, MV_UNFOCUS, Timer_Focus_Item);
					EndPaint(hwnd,hdc);
					break;
#endif
/***************************************/

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
					{
			            if (Current_TimeMode == CS_APP_TIME_MODE_NET || Current_TimeMode == CS_APP_TIME_MODE_UTC)
			                break;

						Num_Input = Key_to_Ascii(wparam);

						switch (Timer_Focus_Item)
			            {
				            case CS_APP_YMD:
				                if (TimeSetting_Check_Input_Date(Num_Input, &Current_ymd))
				                {
				                    if (Current_ymd.current_char != TIME_SETTING_YMD_STR_LEN - 1)
				                        Current_ymd.current_char++;
				                    if (Current_ymd.current_char == 3 || Current_ymd.current_char == 6)
				                        Current_ymd.current_char++;

									ComboList_Update_Element(hwnd, Timer_Focus_Item);
				                }

				                break;
				            case CS_APP_HM:
				                if (TimeSetting_Check_Input_Time(Num_Input, &Current_hm))
				                {
				                    if (Current_hm.current_char != TIME_SETTING_HM_STR_LEN - 1)
				                        Current_hm.current_char++;
				                    if (Current_hm.current_char == 3)
				                        Current_hm.current_char++;

									ComboList_Update_Element(hwnd, Timer_Focus_Item);
				                }
				                break;
				            default:
				                break;
			            }
		        	}
		            break;
		        case CSAPP_KEY_ESC:
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
					EndPaint(hwnd,hdc);
					
		            if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
		            {
						struct timespec time_value;
						struct tm		Temp_Time;
						
		                CS_DT_ManualSetDateAndTime(CS_DT_YMDtoMJD(Current_Date), CS_DT_HMtoUTC(Current_Time));
						CS_MW_SetTimeRegion(eSummer_time);
						CS_MW_SetTimeMode(eCS_DBU_TIME_MANUAL);

						Temp_Time.tm_sec = 0;
						Temp_Time.tm_min = Current_Time.minute;
						Temp_Time.tm_hour = Current_Time.hour;
						Temp_Time.tm_mday = Current_Date.day;
						Temp_Time.tm_mon = Current_Date.month - 1;
						Temp_Time.tm_year = Current_Date.year - 1900;
			
						time_value.tv_sec = (time_t)mktime(&Temp_Time);
						time_value.tv_nsec = 0;

						if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
							// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
							printf("clock_settime :: Error\n");
						}
		            }
		            else if (Current_TimeMode == CS_APP_TIME_MODE_NET)
		            {
		                CS_MW_SetTimeZone(Current_TimeZone);
						CS_MW_SetTimeRegion(eSummer_time);						
		                CS_MW_SetTimeMode(eCS_DBU_TIME_AUTOMATIC);
		            } else {
		            	CS_MW_SetTimeZone(Current_TimeZone);
						CS_MW_SetTimeRegion(eSummer_time);
		                CS_MW_SetTimeMode(eCS_DBU_TIME_INTERNET);
						MV_OS_Get_Time_From_NTP_LongTime();
		            }

		            CS_DBU_SaveUserSettingDataInHW();
		            CSApp_TimeSetting_Applets = CSApp_Applet_Desktop;

					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
					
		            SendMessage(hwnd,MSG_CLOSE,0,0);
			        break;
					
		        case CSAPP_KEY_MENU:
					hdc=BeginPaint(hwnd);
					MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
					EndPaint(hwnd,hdc);
					
		            if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
		            {
						struct timespec time_value;
						struct tm		Temp_Time;
						
		                CS_DT_ManualSetDateAndTime(CS_DT_YMDtoMJD(Current_Date), CS_DT_HMtoUTC(Current_Time));
						CS_MW_SetTimeRegion(eSummer_time);
						CS_MW_SetTimeMode(eCS_DBU_TIME_MANUAL);

						Temp_Time.tm_sec = 0;
						Temp_Time.tm_min = Current_Time.minute;
						Temp_Time.tm_hour = Current_Time.hour;
						Temp_Time.tm_mday = Current_Date.day;
						Temp_Time.tm_mon = Current_Date.month - 1;
						Temp_Time.tm_year = Current_Date.year - 1900;
			
						time_value.tv_sec = (time_t)mktime(&Temp_Time);
						time_value.tv_nsec = 0;

						if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
							// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
							printf("clock_settime :: Error\n");
						}
		            }
		            else if (Current_TimeMode == CS_APP_TIME_MODE_NET)
		            {
		                CS_MW_SetTimeZone(Current_TimeZone);
						CS_MW_SetTimeRegion(eSummer_time);						
		                CS_MW_SetTimeMode(eCS_DBU_TIME_AUTOMATIC);
		            } else {
		            	CS_MW_SetTimeZone(Current_TimeZone);
						CS_MW_SetTimeRegion(eSummer_time);
		                CS_MW_SetTimeMode(eCS_DBU_TIME_INTERNET);
						MV_OS_Get_Time_From_NTP_LongTime();
		            }

		            CS_DBU_SaveUserSettingDataInHW();
		            CSApp_TimeSetting_Applets = b8Last_App_Status;

					hdc=BeginPaint(hwnd);
					Close_Msg_Window(hdc);
					EndPaint(hwnd,hdc);
					
		            SendMessage(hwnd,MSG_CLOSE,0,0);
			        break;

	            case CSAPP_KEY_IDLE:
	            	CSApp_TimeSetting_Applets = CSApp_Applet_Sleep;
	            	SendMessage(hwnd,MSG_CLOSE,0,0);
	            	break;
	                
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
		        case CSAPP_KEY_ENTER:
					{
						int						i = 0;
						RECT					smwRect;
						stPopUp_Window_Contents stContents;

						memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
						smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Timer_Focus_Item + 1 );
						smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
						smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
						
						switch(Timer_Focus_Item)
						{
							case TIME_ITEM_MODE:
								for ( i = 0 ; i < CS_APP_TIME_MODE_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(sTimeMode[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_TIME_MODE_NUM );
								stContents.u8TotalCount = CS_APP_TIME_MODE_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
								
							case TIME_ITEM_ZONE:
								break;
								
							case TIME_ITEM_SUMMER:
								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ON));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 2;
								stContents.u8TotalCount = 2;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
								
							case TIME_ITEM_YEAR:
								break;
								
							case TIME_ITEM_MONTH:
								break;
								
							case TIME_ITEM_DAY:
								break;
								
							case TIME_ITEM_HOUR:
								break;
								
							case TIME_ITEM_MINUTE:
								break;

							default:
								break;
						}
		       	 	}
		            break;
					
		        case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_TimeMenuBar(hdc, MV_UNFOCUS, Timer_Focus_Item);

					if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
					{
						if (Timer_Focus_Item == TIME_ITEM_MODE)
							Timer_Focus_Item = TIME_ITEM_MINUTE;
						else if (Timer_Focus_Item == TIME_ITEM_SUMMER) 
							Timer_Focus_Item = TIME_ITEM_MODE;
						else
							Timer_Focus_Item--;
					} else {
						if (Timer_Focus_Item == TIME_ITEM_MODE)
							Timer_Focus_Item = TIME_ITEM_SUMMER;
						else
							Timer_Focus_Item--;
					}

					MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);
					EndPaint(hwnd,hdc);
		            break;
					
		        case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
		            MV_Draw_TimeMenuBar(hdc, MV_UNFOCUS, Timer_Focus_Item);

					if (Current_TimeMode == CS_APP_TIME_MODE_LOCAL)
					{
			            if (Timer_Focus_Item == TIME_ITEM_MINUTE)
			                Timer_Focus_Item = TIME_ITEM_MODE;
			            else if (Timer_Focus_Item == TIME_ITEM_MODE)
			                Timer_Focus_Item = TIME_ITEM_SUMMER;
						else
							Timer_Focus_Item++;
					} else {
						if (Timer_Focus_Item == TIME_ITEM_SUMMER)
			                Timer_Focus_Item = TIME_ITEM_MODE;
						else
							Timer_Focus_Item++;
					}

		            MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);
					EndPaint(hwnd,hdc);
		            break;
					
		        case CSAPP_KEY_LEFT:
					hdc=BeginPaint(hwnd);
			        {
						switch(Timer_Focus_Item)
						{
							case TIME_ITEM_MODE:
								if ( Current_TimeMode == 0 )
									Current_TimeMode = CS_APP_TIME_MODE_NUM - 1;
								else
									Current_TimeMode--;
								break;
							case TIME_ITEM_ZONE:
								if (Current_TimeZone == 0)
			                    {
			                        Current_TimeZone = CS_APP_TIME_ZONE_NUM - 1;
			                    }
			                    else
			                        Current_TimeZone--;
								break;
							case TIME_ITEM_SUMMER:
								if ( eSummer_time == 0 )
									eSummer_time = 1;
								else
									eSummer_time = 0;
								break;
							case TIME_ITEM_YEAR:
								if ( Current_Date.year > 2010 )
									Current_Date.year--;
								break;
							case TIME_ITEM_MONTH:
								if ( Current_Date.month == 1 )
									Current_Date.month = 12;
								else
									Current_Date.month--;
								break;
							case TIME_ITEM_DAY:
								switch(Current_Date.month)
								{
									case 2:
										break;
									case 1:
									case 3:
									case 5:
									case 7:
									case 8:
									case 10:
									case 12:
										if ( Current_Date.day == 1 )
											Current_Date.day = 31;
										else
											Current_Date.day--;
										break;
									case 4:
									case 6:
									case 9:
									case 11:
									default:
										if ( Current_Date.day == 1 )
											Current_Date.day = 30;
										else
											Current_Date.day--;
										break;
								}
								break;
							case TIME_ITEM_HOUR:
								if ( Current_Time.hour <= 1 )
									Current_Time.hour = 24;
								else
									Current_Time.hour--;
								break;
							case TIME_ITEM_MINUTE:
								if ( Current_Time.minute == 1 )
									Current_Time.minute = 60;
								else
									Current_Time.minute--;
								break;
						}
						if (Timer_Focus_Item == TIME_ITEM_MODE)
						{
							MV_Draw_TIME_MenuBar(hdc);
							MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_TIME);
						}
						else
							MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);

						if ( Timer_Focus_Item == TIME_ITEM_SUMMER || Timer_Focus_Item == TIME_ITEM_ZONE )
						{
							MV_Draw_Timer(hwnd);
						}
		        	}
					EndPaint(hwnd,hdc);
					break;
					
		        case CSAPP_KEY_RIGHT:
					hdc=BeginPaint(hwnd);
			        {
						switch(Timer_Focus_Item)
						{
							case TIME_ITEM_MODE:
								if ( Current_TimeMode == CS_APP_TIME_MODE_NUM - 1 )
									Current_TimeMode = 0;
								else
									Current_TimeMode++;
								break;
							case TIME_ITEM_ZONE:
								if (Current_TimeZone == CS_APP_TIME_ZONE_NUM - 1)
			                    {
			                        Current_TimeZone = 0;
			                    }
			                    else
			                        Current_TimeZone++;
								break;
							case TIME_ITEM_SUMMER:
								if ( eSummer_time == 0 )
									eSummer_time = 1;
								else
									eSummer_time = 0;
								break;
							case TIME_ITEM_YEAR:
								/* For Year Limit problem by KB Kim 2012.03.26 */
								if ( Current_Date.year < 2100 )
									Current_Date.year++;
								break;
							case TIME_ITEM_MONTH:
								if ( Current_Date.month == 12 )
									Current_Date.month = 1;
								else
									Current_Date.month++;
								break;
							case TIME_ITEM_DAY:
								switch(Current_Date.month)
								{
									case 2:
										break;
									case 1:
									case 3:
									case 5:
									case 7:
									case 8:
									case 10:
									case 12:
										if ( Current_Date.day == 31 )
											Current_Date.day = 1;
										else
											Current_Date.day++;
										break;
									case 4:
									case 6:
									case 9:
									case 11:
									default:
										if ( Current_Date.day == 30 )
											Current_Date.day = 1;
										else
											Current_Date.day++;
										break;
								}
								break;
							case TIME_ITEM_HOUR:
								if ( Current_Time.hour >= 24 )
									Current_Time.hour = 1;
								else
									Current_Time.hour++;
								break;
							case TIME_ITEM_MINUTE:
								if ( Current_Time.minute == 60 )
									Current_Time.minute = 1;
								else
									Current_Time.minute++;
								break;
						}
						if (Timer_Focus_Item == TIME_ITEM_MODE)
						{
							MV_Draw_TIME_MenuBar(hdc);
							MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_TIME);
						}
						else
							MV_Draw_TimeMenuBar(hdc, MV_FOCUS, Timer_Focus_Item);
		        	}
					EndPaint(hwnd,hdc);

					if ( Timer_Focus_Item == TIME_ITEM_SUMMER || Timer_Focus_Item == TIME_ITEM_ZONE )
					{
						MV_Draw_Timer(hwnd);
					}
					break;

				case CSAPP_KEY_F1:
					MV_OS_Get_Time_From_NTP();
					break;
					
		        default:
		            break;
	        }
	        break;

	    case MSG_CLOSE:
	        MainPaintFlag = TRUE;
	      	KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
	        PostQuitMessage(hwnd);
	        DestroyMainWindow(hwnd);
	        break;

	    default:
	        break;
    }

    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

