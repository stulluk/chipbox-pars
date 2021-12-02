
#include "linuxos.h"

#include "database.h"
#include "ch_install.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csinstall.h"
#include "ui_common.h"
#include "fe_mngr.h"
#include "mv_motor.h"

U16 SAT_Str[SAT_ITEM_MAX+1] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TPSELECT,
	CSAPP_STR_LNBTYPE,
	CSAPP_STR_H_FREQUENCY,
	CSAPP_STR_L_FREQUENCY,
	CSAPP_STR_22K,
	CSAPP_STR_DISECQ_SETTING,
	CSAPP_STR_TONEBURST,
	CSAPP_STR_SCANTYPE,
	CSAPP_STR_FREQUENCY
};

U8	Sat_Arrow_Kind[SAT_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	Sat_Enter_Kind[SAT_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

/* For Tone Burst Control : By KB Kim 2011.02.28 */
char Tone_Port[3][10] =
{
	"Off",
	"Sat A",
	"Sat B"
};

/* By KB Km 2010.09.04 */
extern eScanConditionID		Search_Condition_Focus_Item;

extern U16 					Connect_Type[5];
extern U16 					Search_Condition[7];
extern char 				LNB_Type[22][30];
extern char 				Diseqc_Port[21][10];

static char					sReturn_str[MAX_SAT_NAME_LANGTH+1];

static CSAPP_Applet_t		CSApp_SAT_Applets;
static eSatItemID			SAT_Focus_Item=SAT_ITEM_SATELLITE;
static U8					Mv_SAT_LNBType=0;
static U8					Mv_SAT_22K=0;
static U8					Mv_SAT_Disecq=0;
static U8					Mv_SAT_Toneburst=0;
static U8					Mv_SAT_ScanType=0;
static U8					u8TpCount;
static BOOL					NumberKeyFlag=FALSE;

static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];  //stSatInfo_Glob
static MV_stTPInfo 			MV_TPInfo;
	
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static U8					u8SatList_Start_Point;
static U8					u8SatList_End_Point;
static BOOL					Sat_List_Status = FALSE;

static U8					u8SatList_Focus_Item;
static U8					Mv_SAT_SatFocus=0;

void MV_Draw_Sat_Selected_Satlist(HDC hdc)
{
	U8				u8SelCount;
	U8				i;
	MV_stSatInfo	Temp_SatData;
	
	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_SELSAT_X), ScalerHeigthPixel(MV_INSTALL_SELSAT_Y + 6), ScalerWidthPixel(MV_INSTALL_SELSAT_DX), ScalerHeigthPixel(MV_INSTALL_SELSAT_DY) );

	u8SelCount = MV_GetSelected_SatData_Count();
	if ( u8SelCount == 0 )
		return;

	for ( i = 0 ; i < u8SelCount ; i++ )
	{
		if ( i == 9 )
			break;
		
		MV_GetSelected_SatData_By_Count(&Temp_SatData, i );

		if ( ( u8Glob_Sat_Focus == Temp_SatData.u8SatelliteIndex ) || Mv_SAT_ScanType == MULTI_SAT )
		{
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} else {
			SetTextColor(hdc,MVAPP_SCROLL_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_SELSAT_X + ( ( MV_INSTALL_SELSAT_DX / 3 ) * (i%3)) + 10),ScalerHeigthPixel(MV_INSTALL_SELSAT_Y + (MV_INSTALL_MENU_BAR_H * (i/3)) + 10 ), Temp_SatData.acSatelliteName);
	}
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
}

CSAPP_Applet_t CSApp_Sat_Setting(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_SAT_Applets = CSApp_Applet_Error;

	SAT_Focus_Item = SAT_ITEM_HIGH_FREQ; 
	
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
	CreateInfo.spCaption 	= "Satellite Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Sat_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	
		return CSApp_Applet_Error;
	
	ShowWindow(hwndMain, SW_SHOWNORMAL);
	
	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	
	//printf("Sat setting Cleanup\n");
	
	return CSApp_SAT_Applets;
	
}

void MV_Sat_Satlist_Item_Draw(HDC hdc, U8 u8Start_Point, U8 u8End_Point)
{
	int 			i;
	MV_stSatInfo	Temp_SatData;
	MV_stTPInfo		Temp_TPData;
	RECT			Scroll_Rect;

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(WINDOW_ITEM_DX), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	for ( i = 0 ; i < LIST_ITEM_NUM ; i++ )
	{
		if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
		{
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Start_Point + i);
			
			if ( u8Start_Point + i > u8End_Point )
				break;
			
			if ( u8Start_Point + i == u8SatList_Focus_Item )
			{
				MV_SatList_Bar_Draw(hdc, i, u8Start_Point, &Temp_SatData, FOCUS);
			} else {
				MV_SatList_Bar_Draw(hdc, i, u8Start_Point, &Temp_SatData, UNFOCUS);
			}
		}
		else
		{
			MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Start_Point + i);
			
			if ( u8Start_Point + i > u8End_Point )
				break;
			
			if ( u8Start_Point + i == u8SatList_Focus_Item )
			{
				MV_TPList_Bar_Draw(hdc, i, u8Start_Point, &Temp_TPData, FOCUS);
			} else {
				MV_TPList_Bar_Draw(hdc, i, u8Start_Point, &Temp_TPData, UNFOCUS);
			}
		}
	}

	Scroll_Rect.top 		= WINDOW_ITEM_Y;
	Scroll_Rect.bottom		= WINDOW_ITEM_Y + WINDOW_ITEM_DY;
	Scroll_Rect.left 		= WINDOW_ITEM_X + WINDOW_ITEM_DX - SCROLL_BAR_DX;
	Scroll_Rect.right 		= WINDOW_ITEM_X + WINDOW_ITEM_DX;

	if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_SAT_LIST, MV_VERTICAL);
	else
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_TP_LIST, MV_VERTICAL);
}

void MV_Sat_List_Window( HWND hwnd )
{
	HDC 	hdc;
	RECT	Temp_Rect;
	/* By KB Kim 2011.01.13 */
	U8      itemCount;

	Sat_List_Status = TRUE;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT + WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP + WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT + WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WINDOW_TOP + WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WINDOW_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WINDOW_TOP),ScalerWidthPixel(WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WINDOW_DX),ScalerHeigthPixel(WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
	
	Temp_Rect.top 	= WINDOW_TOP + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WINDOW_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WINDOW_ITEM_DX;

	/* By KB Kim 2011.01.13 */
	if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
	{
		MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_SEL_SAT);
		itemCount = MV_SAT_MAX;
	}
	else
	{
		MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_SEL_TP);
		itemCount = u8TpCount;
	}
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(WINDOW_ITEM_DX), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	if (itemCount > 0)
	{
		MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
	}
	
	MV_EndPaint(hwnd,hdc);
}

void MV_SAT_List_Window_Close( HWND hwnd )
{
	HDC		hdc;
	
	Sat_List_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

void MV_Draw_SatSetingSelectBar(HDC hdc, int y_gap, eSatItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Sat_Enter_Kind[esItem])
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
	
	if ( Sat_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_SatMenuBar(HDC hdc, U8 u8Focuskind, eSatItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_SatSetingSelectBar(hdc, y_gap, esItem);
	} else {
		if ( esItem == SAT_ITEM_HIGH_FREQ || esItem == SAT_ITEM_LOW_FREQ )
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} 
		else if ( Mv_SAT_LNBType > UNIVERSAL_5750_5150 && esItem == SAT_ITEM_22K ) 
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} 
		else if ( SearchData.ScanMode > SCAN_MODE_DISECQ10 && esItem == SAT_ITEM_DISECQ ) 
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} 
		else 
		{
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	
	if ( esItem == SAT_ITEM_HIGH_FREQ )
	{
		if ( Mv_SAT_LNBType < UNIVERSAL_5150_5750 )
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SAT_Str[SAT_ITEM_MAX]));
		else 
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SAT_Str[esItem]));
	} else if ( esItem == SAT_ITEM_LOW_FREQ ) {
		if ( Mv_SAT_LNBType >= UNIVERSAL_5150_5750 )
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SAT_Str[esItem]));
	} else 
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(SAT_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[30];
		case SAT_ITEM_SATELLITE:
			CS_MW_DrawText(hdc, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, -1, &TmpRect, DT_CENTER);	
			break;
		case SAT_ITEM_TP_SELECT:
			/* By KB Kim 2011.01.13 */
			if ( u8TpCount > 0 )
			{
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&MV_TPInfo, u8Glob_Sat_Focus, u8Glob_TP_Focus);
				
				if ( MV_TPInfo.u8Polar_H == 1 )
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount, MV_TPInfo.u16TPFrequency, "H", MV_TPInfo.u16SymbolRate);
				else
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount, MV_TPInfo.u16TPFrequency, "V", MV_TPInfo.u16SymbolRate);
			} else {
				sprintf(temp_str, "0/0. 0/V/0");
			}
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case SAT_ITEM_LNB_TYPE:
			CS_MW_DrawText(hdc, LNB_Type[Mv_SAT_LNBType], -1, &TmpRect, DT_CENTER);	
			break;
		case SAT_ITEM_HIGH_FREQ:
			sprintf(temp_str, "%d", MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency_High);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SAT_ITEM_LOW_FREQ:
			if ( Mv_SAT_LNBType >= UNIVERSAL_5150_5750 )
			{
				sprintf(temp_str, "%d", MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency);
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			}
			break;
		case SAT_ITEM_22K:
			if ( Mv_SAT_22K == TRUE )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ON));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));

			MV_Sat_Data[u8Glob_Sat_Focus].u16Tone22K = Mv_SAT_22K;
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SAT_ITEM_DISECQ:
			{
				switch(SearchData.ScanMode)
				{
					case SCAN_MODE_SINGLE:
						sprintf(temp_str, "%s", "Direct");
						CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
						break;
						
					case SCAN_MODE_DISECQ10:				
						sprintf(temp_str, "%s", Diseqc_Port[Mv_SAT_Disecq]);
						MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC = Mv_SAT_Disecq;
						CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
						break;

					case SCAN_MODE_UNICABLE:
						CS_MW_DrawText(hdc, "UniCable", -1, &TmpRect, DT_CENTER);
						break;
						
					case SCAN_MODE_USALS:
						CS_MW_DrawText(hdc, "USALS", -1, &TmpRect, DT_CENTER);
						break;
						
					case SCAN_MODE_DISECQ_MOTOR:
						CS_MW_DrawText(hdc, "DiSECq Motor", -1, &TmpRect, DT_CENTER);
						break;
						
					default:
						CS_MW_DrawText(hdc, "Enter", -1, &TmpRect, DT_CENTER);
						break;
				}
			}
			break;
		case SAT_ITEM_TONEBURST:
			/* For Tone Burst Control : By KB Kim 2011.02.28 */
			MV_Sat_Data[u8Glob_Sat_Focus].u16ToneBurst = Mv_SAT_Toneburst;
			switch (Mv_SAT_Toneburst)
			{
				case TONE_OFF :
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
					break;
				case TONE_SAT_A :
					sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_TONE_A));
					break;
				case TONE_SAT_B :
					sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_TONE_B));
					break;
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SAT_ITEM_SCAN_TYPE:
			if ( Mv_SAT_ScanType == MULTI_SAT )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_MULTI_SAT));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ONE_SAT));
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SAT_ITEM_MAX:
		default:
			break;
	}
}

void MV_SAT_Change_LnbType(U8 Mv_LNBType)
{
	U16		u16HighFreq = 0, u16LowFreq = 0;

	switch(Mv_LNBType)
	{
		case C_BAND_5150:
			u16HighFreq = 5150;
			u16LowFreq = 5150;
			break;
		case C_BAND_5750:
			u16HighFreq = 5750;
			u16LowFreq = 5750;
			break;
		case KU_BAND_9750:
			u16HighFreq = 9750;
			u16LowFreq = 9750;
			break;
		case KU_BAND_10000:
			u16HighFreq = 10000;
			u16LowFreq = 10000;
			break;
		case KU_BAND_10600:
			u16HighFreq = 10600;
			u16LowFreq = 10600;
			break;
		case KU_BAND_10678:
			u16HighFreq = 10678;
			u16LowFreq = 10678;
			break;
		case KU_BAND_10700:
			u16HighFreq = 10700;
			u16LowFreq = 10700;
			break;
		case KU_BAND_10750:
			u16HighFreq = 10750;
			u16LowFreq = 10750;
			break;
		case KU_BAND_11000:
			u16HighFreq = 11000;
			u16LowFreq = 11000;
			break;
		case KU_BAND_11200:
			u16HighFreq = 11200;
			u16LowFreq = 11200;
			break;
		case KU_BAND_11250:
			u16HighFreq = 11250;
			u16LowFreq = 11250;
			break;
		case KU_BAND_11300:
			u16HighFreq = 11300;
			u16LowFreq = 11300;
			break;
		case UNIVERSAL_5150_5750:
			u16HighFreq = 5750;
			u16LowFreq = 5150;
			break;
		case UNIVERSAL_5750_5150:
			u16HighFreq = 5150;
			u16LowFreq = 5750;
			break;
		case UNIVERSAL_9750_10600:
			u16HighFreq = 10600;
			u16LowFreq = 9750;
			break;
		case UNIVERSAL_9750_10700:
			u16HighFreq = 10700;
			u16LowFreq = 9750;
			break;
		case UNIVERSAL_9750_10750:
			u16HighFreq = 10750;
			u16LowFreq = 9750;
			break;
		case DIGITURK_1:
			u16HighFreq = 1;
			u16LowFreq = 1;
			break;
		case DIGITURK_2:
			u16HighFreq = 2;
			u16LowFreq = 2;
			break;
		case DIGITURK_3:
			u16HighFreq = 3;
			u16LowFreq = 3;
			break;
		case DIGITURK_4:
			u16HighFreq = 4;
			u16LowFreq = 4;
			break;
		default:
			break;
	}

	MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType = Mv_LNBType;
	MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency_High = u16HighFreq;
	MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency = u16LowFreq;
}

void MV_Draw_SatMenuList(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < SAT_ITEM_MAX ; i++ )
	{
		//printf("%d . =============================\n", i);
		if( SAT_Focus_Item == i )
		{
			MV_Draw_SatMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

int Sat_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	static BOOL   	bk_flag = TRUE;
	
	switch(message)
	{
		case MSG_CREATE:
			u8Multi_Select_Sat = 0;
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);

			SAT_Focus_Item = SAT_ITEM_SATELLITE;
			SearchData.ScanMode = CS_DBU_GetAntenna_Type();
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			Search_Condition_Focus_Item=SCAN_CON_ALL;
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);
			Search_Condition_Status = FALSE;
			Sat_List_Status = FALSE;

			MV_GetSatelliteData(MV_Sat_Data);

			/* By KB Kim 2011.01.13 */
			u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
			Mv_SAT_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
			Mv_SAT_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
			Mv_SAT_22K = MV_Sat_Data[u8Glob_Sat_Focus].u16Tone22K;
			/* For Tone Burst Control : By KB Kim 2011.02.28 */
			Mv_SAT_Toneburst = MV_Sat_Data[u8Glob_Sat_Focus].u16ToneBurst;

#if 0 /* For Motor Control By KB Kim 2011.05.22 */
			if ( SearchData.ScanMode == SCAN_MODE_DISECQ_MOTOR && MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition != 0 )
			{
				DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
			}
			else if ( SearchData.ScanMode == SCAN_MODE_USALS )
			{
				DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
			}
#endif
	
			/* By KB Kim 2011.01.13 */
			if (u8TpCount > 0)
			{
				MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
			}
			else
			{
				u8Glob_TP_Focus = 0;
				CS_FE_StopScan();
			}
			break;
		case MSG_CLOSE:
			bk_flag=FALSE;
			MV_SetSatelliteData(MV_Sat_Data);
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}

			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
		case MSG_TIMER:
			hdc = BeginPaint(hwnd);
			Show_Signal(hdc);
			EndPaint(hwnd,hdc);
			break;
		case MSG_MOTOR_MOVING: /* For Motor Control By KB Kim 2011.05.22 */
			if (wparam)
			{
				Mv_MotorMovingDisplay();
			}
			else
			{
				Motor_Moving_Stop();
			}
			break;
		case MSG_PAINT:

			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, SATELLITE_SETTING);

			hdc = BeginPaint(hwnd);
			
			MV_Draw_SatMenuList(hdc);

			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y ), CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH));
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y + MV_INSTALL_SIGNAL_YGAP ), CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY));

			Show_Signal(hdc);

			MV_Draw_Sat_Selected_Satlist(hdc);
			MV_Install_draw_help_banner(hdc, SATELLITE_SETTING);
			EndPaint(hwnd,hdc);

			return 0;
		case MSG_KEYDOWN:
			if ( MV_Get_NumEdit_Flag() == TRUE )
			{
				char 		acTemp_Str[5];
				
				MV_NumEdit_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					memset(acTemp_Str, 0x00, 5);
					MV_NumEdit_Retrun_Value(acTemp_Str);
					MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude = atoi(acTemp_Str);
				}
				break;
			}
			
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(SAT_Focus_Item)
					{
						case SAT_ITEM_DISECQ:
							hdc = BeginPaint(hwnd);
							Mv_SAT_Disecq = u8Result_Value;							
							MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
							EndPaint(hwnd,hdc);
							
							/* By KB Kim 2011.01.13 */
							if (u8TpCount > 0)
							{
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							}
							else
							{
								/* By KB Kim 2011.01.21 */
								CS_FE_StopSearch();
							}
							break;

						/* For Tone Burst Control : By KB Kim 2011.02.28 */
						case SAT_ITEM_TONEBURST:
							hdc = BeginPaint(hwnd);
							Mv_SAT_Toneburst = u8Result_Value;							
							MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
							EndPaint(hwnd,hdc);
							
							/* By KB Kim 2011.01.13 */
							if (u8TpCount > 0)
							{
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							}
							else
							{
								/* By KB Kim 2011.01.21 */
								CS_FE_StopSearch();
							}
							break;

						case SAT_ITEM_LNB_TYPE:
							hdc = BeginPaint(hwnd);
							Mv_SAT_LNBType = u8Result_Value;
							MV_SAT_Change_LnbType(Mv_SAT_LNBType);
							MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
							EndPaint(hwnd,hdc);
							
							/* By KB Kim 2011.01.13 */
							if (u8TpCount > 0)
							{
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							}
							else
							{
								/* By KB Kim 2011.01.21 */
								CS_FE_StopSearch();
							}
							break;
						default:
							break;
					}
					hdc=BeginPaint(hwnd);
					MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
					EndPaint(hwnd,hdc);
					MV_SetSatelliteData(MV_Sat_Data);
					SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}
				break;
			}
			
			if ( Get_Keypad_Status() == TRUE )
			{
				UI_Keypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
				{
					if ( wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							Get_Save_Str(sReturn_str);
							strcpy(MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, sReturn_str);
							MV_SetSatelliteData(MV_Sat_Data);
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
							hdc = BeginPaint(hwnd);
							MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
							EndPaint(hwnd,hdc);
						}
					} else {
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					}
				}
				
				break;
			}	
		
			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_SAT_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_RED:
					if ( Sat_List_Status == FALSE )
					{
						CSApp_SAT_Applets=CSApp_Applet_Install;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_GREEN:
					if ( Sat_List_Status == FALSE )
					{	
						CSApp_SAT_Applets=CSApp_Applet_TP_Setting;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_YELLOW:
					if ( Sat_List_Status == FALSE && Search_Condition_Status == FALSE )
					{
						KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
						MV_Draw_Keypad(hwnd, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, MAX_SAT_NAME_LANGTH);
					}
					break;

				case CSAPP_KEY_BLUE:
					if ( Sat_List_Status == FALSE )
					{
						/* 2010.09.04 By KB Kim */
						Search_Condition_Focus_Item=SCAN_CON_ALL;
						MV_Search_Condition(hwnd, 0);
						//CSApp_Install_Applets=CSAPP_Applet_Install_Result;
						//SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_F2:
					{
						RECT		Edit_Win_rect;

						Edit_Win_rect.top = JUMP_WINDOW_STARTY;
						Edit_Win_rect.left = JUMP_WINDOW_STARTX;
						Edit_Win_rect.right = JUMP_WINDOW_STARTX + JUMP_WINDOW_STARTDX;
						Edit_Win_rect.bottom = JUMP_WINDOW_STARTY + JUMP_WINDOW_STARTDY;

						MV_Draw_Only_Number_Edit(hwnd, &Edit_Win_rect, MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 4, CSAPP_STR_EDIT);
					}
					break;
					
				case CSAPP_KEY_DOWN: 
					if ( Sat_List_Status == TRUE )
					{
						MV_stSatInfo	Temp_SatData;
						MV_stTPInfo		Temp_TPData;
						U16				Temp_Count;
						
						hdc = BeginPaint(hwnd);
						if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
						{
							/* By KB Kim 2011.01.13 */
							Temp_Count = MV_GetSatelliteData_Num();
							if (Temp_Count == 0)
							{
								break;
							}
							MV_GetSatelliteData_ByIndex(&Temp_SatData, u8SatList_Focus_Item);
							MV_SatList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_SatData, UNFOCUS);
						}
						else
						{
							/* By KB Kim 2011.01.13 */
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
							if (Temp_Count == 0)
							{
								break;
							}
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						/* By KB Kim 2011.01.13 */
						u8SatList_Focus_Item++;
						if ( u8SatList_Focus_Item >= Temp_Count )
						{
							u8SatList_Focus_Item = 0;
							u8SatList_Start_Point = 0;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							/* By KB Kim 2011.01.13 */
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;

							MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
						{
							u8SatList_Start_Point = u8SatList_Focus_Item;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							/* By KB Kim 2011.01.13 */
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;

							MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
							{
								MV_GetSatelliteData_ByIndex(&Temp_SatData, u8SatList_Focus_Item);
								MV_SatList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM , u8SatList_Start_Point, &Temp_SatData, FOCUS);
							} else {
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
								MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM , u8SatList_Start_Point, &Temp_TPData, FOCUS);
							}
						}
						EndPaint(hwnd,hdc);
					} else if ( Search_Condition_Status == TRUE ) {
						hdc = BeginPaint(hwnd);
						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, UNFOCUS);
					
						if ( Search_Condition_Focus_Item == SCAN_CON_MAX - 1 )
							Search_Condition_Focus_Item = SCAN_CON_ALL ;
						else
							Search_Condition_Focus_Item++;

						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, FOCUS);
						EndPaint(hwnd,hdc);
					} else {
						hdc = BeginPaint(hwnd);
						MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_Focus_Item);

						if(SAT_Focus_Item == SAT_ITEM_MAX - 1)
							SAT_Focus_Item = SAT_ITEM_SATELLITE;
						else if ( Mv_SAT_LNBType > UNIVERSAL_5750_5150 && SAT_Focus_Item == SAT_ITEM_LNB_TYPE )
						{
							if ( SearchData.ScanMode > SCAN_MODE_DISECQ10 )
								SAT_Focus_Item+=5;
							else
								SAT_Focus_Item+=4;
						}
						else if ( SAT_Focus_Item == SAT_ITEM_LNB_TYPE )
							SAT_Focus_Item+=3;
						else if ( SearchData.ScanMode > SCAN_MODE_DISECQ10 && SAT_Focus_Item == SAT_ITEM_22K )
							SAT_Focus_Item = SAT_ITEM_TONEBURST;
						else 
							SAT_Focus_Item++;

						MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
						EndPaint(hwnd,hdc);

						NumberKeyFlag=FALSE;						
					}
					break;
						
				case CSAPP_KEY_UP:
					if ( Sat_List_Status == TRUE )
					{
						MV_stSatInfo	Temp_SatData;
						MV_stTPInfo		Temp_TPData;
						U16				Temp_Count;

						hdc = BeginPaint(hwnd);
						if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
						{
							/* By KB Kim 2011.01.13 */
							Temp_Count = MV_GetSatelliteData_Num();
							if (Temp_Count == 0)
							{
								break;
							}
							MV_GetSatelliteData_ByIndex(&Temp_SatData, u8SatList_Focus_Item);
							MV_SatList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_SatData, UNFOCUS);
						} else
						{
							/* By KB Kim 2011.01.13 */
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
							if (Temp_Count == 0)
							{
								break;
							}
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						if ( u8SatList_Focus_Item == 0 )
						{
							/* By KB Kim 2011.01.13 */
							u8SatList_Focus_Item = Temp_Count - 1;
							u8SatList_Start_Point = LIST_ITEM_NUM * (Temp_Count/LIST_ITEM_NUM);
							u8SatList_End_Point = Temp_Count - 1;
							
							MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
						{
							u8SatList_Start_Point = u8SatList_Focus_Item - LIST_ITEM_NUM;
							u8SatList_End_Point = u8SatList_Focus_Item;
							/* By KB Kim 2011.01.13 */
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;
							u8SatList_Focus_Item--;

							MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							u8SatList_Focus_Item--;
							if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
							{
								MV_GetSatelliteData_ByIndex(&Temp_SatData, u8SatList_Focus_Item);
								MV_SatList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM , u8SatList_Start_Point, &Temp_SatData, FOCUS);
							} else {
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
								MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM , u8SatList_Start_Point, &Temp_TPData, FOCUS);
							}
						}
						EndPaint(hwnd,hdc);
					} else if ( Search_Condition_Status == TRUE ) {
						hdc = BeginPaint(hwnd);
						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, UNFOCUS);
					
						if ( Search_Condition_Focus_Item == SCAN_CON_ALL )
							Search_Condition_Focus_Item = SCAN_CON_MAX - 1 ;
						else
							Search_Condition_Focus_Item--;

						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, FOCUS);
						EndPaint(hwnd,hdc);
					} else {
						hdc = BeginPaint(hwnd);
						MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_Focus_Item);
						
						if(SAT_Focus_Item == SAT_ITEM_SATELLITE)
							SAT_Focus_Item = SAT_ITEM_MAX-1;
						else if ( SAT_Focus_Item == SAT_ITEM_DISECQ && Mv_SAT_LNBType > UNIVERSAL_5750_5150 )
							SAT_Focus_Item-=4;
						else if ( SAT_Focus_Item == SAT_ITEM_22K && Mv_SAT_LNBType <= UNIVERSAL_5750_5150 )
							SAT_Focus_Item-=3;
						else if ( SearchData.ScanMode > SCAN_MODE_DISECQ10 && SAT_Focus_Item == SAT_ITEM_TONEBURST && Mv_SAT_LNBType <= UNIVERSAL_5750_5150 )
							SAT_Focus_Item = SAT_ITEM_22K;
						else if ( SearchData.ScanMode > SCAN_MODE_DISECQ10 && SAT_Focus_Item == SAT_ITEM_TONEBURST && Mv_SAT_LNBType > UNIVERSAL_5750_5150 )
							SAT_Focus_Item = SAT_ITEM_LNB_TYPE;
						else
							SAT_Focus_Item--;

						MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
						EndPaint(hwnd,hdc);
						//printf("\n##### UP SAT_Focus_Item = %d\n", SAT_Focus_Item);
						
						NumberKeyFlag=FALSE;
					}
					break;
						
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{
						U16		Temp_Count;

						/* By KB Kim 2011.01.13 */
						if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
							Temp_Count = MV_GetSatelliteData_Num();
						else
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
						if (Temp_Count == 0)
						{
							break;
						}
						
						if ( u8SatList_Start_Point - LIST_ITEM_NUM < 0 )
							u8SatList_Start_Point = LIST_ITEM_NUM * ( Temp_Count / LIST_ITEM_NUM );
						else
						{
							if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
								u8SatList_Start_Point -= LIST_ITEM_NUM;
							else
								u8SatList_Start_Point = LIST_ITEM_NUM * (u8SatList_Focus_Item/LIST_ITEM_NUM);
						}

						/* By KB Kim 2011.01.13 */
						if ( u8SatList_Start_Point + LIST_ITEM_NUM >= Temp_Count )
						{
							u8SatList_End_Point = Temp_Count - 1;
							u8SatList_Focus_Item = u8SatList_Start_Point;
						}
						else
						{
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							u8SatList_Focus_Item = u8SatList_Start_Point;
						}

						hdc = BeginPaint(hwnd);
						MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						switch(SAT_Focus_Item)
						{
							case SAT_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);

								Mv_SAT_SatFocus = u8Glob_Sat_Focus;
								if ( MV_GetSelected_SatData_Count() > 0 )
								{
									if( u8Glob_Sat_Focus == 0 )
										u8Glob_Sat_Focus = MV_SAT_MAX -1;
									else
										u8Glob_Sat_Focus--;

									while( MV_Sat_Data[u8Glob_Sat_Focus].u16Select != SAT_SELECT )
									{
										if( u8Glob_Sat_Focus == 0 )
											u8Glob_Sat_Focus = MV_SAT_MAX -1;
										else
											u8Glob_Sat_Focus--;
									}
								}
								else
									u8Glob_Sat_Focus = 0;
								
								{
									/* By KB Kim 2011.01.13 */
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								}
								
								Mv_SAT_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
								Mv_SAT_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
								Mv_SAT_22K = MV_Sat_Data[u8Glob_Sat_Focus].u16Tone22K;
								u8Glob_TP_Focus = 0;
								
								MV_Draw_SatMenuList(hdc);
								MV_Draw_Sat_Selected_Satlist(hdc);
								EndPaint(hwnd,hdc);
								
#if 0 /* For Motor Control By KB Kim 2011.05.22 */
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ_MOTOR && MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition != 0 )
								{
									Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
									DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
								}
								else if ( SearchData.ScanMode == SCAN_MODE_USALS )
								{
									Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
									DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
								}
#endif

								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_TP_SELECT:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									u8Glob_TP_Focus = 0;
									break;
								}

								hdc = BeginPaint(hwnd);

								/* By KB Kim 2011.01.13 */
								if ( u8Glob_TP_Focus/*Mv_Install_TPFocus*/ == 0 )
								{
									/* By KB Kim 2011.01.13 */
									u8Glob_TP_Focus = u8TpCount - 1;
								}
								else
									u8Glob_TP_Focus--;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;
								
							case SAT_ITEM_LNB_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_LNBType == 0 )
									Mv_SAT_LNBType = MAX_LNBTYPE-1;
								else
									Mv_SAT_LNBType--;

								MV_SAT_Change_LnbType(Mv_SAT_LNBType);
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_HIGH_FREQ);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_LOW_FREQ);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_22K);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_22K:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_22K == TRUE )
									Mv_SAT_22K = FALSE;
								else
									Mv_SAT_22K = TRUE;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_DISECQ:
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
								{
									hdc = BeginPaint(hwnd);

									if ( Mv_SAT_Disecq == 0 )
										Mv_SAT_Disecq = MAX_DISECQ_PORT - 1;
									else
										Mv_SAT_Disecq--;
									
									MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
									EndPaint(hwnd,hdc);
									/* By KB Kim 2011.01.13 */
									if (u8TpCount > 0)
									{
										MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									}
									else
									{
										/* By KB Kim 2011.01.21 */
										CS_FE_StopSearch();
									}
									NumberKeyFlag=FALSE;
								}
								break;

							case SAT_ITEM_TONEBURST:
								hdc = BeginPaint(hwnd);

								/* For Tone Burst Control : By KB Kim 2011.02.28 */
								if ( Mv_SAT_Toneburst == 0 )
								{
									Mv_SAT_Toneburst = TONE_SAT_B;
								}
								else
								{
									Mv_SAT_Toneburst--;
								}
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_ScanType == MULTI_SAT )
									Mv_SAT_ScanType = ONE_SAT;
								else
									Mv_SAT_ScanType = MULTI_SAT;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								MV_Draw_Sat_Selected_Satlist(hdc);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;
								break;

							default:
								break;
						}
					}
        			break;
						
				case CSAPP_KEY_RIGHT:
					if ( Sat_List_Status == TRUE )
					{
						U16		Temp_Count;

						/* By KB Kim 2011.01.13 */
						if ( SAT_Focus_Item == SAT_ITEM_SATELLITE )
							Temp_Count = MV_GetSatelliteData_Num();
						else
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
						if (Temp_Count == 0)
						{
							break;
						}
						
						if ( u8SatList_Start_Point + LIST_ITEM_NUM >= Temp_Count )
							u8SatList_Start_Point = 0;
						else
						{
							if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
								u8SatList_Start_Point += LIST_ITEM_NUM;
							else 
							{
								u8SatList_Start_Point = LIST_ITEM_NUM * ((u8SatList_Focus_Item/LIST_ITEM_NUM) + 1 );

								if ( u8SatList_Start_Point >= Temp_Count )
									u8SatList_Start_Point = Temp_Count - 1;
							}
						}

						if ( u8SatList_Start_Point + LIST_ITEM_NUM >= Temp_Count )
						{
							u8SatList_End_Point = Temp_Count - 1;
							u8SatList_Focus_Item = u8SatList_Start_Point;
						}
						else
						{
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							u8SatList_Focus_Item = u8SatList_Start_Point;
						}

						hdc = BeginPaint(hwnd);
						MV_Sat_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						switch(SAT_Focus_Item)
						{
							case SAT_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);

								Mv_SAT_SatFocus = u8Glob_Sat_Focus;
								if ( MV_GetSelected_SatData_Count() > 0 )
								{
									if( u8Glob_Sat_Focus == MV_SAT_MAX -1 )
										u8Glob_Sat_Focus = 0;
									else
										u8Glob_Sat_Focus++;

									while( MV_Sat_Data[u8Glob_Sat_Focus].u16Select != SAT_SELECT )
									{
										if( u8Glob_Sat_Focus == MV_SAT_MAX -1 )
											u8Glob_Sat_Focus = 0;
										else
											u8Glob_Sat_Focus++;
									}
								}
								else
									u8Glob_Sat_Focus = 0;

								{
									/* By KB Kim 2011.01.13 */
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								}
								
								Mv_SAT_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
								Mv_SAT_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
								Mv_SAT_22K = MV_Sat_Data[u8Glob_Sat_Focus].u16Tone22K;
								u8Glob_TP_Focus = 0;
								
								MV_Draw_SatMenuList(hdc);
								MV_Draw_Sat_Selected_Satlist(hdc);
								EndPaint(hwnd,hdc);

#if 0 /* For Motor Control By KB Kim 2011.05.22 */
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ_MOTOR && MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition != 0 )
								{
									Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
									DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
								}
								else if ( SearchData.ScanMode == SCAN_MODE_USALS )
								{
									Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
									DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
								}
#endif

								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_TP_SELECT:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									u8Glob_TP_Focus = 0;
									break;
								}
								
								hdc = BeginPaint(hwnd);

								u8Glob_TP_Focus++;
								if ( u8Glob_TP_Focus >= u8TpCount )
									u8Glob_TP_Focus = 0;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;
								
							case SAT_ITEM_LNB_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_LNBType == MAX_LNBTYPE - 1 )
									Mv_SAT_LNBType = 0;
								else
									Mv_SAT_LNBType++;

								MV_SAT_Change_LnbType(Mv_SAT_LNBType);
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_HIGH_FREQ);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_LOW_FREQ);
								MV_Draw_SatMenuBar(hdc, MV_UNFOCUS, SAT_ITEM_22K);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_22K:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_22K == TRUE )
									Mv_SAT_22K = FALSE;
								else
									Mv_SAT_22K = TRUE;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_DISECQ:
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
								{
									hdc = BeginPaint(hwnd);

									if ( Mv_SAT_Disecq == MAX_DISECQ_PORT - 1 )
										Mv_SAT_Disecq = 0;
									else
										Mv_SAT_Disecq++;
									
									MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
									EndPaint(hwnd,hdc);
									/* By KB Kim 2011.01.13 */
									if (u8TpCount > 0)
									{
										MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									}
									else
									{
										/* By KB Kim 2011.01.21 */
										CS_FE_StopSearch();
									}
									NumberKeyFlag=FALSE;
								}
								break;

							case SAT_ITEM_TONEBURST:
								hdc = BeginPaint(hwnd);

								/* For Tone Burst Control : By KB Kim 2011.02.28 */
								if ( Mv_SAT_Toneburst == TONE_SAT_B )
								{
									Mv_SAT_Toneburst = 0;
								}
								else
								{
									Mv_SAT_Toneburst++;
								}
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								EndPaint(hwnd,hdc);
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case SAT_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_SAT_ScanType == MULTI_SAT )
									Mv_SAT_ScanType = ONE_SAT;
								else
									Mv_SAT_ScanType = MULTI_SAT;
								
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
								MV_Draw_Sat_Selected_Satlist(hdc);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;

							default:
								break;
						}
					}
					break;
			
				case CSAPP_KEY_ENTER:     
					{
						if ( Sat_List_Status == TRUE )
						{
							MV_SAT_List_Window_Close( hwnd );

							hdc = BeginPaint(hwnd);
							
							if (SAT_Focus_Item == SAT_ITEM_SATELLITE)
							{
								/* By KB Kim 2011.01.13 */
								Mv_SAT_SatFocus = u8Glob_Sat_Focus;
								if ( u8Glob_Sat_Focus != u8SatList_Focus_Item )
								{
									u8Glob_Sat_Focus = u8SatList_Focus_Item;
									u8Glob_TP_Focus	= 0;
									
									/* By KB Kim 2011.01.13 */
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
									
									Mv_SAT_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
									Mv_SAT_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
									Mv_SAT_22K = MV_Sat_Data[u8Glob_Sat_Focus].u16Tone22K;
									MV_Draw_SatMenuList(hdc);
									MV_Draw_Sat_Selected_Satlist(hdc);

#if 0 /* For Motor Control By KB Kim 2011.05.22 */									
									if ( SearchData.ScanMode == SCAN_MODE_DISECQ_MOTOR && MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition != 0 )
									{
										Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
										DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
									}
									else if ( SearchData.ScanMode == SCAN_MODE_USALS )
									{
										Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[Mv_SAT_SatFocus].s16Longitude);
										DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
									}
#endif
								}
							}
							else
							{
								u8Glob_TP_Focus = u8SatList_Focus_Item;
								MV_Draw_SatMenuBar(hdc, MV_FOCUS, SAT_Focus_Item);
							}
							EndPaint(hwnd,hdc);
							
							/* By KB Kim 2011.01.13 */								
							if (u8TpCount > 0)
							{
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							}
							else
							{
								u8Glob_TP_Focus = 0;
								/* By KB Kim 2011.01.21 */
								CS_FE_StopSearch();
							}
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						}else if ( Search_Condition_Status == TRUE ) {
							Search_Condition_Status = FALSE;
							MV_Install_Warning_Window_Close( hwnd );
							set_prev_windown_status(CSApp_Applet_Sat_Setting);
							CSApp_SAT_Applets=CSAPP_Applet_Install_Result;

							if ( Mv_SAT_ScanType == ONE_SAT )
							{
								Search_Condition_Sat_Index[0] = u8Glob_Sat_Focus;
								u8Multi_Select_Sat = 1;
							}
							else
							{
								U8				i;
								MV_stSatInfo	Temp_SatData;
								
								u8Multi_Select_Sat = MV_GetSelected_SatData_Count();

								if ( u8Multi_Select_Sat == 0 )
								{
									return 0;
								} else {
									for ( i =0 ; i < u8Multi_Select_Sat ; i++ )
									{
										MV_GetSelected_SatData_By_Count(&Temp_SatData, i );
										Search_Condition_Sat_Index[i] = Temp_SatData.u8SatelliteIndex;
									}
								}
							}
							
							MV_SetSatelliteData(MV_Sat_Data);
							
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
						else
						{
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);

							switch ( SAT_Focus_Item )
							{
								case SAT_ITEM_SATELLITE:
									{
										u8SatList_Focus_Item = u8Glob_Sat_Focus;
										u8SatList_Start_Point = ( u8SatList_Focus_Item / LIST_ITEM_NUM ) * LIST_ITEM_NUM;

										if ( u8SatList_Start_Point + LIST_ITEM_NUM >= MV_SAT_MAX)
											u8SatList_End_Point = MV_SAT_MAX -1;
										else
											u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
										
										MV_Sat_List_Window( hwnd );
									} 
									break;
									
								case SAT_ITEM_TP_SELECT:
									/* By KB Kim 2011.01.13 */
									if (u8TpCount == 0)
									{
										u8SatList_Focus_Item  = 0;
										u8SatList_Start_Point = 0;
										u8SatList_End_Point   = 0;
									}
									else
									{
										u8SatList_Focus_Item = u8Glob_TP_Focus;
										u8SatList_Start_Point = ( u8SatList_Focus_Item / LIST_ITEM_NUM ) * LIST_ITEM_NUM;

										/* By KB Kim 2011.01.13 */
										if ( u8SatList_Start_Point + LIST_ITEM_NUM >= u8TpCount )
											u8SatList_End_Point = u8TpCount - 1;
										else
											u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
									}

									MV_Sat_List_Window( hwnd );
									break;
									
								case SAT_ITEM_DISECQ:
									if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										for ( i = 0 ; i < 22 ; i++ )
											sprintf(stContents.Contents[i], "%s", Diseqc_Port[i]);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( SAT_Focus_Item - 3 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 21;
										stContents.u8Focus_Position = Mv_SAT_Disecq;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								/* For Tone Burst Control : By KB Kim 2011.02.28 */
								case SAT_ITEM_TONEBURST:
								{
									int						i = 0;
									RECT					smwRect;
									stPopUp_Window_Contents stContents;

									memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
									for ( i = 0 ; i < 3 ; i++ )
										sprintf(stContents.Contents[i], "%s", Tone_Port[i]);
									
									smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * (SAT_Focus_Item - 2);
									smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
									smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
									smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
									stContents.u8TotalCount = 3;
									stContents.u8Focus_Position = Mv_SAT_Toneburst;
									MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								}
									break;

								case SAT_ITEM_LNB_TYPE:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										for ( i = 0 ; i < 22 ; i++ )
											sprintf(stContents.Contents[i], "%s", LNB_Type[i]);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( SAT_Focus_Item + 1);
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4 - 50);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 + 100;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 21;
										stContents.u8Focus_Position = Mv_SAT_LNBType;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;
								
								default:
									break;
							}
						} 
					}
					break;

				case CSAPP_KEY_INFO:
					
					break;
				case CSAPP_KEY_ESC:
					if ( Sat_List_Status == TRUE )
					{
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						MV_SAT_List_Window_Close( hwnd );
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
						if ( MV_DB_GetALLServiceNumber() == 0 )
						{
							CSApp_SAT_Applets=CSApp_Applet_MainMenu;
							MV_SetSatelliteData(MV_Sat_Data);
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
						else
						{
							CSApp_SAT_Applets=CSApp_Applet_Desktop;
							MV_SetSatelliteData(MV_Sat_Data);
							/* By KB Kim 2011.05.28 */
							CS_AV_VideoBlank();
							MV_MW_StartService(CS_DB_GetCurrentServiceIndex());
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
					}
					break;
					
				case CSAPP_KEY_MENU:
					if ( Sat_List_Status == TRUE )
					{
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						MV_SAT_List_Window_Close( hwnd );
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
#ifdef SMART_PHONE
						if ( b8Last_App_Status == CSApp_Applet_Smart_OSD )
							CSApp_SAT_Applets=b8Last_App_Status;
						else
#endif
						{
							if ( MV_DB_GetALLServiceNumber() > 0 )
							{
								/* By KB Kim 2011.05.28 */
								CS_AV_VideoBlank();
							}
							CSApp_SAT_Applets=CSApp_Applet_MainMenu;
						}
						MV_SetSatelliteData(MV_Sat_Data);
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;
				default:					
					break;
			}
			break;
			
		default:
			break;			
	}
	
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

