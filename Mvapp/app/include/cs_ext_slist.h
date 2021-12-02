#ifndef  _CS_EXT_SLIST_H_
#define  _CS_EXT_SLIST_H_

enum
{
	CSAPP_CH_FIRST = 0,
	CSAPP_CH_SECOND = 1,
	CSAPP_CH_THIRD = 2
};

CSAPP_Applet_t CSApp_Ext_SList(CSAPP_Applet_t   slist_type);
void MV_Draw_Ext_CH_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind, U8 u8Ch_Focus);
void MV_Draw_Ext_CH_List(HDC hdc);
void MV_Draw_Ext_List_Full_Item_1(HDC hdc);
void MV_Draw_Ext_List_Full_Item_2(HDC hdc);
void MV_Draw_Ext_List_Full_Item_3(HDC hdc);
void MV_Draw_Ext_List_Item(HDC hdc, int Count_index, U16 u16Focusindex, U8 FocusKind, U8 u8List_Kind);
void MV_Draw_Ext_List_Title(HDC hdc);
void MV_Draw_Ext_CH_Info(HDC hdc);
void MV_Draw_Ext_CH_List_Window(HDC hdc);
int Ext_SList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
#endif

