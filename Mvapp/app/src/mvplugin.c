#include "linuxos.h"

#include "database.h"
#include "timer.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

/* For Plugin Site List by File : KB Kim 2011.09.13 */
#define PLUGIN_SITE_LIST_MODE
static U8               PluginSiteAddMode = 0;

/* By KB Kim for Plugin Setting : 2011.05.07 */
#define PLUGIN_CONFIRM_MODE_NONE	0
#define PLUGIN_CONFIRM_MODE_REBOOT	1
#define PLUGIN_CONFIRM_MODE_PLUGIN	2
#define PLUGIN_CONFIRM_MODE_SITE	3 /* For Plugin Site List by File : KB Kim 2011.09.13 */
#define	PLUGIN_RESET_BUTTON_Y		380
#define	PLUGIN_PLUGIN_BUTTON_Y		410
#define	PLUGIN_CALENDAR_BUTTON_Y	440
#define	PLUGIN_GAME_BUTTON_Y		470
#define PLUGIN_MESSAGE_X			400
#define PLUGIN_MESSAGE_Y			510
#define PLUGIN_MESSAGE_DX			500
#define PLUGIN_MESSAGE_DY			70
static U8               PluginConfirmMode = PLUGIN_CONFIRM_MODE_NONE;
/* By KB Kim for Plugin Setting : 2011.05.07 */
static pthread_t  			hPlugInInstall_TaskHandle;

static CSAPP_Applet_t	CSApp_PlugIn_Applets;

static int PlugIn_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

/* By KB Kim 2011.05.12 */
static void PlugInDeleteProcessFiles(void)
{
	char 				ShellCommand[256];

	sprintf(ShellCommand, "rm -R %s", TMP_PLUGIN_FOLDER);
	system(ShellCommand);
	sprintf(ShellCommand, "rm %s", PLUGIN_INFO_FILE);
	system(ShellCommand);
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
void *Install_Task( void *param )
{
	HWND	hwnd;
	HDC		hdc;
	int		i = 0;
	RECT	TmpRect;

	hwnd = GetActiveWindow();

	while(1)
	{
		hdc=BeginPaint(hwnd);

		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );

		/* By KB Kim for Plugin Setting : 2011.05.07 */
		MV_FillBox( hdc, ScalerWidthPixel(PLUGIN_MESSAGE_X), ScalerHeigthPixel(PLUGIN_MESSAGE_Y), ScalerWidthPixel(PLUGIN_MESSAGE_DX), ScalerHeigthPixel(PLUGIN_MESSAGE_DY));

		if ( i % 10 < 5 )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			TmpRect.left = PLUGIN_MESSAGE_X ;
			TmpRect.right = TmpRect.left + PLUGIN_MESSAGE_DX;
			/* By KB Kim for Plugin Setting : 2011.05.07 */
			TmpRect.top = PLUGIN_MESSAGE_Y;
			TmpRect.bottom = TmpRect.top + 30;

			CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_INSTALLING), -1, &TmpRect, DT_CENTER);
		}

		TmpRect.top = PLUGIN_MESSAGE_Y + 30;
		TmpRect.bottom = TmpRect.top + 30;

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		switch ( i % 3 )
		{
			case 0:
				CS_MW_DrawText (hdc, "/", -1, &TmpRect, DT_CENTER);
				break;
			case 1:
				CS_MW_DrawText (hdc, "-", -1, &TmpRect, DT_CENTER);
				break;
			case 2:
				CS_MW_DrawText (hdc, "\\", -1, &TmpRect, DT_CENTER);
				break;
		}
		i++;

		EndPaint(hwnd,hdc);
		usleep( 100*1000 );
	}
	return ( param );
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
int InstallPlug_Init(void)
{
	pthread_create( &hPlugInInstall_TaskHandle, NULL, Install_Task, NULL );
	return( 0 );
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
void InstallPlug_Stop(void)
{
	pthread_cancel( hPlugInInstall_TaskHandle );

	HWND	hwnd;
	HDC		hdc;





	hwnd = GetActiveWindow();

		hdc=BeginPaint(hwnd);

		MV_Warning_Report_Window_Open( hwnd, MV_WINDOW_PLUGIN_INSTALLED);
		usleep( 2000*1000 );
		MV_Warning_Report_Window_Close( hwnd );

		/*MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );



			CS_MW_TextOut (hdc, 100 , 615, "Eklenti Baþarýyla Yüklendi!  Görmek için, EXIT+MENU+INFO tuþlayýn..");*/
			EndPaint(hwnd,hdc);
			printf("Plugin stopped!!!\n");
}

CSAPP_Applet_t	CSApp_PlugIn(void)
{
	int						BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG						msg;
  	HWND					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_PlugIn_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption = "plugin window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = PlugIn_Msg_cb;
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
	return CSApp_PlugIn_Applets;

}


static int PlugIn_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	RECT 				rc1;
	/* By KB Kim for Plugin Setting : 2011.05.07 */
	char 				ShellCommand[256];
	int					ShellResult = 0;

	switch(message)
	{
		case MSG_CREATE:
			/* By KB Kim for Plugin Setting : 2011.05.07 */
			PluginConfirmMode = PLUGIN_CONFIRM_MODE_NONE;
			break;

		case MSG_PAINT:
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_PLUG_IN);

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
				//InflateRect (&rc1, -1, -1);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_HELP), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);

				/* By KB Kim for Plugin Setting : 2011.05.07 */
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50), ScalerHeigthPixel(PLUGIN_RESET_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(PLUGIN_RESET_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_REBOOT));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50), ScalerHeigthPixel(PLUGIN_PLUGIN_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(PLUGIN_PLUGIN_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_INSTALL_PLUGIN));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50), ScalerHeigthPixel(PLUGIN_CALENDAR_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(PLUGIN_CALENDAR_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_CALENDAR));
				FillBoxWithBitmap(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50), ScalerHeigthPixel(PLUGIN_GAME_BUTTON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
				CS_MW_TextOut(hdc,ScalerWidthPixel(BACKUP_SAT_BUTTON_X - 50 + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(PLUGIN_GAME_BUTTON_Y + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_GAME));
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_DOWNLOAD_COMPLETE: /* By KB Kim for Plugin Setting : 2011.05.07 */
			if ( wparam != 0 )
			{
				MV_Close_Msg_Download(hwnd);
				printf("FULL System == Get Wget File Fail >>>>>>>>>>\n");
				MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD_FAIL);
				usleep(2000*1000);
				MV_Close_Msg_Download(hwnd);
				/* By KB Kim for Plugin Setting : 2011.05.07 */
				PlugInDeleteProcessFiles();
				MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
			}
			else
			{
				MV_Close_Msg_Download(hwnd);
				PluginConfirmMode = PLUGIN_CONFIRM_MODE_PLUGIN;
				MV_Draw_Confirm_Window(hwnd, CSAPP_STR_WANT_TO_INSTALL_PLUGIN);
			}

			Download_Stop();
			break;

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
							/* By KB Kim for Plugin Setting : 2011.05.07 */
							if (PluginConfirmMode == PLUGIN_CONFIRM_MODE_REBOOT)
							{
								memset( ShellCommand, 0x00, 256 );

								sprintf(ShellCommand, "reboot");
								system( ShellCommand );
							}
							else if (PluginConfirmMode == PLUGIN_CONFIRM_MODE_PLUGIN)
							{
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);

								printf("FULL System == Get Wget File Success >>>>>>>>>>\n");

								sprintf(ShellCommand, "cd %s", TMP_PLUGIN_FOLDER);
								ShellResult = system(ShellCommand);

								MV_Draw_Msg_Download(hwnd, CSAPP_STR_INSTALL_PLUGIN);

								InstallPlug_Init();
								sprintf(ShellCommand, "tar -xf %s -C %s/", TMP_PLUGIN_FILE, TMP_PLUGIN_FOLDER);
								ShellResult = system(ShellCommand);

								if ( ShellResult == 0 )
								{
									sprintf(ShellCommand, "chmod -R 777 %s", TMP_PLUGIN_FOLDER);
									ShellResult = system(ShellCommand);
									sprintf(ShellCommand, "%s/install.sh", TMP_PLUGIN_FOLDER);
									ShellResult = system(ShellCommand);
									InstallPlug_Stop();
									MV_Close_Msg_Download(hwnd);
									if (ShellResult != 0)
									{
										MV_Draw_Msg_Download(hwnd, CSAPP_STR_INSTALL_PLUGIN_FAIL);
										usleep(1000*1000);
										MV_Close_Msg_Download(hwnd);
									}
								} else {
									InstallPlug_Stop();
									MV_Close_Msg_Download(hwnd);
									MV_Draw_Msg_Download(hwnd, CSAPP_STR_INSTALL_PLUGIN_FAIL);
									printf("FULL System == tar -xvf Fail >>>>>>>>>>\n");
									usleep(1000*1000);
									MV_Close_Msg_Download(hwnd);
								}
							}
							else if (PluginConfirmMode == PLUGIN_CONFIRM_MODE_SITE)
							{
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);

								MV_Delete_PluginSite(hwnd);
							}
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

					/* By KB Kim for Plugin Setting : 2011.05.07 */
					if (PluginConfirmMode == PLUGIN_CONFIRM_MODE_PLUGIN)
					{
						PlugInDeleteProcessFiles();
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}

					PluginConfirmMode = PLUGIN_CONFIRM_MODE_NONE;
				}

				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
#ifdef PLUGIN_SITE_LIST_MODE  /* For Plugin Site List by File : KB Kim 2011.09.13 */
			else if ( MV_Get_StringKeypad_Status() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_StringKeypad_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					if (PluginSiteAddMode)
					{
						MV_Add_PluginSite(MV_Get_StringEdited_String());
					}
					else
					{
						MV_Edit_PluginSite(MV_Get_StringEdited_String());
					}
				}
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_ENTER )
				{
					MV_Draw_PlugIn_Site_List(hwnd);
				}
				break;
			}
			else if ( MV_CheckSiteListStatus() == TRUE )
			{
				MV_WgetList_Proc(hwnd, wparam);
				/*
				if (MV_Get_StringKeypad_Status() == TRUE)
				{
					break;
				}
				*/

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						hdc = BeginPaint(hwnd);
						MV_Close_SiteList_Window(hdc);
						EndPaint(hwnd,hdc);

						MV_SetNetUpgradeMode(NET_UPGRADE_PLUGIN);
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
						EndPaint(hwnd,hdc);

						sprintf(ShellCommand, "wget -q -O %s \"http://%s/plugin.php\"", PLUGIN_INFO_FILE, MV_GetPluginSite());
						printf("\nCommend : %s\n", ShellCommand);
						ShellResult = system(ShellCommand);

						if ( ShellResult != 0 )
						{
							// printf("FULL System == Get Wget Fail >>>>>>>>>>\n");
							hdc = BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							MV_Draw_Msg_Download(hwnd, CSAPP_STR_GET_LIST_FAIL);
							MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
							usleep(1000*1000);
							MV_Close_Msg_Download(hwnd);
							break;
						}
						else
						{
							// printf("PlugIn Get Wget Success >>>>>>>>>>\n");
							MV_Plugin_Get_Wget_File();

							hdc = BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);

							MV_Draw_Wget_FileList(hwnd);
						}
					} else {
						hdc = BeginPaint(hwnd);
						MV_Close_SiteList_Window(hdc);
						EndPaint(hwnd,hdc);
						/* By KB Kim for Plugin Setting : 2011.05.07 */
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}
				}
				else if (wparam == CSAPP_KEY_RED)
				{
					PluginSiteAddMode = 0;
					hdc = BeginPaint(hwnd);
					MV_Close_SiteList_Window(hdc);
					EndPaint(hwnd,hdc);
					MV_Draw_StringKeypad(hwnd, MV_GetPluginSite(), 64);
				}
				else if (wparam == CSAPP_KEY_GREEN)
				{
					if (MV_GetPluginSiteCount() < 256)
					{
						hdc = BeginPaint(hwnd);
						MV_Close_SiteList_Window(hdc);
						EndPaint(hwnd,hdc);
						PluginSiteAddMode = 1;
						MV_Draw_StringKeypad(hwnd, "\0", 64);
					}
					else
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_CAN_NOT_ADD);
						EndPaint(hwnd,hdc);

						usleep( 2000 * 1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}
				else if (wparam == CSAPP_KEY_YELLOW)
				{
					if (MV_GetPluginSiteCount() > 1)
					{
						PluginConfirmMode = PLUGIN_CONFIRM_MODE_SITE;
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVEHELP);
					}
					else
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_CAN_NOT_DELETE);
						EndPaint(hwnd,hdc);

						usleep( 2000 * 1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}

				break;
			}
#else  // #ifdef PLUGIN_SITE_LIST_MODE
			else if ( MV_Get_StringKeypad_Status() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_StringKeypad_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					CS_DBU_Set_PlugInAddr(MV_Get_StringEdited_String());
					CS_DBU_SaveUserSettingDataInHW();
				}
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_ENTER )
				{
					MV_Draw_NetDown_Confirm(hwnd);
				}
				break;
			}
			else if ( MV_NetCheck_Confirm_Window() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_NetDown_Confirm_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						// printf("\nKEY_ENTER : ===============\n");
						if ( MV_NetCheck_YesNo() == TRUE ) /* By KB Kim for Plugin Setting : 2011.05.07 */
						{
							hdc = BeginPaint(hwnd);
							Restore_NetDown_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);

							sprintf(ShellCommand, "wget -q -O %s \"http://%s/plugin.php\"", PLUGIN_INFO_FILE, CS_DBU_Get_PlugInAddr());
							// printf("\nCommend : %s\n", ShellCommand);
							ShellResult = system(ShellCommand);

							if ( ShellResult != 0 )
							{
								// printf("FULL System == Get Wget Fail >>>>>>>>>>\n");
								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								MV_Draw_Msg_Download(hwnd, CSAPP_STR_GET_LIST_FAIL);
								MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
								usleep(1000*1000);
								MV_Close_Msg_Download(hwnd);
								break;
							}
							else
							{
								// printf("PlugIn Get Wget Success >>>>>>>>>>\n");
								MV_Plugin_Get_Wget_File();

								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);

								MV_Draw_Wget_FileList(hwnd);
							}
						} else {
							hdc = BeginPaint(hwnd);
							Restore_NetDown_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							MV_Draw_StringKeypad(hwnd, CS_DBU_Get_PlugInAddr(), 34);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_NetDown_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}
				}

				break;
			}
#endif // #ifdef PLUGIN_SITE_LIST_MODE
			else if (MV_CheckWgetListStatus() == TRUE) /* By KB Kim for Plugin Setting : 2011.05.07 */
			{
				MV_WgetList_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						hdc = BeginPaint(hwnd);
						MV_Close_Wget_Window(hdc);
						EndPaint(hwnd,hdc);

						Download_Init();
					} else {
						hdc = BeginPaint(hwnd);
						MV_Close_Wget_Window(hdc);
						EndPaint(hwnd,hdc);
						/* By KB Kim for Plugin Setting : 2011.05.07 */
						PlugInDeleteProcessFiles();
						MV_SetNetUpgradeMode(NET_UPGRADE_KIND);
					}
				}
				break;
			}
			/* By KB Kim for Plugin Setting : 2011.05.07 */
			if ( MV_CheckDownLoadStatus() == TRUE )
			{
				switch(wparam)
				{
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						system("killall wget");
						PlugInDeleteProcessFiles();
						MV_Close_Msg_Download(hwnd);
						Download_Stop();
						break;

					case CSAPP_KEY_IDLE:
						system("killall wget");
						PlugInDeleteProcessFiles();
						CSApp_PlugIn_Applets = CSApp_Applet_Sleep;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
				}
				break;
			}

			switch(wparam)
			{
				case CSAPP_KEY_ESC:
					CSApp_PlugIn_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_MENU:
					if ( b8Last_App_Status == CSApp_Applet_Push || b8Last_App_Status == CSApp_Applet_Calendar )
						CSApp_PlugIn_Applets = CSApp_Applet_MainMenu;
					else
					CSApp_PlugIn_Applets = b8Last_App_Status;

					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_PlugIn_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

				case CSAPP_KEY_RED:
					/* By KB Kim for Plugin Setting : 2011.05.07 */
					PluginConfirmMode = PLUGIN_CONFIRM_MODE_REBOOT;
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_REBOOT_SYSTEM);
					break;

				case CSAPP_KEY_GREEN:
					/* By KB Kim for Plugin Setting : 2011.05.07 */
#ifdef PLUGIN_SITE_LIST_MODE  /* For Plugin Site List by File : KB Kim 2011.09.13 */
					MV_SetNetUpgradeMode(NET_PLUGIN_SITE);
					MV_Draw_PlugIn_Site_List(hwnd);
#else  // #ifdef PLUGIN_SITE_LIST_MODE
					MV_SetNetUpgradeMode(NET_UPGRADE_PLUGIN);
					MV_Draw_NetDown_Confirm(hwnd);
#endif // #ifdef PLUGIN_SITE_LIST_MODE
					break;

				case CSAPP_KEY_BLUE:
					CSApp_PlugIn_Applets = CSApp_Applet_Push;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_YELLOW:
					CSApp_PlugIn_Applets = CSApp_Applet_Calendar;
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








