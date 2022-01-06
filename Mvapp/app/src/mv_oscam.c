#include "linuxos.h"
#include "mvosapi.h"

#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "database.h"
#include "mv_oscam.h"

/* By KB Kim 2011.05.14 */
#define SERVER_COUNT_ON

static CSAPP_Applet_t		CSApp_OSCAM_Applets;

static U32					OSCAMItemIdx[CSAPP_OSCAM_ITEM_MAX]={
								CSAPP_STR_SERVER_NO,
								CSAPP_STR_PROTOCOL,
								CSAPP_STR_URL,
								CSAPP_STR_PORT_FROM,
								CSAPP_STR_PORT_TO,
								CSAPP_STR_ID,
								CSAPP_STR_PASSWORD,
								CSAPP_STR_DES_KEY,
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
								CSAPP_STR_ENAVLE,
								CSAPP_STR_STATUS
#else
								CSAPP_STR_CONNECT
#endif
							};

static U32					OSCAM_MODE_Idx[2] = {
								CSAPP_STR_CCCAMD,
								CSAPP_STR_NEWCAMD
							};

#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
static U32					OSCAM_EnableIdx[2] = {
								CSAPP_STR_DISCONNECT,
								CSAPP_STR_CONNECT
							};
#else
static U32					OSCAM_EnableIdx[2] = {
								CSAPP_STR_OFF,
								CSAPP_STR_ON
							};
#endif

// Sertac disabling below to avoid compiler warnings 6.1.2022
// static U32					OSCAM_StatusIdx[2] = {
// 								CSAPP_STR_CONNECTED,
// 								CSAPP_STR_NOT_CONNECTED
// 							};

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

U8	OSCAM_Arrow_Kind[CSAPP_OSCAM_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	MV_STATIC,
	MV_STATIC
#else
	MV_SELECT
#endif
};

U8	OSCAM_Enter_Kind[CSAPP_OSCAM_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	MV_STATIC,
	MV_STATIC
#else
	MV_SELECT
#endif
};

static U8				Current_Item = 0;
static U8				Client_No = 0;
static U8				EnableIdx = 0;
/* By KBKim 2011.04.23 */
// static U8				StatusIdx = 0;

/* For OSCAM Menu by KB Kim 2011.04.22 */
#define SAVE_STATUS_NONE        0
#define SAVE_STATUS_SAVE        1
#define SAVE_STATUS_CLOSE       2
#define SAVE_STATUS_DELETE      3

OscamServerInfo_t 	ServerData[OSCAM_NUM];
int 				NumberOfServerData;
U8  				SaveStatus;
static U8			ServerDataChanged = 0; /* By KB Kim 2011.05.12 */

#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
static U8			CurrentDataNumber = 0;
#endif

static int OSCAM_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_OSCAM_SelectBar(HDC hdc, int y_gap, eMV_Nerwork_Items esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;
	
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	if ( esItem == CSAPP_OSCAM_ITEM_ENABLE )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX - 100),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel((MV_INSTALL_MENU_X + MV_MENU_TITLE_DX - 100) + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(200 - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth )),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel((MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + 100) - MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	}
	else
#endif
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	}
	
	switch(OSCAM_Enter_Kind[esItem])
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
	
	if ( OSCAM_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_OSCAMMenuBar(HDC hdc, U8 u8Focuskind, eMV_CAS_Items esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	TempStr[100];

	memset(TempStr, 0x00, 100);

#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	if ( esItem >= CSAPP_OSCAM_ITEM_ENABLE )
		y_gap += ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP );
#endif

	if( u8Focuskind == MV_UNFOCUS )
	{
		if ( stOSCAM_Data[Client_No].u8Protocal == 0 && ( esItem == CSAPP_OSCAM_ITEM_PORT2 || esItem == CSAPP_OSCAM_ITEM_KEY1 ) )
		{
			SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		}
		
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	} else {
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_Draw_OSCAM_SelectBar(hdc, y_gap, esItem);
	}

#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	if ( esItem != CSAPP_OSCAM_ITEM_ENABLE )
#endif
	{
		sprintf(TempStr, "%s", CS_MW_LoadStringByIdx(OSCAMItemIdx[esItem]));
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), TempStr);
	}

	//printf("\n### %d : %s ##\n",esItem, TempStr);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		case CSAPP_OSCAM_ITEM_NUMBER:
			sprintf(TempStr, "%d", Client_No + 1 );
			CS_MW_DrawText(hdc, TempStr, -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_MODE:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_MODE_Idx[stOSCAM_Data[Client_No].u8Protocal]), -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_URL:
			CS_MW_DrawText(hdc, stOSCAM_Data[Client_No].acURL, -1, &TmpRect, DT_CENTER);
			break;

		case CSAPP_OSCAM_ITEM_PORT1:
			sprintf(TempStr, "%d", stOSCAM_Data[Client_No].u16Port1);
			CS_MW_DrawText(hdc, TempStr, -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_PORT2:
			sprintf(TempStr, "%d", stOSCAM_Data[Client_No].u16Port2 );
			CS_MW_DrawText(hdc, TempStr, -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_ID:
			CS_MW_DrawText(hdc, stOSCAM_Data[Client_No].acID, -1, &TmpRect, DT_CENTER);
			break;

		case CSAPP_OSCAM_ITEM_PW:
			CS_MW_DrawText(hdc, stOSCAM_Data[Client_No].acPW, -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_KEY1:
			CS_MW_DrawText(hdc, stOSCAM_Data[Client_No].btKeyData, -1, &TmpRect, DT_CENTER);
			break;
			
		case CSAPP_OSCAM_ITEM_ENABLE:
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
			TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX - 100);
			TmpRect.right	=TmpRect.left + 200;

			/* By KBKim 2011.04.23 */
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_EnableIdx[stOSCAM_Data[Client_No].statusIdx]), -1, &TmpRect, DT_CENTER);
			/*
			if ( stOSCAM_Data[Client_No].statusIdx == 0 )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_EnableIdx[1]), -1, &TmpRect, DT_CENTER);
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_EnableIdx[0]), -1, &TmpRect, DT_CENTER);
			*/
#else
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_EnableIdx[stOSCAM_Data[Client_No].enable]), -1, &TmpRect, DT_CENTER);
#endif
			break;

#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
		case CSAPP_OSCAM_ITEM_STATUS:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(OSCAM_StatusIdx[stOSCAM_Data[Client_No].statusIdx]), -1, &TmpRect, DT_CENTER);
			break;
#endif
			
		default:
			break;
	}
}

void MV_Draw_OSCAM_MenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_OSCAM_ITEM_MAX ; i++ )
	{
		if( Current_Item == i )
			MV_Draw_OSCAMMenuBar(hdc, MV_FOCUS, i);
		else
			MV_Draw_OSCAMMenuBar(hdc, MV_UNFOCUS, i);
	}
}

void MV_Draw_OSCAMMenuBars(HDC hdc)
{
	MV_Draw_OSCAMMenuBar(hdc, MV_UNFOCUS, CSAPP_OSCAM_ITEM_PORT2);
	MV_Draw_OSCAMMenuBar(hdc, MV_UNFOCUS, CSAPP_OSCAM_ITEM_KEY1);
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
void MV_CAM_draw_help_benner(HDC hdc)
{
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_ADD_SERVER));
#else
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
#endif

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2), ScalerHeigthPixel(MV_HELP_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_SERVER));
}

CSAPP_Applet_t CSApp_OSCAM(void)
{
	int					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_OSCAM_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption	= "oscam info";
	CreateInfo.hMenu		= 0;
	CreateInfo.hCursor		= 0;
	CreateInfo.hIcon		= 0;
	CreateInfo.MainWindowProc = OSCAM_Msg_cb;
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

	return CSApp_OSCAM_Applets;   
}

void OSCAM_Init_Data(void)
{
	int 	i;

	/* For OSCAM Menu by KB Kim 2011.04.22 */
	memset(ServerData, 0x00, sizeof(OscamServerInfo_t) * OSCAM_NUM);
	memset(stOSCAM_Data, 0x00, sizeof(stOSCAM_St) * OSCAM_NUM);
	NumberOfServerData = CasDrvReadOscamServerData(ServerData);
	if (NumberOfServerData >= OSCAM_NUM)
	{
		NumberOfServerData = OSCAM_NUM;
	}

#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
	if (NumberOfServerData > 0)
	{
		CurrentDataNumber = NumberOfServerData;
	}
	else
	{
		CurrentDataNumber = 1;
	}
#endif

	// printf("OSCAM_Init_Data : NumberOfServerData = %d\n", NumberOfServerData);
	if (NumberOfServerData > 0)
	{
		for ( i = 0 ; i < NumberOfServerData ; i++ )
		{
			// stOSCAM_Data[i].u8Num = i;
			stOSCAM_Data[i].u8Protocal = ServerData[i].ProtocolValue;
			StrNcpy(stOSCAM_Data[i].acURL, ServerData[i].Url, sizeof(stOSCAM_Data[i].acURL));
			stOSCAM_Data[i].u16Port1 = ServerData[i].R_PortVal;
			stOSCAM_Data[i].u16Port2 = ServerData[i].L_PortVal;
			StrNcpy(stOSCAM_Data[i].acID, ServerData[i].UserId, sizeof(stOSCAM_Data[i].acID));
			StrNcpy(stOSCAM_Data[i].acPW, ServerData[i].PassWord, sizeof(stOSCAM_Data[i].acPW));
			StrNcpy(stOSCAM_Data[i].btKeyData, ServerData[i].DesKey, sizeof(stOSCAM_Data[i].btKeyData));
			stOSCAM_Data[i].enable    = ServerData[i].Enable;
			stOSCAM_Data[i].statusIdx = 1 - ServerData[i].Enable;
			stOSCAM_Data[i].u8delete  = 0;
		}
	}

	if (NumberOfServerData < OSCAM_NUM)
	{
		for ( i = NumberOfServerData ; i < OSCAM_NUM ; i++ )
		{
			// stOSCAM_Data[i].u8Num = i;
			stOSCAM_Data[i].u8Protocal = 0;
			memset(stOSCAM_Data[i].acURL, 0x00, sizeof(stOSCAM_Data[i].acURL));
			stOSCAM_Data[i].u16Port1 = 0;
			stOSCAM_Data[i].u16Port2 = 0;
			memset(stOSCAM_Data[i].acID, 0x00, sizeof(stOSCAM_Data[i].acID));
			memset(stOSCAM_Data[i].acPW, 0x00, sizeof(stOSCAM_Data[i].acPW));
			strcpy(stOSCAM_Data[i].btKeyData, OSCAM_DEFAULT_DES_KEY);
			stOSCAM_Data[i].enable    = 0;
			stOSCAM_Data[i].statusIdx = 1;
			stOSCAM_Data[i].u8delete  = 0;
		}
	}
}

/* For OSCAM Menu by KB Kim 2011.04.22 */
void OSCAM_WriteServerData(void)
{
	int 	i;

	// memset(ServerData, 0x00, sizeof(OscamServerInfo_t) * OSCAM_NUM);
	for ( i = 0 ; i < OSCAM_NUM ; i++ )
	{
		ServerData[i].ProtocolValue = stOSCAM_Data[i].u8Protocal;
		memset(ServerData[i].Url, 0x00, sizeof(ServerData[i].Url));
		if (ServerData[i].ProtocolValue)
		{
			if (ServerData[i].Protocol[0] == 0)
			{
				snprintf(ServerData[i].Protocol, SERVER_PROTOCOL_LENGTH, "newcamd");
			}

			if (stOSCAM_Data[i].acURL[0] != 0)
			{
				sprintf(ServerData[i].Url, "%s,%d,%d", stOSCAM_Data[i].acURL, stOSCAM_Data[i].u16Port1, stOSCAM_Data[i].u16Port2);
			}
			// StrNcpy(ServerData[i].Url, stOSCAM_Data[i].acURL, sizeof(ServerData[i].Url));
		}
		else
		{
			if (ServerData[i].Protocol[0] == 0)
			{
				snprintf(ServerData[i].Protocol, SERVER_PROTOCOL_LENGTH, "cccam");
			}
			if (stOSCAM_Data[i].acURL[0] != 0)
			{
				sprintf(ServerData[i].Url, "%s,%d", stOSCAM_Data[i].acURL, stOSCAM_Data[i].u16Port1);
			}
		}
		ServerData[i].R_PortVal = stOSCAM_Data[i].u16Port1;
		ServerData[i].L_PortVal = stOSCAM_Data[i].u16Port2;
		StrNcpy(ServerData[i].UserId, stOSCAM_Data[i].acID, sizeof(ServerData[i].UserId));
		StrNcpy(ServerData[i].PassWord, stOSCAM_Data[i].acPW, sizeof(ServerData[i].PassWord));
		StrNcpy(ServerData[i].DesKey, stOSCAM_Data[i].btKeyData, sizeof(ServerData[i].DesKey));
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
		ServerData[i].Enable = 1 - stOSCAM_Data[i].statusIdx;
#else
		ServerData[i].Enable = stOSCAM_Data[i].enable;
#endif
		ServerData[i].Delete = stOSCAM_Data[i].u8delete;
	}

	NumberOfServerData = CasDrvWriteOscamServerData(ServerData, NumberOfServerData, CurrentDataNumber);
}

static int OSCAM_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 				hdc;
	char				acKey_Value[256];
   
	switch(message)
   	{
		case MSG_CREATE:
			OSCAM_Init_Data();
			Current_Item = CSAPP_OSCAM_ITEM_NUMBER;
			Client_No = 0;
			EnableIdx = 0;

			/* By KB Kim 2011.05.12 */
			ServerDataChanged = 0;

			/* For OSCAM Menu by KB Kim 2011.04.22 */
			SaveStatus = SAVE_STATUS_NONE;
			// StatusIdx = 0;
			memset(acKey_Value, 0x00, 256);
			break;
			
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_TOOLS_MAX);
			
			hdc=BeginPaint(hwnd);
			MV_Draw_OSCAM_MenuBar(hdc);
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
			MV_CAM_draw_help_benner(hdc);
#endif
			// MV_Draw_CAS_Info(hwnd, 0);
			EndPaint(hwnd,hdc);
			return 0;
		
		case MSG_KEYDOWN:
			/* For OSCAM Menu by KB Kim 2011.04.22 */
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							if ((SaveStatus == SAVE_STATUS_SAVE) || (SaveStatus == SAVE_STATUS_CLOSE))
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

								OSCAM_WriteServerData();

								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);

								/* By KB Kim 2011.05.12 */
								ServerDataChanged = 0;
							}
							else if (SaveStatus == SAVE_STATUS_DELETE)
							{
								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

								OSCAM_WriteServerData();
								OSCAM_Init_Data();

								hdc = BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);
								
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);

								if (Client_No >= CurrentDataNumber)
								{
									Client_No = CurrentDataNumber - 1;
								}
								ServerDataChanged = 0;
								
								hdc=BeginPaint(hwnd);
								MV_Draw_OSCAM_MenuBar(hdc);
								EndPaint(hwnd,hdc);

							}

						}
						else
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
						}
					}
					else
					{
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
					}

					if (SaveStatus == SAVE_STATUS_CLOSE)
					{
						DestroyMainWindow(hwnd);
						PostQuitMessage(hwnd);
					}

					SaveStatus = SAVE_STATUS_NONE;
				}

				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
/***************************** NumKeypad Process ***********************************************/
			else if ( Get_NumKeypad_Status() == TRUE )
			{
				UI_NumKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_BLUE )
				{
					if ( wparam == CSAPP_KEY_BLUE )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{							
							char sReturn_str[PORT_LENGTH+1];
							
							Get_Save_Str(sReturn_str);

							switch(Current_Item)
							{
								case CSAPP_OSCAM_ITEM_PORT1:
									stOSCAM_Data[Client_No].u16Port1 = atoi(sReturn_str);
									/* By KB Kim 2011.05.12 */
									ServerDataChanged = 1;
									break;

								case CSAPP_OSCAM_ITEM_PORT2:
									stOSCAM_Data[Client_No].u16Port2 = atoi(sReturn_str);
									/* By KB Kim 2011.05.12 */
									ServerDataChanged = 1;
									break;
									
								default:
									break;
							}
							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
					} 
				}
				
				if (wparam == CSAPP_KEY_IDLE)
				{
					hdc = BeginPaint(hwnd);
					Restore_Confirm_Window(hdc);
					EndPaint(hwnd,hdc);
				}
				else
				{
					break;
				}
			}
/***************************** StringKeypad Process ***********************************************/
			else if ( MV_Get_StringKeypad_Status() == TRUE )
			{
				MV_StringKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ENTER )
				{
					memset(acKey_Value , 0x00, 256);
					strcpy(acKey_Value, MV_Get_StringEdited_String());

					switch(Current_Item)
					{
						case CSAPP_OSCAM_ITEM_URL:
							strcpy(stOSCAM_Data[Client_No].acURL, acKey_Value);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;

						case CSAPP_OSCAM_ITEM_ID:
							strcpy(stOSCAM_Data[Client_No].acID, acKey_Value);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;

						case CSAPP_OSCAM_ITEM_PW:
							strcpy(stOSCAM_Data[Client_No].acPW, acKey_Value);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;

						case CSAPP_OSCAM_ITEM_KEY1:
							strcpy(stOSCAM_Data[Client_No].btKeyData, acKey_Value);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;

						default:
							break;
					}

					hdc=BeginPaint(hwnd);
					MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
				}
				break;
			}
/***************************** Popup Process ***********************************************/
			else if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Current_Item)
					{
						case CSAPP_OSCAM_ITEM_NUMBER:
							Client_No = u8Result_Value;
							hdc = BeginPaint(hwnd);
							MV_Draw_OSCAM_MenuBar(hdc);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_OSCAM_ITEM_MODE:
							stOSCAM_Data[Client_No].u8Protocal = u8Result_Value;
							hdc = BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_OSCAMMenuBars(hdc);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#ifndef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
						case CSAPP_OSCAM_ITEM_ENABLE:
							stOSCAM_Data[Client_No].enable = u8Result_Value;
							hdc = BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#endif
#if 0
						case CSAPP_OSCAM_ITEM_ENABLE:
							EnableIdx = u8Result_Value;
							hdc = BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_OSCAM_ITEM_STATUS:
							stOSCAM_Data[Client_No].statusIdx = u8Result_Value;
							hdc = BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
#endif
						default:
							break;
					}
				}
				break;
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_RED:
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
					if (CurrentDataNumber < OSCAM_NUM)
					{
						Client_No = CurrentDataNumber;
						CurrentDataNumber++;
						hdc=BeginPaint(hwnd);
						MV_Draw_OSCAM_MenuBar(hdc);
						EndPaint(hwnd,hdc);
						ServerDataChanged = 1;
					}
					else
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_SERVER_FULL);
						EndPaint(hwnd,hdc);
						usleep( 2000*1000 );
						hdc = BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
#endif
					/* For OSCAM Menu by KB Kim 2011.04.22 */
					// SaveStatus = SAVE_STATUS_SAVE;
					// MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
					// OSCAM_WriteServerData();
					// OSCAM_Init_Data();
					break;
					
				case CSAPP_KEY_BLUE:
					/* For OSCAM Menu by KB Kim 2011.04.22 */
					// if (NumberOfServerData > 0)
					{
						SaveStatus = SAVE_STATUS_DELETE;
						stOSCAM_Data[Client_No].u8delete = 1;
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
					}
					break;

				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_OSCAMMenuBar(hdc, UNFOCUS, Current_Item);

					if ( Current_Item == CSAPP_OSCAM_ITEM_NUMBER )
						Current_Item = CSAPP_OSCAM_ITEM_ENABLE;
					else if ( Current_Item == CSAPP_OSCAM_ITEM_ENABLE && stOSCAM_Data[Client_No].u8Protocal == 0 )
						Current_Item -= 2;
					else if ( Current_Item == CSAPP_OSCAM_ITEM_ID && stOSCAM_Data[Client_No].u8Protocal == 0 )
						Current_Item -= 2;
					else
						Current_Item--;

					MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_DOWN:
					hdc=BeginPaint(hwnd);
					MV_Draw_OSCAMMenuBar(hdc, UNFOCUS, Current_Item);

					if ( Current_Item == CSAPP_OSCAM_ITEM_ENABLE )
						Current_Item = CSAPP_OSCAM_ITEM_NUMBER;
					else if ( Current_Item == CSAPP_OSCAM_ITEM_PORT1 && stOSCAM_Data[Client_No].u8Protocal == 0 )
						Current_Item += 2;					
					else if ( Current_Item == CSAPP_OSCAM_ITEM_PW && stOSCAM_Data[Client_No].u8Protocal == 0 )
						Current_Item += 2;
					else
						Current_Item++;

					MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;

				case CSAPP_KEY_LEFT:
					switch(Current_Item)
					{
						case CSAPP_OSCAM_ITEM_NUMBER:
							if ( Client_No == 0 )
							{
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
								Client_No = CurrentDataNumber - 1;
#else
								Client_No = OSCAM_NUM - 1;
#endif
							}
							else
								Client_No--;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAM_MenuBar(hdc);
							EndPaint(hwnd,hdc);
							break;
							
						case CSAPP_OSCAM_ITEM_MODE:
							if ( stOSCAM_Data[Client_No].u8Protocal == 1 )
								stOSCAM_Data[Client_No].u8Protocal = 0;
							else
								stOSCAM_Data[Client_No].u8Protocal = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_OSCAMMenuBars(hdc);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#ifndef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
						case CSAPP_OSCAM_ITEM_ENABLE:
							stOSCAM_Data[Client_No].enable = 1 - stOSCAM_Data[Client_No].enable;
							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#endif
/*
						case CSAPP_OSCAM_ITEM_ENABLE:
							if ( EnableIdx == 1 )
								EnableIdx = 0;
							else
								EnableIdx = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_OSCAMMenuBar(hdc, UNFOCUS, CSAPP_OSCAM_ITEM_STATUS);
							EndPaint(hwnd,hdc);
							break;

						case CSAPP_OSCAM_ITEM_STATUS:
							if ( stOSCAM_Data[Client_No].statusIdx == 1 )
								stOSCAM_Data[Client_No].statusIdx = 0;
							else
								stOSCAM_Data[Client_No].statusIdx = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
*/
						default:
							break;
					}
					break;

				case CSAPP_KEY_RIGHT:
					switch(Current_Item)
					{
						case CSAPP_OSCAM_ITEM_NUMBER:
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
							if ( Client_No == CurrentDataNumber - 1 )
#else
							if ( Client_No == OSCAM_NUM - 1 )
#endif
								Client_No = 0;
							else
								Client_No++;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAM_MenuBar(hdc);
							EndPaint(hwnd,hdc);
							break;
							
						case CSAPP_OSCAM_ITEM_MODE:
							if ( stOSCAM_Data[Client_No].u8Protocal == 1 )
								stOSCAM_Data[Client_No].u8Protocal = 0;
							else
								stOSCAM_Data[Client_No].u8Protocal = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_OSCAMMenuBars(hdc);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#ifndef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
						case CSAPP_OSCAM_ITEM_ENABLE:
							stOSCAM_Data[Client_No].enable = 1 - stOSCAM_Data[Client_No].enable;
							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							/* By KB Kim 2011.05.12 */
							ServerDataChanged = 1;
							break;
#endif
/*							
						case CSAPP_OSCAM_ITEM_ENABLE:
							if ( EnableIdx == 1 )
								EnableIdx = 0;
							else
								EnableIdx = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							MV_Draw_OSCAMMenuBar(hdc, UNFOCUS, CSAPP_OSCAM_ITEM_STATUS);
							EndPaint(hwnd,hdc);
							break;

						case CSAPP_OSCAM_ITEM_STATUS:
							if ( stOSCAM_Data[Client_No].statusIdx == 1 )
								stOSCAM_Data[Client_No].statusIdx = 0;
							else
								stOSCAM_Data[Client_No].statusIdx = 1;

							hdc=BeginPaint(hwnd);
							MV_Draw_OSCAMMenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
*/
						default:
							break;
					}
					break;
					
				case CSAPP_KEY_ESC:
					CSApp_OSCAM_Applets=CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_ENTER:
					{
						RECT					smwRect;
						stPopUp_Window_Contents stContents;
						int						i;

						memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
						
						switch(Current_Item)
						{
							case CSAPP_OSCAM_ITEM_NUMBER:
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
								for ( i = 0 ; i < CurrentDataNumber ; i++ )
									sprintf(stContents.Contents[i], "%d", (i + 1));	
#else
								for ( i = 0 ; i < OSCAM_NUM ; i++ )
									sprintf(stContents.Contents[i], "%d", (i + 1));	
#endif
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
#ifdef SERVER_COUNT_ON /* By KB Kim 2011.05.14 */
								stContents.u8TotalCount = CurrentDataNumber;
#else
								stContents.u8TotalCount = OSCAM_NUM;
#endif
								stContents.u8Focus_Position = stOSCAM_Data[Client_No].u8Num; /* By KB Kim 2011.05.12 */
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
								
							case CSAPP_OSCAM_ITEM_MODE:
								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(OSCAM_MODE_Idx[0]));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(OSCAM_MODE_Idx[1]));
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;
								stContents.u8Focus_Position = stOSCAM_Data[Client_No].u8Protocal; /* By KB Kim 2011.05.12 */
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;

							case CSAPP_OSCAM_ITEM_URL:
								MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acURL, MAX_URL_LANGTH);
								break;

							case CSAPP_OSCAM_ITEM_PORT1:
								MV_Draw_NumKeypad(hwnd, stOSCAM_Data[Client_No].u16Port1, 0, MAX_NUMERIC_LENGTH);
								break;
								
							case CSAPP_OSCAM_ITEM_PORT2:
								MV_Draw_NumKeypad(hwnd, stOSCAM_Data[Client_No].u16Port2, 0, MAX_NUMERIC_LENGTH);
								break;
								
							case CSAPP_OSCAM_ITEM_ID:
								// printf("\n==== %s ====\n", stOSCAM_Data[Client_No].acID);
								MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acID, MAX_URL_LANGTH);
								break;

							case CSAPP_OSCAM_ITEM_PW:
								MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acPW, MAX_URL_LANGTH);
								break;
								
							case CSAPP_OSCAM_ITEM_KEY1:
								MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].btKeyData, DES_KEY_LENGTH);
								break;
								
							case CSAPP_OSCAM_ITEM_ENABLE:
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
								stOSCAM_Data[Client_No].statusIdx = 1 - stOSCAM_Data[Client_No].statusIdx;

								hdc=BeginPaint(hwnd);
								MV_Draw_OSCAMMenuBar(hdc, FOCUS, CSAPP_OSCAM_ITEM_ENABLE);
								MV_Draw_OSCAMMenuBar(hdc, UNFOCUS, CSAPP_OSCAM_ITEM_STATUS);
								EndPaint(hwnd,hdc);
								break;
#else
								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(OSCAM_EnableIdx[0]));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(OSCAM_EnableIdx[1]));
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;
								stContents.u8Focus_Position = stOSCAM_Data[Client_No].enable;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
#endif
/*
							case CSAPP_OSCAM_ITEM_STATUS:
								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(OSCAM_StatusIdx[0]));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(OSCAM_StatusIdx[1]));
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Current_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
*/
							default:
								break;
						}
					}
					break;
					
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
					switch(Current_Item)
					{
							case CSAPP_OSCAM_ITEM_URL:
								MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acURL, MAX_URL_LANGTH);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;

							case CSAPP_OSCAM_ITEM_PORT1:
								MV_Draw_NumKeypad(hwnd, 0, 0, MAX_NUMERIC_LENGTH);
								// MV_Draw_NumKeypad(hwnd, stOSCAM_Data[Client_No].u16Port1, 0, MAX_NUMERIC_LENGTH);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;
								
							case CSAPP_OSCAM_ITEM_PORT2:
								MV_Draw_NumKeypad(hwnd, 0, 0, MAX_NUMERIC_LENGTH);
								// MV_Draw_NumKeypad(hwnd, stOSCAM_Data[Client_No].u16Port2, 0, MAX_NUMERIC_LENGTH);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;
								
							case CSAPP_OSCAM_ITEM_ID:
								MV_Draw_StringKeypad(hwnd, "\0", MAX_URL_LANGTH);
								// MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acID, MAX_URL_LANGTH);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;

							case CSAPP_OSCAM_ITEM_PW:
								MV_Draw_StringKeypad(hwnd, "\0", MAX_URL_LANGTH);
								// MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].acPW, MAX_URL_LANGTH);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;
								
							case CSAPP_OSCAM_ITEM_KEY1:
								MV_Draw_StringKeypad(hwnd, "\0", DES_KEY_LENGTH);
								// MV_Draw_StringKeypad(hwnd, stOSCAM_Data[Client_No].btKeyData, 30);
								PostMessage(hwnd, MSG_KEYDOWN, wparam, 0);
								break;
								
							default:
								break;
					}
					break;
				case CSAPP_KEY_MENU:
					// CSApp_OSCAM_Applets=CSApp_Applet_CAS;
					CSApp_OSCAM_Applets=b8Last_App_Status; /* By KB Kim 2011.05.12 */
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_IDLE:
					CSApp_OSCAM_Applets = CSApp_Applet_Sleep;
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
			/* By KB Kim 2011.05.12 */
			if (ServerDataChanged)
			{
				/* For OSCAM Menu by KB Kim 2011.04.22 */
				SaveStatus = SAVE_STATUS_CLOSE;
				MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
			}
			else
			{
				DestroyMainWindow(hwnd);
				PostQuitMessage(hwnd);
			}
			break;

	   	default:
			break;
   	}
	
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}



