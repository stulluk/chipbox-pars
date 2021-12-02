#include "mv_menu_ctr.h"
#include "ch_install.h"
#include "ui_common.h"
#include "cs_ext_slist.h"

#define CL_WINDOW_X					160
#define CL_WINDOW_Y					100
//#define	CL_WINDOW_DX				340
//#define	CL_WINDOW_DX				361
#define	CL_WINDOW_DX				1050
#define CL_WINDOW_DY				510
#define CL_WINDOW_ITEM_GAP			10
#define CL_WINDOW_ITEM_DY			30
#define	CL_WINDOW_ITEM_HEIGHT		(CL_WINDOW_ITEM_DY + CL_WINDOW_ITEM_GAP)
#define CL_WINDOW_NO_X				180
#define CL_WINDOW_NO_DX				80
#define CL_WINDOW_NAME_X			260
#define CL_WINDOW_NAME_DX			140
#define CL_WINDOW_SCRAMBLE_X 		405
#define CL_WINDOW_LOCK_X			426
#define CL_WINDOW_FAVORITE_X		447
#define CL_WINDOW_SDHD_X			468
#define CL_WINDOW_FOCUS_DX			( ( CL_WINDOW_SDHD_X + 30 ) - ( CL_WINDOW_X + 10 ) )
#define CL_WINDOW_FOCUS_X1			CL_WINDOW_X+10
#define CL_WINDOW_FOCUS_X2			( CL_WINDOW_FOCUS_X1 + CL_WINDOW_FOCUS_DX + 10 )
#define CL_WINDOW_FOCUS_X3			( CL_WINDOW_FOCUS_X2 + CL_WINDOW_FOCUS_DX + 10 )
#define	CL_WINDOW_TITLE_Y			120
#define	CL_WINDOW_LIST_Y			150
#define	CL_WINDOW_INFOR_Y			470
#define CL_WINDOW_ICON_DX			200
#define	CL_WINDOW_ICON_X			CL_WINDOW_NO_X
#define	CL_WINDOW_ICON_X2			(CL_WINDOW_ICON_X + CL_WINDOW_ICON_DX)
#define	CL_WINDOW_ICON_X3			(CL_WINDOW_ICON_X2 + CL_WINDOW_ICON_DX)
#define	CL_WINDOW_ICON_X4			(CL_WINDOW_ICON_X3 + CL_WINDOW_ICON_DX)
#define	CL_WINDOW_ICON_Y			540
#define	CL_WINDOW_ICON_Y2			570

#define	pin_box_dx					240
#define	pin_box_dy					110
#define	pin_box_base_x				((720-pin_box_dx)/2)
#define	pin_box_base_y 				((576-pin_box_dy)/2)

#define	SERVICES_NUM_PER_PAGE		10
#define SERVICES_NUM_PER_EXPAGE		30
#define	FIELDS_PER_LINE				6

tComboList_Field_Rect		SList_Ext_ComboList_First_Line[FIELDS_PER_LINE] = {
													{CL_WINDOW_NO_X, CL_WINDOW_NO_DX},
													{CL_WINDOW_NAME_X, CL_WINDOW_NAME_DX},
													{CL_WINDOW_SCRAMBLE_X, 21},
													{CL_WINDOW_LOCK_X, 21},
													{CL_WINDOW_FAVORITE_X, 21},
													{CL_WINDOW_SDHD_X, 21}
												};

tComboList_Field_Rect		SList_Ext_ComboList_Second_Line[FIELDS_PER_LINE] = {
													{CL_WINDOW_NO_X + (CL_WINDOW_FOCUS_DX + 10), CL_WINDOW_NO_DX},
													{CL_WINDOW_NAME_X + (CL_WINDOW_FOCUS_DX + 10), CL_WINDOW_NAME_DX},
													{CL_WINDOW_SCRAMBLE_X + (CL_WINDOW_FOCUS_DX + 10), 21},
													{CL_WINDOW_LOCK_X + (CL_WINDOW_FOCUS_DX + 10), 21},
													{CL_WINDOW_FAVORITE_X + (CL_WINDOW_FOCUS_DX + 10), 21},
													{CL_WINDOW_SDHD_X + (CL_WINDOW_FOCUS_DX + 10), 21}
												};

tComboList_Field_Rect		SList_Ext_ComboList_Third_Line[FIELDS_PER_LINE] = {
													{CL_WINDOW_NO_X + (CL_WINDOW_FOCUS_DX + 10)*2, CL_WINDOW_NO_DX},
													{CL_WINDOW_NAME_X + (CL_WINDOW_FOCUS_DX + 10)*2, CL_WINDOW_NAME_DX},
													{CL_WINDOW_SCRAMBLE_X + (CL_WINDOW_FOCUS_DX + 10)*2, 21},
													{CL_WINDOW_LOCK_X + (CL_WINDOW_FOCUS_DX + 10)*2, 21},
													{CL_WINDOW_FAVORITE_X + (CL_WINDOW_FOCUS_DX + 10)*2, 21},
													{CL_WINDOW_SDHD_X + (CL_WINDOW_FOCUS_DX + 10)*2, 21}
												};

tComboList_Element			SList_Ext_ComboList_First = {	
													{ CL_WINDOW_FOCUS_X1, CL_WINDOW_LIST_Y+10, CL_WINDOW_FOCUS_DX, CL_WINDOW_ITEM_DY },
													FIELDS_PER_LINE,
													SList_Ext_ComboList_First_Line
												};

tComboList_Element			SList_Ext_ComboList_Second = {	
													{CL_WINDOW_FOCUS_X2, CL_WINDOW_LIST_Y+10, CL_WINDOW_FOCUS_DX, CL_WINDOW_ITEM_DY},
													FIELDS_PER_LINE,
													SList_Ext_ComboList_Second_Line
												};

tComboList_Element			SList_Ext_ComboList_Third = {	
													{CL_WINDOW_FOCUS_X3, CL_WINDOW_LIST_Y+10, CL_WINDOW_FOCUS_DX, CL_WINDOW_ITEM_DY},
													FIELDS_PER_LINE,
													SList_Ext_ComboList_Third_Line
												};

CSAPP_Applet_t				CSApp_Ext_SList_Applets;

static tCS_DB_ServiceListTriplet	chlist_Ext_triplet;
static U16							chlist_Ext_Total_Service = 0;	
static U16							chlist_Ext_Current_Service = 0;
static U16							chlist_Ext_Current_Page = 0;
static U16							chlist_Ext_Prev_Page = 0;
static U16							chlist_Ext_Current_Focus = 0;
static tCS_DB_ServiceManageData		chlist_Ext_item_data;
static MV_stServiceInfo				chlist_Ext_service_data;
static U32							chlist_Ext_servicelist_type;
static U8							u8Ext_Sat_index = 0;
static U8							u8Channel_Focus = 0;
static BOOL							bExt_Ch_Lock_Flag = FALSE;

static tCS_DBU_Service     		Ext_back_triplet;

#define SORT_NUM  			7

U16 Ext_Sort_Str[SORT_NUM] = {
	CSAPP_STR_A_Z,
	CSAPP_STR_Z_A,
	CSAPP_STR_FTA_CAS,
	CSAPP_STR_CAS_FTA,
	CSAPP_STR_SD_HD,
	CSAPP_STR_HD_SD,
	CSAPP_STR_RESTORE_NORMAL
};

//static  BOOL	Service_Unlocked = FALSE;

void MV_Draw_Ext_CH_List_Window(HDC hdc)
{
#if 0
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(CL_WINDOW_DX),ScalerHeigthPixel(CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
#else
	/**************************************  steel Image Cover Channel List outside ******************************************/
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X - 4 ), ScalerHeigthPixel(CL_WINDOW_Y - 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth + 4), ScalerHeigthPixel(CL_WINDOW_Y - 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X - 4 ), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight + 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth + 4), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight + 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_RIGHT]);
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X - 4), ScalerHeigthPixel(CL_WINDOW_Y + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight - 12), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_LEFT_LINE].bmWidth), ScalerHeigthPixel(CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2 + 8), &MV_BMP[MVBMP_SUBMENU_LEFT_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth - 4), ScalerHeigthPixel(CL_WINDOW_Y + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight - 12), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_RIGHT_LINE].bmWidth), ScalerHeigthPixel(CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2 + 8), &MV_BMP[MVBMP_SUBMENU_RIGHT_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth - 12), ScalerHeigthPixel(CL_WINDOW_Y - 4), ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2 + 8), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_TOP_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth - 12), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight - 4), ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2 + 8), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE]);
	/**************************************  steel Image Cover Channel List outside ******************************************/

	if ( CFG_Menu_Back_Color.MV_R == 0 && CFG_Menu_Back_Color.MV_G == 0 && CFG_Menu_Back_Color.MV_B == 0 )
		SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	else
		SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_R, CFG_Menu_Back_Color.MV_G, CFG_Menu_Back_Color.MV_B, 0xFF));
		// SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_A, CFG_Menu_Back_Color.MV_B, CFG_Menu_Back_Color.MV_G, 0xFF));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + 2), ScalerHeigthPixel(CL_WINDOW_Y),ScalerWidthPixel(CL_WINDOW_DX - 4),ScalerHeigthPixel(CL_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + 2),ScalerWidthPixel(CL_WINDOW_DX),ScalerHeigthPixel(CL_WINDOW_DY - 4));	
#endif

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_TITLE_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_FOCUS_X1), ScalerHeigthPixel(CL_WINDOW_LIST_Y+10),ScalerWidthPixel(CL_WINDOW_FOCUS_DX),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 10));	
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_FOCUS_X2), ScalerHeigthPixel(CL_WINDOW_LIST_Y+10),ScalerWidthPixel(CL_WINDOW_FOCUS_DX),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 10));	
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_FOCUS_X3), ScalerHeigthPixel(CL_WINDOW_LIST_Y+10),ScalerWidthPixel(CL_WINDOW_FOCUS_DX),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 10));	

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FIND));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_EDIT));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X3), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X3)+ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SORT_KEY));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X4), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X4)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_NORMAL));

#if 0
	if (chlist_Ext_servicelist_type == eCS_DB_FAV_TV_LIST || chlist_Ext_servicelist_type == eCS_DB_FAV_RADIO_LIST)
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X), ScalerHeigthPixel(CL_WINDOW_ICON_Y2), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLACK_BUTTON]);
		sprintf(acTmp_Str, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_FAV_KEY), CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));
		CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y2), acTmp_Str);
	}
#endif
}

void MV_Draw_Ext_CH_Info(HDC hdc)
{
	char			buff2[50];
	MV_stTPInfo		tpdata;
	MV_stSatInfo	Temp_SatData;	
	
	if (chlist_Ext_Total_Service != 0)
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	
		
		memset(buff2, 0, 50);
		CS_DB_GetCurrentList_ServiceData(&chlist_Ext_item_data, chlist_Ext_Current_Service);
		MV_DB_GetServiceDataByIndex(&chlist_Ext_service_data, chlist_Ext_item_data.Service_Index);
		MV_DB_GetTPDataByIndex(&tpdata, chlist_Ext_service_data.u16TransponderIndex);
		MV_GetSatelliteData_ByIndex(&Temp_SatData, MV_DB_Get_SatIndex_By_TPindex(chlist_Ext_service_data.u16TransponderIndex));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_NO_X), ScalerHeigthPixel(CL_WINDOW_INFOR_Y + ( CL_WINDOW_ITEM_DY*2 - MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight )/2), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth), &MV_BMP[MVBMP_CHLIST_INFO_ICON]);
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		sprintf(buff2, "%s", Temp_SatData.acSatelliteName);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CL_WINDOW_NO_X + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth + CL_WINDOW_ITEM_GAP),ScalerHeigthPixel(CL_WINDOW_INFOR_Y + 4), buff2);

		if ( tpdata.u8Polar_H == 1 )
			sprintf(buff2, "%d / H / %d", tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		else
			sprintf(buff2, "%d / V / %d", tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CL_WINDOW_NO_X + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth + CL_WINDOW_ITEM_GAP),ScalerHeigthPixel(CL_WINDOW_INFOR_Y + CL_WINDOW_ITEM_DY + 4), buff2);
	}
}

void MV_Draw_Ext_List_Title(HDC hdc)
{
	char 			title[50];
	char			TempStr[50];
	RECT 			TmpRect;
	MV_stSatInfo	Temp_SatData;
	
	memset(title, 0, 50);
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	switch(chlist_Ext_servicelist_type)
	{
		case eCS_DB_TV_LIST:
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST), CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
		case eCS_DB_RADIO_LIST:
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST), CS_MW_LoadStringByIdx(CSAPP_STR_RD_LIST));
			break;
		case eCS_DB_FAV_TV_LIST:
			MV_DB_Get_Favorite_Name(TempStr, chlist_Ext_triplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST),TempStr);
			break;
		case eCS_DB_FAV_RADIO_LIST:
			MV_DB_Get_Favorite_Name(TempStr, chlist_Ext_triplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST),TempStr);
			break;
		case eCS_DB_SAT_TV_LIST:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Ext_Sat_index);
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST),Temp_SatData.acSatelliteName);
			break;
		case eCS_DB_SAT_RADIO_LIST:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Ext_Sat_index);
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST),Temp_SatData.acSatelliteName);
			break;
		default:
			sprintf(title, "%s %s", CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND_LIST),CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
	}

	//printf("\nMV_Draw_List_Title : === %d ==> %s\n\n", chlist_Ext_servicelist_type, title);

	TmpRect.left	= ScalerWidthPixel(CL_WINDOW_X);
	TmpRect.right	= TmpRect.left + ScalerWidthPixel(CL_WINDOW_DX);
	TmpRect.top		= ScalerWidthPixel(CL_WINDOW_TITLE_Y) + 4;
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(CL_WINDOW_ITEM_DY);
	CS_MW_DrawText(hdc, title, -1, &TmpRect, DT_CENTER);
}

void MV_Draw_Ext_List_Item(HDC hdc, int Count_index, U16 u16Focusindex, U8 FocusKind, U8 u8List_Kind)
{
	char				buff1[50];
	RECT				rc_service_name;
	RECT 				Scroll_Rect;
	tComboList_Element	SList_Ext_ComboList;
	/* By KB Kim 2011.01.20 */
	U8      			tvRadio;

	switch(u8List_Kind)
	{
		case CSAPP_CH_FIRST:
			SList_Ext_ComboList = SList_Ext_ComboList_First;
			break;
		case CSAPP_CH_SECOND:
			SList_Ext_ComboList = SList_Ext_ComboList_Second;
			break;
		case CSAPP_CH_THIRD:
			SList_Ext_ComboList = SList_Ext_ComboList_Third;
			break;
		default:
			SList_Ext_ComboList = SList_Ext_ComboList_First;
			break;
	}

	memset(buff1, 0, 50);
	CS_DB_GetCurrentList_ServiceData(&chlist_Ext_item_data, u16Focusindex);
	MV_DB_GetServiceDataByIndex(&chlist_Ext_service_data, chlist_Ext_item_data.Service_Index);

	// printf("%d, %04d : %d : %d : %s ======\n", Count_index, u16Focusindex, chlist_Ext_item_data.Service_Index, chlist_Ext_service_data.u16ChIndex, chlist_Ext_service_data.acServiceName);

	if ( FocusKind == FOCUS)
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element.x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element.dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	else if ( 0 == Count_index%2 )
	{
		SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(SList_Ext_ComboList.element.x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element.dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy));
	}
	else
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(SList_Ext_ComboList.element.x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element.dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy));
	}

	if ( FocusKind != NOTFOCUS )
	{
#ifdef DEBUG_TEST
		printf(" %d === %d === 1 === %s \n", u16Focusindex, chlist_Ext_item_data.Service_Index, chlist_Ext_service_data.acServiceName);
		{
			int 	i;
			for( i = 0 ; i < strlen(chlist_Ext_service_data.acServiceName) ; i++)
			{
				printf("%x ", chlist_Ext_service_data.acServiceName[i]);
			}
			printf("\n");
		}
#else
		//printf("============= 1 ========== %04d \n", u16Focusindex + 1);
#endif
		sprintf(buff1, "%04d", u16Focusindex + 1);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc, CSAPP_WHITE_COLOR);
		rc_service_name.left = ScalerWidthPixel(SList_Ext_ComboList.element_fields[0].x);
		rc_service_name.top = ScalerHeigthPixel( (SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index ) + 4 );
		rc_service_name.right = ScalerWidthPixel(SList_Ext_ComboList.element_fields[0].x + (12 * 13 ));
		rc_service_name.bottom = ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index + 28);
		//CS_MW_TextOut(hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[0].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index + 4), buff1);	
		MV_MW_DrawText_Fixed(hdc, buff1, -1, &rc_service_name, DT_LEFT);
		
		rc_service_name.left = ScalerWidthPixel(SList_Ext_ComboList.element_fields[1].x);
		rc_service_name.top = ScalerHeigthPixel( (SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index ) + 4 );
		rc_service_name.right = ScalerWidthPixel(SList_Ext_ComboList.element_fields[1].x + (12 * 12 ));
		rc_service_name.bottom = ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index + 28);
		//if ( u16Focusindex != 1822 && u16Focusindex != 1823 )
		MV_MW_DrawText_Fixed(hdc, chlist_Ext_service_data.acServiceName, -1, &rc_service_name, DT_LEFT);

		if(chlist_Ext_service_data.u8Scramble)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[2].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[2].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[2].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[2].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON]);
		}

		if(chlist_Ext_service_data.u8Lock)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[3].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[3].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_LOCK_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[3].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[3].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_NLOCK_ICON]);
		}

		/* By KB Kim 2011.01.20 */
		if (chlist_Ext_service_data.u8TvRadio == eCS_DB_RADIO_SERVICE)
		{
			tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
		}
		else
		{
			tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
		}
		if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, chlist_Ext_service_data.u16ChIndex) < MV_MAX_FAV_KIND )
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[4].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[4].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_FAVORITE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[4].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[4].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_NFAVORITE_ICON]);
		}

		if( chlist_Ext_service_data.u8TvRadio == eCS_DB_HDTV_SERVICE )
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[5].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[5].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_HD_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_Ext_ComboList.element_fields[5].x), ScalerHeigthPixel(SList_Ext_ComboList.element.y + SList_Ext_ComboList.element.dy * Count_index), ScalerWidthPixel(SList_Ext_ComboList.element_fields[5].dx), ScalerHeigthPixel(SList_Ext_ComboList.element.dy), &MV_BMP[MVBMP_CHLIST_NHD_ICON]);
		}
	}

	if ( FocusKind == FOCUS )
	{
		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Ext_Current_Service, chlist_Ext_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Ext_List_Full_Item_1(HDC hdc)
{
	int 	i;
	U16		index;
	RECT 	Scroll_Rect;

	for(i = 0 ; i < SERVICES_NUM_PER_PAGE ; i++)
	{
		index = chlist_Ext_Current_Page * SERVICES_NUM_PER_EXPAGE + i;

		//printf("\n === CSAPP_CH_FIRST :: INDEX : %d ============\n", index);
		
		if(index < chlist_Ext_Total_Service)
		{
			if(chlist_Ext_Current_Focus == i)
				MV_Draw_Ext_List_Item(hdc, i, index, FOCUS, CSAPP_CH_FIRST);
			else 
				MV_Draw_Ext_List_Item(hdc, i, index, UNFOCUS, CSAPP_CH_FIRST);
		}	
		else
		{
			MV_Draw_Ext_List_Item(hdc, i, index, NOTFOCUS, CSAPP_CH_FIRST);
		}
	}	

	if (chlist_Ext_Total_Service == 0)
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_Ext_ComboList_First.element_fields[1].x), ScalerHeigthPixel(SList_Ext_ComboList_First.element.y+4),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));

		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Ext_Current_Service, chlist_Ext_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Ext_List_Full_Item_2(HDC hdc)
{
	int 	i;
	U16		index;
	RECT 	Scroll_Rect;

	for(i = 0 ; i < SERVICES_NUM_PER_PAGE ; i++)
	{
		index = ( chlist_Ext_Current_Page) * SERVICES_NUM_PER_EXPAGE + i + SERVICES_NUM_PER_PAGE;

		//printf("\n === CSAPP_CH_SECOND :: INDEX : %d ============\n", index);
		
		if(index < chlist_Ext_Total_Service)
		{
			if(chlist_Ext_Current_Focus == i + SERVICES_NUM_PER_PAGE)
				MV_Draw_Ext_List_Item(hdc, i, index, FOCUS, CSAPP_CH_SECOND);
			else 
				MV_Draw_Ext_List_Item(hdc, i, index, UNFOCUS, CSAPP_CH_SECOND);
		}	
		else
		{
			MV_Draw_Ext_List_Item(hdc, i, index, NOTFOCUS, CSAPP_CH_SECOND);
		}
	}	

	if (chlist_Ext_Total_Service == 0)
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_Ext_ComboList_Second.element_fields[1].x), ScalerHeigthPixel(SList_Ext_ComboList_Second.element.y+4),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));

		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Ext_Current_Service, chlist_Ext_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Ext_List_Full_Item_3(HDC hdc)
{
	int 	i;
	U16		index;
	RECT 	Scroll_Rect;

	for(i = 0 ; i < SERVICES_NUM_PER_PAGE ; i++)
	{
		index = ( chlist_Ext_Current_Page) * SERVICES_NUM_PER_EXPAGE + i + SERVICES_NUM_PER_PAGE*2;

		//printf("\n === CSAPP_CH_THIRD :: INDEX : %d ============\n", index);
		
		if(index < chlist_Ext_Total_Service)
		{
			if(chlist_Ext_Current_Focus == i + SERVICES_NUM_PER_PAGE*2)
				MV_Draw_Ext_List_Item(hdc, i, index, FOCUS, CSAPP_CH_THIRD);
			else 
				MV_Draw_Ext_List_Item(hdc, i, index, UNFOCUS, CSAPP_CH_THIRD);
		}	
		else
		{
			MV_Draw_Ext_List_Item(hdc, i, index, NOTFOCUS, CSAPP_CH_THIRD);
		}
	}	

	if (chlist_Ext_Total_Service == 0)
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_Ext_ComboList_Third.element_fields[1].x), ScalerHeigthPixel(SList_Ext_ComboList_Third.element.y+4),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));

		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Ext_Current_Service, chlist_Ext_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Ext_CH_List(HDC hdc)
{
//	MV_Draw_Ext_CH_List_Window(hdc);
//	MV_Draw_Ext_List_Title(hdc);
	MV_Draw_Ext_List_Full_Item_1(hdc);
	MV_Draw_Ext_List_Full_Item_2(hdc);
	MV_Draw_Ext_List_Full_Item_3(hdc);
	MV_Draw_Ext_CH_Info(hdc);
	//CS_MW_SetSmallWindow((352),(112),(320),(240));   /* Use PIG */
}

void MV_Draw_Ext_CH_Only_List(HDC hdc)
{
	MV_Draw_Ext_List_Full_Item_1(hdc);
	MV_Draw_Ext_List_Full_Item_2(hdc);
	MV_Draw_Ext_List_Full_Item_3(hdc);
	MV_Draw_Ext_CH_Info(hdc);
	//CS_MW_SetSmallWindow((352),(112),(320),(240));   /* Use PIG */
}

void MV_Draw_Ext_CH_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind, U8 u8Ch_Focus)
{
	HDC		hdc;
	int 	Count_index;
	
	hdc = BeginPaint(hwnd);
	
	Count_index = u16Focusindex%SERVICES_NUM_PER_PAGE;

	if ( chlist_Ext_Prev_Page != chlist_Ext_Current_Page && FocusKind == FOCUS )
	{
		switch(u8Ch_Focus)
		{
			case CSAPP_CH_FIRST:
				MV_Draw_Ext_List_Full_Item_1(hdc);
				break;
			case CSAPP_CH_SECOND:
				MV_Draw_Ext_List_Full_Item_2(hdc);
				break;
			case CSAPP_CH_THIRD:
				MV_Draw_Ext_List_Full_Item_3(hdc);
				break;
			default:
				MV_Draw_Ext_List_Full_Item_1(hdc);
				break;
		}
	}
	else
		MV_Draw_Ext_List_Item(hdc, Count_index, u16Focusindex, FocusKind, u8Ch_Focus);
	
	if ( FocusKind == FOCUS )
		MV_Draw_Ext_CH_Info(hdc);
	
	EndPaint(hwnd,hdc);
}

void MV_Set_Ext_Current_List(U32 chlist_type, U8 u8Satlist_Sat_Index)
{
	switch ( chlist_type )
	{
		case eCS_DB_RADIO_LIST:
			chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case eCS_DB_TV_LIST:
			chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case eCS_DB_FAV_TV_LIST:
			chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case eCS_DB_FAV_RADIO_LIST:
			chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case eCS_DB_SAT_TV_LIST:
			if ( u8Satlist_Sat_Index == 255 )
			{
				chlist_Ext_servicelist_type = eCS_DB_TV_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		case eCS_DB_SAT_RADIO_LIST:
			if ( u8Satlist_Sat_Index == 255 )
			{
				chlist_Ext_servicelist_type = eCS_DB_RADIO_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				chlist_Ext_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
				chlist_Ext_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		default:
			break;
	}
	
	chlist_Ext_Total_Service = CS_DB_GetListServiceNumber(chlist_Ext_triplet);
	//printf("=== TOTAL : %d , u8Satlist_Sat_Index : %d , chlist_Ext_servicelist_type : %d \n", chlist_Ext_Total_Service, u8Satlist_Sat_Index, chlist_Ext_servicelist_type);

	if( chlist_Ext_Total_Service > 0 )
	{
		CS_DB_SetCurrentList(chlist_Ext_triplet, FALSE);
	}
}

CSAPP_Applet_t CSApp_Ext_SList(CSAPP_Applet_t   slist_type)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;	

	CSApp_Ext_SList_Applets = CSApp_Applet_Error;

	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);

	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "Ext_slist window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Ext_SList_Msg_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_transparent;
	CreateInfo.dwAddData 		= slist_type;
	CreateInfo.hHosting 		= HWND_DESKTOP;

	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);

	return CSApp_Ext_SList_Applets;

}

int Ext_SList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC			hdc;
	BOOL		SListPaintFlag = TRUE;
	U16			Prev_Service = 0;

	switch (message)
	{
		case MSG_CREATE:
			{
				tCS_DBU_Service ServiceTriplet;
				
				ComboList_Create(&SList_Ext_ComboList_First, SERVICES_NUM_PER_PAGE);
				ComboList_Create(&SList_Ext_ComboList_Second, SERVICES_NUM_PER_PAGE);
				ComboList_Create(&SList_Ext_ComboList_Third, SERVICES_NUM_PER_PAGE);
				
				//Service_Unlocked = FALSE;
				SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, 1000);

				//chlist_Ext_servicelist_type = GetWindowAdditionalData(hwnd);

				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));

				chlist_Ext_servicelist_type = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType;
				
				u8Ext_Sat_index = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
				
				MV_Set_Ext_Current_List(chlist_Ext_servicelist_type, u8Ext_Sat_index);
				
				Ext_back_triplet = CS_DB_GetLastServiceTriplet();

				if(chlist_Ext_Total_Service>0)
				{
					chlist_Ext_Current_Service = CS_DB_GetCurrentService_OrderIndex();  

					chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);
					Prev_Service = chlist_Ext_Current_Service;

					//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
				}	
				//printf("\n========== EXT_CH_List : %d ============\n", chlist_Ext_servicelist_type);
			}
			break;

		case MSG_TIMER:
			if(wparam == CHECK_SIGNAL_TIMER_ID)
			{
				U8  		quality;
				U8  		level;
				BOOL 		lock;

				CS_INSTALL_GetSignalInfo(&quality, &level, &lock);
/*
				hdc =BeginPaint(hwnd);
				if (chlist_Ext_Total_Service != 0)
				{
					if(lock)
					{
						SetBrushColor(hdc, RGB2Pixel(hdc,0x00, 0xff, 0x00));
						FillBox(hdc,ScalerWidthPixel(550), ScalerHeigthPixel(390),ScalerWidthPixel(20),ScalerHeigthPixel(20));
					}
					else
					{
						SetBrushColor(hdc, COLOR_red);
						FillBox(hdc,ScalerWidthPixel(550), ScalerHeigthPixel(390),ScalerWidthPixel(20),ScalerHeigthPixel(20));
					}
				}

				EndPaint(hwnd,hdc);
*/

			}

			break;
			   
 		case MSG_PAINT:	
			hdc = BeginPaint(hwnd);
			MV_Draw_Ext_CH_List_Window(hdc);
			MV_Draw_Ext_List_Title(hdc);
			EndPaint(hwnd,hdc);
			
			hdc = BeginPaint(hwnd);
			MV_Draw_Ext_CH_List(hdc);
			EndPaint(hwnd,hdc);
			//SendMessage (hwnd, MSG_UPDATEPAGE, 0, 0);
			return 0;
			break;
            
		case MSG_VIDEO_FORFMAT_UPDATE:
			{
				
			}
			break;
			
		case MSG_CHECK_SERVICE_LOCK:

			if ( CS_MW_GetServicesLockStatus() && ( chlist_Ext_item_data.Lock_Flag == eCS_DB_LOCKED ) && ( chlist_Ext_Current_Service != CS_APP_GetLastUnlockServiceIndex() ) )
			{
				if(wparam == 1)
					SendMessage(hwnd, MSG_PAINT, 0, 0);
				else
					SendMessage(hwnd, MSG_PIN_INPUT, 0, 0);
				CS_MW_StopService(TRUE);
			} else {
				// CS_DB_SetLastServiceTriplet();				
				CS_DB_GetCurrentList_ServiceData(&chlist_Ext_item_data, chlist_Ext_Current_Service);
				MV_DB_GetServiceDataByIndex(&chlist_Ext_service_data, chlist_Ext_item_data.Service_Index);

				CS_APP_SetLastUnlockServiceIndex(chlist_Ext_Current_Service);
				SendMessage (hwnd, MSG_PLAYSERVICE, 0, 0);
			}

			break;

		case MSG_PLAYSERVICE:
			{	
				tCS_DBU_Service ServiceTriplet;
				
				CS_DB_SetLastServiceTriplet();
				CS_DB_GetCurrentList_ServiceData(&chlist_Ext_item_data, chlist_Ext_Current_Service);
				CS_DB_SetCurrentService_OrderIndex(chlist_Ext_Current_Service);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));				
				ServiceTriplet.sCS_DBU_ServiceIndex =  chlist_Ext_Current_Service;
				
				CS_DBU_SaveCurrentService(ServiceTriplet);

				Ext_back_triplet = ServiceTriplet;

				if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
				{
					FbSendFndDisplayNum((unsigned)chlist_Ext_Current_Service+1);
				}
				else
				{
					FbSendFndDisplayNum((unsigned)chlist_Ext_item_data.LCN);
				}

				if (wparam)
					CS_MW_PlayServiceByIdx(chlist_Ext_item_data.Service_Index, NOT_TUNNING);
				else
					CS_MW_PlayServiceByIdx(chlist_Ext_item_data.Service_Index, RE_TUNNING);
			}
			break;

		case MSG_CLOSE:
			SListPaintFlag = TRUE;
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			ComboList_Destroy();
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			DestroyMainWindow (hwnd);                            
			PostQuitMessage (hwnd);
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
		case MSG_PIN_INPUT:
			bExt_Ch_Lock_Flag = TRUE;
			MV_Draw_Password_Window(hwnd);
			break;
	
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
				CSApp_Ext_SList_Applets = CSApp_Applet_Sleep;
				SendMessage(hwnd,MSG_CLOSE,0,0);
				break;
				
			case CSAPP_KEY_TV_AV:
				ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				break;
					
			}

			if (MV_Get_Password_Flag() == TRUE)
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
							if ( bExt_Ch_Lock_Flag == TRUE )
							{
								bExt_Ch_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								
								CS_APP_SetLastUnlockServiceIndex(chlist_Ext_Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							else 
							{
								switch( chlist_Ext_servicelist_type )
								{	
									case CSApp_Applet_TVSATList:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditTVSAT;
										break;								
									case CSApp_Applet_TVList:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
										break;
									case CSApp_Applet_TVFAVList:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditTVFAV;
										break;
									case CSApp_Applet_RDList:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditRadio;
										break;
									case CSApp_Applet_RADIOFAVList:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOFAV;
										break;
									case CSApp_Applet_RADIOSATList:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOSAT;
										break;
									default:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
										break;
								}
								bExt_Ch_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						}
						break;
						
					case CSAPP_KEY_ENTER:
						if(MV_Password_Retrun_Value() == TRUE)
						{
							if ( bExt_Ch_Lock_Flag == TRUE )
							{
								bExt_Ch_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								CS_APP_SetLastUnlockServiceIndex(chlist_Ext_Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							else 
							{
								switch( chlist_Ext_servicelist_type )
								{	
									case eCS_DB_SAT_TV_LIST:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditTVSAT;
										break;								
									case eCS_DB_TV_LIST:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
										break;
									case eCS_DB_FAV_TV_LIST:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditTVFAV;
										break;
									case eCS_DB_RADIO_LIST:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditRadio;
										break;
									case eCS_DB_FAV_RADIO_LIST:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOFAV;
										break;
									case eCS_DB_SAT_RADIO_LIST:
										CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOSAT;
										break;
									default:
										CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
										break;
								}							
								bExt_Ch_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						}
						break;

					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:							
						bExt_Ch_Lock_Flag = FALSE;
						MV_Password_Set_Flag(FALSE);
						hdc = BeginPaint(hwnd);
						MV_Restore_PopUp_Window( hdc );
						EndPaint(hwnd,hdc);
						break;
				}
				break;
			}

			if(MV_Get_Jump_Flag() == TRUE)
			{
				//printf(" Jump ================\n");
				MV_Jump_Proc(hwnd, wparam);
				if ( wparam == CSAPP_KEY_ENTER )
				{
					U16		u16Jump_Index;
					char	acReturn[5];

					MV_Jump_Retrun_Value( acReturn );

					u16Jump_Index = atoi( acReturn );

					if ( acReturn[0] == 0x00 || u16Jump_Index == 0 )
						break;
					
					if ( u16Jump_Index < chlist_Ext_Total_Service )
					{
						chlist_Ext_Current_Service = u16Jump_Index - 1;
						
						chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);
#if 0
						hdc = BeginPaint(hwnd);
						switch(u8Channel_Focus)
						{
							case CSAPP_CH_FIRST:
								MV_Draw_Ext_List_Full_Item_1(hdc);
								break;
							case CSAPP_CH_SECOND:
								MV_Draw_Ext_List_Full_Item_2(hdc);
								break;
							case CSAPP_CH_THIRD:
								MV_Draw_Ext_List_Full_Item_3(hdc);
								break;
							default:
								MV_Draw_Ext_List_Full_Item_1(hdc);
								break;
						}
						MV_Draw_Ext_CH_Info(hdc);
						EndPaint(hwnd,hdc);
#else
						hdc = BeginPaint(hwnd);
						MV_Draw_Ext_CH_List(hdc);
						EndPaint(hwnd,hdc);
#endif
					} else {
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_INVALID_CHNUM);
						EndPaint(hwnd,hdc);
						
						usleep( 2000 * 1000 );
						
						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}
				break;
			}

			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				//printf(" PopUp ================\n");
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();
					
					switch(MV_Get_PopUp_Window_Kind())
					{
						case eMV_TITLE_SORT:
							{
								tCS_DBU_Service ServiceTriplet;
								
								CS_DB_GetCurrentList_ServiceData(&chlist_Ext_item_data, chlist_Ext_Current_Service);

								hdc=BeginPaint(hwnd);
								MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
								EndPaint(hwnd,hdc);
								
								CS_DB_SortCurrentServiceList(u8Result_Value);

								if ( u8Result_Value == eCS_DB_BY_NORMAL )
									CS_DB_SortCurrentServiceList(u8Result_Value);

								usleep( 500 * 1000 );

								hdc=BeginPaint(hwnd);
								Close_Msg_Window(hdc);
								EndPaint(hwnd,hdc);

								chlist_Ext_Current_Service = CS_DB_GetCurrent_Service_By_ServiceIndex(chlist_Ext_item_data.Service_Index);
								Ext_back_triplet.sCS_DBU_ServiceIndex = chlist_Ext_Current_Service;
								chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

								CS_DB_SetCurrentService_OrderIndex(chlist_Ext_Current_Service);
								CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
								ServiceTriplet.sCS_DBU_ServiceIndex =  chlist_Ext_Current_Service;
								CS_DBU_SaveCurrentService(ServiceTriplet);

								/* For Recall List problem By KB Kim : 2011.08.30 */
								/* Reset Recall List After Sort */
								CS_DB_SetLastServiceTriplet();
								MV_Reset_ReCall_List();

								if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
								{
									FbSendFndDisplayNum((unsigned)chlist_Ext_Current_Service+1);
								}
								else
								{
									FbSendFndDisplayNum((unsigned)chlist_Ext_item_data.LCN);
								}
								
								if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
									u8Channel_Focus = CSAPP_CH_FIRST;
								else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
									u8Channel_Focus = CSAPP_CH_SECOND;
								else
									u8Channel_Focus = CSAPP_CH_THIRD;
								
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_List(hdc);
								MV_Draw_Ext_CH_Info(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_SAT:
							{
								if ( u8Result_Value == 0 )
									u8Ext_Sat_index = 255;
								else
									u8Ext_Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
								
								//printf("\nu8Result_Value :: %d , u8Ext_Sat_index : %d =====\n\n", u8Result_Value, u8Ext_Sat_index);
								
								switch(chlist_Ext_servicelist_type)
								{
									case eCS_DB_TV_LIST:
									case eCS_DB_SAT_TV_LIST:
									case eCS_DB_FAV_TV_LIST:
										chlist_Ext_servicelist_type = eCS_DB_SAT_TV_LIST;
										break;
									case eCS_DB_RADIO_LIST:
									case eCS_DB_SAT_RADIO_LIST:
									case eCS_DB_FAV_RADIO_LIST:
										chlist_Ext_servicelist_type = eCS_DB_SAT_RADIO_LIST;
										break;
									default:
										break;
								}

								//printf("\n chlist_Ext_servicelist_type : %d\n", chlist_Ext_servicelist_type);
								MV_Set_Ext_Current_List(chlist_Ext_servicelist_type, u8Ext_Sat_index);

								if( chlist_Ext_Total_Service > 0 )
								{
									chlist_Ext_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

									chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);
									Prev_Service = chlist_Ext_Current_Service;

									if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
										u8Channel_Focus = CSAPP_CH_FIRST;
									else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
										u8Channel_Focus = CSAPP_CH_SECOND;
									else
										u8Channel_Focus = CSAPP_CH_THIRD;

									//if(APP_GetMainMenuStatus())
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
								}
								
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_List(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_FAV:
							{
								U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;

								if ( u8Result_Value == 0 )
									u8Ext_Sat_index = 255;
								else
									u8Ext_Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - 1);

								switch(chlist_Ext_servicelist_type)
								{
									case eCS_DB_TV_LIST:
									case eCS_DB_SAT_TV_LIST:
									case eCS_DB_FAV_TV_LIST:
										if ( u8Ext_Sat_index == 255 )
											chlist_Ext_servicelist_type = eCS_DB_TV_LIST;
										else
											chlist_Ext_servicelist_type = eCS_DB_FAV_TV_LIST;
										u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
										break;
									case eCS_DB_RADIO_LIST:
									case eCS_DB_SAT_RADIO_LIST:
									case eCS_DB_FAV_RADIO_LIST:
										if ( u8Ext_Sat_index == 255 )
											chlist_Ext_servicelist_type = eCS_DB_RADIO_LIST;
										else
											chlist_Ext_servicelist_type = eCS_DB_FAV_RADIO_LIST;
										u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
										break;
									default:
										break;
								}
								
								MV_Set_Ext_Current_List(chlist_Ext_servicelist_type, u8Ext_Sat_index);

								if(chlist_Ext_Total_Service>0)
								{
									chlist_Ext_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

									chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);
									Prev_Service = chlist_Ext_Current_Service;

									if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
										u8Channel_Focus = CSAPP_CH_FIRST;
									else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
										u8Channel_Focus = CSAPP_CH_SECOND;
									else
										u8Channel_Focus = CSAPP_CH_THIRD;

									//if(APP_GetMainMenuStatus())
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
								}
								
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_List(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_SAT_FAV:
							break;
							
						default:
							break;
					}
				}
				break;
			}
			else
			{
				if ((chlist_Ext_Total_Service == 0) && (wparam != CSAPP_KEY_TVRADIO) && (wparam != CSAPP_KEY_MENU) && (wparam != CSAPP_KEY_ESC)) break;
                                
				switch(wparam)
				{
					case CSAPP_KEY_GREEN:
						if (CS_DBU_GetParentalLockStatus() && CS_DBU_GetEditLockStatus())
						{
							MV_Draw_Password_Window(hwnd);
						} else {
							switch( chlist_Ext_servicelist_type )
							{	
								case eCS_DB_SAT_TV_LIST:
									CSApp_Ext_SList_Applets = CSApp_Applet_EditTVSAT;
									break;								
								case eCS_DB_TV_LIST:
									CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
									break;
								case eCS_DB_FAV_TV_LIST:
									CSApp_Ext_SList_Applets = CSApp_Applet_EditTVFAV;
									break;
								case eCS_DB_RADIO_LIST:
									CSApp_Ext_SList_Applets = CSapp_Applet_EditRadio;
									break;
								case eCS_DB_FAV_RADIO_LIST:
									CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOFAV;
									break;
								case eCS_DB_SAT_RADIO_LIST:
									CSApp_Ext_SList_Applets = CSApp_Applet_EditRADIOSAT;
									break;
								default:
									CSApp_Ext_SList_Applets = CSapp_Applet_EditTV;
									break;
							}
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
						break;
						
					case CSAPP_KEY_YELLOW:
						{
							stPopUp_Window 			stWindow;
							stPopUp_Window_Contents stContents;
							int						i;

							stWindow.b8Scroll_OnOff = FALSE;
							stWindow.b8YesNo_Button = FALSE;
							stWindow.u8Help_Kind = 0;
							stWindow.u16Total = SORT_NUM;
							stWindow.u16Current_Pos = 0;
							stWindow.u8Item_Num = SORT_NUM;
							stWindow.tTitle = eMV_TITLE_SORT;
							stWindow.Window_Rect.top = 200;
							stWindow.Window_Rect.left = 500;
							stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( CL_WINDOW_ITEM_DY * SORT_NUM ) + 58));
							stWindow.Window_Rect.right = 780;

							for ( i = 0 ; i < SORT_NUM ; i++ )
							{
								sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Ext_Sort_Str[i]));
							}
							stContents.u8TotalCount = SORT_NUM;
							
							MV_Draw_PopUp_Window( hwnd, stWindow, &stContents );
						}
						break;

					case CSAPP_KEY_RED:
						CSApp_Ext_SList_Applets = CSAPP_Applet_Finder;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;

					case CSAPP_KEY_BLUE:
						switch( chlist_Ext_servicelist_type )
						{
							case eCS_DB_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_TVList;
								break;
							case eCS_DB_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_RDList;
								break;
							case eCS_DB_FAV_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_RADIOFAVList;
								break;
							case eCS_DB_FAV_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_TVFAVList;
								break;
							case eCS_DB_SAT_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_RADIOSATList;
								break;
							case eCS_DB_SAT_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_TVSATList;
								break;
							default:
								break;
						}
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_TVRADIO:
						switch( chlist_Ext_servicelist_type )
						{
							case eCS_DB_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_TVList;
								break;
							case eCS_DB_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_RDList;
								break;
							case eCS_DB_FAV_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_RADIOFAVList;
								break;
							case eCS_DB_FAV_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_TVFAVList;
								break;
							case eCS_DB_SAT_RADIO_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_RADIOSATList;
								break;
							case eCS_DB_SAT_TV_LIST:
								CSApp_Ext_SList_Applets = CSApp_Applet_Ext_TVSATList;
								break;
							default:
								break;
						}
						
						MV_Set_Ext_Current_List(chlist_Ext_servicelist_type, u8Ext_Sat_index);

						if(chlist_Ext_Total_Service>0)
						{
							chlist_Ext_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);
							Prev_Service = chlist_Ext_Current_Service;

							if(APP_GetMainMenuStatus())
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
						}
						
						hdc = BeginPaint(hwnd);
						MV_Draw_Ext_CH_List(hdc);
						EndPaint(hwnd,hdc);
						
						if ( CFG_Yinhe_Test )
						{
							usleep( 2000 * 1000 );
							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_TVRADIO, 0);
						}
						break;
						
					case CSAPP_KEY_SAT:
						MV_Draw_Satlist_Window(hwnd);
						break;

					case CSAPP_KEY_FAVOLIST:
						{
							U8	Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
							
							switch(chlist_Ext_servicelist_type)
							{								
								case eCS_DB_FAV_RADIO_LIST:
								case eCS_DB_SAT_RADIO_LIST:
								case eCS_DB_RADIO_LIST:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case eCS_DB_TV_LIST:
								case eCS_DB_SAT_TV_LIST:
								case eCS_DB_FAV_TV_LIST:
									Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								default:
									break;								
							}
							
							MV_Draw_Favlist_Window(hwnd, Service_Type, TRUE);
						}
						break;

					case CSAPP_KEY_F2: 
						{
							RECT	jump_rect;

							jump_rect.top = JUMP_WINDOW_STARTY;
							jump_rect.left = JUMP_WINDOW_STARTX;
							jump_rect.right = JUMP_WINDOW_STARTX + JUMP_WINDOW_STARTDX;
							jump_rect.bottom = JUMP_WINDOW_STARTY + JUMP_WINDOW_STARTDY;
							MV_Draw_Jump_Window(hwnd, &jump_rect);
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
						if (MV_Get_Password_Flag() != TRUE)
						{
							RECT	jump_rect;

							jump_rect.top = JUMP_WINDOW_STARTY;
							jump_rect.left = JUMP_WINDOW_STARTX;
							jump_rect.right = JUMP_WINDOW_STARTX + JUMP_WINDOW_STARTDX;
							jump_rect.bottom = JUMP_WINDOW_STARTY + JUMP_WINDOW_STARTDY;
							MV_Draw_Jump_Window(hwnd, &jump_rect);
							MV_Jump_Proc(hwnd, wparam);
						}
						break;
						
					case CSAPP_KEY_DOWN:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, UNFOCUS, u8Channel_Focus);

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;
							
							if(chlist_Ext_Current_Service == chlist_Ext_Total_Service-1)
								chlist_Ext_Current_Service = 0;
							else
								chlist_Ext_Current_Service++;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							if ( chlist_Ext_Prev_Page != chlist_Ext_Current_Page )
							{
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_Only_List(hdc);
								EndPaint(hwnd,hdc);
							}
							else
							{
								//hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, FOCUS, u8Channel_Focus);
								//MV_Draw_Ext_CH_List(hdc);
								//EndPaint(hwnd,hdc);
								//SendMessage(hwnd, MSG_PAINT, 0, 0);
							}
						}
						break;
					case CSAPP_KEY_UP:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, UNFOCUS, u8Channel_Focus);

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;

							if(chlist_Ext_Current_Service == 0)
								chlist_Ext_Current_Service = chlist_Ext_Total_Service-1;
							else
								chlist_Ext_Current_Service--;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							if ( chlist_Ext_Prev_Page != chlist_Ext_Current_Page )
							{
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_Only_List(hdc);
								EndPaint(hwnd,hdc);
							}
							else
							{
								MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, FOCUS, u8Channel_Focus);
							}
						}
						break;

					case CSAPP_KEY_LEFT:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, UNFOCUS, u8Channel_Focus);
							
							if ( chlist_Ext_Current_Service < SERVICES_NUM_PER_PAGE )
								chlist_Ext_Current_Service = chlist_Ext_Total_Service-1;
							else 
								chlist_Ext_Current_Service -= SERVICES_NUM_PER_PAGE;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							if ( chlist_Ext_Prev_Page != chlist_Ext_Current_Page )
							{
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_Only_List(hdc);
								EndPaint(hwnd,hdc);
							}
							else
							{
								MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, FOCUS, u8Channel_Focus);
							}
						}
						break;

					case CSAPP_KEY_RIGHT:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, UNFOCUS, u8Channel_Focus);
							
							chlist_Ext_Current_Service += SERVICES_NUM_PER_PAGE;
							if( chlist_Ext_Current_Service > chlist_Ext_Total_Service-1 )
								chlist_Ext_Current_Service = 0;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							if ( chlist_Ext_Prev_Page != chlist_Ext_Current_Page )
							{
								hdc = BeginPaint(hwnd);
								MV_Draw_Ext_CH_Only_List(hdc);
								EndPaint(hwnd,hdc);
							}
							else
							{
								MV_Draw_Ext_CH_Focus(hwnd, chlist_Ext_Current_Service, FOCUS, u8Channel_Focus);
							}
						}
						break;

					case CSAPP_KEY_PG_DOWN:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;
							
							chlist_Ext_Current_Service += SERVICES_NUM_PER_EXPAGE;
							if( chlist_Ext_Current_Service > chlist_Ext_Total_Service-1 )
								chlist_Ext_Current_Service = 0;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							hdc = BeginPaint(hwnd);
							MV_Draw_Ext_CH_Only_List(hdc);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_PG_UP:
						{
							if(chlist_Ext_Total_Service == 0)
								break;

							chlist_Ext_Prev_Page = chlist_Ext_Current_Page;
							
							if( chlist_Ext_Current_Service < SERVICES_NUM_PER_EXPAGE )
								chlist_Ext_Current_Service = chlist_Ext_Total_Service-1;
							else
								chlist_Ext_Current_Service -= SERVICES_NUM_PER_EXPAGE;

							chlist_Ext_Current_Focus = get_focus_line(&chlist_Ext_Current_Page, chlist_Ext_Current_Service, SERVICES_NUM_PER_EXPAGE);

							if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 1 )
								u8Channel_Focus = CSAPP_CH_FIRST;
							else if ( ( chlist_Ext_Current_Focus/SERVICES_NUM_PER_PAGE ) < 2 )
								u8Channel_Focus = CSAPP_CH_SECOND;
							else
								u8Channel_Focus = CSAPP_CH_THIRD;

							hdc = BeginPaint(hwnd);
							MV_Draw_Ext_CH_Only_List(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
						
					case CSAPP_KEY_ENTER:
						//printf("== Prev : %d , Current : %d =====================\n", CS_DB_GetCurrentService_OrderIndex(), chlist_Ext_Current_Service);
						if ( chlist_Ext_Current_Service == CS_DB_GetCurrentService_OrderIndex() )
						{
							CSApp_Ext_SList_Applets = CSApp_Applet_Desktop;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						} else {
							if (Prev_Service != chlist_Ext_Current_Service)
							{
								CS_APP_SetLastUnlockServiceIndex(0xffff);
							}
							
							Prev_Service = chlist_Ext_Current_Service;
							SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						}
						break;
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_ESC:
							CSApp_Ext_SList_Applets = CSApp_Applet_Desktop;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						break;
					default:
						break;
				}
			}
			break;

			default:
        		break;
    	}
	
	return DefaultMainWinProc(hwnd, message, wparam, lparam);	
}


