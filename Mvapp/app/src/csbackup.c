#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csbackup.h"
#include "csreset.h"

static CSAPP_Applet_t	CSApp_Backup_Applets;
static EN_BACKUP_MSG	backup_status;
static BOOL				Check_F2 = FALSE;

static int Backup_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

CSAPP_Applet_t	CSApp_Backup(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_Backup_Applets = CSApp_Applet_Backup;
    
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
	CreateInfo.spCaption = "csbackup window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Backup_Msg_cb;
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
	return CSApp_Backup_Applets;
    
}


static int Backup_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	RECT 				rc1;

	switch(message)
	{
		case MSG_CREATE:
			Check_F2 = FALSE;
			break;
			
		case MSG_PAINT:
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_BACKUP);
				
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
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP_HELP), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);

				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SUB_TITLE1_X),	ScalerHeigthPixel(BACKUP_SUB_TITLE1_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_DATABASE));

				rc1.top = ScalerHeigthPixel(BACKUP_SAT_BUTTON_Y + 10); 
				rc1.left = ScalerWidthPixel(BACKUP_SUB_TITLE1_X + 160); 
				rc1.right = rc1.left + 50; 
				rc1.bottom = ScalerHeigthPixel(BACKUP_SUB_TITLE1_Y + 40);
				
				MV_Draw_Box(hdc, &rc1, MVAPP_YELLOW_COLOR, DR_LEFT | DR_BOTTOM | DR_TOP );
					
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X), ScalerHeigthPixel(BACKUP_SAT_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_SAT_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP_INTERNAL));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X), ScalerHeigthPixel(BACKUP_CH_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_CH_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_BACKUP_USB));
/*
				rc1.top = ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y + 14); 
				rc1.left = ScalerWidthPixel(BACKUP_SUB_TITLE1_X + 160); 
				rc1.right = rc1.left + 50; 
				rc1.bottom = ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y + 18);
				
				MV_Draw_Box(hdc, &rc1, MVAPP_YELLOW_COLOR, DR_BOTTOM );

				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SUB_TITLE1_X),	ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_BACKUP));
*/
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X), ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_BACKUP));
				//CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_CH_BUTTON_X + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth + 10),	ScalerHeigthPixel(BACKUP_SUB_TITLE2_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_TEST));
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
							
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);
							
							memset( ShellCommand, 0x00, 256 );
							
							if ( Check_F2 == TRUE ) {
								if ( access(USB_BACKUP_DIR, 0) != 0 )
								{
									sprintf(ShellCommand, "mkdir %s", USB_BACKUP_DIR);
									system( ShellCommand );	
								}
								sprintf(ShellCommand, "/usr/work0/app/dumpread32M");
								system( ShellCommand );
								sprintf(ShellCommand, "cp /tmp/www/AllFlash_32M.tar %s", USB_BACKUP_DIR);
								system( ShellCommand );
								Check_F2 = FALSE;
							} else if ( backup_status == EN_BACKUP_MSG_INTER ) {
								if ( access(BACKUP_DIR, 0) != 0 )
								{
									sprintf(ShellCommand, "mkdir %s", BACKUP_DIR);
									system( ShellCommand );	
								}
								
								sprintf(ShellCommand, "cp %s/* %s/", DB_DIR, BACKUP_DIR);
								system( ShellCommand );
							} else if ( backup_status == EN_BACKUP_MSG_USB ) {
								if ( access(USB_BACKUP_DIR, 0) != 0 )
								{
									sprintf(ShellCommand, "mkdir %s", USB_BACKUP_DIR);
									system( ShellCommand );	
								}

								sprintf(ShellCommand, "cp %s/* %s/", DB_DIR, USB_BACKUP_DIR);
								system( ShellCommand );
							
								usleep( 2000*1000 );
							} else {
								if ( access(USB_BACKUP_DIR, 0) != 0 )
								{
									sprintf(ShellCommand, "mkdir %s", USB_BACKUP_DIR);
									system( ShellCommand );	
								}
								sprintf(ShellCommand, "/usr/work0/app/dump.sh");
								system( ShellCommand );
								sprintf(ShellCommand, "tar -cvf /tmp/backup.tar /tmp/backup");
								system( ShellCommand );
								sprintf(ShellCommand, "cp /tmp/backup.tar %s", USB_BACKUP_DIR);
								system( ShellCommand );
								sprintf(ShellCommand, "rm /tmp/backup -R");
								system( ShellCommand );
								sprintf(ShellCommand, "rm /tmp/backup.tar");
								system( ShellCommand );
							}
							
							hdc = BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}else {
							Check_F2 = FALSE;
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}
					} else {
						Check_F2 = FALSE;
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
					CSApp_Backup_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					CSApp_Backup_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Backup_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_RED:
					backup_status = EN_BACKUP_MSG_INTER;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_BACKUP_INTERNAL);
					break;

				case CSAPP_KEY_GREEN:
					backup_status = EN_BACKUP_MSG_USB;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_BACKUP_USB);
					break;
/*
				case CSAPP_KEY_F2:
					backup_status = EN_BACKUP_MSG_ALL;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_TEST);
					break;
*/
				case CSAPP_KEY_BLUE:
					Check_F2 = TRUE;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_ALL_BACKUP);
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








