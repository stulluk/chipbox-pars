#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define	FIELDS_PER_LINE			2

static CSAPP_Applet_t			CSApp_PinSetting_Applets;
static U16						Current_Item = CSAPP_PIN_ENABLE;
static tPIN_keys_t				keys_info[3];
static U16						Current_PinEnable = 1;
static U16						Boot_PinEnable = 0;
static U16						Install_PinEnable = 0;
static U16						Edit_PinEnable = 0;
static U16						Channel_PinEnable = 0;
static U16						Change_Enable = 0;

static U8						Pin_Edit_Count = 0;
static char 					Now_Pin[5];
static char						New_Pin[5];
static char 					Confirm_Pin[5];

static const U16 PinEnable[CS_APP_ENABLE_PIN_NUM] = {
											CSAPP_STR_PIN_DISABLE,
											CSAPP_STR_PIN_ENABLE,
										};

static const U16 Pin_STR[CSAPP_PIN_SETTING_ITEM_MAX] = {
											CSAPP_STR_PIN_STATUS,
											CSAPP_STR_PIN_BOOT,
											CSAPP_STR_PIN_MENU,
											CSAPP_STR_PIN_EDIT,
											CSAPP_STR_PIN_CHANNEL,
											CSAPP_STR_PIN_CHANGE,
											CSAPP_STR_NOW_PIN,
										    CSAPP_STR_NEW_PIN,
										    CSAPP_STR_COMFIRM_PIN
										};

static U8	PIN_Arrow_Kind[CSAPP_PIN_SETTING_ITEM_MAX] = {
											MV_SELECT,
											MV_SELECT,
											MV_SELECT,
											MV_SELECT,
											MV_SELECT,
											MV_SELECT,
											MV_STATIC,
											MV_STATIC,
											MV_STATIC
										};

static U8	PIN_Enter_Kind[CSAPP_PIN_SETTING_ITEM_MAX] = {
											MV_STATIC,
											MV_STATIC,
											MV_STATIC,
											MV_STATIC,
											MV_STATIC,
											MV_STATIC,
											MV_NUMERIC,
											MV_NUMERIC,
											MV_NUMERIC
										};

static char	Edit_Count_Underline[5][5] = {
											"    ",
											"_   ",
											" _  ",
											"  _ ",
											"   _"
										};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static int Pin_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_PinSelectBar(HDC hdc, int y_gap, eMV_PinSetting_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(PIN_Enter_Kind[esItem])
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
	
	if ( PIN_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_PinMenuBar(HDC hdc, U8 u8Focuskind, eMV_PinSetting_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_PinSelectBar(hdc, y_gap, esItem);
	} else {

		if ( esItem == CSAPP_PIN_NOW || esItem == CSAPP_PIN_NEW || esItem == CSAPP_PIN_COMFIRM)
		{
			if ( Change_Enable )
				SetTextColor(hdc,MV_BAR_ENABLE_CHAR_COLOR);
			else
				SetTextColor(hdc,MV_BAR_DISABLE_CHAR_COLOR);
		} else
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Pin_STR[esItem]));

	//printf("\n################ %d - %d ###############\n", esItem, Pin_Edit_Count);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[20];
		
		case CSAPP_PIN_ENABLE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Current_PinEnable]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_BOOT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Boot_PinEnable]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_MENU:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Install_PinEnable]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_EDIT:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Edit_PinEnable]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_CHANNEL:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Channel_PinEnable]), -1, &TmpRect, DT_CENTER);
			break;
		case CASPP_PIN_CHANGE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(PinEnable[Change_Enable]), -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_NOW:
			
			if ( Change_Enable )
			{
				SetTextColor(hdc,MVAPP_RED_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				sprintf(temp_str, "%s", Edit_Count_Underline[Pin_Edit_Count]);
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
				
				if ( u8Focuskind == MV_FOCUS )
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
				else
					SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);

				SetBkMode(hdc,BM_TRANSPARENT);
			}

			switch( strlen( Now_Pin ) )
			{
				case 1:
					sprintf(temp_str, "%s", "*---");
					break;
				case 2:
					sprintf(temp_str, "%s", "**--");
					break;
				case 3:
					sprintf(temp_str, "%s", "***-");
					break;
				case 4:
					sprintf(temp_str, "%s", "****");
					break;
				default:
					sprintf(temp_str, "%s", "----");
					break;
			}
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_NEW:

			if ( Change_Enable )
			{
				SetTextColor(hdc,MVAPP_RED_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				sprintf(temp_str, "%s", Edit_Count_Underline[Pin_Edit_Count]);
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
				
				if ( u8Focuskind == MV_FOCUS )
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
				else
					SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);

				SetBkMode(hdc,BM_TRANSPARENT);
			}

			switch( strlen( New_Pin ) )
			{
				case 1:
					sprintf(temp_str, "%s", "*---");
					break;
				case 2:
					sprintf(temp_str, "%s", "**--");
					break;
				case 3:
					sprintf(temp_str, "%s", "***-");
					break;
				case 4:
					sprintf(temp_str, "%s", "****");
					break;
				default:
					sprintf(temp_str, "%s", "----");
					break;
			}
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case CSAPP_PIN_COMFIRM:

			if ( Change_Enable )
			{
				SetTextColor(hdc,MVAPP_RED_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				sprintf(temp_str, "%s", Edit_Count_Underline[Pin_Edit_Count]);
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);

				if ( u8Focuskind == MV_FOCUS )
					SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
				else
					SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);

				SetBkMode(hdc,BM_TRANSPARENT);			
			}

			switch( strlen( Confirm_Pin ) )
			{
				case 1:
					sprintf(temp_str, "%s", "*---");
					break;
				case 2:
					sprintf(temp_str, "%s", "**--");
					break;
				case 3:
					sprintf(temp_str, "%s", "***-");
					break;
				case 4:
					sprintf(temp_str, "%s", "****");
					break;
				default:
					sprintf(temp_str, "%s", "----");
					break;
			}
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_Pin_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_PIN_SETTING_ITEM_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_PinMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}


CSAPP_Applet_t	CSApp_PinSetting(void)
{
	int   			BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   			msg;
	HWND  			hwndMain;
	MAINWINCREATE	CreateInfo;

	CSApp_PinSetting_Applets = CSApp_Applet_Error;
		
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
	CreateInfo.spCaption 	= "cspinsetting window";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Pin_Msg_cb;
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
	return CSApp_PinSetting_Applets;
    
}


static int Pin_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC					hdc;
	char				key_in = 0x00;

	switch(message)
	{
		case MSG_CREATE:
			memset (&Now_Pin, 0, sizeof (char) * 5);
			memset (&New_Pin, 0, sizeof (char) * 5);
			memset (&Confirm_Pin, 0, sizeof (char) * 5);
			
			Change_Enable = CS_APP_DISABLE_PIN;			
			Current_Item = CSAPP_PIN_ENABLE;

			if (CS_DBU_GetParentalLockStatus())  /*All Lock status*/
			{
				Current_PinEnable = CS_APP_ENABLE_PIN;
			}
			else
				Current_PinEnable = CS_APP_DISABLE_PIN;

			if (CS_DBU_GetBootLockStatus())  /*Installation Lock status*/
			{
				Boot_PinEnable = CS_APP_ENABLE_PIN;
			}
			else
				Boot_PinEnable = CS_APP_DISABLE_PIN;

			if (CS_DBU_GetInstallLockStatus())  /*Installation Lock status*/
			{
				Install_PinEnable = CS_APP_ENABLE_PIN;
			}
			else
				Install_PinEnable = CS_APP_DISABLE_PIN;

			if (CS_DBU_GetEditLockStatus())  /*Channel Edit Lock status*/
			{
				Edit_PinEnable = CS_APP_ENABLE_PIN;
			}
			else
				Edit_PinEnable = CS_APP_DISABLE_PIN;
			
			if (CS_MW_GetServicesLockStatus())  /*Channel Lock status*/
			{
				Channel_PinEnable = CS_APP_ENABLE_PIN;
			}
			else
				Channel_PinEnable = CS_APP_DISABLE_PIN;

			memset(keys_info, 0, CSAPP_PIN_SETTING_ITEM_MAX*sizeof(tPIN_keys_t));
			break;
		case MSG_PAINT:
			{                        
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_PARENTAL);

				hdc=BeginPaint(hwnd);
				
				MV_Draw_Pin_MenuBar(hdc);
				MV_System_draw_help_banner(hdc, EN_ITEM_FOCUS_PARENTAL);

				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_PIN_PAINT:
			{
				hdc = BeginPaint(hwnd);

				EndPaint(hwnd,hdc);
			}
			break;
    
		case MSG_KEYDOWN:
		/************************************** Confirm Process *****************************************/
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							Pin_Edit_Count = 0;
							
							memset (&Now_Pin, 0, sizeof (char) * 5);
							memset (&New_Pin, 0, sizeof (char) * 5);
							memset (&Confirm_Pin, 0, sizeof (char) * 5);

							hdc = BeginPaint(hwnd);
							if ( Current_Item == CSAPP_PIN_NOW )
							{
								MV_Draw_PinMenuBar(hdc, MV_FOCUS, CSAPP_PIN_NOW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NEW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_COMFIRM);
							} else if ( Current_Item == CSAPP_PIN_NEW )
							{
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NOW);
								MV_Draw_PinMenuBar(hdc, MV_FOCUS, CSAPP_PIN_NEW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_COMFIRM);
							} else if ( Current_Item == CSAPP_PIN_COMFIRM )
							{
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NOW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NEW);
								MV_Draw_PinMenuBar(hdc, MV_FOCUS, CSAPP_PIN_COMFIRM);
							} else 
							{
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NOW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NEW);
								MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_COMFIRM);
							}
							EndPaint(hwnd,hdc);
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
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
					switch(wparam)
					{
						case CSAPP_KEY_0:
							key_in = '0';
							break;
						case CSAPP_KEY_1:
							key_in = '1';
							break;
						case CSAPP_KEY_2:
							key_in = '2';
							break;
						case CSAPP_KEY_3:
							key_in = '3';
							break;
						case CSAPP_KEY_4:
							key_in = '4';
							break;
						case CSAPP_KEY_5:
							key_in = '5';
							break;
						case CSAPP_KEY_6:
							key_in = '6';
							break;
						case CSAPP_KEY_7:
							key_in = '7';
							break;
						case CSAPP_KEY_8:
							key_in = '8';
							break;
						case CSAPP_KEY_9:
							key_in = '9';
							break;
						default:
							break;
					}
					
					if ( Current_Item > CASPP_PIN_CHANGE && Current_Item < CSAPP_PIN_SETTING_ITEM_MAX )
					{
						if (Change_Enable == CS_APP_DISABLE_PIN)
						{
							// printf("\n ========= %d ===============\n", Change_Enable);
							Current_Item = CASPP_PIN_CHANGE;
							hdc=BeginPaint(hwnd);
							MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
						} else {
							if ( Pin_Edit_Count < 4 )
							{
								if ( Current_Item == CSAPP_PIN_NOW )
									Now_Pin[Pin_Edit_Count] = key_in;
								else if ( Current_Item == CSAPP_PIN_NEW )
									New_Pin[Pin_Edit_Count] = key_in;
								else
									Confirm_Pin[Pin_Edit_Count] = key_in;

								Pin_Edit_Count++;
							}
							
							//printf("\n ========= %s ===============\n", Now_Pin);
							hdc=BeginPaint(hwnd);
							MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
					}
					break;		

				case CSAPP_KEY_LEFT:
				case CSAPP_KEY_RIGHT:
					switch(Current_Item)
					{
						case CSAPP_PIN_ENABLE:
							if ( Current_PinEnable )
								Current_PinEnable = 0;
							else
								Current_PinEnable = 1;

							CS_DBU_SetParentalLockStatus(Current_PinEnable);
							break;

						case CSAPP_PIN_BOOT:
							if ( Boot_PinEnable )
								Boot_PinEnable = 0;
							else
								Boot_PinEnable = 1;

							CS_DBU_SetBootLockStatus(Boot_PinEnable);
							break;
							
						case CSAPP_PIN_MENU:
							if ( Install_PinEnable )
								Install_PinEnable = 0;
							else
								Install_PinEnable = 1;

							CS_DBU_SetInstallLockStatus(Install_PinEnable);
							break;
							
						case CSAPP_PIN_EDIT:
							if ( Edit_PinEnable )
								Edit_PinEnable = 0;
							else
								Edit_PinEnable = 1;
							
							CS_DBU_SetEditLockStatus(Edit_PinEnable);
							break;
							
						case CSAPP_PIN_CHANNEL:
							if ( Channel_PinEnable )
								Channel_PinEnable = 0;
							else
								Channel_PinEnable = 1;

							CS_MW_SetServicesLockStatus(Channel_PinEnable);
							break;
							
						case CASPP_PIN_CHANGE:
							if ( Change_Enable )
								Change_Enable = 0;
							else
								Change_Enable = 1;
							break;
							
						default:
							break;
					}

					if ( Current_Item == CASPP_PIN_CHANGE )
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
						MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NOW);
						MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_NEW);
						MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, CSAPP_PIN_COMFIRM);
						EndPaint(hwnd,hdc);
					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
					}
					break;		

				case CSAPP_KEY_UP:

					Pin_Edit_Count = 0;

					hdc=BeginPaint(hwnd);
					MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, Current_Item);
						
					if(Current_Item == 0)
					{
						if ( Change_Enable )
							Current_Item = CSAPP_PIN_SETTING_ITEM_MAX - 1;
						else
							Current_Item = CASPP_PIN_CHANGE;
					}
					else
						Current_Item--;

					MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					
					break;
					
				case CSAPP_KEY_DOWN:

					Pin_Edit_Count = 0;

					hdc=BeginPaint(hwnd);
					MV_Draw_PinMenuBar(hdc, MV_UNFOCUS, Current_Item);

					if ( Current_Item >= CASPP_PIN_CHANGE )
					{
						if ( Change_Enable )
						{
							if(Current_Item == CSAPP_PIN_SETTING_ITEM_MAX - 1)
								Current_Item = 0;
							else
								Current_Item++;
						} else {
							Current_Item = 0;
						}
					}
					else
						Current_Item++;

					MV_Draw_PinMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_MENU:
					if ( Change_Enable )
					{
						if ( strcmp(Now_Pin, CS_DBU_GetPinCode()) != 0 )
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_NO_PIN);
						else if ( strcmp(New_Pin, Confirm_Pin) != 0 )
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_NO_CONFIRM);
						else
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_SAVE);
							EndPaint(hwnd,hdc);
							
							CS_DBU_SetPinCode(New_Pin);
							CS_DBU_SaveUserSettingDataInHW();

							usleep( 500 * 1000 );
										
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							
							CSApp_PinSetting_Applets = b8Last_App_Status;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
					} else {
						CS_DBU_SaveUserSettingDataInHW();
						CSApp_PinSetting_Applets = b8Last_App_Status;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
					break;
					
				case CSAPP_KEY_ESC:
					if ( Change_Enable )
					{
						if ( strcmp(Now_Pin, CS_DBU_GetPinCode()) != 0 )
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_NO_PIN);
						else if ( strcmp(New_Pin, Confirm_Pin) != 0 )
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_NO_CONFIRM);
						else
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_SAVE);
							EndPaint(hwnd,hdc);
							
							CS_DBU_SetPinCode(New_Pin);
							CS_DBU_SaveUserSettingDataInHW();

							usleep( 500 * 1000 );
										
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							
							CSApp_PinSetting_Applets = b8Last_App_Status;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
					} else {
						CS_DBU_SaveUserSettingDataInHW();
						CSApp_PinSetting_Applets = CSApp_Applet_Desktop;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_PinSetting_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_ENTER:
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








