#ifndef  _CS_APP_EDIT_SLIST_H_
#define  _CS_APP_EDIT_SLIST_H_

/*-------------------------- Edit List Title - Satellite ----------------------------*/
#define CHEDIT_TITLE_LEFT		90
#define CHEDIT_TITLE_TOP		122
#define CHEDIT_TITLE_RIGHT		1190
#define CHEDIT_TITLE_BOTTOM		152
#define CHEDIT_TITLE_DX			( CHEDIT_TITLE_RIGHT - CHEDIT_TITLE_LEFT )
#define CHEDIT_TITLE_DY			( CHEDIT_TITLE_BOTTOM - CHEDIT_TITLE_TOP )
/*-------------------------- Edit List List Component -------------------------------*/
#define CHEDIT_LIST_LEFT		CHEDIT_TITLE_LEFT + 10
#define CHEDIT_LIST_TOP			172
#define CHEDIT_LIST_RIGHT		600
#define CHEDIT_LIST_BOTTOM		( CHEDIT_LIST_TOP + CHEDIT_LIST_ITEM_DY * 10 )
#define CHEDIT_LIST_DX			( CHEDIT_LIST_RIGHT - CHEDIT_LIST_LEFT )
#define CHEDIT_LIST_DY			( CHEDIT_LIST_ITEM_DY * 10 )
#define CHEDIT_LIST_ITEM_DY		30
#define CHEDIT_LIST_ITEM1_X		100 + 10
#define CHEDIT_LIST_ITEM1_DX	70
#define CHEDIT_LIST_ITEM2_X		180
#define CHEDIT_LIST_ITEM2_DX	276
#define CHEDIT_LIST_ITEM3_X		456
#define CHEDIT_LIST_ITEM3_DX	24
#define CHEDIT_LIST_ITEM4_X		480
#define CHEDIT_LIST_ITEM4_DX	24
#define CHEDIT_LIST_ITEM5_X		504
#define CHEDIT_LIST_ITEM5_DX	24
#define CHEDIT_LIST_ITEM6_X		528
#define CHEDIT_LIST_ITEM6_DX	24
#define CHEDIT_LIST_ITEM7_X		552
#define CHEDIT_LIST_ITEM7_DX	24
#define CHEDIT_LIST_ITEM8_X		576
#define CHEDIT_LIST_ITEM8_DX	24
/*-------------------------- Edit List Help Button Icon -----------------------------*/
#define CHEDIT_SCROLL_LEFT		600
#define CHEDIT_SCROLL_TOP		CHEDIT_LIST_TOP
#define CHEDIT_SCROLL_RIGHT		620
#define CHEDIT_SCROLL_BOTTOM	CHEDIT_LIST_BOTTOM
#define CHEDIT_SCROLL_DX		( CHEDIT_SCROLL_RIGHT - CHEDIT_SCROLL_LEFT )
#define CHEDIT_SCROLL_DY		( CHEDIT_LIST_ITEM_DY * 10 )
/*-------------------------- Edit List Help Button Icon -----------------------------*/
#define CHEDIT_ICON_LEFT		CHEDIT_TITLE_LEFT
#define CHEDIT_ICON_TOP			572
#define CHEDIT_ICON_RIGHT		CHEDIT_TITLE_RIGHT
#define CHEDIT_ICON_BOTTOM		( CHEDIT_ICON_TOP + CHEDIT_LIST_ITEM_DY )
#define CHEDIT_ICON_DX			( CHEDIT_ICON_RIGHT - CHEDIT_ICON_LEFT )
#define CHEDIT_ICON_DY			CHEDIT_LIST_ITEM_DY

#define CHEDIT_TEXT_DX			150
#define CHEDIT_ICON1_X			CHEDIT_ICON_LEFT
#define CHEDIT_ICON1_TXT_X		( CHEDIT_ICON1_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
#define CHEDIT_ICON2_X			( CHEDIT_ICON1_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + CHEDIT_TEXT_DX )
#define CHEDIT_ICON2_TXT_X		( CHEDIT_ICON2_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
#define CHEDIT_ICON3_X			( CHEDIT_ICON2_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + CHEDIT_TEXT_DX )
#define CHEDIT_ICON3_TXT_X		( CHEDIT_ICON3_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
#define CHEDIT_ICON4_X			( CHEDIT_ICON3_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + CHEDIT_TEXT_DX )
#define CHEDIT_ICON4_TXT_X		( CHEDIT_ICON4_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
#define CHEDIT_ICON5_X			( CHEDIT_ICON4_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + CHEDIT_TEXT_DX )
#define CHEDIT_ICON5_TXT_X		( CHEDIT_ICON5_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
#define CHEDIT_ICON6_X			( CHEDIT_ICON5_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + CHEDIT_TEXT_DX )
#define CHEDIT_ICON6_TXT_X		( CHEDIT_ICON6_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 4 )
/*-------------------------- Edit List Sat & TP & Channel Info ----------------------*/
#define CHEDIT_INFO_LEFT		CHEDIT_TITLE_LEFT
#define CHEDIT_INFO_TOP			( CHEDIT_ICON_BOTTOM + 10 )
#define CHEDIT_INFO_RIGHT		CHEDIT_TITLE_RIGHT
#define CHEDIT_INFO_BOTTOM		( CHEDIT_INFO_TOP + CHEDIT_LIST_ITEM_DY )
#define CHEDIT_INFO_DX			( CHEDIT_INFO_RIGHT - CHEDIT_INFO_LEFT )
#define CHEDIT_INFO_DY			36
#define CHEDIT_INFO_TEXT_X		( CHEDIT_INFO_LEFT + 10 + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth )
/*-------------------------- Edit List List Sat & Channel Info ----------------------*/
#define CHEDIT_LIST_INFO_LEFT	CHEDIT_INFO_LEFT
#define CHEDIT_LIST_INFO_TOP	( CHEDIT_LIST_BOTTOM + 20 )
#define CHEDIT_LIST_INFO_RIGHT	CHEDIT_LIST_RIGHT + 10
#define CHEDIT_LIST_INFO_BOTTOM	( CHEDIT_LIST_INFO_TOP + CHEDIT_LIST_ITEM_DY * 2 )
#define CHEDIT_LIST_INFO_DX		( CHEDIT_LIST_INFO_RIGHT - CHEDIT_LIST_INFO_LEFT + 20 )
#define CHEDIT_LIST_INFO_DY		60
#define CHEDIT_LIST_INFO_TEXT_X	( CHEDIT_INFO_LEFT + 10 )
/*--------------------------------  Edit Warning Window  ----------------------------*/
#define CHEDIT_WARNING_WINDOW_X						314
#define	CHEDIT_WARNING_WINDOW_Y						214
#define	CHEDIT_WARNING_WINDOW_DX					612
#define	CHEDIT_WARNING_WINDOW_DY					162
#define	CHEDIT_WARNING_WINDOW_TITLE_X				320
#define	CHEDIT_WARNING_WINDOW_TITLE_Y				220
#define	CHEDIT_WARNING_WINDOW_TITLE_DX				600
#define	CHEDIT_WARNING_WINDOW_TITLE_DY				30
#define	CHEDIT_WARNING_WINDOW_CONTENT_X				320
#define	CHEDIT_WARNING_WINDOW_CONTENT_Y				254
#define	CHEDIT_WARNING_WINDOW_CONTENT_DX			600
#define	CHEDIT_WARNING_WINDOW_CONTENT_DY			116
#define	CHEDIT_WARNING_WINDOW_CONTENT_ITEM_DY		30
/*-----------------------------------------------------------------------------------*/

CSAPP_Applet_t CSApp_EditSList(CSAPP_Applet_t   slist_type);
void MV_Save_Change_Value(HWND hwnd);
void MV_Draw_ChEdit_Signal(HDC hdc);
void MV_Draw_ChEdit_List_State(HDC hdc);
void MV_Draw_ChEdit_Info_Bar(HDC hdc, U16 Item_index);
void MV_Draw_ChEdit_List_Info_Bar(HDC hdc, U16 Item_index);
void MV_Draw_ChEdit_Help_Button(HDC hdc);
void MV_Draw_ChEdit_List_Item(HDC hdc, int Index_Count, U16 Item_index, U16 Total_Service, U8 u8Kind);
void MV_Draw_ChEdit_List(HDC hdc, U16 u16Current_Service, U16 u16Current_Page, U16 u16Current_Focus, U16 u16Total_Service );
void EditSList_Draw_Confirm(HDC hdc);
void MV_Set_ChEdit_Current_List(U32 chlist_type, U8 u8Satlist_Sat_Index);
void Draw_ChEdit_Full(HDC hdc);
void Draw_ChEdit_Full2(HDC hdc);
int EditSList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

#endif

