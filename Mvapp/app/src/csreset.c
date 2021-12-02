#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

static CSAPP_Applet_t	CSApp_Reset_Applets;
static BOOL				reset_warning_status = FALSE;
static BOOL				delete_warning_status = FALSE;
static BOOL				Satdelete_warning_status = FALSE;
static BOOL				Favdelete_warning_status = FALSE;
static BOOL				u8YES_NO = FALSE;
static U8				u8Delete_SatIndex = 0;
static U8				u8Delete_FavIndex = 0;
static U8				u8Delete_Type = 0;

static int Reset_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void reset_draw_key( HWND hwnd )
{
	HDC		hdc;
	RECT	Temp_Rect;
	
	hdc = MV_BeginPaint(hwnd);
	
	if ( u8YES_NO )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(YES_ICON_X - 30), ScalerHeigthPixel(NO_ICON_Y - WARNING_OUT_GAP*2), ScalerWidthPixel(YES_ICON_DX), ScalerHeigthPixel(RESET_WARNING_DY), &MV_BMP[MVBMP_CHLIST_SELBAR]);
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(NO_ICON_X - 30), ScalerHeigthPixel(NO_ICON_Y - WARNING_OUT_GAP*2), ScalerWidthPixel(YES_ICON_DX), ScalerHeigthPixel(RESET_WARNING_DY) );
	} else {
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(YES_ICON_X - 30), ScalerHeigthPixel(NO_ICON_Y - WARNING_OUT_GAP*2), ScalerWidthPixel(YES_ICON_DX), ScalerHeigthPixel(RESET_WARNING_DY) );
		FillBoxWithBitmap(hdc,ScalerWidthPixel(NO_ICON_X - 30), ScalerHeigthPixel(NO_ICON_Y - WARNING_OUT_GAP*2), ScalerWidthPixel(YES_ICON_DX), ScalerHeigthPixel(RESET_WARNING_DY), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	}
	
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	Temp_Rect.top 	= NO_ICON_Y - WARNING_OUT_GAP*2;
	Temp_Rect.bottom	= Temp_Rect.top + RESET_WARNING_DY;
	Temp_Rect.left	= YES_ICON_X - 30;
	Temp_Rect.right	= Temp_Rect.left + YES_ICON_DX;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OK), -1, &Temp_Rect, DT_CENTER);
	
	Temp_Rect.top 	= NO_ICON_Y - WARNING_OUT_GAP*2;
	Temp_Rect.bottom	= Temp_Rect.top + RESET_WARNING_DY;
	Temp_Rect.left	= NO_ICON_X - 30;
	Temp_Rect.right	= Temp_Rect.left + YES_ICON_DX;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL), -1, &Temp_Rect, DT_CENTER);
	
	MV_EndPaint(hwnd,hdc);
}

void Reset_Warning_Close( HWND hwnd )
{
	HDC		hdc;

	reset_warning_status = FALSE;
	delete_warning_status = FALSE;
	Satdelete_warning_status = FALSE;
	Favdelete_warning_status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(RESET_WARNING_X - 100), ScalerHeigthPixel(RESET_WARNING_Y), ScalerWidthPixel(RESET_WARNING_DX + 200), ScalerHeigthPixel(RESET_WARNING_DY + RESET_WARNING_MSG_DY), &WarningCapture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&WarningCapture_bmp);
}

void Reset_Warning_Draw(HWND hwnd, EN_RESET_MSG en_Kind)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	Reset_Warning_Close(hwnd);
	
	if ( en_Kind == EN_RESET_MSG_RESET )
		reset_warning_status = TRUE;
	else if ( en_Kind == EN_RESET_MSG_DELETE )
		delete_warning_status = TRUE;
	else if ( en_Kind == EN_RESET_MSG_SAT_DELETE )
		Satdelete_warning_status = TRUE;
	else if ( en_Kind == EN_RESET_MSG_FAV_DELETE )
		Favdelete_warning_status = TRUE;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(RESET_WARNING_X - 100), ScalerHeigthPixel(RESET_WARNING_Y), ScalerWidthPixel(RESET_WARNING_DX + 200), ScalerHeigthPixel(RESET_WARNING_DY + RESET_WARNING_MSG_DY), &WarningCapture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(RESET_WARNING_X - 100), ScalerHeigthPixel(RESET_WARNING_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(RESET_WARNING_X + 100 + RESET_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(RESET_WARNING_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(RESET_WARNING_X - 100), ScalerHeigthPixel(RESET_WARNING_Y + (RESET_WARNING_DY + RESET_WARNING_MSG_DY) - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(RESET_WARNING_X + 100 + RESET_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(RESET_WARNING_Y + (RESET_WARNING_DY + RESET_WARNING_MSG_DY) - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(RESET_WARNING_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth - 100), ScalerHeigthPixel(RESET_WARNING_Y),ScalerWidthPixel(RESET_WARNING_DX + 200 - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(RESET_WARNING_DY + RESET_WARNING_MSG_DY));
	FillBox(hdc,ScalerWidthPixel(RESET_WARNING_X - 100), ScalerHeigthPixel(RESET_WARNING_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(RESET_WARNING_DX + 200),ScalerHeigthPixel(RESET_WARNING_DY + RESET_WARNING_MSG_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

/*	
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(RESET_WARNING_X), ScalerHeigthPixel(RESET_WARNING_Y), ScalerWidthPixel(RESET_WARNING_DX), ScalerHeigthPixel(RESET_WARNING_DY + RESET_WARNING_MSG_DY) );

	MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(RESET_WARNING_X + WARNING_OUT_GAP), ScalerHeigthPixel(RESET_WARNING_Y + WARNING_OUT_GAP), ScalerWidthPixel(RESET_WARNING_DX - WARNING_OUT_GAP*2), ScalerHeigthPixel(RESET_WARNING_DY) );
*/

	Temp_Rect.top 	= RESET_WARNING_Y + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + RESET_WARNING_DY;
	Temp_Rect.left	= RESET_WARNING_X + WARNING_OUT_GAP - 100;
	Temp_Rect.right	= Temp_Rect.left + (RESET_WARNING_DX - WARNING_OUT_GAP*2) + 200;
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_WARNING);
//	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_WARNING), -1, &Temp_Rect, DT_CENTER);	

	if ( en_Kind == EN_RESET_MSG_RESET || en_Kind == EN_RESET_MSG_DELETE || en_Kind == EN_RESET_MSG_SAT_DELETE || en_Kind == EN_RESET_MSG_FAV_DELETE ) {
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(RESET_WARNING_X + WARNING_OUT_GAP - 100), ScalerHeigthPixel(RESET_WARNING_Y + (WARNING_OUT_GAP*2) + RESET_WARNING_DY), ScalerWidthPixel(RESET_WARNING_DX - WARNING_OUT_GAP*2 + 200), ScalerHeigthPixel(RESET_WARNING_MSG_DY - WARNING_OUT_GAP*3) );
	} else {
		MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(RESET_WARNING_X + WARNING_OUT_GAP - 100), ScalerHeigthPixel(RESET_WARNING_Y + (WARNING_OUT_GAP*2) + RESET_WARNING_DY), ScalerWidthPixel(RESET_WARNING_DX - WARNING_OUT_GAP*2 + 200), ScalerHeigthPixel(RESET_WARNING_MSG_DY - WARNING_OUT_GAP*3) );
	}

	Temp_Rect.top 	= RESET_WARNING_MSG_Y + RESET_WARNING_DY;
	Temp_Rect.bottom	= Temp_Rect.top + (RESET_WARNING_MSG_DY - WARNING_OUT_GAP*3);
	Temp_Rect.left	= RESET_WARNING_X + WARNING_OUT_GAP - 100;
	Temp_Rect.right	= Temp_Rect.left + (RESET_WARNING_DX - WARNING_OUT_GAP*2) + 200;
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if ( en_Kind == EN_RESET_MSG_RESET )
	{
		Temp_Rect.top 	= Temp_Rect.top - WARNING_OUT_GAP*3;
		Temp_Rect.bottom	= Temp_Rect.top + WARNING_OUT_GAP*3*6;
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_FACTORY_DEFAULT_HELP), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
		//Temp_Rect.top 	= Temp_Rect.top + WARNING_OUT_GAP*3*4;
		//Temp_Rect.bottom	= Temp_Rect.top + WARNING_OUT_GAP*3;
		//CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SURE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
	}
	else if ( en_Kind == EN_RESET_MSG_DELETE )
	{
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CH_DELETE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
		Temp_Rect.top 	= Temp_Rect.top + WARNING_OUT_GAP*3;
		Temp_Rect.bottom	= Temp_Rect.top + WARNING_OUT_GAP*3;
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SURE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
	} 
	else if ( en_Kind == EN_RESET_MSG_SAT_DELETE )
	{
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SAT_CH_DELETE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
		Temp_Rect.top 	= Temp_Rect.top + WARNING_OUT_GAP*3;
		Temp_Rect.bottom	= Temp_Rect.top + WARNING_OUT_GAP*3;
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SURE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
	}
	else if ( en_Kind == EN_RESET_MSG_FAV_DELETE )
	{
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_FAV_CH_DELETE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
		Temp_Rect.top 	= Temp_Rect.top + WARNING_OUT_GAP*3;
		Temp_Rect.bottom	= Temp_Rect.top + WARNING_OUT_GAP*3;
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SURE), -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
	}
	else
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_FACTORY_DEFAULT_ON), -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);
	
	MV_EndPaint(hwnd,hdc);

	if ( en_Kind == EN_RESET_MSG_RESET || en_Kind == EN_RESET_MSG_DELETE || en_Kind == EN_RESET_MSG_SAT_DELETE || en_Kind == EN_RESET_MSG_FAV_DELETE )
		reset_draw_key(hwnd);
}

CSAPP_Applet_t	CSApp_Reset(void)
{
    int   			BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   			msg;
  	HWND  			hwndMain;
	MAINWINCREATE	CreateInfo;

	CSApp_Reset_Applets = CSApp_Applet_Error;
    
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
	CreateInfo.spCaption = "csreset window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Reset_Msg_cb;
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
	return CSApp_Reset_Applets;
    
}


static int Reset_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	RECT 				rc1;

	switch(message)
	{
		case MSG_CREATE:
			break;
		case MSG_PAINT:
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_FAC_RESET);
				
				hdc=BeginPaint(hwnd);

				rc1.left = ScalerWidthPixel(RESET_WARNING_X); 
				rc1.top = ScalerHeigthPixel(RESET_WARNING_Y + WARNING_OUT_GAP); 
				rc1.right = ScalerWidthPixel(RESET_WARNING_X + RESET_WARNING_DX); 
				rc1.bottom = ScalerHeigthPixel(RESET_WARNING_Y + RESET_WARNING_DY + WARNING_OUT_GAP);
				MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_WARNING);

				rc1.left = ScalerWidthPixel(RESET_WARNING_MSG_X); 
				rc1.top = ScalerHeigthPixel(RESET_WARNING_MSG_Y); 
				rc1.right = ScalerWidthPixel(RESET_WARNING_MSG_X + RESET_WARNING_MSG_DX); 
				rc1.bottom = ScalerHeigthPixel(RESET_WARNING_MSG_Y + RESET_WARNING_MSG_DY);

				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				InflateRect (&rc1, -1, -1);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_FACTORY_DEFAULT_HELP), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);

				FillBoxWithBitmap(hdc,ScalerWidthPixel(YES_ICON_X), ScalerHeigthPixel(YES_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(YES_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(YES_ICON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_OK));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(NO_ICON_X), ScalerHeigthPixel(NO_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(NO_ICON_X + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth + 10),	ScalerHeigthPixel(NO_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL));

				MV_TOOLS_draw_help_banner(hdc, EN_ITEM_FOCUS_FAC_RESET);
				EndPaint(hwnd,hdc);
			}
		return 0;

		case MSG_KEYDOWN:
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					if ( u8Delete_Type == EN_RESET_LIST_SAT )
					{
						u8Delete_SatIndex = MV_Get_PopUp_Window_Result();
						Reset_Warning_Draw(hwnd, EN_RESET_MSG_SAT_DELETE);
					} else if ( u8Delete_Type == EN_RESET_LIST_FAV ) {
						u8Delete_FavIndex = MV_Get_PopUp_Window_Result();
						Reset_Warning_Draw(hwnd, EN_RESET_MSG_FAV_DELETE);
					}
					
					u8Delete_Type = EN_RESET_LIST_MAX;
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
						if ( reset_warning_status == TRUE || delete_warning_status == TRUE || Satdelete_warning_status == TRUE || Favdelete_warning_status == TRUE )
						{
							Reset_Warning_Close( hwnd );
						} else {
							CSApp_Reset_Applets = CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
						
					case CSAPP_KEY_MENU:
						if ( reset_warning_status == TRUE || delete_warning_status == TRUE || Satdelete_warning_status == TRUE || Favdelete_warning_status == TRUE )
						{
							Reset_Warning_Close( hwnd );
						} else {
							CSApp_Reset_Applets = b8Last_App_Status;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_IDLE:
						CSApp_Reset_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_TV_AV:
						ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						break;
							
					case CSAPP_KEY_LEFT:
					case CSAPP_KEY_RIGHT:
						if ( reset_warning_status == TRUE || delete_warning_status == TRUE || Satdelete_warning_status == TRUE || Favdelete_warning_status == TRUE )
						{
							if (u8YES_NO == TRUE)
								u8YES_NO = FALSE;
							else
								u8YES_NO = TRUE;
							
							reset_draw_key(hwnd);
						}
						break;
						
					case CSAPP_KEY_RED:
						Reset_Warning_Draw(hwnd, EN_RESET_MSG_RESET);
						break;

					case CSAPP_KEY_BLUE:
						CSApp_Reset_Applets = b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_F2:
						{
							int						i = 0;
							RECT					smwRect;
							stPopUp_Window_Contents stContents;
							U8						stCount = 0;
							char					Temp_Str[32];

							u8Delete_Type = EN_RESET_LIST_FAV;
							memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
							
							for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
							{
								memset( Temp_Str, 0x00, 32 );
								MV_DB_Get_Favorite_Name(Temp_Str, i);
								sprintf(stContents.Contents[i], "%s : %d", Temp_Str, MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_TV_LIST_ID, i) + MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_RADIO_LIST_ID, i));
								stCount++;
							}
							
							if ( i > 8 )
								smwRect.top = RESET_WARNING_Y - 50;
							else
								smwRect.top = RESET_WARNING_Y;
							
							smwRect.left = RESET_WARNING_X - 100;
							smwRect.right = smwRect.left + RESET_WARNING_DX + 200;
							smwRect.bottom = smwRect.top + RESET_WARNING_DY + RESET_WARNING_MSG_DY;

							stContents.u8TotalCount = MV_MAX_FAV_KIND - 1;
							stContents.u8Focus_Position = 0 ;
							
							MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
						}
						break;

					case CSAPP_KEY_GREEN:
						{
							int						i = 0;
							RECT					smwRect;
							MV_stSatInfo			Temp_SatData;
							stPopUp_Window_Contents stContents;
							U8						stCount = 0;

							u8Delete_Type = EN_RESET_LIST_SAT;
							memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
							
							for ( i = 0 ; i < MV_SAT_MAX ; i++ )
							{
								if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
								{
									MV_GetSatelliteData_ByIndex(&Temp_SatData, i);
									sprintf(stContents.Contents[stCount], "%s : %d - %d Ch.", Temp_SatData.acSatelliteName, MV_Get_TVServiceCount_at_Sat(Temp_SatData.u8SatelliteIndex), MV_Get_RDServiceCount_at_Sat(Temp_SatData.u8SatelliteIndex));
									stCount++;
								}
							}
							
							if ( i > 8 )
								smwRect.top = RESET_WARNING_Y - 50;
							else
								smwRect.top = RESET_WARNING_Y;
							
							smwRect.left = RESET_WARNING_X - 100;
							smwRect.right = smwRect.left + RESET_WARNING_DX + 200;
							smwRect.bottom = smwRect.top + RESET_WARNING_DY + RESET_WARNING_MSG_DY;

							stContents.u8TotalCount = stCount;
							stContents.u8Focus_Position = 0 ;
							
							MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
						}
						//Reset_Warning_Draw(hwnd, EN_RESET_MSG_SAT_DELETE);
						break;

					case CSAPP_KEY_YELLOW:
						Reset_Warning_Draw(hwnd, EN_RESET_MSG_DELETE);
						break;

					case CSAPP_KEY_ENTER:
						if ( reset_warning_status == TRUE )
						{
							if (u8YES_NO == TRUE)
							{
								MV_MW_StopService();
								
								Reset_Warning_Close( hwnd );
								Reset_Warning_Draw(hwnd, EN_RESET_MSG_OFF);
								CS_DB_ResetDatabase();
								CS_TIMER_Reset();
								CS_MW_ValidCurrentSetting();
								CS_MW_SetVideoDefinition(CS_MW_GetVideoDefinition()); /* By KB Kim 2011.06.07 */
								
								u8Glob_Sat_Focus = 0;
								u8Glob_TP_Focus = 0;

								//MV_Submenu_Clear();
								CS_MW_Font_Creation(0);
								MV_MW_StartService(CS_DB_GetCurrentServiceIndex());
								MainMenuSetSubtApplet(CSAPP_MAINMENU_INSTALL, 0);
								CSApp_Reset_Applets = CSApp_Applet_Desktop;
								SendMessage(hwnd,MSG_CLOSE,0,0);
							} else {
								Reset_Warning_Close( hwnd );
							}
						} 
						else if ( delete_warning_status == TRUE )
						{
							if (u8YES_NO == TRUE)
							{
								Reset_Warning_Close( hwnd );
								Reset_Warning_Draw(hwnd, EN_RESET_MSG_OFF);
								CS_DB_All_Ch_Delete();
								CS_MW_ValidCurrentSetting();
								MV_MW_StopService();
								MainMenuSetSubtApplet(CSAPP_MAINMENU_INSTALL, 0);
								CSApp_Reset_Applets = CSApp_Applet_MainMenu;
								SendMessage(hwnd,MSG_CLOSE,0,0);
							} else {
								Reset_Warning_Close( hwnd );
							}
						}
						else if ( Satdelete_warning_status == TRUE )
						{
							if (u8YES_NO == TRUE)
							{
								Reset_Warning_Close( hwnd );
								Reset_Warning_Draw(hwnd, EN_RESET_MSG_OFF);
								MV_SetCHData_DEL_BySatellite(u8Delete_SatIndex);
								Reset_Warning_Close( hwnd );
							} else {
								Reset_Warning_Close( hwnd );
							}
						}
						else if ( Favdelete_warning_status == TRUE )
						{
							if (u8YES_NO == TRUE)
							{
								Reset_Warning_Close( hwnd );
								Reset_Warning_Draw(hwnd, EN_RESET_MSG_OFF);
								MV_SetCHIndex_DEL_ByFavorite(u8Delete_FavIndex);
								Reset_Warning_Close( hwnd );
							} else {
								Reset_Warning_Close( hwnd );
							}
						}
						break;
					default:
						break;
				}
			break;

		case MSG_CLOSE:
			UnloadBitmap(&WarningCapture_bmp);
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
		default:
			break;		
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}








