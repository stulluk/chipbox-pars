#include "linuxos.h"
#include "database.h"
#include "eit_engine.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mv_menu_ctr.h"
#include "mv_gui_interface.h"
#include "fe_mngr.h"
#include "csmpr_usb.h"
#include "csinstall.h"
#include "ui_common.h"
#include "../../system/include/av_zapping.h"
//#include "../../system/include/csmpr_av.h"


/* For S_CAM Menu By Jacob 14 May 2011 */
#include "casapi.h"

#define         			pin_box_dx                      240
#define         			pin_box_dy                      110
#define         			pin_box_base_x              	(720-pin_box_dx)/2
#define         			pin_box_base_y              	(576-pin_box_dy)/2

static CSAPP_Applet_t		CSApp_MainMenu_Applets;
static U8 					MainMenu_Focus_Item	=			CSAPP_MAINMENU_INSTALL;
static BOOL					b8Enter_Blue_Key = FALSE;

BITMAP						btMain;

static int MainMenu_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
static CSAPP_Applet_t MainMenuGetSubtApplet(U8 Idx);

void *vidopen_result;

//v37den itibaren...
FILE* testoscam;
//int selector = 0;
FILE* pluginfile;
FILE* eklfile;
//int eklyukludegil = 1;

void MV_MW_ReStartService(U16 u16ChIndex)
{
	/* By KB Kim : 2010_06_25 */
	if ( b8Last_App_Status == CSApp_Applet_Install ||
		 b8Last_App_Status == CSApp_Applet_Sat_Setting ||
		 b8Last_App_Status == CSApp_Applet_TP_Setting )
		 /*
		 b8Last_App_Status == CSApp_Applet_TP_Setting ||
		 (b8Last_App_Status == CSApp_Applet_Rec_File && CS_PVR_GetExecStatus() == CS_PVR_STATUS_IDLE))
		 */
	{
		MV_MW_StartService(u16ChIndex);
	}
}

CSAPP_Applet_t  CSApp_MainMenu(void)
{
	MSG                  			Msg;
	HWND                    		hMainWnd;
	MAINWINCREATE    				CreateInfo;
	int                    			BASE_X, BASE_Y, WIDTH, HEIGHT;

	CSApp_MainMenu_Applets = CSApp_Applet_Error;

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

	CreateInfo.dwStyle   	= WS_VISIBLE;
	CreateInfo.dwExStyle 	= WS_EX_NONE;
	CreateInfo.spCaption 	= "mainmenu window";
	CreateInfo.hMenu     	= 0;
	CreateInfo.hCursor   	= 0;
	CreateInfo.hIcon     	= 0;
	CreateInfo.MainWindowProc = MainMenu_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;
	hMainWnd = CreateMainWindow (&CreateInfo);

	if (hMainWnd == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	MainWindowThreadCleanup (hMainWnd);

	return CSApp_MainMenu_Applets;
}


//static int msg_event_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam, int* result)
static int MainMenu_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	int 				temp_with,temp_heigh;
	static RECT  		prc = {0, 0, 0, 0};
	HDC					hdc;
	int 				i;
	static int 			input_count;
	static char 		input_keys[PIN_MAX_NUM+2];
	int 				x = 0;
	U16					Start_y = 0;


#ifdef Screen_1080
	temp_with = ScalerWidthPixel(1920);
	temp_heigh= ScalerHeigthPixel(1080);
#else
	temp_with = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	temp_heigh= ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif

	prc.right = temp_with;
	prc.bottom = temp_heigh;

	switch (message)
	{
		case MSG_CREATE:

			//MainMenu_Focus_Item = CSAPP_MAINMENU_INSTALL;
			b8Enter_Blue_Key = FALSE;

			stFocus[0].u8Focus_MAX = EN_ITEM_FOCUS_INSTALL_MAX;
			stFocus[1].u8Focus_MAX = EN_ITEM_FOCUS_SYSTEM_MAX;
			stFocus[2].u8Focus_MAX = EN_ITEM_FOCUS_MEDIA_MAX;
			stFocus[3].u8Focus_MAX = EN_ITEM_FOCUS_TOOLS_MAX;

			//printf("= CREATE : stFocus[0].u8Focus_MAX : %d , %d , %d , %d ====\n", stFocus[0].u8Focus_MAX, stFocus[1].u8Focus_MAX, stFocus[2].u8Focus_MAX, stFocus[3].u8Focus_MAX);

			MV_Loading_Main_Image();

			FbSendFndDisplay("Menu");

			if ( MV_DB_GetALLServiceNumber() == 0 )
				MV_MW_StopService();
			else
				MV_MW_ReStartService(CS_DB_GetCurrentServiceIndex());

			CS_Eit_Stop();

#if 0 /* No need to be here. Move to Desktop menu : By KB Kim 2011.01.13 */
			if ( u8Glob_Sat_Focus == 0 && u8Glob_TP_Focus == 0 )
			{
				if ( MV_DB_GetALLServiceNumber() == 0 )
				{
					u8Glob_Sat_Focus = 0;
					u8Glob_TP_Focus = 0;
				}
				else
				{
					u8Glob_Sat_Focus = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
					u8Glob_TP_Focus = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Glob_Sat_Focus, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
				}
			}
#endif

			APP_SetMainMenuStatus(TRUE);

			break;

		case MSG_TIMER:
			break;

		case MSG_USB_MSG:
				SendMessage(hwnd,MSG_PAINT,0,0);
			break;

		case MSG_PAINT:
#ifdef JUST_TEST
			hdc=BeginPaint(hwnd);
			SetBkMode(hdc, BM_TRANSPARENT);
			SetTextColor(hdc, CSAPP_BLACK_COLOR);
			CS_MW_TextOut(hdc, ScalerWidthPixel(100), ScalerHeigthPixel(100), CS_MW_LoadStringByIdx(CSAPP_STR_JUST_TEST));
			EndPaint(hwnd,hdc);
#endif
			MV_Mainmenu_Draw(hwnd, MainMenu_Focus_Item);
			return 0;

		case MSG_CLOSE:
			/* By KB Kim : 20100415 */

			if ((CSApp_MainMenu_Applets == CSApp_Applet_Install ) ||
				(CSApp_MainMenu_Applets == CSApp_Applet_Sat_Setting ) ||
				(CSApp_MainMenu_Applets == CSApp_Applet_TP_Setting ))
			{
				MV_MW_StopService();
			}
			/********************************/
/*			if ( CS_DBU_GetANI_Type() && CSApp_MainMenu_Applets == CSApp_Applet_Desktop )
			{
				memset(&btMain, 0x00, sizeof(BITMAP));

				hdc = BeginPaint(hwnd);
				MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(0), ScalerHeigthPixel(0), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(BMP_MENU_Y) );
				MV_GetBitmapFromDC (hdc, 0, ScalerHeigthPixel(BMP_MENU_Y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &btMain);
				EndPaint(hwnd,hdc);
				for ( x = 0 ; x < 5 ; x++ )
				{
					Start_y = BMP_MENU_Y + ((BMP_MENU_DY/5) * (x + 1));
					hdc = BeginPaint(hwnd);
					SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
					FillBox(hdc,ScalerWidthPixel(0), ScalerHeigthPixel(BMP_MENU_Y),ScalerWidthPixel(MAX_DX),ScalerHeigthPixel(BMP_MENU_DY));
					MV_FillBoxWithBitmap (hdc, 0, ScalerHeigthPixel(Start_y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &btMain);
					EndPaint(hwnd,hdc);
					//usleep(10);
			    }
				UnloadBitmap(&btMain);

				hdc=BeginPaint(hwnd);
				MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(0), ScalerHeigthPixel(0), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT) );
				EndPaint(hwnd,hdc);
			}
			else*/
			{
				hdc=BeginPaint(hwnd);
				MV_SetBrushColor( hdc, MVAPP_TRANSPARENTS_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(0), ScalerHeigthPixel(0), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT) );
				EndPaint(hwnd,hdc);
			}
			/********************************/

			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;

		case MSG_PIN_INPUT:
			{
				int 	x,y;

				x = pin_box_base_x + 156;
				y = pin_box_base_y + 48;

				hdc = BeginPaint(hwnd);

				SetBkMode(hdc, BM_TRANSPARENT);
				SetTextColor(hdc, CSAPP_BLACK_COLOR);
				CS_MW_TextOut(hdc, ScalerWidthPixel(pin_box_base_x+38), ScalerHeigthPixel(pin_box_base_y+3), CS_MW_LoadStringByIdx(CSAPP_STR_INPUT_PIN));
				CS_MW_TextOut(hdc, ScalerWidthPixel(pin_box_base_x+28), ScalerHeigthPixel(y), CS_MW_LoadStringByIdx(CSAPP_STR_PASSWORD));

				SetTextColor(hdc, CSAPP_WHITE_COLOR);

				for(i=0;i<input_count && i < PIN_MAX_NUM;i++)
				{
					CS_MW_TextOut(hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y), "*");
					x += 12;
				}

				for(;i<PIN_MAX_NUM;i++)
				{
					CS_MW_TextOut(hdc, ScalerWidthPixel(x), ScalerHeigthPixel(y), "-");
					x += 12;
				}

	            EndPaint(hwnd,hdc);

				if(input_count >= PIN_MAX_NUM)
				{
					if(Pin_Verify(input_keys))
					{
						input_count = 0;
						memset(input_keys,0,sizeof(input_keys));
						SendMessage(hwnd, MSG_CLOSE, 0, 0);
					}
					else
					{
						input_count = 0;
						memset(input_keys,0,sizeof(input_keys));
						SendMessage(hwnd, MSG_PIN_INPUT, 0, 0);
					}
				}

				//EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
            switch(wparam)
            {
                case CSAPP_KEY_IDLE:
	        		CSApp_MainMenu_Applets = CSApp_Applet_Sleep;
	        		SendMessage(hwnd,MSG_CLOSE,0,0);
	        		break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;

            }

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
							if ( b8Enter_Blue_Key == TRUE )
							{
								b8Enter_Blue_Key = FALSE;
								CSApp_MainMenu_Applets = CSApp_Applet_OSCAM_Setting;
							}
							else
							CSApp_MainMenu_Applets = MainMenuGetSubtApplet(MainMenu_Focus_Item);
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
							if ( b8Enter_Blue_Key == TRUE )
							{
								b8Enter_Blue_Key = FALSE;
								CSApp_MainMenu_Applets = CSApp_Applet_OSCAM_Setting;
							}
							else
							CSApp_MainMenu_Applets = MainMenuGetSubtApplet(MainMenu_Focus_Item);
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
						break;
			        case CSAPP_KEY_ESC:
			        case CSAPP_KEY_MENU:
						b8Enter_Blue_Key = FALSE;
						break;
				}
				break;
			}
#if 0
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
#endif
			{
				switch(wparam)
				{
					case CSAPP_KEY_0:

					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc0.sh" );
						break;

					case CSAPP_KEY_1:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc1.sh" );
						break;

					case CSAPP_KEY_2:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc2.sh" );
						break;

					case CSAPP_KEY_3:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc3.sh" );
						break;

					case CSAPP_KEY_4:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc4.sh" );
						break;

					case CSAPP_KEY_5:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc5.sh" );
						break;

					case CSAPP_KEY_6:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc6.sh" );
						break;

					case CSAPP_KEY_7:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc7.sh" );
						break;

					case CSAPP_KEY_8:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
                    CS_AV_VideoBlank();
						system( "/usr/work1/sc8.sh" );
                    CS_AV_VideoUnblank();
						break;

					case CSAPP_KEY_9:
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sc9.sh" );
						break;

					case CSAPP_KEY_RECALL :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/screcall.sh" );
						break;

					case CSAPP_KEY_MUTE :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scmute.sh" );
						break;

					case CSAPP_KEY_EPG :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scepg.sh" );
						break;

					case CSAPP_KEY_INFO :

					//v37den itibaren komple degiþtirdim aþaðýyý...

					eklfile = fopen("/usr/work1/scinfo.sh" , "r");
					if (eklfile){
                            fclose(eklfile);
                            system( "/usr/work1/scinfo.sh" );
                            break;

					}


					else{
					    hdc=BeginPaint(hwnd);
                        //MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                        MV_SetBrushColor( hdc, MVAPP_BLACK_GRAY_COLOR);
                        MV_FillBox( hdc, ScalerWidthPixel(100), ScalerHeigthPixel(10), ScalerWidthPixel(1080), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT-205) );
                        MV_SetBrushColor( hdc, MVAPP_BLUE_COLOR);
                        MV_FillBox( hdc, ScalerWidthPixel(640), ScalerHeigthPixel(60), ScalerWidthPixel(2), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT-300) );
                        SetBkMode(hdc, BM_TRANSPARENT);

                        SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                        CS_MW_TextOut(hdc, ScalerWidthPixel(500), ScalerHeigthPixel(20), "KISAYOL BÝLGÝ EKRANI");


                        if (pluginfile = fopen("/usr/work1/scplay.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(60), "IPTV =======> PLAY");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(60), "IPTV yüklü deðil..");
                        }

                        if (pluginfile = fopen("/usr/work1/scsat.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(60), "MAÇ BÝLGÝ =======> UYDU");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(60), "MAÇ BÝLGÝ yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc8.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(90), "Gazeteler =========> 8");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(90), "Gazeteler yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/scf1.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(90), "Puan Durumu =======> F1");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(90), "Puan Durumu yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/S70oscam", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(120), "OSCAM sunucularý ====> MAVÝ");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(150), "OSCAM restart ======> SARI");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(120), "OSCAM durdur ====> 4");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(150), "OSCAM bilgiler ======> 9");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(120), "OSCAM yüklü deðil..");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(150), "OSCAM yüklü deðil..");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(120), "OSCAM yüklü deðil..");
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(150), "OSCAM yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/scred.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(180), "Haritalar ==========> KIRMIZI");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(180), "Haritalar yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/scf2.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(180), "FIKRA ========> F2");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(180), "FIKRA yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc3.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(210), "Hava Durumu ==========> 3");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(210), "Hava Durumu yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/screc.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(210), "NFS KAYIT =====> KAYIT");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(210), "NFS KAYIT yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc1.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(240), "AutoZAP ==========> 1");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(240), "AutoZAP yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/screcall.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(240), "Yeniden Baþlat =====> GERÝ");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(240), "Yeniden Baþlat yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc5.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(270), "Döviz / Altýn =========> 5");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(270), "Döviz / Altýn yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc6.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(300), "Namaz Vakitleri =========> 6");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(300), "Namaz Vakitleri yüklü deðil..");

                        }


                        if(pluginfile = fopen("/usr/work1/scepg.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(270), "Yayýn Akýþý ==========> EPG");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(270), "Yayýn Akýþý yüklü deðil..");
                        }


                        if(pluginfile = fopen("/usr/work1/scepg.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(300), "MailYedek ========> PageDown");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(660), ScalerHeigthPixel(300), "MailYedek yüklü deðil..");
                        }


                        if(pluginfile = fopen("/usr/work1/sc7.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(330), "InfoShow ==========> 7");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(330), "InfoShow yüklü deðil..");
                        }

                        if(pluginfile = fopen("/usr/work1/sc0.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(360), "Turksat frekans ====> 0");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(360), "Turksat frekans yüklü deðil..");
                        }

                        /*if(pluginfile = fopen("/usr/work1/scinfo.sh", "r")){
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(390), "Ekl. Eklentisi ==> INFO");
                            fclose(pluginfile);
                        }
                        else{
                            SetTextColor(hdc, MVAPP_YELLOW_COLOR_ALPHA);
                            CS_MW_TextOut(hdc, ScalerWidthPixel(120), ScalerHeigthPixel(390), "Ekl. Eklentisi yüklü deðil..");
                        }*/




                        SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                        CS_MW_TextOut(hdc, ScalerWidthPixel(350), ScalerHeigthPixel(490), "Çýkmak için EXIT butonuna basýnýz");
                        EndPaint(hwnd,hdc);

                        break;
					}


					case CSAPP_KEY_PG_UP :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scpgup.sh" );
						break;

					case CSAPP_KEY_PG_DOWN :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scpgdown.sh" );
						break;

					case CSAPP_KEY_SD_HD :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scsdhd.sh" );
						break;

					case CSAPP_KEY_TVRADIO :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sctvradio.sh" );
						break;

					case CSAPP_KEY_F1 :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scf1.sh" );
						break;

					case CSAPP_KEY_F2 :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scf2.sh" );
						break;

					case CSAPP_KEY_SAT :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scsat.sh" );
						break;

					case CSAPP_KEY_TV_AV :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/sctvav.sh" );
						break;

					case CSAPP_KEY_FAVOLIST :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scfav.sh" );
						break;

					case CSAPP_KEY_REC :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/screc.sh" );
						break;

					case CSAPP_KEY_PLAY :

                                printf("PLAY key pressed..\n");
                                FbSendFndDisplay("IPTV");

                                hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);


                                        CS_AV_Audio_SetMuteStatus(FALSE);

                                            if ( eCS_AV_ERROR == CS_Video_Close())
                                              printf("CS_Video_Close() Failed!!!\n");
                                            else
                                                printf("CS_Video_Close() Succesfull\n");

                                            CS_MW_StopService(TRUE);
                                            CS_AV_Play_IFrame(BOOT_LOGO);


                                            CSDEMUX_Terminate();


                                        system( "/usr/work1/scplay.sh" );

                                        if ( CSAPI_FAILED == CSDEMUX_Init())
                                            printf("CSDEMUX_Init() Failed!!!\n");
                                        else
                                            printf("CSDEMUX_Init() Succeed!!!\n");

                                        if ( eCS_AV_ERROR == CS_Audio_Close() )
                                            printf("CS_Audio_Close() Error!!\n");
                                        else
                                            printf("CS_Audio_Close() Succesfull!\n");



                                        if ( !CS_AV_Init() )
                                            printf("CS_AV_Init Error!!\n");
                                        else
                                            printf("CS_AV_Init Succesfull!\n");



                                        CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);

                                        FbSendFndDisplay("MENU");

                                        //printf("Current program Idx is = %d (int) %x (hex)\n", CS_MW_GetCurrentPlayProgram(), CS_MW_GetCurrentPlayProgram());

                                        break;

					case CSAPP_KEY_STOP :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);



						system( "/usr/work1/scstop.sh" );



						break;
					case CSAPP_KEY_SLEEP :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scsleep.sh" );



						break;
					case CSAPP_KEY_REW :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
                    system( "/usr/work1/screw.sh" );


						break;

					case CSAPP_KEY_FF :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), "Streaming baþlýyor...");
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scff.sh" );
						/*printf("Streaming baþlýyor...\n");
                                FbSendFndDisplay("STRM");
                                FbSendFndDisplay("MENU");*/


						break;

					case CSAPP_KEY_SLOW :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scslow.sh" );
						break;

					case CSAPP_KEY_SLOW_B :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scslowb.sh" );
						break;

					case CSAPP_KEY_RED :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usr/work1/scred.sh" );
						break;

					case CSAPP_KEY_GREEN :
					hdc=BeginPaint(hwnd);
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                    MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                    SetBkMode(hdc, BM_TRANSPARENT);
                    SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                    CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), CS_MW_LoadStringByIdx(CSAPP_STR_PLUGIN_MSG));
                    EndPaint(hwnd,hdc);
						system( "/usrwork1/scgreen.sh" );
						break;

					case CSAPP_KEY_YELLOW :
					testoscam = fopen("/usr/work1/oscam-chipbox", "rb");

					if(testoscam != NULL){
                        hdc=BeginPaint(hwnd);
                        MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                        MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                        SetBkMode(hdc, BM_TRANSPARENT);
                        SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                        CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), "Oscam yeniden baþlatýlýyor..");
                        EndPaint(hwnd,hdc);
                        system( "/usr/work1/S70oscam restart" );
					}
					else{
					    hdc=BeginPaint(hwnd);
                        MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR);
                        MV_FillBox( hdc, ScalerWidthPixel(700), ScalerHeigthPixel(100), ScalerWidthPixel(500), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT/10) );
                        SetBkMode(hdc, BM_TRANSPARENT);
                        SetTextColor(hdc, MVAPP_YELLOW_COLOR);
                        CS_MW_TextOut(hdc, ScalerWidthPixel(720), ScalerHeigthPixel(120), "Oscam zaten yüklü deðil..");
                        EndPaint(hwnd,hdc);

					}

					if (testoscam)
                        fclose(testoscam);


						break;
					case CSAPP_KEY_BLUE:
						/* For S_CAM Menu By Jacob 14 May 2011 */
						if (StbSGetOscamStatus())
						{
							b8Enter_Blue_Key = TRUE;
							MainMenu_Ani_Stop();
							MV_Draw_Password_Window(hwnd);
						}
						break;

					case CSAPP_KEY_RIGHT:
						hdc = BeginPaint(hwnd);
						MV_Mainmenu_UnFocus( hdc, MainMenu_Focus_Item );

						//printf("=== RIGHT : %d ----> ", MainMenu_Focus_Item);

						if(MainMenu_Focus_Item == CSAPP_MAINMENU_TOOL)
							MainMenu_Focus_Item = CSAPP_MAINMENU_INSTALL;
						else
							MainMenu_Focus_Item ++ ;

						MV_Set_Submenu_Focus(MainMenu_Focus_Item, stFocus[MainMenu_Focus_Item].u8Sub_Focus);

						//printf("%d\n", MainMenu_Focus_Item);
						//printf("= RIGHT : stFocus[MainMenu_Focus_Item].SubBmp.bmHeight : %d ====\n", stFocus[MainMenu_Focus_Item].SubBmp.bmHeight);
						if ( stFocus[MainMenu_Focus_Item].SubBmp.bmHeight == 0 )
						{
							EndPaint(hwnd,hdc);

							MV_Mainmenu_Focus( hwnd, MainMenu_Focus_Item );

							hdc = BeginPaint(hwnd);
							MV_Submenu_Focus( hdc, MainMenu_Focus_Item );
						} else {
							MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(0),ScalerHeigthPixel(MENU_CAP_HEIGHT), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT - MENU_CAP_HEIGHT), &stFocus[MainMenu_Focus_Item].SubBmp);
						}
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_LEFT:
						hdc = BeginPaint(hwnd);
						MV_Mainmenu_UnFocus( hdc, MainMenu_Focus_Item );

						//printf("=== LEFT : %d ----> ", MainMenu_Focus_Item);

						if(MainMenu_Focus_Item == CSAPP_MAINMENU_INSTALL)
							MainMenu_Focus_Item = CSAPP_MAINMENU_TOOL;
						else
							MainMenu_Focus_Item -- ;

						MV_Set_Submenu_Focus(MainMenu_Focus_Item, stFocus[MainMenu_Focus_Item].u8Sub_Focus);

						//printf("%d\n", MainMenu_Focus_Item);
						//printf("= LEFT : stFocus[MainMenu_Focus_Item].SubBmp.bmHeight : %d ====\n", stFocus[MainMenu_Focus_Item].SubBmp.bmHeight);
						if ( stFocus[MainMenu_Focus_Item].SubBmp.bmHeight == 0 )
						{
							EndPaint(hwnd,hdc);

							MV_Mainmenu_Focus( hwnd, MainMenu_Focus_Item );

							hdc = BeginPaint(hwnd);
							MV_Submenu_Focus( hdc, MainMenu_Focus_Item );
						} else {
							MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(0),ScalerHeigthPixel(MENU_CAP_HEIGHT), ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH), ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT - MENU_CAP_HEIGHT), &stFocus[MainMenu_Focus_Item].SubBmp);
						}
						EndPaint(hwnd,hdc);
						break;

					case CSAPP_KEY_DOWN:
						if ( MainMenu_Focus_Item == CSAPP_MAINMENU_MEDIA && UsbCon_GetStatus() != USB_STATUS_MOUNTED )
						{
							printf("\n");
						}
						else if ( MainMenu_Focus_Item == CSAPP_MAINMENU_INSTALL && MV_GetSelected_SatData_Count() == 0 )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_FIRST_SELECT);
							EndPaint(hwnd,hdc);

							usleep( 1000 * 1000 );

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_SubmenuBar_Focus( hdc, MainMenu_Focus_Item, MV_Get_Submenu_Focus(), UNFOCUS );

							if(MV_Get_Submenu_Focus() == stFocus[MainMenu_Focus_Item].u8Focus_MAX - 1)
								MV_Set_Submenu_Focus(MainMenu_Focus_Item, 0);
							else
								MV_Set_Submenu_Focus(MainMenu_Focus_Item, MV_Get_Submenu_Focus()+1);

							MV_SubmenuBar_Focus( hdc, MainMenu_Focus_Item, MV_Get_Submenu_Focus(), FOCUS );
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_UP:
						if ( MainMenu_Focus_Item == CSAPP_MAINMENU_MEDIA && UsbCon_GetStatus() != USB_STATUS_MOUNTED )
						{
							printf("\n");
						}
						else if ( MainMenu_Focus_Item == CSAPP_MAINMENU_INSTALL && MV_GetSelected_SatData_Count() == 0 )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_FIRST_SELECT);
							EndPaint(hwnd,hdc);

							usleep( 1000 * 1000 );

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_SubmenuBar_Focus( hdc, MainMenu_Focus_Item, MV_Get_Submenu_Focus(), UNFOCUS );

							if(MV_Get_Submenu_Focus() == 0 )
								MV_Set_Submenu_Focus(MainMenu_Focus_Item, stFocus[MainMenu_Focus_Item].u8Focus_MAX - 1);
							else
								MV_Set_Submenu_Focus(MainMenu_Focus_Item, MV_Get_Submenu_Focus()-1);

							MV_SubmenuBar_Focus( hdc, MainMenu_Focus_Item, MV_Get_Submenu_Focus(), FOCUS );
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_ENTER:
						if ( MainMenu_Focus_Item == CSAPP_MAINMENU_MEDIA && UsbCon_GetStatus() != USB_STATUS_MOUNTED )
						{
							printf("\n");
						}
						else if ( MainMenu_Focus_Item == CSAPP_MAINMENU_MEDIA && MV_Get_Submenu_Focus() == EN_ITEM_FOCUS_USB_REMOVE )
						{
							UsbCon_Umount();
							usleep( 5000*1000 );
						}
#if 0
						else if ( MainMenu_Focus_Item == CSAPP_MAINMENU_TOOL && MV_Get_Submenu_Focus() == EN_ITEM_FOCUS_PLUG_IN )
						{
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_REBOOT_SYSTEM);
						}
#endif
						else
						{
							MainMenu_Ani_Stop();
							if ( ( CS_DBU_GetParentalLockStatus() && CS_DBU_GetInstallLockStatus() ) || ( MV_Get_Submenu_Focus() == EN_ITEM_FOCUS_PARENTAL && MainMenu_Focus_Item == CSAPP_MAINMENU_SYSTEM ) )
							{
								MV_Draw_Password_Window(hwnd);
							} else {
								CSApp_MainMenu_Applets = MainMenuGetSubtApplet(MainMenu_Focus_Item);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						}
						break;

					case CSAPP_KEY_MENU:
					case CSAPP_KEY_ESC:
						MainMenu_Ani_Stop();
						if ( MV_DB_GetALLServiceNumber() == 0 )
						{
							return 0;
						} else {
							CSApp_MainMenu_Applets = CSApp_Applet_Desktop;
							u8Glob_Sat_Focus = 0;
							u8Glob_TP_Focus = 0;
							//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
//							printf("\n MainMenu Message -- ESC\n\n");
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
						break;
                    case CSAPP_KEY_PAUSE:
						/*if ( CFG_Factory_Mode == FALSE )
						{
							//CFG_Factory_Mode = FALSE; //Sertac v37-test1 fabrika menüsünü kapatma için...
							//MV_Fac_End_cfg(KIND_ON);
							CSApp_MainMenu_Applets = CSApp_Applet_MainMenu;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
                    	break;*/

                        break;

					default:
						break;
				}
			}
			break;
	}

	return DefaultMainWinProc(hwnd, message, wparam, lparam);

}

static CSAPP_Applet_t MainMenuGetSubtApplet(U8 Idx)
{
	CSAPP_Applet_t	CurrentApplet=CSApp_Applet_Error;

	switch(Idx)
	{
		case CSAPP_MAINMENU_INSTALL:
			switch(MV_Get_Submenu_Focus())
			{
				case 0:
					CurrentApplet=CSApp_Applet_Install;
					break;
				case 1:
					CurrentApplet=CSApp_Applet_Sat_Setting;
					break;
				case 2:
					CurrentApplet=CSApp_Applet_TP_Setting;
					break;
				default:
					CurrentApplet=CSApp_Applet_Install;
					break;
			}
			break;

		case CSAPP_MAINMENU_SYSTEM:
			switch(MV_Get_Submenu_Focus())
			{
				case 0 :
					CurrentApplet=CSApp_Applet_Language;
					break;
				case 1 :
					CurrentApplet=CSApp_Applet_TimeSetting;
					break;
				case 2 :
					CurrentApplet=CSapp_Applet_SystemSetting;
					break;
				case 3 :
					CurrentApplet=CSApp_Applet_AVSetting;
					break;
				case 4 :
					CurrentApplet=CSApp_Applet_PinSetting;
					break;
				case 5 :
					CurrentApplet=CSApp_Applet_NetSetting;
					break;
				case 6 :
					CurrentApplet=CSapp_Applet_SystemInfo;
					break;
				default :
					CurrentApplet=CSapp_Applet_SystemInfo;
					break;
			}
			break;

		case CSAPP_MAINMENU_MEDIA:
			switch(MV_Get_Submenu_Focus())
			{
				case 0 :
					CurrentApplet=CSApp_Applet_Rec_File;
					break;
#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
				case 1 :
					CurrentApplet=CSApp_Applet_Rec_Config;
					break;
				case 2 :
					CurrentApplet=CSApp_Applet_File_Tool;
					break;
				case 3 :
					CurrentApplet=CSApp_Applet_USB_Remove;
					break;
				case 4 :
					CurrentApplet=CSApp_Applet_Storage_Info;
					break;
				case 5 :
					CurrentApplet=CSApp_Applet_Media_Player;
					break;
#else
				case 1 :
					CurrentApplet=CSApp_Applet_File_Tool;
					break;
				case 2 :
					CurrentApplet=CSApp_Applet_USB_Remove;
					break;
				case 3 :
					CurrentApplet=CSApp_Applet_Storage_Info;
					break;
				case 4 :
					CurrentApplet=CSApp_Applet_Media_Player;
					break;
#endif
				default :
					CurrentApplet=CSApp_Applet_Rec_File;
					break;
			}
			break;

		case CSAPP_MAINMENU_TOOL:
			switch(MV_Get_Submenu_Focus())
			{
				case 0 :
					CurrentApplet=CSApp_Applet_Upgrade;
					break;
				case 1 :
					CurrentApplet=CSApp_Applet_CI;
					break;
				case 2 :
					CurrentApplet=CSApp_Applet_CAS;
					break;
				case 3 :
					CurrentApplet=CSApp_Applet_Backup;
					break;
				case 4 :
					CurrentApplet=CSApp_Applet_Restore;
					break;
				case 5 :
					CurrentApplet=CSApp_Applet_PlugIn;
					break;
				case 6 :
					CurrentApplet=CSApp_Applet_Reset;
					break;
				default :
					CurrentApplet=CSApp_Applet_Upgrade;
					break;
			}
			break;
	}

	return CurrentApplet;
}

void MainMenuSetSubtApplet(U8 Idx, U8 Subidx)
{
	MV_Set_Submenu_Focus(Idx, Subidx);
	MainMenu_Focus_Item = Idx;
}

