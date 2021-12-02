#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define	MV_FAV_MENU_X		500
#define MV_FAV_MENU_Y		MV_INSTALL_MENU_Y
#define MV_FAV_MENU_DX		280

static CSAPP_Applet_t	CSApp_Fav_Name_Applets;
static U16				Current_Item = 0;

U8	Fav_Enter_Kind[MV_MAX_FAV_KIND] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

static int Fav_Edit_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);
static void MV_Draw_FavMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem);
static void MV_Draw_FavSelectBar(HDC hdc, int y_gap, eMV_SYSSetting_Items esItem);
static void MV_Draw_Fav_MenuBar(HDC hdc);

void MV_Draw_FavSelectBar(HDC hdc, int y_gap, eMV_SYSSetting_Items esItem)
{
	int mid_width = MV_FAV_MENU_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_FAV_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_FAV_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_FAV_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Fav_Enter_Kind[esItem])
	{
		case MV_NUMERIC:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_FAV_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
			break;
		case MV_SELECT:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_FAV_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
			break;
		default:
			break;
	}
}

void MV_Draw_FavMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 	y_gap = MV_FAV_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	CRect;
	char	acTemp[20];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_FavSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_FAV_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_FAV_MENU_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}

	MV_DB_Get_Favorite_Name(acTemp, esItem);

	CRect.top = y_gap + 4;
	CRect.bottom = CRect.top + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) - 4;
	CRect.left = MV_FAV_MENU_X;
	CRect.right = CRect.left + MV_FAV_MENU_DX;
	CS_MW_DrawText(hdc, acTemp, -1, &CRect, DT_CENTER );
}

void MV_Draw_Fav_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_FavMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_FavMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_Fav_Edit(void)
{
   	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Fav_Name_Applets = CSApp_Applet_Error;
	
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
	CreateInfo.spCaption = "Change_Fav_name";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Fav_Edit_Msg_cb;
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
	
	return CSApp_Fav_Name_Applets;
	
}



static int Fav_Edit_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc;
	char			sReturn_str[MAX_SERVICE_NAME_LENGTH+1];

	switch(message)
    {
        case MSG_CREATE:			
			Current_Item = 0;
			break;
		case MSG_PAINT:
			{	
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_SYSTEM, EN_ITEM_FOCUS_SYS);

				hdc=BeginPaint(hwnd);
				MV_Draw_Fav_MenuBar(hdc);
				EndPaint(hwnd,hdc);
			}
			// printf("MSG_PAINT end : Font_Kind : %d\n", CS_DBU_GetFont_Kind());

			return 0;

		case MSG_KEYDOWN:
			if ( Get_Keypad_Status() == TRUE )
			{
				UI_Keypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW)
				{
					if ( wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
							EndPaint(hwnd,hdc);
							
							Get_Save_Str(sReturn_str);
							MV_DB_Set_Favorite_Name(sReturn_str, Current_Item);
							CS_DB_Save_CH_Database();
							
							hdc = BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							MV_Draw_FavMenuBar(hdc, MV_FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
					}
				}
				
				break;
			}	
			
			switch(wparam)
			{
				case CSAPP_KEY_ESC:
					CSApp_Fav_Name_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_MENU:
					CSApp_Fav_Name_Applets = b8Last_App_Status;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_ENTER:
					{
						char 		TempStr[20];
						
						MV_DB_Get_Favorite_Name(TempStr, Current_Item);						
						MV_Draw_Keypad(hwnd, TempStr, MAX_SAT_NAME_LANGTH);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Fav_Name_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_FavMenuBar(hdc, MV_UNFOCUS, Current_Item);
					
					if(Current_Item == 0)
						Current_Item = MV_MAX_FAV_KIND - 1;
					else
						Current_Item--;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_FavMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
					MV_Draw_FavMenuBar(hdc, MV_UNFOCUS, Current_Item);

					if(Current_Item == MV_MAX_FAV_KIND - 1)
						Current_Item = 0;
					else
						Current_Item++;

					//ComboList_UpdateAll(hwnd);
					MV_Draw_FavMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);

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


