#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "push.h"

#define S_X								90
#define S_Y								162
#define T_S_Y							( S_Y - 32 )
#define S_T_X							( S_X + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10 )
#define S_T_Y							( S_Y + 350 )
#define BLOCK_YS						30

#define CALENDER_ITEM_WIDTH    			70//72
#define CALENDER_ITEM_HEIGHT   			39//60
#define CALENDER_ITEM_X_OFFSET 			16
#define CALENDER_ITEM_BORDER   			2

#define CALENDER_HEAD_HEIGHT_OF_YEAR	40
#define CALENDER_HEAD_HEIGHT_OF_WEEKLY	30
#define CALENDER_HEAD_HEIGHT			(CALENDER_HEAD_HEIGHT_OF_YEAR+CALENDER_HEAD_HEIGHT_OF_WEEKLY)
#define CALENDER_BODY_HEIGHT			(CALENDER_ITEM_HEIGHT*6 + 2*CALENDER_ITEM_BORDER)
#define CALENDER_TILE_HEIGHT			70
#define CALENDER_X_OFFSET				16

#define CAL_FIRSTYEAR					1900//1999
#define CAL_LASTYEAR					2100//2031
#define CALENDER_BACKGROUND_BORDER   	16

#define FOCUS_REC_OFFSET_X 				4
#define FOCUS_REC_OFFSET_Y 				4

static const U16  enYearMonth[] = 
                                {
									CSAPP_STR_JANUARY,
									CSAPP_STR_FEBRUARY,
									CSAPP_STR_MARCH,
									CSAPP_STR_APRIL,
									CSAPP_STR_MAY,
									CSAPP_STR_JUNE,
									CSAPP_STR_JULY,
									CSAPP_STR_AUGUST,
									CSAPP_STR_SEPTEMBER,
									CSAPP_STR_OCTOBER,
									CSAPP_STR_NOVEMBER,
									CSAPP_STR_DECEMBER,
									CSAPP_STR_JANUARY,
									CSAPP_STR_FEBRUARY
                              };

static const U16  enWeeklyDay[] = 
                                {
									CSAPP_STR_EPG_SUN,
									CSAPP_STR_EPG_MON,
									CSAPP_STR_EPG_TUSE,
									CSAPP_STR_EPG_WED,
									CSAPP_STR_EPG_THUS,
									CSAPP_STR_EPG_FRI,
									CSAPP_STR_EPG_SAT,
									CSAPP_STR_EPG_SUN,
									CSAPP_STR_EPG_MON,
									CSAPP_STR_EPG_TUSE
                              };

static U8 SolarCal[12] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static U16				u16TotalH;
static U16				u16TotalW;
static U16				u16StartX;
static U16				u16StartY;
static U16 				u16CurrentYear;
static U8 				u8CurrentMonth, u8CurrentDate;
static U8 				u8LastDay;
static U16 				u16FocusYear ;
static U8 				u8FocusMonth;
static U8 				u8CalX, u8CalY;

static CSAPP_Applet_t	CSApp_Calendar_Applets;

static int Calendar_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

static U8 _GetLeap( U16 year )
{
	if (year % 400 == 0)
		return 1;

	else if (year % 100 == 0)
		return 0;

	else if (year % 4 == 0)
		return 1;

	return 0;
}

U8 DVB_GetDaysofMonth( U16 u16SolarYear, U8 u8SolarMonth )
{
	if (_GetLeap(u16SolarYear) != 0 && u8SolarMonth == 2)
	{
		return SolarCal[u8SolarMonth - 1] + 1;
	}

	if (u8SolarMonth>12)
		return 0;

	return SolarCal[u8SolarMonth - 1];
}

#if 0
static U16 _GetDaysofYear( U16 u16SolarYear )
{
	if (_GetLeap(u16SolarYear) != 0)
		return 366;
	else
		return 365;
}
#endif

U8 DVB_SolarCalendar( U16 u16Year, U8 u8Month, U8 u8Day )
{
	U8		u8Week, u8idx;
	U16 	u16TheDayInYear, u16TheDayInPeriod;

	u16TheDayInYear = u8Day;

	for (u8idx = 0; u8idx<u8Month - 1; u8idx++)
	{
		if (u8idx == 1)
		{
			if ( _GetLeap(u16Year)==1)
			u16TheDayInYear++;
		}

		u16TheDayInYear += SolarCal[u8idx];
	}

	u16TheDayInPeriod = u16Year - 1 + (u16Year - 1) / 4 - (u16Year - 1) / 100 + (u16Year - 1) / 400 + u16TheDayInYear;

	u8Week = u16TheDayInPeriod % 7;

	return (u8Week);
}

void calender_draw_title(HDC hdc)
{
	U16 	u16YearX, u16YearY;
	RECT	lrect;
	char	acTemp_Str[100];

	memset(acTemp_Str, 0x00, 100);
	
	u16YearX = u16StartX + CALENDER_ITEM_X_OFFSET;
	u16YearY = u16StartY + 8;

	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	MV_FillBox( hdc, ScalerWidthPixel(u16StartX),ScalerHeigthPixel(u16StartY), ScalerWidthPixel(u16TotalW),ScalerHeigthPixel(CALENDER_HEAD_HEIGHT_OF_YEAR) );

	lrect.left = u16StartX;
	lrect.right = lrect.left + u16TotalW;
	lrect.top = u16StartY + 10;
	lrect.bottom = lrect.top + CALENDER_HEAD_HEIGHT_OF_YEAR;

	sprintf( acTemp_Str, "%d - %s", u16CurrentYear, CS_MW_LoadStringByIdx(enYearMonth[u8CurrentMonth-1]));
	CS_MW_DrawText (hdc, acTemp_Str, -1, &lrect, DT_CENTER | DT_VCENTER);
}

void calender_draw_item(HDC hdc, U8 u8Date, BOOL b8Focus)
{
	U16 	u16ContentX, u16ContentY, u16ContentW, u16ContentH;
	U16 	u16NumberX, u16NumberY;
	U8 		u8idx;
	char	acTemp_Str[100];

	memset(acTemp_Str, 0x00, 100);

	u8CalX = DVB_SolarCalendar(u16CurrentYear, u8CurrentMonth, u8Date);
	for( u8idx = 1 ; u8idx < 8 ; u8idx++ )//check the 1st satday
		if( DVB_SolarCalendar(u16CurrentYear, u8CurrentMonth, u8idx) == 6 )
			break;

	if( u8Date <= u8idx )
		u8CalY = 0;
	else
	{
		if( ( u8Date-u8idx ) % 7 == 0 )
			u8CalY = ( u8Date-u8idx ) / 7;
		else
			u8CalY = ( u8Date-u8idx ) / 7 + 1;
	}
	
	u16ContentX = u16StartX + CALENDER_ITEM_X_OFFSET + ( u8CalX * CALENDER_ITEM_WIDTH );
	u16ContentY = u16StartY + CALENDER_HEAD_HEIGHT + CALENDER_ITEM_BORDER + ( u8CalY * CALENDER_ITEM_HEIGHT );
	u16ContentW = CALENDER_ITEM_WIDTH;
	u16ContentH = CALENDER_ITEM_HEIGHT;

	MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if(b8Focus)
	{
		MV_FillBox( hdc, ScalerWidthPixel(u16ContentX - FOCUS_REC_OFFSET_X), ScalerHeigthPixel(u16ContentY - FOCUS_REC_OFFSET_Y + 2), ScalerWidthPixel(u16ContentW),ScalerHeigthPixel(2) );
		MV_FillBox( hdc, ScalerWidthPixel(u16ContentX - FOCUS_REC_OFFSET_X), ScalerHeigthPixel(u16ContentY - FOCUS_REC_OFFSET_Y + 2), ScalerWidthPixel(2),ScalerHeigthPixel(u16ContentH) );
		MV_FillBox( hdc, ScalerWidthPixel(u16ContentX + u16ContentW-2 - FOCUS_REC_OFFSET_X), ScalerHeigthPixel(u16ContentY - FOCUS_REC_OFFSET_Y + 2), ScalerWidthPixel(2),ScalerHeigthPixel(u16ContentH) );
		MV_FillBox( hdc, ScalerWidthPixel(u16ContentX - FOCUS_REC_OFFSET_X), ScalerHeigthPixel(u16ContentY + u16ContentH-2 - FOCUS_REC_OFFSET_Y + 2), ScalerWidthPixel(u16ContentW),ScalerHeigthPixel(2) );
	}

	if(u8CalX==0)
		SetTextColor(hdc,MVAPP_ORANGE_COLOR);
	else
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	
	if(u8Date<10)
		u16NumberX = u16ContentX + (u16ContentW-22)/2;
	else
		u16NumberX = u16ContentX + (u16ContentW-44)/2;

	u16NumberY = u16ContentY + 2;

	sprintf(acTemp_Str, "%d", u8Date);
	CS_MW_TextOut(hdc,ScalerWidthPixel(u16NumberX - 16),ScalerHeigthPixel(u16NumberY), acTemp_Str);

}

void calender_draw_background(HDC hdc)
{
	U8 		u8idx;
	U16 	u16WeeklyX, u16WeeklyY;

	MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	MV_FillBox( hdc, ScalerWidthPixel(u16StartX),ScalerHeigthPixel(u16StartY + CALENDER_HEAD_HEIGHT_OF_YEAR), ScalerWidthPixel(u16TotalW),ScalerHeigthPixel(CALENDER_HEAD_HEIGHT_OF_WEEKLY) );
	
	for( u8idx = 0 ; u8idx < 7 ; u8idx++)
	{
		u16WeeklyX = 70;
		if( CALENDER_ITEM_WIDTH >= u16WeeklyX )
			u16WeeklyX = u16StartX + CALENDER_ITEM_X_OFFSET + (U16)(u8idx)*CALENDER_ITEM_WIDTH + (CALENDER_ITEM_WIDTH-u16WeeklyX)/2;
		else
			u16WeeklyX = u16StartX + CALENDER_ITEM_X_OFFSET + (U16)(u8idx)*CALENDER_ITEM_WIDTH;

		u16WeeklyY = u16StartY + CALENDER_HEAD_HEIGHT - 28;

		CS_MW_TextOut(hdc,ScalerWidthPixel(u16WeeklyX),ScalerHeigthPixel(u16WeeklyY), CS_MW_LoadStringByIdx(enWeeklyDay[u8idx]));
	}
}

void calender_draw_all(HDC hdc)
{
	U8 		u8idx;

	// clean working area
	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(u16StartX),ScalerHeigthPixel(u16StartY + CALENDER_HEAD_HEIGHT), ScalerWidthPixel(u16TotalW),ScalerHeigthPixel(CALENDER_BODY_HEIGHT) );

	calender_draw_title(hdc);

	for(u8idx = 1 ; u8idx <= u8LastDay ; u8idx++)
	{
		if(( u8idx == u8CurrentDate ) && ( u8CurrentMonth == u8FocusMonth ) && ( u16CurrentYear == u16FocusYear ))
		{
			//printf("\n u16CurrentYear : %04d, u8CurrentMonth : %02d, u8CurrentDate : %02d ==>>> u16FocusYear : %04d, u8FocusMonth : %02d\n", u16CurrentYear, u8CurrentMonth, u8CurrentDate, u16FocusYear, u8FocusMonth);
    		calender_draw_item(hdc, u8idx, TRUE);
		}
    	else
    		calender_draw_item(hdc, u8idx, FALSE);
	}
}

CSAPP_Applet_t	CSApp_Calendar(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Calendar_Applets = CSApp_Applet_Error;
    
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
	CreateInfo.spCaption = "calendar";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Calendar_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = CSAPP_BLACK_COLOR;
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
	return CSApp_Calendar_Applets;
    
}


static int Calendar_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	struct tm 			tm_time;
	struct timespec 	time_value;

	switch(message)
	{
		case MSG_CREATE:
			u16TotalH = ( 6 * CALENDER_ITEM_HEIGHT ) + CALENDER_HEAD_HEIGHT_OF_YEAR + CALENDER_HEAD_HEIGHT_OF_WEEKLY;
    		u16TotalW = ( 2 * CALENDER_X_OFFSET ) + ( 7 * CALENDER_ITEM_WIDTH );
			u16StartX = S_X + 10;
    		u16StartY = S_Y + 10;

			clock_gettime(CLOCK_REALTIME, &time_value);
			memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

			u16CurrentYear = tm_time.tm_year+1900;
			u8CurrentMonth = tm_time.tm_mon + 1;
			u8CurrentDate = tm_time.tm_mday;

			u16FocusYear = u16CurrentYear;
			u8FocusMonth = u8CurrentMonth;
				
			u8LastDay = DVB_GetDaysofMonth(u16CurrentYear, u8CurrentMonth);
			break;
			
		case MSG_PAINT:
			{
				CS_MW_SetSmallWindow(ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_TOP),ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_PIG_DY));
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_CALENDAR, 0);

				hdc=BeginPaint(hwnd);
				MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
				MV_FillBox( hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_Y), ScalerWidthPixel(u16TotalW + 20), ScalerHeigthPixel(u16TotalH + 20) );
				//MV_FillBox( hdc, ScalerWidthPixel(S_X - 10), ScalerHeigthPixel(S_Y - 10), ScalerWidthPixel(460), ScalerHeigthPixel(320) );
				calender_draw_background(hdc);
				calender_draw_all(hdc);

				FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X), ScalerHeigthPixel(S_T_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X),ScalerHeigthPixel(S_T_Y), CS_MW_LoadStringByIdx(CSAPP_STR_PREV_YEAR));

				FillBoxWithBitmap (hdc, ScalerWidthPixel(S_X + 300), ScalerHeigthPixel(S_T_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(S_T_X + 300),ScalerHeigthPixel(S_T_Y), CS_MW_LoadStringByIdx(CSAPP_STR_NEXT_YEAR));

				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							char 	ShellCommand[256];
							
							memset( ShellCommand, 0x00, 256 );
							
							sprintf(ShellCommand, "reboot");
							system( ShellCommand );	
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_ESC:
					CS_MW_SetNormalWindow();
					CSApp_Calendar_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					CS_MW_SetNormalWindow();
					CSApp_Calendar_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Calendar_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_LEFT:
					if( u8CurrentMonth > 1)
						u8CurrentMonth--;
					else
						if(u16CurrentYear>CAL_FIRSTYEAR)
						{
							u16CurrentYear--;
							u8CurrentMonth = 12;
						}

					u8LastDay = DVB_GetDaysofMonth(u16CurrentYear, u8CurrentMonth);
					hdc = BeginPaint(hwnd);
					calender_draw_all(hdc);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_RIGHT:
					if(u8CurrentMonth<12)
						u8CurrentMonth++;
					else
						if(u16CurrentYear<CAL_LASTYEAR)
						{
							u16CurrentYear++;
							u8CurrentMonth = 1;
						}
						
				 	u8LastDay = DVB_GetDaysofMonth(u16CurrentYear, u8CurrentMonth);
				 	hdc = BeginPaint(hwnd);
					calender_draw_all(hdc);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_RED:
					if(u16CurrentYear>CAL_FIRSTYEAR)
						u16CurrentYear--;
					else
						u16CurrentYear=CAL_LASTYEAR;

					u8LastDay = DVB_GetDaysofMonth(u16CurrentYear, u8CurrentMonth);
					hdc = BeginPaint(hwnd);
					calender_draw_all(hdc);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_GREEN:
					break;

				case CSAPP_KEY_BLUE:
					if(u16CurrentYear<CAL_LASTYEAR)
						u16CurrentYear++;
					else
						u16CurrentYear=CAL_FIRSTYEAR;

					u8LastDay = DVB_GetDaysofMonth(u16CurrentYear, u8CurrentMonth);
					hdc = BeginPaint(hwnd);
					calender_draw_all(hdc);
					EndPaint(hwnd,hdc);
					break;
					
				default:
					break;
			}
			break;

		case MSG_CLOSE:
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
			
		default:
			break;		
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}








