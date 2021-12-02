
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
#include "table_def.h"
#include "mv_motor.h"

#define		ADD_TP	1
#define 	ADD_CH	2

U16 TP_Str[TP_ITEM_MAX] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TPSELECT,
	CSAPP_STR_FREQUENCY,
	CSAPP_STR_SYMBOLRATE,
	CSAPP_STR_POLARITY,
	CSAPP_STR_TP_DELETE,
	CSAPP_STR_ADD_TP,
	CSAPP_STR_MAN_ADDCH
};

U16 TP_AddStr[TPADD_ITEM_MAX] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_FREQUENCY,
	CSAPP_STR_SYMBOLRATE,
	CSAPP_STR_POLARITY
};

U16 CH_AddStr[CHADD_ITEM_MAX] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TP,
	CSAPP_STR_NAME,
	CSAPP_STR_VPID,
	CSAPP_STR_APID,
	CSAPP_STR_PPID,
	CSAPP_STR_TIME_TS_TYPE
};

U8	TP_Arrow_Kind[TP_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

U8	TP_Enter_Kind[TP_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	TPADD_Arrow_Kind[TPADD_ITEM_MAX] = {
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_SELECT
};

U8	TPADD_Enter_Kind[TPADD_ITEM_MAX] = {
	MV_STATIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_STATIC
};

U8	CHADD_Arrow_Kind[CHADD_ITEM_MAX] = {
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_SELECT
};

U8	CHADD_Enter_Kind[CHADD_ITEM_MAX] = {
	MV_STATIC,
	MV_STATIC,
	MV_SELECT,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_STATIC
};

/* By KB Km 2010.09.04 */
extern eScanConditionID		Search_Condition_Focus_Item;

static CSAPP_Applet_t		CSApp_TP_Applets;
static eTPItemID			TP_Focus_Item=TP_ITEM_SATELLITE;
static eTPADDItemID			TP_ADDFocus_Item=TPADD_ITEM_FREQ;
static eCHADDItemID			CH_ADDFocus_Item=CHADD_ITEM_NAME;
static U8					Mv_TP_Polarity=0;
static U8					u8TpCount;
static BOOL					NumberKeyFlag=FALSE;
static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];  //stSatInfo_Glob
static MV_stTPInfo 			MV_TPInfo;
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static U8					u8SatList_Start_Point;
static U8					u8SatList_End_Point;
static BOOL					Sat_List_Status = FALSE;
static BOOL					TP_Addition_Status = FALSE;
static BOOL					CH_Addition_Status = FALSE;
static BOOL					TP_All_Delete_Flag = FALSE;
static U8					u8SatList_Focus_Item;
static MV_stTPInfo			Modify_TPData;
static MV_stTPInfo			ADD_TPData;
static MV_stServiceInfo		ADD_CHData;
static char					sReturn_str[MAX_NUMERIC_LENGTH+1];

static U8					Mv_SAT_SatFocus;

void MV_SetTPData_DEL_All_TPIndex(HWND hwnd, U8 u8Max_Count)
{
	U8			i;
	MV_stTPInfo Temp_TPData;

	MV_Draw_Progress_Window(hwnd);

	// printf("\n========= START DELETE ALL TP ===========\n");

	i = u8Max_Count;
	while( i > 0 )
	{
		MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, i - 1);
		MV_Draw_Progress_status(hwnd, Temp_TPData, u8Max_Count, i - 1);
		
		//printf("\n========= %d : %d , %d =======\n", i, Temp_TPData.u16TPFrequency, Temp_TPData.u16SymbolRate);
		
		MV_SetTPData_DEL_ALLTPIndex(Temp_TPData);
		i--;
	}
	MV_DEL_ALLTPSave();
	Restore_Progress_Window(hwnd);
}

void MV_Draw_Selected_TP_Satlist(HDC hdc)
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

		if ( u8Glob_Sat_Focus == Temp_SatData.u8SatelliteIndex )
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

BOOL MV_Save_New_TPData(U8 u8Current_SatIndex)
{	
	strcpy(ADD_TPData.acTPName, "" );
	ADD_TPData.u16NID = 0xFFFF;
	ADD_TPData.u16TSID = 0xFFFF;
	ADD_TPData.u8SatelliteIndex = u8Current_SatIndex;
	ADD_TPData.u8Unused = 0;
	ADD_TPData.u8Valid = DB_VALID;
	ADD_TPData.u16TPIndex = CS_DB_Add_TP();

	//printf("\n== Index : %d , Satelite : %d , MAX : %d ===\n", ADD_TPData.u16TPIndex, u8Current_SatIndex, CS_DB_Get_AllTPCount());

	/* By KB Kim 2011.01.13 */
	MV_SetTPData_ADD_TPIndex(ADD_TPData);
	/*
	if ( ADD_TPData.u16TPIndex < CS_DB_Get_AllTPCount() )
		MV_SetTPData_By_TPIndex(ADD_TPData, ADD_TPData.u16TPIndex);
	else
	*/

	/* By KB Kim 2011.01.13 */
	u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

	u8Glob_TP_Focus = MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(u8Glob_Sat_Focus, ADD_TPData.u16TPIndex);
	
	return TRUE;
}

void MV_TP_Satlist_Item_Draw(HDC hdc, U8 u8Start_Point, U8 u8End_Point)
{
	int 			i;
	MV_stSatInfo	Temp_SatData;
	MV_stTPInfo		Temp_TPData;
	RECT			Scroll_Rect;

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(WINDOW_ITEM_DX), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	for ( i = 0 ; i < LIST_ITEM_NUM ; i++ )
	{
		if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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

	if ( TP_Focus_Item == TP_ITEM_SATELLITE )
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_SAT_LIST, MV_VERTICAL);
	else
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u8SatList_Focus_Item, u8Glob_Sat_Focus, EN_ITEM_TP_LIST, MV_VERTICAL);
}

void MV_TP_List_Window( HWND hwnd )
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
	if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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
		MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
	}

	MV_EndPaint(hwnd,hdc);
}

void MV_TP_List_Window_Close( HWND hwnd )
{
	HDC		hdc;
	
	Sat_List_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

CSAPP_Applet_t CSApp_TP_Setting(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_TP_Applets = CSApp_Applet_Error;
	
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
	CreateInfo.spCaption 	= "TP Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = TP_Msg_cb;
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
	
	// printf("TP Setting Cleanup\n");
	
	return CSApp_TP_Applets;
	
}

void MV_Draw_TPSetingSelectBar(HDC hdc, int y_gap, eTPItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(TP_Enter_Kind[esItem])
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
	
	if ( TP_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_TPMenuBar(HDC hdc, U8 u8Focuskind, eTPItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;

	/* By KB Kim 2011.01.13 */
	if ( u8TpCount > 0 )
	{
		MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&MV_TPInfo, u8Glob_Sat_Focus, u8Glob_TP_Focus);
	}
	else
	{
		memset(&MV_TPInfo, 0x00, sizeof(MV_TPInfo));
	}

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_TPSetingSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(TP_Str[esItem]));

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	switch(esItem)
	{
		char	temp_str[30];
		case TP_ITEM_SATELLITE:
			CS_MW_DrawText(hdc, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, -1, &TmpRect, DT_CENTER);	
			break;
		case TP_ITEM_TP_SELECT:
			/* By KB Kim 2011.01.13 */
			if ( u8TpCount > 0 )
			{
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
		case TP_ITEM_FREQ:
			sprintf(temp_str, "%d", MV_TPInfo.u16TPFrequency);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TP_ITEM_SYMBOL:
			sprintf(temp_str, "%d", MV_TPInfo.u16SymbolRate);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case TP_ITEM_POL:			
			if ( MV_TPInfo.u8Polar_H == TRUE )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_H));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_V));
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
/*
		case TP_ITEM_SCAN_TYPE:
			if ( Mv_TP_ScanType == MULTI_SAT )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_MULTI_SAT));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ONE_SAT));
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
*/
		case TP_ITEM_MAX:
		default:
			break;
	}
}

void MV_Draw_TPMenuList(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < TP_ITEM_MAX ; i++ )
	{
		if( TP_Focus_Item == i )
		{
			MV_Draw_TPMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

int TP_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	static BOOL   	bk_flag = TRUE;
	
	switch(message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);

			TP_Focus_Item = TP_ITEM_SATELLITE;
			SearchData.ScanMode = CS_DBU_GetAntenna_Type();
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			memset (&Capture_bmp2, 0, sizeof (BITMAP));			
			Search_Condition_Focus_Item=SCAN_CON_ALL;
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);
			Search_Condition_Status = FALSE;
			Sat_List_Status = FALSE;

			MV_GetSatelliteData(MV_Sat_Data);

			/* By KB Kim 2011.01.13 */
			u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

			/* By KB Kim 2011.01.13 */
			if ( u8TpCount > 0 )
			{
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&MV_TPInfo, u8Glob_Sat_Focus, u8Glob_TP_Focus);
				Mv_TP_Polarity = MV_TPInfo.u8Polar_H;

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

				MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
			}
			else
			{
				memset(&MV_TPInfo, 0x00, sizeof(MV_TPInfo));
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
			{
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, TP_SETTING);

				hdc = BeginPaint(hwnd);
				
				MV_Draw_TPMenuList(hdc);

				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y ), CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH));
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y + MV_INSTALL_SIGNAL_YGAP ), CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY));

				Show_Signal(hdc);
			}

			MV_Draw_Selected_TP_Satlist(hdc);
			MV_Install_draw_help_banner(hdc, TP_SETTING);
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
			if ( Warning_Window_Status == TRUE )
			{
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
					MV_Install_Warning_Window_Close(hwnd);
				
				break;
			}
			
			if ( TP_Addition_Status == TRUE )
			{
				if ( Get_NumKeypad_Status() == FALSE )
				{
					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_BLUE )
					{
						if ( wparam == CSAPP_KEY_BLUE )
						{							
							/* For Add TP Problem By KB Kim 2011.02.26 */
							ADD_TPData.u8SatelliteIndex = u8Glob_Sat_Focus;
							if ( !compare_add_tp(&ADD_TPData) )
							{
								Install_Draw_Warning(hwnd, CSAPP_STR_SAMETP);
								break;
							}
							else if ( ADD_TPData.u16TPFrequency < 1000 || ADD_TPData.u16SymbolRate < 1000 )
							{
								Install_Draw_Warning(hwnd, CSAPP_STR_INVALID_DATA);
								break;
							}
						}
						
						MV_Addition_Window_Close( hwnd );
						NumberKeyFlag=FALSE;

						if ( wparam == CSAPP_KEY_BLUE )
						{
							MV_Save_New_TPData(u8Glob_Sat_Focus);
							/* By KB Kim 2011.01.18 */
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							hdc = BeginPaint(hwnd);
							MV_Draw_TPMenuList(hdc);
							EndPaint(hwnd,hdc);
						}
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					}
				}
				
				TP_Add_Proc(hwnd, wparam);
				break;
			}

			if ( CH_Addition_Status == TRUE )
			{
				if ( Get_NumKeypad_Status() == FALSE && Get_Keypad_Status() == FALSE )
				{
					if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_BLUE )
					{
						MV_Addition_Window_Close( hwnd );
						NumberKeyFlag=FALSE;

						if ( wparam == CSAPP_KEY_BLUE )
						{
							U16 				Service_Index;
							tCS_DBU_Service     ServiceTriplet;
							
							ADD_CHData.u8AudioVolume = 32;
							ADD_CHData.u8Watch = 1;
							MV_DB_AddOneService(ADD_CHData, &Service_Index);
							
							MV_DB_Set_Replace_Index();
							CS_DB_Save_CH_Database();
							//CS_DB_LoadDatabase();	
							Load_CH_UseIndex();
							CS_DBU_LoadCurrentService(& ServiceTriplet);
							CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList,TRUE);
							CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
						}
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					}
				}
				
				CH_Add_Proc(hwnd, wparam);
				break;
			}
	
			if ( Get_NumKeypad_Status() == TRUE )
			{
				UI_NumKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_BLUE )
				{
					if ( wparam == CSAPP_KEY_BLUE )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							Get_Save_Str(sReturn_str);
							//printf("\n2 ========= %d ============\n", atoi(sReturn_str));

							if ( TP_Focus_Item == TP_ITEM_FREQ )
							{
								Modify_TPData.u16TPFrequency = atoi(sReturn_str);
							} else if ( TP_Focus_Item == TP_ITEM_SYMBOL ) {
								Modify_TPData.u16SymbolRate = atoi(sReturn_str);
							}
							else
							{
								/* By KB Kim 2011.01.18 */
								break;
							}

							/* By KB Kim 2011.01.13 */
							if (u8TpCount > 0)
							{
								MV_SetTPData_By_TPIndex(Modify_TPData, Modify_TPData.u16TPIndex);
							}
							else
							{
								ADD_TPData = Modify_TPData;
								MV_Save_New_TPData(u8Glob_Sat_Focus);
								/* By KB Kim 2011.01.18 */
								u8Glob_TP_Focus = 0;
							}
							/* By KB Kim 2011.01.18 */
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
							SendMessage(hwnd, MSG_PAINT, 0, 0);
						}
					} else {
						memset(&Modify_TPData, 0, sizeof(MV_stTPInfo));
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					}
				}
				
				break;
			}

			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_stTPInfo Temp_TPData;
				
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							if ( TP_All_Delete_Flag == TRUE )
							{
								U8	Max_TP_count;
								
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);

								Max_TP_count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

								MV_SetTPData_DEL_All_TPIndex(hwnd, Max_TP_count);

								{
									/* By KB Kim 2011.01.13 */
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
									if (u8TpCount == 0)
									{
										u8Glob_TP_Focus = 0;
									}
									else if (u8Glob_TP_Focus >= u8TpCount)
									{
										u8Glob_TP_Focus = u8TpCount - 1;
									}
								}

								hdc=BeginPaint(hwnd);
								MV_Draw_TPMenuList(hdc);
								EndPaint(hwnd,hdc);
								
								TP_All_Delete_Flag = FALSE;
							}
							else if ( TP_Focus_Item == TP_ITEM_DEL_TP )
							{
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								hdc = BeginPaint(hwnd);
								Restore_Confirm_Window(hdc);
								EndPaint(hwnd,hdc);
								
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);

								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);

								MV_SetTPData_DEL_TPIndex(Temp_TPData);

								/* By KB Kim 2011.01.13 */
								u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								if (u8TpCount == 0)
								{
									u8Glob_TP_Focus = 0;
								}
								else if (u8Glob_TP_Focus >= u8TpCount)
								{
									u8Glob_TP_Focus = u8TpCount - 1;
								}

								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								MV_Draw_TPMenuList(hdc);
								EndPaint(hwnd,hdc);
							}
						}else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							TP_All_Delete_Flag = FALSE;
						}
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
						TP_All_Delete_Flag = FALSE;
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_TP_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_RED:
					if ( Sat_List_Status == FALSE )
					{
						CSApp_TP_Applets=CSApp_Applet_Install;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_GREEN:
					if ( Sat_List_Status == FALSE )
					{
						CSApp_TP_Applets=CSApp_Applet_Sat_Setting;
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
					}
					break;

				case CSAPP_KEY_YELLOW:
					if ( MV_Check_Confirm_Window() == FALSE &&
						 Get_NumKeypad_Status() == FALSE &&
						 CH_Addition_Status == FALSE &&
						 TP_Addition_Status == FALSE &&
						 Warning_Window_Status == FALSE &&
						 Sat_List_Status == FALSE )
					{
						TP_All_Delete_Flag = TRUE;
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
					}
					break;

				case CSAPP_KEY_BLUE:
					if ( Sat_List_Status == FALSE )
					{
						/* 2010.09.04 By KB Kim */
						if (u8TpCount > 0)
						{
							Search_Condition_Focus_Item=SCAN_CON_ALL;
							MV_Search_Condition(hwnd, 1);
							NumberKeyFlag=FALSE;
						}
						else
						{
							Install_Draw_Warning(hwnd, CSAPP_STR_NO_TP_DATA);
						}
					}
					break;
					
				case CSAPP_KEY_DOWN: 
					if ( Sat_List_Status == TRUE )
					{
						MV_stSatInfo	Temp_SatData;
						MV_stTPInfo		Temp_TPData;
						U16				Temp_Count;

						hdc = BeginPaint(hwnd);
						if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						/* By KB Kim 2011.01.13 */
						u8SatList_Focus_Item++;
						if ( u8SatList_Focus_Item >= Temp_Count )
						{
							u8SatList_Focus_Item = 0;
							u8SatList_Start_Point = 0;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							if ( u8SatList_End_Point >= Temp_Count)
								u8SatList_End_Point = Temp_Count - 1;

							MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
						{
							u8SatList_Start_Point = u8SatList_Focus_Item;
							u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
							if ( u8SatList_End_Point >= Temp_Count )
								u8SatList_End_Point = Temp_Count - 1;

							MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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
					
						if ( Search_Condition_Focus_Item >= SCAN_CON_FTA_NIT )
							Search_Condition_Focus_Item = SCAN_CON_ALL ;
						else
							Search_Condition_Focus_Item++;

						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, FOCUS);
						EndPaint(hwnd,hdc);
					} else {
						hdc = BeginPaint(hwnd);
						MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item);

						if(TP_Focus_Item == TP_ITEM_MAX - 1)
							TP_Focus_Item = TP_ITEM_SATELLITE;
						else
							TP_Focus_Item++;

						MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
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
						if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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
							MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8SatList_Focus_Item);
							MV_TPList_Bar_Draw(hdc, u8SatList_Focus_Item%LIST_ITEM_NUM, u8SatList_Start_Point, &Temp_TPData, UNFOCUS);
						}

						if ( u8SatList_Focus_Item == 0 )
						{
							u8SatList_Focus_Item = Temp_Count - 1;
							u8SatList_Start_Point = LIST_ITEM_NUM * (Temp_Count/LIST_ITEM_NUM);
							u8SatList_End_Point = Temp_Count - 1;
							MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else if ( u8SatList_Focus_Item%LIST_ITEM_NUM == 0 )
						{
							u8SatList_Start_Point = u8SatList_Focus_Item - LIST_ITEM_NUM;
							u8SatList_End_Point = u8SatList_Focus_Item;
							u8SatList_Focus_Item--;
							MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						} else 
						{
							u8SatList_Focus_Item--;
							if ( TP_Focus_Item == TP_ITEM_SATELLITE )
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
							Search_Condition_Focus_Item = SCAN_CON_FTA_NIT;
						else
							Search_Condition_Focus_Item--;

						MV_SearchCondition_Bar_Draw(hdc, Search_Condition_Focus_Item, FOCUS);
						EndPaint(hwnd,hdc);
					} else {
						hdc = BeginPaint(hwnd);
						MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item);
						
						if(TP_Focus_Item == TP_ITEM_SATELLITE)
							TP_Focus_Item = TP_ITEM_MAX-1;
						else						
							TP_Focus_Item--;

						MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
						EndPaint(hwnd,hdc);
						// printf("\n##### UP TP_Focus_Item = %d\n", TP_Focus_Item);
						
						NumberKeyFlag=FALSE;
					}
					break;
						
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{
						U16		Temp_Count;

						/* By KB Kim 2011.01.13 */
						if ( TP_Focus_Item == TP_ITEM_SATELLITE )
							Temp_Count = MV_GetSatelliteData_Num();
						else
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
						/* By KB Kim 2011.01.13 */
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
						MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
						
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						
						MV_stTPInfo		Temp_TPData;
						
						switch(TP_Focus_Item)
						{
							case TP_ITEM_SATELLITE:
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

								/* By KB Kim 2011.01.13 */
								u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
								u8Glob_TP_Focus = 0;
								Mv_TP_Polarity = Temp_TPData.u8Polar_H;
								
								MV_Draw_Selected_TP_Satlist(hdc);
								MV_Draw_TPMenuList(hdc);
								EndPaint(hwnd,hdc);
								
								if (u8TpCount > 0)
								{

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

									MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									memset(&Temp_TPData, 0x00, sizeof(Temp_TPData));
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_TP_SELECT:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								
								hdc = BeginPaint(hwnd);
								if ( u8Glob_TP_Focus == 0 )
								{
									u8Glob_TP_Focus = u8TpCount - 1;
								}
								else
								{
									u8Glob_TP_Focus--;
								}

								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								Mv_TP_Polarity = Temp_TPData.u8Polar_H;
								
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+1);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+2);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+3);
								
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_FREQ:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								if ( Temp_TPData.u16TPFrequency > 0 )
									Temp_TPData.u16TPFrequency--;
								else
									Temp_TPData.u16TPFrequency = 27500;

								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_SYMBOL:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								if ( Temp_TPData.u16SymbolRate > 0 )
									Temp_TPData.u16SymbolRate--;
								else
									Temp_TPData.u16SymbolRate = 35000;

								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_POL:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								
								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								if ( Temp_TPData.u8Polar_H == 1 )
									Temp_TPData.u8Polar_H = 0;
								else
									Temp_TPData.u8Polar_H = 1;
								
								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
/*
							case TP_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_TP_ScanType == MULTI_SAT )
									Mv_TP_ScanType = ONE_SAT;
								else
									Mv_TP_ScanType = MULTI_SAT;
								
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;
								break;
*/
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
						if ( TP_Focus_Item == TP_ITEM_SATELLITE )
							Temp_Count = MV_GetSatelliteData_Num();
						else
							Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
						/* By KB Kim 2011.01.13 */
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
						MV_TP_Satlist_Item_Draw(hdc, u8SatList_Start_Point, u8SatList_End_Point);
						EndPaint(hwnd,hdc);
					}
					else if (Search_Condition_Status == TRUE) 
					{
						break;
					}
					else 
					{
						
						MV_stTPInfo		Temp_TPData;
						
						switch(TP_Focus_Item)
						{
							case TP_ITEM_SATELLITE:
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
								u8Glob_TP_Focus = 0;
								Mv_TP_Polarity = Temp_TPData.u8Polar_H;

								MV_Draw_Selected_TP_Satlist(hdc);
								MV_Draw_TPMenuList(hdc);
								EndPaint(hwnd,hdc);
								
								if (u8TpCount > 0)
								{
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

									MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									memset(&Temp_TPData, 0x00, sizeof(Temp_TPData));
									/* By KB Kim 2011.01.21 */
									CS_FE_StopSearch();
								}
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_TP_SELECT:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}
								
								hdc = BeginPaint(hwnd);

								/* By KB Kim 2011.01.13 */
								u8Glob_TP_Focus++;
								if ( u8Glob_TP_Focus >= u8TpCount )
									u8Glob_TP_Focus = 0;

								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								Mv_TP_Polarity = Temp_TPData.u8Polar_H;
								
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+1);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+2);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+3);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_FREQ:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}

								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								if ( Temp_TPData.u16TPFrequency < 27500 )
									Temp_TPData.u16TPFrequency++;
								else
									Temp_TPData.u16TPFrequency = 0;

								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;

							case TP_ITEM_SYMBOL:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}

								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);

								if ( Temp_TPData.u16SymbolRate < 35000 )
									Temp_TPData.u16SymbolRate++;
								else
									Temp_TPData.u16SymbolRate = 0;

								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
									
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								NumberKeyFlag=FALSE;
								break;
								
							case TP_ITEM_POL:
								/* By KB Kim 2011.01.13 */
								if (u8TpCount == 0)
								{
									break;
								}

								MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								if ( Temp_TPData.u8Polar_H == 1 )
									Temp_TPData.u8Polar_H = 0;
								else
									Temp_TPData.u8Polar_H = 1;
								
								MV_DB_Set_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								
								hdc = BeginPaint(hwnd);
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
/*
							case TP_ITEM_SCAN_TYPE:
								hdc = BeginPaint(hwnd);

								if ( Mv_TP_ScanType == MULTI_SAT )
									Mv_TP_ScanType = ONE_SAT;
								else
									Mv_TP_ScanType = MULTI_SAT;
								
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								EndPaint(hwnd,hdc);
								NumberKeyFlag=FALSE;
*/
							default:
								break;
						}
					}

					break;
			
				case CSAPP_KEY_ENTER:     
					{
						if ( Sat_List_Status == TRUE )
						{
							MV_TP_List_Window_Close( hwnd );

							hdc = BeginPaint(hwnd);
							if (TP_Focus_Item == TP_ITEM_SATELLITE)
							{
								/* By KB Kim 2011.01.13 */
								Mv_SAT_SatFocus = u8Glob_Sat_Focus;
								
								if ( u8Glob_Sat_Focus != u8SatList_Focus_Item )
								{
									u8Glob_Sat_Focus = u8SatList_Focus_Item;
									u8Glob_TP_Focus = 0;
									
									u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
									MV_Draw_TPMenuList(hdc);
									MV_Draw_Selected_TP_Satlist(hdc);
								}
							}
							else
							{
								u8Glob_TP_Focus = u8SatList_Focus_Item;
								MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+1);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+2);
								MV_Draw_TPMenuBar(hdc, MV_UNFOCUS, TP_Focus_Item+3);
							}
							EndPaint(hwnd,hdc);

							/* By KB Kim 2011.01.13 */
							if (u8TpCount > 0)
							{
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

								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							}
							else
							{
								u8Glob_TP_Focus = 0;
								/* By KB Kim 2011.01.21 */
								CS_FE_StopSearch();
							}
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						} else if ( Search_Condition_Status == TRUE )
						{
							Search_Condition_Status = FALSE;
							Search_Condition_Focus_Item |= 0x80;
							MV_Install_Warning_Window_Close( hwnd );
							set_prev_windown_status(CSApp_Applet_TP_Setting);
							CSApp_TP_Applets=CSAPP_Applet_Install_Result;
							Search_Condition_Sat_Index[0] = u8Glob_Sat_Focus;
							u8Multi_Select_Sat = 0;
							u8TPScan_Select_TP = u8Glob_TP_Focus;

							MV_SetSatelliteData(MV_Sat_Data);
							
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
						else if ( TP_Focus_Item == TP_ITEM_DEL_TP )
						{
							/* By KB Kim 2011.01.13 */
							if (u8TpCount == 0)
							{
								Install_Draw_Warning(hwnd, CSAPP_STR_NO_TP_DATA);
								break;
							}
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
						}
						else
						{
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
							if ( TP_Focus_Item == TP_ITEM_SATELLITE )
							{
								u8SatList_Focus_Item = u8Glob_Sat_Focus;
								u8SatList_Start_Point = ( u8SatList_Focus_Item / LIST_ITEM_NUM ) * LIST_ITEM_NUM;

								/* By KB Kim 2011.01.13 */
								if ( u8SatList_Start_Point + LIST_ITEM_NUM >= MV_SAT_MAX)
									u8SatList_End_Point = MV_SAT_MAX - 1;
								else
									u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
										
								MV_TP_List_Window( hwnd );
							} else if ( TP_Focus_Item == TP_ITEM_TP_SELECT )
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
									if ( u8SatList_Start_Point + LIST_ITEM_NUM >= u8TpCount )
										u8SatList_End_Point = u8TpCount - 1;
									else
										u8SatList_End_Point = u8SatList_Start_Point + LIST_ITEM_NUM;
								}
								
								MV_TP_List_Window( hwnd );
							}
							else if ( TP_Focus_Item == TP_ITEM_FREQ || TP_Focus_Item == TP_ITEM_SYMBOL )
							{
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Modify_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
								}
								else
								{
									memset(&Modify_TPData, 0x00, sizeof(Modify_TPData));
								}
								
								if ( TP_Focus_Item == TP_ITEM_FREQ )
								{
									MV_Draw_NumKeypad(hwnd, Modify_TPData.u16TPFrequency, 0, MAX_NUMERIC_LENGTH);
								} else if ( TP_Focus_Item == TP_ITEM_SYMBOL ) {
									MV_Draw_NumKeypad(hwnd, Modify_TPData.u16SymbolRate, 0, MAX_NUMERIC_LENGTH);
								}
							} else if ( TP_Focus_Item == TP_ITEM_TP_ADD ) {
								TP_Addition_Status = TRUE;
								/* By KB Kim 2011.01.13 */
								TP_ADDFocus_Item = TPADD_ITEM_FREQ;
								MV_TP_ADD_Window( hwnd, ADD_TP );
							} else if ( TP_Focus_Item == TP_ITEM_MAN_CHADD )
							{
								/* By KB Kim 2011.01.13 */
								if (u8TpCount > 0)
								{
									CH_Addition_Status = TRUE;
									CH_ADDFocus_Item = CHADD_ITEM_NAME;
									MV_TP_ADD_Window( hwnd, ADD_CH );
								}
								else
								{
									Install_Draw_Warning(hwnd, CSAPP_STR_NO_TP_DATA);
								}
							}
						} 
					}
					break;

				case CSAPP_KEY_INFO:
					
					break;
				case CSAPP_KEY_ESC:
					SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					if ( Sat_List_Status == TRUE )
					{
						MV_TP_List_Window_Close( hwnd );
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
						if ( MV_DB_GetALLServiceNumber() == 0 )
						{
							CSApp_TP_Applets=CSApp_Applet_MainMenu;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
						else {
							CSApp_TP_Applets=CSApp_Applet_Desktop;
							/* By KB Kim 2011.05.28 */
							CS_AV_VideoBlank();
							MV_MW_StartService(CS_DB_GetCurrentServiceIndex());
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
							NumberKeyFlag=FALSE;
						}
					}
					break;
					
				case CSAPP_KEY_MENU:
					SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					if ( Sat_List_Status == TRUE )
					{
						MV_TP_List_Window_Close( hwnd );
					} else if ( Search_Condition_Status == TRUE ) {
						Search_Condition_Status = FALSE;
						MV_Install_Warning_Window_Close( hwnd );
						NumberKeyFlag=FALSE;
					} else {
#ifdef SMART_PHONE
						if ( b8Last_App_Status == CSApp_Applet_Smart_OSD )
							CSApp_TP_Applets=b8Last_App_Status;
						else
#endif
						{
							if ( MV_DB_GetALLServiceNumber() > 0 )
							{
								/* By KB Kim 2011.05.28 */
								CS_AV_VideoBlank();
							}
							CSApp_TP_Applets=CSApp_Applet_MainMenu;
						}
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
						NumberKeyFlag=FALSE;
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
					{
						U8	Input_Key;

						switch(wparam)
						{
							case CSAPP_KEY_0:
								Input_Key = 0;
								break;
							case CSAPP_KEY_1:
								Input_Key = 1;
								break;
							case CSAPP_KEY_2:
								Input_Key = 2;
								break;
							case CSAPP_KEY_3:
								Input_Key = 3;
								break;
							case CSAPP_KEY_4:
								Input_Key = 4;
								break;
							case CSAPP_KEY_5:
								Input_Key = 5;
								break;
							case CSAPP_KEY_6:
								Input_Key = 6;
								break;
							case CSAPP_KEY_7:
								Input_Key = 7;
								break;
							case CSAPP_KEY_8:
								Input_Key = 8;
								break;
							case CSAPP_KEY_9:
								Input_Key = 9;
								break;
							default:
								Input_Key = 0;
								break;
						}
						
						MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Modify_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);
						if ( TP_Focus_Item == TP_ITEM_FREQ )
						{
							MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_NUMERIC_LENGTH);
						} else if ( TP_Focus_Item == TP_ITEM_SYMBOL ) {
							MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_NUMERIC_LENGTH);
						}
					//MV_Draw_TPMenuBar(hdc, MV_FOCUS, TP_Focus_Item);
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

void MV_Draw_ADDSelectBar(HDC hdc, int y_gap, U8 AddKind, U8 LineIndex)
{
	int mid_width = WINDOW_ITEM_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	if ( AddKind == ADD_TP )
	{
		switch(TPADD_Enter_Kind[LineIndex])
		{
			case MV_NUMERIC:
				FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
				break;
			case MV_SELECT:
				FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
				break;
			default:
				break;
		}

		if ( TPADD_Arrow_Kind[LineIndex] == MV_SELECT )
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + WINDOW_ITEM_DX/2 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
		}
	} else {
		switch(CHADD_Enter_Kind[LineIndex])
		{
			case MV_NUMERIC:
				FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
				break;
			case MV_SELECT:
				FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
				break;
			default:
				break;
		}

		if ( CHADD_Arrow_Kind[LineIndex] == MV_SELECT )
		{
			FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + WINDOW_ITEM_DX/2 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
		}
	}
}

void MV_Add_Item_Draw(HDC hdc, U8 LineIndex, U8 AddKind, U8 FocusKind)
{
	int 			y_gap = WINDOW_ITEM_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * LineIndex;
	RECT			TmpRect;
	MV_stSatInfo	Temp_SatData;
	MV_stTPInfo		Temp_TPData;
	char			Temp_Str[30];

	MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Glob_Sat_Focus);
	MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPData, u8Glob_Sat_Focus, u8Glob_TP_Focus);

	ADD_CHData.u16TransponderIndex = Temp_TPData.u16TPIndex;
	
	if ( FocusKind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_ADDSelectBar(hdc, y_gap, AddKind, LineIndex);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(WINDOW_ITEM_DX),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}

	if ( AddKind == ADD_TP )
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(TP_AddStr[LineIndex]));
	else
		MV_CS_MW_TextOut( hdc, ScalerWidthPixel(WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(CH_AddStr[LineIndex]));

	TmpRect.left = WINDOW_ITEM_X + WINDOW_ITEM_DX / 2;
	TmpRect.top = y_gap + 2;
	TmpRect.right = WINDOW_ITEM_X + WINDOW_ITEM_DX;
	TmpRect.bottom = y_gap + MV_INSTALL_MENU_BAR_H - 2;

	if ( AddKind == ADD_TP )
	{
		switch( LineIndex )
		{
			case TPADD_ITEM_SATELLITE: 	
				CS_MW_DrawText(hdc, Temp_SatData.acSatelliteName, -1, &TmpRect, DT_CENTER);
				break;
			case TPADD_ITEM_FREQ:
				sprintf(Temp_Str, "%d", ADD_TPData.u16TPFrequency );
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case TPADD_ITEM_SYMBOL: 	
				sprintf(Temp_Str, "%d", ADD_TPData.u16SymbolRate);
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case TPADD_ITEM_POL:
				if ( ADD_TPData.u8Polar_H == 1 )
					sprintf(Temp_Str, "%s", "H" );
				else
					sprintf(Temp_Str, "%s", "V" );
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			default:
				break;
		}
	} else {
		switch( LineIndex )
		{
			case CHADD_ITEM_SATELLITE: 	
				CS_MW_DrawText(hdc, Temp_SatData.acSatelliteName, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_TP: 	
				if ( ADD_TPData.u8Polar_H == 1 )
					sprintf(Temp_Str, "%d/%s/%d", Temp_TPData.u16TPFrequency, "H", Temp_TPData.u16SymbolRate);
				else
					sprintf(Temp_Str, "%d/%s/%d", Temp_TPData.u16TPFrequency, "V", Temp_TPData.u16SymbolRate);
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_NAME: 	
				CS_MW_DrawText(hdc, ADD_CHData.acServiceName, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_VPID: 
				sprintf(Temp_Str, "%d", ADD_CHData.u16VideoPid );
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_APID:
				sprintf(Temp_Str, "%d", ADD_CHData.u16AudioPid );
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_PPID: 
				sprintf(Temp_Str, "%d", ADD_CHData.u16PCRPid );
				CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
				break;
			case CHADD_ITEM_TVRADIO: 
				switch(ADD_CHData.u8TvRadio)
				{
					case TV_SERVICE:
						sprintf(Temp_Str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_TV) );
						CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
						break;
					case RADIO_SERVICE:
						sprintf(Temp_Str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_RADIO) );
						CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
						break;
					case HDTV_SERVICE:
						sprintf(Temp_Str, "HD %s", CS_MW_LoadStringByIdx(CSAPP_STR_TV) );
						CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
						break;
					case DATA_SERVICE:
						sprintf(Temp_Str, "%s", "Data" );
						CS_MW_DrawText(hdc, Temp_Str, -1, &TmpRect, DT_CENTER);
						break;
				}
				break;
			default:
				break;
		}
	}
}

void MV_TP_ADD_Window( HWND hwnd, U8 AddKind )
{
	HDC 	hdc;
	RECT	Temp_Rect;

	memset(&ADD_TPData, 0, sizeof(MV_stTPInfo));
	memset(&ADD_CHData, 0, sizeof(MV_stServiceInfo));
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp2);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT + WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP + WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WINDOW_LEFT + WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WINDOW_TOP + WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WINDOW_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WINDOW_TOP),ScalerWidthPixel(WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WINDOW_DX),ScalerHeigthPixel(WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
	
	Temp_Rect.top 	= WINDOW_TOP + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WINDOW_LEFT + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + WINDOW_DX - WINDOW_OUT_GAP * 2;

	if ( AddKind == ADD_TP )
		MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_TP_ADD);
	else
		MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_CH_ADD);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WINDOW_ITEM_X), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(WINDOW_ITEM_DX), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	MV_Add_FullItem_Draw(hdc, AddKind);

/*
	FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 4), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 4 + 2),	"Back Space");
*/
	if ( AddKind == ADD_TP )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 3), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 3 + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CLEAR));
		FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 2), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 2 + 2),	CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 3 + MV_INSTALL_MENU_HEIGHT), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 3 + 2 + MV_INSTALL_MENU_HEIGHT),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CLEAR));
		FillBoxWithBitmap(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 2 + MV_INSTALL_MENU_HEIGHT), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		CS_MW_TextOut(hdc,ScalerWidthPixel(WINDOW_ITEM_X + MV_INSTALL_MENU_BAR_H + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - MV_INSTALL_MENU_BAR_H * 2 + 2 + MV_INSTALL_MENU_HEIGHT),	CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
	}
	
	MV_EndPaint(hwnd,hdc);
}

void MV_Add_FullItem_Draw(HDC hdc, U8 AddKind)
{
	U8	i;

	if ( AddKind == ADD_TP ){
		for ( i = 0 ; i < TPADD_ITEM_MAX ; i++ )
		{
			if ( i == TP_ADDFocus_Item )
				MV_Add_Item_Draw(hdc, i, AddKind, MV_FOCUS);
			else
				MV_Add_Item_Draw(hdc, i, AddKind, MV_UNFOCUS);
		}
	} else {
		for ( i = 0 ; i < CHADD_ITEM_MAX ; i++ )
		{
			if ( i == CH_ADDFocus_Item )
				MV_Add_Item_Draw(hdc, i, AddKind, MV_FOCUS);
			else
				MV_Add_Item_Draw(hdc, i, AddKind, MV_UNFOCUS);
		}
	}
}

void MV_Addition_Window_Close( HWND hwnd )
{
	HDC		hdc;
	
	TP_Addition_Status = FALSE;
	CH_Addition_Status = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(WINDOW_LEFT), ScalerHeigthPixel(WINDOW_TOP), ScalerWidthPixel(WINDOW_DX), ScalerHeigthPixel(WINDOW_DY), &Capture_bmp2);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp2);
}

BOOL TP_Add_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		     hdc=0;
	/* By KB Kim 2011.01.13 */
	eTPADDItemID nextAddFocusItem;

	if ( Get_NumKeypad_Status() == TRUE )
	{
		UI_NumKeypad_Proc(hwnd, u8Key);
		
		if ( u8Key == CSAPP_KEY_ESC || u8Key == CSAPP_KEY_MENU || u8Key == CSAPP_KEY_BLUE )
		{
			if ( u8Key == CSAPP_KEY_BLUE )
			{
				if ( Get_Keypad_is_Save() == TRUE )
				{
					Get_Save_Str(sReturn_str);
					//printf("\n======= %s : %d ==========\n", sReturn_str, atoi(sReturn_str));

					/* By KB Kim 2011.01.13 */
					nextAddFocusItem = TP_ADDFocus_Item;
					if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ )
					{
						nextAddFocusItem = TPADD_ITEM_SYMBOL;
						ADD_TPData.u16TPFrequency = atoi(sReturn_str);
					} else if ( TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
						nextAddFocusItem = TPADD_ITEM_POL;
						ADD_TPData.u16SymbolRate = atoi(sReturn_str);
					}

					if (nextAddFocusItem != TP_ADDFocus_Item)
					{
						hdc = MV_BeginPaint(hwnd);
						MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_UNFOCUS);
						MV_EndPaint(hwnd,hdc);
						TP_ADDFocus_Item = nextAddFocusItem;
					}
				}
				hdc = MV_BeginPaint(hwnd);
				MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_FOCUS);
				MV_EndPaint(hwnd,hdc);
			} else {
				memset(&ADD_TPData, 0, sizeof(MV_stTPInfo));
			}
		}
		
		return FALSE;
	}
	
	switch (u8Key)
    {
        case CSAPP_KEY_UP:
			hdc = MV_BeginPaint(hwnd);
			MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_UNFOCUS);

        	if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ )
				TP_ADDFocus_Item = TPADD_ITEM_MAX - 1;
    		else
    			TP_ADDFocus_Item--;    		
			
   			MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
        case CSAPP_KEY_DOWN:
			hdc = MV_BeginPaint(hwnd);
			MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_UNFOCUS);
			
        	if ( TP_ADDFocus_Item == TPADD_ITEM_MAX - 1 )
				TP_ADDFocus_Item = TPADD_ITEM_FREQ;
    		else
    			TP_ADDFocus_Item++;
			
			MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
		case CSAPP_KEY_LEFT:
        case CSAPP_KEY_RIGHT:
			hdc = MV_BeginPaint(hwnd);
			
        	if ( ADD_TPData.u8Polar_H == P_H )
				ADD_TPData.u8Polar_H = P_V;
    		else
    			ADD_TPData.u8Polar_H = P_H;
			
			MV_Add_Item_Draw(hdc, TP_ADDFocus_Item, ADD_TP, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
		case CSAPP_KEY_GREEN:
			memset(&ADD_TPData, 0, sizeof(MV_stTPInfo));
			/* By KB Kim 2011.01.13 */
			TP_ADDFocus_Item = TPADD_ITEM_FREQ;
			MV_Add_FullItem_Draw(hdc, ADD_TP);
        	break;
		case CSAPP_KEY_BLUE:
        	break;
        case CSAPP_KEY_ENTER:
			if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ || TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
				
				if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ )
				{
					MV_Draw_NumKeypad(hwnd, ADD_TPData.u16TPFrequency, 0, MAX_NUMERIC_LENGTH);
				} else if ( TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
					MV_Draw_NumKeypad(hwnd, ADD_TPData.u16SymbolRate, 0, MAX_NUMERIC_LENGTH);
				}
			}
        	break;
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;
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
			{
				U8	Input_Key;

				switch(u8Key)
				{
					case CSAPP_KEY_0:
						Input_Key = 0;
						break;
					case CSAPP_KEY_1:
						Input_Key = 1;
						break;
					case CSAPP_KEY_2:
						Input_Key = 2;
						break;
					case CSAPP_KEY_3:
						Input_Key = 3;
						break;
					case CSAPP_KEY_4:
						Input_Key = 4;
						break;
					case CSAPP_KEY_5:
						Input_Key = 5;
						break;
					case CSAPP_KEY_6:
						Input_Key = 6;
						break;
					case CSAPP_KEY_7:
						Input_Key = 7;
						break;
					case CSAPP_KEY_8:
						Input_Key = 8;
						break;
					case CSAPP_KEY_9:
						Input_Key = 9;
						break;
					default:
						Input_Key = 0;
						break;
				}

				/*
				if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ || TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
					if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ )
					{
						MV_Draw_NumKeypad(hwnd, ADD_TPData.u16TPFrequency, 0, MAX_NUMERIC_LENGTH);
					} else if ( TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
						MV_Draw_NumKeypad(hwnd, ADD_TPData.u16SymbolRate, 0, MAX_NUMERIC_LENGTH);
					}
				}
				*/
				if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ || TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
					if ( TP_ADDFocus_Item == TPADD_ITEM_FREQ )
					{
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_NUMERIC_LENGTH);
					} else if ( TP_ADDFocus_Item == TPADD_ITEM_SYMBOL ) {
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_NUMERIC_LENGTH);
					}
				}
			}
			break;   
    }
	return TRUE;
}

BOOL CH_Add_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	if ( Get_Keypad_Status() == TRUE ) 
	{
		UI_Keypad_Proc(hwnd, u8Key);
		
		if ( u8Key == CSAPP_KEY_ESC || u8Key == CSAPP_KEY_MENU || u8Key == CSAPP_KEY_ENTER || u8Key == CSAPP_KEY_YELLOW )
		{
			if ( u8Key == CSAPP_KEY_ENTER || u8Key == CSAPP_KEY_YELLOW )
			{
				if ( Get_Keypad_is_Save() == TRUE )
				{
					Get_Save_Str(sReturn_str);
					
					if ( CH_ADDFocus_Item == CHADD_ITEM_NAME ) {
						sprintf(ADD_CHData.acServiceName, "%s", sReturn_str);
					}					
					hdc = MV_BeginPaint(hwnd);
					MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
					MV_EndPaint(hwnd,hdc);
				}
			} else {
				memset(&ADD_CHData.acServiceName, 0, MAX_SERVICE_NAME_LENGTH);
			}
		}
		return FALSE;
	}
	else if ( Get_NumKeypad_Status() == TRUE )
	{
		UI_NumKeypad_Proc(hwnd, u8Key);
		
		if ( u8Key == CSAPP_KEY_ESC || u8Key == CSAPP_KEY_MENU || u8Key == CSAPP_KEY_BLUE )
		{
			if ( u8Key == CSAPP_KEY_BLUE )
			{
				if ( Get_Keypad_is_Save() == TRUE )
				{
					Get_Save_Str(sReturn_str);
					
					if ( CH_ADDFocus_Item == CHADD_ITEM_VPID ) {
						ADD_CHData.u16VideoPid = atoi(sReturn_str);
					} else if ( CH_ADDFocus_Item == CHADD_ITEM_APID ) {
						ADD_CHData.u16AudioPid = atoi(sReturn_str);
					} else if ( CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
						ADD_CHData.u16PCRPid = atoi(sReturn_str);
					}
				}
			} else {
				if ( CH_ADDFocus_Item == CHADD_ITEM_VPID ) {
					ADD_CHData.u16VideoPid = 0;
				} else if ( CH_ADDFocus_Item == CHADD_ITEM_APID ) {
					ADD_CHData.u16AudioPid = 0;
				} else if ( CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
					ADD_CHData.u16PCRPid = 0;
				}
			}
			hdc = MV_BeginPaint(hwnd);
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
		}
		return FALSE;
	}
	
	switch (u8Key)
    {
        case CSAPP_KEY_UP:
			hdc = MV_BeginPaint(hwnd);
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_UNFOCUS);

        	if ( CH_ADDFocus_Item == CHADD_ITEM_NAME )
				CH_ADDFocus_Item = CHADD_ITEM_MAX - 1;
    		else
    			CH_ADDFocus_Item--;    		
			
   			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
        case CSAPP_KEY_DOWN:
			hdc = MV_BeginPaint(hwnd);
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_UNFOCUS);
			
        	if ( CH_ADDFocus_Item == CHADD_ITEM_MAX - 1 )
				CH_ADDFocus_Item = CHADD_ITEM_NAME;
    		else
    			CH_ADDFocus_Item++;
			
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
		case CSAPP_KEY_LEFT:
			hdc = MV_BeginPaint(hwnd);
			
        	if ( ADD_CHData.u8TvRadio == DATA_SERVICE )
				ADD_CHData.u8TvRadio = HDTV_SERVICE;
    		else
    			ADD_CHData.u8TvRadio--;
			
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
		case CSAPP_KEY_RIGHT:
			hdc = MV_BeginPaint(hwnd);
			
        	if ( ADD_CHData.u8TvRadio == HDTV_SERVICE )
				ADD_CHData.u8TvRadio = DATA_SERVICE;
    		else
    			ADD_CHData.u8TvRadio++;
			
			MV_Add_Item_Draw(hdc, CH_ADDFocus_Item, ADD_CH, MV_FOCUS);
			MV_EndPaint(hwnd,hdc);
        	break;
		case CSAPP_KEY_GREEN:
        	break;
		case CSAPP_KEY_BLUE:
        	break;
        case CSAPP_KEY_ENTER:
			if ( CH_ADDFocus_Item == CHADD_ITEM_NAME )
			{
				MV_Draw_Keypad(hwnd, ADD_CHData.acServiceName, MAX_SERVICE_NAME_LENGTH);
			}
			
			if ( CH_ADDFocus_Item == CHADD_ITEM_VPID || CH_ADDFocus_Item == CHADD_ITEM_APID || CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
				if ( CH_ADDFocus_Item == CHADD_ITEM_VPID ) {
					MV_Draw_NumKeypad(hwnd, 0, 0, MAX_PID_LENGTH);
				} else if ( CH_ADDFocus_Item == CHADD_ITEM_APID ) {
					MV_Draw_NumKeypad(hwnd, 0, 0, MAX_PID_LENGTH);
				} else if ( CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
					MV_Draw_NumKeypad(hwnd, 0, 0, MAX_PID_LENGTH);
				}
			}
        	break;
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;
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
			{
				U8	Input_Key;

				switch(u8Key)
				{
					case CSAPP_KEY_0:
						Input_Key = 0;
						break;
					case CSAPP_KEY_1:
						Input_Key = 1;
						break;
					case CSAPP_KEY_2:
						Input_Key = 2;
						break;
					case CSAPP_KEY_3:
						Input_Key = 3;
						break;
					case CSAPP_KEY_4:
						Input_Key = 4;
						break;
					case CSAPP_KEY_5:
						Input_Key = 5;
						break;
					case CSAPP_KEY_6:
						Input_Key = 6;
						break;
					case CSAPP_KEY_7:
						Input_Key = 7;
						break;
					case CSAPP_KEY_8:
						Input_Key = 8;
						break;
					case CSAPP_KEY_9:
						Input_Key = 9;
						break;
					default:
						Input_Key = 0;
						break;
				}
				
				if ( CH_ADDFocus_Item == CHADD_ITEM_VPID || CH_ADDFocus_Item == CHADD_ITEM_APID || CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
					if ( CH_ADDFocus_Item == CHADD_ITEM_VPID ) {
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_PID_LENGTH);
					} else if ( CH_ADDFocus_Item == CHADD_ITEM_APID ) {
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_PID_LENGTH);
					} else if ( CH_ADDFocus_Item == CHADD_ITEM_PPID ) {
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_PID_LENGTH);
					}
				}
			}
			break;   
    }
	return TRUE;
}

