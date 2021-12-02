#include "mv_menu_ctr.h"
#include "ch_install.h"
#include "ui_common.h"
#include "mv_new_menu.h"
#include "dvbtuner.h"

#ifdef SMART_PHONE

#define	CHLIST_WARNING_W		MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmWidth
#define	CHLIST_WARNING_H		MV_BMP[MVBMP_DESCTOP_MSG_PANEL].bmHeight
#define	CHLIST_WARNING_X		( SMART_WINDOW_X + SMART_WINDOW_DX + 200 )
#define	CHLIST_WARNING_Y		( (720-CHLIST_WARNING_H)/2-50 )

CSAPP_Applet_t					CSApp_Smart_Applets;

static MV_Menu_Item_t			MV_Menu_Item[MENU_ITEM_MAX + 2];
static eCSCh_ListClew			u8Warning_Status = MV_SIGNAL_UNLOCK;
static U8						u8Selected_Item = 0;
static U8						u8Prev_Sel_Item = 0;
static BOOL 					Mv_Move_Flag = FALSE;
static BOOL 					Mv_FirstBoot_Flag = TRUE;

BITMAP							TemBit;
BITMAP							TemBit2;

static void MV_Replace_Menu_Index(U8 u8Key)
{
	MV_Menu_Item_t		TempMenu_Item;
	MV_Menu_Item_t		Temp_Menu_Item[MENU_ITEM_MAX + 1];

	memset(&TempMenu_Item, 0x00, sizeof(MV_Menu_Item_t));
	memset(&Temp_Menu_Item, 0x00, sizeof(MV_Menu_Item_t) * ( MENU_ITEM_MAX + 1 ));
	
	switch(u8Key)
	{
		case CSAPP_KEY_DOWN:
			if ( ( u8Prev_Sel_Item / SERVICES_NUM_WIDTH ) == ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH ))
			{
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&Temp_Menu_Item, &MV_Menu_Item[u8Selected_Item], sizeof(MV_Menu_Item_t) * ( u8Prev_Sel_Item - u8Selected_Item) - 1);
				memcpy(&MV_Menu_Item[u8Selected_Item + 1], &Temp_Menu_Item, sizeof(MV_Menu_Item_t) * ( u8Prev_Sel_Item - u8Selected_Item) - 1);
				MV_Menu_Item[u8Selected_Item] = TempMenu_Item;
			} else {
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&Temp_Menu_Item, &MV_Menu_Item[u8Prev_Sel_Item + 1], sizeof(MV_Menu_Item_t) * SERVICES_NUM_WIDTH);
				memcpy(&MV_Menu_Item[u8Prev_Sel_Item], &Temp_Menu_Item, sizeof(MV_Menu_Item_t) * SERVICES_NUM_WIDTH);
				MV_Menu_Item[u8Selected_Item] = TempMenu_Item;
			}
			break;
			
		case CSAPP_KEY_UP:
			if ( u8Prev_Sel_Item/SERVICES_NUM_WIDTH == 0 )
			{
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&Temp_Menu_Item, &MV_Menu_Item[u8Prev_Sel_Item + 1], sizeof(MV_Menu_Item_t) * ( u8Selected_Item - u8Prev_Sel_Item ));
				memcpy(&MV_Menu_Item[u8Prev_Sel_Item], &Temp_Menu_Item, sizeof(MV_Menu_Item_t) * ( u8Selected_Item - u8Prev_Sel_Item ));
				MV_Menu_Item[u8Selected_Item] = TempMenu_Item;
			} else {
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&Temp_Menu_Item, &MV_Menu_Item[u8Selected_Item], sizeof(MV_Menu_Item_t) * SERVICES_NUM_WIDTH);
				memcpy(&MV_Menu_Item[u8Selected_Item + 1], &Temp_Menu_Item, sizeof(MV_Menu_Item_t) * SERVICES_NUM_WIDTH);
				MV_Menu_Item[u8Selected_Item] = TempMenu_Item;
			}
			break;
			
		case CSAPP_KEY_LEFT:
			if ( u8Prev_Sel_Item == 0 )
			{
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&MV_Menu_Item[0], &MV_Menu_Item[1], sizeof(MV_Menu_Item_t) * MENU_ITEM_MAX);
				MV_Menu_Item[MENU_ITEM_MAX] = TempMenu_Item;
			} else {
				TempMenu_Item = MV_Menu_Item[u8Selected_Item];
				MV_Menu_Item[u8Selected_Item] = MV_Menu_Item[u8Prev_Sel_Item];
				MV_Menu_Item[u8Prev_Sel_Item] = TempMenu_Item;
			}
			break;
			
		case CSAPP_KEY_RIGHT:
			if ( u8Prev_Sel_Item == MENU_ITEM_MAX )
			{
				TempMenu_Item = MV_Menu_Item[u8Prev_Sel_Item];
				memcpy(&Temp_Menu_Item, &MV_Menu_Item[0], sizeof(MV_Menu_Item_t) * MENU_ITEM_MAX);
				memcpy(&MV_Menu_Item[1], &Temp_Menu_Item, sizeof(MV_Menu_Item_t) * MENU_ITEM_MAX);
				MV_Menu_Item[0] = TempMenu_Item;
			} else {
				TempMenu_Item = MV_Menu_Item[u8Selected_Item];
				MV_Menu_Item[u8Selected_Item] = MV_Menu_Item[u8Prev_Sel_Item];
				MV_Menu_Item[u8Prev_Sel_Item] = TempMenu_Item;
			}
			break;
			
		default:
			break;
	}
	//MV_Menu_Item[u8Selected_Item];
	//MV_Menu_Item[u8Prev_Sel_Item];
}

static void MV_Menu_Notify(tMWNotifyData NotifyData)
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

void MV_Draw_Menu_Warning_Window(HDC hdc)
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

void MV_Close_Menu_Warning_Window(HDC hdc)
{
	SetBrushColor(hdc, COLOR_transparent);
	FillBox(hdc,ScalerWidthPixel(CHLIST_WARNING_X),ScalerHeigthPixel(CHLIST_WARNING_Y),ScalerWidthPixel(CHLIST_WARNING_W), ScalerHeigthPixel(CHLIST_WARNING_H));
}

void MV_Draw_Menu_List_Button(HDC hdc)
{
	if ( CFG_Menu_Back_Color.MV_R == 0 && CFG_Menu_Back_Color.MV_G == 0 && CFG_Menu_Back_Color.MV_B == 0 )
		MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	else
		// SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_A, CFG_Menu_Back_Color.MV_B, CFG_Menu_Back_Color.MV_G, 0xFF));
		SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_R, CFG_Menu_Back_Color.MV_G, CFG_Menu_Back_Color.MV_B, 0xFF));

	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X), ScalerHeigthPixel(SMART_WINDOW_ICON_Y),ScalerWidthPixel(SMART_WINDOW_DX),ScalerHeigthPixel(SMART_WINDOW_ITEM_DY));	
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if ( Mv_Move_Flag == TRUE )
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X), ScalerHeigthPixel(SMART_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_PASTE));
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2), ScalerHeigthPixel(SMART_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SELECT));
	} else {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X), ScalerHeigthPixel(SMART_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_MOVE));
		FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2), ScalerHeigthPixel(SMART_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_RESET));
	}
/*
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X), ScalerHeigthPixel(SMART_WINDOW_ICON_Y2), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y2), CS_MW_LoadStringByIdx(CSAPP_STR_SORT_KEY));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2), ScalerHeigthPixel(SMART_WINDOW_ICON_Y2), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SMART_WINDOW_ICON_X2)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(SMART_WINDOW_ICON_Y2), CS_MW_LoadStringByIdx(CSAPP_STR_EXTEND));
*/
}

void MV_Draw_Menu_List_Window(HDC hdc)
{
/**************************************  steel Image Cover Channel List outside ******************************************/
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X - 4 ), ScalerHeigthPixel(SMART_WINDOW_Y - 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth + 4), ScalerHeigthPixel(SMART_WINDOW_Y - 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X - 4 ), ScalerHeigthPixel(SMART_WINDOW_Y + SMART_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight + 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth + 4), ScalerHeigthPixel(SMART_WINDOW_Y + SMART_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight + 4), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_RIGHT]);
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X - 4), ScalerHeigthPixel(SMART_WINDOW_Y + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight - 12), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_LEFT_LINE].bmWidth), ScalerHeigthPixel(SMART_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2 + 8), &MV_BMP[MVBMP_SUBMENU_LEFT_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth - 4), ScalerHeigthPixel(SMART_WINDOW_Y + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight - 12), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_RIGHT_LINE].bmWidth), ScalerHeigthPixel(SMART_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2 + 8), &MV_BMP[MVBMP_SUBMENU_RIGHT_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth - 12), ScalerHeigthPixel(SMART_WINDOW_Y - 4), ScalerWidthPixel(SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2 + 8), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_TOP_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LINE]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SMART_WINDOW_X + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth - 12), ScalerHeigthPixel(SMART_WINDOW_Y + SMART_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight - 4), ScalerWidthPixel(SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2 + 8), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE]);
/**************************************  steel Image Cover Channel List outside ******************************************/

	if ( CFG_Menu_Back_Color.MV_R == 0 && CFG_Menu_Back_Color.MV_G == 0 && CFG_Menu_Back_Color.MV_B == 0 )
		MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	else
		SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_R, CFG_Menu_Back_Color.MV_G, CFG_Menu_Back_Color.MV_B, 0xFF));
		// SetBrushColor(hdc, RGBA2Pixel(hdc, CFG_Menu_Back_Color.MV_A, CFG_Menu_Back_Color.MV_B, CFG_Menu_Back_Color.MV_G, 0xFF));
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + 2), ScalerHeigthPixel(SMART_WINDOW_Y),ScalerWidthPixel(SMART_WINDOW_DX - 4),ScalerHeigthPixel(SMART_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X), ScalerHeigthPixel(SMART_WINDOW_Y + 2),ScalerWidthPixel(SMART_WINDOW_DX),ScalerHeigthPixel(SMART_WINDOW_DY - 4));	

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SMART_WINDOW_TITLE_Y),ScalerWidthPixel(SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(SMART_WINDOW_ITEM_DY));
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(SMART_WINDOW_LIST_Y),ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY));
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(SMART_WINDOW_INFOR_Y),ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_ITEM_DY));	

	MV_Draw_Menu_List_Button(hdc);
	
	FillCircle (hdc, 100, 100, 20);
}

void MV_Draw_Menu_Info(HDC hdc)
{
	char			buff2[256];
	RECT			TempRect;

	memset(buff2, 0x00, 256);
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(SMART_WINDOW_INFOR_Y),ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_ITEM_DY));

	TempRect.left = ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP) + 10;
	TempRect.right = TempRect.left + ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2) - 10;
	TempRect.top = SMART_WINDOW_INFOR_Y + 4;
	TempRect.bottom = TempRect.top + SMART_WINDOW_ITEM_DY;
	MV_MW_DrawText(hdc, CS_MW_LoadStringByIdx(MV_Menu_Item[u8Selected_Item].u16exp_Index) , -1, &TempRect, DT_CENTER | DT_VCENTER);
}

void MV_Draw_Menu_Clock(HDC hdc)
{
	struct tm		tm_time;
	struct timespec time_value;
	char			acTempStr[64];
	RECT			Temp_Rect;

	memset(acTempStr, 0x00, 64);
	clock_gettime(CLOCK_REALTIME, &time_value);

	memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	Temp_Rect.top = SMART_WINDOW_CLOCK_Y + 5;
	Temp_Rect.bottom = SMART_WINDOW_CLOCK_Y + SMART_WINDOW_CLOCK_DY - 10;
	Temp_Rect.left = SMART_WINDOW_CLOCK_X;
	Temp_Rect.right = SMART_WINDOW_CLOCK_X + SMART_WINDOW_CLOCK_DX - 40;
	sprintf(acTempStr, "%02d/%02d %02d:%02d:%02d", tm_time.tm_mday, tm_time.tm_mon + 1, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	MV_MW_DrawText_Fixed_Small(hdc, acTempStr, -1, &Temp_Rect, DT_RIGHT | DT_BOTTOM);
}

void MV_Draw_Menu_List_Title(HDC hdc)
{	
	RECT		rRect;
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SMART_WINDOW_TITLE_Y),ScalerWidthPixel(SMART_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(SMART_WINDOW_ITEM_DY));

	rRect.left = SMART_WINDOW_SIGNAL_X;
	rRect.right = rRect.left + 120;  //// No meaning .. not use
	rRect.top = SMART_WINDOW_SIGNAL_Y;
	rRect.bottom = rRect.top + 20;  //// No meaning .. not use
	MV_Draw_Menu_Signal(hdc, rRect);
	MV_Draw_Menu_Clock(hdc);
}

void MV_Draw_Menu_List_Item(HDC hdc, int Count_index, U8 FocusKind)
{
	U8		u8Col = 0;
	U8		u8Low = 0;
	U16		u16Start_X = 0, u16Start_Y = 0;
	RECT	TempRect;
	char	acTemStr[8];

	memset(acTemStr, 0x00, 8);
	
	u8Col = Count_index % SERVICES_NUM_WIDTH;
	u8Low = Count_index / SERVICES_NUM_WIDTH;

	u16Start_X = SMART_WINDOW_NO_X + ( u8Col * ( MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth + 25 ) ) + 10;
	u16Start_Y = SMART_WINDOW_LIST_Y + ( u8Low * ( MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight + 26 ) ) + 10;

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(u16Start_Y + MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight + 2),ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(24));		
	
	if( FocusKind == FOCUS )
	{
		if (Mv_Move_Flag == TRUE)
		{
			SetBrushColor(hdc, MVAPP_LIGHT_GREEN_COLOR);
			FillBox(hdc,ScalerWidthPixel(u16Start_X - 2), ScalerHeigthPixel(u16Start_Y - 2),ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth + 4),ScalerHeigthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight + 4));	
		} else {
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			FillBox(hdc,ScalerWidthPixel(u16Start_X - 2), ScalerHeigthPixel(u16Start_Y - 2),ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth + 4),ScalerHeigthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight + 4));	
		}
		FillBoxWithBitmap (hdc, ScalerWidthPixel(u16Start_X), ScalerHeigthPixel(u16Start_Y), ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth), ScalerHeigthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight), &MV_BMP[MV_Menu_Item[Count_index].u16Image_Index]);

		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		TempRect.left = u16Start_X + 2;
		TempRect.right = TempRect.left + ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth) - 2;;
		TempRect.top = u16Start_Y;
		TempRect.bottom = TempRect.top + ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight);
		sprintf(acTemStr, "%d", MV_Menu_Item[u8Selected_Item].u8Item_Index + 1);
		MV_MW_DrawText_Fixed_Small(hdc, acTemStr , -1, &TempRect, DT_LEFT | DT_VCENTER);
		
		if ( u8Col == 0 )
			TempRect.left = ScalerWidthPixel(u16Start_X);
		else if ( u8Col == SERVICES_NUM_WIDTH - 1 )
			TempRect.left = ScalerWidthPixel( ( u16Start_X + MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth) - 200);
		else
			TempRect.left = ScalerWidthPixel(u16Start_X) - 80;
		TempRect.right = TempRect.left + 200;
		TempRect.top = u16Start_Y + MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight + 4;
		TempRect.bottom = TempRect.top + SMART_WINDOW_ITEM_DY;

		if ( u8Col == 0 )
			MV_MW_DrawText_Fixed_Small(hdc, CS_MW_LoadStringByIdx(MV_Menu_Item[u8Selected_Item].u16Name_Index) , -1, &TempRect, DT_LEFT | DT_VCENTER);
		else if ( u8Col == SERVICES_NUM_WIDTH - 1 )
			MV_MW_DrawText_Fixed_Small(hdc, CS_MW_LoadStringByIdx(MV_Menu_Item[u8Selected_Item].u16Name_Index) , -1, &TempRect, DT_RIGHT | DT_VCENTER);
		else
			MV_MW_DrawText_Fixed_Small(hdc, CS_MW_LoadStringByIdx(MV_Menu_Item[u8Selected_Item].u16Name_Index) , -1, &TempRect, DT_CENTER | DT_VCENTER);
	}
	else 
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(u16Start_X - 2), ScalerHeigthPixel(u16Start_Y - 2),ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight + 4),ScalerHeigthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth + 4));	
		FillBoxWithBitmap (hdc, ScalerWidthPixel(u16Start_X), ScalerHeigthPixel(u16Start_Y), ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight), ScalerHeigthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth), &MV_BMP[MV_Menu_Item[Count_index].u16Image_Index]);

		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		TempRect.left = u16Start_X + 2;
		TempRect.right = TempRect.left + ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmWidth) - 2;;
		TempRect.top = u16Start_Y;
		TempRect.bottom = TempRect.top + ScalerWidthPixel(MV_BMP[MV_Menu_Item[Count_index].u16Image_Index].bmHeight);
		sprintf(acTemStr, "%d", MV_Menu_Item[Count_index].u8Item_Index + 1);
		MV_MW_DrawText_Fixed_Small(hdc, acTemStr , -1, &TempRect, DT_LEFT | DT_VCENTER);
	}
}

void MV_Draw_Menu_List_Full_Item(HDC hdc)
{
	int 	i, j;

	for(i = 0 ; i < SERVICES_NUM_HEIGHT ; i++)
	{
		for ( j = 0 ; j < SERVICES_NUM_WIDTH ; j++ )
		{
			if ( ( i * SERVICES_NUM_WIDTH + j ) > MENU_ITEM_MAX )
				return;
			
			MV_Draw_Menu_List_Item(hdc, ( i * SERVICES_NUM_WIDTH + j ), NOTFOCUS);
		}
	}
	MV_Draw_Menu_List_Item(hdc, u8Selected_Item, FOCUS);
}

void MV_Draw_Menu_List(HDC hdc)
{
	MV_Draw_Menu_List_Window(hdc);
	MV_Draw_Menu_List_Title(hdc);
	MV_Draw_Menu_List_Full_Item(hdc);
	MV_Draw_Menu_Info(hdc);
}

CSAPP_Applet_t CSApp_Smart_Menu(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;	

	CSApp_Smart_Applets = CSApp_Applet_Error;

	BASE_X = 0;
	BASE_Y = 0;
	WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
	HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);

	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "smart window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Smart_Menu_Msg_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_transparent;
	CreateInfo.dwAddData 		= 0;
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

	return CSApp_Smart_Applets;

}

int Smart_Menu_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC			hdc;

	switch (message)
	{
		case MSG_CREATE:
			{
				if ( Mv_FirstBoot_Flag == TRUE )
				{
					Mv_Default_Menu_Item(MV_Menu_Item);
					Mv_FirstBoot_Flag = FALSE;
				}
				
				Mv_Move_Flag = FALSE;
				CS_MW_SVC_Open(MV_Menu_Notify);
				
				SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
			}
			break;

		case MSG_TIMER:
			if(wparam == CHECK_SIGNAL_TIMER_ID)
			{
				hdc = BeginPaint(hwnd);
				MV_Draw_Menu_List_Title(hdc);
				EndPaint(hwnd,hdc);
			}

			break;
			   
 		case MSG_PAINT:	
			hdc = BeginPaint(hwnd);
			MV_Draw_Menu_List(hdc);
			
				memset(&TemBit, 0x00, sizeof(BITMAP));
				memset(&TemBit2, 0x00, sizeof(BITMAP));
	
			GetBitmapFromDC (hdc, ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(SMART_WINDOW_LIST_Y), ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY), &TemBit);
			GetBitmapFromDC (hdc, ScalerWidthPixel((SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP) - (SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2)), ScalerHeigthPixel(SMART_WINDOW_LIST_Y), ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY), &TemBit2);
			EndPaint(hwnd,hdc);
			return 0;
			break;
            
		case MSG_VIDEO_FORFMAT_UPDATE:
			{
				
			}
			break;
			
		case MSG_CHECK_SERVICE_LOCK:
			
			break;

		case MSG_PLAYSERVICE:
			{	
			}
			break;

		case MSG_UPDATE_FE:
			hdc = BeginPaint(hwnd);
			if(wparam == FE_LOCK)
			{
				MV_Close_Menu_Warning_Window(hdc);
			}
			else if((wparam == FE_UNLOCK)||(wparam == FE_LOST))
			{
				if (( MV_Get_Password_Flag() != TRUE ) &&
					( MV_Get_PopUp_Window_Status() != TRUE ) &&
					( MV_Get_Report_Window_Status() != TRUE ))
				{
					u8Warning_Status = MV_SIGNAL_UNLOCK;
					MV_Draw_Menu_Warning_Window(hdc);
				}
			}
			EndPaint(hwnd,hdc);
			break;

		case MSG_CLOSE:
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			DestroyMainWindow (hwnd);                            
			PostQuitMessage (hwnd);
			break;

		case MSG_PIN_INPUT:
			MV_Draw_Password_Window(hwnd);
			break;
			
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
				CSApp_Smart_Applets = CSApp_Applet_Sleep;
				SendMessage(hwnd,MSG_CLOSE,0,0);
				break;
				
			case CSAPP_KEY_TV_AV:
				ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				break;
					
			}

			if (MV_Get_Password_Flag() == TRUE)
			{
				break;
			}

			if(MV_Get_Jump_Flag() == TRUE)
			{
				break;
			}

			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				break;
			}

			else if ( Get_Keypad_Status() == TRUE )
			{
				break;
			}	
			else
			{
				switch(wparam)
				{
					case CSAPP_KEY_RED:
						if( Mv_Move_Flag == TRUE )
						{
							Mv_Move_Flag = FALSE;
						}
						else
						{
							Mv_Move_Flag = TRUE;
						}
						
						hdc = BeginPaint(hwnd);
						MV_Draw_Menu_List_Full_Item(hdc);
						MV_Draw_Menu_List_Button(hdc);
						EndPaint(hwnd,hdc);
						break;
						
					case CSAPP_KEY_GREEN:
						if( Mv_Move_Flag == FALSE )
						{
							Mv_Default_Menu_Item(MV_Menu_Item);
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
						
					case CSAPP_KEY_YELLOW:

						break;

					case CSAPP_KEY_BLUE:
						#if 1
						{
							int 		x = 0;
							U16			Start_x = 0;

							//for ( y = 0 ; y < 10 ; y++ )
							//{
						        for ( x = 0 ; x < 7 ; x++ ) {
									Start_x = SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP - (((SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2)/7) * (x + 1));
									hdc = BeginPaint(hwnd);
									SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
									FillBox(hdc,ScalerWidthPixel(SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP), ScalerHeigthPixel(SMART_WINDOW_LIST_Y),ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY));
									FillBoxWithBitmap (hdc, ScalerWidthPixel(Start_x), ScalerHeigthPixel(SMART_WINDOW_LIST_Y), ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY), &TemBit);
									FillBoxWithBitmap (hdc, ScalerWidthPixel((SMART_WINDOW_X + SMART_WINDOW_ITEM_GAP) - (SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2)), ScalerHeigthPixel(SMART_WINDOW_LIST_Y), ScalerWidthPixel(SMART_WINDOW_DX - SMART_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(SMART_WINDOW_LIST_DY), &TemBit2);
									EndPaint(hwnd,hdc);
									usleep(20);
						        }
							
								hdc = BeginPaint(hwnd);
								MV_Draw_Menu_List_Full_Item(hdc);
								EndPaint(hwnd,hdc);
							//}
						}
#else
						usleep(1000 * 1000);
						hdc = BeginPaint(hwnd);
						StretchBlt(hdc, 80, 80, 40, 40, hdc, 100, 200, 100, 100, 0);
						EndPaint(hwnd,hdc);
#endif
						break;
						
					case CSAPP_KEY_TVRADIO:

						break;
						
					case CSAPP_KEY_SAT:

						break;

					case CSAPP_KEY_FAVOLIST:
						
						break;

					case CSAPP_KEY_F2: 

						break;
						
					case CSAPP_KEY_F1:
						
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
						}
						break;
						
					case CSAPP_KEY_DOWN:
						if( Mv_Move_Flag == TRUE )
						{
							u8Prev_Sel_Item = u8Selected_Item;
							
							if ( ( u8Selected_Item / SERVICES_NUM_WIDTH ) == ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH ))
								u8Selected_Item -= SERVICES_NUM_WIDTH * ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH );
							else 
								u8Selected_Item += SERVICES_NUM_WIDTH;

							MV_Replace_Menu_Index(CSAPP_KEY_DOWN);
							
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							if ( ( u8Selected_Item / SERVICES_NUM_WIDTH ) == ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH ))
								u8Selected_Item -= SERVICES_NUM_WIDTH * ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH );
							else 
								u8Selected_Item += SERVICES_NUM_WIDTH;

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, FOCUS);
							MV_Draw_Menu_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
					case CSAPP_KEY_UP:
						if( Mv_Move_Flag == TRUE )
						{
							u8Prev_Sel_Item = u8Selected_Item;
							
							if ( ( u8Selected_Item / SERVICES_NUM_WIDTH ) == 0 )
								u8Selected_Item += SERVICES_NUM_WIDTH * ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH );
							else 
								u8Selected_Item -= SERVICES_NUM_WIDTH;

							MV_Replace_Menu_Index(CSAPP_KEY_UP);

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							if ( ( u8Selected_Item / SERVICES_NUM_WIDTH ) == 0 )
								u8Selected_Item += SERVICES_NUM_WIDTH * ( MENU_ITEM_MAX / SERVICES_NUM_WIDTH );
							else 
								u8Selected_Item -= SERVICES_NUM_WIDTH;

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, FOCUS);
							MV_Draw_Menu_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_LEFT:
						if( Mv_Move_Flag == TRUE )
						{
							u8Prev_Sel_Item = u8Selected_Item;
							
							if ( u8Selected_Item == 0 )
								u8Selected_Item = MENU_ITEM_MAX;
							else 
								u8Selected_Item--;

							MV_Replace_Menu_Index(CSAPP_KEY_LEFT);

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							if ( u8Selected_Item == 0 )
								u8Selected_Item = MENU_ITEM_MAX;
							else 
								u8Selected_Item--;

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, FOCUS);
							MV_Draw_Menu_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;

					case CSAPP_KEY_RIGHT:
						if( Mv_Move_Flag == TRUE )
						{
							u8Prev_Sel_Item = u8Selected_Item;
							
							if ( u8Selected_Item == MENU_ITEM_MAX )
								u8Selected_Item = 0;
							else 
								u8Selected_Item++;

							MV_Replace_Menu_Index(CSAPP_KEY_RIGHT);

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							EndPaint(hwnd,hdc);
						}
						else
						{
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, UNFOCUS);
							EndPaint(hwnd,hdc);
							
							if ( u8Selected_Item == MENU_ITEM_MAX )
								u8Selected_Item = 0;
							else 
								u8Selected_Item++;

							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Item(hdc, u8Selected_Item, FOCUS);
							MV_Draw_Menu_Info(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
						
					case CSAPP_KEY_ENTER:
						if( Mv_Move_Flag == TRUE )
						{
							Mv_Move_Flag = FALSE;
							
							hdc = BeginPaint(hwnd);
							MV_Draw_Menu_List_Full_Item(hdc);
							MV_Draw_Menu_List_Button(hdc);
							EndPaint(hwnd,hdc);
						} else {
							CSApp_Smart_Applets = MV_Menu_Item[u8Selected_Item].u16Menu_Link;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
						break;
						
					case CSAPP_KEY_MENU:
					case CSAPP_KEY_ESC:
						{
							UnloadBitmap (&TemBit);
							UnloadBitmap (&TemBit2);
							CSApp_Smart_Applets = CSApp_Applet_Desktop;
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
#endif  //#ifdef SMART_PHONE

