#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

static CSAPP_Applet_t		CSApp_Pass_Applets;

static int Pass_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

CSAPP_Applet_t CSApp_Check_Pass(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Pass_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "PassCheck";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = Pass_Msg_cb;
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

	return CSApp_Pass_Applets;   
}


static int Pass_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
   
	switch(message)
	   	{
			case MSG_CREATE:
				break;
				
			case MSG_PAINT:
				MV_Draw_Password_Window(hwnd);
				return 0;
			
			case MSG_KEYDOWN:
				if (MV_Get_Password_Flag() == TRUE)
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
								CSApp_Pass_Applets = CSApp_Applet_Exit;
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
							break;
							
						case CSAPP_KEY_ENTER:
							if(MV_Password_Retrun_Value() == TRUE)
							{
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								CSApp_Pass_Applets = CSApp_Applet_Exit;
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							} else {
								CSApp_Pass_Applets = CSApp_Applet_Error;
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
							break;

						case CSAPP_KEY_EXIT:
						case CSAPP_KEY_MENU:
							CSApp_Pass_Applets = CSApp_Applet_Error;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							break;
					}
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



