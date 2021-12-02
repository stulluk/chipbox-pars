
#include "linuxos.h"

#include "database.h"
#include "ch_install.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csinstall.h"
#include "dvbtuner.h"
#include "fe_mngr.h"
#include "ui_common.h"
#include "mv_motor.h"

U16 Install_Str[SCAN_ITEM_MAX+1] = {
	CSAPP_STR_CONECT_TYPE,
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TPSELECT,
	CSAPP_STR_LNBTYPE,
	CSAPP_STR_H_FREQUENCY,
	CSAPP_STR_L_FREQUENCY,
	CSAPP_STR_DISECQ_SETTING,
	CSAPP_STR_SCANTYPE,
	CSAPP_STR_FREQUENCY
};

U8	Install_Arrow_Kind[SCAN_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT
};

U8	Install_Enter_Kind[SCAN_ITEM_MAX] = {
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_STATIC,
	MV_STATIC
};

U16 Connect_Type[5] = {
	CSAPP_STR_SINGLE,
	CSAPP_STR_DISEQC_PORT,
	CSAPP_STR_DISEQC_MOTOR,
	CSAPP_STR_USALS,
	CSAPP_STR_UNICABLE
};

U16 Search_Condition[7] = {
	CSAPP_STR_FTASAT,
	CSAPP_STR_FTASAT_NIT,
	CSAPP_STR_FTA,
	CSAPP_STR_FTA_NIT,
	CSAPP_STR_FTASAT_BLIND,
	CSAPP_STR_FTA_BLIND
};

char LNB_Type[22][30] = {	
	"Single 5150",
	"Single 5750",
	"Single 9750",
	"Single 10000",
	"Single 10600",
	"Single 10678",
	"Single 10700",
	"Single 10750",
	"Single 11000",
	"Single 11200",
	"Single 11250",
	"Single 11300",
	"Wide 5150-5750",
	"Wide 5750-5150",
	"Universal 9750-10600",
	"Universal 9750-10700",
	"Universal 9750-10750",
	"DigiTurk 1",
	"DigiTurk 2",
	"DigiTurk 3",
	"DigiTurk 4"
};

char Diseqc_Port[21][10] = {
	"Direct",
	"A",
	"B",
	"C",
	"D",
	"1/16",
	"2/16",
	"3/16",
	"4/16",
	"5/16",
	"6/16",
	"7/16",
	"8/16",
	"9/16",
	"10/16",
	"11/16",
	"12/16",
	"13/16",
	"14/16",
	"15/16",
	"16/16"
};

/* For Blind Scan By KB Kim 2011.02.26 */
U8 SearchBlindModeOn;
U8 CurrentBlindScanSatIndex;
U8 BlindTpSearch;
U8 CurrentBlindProcess;
U16 CurrentBlindTpTotal;
MV_BlindScanParams      BlindScanParam;


/* Move By KB Km 2010.09.04 */
eScanConditionID			Search_Condition_Focus_Item;

static CSAPP_Applet_t		CSApp_Install_Applets;
static eScanItemID			Install_Focus_Item=SCAN_ITEM_CONNECTION;
static U8					Mv_Install_LNBType=0;
static U16					Mv_Install_Disecq=0;
static U8					Mv_Install_ScanType=0;
static U8					u8TpCount;
static BOOL					NumberKeyFlag=FALSE;

static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];  //stSatInfo_Glob
static MV_stTPInfo 			MV_TPInfo;

//below for search result
static CSAPP_Applet_t		CSApp_SearchResult_Applets;

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

static U8   				Install_client = 0;
static U8					u8SatList_Start_Point;
static U8					u8SatList_End_Point;
static BOOL					Sat_List_Status = FALSE;
static BOOL					Flag_SatList_Change = FALSE;

static U8					u8SatList_Focus_Item;
extern U32					*Tuner_HandleId;
static TunerSignalState_t 	Siganl_State;
static U16					u16Search_Current_Sat_Index;
static MV_stTPInfo 			Display_TPDatas;

static U8					Mv_SAT_SatFocus;

static int Install_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
static int SearchResult_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

/*
void MV_Setting_Disecq_Motor(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(D_WIN_X), ScalerHeigthPixel(D_WIN_Y), ScalerWidthPixel(D_WIN_DX), ScalerHeigthPixel(D_WIN_DY), &big_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(D_WIN_X), ScalerHeigthPixel(D_WIN_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(D_WIN_X + D_WIN_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(D_WIN_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(D_WIN_X), ScalerHeigthPixel(D_WIN_Y + D_WIN_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(D_WIN_X + D_WIN_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(D_WIN_Y + D_WIN_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(D_WIN_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(D_WIN_Y),ScalerWidthPixel(D_WIN_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(D_WIN_DY));
	FillBox(hdc,ScalerWidthPixel(D_WIN_X), ScalerHeigthPixel(D_WIN_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(D_WIN_DX),ScalerHeigthPixel(D_WIN_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
	
	Temp_Rect.top 	= D_WIN_Y + D_WIN_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= D_WIN_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + D_WIN_ITEM_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_WARNING);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(D_WIN_ITEM_X), ScalerHeigthPixel(D_WIN_ITEM_Y), ScalerWidthPixel(D_WIN_ITEM_DX), ScalerHeigthPixel(D_WIN_ITEM_DY) );
	
	MV_EndPaint(hwnd,hdc);
}

void Close_Antena_Setting_Window(HWND hwnd)
{
	HDC 	hdc;

	hdc = MV_BeginPaint(hwnd);
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(D_WIN_X), ScalerHeigthPixel(D_WIN_Y), ScalerWidthPixel(D_WIN_DX), ScalerHeigthPixel(D_WIN_DY), &big_bmp);
	UnloadBitmap (&big_bmp);
	MV_EndPaint(hwnd,hdc);
}
*/

void MV_Draw_Selected_Satlist(HDC hdc)
{
	U8				u8SelCount;
	U8				i;
	MV_stSatInfo	Temp_SatData;
	
	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_SELSAT_X), ScalerHeigthPixel(MV_INSTALL_SELSAT_Y), ScalerWidthPixel(MV_INSTALL_SELSAT_DX), ScalerHeigthPixel(MV_INSTALL_SELSAT_DY) );

	u8SelCount = MV_GetSelected_SatData_Count();
	if ( u8SelCount == 0 )
		return;

	for ( i = 0 ; i < u8SelCount ; i++ )
	{
		if ( i == MAX_MULTI_SAT )
			break;
		
		MV_GetSelected_SatData_By_Count(&Temp_SatData, i );

		if ( ( u8Glob_Sat_Focus == Temp_SatData.u8SatelliteIndex ) || Mv_Install_ScanType == MULTI_SAT )
		{
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} else {
			SetTextColor(hdc,MVAPP_SCROLL_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_SELSAT_X + ( ( MV_INSTALL_SELSAT_DX / 3 ) * (i%3)) + 10),ScalerHeigthPixel(MV_INSTALL_SELSAT_Y + (MV_INSTALL_MENU_BAR_H * (i/3)) + 4 ), Temp_SatData.acSatelliteName);
	}

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
}

void MV_Draw_SatSelectBar(HDC hdc, int y_gap)
{
	int mid_width = WINDOW_ITEM_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth ) - SCROLL_BAR_DX;
	int right_x = WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
}

void MV_Draw_SearchConditionSelectBar(HDC hdc, int y_gap)
{
	int mid_width = WARNING_ITEM_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = WARNING_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth)/2 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
}

void MV_SearchCondition_Bar_Draw(HDC hdc, int iCount, U8 FocusKind)
{
	int 	y_gap = WARNING_ITEM_Y + ( MV_INSTALL_MENU_BAR_H ) * iCount;
	int		x_point=0;

	if ( FocusKind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		MV_Draw_SearchConditionSelectBar(hdc, y_gap);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(WARNING_ITEM_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}
	SetBkMode(hdc,BM_TRANSPARENT);
	x_point = WARNING_ITEM_X + 20;
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Search_Condition[iCount]));
}

void MV_SatList_Bar_Draw(HDC hdc, int iCount, U8 u8Start_Point, MV_stSatInfo *Temp_SatData, U8 FocusKind)
{
	int 	y_gap = WINDOW_ITEM_Y + ( MV_INSTALL_MENU_BAR_H ) * iCount;
	int		x_point=0;
	char	TmpStr[30];

	if ( FocusKind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		MV_Draw_SatSelectBar(hdc, y_gap);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}
	SetBkMode(hdc,BM_TRANSPARENT);

	x_point = WINDOW_ITEM_X + 10;
	sprintf(TmpStr, "%d", iCount + u8Start_Point + 1);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);
	
	if ( Temp_SatData->u16Select == SAT_SELECT )
	{
		x_point += 50;
		//FillBoxWithBitmap(hdc,ScalerWidthPixel(x_point), ScalerHeigthPixel(y_gap), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(x_point), ScalerHeigthPixel(y_gap), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_NCHECK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_NCHECK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NCHECK_ICON]);
		x_point += 50;
	} else {
		x_point += 100;
	}
	
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), Temp_SatData->acSatelliteName);

	x_point += 300;
	sprintf(TmpStr, "%d", Temp_SatData->s16Longitude);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);
}

void MV_TPList_Bar_Draw(HDC hdc, int iCount, U8 u8Start_Point, MV_stTPInfo *Temp_TPData, U8 FocusKind)
{
	int 	y_gap = WINDOW_ITEM_Y + ( MV_INSTALL_MENU_BAR_H ) * iCount;
	int		x_point=0;
	char	TmpStr[30];

	if ( FocusKind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		MV_Draw_SatSelectBar(hdc, y_gap);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	}
	SetBkMode(hdc,BM_TRANSPARENT);

	x_point = WINDOW_ITEM_X + 10;
	sprintf(TmpStr, "%d", iCount + u8Start_Point + 1);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);
	
	x_point += 100;
	sprintf(TmpStr, "%d", Temp_TPData->u16TPFrequency);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);

	x_point += 150;
	if ( Temp_TPData->u8Polar_H == 1 )
		sprintf(TmpStr, "%s", "H");
	else
		sprintf(TmpStr, "%s", "V");
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);

	x_point += 100;
	sprintf(TmpStr, "%d", Temp_TPData->u16SymbolRate);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(x_point),ScalerHeigthPixel(y_gap+4), TmpStr);
}

void MV_Install_Satlist_Item_Draw(HDC hdc, U8 u8Start_Point, U8 u8End_Point)
{
	int 			i;
	MV_stSatInfo	Temp_SatData;
	MV_stTPInfo		Temp_TPData;
	RECT			Scroll_Rect;

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(WINDOW_ITEM_DX), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	for ( i = 0 ; i < LIST_ITEM_NUM ; i++ )
	{
		if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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

	if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_SAT_LIST, MV_VERTICAL);
	else
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_TP_LIST, MV_VERTICAL);
}

/* For Blind Scan By KB Kim 2011.03.10 */
void MV_Search_Condition_Item_Draw(HDC hdc, U8 mode)
{
	int 			i;
	int             item;
	U32             boxDY;

	if (mode)
	{
		boxDY = WARNING_ITEM_DY;
		item  = 4;
	}
	else
	{
		boxDY = WARNING_ITEM_DY + 60;
		item  = SCAN_CON_MAX;
	}

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(boxDY) );

	for ( i = 0 ; i < item ; i++ )
	{
		if ( i == (int)Search_Condition_Focus_Item )
			MV_SearchCondition_Bar_Draw(hdc, i, FOCUS);
		else
			MV_SearchCondition_Bar_Draw(hdc, i, UNFOCUS);
	}
}

void MV_Install_List_Window( HWND hwnd )
{
	HDC 	hdc;
	RECT	Temp_Rect;
	/* By KB Kim 2011.01.13 */
	U8      itemCount;

	Sat_List_Status = TRUE;
	Flag_SatList_Change = FALSE;

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
	if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
	MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
	}

	if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + 30), ScalerHeigthPixel(WINDOW_BOTTOM - 40), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(WINDOW_ITEM_X + 40 + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(WINDOW_BOTTOM - 40), CS_MW_LoadStringByIdx(CSAPP_STR_SELECT));
	}
	
	MV_EndPaint(hwnd,hdc);
}

/* For Blind Scan By KB Kim 2011.03.10 */
void MV_Search_Condition( HWND hwnd, U8 mode)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	U32     boxDY;

	Search_Condition_Status = TRUE;

	if (mode)
	{
		boxDY = WARNING_DY;
	}
	else
	{
		boxDY = WARNING_DY + 60;
	}

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY + 60), &WarningCapture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + boxDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP + boxDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WARNING_TOP),ScalerWidthPixel(WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(boxDY));
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_DX),ScalerHeigthPixel(boxDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	Temp_Rect.top 	= WARNING_TOP + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_LEFT + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + WARNING_DX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_SEARCH_CONDITION);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	if (mode)
	{
		MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY) );
	}
	else
	{
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY + 60) );
	}

	MV_Search_Condition_Item_Draw(hdc, mode);
	
	MV_EndPaint(hwnd,hdc);
}

void MV_Install_List_Window_Close( HWND hwnd )
{
	HDC		hdc;
	
	Sat_List_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

void MV_Install_Warning_Window_Close( HWND hwnd )
{
	HDC		hdc;

	Warning_Window_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY + 60), &WarningCapture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&WarningCapture_bmp);
}

void MV_Install_draw_help_banner(HDC hdc, MV_Menu_Kind mv_list_kind)
{	
	switch(mv_list_kind)
	{
		case INSTALL_SEARCH:
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_SATELLITE_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX3 + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_TP_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX3*2), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX3*2 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_SCAN));
//			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
//			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(MV_HELP_ICON_Y),	"Edit");
			break;
		case SATELLITE_SETTING:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth),	ScalerHeigthPixel(MV_HELP_ICON_Y - 10),	CS_MW_LoadStringByIdx(CSAPP_STR_INSTALL_SEARCH));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + 80), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + 80 + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth),	ScalerHeigthPixel(MV_HELP_ICON_Y - 10),	CS_MW_LoadStringByIdx(CSAPP_STR_TP_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + 40), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + 40 + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth),	ScalerHeigthPixel(MV_HELP_ICON_Y - 10),	CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3), ScalerHeigthPixel(MV_HELP_ICON_Y - 10), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth),	ScalerHeigthPixel(MV_HELP_ICON_Y - 10),	CS_MW_LoadStringByIdx(CSAPP_STR_SCAN));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y + 25), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
			CS_MW_TextOut(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth),	ScalerHeigthPixel(MV_HELP_ICON_Y + 25),	CS_MW_LoadStringByIdx(CSAPP_STR_MODIFY_LONGITUDE));
			break;
		case TP_SETTING:
		default:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
			CS_MW_TextOut_Static(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 ),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_INSTALL_SEARCH));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + 80), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
			CS_MW_TextOut_Static(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4 + 80 + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth + 4),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_SATELLITE_SETTING));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + 110), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
			CS_MW_TextOut_Static(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*2 + 110 + MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth + 4),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_ALLTP));
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + 70), ScalerHeigthPixel(MV_HELP_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
			CS_MW_TextOut_Static(hdc,ScalerWidthPixel(MV_HELP_ICON_X + MV_HELP_ICON_DX4*3 + 70 + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth + 4),	ScalerHeigthPixel(MV_HELP_ICON_Y),	CS_MW_LoadStringByIdx(CSAPP_STR_SCAN));
			break;
	}
}

void Install_Draw_Warning(HWND hwnd, U16 u16Message_index)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	Warning_Window_Status = TRUE;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &WarningCapture_bmp);
	
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY) );

	MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_TOP + WARNING_OUT_GAP), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	Temp_Rect.top 	= WARNING_TOP + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_WARNING), -1, &Temp_Rect, DT_CENTER);	

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY) );

	Temp_Rect.top 	= WARNING_ITEM_Y + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(u16Message_index), -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);
	
	MV_EndPaint(hwnd,hdc);
}

CSAPP_Applet_t CSApp_Install(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_Install_Applets = CSApp_Applet_Error;

	Install_Focus_Item = SCAN_ITEM_HIGH_FREQ; 
	
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
	CreateInfo.spCaption 	= "Install";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Install_Msg_cb;
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
	
	//printf("Install Cleanup\n");
	
	return CSApp_Install_Applets;
	
}

CSAPP_Applet_t CSApp_SearchResult(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
	
	CSApp_SearchResult_Applets = CSApp_Applet_Error;
	
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
	CreateInfo.spCaption = "searchresult";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = SearchResult_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = CSAPP_BLACK_COLOR;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);
	//printf("SearchResult CreateMainWindow = %d\n", hwndMain);
	
	if (hwndMain == HWND_INVALID)	
		return CSApp_Applet_Error;
	
	ShowWindow(hwndMain, SW_SHOWNORMAL);
	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	
	//printf("SearchResult Cleanup\n");
	
	return CSApp_SearchResult_Applets;	
}

void MV_Draw_SelectBar(HDC hdc, int y_gap, eScanItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Install_Enter_Kind[esItem])
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
	
	if ( Install_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

static void MV_Change_LnbType(U8 Mv_LNBType)
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

void MV_Draw_MenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	temp_str[30];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_SelectBar(hdc, y_gap, esItem);
	} else {
		if ( esItem == SCAN_ITEM_HIGH_FREQ || esItem == SCAN_ITEM_LOW_FREQ )
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}

	if ( esItem == SCAN_ITEM_HIGH_FREQ )
	{
		if ( Mv_Install_LNBType < UNIVERSAL_5150_5750 )
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Install_Str[SCAN_ITEM_MAX]));
		else 
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Install_Str[esItem]));
	} else if ( esItem == SCAN_ITEM_LOW_FREQ ) {
		if ( Mv_Install_LNBType >= UNIVERSAL_5150_5750 )
			MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Install_Str[esItem]));
	} else 
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(Install_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	memset ( temp_str, 0, 30 );

	switch(esItem)
	{
		
		case SCAN_ITEM_CONNECTION:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Connect_Type[SearchData.ScanMode]), -1, &TmpRect, DT_CENTER);		
			break;
		case SCAN_ITEM_SATELLITE:
			CS_MW_DrawText(hdc, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, -1, &TmpRect, DT_CENTER);	
			break;
		case SCAN_ITEM_TP_SELECT:
			/* By KB Kim 2011.01.13 */
			if ( u8TpCount > 0 )
			{
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&MV_TPInfo, u8Glob_Sat_Focus, u8Glob_TP_Focus);
				/* By KB Kim 2011.01.13 */
				if ( MV_TPInfo.u8Polar_H == 1 )
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount, MV_TPInfo.u16TPFrequency, "H", MV_TPInfo.u16SymbolRate);
				else
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount, MV_TPInfo.u16TPFrequency, "V", MV_TPInfo.u16SymbolRate);
			} else {
				sprintf(temp_str, "0/0. 0/V/0");
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case SCAN_ITEM_LNB_TYPE:
			CS_MW_DrawText(hdc, LNB_Type[Mv_Install_LNBType], -1, &TmpRect, DT_CENTER);	
			break;
		case SCAN_ITEM_HIGH_FREQ:
			sprintf(temp_str, "%d", MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency_High);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SCAN_ITEM_LOW_FREQ:
			if ( Mv_Install_LNBType >= UNIVERSAL_5150_5750 )
			{
				sprintf(temp_str, "%d", MV_Sat_Data[u8Glob_Sat_Focus].u16LocalFrequency);
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			}
			break;
		case SCAN_ITEM_DISECQ:
			{
				switch(SearchData.ScanMode)
				{
					case SCAN_MODE_SINGLE:
						sprintf(temp_str, "%s", "Direct");
						CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
						break;
						
					case SCAN_MODE_DISECQ10:				
						sprintf(temp_str, "%s", Diseqc_Port[Mv_Install_Disecq]);
						CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
						break;

					case SCAN_MODE_UNICABLE:
					case SCAN_MODE_USALS:
					case SCAN_MODE_DISECQ_MOTOR:
					default:
						sprintf(temp_str, "%s", "Enter");
						CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
						break;
				}
			}
			break;
		case SCAN_ITEM_SCAN_TYPE:
			if ( Mv_Install_ScanType == MULTI_SAT )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_MULTI_SAT));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ONE_SAT));
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case SCAN_ITEM_MAX:
		default:
			break;
	}
}

void MV_Draw_InstallMenuBar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < SCAN_ITEM_MAX ; i++ )
	{
		if( Install_Focus_Item == i )
		{
			MV_Draw_MenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_MenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

void Draw_Progress_Bar(HDC hdc, U8 u8ProgressValue)
{
	RECT Temp_Rect;

	Temp_Rect.top 		= RE_PROG_TOP + 5;
	Temp_Rect.bottom	= RE_PROG_BOTTOM - 5;
	Temp_Rect.left		= RE_PROG_LEFT + 10;
	Temp_Rect.right		= RE_PROG_RIGHT - 100;
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	MV_Draw_LevelBar(hdc, &Temp_Rect, u8ProgressValue, EN_ITEM_SIGNAL_LEVEL);
}

void Draw_Blind_Progress_Bar(HDC hdc, U8 u8ProgressValue)
{
	RECT Temp_Rect;

	Temp_Rect.top 		= RE_PROG_TOP + 35;
	Temp_Rect.bottom	= RE_PROG_BOTTOM + 25;
	Temp_Rect.left		= RE_PROG_LEFT + 10;
	Temp_Rect.right		= RE_PROG_RIGHT - 100;
	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	MV_Draw_LevelBar(hdc, &Temp_Rect, u8ProgressValue, EN_ITEM_SIGNAL_LEVEL);
}

void Show_Signal(HDC hdc)
{	
	RECT	Temp_Rect;

	TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);
	//printf("\n ====== %d % ==== %d % ======\n", Siganl_State.Strength, Siganl_State.Quality);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		
	Temp_Rect.top 		= MV_INSTALL_SIGNAL_Y;
	Temp_Rect.bottom	= Temp_Rect.top + MV_BMP[MVBMP_RED_SIGNAL].bmHeight + 10;
	Temp_Rect.left		= MV_INSTALL_SIGNAL_X;
	Temp_Rect.right		= MV_INSTALL_SIGNAL_X + MV_INSTALL_SIGNAL_DX;
	
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Strength, EN_ITEM_SIGNAL_LEVEL);

	Temp_Rect.top 		= Temp_Rect.top + MV_INSTALL_SIGNAL_YGAP;
	Temp_Rect.bottom	= Temp_Rect.top + MV_BMP[MVBMP_RED_SIGNAL].bmHeight + 10;
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Quality, EN_ITEM_SIGNAL_LEVEL);
}

static int Install_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	static BOOL   	bk_flag = TRUE;
	
	switch(message)
	{
		case MSG_CREATE:
			u8Multi_Select_Sat = 0;
			
			Install_Focus_Item = SCAN_ITEM_CONNECTION;
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
			Mv_Install_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
			Mv_Install_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;

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
				/* By KB Kim 2011.01.21 */
				CS_FE_StopSearch();
			}
			break;
		case MSG_CLOSE:
			bk_flag=FALSE;
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);

			if(CS_DBU_CheckIfUserSettingDataChanged())
			{
				CS_DBU_SaveUserSettingDataInHW();
			}
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, INSTALL_SEARCH);

			hdc = BeginPaint(hwnd);

			MV_Draw_InstallMenuBar(hdc);
			MV_Draw_Selected_Satlist(hdc);

			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y ), CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH));
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y + MV_INSTALL_SIGNAL_YGAP ), CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY));
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			Show_Signal(hdc);

			MV_Install_draw_help_banner(hdc, INSTALL_SEARCH);
			EndPaint(hwnd,hdc);
			return 0;
			
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
		case MSG_KEYDOWN:				 
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Install_Focus_Item)
					{
						case SCAN_ITEM_CONNECTION:
							hdc = BeginPaint(hwnd);

							// printf("SCAN_ITEM_CONNECTION: %d\n", u8Result_Value);

							/* By KB Kim 2011.01.17 */
							if (SearchData.ScanMode != u8Result_Value)
							{
								SearchData.ScanMode = u8Result_Value;							
								CS_DBU_SetAntenna_Type(SearchData.ScanMode);
							}
							MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
							MV_Draw_MenuBar(hdc, MV_UNFOCUS, SCAN_ITEM_DISECQ);
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
							
						case SCAN_ITEM_DISECQ:
							hdc = BeginPaint(hwnd);
							Mv_Install_Disecq = u8Result_Value;							
							MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC = Mv_Install_Disecq;
							MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
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
						case SCAN_ITEM_LNB_TYPE:
							hdc = BeginPaint(hwnd);
							Mv_Install_LNBType = u8Result_Value;
							MV_Change_LnbType(Mv_Install_LNBType);							
							MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
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
					MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
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
			
			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_Install_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_DOWN:
					
					if ( Sat_List_Status == TRUE )
					{
						MV_stSatInfo	Temp_SatData;
						MV_stTPInfo		Temp_TPData;
						U16				Temp_Count;

						hdc = BeginPaint(hwnd);
						if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						if ( u8SatList_Focus_Item >= (Temp_Count - 1) )
						{
							u8SatList_Focus_Item = 0;
							u8SatList_Start_Point = 0;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							if ( u8SatList_End_Point > Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;

							MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 9 )
						{
							u8SatList_Start_Point += LIST_ITEM_NUM;
							u8SatList_Focus_Item++;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;

							MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							u8SatList_Focus_Item++;

							if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
						MV_Draw_MenuBar(hdc, MV_UNFOCUS, Install_Focus_Item);

						if(Install_Focus_Item >= SCAN_ITEM_MAX - 1)
							Install_Focus_Item = SCAN_ITEM_CONNECTION;
						else if (Install_Focus_Item == SCAN_ITEM_LNB_TYPE)
							Install_Focus_Item = Install_Focus_Item + 3;
						else
							Install_Focus_Item++;

						MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
						EndPaint(hwnd,hdc);
						//printf("\n##### DOWN Install_Focus_Item = %d\n", Install_Focus_Item);

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
						if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
						{
							/* No Sat Data. Do nothing : by KB Kim 2011.01.13 */
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
							/* No Sat Data. Do nothing : by KB Kim 2011.01.13 */
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
							if (Temp_Count == 0)
							{
								break;
							}
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						if ( u8SatList_Focus_Item == 0 )
						{
							/* By KB Kim 2011.01.13 */
							u8SatList_Focus_Item = Temp_Count - 1;
							u8SatList_Start_Point = LIST_ITEM_NUM * (Temp_Count/LIST_ITEM_NUM);
							u8SatList_End_Point = Temp_Count - 1;

							MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
						{
							u8SatList_Start_Point = u8SatList_Focus_Item - LIST_ITEM_NUM;
							u8SatList_End_Point = u8SatList_Focus_Item;
							
							/* By KB Kim 2011.01.13 */
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = (Temp_Count -1);
							u8SatList_Focus_Item--;

							MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							u8SatList_Focus_Item--;
							if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
						MV_Draw_MenuBar(hdc, MV_UNFOCUS, Install_Focus_Item);
						
						if(Install_Focus_Item <= SCAN_ITEM_CONNECTION)
							Install_Focus_Item = SCAN_ITEM_MAX-1;
						else if (Install_Focus_Item == SCAN_ITEM_DISECQ)
							Install_Focus_Item = Install_Focus_Item - 3;
						else
							Install_Focus_Item--;

						MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
						EndPaint(hwnd,hdc);
						//printf("\n##### UP Install_Focus_Item = %d\n", Install_Focus_Item);
						
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_RED:
					if ( Sat_List_Status == FALSE && MV_GetSelected_SatData_Count() != 0 )
					{
						CSApp_Install_Applets=CSApp_Applet_Sat_Setting;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_GREEN:
					if ( Sat_List_Status == FALSE && MV_GetSelected_SatData_Count() != 0 )
					{
						CSApp_Install_Applets=CSApp_Applet_TP_Setting;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_YELLOW:
					if ( Sat_List_Status == TRUE )
					{
						if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
						{
							MV_stSatInfo	Temp_SatData;

							MV_GetSatelliteData_ByIndex(&Temp_SatData, u8SatList_Focus_Item);
							
							if ( Temp_SatData.u16Select == SAT_SELECT )
								Temp_SatData.u16Select = SAT_UNSELECT;
							else
								Temp_SatData.u16Select = SAT_SELECT;

							Flag_SatList_Change = TRUE;
							hdc = BeginPaint(hwnd);
							MV_SatList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM , u8SatList_Start_Point, &Temp_SatData, FOCUS);
							EndPaint(hwnd,hdc);
							MV_Sat_Data[u8SatList_Focus_Item] = Temp_SatData;
							MV_SetSatelliteData_By_SatIndex(Temp_SatData);

							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_DOWN, 0);
						}
					}
					break;

				case CSAPP_KEY_BLUE:
					/* 2010.09.04 By KB Kim */
					if ( Sat_List_Status == FALSE )
					{
						if ( MV_GetSelected_SatData_Count() > 0 )
						{
							Search_Condition_Focus_Item = SCAN_CON_ALL ;
							MV_Search_Condition(hwnd, 0);
							//CSApp_Install_Applets=CSAPP_Applet_Install_Result;
							//SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						} else {
							
						}
					}
					break;
						
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{
						U16		Temp_Count;

						/* 2010.09.04 By KB Kim */
						if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
							if ( (u8SatList_Focus_Item%LIST_ITEM_NUM) == 0 )
								u8SatList_Start_Point -= LIST_ITEM_NUM;
							else
								u8SatList_Start_Point = LIST_ITEM_NUM * (u8SatList_Focus_Item/LIST_ITEM_NUM);
						}

						/* 2010.09.04 By KB Kim */
						if ( (u8SatList_Start_Point + LIST_ITEM_NUM) >= Temp_Count )
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
						MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						switch(Install_Focus_Item)
						{
							case SCAN_ITEM_CONNECTION:
								hdc = BeginPaint(hwnd);

								if(SearchData.ScanMode==SCAN_MODE_SINGLE)
								{
									#ifdef UNICABLE_ON
									SearchData.ScanMode=SCAN_MODE_UNICABLE;
									#else
									SearchData.ScanMode=SCAN_MODE_USALS;
									#endif
								}
								else
									SearchData.ScanMode--;

								CS_DBU_SetAntenna_Type(SearchData.ScanMode);
								MV_Draw_InstallMenuBar(hdc);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;
								break;

							case SCAN_ITEM_SATELLITE:

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
								
								hdc = BeginPaint(hwnd);

								/* By KB Kim 2011.01.13 */
								u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								
								Mv_Install_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
								Mv_Install_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
								u8Glob_TP_Focus = 0;
								
								MV_Draw_InstallMenuBar(hdc);
								
								MV_Draw_Selected_Satlist(hdc);
								
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

							case SCAN_ITEM_TP_SELECT:
								if (u8TpCount == 0)
								{
									u8Glob_TP_Focus = 0;
									break;
								}

								hdc = BeginPaint(hwnd);

								/* By KB Kim 2011.01.13 */
								if ( u8Glob_TP_Focus == 0 )
								{
									/* By KB Kim 2011.01.13 */
									u8Glob_TP_Focus = u8TpCount - 1;
								}
								else
									u8Glob_TP_Focus--;
								
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;
								
							case SCAN_ITEM_LNB_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_Install_LNBType == 0 )
									Mv_Install_LNBType = MAX_LNBTYPE-1;
								else
									Mv_Install_LNBType--;

								MV_Change_LnbType(Mv_Install_LNBType);
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
								MV_Draw_MenuBar(hdc, MV_UNFOCUS, SCAN_ITEM_HIGH_FREQ);
								MV_Draw_MenuBar(hdc, MV_UNFOCUS, SCAN_ITEM_LOW_FREQ);
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

							case SCAN_ITEM_DISECQ:
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
								{									
									hdc = BeginPaint(hwnd);

									if ( Mv_Install_Disecq == 0 )
										Mv_Install_Disecq = MAX_DISECQ_PORT-1;
									else
										Mv_Install_Disecq--;

									MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC = Mv_Install_Disecq;
									MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
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

							case SCAN_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_Install_ScanType == MULTI_SAT )
									Mv_Install_ScanType = ONE_SAT;
								else
									Mv_Install_ScanType = MULTI_SAT;
								
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);

								MV_Draw_Selected_Satlist(hdc);
								
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
						if ( Install_Focus_Item == SCAN_ITEM_SATELLITE )
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
						MV_Install_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						switch(Install_Focus_Item)
						{
							case SCAN_ITEM_CONNECTION:	
								hdc = BeginPaint(hwnd);
								
								#ifdef UNICABLE_ON
								if(SearchData.ScanMode==SCAN_MODE_UNICABLE )
									SearchData.ScanMode=SCAN_MODE_SINGLE;
								#else
								if(SearchData.ScanMode==SCAN_MODE_USALS )
									SearchData.ScanMode=SCAN_MODE_SINGLE;
								#endif
								else
									SearchData.ScanMode++;

								CS_DBU_SetAntenna_Type(SearchData.ScanMode);
								MV_Draw_InstallMenuBar(hdc);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;
								break;

							case SCAN_ITEM_SATELLITE:
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

								/* By KB Kim 2011.01.13 */
								u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								
								Mv_Install_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
								Mv_Install_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
								u8Glob_TP_Focus = 0;
								
								MV_Draw_InstallMenuBar(hdc);

								MV_Draw_Selected_Satlist(hdc);
								
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

							case SCAN_ITEM_TP_SELECT:
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
								
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;
								
							case SCAN_ITEM_LNB_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_Install_LNBType == MAX_LNBTYPE - 1 )
									Mv_Install_LNBType = 0;
								else
									Mv_Install_LNBType++;
								
								MV_Change_LnbType(Mv_Install_LNBType);
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
								MV_Draw_MenuBar(hdc, MV_UNFOCUS, SCAN_ITEM_HIGH_FREQ);
								MV_Draw_MenuBar(hdc, MV_UNFOCUS, SCAN_ITEM_LOW_FREQ);
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

							case SCAN_ITEM_DISECQ:
								if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
								{
									hdc = BeginPaint(hwnd);

									if ( Mv_Install_Disecq == MAX_DISECQ_PORT - 1 )
										Mv_Install_Disecq = 0;
									else
										Mv_Install_Disecq++;

									MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC = Mv_Install_Disecq;
									MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
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

							case SCAN_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_Install_ScanType == MULTI_SAT )
									Mv_Install_ScanType = ONE_SAT;
								else
									Mv_Install_ScanType = MULTI_SAT;
								
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);

								MV_Draw_Selected_Satlist(hdc);
								
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
							MV_Install_List_Window_Close( hwnd );

							hdc = BeginPaint(hwnd);
							if (Install_Focus_Item == SCAN_ITEM_SATELLITE)
							{
								/* By KB Kim 2011.01.13 */
								Mv_SAT_SatFocus = u8Glob_Sat_Focus;
								if (u8Glob_Sat_Focus != u8SatList_Focus_Item) 
								{
									u8Glob_Sat_Focus = u8SatList_Focus_Item;
									u8Glob_TP_Focus = 0;

									/* By KB Kim 2011.01.13 */
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
									
									Mv_Install_LNBType = MV_Sat_Data[u8Glob_Sat_Focus].u8LNBType;
									Mv_Install_Disecq = MV_Sat_Data[u8Glob_Sat_Focus].u16DiSEqC;
									
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
									
									MV_Draw_InstallMenuBar(hdc);
								}
							}
							else
							{
								u8Glob_TP_Focus = u8SatList_Focus_Item;
								MV_Draw_MenuBar(hdc, MV_FOCUS, Install_Focus_Item);
							}

							if ( Flag_SatList_Change == TRUE )
								MV_Draw_Selected_Satlist(hdc);
							
							EndPaint(hwnd,hdc);
							
							Flag_SatList_Change = FALSE;			// Must after Upper Drawing Job ...
							
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
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
						} else if ( Search_Condition_Status == TRUE ) {
							Search_Condition_Status = FALSE;
							MV_Install_Warning_Window_Close( hwnd );
							set_prev_windown_status(CSApp_Applet_Install);
							CSApp_Install_Applets=CSAPP_Applet_Install_Result;
							
							if ( Mv_Install_ScanType == ONE_SAT )
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
							usleep(100);

							switch(Install_Focus_Item)
							{
								case SCAN_ITEM_CONNECTION:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										#ifdef UNICABLE_ON
										for ( i = 0 ; i < 5 ; i++ )
											sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Connect_Type[i]));
										#else
										for ( i = 0 ; i < 4 ; i++ )
											sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Connect_Type[i]));
										#endif
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Install_Focus_Item + 1);
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4 - 50);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 + 100;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										#ifdef UNICABLE_ON
										stContents.u8TotalCount = 5;
										#else
										stContents.u8TotalCount = 4;
										#endif
										stContents.u8Focus_Position = SearchData.ScanMode;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;
									
								case SCAN_ITEM_SATELLITE:
									{
										u8SatList_Focus_Item = u8Glob_Sat_Focus;
										u8SatList_Start_Point = ( u8SatList_Focus_Item / LIST_ITEM_NUM ) * LIST_ITEM_NUM;

										if ( u8SatList_Start_Point + LIST_ITEM_NUM >= MV_SAT_MAX)
											u8SatList_End_Point = MV_SAT_MAX -1;
										else
											u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
										
										MV_Install_List_Window( hwnd );
									} 
									break;
								case SCAN_ITEM_TP_SELECT:
									{
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
										
										MV_Install_List_Window( hwnd );
									}
									break;

								case SCAN_ITEM_LNB_TYPE:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										for ( i = 0 ; i < 22 ; i++ )
											sprintf(stContents.Contents[i], "%s", LNB_Type[i]);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Install_Focus_Item + 1);
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4 - 50);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 + 100;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 21;
										stContents.u8Focus_Position = Mv_Install_LNBType;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								case SCAN_ITEM_DISECQ:
									if ( SearchData.ScanMode == SCAN_MODE_DISECQ10 )
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										for ( i = 0 ; i < 22 ; i++ )
											sprintf(stContents.Contents[i], "%s", Diseqc_Port[i]);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Install_Focus_Item - 3 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 21;
										stContents.u8Focus_Position = Mv_Install_Disecq;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									} else {
										if ( MV_GetSelected_SatData_Count() > 0 )
										{
											switch ( SearchData.ScanMode )
											{
												case SCAN_MODE_DISECQ_MOTOR:
													CSApp_Install_Applets = CSAPP_Applet_D_motor;
													MV_SetSatelliteData(MV_Sat_Data);
													SendMessage (hwnd, MSG_CLOSE, 0, 0);
													break;
												case SCAN_MODE_USALS:
													CSApp_Install_Applets = CSAPP_Applet_USALS;
													MV_SetSatelliteData(MV_Sat_Data);
													SendMessage (hwnd, MSG_CLOSE, 0, 0);
													break;
												case SCAN_MODE_UNICABLE:
													CSApp_Install_Applets = CSAPP_Applet_UniCable;
													MV_SetSatelliteData(MV_Sat_Data);
													SendMessage (hwnd, MSG_CLOSE, 0, 0);
													break;
												default:
													break;
											}
										}else {
											hdc=BeginPaint(hwnd);
											MV_Draw_Msg_Window(hdc, CSAPP_STR_FIRST_SELECT);
											EndPaint(hwnd,hdc);
											
											usleep( 2000 * 1000 );
											
											hdc=BeginPaint(hwnd);
											Close_Msg_Window(hdc);
											EndPaint(hwnd,hdc);
										}
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
						MV_Install_List_Window_Close( hwnd );						

						if ( Flag_SatList_Change == TRUE )
						{	
							hdc = BeginPaint(hwnd);
							MV_Draw_Selected_Satlist(hdc);
							EndPaint(hwnd,hdc);
						}
						Flag_SatList_Change = FALSE;			// Must after Upper Drawing Job ...
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
						if ( MV_DB_GetALLServiceNumber() == 0 )
						{
							CSApp_Install_Applets=CSApp_Applet_MainMenu;
							MV_SetSatelliteData(MV_Sat_Data);
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
						else 
						{
							CSApp_Install_Applets=CSApp_Applet_Desktop;
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
						MV_Install_List_Window_Close( hwnd );						

						if ( Flag_SatList_Change == TRUE )
						{	
							hdc = BeginPaint(hwnd);
							MV_Draw_Selected_Satlist(hdc);
							EndPaint(hwnd,hdc);
						}
						Flag_SatList_Change = FALSE;			// Must after Upper Drawing Job ...
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
#ifdef SMART_PHONE
						if ( b8Last_App_Status == CSApp_Applet_Smart_OSD )
							CSApp_Install_Applets=b8Last_App_Status;
						else
#endif
						/* By KB Kim 2011.05.28 */
						{
							CSApp_Install_Applets=CSApp_Applet_MainMenu;
							if ( MV_DB_GetALLServiceNumber() > 0 )
							{
								CS_AV_VideoBlank();
							}
						}
						MV_SetSatelliteData(MV_Sat_Data);
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;
				default:					
					//NumberKeyFlag=FALSE;	
					break;
			}
			break;
			
		default:
			break;			
	}
	
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


#define NUMBER_OF_SERVICE_NAME_TO_SHOW          5

typedef struct
{
	U8		ServiceScramble;
	char 	ServiceName[MAX_SERVICE_NAME_LENGTH];
}ServiceInfo_ShowParams;

static S32  	searched_tvch_num = 0, searched_radioch_num = 0;
ServiceInfo_ShowParams 	TV_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW];
ServiceInfo_ShowParams 	Radio_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW];
tComboList_Element_Rect	tvname = {150,220, 200, 30};
tComboList_Element_Rect	radioname = {400, 220, 200, 30};
tComboList_Element_Rect	progress = {220, 160, 370, 10};

void INSTALL_CallBack( tCS_INSTALL_Notification notification )
{
	//HDC 		hdc;
	//HWND hwnd;
	//char buffer[30];

	//hwnd = GetActiveWindow();

	//printf("INSTALL_CallBack %d\n", notification);
	switch(notification)
	{
		case eCS_INSTALL_SERVICEINFO:
			{
				BroadcastMessage( MSG_SERVICEINFO_UPDATE, 0, 0);
			}
			break;
		case eCS_INSTALL_TPINFO:
			{
				BroadcastMessage( MSG_TPINFO_UPDATE, 0, 0);
				BroadcastMessage( MSG_SCANINFO_UPDATE, eCS_FE_NONE, 0);
			}
			break;
		case eCS_INSTALL_MAX_SERVICE_NUMBER_REACHED:
		case eCS_INSTALL_COMPLETE:
			{
				/*tCS_DBU_Service currentservice;

				CS_INSTALL_StopInstallation();
				CS_DBU_LoadCurrentService(&currentservice);

				CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
				CS_DB_SetCurrentService_OrderIndex(0);

				currentservice.sCS_DBU_ServiceIndex = 0;
				CS_DBU_SaveCurrentService(currentservice);

				CS_DB_SaveDatabase();*/
				BroadcastMessage (MSG_SEARCH_COMPLETE, 0, 0);
			}
			break;

		default:
			break;
	} 
		
}

static U8				u8TPData_Start;

void MV_Display_TPList(HDC hdc, MV_stTPInfo *Temp_TPDatas)
{
	int		i;
	char	buffer[30];
	RECT	lrect;
	
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	
	MV_FillBox( hdc, ScalerWidthPixel(RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_TOP), ScalerWidthPixel(RE_LIST2_RIGHT - RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_BOTTOM - RE_LIST2_TOP) );

	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( Temp_TPDatas[i].u8Polar_H == 1 )
			sprintf(buffer, "%d/%s/%d", Temp_TPDatas[i].u16TPFrequency, "H", Temp_TPDatas[i].u16SymbolRate);
		else
			sprintf(buffer, "%d/%s/%d", Temp_TPDatas[i].u16TPFrequency, "V", Temp_TPDatas[i].u16SymbolRate);

		if ( i == 0 )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST2_LEFT ), ScalerHeigthPixel( RE_LIST2_TOP ), ScalerWidthPixel( RE_LIST2_RIGHT - RE_LIST2_LEFT ), ScalerHeigthPixel( MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight ), &MV_BMP[MVBMP_CHLIST_SELBAR]);
		}
		else
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);			
		}

		lrect.left = RE_LIST2_LEFT+10;
		lrect.top = RE_LIST2_TOP + i*30 + 4;
		lrect.right = RE_LIST2_RIGHT - 10;
		lrect.bottom = lrect.top + 25;
		
		if ( Temp_TPDatas[i].u16TPFrequency != 0 )
		{
			//CS_MW_DrawText(hdc, buffer, -1, &lrect, DT_LEFT);
			MV_MW_DrawText_Fixed_Small( hdc, buffer, -1, &lrect, DT_LEFT );
		}
			//CS_MW_TextOut(hdc,ScalerWidthPixel(lrect.left),ScalerHeigthPixel(), buffer);
	}
}

/* For More than 5 Satellite */
void MV_Display_SatList(HDC hdc, U8 u8Search_Sat_Index)
{
	int				i;
	int             j = 0;
	MV_stSatInfo	Temp_SatData;
	RECT			lrect;
	
	if (u8Multi_Select_Sat > 5)
	{
		for (i = 0 ; i < u8Multi_Select_Sat ; i ++)
		{
			if ( Search_Condition_Sat_Index[i] != 0xFF )
			{
				if ( u8Search_Sat_Index == Search_Condition_Sat_Index[i] )
				{
					if (i >= 5)
					{
						j = i - 4;
					}
					else
					{
						j = 0;
					}

					break;
				}
			}
		}
	}
	else
	{
		j = 0;
	}
	
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	
	MV_FillBox( hdc, ScalerWidthPixel(RE_LIST1_LEFT), ScalerHeigthPixel(RE_LIST1_TOP), ScalerWidthPixel(RE_LIST1_RIGHT - RE_LIST1_LEFT), ScalerHeigthPixel(RE_LIST1_BOTTOM - RE_LIST1_TOP) );

	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( Search_Condition_Sat_Index[i + j] != 0xFF )
		{
			/* by KB Kim 2011.01.15 */
			// if ( u8Search_Sat_Index == i )
			if ( u8Search_Sat_Index == Search_Condition_Sat_Index[i+ j] )
				FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST1_LEFT ), ScalerHeigthPixel( RE_LIST1_TOP + ( i * MV_INSTALL_MENU_BAR_H ) ), ScalerWidthPixel( RE_LIST1_RIGHT - RE_LIST1_LEFT ), ScalerHeigthPixel( MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight ), &MV_BMP[MVBMP_CHLIST_SELBAR]);
			
			MV_GetSatelliteData_ByIndex(&Temp_SatData, MV_DB_Get_SatIndex_By_Satindex(Search_Condition_Sat_Index[i + j]));

			lrect.left = RE_LIST1_LEFT+10;
			lrect.top = (RE_LIST3_TOP + ( i * MV_INSTALL_MENU_BAR_H )) + 4;
			lrect.right = RE_LIST1_RIGHT - 10;
			lrect.bottom = lrect.top + 25;
			//CS_MW_DrawText(hdc, Temp_SatData.acSatelliteName, -1, &lrect, DT_LEFT);
			MV_MW_DrawText_Fixed_Small( hdc, Temp_SatData.acSatelliteName, -1, &lrect, DT_LEFT );
			//CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST1_LEFT+10),ScalerHeigthPixel((RE_LIST3_TOP + ( i * MV_INSTALL_MENU_BAR_H )) + 4), Temp_SatData.acSatelliteName);
		} else 
			break;
	}
}

void Show_Scan_Animation(HWND hwnd)
{
	static U8		Animation_Count;
	HDC				hdc;

	hdc =BeginPaint(hwnd);
	
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(RE_LIST4_LEFT + 50), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(MV_BMP[MVBMP_SCAN_ANI1].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_SCAN_ANI1].bmHeight) );

	switch(Animation_Count)
	{
		case 0:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST4_LEFT + 50 ), ScalerHeigthPixel( RE_INFO_TOP ), ScalerWidthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmHeight ), &MV_BMP[MVBMP_SCAN_ANI1]);
			Animation_Count = 1;
			break;
		case 1:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST4_LEFT + 50 ), ScalerHeigthPixel( RE_INFO_TOP ), ScalerWidthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmHeight ), &MV_BMP[MVBMP_SCAN_ANI2]);
			Animation_Count = 2;
			break;
		case 2:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST4_LEFT + 50 ), ScalerHeigthPixel( RE_INFO_TOP ), ScalerWidthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmHeight ), &MV_BMP[MVBMP_SCAN_ANI3]);
			Animation_Count = 3;
			break;
		case 3:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST4_LEFT + 50 ), ScalerHeigthPixel( RE_INFO_TOP ), ScalerWidthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmHeight ), &MV_BMP[MVBMP_SCAN_ANI4]);
			Animation_Count = 4;
			break;
		case 4:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( RE_LIST4_LEFT + 50 ), ScalerHeigthPixel( RE_INFO_TOP ), ScalerWidthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_SCAN_ANI1].bmHeight ), &MV_BMP[MVBMP_SCAN_ANI5]);
			Animation_Count = 0;
			break;
		default:
			break;
	}
	EndPaint(hwnd,hdc);
}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL SetNextBlindSearch(void)
{
	if (SearchBlindModeOn)
	{
		if (BlindScanParam.PolarH)
		{
			BlindScanParam.PolarH = 0;
			if (BlindLnbMode[CurrentBlindScanSatIndex])
			{
				if (BlindScanParam.LnbHi)
				{
					BlindScanParam.LnbHi = 0;
					CurrentBlindScanSatIndex++;
					if (CurrentBlindScanSatIndex < u8Multi_Select_Sat)
					{
						BlindScanParam.SatIndex = Search_Condition_Sat_Index[CurrentBlindScanSatIndex];
					}
					else
					{
						return FALSE;
					}
				}
				else
				{
					BlindScanParam.LnbHi = 1;
				}
			}
			else
			{
				CurrentBlindScanSatIndex++;
				BlindScanParam.LnbHi = 0;
				if (CurrentBlindScanSatIndex < u8Multi_Select_Sat)
				{
					BlindScanParam.SatIndex = Search_Condition_Sat_Index[CurrentBlindScanSatIndex];
				}
				else
				{
					return FALSE;
				}
			}
		}
		else
		{
			/* Change */
			BlindScanParam.PolarH = 1;
		}
	}

	printf ("SetNextBlindSearch [%d - %d]: Pol [%d] , LnbHi[%d], Univesal[%d]\n", CurrentBlindScanSatIndex, BlindScanParam.SatIndex, BlindScanParam.PolarH, BlindScanParam.LnbHi, BlindLnbMode[CurrentBlindScanSatIndex]);

	return TRUE;
}

/* For Blind Scan By KB Kim 2011.02.28 */
void GetDisplayPercentRange (U8 *start, U8 *range)
{
	U16 u16Val;
	U16 u16Range;
	U16 max;
	
	if (SearchBlindModeOn)
	{
		u16Val = CurrentBlindScanSatIndex * 100;

		if (BlindLnbMode[CurrentBlindScanSatIndex])
		{
			if (BlindScanParam.PolarH)
			{
				u16Val += 25;
			}
			
			if (BlindScanParam.LnbHi)
			{
				u16Val += 50;
			}
			
			max = u16Val + 25;
		}
		else
		{
			if (BlindScanParam.PolarH)
			{
				u16Val += 50;
			}
			
			max = u16Val + 50;
		}

		max      = max / u8Multi_Select_Sat;
		u16Val   = u16Val / u8Multi_Select_Sat;
		u16Range = max - u16Val;
	}
	else
	{
		u16Val   = 0;
		u16Range = 100;
	}

	*start = (U8)u16Val;
	*range = (U8)u16Range;
}

static int SearchResult_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC						hdc;
	U16						i;
	U8                      blindProgress;
	char					buffer[30];
	MV_stTPInfo 			Temp_TPDatas[5];
	
	switch(message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			searched_tvch_num = 0;
			searched_radioch_num = 0;
			u8TPData_Start = 0;
			memset(TV_Service, 0, NUMBER_OF_SERVICE_NAME_TO_SHOW*sizeof(ServiceInfo_ShowParams));
			memset(Radio_Service, 0, NUMBER_OF_SERVICE_NAME_TO_SHOW*sizeof(ServiceInfo_ShowParams));

			for(i=0;i<NUMBER_OF_SERVICE_NAME_TO_SHOW;i++)
			{
				strcpy(TV_Service[i].ServiceName, " ");
				strcpy(Radio_Service[i].ServiceName, " ");
			}

			/* For Blind Scan By KB Kim 2011.02.26 */
			BlindScanParam.SatIndex = 0;
			BlindScanParam.PolarH   = 0;
			BlindScanParam.LnbHi    = 0;
			if ((Search_Condition_Focus_Item == SCAN_CON_ALL_BLIND) || (Search_Condition_Focus_Item == SCAN_CON_FTA_BLIND))
			{
				SearchBlindModeOn = 1;
				BlindTpSearch     = 0;
				CurrentBlindScanSatIndex = 0;
				MV_INSTALL_Init_Blind_TPdata(u8Multi_Select_Sat);
			}
			else
			{
				SearchBlindModeOn = 0;
				CS_INSTALL_Read_TPdata(Search_Condition_Sat_Index, u8Multi_Select_Sat, u8TPScan_Select_TP);
			}

			CS_INSTALL_Register_Notify(&Install_client, INSTALL_CallBack);
			break;
		
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, INSTALL_SEARCH);
			
			hdc =BeginPaint(hwnd);

			MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(RE_LIST1_LEFT), ScalerHeigthPixel(RE_LIST1_TOP), ScalerWidthPixel(RE_LIST1_RIGHT - RE_LIST1_LEFT), ScalerHeigthPixel(RE_LIST1_BOTTOM - RE_LIST1_TOP) );
			MV_FillBox( hdc, ScalerWidthPixel(RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_TOP), ScalerWidthPixel(RE_LIST2_RIGHT - RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_BOTTOM - RE_LIST2_TOP) );
			MV_FillBox( hdc, ScalerWidthPixel(RE_LIST3_LEFT), ScalerHeigthPixel(RE_LIST3_TOP), ScalerWidthPixel(RE_LIST3_RIGHT - RE_LIST3_LEFT), ScalerHeigthPixel(RE_LIST3_BOTTOM - RE_LIST3_TOP) );
			MV_FillBox( hdc, ScalerWidthPixel(RE_LIST4_LEFT), ScalerHeigthPixel(RE_LIST4_TOP), ScalerWidthPixel(RE_LIST4_RIGHT - RE_LIST4_LEFT), ScalerHeigthPixel(RE_LIST4_BOTTOM - RE_LIST4_TOP) );
			MV_FillBox( hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_BOTTOM - RE_INFO_TOP) );

			if(SearchData.ScanMode==SCAN_MODE_SINGLE)
				SearchData.StrIdx=CSAPP_STR_SEARCH_MANUAL;
			else
				SearchData.StrIdx=CSAPP_STR_SEARCH_AUTO;

			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			/* by KB Kim 2011.01.15 */ 
			u16Search_Current_Sat_Index = Search_Condition_Sat_Index[0];
			MV_Display_SatList(hdc, Search_Condition_Sat_Index[0]);
			
			memset(Temp_TPDatas, 0x00, 5*sizeof(MV_stTPInfo));

			/* For TP Data Sync By KB KIm 2011.03.03 */
#if 0
			if ( u8Multi_Select_Sat != 0 )
				CS_INSTALL_Get_TPdata(Temp_TPDatas, MV_DB_Get_SatIndex_By_Satindex(Search_Condition_Sat_Index[0]), u8TPData_Start);
			else  
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPDatas[0], Search_Condition_Sat_Index[0], u8TPScan_Select_TP);
#else
			CS_INSTALL_Get_TPdata(Temp_TPDatas);
#endif
			MV_Display_TPList(hdc, Temp_TPDatas);
			
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST1_LEFT),ScalerHeigthPixel(RE_LIST1_TOP - 30), CS_MW_LoadStringByIdx(CSAPP_STR_SATELLITE));
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST2_LEFT),ScalerHeigthPixel(RE_LIST2_TOP - 30), CS_MW_LoadStringByIdx(CSAPP_STR_TP));
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST3_LEFT),ScalerHeigthPixel(RE_LIST3_TOP - 30), CS_MW_LoadStringByIdx(CSAPP_STR_TV));
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST4_LEFT),ScalerHeigthPixel(RE_LIST4_TOP - 30), CS_MW_LoadStringByIdx(CSAPP_STR_RADIO));
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST3_LEFT),ScalerHeigthPixel(RE_LIST3_BOTTOM + 10), CS_MW_LoadStringByIdx(CSAPP_STR_SEARCH_TV_NUM));
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST4_LEFT),ScalerHeigthPixel(RE_LIST4_BOTTOM + 10), CS_MW_LoadStringByIdx(CSAPP_STR_SEARCH_RD_NUM));
			
			sprintf(buffer, "%d", searched_tvch_num);
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST3_LEFT + 260),ScalerHeigthPixel(RE_LIST3_BOTTOM + 10), buffer);

			sprintf(buffer, "%d", searched_radioch_num);
			CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST4_LEFT + 260),ScalerHeigthPixel(RE_LIST4_BOTTOM + 10), buffer);
			
			Draw_Progress_Bar(hdc, 0);

			/* For Blind Scan By KB Kim 2011.02.26 */
			if (SearchBlindModeOn)
			{
				MV_FE_SetBlindProcess(0);
				blindProgress = 0;
				Draw_Blind_Progress_Bar(hdc, 0);
				CurrentBlindProcess = 0;
				CurrentBlindTpTotal = 0;
				BlindScanParam.SatIndex = Search_Condition_Sat_Index[CurrentBlindScanSatIndex];
				memset(BlindLnbMode, 0x00, MAX_MULTI_SAT);
				MvGetLnbMode(Search_Condition_Sat_Index, u8Multi_Select_Sat, BlindLnbMode);
				MvInstallSetBlind((U8)Search_Condition_Focus_Item);
				MvInstallStartBlind(BlindScanParam);
				BlindTpSearch     = 1;
			}
			else
			{
			/* 2010.09.04 By KB Kim */
			CS_INSTALL_StartInstallation((U8)Search_Condition_Focus_Item);
			}
			EndPaint(hwnd,hdc);

			return 0;
			break;

		case MSG_TIMER:
			if (SearchBlindModeOn && BlindTpSearch)
			{
				blindProgress = MV_FE_GetBlindProcess();
				if (blindProgress != CurrentBlindProcess)
				{
					hdc =BeginPaint(hwnd);
					Draw_Blind_Progress_Bar(hdc, blindProgress);
					EndPaint(hwnd,hdc);
					CurrentBlindProcess = blindProgress;
				}
			}
			Show_Scan_Animation(hwnd);
			break;
			
		case MSG_KEYDOWN:
#ifdef NEW_INSTALL
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					tCS_DBU_Service 			currentservice;
					U16							firstservice;
					tCS_DB_ServiceManageData	tempdata;
					U16							i = 0;
					U16							num = 0;
					
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_SAVING);
							EndPaint(hwnd,hdc);

							/* For First search channel By KB Kim 2011.01.19 */
							MV_DB_Add_Temp_Service(&firstservice);
							
							currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
							currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
							CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
							/* For First search channel By KB Kim 2011.01.19 */
							// firstservice = CS_INSTALL_GetFirstInstallService();
							if(firstservice != MV_DB_INVALID_SERVICE_INDEX)
							{
								MV_stServiceInfo  tservice_data;

								/* By KB Kim for Multi-Recall list problem : 2011.08.30 */
								CS_DB_SetLastServiceTriplet();

								MV_DB_GetServiceDataByIndex(&tservice_data, firstservice);

								if(tservice_data.u8TvRadio== eCS_DB_RADIO_SERVICE)
								{
									currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
									currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
									CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, FALSE);
								}
								else
								{
									currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
									currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
									CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, FALSE);
								}

								num = CS_DB_GetCurrentList_ServiceNum();

								for(i=0;i<num;i++)
								{
									CS_DB_GetCurrentList_ServiceData(&tempdata, i);
									if(tempdata.Service_Index == firstservice)
									{
										break;
									}
								}

								CS_DB_SetCurrentService_OrderIndex(i);
								currentservice.sCS_DBU_ServiceIndex = i;

								/* By KB Kim for Recall List problem : 2011.08.30*/
								CS_DBU_SaveCurrentService(currentservice);
							}
							else
							{
								CS_DBU_LoadCurrentService(&currentservice);

								/* By KB Kim 2011.01.13 */
								if ( MV_DB_GetALLServiceNumber() > 0 )
								{
									CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
									if(CS_DB_SetCurrentService_OrderIndex(currentservice.sCS_DBU_ServiceIndex)==eCS_DB_ERROR)
									{
										CS_DB_SetCurrentService_OrderIndex(0);
										currentservice.sCS_DBU_ServiceIndex = 0;

										/* By KB Kim for Recall List problem : 2011.08.30*/
										CS_DBU_SaveCurrentService(currentservice);
									}
								}
								else
								{
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
									CSApp_SearchResult_Applets = get_prev_windown_status();
									hdc=BeginPaint(hwnd);
									Close_Msg_Window(hdc);
									EndPaint(hwnd,hdc);

									/* By KB Kim 2011.01.13 */
									PostMessage (hwnd, MSG_CLOSE, 0, 0);
									break;
								}
							}

							/* By KB Kim for Recall List problem : 2011.08.30*/
							// CS_DBU_SaveCurrentService(currentservice);
							CS_DB_SaveDatabase();
							CSApp_SearchResult_Applets = CSApp_Applet_Desktop;

							/* By KB Kim 2011.05.28 */
							CS_AV_VideoBlank();
							CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
#if 0 /* No need this, "CS_MW_PlayServiceByIdx" will do, By KB Kim 2011.01.13 */
							u8Temp_Sat_Index = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
							u16Temp_TP_Index = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Temp_Sat_Index, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
							MV_GetSatelliteData_ByIndex(&tTemp_Sat_Data, u8Temp_Sat_Index);
							//printf("SEARCH COMPLETE ========= %d : %s , %d ===========\n", u8Temp_Sat_Index, tTemp_Sat_Data.acSatelliteName, u16Temp_TP_Index);
							MV_FE_SetTuning_TP(&tTemp_Sat_Data, u8Temp_Sat_Index, u16Temp_TP_Index);
#endif

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);

							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						} else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							
							/* By KB Kim 2011.01.18 */
							MV_DB_Reset_Temp_AddService();
							CSApp_SearchResult_Applets = get_prev_windown_status();
							// printf("\n======= No CSApp_SearchResult_Applets : %d ===============\n", CSApp_SearchResult_Applets);

							CS_DBU_LoadCurrentService(&currentservice);

							CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
							if(CS_DB_SetCurrentService_OrderIndex(currentservice.sCS_DBU_ServiceIndex)==eCS_DB_ERROR)
							{
								CS_DB_SetCurrentService_OrderIndex(0);
								currentservice.sCS_DBU_ServiceIndex = 0;

								/* By KB Kim for Recall List problem : 2011.08.30*/
								CS_DBU_SaveCurrentService(currentservice);
							}

							/* By KB Kim for Recall List problem : 2011.08.30*/
							// CS_DBU_SaveCurrentService(currentservice);
							
							//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
#if 0 /* By KB Kim 2011.01.13 */
							u8Temp_Sat_Index = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
							u16Temp_TP_Index = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Temp_Sat_Index, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
							MV_GetSatelliteData_ByIndex(&tTemp_Sat_Data, u8Temp_Sat_Index);
							MV_FE_SetTuning_TP(&tTemp_Sat_Data, u8Temp_Sat_Index, u16Temp_TP_Index);
#endif							
							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);

							PostMessage (hwnd, MSG_CLOSE, 0, 0);
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						
						/* By KB Kim 2011.01.18 */
						MV_DB_Reset_Temp_AddService();
						CSApp_SearchResult_Applets = get_prev_windown_status();
						// printf("\n======= EXIT MENU CSApp_SearchResult_Applets : %d ===============\n", CSApp_SearchResult_Applets);

						CS_DBU_LoadCurrentService(&currentservice);

						CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
						if(CS_DB_SetCurrentService_OrderIndex(currentservice.sCS_DBU_ServiceIndex)==eCS_DB_ERROR)
						{
							CS_DB_SetCurrentService_OrderIndex(0);
							currentservice.sCS_DBU_ServiceIndex = 0;
							
							/* By KB Kim for Recall List problem : 2011.08.30*/
							CS_DBU_SaveCurrentService(currentservice);
						}

						/* By KB Kim for Recall List problem : 2011.08.30*/
						// CS_DBU_SaveCurrentService(currentservice);
						
						//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
#if 0 /* By KB Kim 2011.01.13 */
						u8Temp_Sat_Index = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
						u16Temp_TP_Index = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Temp_Sat_Index, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
						MV_GetSatelliteData_ByIndex(&tTemp_Sat_Data, u8Temp_Sat_Index);
						MV_FE_SetTuning_TP(&tTemp_Sat_Data, u8Temp_Sat_Index, u16Temp_TP_Index);
#endif							
							
						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
						
						PostMessage (hwnd, MSG_CLOSE, 0, 0);
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
				break;
			}
			}
#endif // #ifdef NEW_INSTALL
			switch(wparam)
			{
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					/* For Blind Scan By KB Kim 2011.02.28 */
					SearchBlindModeOn = 0;
					BlindTpSearch     = 0;
					CSApp_SearchResult_Applets = CSApp_Applet_MainMenu;
					PostMessage (hwnd, MSG_SEARCH_COMPLETE, 0, 0);
					break;

				case CSAPP_KEY_IDLE:
					/* For Blind Scan By KB Kim 2011.02.28 */
					SearchBlindModeOn = 0;
					BlindTpSearch     = 0;
					CSApp_SearchResult_Applets = CSApp_Applet_Sleep;
					PostMessage (hwnd, MSG_SEARCH_COMPLETE, 0, 0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				default:
					break;
			}		   	
			break;

		case MSG_SERVICEINFO_UPDATE:
			{
				tMV_Display_ServiceInfo		*serviceitem;
				U16     					i, j=0;
				RECT						Svc_Rect;

				serviceitem = MV_INSTALL_GetServiceList();

				//printf("======= DREAM_FONT : %x, %s\n", *(serviceitem[j].acServiceNames), serviceitem[j].acServiceNames);
							
				while(serviceitem[j].acServiceNames[0] != 0x00)
				{					
					hdc =BeginPaint(hwnd);
					if(serviceitem[j].u8ServiceType == eCS_DB_TV_SERVICE || serviceitem[j].u8ServiceType == eCS_DB_HDTV_SERVICE)
					{
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_LIST3_LEFT), ScalerHeigthPixel(RE_LIST3_TOP), ScalerWidthPixel(RE_LIST3_RIGHT - RE_LIST3_LEFT), ScalerHeigthPixel(RE_LIST3_BOTTOM - RE_LIST3_TOP) );
						if(searched_tvch_num < NUMBER_OF_SERVICE_NAME_TO_SHOW)
						{
							TV_Service[searched_tvch_num].ServiceScramble = serviceitem[j].u8ServiceScramble;
							memcpy(TV_Service[searched_tvch_num].ServiceName, serviceitem[j].acServiceNames, MAX_SERVICE_NAME_LENGTH);
						}
						else
						{
							memcpy(&TV_Service[0], &TV_Service[1], (NUMBER_OF_SERVICE_NAME_TO_SHOW-1)*sizeof(ServiceInfo_ShowParams));
							TV_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW-1].ServiceScramble = serviceitem[j].u8ServiceScramble;
							memcpy(TV_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW-1].ServiceName, serviceitem[j].acServiceNames, MAX_SERVICE_NAME_LENGTH);
						}

						for(i = 0; i<NUMBER_OF_SERVICE_NAME_TO_SHOW; i++)
						{
							SetBkMode(hdc,BM_TRANSPARENT);
							memset(buffer, 0, 30);

							if(TV_Service[i].ServiceScramble == 0)
							{
								SetTextColor(hdc,CSAPP_WHITE_COLOR);
								sprintf(buffer, "%s", TV_Service[i].ServiceName);
							}
							else
							{
								SetTextColor(hdc,COLOR_yellow);								
								sprintf(buffer, "%s %s", "$", TV_Service[i].ServiceName);
							}

							//CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST3_LEFT+10),ScalerHeigthPixel(RE_LIST3_TOP + 4 + 30*i), buffer);
							Svc_Rect.left = RE_LIST3_LEFT+10;
							Svc_Rect.right = RE_LIST3_RIGHT;
							Svc_Rect.top = (RE_LIST3_TOP + 4 + 30*i);
							Svc_Rect.bottom = Svc_Rect.top + MV_INSTALL_MENU_BAR_H;
							MV_MW_DrawText_Fixed_Small( hdc, buffer, -1, &Svc_Rect, DT_LEFT );
						}

						searched_tvch_num++;                      

					}
					else if(serviceitem[j].u8ServiceType == eCS_DB_RADIO_SERVICE)
					{
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_LIST4_LEFT), ScalerHeigthPixel(RE_LIST4_TOP), ScalerWidthPixel(RE_LIST4_RIGHT - RE_LIST4_LEFT), ScalerHeigthPixel(RE_LIST4_BOTTOM - RE_LIST4_TOP) );
						if(searched_radioch_num < NUMBER_OF_SERVICE_NAME_TO_SHOW)
						{
							Radio_Service[searched_radioch_num].ServiceScramble = serviceitem[j].u8ServiceScramble;
							memcpy(Radio_Service[searched_radioch_num].ServiceName, serviceitem[j].acServiceNames, MAX_SERVICE_NAME_LENGTH);
						}
						else
						{
							memcpy(&Radio_Service[0], &Radio_Service[1], (NUMBER_OF_SERVICE_NAME_TO_SHOW-1)*sizeof(ServiceInfo_ShowParams));
							Radio_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW-1].ServiceScramble = serviceitem[j].u8ServiceScramble;
							memcpy(Radio_Service[NUMBER_OF_SERVICE_NAME_TO_SHOW-1].ServiceName, serviceitem[j].acServiceNames, MAX_SERVICE_NAME_LENGTH);
						}

						for(i = 0; i<NUMBER_OF_SERVICE_NAME_TO_SHOW; i++)
						{
							SetBkMode(hdc,BM_TRANSPARENT);
							memset(buffer, 0, 30);

							if(Radio_Service[i].ServiceScramble == 0)
							{
								SetTextColor(hdc,CSAPP_WHITE_COLOR);
								sprintf(buffer, "%s", Radio_Service[i].ServiceName);
							}
							else
							{
								SetTextColor(hdc,COLOR_yellow);
								sprintf(buffer, "%s %s", "$", Radio_Service[i].ServiceName);
							}

							//CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST4_LEFT + 10),ScalerHeigthPixel(RE_LIST4_TOP + 4 + 30*i), buffer);
							Svc_Rect.left = RE_LIST4_LEFT+10;
							Svc_Rect.right = RE_LIST4_RIGHT;
							Svc_Rect.top = (RE_LIST4_TOP + 4 + 30*i);
							Svc_Rect.bottom = Svc_Rect.top + MV_INSTALL_MENU_BAR_H;
							MV_MW_DrawText_Fixed_Small( hdc, buffer, -1, &Svc_Rect, DT_LEFT );
						}

						searched_radioch_num++;

					}

					SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
					FillBox(hdc,ScalerWidthPixel(RE_LIST3_LEFT + 220),ScalerHeigthPixel(RE_LIST3_BOTTOM + 10),ScalerWidthPixel(100),ScalerHeigthPixel(24));
					FillBox(hdc,ScalerWidthPixel(RE_LIST4_LEFT + 220),ScalerHeigthPixel(RE_LIST4_BOTTOM + 10),ScalerWidthPixel(100),ScalerHeigthPixel(24));

					SetBkMode(hdc,BM_TRANSPARENT);
					SetTextColor(hdc,CSAPP_WHITE_COLOR);

					sprintf(buffer, "%d", searched_tvch_num);
					CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST3_LEFT + 260),ScalerHeigthPixel(RE_LIST3_BOTTOM + 10), buffer);

					sprintf(buffer, "%d", searched_radioch_num);
					CS_MW_TextOut(hdc,ScalerWidthPixel(RE_LIST4_LEFT + 260),ScalerHeigthPixel(RE_LIST4_BOTTOM + 10), buffer);

					j++;
					EndPaint(hwnd,hdc);
				}
			}
			break;

		case MSG_TPINFO_UPDATE:
			{
				U16         		num_installed = 0;
				U16					current_install_tp = 0;
				U16					current_install_sat = 0;
				U16         		total = 0;
				/* For Blind Scan By KB Kim 2011.02.28 */
				U8                  progress;
				U8                  minPercent;
				U8                  range;
				// U16					get_tp_num = 0; /* For TP Data Sync By KB KIm 2011.03.03 */
				// U16					u16Temp_TpIndex[5];

				num_installed = CS_INSTALL_GetNumOfTPInstalled();
				current_install_tp = CS_INSTALL_GetIndexOfTPInstalled();
				total= CS_INSTALL_GetTPListNum();
				/* For TP Data Sync By KB KIm 2011.03.03 */
				// get_tp_num = CS_INSTALL_Get_Num_Of_TP_BySat(Search_Condition_Sat_Index, num_installed);

				hdc =BeginPaint(hwnd);
				memset(Temp_TPDatas, 0, 5*sizeof(MV_stTPInfo));
				
				current_install_sat = MV_DB_Get_SatIndex_By_TPindex(current_install_tp);
				
#if 0 /* For TP Data Sync By KB KIm 2011.03.03 */
				if ( u8Multi_Select_Sat != 0 )
					CS_INSTALL_Get_TPdata(Temp_TPDatas, current_install_sat, get_tp_num);
				else  
				{
					if ( total > 1 )
					{
						int		i = 0;
						memset( u16Temp_TpIndex, 0xFF, 5*sizeof(U16));

						if ( num_installed <= total )
						{
							CS_INSTALL_Get_AddTP_Data( u16Temp_TpIndex, num_installed );
							for ( i = 0 ; i < 5 ; i++ )
							{
								if ( u16Temp_TpIndex[i] == 0xFFFF )
									break;
								else
									MV_GetTPData_By_TPIndex(&Temp_TPDatas[i], u16Temp_TpIndex[i]);
							}						
						}
					}
					else
						MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPDatas[0], Search_Condition_Sat_Index[0], u8TPScan_Select_TP);
				}
#else
				CS_INSTALL_Get_TPdata(Temp_TPDatas);
#endif

				if ( current_install_sat != u16Search_Current_Sat_Index )
					MV_Display_SatList(hdc, current_install_sat);
				
				u16Search_Current_Sat_Index = current_install_sat;
				MV_Display_TPList(hdc, Temp_TPDatas);

				MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);

				/* For Blind Scan By KB Kim 2011.02.28 */
				num_installed ++;
				if (num_installed > total)
				{
					num_installed = total;
				}

				if (SearchBlindModeOn)
				{
					sprintf(buffer, "%d", num_installed);
					GetDisplayPercentRange (&minPercent, &range);
					if (total > CurrentBlindTpTotal)
					{
						if (num_installed <= CurrentBlindTpTotal)
						{
							progress = minPercent;
						}
						else
						{
							progress = minPercent + (U8)(((num_installed - CurrentBlindTpTotal) * (U16)range) / (total - CurrentBlindTpTotal));
						}
					}
					else
					{
						progress = minPercent + range;
					}
				}
				else
				{
				sprintf(buffer, "%d/%d", num_installed, total);
					progress = (U8)((num_installed * 100) / total);
				}

				MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,CSAPP_WHITE_COLOR);
				FillBox(hdc,ScalerWidthPixel(RE_PROG_RIGHT - 100),ScalerHeigthPixel(RE_PROG_TOP),ScalerWidthPixel(120),ScalerHeigthPixel(30));
				CS_MW_TextOut(hdc,ScalerWidthPixel(RE_PROG_RIGHT - 100),ScalerHeigthPixel(RE_PROG_TOP + 4), buffer);

				if ( total != 0 )
					Draw_Progress_Bar(hdc, progress);

				EndPaint(hwnd,hdc);
			}
			break;
			
		case MSG_SCANINFO_UPDATE:
			{
				char			Temp_Str[128];
				U16         	num_installed = 0;
				U16				current_install_tp = 0;
				U16				current_install_sat = 0;
				U16				get_tp_num = 0;
				U16        		total = 0;
				/* For Blind Scan By KB Kim 2011.02.28 */
				// U16				u16Temp_TpIndex[5];

				memset( Temp_Str , 0x00 , 128 );

				num_installed = CS_INSTALL_GetNumOfTPInstalled();
				current_install_tp = CS_INSTALL_GetIndexOfTPInstalled();
				get_tp_num = CS_INSTALL_Get_Num_Of_TP_BySat(Search_Condition_Sat_Index, num_installed);
				total= CS_INSTALL_GetTPListNum();
				
				current_install_sat = MV_DB_Get_SatIndex_By_TPindex(current_install_tp);
#if 0 /* For TP Data Sync By KB KIm 2011.03.03 */
				if ( u8Multi_Select_Sat != 0 )
				{
					if ( wparam == eCS_FE_UNLOCKED && get_tp_num > 0 )
						MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Display_TPDatas, current_install_sat, get_tp_num - 1);
					else
						MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Display_TPDatas, current_install_sat, get_tp_num);
				}
				else  
				{
					if ( total > 1 )
					{
						memset( u16Temp_TpIndex, 0xFFFF, 5*sizeof(U16));

						if ( num_installed <= total )
						{
							CS_INSTALL_Get_AddTP_Data( u16Temp_TpIndex, num_installed );
							MV_GetTPData_By_TPIndex(&Display_TPDatas, u16Temp_TpIndex[0]);
						}
					}
					else
					{
						if ( wparam == eCS_FE_UNLOCKED && u8TPScan_Select_TP > 0 )
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Display_TPDatas, Search_Condition_Sat_Index[0], u8TPScan_Select_TP - 1);
						else
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Display_TPDatas, Search_Condition_Sat_Index[0], u8TPScan_Select_TP);
					}
				}
#else
				MV_GetTPData_By_TPIndex(&Display_TPDatas, current_install_tp);
#endif

				hdc =BeginPaint(hwnd);				

				if ( num_installed < total )
				{
					if ( wparam == eCS_FE_NONE )
					{
						MV_GetBitmapFromDC(hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_BOTTOM - RE_INFO_TOP - MV_INSTALL_MENU_BAR_H), &Capture_bmp);
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_BOTTOM - RE_INFO_TOP) );

						if ( Display_TPDatas.u8Polar_H == TRUE )
							sprintf(Temp_Str, "%d / H / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_SCANING));
						else
							sprintf(Temp_Str, "%d / V / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_SCANING));
			
						SetBkMode(hdc,BM_TRANSPARENT);
						SetTextColor(hdc,CSAPP_WHITE_COLOR);
						CS_MW_TextOut(hdc,ScalerWidthPixel(RE_INFO_LEFT + 20),ScalerHeigthPixel(RE_INFO_TOP + 4), Temp_Str);					
					
						FillBoxWithBitmap(hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP + MV_INSTALL_MENU_BAR_H), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_BOTTOM - RE_INFO_TOP - MV_INSTALL_MENU_BAR_H), &Capture_bmp);
						UnloadBitmap(&Capture_bmp);
					} else if ( wparam == eCS_FE_LOCKED )
					{
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
						
						if ( Display_TPDatas.u8Polar_H == TRUE )
							sprintf(Temp_Str, "%d / H / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_LOCK));
						else
							sprintf(Temp_Str, "%d / V / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_LOCK));

						SetBkMode(hdc,BM_TRANSPARENT);
						SetTextColor(hdc,MVAPP_YELLOW_COLOR);
						CS_MW_TextOut(hdc,ScalerWidthPixel(RE_INFO_LEFT + 20),ScalerHeigthPixel(RE_INFO_TOP + 4), Temp_Str);
					} else if ( wparam == eCS_FE_UNLOCKED )
					{
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
						if ( Display_TPDatas.u8Polar_H == TRUE )
							sprintf(Temp_Str, "%d / H / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_UNLOCK));
						else
							sprintf(Temp_Str, "%d / V / %d .... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_UNLOCK));

						SetBkMode(hdc,BM_TRANSPARENT);
						SetTextColor(hdc,MVAPP_RED_COLOR);
						CS_MW_TextOut(hdc,ScalerWidthPixel(RE_INFO_LEFT + 20),ScalerHeigthPixel(RE_INFO_TOP + 4), Temp_Str);
						/* For Blind Scan By KB Kim 2011.02.28 */
						INSTALL_TPUnlocked();
					} else {
						MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
						MV_FillBox( hdc, ScalerWidthPixel(RE_INFO_LEFT), ScalerHeigthPixel(RE_INFO_TOP), ScalerWidthPixel(RE_INFO_RIGHT - RE_INFO_LEFT), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
						if ( Display_TPDatas.u8Polar_H == TRUE )
							sprintf(Temp_Str, "%d / H / %d ....... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_SCANING));
						else
							sprintf(Temp_Str, "%d / V / %d ....... %s", Display_TPDatas.u16TPFrequency, Display_TPDatas.u16SymbolRate, CS_MW_LoadStringByIdx(CSAPP_STR_SCANING));

						SetBkMode(hdc,BM_TRANSPARENT);
						SetTextColor(hdc,CSAPP_WHITE_COLOR);
						CS_MW_TextOut(hdc,ScalerWidthPixel(RE_INFO_LEFT + 20),ScalerHeigthPixel(RE_INFO_TOP + 4), Temp_Str);
					}
				}
				EndPaint(hwnd,hdc);
			}
			break;
		
		/* For Blind Scan By KB Kim 2011.02.26 */
		case MSG_INSTALL_BLIND_UPDATED:
			if (SearchBlindModeOn)
			{
				BlindTpSearch = 0;
				hdc =BeginPaint(hwnd);
				Draw_Blind_Progress_Bar(hdc, 100);
				EndPaint(hwnd,hdc);
				
				if ( wparam == eCS_FE_LOCKED )
				{
					printf("MSG_INSTALL_BLIND_UPDATED \n");
					MV_INSTALL_Add_Blind_TPData(Search_Condition_Sat_Index, CurrentBlindScanSatIndex, BlindScanParam.PolarH);
					MvInstallStopBlind();
					MvBlindStartInstallation();
				}
				else
				{
					U8                  progress;
					U8                  minPercent;
					U8                  range;
					
					GetDisplayPercentRange (&minPercent, &range);
					progress = minPercent + range;
					hdc =BeginPaint(hwnd);
					Draw_Progress_Bar(hdc, progress);
					EndPaint(hwnd,hdc);
					
					printf("\n==== MSG_INSTALL_BLIND_UPDATED : Un Locked ====\n");
					PostMessage (hwnd, MSG_SEARCH_COMPLETE, 0, 0);
				}
			}
			else
			{
				PostMessage (hwnd, MSG_CLOSE, 0, 0);
			}
			break;
			
		case MSG_SEARCH_COMPLETE:
			/* For Blind Scan By KB Kim 2011.02.26 */
			if (SearchBlindModeOn)
			{
				if (SetNextBlindSearch())
				{
					U16 current_install_sat = 0;
					
					MV_FE_SetBlindProcess(0);
					blindProgress = 0;
					
					hdc =BeginPaint(hwnd);
					Draw_Blind_Progress_Bar(hdc, 0);
					current_install_sat = Search_Condition_Sat_Index[CurrentBlindScanSatIndex];
					if ( current_install_sat != u16Search_Current_Sat_Index )
					{
						MV_Display_SatList(hdc, current_install_sat);
						u16Search_Current_Sat_Index = current_install_sat;
					}
					MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_TOP), ScalerWidthPixel(RE_LIST2_RIGHT - RE_LIST2_LEFT), ScalerHeigthPixel(RE_LIST2_BOTTOM - RE_LIST2_TOP) );
					EndPaint(hwnd,hdc);
					
					CurrentBlindProcess = 0;
					BlindTpSearch       = 1;
					CurrentBlindTpTotal = CS_INSTALL_GetTPListNum();
					MvBlindStopInstallation();
					MvInstallStartBlind(BlindScanParam);
					break;
				}
			}

			{
#ifdef NEW_INSTALL
				// printf("\n==== MSG_SEARCH_COMPLETE ====\n");
				CS_INSTALL_StopInstallation();

				if( MV_DB_Temp_AddService_Count() > 0 )
				{
					MV_Set_Confirm(TRUE);
					MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVE_OR_NOT);
				} else {
					tCS_DBU_Service 			currentservice;
					
					CSApp_SearchResult_Applets = get_prev_windown_status();
					// printf("\n======= No CSApp_SearchResult_Applets : %d ===============\n", CSApp_SearchResult_Applets);

					CS_DBU_LoadCurrentService(&currentservice);

					CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);

					if(CS_DB_SetCurrentService_OrderIndex(currentservice.sCS_DBU_ServiceIndex)==eCS_DB_ERROR)
					{
						CS_DB_SetCurrentService_OrderIndex(0);
						currentservice.sCS_DBU_ServiceIndex = 0;

						/* By KB Kim for Recall List problem : 2011.08.30*/
						CS_DBU_SaveCurrentService(currentservice);
					}

					/* By KB Kim for Recall List problem : 2011.08.30*/
					// CS_DBU_SaveCurrentService(currentservice);
					
					//CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
#if 0 /* By KB Kim 2011.01.13 */
					u8Temp_Sat_Index = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
					u16Temp_TP_Index = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Temp_Sat_Index, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
					MV_GetSatelliteData_ByIndex(&tTemp_Sat_Data, u8Temp_Sat_Index);
					MV_FE_SetTuning_TP(&tTemp_Sat_Data, u8Temp_Sat_Index, u16Temp_TP_Index);
#endif

					PostMessage (hwnd, MSG_CLOSE, 0, 0);
				}
#else	// #ifdef NEW_INSTALL
				
				tCS_DBU_Service 			currentservice;
				U16							firstservice;
				tCS_DB_ServiceManageData	tempdata;
				U16							i = 0;
				U16							num = 0;
				U8							u8Temp_Sat_Index;
				U16							u16Temp_TP_Index;
				MV_stSatInfo				tTemp_Sat_Data;
				
				CS_INSTALL_StopInstallation();
				currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
				CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
				firstservice = CS_INSTALL_GetFirstInstallService();
				if(firstservice != MV_DB_INVALID_SERVICE_INDEX)
				{
					MV_stServiceInfo  tservice_data;

					MV_DB_GetServiceDataByIndex(&tservice_data, firstservice);

					if(tservice_data.u8TvRadio== eCS_DB_RADIO_SERVICE)
					{
						currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
						currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
						CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, FALSE);
					}
					else
					{
						currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
						currentservice.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
						CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, FALSE);
					}

					num = CS_DB_GetCurrentList_ServiceNum();

					for(i=0;i<num;i++)
					{
						CS_DB_GetCurrentList_ServiceData(&tempdata, i);
						if(tempdata.Service_Index == firstservice)
							break;
					}

					CS_DB_SetCurrentService_OrderIndex(i);
					currentservice.sCS_DBU_ServiceIndex = i;

					/* By KB Kim for Recall List problem : 2011.08.30*/
					CS_DBU_SaveCurrentService(currentservice);
				}
				else
				{
					CS_DBU_LoadCurrentService(&currentservice);

					CS_DB_SetCurrentList(currentservice.sCS_DBU_ServiceList, TRUE);
					if(CS_DB_SetCurrentService_OrderIndex(currentservice.sCS_DBU_ServiceIndex)==eCS_DB_ERROR)
					{
						CS_DB_SetCurrentService_OrderIndex(0);
						currentservice.sCS_DBU_ServiceIndex = 0;

						/* By KB Kim for Recall List problem : 2011.08.30*/
						CS_DBU_SaveCurrentService(currentservice);
					}
				}

				/* By KB Kim for Recall List problem : 2011.08.30*/
				// CS_DBU_SaveCurrentService(currentservice);
				CS_DB_SaveDatabase();
				CSApp_SearchResult_Applets = CSApp_Applet_Desktop;

				/* By KB Kim 2011.05.28 */
				CS_AV_VideoBlank();

				CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
				u8Temp_Sat_Index = MV_DB_Get_SatIndex_By_Chindex(CS_DB_GetCurrentServiceIndex());
				u16Temp_TP_Index = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Temp_Sat_Index, MV_DB_Get_TPIndex_By_Chindex(CS_DB_GetCurrentServiceIndex()));
				MV_GetSatelliteData_ByIndex(&tTemp_Sat_Data, u8Temp_Sat_Index);
				//printf("SEARCH COMPLETE ========= %d : %s , %d ===========\n", u8Temp_Sat_Index, tTemp_Sat_Data.acSatelliteName, u16Temp_TP_Index);
				MV_FE_SetTuning_TP(&tTemp_Sat_Data, u8Temp_Sat_Index, u16Temp_TP_Index);

				SendMessage (hwnd, MSG_CLOSE, 0, 0);
#endif 	//#ifdef NEW_INSTALL
			}
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
		case MSG_CLOSE:
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			CS_INSTALL_Unregister_Notify(Install_client);
			//printf("search closed\n");
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			DestroyMainWindow(hwnd);
			PostQuitMessage (hwnd);
			break;
			
		default:			
			break;
	}
		
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

