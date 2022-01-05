#ifdef SMART_PHONE

#ifndef  _MV_SMART_H_
#define  _MV_SMART_H_

#include "database.h"

#define SMART_WINDOW_X				660
#define SMART_WINDOW_Y				200
#define	SMART_WINDOW_ITEM_DX		21
#define	SMART_WINDOW_DX				426
#define SMART_WINDOW_DY				410
#define SMART_WINDOW_ITEM_GAP		10
#define SMART_WINDOW_ITEM_DY		30
#define	SMART_WINDOW_ITEM_HEIGHT	(SMART_WINDOW_ITEM_DY + SMART_WINDOW_ITEM_GAP)
#define SMART_WINDOW_NO_X			680
#define SMART_WINDOW_NO_DX			80
#define SMART_WINDOW_NAME_X			( SMART_WINDOW_NO_X + SMART_WINDOW_NO_DX )
#define SMART_WINDOW_NAME_DX		210
#define SMART_WINDOW_SCRAMBLE_X 	( SMART_WINDOW_NAME_X + SMART_WINDOW_NAME_DX )
#define SMART_WINDOW_LOCK_X			( SMART_WINDOW_SCRAMBLE_X + SMART_WINDOW_ITEM_DX )
#define SMART_WINDOW_FAVORITE_X		( SMART_WINDOW_LOCK_X + SMART_WINDOW_ITEM_DX )
#define SMART_WINDOW_SDHD_X			( SMART_WINDOW_FAVORITE_X + SMART_WINDOW_ITEM_DX )
#define	SMART_WINDOW_TITLE_Y		208
#define	SMART_WINDOW_LIST_Y			250
#define	SMART_WINDOW_LIST_DY		270
#define	SMART_WINDOW_INFOR_Y		530
#define	SMART_WINDOW_ICON_X			SMART_WINDOW_NO_X
#define	SMART_WINDOW_ICON_X2		(SMART_WINDOW_X + SMART_WINDOW_DX/2)
#define	SMART_WINDOW_ICON_Y			570
//#define	SMART_WINDOW_ICON_Y2		570
#define	SMART_WINDOW_SIGNAL_X		( SMART_WINDOW_NO_X + 20 )
#define	SMART_WINDOW_SIGNAL_Y		( SMART_WINDOW_TITLE_Y + 5 )
#define SMART_WINDOW_SIGNAL_DX		SMART_WINDOW_NAME_DX
#define SMART_WINDOW_SIGNAL_DY		( SMART_WINDOW_ITEM_DY - 10 )
#define	SMART_WINDOW_CLOCK_X		( SMART_WINDOW_SIGNAL_X + SMART_WINDOW_NAME_DX )
#define	SMART_WINDOW_CLOCK_Y		SMART_WINDOW_TITLE_Y
#define SMART_WINDOW_CLOCK_DX		( SMART_WINDOW_DX - SMART_WINDOW_NAME_DX - 40 )
#define SMART_WINDOW_CLOCK_DY		SMART_WINDOW_ITEM_DY

CSAPP_Applet_t CSApp_Smart_Menu(void);
void MV_Draw_Menu_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind);
void MV_Draw_Menu_List(HDC hdc);
void MV_Draw_Menu_List_Full_Item(HDC hdc);
void MV_Draw_Menu_List_Item(HDC hdc, int Count_index, U8 FocusKind);
void MV_Draw_List_Title(HDC hdc);
void MV_Draw_Menu_Info(HDC hdc);
void MV_Draw_Menu_List_Window(HDC hdc);
int Smart_Menu_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
#endif // #ifndef  _MV_SMART_H_

#endif // #ifdef SMART_PHONE
