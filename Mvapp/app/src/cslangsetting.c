#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define	FIELDS_PER_LINE				        2

static CSAPP_Applet_t	CSApp_LnagSetting_Applets;
static U16				Current_Item = 0;
static U32				Current_OSD_Language = 0;
static U32				Current_SUB_Language = 0;
static U32				Current_AUD1_Language = 0;

static U32				ScreenWidth = CSAPP_OSD_MAX_WIDTH;

U16 Lang_Setting_Str[LANG_SETTING_MAX] = {
	CSAPP_STR_MENU_LANG,
	CSAPP_STR_SUB,
	CSAPP_STR_AUDIO_LANG
};

U16 Lang_Str[LANG_MAX] = {
	CSAPP_STR_ENGLISH,
	CSAPP_STR_TURKISH,
	CSAPP_STR_GERMAN,
	CSAPP_STR_FRANCE
#if 0
	,
	CSAPP_STR_GREEK,
	CSAPP_STR_ARABIC,
	CSAPP_STR_PERCIAN
#endif
};

U8	Lang_Arrow_Kind[LANG_SETTING_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	Lang_Enter_Kind[LANG_SETTING_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

static int Lang_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_LangSelectBar(HDC hdc, int y_gap, eMV_LangSetting_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Lang_Enter_Kind[esItem])
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
	
	if ( Lang_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_LangMenuBar(HDC hdc, U8 u8Focuskind, eMV_LangSetting_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_LangSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Lang_Setting_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		case LANG_OSD:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Lang_Str[Current_OSD_Language]), -1, &TmpRect, DT_CENTER);		
			break;
		case LANG_SUBTITLE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Lang_Str[Current_SUB_Language]), -1, &TmpRect, DT_CENTER);	
			break;
		case LANG_AUDIO:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Lang_Str[Current_AUD1_Language]), -1, &TmpRect, DT_CENTER);	
			break;
		default:
			break;
	}
}

void MV_Draw_Lang_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < LANG_SETTING_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_LangMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_LangMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}


CSAPP_Applet_t	CSApp_LangSetting(void)
{
	int   		BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   	msg;
	HWND  	hwndMain;
	MAINWINCREATE			CreateInfo;

	CSApp_LnagSetting_Applets = CSApp_Applet_Error;
		
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
	
	CreateInfo.dwStyle	 	= WS_VISIBLE;
	CreateInfo.dwExStyle 	= WS_EX_NONE;
	CreateInfo.spCaption 	= "cslangsetting window";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Lang_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= CSAPP_BLACK_COLOR;
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

	return CSApp_LnagSetting_Applets;
    
}


static int Lang_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;

	switch(message)
    {
        case MSG_CREATE:
			Current_Item = 0;

			Current_OSD_Language = CS_MW_GetCurrentMenuLanguage();
			if ( Current_OSD_Language > CS_APP_LANG_NUM_MAX )
				Current_OSD_Language = CS_APP_ENGLISH;
			
			Current_SUB_Language = CS_MW_GetCurrentSubLanguage();
			if ( Current_SUB_Language > CS_APP_LANG_NUM_MAX )
				Current_SUB_Language = CS_APP_ENGLISH;
			
			Current_AUD1_Language = CS_MW_GetCurrentAudioLanguage();
			if ( Current_AUD1_Language > CS_APP_LANG_NUM_MAX )
				Current_AUD1_Language = CS_APP_ENGLISH;
			//printf("\n== menu : %d, sub : %d, aud : %d \n", Current_OSD_Language, Current_SUB_Language, Current_AUD1_Language);
			break;
		case MSG_PAINT:	
			//printf("======== Language Setting MSG PAINT ===============\n");
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_LANGUAGE);

			hdc=BeginPaint(hwnd);
			MV_Draw_Lang_MenuBar(hdc);
			//MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_LANGUAGE);
			EndPaint(hwnd,hdc);			
			//printf("======== Language Setting MSG PAINT END ===============\n");
			return 0;

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
						case LANG_OSD:
							Current_OSD_Language = u8Result_Value;
							CS_MW_SetCurrentMenuLanguage(Current_OSD_Language);
							break;
						case LANG_SUBTITLE:
							Current_SUB_Language = u8Result_Value;
							CS_MW_SetCurrentSubtitleLanguage(Current_SUB_Language);
							break;
						case LANG_AUDIO:
							Current_AUD1_Language = u8Result_Value;
							CS_MW_SetCurrentAudioLanguage(Current_AUD1_Language);
							break;
						default:
							break;
					}
					
					SendMessage(hwnd,MSG_PAINT,0,0);
					
				}
				break;
			}
			
			switch(wparam)
			{
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
							case LANG_OSD:
								for ( i = 0 ; i < CS_APP_LANG_NUM_MAX ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Lang_Str[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LANG_NUM_MAX );
								stContents.u8TotalCount = CS_APP_LANG_NUM_MAX;
								stContents.u8Focus_Position = Current_OSD_Language;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case LANG_SUBTITLE:
								for ( i = 0 ; i < CS_APP_LANG_NUM_MAX ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Lang_Str[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LANG_NUM_MAX );
								stContents.u8TotalCount = CS_APP_LANG_NUM_MAX;
								stContents.u8Focus_Position = Current_SUB_Language;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case LANG_AUDIO:
								for ( i = 0 ; i < CS_APP_LANG_NUM_MAX ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Lang_Str[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LANG_NUM_MAX );
								stContents.u8TotalCount = CS_APP_LANG_NUM_MAX;
								stContents.u8Focus_Position = Current_AUD1_Language;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							default:
								break;
						}
					}
					break;
				case CSAPP_KEY_ESC:
					CSApp_LnagSetting_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					CSApp_LnagSetting_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_LnagSetting_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_LangMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					if(Current_Item == 0)
						Current_Item = LANG_SETTING_MAX - 1;
					else
						Current_Item--;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_LangMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
					MV_Draw_LangMenuBar(hdc, MV_UNFOCUS, Current_Item);

					if(Current_Item == LANG_SETTING_MAX - 1)
						Current_Item = 0;
					else
						Current_Item++;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_LangMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_LEFT:
					{
						U32	total_element;
						U32	*current;
						
						switch(Current_Item)
						{
							case LANG_OSD:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_OSD_Language;
								break;
							case LANG_SUBTITLE:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_SUB_Language;
								break;
							case LANG_AUDIO:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_AUD1_Language;
								break;
							default:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_OSD_Language;
								break;
						}

						if(*current == 0)
							*current = total_element-1;
						else
							(*current)--;

						switch(Current_Item)
						{
							case LANG_OSD:
								CS_MW_SetCurrentMenuLanguage(Current_OSD_Language);
								break;
							case LANG_SUBTITLE:
								CS_MW_SetCurrentSubtitleLanguage(Current_SUB_Language);
								break;
							case LANG_AUDIO:
								CS_MW_SetCurrentAudioLanguage(Current_AUD1_Language);
								break;
							default:
								break;
						}	

						SendMessage(hwnd,MSG_PAINT,0,0);
					}
					break;
				case CSAPP_KEY_RIGHT:
					{
						U32	total_element;
						U32	*current;
						
						switch(Current_Item)
						{
							case LANG_OSD:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_OSD_Language;
								break;
							case LANG_SUBTITLE:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_SUB_Language;
								break;
							case LANG_AUDIO:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_AUD1_Language;
								break;
							default:
								total_element = CS_APP_LANG_NUM_MAX;
								current = &Current_OSD_Language;
								break;
						}

						if(*current == total_element-1)
							*current = 0;
						else
							(*current)++;

						switch(Current_Item)
						{
							case LANG_OSD:
								CS_MW_SetCurrentMenuLanguage(Current_OSD_Language);
								break;
							case LANG_SUBTITLE:
								CS_MW_SetCurrentSubtitleLanguage(Current_SUB_Language);
								break;
							case LANG_AUDIO:
								CS_MW_SetCurrentAudioLanguage(Current_AUD1_Language);
								break;
							default:
								break;
						}

						SendMessage(hwnd,MSG_PAINT,0,0);
					}
					break;
				default:
					break;
			}
			break;
		case MSG_CLOSE:
			if(CS_DBU_CheckIfUserSettingDataChanged())
			{
				CS_DBU_SaveUserSettingDataInHW();
			}
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
		default:
			break;		
    }
    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}






