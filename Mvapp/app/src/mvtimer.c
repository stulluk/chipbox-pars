#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "mvtimer.h"
#include "csreset.h"

static CSAPP_Applet_t			CSApp_Timer_Applets;
static tCS_TIMER_JobInfo		stTimerJob;
static U8						Current_Item = 0;
static MV_stServiceInfo			ServiceData;

static int Timer_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Timer_Draw_SelectBar(HDC hdc, int y_gap)
{
	int mid_width = TIMER_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = TIMER_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
}

void MV_Timer_Draw_Job_Item(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 						y_gap = TIMER_WINDOW_CONT_Y + TIMER_WINDOW_ITEM_DY * esItem;
	RECT						TmpRect;
	char						temp_str[256];
	tCS_DT_Date					Temp_YMD;
	tCS_DT_Time					Temp_HM;
	tCS_DB_ServiceManageData 	item_data;
	
	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Timer_Draw_SelectBar(hdc, y_gap);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(TIMER_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(TIMER_WINDOW_CONT_DX),ScalerHeigthPixel(TIMER_WINDOW_ITEM_DY) );
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_WINDOW_ITEM_X + TIMER_WINDOW_CONT_DX - ( MV_BMP[MVBMP_Y_ENTER].bmWidth + 10 )), ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight), &MV_BMP[MVBMP_Y_ENTER]);
	
	memset( temp_str, 0x00, 256);

	CS_TIMER_GetJobInfo(esItem, &stTimerJob);

	if ( stTimerJob.CS_Timer_Status == eCS_TIMER_Enable )
	{
		TmpRect.left	= TIMER_WINDOW_ITEM_NO_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NO_DX;
		TmpRect.top		= y_gap + 4;
		TmpRect.bottom	= TmpRect.top + TIMER_WINDOW_ITEM_DY;
		sprintf(temp_str, "%d", esItem + 1);
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_NAME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NAME_DX;
		CS_DB_GetCurrentList_ServiceData(&item_data, stTimerJob.CS_Wakeup_Service.Service_Index);
		MV_DB_GetServiceDataByIndex(&ServiceData, item_data.Service_Index);		
		sprintf(temp_str, "%s", ServiceData.acServiceName);
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);

		TmpRect.left	= TIMER_WINDOW_ITEM_DATE_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_DATE_DX;
		Temp_YMD = CS_DT_MJDtoYMD(stTimerJob.CS_Begin_MDJ);
		sprintf(temp_str, "%02d/%02d/%04d", Temp_YMD.day, Temp_YMD.month, Temp_YMD.year);
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_TIME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_TIME_DX;
		Temp_HM = CS_DT_UTCtoHM(stTimerJob.CS_Begin_UTC);
		sprintf(temp_str, "%02d:%02d", Temp_HM.hour, Temp_HM.minute);
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);

		TmpRect.left	= TIMER_WINDOW_ITEM_TYPE_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_TYPE_DX;
		if (stTimerJob.CS_Timer_Type == eCS_TIMER_Wakeup)
			sprintf(temp_str, "Turn ON");
		else if (stTimerJob.CS_Timer_Type == eCS_TIMER_Sleep)
			sprintf(temp_str, "Turn OFF");
		else if (stTimerJob.CS_Timer_Type == eCS_TIMER_Record)
			sprintf(temp_str, "Record");
		else 
			sprintf(temp_str, CS_MW_LoadStringByIdx(CSAPP_STR_DURATION));
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
	} else {	
		TmpRect.left	= TIMER_WINDOW_ITEM_NO_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NO_DX;
		TmpRect.top		= y_gap + 4;
		TmpRect.bottom	= TmpRect.top + TIMER_WINDOW_ITEM_DY;
		sprintf(temp_str, "%d", esItem + 1);
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
		
		TmpRect.left	= TIMER_WINDOW_ITEM_NAME_X;
		TmpRect.right	= TmpRect.left + TIMER_WINDOW_ITEM_NAME_DX;	
		sprintf(temp_str, "Off");
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_LEFT);
	}		 
}

void MV_Timer_Draw_Job_List(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < kCS_TIMER_MAX_NO_OF_JOB - 1 ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Timer_Draw_Job_Item(hdc, FOCUS, i);
		} else {
			MV_Timer_Draw_Job_Item(hdc, UNFOCUS, i);
		}
	}
}

void MV_Timer_Draw_Window(HDC hdc)
{
	RECT	rc1;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_WINDOW_X + TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y + TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(TIMER_WINDOW_Y),ScalerWidthPixel(TIMER_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(TIMER_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(TIMER_WINDOW_X), ScalerHeigthPixel(TIMER_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(TIMER_WINDOW_DX),ScalerHeigthPixel(TIMER_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = TIMER_WINDOW_TITLE_Y;
	rc1.left = TIMER_WINDOW_ITEM_X;
	rc1.bottom = TIMER_WINDOW_TITLE_Y + TIMER_WINDOW_ITEM_DY;
	rc1.right = rc1.left + TIMER_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_TIMER);
}

CSAPP_Applet_t	CSApp_Timer(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Timer_Applets = CSApp_Applet_Error;
    
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
	CreateInfo.spCaption = "Timer window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Timer_Msg_cb;
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
	return CSApp_Timer_Applets;
    
}


static int Timer_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;

	switch(message)
	{
		case MSG_CREATE:
			Current_Item = 0;
			break;
			
		case MSG_PAINT:
			{
				hdc=BeginPaint(hwnd);

				MV_Timer_Draw_Window(hdc);
				MV_Timer_Draw_Job_List(hdc);
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
			if ( MV_Check_Timer_Window_Status() == TRUE )
			{
				MV_Timer_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_Timer_Save_Status() == TRUE )
						{
							if ( MV_Timer_Upload_Data() == eCS_TIMER_NO_ERROR )
							{
								//printf("\n SAVE OK =====\n");
								MV_Timer_Close_Window(hwnd);
								
								hdc = BeginPaint(hwnd);
								MV_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
								EndPaint(hwnd,hdc);
							} else {
								printf("\n SAVE FAIL =====\n");
							}
						}
					} else {
						MV_Timer_Close_Window(hwnd);
					}
				}
				break;
				
			}	
			
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_Timer_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_RED:
					break;

				case CSAPP_KEY_GREEN:
					break;

				case CSAPP_KEY_BLUE:
					break;

				case CSAPP_KEY_UP:
					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Job_Item(hdc, UNFOCUS, Current_Item);
					
					if ( Current_Item == 0 )
						Current_Item = kCS_TIMER_MAX_NO_OF_JOB - 2;
					else
						Current_Item--;

					MV_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_DOWN:
					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Job_Item(hdc, UNFOCUS, Current_Item);
					
					if ( Current_Item == kCS_TIMER_MAX_NO_OF_JOB - 2 )
						Current_Item = 0;
					else
						Current_Item++;

					MV_Timer_Draw_Job_Item(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_ENTER:
					MV_Timer_Draw_Modify_Window(hwnd, Current_Item);
					break;
					
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Timer_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
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








