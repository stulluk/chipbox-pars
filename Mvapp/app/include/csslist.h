#ifndef  _CS_SLIST_H_
#define  _CS_SLIST_H_

typedef enum
{
	MV_SIGNAL_UNLOCK = 0,
	MV_NO_SERVICE,
	MV_SERVICE_ENCRYPT
} eCSCh_ListClew;

CSAPP_Applet_t CSApp_SList(CSAPP_Applet_t   slist_type);
void MV_Draw_CH_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind);
void MV_Draw_CH_List(HDC hdc);
void MV_Draw_List_Full_Item(HDC hdc);
void MV_Draw_List_Item(HDC hdc, int Count_index, U16 u16Focusindex, U8 FocusKind);
void MV_Draw_List_Title(HDC hdc);
void MV_Draw_CH_Info(HDC hdc);
void MV_Draw_CH_List_Window(HDC hdc);
int SList_Msg_cb (HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
#endif

