#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

static CSAPP_Applet_t		CSApp_Viewer_Applets;

static int Viewer_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

CSAPP_Applet_t CSApp_Viewer(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Viewer_Applets = CSApp_Applet_Error;

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

	CreateInfo.dwStyle		= WS_VISIBLE;
	CreateInfo.dwExStyle	= WS_EX_NONE;
	CreateInfo.spCaption	= "Image Viewer";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = Viewer_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_Viewer_Applets;   
}


static int Viewer_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
	switch(message)
   	{
		case MSG_CREATE:
			break;
			
		case MSG_PAINT:
			return 0;
		
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_RED:
					break;
				case CSAPP_KEY_BLUE:
					break;
					
				case CSAPP_KEY_ENTER:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Viewer_Applets = get_prev_windown_status();
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Viewer_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					break;
						
				default:
					break;
			}
			break;
		
	   	case MSG_CLOSE:
			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;

	   	default:
			break;
   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}



