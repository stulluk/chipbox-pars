#include "linuxos.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "database.h"
#include "casapi.h"

static CSAPP_Applet_t		CSApp_Cas_Applets;

/* For S_CAM Menu By Jacob 14 May 2011 */
static U8                   SCamStatus = 0;

static U8					u8Card_ID = 1;
static U16					u16CAS_ID = 0xFFFF;
static BOOL					b8Check_Card = FALSE;

static U32					SystemSettingItemIdx[CSAPP_CAS_ITEM_MAX]={
								CSAPP_STR_CARD_STATUS,
								CSAPP_STR_CARD,
								CSAPP_STR_CARD
							};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static U32					pInfoText[CSAPP_CAS_CARD_STATUS_MAX] = {
								CSAPP_STR_EMPTY_SLOT,
								CSAPP_STR_INSERTED_CARD
							};

static int CAS_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_CASMenuBar(HDC hdc, U8 u8Focuskind, eMV_CAS_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	TempStr[100];

	memset(TempStr, 0x00, 100);

	if( u8Focuskind == MV_UNFOCUS )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}

	switch(esItem)
	{
		case CSAPP_CAS_ITEM_CARD_STATUS:
			sprintf(TempStr, "%s", CS_MW_LoadStringByIdx(SystemSettingItemIdx[esItem]));
			break;
		case CSAPP_CAS_ITEM_CARD:
			sprintf(TempStr, "%s %02d CAID", CS_MW_LoadStringByIdx(SystemSettingItemIdx[esItem]), u8Card_ID);
			break;
		case CSAPP_CAS_ITEM_CARD_NAME:
			sprintf(TempStr, "%s %02d %s", CS_MW_LoadStringByIdx(SystemSettingItemIdx[esItem]), u8Card_ID, CS_MW_LoadStringByIdx(CSAPP_STR_NAME));
			break;
		default:
			break;
	}

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), TempStr);

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		case CSAPP_CAS_ITEM_CARD_STATUS:
			if ( b8Check_Card == TRUE )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(pInfoText[CSAPP_CAS_INSERT_CARD]), -1, &TmpRect, DT_CENTER);		
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(pInfoText[CSAPP_CAS_EMPTY_SLOT]), -1, &TmpRect, DT_CENTER);		
			break;
		case CSAPP_CAS_ITEM_CARD:
			if ( b8Check_Card == TRUE )
			{
				U8				u8Temp_str[100];
				U16				u16Length = u16CAS_ID;
				
				if ( u16Length == 0 )
				{	
					CS_MW_DrawText(hdc, "Insert Card", -1, &TmpRect, DT_CENTER);
				} else {	
					//sprintf(u8Temp_str, "Card Num : 0x%04X , Name : %s", u16CAS_ID, cardName);
					sprintf(u8Temp_str, "0x%04X", u16CAS_ID);
					CS_MW_DrawText(hdc, u8Temp_str, -1, &TmpRect, DT_CENTER);
				}
			}
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(pInfoText[CSAPP_CAS_EMPTY_SLOT]), -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_CAS_ITEM_CARD_NAME:
			if ( b8Check_Card == TRUE )
			{
				U8  			cardName[50];
				U8				u8Temp_str[100];
				U16				u16Length = 0;
			
				if ( u16CAS_ID )
				{
					CasGetSystemInfo (u16CAS_ID, cardName);
					u16Length = u16CAS_ID;
				} else {
					u16Length = 0;
				}

				if ( u16Length == 0 )
				{	
					CS_MW_DrawText(hdc, "Insert Card", -1, &TmpRect, DT_CENTER);
				} else {	
					sprintf(u8Temp_str, "%s", cardName);
					CS_MW_DrawText(hdc, u8Temp_str, -1, &TmpRect, DT_CENTER);
				}
			}
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(pInfoText[CSAPP_CAS_EMPTY_SLOT]), -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_CAS_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_CAS_ITEM_MAX ; i++ )
	{
		MV_Draw_CASMenuBar(hdc, MV_UNFOCUS, i);
	}
}

/* For S_CAM Menu By Jacob 14 May 2011 */
void MV_Draw_CAS_Bottom(HDC hdc)
{
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(50) );
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_CAS_DETAIL_INFO));

	if (SCamStatus)
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_OSCAM));
	}
}

void MV_Draw_CAS_Info(HWND hwnd, U16 tmpCasId)
{
	HDC				hdc;
	U8  			cardName[50];
	U32				u32Length = 0;
	U8				u8Temp_str[100];
	int 			y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP );
	RECT			TmpRect;

	TmpRect.left	= ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	= TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);
	
	if ( tmpCasId )
	{
		CasGetSystemInfo (tmpCasId, cardName);
		u32Length = tmpCasId;
	} else {
		u32Length = 0;
	}

	hdc=BeginPaint(hwnd);
	memset(u8Temp_str, 0x00, 100);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	if ( u32Length == 0 )
	{	
		MV_FillBox( hdc, ScalerWidthPixel(TmpRect.left), ScalerHeigthPixel(TmpRect.top), ScalerWidthPixel(TmpRect.right - TmpRect.left), ScalerHeigthPixel(TmpRect.bottom - TmpRect.top) );
		CS_MW_DrawText(hdc, "Insert Card", -1, &TmpRect, DT_CENTER);
	}
	else
	{	
		sprintf(u8Temp_str, "Card Num : %X , Name : %s", tmpCasId, cardName);

		MV_FillBox( hdc, ScalerWidthPixel(TmpRect.left), ScalerHeigthPixel(TmpRect.top), ScalerWidthPixel(TmpRect.right - TmpRect.left), ScalerHeigthPixel(TmpRect.bottom - TmpRect.top) );
		CS_MW_DrawText(hdc, u8Temp_str, -1, &TmpRect, DT_CENTER);
	}
	EndPaint(hwnd,hdc);
}

CSAPP_Applet_t CSApp_CAS(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Cas_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "cas info";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = CAS_Msg_cb;
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

	return CSApp_Cas_Applets;   
}


static int CAS_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
	U8					u8Key_length = 0;
	char				acKey_Value[256];
   
	switch(message)
   	{
		case MSG_CREATE:
			u8Key_length = 16;
			u8Card_ID = 1;
			memset(acKey_Value, 0x00, 256);
			b8Check_Card = FALSE;
			u16CAS_ID = CasGetCurrentCardId(0);
			/* For S_CAM Menu By Jacob 14 May 2011 */
			SCamStatus = StbSGetOscamStatus();
			break;
			
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_CAS);
			
			if ( u16CAS_ID != 0 )
				b8Check_Card = TRUE;
			
			hdc=BeginPaint(hwnd);
			MV_Draw_CAS_MenuBar(hdc);

			/* For S_CAM Menu By Jacob 14 May 2011 */
			MV_Draw_CAS_Bottom(hdc);
			// MV_TOOLS_draw_help_banner(hdc, EN_ITEM_FOCUS_CAS);
			
			//MV_Draw_CAS_Info(hwnd, 0);
			EndPaint(hwnd,hdc);
			return 0;

		/* For S_CAM Menu By Jacob 14 May 2011 */
		case MSG_S_CAM_CHANGED:
			SCamStatus = StbSGetOscamStatus();
			hdc=BeginPaint(hwnd);
			MV_Draw_CAS_Bottom(hdc);
			EndPaint(hwnd,hdc);
			break;
			
		case MSG_SMART_CARD_INSERT:
			b8Check_Card = TRUE;
			u16CAS_ID = (U16)wparam;
			
			hdc=BeginPaint(hwnd);
			MV_Draw_CAS_MenuBar(hdc);
			EndPaint(hwnd,hdc);
			break;

		case MSG_SMART_CARD_REMOVE:
			b8Check_Card = FALSE;
			
			hdc=BeginPaint(hwnd);
			MV_Draw_CAS_MenuBar(hdc);
			EndPaint(hwnd,hdc);
			break;
		
		case MSG_KEYDOWN:
			if ( MV_Get_HexaKeypad_Status() == TRUE )
			{
				MV_HexaKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ENTER )
					strcpy(acKey_Value, MV_Get_HexaEdited_String());
				
				break;
			} else if (MV_Get_Password_Flag() == TRUE)
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
							
							CSApp_Cas_Applets=CSApp_Applet_OSCAM_Setting;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
						
					case CSAPP_KEY_ENTER:
						if(MV_Password_Retrun_Value() == TRUE)
						{
							MV_Password_Set_Flag(FALSE);
							hdc = BeginPaint(hwnd);
							MV_Restore_PopUp_Window( hdc );
							EndPaint(hwnd,hdc);
							
							CSApp_Cas_Applets=CSApp_Applet_OSCAM_Setting;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
				}
				break;
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_RED:
#if 0
					sprintf(acKey_Value, "1234567890abcdef");
					printf("=== %s ===\n", acKey_Value);

					MV_Draw_HexaKeypad(hwnd, acKey_Value, u8Key_length);
#else
					CSApp_Cas_Applets=CSAPP_Applet_KeyEdit;
					SendMessage(hwnd,MSG_CLOSE,0,0);
#endif
					break;
					
				case CSAPP_KEY_BLUE:
					/* For S_CAM Menu By Jacob 14 May 2011 */
					if (SCamStatus)
					{
						MV_Draw_Password_Window(hwnd);
					}
					break;
					
				case CSAPP_KEY_ESC:
					CSApp_Cas_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_ENTER:
				case CSAPP_KEY_MENU:
					// CSApp_Cas_Applets=b8Last_App_Status;
					CSApp_Cas_Applets = CSApp_Applet_MainMenu;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Cas_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
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



