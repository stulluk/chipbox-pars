#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define	FIELDS_PER_LINE				        2

static CSAPP_Applet_t	CSApp_AVSetting_Applets;
static U16				Current_Item = 0;
static U16				Current_AspectRatio = 0;
static U16				Current_AspectMode = 0;
static U16				Current_VideoDefinition = 0;
static U16				Pre_VideoDefinition = 0;
static U16				Current_VideoOutput = 0;
static U16				Current_Audio_SPDIF_Output = 0;

static const U16 AspectRatio[CS_APP_ASPECT_RATIO_NUM] = {
							CSAPP_STR_ASPECT_4_3,
							CSAPP_STR_ASPECT_16_9,
							CSAPP_STR_AUTO
						};

static const U16 AspectMode[CS_APP_ASPECT_MODE_NUM] = {
							CSAPP_STR_ASPECT_FULL,
							CSAPP_STR_ASPECT_LETTER_BOX,
							CSAPP_STR_ASPECT_PAN_SCAN
						};

static const char* VideoDefinition[CS_APP_DEFINITION_NUM] = {
							"480i",
							"576i",
							"576p",
							"720p",
							"1080i",
							//CSAPP_STR_DEFINITION_1080P,
							"Auto"
						};

#if 0 /* By KB Kim 2010.08.31 */
static const U16 VideoOutput[CS_APP_OUTPUT_NUM] = {
							CSAPP_STR_OUTPUT_CVBS,
							CSAPP_STR_OUTPUT_YC,
							CSAPP_STR_OUTPUT_YCBCR,
							CSAPP_STR_OUTPUT_YPBPR,
							CSAPP_STR_OUTPUT_RGB,
							CSAPP_STR_OUTPUT_HDMI
						};
#else
static const U16 VideoOutput[CS_APP_OUTPUT_NUM] = {
							CSAPP_STR_OUTPUT_RGB,
							CSAPP_STR_OUTPUT_YPBPR,
							// CSAPP_STR_OUTPUT_YC,
						};
#endif

static const U16 AudioOutput[CS_APP_AUDIO_SPDIF_OUTPUT_NUM] = {
							CSAPP_STR_AUTO,
							CSAPP_STR_AUDIO_SPDIF_OUTPUT_PCM,
							CSAPP_STR_AUDIO_SPDIF_OUTPUT_AC3
						};

U8	AV_Arrow_Kind[AV_SETTING_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	AV_Enter_Kind[AV_SETTING_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U16 AV_Setting_Str[AV_SETTING_ITEM_MAX] = {
	CSAPP_STR_ASPECT_RATIO,
	CSAPP_STR_ASPECT_MODE,
	CSAPP_STR_VIDEO_DEFINITION,
	CSAPP_STR_VIDEO_OUTPUT,
	CSAPP_STR_AUDIO_SPDIF_OUTPUT
};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static int AV_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_AVSelectBar(HDC hdc, int y_gap, eMV_Setting_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(AV_Enter_Kind[esItem])
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
	
	if ( AV_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_AVMenuBar(HDC hdc, U8 u8Focuskind, eMV_Setting_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_AVSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(AV_Setting_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[20];
		
		case AV_ASPECT_RATIO:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(AspectRatio[Current_AspectRatio]), -1, &TmpRect, DT_CENTER);		
			break;
		case AV_ASPECT_MODE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(AspectMode[Current_AspectMode]), -1, &TmpRect, DT_CENTER);	
			break;
		case AV_VIDEO_DEFINITION:
			sprintf(temp_str, "%s", VideoDefinition[Current_VideoDefinition]);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case AV_VIDEO_OUTPUT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(VideoOutput[Current_VideoOutput]), -1, &TmpRect, DT_CENTER);
			break;
		case AV_AUDIO_SPDIF_OUTPUT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(AudioOutput[Current_Audio_SPDIF_Output]), -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_AV_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < AV_SETTING_ITEM_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_AVMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_AVMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t	CSApp_AVSetting(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_AVSetting_Applets = CSApp_Applet_AVSetting;
		
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
	CreateInfo.spCaption = "csavsetting window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = AV_Msg_cb;
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

	return CSApp_AVSetting_Applets;
    
}


static int AV_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;

	switch(message)
    {
        case MSG_CREATE:
			
			Current_Item = 0;
			Current_AspectRatio = CS_MW_GetAspectRatio();
			Current_AspectMode = CS_MW_GetAspectMode();
			Current_VideoDefinition = CS_MW_GetVideoDefinition();
			Pre_VideoDefinition = Current_VideoDefinition;
			Current_VideoOutput = CS_MW_GetVideoOutput();
			Current_Audio_SPDIF_Output = CS_MW_GetDefaultSpdifAudioType();
			break;
		case MSG_PAINT:
			{	
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_AV);

				hdc=BeginPaint(hwnd);
				MV_Draw_AV_MenuBar(hdc);
				MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_AV);
				EndPaint(hwnd,hdc);

				if ( CFG_Yinhe_Test )
				{
					usleep( 1000 * 1000 );
					SendMessage(hwnd,MSG_KEYDOWN,CSAPP_KEY_LEFT,0);
				}
			}
			// printf("MSG_PAINT end\n");

			return 0;

		case MSG_VIDEO_FORFMAT_UPDATE:
			{

			}
			break;

		case MSG_KEYDOWN:
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Current_Item)
					{
						case AV_ASPECT_RATIO:
							Current_AspectRatio = u8Result_Value;
							CS_MW_SetAspectRatio(Current_AspectRatio);
							break;
						case AV_ASPECT_MODE:
							Current_AspectMode = u8Result_Value;
							CS_MW_SetAspectMode(Current_AspectMode);
							break;
						case AV_VIDEO_DEFINITION:
							Current_VideoDefinition = u8Result_Value;
							CS_MW_SetVideoDefinition(Current_VideoDefinition);
							break;
						case AV_VIDEO_OUTPUT:
							Current_VideoOutput = u8Result_Value;
							CS_MW_SetVideoOutput(Current_VideoOutput);
							break;
						case AV_AUDIO_SPDIF_OUTPUT:
							Current_Audio_SPDIF_Output = u8Result_Value;
							CS_MW_SetDefaultSpdifAudioType(Current_Audio_SPDIF_Output);
							break;
						default:
							break;
					}
					hdc=BeginPaint(hwnd);
					MV_Draw_AVMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
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
					{
						if(CS_DBU_CheckIfUserSettingDataChanged())
						{
							CS_DBU_SaveUserSettingDataInHW();
						}

						CSApp_AVSetting_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;
					
				case CSAPP_KEY_MENU:
					{
						if(CS_DBU_CheckIfUserSettingDataChanged())
						{
							CS_DBU_SaveUserSettingDataInHW();
						}

						CSApp_AVSetting_Applets = b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;

				case CSAPP_KEY_ENTER:
					{
						int						i = 0;
						RECT					smwRect;
						stPopUp_Window_Contents stContents;

						memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
						smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
						smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
						smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
						
						switch(Current_Item)
						{
							case AV_ASPECT_RATIO:
								for ( i = 0 ; i < CS_APP_ASPECT_RATIO_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(AspectRatio[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_ASPECT_RATIO_NUM );
								stContents.u8TotalCount = CS_APP_ASPECT_RATIO_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case AV_ASPECT_MODE:
								for ( i = 0 ; i < CS_APP_ASPECT_MODE_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(AspectMode[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_ASPECT_MODE_NUM );
								stContents.u8TotalCount = CS_APP_ASPECT_MODE_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case AV_VIDEO_DEFINITION:
								for ( i = 0 ; i < CS_APP_DEFINITION_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", VideoDefinition[i]);
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_DEFINITION_NUM );
								stContents.u8TotalCount = CS_APP_DEFINITION_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case AV_VIDEO_OUTPUT:
								for ( i = 0 ; i < CS_APP_OUTPUT_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(VideoOutput[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_OUTPUT_NUM );
								stContents.u8TotalCount = CS_APP_OUTPUT_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case AV_AUDIO_SPDIF_OUTPUT:
								for ( i = 0 ; i < CS_APP_AUDIO_SPDIF_OUTPUT_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(AudioOutput[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_AUDIO_SPDIF_OUTPUT_NUM );
								stContents.u8TotalCount = CS_APP_AUDIO_SPDIF_OUTPUT_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							default:
								break;
						}
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_AVSetting_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_AVMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					if(Current_Item == 0)
						Current_Item = AV_SETTING_ITEM_MAX - 1;
					else
						Current_Item--;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_AVMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
					MV_Draw_AVMenuBar(hdc, MV_UNFOCUS, Current_Item);

					if(Current_Item == AV_SETTING_ITEM_MAX - 1)
						Current_Item = 0;
					else
						Current_Item++;

					MV_Draw_AVMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_LEFT:
					{
						switch(Current_Item)
						{
							case AV_ASPECT_RATIO:
								if(Current_AspectRatio == 0)
									Current_AspectRatio = CS_APP_ASPECT_RATIO_NUM - 1;
								else
									Current_AspectRatio--;

								CS_MW_SetAspectRatio(Current_AspectRatio);
								break;
							case AV_ASPECT_MODE:
								if(Current_AspectMode == 0)
									Current_AspectMode = CS_APP_ASPECT_MODE_NUM - 1;
								else
									Current_AspectMode--;

								CS_MW_SetAspectMode(Current_AspectMode);
								break;
							case AV_VIDEO_DEFINITION:
								if(Current_VideoDefinition == 0)
									Current_VideoDefinition = CS_APP_DEFINITION_NUM - 1;
								else
									Current_VideoDefinition--;

								Pre_VideoDefinition = Current_VideoDefinition;
								CS_MW_SetVideoDefinition(Current_VideoDefinition);
								break;
							case AV_VIDEO_OUTPUT:
								if(Current_VideoOutput == 0)
									Current_VideoOutput = CS_APP_OUTPUT_NUM - 1;
								else
									Current_VideoOutput--;

								CS_MW_SetVideoOutput(Current_VideoOutput);
								break;
							case AV_AUDIO_SPDIF_OUTPUT:
								if(Current_Audio_SPDIF_Output == 0)
									Current_Audio_SPDIF_Output = CS_APP_AUDIO_SPDIF_OUTPUT_NUM - 1;
								else
									Current_Audio_SPDIF_Output--;

								CS_MW_SetDefaultSpdifAudioType(Current_Audio_SPDIF_Output);
								break;
							default:
								if(Current_AspectRatio == 0)
									Current_AspectRatio = CS_APP_ASPECT_RATIO_NUM - 1;
								else
									Current_AspectRatio--;
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_AVMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;
				case CSAPP_KEY_RIGHT:
					{
						switch(Current_Item)
						{
							case AV_ASPECT_RATIO:
								if(Current_AspectRatio == CS_APP_ASPECT_RATIO_NUM - 1)
									Current_AspectRatio = 0;
								else
									Current_AspectRatio++;

								CS_MW_SetAspectRatio(Current_AspectRatio);
								break;
							case AV_ASPECT_MODE:
								if(Current_AspectMode == CS_APP_ASPECT_MODE_NUM - 1)
									Current_AspectMode = 0;
								else
									Current_AspectMode++;

								CS_MW_SetAspectMode(Current_AspectMode);
								break;
							case AV_VIDEO_DEFINITION:
								if(Current_VideoDefinition == CS_APP_DEFINITION_NUM - 1)
									Current_VideoDefinition = 0;
								else
									Current_VideoDefinition++;

								Pre_VideoDefinition = Current_VideoDefinition;
								CS_MW_SetVideoDefinition(Current_VideoDefinition);
								break;
							case AV_VIDEO_OUTPUT:
								if(Current_VideoOutput == CS_APP_OUTPUT_NUM - 1)
									Current_VideoOutput = 0;
								else
									Current_VideoOutput++;

								CS_MW_SetVideoOutput(Current_VideoOutput);
								break;
							case AV_AUDIO_SPDIF_OUTPUT:
								if(Current_Audio_SPDIF_Output == CS_APP_AUDIO_SPDIF_OUTPUT_NUM - 1)
									Current_Audio_SPDIF_Output = 0;
								else
									Current_Audio_SPDIF_Output++;

								CS_MW_SetDefaultSpdifAudioType(Current_Audio_SPDIF_Output);
								break;
							default:
								if(Current_AspectRatio == CS_APP_ASPECT_RATIO_NUM - 1)
									Current_AspectRatio = 0;
								else
									Current_AspectRatio++;
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_AVMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
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






