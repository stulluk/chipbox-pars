#include "mv_menu_ctr.h"
#include "ch_install.h"
#include "ui_common.h"
#include "csslist.h"

#define	SUPORT_ADVANCE

#define CL_WINDOW_X					160
#define CL_WINDOW_Y					100
//#define	CL_WINDOW_DX				340
#define	CL_WINDOW_ITEM_DX			21
#define	CL_WINDOW_DX				426
#define CL_WINDOW_DY				540
#define CL_WINDOW_ITEM_GAP			10
#define CL_WINDOW_ITEM_DY			30
#define	CL_WINDOW_ITEM_HEIGHT		(CL_WINDOW_ITEM_DY + CL_WINDOW_ITEM_GAP)
#define CL_WINDOW_NO_X				180
#define CL_WINDOW_NO_DX				80
#define CL_WINDOW_NAME_X			( CL_WINDOW_NO_X + CL_WINDOW_NO_DX )
#define CL_WINDOW_NAME_DX			210
#define CL_WINDOW_SCRAMBLE_X 		( CL_WINDOW_NAME_X + CL_WINDOW_NAME_DX )
#define CL_WINDOW_LOCK_X			( CL_WINDOW_SCRAMBLE_X + CL_WINDOW_ITEM_DX )
#define CL_WINDOW_FAVORITE_X		( CL_WINDOW_LOCK_X + CL_WINDOW_ITEM_DX )
#define CL_WINDOW_SDHD_X			( CL_WINDOW_FAVORITE_X + CL_WINDOW_ITEM_DX )
#define	CL_WINDOW_TITLE_Y			120
#define	CL_WINDOW_LIST_Y			150
#define	CL_WINDOW_INFOR_Y			470
#define	CL_WINDOW_ICON_X			CL_WINDOW_NO_X
#define	CL_WINDOW_ICON_X2			(CL_WINDOW_X + CL_WINDOW_DX/2)
#define	CL_WINDOW_ICON_Y			540
#define	CL_WINDOW_ICON_Y2			570
#define	CL_WINDOW_ICON_Y3			600

#define	pin_box_dx					240
#define	pin_box_dy					110
#define	pin_box_base_x				((720-pin_box_dx)/2)
#define	pin_box_base_y 				((576-pin_box_dy)/2)

#define	SERVICES_NUM_PER_PAGE		10
#define	FIELDS_PER_LINE				6

#define	CHLIST_WARNING_W			MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmWidth
#define	CHLIST_WARNING_H			MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmHeight
#define	CHLIST_WARNING_X			( CL_WINDOW_X + CL_WINDOW_DX + 200 )
#define	CHLIST_WARNING_Y			( (720-CHLIST_WARNING_H)/2-50 )

tComboList_Field_Rect		SList_ComboList_First_Line[FIELDS_PER_LINE] = {
													{CL_WINDOW_NO_X, CL_WINDOW_NO_DX},
													{CL_WINDOW_NAME_X, CL_WINDOW_NAME_DX},
													{CL_WINDOW_SCRAMBLE_X, 21},
													{CL_WINDOW_LOCK_X, 21},
													{CL_WINDOW_FAVORITE_X, 21},
													{CL_WINDOW_SDHD_X, 21}
												};

tComboList_Element			SList_ComboList_First = {	
													{CL_WINDOW_X+10, CL_WINDOW_LIST_Y+10, CL_WINDOW_DX - 40, CL_WINDOW_ITEM_DY},
													FIELDS_PER_LINE,
													SList_ComboList_First_Line
												};

CSAPP_Applet_t				CSApp_SList_Applets;

tCS_DB_ServiceListTriplet	chlist_triplet;
U16							chlist_Total_Service = 0;	
U16							chlist_Current_Service = 0;
U16							chlist_Current_Page = 0;
U16							chlist_Prev_Page = 0;
U16							chlist_Current_Focus = 0;
tCS_DB_ServiceManageData	chlist_item_data;
MV_stServiceInfo			chlist_service_data;
U32							chlist_servicelist_type;
U8							u8Sat_index = 0;
BOOL						bCh_Lock_Flag = FALSE;
static eCSCh_ListClew		u8Warning_Status = MV_SIGNAL_UNLOCK;

tCS_DBU_Service     		back_triplet;

#define SORT_NUM  			7

U16 Sort_Str[SORT_NUM] = {
	CSAPP_STR_A_Z,
	CSAPP_STR_Z_A,
	CSAPP_STR_FTA_CAS,
	CSAPP_STR_CAS_FTA,
	CSAPP_STR_SD_HD,
	CSAPP_STR_HD_SD,
	CSAPP_STR_RESTORE_NORMAL
};

#define DEBUG_TEST
#ifdef DEBUG_TEST
U32	u32Count_Loop = 0;
#endif
//static  BOOL	Service_Unlocked = FALSE;

static void ChList_Notify(tMWNotifyData NotifyData)
{
	switch(NotifyData.type)
	{
		case UPDATE_FE:
			BroadcastMessage (MSG_UPDATE_FE, NotifyData.uData.FEStatus, 0);
			break;

		default:
			break;
	}
}

void MV_Draw_Warning_Window(HDC hdc)
{
	char		acTemp_Str[100];
	RECT		Temp_Rect;

	memset(acTemp_Str, 0x00, 100);
	
	FillBoxWithBitmap(hdc,ScalerWidthPixel(CHLIST_WARNING_X),ScalerHeigthPixel(CHLIST_WARNING_Y),ScalerHeigthPixel(CHLIST_WARNING_W), ScalerHeigthPixel(CHLIST_WARNING_H), &MV_BMP[MVBMP_DESCTOP_MSG_PANEL]);
	
	if(u8Warning_Status == MV_SIGNAL_UNLOCK)
	{
		sprintf(acTemp_Str ,"%s !", CS_MW_LoadStringByIdx(CSAPP_STR_NO_SIGNAL));
	}
	else if(u8Warning_Status == MV_NO_SERVICE)
	{
		sprintf(acTemp_Str, "%s !", CS_MW_LoadStringByIdx(CSAPP_STR_NO_SERVICE));
	}
	else if(u8Warning_Status == MV_SERVICE_ENCRYPT )
	{
		sprintf(acTemp_Str, "%s !", CS_MW_LoadStringByIdx(CSAPP_STR_PROG_ENCRYPT));
	}

	Temp_Rect.top = CHLIST_WARNING_Y + (CHLIST_WARNING_H-30)/2;
	Temp_Rect.bottom = Temp_Rect.top + 30;
	Temp_Rect.left = CHLIST_WARNING_X+20;
	Temp_Rect.right = Temp_Rect.left + CHLIST_WARNING_W - 40;
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	CS_MW_DrawText(hdc, acTemp_Str, -1, &Temp_Rect, DT_CENTER | DT_VCENTER);
}

void MV_Close_Warning_Window(HDC hdc)
{
	SetBrushColor(hdc, COLOR_transparent);
	FillBox(hdc,ScalerWidthPixel(CHLIST_WARNING_X),ScalerHeigthPixel(CHLIST_WARNING_Y),ScalerWidthPixel(CHLIST_WARNING_W), ScalerHeigthPixel(CHLIST_WARNING_H));
}

void MV_Draw_CH_List_Window(HDC hdc)
{
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
		MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	else
		SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_R, CFG_Menu_Back_Color.MV_G, CFG_Menu_Back_Color.MV_B, 0xFF));
		// SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_A, CFG_Menu_Back_Color.MV_B, CFG_Menu_Back_Color.MV_G, 0xFF));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + 2), ScalerHeigthPixel(CL_WINDOW_Y),ScalerWidthPixel(CL_WINDOW_DX - 4),ScalerHeigthPixel(CL_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + 2),ScalerWidthPixel(CL_WINDOW_DX),ScalerHeigthPixel(CL_WINDOW_DY - 4));	

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_TITLE_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY));
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FIND));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_EDIT));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X), ScalerHeigthPixel(CL_WINDOW_ICON_Y2), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y2), CS_MW_LoadStringByIdx(CSAPP_STR_SORT_KEY));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2), ScalerHeigthPixel(CL_WINDOW_ICON_Y2), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y2), CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND));
}

void MV_Draw_CH_Info(HDC hdc)
{
	char			buff2[50];
	MV_stTPInfo		tpdata;
	MV_stSatInfo	Temp_SatData;	
	
	if (chlist_Total_Service != 0)
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	
		
		memset(buff2, 0, 50);
		CS_DB_GetCurrentList_ServiceData(&chlist_item_data, chlist_Current_Service);
		MV_DB_GetServiceDataByIndex(&chlist_service_data, chlist_item_data.Service_Index);
		MV_DB_GetTPDataByIndex(&tpdata, chlist_service_data.u16TransponderIndex);
		MV_GetSatelliteData_ByIndex(&Temp_SatData, MV_DB_Get_SatIndex_By_TPindex(chlist_service_data.u16TransponderIndex));

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

void MV_Draw_List_Title(HDC hdc)
{
	char 			title[50];
	char			TempStr[50];
	RECT 			TmpRect;
	MV_stSatInfo	Temp_SatData;
	
	memset(title, 0, 50);
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	switch(chlist_servicelist_type)
	{
		case CSApp_Applet_TVList:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
		case CSApp_Applet_RDList:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_RD_LIST));
			break;
		case CSApp_Applet_TVFAVList:
			MV_DB_Get_Favorite_Name(TempStr, chlist_triplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s" ,TempStr);
			break;
		case CSApp_Applet_RADIOFAVList:
			MV_DB_Get_Favorite_Name(TempStr, chlist_triplet.sCS_DB_ServiceListTypeValue);
			sprintf(title, "%s" ,TempStr);
			break;
		case CSApp_Applet_TVSATList:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Sat_index);
			sprintf(title, "%s" ,Temp_SatData.acSatelliteName);
			break;
		case CSApp_Applet_RADIOSATList:
			MV_GetSatelliteData_ByIndex(&Temp_SatData, u8Sat_index);
			sprintf(title, "%s" ,Temp_SatData.acSatelliteName);
			break;
		default:
			sprintf(title, "%s" ,CS_MW_LoadStringByIdx(CSAPP_STR_TV_LIST));
			break;
	}

	//printf("\nMV_Draw_List_Title : === %d ==> %s\n\n", chlist_servicelist_type, title);

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_TITLE_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY));
	
	TmpRect.left	= ScalerWidthPixel(CL_WINDOW_X);
	TmpRect.right	= TmpRect.left + ScalerWidthPixel(CL_WINDOW_DX);
	TmpRect.top		= ScalerWidthPixel(CL_WINDOW_TITLE_Y) + 4;
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(CL_WINDOW_ITEM_DY);
	CS_MW_DrawText(hdc, title, -1, &TmpRect, DT_CENTER);
}

void MV_Draw_List_Item(HDC hdc, int Count_index, U16 u16Focusindex, U8 FocusKind)
{
	char	buff1[50];
	RECT	rc_service_name;
	RECT 	Scroll_Rect;
	/* By KB Kim 2011.01.20 */
	U8      tvRadio;

	memset(buff1, 0, 50);
	memset(&chlist_item_data, 0x00, sizeof(tCS_DB_ServiceManageData));
	memset(&chlist_service_data, 0x00, sizeof(MV_stServiceInfo));
	CS_DB_GetCurrentList_ServiceData(&chlist_item_data, u16Focusindex);
	MV_DB_GetServiceDataByIndex(&chlist_service_data, chlist_item_data.Service_Index);

	//printf("%04d : %d : %d : %s ======\n", u16Focusindex, chlist_item_data.Service_Index, chlist_service_data.u16ChIndex, chlist_service_data.acServiceName);

	if ( FocusKind == FOCUS)
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element.x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element.dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	else if ( 0 == Count_index%2 )
	{
		SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(SList_ComboList_First.element.x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element.dx), ScalerHeigthPixel(SList_ComboList_First.element.dy));
	}
	else
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(SList_ComboList_First.element.x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element.dx), ScalerHeigthPixel(SList_ComboList_First.element.dy));
	}

	if ( FocusKind != NOTFOCUS )
	{
#ifndef DEBUG_TEST
		printf("\n %d === %d === %d ,  %s \n", u16Focusindex, chlist_item_data.Service_Index, chlist_service_data.u16ChIndex, chlist_service_data.acServiceName);
		{
			int 	i;
/*
			for( i = 0 ; i < strlen(chlist_service_data.acServiceName) ; i++)
			{
				printf("%x ", chlist_service_data.acServiceName[i]);
			}
*/
			printf("\n");
		}
#else
		//printf("============= 1 ========== %04d \n", u16Focusindex + 1);
#endif
		sprintf(buff1, "%04d", u16Focusindex + 1);

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc, CSAPP_WHITE_COLOR);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[0].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index + 4), buff1);	
		
		rc_service_name.left = ScalerWidthPixel(SList_ComboList_First.element_fields[1].x);
		rc_service_name.top = ScalerHeigthPixel( (SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index ) + 4 );
		rc_service_name.right = ScalerWidthPixel(SList_ComboList_First.element_fields[1].x + CL_WINDOW_NAME_DX /*(12 * 13 )*/);
		rc_service_name.bottom = ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index + 28);
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		//if ( u16Focusindex != 1822 && u16Focusindex != 1823 )
			CS_MW_DrawText(hdc, chlist_service_data.acServiceName, -1, &rc_service_name, DT_LEFT);

		if(chlist_service_data.u8Scramble)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[2].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[2].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[2].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[2].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON]);
		}

		if(chlist_service_data.u8Lock)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[3].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[3].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_LOCK_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[3].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[3].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NLOCK_ICON]);
		}

		/* By KB Kim 2011.01.20 */
		if (chlist_service_data.u8TvRadio == eCS_DB_RADIO_SERVICE)
		{
			tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
		}
		else
		{
			tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
		}
		if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, chlist_service_data.u16ChIndex) < MV_MAX_FAV_KIND )
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[4].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[4].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_FAVORITE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[4].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[4].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NFAVORITE_ICON]);
		}

		if( chlist_service_data.u8TvRadio == eCS_DB_HDTV_SERVICE )
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[5].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[5].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_HD_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[5].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[5].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NHD_ICON]);
		}
	}

	if ( FocusKind == FOCUS )
	{
		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Current_Service, chlist_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_List_Full_Item(HDC hdc)
{
	int 	i;
	U16		index;
	RECT 	Scroll_Rect;

	for(i =0; i<SERVICES_NUM_PER_PAGE; i++)
	{
		index = chlist_Current_Page * SERVICES_NUM_PER_PAGE + i;
		
		if(index < chlist_Total_Service)
		{
			if(chlist_Current_Focus == i)
				MV_Draw_List_Item(hdc, i, index, FOCUS);
			else 
				MV_Draw_List_Item(hdc, i, index, UNFOCUS);
		}	
		else
		{
			MV_Draw_List_Item(hdc, i, index, NOTFOCUS);
		}
	}	

	if (chlist_Total_Service == 0)
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[1].x), ScalerHeigthPixel(SList_ComboList_First.element.y+4),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));

		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, chlist_Current_Service, chlist_Total_Service, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_CH_List(HDC hdc)
{
	MV_Draw_CH_List_Window(hdc);
	MV_Draw_List_Title(hdc);
	MV_Draw_List_Full_Item(hdc);
	MV_Draw_CH_Info(hdc);
	//CS_MW_SetSmallWindow((352),(112),(320),(240));   /* Use PIG */
}

void MV_Draw_CH_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind)
{
	HDC		hdc;
	int 	Count_index;
	
	hdc = BeginPaint(hwnd);
	
	Count_index = u16Focusindex%SERVICES_NUM_PER_PAGE;

	if ( chlist_Prev_Page != chlist_Current_Page && FocusKind == FOCUS )
		MV_Draw_List_Full_Item(hdc);
	else
		MV_Draw_List_Item(hdc, Count_index, u16Focusindex, FocusKind);
	
	if ( FocusKind == FOCUS )
		MV_Draw_CH_Info(hdc);
	
	EndPaint(hwnd,hdc);
}

void MV_Set_Current_List(U32 chlist_type, U8 u8Satlist_Sat_Index)
{
	switch ( chlist_type )
	{
		case CSApp_Applet_RDList:
			chlist_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			chlist_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_TVList:
			chlist_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			chlist_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case CSApp_Applet_TVFAVList:
			chlist_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			chlist_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case CSApp_Applet_RADIOFAVList:
			chlist_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			chlist_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			break;
		case CSApp_Applet_TVSATList:
			if ( u8Satlist_Sat_Index == 255 )
			{
				chlist_servicelist_type = CSApp_Applet_TVList;
				chlist_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
				chlist_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				chlist_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
				chlist_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		case CSApp_Applet_RADIOSATList:
			if ( u8Satlist_Sat_Index == 255 )
			{
				chlist_servicelist_type = CSApp_Applet_RDList;
				chlist_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
				chlist_triplet.sCS_DB_ServiceListTypeValue = 0;
			} else {
				chlist_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
				chlist_triplet.sCS_DB_ServiceListTypeValue = u8Satlist_Sat_Index;
			}
			break;
		default:
			break;
	}
	
	chlist_Total_Service = CS_DB_GetListServiceNumber(chlist_triplet);
	//printf("=== TOTAL : %d , u8Satlist_Sat_Index : %d , chlist_servicelist_type : %d \n", chlist_Total_Service, u8Satlist_Sat_Index, chlist_servicelist_type);

	if( chlist_Total_Service > 0 )
	{
		CS_DB_SetCurrentList(chlist_triplet, FALSE);
	}
}

CSAPP_Applet_t CSApp_SList(CSAPP_Applet_t   slist_type)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;	

	CSApp_SList_Applets = CSApp_Applet_Error;

	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);

	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "slist window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= SList_Msg_cb;
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

	return CSApp_SList_Applets;

}

int SList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC			hdc;
	BOOL		SListPaintFlag = TRUE;
	U16			Prev_Service = 0;
	char		sReturn_str[MAX_SAT_NAME_LANGTH+1];

	switch (message)
	{
		case MSG_CREATE:
			{
				tCS_DBU_Service ServiceTriplet;

				ComboList_Create(&SList_ComboList_First, SERVICES_NUM_PER_PAGE);
				
				SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, 1000);

				chlist_servicelist_type = GetWindowAdditionalData(hwnd);

				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
					
				u8Sat_index = ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue;
				
				MV_Set_Current_List(chlist_servicelist_type, u8Sat_index);
				
				back_triplet = CS_DB_GetLastServiceTriplet();

				if(chlist_Total_Service>0)
				{
					chlist_Current_Service = CS_DB_GetCurrentService_OrderIndex();  

					chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);
					Prev_Service = chlist_Current_Service;
				}
				CS_MW_SVC_Open(ChList_Notify);
			}
			break;

		case MSG_TIMER:
			if(wparam == CHECK_SIGNAL_TIMER_ID)
			{
				U8  		quality;
				U8  		level;
				BOOL 		lock;

				CS_INSTALL_GetSignalInfo(&quality, &level, &lock);
			}

			break;
			   
 		case MSG_PAINT:	
			hdc = BeginPaint(hwnd);
			MV_Draw_CH_List(hdc);
			EndPaint(hwnd,hdc);
			return 0;
			break;
            
		case MSG_VIDEO_FORFMAT_UPDATE:
			{
				
			}
			break;
			
		case MSG_CHECK_SERVICE_LOCK:
			if ( CS_MW_GetServicesLockStatus() && ( chlist_item_data.Lock_Flag == eCS_DB_LOCKED ) && ( chlist_Current_Service != CS_APP_GetLastUnlockServiceIndex() ) )
			{
				if(wparam == 1)
					SendMessage(hwnd, MSG_PAINT, 0, 0);
				
				SendMessage(hwnd, MSG_PIN_INPUT, 0, 0);
				CS_MW_StopService(TRUE);
			} else {
				CS_DB_SetLastServiceTriplet();				
				CS_DB_GetCurrentList_ServiceData(&chlist_item_data, chlist_Current_Service);
				MV_DB_GetServiceDataByIndex(&chlist_service_data, chlist_item_data.Service_Index);

				CS_APP_SetLastUnlockServiceIndex(chlist_Current_Service);
				SendMessage (hwnd, MSG_PLAYSERVICE, 0, 0);
			}
			break;

		case MSG_PLAYSERVICE:
			{	
				tCS_DBU_Service 	ServiceTriplet;

				CS_DB_GetCurrentList_ServiceData(&chlist_item_data, chlist_Current_Service);
				CS_DB_SetCurrentService_OrderIndex(chlist_Current_Service);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));				
				ServiceTriplet.sCS_DBU_ServiceIndex =  chlist_Current_Service;
				
				CS_DBU_SaveCurrentService(ServiceTriplet);

				back_triplet = ServiceTriplet;

				if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
					FbSendFndDisplayNum((unsigned)chlist_Current_Service+1);
				else
					FbSendFndDisplayNum((unsigned)chlist_item_data.LCN);

				printf("============ wparam : %d ============\n", wparam);
				if (wparam)
					CS_MW_PlayServiceByIdx(chlist_item_data.Service_Index, NOT_TUNNING);
				else
					CS_MW_PlayServiceByIdx(chlist_item_data.Service_Index, RE_TUNNING);
			}
			break;

		case MSG_UPDATE_FE:
			hdc = BeginPaint(hwnd);
			if(wparam == FE_LOCK)
			{
				MV_Close_Warning_Window(hdc);
			}
			else if((wparam == FE_UNLOCK)||(wparam == FE_LOST))
			{
				if (( MV_Get_Password_Flag() != TRUE ) &&
					( MV_Get_PopUp_Window_Status() != TRUE ) &&
					( MV_Get_Report_Window_Status() != TRUE ))
				{
					u8Warning_Status = MV_SIGNAL_UNLOCK;
					MV_Draw_Warning_Window(hdc);
				}
			}
			EndPaint(hwnd,hdc);
			break;

		case MSG_CLOSE:
			SListPaintFlag = TRUE;
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			CS_MW_SVC_Close();
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
			bCh_Lock_Flag = TRUE;
			MV_Draw_Password_Window(hwnd);
			break;
			
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
				CSApp_SList_Applets = CSApp_Applet_Sleep;
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
							if ( bCh_Lock_Flag == TRUE )
							{
								bCh_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(chlist_Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							else 
							{
								switch( chlist_servicelist_type )
								{	
									case CSApp_Applet_TVSATList:
										CSApp_SList_Applets = CSApp_Applet_EditTVSAT;
										break;								
									case CSApp_Applet_TVList:
										CSApp_SList_Applets = CSapp_Applet_EditTV;
										break;
									case CSApp_Applet_TVFAVList:
										CSApp_SList_Applets = CSApp_Applet_EditTVFAV;
										break;
									case CSApp_Applet_RDList:
										CSApp_SList_Applets = CSapp_Applet_EditRadio;
										break;
									case CSApp_Applet_RADIOFAVList:
										CSApp_SList_Applets = CSApp_Applet_EditRADIOFAV;
										break;
									case CSApp_Applet_RADIOSATList:
										CSApp_SList_Applets = CSApp_Applet_EditRADIOSAT;
										break;
									default:
										CSApp_SList_Applets = CSapp_Applet_EditTV;
										break;
								}
								bCh_Lock_Flag = FALSE;
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
							if ( bCh_Lock_Flag == TRUE )
							{
								bCh_Lock_Flag = FALSE;
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);

								CS_APP_SetLastUnlockServiceIndex(chlist_Current_Service);
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
							}
							else 
							{
								switch( chlist_servicelist_type )
								{	
									case CSApp_Applet_TVSATList:
										CSApp_SList_Applets = CSApp_Applet_EditTVSAT;
										break;								
									case CSApp_Applet_TVList:
										CSApp_SList_Applets = CSapp_Applet_EditTV;
										break;
									case CSApp_Applet_TVFAVList:
										CSApp_SList_Applets = CSApp_Applet_EditTVFAV;
										break;
									case CSApp_Applet_RDList:
										CSApp_SList_Applets = CSapp_Applet_EditRadio;
										break;
									case CSApp_Applet_RADIOFAVList:
										CSApp_SList_Applets = CSApp_Applet_EditRADIOFAV;
										break;
									case CSApp_Applet_RADIOSATList:
										CSApp_SList_Applets = CSApp_Applet_EditRADIOSAT;
										break;
									default:
										CSApp_SList_Applets = CSapp_Applet_EditTV;
										break;
								}
															
								bCh_Lock_Flag = FALSE;
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
						bCh_Lock_Flag = FALSE;
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
					
					if ( u16Jump_Index < chlist_Total_Service )
					{
						chlist_Current_Service = u16Jump_Index - 1;
						
						chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);
						hdc = BeginPaint(hwnd);
						MV_Draw_List_Full_Item(hdc);
						MV_Draw_CH_Info(hdc);
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
								
								CS_DB_GetCurrentList_ServiceData(&chlist_item_data, chlist_Current_Service);

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

								chlist_Current_Service = CS_DB_GetCurrent_Service_By_ServiceIndex(chlist_item_data.Service_Index);
								back_triplet.sCS_DBU_ServiceIndex = chlist_Current_Service;
								chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);

								CS_DB_SetCurrentService_OrderIndex(chlist_Current_Service);
								CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
								ServiceTriplet.sCS_DBU_ServiceIndex =  chlist_Current_Service;
								CS_DBU_SaveCurrentService(ServiceTriplet);

								/* For Recall List problem By KB Kim : 2011.08.30 */
								/* Reset Recall List After Sort */
								CS_DB_SetLastServiceTriplet();
								MV_Reset_ReCall_List();

								if(CS_MW_GetLcnMode() == eCS_DB_Appearing_Order)
								{
									FbSendFndDisplayNum((unsigned)chlist_Current_Service+1);
								}
								else
								{
									FbSendFndDisplayNum((unsigned)chlist_item_data.LCN);
								}
								
								hdc = BeginPaint(hwnd);
								MV_Draw_List_Full_Item(hdc);
								MV_Draw_CH_Info(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_SAT:
							{
								if ( u8Result_Value == 0 )
									u8Sat_index = 255;
								else
									u8Sat_index = MV_Get_Satindex_by_Seq(u8Result_Value);
								
								//printf("\nu8Result_Value :: %d , u8Sat_index : %d =====\n\n", u8Result_Value, u8Sat_index);
								
								switch(chlist_servicelist_type)
								{
									case CSApp_Applet_TVList:
									case CSApp_Applet_TVFAVList:
									case CSApp_Applet_TVSATList:
										chlist_servicelist_type = CSApp_Applet_TVSATList;
										break;
									case CSApp_Applet_RDList:
									case CSApp_Applet_RADIOFAVList:
									case CSApp_Applet_RADIOSATList:
										chlist_servicelist_type = CSApp_Applet_RADIOSATList;
										break;
									default:
										break;
								}

								MV_Set_Current_List(chlist_servicelist_type, u8Sat_index);

								if(chlist_Total_Service>0)
								{
									chlist_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

									chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);
									Prev_Service = chlist_Current_Service;

									//if(APP_GetMainMenuStatus())
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
								}
								
								hdc = BeginPaint(hwnd);
								MV_Draw_CH_List(hdc);
								EndPaint(hwnd,hdc);
							}
							break;
							
						case eMV_TITLE_FAV:
							{
								U8		u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;

								if ( u8Result_Value == 0 )
									u8Sat_index = 255;
								else
									u8Sat_index = MV_Get_Favindex_by_Seq(u8TVRadio, u8Result_Value - 1);

								switch(chlist_servicelist_type)
								{
									case CSApp_Applet_TVList:
									case CSApp_Applet_TVFAVList:
									case CSApp_Applet_TVSATList:
										if ( u8Sat_index == 255 )
											chlist_servicelist_type = CSApp_Applet_TVList;
										else
											chlist_servicelist_type = CSApp_Applet_TVFAVList;
										u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
										break;
									case CSApp_Applet_RDList:
									case CSApp_Applet_RADIOFAVList:
									case CSApp_Applet_RADIOSATList:
										if ( u8Sat_index == 255 )
											chlist_servicelist_type = CSApp_Applet_RDList;
										else
											chlist_servicelist_type = CSApp_Applet_RADIOFAVList;
										u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
										break;
									default:
										break;
								}								
								
								//printf("\nu8Result_Value :: %d , u8Sat_index : %d =====\n\n", u8Result_Value, u8Sat_index);

								MV_Set_Current_List(chlist_servicelist_type, u8Sat_index);

								if(chlist_Total_Service>0)
								{
									chlist_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

									chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);
									Prev_Service = chlist_Current_Service;

									//if(APP_GetMainMenuStatus())
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
								}
								
								hdc = BeginPaint(hwnd);
								MV_Draw_CH_List(hdc);
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

			else if ( Get_Keypad_Status() == TRUE )
			{
				UI_Keypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
				{
					if ( wparam == CSAPP_KEY_ENTER || wparam == CSAPP_KEY_YELLOW )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							Get_Save_Str(sReturn_str);
							MV_DB_Set_Favorite_Name(sReturn_str, chlist_triplet.sCS_DB_ServiceListTypeValue);
							CS_DB_Save_CH_Database();
							hdc = BeginPaint(hwnd);
							MV_Draw_List_Title(hdc);
							EndPaint(hwnd,hdc);
						}
					}
				}
				
				break;
			}	
			else
			{
				if ((chlist_Total_Service == 0) && (wparam != CSAPP_KEY_TVRADIO) && (wparam != CSAPP_KEY_MENU) && (wparam != CSAPP_KEY_ESC)) break;
			
				//printf(" Key Process ================\n");
				switch(wparam)
				{
					case CSAPP_KEY_RED:
						CSApp_SList_Applets = CSAPP_Applet_Finder;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_GREEN:
						if (CS_DBU_GetParentalLockStatus() && CS_DBU_GetEditLockStatus())
						{
							MV_Draw_Password_Window(hwnd);
						} else {
							switch( chlist_servicelist_type )
							{	
								case CSApp_Applet_TVSATList:
									CSApp_SList_Applets = CSApp_Applet_EditTVSAT;
									break;								
								case CSApp_Applet_TVList:
									CSApp_SList_Applets = CSapp_Applet_EditTV;
									break;
								case CSApp_Applet_TVFAVList:
									CSApp_SList_Applets = CSApp_Applet_EditTVFAV;
									break;
								case CSApp_Applet_RDList:
									CSApp_SList_Applets = CSapp_Applet_EditRadio;
									break;
								case CSApp_Applet_RADIOFAVList:
									CSApp_SList_Applets = CSApp_Applet_EditRADIOFAV;
									break;
								case CSApp_Applet_RADIOSATList:
									CSApp_SList_Applets = CSApp_Applet_EditRADIOSAT;
									break;
								default:
									CSApp_SList_Applets = CSapp_Applet_EditTV;
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
								sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(Sort_Str[i]));
							}
							stContents.u8TotalCount = SORT_NUM;
							
							MV_Draw_PopUp_Window( hwnd, stWindow, &stContents );
						}
						break;

					case CSAPP_KEY_BLUE:
#ifdef SUPORT_ADVANCE  // Advanced Channel List
						printf(" CSAPP_KEY_GREEN ================\n");
						switch( chlist_servicelist_type )
						{	
							case CSApp_Applet_TVSATList:
								CSApp_SList_Applets = CSApp_Applet_Ext_TVSATList;
								break;
							case CSApp_Applet_TVList:
								CSApp_SList_Applets = CSApp_Applet_Ext_TVList;
								break;
							case CSApp_Applet_TVFAVList:
								CSApp_SList_Applets = CSApp_Applet_Ext_TVFAVList;
								break;
							case CSApp_Applet_RDList:
								CSApp_SList_Applets = CSApp_Applet_Ext_RDList;
								break;
							case CSApp_Applet_RADIOFAVList:
								CSApp_SList_Applets = CSApp_Applet_Ext_RADIOFAVList;
								break;
							case CSApp_Applet_RADIOSATList:
								CSApp_SList_Applets = CSApp_Applet_Ext_RADIOSATList;
								break;
							default:
								CSApp_SList_Applets = CSApp_Applet_Ext_TVList;
								break;
						}
						SendMessage(hwnd,MSG_CLOSE,0,0);
#endif  // Advanced Channel List
						break;
						
					case CSAPP_KEY_TVRADIO:
						switch( chlist_servicelist_type )
						{
							case CSApp_Applet_RDList:
								chlist_servicelist_type = CSApp_Applet_TVList;
								break;
							case CSApp_Applet_TVList:
								chlist_servicelist_type = CSApp_Applet_RDList;
								break;
							case CSApp_Applet_TVFAVList:
								chlist_servicelist_type = CSApp_Applet_RADIOFAVList;
								break;
							case CSApp_Applet_RADIOFAVList:
								chlist_servicelist_type = CSApp_Applet_TVFAVList;
								break;
							case CSApp_Applet_TVSATList:
								chlist_servicelist_type = CSApp_Applet_RADIOSATList;
								break;
							case CSApp_Applet_RADIOSATList:
								chlist_servicelist_type = CSApp_Applet_TVSATList;
								break;
							default:
								break;
						}
						
						MV_Set_Current_List(chlist_servicelist_type, u8Sat_index);

						if(chlist_Total_Service>0)
						{
							chlist_Current_Service = CS_DB_GetCurrentService_OrderIndex();   

							chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);
							Prev_Service = chlist_Current_Service;

							if(APP_GetMainMenuStatus())
								SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 1, 0);
						}
						
						hdc = BeginPaint(hwnd);
						MV_Draw_CH_List(hdc);
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
							
							switch(chlist_servicelist_type)
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
#if 0
					case CSAPP_KEY_F1:
						if (chlist_triplet.sCS_DB_ServiceListType == eCS_DB_FAV_TV_LIST || chlist_triplet.sCS_DB_ServiceListType == eCS_DB_FAV_RADIO_LIST)
						{
							char 		TempStr[20];
							
							MV_DB_Get_Favorite_Name(TempStr, chlist_triplet.sCS_DB_ServiceListTypeValue);
							
							MV_Draw_Keypad(hwnd, TempStr, MAX_SAT_NAME_LANGTH);
						}
						break;
#endif
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
						if ( Get_Keypad_Status() != TRUE && MV_Get_Password_Flag() != TRUE)
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
#ifdef DEBUG_TEST
							u32Count_Loop++;
#endif
							if(chlist_Total_Service == 0)
								break;

							MV_Draw_CH_Focus(hwnd, chlist_Current_Service, UNFOCUS);

							chlist_Prev_Page = chlist_Current_Page;
							
							if(chlist_Current_Service == chlist_Total_Service-1)
								chlist_Current_Service = 0;
							else
								chlist_Current_Service++;

							chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);

							//hdc = BeginPaint(hwnd);
							MV_Draw_CH_Focus(hwnd, chlist_Current_Service, FOCUS);
							//MV_Draw_CH_List(hdc);
							//EndPaint(hwnd,hdc);
							//SendMessage(hwnd, MSG_PAINT, 0, 0);
							if ( CFG_Yinhe_Test )
							{
								usleep( 1000 * 1000 );
								SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_LEFT, 0);
							}
						}
						break;
					case CSAPP_KEY_UP:
						{
#ifdef DEBUG_TEST
							u32Count_Loop++;
#endif
							if(chlist_Total_Service == 0)
								break;

							MV_Draw_CH_Focus(hwnd, chlist_Current_Service, UNFOCUS);

							chlist_Prev_Page = chlist_Current_Page;

							if(chlist_Current_Service == 0)
								chlist_Current_Service = chlist_Total_Service-1;
							else
								chlist_Current_Service--;

							chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);

							MV_Draw_CH_Focus(hwnd, chlist_Current_Service, FOCUS);
							
							if ( CFG_Yinhe_Test )
							{
								usleep( 1000 * 1000 );
								SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_RIGHT, 0);
							}
						}
						break;

					case CSAPP_KEY_LEFT:
					case CSAPP_KEY_PG_UP:
						{
#ifdef DEBUG_TEST
							u32Count_Loop++;
#endif
							if(chlist_Total_Service == 0)
								break;

							if(chlist_Current_Service < SERVICES_NUM_PER_PAGE)
							{
								chlist_Current_Service = (getpage(chlist_Total_Service, SERVICES_NUM_PER_PAGE)+1)*SERVICES_NUM_PER_PAGE + chlist_Current_Service-SERVICES_NUM_PER_PAGE;
							}
							else
								chlist_Current_Service -= SERVICES_NUM_PER_PAGE;

							if(chlist_Current_Service>chlist_Total_Service-1)
								chlist_Current_Service = chlist_Total_Service-1;

							chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);

							hdc = BeginPaint(hwnd);
							MV_Draw_List_Full_Item(hdc);
							MV_Draw_CH_Info(hdc);
							EndPaint(hwnd,hdc);
							
							if ( CFG_Yinhe_Test )
							{
								usleep( 1000 * 1000 );
								SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_UP, 0);
							}
						}
						break;

					case CSAPP_KEY_RIGHT:
					case CSAPP_KEY_PG_DOWN:
						{
#ifdef DEBUG_TEST
							u32Count_Loop++;
#endif
							U8	current_page, total_page;

							if(chlist_Total_Service == 0)
								break;

							current_page = getpage((chlist_Current_Service+1), SERVICES_NUM_PER_PAGE);
							total_page = getpage(chlist_Total_Service, SERVICES_NUM_PER_PAGE);	

							chlist_Current_Service += SERVICES_NUM_PER_PAGE;
							if(chlist_Current_Service > chlist_Total_Service-1)
							{
								if(current_page<total_page)
									chlist_Current_Service = chlist_Total_Service-1;
								else
									chlist_Current_Service = 0;
							}

							chlist_Current_Focus = get_focus_line(&chlist_Current_Page, chlist_Current_Service, SERVICES_NUM_PER_PAGE);

							hdc = BeginPaint(hwnd);
							MV_Draw_List_Full_Item(hdc);
							MV_Draw_CH_Info(hdc);
							EndPaint(hwnd,hdc);

							if ( CFG_Yinhe_Test )
							{
								usleep( 1000 * 1000 );
								SendMessage(hwnd, MSG_KEYDOWN, CSAPP_KEY_PG_UP, 0);
							}
						}
						break;
						
					case CSAPP_KEY_ENTER:
						//printf("== Prev : %d , Current : %d =====================\n", CS_DB_GetCurrentService_OrderIndex(), chlist_Current_Service);
						if ( chlist_Current_Service == CS_DB_GetCurrentService_OrderIndex() )
						{
							CSApp_SList_Applets = CSApp_Applet_Desktop;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						} else {
							if (/*(chlist_Current_Service == CS_APP_GetLastUnlockServiceIndex()) && */(Prev_Service != chlist_Current_Service))
							{
								CS_APP_SetLastUnlockServiceIndex(0xffff);
							}

							Prev_Service = chlist_Current_Service;
							SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						}
						break;
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_ESC:
						{
							CSApp_SList_Applets = CSApp_Applet_Desktop;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
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


