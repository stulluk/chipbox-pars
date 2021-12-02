#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define FONT_TEST  // 한개의 폰트 creation 테스트

#define	FIELDS_PER_LINE				        2

static CSAPP_Applet_t	CSApp_SYSSetting_Applets;
static U16				Current_Item = 0;
#ifdef	USE_LNB
static U16				Current_LNB_Power = 0;
#endif
static U16				Current_Banner_Time = 0;
static U16				Current_Transparency = 0;
static U16				Current_Channel_type = 0;
static U16				Current_Chlist_type = 0;
static U16				Recall_type = 0;
static U16				Power_type = 0;
static U16 				LED_type = 0;
static U8 				ANI_type = 0;
static U8 				HeartBit_type = 0; /* For Heart bit control By KB Kim 2011.03.11 */
static U8				Font_type = 0;
static U8				Font_size = 0;
static U8				Fix_Font_size = 0;
static U8				Skin_kind = 0;
static U8				Subtitle_type = 0;

U16 SYS_Setting_Str[CSAPP_SETTING_ITEM_MAX] = {
#ifdef	USE_LNB
							CSAPP_STR_LNBPOWER,
#endif
							CSAPP_STR_BANNER_TIME,
							CSAPP_STR_TRANSPARENCY,
							CSAPP_STR_CH_CHANGE,
							CSAPP_STR_CHLIST_TYPE,
							CSAPP_STR_RECALL,
							CSAPP_STR_POWER,
							CSAPP_STR_LED,
							CSAPP_STR_ANIMATION,
							CSAPP_STR_HEARTBIT,    /* For Heart bit control By KB Kim 2011.03.11 */
							CSAPP_STR_FONT,
							CSAPP_STR_FONT_SIZE,
							CSAPP_STR_SKIN,
							CSAPP_STR_SUB
						};

#ifdef	USE_LNB
U16 LNB_Power[CS_APP_LNB_P_NUM] = {
							CSAPP_STR_ON,
							CSAPP_STR_OFF
						};
#endif

U16 CH_Change[CS_APP_CH_CHANGE_NUM] = {
							CSAPP_STR_BLACK,
							CSAPP_STR_PAUSE
						};

U16 CH_List[CS_APP_LIST_NUM] = {
							CSAPP_STR_NORMAL,
							CSAPP_STR_EXTEND
						};


U16 Recall_option[CS_APP_RECALL_NUM] = {
							CSAPP_STR_SINGLE,
							CSAPP_STR_MULTI
						};

U16 Power_option[CS_APP_POWER_NUM] = {
							CSAPP_STR_SLEEP_MODE,
							CSAPP_STR_REAL_MODE
						};

U16 LED_option[CS_APP_LED_NUM] = {
							CSAPP_STR_OFF,
							CSAPP_STR_ON
						};

U16 ANI_option[CS_APP_LED_NUM] = {
							CSAPP_STR_OFF,
							CSAPP_STR_ON
						};

U16 SUBT_option[CS_APP_LED_NUM] = {
							CSAPP_STR_OFF,
							CSAPP_STR_ON
						};

U8	SYS_Arrow_Kind[CSAPP_SETTING_ITEM_MAX] = {
#ifdef	USE_LNB
	MV_SELECT,
#endif
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	SYS_Enter_Kind[CSAPP_SETTING_ITEM_MAX] = {
#ifdef	USE_LNB
	MV_SELECT,
#endif
	MV_STATIC,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8			Font_Size[EN_FONT_TYPE8_MAX] = { 22, 24, 26, 28, 30, 32 };

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static int Setting_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);

void MV_Draw_SYSSelectBar(HDC hdc, int y_gap, eMV_SYSSetting_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(SYS_Enter_Kind[esItem])
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
	
	if ( SYS_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_SYSMenuBar(HDC hdc, U8 u8Focuskind, eMV_SYSSetting_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	acTemp[100];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_SYSSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SYS_Setting_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
#ifdef	USE_LNB
		case CSAPP_SETTING_LNB:
			if ( Current_LNB_Power == eCS_DBU_ON )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(LNB_Power[CS_APP_LNB_P_ON]), -1, &TmpRect, DT_CENTER);		
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(LNB_Power[CS_APP_LNB_P_OFF]), -1, &TmpRect, DT_CENTER);		
			break;
#endif
		case CSAPP_SETTING_INFO:
			MV_Draw_LevelBar(hdc, &TmpRect, Current_Banner_Time, EN_ITEM_10_BAR_LEVEL);
			break;
		case CSAPP_SETTING_TRANS:
			MV_Draw_LevelBar(hdc, &TmpRect, (Current_Transparency - 55)/20, EN_ITEM_10_BAR_LEVEL);
			break;
		case CSAPP_SETTING_CH_CHANGE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CH_Change[Current_Channel_type]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_CH_LIST_TYPE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CH_List[Current_Chlist_type]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_RECALL:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Recall_option[Recall_type]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_POWER:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Power_option[Power_type]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_LED:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(LED_option[LED_type]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_ANI:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(ANI_option[ANI_type]), -1, &TmpRect, DT_CENTER);
			break;
		/* For Heart bit control By KB Kim 2011.03.11 */
		case CSAPP_SETTING_HEARTBIT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(LED_option[HeartBit_type]), -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_SETTING_FONT:
			//printf("\n\n\n=== FONT : %d - %s ======\n\n\n", Font_type, acFont_Name[Font_type]);
			CS_MW_DrawText(hdc, acFont_Name[Font_type], -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_FONT_SIZE:
			if ( Font_type == 0 )
			{
				memset(acTemp, 0x00, 100);
				sprintf(acTemp, "%d", Font_Size[Fix_Font_size]);
			} else {
				memset(acTemp, 0x00, 100);
				sprintf(acTemp, "%d", Font_size);
			}
			CS_MW_DrawText(hdc, acTemp, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_SKIN:
			//printf("\n===== %d ==========\n", Skin_kind);
			sprintf(acTemp, "%d", Skin_kind + 1);
			CS_MW_DrawText(hdc, acTemp, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_SETTING_AUTO_SUBT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(SUBT_option[Subtitle_type]), -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_SYS_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_SETTING_ITEM_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_SYSMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_SYSMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_Setting(void)
{
   	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_SYSSetting_Applets = CSApp_Applet_Error;
	
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
	CreateInfo.spCaption = "Setting";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Setting_Msg_cb;
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
	
	return CSApp_SYSSetting_Applets;
	
}



static int Setting_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{
	HDC 				hdc;

	switch(message)
    {
        case MSG_CREATE:			
			Current_Item = 0;
			Current_Channel_type = CS_DBU_GetCH_Change_Type();
			Current_Chlist_type = CS_DBU_GetCH_List_Type();
			Current_Transparency = CS_MW_GetTxprc();
			Current_Banner_Time = CS_DBU_GetBannerKeepTime();
#ifdef	USE_LNB
			Current_LNB_Power = CS_DBU_GetLNB_Power();
#endif
			Recall_type = CS_DBU_GetRecall_Type();
			Power_type = CS_DBU_GetPower_Type();
			LED_type = CS_DBU_GetLED_Type();
			Font_type = CS_DBU_GetFont_Type();
			Font_size = CS_DBU_GetFont_Size();
			Skin_kind = CS_DBU_Get_Skin();
			ANI_type = CS_DBU_GetANI_Type();
			HeartBit_type = CS_DBU_GetHeartBit();
			Subtitle_type = CS_DBU_Get_Use_SubTitle();
			Fix_Font_size = CS_DBU_Get_Fixed_Font_Size();
			break;
		case MSG_PAINT:
			{	
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_SYS);

				hdc=BeginPaint(hwnd);
				MV_Draw_SYS_MenuBar(hdc);
				MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_SYS);
				EndPaint(hwnd,hdc);
			}
			//printf("MSG_PAINT end : Font_Kind : %d\n", CS_DBU_GetFont_Kind());

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
#ifdef	USE_LNB
						case CSAPP_SETTING_LNB:
							if ( u8Result_Value == 0 )
							{
								Current_LNB_Power = eCS_DBU_ON;
								CS_MW_SetLNB_Power(eCS_DBU_ON);
							}
							else
							{
								Current_LNB_Power = eCS_DBU_OFF;
								CS_MW_SetLNB_Power(eCS_DBU_OFF);
							}
							break;
#endif
						case CSAPP_SETTING_INFO:
							break;
						case CSAPP_SETTING_TRANS:
							break;
						case CSAPP_SETTING_CH_CHANGE:
							Current_Channel_type = u8Result_Value;
							CS_DBU_SetCH_Change_Type(Current_Channel_type);
							break;
						case CSAPP_SETTING_CH_LIST_TYPE:
							Current_Chlist_type = u8Result_Value;
							CS_DBU_SetCH_List_Type(Current_Chlist_type);
							break;
						case CSAPP_SETTING_RECALL:
							Recall_type = u8Result_Value;
							CS_DBU_SetRecall_Type(Recall_type);
							break;
						case CSAPP_SETTING_POWER:
							Power_type = u8Result_Value;
							CS_DBU_SetPower_Type(Power_type);
							break;
						case CSAPP_SETTING_LED:
							LED_type = u8Result_Value;
							CS_DBU_SetLED_Type(LED_type);
							break;
						case CSAPP_SETTING_ANI:
							ANI_type = u8Result_Value;
							CS_DBU_SetANI_Type(ANI_type);
							break;
						/* For Heart bit control By KB Kim 2011.03.11 */
						case CSAPP_SETTING_HEARTBIT:
							HeartBit_type = u8Result_Value;
							CS_DBU_SetHeartBit(HeartBit_type);
							break;
						case CSAPP_SETTING_FONT:
							Font_type = u8Result_Value;
							CS_DBU_SetFont_Type(Font_type);

							CS_MW_Font_Creation(0);
							SendMessage(hwnd,MSG_PAINT,0,0);
							break;
						case CSAPP_SETTING_FONT_SIZE:
							if ( Font_type == 0 )
							{
								Fix_Font_size = u8Result_Value;
								CS_DBU_Set_Fixed_Font_Size(Fix_Font_size);
							}
							else
							{								
								Font_size = u8Result_Value + FONT_SIZE_MIN;
								CS_DBU_SetFont_Size(Font_size);
							}
							CS_MW_Font_Creation(0);
							SendMessage(hwnd,MSG_PAINT,0,0);
							break;
						case CSAPP_SETTING_SKIN:
							{
								U8			Backup_Skin;
								
								Backup_Skin = Skin_kind;
								Skin_kind = u8Result_Value;
								
								if( Skin_kind != Backup_Skin )
								{
									hdc=BeginPaint(hwnd);
									MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
									EndPaint(hwnd,hdc);
									
									CS_DBU_Set_Skin(Skin_kind);
									CS_DBU_SaveUserSettingDataInHW();

									MV_InitBmp();

									MV_Resource_Change_Cfg_File(Skin_kind);

									MV_LoadBmp_NoProgress(hwnd);
	
									hdc = BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);
									
									SendMessage(hwnd,MSG_PAINT,0,0);
								}
							}
							break;
						case CSAPP_SETTING_AUTO_SUBT:
							Subtitle_type = u8Result_Value;
							CS_DBU_Set_Use_SubTitle(Subtitle_type);
							break;
						default:
							break;
					}
					hdc=BeginPaint(hwnd);
					MV_Draw_SYSMenuBar(hdc, MV_FOCUS, Current_Item);
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
#if 0
				case CSAPP_KEY_GREEN:
					{
						int i;
						char tempstr[20];
						for( i = 0 ; i < 255 ; i++ )
						{
							//CS_MW_SetTxprc(i);
							hdc=BeginPaint(hwnd);
							MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
							MV_FillBox( hdc, ScalerWidthPixel(300),ScalerHeigthPixel( 360 ), ScalerWidthPixel(100),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );	
							SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
							SetBkMode(hdc,BM_TRANSPARENT);
							sprintf(tempstr , "%d", i );
							MV_CS_MW_TextOut( hdc, ScalerWidthPixel(300),ScalerHeigthPixel(360), tempstr);
							EndPaint(hwnd,hdc);
							
							CS_AV_SetOSDAlpha(i);
							usleep( 1000 * 1000 );
						}
					}				
					break;
#endif
				case CSAPP_KEY_ESC:
					CSApp_SYSSetting_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					if ( b8Last_App_Status == CSApp_Applet_Change_Fav )
						CSApp_SYSSetting_Applets = CSApp_Applet_MainMenu;
					else
						CSApp_SYSSetting_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_ENTER:
					{
						int						i = 0;
						RECT					smwRect;
						stPopUp_Window_Contents stContents;

						memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
						if ( Current_Item == CSAPP_SETTING_FONT_SIZE || Current_Item == CSAPP_SETTING_FONT)
						{
							smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item - 3 );
							smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4) - 150;
							smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
						} else {
							smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
							smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
							smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
						}
						
						switch(Current_Item)
						{
#ifdef	USE_LNB
							case CSAPP_SETTING_LNB:
								for ( i = 0 ; i < CS_APP_LNB_P_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(LNB_Power[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LNB_P_NUM );
								stContents.u8TotalCount = CS_APP_LNB_P_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
#endif
							case CSAPP_SETTING_INFO:
								break;
							case CSAPP_SETTING_TRANS:
								break;
							case CSAPP_SETTING_CH_CHANGE:
								for ( i = 0 ; i < CS_APP_CH_CHANGE_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(CH_Change[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_CH_CHANGE_NUM );
								stContents.u8TotalCount = CS_APP_CH_CHANGE_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_CH_LIST_TYPE:
								for ( i = 0 ; i < CS_APP_LIST_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(CH_List[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LIST_NUM );
								stContents.u8TotalCount = CS_APP_LIST_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_RECALL:
								for ( i = 0 ; i < CS_APP_RECALL_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Recall_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_RECALL_NUM );
								stContents.u8TotalCount = CS_APP_RECALL_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_POWER:
								for ( i = 0 ; i < CS_APP_POWER_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Power_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_POWER_NUM );
								stContents.u8TotalCount = CS_APP_POWER_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_LED:
								for ( i = 0 ; i < CS_APP_LED_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(LED_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LED_NUM );
								stContents.u8TotalCount = CS_APP_LED_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_ANI:
								for ( i = 0 ; i < CS_APP_LED_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(ANI_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LED_NUM );
								stContents.u8TotalCount = CS_APP_LED_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							/* For Heart bit control By KB Kim 2011.03.11 */
							case CSAPP_SETTING_HEARTBIT:
								for ( i = 0 ; i < CS_APP_LED_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(LED_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LED_NUM );
								stContents.u8TotalCount = CS_APP_LED_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_FONT:
								for ( i = 0 ; i < u8Font_Kind ; i++ )
									sprintf(stContents.Contents[i], "%s", acFont_Name[i]);
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( u8Font_Kind );
								stContents.u8TotalCount = u8Font_Kind;
								stContents.u8Focus_Position = CS_DBU_GetFont_Type();
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_FONT_SIZE:
								if ( Font_type == 0 )
								{
									for ( i = 0 ; i < EN_FONT_TYPE8_MAX ; i++ )
										sprintf(stContents.Contents[i], "%d", Font_Size[i]);
						
									smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( EN_FONT_TYPE8_MAX );
									stContents.u8TotalCount = EN_FONT_TYPE8_MAX;
									stContents.u8Focus_Position = CS_DBU_Get_Fixed_Font_Size();
									MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								} else {
									for ( i = FONT_SIZE_MIN ; i < FONT_SIZE_MAX+1 ; i++ )
										sprintf(stContents.Contents[i-FONT_SIZE_MIN], "%d", i);

									smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ((FONT_SIZE_MAX+1) - FONT_SIZE_MIN);
									stContents.u8TotalCount = ((FONT_SIZE_MAX+1) - FONT_SIZE_MIN);
									stContents.u8Focus_Position = CS_DBU_GetFont_Size()-FONT_SIZE_MIN;
									MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								}
								break;
							case CSAPP_SETTING_SKIN:
								for ( i = 0 ; i < CFG_Skin_Kind ; i++ )
									sprintf(stContents.Contents[i], "%d", i + 1);
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CFG_Skin_Kind );
								stContents.u8TotalCount = CFG_Skin_Kind;
								stContents.u8Focus_Position = CS_DBU_Get_Skin();
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							case CSAPP_SETTING_AUTO_SUBT:
								for ( i = 0 ; i < CS_APP_LED_NUM ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(SUBT_option[i]));
						
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * ( CS_APP_LED_NUM );
								stContents.u8TotalCount = CS_APP_LED_NUM;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							default:
								break;
						}
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_SYSSetting_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_SYSMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					if(Current_Item == 0)
						Current_Item = CSAPP_SETTING_ITEM_MAX - 1;
					else
						Current_Item--;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_SYSMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
					MV_Draw_SYSMenuBar(hdc, MV_UNFOCUS, Current_Item);

					if(Current_Item == CSAPP_SETTING_ITEM_MAX - 1)
						Current_Item = 0;
					else
						Current_Item++;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_SYSMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);

					break;
				case CSAPP_KEY_LEFT:
					{
						switch(Current_Item)
						{
#ifdef	USE_LNB
							case CSAPP_SETTING_LNB:
								if(Current_LNB_Power == eCS_DBU_OFF)
									Current_LNB_Power = eCS_DBU_ON;
								else
									Current_LNB_Power = eCS_DBU_OFF;

								CS_MW_SetLNB_Power(Current_LNB_Power);
								break;
#endif
							case CSAPP_SETTING_INFO:
								if(Current_Banner_Time != CA_APP_BANNET_TIME_NIM)
									Current_Banner_Time--;

								CS_DBU_SetBannerKeepTime(Current_Banner_Time);
								break;
							case CSAPP_SETTING_TRANS:
								if ( Current_Transparency > 75 )
									Current_Transparency -= 20;
								else
									Current_Transparency = 55;

								CS_MW_SetTxprc(Current_Transparency);
								break;
							case CSAPP_SETTING_CH_CHANGE:
								if( Current_Channel_type == CS_APP_CH_CHANGE_BALCK )
									Current_Channel_type = CS_APP_CH_CHANGE_PAUSE;
								else
									Current_Channel_type = CS_APP_CH_CHANGE_BALCK;
								
								CS_DBU_SetCH_Change_Type(Current_Channel_type);
								break;
							case CSAPP_SETTING_CH_LIST_TYPE:
								if( Current_Chlist_type == CS_APP_LIST_NORMAL )
									Current_Chlist_type = CS_APP_LIST_EXTEND;
								else
									Current_Chlist_type = CS_APP_LIST_NORMAL;
								
								CS_DBU_SetCH_List_Type(Current_Chlist_type);
								break;
							case CSAPP_SETTING_RECALL:
								if( Recall_type == CS_APP_RECALL_ONE )
									Recall_type = CS_APP_RECALL_MULTI;
								else
									Recall_type = CS_APP_RECALL_ONE;
								
								CS_DBU_SetRecall_Type(Recall_type);
								break;
							case CSAPP_SETTING_POWER:
								if( Power_type == CS_APP_POWER_SLEEP )
									Power_type = CS_APP_POWER_REAL;
								else
									Power_type = CS_APP_POWER_SLEEP;
								
								CS_DBU_SetPower_Type(Power_type);
								break;
							case CSAPP_SETTING_LED:
								if( LED_type == CS_APP_LED_OFF )
									LED_type = CS_APP_LED_ON;
								else
									LED_type = CS_APP_LED_OFF;
								
								CS_DBU_SetLED_Type(LED_type);
								break;
							case CSAPP_SETTING_ANI:
								if( ANI_type == CS_APP_LED_OFF )
									ANI_type = CS_APP_LED_ON;
								else
									ANI_type = CS_APP_LED_OFF;
								
								CS_DBU_SetANI_Type(ANI_type);
								break;
							/* For Heart bit control By KB Kim 2011.03.11 */
							case CSAPP_SETTING_HEARTBIT:
								if( HeartBit_type == CS_APP_LED_OFF )
									HeartBit_type = CS_APP_LED_ON;
								else
									HeartBit_type = CS_APP_LED_OFF;
								
								CS_DBU_SetHeartBit(HeartBit_type);
								break;
							case CSAPP_SETTING_FONT:
								if( Font_type == 0 )
									Font_type = u8Font_Kind - 1;
								else
									Font_type--;
								
								CS_DBU_SetFont_Type(Font_type);
								CS_MW_Font_Creation(0);
								SendMessage(hwnd,MSG_PAINT,0,0);
								break;
							case CSAPP_SETTING_FONT_SIZE:
								if ( Font_type == 0 )
								{
									if( Fix_Font_size == 0 )
										Fix_Font_size = EN_FONT_TYPE8_MAX - 1;
									else
										Fix_Font_size--;

									CS_DBU_Set_Fixed_Font_Size(Fix_Font_size);
								} else {
									if( Font_size == FONT_SIZE_MIN )
										Font_size = FONT_SIZE_MAX;
									else
										Font_size--;
									
									CS_DBU_SetFont_Size(Font_size);
								}
								
								CS_MW_Font_Creation(0);
								SendMessage(hwnd,MSG_PAINT,0,0);
								break;
							case CSAPP_SETTING_AUTO_SUBT:
								if ( Subtitle_type == CS_APP_LED_OFF )
									Subtitle_type = CS_APP_LED_ON;
								else
									Subtitle_type = CS_APP_LED_OFF;
								
								CS_DBU_Set_Use_SubTitle(Subtitle_type);
								break;
								
							default:
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_SYSMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;
				case CSAPP_KEY_RIGHT:
					{
						switch(Current_Item)
						{
#ifdef	USE_LNB
							case CSAPP_SETTING_LNB:
								if(Current_LNB_Power == eCS_DBU_OFF)
									Current_LNB_Power = eCS_DBU_ON;
								else
									Current_LNB_Power = eCS_DBU_OFF;
								
								CS_MW_SetLNB_Power(Current_LNB_Power);
								break;
#endif
							case CSAPP_SETTING_INFO:
								if(Current_Banner_Time != CS_APP_BANNER_TIME_NUM)
									Current_Banner_Time++;

								CS_DBU_SetBannerKeepTime(Current_Banner_Time);
								break;
							case CSAPP_SETTING_TRANS:
								if ( Current_Transparency < 235 )
									Current_Transparency += 20;
								else
									Current_Transparency = 255;
								
								CS_MW_SetTxprc(Current_Transparency);
								break;
							case CSAPP_SETTING_CH_CHANGE:
								if( Current_Channel_type == CS_APP_CH_CHANGE_BALCK )
									Current_Channel_type = CS_APP_CH_CHANGE_PAUSE;
								else
									Current_Channel_type = CS_APP_CH_CHANGE_BALCK;
								
								CS_DBU_SetCH_Change_Type(Current_Channel_type);
								break;
							
							case CSAPP_SETTING_CH_LIST_TYPE:
								if( Current_Chlist_type == CS_APP_LIST_NORMAL )
									Current_Chlist_type = CS_APP_LIST_EXTEND;
								else
									Current_Chlist_type = CS_APP_LIST_NORMAL;
								
								CS_DBU_SetCH_List_Type(Current_Chlist_type);
								break;
							case CSAPP_SETTING_RECALL:
								if( Recall_type == CS_APP_RECALL_ONE )
									Recall_type = CS_APP_RECALL_MULTI;
								else
									Recall_type = CS_APP_RECALL_ONE;
								
								CS_DBU_SetRecall_Type(Recall_type);
								break;
							case CSAPP_SETTING_POWER:
								if( Power_type == CS_APP_POWER_SLEEP )
									Power_type = CS_APP_POWER_REAL;
								else
									Power_type = CS_APP_POWER_SLEEP;
								
								CS_DBU_SetPower_Type(Power_type);
								break;
							case CSAPP_SETTING_LED:
								if( LED_type == CS_APP_LED_OFF )
									LED_type = CS_APP_LED_ON;
								else
									LED_type = CS_APP_LED_OFF;
								
								CS_DBU_SetLED_Type(LED_type);
								break;
							case CSAPP_SETTING_ANI:
								if( ANI_type == CS_APP_LED_OFF )
									ANI_type = CS_APP_LED_ON;
								else
									ANI_type = CS_APP_LED_OFF;
								
								CS_DBU_SetANI_Type(ANI_type);
								break;
							/* For Heart bit control By KB Kim 2011.03.11 */
							case CSAPP_SETTING_HEARTBIT:
								if( HeartBit_type == CS_APP_LED_OFF )
									HeartBit_type = CS_APP_LED_ON;
								else
									HeartBit_type = CS_APP_LED_OFF;
								
								CS_DBU_SetHeartBit(HeartBit_type);
								break;
							case CSAPP_SETTING_FONT:
								if( Font_type == u8Font_Kind - 1 )
									Font_type = 0;
								else
									Font_type++;
								
								CS_DBU_SetFont_Type(Font_type);
								CS_MW_Font_Creation(0);
								SendMessage(hwnd,MSG_PAINT,0,0);
								break;
							case CSAPP_SETTING_FONT_SIZE:
								if ( Font_type == 0 )
								{
									if ( Fix_Font_size == EN_FONT_TYPE8_MAX - 1 )
										Fix_Font_size = 0;
									else
										Fix_Font_size++;
									
									CS_DBU_Set_Fixed_Font_Size(Fix_Font_size);
								} else {
									if( Font_size == FONT_SIZE_MAX )
										Font_size = FONT_SIZE_MIN;
									else
										Font_size++;
									
									CS_DBU_SetFont_Size(Font_size);
								}
								
								CS_MW_Font_Creation(0);
								SendMessage(hwnd,MSG_PAINT,0,0);
								break;
							case CSAPP_SETTING_AUTO_SUBT:
								if ( Subtitle_type == CS_APP_LED_OFF )
									Subtitle_type = CS_APP_LED_ON;
								else
									Subtitle_type = CS_APP_LED_OFF;
								CS_DBU_Set_Use_SubTitle(Subtitle_type);
								break;
							default:
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_SYSMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_RED:
					CSApp_SYSSetting_Applets = CSApp_Applet_Change_Fav;
					SendMessage(hwnd,MSG_CLOSE,0,0);
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


