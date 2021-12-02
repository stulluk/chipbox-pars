#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csbackup.h"

static CSAPP_Applet_t	CSApp_Restore_Applets;
static EN_RESTORE_MSG	restore_status = EN_RESTORE_MSG_MAX;

static int Restore_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

CSAPP_Applet_t	CSApp_Restore(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Restore_Applets = CSApp_Applet_Error;
    
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
	CreateInfo.spCaption = "resotre window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Restore_Msg_cb;
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
	return CSApp_Restore_Applets;
    
}

static int Restore_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	RECT 				rc1;

	switch(message)
	{
		case MSG_CREATE:
			break;
			
		case MSG_PAINT:
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_RESTORE);
				
				hdc=BeginPaint(hwnd);

				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_WARNING_X), ScalerHeigthPixel(BACKUP_WARNING_Y), ScalerWidthPixel(BACKUP_WARNING_DX), ScalerHeigthPixel(BACKUP_WARNING_DY), &MV_BMP[MVBMP_CHLIST_SELBAR]);
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				rc1.left = ScalerWidthPixel(BACKUP_WARNING_X); 
				rc1.top = ScalerHeigthPixel(BACKUP_WARNING_Y); 
				rc1.right = ScalerWidthPixel(BACKUP_WARNING_X+BACKUP_WARNING_DX); 
				rc1.bottom = ScalerHeigthPixel(BACKUP_WARNING_Y+BACKUP_WARNING_DY);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_ATTENTION), -1, &rc1, DT_CENTER | DT_VCENTER);

				rc1.left = ScalerWidthPixel(BACKUP_WARNING_MSG_X - 100); 
				rc1.top = ScalerHeigthPixel(BACKUP_WARNING_MSG_Y); 
				rc1.right = ScalerWidthPixel(BACKUP_WARNING_MSG_X + BACKUP_WARNING_MSG_DX + 100); 
				rc1.bottom = ScalerHeigthPixel(BACKUP_WARNING_MSG_Y + BACKUP_WARNING_MSG_DY);

				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				InflateRect (&rc1, -1, -1);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_RESTORE_HELP), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);

				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50), ScalerHeigthPixel(BACKUP_SAT_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_SAT_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_RESTORE_INTERNAL));
#if 0
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X), ScalerHeigthPixel(BACKUP_CH_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_CH_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_RESTORE_DEFAULT));
#endif
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X - 50), ScalerHeigthPixel(BACKUP_CH_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X - 50 + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_CH_BUTTON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_RESTORE_USB));
#if 0
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X), ScalerHeigthPixel(BACKUP_CH_BUTTON_Y + 30), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_CH_BUTTON_Y + 32),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_RESTORE));
#endif
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

							if ( restore_status == EN_RESTORE_MSG_INTER )
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);
								
								sprintf(ShellCommand, "cp %s/* %s/", BACKUP_DIR, DB_DIR );
								system( ShellCommand );
								CS_DB_RestoreDatabase();
								
								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);	
							} else if ( restore_status == EN_RESTORE_MSG_DEFAULT ) {
								if ( access(DEFAULT_DIR, 0) != 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_DIR);
									EndPaint(hwnd,hdc);
									
									usleep( 2000*1000 );	
							
									hdc = BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);									
									break;
								}

								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

								sprintf(ShellCommand, "cp %s/* %s/", DEFAULT_DIR, DB_DIR);
								system( ShellCommand );
								CS_DB_RestoreDatabase();
								
								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);	
							}else {
								if ( access(USB_BACKUP_DIR, 0) != 0 )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_NO_DIR);
									EndPaint(hwnd,hdc);
									
									usleep( 2000*1000 );
							
									hdc = BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);
									break;
								}

								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

								sprintf(ShellCommand, "cp %s/database %s/", USB_BACKUP_DIR, DB_DIR);
								system( ShellCommand );
								sprintf(ShellCommand, "cp %s/sattp %s/", USB_BACKUP_DIR, DB_DIR);
								system( ShellCommand );
								sprintf(ShellCommand, "cp %s/indexdb %s/", USB_BACKUP_DIR, DB_DIR);
								system( ShellCommand );
								CS_DB_RestoreDatabase();
								
								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);	
							}
							
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}else {
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
					CSApp_Restore_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					CSApp_Restore_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Restore_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_RED:
					restore_status = EN_RESTORE_MSG_INTER;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_RESTORE_INTERNAL);
					break;
#if 0
				case CSAPP_KEY_GREEN:
					restore_status = EN_RESTORE_MSG_DEFAULT;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_RESTORE_DEFAULT);
					break;
#endif
				case CSAPP_KEY_YELLOW:
					restore_status = EN_RESTORE_MSG_USB;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_RESTORE_USB);
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








