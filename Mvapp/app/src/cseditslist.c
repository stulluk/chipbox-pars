#include "linuxos.h"
#include "database.h"
#include "ch_install.h"
#include "mwsvc.h"
#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "dvbtuner.h"
#include "ui_common.h"

#define	pin_box_dx				240
#define	pin_box_dy				110
#define	pin_box_base_x			(720-pin_box_dx)/2
#define	pin_box_base_y			(576-pin_box_dy)/2
#define	SERVICES_NUM_PER_PAGE	10
#define	FIELDS_PER_LINE			8
#define	IDC_PROMPTINFO 			100

static tComboList_Field_Rect	EditSLlist_ComboList_First_Line[FIELDS_PER_LINE] = {
										{CHEDIT_LIST_ITEM1_X, CHEDIT_LIST_ITEM1_DX},
										{CHEDIT_LIST_ITEM2_X, CHEDIT_LIST_ITEM2_DX},
										{CHEDIT_LIST_ITEM3_X, CHEDIT_LIST_ITEM3_DX},
										{CHEDIT_LIST_ITEM4_X, CHEDIT_LIST_ITEM4_DX},
										{CHEDIT_LIST_ITEM5_X, CHEDIT_LIST_ITEM5_DX},
										{CHEDIT_LIST_ITEM6_X, CHEDIT_LIST_ITEM6_DX},
										{CHEDIT_LIST_ITEM7_X, CHEDIT_LIST_ITEM7_DX},
										{CHEDIT_LIST_ITEM8_X, CHEDIT_LIST_ITEM8_DX}
									};

static tComboList_Element		EditSLlist_ComboList_First = {	
										{CHEDIT_LIST_LEFT, CHEDIT_LIST_TOP, CHEDIT_LIST_DX - 20, CHEDIT_LIST_ITEM_DY},
										FIELDS_PER_LINE,
										EditSLlist_ComboList_First_Line
									};

static 	CSAPP_Applet_t			CSApp_EditSLlist_Applets;
static 	BOOL 					show_confirm = FALSE;
static 	BOOL 					sat_change_confirm = FALSE;

extern U32						*Tuner_HandleId;
static TunerSignalState_t 		Siganl_State;
static U8						u8Sat_index = 0;
tCS_DB_ServiceListTriplet		chedit_triplet;
tCS_DB_ServiceListTriplet		list_triplet;
tCS_DBU_Service     			back_triplet;
static U16						Total_Service = 0;	
static U16						Current_Service = 0;
static U16						Current_Page = 0;
static U16						Current_Focus = 0;
static U32						servicelist_type;
BOOL							ChEdit_Move_Status = FALSE;
BOOL							ChEdit_Fav_Status = FALSE;
BOOL							b8Change_Name = FALSE;
static char						sReturn_str[MAX_SAT_NAME_LANGTH+1];
char							sBackup_str[MAX_SAT_NAME_LANGTH+1];
U16								u16Delete_Count = 0;
U16								u16Delete_Temp_Count = 0;
U16								u16Lock_Count = 0;
BITMAP							btWarning_cap;
BITMAP							btCap_bmp;
BOOL							bChEdit_Lock_Flag = FALSE;
MV_stServiceInfo				Ch_service_data;

//static  BOOL					Service_Unlocked = FALSE;

void MV_Draw_ChEdit_Signal(HDC hdc)
{	
	RECT	Temp_Rect;

	TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);
	//printf("\n ====== %d % ==== %d % ======\n", Siganl_State.Strength, Siganl_State.Quality);

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	Temp_Rect.top 		= MV_PIG_BOTTOM + 12;
	Temp_Rect.bottom	= Temp_Rect.top + MV_BMP[MVBMP_RED_SIGNAL].bmHeight;
	Temp_Rect.left		= MV_PIG_LEFT + 24;
	Temp_Rect.right		= MV_PIG_RIGHT;
	
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Strength, EN_ITEM_CHEDIT_SIGNAL_LEVEL);

	Temp_Rect.top 		= Temp_Rect.top + MV_INSTALL_SIGNAL_YGAP;
	Temp_Rect.bottom	= Temp_Rect.top + MV_BMP[MVBMP_RED_SIGNAL].bmHeight;
	MV_Draw_LevelBar(hdc, &Temp_Rect, Siganl_State.Quality, EN_ITEM_CHEDIT_SIGNAL_LEVEL);
}

void MV_Draw_ChEdit_List_State(HDC hdc)
{
	char 			TempStr[50];
	RECT			rc_service_name;
	MV_stSatInfo	Temp_SatData;

	switch(servicelist_type)
	{
		case CSapp_Applet_EditTV :
			sprintf( TempStr, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
		case CSapp_Applet_EditRadio :
			sprintf( TempStr, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_RD_LIST));
			break;
		case CSApp_Applet_EditTVFAV :
			MV_DB_Get_Favorite_Name( TempStr, chedit_triplet.sCS_DB_ServiceListTypeValue);
			break;
		case CSApp_Applet_EditRADIOFAV :
			MV_DB_Get_Favorite_Name( TempStr, chedit_triplet.sCS_DB_ServiceListTypeValue);
			break;
		case CSApp_Applet_EditTVSAT:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Sat_index);
			sprintf( TempStr, "%s" ,Temp_SatData.acSatelliteName);
			break;
		case CSApp_Applet_EditRADIOSAT:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Sat_index);
			sprintf( TempStr, "%s" ,Temp_SatData.acSatelliteName);
			break;
		default:
			sprintf( TempStr, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
	}
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_TITLE_LEFT), ScalerHeigthPixel(CHEDIT_TITLE_TOP), ScalerWidthPixel(CHEDIT_TITLE_DX), ScalerHeigthPixel(CHEDIT_TITLE_DY));
	
	rc_service_name.left = CHEDIT_TITLE_LEFT;
	rc_service_name.top = CHEDIT_TITLE_TOP + 2;
	rc_service_name.right = CHEDIT_TITLE_RIGHT;
	rc_service_name.bottom = CHEDIT_TITLE_BOTTOM;

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, TempStr, -1, &rc_service_name, DT_CENTER | DT_VCENTER );
}
	
void MV_Draw_ChEdit_Info_Bar(HDC hdc, U16 Item_index)
{
	char 						TempStr[100];
	MV_stServiceInfo			Tempservice_data;
	MV_stSatInfo				Tempsat_data;
	MV_stTPInfo					TempTP_data;
	tCS_DB_ServiceManageData 	item_data;

	CS_DB_GetCurrentList_ServiceData(&item_data, Item_index);
	MV_DB_GetServiceDataByIndex(&Tempservice_data, item_data.Service_Index);
	MV_DB_GetTPDataByIndex(&TempTP_data, Tempservice_data.u16TransponderIndex);
	MV_GetSatelliteData_ByIndex(&Tempsat_data, TempTP_data.u8SatelliteIndex);

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_INFO_LEFT), ScalerHeigthPixel(CHEDIT_INFO_TOP), ScalerWidthPixel(CHEDIT_INFO_DX), ScalerHeigthPixel(CHEDIT_INFO_DY));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_INFO_LEFT + 4),ScalerHeigthPixel(CHEDIT_INFO_TOP + 2),ScalerWidthPixel(MV_BMP[MVBMP_INFO_BANNER_INFO_ICON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_INFO_BANNER_INFO_ICON].bmHeight), &MV_BMP[MVBMP_INFO_BANNER_INFO_ICON]);

	if ( TempTP_data.u8Polar_H == P_H )
		sprintf( TempStr , "%04d %s : %s   %d/%s/%d  PID V:%d A:%d P:%d", Item_index+1, Tempservice_data.acServiceName, Tempsat_data.acSatelliteName, TempTP_data.u16TPFrequency, "H", TempTP_data.u16SymbolRate, Tempservice_data.u16VideoPid, Tempservice_data.u16AudioPid, Tempservice_data.u16PCRPid);
	else
		sprintf( TempStr , "%04d %s : %s   %d/%s/%d  PID V:%d A:%d P:%d", Item_index+1, Tempservice_data.acServiceName, Tempsat_data.acSatelliteName, TempTP_data.u16TPFrequency, "V", TempTP_data.u16SymbolRate, Tempservice_data.u16VideoPid, Tempservice_data.u16AudioPid, Tempservice_data.u16PCRPid);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_INFO_TEXT_X),ScalerHeigthPixel(CHEDIT_INFO_TOP + 6), TempStr);
}

void MV_Draw_ChEdit_List_Info_Bar(HDC hdc, U16 Item_index)
{
	char 						TempStr[64];
	MV_stSatInfo				Tempsat_data;
	MV_stTPInfo					TempTP_data;
	tCS_DB_ServiceManageData 	item_data;

	CS_DB_GetCurrentList_ServiceData(&item_data, Item_index);
	MV_DB_GetServiceDataByIndex(&Ch_service_data, item_data.Service_Index);
	MV_DB_GetTPDataByIndex(&TempTP_data, Ch_service_data.u16TransponderIndex);
	MV_GetSatelliteData_ByIndex(&Tempsat_data, TempTP_data.u8SatelliteIndex);

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_LIST_INFO_LEFT), ScalerHeigthPixel(CHEDIT_LIST_INFO_TOP), ScalerWidthPixel(CHEDIT_LIST_INFO_DX), ScalerHeigthPixel(CHEDIT_LIST_INFO_DY));

	if ( TempTP_data.u8Polar_H == P_H )
		sprintf( TempStr , "%s %d/%s/%d", Tempsat_data.acSatelliteName, TempTP_data.u16TPFrequency, "H", TempTP_data.u16SymbolRate);
	else
		sprintf( TempStr , "%s %d/%s/%d", Tempsat_data.acSatelliteName, TempTP_data.u16TPFrequency, "V", TempTP_data.u16SymbolRate);
	
	SetTextColor(hdc,MVAPP_GRAY_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_LIST_INFO_TEXT_X),ScalerHeigthPixel(CHEDIT_LIST_INFO_TOP + 6), TempStr);

	sprintf( TempStr , "PID V:%d A:%d P:%d", Ch_service_data.u16VideoPid, Ch_service_data.u16AudioPid, Ch_service_data.u16PCRPid);
	
	SetTextColor(hdc,MVAPP_GRAY_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_LIST_INFO_TEXT_X),ScalerHeigthPixel(CHEDIT_LIST_INFO_TOP + CHEDIT_LIST_ITEM_DY + 2), TempStr);
}

void MV_Draw_ChEdit_Help_Button(HDC hdc)
{
#if 1
	if ( btCap_bmp.bmHeight == 0 )
	{
		MV_GetBitmapFromDC (hdc, ScalerWidthPixel(CHEDIT_ICON1_X), ScalerHeigthPixel(CHEDIT_ICON_TOP - 6), ScalerWidthPixel(CHEDIT_ICON_DX), ScalerHeigthPixel(CHEDIT_ICON_DY + 6), &btCap_bmp);
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON1_X), ScalerHeigthPixel(CHEDIT_ICON_TOP - 6), ScalerWidthPixel(CHEDIT_ICON_DX), ScalerHeigthPixel(CHEDIT_ICON_DY + 6), &btCap_bmp);
	}
	
	//SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	//FillBox(hdc,ScalerWidthPixel(CHEDIT_ICON1_X), ScalerHeigthPixel(CHEDIT_ICON_TOP - 6), ScalerWidthPixel(CHEDIT_ICON_DX), ScalerHeigthPixel(CHEDIT_ICON_DY + 6));
#else
	SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_ICON1_X), ScalerHeigthPixel(CHEDIT_ICON_TOP), ScalerWidthPixel(CHEDIT_ICON_DX), ScalerHeigthPixel(CHEDIT_ICON_DY));
#endif
	
	if ( ChEdit_Move_Status == TRUE )
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON1_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON1_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_MOVE));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON2_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON2_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_SELECT));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON3_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_F2_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_F2_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON3_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_JUMP));
	} else if ( ChEdit_Fav_Status == TRUE ) {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON1_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_BLACK_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON1_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_FAV_KEY));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON2_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON2_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_SELECT));
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON1_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON1_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_MOVE));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON2_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON2_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
		
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON3_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON3_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_RENAME));
		
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON4_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON4_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_LOCK_KEY));
		
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON5_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_GRAY_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON5_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_FAV_KEY));
		
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_ICON6_X),ScalerHeigthPixel(CHEDIT_ICON_TOP),ScalerWidthPixel(MV_BMP[MVBMP_F2_BUTTON].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_F2_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CHEDIT_ICON6_TXT_X),ScalerHeigthPixel(CHEDIT_ICON_TOP), CS_MW_LoadStringByIdx(CSAPP_STR_JUMP));
	}
}

void MV_Draw_ChEdit_List_Item(HDC hdc, int Index_Count, U16 Item_index, U16 Total_Service, U8 u8Kind)
{
	RECT						rc_service_name;
	MV_stServiceInfo			service_data;
	tCS_DB_ServiceManageData 	item_data;
	char						buff[20];
	RECT						Scroll_Rect;
	/* By KB Kim 2011.01.20 */
	U8                          tvRadio;
	
	CS_DB_GetCurrentList_ServiceData(&item_data, Item_index);
	MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);
	
	if( u8Kind == FOCUS ) {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element.x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(EditSLlist_ComboList_First.element.dx), ScalerHeigthPixel(EditSLlist_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	} else if( 0 == Index_Count%2 ) {
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(EditSLlist_ComboList_First.element.x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(EditSLlist_ComboList_First.element.dx), ScalerHeigthPixel(EditSLlist_ComboList_First.element.dy));
	} else {
		SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(EditSLlist_ComboList_First.element.x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(EditSLlist_ComboList_First.element.dx), ScalerHeigthPixel(EditSLlist_ComboList_First.element.dy));
	}

	if ( Item_index < Total_Service )
	{
		if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
			sprintf(buff, "%04d", Item_index + 1);
		else
			sprintf( buff, "%03d", item_data.LCN);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc, CSAPP_WHITE_COLOR);
		CS_MW_TextOut(hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[0].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count + 4), buff);	

		rc_service_name.left = ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[1].x);
		rc_service_name.top = ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count + 4);
		rc_service_name.right = ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[1].x+ CHEDIT_LIST_ITEM2_DX);
		rc_service_name.bottom = ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count + CHEDIT_LIST_ITEM_DY);

		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_DrawText(hdc, service_data.acServiceName, -1, &rc_service_name, DT_LEFT);

		if ( ChEdit_Move_Status == TRUE )
		{
			if(item_data.Move_Flag == eDBASE_SELECT)
			{
				if( u8Kind == FOCUS )
					FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[2].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_MOVE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_MOVE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_MOVE_ICON]);
				else
					FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[2].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_MOVE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_MOVE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NMOVE_ICON]);
			}
		} 
		
		else if ( ChEdit_Fav_Status == TRUE )
		{
			if(item_data.Select_Flag == eDBASE_SELECT)
			{
				if( u8Kind == FOCUS )
					FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[2].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_CHECK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_CHECK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_CHECK_ICON]);
				else
					FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[2].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_NCHECK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_NCHECK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NCHECK_ICON]);
			}
		}

		if(service_data.u8Scramble)
		{
			if( u8Kind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[3].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[3].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON]);
		}

		if(item_data.Lock_Flag == eCS_DB_LOCKED)
		{
			if( u8Kind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[4].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_LOCK_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[4].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_LOCK_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NLOCK_ICON]);
		}
		
		/* By KB Kim 2011.01.20 */
		if (service_data.u8TvRadio == eCS_DB_RADIO_SERVICE)
		{
			tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
		}
		else
		{
			tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
		}
		if (MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, service_data.u16ChIndex) < MV_MAX_FAV_KIND)
		{
			if( u8Kind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[5].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_FAVORITE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_FAVORITE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_FAVORITE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[5].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_FAVORITE_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_FAVORITE_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NFAVORITE_ICON]);
		}

		if (service_data.u8TvRadio == eCS_DB_HDTV_SERVICE)
		{
			if( u8Kind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[6].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_HD_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_HD_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_HD_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[6].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_NHD_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_NHD_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NHD_ICON]);
		}
		
		if(item_data.Delete_Flag == eDBASE_DELETE)
		{
			if( u8Kind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[7].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_DEL_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_DEL_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_DEL_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[7].x), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y + EditSLlist_ComboList_First.element.dy * Index_Count), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_NDEL_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_NDEL_ICON].bmHeight), &MV_BMP[MVBMP_CHLIST_NDEL_ICON]);
		}
		
		if ( u8Kind == FOCUS )
		{
			Scroll_Rect.top = CHEDIT_SCROLL_TOP;
			Scroll_Rect.left = CHEDIT_SCROLL_LEFT;
			Scroll_Rect.right = CHEDIT_SCROLL_RIGHT;
			Scroll_Rect.bottom = CHEDIT_SCROLL_BOTTOM;
			MV_Draw_ScrollBar(hdc, Scroll_Rect, Item_index, Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);

			MV_Draw_ChEdit_List_Info_Bar(hdc, Item_index);
		}
	}
}

void MV_Draw_ChEdit_List(HDC hdc, U16 u16Current_Service, U16 u16Current_Page, U16 u16Current_Focus, U16 u16Total_Service )
{
	U16							index;
	int							i;

	i = u16Current_Service;  // Just Solve For Warning
	
 	for(i = 0 ; i < SERVICES_NUM_PER_PAGE ; i++)
	{
		index = ( u16Current_Page * SERVICES_NUM_PER_PAGE ) + i;
		
		if(index < u16Total_Service)
			if(u16Current_Focus == i) 
				MV_Draw_ChEdit_List_Item(hdc, i, index, u16Total_Service, FOCUS);
			else
				MV_Draw_ChEdit_List_Item(hdc, i, index, u16Total_Service, UNFOCUS);
		else
			MV_Draw_ChEdit_List_Item(hdc, i, index, u16Total_Service, UNFOCUS);
				
	}	

	if (u16Total_Service == 0)
	{
		SetTextColor(hdc,CSAPP_BLACK_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(EditSLlist_ComboList_First.element_fields[1].x-48), ScalerHeigthPixel(EditSLlist_ComboList_First.element.y),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));
	}
}	

void EditSList_Draw_Confirm(HDC hdc)
{
	RECT rc1;

	memset(&btWarning_cap, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC (hdc, ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY), &btWarning_cap);
	
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY));

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X + 3), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y + 3), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX - 6), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY - 6));

	rc1.top = CHEDIT_WARNING_WINDOW_TITLE_Y;
	rc1.left = CHEDIT_WARNING_WINDOW_TITLE_X;
	rc1.bottom = CHEDIT_WARNING_WINDOW_TITLE_Y + CHEDIT_WARNING_WINDOW_TITLE_DY;
	rc1.right = CHEDIT_WARNING_WINDOW_TITLE_X + CHEDIT_WARNING_WINDOW_TITLE_DX;
	MV_Draw_PopUp_Title_Bar(hdc, &rc1, eMV_TITLE_ATTENTION);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_CONTENT_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_CONTENT_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_CONTENT_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_CONTENT_DY));

	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	rc1.left = CHEDIT_WARNING_WINDOW_CONTENT_X; 
	rc1.top = CHEDIT_WARNING_WINDOW_CONTENT_Y + 20; 
	rc1.right = CHEDIT_WARNING_WINDOW_CONTENT_X + CHEDIT_WARNING_WINDOW_CONTENT_DX; 
	rc1.bottom = CHEDIT_WARNING_WINDOW_CONTENT_Y + CHEDIT_WARNING_WINDOW_CONTENT_ITEM_DY;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SAVEORNOT), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);
	
	rc1.top = rc1.top + 30; 
	rc1.bottom = rc1.top + CHEDIT_WARNING_WINDOW_CONTENT_DY - 50;
	
	SetBkColor (hdc, COLOR_transparent);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	InflateRect (&rc1, -1, -1);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SAVEHELP), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);
	
}

void EditSList_Draw_Saving(HDC hdc)
{
	RECT rc1;

	memset(&btWarning_cap, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC (hdc, ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY), &btWarning_cap);
	
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY));

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X + 3), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y + 3), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX - 6), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY - 6));

	rc1.top = CHEDIT_WARNING_WINDOW_TITLE_Y;
	rc1.left = CHEDIT_WARNING_WINDOW_TITLE_X;
	rc1.bottom = CHEDIT_WARNING_WINDOW_TITLE_Y + CHEDIT_WARNING_WINDOW_TITLE_DY;
	rc1.right = CHEDIT_WARNING_WINDOW_TITLE_X + CHEDIT_WARNING_WINDOW_TITLE_DX;
	MV_Draw_PopUp_Title_Bar(hdc, &rc1, eMV_TITLE_ATTENTION);
	
	SetBrushColor(hdc, MVAPP_BLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(CHEDIT_WARNING_WINDOW_CONTENT_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_CONTENT_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_CONTENT_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_CONTENT_DY));

	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	rc1.left = CHEDIT_WARNING_WINDOW_CONTENT_X; 
	rc1.top = CHEDIT_WARNING_WINDOW_CONTENT_Y + 20; 
	rc1.right = CHEDIT_WARNING_WINDOW_CONTENT_X + CHEDIT_WARNING_WINDOW_CONTENT_DX; 
	rc1.bottom = CHEDIT_WARNING_WINDOW_CONTENT_Y + CHEDIT_WARNING_WINDOW_CONTENT_DY - 20;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_SAVING), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);	
}

void EditSList_Close_Confirm(HDC hdc)
{
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(CHEDIT_WARNING_WINDOW_X), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_Y), ScalerWidthPixel(CHEDIT_WARNING_WINDOW_DX), ScalerHeigthPixel(CHEDIT_WARNING_WINDOW_DY), &btWarning_cap);
	UnloadBitmap (&btWarning_cap);
}

void MV_Set_ChEdit_Current_List(U32 chlist_type, U8 u8Satlist_Sat_Index)
{
	switch ( chlist_type )
	{
		case CSapp_Applet_EditRadio:
			chedit_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			chedit_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSapp_Applet_EditTV:
			chedit_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			chedit_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_EditTVFAV:
			chedit_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			chedit_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case CSApp_Applet_EditRADIOFAV:
			chedit_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			chedit_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case CSApp_Applet_EditTVSAT:
			if ( u8Satlist_Sat_Index == 255 )
			{
				servicelist_type = CSapp_Applet_EditTV;
				chedit_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				chedit_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				servicelist_type = CSApp_Applet_EditTVSAT;
				chedit_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
				chedit_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		case CSApp_Applet_EditRADIOSAT:
			if ( u8Satlist_Sat_Index == 255 )
			{
				servicelist_type = CSApp_Applet_EditRADIOSAT;
				chedit_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				chedit_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				servicelist_type = CSApp_Applet_EditRADIOSAT;
				chedit_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
				chedit_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		default:
			break;
	}
	
	Total_Service = CS_DB_GetListServiceNumber(chedit_triplet);
	//printf("=== CH EDIT TOTAL : %d , %d , %d ====\n", chedit_triplet.sCS_DB_ServiceListType, chedit_triplet.sCS_DB_ServiceListTypeValue, Total_Service);

	if( Total_Service > 0 )
	{
		CS_DB_SetCurrentList(chedit_triplet, FALSE);
	}
}

void Draw_ChEdit_Full(HDC hdc)
{
	/* Draw Edit list kind */
	//printf("============ %d ================\n",servicelist_type);
	MV_Draw_ChEdit_List_State(hdc);
	/* Draw channel list dispaly full list by 10 item and dispaly channel list selected channel select channel infomation */
	MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
	/* Draw play channel information bar on bottom  */
	MV_Draw_ChEdit_Info_Bar(hdc, Current_Service);
}

void Draw_ChEdit_Full2(HDC hdc)
{
	/* Draw Edit list kind */
	//printf("============ %d ================\n",servicelist_type);
	MV_Draw_ChEdit_List_State(hdc);
	/* Draw channel list dispaly full list by 10 item and dispaly channel list selected channel select channel infomation */
	MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
}

CSAPP_Applet_t CSApp_EditSList(CSAPP_Applet_t   slist_type)
{
	int 					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG 				msg;
	HWND				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_EditSLlist_Applets = CSApp_Applet_Error;
		
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
	CreateInfo.spCaption = "editslist";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = EditSList_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = COLOR_transparent;
	CreateInfo.dwAddData = slist_type;
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

	return CSApp_EditSLlist_Applets;
	
}

int EditSList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC							hdc;
   	static BOOL					tv_bk_flag = TRUE;      //for GUI Error
	tCS_DB_ServiceManageData	item_data;
	MV_stServiceInfo			service_data;
	static int 					input_count;
	static char 				input_keys[PIN_MAX_NUM+2];
	static U16					Prev_Service = 0;
	static U16					u32BackService_Index = 0;

	switch (message)
	{
		case MSG_CREATE:
			{
				tCS_DBU_Service ServiceTriplet;

				memset(&btCap_bmp, 0x00, sizeof(BITMAP));
				MV_DB_Set_Moving_Flag(FALSE);
				ChEdit_Move_Status = FALSE;
				ChEdit_Fav_Status = FALSE;
				show_confirm = FALSE;
				u16Delete_Count = 0;
				u16Delete_Temp_Count = 0;
				u16Lock_Count = 0;
				MV_Set_Save_Flag(FALSE);
				
				ComboList_Create(&EditSLlist_ComboList_First, SERVICES_NUM_PER_PAGE);

				PinDlg_SetStatus(FALSE);

				SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, 1000);

				servicelist_type = GetWindowAdditionalData(hwnd);

				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
					
				u8Sat_index = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
					
				MV_Set_ChEdit_Current_List(servicelist_type, u8Sat_index);
					
				back_triplet = CS_DB_GetLastServiceTriplet();
				u32BackService_Index = CS_DB_GetCurrent_ServiceIndex_By_Index(back_triplet.sCS_DBU_ServiceIndex);

				Total_Service = CS_DB_GetListServiceNumber(chedit_triplet);

				if(Total_Service>0)
				{
					CS_DB_SetCurrentList(chedit_triplet, FALSE);
	                                        
					Current_Service = CS_DB_GetCurrentService_OrderIndex();   

					Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);

					Prev_Service = Current_Service;

					//SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
				}       
			}
			break;

		case MSG_TIMER:
			if(wparam == CHECK_SIGNAL_TIMER_ID)
			{
				if ((Total_Service != 0) && (!show_confirm))
				{	
					hdc =BeginPaint(hwnd);
					MV_Draw_ChEdit_Signal(hdc);
					EndPaint(hwnd,hdc);
				}
			}
		    break;
			   
		case MSG_PAINT:
    		if (show_confirm)
    		{
				hdc = BeginPaint(hwnd);
    			EditSList_Draw_Confirm(hdc);
				EndPaint(hwnd,hdc);
    		} else {
			/* Draw background rectangle & pig transparecy rectangle */
				MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_CH_EDIT, 0);
			
				hdc = BeginPaint(hwnd);

				CS_MW_SetSmallWindow(ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_TOP),ScalerWidthPixel(MV_PIG_DX),ScalerHeigthPixel(MV_PIG_DY));
				
				Draw_ChEdit_Full(hdc);
			/* Draw bottom button icon & text */
				MV_Draw_ChEdit_Help_Button(hdc);

				SetBkColor (hdc, COLOR_transparent);
				SetTextColor(hdc, MV_BAR_UNFOCUS_CHAR_COLOR);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_BOTTOM + 10), "S :"/*CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH)*/);
				CS_MW_TextOut(hdc,ScalerWidthPixel(MV_PIG_LEFT),ScalerHeigthPixel(MV_PIG_BOTTOM + MV_INSTALL_SIGNAL_YGAP + 10), "Q :" /*CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY)*/);
				MV_Draw_ChEdit_Signal(hdc);
				
				EndPaint(hwnd,hdc);
    		}
			return 0;
			break;

		case MSG_VIDEO_FORFMAT_UPDATE:
			{
				
			}
			break;
            
		case MSG_CHECK_SERVICE_LOCK:
			//printf("========= Current : %d , Last : %d , %d , %d ===========\n", Current_Service, CS_APP_GetLastUnlockServiceIndex(), item_data.Lock_Flag, Ch_service_data.u8Lock);
			if((CS_MW_GetServicesLockStatus()) && (Ch_service_data.u8Lock == eCS_DB_LOCKED) && (Current_Service != CS_APP_GetLastUnlockServiceIndex()))
			{
				//PinDlg_SetStatus(TRUE);
				input_count = 0;
				memset(input_keys,0,sizeof(input_keys));
				
				if(wparam == 1)
					SendMessage(hwnd, MSG_PAINT, 0, 0);
				
				SendMessage(hwnd, MSG_PIN_INPUT, 0, 0);
				CS_MW_StopService(TRUE);
			}
			else
			{
				CS_DB_SetLastServiceTriplet();
				CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
				MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);
				//PinDlg_SetStatus(TRUE);
				input_count = 0;
				memset(input_keys,0,sizeof(input_keys));

				CS_APP_SetLastUnlockServiceIndex(Current_Service);
				SendMessage (hwnd, MSG_PLAYSERVICE, 0, 0);
			}
			
			break;

		case MSG_PLAYSERVICE:
			{	
				tCS_DBU_Service ServiceTriplet;
				
				CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
				CS_DB_SetCurrentService_OrderIndex(Current_Service);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
				ServiceTriplet.sCS_DBU_ServiceIndex =  Current_Service;
				
				CS_DBU_SaveCurrentService(ServiceTriplet);	

				back_triplet = ServiceTriplet;
				
				u32BackService_Index = CS_DB_GetCurrent_ServiceIndex_By_Index(back_triplet.sCS_DBU_ServiceIndex);

				if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
				{
					FbSendFndDisplayNum((unsigned)Current_Service+1);
				}
				else
				{
					FbSendFndDisplayNum((unsigned)item_data.LCN);
				}

				if (wparam)
					CS_MW_PlayServiceByIdx(item_data.Service_Index, NOT_TUNNING);
				else
					CS_MW_PlayServiceByIdx(item_data.Service_Index, RE_TUNNING);
				
				hdc = BeginPaint(hwnd);
				MV_Draw_ChEdit_Info_Bar(hdc, Current_Service);
				EndPaint(hwnd,hdc);
			}
			break;

		case MSG_CLOSE:
			tv_bk_flag = TRUE;
			#if 0 /* ?? by KB Kim */
			{
				tCS_DBU_Service     ServiceTriplet;
				U32					u32Service_Index = 0;

				ServiceTriplet = back_triplet;
					
				//CS_DB_LoadDatabase();

				// printf("%d \n\n", u32BackService_Index);
				u32BackService_Index -= u16Delete_Temp_Count;
				u32Service_Index = CS_DB_GetCurrent_Service_By_ServiceIndex(u32BackService_Index);
				printf("%d sCS_DBU_ServiceIndex : %d ,,, u32Service_Index : %d, %d\n\n", u32BackService_Index, ServiceTriplet.sCS_DBU_ServiceIndex, u32Service_Index, CS_DB_GetCurrentServiceIndex());
				ServiceTriplet.sCS_DBU_ServiceIndex = u32Service_Index;
				
				CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
				
				CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
			}
			#endif
			
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			UnloadBitmap(&btCap_bmp);
			ComboList_Destroy();
			DestroyMainWindow (hwnd);
			PostQuitMessage (hwnd);
			break;

		case MSG_PIN_INPUT:
			bChEdit_Lock_Flag = TRUE;
			MV_Draw_Password_Window(hwnd);
			break;
			
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_EditSLlist_Applets = CSApp_Applet_Sleep;
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
							if ( bChEdit_Lock_Flag == TRUE )
							{
								bChEdit_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
						}
						break;
						
					case CSAPP_KEY_ENTER:
						if(MV_Password_Retrun_Value() == TRUE)
						{
							if ( bChEdit_Lock_Flag == TRUE )
							{
								bChEdit_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
						}
						break;

					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:							
						bChEdit_Lock_Flag = FALSE;
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
				MV_Jump_Proc(hwnd, wparam);
				if ( wparam == CSAPP_KEY_ENTER )
				{
					U16		u16Jump_Index;
					char	acReturn[5];

					MV_Jump_Retrun_Value(acReturn);

					u16Jump_Index = atoi( acReturn );

					if ( acReturn[0] == 0x00 || u16Jump_Index == 0 )
						break;
						
					if ( u16Jump_Index < Total_Service )
					{
						Current_Service = u16Jump_Index - 1;
						
						Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
						hdc = BeginPaint(hwnd);
						MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
						EndPaint(hwnd,hdc);
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
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();
					
					switch(MV_Get_PopUp_Window_Kind())
					{
						case eMV_TITLE_SORT:
							{
							}
							break;
							
						case eMV_TITLE_SAT:
							break;
							
						case eMV_TITLE_FAV:
							{
								if ( ChEdit_Fav_Status == TRUE )
									ChEdit_Fav_Status = FALSE;
								MV_DB_AddFavoriteService_Select(u8Result_Value);
								//printf("\nFavorite Addition :: Favorite : %d\n\n", u8Result_Value);
								hdc = BeginPaint(hwnd);
								Draw_ChEdit_Full2(hdc);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_SAT_FAV:
							{
								U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;

								if ( MV_Get_Attention_Flag() == FALSE )
								{
									if ( CS_DB_CheckIfChanged() || MV_DB_Get_Moving_Flag() || (u16Lock_Count > 0) || ( u16Delete_Count > 0 ) )
										MV_Save_Change_Value(hwnd);
									
									switch(servicelist_type)
									{
										case CSapp_Applet_EditTV:
										case CSApp_Applet_EditTVFAV:
										case CSApp_Applet_EditTVSAT:
											u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
											
											if ( u8Result_Value == 0 )
											{
												servicelist_type = CSApp_Applet_EditTVSAT;
												u8Sat_index = 255;
											}
											else if ( u8Result_Value < MV_Get_Searched_SatCount()  + 1 )
											{
												servicelist_type = CSApp_Applet_EditTVSAT;
												u8Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
											}
											else
											{
												servicelist_type = CSApp_Applet_EditTVFAV;
												u8Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, (u8Result_Value - MV_Get_Searched_SatCount()) - 1);
											}
											break;
										case CSapp_Applet_EditRadio:
										case CSApp_Applet_EditRADIOFAV:
										case CSApp_Applet_EditRADIOSAT:
											u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
											
											if ( u8Result_Value == 0 )
											{
												servicelist_type = CSapp_Applet_EditRadio;
												u8Sat_index = 255;
											}
											else if ( u8Result_Value <= MV_Get_Searched_SatCount() )
											{
												servicelist_type = CSApp_Applet_EditRADIOSAT;
												u8Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
											}
											else
											{
												servicelist_type = CSApp_Applet_EditRADIOFAV;
												u8Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - MV_Get_Searched_SatCount());
											}
											break;
										default:
											break;
									}
									
									MV_Set_ChEdit_Current_List(servicelist_type, u8Sat_index);

									if(Total_Service>0)
									{
										Current_Service = CS_DB_GetCurrentService_OrderIndex();   

										Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
										Prev_Service = Current_Service;
									
										hdc = BeginPaint(hwnd);
										Draw_ChEdit_Full2(hdc);
										EndPaint(hwnd,hdc);

										//if(APP_GetMainMenuStatus())
										SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
									}
								}
							}
							break;
							
						default:
							break;
					}
				}
				else if ( wparam == CSAPP_KEY_EXIT || wparam == CSAPP_KEY_ESC )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();
					
					if ( MV_Get_PopUp_Window_Kind() == eMV_TITLE_SAT_FAV )
					{
						U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;

						if ( MV_Get_Attention_Flag() == FALSE )
						{
							switch(servicelist_type)
							{
								case CSapp_Applet_EditTV:
								case CSApp_Applet_EditTVFAV:
								case CSApp_Applet_EditTVSAT:
									u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
									
									if ( u8Result_Value == 0 )
									{
										servicelist_type = CSApp_Applet_EditTVSAT;
										u8Sat_index = 255;
									}
									else if ( u8Result_Value < MV_Get_Searched_SatCount()  + 1 )
									{
										servicelist_type = CSApp_Applet_EditTVSAT;
										u8Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
									}
									else
									{
										servicelist_type = CSApp_Applet_EditTVFAV;
										u8Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, (u8Result_Value - MV_Get_Searched_SatCount()) - 1);
									}
									break;
								case CSapp_Applet_EditRadio:
								case CSApp_Applet_EditRADIOFAV:
								case CSApp_Applet_EditRADIOSAT:
									u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
									
									if ( u8Result_Value == 0 )
									{
										servicelist_type = CSapp_Applet_EditRadio;
										u8Sat_index = 255;
									}
									else if ( u8Result_Value <= MV_Get_Searched_SatCount() )
									{
										servicelist_type = CSApp_Applet_EditRADIOSAT;
										u8Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
									}
									else
									{
										servicelist_type = CSApp_Applet_EditRADIOFAV;
										u8Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - MV_Get_Searched_SatCount());
									}
									break;
								default:
									break;
							}
							
							MV_Set_ChEdit_Current_List(servicelist_type, u8Sat_index);

							if(Total_Service>0)
							{
								Current_Service = CS_DB_GetCurrentService_OrderIndex();   

								Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
								Prev_Service = Current_Service;
							
								hdc = BeginPaint(hwnd);
								Draw_ChEdit_Full2(hdc);
								EndPaint(hwnd,hdc);

								//if(APP_GetMainMenuStatus())
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
						}
					}
				}
				break;
			}
			else if ( Get_Keypad_Status() == TRUE )
			{
				UI_Keypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
				{
					if ( wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							MV_stServiceInfo			service_data;
							tCS_DB_ServiceManageData 	item_data;

							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
							MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);
							
							Get_Save_Str(sReturn_str);

							if ( strcmp(sBackup_str, sReturn_str) != 0 )
							{
								if(MV_DB_RenameServiceName(sReturn_str, item_data.Service_Index) == eCS_DB_OK)
								{
									printf("\n=============== SAVE OK ===================\n");
									b8Change_Name = TRUE;
								}
								else
									printf("\n\n\n#### WARNING : Not Change Service Name : %s => %s\n\n", sBackup_str, sReturn_str);
							}
								
							SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
							
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc);
						}
					} else {
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
					}
				}
				
				break;
			}
			else
			{         
				if ((Total_Service == 0) && (wparam != CSAPP_KEY_MENU) && (wparam != CSAPP_KEY_ESC)) break;
                                
				switch(wparam)
				{
					case CSAPP_KEY_DOWN:
						{
							U16		prev_page = 0;
							U16		prev_focus = 0;
							BOOL 	updateall = FALSE;
#ifdef ONE_ITEM_MOVE
							if ( ChEdit_Move_Status == TRUE ){
								MV_DB_ChangeService_OrderIndex(CSAPP_KEY_DOWN, Current_Service, SERVICES_NUM_PER_PAGE);
							}
#endif
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);

							prev_page = Current_Page;
							prev_focus = Current_Focus;
							
							Current_Service++;
							if(Current_Service > Total_Service-1)
							{
								Current_Service = 0;
								updateall = TRUE;
							}
						
							Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);

							hdc = BeginPaint(hwnd);
							if ( prev_page != Current_Page )
								MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
							else
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc);
							
							//SendMessage(hwnd, MSG_PAINT, 0, 0);
						}
						break;

					case CSAPP_KEY_UP:
						{
							U16		prev_page = 0;
							U16		prev_focus = 0;
							BOOL 	updateall = FALSE;
#ifdef ONE_ITEM_MOVE
							if ( ChEdit_Move_Status == TRUE ){
								MV_DB_ChangeService_OrderIndex(CSAPP_KEY_UP, Current_Service, SERVICES_NUM_PER_PAGE);
							}
#endif
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);
                                                                
							prev_page = Current_Page;
							prev_focus = Current_Focus;
							if(Current_Service == 0)
							{
								Current_Service = Total_Service-1;
								updateall = TRUE;
							}
							else
							{
								Current_Service--;
							}

							Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);

							hdc = BeginPaint(hwnd);
							if ( prev_page != Current_Page )
								MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
							else
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc);
							//SendMessage(hwnd, MSG_PAINT, 0, 0);
						}
						break;

					case CSAPP_KEY_LEFT:
						{
#ifdef ONE_ITEM_MOVE
							if ( ChEdit_Move_Status == TRUE ){
								MV_DB_ChangeService_OrderIndex(CSAPP_KEY_LEFT, Current_Service, SERVICES_NUM_PER_PAGE);
							}
#endif
        					if(Current_Service < SERVICES_NUM_PER_PAGE)
        					{
        						Current_Service = (getpage(Total_Service, SERVICES_NUM_PER_PAGE)+1)*SERVICES_NUM_PER_PAGE + Current_Service-SERVICES_NUM_PER_PAGE;
        					}
							else
								Current_Service -= SERVICES_NUM_PER_PAGE;
                                    
        					if(Current_Service>Total_Service-1)
        						Current_Service = Total_Service-1;

							Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
							EndPaint(hwnd,hdc);
							//SendMessage(hwnd, MSG_PAINT, 0, 0);
						}
						break;

					case CSAPP_KEY_RIGHT:
						{
							U8	current_page, total_page;	
#ifdef ONE_ITEM_MOVE
							if ( ChEdit_Move_Status == TRUE ){
								MV_DB_ChangeService_OrderIndex(CSAPP_KEY_RIGHT, Current_Service, SERVICES_NUM_PER_PAGE);
							}
#endif
        					current_page = getpage((Current_Service+1), SERVICES_NUM_PER_PAGE);
        					total_page = getpage(Total_Service, SERVICES_NUM_PER_PAGE);	
                            
							Current_Service += SERVICES_NUM_PER_PAGE;
							
							if(Current_Service > Total_Service-1)
    						{
    							if(current_page<total_page)
									Current_Service = Total_Service-1;
								else
									Current_Service = 0;
            				}

							Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
							EndPaint(hwnd,hdc);
							//SendMessage(hwnd, MSG_PAINT, 0, 0);
						}
						break;

					case CSAPP_KEY_GREEN: 
						if ( ChEdit_Move_Status == TRUE )
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);

							if(item_data.Move_Flag == eDBASE_SELECT)
								CS_DB_ModifyCurrentService_MoveFlag(eDBASE_NOT_SELECT, Current_Service);
							else
								CS_DB_ModifyCurrentService_MoveFlag(eDBASE_SELECT, Current_Service);

							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc); 
							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_DOWN, 0);
						}
						else if ( ChEdit_Fav_Status == TRUE )
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);

							if(item_data.Select_Flag == eDBASE_SELECT)
								CS_DB_ModifyCurrentService_SelectFlag(eDBASE_NOT_SELECT, Current_Service);
							else
								CS_DB_ModifyCurrentService_SelectFlag(eDBASE_SELECT, Current_Service);

							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc); 
							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_DOWN, 0);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);
						
							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
							MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);

							if(item_data.Delete_Flag == eDBASE_DELETE)
							{
								CS_DB_ModifyCurrentService_DeleteFlag(eDBASE_NOT_DELETE, Current_Service);
								if ( u16Delete_Count != 0 )
									u16Delete_Count--;
							}
							else
							{
								CS_DB_ModifyCurrentService_DeleteFlag(eDBASE_DELETE, Current_Service);
								u16Delete_Count++;
							}

							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc);
							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_DOWN, 0);
						}
						break;

					case CSAPP_KEY_BLUE: 
						if ( ChEdit_Move_Status == FALSE && ChEdit_Fav_Status == FALSE )
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
							MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);

							if (item_data.Lock_Flag == eCS_DB_LOCKED)
							{
								MV_DB_ModifyServiceLockStatus(eCS_DB_NOT_LOCKED, Current_Service);
								if ( u16Lock_Count != 0 )
									u16Lock_Count--;
								else
									u16Lock_Count = 1;
							}
							else
							{
								MV_DB_ModifyServiceLockStatus(eCS_DB_LOCKED, Current_Service);
								u16Lock_Count++;
							}

							hdc = BeginPaint(hwnd);
							MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
							EndPaint(hwnd,hdc); 
							SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_DOWN, 0);
						}
						break;
						
					case CSAPP_KEY_YELLOW: 
						if ( ChEdit_Move_Status == FALSE && ChEdit_Fav_Status == FALSE )
						{
							MV_stServiceInfo			service_data;
							tCS_DB_ServiceManageData 	item_data;
							
							CS_DB_GetCurrentList_ServiceData(&item_data, Current_Service);
							MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
							memset(sBackup_str, 0x00, MAX_SERVICE_NAME_LENGTH+1);
							strcpy(sBackup_str, service_data.acServiceName);
							b8Change_Name = FALSE;
							MV_Draw_Keypad(hwnd, service_data.acServiceName, MAX_SERVICE_NAME_LENGTH);
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

					case CSAPP_KEY_RED: 
						{
							if ( ChEdit_Move_Status == TRUE )
							{
#ifndef ONE_ITEM_MOVE
								Current_Service = MV_DB_ChangeService_OrderIndex(Current_Service);
								hdc = BeginPaint(hwnd);
								Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
								MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
#else
								hdc = BeginPaint(hwnd);
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
#endif // #ifndef ONE_ITEM_MOVE
							}
							else if ( ChEdit_Fav_Status == FALSE )
							{
								ChEdit_Move_Status = TRUE;
								
								hdc = BeginPaint(hwnd);
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
							}
							/*
							tCS_DB_SortType SortType = eCS_DB_BY_NAME_A_Z;
							dprintf(("CSAPP_KEY_CB\n"));
							CS_DB_SortCurrentServiceList(SortType);
							ComboList_UpdateAll(hwnd);
							*/
						}
						break;

					case CSAPP_KEY_SAT: 
						if ( ChEdit_Move_Status == FALSE && ChEdit_Fav_Status == FALSE )
						{
							U8 Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
							
							if ( CS_DB_CheckIfChanged() || MV_DB_Get_Moving_Flag() || (u16Lock_Count > 0) || ( u16Delete_Count > 0 ) )
								MV_Set_Save_Flag(TRUE);
			    			sat_change_confirm = FALSE;

							switch(servicelist_type)
							{								
								case CSApp_Applet_RADIOFAVList:
								case CSApp_Applet_RADIOSATList:
								case CSApp_Applet_RDList:
									Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
									break;
									
								case CSApp_Applet_TVList:
								case CSApp_Applet_TVFAVList:
								case CSApp_Applet_TVSATList:
									Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								default:
									break;								
							}
							
							MV_Draw_SatFavlist_Window(hwnd, Service_Type);
						}
						break;

					case CSAPP_KEY_FAVOLIST:
						if ( ChEdit_Move_Status == FALSE )
						{
							if ( ChEdit_Fav_Status == TRUE )
							{
								U8	Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
								
								switch(servicelist_type)
								{								
									case CSApp_Applet_RADIOFAVList:
									case CSApp_Applet_RADIOSATList:
									case CSApp_Applet_RDList:
										Service_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
										break;
										
									case CSApp_Applet_TVList:
									case CSApp_Applet_TVFAVList:
									case CSApp_Applet_TVSATList:
										Service_Type = kCS_DB_DEFAULT_TV_LIST_ID;
									default:
										break;								
								}
								MV_Draw_Favlist_Window(hwnd, Service_Type, FALSE);
							} else {
								ChEdit_Fav_Status = TRUE;

								hdc = BeginPaint(hwnd);
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
							}
						}
						break;

					case CSAPP_KEY_ENTER:
						if ( sat_change_confirm )
						{
							MV_Save_Change_Value(hwnd);
							SendMessage (hwnd, MSG_KEYDOWN, CSAPP_KEY_SAT, 0);
						}
        				else if (show_confirm)
        				{
							hdc = BeginPaint(hwnd);
							EditSList_Close_Confirm(hdc);
							EndPaint(hwnd,hdc);
							
							MV_Save_Change_Value(hwnd);
        					CSApp_EditSLlist_Applets = CSApp_Applet_Desktop;
        					SendMessage (hwnd, MSG_CLOSE, 0, 0);
        				}
						else
						{
							if (Prev_Service != Current_Service)
                    		{
								CS_APP_SetLastUnlockServiceIndex(0xffff);
                    		}
							Prev_Service = Current_Service;
							SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						}
						break;

					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						{	
							if ( ChEdit_Move_Status == TRUE )
							{								
								ChEdit_Move_Status = FALSE;
#ifndef ONE_ITEM_MOVE
								Current_Service = MV_DB_ChangeService_OrderIndex(Current_Service);
								hdc = BeginPaint(hwnd);
								Current_Focus = get_focus_line(&Current_Page, Current_Service, SERVICES_NUM_PER_PAGE);
								MV_Draw_ChEdit_List(hdc, Current_Service, Current_Page, Current_Focus, Total_Service );;
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
#else
								hdc = BeginPaint(hwnd);
								MV_Draw_ChEdit_List_Item(hdc, Current_Focus, Current_Service, Total_Service, FOCUS);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
#endif // #ifndef ONE_ITEM_MOVE
							}
							else if ( ChEdit_Fav_Status == TRUE )
							{
								ChEdit_Fav_Status = FALSE;
								MV_DB_AddFavoriteService_Select_Clear();
								hdc = BeginPaint(hwnd);
								Draw_ChEdit_Full2(hdc);
								MV_Draw_ChEdit_Help_Button(hdc);
								EndPaint(hwnd,hdc);
							}
							else if (!show_confirm)
							{
							//	CS_DB_SaveListModifications();

								if(CS_DB_CheckIfChanged() || MV_DB_Get_Moving_Flag() || u16Lock_Count > 0 || u16Delete_Count > 0 )
								{
									show_confirm = TRUE;
									SendMessage (hwnd, MSG_PAINT, 0, 0);
								}
								else
								{
									CSApp_EditSLlist_Applets = CSApp_Applet_Desktop;
									SendMessage (hwnd, MSG_CLOSE, 0, 0);
								}
							}
							else
							{
								CSApp_EditSLlist_Applets = CSApp_Applet_Desktop;
								SendMessage (hwnd, MSG_CLOSE, 0, 0);
							}
						}
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

void MV_Save_Change_Value(HWND hwnd)
{
	HDC 	hdc;
	
	hdc = BeginPaint(hwnd);
	EditSList_Draw_Saving(hdc);
	EndPaint(hwnd,hdc);
	
	if ( MV_DB_Get_Moving_Flag() )
	{
		//printf("\nMV_Save_Change_Value :: MV_DB_Set_Service_Move ==== %d =======\n\n", servicelist_type);
		MV_DB_Set_Service_Move();
	}

	if ( u16Lock_Count > 0 )
	{
		//printf("\nMV_Save_Change_Value :: MV_DB_Set_Service_Lock ==============\n\n");
		MV_DB_Set_Service_Lock();
	}

	if ( u16Delete_Count > 0 )
	{
		//printf("\nMV_Save_Change_Value :: CS_DB_SaveListModifications ==============\n\n");
		u16Delete_Temp_Count = CS_DB_Get_Sub_Count();
		CS_DB_SaveListModifications();
		//MV_DB_Set_Service_Delete();
	}

	if ( MV_DB_Get_Moving_Flag() || ( u16Delete_Count > 0 ) )
	{
		//printf("\nMV_Save_Change_Value :: MV_DB_Set_Replace_Index %d : %d : %d ==============\n\n", MV_DB_Get_Moving_Flag(), u16Lock_Count, u16Delete_Count);
		MV_DB_Set_Replace_Index();
	}

	//printf("\nMV_Save_Change_Value :: 1 ==============\n\n");
	CS_DB_SaveDatabase();
	//printf("\nMV_Save_Change_Value :: 2 ==============\n\n");
	
	if ( chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditTVFAV || chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditRADIOFAV )
	{
		if ( chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditTVFAV )
		{
			if ( MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_TV_LIST_ID, chedit_triplet.sCS_DB_ServiceListTypeValue) == 0 )
			{
				MV_Set_ChEdit_Current_List(CSapp_Applet_EditTV, 0);
			}
		} else {
			if ( MV_Get_ServiceCount_at_Favorite(kCS_DB_DEFAULT_RADIO_LIST_ID, chedit_triplet.sCS_DB_ServiceListTypeValue) == 0 )
			{
				MV_Set_ChEdit_Current_List(CSapp_Applet_EditRadio, 0);
			}
		}
	} 
	else if ( chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditTVSAT || chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditRADIOSAT ) 
	{
		if ( chedit_triplet.sCS_DB_ServiceListType == CSApp_Applet_EditTVSAT )
		{
			if ( MV_Get_TVServiceCount_at_Sat(chedit_triplet.sCS_DB_ServiceListTypeValue) == 0 )
			{
				MV_Set_ChEdit_Current_List(CSapp_Applet_EditTV, 0);
			}
		} else {
			if ( MV_Get_RDServiceCount_at_Sat(chedit_triplet.sCS_DB_ServiceListTypeValue) == 0 )
			{
				MV_Set_ChEdit_Current_List(CSapp_Applet_EditRadio, 0);
			}
		}
	}

	//printf("\nMV_Save_Change_Value :: 3 ==============\n\n");
	//CS_DB_RestoreDatabase ();
	CS_DB_Restore_CH_UseIndex();
	//printf("\nMV_Save_Change_Value :: 4 ==============\n\n");
	
	hdc = BeginPaint(hwnd);
	EditSList_Close_Confirm(hdc);
	EndPaint(hwnd,hdc);
}


