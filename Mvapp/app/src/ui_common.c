#include "ui_common.h"
#include "mvosapi.h"
#include "mv_menu_ctr.h"
#include "csupgrade.h"
#include "timer.h"
#include "mvtimer.h"
#include "ctype.h"
#include "dvbtuner.h"

#define CHA					1
#define NO					2
#define UnSelect			1
#define Select				2
#define STR_BUFF_STRLEN_MAX 50
#define ITEM_PER_PAGE		10

static U8					Keypad_X;
static U8					Keypad_Y;
static U8					u8max_str_length;
static U8					Keypad_Index=0;

static U8					HexaKeypad_X;
static U8					HexaKeypad_Y;
static U8					u8max_Hexastr_length;
static U8					HexaKeypad_Index=0;

static BOOL 				b8Keypad_Cancel_Key;
static BOOL 				b8Keypad_Ok_Key;
static BOOL 				b8Keypad_Space_Key;
static BOOL					Scroll_Status = FALSE;

static U8 					Returnbuff[STR_BUFF_STRLEN_MAX];
static U8 					Returnbuff_Temp[STR_BUFF_STRLEN_MAX];
RECT						TmpRect;

RECT						Capture_WinRect;
RECT						Scroll_Rect;
RECT						jump_rect;
RECT						Password_rect;
BOOL						b8PopUp_Status = FALSE;
BITMAP						PopUpBmp;
U16							Window_PosX = 0;
U16							Window_PosY = 0;
U16							Window_PosDX = 0;
U8							u8Current_Focus = 0;
U8							u8Return_Focus = 0;
U16							Scroll_Total = 0;
U16							Scroll_Current = 0;
U8							u8StartIndex = 0;
U8							u8LastIndex = 0;
U8							Jump_Value_index = 0;
U8							Password_Value_index = 0;
tWindow_Title				u8PopUp_Kind = eMV_TITLE_MAX;

stPopUp_Window_Contents		stTemp_Contents;

U8 							Returnbuff_HexaTemp[256];
char						Returnbuff_Jump[5];

static U16					u16File_Focus;
static U16					u16Current_FilePage;
static U16					u16Prev_FilePage;
static U16					u16Current_Fileindex;

static BOOL					b8Save_Flag = FALSE;
static BOOL					b8Attention_Flag = FALSE;
static BOOL					b8jump_Flag = FALSE;
static BOOL					b8Password_Flag = FALSE;
static BOOL					Confirm_YesNo = FALSE;
static BOOL					Confirm_Window_Flag = FALSE;
static BOOL					File_Window_Flag = FALSE;
static BOOL					b8Check_Enter = FALSE;
static BOOL					b8Timer_Window_Flag = FALSE;
static BOOL					b8Timer_Save_Flag = FALSE;
static char					acReturn_File[FILE_MAX_NAME_LENGTH];
static char					acReturn_File_Ext[FILE_MAX_NAME_LENGTH];
static pthread_t  			hAni_TaskHandle;
extern U32					*Tuner_HandleId;

U16 WinTitle_Str[eMV_TITLE_MAX] = {
	CSAPP_STR_SORT_KEY,
	CSAPP_STR_SATELLITE,
	CSAPP_STR_FAV_LIST,
	CSAPP_STR_ALL,
	CSAPP_STR_ATTENTION,
	CSAPP_STR_WARNING
};

char	PVR_Check_String[PVR_DATA_KIND][PVR_DATA_LEN] = {
								"ID",
								"SA",
								"TP",
								"TI",			/* Event Tiel */
								"ST",			/* Record Start UTC Time */
								"SD",			/* Record Start OS Time For Sec Count */
								"ET",			/* Record End UTC Time */
								"ED",			/* Record End OS Time For Sec Count */
								"LP"
							};

static BOOL		b8keypad_save_str;

/* By KB Kim for Plugin Setting : 2011.05.07 */
#ifdef PLUGIN_MENU

/* For Plugin Site List by File : KB Kim 2011.09.13 */
#define MAX_UPGRADE_LIST_COUNT          256
#define MAX_UPGRADE_NAME_COUNT          256

static eMV_NET_Upgrade_Items	CurrentNetUpgradeMode;
static BOOL						NetConfirm_YesNo = FALSE;
static BOOL						NetConfirm_Window_Flag = FALSE;
static BOOL						Download_Status = FALSE;
static stMV_Upgrade_Info		stUpgrade_Data[MAX_UPGRADE_LIST_COUNT];
static stMV_Plugin_Info	    	stPlugin_Data[MAX_UPGRADE_LIST_COUNT];
static U16						u16Upgrade_Data_Count = 0; /* For Plugin Site List by File : KB Kim 2011.09.13 */
static U16				    	u16Plugin_Data_Count = 0;  /* For Plugin Site List by File : KB Kim 2011.09.13 */
/**************************************************************************/
/* For Plugin Site List by File : KB Kim 2011.09.13 */
static BOOL						PlugInSite_List = FALSE;
static char	    	            PluginSite_Data[MAX_UPGRADE_LIST_COUNT][MAX_UPGRADE_NAME_COUNT];
static char 					CurrentSite[MAX_UPGRADE_NAME_COUNT];

static U16				    	u16PluginSite_Count = 0;
static U16						u16Current_Siteindex = 0;

static BOOL						Wget_File_List = FALSE;
static U16						u16Wget_Focus;
static U16						u16Current_WgetPage;
static U16						u16Prev_WgetPage;
static U16						u16Current_Wgetindex;
static pthread_t  				hProgress_TaskHandle;
static pthread_t				hDownload_TaskHandle;
static long long				DownloadFileSize = 1;
#endif

extern void EditSList_Draw_Confirm(HDC hdc);
extern void EditSList_Close_Confirm(HDC hdc);

/* For Key Emulation By KB Kim 2011.05.13 */
void Mv_BroadcastMessage(int msg, U32 param, U32 subParam)
{
	BroadcastMessage (msg, (WPARAM)param, (LPARAM)subParam);
}

void MvSendMotorMove(U8 moveOn)
{
	BroadcastMessage(MSG_MOTOR_MOVING, moveOn, 0);
}

/* By KB Kim 2011.05.28 */
void MvSendRcuKeyData(U32 keyValue)
{
	BroadcastMessage(MSG_KEYDOWN, keyValue, 0);
}

long long MV_Trans_String_to_long(char *Temp_Str)
{
	int 		StrLen;
	int 		i;
	int			j = 0;
	long long 	Return_Value = 0;

	StrLen = strlen(Temp_Str);

	for ( i = 0 ; i < StrLen ; i++ )
	{
		if ( !isdigit(Temp_Str[i]) )
		{
			return 0;
		} else {
			switch(Temp_Str[i])
			{
				case '0':
					j = 0;
					break;
				case '1':
					j = 1;
					break;
				case '2':
					j = 2;
					break;
				case '3':
					j = 3;
					break;
				case '4':
					j = 4;
					break;
				case '5':
					j = 5;
					break;
				case '6':
					j = 6;
					break;
				case '7':
					j = 7;
					break;
				case '8':
					j = 8;
					break;
				case '9':
					j = 9;
					break;
				default:
					j = 0;
					break;
			}

			Return_Value += j * (long long)pow(10.0, (double)(StrLen - i - 1));
			//printf("=== %lld ====\n", Return_Value);
		}
	}
	return Return_Value;
}

void MV_Set_Save_Flag(BOOL b8TureOrFalse)
{
	b8Save_Flag = b8TureOrFalse;
}

BOOL MV_Get_Save_Flag(void)
{
	return b8Save_Flag;
}

BOOL MV_Get_Attention_Flag(void)
{
	return b8Attention_Flag;
}

BOOL MV_Get_Jump_Flag(void)
{
	return b8jump_Flag;
}

BOOL MV_Get_Password_Flag(void)
{
	return b8Password_Flag;
}

/************************************************************************************************/
/*****************************  Popup Window PROCESS ********************************************/
/************************************************************************************************/

BOOL MV_Get_PopUp_Window_Status(void)
{
	return b8PopUp_Status;
}

U8 MV_Get_PopUp_Window_Result(void)
{
	return u8Return_Focus;
}

tWindow_Title MV_Get_PopUp_Window_Kind(void)
{
	return u8PopUp_Kind;
}

void MV_Capture_PopUp_Window(HDC hdc)
{
	memset(&PopUpBmp, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC (hdc, ScalerWidthPixel(Capture_WinRect.left), ScalerHeigthPixel(Capture_WinRect.top), ScalerWidthPixel(Capture_WinRect.right - Capture_WinRect.left), ScalerHeigthPixel(Capture_WinRect.bottom - Capture_WinRect.top), &PopUpBmp);
}

void MV_Restore_PopUp_Window(HDC hdc)
{
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(Capture_WinRect.left), ScalerHeigthPixel(Capture_WinRect.top), ScalerWidthPixel(Capture_WinRect.right - Capture_WinRect.left), ScalerHeigthPixel(Capture_WinRect.bottom - Capture_WinRect.top), &PopUpBmp);
	UnloadBitmap (&PopUpBmp);
}

void MV_Draw_PopUp_Title_Bar(HDC hdc, RECT *Title_Rect, tWindow_Title title_i)
{
	RECT	Temp_Rect;

	Temp_Rect.top = Title_Rect->top;
	Temp_Rect.left = Title_Rect->left;
	Temp_Rect.bottom = Title_Rect->bottom;
	Temp_Rect.right = Title_Rect->right;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel((Title_Rect->right - Title_Rect->left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);

	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Title_Rect->top + 4;
	Temp_Rect.left = Title_Rect->left + 4;
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(WinTitle_Str[title_i]), -1, &Temp_Rect, DT_CENTER);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Temp_Rect.top - 2;
	Temp_Rect.left = Temp_Rect.left - 2;
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(WinTitle_Str[title_i]), -1, &Temp_Rect, DT_CENTER);
}

void MV_Draw_PopUp_Title_Bar_ByName(HDC hdc, RECT *Title_Rect, U32 title_i)
{
	RECT	Temp_Rect;

	Temp_Rect.top = Title_Rect->top;
	Temp_Rect.left = Title_Rect->left;
	Temp_Rect.bottom = Title_Rect->bottom;
	Temp_Rect.right = Title_Rect->right;

	if ( MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight == 0 )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel((Title_Rect->right - Title_Rect->left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);
	} else {
		MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(Title_Rect->left), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(Title_Rect->right - Title_Rect->left), ScalerHeigthPixel(Title_Rect->bottom - Title_Rect->top) );
	}

	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Title_Rect->top + 4;
	Temp_Rect.left = Title_Rect->left + 4;
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(title_i), -1, &Temp_Rect, DT_CENTER);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Temp_Rect.top - 2;
	Temp_Rect.left = Temp_Rect.left - 2;
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(title_i), -1, &Temp_Rect, DT_CENTER);
}

void MV_Draw_PopUp_Title_Bar_ByStr(HDC hdc, RECT *Title_Rect, char *title_str)
{
	RECT	Temp_Rect;

	Temp_Rect.top = Title_Rect->top;
	Temp_Rect.left = Title_Rect->left;
	Temp_Rect.bottom = Title_Rect->bottom;
	Temp_Rect.right = Title_Rect->right;

	if ( MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight == 0 )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel((Title_Rect->right - Title_Rect->left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Title_Rect->right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);
	} else {
		MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(Title_Rect->left), ScalerHeigthPixel(Title_Rect->top), ScalerWidthPixel(Title_Rect->right - Title_Rect->left), ScalerHeigthPixel(Title_Rect->bottom - Title_Rect->top) );
	}

	SetTextColor(hdc,CSAPP_BLACK_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Title_Rect->top + 4;
	Temp_Rect.left = Title_Rect->left + 4;
	CS_MW_DrawText(hdc, title_str, -1, &Temp_Rect, DT_CENTER);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	Temp_Rect.top = Temp_Rect.top - 2;
	Temp_Rect.left = Temp_Rect.left - 2;
	CS_MW_DrawText(hdc, title_str, -1, &Temp_Rect, DT_CENTER);
}

void MV_Draw_PopUp_Item(HDC hdc, U8 u8Item_Num, U8 u8Focus_Kind)
{
	int		Y_pos = 0;
	U8		u8ItemIndex = 0;

	if ( stTemp_Contents.u8TotalCount > ITEM_PER_PAGE )
		u8ItemIndex = u8Item_Num + u8StartIndex;
	else
		u8ItemIndex = u8Item_Num;

	Y_pos = Window_PosY + ( u8Item_Num * MV_INSTALL_MENU_BAR_H );

	if ( u8Focus_Kind == FOCUS )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(Window_PosX), ScalerHeigthPixel(Y_pos), ScalerWidthPixel(Window_PosDX), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	}
	else if ( 1 == u8Item_Num%2 )
	{
		SetBrushColor(hdc, MVAPP_DARKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(Window_PosX), ScalerHeigthPixel(Y_pos), ScalerWidthPixel(Window_PosDX), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight));
	}
	else
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(Window_PosX), ScalerHeigthPixel(Y_pos), ScalerWidthPixel(Window_PosDX), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_SELBAR].bmHeight));
	}

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	//printf("u8Item_Num : %d :: stTemp_Contents.Contents[u8Item_Num] : %s \n", u8Item_Num, stTemp_Contents.Contents[u8Item_Num]);
	//printf("u8ItemIndex : %d :: stTemp_Contents.Contents[u8ItemIndex] : %s \n", u8ItemIndex, stTemp_Contents.Contents[u8ItemIndex]);
	CS_MW_TextOut( hdc, Window_PosX + 10 , Y_pos+4, stTemp_Contents.Contents[u8ItemIndex] );
}

void MV_Draw_Popup_Full_Item(HDC hdc, U8 Startindex, U8 LastIndex, U8 FocusIndex)
{
	int		i;

	//printf("MV_Draw_Popup_Full_Item :: Startindex : %d , LastIndex : %d , %d , %d\n", Startindex, LastIndex, LastIndex - Startindex, FocusIndex);
	for ( i = 0 ; i < ( LastIndex - Startindex) ; i++ )
	{
		//printf("%d ==== stContents.Contents[%d] : %s\n", stTemp_Contents.u8TotalCount, i, stTemp_Contents.Contents[i]);
		if ( i + Startindex == FocusIndex )
			MV_Draw_PopUp_Item(hdc, i, FOCUS);
		else
			MV_Draw_PopUp_Item(hdc, i, UNFOCUS);
	}
}

void MV_Draw_PopUp_Window(HWND hwnd, stPopUp_Window stWindow, stPopUp_Window_Contents *stContents)
{
	HDC			hdc = 0;
	U16			Win_x = 0;
	U16			Win_y = 0;
	U16			Win_Dx = 0;
	U16			Win_Dy = 0;
	RECT		Title_Rect;

	if ( stWindow.b8Scroll_OnOff )
		Scroll_Total = 0;

	b8PopUp_Status = TRUE;
	u8Current_Focus = stWindow.u16Current_Pos;
	b8Attention_Flag = FALSE;
	memset( &stTemp_Contents, 0x00, sizeof(stPopUp_Window_Contents));
	memcpy( &stTemp_Contents, stContents, sizeof(stPopUp_Window_Contents));

/*
	for ( i = 0 ; i < stContents->u8TotalCount ; i++ )
		printf("%d ==== stContents.Contents[%d] : %s\n", stContents->u8TotalCount, i, stContents->Contents[i]);

	for ( i = 0 ; i < stTemp_Contents.u8TotalCount ; i++ )
		printf("%d ==== stContents.Contents[%d] : %s\n", stTemp_Contents.u8TotalCount, i, stTemp_Contents.Contents[i]);
*/

	Capture_WinRect.top = stWindow.Window_Rect.top;
	Capture_WinRect.bottom = stWindow.Window_Rect.bottom;
	Capture_WinRect.left = stWindow.Window_Rect.left;
	Capture_WinRect.right = stWindow.Window_Rect.right;
	u8Current_Focus = stWindow.u16Current_Pos;
	u8PopUp_Kind = stWindow.tTitle;

	hdc = BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

/**************************** Rectagle Background outside ***************************************/
	Win_x = stWindow.Window_Rect.left;
	Win_y = stWindow.Window_Rect.top;
	Win_Dx = stWindow.Window_Rect.right - stWindow.Window_Rect.left;
	Win_Dy = stWindow.Window_Rect.bottom - stWindow.Window_Rect.top;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(Win_x), ScalerHeigthPixel(Win_y),ScalerWidthPixel(Win_Dx),ScalerHeigthPixel(Win_Dy));

/**************************** Rectagle Background blue color ************************************/
	Win_x = Win_x + 4;
	Win_y = Win_y + 4;
	if ( stWindow.b8Scroll_OnOff == TRUE )
		Win_Dx = Win_Dx - 8 - SCROLL_BAR_DX;
	else
		Win_Dx = Win_Dx - 8;
	Win_Dy = Win_Dy - 8;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(Win_x), ScalerHeigthPixel(Win_y),ScalerWidthPixel(Win_Dx),ScalerHeigthPixel(Win_Dy));

/********************************* DrawBar for Title  *******************************************/
	if ( stWindow.tTitle != eMV_NO_TITLE )
	{
		Title_Rect.left = Win_x + 8;
		Title_Rect.top = Win_y + 8;
		if ( stWindow.b8Scroll_OnOff )
			Title_Rect.right = stWindow.Window_Rect.right - 12 - SCROLL_BAR_DX;
		else
			Title_Rect.right = stWindow.Window_Rect.right - 12;
		Title_Rect.bottom = Title_Rect.top + MV_INSTALL_MENU_BAR_H;
		MV_Draw_PopUp_Title_Bar(hdc, &Title_Rect, stWindow.tTitle);
	}
/**************************** Rectagle contents list color **************************************/
	Win_x = Win_x + 8;
	if ( stWindow.tTitle != eMV_NO_TITLE )
		Win_y = Win_y + 46;
	else
		Win_y = Win_y + 6;

	if ( stWindow.b8Scroll_OnOff )
		Win_Dx = Win_Dx - 16;
	else
		Win_Dx = Win_Dx - 16;

	if ( stWindow.tTitle == eMV_NO_TITLE )
	{
		Win_Dy = Win_Dy - 18;
	} else {
		if ( stWindow.b8YesNo_Button == TRUE )
			Win_Dy = Win_Dy - 94;
		else
			Win_Dy = Win_Dy - 54;
	}
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(Win_x), ScalerHeigthPixel(Win_y),ScalerWidthPixel(Win_Dx),ScalerHeigthPixel(Win_Dy));

	Window_PosX = Win_x;
	Window_PosY = Win_y;
	Window_PosDX = Win_Dx;

	//printf("MV_Draw_PopUp_Window :: u8Current_Focus : %d <- %d : %d\n", u8Current_Focus, stWindow.u16Current_Pos, stTemp_Contents.u8TotalCount);

	if ( stTemp_Contents.u8TotalCount > ITEM_PER_PAGE )
	{
		if ( u8Current_Focus + ITEM_PER_PAGE > stTemp_Contents.u8TotalCount )
		{
			u8StartIndex = stTemp_Contents.u8TotalCount - ITEM_PER_PAGE;
			u8LastIndex = stTemp_Contents.u8TotalCount;
		} else {
			u8StartIndex = u8Current_Focus;
			u8LastIndex = u8StartIndex + ITEM_PER_PAGE;
		}
		MV_Draw_Popup_Full_Item(hdc, u8StartIndex, u8LastIndex, u8Current_Focus);
	}
	else
		MV_Draw_Popup_Full_Item(hdc, 0, stTemp_Contents.u8TotalCount, stWindow.u16Current_Pos);
/*
	for ( i = 0 ; i < stTemp_Contents.u8TotalCount ; i++ )
	{
		if ( i == stWindow.u16Current_Pos )
			MV_Draw_PopUp_Item(hdc, i, FOCUS);
		else
			MV_Draw_PopUp_Item(hdc, i, UNFOCUS);
	}
*/
	if ( stWindow.b8Scroll_OnOff )
	{
		Scroll_Rect.top = Window_PosY;
		Scroll_Rect.bottom = Window_PosY + Win_Dy + 15;
		Scroll_Rect.left = Window_PosX + Win_Dx + 8;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Total = stTemp_Contents.u8TotalCount - 1;

		MV_Draw_ScrollBar(hdc, Scroll_Rect, stWindow.u16Current_Pos, Scroll_Total, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}

	EndPaint(hwnd,hdc);
}

BOOL MV_PopUp_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
			hdc = BeginPaint(hwnd);
			if ( stTemp_Contents.u8TotalCount < ITEM_PER_PAGE )
				MV_Draw_PopUp_Item(hdc, u8Current_Focus, UNFOCUS);
			else
			{
				if ( u8Current_Focus < u8LastIndex )
					MV_Draw_PopUp_Item(hdc, u8Current_Focus - u8StartIndex, UNFOCUS);
			}

        	if ( u8Current_Focus >= stTemp_Contents.u8TotalCount - 1 )
        		u8Current_Focus = 0;
        	else
        		u8Current_Focus++;

			if ( stTemp_Contents.u8TotalCount < ITEM_PER_PAGE )
				MV_Draw_PopUp_Item(hdc, u8Current_Focus, FOCUS);
			else
			{
				if ( u8Current_Focus > u8LastIndex - 1 )
				{
					u8StartIndex = u8StartIndex + 1;
					u8LastIndex = u8StartIndex + ITEM_PER_PAGE;

					MV_Draw_Popup_Full_Item(hdc, u8StartIndex, u8LastIndex, u8Current_Focus);
				}
				else if ( u8Current_Focus == 0 )
				{
					u8StartIndex = u8Current_Focus;
					u8LastIndex = ITEM_PER_PAGE;
					MV_Draw_Popup_Full_Item(hdc, u8StartIndex, u8LastIndex, u8Current_Focus);
				}
				else
					MV_Draw_PopUp_Item(hdc, u8Current_Focus - u8StartIndex, FOCUS);

				MV_Draw_ScrollBar(hdc, Scroll_Rect, u8Current_Focus, Scroll_Total, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
			}
			EndPaint(hwnd,hdc);
        	break;

        case CSAPP_KEY_UP:
        	hdc = BeginPaint(hwnd);
			if ( stTemp_Contents.u8TotalCount < ITEM_PER_PAGE )
				MV_Draw_PopUp_Item(hdc, u8Current_Focus, UNFOCUS);
			else
			{
				if ( u8Current_Focus > u8StartIndex )
					MV_Draw_PopUp_Item(hdc, u8Current_Focus - u8StartIndex, UNFOCUS);
			}

        	if ( u8Current_Focus <= 0 )
        		u8Current_Focus = stTemp_Contents.u8TotalCount - 1;
        	else
        		u8Current_Focus--;

			if ( stTemp_Contents.u8TotalCount < ITEM_PER_PAGE )
				MV_Draw_PopUp_Item(hdc, u8Current_Focus, FOCUS);
			else
			{
				if ( u8Current_Focus == stTemp_Contents.u8TotalCount - 1 )
				{
					u8StartIndex = u8Current_Focus - ITEM_PER_PAGE + 1;
					u8LastIndex = u8StartIndex + ITEM_PER_PAGE;
					MV_Draw_Popup_Full_Item(hdc, u8StartIndex, u8LastIndex, u8Current_Focus);
				} else if ( u8Current_Focus < u8StartIndex )
				{
					u8StartIndex = u8Current_Focus;
					u8LastIndex = u8LastIndex - 1;
					MV_Draw_Popup_Full_Item(hdc, u8StartIndex, u8LastIndex, u8Current_Focus);
				}
				else
					MV_Draw_PopUp_Item(hdc, u8Current_Focus - u8StartIndex, FOCUS);

				MV_Draw_ScrollBar(hdc, Scroll_Rect, u8Current_Focus, Scroll_Total, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
			}
			EndPaint(hwnd,hdc);
        	break;

        case CSAPP_KEY_LEFT:
        	if ( stTemp_Contents.u8TotalCount != stTemp_Contents.u8TotalCount )
        	{

        	}
        	break;

        case CSAPP_KEY_RIGHT:
        	if ( stTemp_Contents.u8TotalCount != stTemp_Contents.u8TotalCount )
        	{

        	}
        	break;

        case CSAPP_KEY_ENTER:
			if ( b8Save_Flag )
			{
				b8Save_Flag = FALSE;
				b8Attention_Flag = TRUE;
				hdc = BeginPaint(hwnd);
    			EditSList_Draw_Confirm(hdc);
				EndPaint(hwnd,hdc);
			} else {
				b8PopUp_Status = FALSE;
				u8Return_Focus = u8Current_Focus;
				hdc = BeginPaint(hwnd);
				if ( b8Attention_Flag == TRUE )
					EditSList_Close_Confirm(hdc);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
				b8Attention_Flag = FALSE;
			}
			return FALSE;

        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			if ( b8Attention_Flag )
			{
				b8Save_Flag = FALSE;
				b8Attention_Flag = FALSE;
				b8PopUp_Status = FALSE;
				u8Return_Focus = u8Current_Focus;
				hdc = BeginPaint(hwnd);
				EditSList_Close_Confirm(hdc);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
			} else {
				b8PopUp_Status = FALSE;
				b8Attention_Flag = FALSE;
	            hdc = BeginPaint(hwnd);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
			}
			return FALSE;
		case CSAPP_KEY_IDLE:
			b8PopUp_Status = FALSE;
			b8Attention_Flag = FALSE;
			hdc = BeginPaint(hwnd);
			MV_Restore_PopUp_Window( hdc );
			EndPaint(hwnd,hdc);
			return FALSE;
		case CSAPP_KEY_TV_AV:
			// ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
			break;

    }
	return TRUE;
}

/************************************************************************************************/
/*********************************  SAT List Window Process *************************************/
/************************************************************************************************/

void MV_Draw_Satlist_Window(HWND hwnd)
{
	stPopUp_Window 			stWindow;
	stPopUp_Window_Contents stContents;
	U8						stCount;
	U8						i;
	MV_stSatInfo			Temp_SatData;

	memset( &stWindow, 0x00, sizeof(stPopUp_Window));
	memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
	sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ALL_SAT));
	stCount = 1;
	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
		{
			MV_GetSatelliteData_ByIndex(&Temp_SatData, i);
			sprintf(stContents.Contents[stCount], "%s", Temp_SatData.acSatelliteName);
			stCount++;
		}
	}

	stWindow.b8Scroll_OnOff = FALSE;
	stWindow.b8YesNo_Button = FALSE;
	stWindow.u8Help_Kind = 0;
	stWindow.u16Total = stCount;
	stWindow.u16Current_Pos = 0;
	stWindow.u8Item_Num = stCount;
	stWindow.tTitle = eMV_TITLE_SAT;
	if ( stCount > 10 )
		stWindow.Window_Rect.top = 100;
	else
		stWindow.Window_Rect.top = 200;
	stWindow.Window_Rect.left = 500;
	stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( MV_INSTALL_MENU_BAR_H * stCount ) + 58));
	stWindow.Window_Rect.right = 780;
	stContents.u8TotalCount = stCount;
	stContents.u8Focus_Position= 0;

	MV_Draw_PopUp_Window( hwnd, stWindow, &stContents );
}

void MV_Draw_Favlist_Window(HWND hwnd, U8 u8TVRadio, BOOL b8TrueORFalse)
{
	stPopUp_Window 			stWindow;
	stPopUp_Window_Contents stContents;
	U8						stCount;
	U8						i;
	char 					TempStr[20];

	memset( &stWindow, 0x00, sizeof(stPopUp_Window));
	memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
	stCount = 0;

	/* By KB Kim : 20110712 */
	if ( b8TrueORFalse )
	{
		sprintf(stContents.Contents[stCount], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ALL_SAT));
		stCount++;
	}

	for ( i = 1 ; i < MV_MAX_FAV_KIND + 1 ; i++ )
	{
		if ( b8TrueORFalse )
		{
			if ( MV_Get_ServiceCount_at_Favorite(u8TVRadio, i - 1) > 0 )
			{
				MV_DB_Get_Favorite_Name(TempStr, i - 1);
				sprintf(stContents.Contents[stCount], "%s %d", TempStr, MV_Get_ServiceCount_at_Favorite(u8TVRadio, i - 1));
				stCount++;
			}
		} else {
			MV_DB_Get_Favorite_Name(TempStr, i - 1);
			sprintf(stContents.Contents[stCount], "%s %d", TempStr, MV_Get_ServiceCount_at_Favorite(u8TVRadio, i - 1));
			stCount++;
		}
	}

	if (b8TrueORFalse && ( stCount <= 1 )) /* By KB Kim : 20110712 */
	{
		MV_Warning_Report_Window_Open( hwnd, MV_WINDOW_NO_FAV_CHANNEL );
		usleep( 2000*1000 );
		MV_Warning_Report_Window_Close( hwnd );
	} else {
		stWindow.b8Scroll_OnOff = FALSE;
		stWindow.b8YesNo_Button = FALSE;
		stWindow.u8Help_Kind = 0;
		stWindow.u16Total = stCount;
		stWindow.u16Current_Pos = 0;
		stWindow.u8Item_Num = stCount;
		stWindow.tTitle = eMV_TITLE_FAV;
		stWindow.Window_Rect.top = 200;
		stWindow.Window_Rect.left = 500;
		stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( MV_INSTALL_MENU_BAR_H * stCount ) + 58));
		stWindow.Window_Rect.right = 780;
		stContents.u8TotalCount = stCount;
		stContents.u8Focus_Position= 0;

		MV_Draw_PopUp_Window( hwnd, stWindow, &stContents );
	}
}

void MV_Draw_SatFavlist_Window(HWND hwnd, U8 u8TVRadio)
{
	stPopUp_Window 			stWindow;
	stPopUp_Window_Contents stContents;
	U8						stCount;
	U8						i;
	MV_stSatInfo			Temp_SatData;
	char 					TempStr[20];

	memset( &stWindow, 0x00, sizeof(stPopUp_Window));
	memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
	sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ALL_SAT));
	stCount = 1;
	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
		{
			MV_GetSatelliteData_ByIndex(&Temp_SatData, i);
			sprintf(stContents.Contents[stCount], "%s", Temp_SatData.acSatelliteName);
			stCount++;
		}
	}

	for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Favorite(u8TVRadio, i) > 0 )
		{
			MV_DB_Get_Favorite_Name(TempStr, i);
			sprintf(stContents.Contents[stCount], "%s %d", TempStr, MV_Get_ServiceCount_at_Favorite(u8TVRadio, i));
			stCount++;
		}
	}

	stWindow.b8Scroll_OnOff = FALSE;
	stWindow.b8YesNo_Button = FALSE;
	stWindow.u8Help_Kind = 0;
	stWindow.u16Total = stCount;
	stWindow.u16Current_Pos = 0;
	stWindow.u8Item_Num = stCount;
	stWindow.tTitle = eMV_TITLE_SAT_FAV;
	if ( stCount > 10 )
		stWindow.Window_Rect.top = 100;
	else
		stWindow.Window_Rect.top = 200;
	stWindow.Window_Rect.left = 500;
	stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( MV_INSTALL_MENU_BAR_H * stCount ) + 58));
	stWindow.Window_Rect.right = 780;
	stContents.u8TotalCount = stCount;
	stContents.u8Focus_Position= 0;

	MV_Draw_PopUp_Window( hwnd, stWindow, &stContents );
}

void MV_Draw_NoName_Window(HWND hwnd, RECT *reRect, stPopUp_Window_Contents *stContents)
{
	stPopUp_Window 			stWindow;
	U8						stCount;

	memset( &stWindow, 0x00, sizeof(stPopUp_Window));
	stCount = stContents->u8TotalCount;

	if ( stCount > ITEM_PER_PAGE )
	{
		Scroll_Status = TRUE;
		stWindow.b8Scroll_OnOff = TRUE;
	}
	else
		stWindow.b8Scroll_OnOff = FALSE;

	stWindow.b8YesNo_Button = FALSE;
	stWindow.u8Help_Kind = 0;
	stWindow.u16Total = stCount;
	stWindow.u16Current_Pos = stContents->u8Focus_Position;
	//printf("MV_Draw_NoName_Window :: stWindow.u16Current_Pos : %d <- %d \n", stWindow.u16Current_Pos, stContents->u8Focus_Position);
	stWindow.u8Item_Num = stCount;
	stWindow.tTitle = eMV_NO_TITLE;
	stWindow.Window_Rect.top = reRect->top;
	stWindow.Window_Rect.left = reRect->left;

	if ( stCount > ITEM_PER_PAGE )
		stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( MV_INSTALL_MENU_BAR_H * ITEM_PER_PAGE ) + 18));
	else
		stWindow.Window_Rect.bottom = (stWindow.Window_Rect.top + (( MV_INSTALL_MENU_BAR_H * stCount ) + 18));

//	printf("\n=== %d : %d - %d  ====\n", stCount, stWindow.Window_Rect.top, stWindow.Window_Rect.bottom);

	stWindow.Window_Rect.right = reRect->right;

	MV_Draw_PopUp_Window( hwnd, stWindow, stContents );
}

/************************************************************************************************/
/**********************************  JUMP PROCESS ***********************************************/
/************************************************************************************************/

void MV_Jump_Retrun_Value(char *Temp)
{
	strcpy( Temp, Returnbuff_Jump );
}

void MV_Jump_Value_Display(HWND hwnd)
{
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);
	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(jump_rect.left), ScalerHeigthPixel(jump_rect.top),ScalerWidthPixel(jump_rect.right - jump_rect.left),ScalerHeigthPixel(jump_rect.bottom - jump_rect.top));
	CS_MW_DrawText(hdc, Returnbuff_Jump, -1, &jump_rect, DT_CENTER);
	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Jump_Window(HWND hwnd, RECT *reRect)
{
	RECT	rc1;
	HDC		hdc;

	b8jump_Flag = TRUE;
	Jump_Value_index = 0;
	memset(Returnbuff_Jump, 0x00, 5 );
	Capture_WinRect.top = reRect->top;
	Capture_WinRect.bottom = reRect->bottom;
	Capture_WinRect.left = reRect->left;
	Capture_WinRect.right = reRect->right;

	hdc = MV_BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

	rc1.left = reRect->left;
	rc1.top = reRect->top;
	rc1.right = reRect->right;
	rc1.bottom = reRect->bottom;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + JUMP_WINDOW_OUTGAP;
	rc1.top = rc1.top + JUMP_WINDOW_OUTGAP;
	rc1.right = rc1.right - JUMP_WINDOW_OUTGAP;
	rc1.bottom = rc1.bottom - JUMP_WINDOW_OUTGAP;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + JUMP_WINDOW_OUTGAP;
	rc1.top = rc1.top + JUMP_WINDOW_OUTGAP;
	rc1.right = rc1.right - JUMP_WINDOW_OUTGAP;
	rc1.bottom = rc1.top + JUMP_WINDOW_ITEM_DY;
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_JUMP);

	rc1.top = rc1.top + JUMP_WINDOW_ITEM_DY + 8;
	rc1.bottom = rc1.top + JUMP_WINDOW_ITEM_DY + 10;
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	jump_rect.left = rc1.left + 10;
	jump_rect.right = rc1.right - 10;
	jump_rect.top = rc1.top + 5;
	jump_rect.bottom = rc1.bottom - 5;

	MV_EndPaint(hwnd,hdc);

	MV_Jump_Value_Display(hwnd);
}

BOOL MV_Jump_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
		case CSAPP_KEY_0:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '0';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_1:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '1';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_2:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '2';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_3:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '3';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_4:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '4';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_5:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '5';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_6:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '6';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_7:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '7';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_8:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '8';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

		case CSAPP_KEY_9:
			if ( Jump_Value_index > 3 )
				break;
			Returnbuff_Jump[Jump_Value_index] = '9';
			Jump_Value_index++;
			MV_Jump_Value_Display(hwnd);
			break;

        case CSAPP_KEY_LEFT:
			if ( Jump_Value_index > 0 )
			{
				Jump_Value_index--;
				Returnbuff_Jump[Jump_Value_index] = 0x00;
				MV_Jump_Value_Display(hwnd);
			}
        	break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			{
				b8jump_Flag = FALSE;
				hdc = BeginPaint(hwnd);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
			}
			return FALSE;
    }
	return TRUE;
}

/************************************************************************************************/
/********************************  PASSWORD PROCESS *********************************************/
/************************************************************************************************/

void MV_Password_Set_Flag(BOOL bflag)
{
	b8Password_Flag = bflag;
}

BOOL MV_Password_Retrun_Value(void)
{
	//printf("MV_Password_Retrun_Value :: Password : %s\n", CS_DBU_GetPinCode());
	return Pin_Verify( Returnbuff_Jump );
}

char *MV_Password_Return_Str(void)
{
	return Returnbuff_Jump;
}

void MV_Password_Value_Display(HWND hwnd)
{
	HDC		hdc;
	char	Temp[2];
	int		i;

	hdc = MV_BeginPaint(hwnd);
	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);

	for( i = 0 ; i < 4 ; i++ )
	{
		memset( Temp, 0x00, sizeof(char)*2 );

		if ( Returnbuff_Jump[i] == 0x00 )
			sprintf(Temp, "%c", '-');
		else
			sprintf(Temp, "%c", '*' /*Returnbuff_Jump[i]*/);

		//printf("==== %d , %d , %d , %d ====\n", ScalerWidthPixel(Password_rect.left + ((PASSWORD_WINDOW_ITEM_DY + 20) * i) + 20), ScalerHeigthPixel(Password_rect.top),ScalerWidthPixel(PASSWORD_WINDOW_ITEM_DY),ScalerHeigthPixel(PASSWORD_WINDOW_ITEM_DY));
		FillBox(hdc,ScalerWidthPixel(Password_rect.left + ((PASSWORD_WINDOW_ITEM_DY + 20) * i)), ScalerHeigthPixel(Password_rect.top),ScalerWidthPixel(PASSWORD_WINDOW_ITEM_DY),ScalerHeigthPixel(PASSWORD_WINDOW_ITEM_DY));
		CS_MW_TextOut( hdc, ScalerWidthPixel(Password_rect.left + ((PASSWORD_WINDOW_ITEM_DY + 20) * i) + 8), ScalerHeigthPixel(Password_rect.top + 2), Temp );
	}
	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Password_Window(HWND hwnd)
{
	RECT	rc1;
	HDC		hdc;

	b8Password_Flag = TRUE;
	Password_Value_index = 0;
	memset(Returnbuff_Jump, 0x00, 5 );
	Capture_WinRect.left = PASSWORD_WINDOW_STARTX;
	Capture_WinRect.top = PASSWORD_WINDOW_STARTY;
	Capture_WinRect.right = Capture_WinRect.left + PASSWORD_WINDOW_STARTDX;
	Capture_WinRect.bottom = Capture_WinRect.top + PASSWORD_WINDOW_STARTDY;

	hdc = MV_BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

	rc1.left = PASSWORD_WINDOW_STARTX;
	rc1.top = PASSWORD_WINDOW_STARTY;
	rc1.right = rc1.left + PASSWORD_WINDOW_STARTDX;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_STARTDY;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.bottom - PASSWORD_WINDOW_OUTGAP;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_PASSWORD);

	rc1.top = rc1.top + PASSWORD_WINDOW_ITEM_DY + 8;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY + 60;
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.top = rc1.top + 10;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_INPUT_PIN), -1, &rc1, DT_CENTER);

	Password_rect.left = 550;
	Password_rect.right = Password_rect.left + 180;
	Password_rect.top = rc1.top + 40;
	Password_rect.bottom = Password_rect.top + PASSWORD_WINDOW_ITEM_DY;

	MV_EndPaint(hwnd,hdc);

	MV_Password_Value_Display(hwnd);
}

void MV_Draw_PIN_Window(HWND hwnd, char *Str_Temp)
{
	RECT	rc1;
	HDC		hdc;

	b8Password_Flag = TRUE;
	Password_Value_index = 0;
	memset(Returnbuff_Jump, 0x00, 5 );
	Capture_WinRect.left = PASSWORD_WINDOW_STARTX;
	Capture_WinRect.top = PASSWORD_WINDOW_STARTY;
	Capture_WinRect.right = Capture_WinRect.left + PASSWORD_WINDOW_STARTDX;
	Capture_WinRect.bottom = Capture_WinRect.top + PASSWORD_WINDOW_STARTDY;

	hdc = MV_BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

	rc1.left = PASSWORD_WINDOW_STARTX;
	rc1.top = PASSWORD_WINDOW_STARTY;
	rc1.right = rc1.left + PASSWORD_WINDOW_STARTDX;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_STARTDY;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.bottom - PASSWORD_WINDOW_OUTGAP;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	MV_Draw_PopUp_Title_Bar_ByStr(hdc, &rc1, Str_Temp);

	rc1.top = rc1.top + PASSWORD_WINDOW_ITEM_DY + 8;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY + 60;
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.top = rc1.top + 10;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, Str_Temp, -1, &rc1, DT_CENTER);

	Password_rect.left = 550;
	Password_rect.right = Password_rect.left + 180;
	Password_rect.top = rc1.top + 40;
	Password_rect.bottom = Password_rect.top + PASSWORD_WINDOW_ITEM_DY;

	MV_EndPaint(hwnd,hdc);

	MV_Password_Value_Display(hwnd);
}

void MV_Draw_PIN_Window_For_CI(HWND hwnd, char *Str_Temp)
{
	RECT	rc1;
	HDC		hdc;

	b8Password_Flag = TRUE;
	Password_Value_index = 0;
	memset(Returnbuff_Jump, 0x00, 5 );
	Capture_WinRect.left = PASSWORD_WINDOW_STARTX_CI;
	Capture_WinRect.top = PASSWORD_WINDOW_STARTY;
	Capture_WinRect.right = Capture_WinRect.left + PASSWORD_WINDOW_STARTDX_CI;
	Capture_WinRect.bottom = Capture_WinRect.top + PASSWORD_WINDOW_STARTDY;

	hdc = MV_BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

	rc1.left = PASSWORD_WINDOW_STARTX_CI;
	rc1.top = PASSWORD_WINDOW_STARTY;
	rc1.right = rc1.left + PASSWORD_WINDOW_STARTDX_CI;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_STARTDY;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.bottom - PASSWORD_WINDOW_OUTGAP;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + PASSWORD_WINDOW_OUTGAP;
	rc1.top = rc1.top + PASSWORD_WINDOW_OUTGAP;
	rc1.right = rc1.right - PASSWORD_WINDOW_OUTGAP;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	MV_Draw_PopUp_Title_Bar_ByStr(hdc, &rc1, Str_Temp);

	rc1.top = rc1.top + PASSWORD_WINDOW_ITEM_DY + 8;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY + 60;
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.top = rc1.top + 10;
	rc1.bottom = rc1.top + PASSWORD_WINDOW_ITEM_DY;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, Str_Temp, -1, &rc1, DT_CENTER);

	Password_rect.left = 550;
	Password_rect.right = Password_rect.left + 180;
	Password_rect.top = rc1.top + 40;
	Password_rect.bottom = Password_rect.top + PASSWORD_WINDOW_ITEM_DY;

	MV_EndPaint(hwnd,hdc);

	MV_Password_Value_Display(hwnd);
}

BOOL MV_Password_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
		case CSAPP_KEY_0:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '0';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_1:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '1';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_2:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '2';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_3:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '3';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_4:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '4';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_5:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '5';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_6:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '6';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_7:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '7';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_8:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '8';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

		case CSAPP_KEY_9:
			if ( Password_Value_index > 3 )
				break;
			Returnbuff_Jump[Password_Value_index] = '9';
			Password_Value_index++;
			MV_Password_Value_Display(hwnd);
			break;

        case CSAPP_KEY_LEFT:
			if ( Password_Value_index > 0 )
			{
				Password_Value_index--;
				Returnbuff_Jump[Password_Value_index] = 0x00;
				MV_Password_Value_Display(hwnd);
			}
        	break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			{
				b8Password_Flag = FALSE;
				hdc = BeginPaint(hwnd);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
			}
			return FALSE;
    }

	if ( Password_Value_index == 3 )
		return FALSE;
	else
	return TRUE;
}

/************************************************************************************************/
/*********************************  KEYPAD PROCESS **********************************************/
/************************************************************************************************/

void MV_Draw_Keypad(HWND hwnd, U8 *item_str, U8 max_string_length)
{
	HDC		hdc;
	RECT	Temp_Rect;

	Keypad_X = 0;
	Keypad_Y = 1;
	keypad_enable = TRUE;
	u8max_str_length = max_string_length;
	Returnbuff_Temp[0] = '\0';
	b8keypad_save_str = FALSE;

	TmpRect.left	=KEYBOARD_INPUT_WINDOW_STRING_STARTX;
	TmpRect.right	=KEYBOARD_INPUT_WINDOW_STRING_STARTX + KEYBOARD_STARTDX - WINDOW_OUT_GAP*6;
	TmpRect.top		=KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4;
	TmpRect.bottom	=KEYBOARD_INPUT_WINDOW_STRING_STARTY + MV_INSTALL_MENU_BAR_H;

	if(item_str[0] == '\0')
	{
		Keypad_Index = 0;
		memset(item_str,0x00,(max_string_length+1));
	}
	else
		Keypad_Index = strlen(item_str);

#ifdef ENABLE_CH_FIND
	if( FlowCtl_GetCurrentState() == FS_CH_LIST_TV_1
	 || FlowCtl_GetCurrentState() == FS_CH_LIST_RADIO_1 )
	{
		memcpy(Returnbuff_Temp,"\0",STR_BUFF_STRLEN_MAX);
	}
	else
#endif
		strcpy(Returnbuff_Temp,item_str);

	hdc = MV_BeginPaint(hwnd);

	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(KEYBOARD_STARTX), ScalerHeigthPixel(KEYBOARD_STARTY), ScalerWidthPixel(KEYBOARD_STARTDX), ScalerHeigthPixel(KEYBOARD_STARTDY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(KEYBOARD_STARTX), ScalerHeigthPixel(KEYBOARD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(KEYBOARD_STARTX + KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(KEYBOARD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(KEYBOARD_STARTX), ScalerHeigthPixel(KEYBOARD_STARTY + KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(KEYBOARD_STARTX + KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(KEYBOARD_STARTY + KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(KEYBOARD_STARTX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(KEYBOARD_STARTY),ScalerWidthPixel(KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(KEYBOARD_STARTDY));
	FillBox(hdc,ScalerWidthPixel(KEYBOARD_STARTX), ScalerHeigthPixel(KEYBOARD_STARTY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(KEYBOARD_STARTDX),ScalerHeigthPixel(KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= KEYBOARD_STARTY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= KEYBOARD_STARTX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + KEYBOARD_STARTDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_KEY_BOARD);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_STARTX + WINDOW_OUT_GAP), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*2), ScalerHeigthPixel(WINDOW_ITEM_DY) );

	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );

	Draw_keypad(hdc, Returnbuff_Temp);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY - 30),	CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));

	FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY + 8), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(WINDOW_ITEM_Y + WINDOW_ITEM_DY + 8),	CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));

	MV_EndPaint(hwnd,hdc);
}

void MV_Close_Keypad( HWND hwnd )
{
	HDC		hdc;

	keypad_enable = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(KEYBOARD_STARTX), ScalerHeigthPixel(KEYBOARD_STARTY), ScalerWidthPixel(KEYBOARD_STARTDX), ScalerHeigthPixel(KEYBOARD_STARTDY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

void Draw_keypad(HDC hdc, U8 *item_str)
{
	U16 	i,j;
	U8		a[5];
	char	temp_str[10];

	for ( i = 0 ; i < ROW_COUNT ; i++ )
	{
		if ( i ==  0 )
		{
			for ( j = 0 ; j < NUM_COUNT ; j++ )
			{
				SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				sprintf(temp_str, "%d", j );
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
			}
		}
		else
		{
			for ( j = 0 ; j < COL_COUNT ; j++ )
			{
				if ( ((i-1)*COL_COUNT) + j > 56 )
					break;

				if ( j == 0 && i == 1 )
					FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				else
					if ( j > 1 && i == 6 )
					{
						FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
					}
					else
						FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);

				if ( ( ((i-1) * COL_COUNT) + j ) > 25	&& ( ((i-1)*COL_COUNT) + j < 52 ) )
				{
					a[0] = 'A' + ( (i-1) * COL_COUNT ) + j + 6;
					a[1] = '\0';
				}
				else if ( ( ((i-1)*COL_COUNT) + j < 26 ))
				{
					a[0] = 'A' + ( (i-1) * COL_COUNT ) + j;
					a[1] = '\0';
				}
				else
				{
					if ( ((i-1)*COL_COUNT) + j == 52 )
					{
						a[0] = 'S';
						a[1] = 'P';
						a[2] = '\0';
					}

					if ( ((i-1)*COL_COUNT) + j == 54 )
					{
						a[0] = 'O';
						a[1] = 'K';
						a[2] = '\0';
					}

					if ( ((i-1)*COL_COUNT) + j == 56 )
					{
						a[0] = 'E';
						a[1] = 'x';
						a[2] = 'i';
						a[3] = 't';
						a[4] = '\0';
					}
				}

				sprintf(temp_str, "%s", a );

				if ( j == 0 && i == 1 )
				{
					SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
				}
				else
				{
					SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					if ( ((i-1)*COL_COUNT) + j == 52 || ((i-1)*COL_COUNT) + j == 54 )
					{
						MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (j*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
						j += 1;
					}
					else
					{
						MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (j*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (i*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
						if ( ((i-1)*COL_COUNT) + j == 56 )
							j += 2;
					}
				}
			}
		}
	}

	if(!( get_windown_status() == CSApp_Applet_TVList || get_windown_status() == CSApp_Applet_RDList ))
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		if ( item_str !='\0')
		{
			MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			CS_MW_DrawText(hdc, item_str, -1, &TmpRect, DT_CENTER);
			//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + 4),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4), item_str);
		}
		else
		{
			if ( ReturnV[0] != '\0' )
			{
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				CS_MW_DrawText(hdc, ReturnV, -1, &TmpRect, DT_CENTER);
				//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + 4),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4), ReturnV);
			}
		}
	}
}

BOOL UI_Keypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
		case CSAPP_KEY_YELLOW:
			if(Returnbuff_Temp[0] == '\0')
				b8keypad_save_str = FALSE;
			else
				b8keypad_save_str = TRUE;

			printf("\n=== %d , %s ====\n", b8keypad_save_str, Returnbuff_Temp);
			MV_Close_Keypad( hwnd );
			return FALSE;
			break;

        case CSAPP_KEY_DOWN:
        	if ( Keypad_Y == 6 )
        	{
        		if ( Keypad_X > 5 )
        			Keypad_X = 9;

        		Selected_Key(hwnd, CHA, UnSelect);

        		Keypad_Y = 0;

        		Selected_Key(hwnd, NO, Select);
        	}
        	else if ( Keypad_Y == 5 )
        	{
        		switch(Keypad_X)
        		{
        			case 2:
        			case 3:
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_Y += 1;
	        			Keypad_X = 2;
	        			Selected_Key(hwnd, CHA, Select);
	        		break;

	        		case 4:
	        		case 5:
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_Y += 1;
	        			Keypad_X = 4;
	        			Selected_Key(hwnd, CHA, Select);
	        		break;

	        		case 6:
	        		case 7:
					case 8:
					case 9:
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_Y += 1;
	        			Keypad_X = 6;
	        			Selected_Key(hwnd, CHA, Select);
	        		break;

	        		default:
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_Y += 1;
	        			Selected_Key(hwnd, CHA, Select);
	        		break;
	        	}
        	}
        	else if ( Keypad_Y == 0 )
        	{
        		Selected_Key(hwnd, NO, UnSelect);
        		Keypad_Y += 1;
        		Selected_Key(hwnd, CHA, Select);
        	}
        	else
        	{
        		Selected_Key(hwnd, CHA, UnSelect);
        		Keypad_Y += 1;
        		Selected_Key(hwnd, CHA, Select);
        	}
			//printf("\n====== X : %d , Y : %d =====\n", Keypad_X, Keypad_Y);
        	break;
        case CSAPP_KEY_UP:
        	if ( Keypad_Y == 0 )
        	{
        		switch(Keypad_X)
        		{
        			case 2:
        			case 3:
        				Selected_Key(hwnd, NO, UnSelect);
        				Keypad_X = 2;
        			break;

        			case 4:
        			case 5:
        				Selected_Key(hwnd, NO, UnSelect);
        				Keypad_X = 4;
        			break;

        			case 6:
        			case 7:
					case 8:
					case 9:
        				Selected_Key(hwnd, NO, UnSelect);
        				Keypad_X = 6;
        			break;

        			default:
        				Selected_Key(hwnd, NO, UnSelect);
        				break;
        		}
        		Keypad_Y = 6;
        		Selected_Key(hwnd, CHA, Select);
        	}
        	else if ( Keypad_Y == 1 )
        	{
        		Selected_Key(hwnd, CHA, UnSelect);
        		Keypad_Y -= 1;
        		Selected_Key(hwnd, NO, Select);
        	}
        	else
        	{
        		Selected_Key(hwnd, CHA, UnSelect);
        		Keypad_Y -= 1;
        		Selected_Key(hwnd, CHA, Select);
        	}
        	break;
        case CSAPP_KEY_LEFT:
        	if ( Keypad_Y == 0 )
        	{
        		if ( Keypad_X == 0 )
        		{
        			Selected_Key(hwnd, NO, UnSelect);
        			Keypad_X = 6;
        			Keypad_Y = 6;
        			Selected_Key(hwnd, CHA, Select);
        		} else {
        			Selected_Key(hwnd, NO, UnSelect);
        			Keypad_X -= 1;
        			Selected_Key(hwnd, NO, Select);
        		}
        	}
        	else
        	{
        		if ( Keypad_Y == 6 && ( Keypad_X == 6 || Keypad_X == 4 ))
        		{
        			if ( Keypad_X == 6 )
        			{
	        			Selected_Key(hwnd, CHA, UnSelect);
	    				Keypad_X -= 2;
	    				Selected_Key(hwnd, CHA, Select);
	    			} else {
	    				Selected_Key(hwnd, CHA, UnSelect);
	    				Keypad_X -= 2;
	    				Selected_Key(hwnd, CHA, Select);
	    			}
        		} else if ( Keypad_X == 0 )
        		{
        			if ( Keypad_Y == 1 )
        			{
        				Selected_Key(hwnd, CHA, UnSelect);
        				Keypad_X = 9;
        				Keypad_Y -= 1;
        				Selected_Key(hwnd, NO, Select);
        			} else {
        				Selected_Key(hwnd, CHA, UnSelect);
        				Keypad_X = 9;
        				Keypad_Y -= 1;
        				Selected_Key(hwnd, CHA, Select);
        			}
        		} else {
        			Selected_Key(hwnd, CHA, UnSelect);
        			Keypad_X -= 1;
        			Selected_Key(hwnd, CHA, Select);
        		}
        	}
        	break;
        case CSAPP_KEY_RIGHT:
        	if ( Keypad_Y == 6 )
        	{
        		if ( Keypad_X == 6 )
        		{
        			Selected_Key(hwnd, CHA, UnSelect);
        			Keypad_X = 0;
        			Keypad_Y = 0;
        			Selected_Key(hwnd, NO, Select);
        		} else if ( Keypad_X == 2 || Keypad_X == 4 ) {
        			Selected_Key(hwnd, CHA, UnSelect);
        			Keypad_X += 2;
        			Selected_Key(hwnd, CHA, Select);
        		} else {
        			Selected_Key(hwnd, CHA, UnSelect);
        			Keypad_X += 1;
        			Selected_Key(hwnd, CHA, Select);
        		}
        	}
        	else
        	{
        		if ( Keypad_Y == 0 )
        		{
        			if ( Keypad_X == 9 )
	        		{
	        			Selected_Key(hwnd, NO, UnSelect);
	        			Keypad_X = 0;
	        			Keypad_Y += 1;
	        			Selected_Key(hwnd, CHA, Select);
	        		} else {
	        			Selected_Key(hwnd, NO, UnSelect);
	        			Keypad_X += 1;
	        			Selected_Key(hwnd, NO, Select);
	        		}
        		} else {
	        		if ( Keypad_X == 9 )
	        		{
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_X = 0;
	        			Keypad_Y += 1;
	        			Selected_Key(hwnd, CHA, Select);
	        		} else {
	        			Selected_Key(hwnd, CHA, UnSelect);
	        			Keypad_X += 1;
	        			Selected_Key(hwnd, CHA, Select);
	        		}
	        	}
        	}
        	break;
        case CSAPP_KEY_ENTER:
			if ( Keypad_Index < u8max_str_length )
			{
/*
				if(get_windown_status() == CSApp_Applet_TP_Setting
#ifdef DONGLE_MODE
					 || FlowCtl_GetCurrentState() == FS_DONGLE_SETTING
#endif
#ifdef CODE_EDIT
					 || FlowCtl_GetCurrentState() == FS_CODE_EDIT
#endif
#ifdef ENABLE_CH_FIND
					 || FlowCtl_GetCurrentState() == FS_CH_LIST_TV_1
					 || FlowCtl_GetCurrentState() == FS_CH_LIST_RADIO_1
#endif
					)
*/
					Returnbuff_Temp[Keypad_Index] = Press_Key();

				if ( b8Keypad_Space_Key == TRUE )
				{
					Returnbuff_Temp[Keypad_Index] = ' ';
					Returnbuff_Temp[Keypad_Index+1] = '\0';
					hdc = MV_BeginPaint(hwnd);
					SetTextColor(hdc,MVAPP_YELLOW_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);
					//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY), Returnbuff_Temp);
					MV_EndPaint(hwnd,hdc);
					Keypad_Index++;

					b8Keypad_Space_Key = FALSE;
				}
				else if ( b8Keypad_Ok_Key == TRUE )
				{
					b8Keypad_Ok_Key = FALSE;
					if( get_windown_status() == CSApp_Applet_TVList || get_windown_status() == CSApp_Applet_RDList)
					{
						if(Returnbuff_Temp[0] == '\0')
							b8keypad_save_str = FALSE;
						else
							b8keypad_save_str = TRUE;
					}
					else
						b8keypad_save_str = TRUE;

					MV_Close_Keypad( hwnd );
					return FALSE;
				}
				else if ( b8Keypad_Cancel_Key == TRUE )
				{
					Returnbuff_Temp[0] = '\0';
					b8Keypad_Cancel_Key = FALSE;
					b8keypad_save_str = FALSE;
					MV_Close_Keypad( hwnd );
					return FALSE;
				}
				else
				{
					Returnbuff_Temp[Keypad_Index+1] = '\0';
	/*
					Show_Bar(KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTX, KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTY,
					16*20, MSG_NULL, EN_MENU_BMP_POPUP_BOX_BAR_BODY, 1);*/
					hdc = MV_BeginPaint(hwnd);
					SetTextColor(hdc,MVAPP_YELLOW_COLOR);
					SetBkMode(hdc,BM_TRANSPARENT);
					MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
					MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
					CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);
					//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY), Returnbuff_Temp);
					MV_EndPaint(hwnd,hdc);
					Keypad_Index++;
				}
			}
			else
			{
				Press_Key();
				if ( b8Keypad_Ok_Key == TRUE )
				{
					b8Keypad_Ok_Key = FALSE;
					b8keypad_save_str = TRUE;
					MV_Close_Keypad( hwnd );
					return FALSE;
				}
				else if ( b8Keypad_Cancel_Key == TRUE )
				{
					Returnbuff_Temp[0] = '\0';
					b8Keypad_Cancel_Key = FALSE;
					b8keypad_save_str = FALSE;
					MV_Close_Keypad( hwnd );
					return FALSE;
				}
			}
        	break;
        case CSAPP_KEY_SWAP:
        	if ( Keypad_Index > 0 && Keypad_Index <= u8max_str_length )
        	{
	    		Returnbuff_Temp[Keypad_Index-1] = '\0';
				strcpy(Returnbuff, Returnbuff_Temp);

				if((Keypad_Index-2)>=0)
		    		Returnbuff[Keypad_Index-2] = '\0';
/*
	        	Show_Bar(KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTX, KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTY,
					16*20, MSG_NULL, EN_MENU_BMP_POPUP_BOX_BAR_BODY, 1);
*/
				//printf("\n%s == Keypad_Index : %d , u8max_str_length: %d  \n", Returnbuff_Temp, Keypad_Index, u8max_str_length);

				hdc = MV_BeginPaint(hwnd);
	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX), ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY ), ScalerWidthPixel(KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);
				//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY), Returnbuff_Temp);
				MV_EndPaint(hwnd,hdc);
	        	Keypad_Index--;
	        }
        	break;
#ifdef ENABLE_CH_FIND
		case KEY_CH_SEARCH:
			if( FlowCtl_GetCurrentState() == FS_CH_LIST_TV_1 || FlowCtl_GetCurrentState() == FS_CH_LIST_RADIO_1)
			{
				if(Returnbuff_Temp[0] == '\0')
					b8keypad_save_str = FALSE;
				else
					b8keypad_save_str = TRUE;

				keypad_enable = FALSE;
				return FALSE;
			}
			break;
#endif
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			Returnbuff_Temp[0] = '\0';
			b8keypad_save_str = FALSE;
			MV_Close_Keypad( hwnd );
			return FALSE;
            break;
    }
	return TRUE;
}

/************************************************************************************************/
/*********************************  HEXA KEYPAD PROCESS **********************************************/
/************************************************************************************************/

void Draw_Hexa_keypad( HDC hdc );

BOOL MV_Get_HexaKeypad_Status(void)
{
	return hexa_keypad_enable;
}

void MV_Set_HexaKeypad_Status(BOOL bStatus)
{
	hexa_keypad_enable = bStatus;
}

char *MV_Get_HexaEdited_String(void)
{
	return Returnbuff_HexaTemp;
}

void MV_Close_HexaKeypad(HWND hwnd)
{
	HDC		hdc;

	MV_Set_HexaKeypad_Status(FALSE);
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX), ScalerHeigthPixel(HEXA_KEYPAD_WINDOWDY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);

	UnloadBitmap(&Capture_bmp); /* By KB Kim 2011.09.20 */
}

void MV_Draw_Edit_String(HDC hdc, RECT *Temp_Rect)
{
	char	acTemp[2];
	char	acDash[2];
	int		i, str_len;
	int		Srt_length = 0;
	int		Str_point = 0;

	acDash[0] = '_';
	acDash[1] = '\0';
	str_len = strlen(Returnbuff_HexaTemp);
	Srt_length = str_len * 14;

	//printf("1 LEFT : %d ======> RIGHT : %d : %s\n", Temp_Rect->left, Temp_Rect->right, Returnbuff_HexaTemp);
	Str_point = ((Temp_Rect->right - Temp_Rect->left) - Srt_length)/2 - 20;
	Temp_Rect->left = Temp_Rect->left + Str_point;
	Temp_Rect->right = Temp_Rect->left + Srt_length;
	//printf("2 LEFT : %d ======> RIGHT : %d\n", Temp_Rect->left, Temp_Rect->right);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - KEYBOARD_KEY_OFFSET*2), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2) );

	for ( i = 0 ; i < str_len ; i++ )
	{
		acTemp[0] = Returnbuff_HexaTemp[i];
		acTemp[1] = '\0';

		Temp_Rect->right = Temp_Rect->left + 14;

		if ( HexaKeypad_Index == i )
		{
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(Temp_Rect->left), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET),ScalerWidthPixel(Temp_Rect->right - Temp_Rect->left),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2));
		} else {
			SetBrushColor(hdc, MVAPP_BLACK_COLOR);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(Temp_Rect->left), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET),ScalerWidthPixel(Temp_Rect->right - Temp_Rect->left),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2));
		}
		CS_MW_DrawText(hdc, acTemp, -1, Temp_Rect, DT_CENTER);

		Temp_Rect->left = Temp_Rect->right;
	}
}

void Draw_Hexa_keypad( HDC hdc )
{
	U32		u32Key_X1 = HEXA_KEYPAD_KEY_BOARDX + HEXA_KEYPAD_OUTLINE_OFFSET + KEYBOARD_KEY_OFFSET + 4;
	U32		u32Key_X2 = u32Key_X1 + KEYBOARD_KEY_OFFSET;
	U32		u32Key_X3 = u32Key_X2 + KEYBOARD_KEY_OFFSET;

	U32		u32Key_Y1 = HEXA_KEYPAD_KEY_BOARDY + 50;
	U32		u32Key_Y2 = u32Key_Y1 + KEYBOARD_KEY_OFFSET;
	U32		u32Key_Y3 = u32Key_Y2 + KEYBOARD_KEY_OFFSET;
	U32		u32Key_Y4 = u32Key_Y3 + KEYBOARD_KEY_OFFSET;
	U32		u32Key_Y5 = u32Key_Y4 + KEYBOARD_KEY_OFFSET;

	char	a[2];

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_1_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_2_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_3_ICON]);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_4_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_5_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_6_ICON]);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_7_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_8_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_9_ICON]);

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	a[0] = 'a';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 20),ScalerHeigthPixel(u32Key_Y4), a);
	a[0] = 'b';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 + KEYBOARD_KEY_OFFSET),ScalerHeigthPixel(u32Key_Y4), a);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_0_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_MUTE_ICON]);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1 - 15), ScalerHeigthPixel(u32Key_Y5), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2 - 15), ScalerHeigthPixel(u32Key_Y5), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3 - 15), ScalerHeigthPixel(u32Key_Y5), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3 + 15), ScalerHeigthPixel(u32Key_Y5), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);

	a[0] = 'c';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 8),ScalerHeigthPixel(u32Key_Y5 + KEYBOARD_KEY_OFFSET), a);
	a[0] = 'd';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 - 8),ScalerHeigthPixel(u32Key_Y5 + KEYBOARD_KEY_OFFSET), a);
	a[0] = 'e';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 - 8),ScalerHeigthPixel(u32Key_Y5 + KEYBOARD_KEY_OFFSET), a);
	a[0] = 'f';
	a[1] = '\0';
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 + 22),ScalerHeigthPixel(u32Key_Y5 + KEYBOARD_KEY_OFFSET), a);
}

void Draw_Hexa_Keypad_Help(HDC hdc)
{
	U32		u32Key_X1 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_OUTLINE_OFFSET;
	U32		u32Key_X2 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX/3;
	U32		u32Key_X3 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX*2/3;
	U32		u32Key_Y1 = HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_OUTLINE_OFFSET;

	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_HELP_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y),ScalerWidthPixel(HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(HEXA_KEYPAD_HELP_DY));
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(HEXA_KEYPAD_HELP_DX),ScalerHeigthPixel(HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_EXIT_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_EXIT_ICON].bmHeight), &MV_BMP[MVBMP_EXIT_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_F2_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_F2_BUTTON].bmHeight), &MV_BMP[MVBMP_F2_BUTTON]);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL));
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));

}

void MV_Draw_HexaKeypad(HWND hwnd, U8 *item_str, U8 max_string_length)
{
	HDC		hdc;
	RECT	Temp_Rect;

	HexaKeypad_X = 0;
	HexaKeypad_Y = 1;
	hexa_keypad_enable = TRUE;
	//u8max_Hexastr_length = max_string_length;
	u8max_Hexastr_length = max_string_length;
	Returnbuff_HexaTemp[0] = '\0';
	b8keypad_save_str = FALSE;

	HexaKeypad_Index = 0;

	memset(Returnbuff_HexaTemp, 0x00, 256);
	strcpy(Returnbuff_HexaTemp, item_str);

	hdc = MV_BeginPaint(hwnd);

	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX), ScalerHeigthPixel(HEXA_KEYPAD_WINDOWDY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY),ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY));
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - KEYBOARD_KEY_OFFSET*2), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2) );

	Temp_Rect.top 	= HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET;
	Temp_Rect.right	= Temp_Rect.left + HEXA_KEYPAD_STR_BOARDDX - HEXA_KEYPAD_OUTLINE_OFFSET*2;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_Draw_Edit_String(hdc, &Temp_Rect);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX + HEXA_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY + HEXA_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX + HEXA_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY + HEXA_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY),ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDDY));
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(HEXA_KEYPAD_KEY_BOARDDX),ScalerHeigthPixel(HEXA_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= HEXA_KEYPAD_KEY_BOARDY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= HEXA_KEYPAD_KEY_BOARDX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + HEXA_KEYPAD_KEY_BOARDDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_KEY_BOARD);

	Draw_Hexa_keypad(hdc);

	Draw_Hexa_Keypad_Help(hdc);

	MV_EndPaint(hwnd,hdc);
}

void MV_HexaKeypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC			hdc;
	RECT		Temp_Rect;
	int			str_len2 = strlen(Returnbuff_HexaTemp) - 1;
	int			str_len = u8max_Hexastr_length - 1;

	Temp_Rect.top 	= HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET;
	Temp_Rect.right	= Temp_Rect.left + HEXA_KEYPAD_STR_BOARDDX - HEXA_KEYPAD_OUTLINE_OFFSET*2;

	switch( u8Key )
	{
		case CSAPP_KEY_LEFT:
			if ( HexaKeypad_Index > 0 )
				HexaKeypad_Index--;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_0:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '0';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_1:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '1';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_2:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '2';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_3:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '3';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_4:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '4';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_5:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '5';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_6:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '6';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_7:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '7';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_8:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '8';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_9:
			Returnbuff_HexaTemp[HexaKeypad_Index] = '9';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_SWAP:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'a';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_MUTE:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'b';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RED:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'c';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_GREEN:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'd';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_YELLOW:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'e';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_BLUE:
			Returnbuff_HexaTemp[HexaKeypad_Index] = 'f';

			if ( HexaKeypad_Index < str_len )
				HexaKeypad_Index++;

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_F2:
			if ( HexaKeypad_Index > 0 )
			{
				memcpy(&Returnbuff_HexaTemp[HexaKeypad_Index], &Returnbuff_HexaTemp[HexaKeypad_Index+1], str_len2 - HexaKeypad_Index);
				Returnbuff_HexaTemp[str_len2] = 0x00;
				HexaKeypad_Index--;
			}
			hdc = MV_BeginPaint(hwnd);
			MV_Draw_Edit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_ENTER:
			MV_Close_HexaKeypad(hwnd);
			break;

		case CSAPP_KEY_EXIT:
			MV_Close_HexaKeypad(hwnd);
			break;

		default:
			break;
	}
}

/************************************************************************************************/
/************************************************************************************************/

void Get_Save_Str(char *Return_Value)
{
	strcpy(Return_Value, Returnbuff_Temp);
}

void Selected_Key(HWND hwnd, U8 u8_NO, U8 Select_Kind)
{
	U8		a[5];
	char	temp_str[50];
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);
	if ( ( ((Keypad_Y-1) * COL_COUNT) + Keypad_X ) > 25  && ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X < 52 ) )
	{
		a[0] = 'A' + ( (Keypad_Y-1) * COL_COUNT ) + Keypad_X + 6;
		a[1] = '\0';
	}
	else if ( ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X < 26 ))
	{
		a[0] = 'A' + ( (Keypad_Y-1) * COL_COUNT ) + Keypad_X;
		a[1] = '\0';
	}
	else
	{
		if ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X == 52 )
		{
			a[0] = 'S';
			a[1] = 'P';
			a[2] = '\0';
		}

		if ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X == 54 )
		{
			a[0] = 'O';
			a[1] = 'K';
			a[2] = '\0';
		}

		if ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X == 56 )
		{
			a[0] = 'E';
			a[1] = 'X';
			a[2] = 'I';
			a[3] = 'T';
			a[4] = '\0';
		}
	}

	if ( Select_Kind == UnSelect )
	{
		SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		if ( u8_NO == CHA )
		{
			sprintf(temp_str, "%s", a );
			//printf("\n===== %d , %d , %s =====\n", KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET), KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET), temp_str );
			if ( ( Keypad_X == 2 && Keypad_Y == 6 ) || ( Keypad_X == 4 && Keypad_Y == 6 ) )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else if ( Keypad_X == 6 && Keypad_Y == 6 ) {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			}

		} else {
			FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
			sprintf(temp_str, "%d", Keypad_X );
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
		}
	} else {
		SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		if ( u8_NO == CHA )
		{
			sprintf(temp_str, "%s", a );
			if ( ( Keypad_X == 2 && Keypad_Y == 6 ) || ( Keypad_X == 4 && Keypad_Y == 6 ) )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else if ( Keypad_X == 6 && Keypad_Y == 6 ) {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_CHAR_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2), temp_str);
			}

		} else {
			FillBoxWithBitmap(hdc,ScalerWidthPixel(KEYBOARD_NUM_KEY_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
			sprintf(temp_str, "%d", Keypad_X );
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_CHAR_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(KEYBOARD_NUM_KEY_STARTY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
		}
	}
	MV_EndPaint(hwnd,hdc);
}

/************************************************************************************************/
/*****************************  NUMERIC KEYPAD PROCESS ******************************************/
/************************************************************************************************/

static BOOL 	b8NumEdit_Flag = FALSE;
static char		NumEdit_Buffer[5];
static U8		NumEdit_MaxLen;
static U8		NumEdit_Value_index;

BOOL MV_Get_NumEdit_Flag(void)
{
	return b8NumEdit_Flag;
}

void MV_NumEdit_Retrun_Value(char *Temp)
{
	strcpy( Temp, NumEdit_Buffer );
}

void MV_NumEdit_Draw_Value(HDC hdc, RECT *lrect)
{
	RECT		acRect;
	int			i;
	char		acTemp_Str[20];

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	acRect.top = lrect->top + 4;
	acRect.left = lrect->left + (( lrect->right - lrect->left )/2 - 50) - 10;
	acRect.bottom = lrect->bottom + 4;
	acRect.right = acRect.left + 25;

	memset( acTemp_Str, 0x00, 20 );

	SetBkMode(hdc,BM_TRANSPARENT);

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(acRect.left + ( 3 * 25)),ScalerHeigthPixel(acRect.top), ".");

	acRect.left -= 25;
	acRect.right = acRect.left + 25;

	for ( i = 0 ; i < 4 ; i++ )
	{
		if ( i == 3 )
			acRect.left += 35;
		else
			acRect.left += 25;

		acRect.right = acRect.left + 25;

		sprintf(acTemp_Str, "%c", NumEdit_Buffer[i]);

		if ( i == NumEdit_Value_index )
		{
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top - 4));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
		else
		{
			SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top - 4));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
	}
}

void MV_Draw_Only_Number_Edit(HWND hwnd, RECT *reRect, S16 acEdit_Number, U8 u8MaxLength, U16 u16String_Index)
{
	RECT	rc1;
	HDC		hdc;

	b8NumEdit_Flag = TRUE;
	NumEdit_MaxLen = u8MaxLength;

	memset(NumEdit_Buffer, 0x00, 5 );
	sprintf(NumEdit_Buffer, "%04d", acEdit_Number);
	NumEdit_Value_index = strlen(NumEdit_Buffer) - 1;

	Capture_WinRect.top = reRect->top;
	Capture_WinRect.bottom = reRect->bottom;
	Capture_WinRect.left = reRect->left;
	Capture_WinRect.right = reRect->right;

	hdc = MV_BeginPaint(hwnd);

	MV_Capture_PopUp_Window(hdc);

	rc1.left = reRect->left;
	rc1.top = reRect->top;
	rc1.right = reRect->right;
	rc1.bottom = reRect->bottom;
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + JUMP_WINDOW_OUTGAP;
	rc1.top = rc1.top + JUMP_WINDOW_OUTGAP;
	rc1.right = rc1.right - JUMP_WINDOW_OUTGAP;
	rc1.bottom = rc1.bottom - JUMP_WINDOW_OUTGAP;
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	rc1.left = rc1.left + JUMP_WINDOW_OUTGAP;
	rc1.top = rc1.top + JUMP_WINDOW_OUTGAP;
	rc1.right = rc1.right - JUMP_WINDOW_OUTGAP;
	rc1.bottom = rc1.top + JUMP_WINDOW_ITEM_DY;
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, u16String_Index);

	rc1.top = rc1.top + JUMP_WINDOW_ITEM_DY + 8;
	rc1.bottom = rc1.top + JUMP_WINDOW_ITEM_DY + 10;
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(rc1.left), ScalerHeigthPixel(rc1.top),ScalerWidthPixel(rc1.right - rc1.left),ScalerHeigthPixel(rc1.bottom - rc1.top));

	jump_rect.left = rc1.left + 10;
	jump_rect.right = rc1.right - 10;
	jump_rect.top = rc1.top + 5;
	jump_rect.bottom = rc1.bottom - 5;

	MV_NumEdit_Draw_Value(hdc, &jump_rect);

	MV_EndPaint(hwnd,hdc);
}

void MV_NumEdit_Update_Value(WPARAM wparam)
{
	char	Temp;

	switch(wparam)
	{
		case CSAPP_KEY_0:
			Temp = '0';
			break;

		case CSAPP_KEY_1:
			Temp = '1';
			break;

		case CSAPP_KEY_2:
			Temp = '2';
			break;

		case CSAPP_KEY_3:
			Temp = '3';
			break;

		case CSAPP_KEY_4:
			Temp = '4';
			break;

		case CSAPP_KEY_5:
			Temp = '5';
			break;

		case CSAPP_KEY_6:
			Temp = '6';
			break;

		case CSAPP_KEY_7:
			Temp = '7';
			break;

		case CSAPP_KEY_8:
			Temp = '8';
			break;

		case CSAPP_KEY_9:
			Temp = '9';
			break;

		default:
			Temp = '0';
			break;
	}

	NumEdit_Buffer[NumEdit_Value_index] = Temp;
}

BOOL MV_NumEdit_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
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
			MV_NumEdit_Update_Value(u8Key);

			if ( NumEdit_Value_index < 3 )
				NumEdit_Value_index++;

			hdc = BeginPaint(hwnd);
			MV_NumEdit_Draw_Value(hdc, &jump_rect);
			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			hdc = BeginPaint(hwnd);

			if ( NumEdit_Value_index < 3 )
				NumEdit_Value_index++;

			MV_NumEdit_Draw_Value(hdc, &jump_rect);
			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_LEFT:
			hdc = BeginPaint(hwnd);

			if ( NumEdit_Value_index > 0 )
				NumEdit_Value_index--;

			MV_NumEdit_Draw_Value(hdc, &jump_rect);
			EndPaint(hwnd,hdc);
			break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			{
				b8NumEdit_Flag = FALSE;
				hdc = BeginPaint(hwnd);
				MV_Restore_PopUp_Window( hdc );
				EndPaint(hwnd,hdc);
			}
			return FALSE;
    }
	return TRUE;
}

/************************************************************************************************/
/*****************************  NUMERIC KEYPAD PROCESS ******************************************/
/************************************************************************************************/

void MV_Draw_NumKeypad(HWND hwnd, U16 u16Num, U16 u16Input_Key, U8 max_string_length)
{
	HDC		hdc;
	RECT	Temp_Rect;

	Keypad_X = u16Input_Key;
	Keypad_Y = 0;
	num_keypad_enable = TRUE;
	u8max_str_length = max_string_length;
	memset(Returnbuff_Temp,0x00,(max_string_length+1));
	Returnbuff_Temp[0] = '\0';

	TmpRect.left	=INPUT_WINDOW_NUM_STARTX;
	TmpRect.right	=INPUT_WINDOW_NUM_STARTX + INPUT_WINDOW_NUM_STARTDX;
	TmpRect.top		=INPUT_WINDOW_NUM_STARTY + 4 ;
	TmpRect.bottom	=INPUT_WINDOW_NUM_STARTY + INPUT_WINDOW_NUM_STARTDY;

	if(u16Num == 0)
	{
		Keypad_Index = 0;
		memset(Returnbuff_Temp,0x00,(max_string_length+1));
	}
	else
	{
		sprintf(Returnbuff_Temp, "%d", u16Num);
		Keypad_Index = strlen(Returnbuff_Temp);
	}

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(NUM_KEYPAD_STARTDX), ScalerHeigthPixel(NUM_KEYPAD_STARTDY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY + NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY + NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(NUM_KEYPAD_STARTX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(NUM_KEYPAD_STARTY),ScalerWidthPixel(NUM_KEYPAD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(NUM_KEYPAD_STARTDY));
	FillBox(hdc,ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(NUM_KEYPAD_STARTDX),ScalerHeigthPixel(NUM_KEYPAD_STARTDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= NUM_KEYPAD_STARTY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= NUM_KEYPAD_STARTX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + NUM_KEYPAD_STARTDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_NUMKEY_BOARD);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX + WINDOW_OUT_GAP), ScalerHeigthPixel(NUM_KEYPAD_STARTY + 50), ScalerWidthPixel(NUM_KEYPAD_STARTDX - WINDOW_OUT_GAP*2), ScalerHeigthPixel(NUM_KEYPAD_STARTDY - 60) );

	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY ), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );

	Draw_Numkeypad(hdc, Returnbuff_Temp);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 2 + 10), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth * 2),	ScalerHeigthPixel( INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 2 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_BACKSPACE));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 3 + 10), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_GREEN_BUTTON].bmWidth * 2),	ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 3 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_ALL_CLEAR));
	FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 4 + 10), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_BLUE_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + MV_BMP[MVBMP_BLUE_BUTTON].bmWidth * 2),	ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY + KEYBOARD_KEY_OFFSET * 4 + 12),	CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));

	MV_EndPaint(hwnd,hdc);
}

void MV_Close_NumKeypad( HWND hwnd )
{
	HDC		hdc;

	num_keypad_enable = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(NUM_KEYPAD_STARTX), ScalerHeigthPixel(NUM_KEYPAD_STARTY), ScalerWidthPixel(NUM_KEYPAD_STARTDX), ScalerHeigthPixel(NUM_KEYPAD_STARTDY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

void Draw_Numkeypad(HDC hdc, U8 *item_str)
{
	U16 	i;
	char	temp_str[10];

	for ( i = 0 ; i < NUM_COUNT ; i++ )
	{
		if ( i == Keypad_X )
		{
			SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
		} else {
			SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
		}
		sprintf(temp_str, "%d", i );
		MV_CS_MW_TextOut( hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (i*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY + 2 ), temp_str);
	}

	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
	CS_MW_DrawText(hdc, item_str, -1, &TmpRect, DT_CENTER);
		//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + 4),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4), item_str);
}

BOOL UI_NumKeypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_LEFT:
        case CSAPP_KEY_UP:
        	if ( Keypad_X == 0 )
    		{
				NumSelected_Key(hwnd, UnSelect);
				Keypad_X = 9;
				NumSelected_Key(hwnd, Select);
    		} else {
    			NumSelected_Key(hwnd, UnSelect);
    			Keypad_X -= 1;
    			NumSelected_Key(hwnd, Select);
    		}
        	break;
        case CSAPP_KEY_RIGHT:
        case CSAPP_KEY_DOWN:
        	if ( Keypad_X == 9 )
    		{
				NumSelected_Key(hwnd, UnSelect);
				Keypad_X = 0;
				NumSelected_Key(hwnd, Select);
    		} else {
    			NumSelected_Key(hwnd, UnSelect);
    			Keypad_X += 1;
    			NumSelected_Key(hwnd, Select);
    		}
        	break;
        case CSAPP_KEY_ENTER:
			if ( Keypad_Index < u8max_str_length )
			{
				Returnbuff_Temp[Keypad_Index] = Press_Key();
				hdc = MV_BeginPaint(hwnd);

	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
				CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);

				MV_EndPaint(hwnd,hdc);
	        	Keypad_Index++;
			}
        	break;
        case CSAPP_KEY_RED:
        	if ( Keypad_Index > 0 && Keypad_Index <= u8max_str_length )
        	{
	    		Returnbuff_Temp[Keypad_Index-1] = '\0';
				strcpy(Returnbuff, Returnbuff_Temp);
				hdc = MV_BeginPaint(hwnd);

	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
				CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);

				MV_EndPaint(hwnd,hdc);
	        	Keypad_Index--;
	        }
        	break;
		case CSAPP_KEY_GREEN:
				memset(&Returnbuff_Temp, 0, STR_BUFF_STRLEN_MAX);
				hdc = MV_BeginPaint(hwnd);

	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
				CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);

				MV_EndPaint(hwnd,hdc);
	        	Keypad_Index=0;
        	break;
		case CSAPP_KEY_BLUE:
			b8keypad_save_str = TRUE;
			MV_Close_NumKeypad( hwnd );
        	break;
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			Returnbuff_Temp[0] = '\0';
			b8keypad_save_str = FALSE;
			MV_Close_NumKeypad( hwnd );
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
				switch (u8Key)
				{
					case CSAPP_KEY_0:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 0;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_1:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 1;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_2:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 2;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_3:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 3;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_4:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 4;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_5:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 5;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_6:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 6;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_7:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 7;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_8:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 8;
						NumSelected_Key(hwnd, Select);
						break;

					case CSAPP_KEY_9:
						NumSelected_Key(hwnd, UnSelect);
						Keypad_X = 9;
						NumSelected_Key(hwnd, Select);
						break;
				}
				//printf("\n %d ==> %d =============\n", Keypad_Index, u8max_str_length );
				if ( Keypad_Index < u8max_str_length )
				{
					Returnbuff_Temp[Keypad_Index] = Press_Key();
		        	Keypad_Index++;
				} else {
					//U8	Length = 0;

					//Length = strlen( Returnbuff_Temp );
					memcpy( &Returnbuff_Temp[0], &Returnbuff_Temp[1], 4 );
/*					if ( Length > 4 )
						Returnbuff_Temp[--Keypad_Index] = Press_Key();
					else*/
					Returnbuff_Temp[Keypad_Index-1] = Press_Key();

				}

				hdc = MV_BeginPaint(hwnd);

	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTY), ScalerWidthPixel(INPUT_WINDOW_NUM_STARTDX), ScalerHeigthPixel(INPUT_WINDOW_NUM_STARTDY) );
				CS_MW_DrawText(hdc, Returnbuff_Temp, -1, &TmpRect, DT_CENTER);

				MV_EndPaint(hwnd,hdc);
			}
			break;
    }
	return TRUE;
}

void NumSelected_Key(HWND hwnd, U8 Select_Kind)
{
	char	temp_str[50];
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);

	SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	sprintf(temp_str, "%d", Keypad_X );

	if ( Select_Kind == UnSelect )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
	} else {
		FillBoxWithBitmap(hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( INPUT_WINDOW_NUM_ITEMY ), ScalerWidthPixel(KEYBOARD_KEY_SIZE),ScalerHeigthPixel(KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
	}

	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(INPUT_WINDOW_NUM_STARTX + (Keypad_X*KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(INPUT_WINDOW_NUM_ITEMY + (Keypad_Y*KEYBOARD_KEY_OFFSET)+2 ), temp_str);
	MV_EndPaint(hwnd,hdc);
}

U8 Press_Key(void)
{
	U8	a=0;

	if ( Keypad_Y == 6 && Keypad_X == 2 )
	{
		b8Keypad_Space_Key = TRUE;
		return '\0';
	}
	if ( Keypad_Y == 6 && Keypad_X == 4 )
	{
		b8Keypad_Ok_Key = TRUE;
		return '\0';
	}
	if ( Keypad_Y == 6 && Keypad_X == 6 )
	{
		b8Keypad_Cancel_Key = TRUE;
		return '\0';
	}
	else
	{
		if ( Keypad_Y == 0 )
		{
#if 0
                        // 07.01.08 junghoon : 숫자키 동작하지 않는것 수정
			sprintf(temp_str, "%d",(U8)Keypad_X);
			a = temp_str[0];
#else
			a = '0' + Keypad_X;
#endif
		}
		else if ( ( ((Keypad_Y-1) * COL_COUNT) + Keypad_X ) > 25  && ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X < 52 ) )
		{
			a = 'A' + ( (Keypad_Y-1) * COL_COUNT ) + Keypad_X + 6;
		}
		else if ( ( ((Keypad_Y-1)*COL_COUNT) + Keypad_X < 26 ))
		{
			a = 'A' + ( (Keypad_Y-1) * COL_COUNT ) + Keypad_X;
		}

		return a;
	}
}

BOOL Get_Keypad_Status(void)
{
	return keypad_enable;
}

BOOL Get_NumKeypad_Status(void)
{
	return num_keypad_enable;
}

BOOL Get_Keypad_is_Save(void)
{
	return b8keypad_save_str;
}

#ifdef GOLDBOX

	#define 	SCROLL_WIN_X		200
	#define		SCROLL_WIN_Y		424
	#define		SCROLL_WIN_DX		880
	#define 	SCROLL_WIN_DY		140

	#define 	SCROLL_WIN_CAP_X	SCROLL_WIN_X
	#define		SCROLL_WIN_CAP_Y	SCROLL_WIN_Y + 20
	#define		SCROLL_WIN_CAP_DX	SCROLL_WIN_DX + 2
	#define		SCROLL_WIN_CAP_DY	120

#else

	#define 	SCROLL_WIN_X		104
	#define		SCROLL_WIN_Y		104
	#define		SCROLL_WIN_DX		300
	#define 	SCROLL_WIN_DY		220

	#define 	SCROLL_WIN_CAP_X	104
	#define		SCROLL_WIN_CAP_Y	124
	#define		SCROLL_WIN_CAP_DX	302
	#define		SCROLL_WIN_CAP_DY	200

#endif

void MV_Scroll_Window(HDC hdc, char *char_str)
{
	BITMAP	temp_bmp;
	char	temp_char[512];
	RECT	TmpRect;

	memset (&temp_bmp, 0, sizeof (BITMAP));
	memset (&temp_char, 0, 512);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(SCROLL_WIN_CAP_X), ScalerHeigthPixel(SCROLL_WIN_CAP_Y), ScalerWidthPixel(SCROLL_WIN_CAP_DX), ScalerHeigthPixel(SCROLL_WIN_CAP_DY), &temp_bmp);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(SCROLL_WIN_X),ScalerHeigthPixel(SCROLL_WIN_Y), ScalerWidthPixel(SCROLL_WIN_DX),ScalerHeigthPixel(SCROLL_WIN_DY) );

	FillBoxWithBitmap(hdc, ScalerWidthPixel(SCROLL_WIN_CAP_X), ScalerHeigthPixel(SCROLL_WIN_Y), ScalerWidthPixel(SCROLL_WIN_CAP_DX), ScalerHeigthPixel(SCROLL_WIN_CAP_DY), &temp_bmp);
	SetTextColor(hdc,MVAPP_SCROLL_GRAY_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	sprintf(temp_char , " %s", char_str);

	TmpRect.left = SCROLL_WIN_X;
	TmpRect.top = SCROLL_WIN_Y + SCROLL_WIN_CAP_DY - 8;
	TmpRect.right = SCROLL_WIN_X + SCROLL_WIN_CAP_DX - 8;
	TmpRect.bottom = SCROLL_WIN_Y + SCROLL_WIN_DY;
	MV_MW_DrawText(hdc, temp_char, -1, &TmpRect, DT_LEFT);

	UnloadBitmap(&temp_bmp);
}

void Draw_Loading_Progress_Bar(HDC hdc, U8 u8ProgressValue, char *char_str)
{
	RECT	Temp_Rect;

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );

	Temp_Rect.top 		= 600;
	Temp_Rect.bottom	= 620;
	Temp_Rect.left		= 170;
	Temp_Rect.right		= 1050;

	MV_Scroll_Window(hdc, char_str);
	MV_Draw_LevelBar(hdc, &Temp_Rect, u8ProgressValue, EN_TTEM_PROGRESS_NO_IMG);
}

void MV_Draw_Msg_Window(HDC hdc, U32 u32Str)
{
	RECT rc1;

	memset(&btMsg_cap, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC (hdc, ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y), ScalerWidthPixel(WARNING_WINDOW_DX), ScalerHeigthPixel(WARNING_WINDOW_DY), &btMsg_cap);

/*
	SetBrushColor(hdc, MVAPP_DARK_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y), ScalerWidthPixel(WARNING_WINDOW_DX), ScalerHeigthPixel(WARNING_WINDOW_DY));

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_X + 3), ScalerHeigthPixel(WARNING_WINDOW_Y + 3), ScalerWidthPixel(WARNING_WINDOW_DX - 6), ScalerHeigthPixel(WARNING_WINDOW_DY - 6));

*/

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_WINDOW_X + WARNING_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y + WARNING_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_WINDOW_X + WARNING_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_WINDOW_Y + WARNING_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WARNING_WINDOW_Y),ScalerWidthPixel(WARNING_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WARNING_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_WINDOW_DX),ScalerHeigthPixel(WARNING_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	rc1.top = WARNING_WINDOW_TITLE_Y;
	rc1.left = WARNING_WINDOW_TITLE_X;
	rc1.bottom = WARNING_WINDOW_TITLE_Y + WARNING_WINDOW_TITLE_DY;
	rc1.right = WARNING_WINDOW_TITLE_X + WARNING_WINDOW_TITLE_DX;
	MV_Draw_PopUp_Title_Bar(hdc, &rc1, eMV_TITLE_ATTENTION);

	if ( u32Str == CSAPP_STR_GET_DHCP || u32Str == CSAPP_STR_PINGING_IP || u32Str == CSAPP_STR_PINGING || u32Str == CSAPP_STR_PING_OK || u32Str == CSAPP_STR_WAIT )
	{
		SetBrushColor(hdc, MVAPP_BLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_CONTENT_X), ScalerHeigthPixel(WARNING_WINDOW_CONTENT_Y), ScalerWidthPixel(WARNING_WINDOW_CONTENT_DX), ScalerHeigthPixel(WARNING_WINDOW_CONTENT_DY));
	} else {
		SetBrushColor(hdc, MVAPP_DARK_RED_COLOR);
		FillBox(hdc,ScalerWidthPixel(WARNING_WINDOW_CONTENT_X), ScalerHeigthPixel(WARNING_WINDOW_CONTENT_Y), ScalerWidthPixel(WARNING_WINDOW_CONTENT_DX), ScalerHeigthPixel(WARNING_WINDOW_CONTENT_DY));
	}


	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	rc1.left = WARNING_WINDOW_CONTENT_X;
	rc1.top = WARNING_WINDOW_CONTENT_Y + 20;
	rc1.right = WARNING_WINDOW_CONTENT_X + WARNING_WINDOW_CONTENT_DX;
	rc1.bottom = WARNING_WINDOW_CONTENT_Y + WARNING_WINDOW_CONTENT_DY - 20;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(u32Str), -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK | DT_VCENTER);
}

void Close_Msg_Window(HDC hdc)
{
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_WINDOW_X), ScalerHeigthPixel(WARNING_WINDOW_Y), ScalerWidthPixel(WARNING_WINDOW_DX), ScalerHeigthPixel(WARNING_WINDOW_DY), &btMsg_cap);
	UnloadBitmap (&btMsg_cap);
}

void MV_Change_Confirm(void)
{
	if ( Confirm_YesNo == TRUE )
		Confirm_YesNo = FALSE;
	else
		Confirm_YesNo = TRUE;
}

void MV_Set_Confirm(BOOL b8Confirm)
{
	Confirm_YesNo = b8Confirm;
}

BOOL MV_Check_Confirm_Window(void)
{
	return Confirm_Window_Flag;
}

BOOL MV_Check_YesNo(void)
{
	return Confirm_YesNo;
}

void MV_Draw_Confirm_Button(HDC hdc)
{
	RECT 	Temp_Rect_Button;
	RECT 	Temp_Rect_Blank;
	RECT	Yes_Button = {	WARNING_ITEM_X + 130,
							WARNING_ITEM_Y + WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							WARNING_ITEM_X + 130 + 120,
							WARNING_ITEM_Y + WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};
	RECT	No_Button = {	WARNING_ITEM_X + 130 + 120 + 60,
							WARNING_ITEM_Y + WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							WARNING_ITEM_X + 130 + 120 + 60 + 120,
							WARNING_ITEM_Y + WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};

	if ( Confirm_YesNo == TRUE )
	{
		Temp_Rect_Button = Yes_Button;
		Temp_Rect_Blank = No_Button;
	} else {
		Temp_Rect_Button = No_Button;
		Temp_Rect_Blank = Yes_Button;
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel((Temp_Rect_Button.right - Temp_Rect_Button.left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.top), ScalerWidthPixel(Temp_Rect_Blank.right - Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.bottom - Temp_Rect_Blank.top) );

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OK), -1, &Yes_Button, DT_CENTER);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL), -1, &No_Button, DT_CENTER);
}

void MV_Draw_Progress_Window(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &btMsg_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WARNING_TOP),ScalerWidthPixel(WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WARNING_DY));
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_DX),ScalerHeigthPixel(WARNING_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= WARNING_TOP + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_PROGRESS);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY) );

	Temp_Rect.top 	= WARNING_ITEM_Y + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;

	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_WAIT), -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);

	MV_EndPaint(hwnd,hdc);
}

void Restore_Progress_Window(HWND hwnd)
{
	HDC 	hdc;

	hdc = MV_BeginPaint(hwnd);

	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &btMsg_cap);
	UnloadBitmap (&btMsg_cap);

	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Progress_status(HWND hwnd, MV_stTPInfo Temp_TPData, U8 u8Max_Count, U8 Now_Count)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	U8		u8ProgressValue = ((u8Max_Count - Now_Count)*100)/u8Max_Count;
	U8		Temp = 0;

	Temp = Temp_TPData.u8Polar_H;
	Temp_Rect.top 	= WARNING_ITEM_Y + ( MV_INSTALL_MENU_BAR_H * 2 ) + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX - ( MV_INSTALL_MENU_BAR_H * 2 );

	hdc = MV_BeginPaint(hwnd);
	MV_Draw_LevelBar(hdc, &Temp_Rect, u8ProgressValue, EN_TTEM_PROGRESS_NO_IMG);
	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Confirm_Window(HWND hwnd, U16 u16Message_index)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	Confirm_Window_Flag = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &btMsg_Con_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT + WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(WARNING_TOP + WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(WARNING_TOP),ScalerWidthPixel(WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(WARNING_DY));
	FillBox(hdc,ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(WARNING_DX),ScalerHeigthPixel(WARNING_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= WARNING_TOP + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_WARNING);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(WARNING_ITEM_X), ScalerHeigthPixel(WARNING_ITEM_Y), ScalerWidthPixel(WARNING_ITEM_DX), ScalerHeigthPixel(WARNING_ITEM_DY) );

	Temp_Rect.top 	= WARNING_ITEM_Y + WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + WARNING_ITEM_DX;

	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(u16Message_index), -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);
/*
	Temp_Rect.top 		= Temp_Rect.top + MV_INSTALL_MENU_BAR_H *2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left		= WARNING_ITEM_X + 60;
	Temp_Rect.right		= Temp_Rect.left + 100;
	FillBoxWithBitmap(hdc,ScalerWidthPixel(WARNING_ITEM_X + 30), ScalerHeigthPixel(Temp_Rect.top), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OK), -1, &Temp_Rect, DT_CENTER);

	Temp_Rect.left		= Temp_Rect.right + 30;
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect.left), ScalerHeigthPixel(Temp_Rect.top), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	Temp_Rect.left 		= Temp_Rect.left + 30;
	Temp_Rect.right		= Temp_Rect.left + 100;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL), -1, &Temp_Rect, DT_CENTER);
*/

	MV_Draw_Confirm_Button(hdc);
	MV_EndPaint(hwnd,hdc);
}

void Restore_Confirm_Window(HDC hdc)
{
	Confirm_Window_Flag = FALSE;
	Confirm_YesNo = FALSE;
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(WARNING_LEFT), ScalerHeigthPixel(WARNING_TOP), ScalerWidthPixel(WARNING_DX), ScalerHeigthPixel(WARNING_DY), &btMsg_Con_cap);
	UnloadBitmap (&btMsg_Con_cap);
}

BOOL MV_Confirm_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
		case CSAPP_KEY_RIGHT:
        case CSAPP_KEY_UP:
		case CSAPP_KEY_LEFT:
        	hdc = BeginPaint(hwnd);
			MV_Change_Confirm();
			MV_Draw_Confirm_Button(hdc);
			EndPaint(hwnd,hdc);
        	break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			Confirm_YesNo = FALSE;
			hdc = BeginPaint(hwnd);
			Restore_Confirm_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
/************************************************************************************************/
/************************* ********  NET UPGRADE PROCESS ********************** *****************/
/************************************************************************************************/
#ifdef PLUGIN_MENU
void MV_SetNetUpgradeMode(eMV_NET_Upgrade_Items mode)
{
	CurrentNetUpgradeMode = mode;
}

eMV_NET_Upgrade_Items MV_GetNetUpgradeMode(void)
{
	return CurrentNetUpgradeMode;
}

BOOL MV_NetCheck_Confirm_Window(void)
{
	return NetConfirm_Window_Flag;
}

BOOL MV_NetCheck_YesNo(void)
{
	return NetConfirm_YesNo;
}

void MV_NetDown_Change_Confirm(void)
{
	if ( NetConfirm_YesNo == TRUE )
		NetConfirm_YesNo = FALSE;
	else
		NetConfirm_YesNo = TRUE;
}

void Restore_NetDown_Confirm_Window(HDC hdc)
{
	NetConfirm_Window_Flag = FALSE;
	NetConfirm_YesNo = FALSE;
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(UPGRADE_WARNING_DX), ScalerHeigthPixel(UPGRADE_WARNING_DY), &btMsg_Con_cap);
	UnloadBitmap (&btMsg_Con_cap);
}

void MV_Draw_NetDown_Confirm_Button(HDC hdc)
{
	RECT 	Temp_Rect_Button;
	RECT 	Temp_Rect_Blank;
	RECT	Yes_Button = {	UPGRADE_WARNING_ITEM_X + 30,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							UPGRADE_WARNING_ITEM_X + 30 + 200,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};
	RECT	No_Button = {	UPGRADE_WARNING_ITEM_X + 30 + 200 + 20,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2,
							UPGRADE_WARNING_ITEM_X + 30 + 200 + 20 + 200,
							UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2 + MV_INSTALL_MENU_BAR_H * 2 + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight};

	if ( NetConfirm_YesNo == TRUE )
	{
		Temp_Rect_Button = Yes_Button;
		Temp_Rect_Blank = No_Button;
	} else {
		Temp_Rect_Button = No_Button;
		Temp_Rect_Blank = Yes_Button;
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_LEFT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.left + MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel((Temp_Rect_Button.right - Temp_Rect_Button.left) - ( MV_BMP[MVBMP_MENU_TITLE_LEFT].bmWidth + MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth )), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_MID].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_MID]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(Temp_Rect_Button.right - MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(Temp_Rect_Button.top), ScalerWidthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MENU_TITLE_RIGHT].bmHeight), &MV_BMP[MVBMP_MENU_TITLE_RIGHT]);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.top), ScalerWidthPixel(Temp_Rect_Blank.right - Temp_Rect_Blank.left), ScalerHeigthPixel(Temp_Rect_Blank.bottom - Temp_Rect_Blank.top) );

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	Yes_Button.top += 4;
	No_Button.top += 4;
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OK), -1, &Yes_Button, DT_CENTER);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_EDIT), -1, &No_Button, DT_CENTER);
}

void MV_Draw_NetDown_Confirm(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	char	Temp_Str[64];

	memset(Temp_Str, 0x00, 64);

	NetConfirm_Window_Flag = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(UPGRADE_WARNING_DX), ScalerHeigthPixel(UPGRADE_WARNING_DY), &btMsg_Con_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT + UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP + UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(UPGRADE_WARNING_LEFT + UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP + UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(UPGRADE_WARNING_LEFT + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(UPGRADE_WARNING_TOP),ScalerWidthPixel(UPGRADE_WARNING_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(UPGRADE_WARNING_DY));
	FillBox(hdc,ScalerWidthPixel(UPGRADE_WARNING_LEFT), ScalerHeigthPixel(UPGRADE_WARNING_TOP + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(UPGRADE_WARNING_DX),ScalerHeigthPixel(UPGRADE_WARNING_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= UPGRADE_WARNING_TOP + UPGRADE_WARNING_OUT_GAP + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= UPGRADE_WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + UPGRADE_WARNING_ITEM_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_ATTENTION);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_WARNING_ITEM_X), ScalerHeigthPixel(UPGRADE_WARNING_ITEM_Y), ScalerWidthPixel(UPGRADE_WARNING_ITEM_DX), ScalerHeigthPixel(UPGRADE_WARNING_ITEM_DY) );

	Temp_Rect.top 	= UPGRADE_WARNING_ITEM_Y + UPGRADE_WARNING_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= UPGRADE_WARNING_ITEM_X;
	Temp_Rect.right	= Temp_Rect.left + UPGRADE_WARNING_ITEM_DX;

	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		sprintf(Temp_Str, "%s", CS_DBU_Get_Webaddr());
	}
	else if (MV_GetNetUpgradeMode() == NET_UPGRADE_PLUGIN)
	{
		sprintf(Temp_Str, "%s", CS_DBU_Get_PlugInAddr());
	}
	CS_MW_DrawText (hdc, Temp_Str, -1, &Temp_Rect, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);

	MV_Draw_NetDown_Confirm_Button(hdc);
	MV_EndPaint(hwnd,hdc);
}

BOOL MV_NetDown_Confirm_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
		case CSAPP_KEY_RIGHT:
        case CSAPP_KEY_UP:
		case CSAPP_KEY_LEFT:
        	hdc = BeginPaint(hwnd);
			MV_NetDown_Change_Confirm();
			MV_Draw_NetDown_Confirm_Button(hdc);
			EndPaint(hwnd,hdc);
        	break;

        case CSAPP_KEY_ENTER:
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			NetConfirm_YesNo = FALSE;
			hdc = BeginPaint(hwnd);
			Restore_NetDown_Confirm_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

BOOL MV_CheckWgetListStatus(void)
{
	return Wget_File_List;
}

void MV_Close_Wget_Window(HDC hdc)
{
	Wget_File_List = FALSE;
	FillBoxWithBitmap(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);
	UnloadBitmap (&btFile_cap);
}

MV_CFG_RETURN MV_Upgrade_Get_Wget_File(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;
	U16				i = 0, j = 0, k = 0, l = 0;

	/* By KB Kim for Plugin Setting : 2011.05.07 */
	u16Upgrade_Data_Count = 0;
	memset( stUpgrade_Data, 0x00, sizeof(stMV_Upgrade_Info) * 256 );

	if (!(fp = fopen(UPGRADE_CFG_FILE, "r")))
	{
         ret = CFG_NOFILE;
		 return ret;
	}

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	while ((!feof(fp)) && (u16Upgrade_Data_Count < MAX_UPGRADE_LIST_COUNT)){
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			return ret;
        }

		//printf("%s ===>\n", tempSection);

		for ( i = 0 ; i < strlen(tempSection) ; i++ )
		{
			if ( tempSection[i] == ';' || tempSection[i] == '\n')
			{
				switch(l)
				{
					case 0:
						stUpgrade_Data[u16Upgrade_Data_Count].u8Set_Version = atoi(Temp);
						break;

					case 1:
						stUpgrade_Data[u16Upgrade_Data_Count].u8SW_Version = atoi(Temp);
						break;

					case 2:
						sprintf(stUpgrade_Data[u16Upgrade_Data_Count].acFile_Location, "%s", Temp);

						for ( j = 0 ; j < strlen(Temp) ; j++ )
							if ( Temp[j] == '/' )
								strncpy(stUpgrade_Data[u16Upgrade_Data_Count].acFile_Name, &Temp[j+1], strlen(Temp) - ( j + 1 ) );
						break;

					case 3:
						sprintf(stUpgrade_Data[u16Upgrade_Data_Count].acFile_size, "%s", Temp);
						break;

					case 4:
						sprintf(stUpgrade_Data[u16Upgrade_Data_Count].acFile_date, "%s", Temp);
						break;

					default:
						break;
				}

				memset (Temp, 0, sizeof(char) * 100);
				k = 0;
				l++;
			}
			else
			{
				Temp[k] = tempSection[i];
				k++;
			}
		}
		l = 0;
		u16Upgrade_Data_Count++;
    }

	fclose (fp);
	return ret;
}

MV_CFG_RETURN MV_Plugin_Get_Wget_File(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;
	U16				i = 0, k = 0, l = 0;

	u16Plugin_Data_Count = 0;
	memset( stPlugin_Data, 0x00, sizeof(stMV_Plugin_Info) * 256 );

	if (!(fp = fopen(PLUGIN_INFO_FILE, "r")))
	{
         ret = CFG_NOFILE;
		 return ret;
	}

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	while ((!feof(fp)) && (u16Plugin_Data_Count < MAX_UPGRADE_LIST_COUNT)) {
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			return ret;
        }

		//printf("%s ===>\n", tempSection);

		for ( i = 0 ; i < strlen(tempSection) ; i++ )
		{
			if ( tempSection[i] == ';' || tempSection[i] == '\n')
			{
				switch(l)
				{
					case 0:
						sprintf(stPlugin_Data[u16Plugin_Data_Count].pFile_Detail, "%s", Temp);
						break;

					case 1:
						sprintf(stPlugin_Data[u16Plugin_Data_Count].pFile_Name, "%s", Temp);
						break;

					case 2:
						sprintf(stPlugin_Data[u16Plugin_Data_Count].pFile_size, "%s", Temp);
						break;

					default:
						break;
				}

				memset (Temp, 0, sizeof(char) * 100);
				k = 0;
				l++;
			}
			else
			{
				Temp[k] = tempSection[i];
				k++;
			}
		}

		// printf ("Plugin : %s %s %s\n", stPlugin_Data[u16Plugin_Data_Count].pFile_Detail, stPlugin_Data[u16Plugin_Data_Count].pFile_Name, stPlugin_Data[u16Plugin_Data_Count].pFile_size);
		l = 0;
		u16Plugin_Data_Count++;
    }

	fclose (fp);
	return ret;
}

void MV_Draw_WgetListBar(HDC hdc, int esItem, U8 u8Kind)
{
	int 			y_gap = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_HEIGHT * esItem;
	RECT			TmpRect;
	U16             dataMaxCount = 0;

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if( u8Kind == FOCUS ) {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(y_gap), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	} else {
		if ( esItem % 2 == 0 )
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		} else {
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		}
	}

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	// TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_NAME_X + MV_BMP[MVBMP_TS_FILE].bmWidth + 10);
	TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_NAME_X + 10);
	TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_NAME_DX - ( MV_BMP[MVBMP_TS_FILE].bmWidth + 10 );
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		CS_MW_DrawText(hdc, stUpgrade_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].acFile_Name, -1, &TmpRect, DT_LEFT);

		TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_DATE_X);
		TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_DATE_DX;
		CS_MW_DrawText(hdc, stUpgrade_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].acFile_date, -1, &TmpRect, DT_LEFT);
		dataMaxCount = u16Upgrade_Data_Count;
	}
	else if (MV_GetNetUpgradeMode() == NET_UPGRADE_PLUGIN)
	{
		CS_MW_DrawText(hdc, stPlugin_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].pFile_Name, -1, &TmpRect, DT_LEFT);

		TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_SIZE_X);
		TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_SIZE_DX + FILE_WINDOW_ITEM_DATE_DX;
		CS_MW_DrawText(hdc, stPlugin_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)].pFile_Detail, -1, &TmpRect, DT_LEFT);
		dataMaxCount = u16Plugin_Data_Count;
	}
	else if (MV_GetNetUpgradeMode() == NET_PLUGIN_SITE)
	{
		TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_DX - 20;
		CS_MW_DrawText(hdc, PluginSite_Data[esItem + ( u16Current_WgetPage * FILE_LIST_MAX_ITEM)], -1, &TmpRect, DT_CENTER);
	}

	if ( u8Kind == FOCUS )
	{
		TmpRect.top = FILE_WINDOW_ITEM_Y;
		TmpRect.left = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX;
		TmpRect.right = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX;
		TmpRect.bottom = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_DY;
		MV_Draw_ScrollBar(hdc, TmpRect, u16Current_Wgetindex, dataMaxCount, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Wget_List(HWND hwnd)
{
	U16 	i = 0;
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);

	for( i = 0 ; i < FILE_LIST_MAX_ITEM ; i++ )
	{
		if ( i == u16Wget_Focus )
			MV_Draw_WgetListBar(hdc, i, FOCUS);
		else
			MV_Draw_WgetListBar(hdc, i, UNFOCUS);
	}

	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_Wget_FileList(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	u16Wget_Focus = 0;
	u16Current_WgetPage = 0;
	u16Prev_WgetPage = 0;
	u16Current_Wgetindex = 0;

	Wget_File_List = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y),ScalerWidthPixel(FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(FILE_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(FILE_WINDOW_DX),ScalerHeigthPixel(FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= FILE_WINDOW_TITLE_Y;
	Temp_Rect.bottom	= FILE_WINDOW_TITLE_Y + FILE_WINDOW_ITEM_HEIGHT;
	Temp_Rect.left	= FILE_WINDOW_TITLE_X;
	Temp_Rect.right	= Temp_Rect.left + FILE_WINDOW_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_FILE_EXPLORE);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(FILE_WINDOW_ITEM_Y), ScalerWidthPixel(FILE_WINDOW_ITEM_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_DY) );

	MV_EndPaint(hwnd,hdc);

	MV_Draw_Wget_List(hwnd);
}

BOOL MV_WgetList_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;
	U16     dataMaxCount = 0;

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		dataMaxCount = u16Upgrade_Data_Count;
	}
	else if (MV_GetNetUpgradeMode() == NET_UPGRADE_PLUGIN)
	{
		dataMaxCount = u16Plugin_Data_Count;
	}
	else if (MV_GetNetUpgradeMode() == NET_PLUGIN_SITE)
	{
		dataMaxCount = u16PluginSite_Count;
	}

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
			if (dataMaxCount < 2)
			{
				break;
			}
			hdc = BeginPaint(hwnd);
			u16Prev_WgetPage = u16Current_WgetPage;
			MV_Draw_WgetListBar(hdc, u16Wget_Focus, UNFOCUS);

			if ( u16Current_Wgetindex >= dataMaxCount - 1 )
				u16Current_Wgetindex = 0;
			else
				u16Current_Wgetindex++;

			u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_WgetPage != u16Current_WgetPage )
				MV_Draw_Wget_List(hdc);
			else
				MV_Draw_WgetListBar(hdc, u16Wget_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

        case CSAPP_KEY_UP:
			if (dataMaxCount < 2)
			{
				break;
			}
			hdc = BeginPaint(hwnd);
			u16Prev_WgetPage = u16Current_WgetPage;
			MV_Draw_WgetListBar(hdc, u16Wget_Focus, UNFOCUS);

			if ( u16Current_Wgetindex <= 0 )
				u16Current_Wgetindex = dataMaxCount - 1;
			else
				u16Current_Wgetindex--;

			u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_WgetPage != u16Current_WgetPage )
				MV_Draw_Wget_List(hdc);
			else
				MV_Draw_WgetListBar(hdc, u16Wget_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			{
				U8	current_page, total_page;

				if(dataMaxCount == 0)
					break;

				current_page = getpage((u16Current_Wgetindex+1), FILE_LIST_MAX_ITEM);
				total_page = getpage(dataMaxCount, FILE_LIST_MAX_ITEM);

				u16Current_Wgetindex += FILE_LIST_MAX_ITEM;
				if(u16Current_Wgetindex > dataMaxCount-1)
				{
					if(current_page < total_page)
						u16Current_Wgetindex = dataMaxCount-1;
					else
						u16Current_Wgetindex = 0;
				}

				u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_Wget_List(hdc);
				EndPaint(hwnd,hdc);
			}
			break;

		case CSAPP_KEY_LEFT:
			{
				if(dataMaxCount == 0)
					break;

				if(u16Current_Wgetindex < FILE_LIST_MAX_ITEM)
				{
					u16Current_Wgetindex = (getpage(dataMaxCount, FILE_LIST_MAX_ITEM)+1)*FILE_LIST_MAX_ITEM + u16Current_Wgetindex-FILE_LIST_MAX_ITEM;
				}
				else
					u16Current_Wgetindex -= FILE_LIST_MAX_ITEM;

				if(u16Current_Wgetindex > dataMaxCount-1)
					u16Current_Wgetindex = dataMaxCount-1;

				u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_Wget_List(hdc);
				EndPaint(hwnd,hdc);
			}
        	break;

        case CSAPP_KEY_ENTER:
			break;

        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			hdc = BeginPaint(hwnd);
			MV_Close_Wget_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

/* For Plugin Site List by File : KB Kim 2011.09.13 */
char *MV_GetPluginSite(void)
{
	memset(CurrentSite, 0x00, MAX_UPGRADE_NAME_COUNT);

	if (u16Current_Siteindex < u16PluginSite_Count)
	{
		strcpy(CurrentSite, PluginSite_Data[u16Current_Siteindex]);
	}
	else
	{
		strcpy(CurrentSite, CS_DBU_Get_PlugInAddr());
	}

	// printf("MV_GetPluginSite : %s\n", CurrentSite);

	return CurrentSite;
}

BOOL MV_CheckSiteListStatus(void)
{
	return PlugInSite_List;
}

U16 MV_GetPluginSiteCount(void)
{
	return u16PluginSite_Count;
}

void MV_Close_SiteList_Window(HDC hdc)
{
	u16Current_Siteindex = u16Current_Wgetindex;
	PlugInSite_List = FALSE;
	FillBoxWithBitmap(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(SITE_WINDOW_DY), &btFile_cap);
	UnloadBitmap (&btFile_cap);
}

MV_CFG_RETURN MV_Plugin_Save_SiteList(void)
{
	FILE*		      fp;
	U16               count;
	MV_CFG_RETURN	ret = CFG_OK;

	if (u16PluginSite_Count > 0)
	{
		fp = fopen(PLUGIN_SITE_FILE, "wt");
		if (fp == NULL)
		{
			ret = CFG_NOFILE;
			return ret;
		}

		for (count = 0; count < u16PluginSite_Count; count++)
		{
			fprintf(fp, "%s\n", PluginSite_Data[count]);
		}

		fclose (fp);
	}
	else
	{
		ret = CFG_NO_NAME;
	}

	return ret;
}

MV_CFG_RETURN MV_Plugin_Set_DefaultSiteList(void)
{
	sprintf(PluginSite_Data[0], "%s", CS_DBU_Get_PlugInAddr());
	u16PluginSite_Count = 1;

	return MV_Plugin_Save_SiteList();
}

void MV_Delete_PluginSite(HWND hwnd)
{
	U16 count;

	if (u16PluginSite_Count > 1)
	{
		if (u16Current_Wgetindex < (u16PluginSite_Count - 1))
		{
			for (count = u16Current_Wgetindex; count < (u16PluginSite_Count - 1); count++)
			{
				memcpy(PluginSite_Data[count], PluginSite_Data[count + 1], MAX_UPGRADE_NAME_COUNT);
			}
		}
		else
		{
			u16Current_Wgetindex--;
		}

		memset(PluginSite_Data[u16PluginSite_Count - 1], 0x00, MAX_UPGRADE_NAME_COUNT);

		u16PluginSite_Count--;
		MV_Plugin_Save_SiteList();

		u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);
		MV_Draw_Wget_List(hwnd);
	}
}

void MV_Add_PluginSite(char *siteData)
{
	U32             lineLength;

	if (u16PluginSite_Count < MAX_UPGRADE_LIST_COUNT)
	{
		lineLength = strlen(siteData);
		if (lineLength >= MAX_UPGRADE_NAME_COUNT)
		{
			lineLength = MAX_UPGRADE_NAME_COUNT - 1;
		}

		memcpy(PluginSite_Data[u16PluginSite_Count], siteData, lineLength);
		u16Current_Siteindex = u16PluginSite_Count;

		u16PluginSite_Count++;
		MV_Plugin_Save_SiteList();

	}
}

void MV_Edit_PluginSite(char *siteData)
{
	U32             lineLength;

	lineLength = strlen(siteData);
	if (lineLength >= MAX_UPGRADE_NAME_COUNT)
	{
		lineLength = MAX_UPGRADE_NAME_COUNT - 1;
	}

	memset(PluginSite_Data[u16Current_Wgetindex], 0x00, MAX_UPGRADE_NAME_COUNT);
	memcpy(PluginSite_Data[u16Current_Wgetindex], siteData, lineLength);

	MV_Plugin_Save_SiteList();
}

MV_CFG_RETURN MV_Plugin_Get_SiteList(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL];
	U32             lineLength;
	U32             pointer;
	MV_CFG_RETURN	ret = CFG_OK;

	u16PluginSite_Count = 0;
	memset(PluginSite_Data, 0x00, MAX_UPGRADE_LIST_COUNT * MAX_UPGRADE_NAME_COUNT );

	fp = fopen(PLUGIN_SITE_FILE, "r");
	if (fp == NULL)
	{
		/* File Open Error */
		// printf("MV_Plugin_Get_SiteList : File Open Error\n");
		MV_Plugin_Set_DefaultSiteList();

		ret = CFG_NOFILE;
		return ret;
	}

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	while ((!feof(fp)) && (u16PluginSite_Count < MAX_UPGRADE_LIST_COUNT))
	{
        if (!fgets(tempSection, CFG_MAX_COL, fp))
        {
			// printf("MV_Plugin_Get_SiteList : File Get Error\n");
			fclose (fp);
			if (u16PluginSite_Count == 0)
			{
				MV_Plugin_Set_DefaultSiteList();
			}
			ret = CFG_READ_FAIL;
			return ret;
        }

		lineLength = strlen(tempSection);
		pointer = 0;
		while(pointer < lineLength)
		{
			if ((tempSection[pointer] == 0x0d) || (tempSection[pointer] == 0x0a))
			{
				lineLength = pointer;
				tempSection[pointer] = 0x00;
			}

			pointer++;
		}

		if (lineLength >= MAX_UPGRADE_NAME_COUNT)
		{
			lineLength = MAX_UPGRADE_NAME_COUNT - 1;
		}

		memcpy(PluginSite_Data[u16PluginSite_Count], tempSection, lineLength);
		// printf("MV_Plugin_Get_SiteList : Get Site List[%d] %s / %s\n", lineLength, tempSection, PluginSite_Data[u16PluginSite_Count]);

		u16PluginSite_Count++;
	}

	fclose (fp);

	if (u16PluginSite_Count == 0)
	{
		MV_Plugin_Set_DefaultSiteList();
	}
	return ret;
}

void MV_Draw_PlugIn_Site_List(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	MV_Plugin_Get_SiteList();

	u16Prev_WgetPage = 0;
	if (u16Current_Siteindex < u16PluginSite_Count)
	{
		u16Current_Wgetindex = u16Current_Siteindex;
	}
	else
	{
		u16Current_Wgetindex = 0;
	}
	u16Wget_Focus = get_focus_line(&u16Current_WgetPage, u16Current_Wgetindex, FILE_LIST_MAX_ITEM);

	PlugInSite_List = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(SITE_WINDOW_DY), &btFile_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + SITE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y + SITE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y),ScalerWidthPixel(FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(SITE_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(FILE_WINDOW_DX),ScalerHeigthPixel(SITE_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= FILE_WINDOW_TITLE_Y;
	Temp_Rect.bottom	= FILE_WINDOW_TITLE_Y + FILE_WINDOW_ITEM_HEIGHT;
	Temp_Rect.left	= FILE_WINDOW_TITLE_X;
	Temp_Rect.right	= Temp_Rect.left + FILE_WINDOW_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_SITE_LIST);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(FILE_WINDOW_ITEM_Y), ScalerWidthPixel(FILE_WINDOW_ITEM_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_DY) );

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SITE_HELP_X), ScalerHeigthPixel(SITE_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SITE_HELP_X)+ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth)+6, ScalerHeigthPixel(SITE_HELP_Y), CS_MW_LoadStringByIdx(CSAPP_STR_SELECT));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SITE_HELP_X1), ScalerHeigthPixel(SITE_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SITE_HELP_X1)+ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth)+6, ScalerHeigthPixel(SITE_HELP_Y), CS_MW_LoadStringByIdx(CSAPP_STR_EDIT));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SITE_HELP_X2), ScalerHeigthPixel(SITE_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_GREEN_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SITE_HELP_X2)+ScalerWidthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmWidth)+6, ScalerHeigthPixel(SITE_HELP_Y), CS_MW_LoadStringByIdx(CSAPP_STR_ADD));
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SITE_HELP_X3), ScalerHeigthPixel(SITE_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_GREEN_BUTTON].bmHeight), &MV_BMP[MVBMP_YELLOW_BUTTON]);
	CS_MW_TextOut(hdc, ScalerWidthPixel(SITE_HELP_X3)+ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BUTTON].bmWidth)+6, ScalerHeigthPixel(SITE_HELP_Y), CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));

	MV_EndPaint(hwnd,hdc);

	MV_Draw_Wget_List(hwnd);
}
/**********************************************************************/

int Show_Progress(HWND hwnd)
{
	HDC 			hdc;
	struct stat		statbuffer;
	U8				file_Percent;
	RECT			TmpRect;
	char 			TmpStr[20];

	memset(TmpStr, 0x00, 20 );

	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		if( stat(TMP_UPGRADE_FILE, &statbuffer ) != 0 )
		{
			// printf("STAT ERROR========================\n");
			statbuffer.st_size = 0;
		}
	}
	else if (MV_GetNetUpgradeMode() == NET_UPGRADE_PLUGIN)
	{
		if( stat(TMP_PLUGIN_FILE, &statbuffer ) != 0 )
		{
			// printf("STAT ERROR========================\n");
			statbuffer.st_size = 0;
		}
	}

	file_Percent = (U32)( statbuffer.st_size / ( DownloadFileSize / 100 ));

	// printf("==== %d ,  %ld / %ld\n", file_Percent, statbuffer.st_size, DownloadFileSize);

	TmpRect.left = UPGRADE_MESSAGE_X ;
	TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
	TmpRect.top = UPGRADE_MESSAGE_Y + 40;
	TmpRect.bottom = TmpRect.top + 30;

	sprintf(TmpStr, "%lld / %lld", statbuffer.st_size, DownloadFileSize);

	hdc=BeginPaint(hwnd);

	if ( file_Percent <= 100 )
	{
		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y + 40), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(30) );
		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		CS_MW_DrawText (hdc, TmpStr, -1, &TmpRect, DT_CENTER);

		TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX - 70;
		TmpRect.top = UPGRADE_MESSAGE_Y + 80;
		TmpRect.bottom = TmpRect.top + 14;
		MV_Draw_LevelBar(hdc, &TmpRect, file_Percent, EN_TTEM_PROGRESS_NO_IMG_MP3);
	}

	EndPaint(hwnd,hdc);

	return 0;
}

void *Progress_Task( void *param )
{
	HWND	hwnd;

	hwnd = GetActiveWindow();

	while(1)
	{
		if ( Show_Progress(hwnd) != 0 )
		{
			printf("\n=========== ERROR ==================\n");
			break;
		}
		usleep( 400*1000 );
	}

	return ( param );
}

#if 0
void *Upgrade_Task( void *param )
{
	HWND	hwnd;
	HDC		hdc;
	int		i = 0, j = 0;
	RECT	TmpRect;
	U8		old_status = 0;

	hwnd = GetActiveWindow();

	while(1)
	{
		hdc=BeginPaint(hwnd);

		MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
		MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y2 - 30), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY2 + 30) );

		for ( j = 0 ;  j < UPGRADE_STATUS ; j ++ )
		{
			TmpRect.left = UPGRADE_MESSAGE_X;
			TmpRect.top = UPGRADE_MESSAGE_Y2 + (MV_INSTALL_MENU_BAR_H * j) + 10;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			TmpRect.bottom = TmpRect.top + MV_INSTALL_MENU_BAR_H;

			if ( j == Sys_Com_Status )
			{
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(System_Command_State[j]), -1, &TmpRect, DT_CENTER);
			} else {
				SetBkMode(hdc,BM_TRANSPARENT);
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(System_Command_UnState[j]), -1, &TmpRect, DT_CENTER);
			}
		}

		if ( i % 10 < 5 )
		{
			SetBkMode(hdc,BM_TRANSPARENT);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);

			TmpRect.left = UPGRADE_MESSAGE_X ;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			TmpRect.top = UPGRADE_MESSAGE_Y2 - 30;
			TmpRect.bottom = TmpRect.top + 30;

			CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_IF_UPGRADE_FAIL), -1, &TmpRect, DT_CENTER);

			TmpRect.left = UPGRADE_MESSAGE_X ;
			TmpRect.right = TmpRect.left + UPGRADE_MESSAGE_DX;
			TmpRect.top = UPGRADE_MESSAGE_Y + 40;
			TmpRect.bottom = TmpRect.top + 30;

			CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(CSAPP_STR_UPGRADING_WAIT), -1, &TmpRect, DT_CENTER);
		}

		TmpRect.top = UPGRADE_MESSAGE_Y + 70;
		TmpRect.bottom = TmpRect.top + 30;

		SetBkMode(hdc,BM_TRANSPARENT);
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		switch ( i % 3 )
		{
			case 0:
				CS_MW_DrawText (hdc, "/", -1, &TmpRect, DT_CENTER);
				break;
			case 1:
				CS_MW_DrawText (hdc, "=", -1, &TmpRect, DT_CENTER);
				break;
			case 2:
				CS_MW_DrawText (hdc, "\\", -1, &TmpRect, DT_CENTER);
				break;
		}
		i++;

		EndPaint(hwnd,hdc);
		old_status = Sys_Com_Status;
		usleep( 100*1000 );
	}
	return ( param );
}
#endif

int Progress_Init(void)
{
	// if ( b8Type == TRUE )
		pthread_create( &hProgress_TaskHandle, NULL, Progress_Task, NULL );
#if 0
	else
		pthread_create( &hProgress_TaskHandle, NULL, Upgrade_Task, NULL );
#endif
	return( 0 );
}

void Progress_Stop(void)
{
	pthread_cancel( hProgress_TaskHandle );
}

BOOL MV_CheckDownLoadStatus(void)
{
	return Download_Status;
}

void Download_Stop(void)
{
	Download_Status = FALSE;
	pthread_cancel( hDownload_TaskHandle );
}

void *Download_Task( void *param )
{
	char 		ShellCommand[256];
	HWND		hwnd;
	int			ShellResult = 0;

	hwnd = GetActiveWindow();

	Download_Status = TRUE;

	MV_Draw_Msg_Download(hwnd, CSAPP_STR_DOWNLOAD);

	if (MV_GetNetUpgradeMode() == NET_UPGRADE_MAIN)
	{
		DownloadFileSize = atoi(stUpgrade_Data[u16Current_Wgetindex].acFile_size);
		sprintf(ShellCommand, "wget -q -O %s \"http://%s/chipbox/%s\"",TMP_UPGRADE_FILE , CS_DBU_Get_Webaddr(), stUpgrade_Data[u16Current_Wgetindex].acFile_Location);
	}
	else if (MV_GetNetUpgradeMode() == NET_UPGRADE_PLUGIN)
	{
		DownloadFileSize = atoi(stPlugin_Data[u16Current_Wgetindex].pFile_size);
		if (access(TMP_PLUGIN_FOLDER, F_OK) != 0)
		{
			sprintf(ShellCommand, "mkdir  %s", TMP_PLUGIN_FOLDER);
			system(ShellCommand);
		}
		// sprintf(ShellCommand, "wget -q -O %s \"http://%s/%s\"",TMP_PLUGIN_FILE , CS_DBU_Get_PlugInAddr(), stPlugin_Data[u16Current_Wgetindex].pFile_Name);
		sprintf(ShellCommand, "wget -q -O %s \"http://%s/%s\"",TMP_PLUGIN_FILE , MV_GetPluginSite(), stPlugin_Data[u16Current_Wgetindex].pFile_Name);
	}
	printf("Download_Task : %s\n", ShellCommand);
	ShellResult = system(ShellCommand);
	// printf("Download_Task : Download Completed %d\n", ShellResult);

	BroadcastMessage (MSG_DOWNLOAD_COMPLETE, ShellResult, 0);

	return ( param );
}

int Download_Init(void)
{
	Download_Status = FALSE;
	pthread_create( &hDownload_TaskHandle, NULL, Download_Task, NULL );
	return( 0 );
}

void MV_Draw_Msg_Download(HWND hwnd, U32 u32Message)
{
	HDC 		hdc;
	RECT		rc1;

	rc1.left = UPGRADE_MESSAGE_X;
	rc1.top = UPGRADE_MESSAGE_Y + 10;
	rc1.right = rc1.left + UPGRADE_MESSAGE_DX;
	rc1.bottom = rc1.top + MV_INSTALL_MENU_BAR_H * 2;

	hdc=BeginPaint(hwnd);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY) );

	SetBkMode(hdc,BM_TRANSPARENT);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	CS_MW_DrawText (hdc, CS_MW_LoadStringByIdx(u32Message), -1, &rc1, DT_CENTER);
	EndPaint(hwnd,hdc);

	if ( u32Message == CSAPP_STR_DOWNLOAD )
		Progress_Init();
	/*
	else if ( u32Message == CSAPP_STR_UPGRADE )
		Progress_Init(FALSE);
		*/
}

void MV_Close_Msg_Download(HWND hwnd)
{
	HDC 		hdc;

	Download_Status = FALSE;
	Progress_Stop();
	hdc=BeginPaint(hwnd);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(UPGRADE_MESSAGE_X), ScalerHeigthPixel(UPGRADE_MESSAGE_Y), ScalerWidthPixel(UPGRADE_MESSAGE_DX), ScalerHeigthPixel(UPGRADE_MESSAGE_DY) );
	EndPaint(hwnd,hdc);
}
#endif


/************************************************************************************************/
/*********************************  STRING KEYPAD PROCESS ***************************************/
/************************************************************************************************/

WPARAM	Before_Key;
#define	CHAR_LEN	6
typedef enum
{
	SMALL_CHAR = 0,
	BIG_CHAR,
	MAX_KIND_CHAR
} MV_KIND_CHAR;

char 	ac0_Key[MAX_KIND_CHAR][CHAR_LEN] = {"0 _-:/","0 _-:/"};
char 	ac1_Key[MAX_KIND_CHAR][CHAR_LEN] = {"1.,?!@","1.,?!@"};
char 	ac2_Key[MAX_KIND_CHAR][CHAR_LEN] = {"2abc","2ABC"};
char 	ac3_Key[MAX_KIND_CHAR][CHAR_LEN] = {"3def","3DEF"};
char 	ac4_Key[MAX_KIND_CHAR][CHAR_LEN] = {"4ghi","4GHI"};
char 	ac5_Key[MAX_KIND_CHAR][CHAR_LEN] = {"5jkl","5JKL"};
char 	ac6_Key[MAX_KIND_CHAR][CHAR_LEN] = {"6mno","6MNO"};
char 	ac7_Key[MAX_KIND_CHAR][CHAR_LEN] = {"7pqrs","7PQRS"};
char 	ac8_Key[MAX_KIND_CHAR][CHAR_LEN] = {"8tuv","8TUV"};
char 	ac9_Key[MAX_KIND_CHAR][CHAR_LEN] = {"9wxyz","9WXYZ"};

static U8		u8Input_Key_Index;

void Draw_String_keypad( HDC hdc );

BOOL MV_Get_StringKeypad_Status(void)
{
	return String_keypad_enable;
}

void MV_Set_StringKeypad_Status(BOOL bStatus)
{
	String_keypad_enable = bStatus;
}

char *MV_Get_StringEdited_String(void)
{
	return Returnbuff_HexaTemp;
}

void MV_Close_StringKeypad(HWND hwnd)
{
	HDC		hdc;

	MV_Set_StringKeypad_Status(FALSE);
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX), ScalerHeigthPixel(HEXA_KEYPAD_WINDOWDY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);

	UnloadBitmap(&Capture_bmp); /* By KB Kim 2011.09.20 */

}

void MV_Draw_StringEdit_String(HDC hdc, RECT *Temp_Rect)
{
	char	acTemp[2];
	int		i, str_len;
	int		Srt_length = 0;

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - KEYBOARD_KEY_OFFSET*2), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2) );

	str_len = strlen(Returnbuff_HexaTemp);
	Srt_length = str_len * 15;
	//printf("Draw_String === %d , %s ====\n", strlen(Returnbuff_HexaTemp), Returnbuff_HexaTemp);
	for ( i = 0 ; i < str_len ; i++ )
	{
		acTemp[0] = Returnbuff_HexaTemp[i];
		acTemp[1] = '\0';

		Temp_Rect->right = Temp_Rect->left + 15;

		if ( HexaKeypad_Index == i )
		{
			SetBrushColor(hdc, MVAPP_YELLOW_COLOR);
			SetTextColor(hdc,CSAPP_BLACK_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(Temp_Rect->left), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET),ScalerWidthPixel(Temp_Rect->right - Temp_Rect->left),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2));
		} else {
			SetBrushColor(hdc, MVAPP_BLACK_COLOR);
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(Temp_Rect->left), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET),ScalerWidthPixel(Temp_Rect->right - Temp_Rect->left),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2));
		}
		MV_MW_DrawText_Fixed(hdc, acTemp, -1, Temp_Rect, DT_CENTER);

		Temp_Rect->left = Temp_Rect->right;
	}
}

void Draw_String_keypad( HDC hdc )
{
	U32     keyWidth = KEYBOARD_KEY_OFFSET + 10;
	U32		u32Key_X1 = STRING_KEYPAD_KEY_BOARDX + HEXA_KEYPAD_OUTLINE_OFFSET + keyWidth;
	U32		u32Key_X2 = u32Key_X1 + keyWidth*2 + 20;
	U32		u32Key_X3 = u32Key_X2 + keyWidth*2 + 20;

	U32		u32Key_Y1 = STRING_KEYPAD_KEY_BOARDY + 50;
	U32		u32Key_Y2 = u32Key_Y1 + KEYBOARD_KEY_OFFSET + 20;
	U32		u32Key_Y3 = u32Key_Y2 + KEYBOARD_KEY_OFFSET + 20;
	U32		u32Key_Y4 = u32Key_Y3 + KEYBOARD_KEY_OFFSET + 20;

	char	a[20];
	RECT	Temp_Rect;

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY),ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDDY));
	FillBox(hdc,ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDDX),ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= STRING_KEYPAD_KEY_BOARDY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= STRING_KEYPAD_KEY_BOARDX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + STRING_KEYPAD_KEY_BOARDDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_KEY_BOARD);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_1_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_2_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_3_ICON]);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	a[0] = '.';
	a[1] = ',';
	a[2] = '?';
	a[3] = '!';
	a[4] = '@';
	a[5] = '\0';

	Temp_Rect.top 	= ScalerHeigthPixel(u32Key_Y1 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight));
	Temp_Rect.bottom	= Temp_Rect.top + ScalerHeigthPixel(KEYBOARD_KEY_OFFSET);
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X1 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 5),ScalerHeigthPixel(u32Key_Y1 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight) ), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'A';
		a[1] = 'B';
		a[2] = 'C';
		a[3] = '\0';
	} else {
		a[0] = 'a';
		a[1] = 'b';
		a[2] = 'c';
		a[3] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X2 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 - 5),ScalerHeigthPixel(u32Key_Y1 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'D';
		a[1] = 'E';
		a[2] = 'F';
		a[3] = '\0';
	} else {
		a[0] = 'd';
		a[1] = 'e';
		a[2] = 'f';
		a[3] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X3 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 - 5),ScalerHeigthPixel(u32Key_Y1 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_4_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_5_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y2), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_6_ICON]);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'G';
		a[1] = 'H';
		a[2] = 'I';
		a[3] = '\0';
	} else {
		a[0] = 'g';
		a[1] = 'h';
		a[2] = 'i';
		a[3] = '\0';
	}
	Temp_Rect.top 	= ScalerHeigthPixel(u32Key_Y2 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight));
	Temp_Rect.bottom	= Temp_Rect.top + ScalerHeigthPixel(KEYBOARD_KEY_OFFSET);
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X1 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 5),ScalerHeigthPixel(u32Key_Y2 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight) ), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'J';
		a[1] = 'K';
		a[2] = 'L';
		a[3] = '\0';
	} else {
		a[0] = 'j';
		a[1] = 'h';
		a[2] = 'l';
		a[3] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X2 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 - 5),ScalerHeigthPixel(u32Key_Y2 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'M';
		a[1] = 'N';
		a[2] = 'O';
		a[3] = '\0';
	} else {
		a[0] = 'm';
		a[1] = 'n';
		a[2] = 'o';
		a[3] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X3 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 - 5),ScalerHeigthPixel(u32Key_Y2 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_7_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_8_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y3), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_9_ICON]);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'P';
		a[1] = 'Q';
		a[2] = 'R';
		a[3] = 'S';
		a[4] = '\0';
	} else {
		a[0] = 'p';
		a[1] = 'q';
		a[2] = 'r';
		a[3] = 's';
		a[4] = '\0';
	}
	Temp_Rect.top 	= ScalerHeigthPixel(u32Key_Y3 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight));
	Temp_Rect.bottom	= Temp_Rect.top + ScalerHeigthPixel(KEYBOARD_KEY_OFFSET);
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X1 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 12),ScalerHeigthPixel(u32Key_Y3 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight) ), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'T';
		a[1] = 'U';
		a[2] = 'V';
		a[3] = '\0';
	} else {
		a[0] = 't';
		a[1] = 'u';
		a[2] = 'v';
		a[3] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X2 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 - 5),ScalerHeigthPixel(u32Key_Y3 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);
	if ( b8CapsLock == BIG_CHAR )
	{
		a[0] = 'W';
		a[1] = 'X';
		a[2] = 'Y';
		a[3] = 'Z';
		a[4] = '\0';
	} else {
		a[0] = 'w';
		a[1] = 'x';
		a[2] = 'y';
		a[3] = 'z';
		a[4] = '\0';
	}
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X3 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 - 12),ScalerHeigthPixel(u32Key_Y3 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_0_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y4), ScalerWidthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight), &MV_BMP[MVBMP_KEY_MUTE_ICON]);

	a[0] = 's';
	a[1] = 'p';
	a[2] = '_';
	a[3] = '-';
	a[4] = ':';
	a[5] = '/';
	a[6] = '\0';
	Temp_Rect.top 	= ScalerHeigthPixel(u32Key_Y4 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight));
	Temp_Rect.bottom	= Temp_Rect.top + ScalerHeigthPixel(KEYBOARD_KEY_OFFSET);
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X2 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 - 10),ScalerHeigthPixel(u32Key_Y4 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);

	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	sprintf(a, CS_MW_LoadStringByIdx(CSAPP_STR_CAPS));
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X1 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 - 10),ScalerHeigthPixel(u32Key_Y4 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);
	sprintf(a, CS_MW_LoadStringByIdx(CSAPP_STR_DEL));
	Temp_Rect.left	= ScalerWidthPixel(u32Key_X3 - keyWidth);
	Temp_Rect.right	= Temp_Rect.left + ScalerWidthPixel((keyWidth * 2) + 20);
	CS_MW_DrawText(hdc, a, -1, &Temp_Rect, DT_CENTER);
	// MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 - 10),ScalerHeigthPixel(u32Key_Y4 + ScalerHeigthPixel(MV_BMP[MVBMP_KEY_0_ICON].bmHeight)), a);
}

void Draw_String_Keypad_Help(HDC hdc)
{
	U32		u32Key_X1 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_OUTLINE_OFFSET;
	U32		u32Key_X2 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX/3;
	U32		u32Key_X3 = HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX*2/3;
	U32		u32Key_Y1 = HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_OUTLINE_OFFSET;

	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_HELP_X + HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_HELP_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y),ScalerWidthPixel(HEXA_KEYPAD_HELP_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(HEXA_KEYPAD_HELP_DY));
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_HELP_X), ScalerHeigthPixel(HEXA_KEYPAD_HELP_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(HEXA_KEYPAD_HELP_DX),ScalerHeigthPixel(HEXA_KEYPAD_HELP_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X1), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_OK_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_OK_ICON].bmHeight), &MV_BMP[MVBMP_OK_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X2), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_EXIT_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_EXIT_ICON].bmHeight), &MV_BMP[MVBMP_EXIT_ICON]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(u32Key_X3), ScalerHeigthPixel(u32Key_Y1), ScalerWidthPixel(MV_BMP[MVBMP_KEY_PREV_ICON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_KEY_PREV_ICON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X1 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_SAVE));
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X2 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_CANCEL));
	MV_CS_MW_TextOut( hdc,ScalerWidthPixel(u32Key_X3 + 40),ScalerHeigthPixel(u32Key_Y1 + 4), CS_MW_LoadStringByIdx(CSAPP_STR_CAPSLOCK));

}

void MV_Draw_StringKeypad(HWND hwnd, U8 *item_str, U8 max_string_length)
{
	HDC		hdc;
	RECT	Temp_Rect;

	HexaKeypad_X = 0;
	HexaKeypad_Y = 1;
	Before_Key = 0;
	String_keypad_enable = TRUE;
	b8CapsLock = SMALL_CHAR;
	u8Input_Key_Index = 0;
	u8max_Hexastr_length = max_string_length;
	Returnbuff_HexaTemp[0] = '\0';
	b8keypad_save_str = FALSE;

	HexaKeypad_Index = 0;

	memset(Returnbuff_HexaTemp, 0x00, 256);

	/* For Plugin Site List by File : KB Kim 2011.09.13 */
	if (item_str != NULL)
	{
		strcpy(Returnbuff_HexaTemp, item_str);
	}

	hdc = MV_BeginPaint(hwnd);

	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX), ScalerHeigthPixel(HEXA_KEYPAD_WINDOWDY), &Capture_bmp);
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX), ScalerHeigthPixel(HEXA_KEYPAD_WINDOWDY) );

/*******************************  Display Edited String  ******************************************/
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY),ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY));
	FillBox(hdc,ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX),ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET), ScalerWidthPixel(HEXA_KEYPAD_STR_BOARDDX - KEYBOARD_KEY_OFFSET*2), ScalerHeigthPixel(HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2) );

	Temp_Rect.top 	= HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET;
	Temp_Rect.right	= Temp_Rect.left + HEXA_KEYPAD_STR_BOARDDX - HEXA_KEYPAD_OUTLINE_OFFSET*2;
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_Draw_StringEdit_String(hdc, &Temp_Rect);

/*******************************  Draw Keypad  ******************************************/
	FillBoxWithBitmap (hdc, ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX + STRING_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY + STRING_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX + STRING_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY + STRING_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY),ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDDY));
	FillBox(hdc,ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDX), ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(STRING_KEYPAD_KEY_BOARDDX),ScalerHeigthPixel(STRING_KEYPAD_KEY_BOARDDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= STRING_KEYPAD_KEY_BOARDY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= STRING_KEYPAD_KEY_BOARDX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + STRING_KEYPAD_KEY_BOARDDX - WINDOW_OUT_GAP*2;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_KEY_BOARD);

	Draw_String_keypad(hdc);

/*******************************  Draw Keypad Help  ******************************************/
	Draw_String_Keypad_Help(hdc);

	MV_EndPaint(hwnd,hdc);
}

void MV_StringKeypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC			hdc;
	RECT		Temp_Rect;
	int			str_len = u8max_Hexastr_length - 1; //strlen(Returnbuff_HexaTemp) - 1;

	Temp_Rect.top 	= HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_OUTLINE_OFFSET + 4;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= HEXA_KEYPAD_STR_BOARDX + KEYBOARD_KEY_OFFSET;
	Temp_Rect.right	= Temp_Rect.left + HEXA_KEYPAD_STR_BOARDDY - HEXA_KEYPAD_OUTLINE_OFFSET*2;

	if ( u8Key <= CSAPP_KEY_0 && u8Key >= CSAPP_KEY_1 )
	{
		if ( HexaKeypad_Index < u8max_Hexastr_length )
		{
			if ( Before_Key == 0 )
			{
				Before_Key = u8Key;
			}
			else if ( Before_Key != u8Key )
			{
				HexaKeypad_Index++;
				u8Input_Key_Index = 0;
				Before_Key = u8Key;

				if (HexaKeypad_Index >= u8max_Hexastr_length)
				{
					HexaKeypad_Index = u8max_Hexastr_length - 1;
				}
			} else
				u8Input_Key_Index++;
		}
		else
		{
			return;
		}
	} else {
		Before_Key = 0;
	}

	switch( u8Key )
	{
		case CSAPP_KEY_LEFT:
			if ( HexaKeypad_Index > 0 )
			{
				HexaKeypad_Index--;
				u8Input_Key_Index = 0;
			}

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			if ( HexaKeypad_Index < str_len && HexaKeypad_Index < strlen(Returnbuff_HexaTemp))
			{
				HexaKeypad_Index++;
				u8Input_Key_Index = 0;
			}

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_0:
			if ( u8Input_Key_Index == 6 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac0_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_1:
			if ( u8Input_Key_Index == 6 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac1_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_2:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac2_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_3:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac3_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_4:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac4_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_5:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac5_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_6:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac6_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_7:
			if ( u8Input_Key_Index == 5 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac7_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_8:
			if ( u8Input_Key_Index == 4 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac8_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_9:
			if ( u8Input_Key_Index == 5 )
				u8Input_Key_Index = 0;

			Returnbuff_HexaTemp[HexaKeypad_Index] = ac9_Key[b8CapsLock][u8Input_Key_Index];

			hdc = MV_BeginPaint(hwnd);
			MV_Draw_StringEdit_String(hdc, &Temp_Rect);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_SWAP:
			if ( b8CapsLock == BIG_CHAR )
				b8CapsLock = SMALL_CHAR;
			else
				b8CapsLock = BIG_CHAR;
			hdc = MV_BeginPaint(hwnd);
			Draw_String_keypad(hdc);
			MV_EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_MUTE:
			{
				int 		i;

				for ( i = HexaKeypad_Index ; i < str_len ; i++ )
				{
					Returnbuff_HexaTemp[i] = Returnbuff_HexaTemp[i + 1];
				}

				Returnbuff_HexaTemp[str_len] = 0x00;

				// str_len = strlen( Returnbuff_HexaTemp ) - 1;

				//printf("==== %d : %s ====\n", strlen(Returnbuff_HexaTemp), Returnbuff_HexaTemp);

				hdc = MV_BeginPaint(hwnd);
				MV_Draw_StringEdit_String(hdc, &Temp_Rect);
				MV_EndPaint(hwnd,hdc);
			}
			break;

		case CSAPP_KEY_ENTER:
			MV_Close_StringKeypad(hwnd);
			break;

		case CSAPP_KEY_EXIT:
			MV_Close_StringKeypad(hwnd);
			break;

		default:
			break;
	}
}

/************************************************************************************************/
/************************************************************************************************/

st_pvr_data	Temp_PVR_Rec_Data;

void MV_Parser_PVR_Value( char *Temp, char *tempSection )
{
	U16		i, j;
	char 	Check_Str[10];

	memset (Check_Str, 0, sizeof(char) * 10);
//printf("=1= %s == %s\n", tempSection, Temp);
	for ( i = 0 ; i < strlen(tempSection) ; i++ )
	{
		if ( tempSection[i] == ':' )
			break;
		else
			Check_Str[i] = tempSection[i];
	}

	j = i+1;

	for ( i = 0 ; i < strlen(tempSection) - j ; i++ )
	{
		if ( tempSection[i+j] == '\n' )
			break;

		Temp[i] = tempSection[i+j];
	}
//printf("2== %s == %s\n\n", Check_Str, Temp);
	for ( i = 0 ; i < PVR_DATA_KIND ; i++ )
		if ( strcmp(PVR_Check_String[i], Check_Str) == 0 )
		{
			switch(i)
			{
				case PVR_REC_ITEM_CHINDEX:
					//printf("TI : Temp : %s\n", Temp);
					strncpy(Temp_PVR_Rec_Data.PVR_Ch_Index, Temp, strlen(Temp) );
					break;
				case PVR_REC_ITEM_SAT:
					strncpy(Temp_PVR_Rec_Data.PVR_Sat_Name, Temp, strlen(Temp) );
					break;
				case PVR_REC_ITEM_TP:
					strncpy(Temp_PVR_Rec_Data.PVR_TP_Info, Temp, strlen(Temp) );
					break;
				case PVR_REC_ITEM_TITLE:
					//printf("TI : Temp : %s\n", Temp);
					if ( strlen(Temp) > 0 )
						strncpy( Temp_PVR_Rec_Data.PVR_Title, Temp, strlen(Temp) );
					else
						sprintf( Temp_PVR_Rec_Data.PVR_Title, "No Title");
					break;
				case PVR_REC_ITEM_ST:
					//printf("ST : Temp : %s\n", Temp);
					strncpy( Temp_PVR_Rec_Data.PVR_Start_UTC, Temp, strlen(Temp) );
					//printf("ST : Temp_PVR_Rec_Data.PVR_Start_UTC : %s\n", Temp_PVR_Rec_Data.PVR_Start_UTC);
					break;
				case PVR_REC_ITEM_SD:
					Temp_PVR_Rec_Data.PVR_Start_OS = (U32)atoi(Temp);
					break;
				case PVR_REC_ITEM_ET:
					//printf("ET : Temp : %s\n", Temp);
					strncpy( Temp_PVR_Rec_Data.PVR_End_UTC, Temp, strlen(Temp) );
					//printf("ET : Temp_PVR_Rec_Data.PVR_Start_UTC : %s\n", Temp_PVR_Rec_Data.PVR_Start_UTC);
					break;
				case PVR_REC_ITEM_ED:
					Temp_PVR_Rec_Data.PVR_End_OS = (U32)atoi(Temp);
					break;
				case PVR_REC_ITEM_LP:
					Temp_PVR_Rec_Data.PVR_Last_Posion = MV_Trans_String_to_long(Temp);
					//printf("LP : Temp_PVR_Rec_Data.PVR_Last_Posion : %s ==> %lld\n", Temp, Temp_PVR_Rec_Data.PVR_Last_Posion);
					break;
				default :
					break;
			}

			break;
		}

}

void MV_PVR_CFG_File_Parser(st_pvr_data *PVR_Rec_Data, char *PVR_FileName)
{
	int				i;
	FILE* 			fp = NULL;
	char			TmpStr[256];
	char			Temp[100];
	char			Temp_Str[256];
	char			File_Str[256];
	char 			tempSection [DHCP_FILE_MAX_COL + 2];

	memset (&Temp_PVR_Rec_Data, 0x00, sizeof(st_pvr_data));
	memset (File_Str, 0, sizeof(char) * 256);
	memset (TmpStr, 0, sizeof(char) * 256);
	memset (Temp, 0, sizeof(char) * 100);
	memset (Temp_Str, 0, sizeof(char) * 256);
	memset (tempSection, 0, DHCP_FILE_MAX_COL + 2);

	sprintf(TmpStr, "%s", PVR_FileName);

	for ( i = strlen(TmpStr) ; i > 0 ; i-- )
	{
		if ( TmpStr[i] == '.' )
		{
			strncpy(Temp_Str, TmpStr, i+1 );
			break;
		}
	}

	sprintf(File_Str, "%s%s", Temp_Str, "cfg");

	if (access( File_Str, 0 ) == 0 )
	{
		if (!(fp = fopen(File_Str, "r")))
		{
			printf("\nError !!!!  Read File : %s ==\n", File_Str);
			return;
		}
	} else {
		return;
	}

	while (!feof(fp)) {
		memset (tempSection, 0, sizeof(char) * (DHCP_FILE_MAX_COL + 2));
		memset (Temp, 0, sizeof(char) * 100);

        if ( !fgets(tempSection, DHCP_FILE_MAX_COL, fp))
			break;

		//printf("Read Data : %s - %d ==\n", tempSection, strlen(tempSection));

		if ( strlen(tempSection) > 1 )
			MV_Parser_PVR_Value(Temp, tempSection);
    }

	memcpy( PVR_Rec_Data, &Temp_PVR_Rec_Data, sizeof(st_pvr_data));
	fclose (fp);
}

MV_CFG_RETURN MV_PVR_CFG_File_Set_LastPosition(char *PVR_FileName, long long Last_Position)
{
	U8		i;
	FILE* 	fp;
	FILE*	fp_temp;
    char 	tempSection [CFG_MAX_COL + 2];
	char	Temp[100];
	char 	ShellCommand[64];
	char 	Check_Str[10];

	if (!(fp = fopen(PVR_FileName, "r")))
	{
		printf("\n File open Error\n");
        return CFG_NOFILE;
	}

	if (!(fp_temp = fopen(TEMP_FILE, "w+a")))
	{
		printf("\n Temp File create Error\n");
		fclose (fp);
        return CFG_NOFILE;
	}

	while (!feof(fp)) {
		memset (Check_Str, 0, sizeof(char) * 10);
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			printf("\n File read Error\n");
			fclose (fp);
			fclose (fp_temp);
			return CFG_OK;
        }

		for ( i = 0 ; i < strlen(tempSection) ; i++ )
		{
			if ( tempSection[i] == ':' )
				break;
			else
				Check_Str[i] = tempSection[i];
		}

		if ( strcmp(PVR_Check_String[PVR_REC_ITEM_LP], Check_Str) == 0 )
		{
			fprintf(fp_temp, "LP:%lld\n", Last_Position);
//			printf("Last_Position : %lld\n", Last_Position);
			break;
		} else {
			fprintf(fp_temp, "%s", tempSection);
//			printf("%s = WRITE =======\n", tempSection);
		}
    }

	fclose (fp);
	fclose (fp_temp);

	sprintf(ShellCommand, "rm '%s'", PVR_FileName);

	if ( system( ShellCommand ) )
	{
		printf(" Remove Error : %s =============== \n", ShellCommand);
		return CFG_READ_FAIL;
	}

	sprintf(ShellCommand, "cp '%s' '%s'", TEMP_FILE, PVR_FileName);

	if ( system( ShellCommand ) )
	{
		printf(" Copy Error : %s =============== \n", ShellCommand);
		return CFG_READ_FAIL;
	}

	sprintf(ShellCommand, "rm '%s'", TEMP_FILE);

	if ( system( ShellCommand ) )
	{
		printf(" Remove Error : %s =============== \n", ShellCommand);
		return CFG_READ_FAIL;
	}

	return CFG_OK;
}

static stFile_db			stFile;
static char 				acCurrent_Dir[FILE_MAX_NAME_LENGTH];

extern void MV_Calculate_Size(long long llong, char *temp);

void MV_File_Sort(void)
{
	int 			i = 0, j = 0;
	U16				Dir_count = 0;
	U16				File_count = 0;
	stFile_db_temp	stSort_Temp;
	stFile_db		stFileDB_Dir;
	stFile_db		stFileDB_File;
	struct stat		statbuffer;
	char			TmpStr[FILE_MAX_NAME_LENGTH];

	memset ( &stSort_Temp, 0x00, sizeof(stFile_db_temp));
	memset ( &stFileDB_Dir, 0x00, sizeof(stFile_db));
	memset ( &stFileDB_File, 0x00, sizeof(stFile_db));

	for ( i = 0 ; i < stFile.u16file_Count ; i++ )
	{
		sprintf(TmpStr, "%s/%s", acCurrent_Dir, stFile.acFileName[i]);

		if( stat(TmpStr, &statbuffer ) != 0 )
			printf("STAT ERROR========================\n");

		if( S_ISDIR(statbuffer.st_mode) )
		{
			strcpy(stFileDB_Dir.acFileName[Dir_count], stFile.acFileName[i]);
			strcpy(stFileDB_Dir.acFileExt[Dir_count], stFile.acFileExt[i]);
			Dir_count++;
			//printf("DIRECTORY : %d : %s <- %s \n", Dir_count, stFileDB_Dir.acFileName[Dir_count-1] , stFile.acFileName[i]);
		} else {
			strcpy(stFileDB_File.acFileName[File_count], stFile.acFileName[i]);
			strcpy(stFileDB_File.acFileExt[File_count], stFile.acFileExt[i]);
			File_count++;
			//printf("FILE : %d : %s <- %s\n", File_count, stFileDB_File.acFileName[File_count-1] , stFile.acFileName[i]);
		}
	}
	stFileDB_Dir.u16file_Count = Dir_count;
	stFileDB_File.u16file_Count = File_count;

	i = 0 ;

	while ( i < stFileDB_Dir.u16file_Count )
	{
		for ( j = i ; j < stFileDB_Dir.u16file_Count - 1 ; j++ )
		{
			if ( strcasecmp( stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]) > 0 )
			{
				//printf("=== %s => %s \n", stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]);
				strcpy(stSort_Temp.acFileName, stFileDB_Dir.acFileName[j]);
				strcpy(stSort_Temp.acFileExt, stFileDB_Dir.acFileExt[j]);

				strcpy(stFileDB_Dir.acFileName[j], stFileDB_Dir.acFileName[j+1]);
				strcpy(stFileDB_Dir.acFileExt[j], stFileDB_Dir.acFileExt[j+1]);

				strcpy(stFileDB_Dir.acFileName[j+1], stSort_Temp.acFileName);
				strcpy(stFileDB_Dir.acFileExt[j+1], stSort_Temp.acFileExt);
			}
		}
		i++;
	}

	i = 0;

	while ( i < stFileDB_File.u16file_Count )
	{
		for ( j = i ; j < stFileDB_File.u16file_Count - 1 ; j++ )
		{
			if ( strcasecmp( stFileDB_File.acFileName[j], stFileDB_File.acFileName[j+1]) > 0 )
			{
				strcpy(stSort_Temp.acFileName, stFileDB_File.acFileName[j]);
				strcpy(stSort_Temp.acFileExt, stFileDB_File.acFileExt[j]);

				strcpy(stFileDB_File.acFileName[j], stFileDB_File.acFileName[j+1]);
				strcpy(stFileDB_File.acFileExt[j], stFileDB_File.acFileExt[j+1]);

				strcpy(stFileDB_File.acFileName[j+1], stSort_Temp.acFileName);
				strcpy(stFileDB_File.acFileExt[j+1], stSort_Temp.acFileExt);
			}
		}
		i++;
	}

	//printf("\n=== DIR : %d === FILE : %d ===\n", stFileDB_Dir.u16file_Count, stFileDB_File.u16file_Count);

	memset ( &stFile, 0x00, sizeof(stFile_db));

	for ( i = 0; i < stFileDB_Dir.u16file_Count ; i++ )
	{
		strcpy(stFile.acFileName[i] , stFileDB_Dir.acFileName[i]);
		strcpy(stFile.acFileExt[i] , stFileDB_Dir.acFileExt[i]);
		//printf("DIRECTORY : %d : %s -> %s \n", i, stFileDB_Dir.acFileName[i] , stFile.acFileName[i]);
	}

	for ( i = 0; i < stFileDB_File.u16file_Count ; i++ )
	{
		strcpy(stFile.acFileName[stFileDB_Dir.u16file_Count + i] , stFileDB_File.acFileName[i]);
		strcpy(stFile.acFileExt[stFileDB_Dir.u16file_Count + i] , stFileDB_File.acFileExt[i]);
		//printf("FILE : %d : %s -> %s \n", i, stFileDB_File.acFileName[i] , stFile.acFileName[stFileDB_Dir.u16file_Count + i]);
	}

	stFile.u16file_Count = stFileDB_Dir.u16file_Count + stFileDB_File.u16file_Count;
}

MV_File_Return MV_File_Load_Data(char *Read_Dir)
{
	DIR				*pDir = NULL;
	struct dirent 	*pDirEnt;
	struct stat		statbuffer;
	U16				i;
	char			TmpStr[FILE_MAX_NAME_LENGTH];
	char			Tmp_DirStr[FILE_MAX_NAME_LENGTH];
	char			Tmp_ExtStr[10];

	memset(&stFile, 0x00, sizeof(stFile_db));

	if( (pDir=opendir( Read_Dir )) == NULL )
	{
		closedir( pDir );
		printf( "[UsbCon_Mount]:GetUsbDevFileName(): Failed to open dir.\n" );
		return FILE_READ_FAIL;
	}

	if ( strcmp(Read_Dir, USB_ROOT) == 0 )
	{
		while( (pDirEnt=readdir( pDir )) != NULL )
		{
			memset(Tmp_DirStr, 0x00, FILE_MAX_NAME_LENGTH);
			memset(Tmp_ExtStr, 0x00, 10);

			sprintf(TmpStr, "%s/%s", acCurrent_Dir, pDirEnt->d_name);
			if( stat(TmpStr, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");

			if( strcmp( pDirEnt->d_name, "." ) != 0 && strcmp( pDirEnt->d_name, "..") != 0 )
			{
				strcpy(Tmp_DirStr, pDirEnt->d_name);

				for ( i = strlen(pDirEnt->d_name) ; i > 0 ; i-- )
				{
					if ( pDirEnt->d_name[i] == '.' )
						break;
				}

				if( S_ISDIR(statbuffer.st_mode) )
					memset(stFile.acFileExt[stFile.u16file_Count], 0x00, 10);
				else if ( i < strlen(pDirEnt->d_name) - 1 && i > 1 )
				{
					//printf("==== %d , %s , %c , %d ====\n", i, pDirEnt->d_name, pDirEnt->d_name[i+1], strlen(pDirEnt->d_name));
					strncpy(Tmp_ExtStr, &pDirEnt->d_name[i+1], strlen(pDirEnt->d_name) - i);
					//printf("==== %s : EXT : %d : %s %s =======\n", stFile.acFileName[stFile.u16file_Count], i, stFile.acFileExt[stFile.u16file_Count], Tmp_ExtStr);
				}
			}

			if( !(S_ISDIR(statbuffer.st_mode)) && strcmp(Tmp_ExtStr, "tar") == 0 /*&& strncmp(Tmp_DirStr,"All",3) == 0*/ )
			{
				strncpy(stFile.acFileName[stFile.u16file_Count], Tmp_DirStr, strlen(Tmp_DirStr));
				strncpy(stFile.acFileExt[stFile.u16file_Count], Tmp_ExtStr, strlen(Tmp_ExtStr));
				stFile.u16file_Count++;
			}
		}
	}
#if 0
	else {
		while( (pDirEnt=readdir( pDir )) != NULL )
		{
			sprintf(TmpStr, "%s/%s", acCurrent_Dir, pDirEnt->d_name);
			if( stat(TmpStr, &statbuffer ) != 0 )
				printf("STAT ERROR========================\n");

			if( strcmp( pDirEnt->d_name, "." ) != 0 )
			{
				strcpy(stFile.acFileName[stFile.u16file_Count], pDirEnt->d_name);

				if( strcmp( pDirEnt->d_name, ".." ) != 0 )
				{
					for ( i = strlen(pDirEnt->d_name) ; i > 0 ; i-- )
					{
						if ( pDirEnt->d_name[i] == '.' )
							break;
					}

					if( S_ISDIR(statbuffer.st_mode) )
						memset(stFile.acFileExt[stFile.u16file_Count], 0x00, 10);
					else if ( i < strlen(pDirEnt->d_name) - 1 && i > 1 )
					{
						//printf("==== %d , %s , %c , %d ====\n", i, pDirEnt->d_name, pDirEnt->d_name[i+1], strlen(pDirEnt->d_name));
						strncpy(stFile.acFileExt[stFile.u16file_Count], &pDirEnt->d_name[i+1], strlen(pDirEnt->d_name) - i);
						//printf("==== %s : EXT : %d : %s =======\n", stFile.acFileName[stFile.u16file_Count], i, stFile.acFileExt[stFile.u16file_Count]);
					}
				}

				stFile.u16file_Count++;
			}
		}
	}
#endif

	MV_File_Sort();

	closedir( pDir );
	return FILE_OK;
}

void MV_File_Under_Dir(void)
{
	int		i = 0;

	for ( i = strlen(acCurrent_Dir) ; i > 0 ; i-- )
	{
		if ( acCurrent_Dir[i] == '/' )
		{
			acCurrent_Dir[i] = 0x00;
			break;
		} else {
			acCurrent_Dir[i] = 0x00;
		}
	}
}

void MV_Draw_FileListBar(HDC hdc, int esItem, U8 u8Kind)
{
	int 			y_gap = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_HEIGHT * esItem;
	RECT			TmpRect;
	struct stat		statbuffer;
	char			TmpStr[FILE_MAX_NAME_LENGTH];

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	if( u8Kind == FOCUS ) {
		FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(y_gap), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT), &MV_BMP[MVBMP_CHLIST_SELBAR]);
	} else {
		if ( esItem % 2 == 0 )
		{
			MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		} else {
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX),ScalerHeigthPixel(FILE_WINDOW_ITEM_HEIGHT) );
		}
	}

	if ( ( esItem + ( u16Current_FilePage * FILE_LIST_MAX_ITEM) ) < stFile.u16file_Count )
	{
		TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_ITEM_X + MV_BMP[MVBMP_TS_FILE].bmWidth + 4);
		TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_NAME_DX;
		TmpRect.top		=ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

		sprintf(TmpStr, "%s/%s", acCurrent_Dir, stFile.acFileName[esItem + ( u16Current_FilePage * FILE_LIST_MAX_ITEM)]);
		if( stat(TmpStr, &statbuffer ) != 0 )
			printf("STAT ERROR========================\n");

		if( S_ISDIR(statbuffer.st_mode) )
		{
			if ( strcmp ( stFile.acFileName[esItem + ( u16Current_FilePage * FILE_LIST_MAX_ITEM)], ".." ) != 0 )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_DIR_FOLDER].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_DIR_FOLDER].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_DIR_FOLDER].bmHeight), &MV_BMP[MVBMP_DIR_FOLDER]);
		}
		else
		{
			switch( MV_Check_Filetype(stFile.acFileExt[esItem + ( u16Current_FilePage * FILE_LIST_MAX_ITEM)] ) )
			{
				case MVAPP_FILE_TS:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_TS_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_TS_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_TS_FILE].bmHeight), &MV_BMP[MVBMP_TS_FILE]);
					break;
				case MVAPP_FILE_MOVIE:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_MOVIE_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_MOVIE_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MOVIE_FILE].bmHeight), &MV_BMP[MVBMP_MOVIE_FILE]);
					break;
				case MVAPP_FILE_MUSIC:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_MUSIC_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_MUSIC_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_MUSIC_FILE].bmHeight), &MV_BMP[MVBMP_MUSIC_FILE]);
					break;
				case MVAPP_FILE_PIC:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_IMAGE_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_IMAGE_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_IMAGE_FILE].bmHeight), &MV_BMP[MVBMP_IMAGE_FILE]);
					break;
				case MVAPP_FILE_TEXT:
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_TEXT_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_TEXT_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_TEXT_FILE].bmHeight), &MV_BMP[MVBMP_TEXT_FILE]);
					break;
				default :
					FillBoxWithBitmap (hdc, ScalerWidthPixel(TmpRect.left - MV_BMP[MVBMP_NORMAL_FILE].bmWidth ), ScalerHeigthPixel(y_gap + 2), ScalerWidthPixel(MV_BMP[MVBMP_NORMAL_FILE].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_NORMAL_FILE].bmHeight), &MV_BMP[MVBMP_NORMAL_FILE]);
					break;
			}
		}

		TmpRect.left	=ScalerWidthPixel(FILE_WINDOW_ITEM_X + MV_BMP[MVBMP_TS_FILE].bmWidth + 10);
		CS_MW_DrawText(hdc, stFile.acFileName[esItem + ( u16Current_FilePage * FILE_LIST_MAX_ITEM)], -1, &TmpRect, DT_LEFT);

		if( !(S_ISDIR(statbuffer.st_mode)) )
		{
			TmpRect.left	=TmpRect.right;
			TmpRect.right	=TmpRect.left + FILE_WINDOW_ITEM_SIZE_DX;

			MV_Calculate_Size( ( statbuffer.st_size / 1024 ), TmpStr);
			//sprintf(TmpStr, "%d", statbuffer.st_size);
			CS_MW_DrawText(hdc, TmpStr, -1, &TmpRect, DT_RIGHT);
		}
	}

	if ( u8Kind == FOCUS )
	{
		TmpRect.top = FILE_WINDOW_ITEM_Y;
		TmpRect.left = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX - SCROLL_BAR_DX;
		TmpRect.right = FILE_WINDOW_ITEM_X + FILE_WINDOW_ITEM_DX;
		TmpRect.bottom = FILE_WINDOW_ITEM_Y + FILE_WINDOW_ITEM_DY;

		if ( stFile.u16file_Count > 0 )
			MV_Draw_ScrollBar(hdc, TmpRect, u16Current_Fileindex, stFile.u16file_Count - 1, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
		else
			MV_Draw_ScrollBar(hdc, TmpRect, u16Current_Fileindex, stFile.u16file_Count, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_File_List(HWND hwnd)
{
	U16 	i = 0;
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);

	for( i = 0 ; i < FILE_LIST_MAX_ITEM ; i++ )
	{
		if ( i == u16File_Focus )
			MV_Draw_FileListBar(hdc, i, FOCUS);
		else
			MV_Draw_FileListBar(hdc, i, UNFOCUS);
	}

	MV_EndPaint(hwnd,hdc);
}

void MV_Draw_File_Window(HWND hwnd)
{
	HDC 	hdc;
	RECT	Temp_Rect;

	u16File_Focus = 0;
	u16Current_FilePage = 0;
	u16Prev_FilePage = 0;
	b8Check_Enter = FALSE;
	u16Current_Fileindex = 0;
	memset( acReturn_File, 0x00, FILE_MAX_NAME_LENGTH );
	File_Window_Flag = TRUE;
	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FILE_WINDOW_X + FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y + FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(FILE_WINDOW_Y),ScalerWidthPixel(FILE_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(FILE_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(FILE_WINDOW_DX),ScalerHeigthPixel(FILE_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= FILE_WINDOW_TITLE_Y;
	Temp_Rect.bottom	= FILE_WINDOW_TITLE_Y + FILE_WINDOW_ITEM_HEIGHT;
	Temp_Rect.left	= FILE_WINDOW_TITLE_X;
	Temp_Rect.right	= Temp_Rect.left + FILE_WINDOW_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_FILE_EXPLORE);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(FILE_WINDOW_ITEM_X), ScalerHeigthPixel(FILE_WINDOW_ITEM_Y), ScalerWidthPixel(FILE_WINDOW_ITEM_DX), ScalerHeigthPixel(FILE_WINDOW_ITEM_DY) );

	MV_EndPaint(hwnd,hdc);

	memset(acCurrent_Dir, 0x00, FILE_MAX_NAME_LENGTH);
	strncpy( acCurrent_Dir, USB_ROOT, strlen(USB_ROOT));
	MV_File_Load_Data(acCurrent_Dir);
	MV_Draw_File_List(hwnd);
}

BOOL MV_Check_File_Window(void)
{
	return File_Window_Flag;
}

void MV_Close_File_Window(HDC hdc)
{
	File_Window_Flag = FALSE;
	FillBoxWithBitmap(hdc, ScalerWidthPixel(FILE_WINDOW_X), ScalerHeigthPixel(FILE_WINDOW_Y), ScalerWidthPixel(FILE_WINDOW_DX), ScalerHeigthPixel(FILE_WINDOW_DY), &btFile_cap);
	UnloadBitmap (&btFile_cap);
}

BOOL MV_File_Check_Enter(void)
{
	//printf("=== b8Check_Enter : %d ===== \n", b8Check_Enter);
	return b8Check_Enter;
}

void MV_File_Set_Enter(BOOL b8Enter, char *acTemp_Str, char *acTemp_Ext)
{
	b8Check_Enter = b8Enter;

	memset(acReturn_File, 0x00, FILE_MAX_NAME_LENGTH);
	memset(acReturn_File_Ext, 0x00, FILE_MAX_NAME_LENGTH);

	strncpy(acReturn_File, acTemp_Str, strlen(acTemp_Str));
	strncpy(acReturn_File_Ext, acTemp_Ext, strlen(acTemp_Ext));
}

char *MV_Get_FileName(void)
{
	return acReturn_File;
}

char *MV_Get_FileExt(void)
{
	return acReturn_File_Ext;
}


BOOL MV_FileList_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
			hdc = BeginPaint(hwnd);
			u16Prev_FilePage = u16Current_FilePage;
			MV_Draw_FileListBar(hdc, u16File_Focus, UNFOCUS);

			if ( ( u16Current_Fileindex >= stFile.u16file_Count - 1 ) || ( stFile.u16file_Count == 0 ) )
				u16Current_Fileindex = 0;
			else
				u16Current_Fileindex++;

			u16File_Focus = get_focus_line(&u16Current_FilePage, u16Current_Fileindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_FilePage != u16Current_FilePage )
				MV_Draw_File_List(hdc);
			else
				MV_Draw_FileListBar(hdc, u16File_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

        case CSAPP_KEY_UP:
			hdc = BeginPaint(hwnd);
			u16Prev_FilePage = u16Current_FilePage;
			MV_Draw_FileListBar(hdc, u16File_Focus, UNFOCUS);

			if ( stFile.u16file_Count == 0 )
				u16Current_Fileindex = 0;
			else if ( u16Current_Fileindex <= 0 )
				u16Current_Fileindex = stFile.u16file_Count - 1;
			else
				u16Current_Fileindex--;

			u16File_Focus = get_focus_line(&u16Current_FilePage, u16Current_Fileindex, FILE_LIST_MAX_ITEM);

			if ( u16Prev_FilePage != u16Current_FilePage )
				MV_Draw_File_List(hdc);
			else
				MV_Draw_FileListBar(hdc, u16File_Focus, FOCUS);

			EndPaint(hwnd,hdc);
			break;

		case CSAPP_KEY_RIGHT:
			{
				U8	current_page, total_page;

				if(stFile.u16file_Count == 0)
					break;

				current_page = getpage((u16Current_Fileindex+1), FILE_LIST_MAX_ITEM);
				total_page = getpage(stFile.u16file_Count, FILE_LIST_MAX_ITEM);

				u16Current_Fileindex += FILE_LIST_MAX_ITEM;
				if(u16Current_Fileindex > stFile.u16file_Count-1)
				{
					if(current_page < total_page)
						u16Current_Fileindex = stFile.u16file_Count-1;
					else
						u16Current_Fileindex = 0;
				}

				u16File_Focus = get_focus_line(&u16Current_FilePage, u16Current_Fileindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_File_List(hdc);
				EndPaint(hwnd,hdc);
			}
			break;

		case CSAPP_KEY_LEFT:
			{
				if(stFile.u16file_Count == 0)
					break;

				if(u16Current_Fileindex < FILE_LIST_MAX_ITEM)
				{
					u16Current_Fileindex = (getpage(stFile.u16file_Count, FILE_LIST_MAX_ITEM)+1)*FILE_LIST_MAX_ITEM + u16Current_Fileindex-FILE_LIST_MAX_ITEM;
				}
				else
					u16Current_Fileindex -= FILE_LIST_MAX_ITEM;

				if(u16Current_Fileindex > stFile.u16file_Count-1)
					u16Current_Fileindex = stFile.u16file_Count-1;

				u16File_Focus = get_focus_line(&u16Current_FilePage, u16Current_Fileindex, FILE_LIST_MAX_ITEM);

				hdc = BeginPaint(hwnd);
				MV_Draw_File_List(hdc);
				EndPaint(hwnd,hdc);
			}
        	break;

        case CSAPP_KEY_ENTER:
			{
				struct stat		statbuffer;
				char			Temp_str[FILE_MAX_NAME_LENGTH];

				memset(Temp_str, 0x00, FILE_MAX_NAME_LENGTH );

				sprintf(Temp_str, "%s/%s", acCurrent_Dir, stFile.acFileName[u16Current_Fileindex]);

				if( stat(Temp_str, &statbuffer ) != 0 )
				{
					MV_File_Set_Enter(FALSE, Temp_str, stFile.acFileExt[u16Current_Fileindex]);
					break;
				}

				if( S_ISDIR(statbuffer.st_mode) )
				{
					MV_File_Set_Enter(FALSE, Temp_str, stFile.acFileExt[u16Current_Fileindex]);

					if ( strcmp( stFile.acFileName[u16Current_Fileindex] , "..") == 0 )
					{
						MV_File_Under_Dir();
						MV_File_Load_Data(acCurrent_Dir);
					} else {
						strncpy(acCurrent_Dir, Temp_str, strlen(Temp_str));
						MV_File_Load_Data(acCurrent_Dir);
					}

					u16Current_Fileindex = 0;
					u16Current_FilePage = 0;
					u16File_Focus = 0;

					hdc = BeginPaint(hwnd);
					MV_Draw_File_List(hdc);
					EndPaint(hwnd,hdc);
				} else {
					MV_File_Set_Enter(TRUE, Temp_str, stFile.acFileExt[u16Current_Fileindex]);
					printf("FILE STAT OK========================\n");
				}
			}
			break;

        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			hdc = BeginPaint(hwnd);
			MV_Close_File_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

void MV_Draw_Box(HDC hdc, RECT *rRect, gal_pixel Line_color, UINT nFormat)
{
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(rRect->left), ScalerHeigthPixel(rRect->top), ScalerWidthPixel(rRect->right - rRect->left), ScalerHeigthPixel(rRect->bottom - rRect->top) );

	MV_SetBrushColor( hdc, Line_color );

	if ( nFormat & DR_TOP )
	{
		//printf("===== TOP %x ===>>>>>\n", nFormat);
		MV_FillBox( hdc, ScalerWidthPixel(rRect->left), ScalerHeigthPixel(rRect->top), ScalerWidthPixel(rRect->right - rRect->left), ScalerHeigthPixel(4) );
	}

	if ( nFormat & DR_BOTTOM )
	{
		//printf("===== BOTTOM %x ===>>>>>\n", nFormat);
		MV_FillBox( hdc, ScalerWidthPixel(rRect->left), ScalerHeigthPixel(rRect->bottom - 4), ScalerWidthPixel(rRect->right - rRect->left), ScalerHeigthPixel(4) );
	}

	if ( nFormat & DR_LEFT )
	{
		//printf("===== LEFT %x ===>>>>>\n", nFormat);
		MV_FillBox( hdc, ScalerWidthPixel(rRect->left), ScalerHeigthPixel(rRect->top), ScalerWidthPixel(4), ScalerHeigthPixel(rRect->bottom - rRect->top) );
	}

	if ( nFormat & DR_RIGHT )
	{
		//printf("===== RIGHT %x ===>>>>>\n", nFormat);
		MV_FillBox( hdc, ScalerWidthPixel(rRect->right - 4), ScalerHeigthPixel(rRect->top), ScalerWidthPixel(rRect->right - rRect->left), ScalerHeigthPixel(4) );
	}
}

void Show_Disc_Animation(HWND hwnd)
{
	static U8		Disc_Animation_Count;
	HDC				hdc;
	U32				u32Ani_X = (WARNING_WINDOW_X + ( WARNING_WINDOW_DX - MV_BMP[MVBMP_ANI_DISC1].bmWidth )/2);
	U32				u32Ani_Y = (( WARNING_WINDOW_CONTENT_Y + WARNING_WINDOW_CONTENT_DY - 20 ) - MV_BMP[MVBMP_ANI_DISC1].bmHeight);

	hdc = BeginPaint(hwnd);

	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(u32Ani_X), ScalerHeigthPixel(u32Ani_Y), ScalerWidthPixel(MV_BMP[MVBMP_ANI_DISC1].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_ANI_DISC1].bmHeight) );

	switch(Disc_Animation_Count)
	{
		case 0:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( u32Ani_X ), ScalerHeigthPixel( u32Ani_Y ), ScalerWidthPixel( MV_BMP[MVBMP_ANI_DISC1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_ANI_DISC1].bmHeight ), &MV_BMP[MVBMP_ANI_DISC1]);
			Disc_Animation_Count = 1;
			break;
		case 1:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( u32Ani_X ), ScalerHeigthPixel( u32Ani_Y ), ScalerWidthPixel( MV_BMP[MVBMP_ANI_DISC1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_ANI_DISC1].bmHeight ), &MV_BMP[MVBMP_ANI_DISC2]);
			Disc_Animation_Count = 2;
			break;
		case 2:
			FillBoxWithBitmap(hdc,ScalerWidthPixel( u32Ani_X ), ScalerHeigthPixel( u32Ani_Y ), ScalerWidthPixel( MV_BMP[MVBMP_ANI_DISC1].bmWidth ), ScalerHeigthPixel( MV_BMP[MVBMP_ANI_DISC1].bmHeight ), &MV_BMP[MVBMP_ANI_DISC3]);
			Disc_Animation_Count = 0;
			break;
		default:
			break;
	}
	EndPaint(hwnd,hdc);
}

void *Disk_Ani_Task( void *param )
{
	HWND	hwnd;

	hwnd = GetActiveWindow();

	while(1)
	{
		Show_Disc_Animation(hwnd);
		usleep( 500*1000 );
	}
	return ( param );
}

int Disc_Ani_Init(void)
{
	pthread_create( &hAni_TaskHandle, NULL, Disk_Ani_Task, NULL );
	return( 0 );
}

void Disc_Ani_Stop(void)
{
	pthread_cancel( hAni_TaskHandle );
}

U8		u8Main_Focus_Item;
BOOL	b8Main_Focus_State;

void *Main_Ani_Task( void *param )
{
	HDC		hdc;
	HWND	hwnd;
	int		i, j = 0;
	int		width_dx = BMP_MENU_X;

	hwnd = GetActiveWindow();

	for ( i = 0 ; i < u8Main_Focus_Item ; i++ )
		width_dx += mv_main_bmp_focus[0][0].bmWidth + BMP_MENU_GAP;

//	while( j < 2 )
	{
		for ( i = 0 ; i < MAIN_ANI_MAX ; i++ )
		{
			if ( b8Main_Focus_State == TRUE )
			{
				hdc = MV_BeginPaint(hwnd);
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[u8Main_Focus_Item][i].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[u8Main_Focus_Item][i].bmHeight), &mv_main_bmp_focus[u8Main_Focus_Item][i]);
				MV_EndPaint (hwnd, hdc);
				usleep( 100 * 1000 );
			}
		}

		for ( i = MAIN_ANI_MAX - 1 ; i <= 0 ; i-- )
		{
			if ( b8Main_Focus_State == TRUE )
			{
				hdc = MV_BeginPaint(hwnd);
				MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[u8Main_Focus_Item][i].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[u8Main_Focus_Item][i].bmHeight), &mv_main_bmp_focus[u8Main_Focus_Item][i]);
				MV_EndPaint (hwnd, hdc);
				usleep( 100 * 1000 );
			}
		}
		j++;
	}
	return ( param );
}

int MainMenu_Ani_Init(U8 MainMenu_Focus_Item)
{
	b8Main_Focus_State = TRUE;
	u8Main_Focus_Item = MainMenu_Focus_Item;
	pthread_create( &hAni_TaskHandle, NULL, Main_Ani_Task, NULL );
	return( 0 );
}

void MainMenu_Ani_Stop(void)
{
	b8Main_Focus_State = FALSE;
	pthread_cancel( hAni_TaskHandle );
}

MV_File_Return MV_Load_TS_fileData(stFile_db *stFileDB)
{
	DIR				*pDir = NULL;
	struct dirent 	*pDirEnt;

	if( (pDir=opendir( RECfile )) == NULL )
	{
		closedir( pDir );
		printf( "[UsbCon_Mount]:GetUsbDevFileName(): Failed to open dir.\n" );
		return FILE_READ_FAIL;
	}

	while( (pDirEnt=readdir( pDir )) != NULL )
	{
		if( (strcmp( pDirEnt->d_name, "." ) != 0 && strcmp( pDirEnt->d_name, "..") != 0 )
			&& (( pDirEnt->d_name[strlen(pDirEnt->d_name) - 2] == 't' ) && ( pDirEnt->d_name[strlen(pDirEnt->d_name) - 1] == 's' )) )
		{
			strcpy(stFileDB->acFileName[stFileDB->u16file_Count], pDirEnt->d_name);
			stFileDB->u16file_Count++;
		}
	}
	closedir( pDir );
	return FILE_OK;
}

U16 Timer_Modify_Item[MV_TIMER_ITEM_MAX] = {
									CSAPP_STR_NUMBER,
									CSAPP_STR_ENAVLE,
									CSAPP_STR_TYPE,
									CSAPP_STR_TERM,
									CSAPP_STR_CH_NAME,
									CSAPP_STR_START,
									CSAPP_STR_TIME,
									CSAPP_STR_DURATION,
									CSAPP_STR_SAVE
								};

U8 u8Timer_Arrow[MV_TIMER_ITEM_MAX] = {
									MV_STATIC,
									MV_SELECT,
									MV_SELECT,
									MV_SELECT,
									MV_STATIC,
									MV_SELECT,
									MV_NUMERIC,
									MV_NUMERIC,
									MV_STATIC
								};

static RECT			Timer_Clock_Rect;
static DWORD		Timer_Clock_Back;
static DWORD		Timer_Clock_Font;
static U16			u16Temp_MJD;
static U16			u16Temp_UTC;
static U16			u16Temp_Duration_UTC;
static tCS_DT_Date	tTemp_YMD;
static tCS_DT_Time	tTemp_HM;
static U8			u8Timer_InCnt;
static char			acStartHour[3];
static char			acStartMin[3];
static char			acDurationHour[3];
static char			acDurationMin[3];

void *Timer_Clock_Task( void *param )
{
	HDC		hdc;
	HWND	hwnd;
	char 	Temp_Str[64];

	memset(Temp_Str, 0x00, 64);

	hwnd = GetActiveWindow();

	while( 1 )
	{
		hdc = MV_BeginPaint(hwnd);
		MV_SetBrushColor( hdc, Timer_Clock_Back );
		MV_FillBox( hdc, ScalerWidthPixel(Timer_Clock_Rect.left),ScalerHeigthPixel(Timer_Clock_Rect.top), ScalerWidthPixel(Timer_Clock_Rect.right-Timer_Clock_Rect.left),ScalerHeigthPixel(Timer_Clock_Rect.bottom - Timer_Clock_Rect.top) );

		MV_OS_Get_Time_Offset(Temp_Str, TRUE);

		SetTextColor(hdc, Timer_Clock_Font);
		SetBkMode(hdc, BM_TRANSPARENT);
		CS_MW_DrawText(hdc, Temp_Str, -1, &Timer_Clock_Rect, DT_CENTER);

		MV_EndPaint (hwnd, hdc);
		usleep( 1000 * 1000 );
	}
	return ( param );
}

int Timer_Clock_Init(RECT *Dr_Rect, DWORD Clock_Back, DWORD Color_Font)
{
	Timer_Clock_Rect = *Dr_Rect;
	Timer_Clock_Back = Clock_Back;
	Timer_Clock_Font = Color_Font;
	pthread_create( &hAni_TaskHandle, NULL, Timer_Clock_Task, NULL );
	return( 0 );
}

void Timer_Clock_Stop(void)
{
	pthread_cancel( hAni_TaskHandle );
}

/***************************************************************************/
/************************ Motor Moving Massage *****************************/
/***************************************************************************/

static U32 					u32Moving_Time;
static pthread_t  			hMotor_Moving_TaskHandle;
static BOOL					b8Moving_Window_Flag = FALSE;

BOOL Motor_Moving_State(void)
{
	return b8Moving_Window_Flag;
}

void Draw_Motor_Moving_Massage(HDC hdc)
{
	RECT	box_Rect;
	int     width;
	int     height;

	height = MV_INSTALL_MENU_BAR_H + 8;
	width  = 250;

	box_Rect.left		= MV_MENU_BACK_X + MV_MENU_BACK_DX - width - 10;
	box_Rect.right		= box_Rect.left + width;
	box_Rect.top 		= MV_MENU_BACK_Y - 60;
	box_Rect.bottom		= box_Rect.top + height;

	memset(&btFile_cap, 0x00, sizeof(BITMAP));
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(box_Rect.left), ScalerHeigthPixel(box_Rect.top), ScalerWidthPixel(width), ScalerHeigthPixel(height), &btFile_cap);

	SetBrushColor(hdc, MVAPP_GRAY_COLOR);
	FillBox(hdc,ScalerWidthPixel(box_Rect.left), ScalerHeigthPixel(box_Rect.top), ScalerWidthPixel(width), ScalerHeigthPixel(height));

	box_Rect.left = box_Rect.left + 4;
	box_Rect.top  = box_Rect.top  + 4;
	width  -= 8;
	height -= 8;
	box_Rect.right	= box_Rect.left + width;
	box_Rect.bottom	= box_Rect.top + height;

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(box_Rect.left), ScalerHeigthPixel(box_Rect.top), ScalerWidthPixel(width), ScalerHeigthPixel(height));

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	/*
	lrect.left = 806;
	lrect.right = lrect.left + 190;
	lrect.top = 72;
	lrect.bottom = lrect.top + 30;
	*/
	CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_MOVE_DISH), -1, &box_Rect, DT_CENTER);
}

void Clear_Motor_Moving_Massage(HDC hdc)
{
	RECT	box_Rect;
	int     width;
	int     height;

	if ( btFile_cap.bmHeight == 0 )
	{
		return;
	}

	height = MV_INSTALL_MENU_BAR_H + 8;
	width  = 250;

	box_Rect.left		= MV_MENU_BACK_X + MV_MENU_BACK_DX - width - 10;
	box_Rect.right		= box_Rect.left + width;
	box_Rect.top 		= MV_MENU_BACK_Y - 60;
	box_Rect.bottom		= box_Rect.top + height;

	FillBoxWithBitmap(hdc, ScalerWidthPixel(box_Rect.left), ScalerHeigthPixel(box_Rect.top), ScalerWidthPixel(width), ScalerHeigthPixel(height), &btFile_cap);
	UnloadBitmap (&btFile_cap);
}

void Motor_Moving_Stop(void)
{
	HDC		hdc;
	HWND	hwnd;

	if ((btFile_cap.bmHeight > 0 ) && b8Moving_Window_Flag)
	{
		hwnd = GetActiveWindow();

		hdc = MV_BeginPaint(hwnd);
		Clear_Motor_Moving_Massage(hdc);
		MV_EndPaint (hwnd, hdc);
		pthread_cancel( hMotor_Moving_TaskHandle );
	}

	b8Moving_Window_Flag = FALSE;
}

void *Motor_Moving_Task( void *param )
{
	HDC		hdc;
	HWND	hwnd;

	hwnd = GetActiveWindow();

	b8Moving_Window_Flag = TRUE;

	hdc = MV_BeginPaint(hwnd);
	Draw_Motor_Moving_Massage(hdc);
	MV_EndPaint (hwnd, hdc);

#if 0
		if ( b8Moving_Window_Flag == FALSE )
		{
			hdc = MV_BeginPaint(hwnd);
			Clear_Motor_Moving_Massage(hdc);
			Motor_Moving_Stop();
			return ( param );
		}
#endif
	usleep( u32Moving_Time * 1000 );

	Motor_Moving_Stop();
	return ( param );
}

int Motor_Moving_Start (U16 u16First_Longitude, U16 u16Second_Longitude)
{
	u32Moving_Time = ( abs( u16First_Longitude - u16Second_Longitude ) * 67 );

	Motor_Moving_Stop();
	if ( u32Moving_Time > 0 )
#if 1
		pthread_create( &hMotor_Moving_TaskHandle, NULL, Motor_Moving_Task, NULL );
#else
	{
		HDC		hdc;
		HWND	hwnd;

		hwnd = GetActiveWindow();

		hdc = MV_BeginPaint(hwnd);
		b8Moving_Window_Flag = TRUE;
		MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
		MV_EndPaint (hwnd, hdc);

		usleep( u16Moving_Time * 1000 );

		hdc = MV_BeginPaint(hwnd);
		Close_Msg_Window(hdc);
		b8Moving_Window_Flag = FALSE;
		MV_EndPaint(hwnd,hdc);
	}
#endif
	return( 0 );
}

int Mv_MotorMovingDisplay (void)
{
	u32Moving_Time = 3000;

	Motor_Moving_Stop();
	if ( u32Moving_Time > 0 )
#if 1
		pthread_create( &hMotor_Moving_TaskHandle, NULL, Motor_Moving_Task, NULL );
#else
	{
		HDC		hdc;
		HWND	hwnd;

		hwnd = GetActiveWindow();

		hdc = MV_BeginPaint(hwnd);
		b8Moving_Window_Flag = TRUE;
		MV_Draw_Msg_Window(hdc, CSAPP_STR_WAIT);
		MV_EndPaint (hwnd, hdc);

		usleep( u16Moving_Time * 1000 );

		hdc = MV_BeginPaint(hwnd);
		Close_Msg_Window(hdc);
		b8Moving_Window_Flag = FALSE;
		MV_EndPaint(hwnd,hdc);
	}
#endif
	return( 0 );
}

/************************ Motor Moving Massage *****************************/

void MV_Timer_Draw_Modify_Item_SelectBar(HDC hdc, int y_gap, U8 esItem, U8 u8Focuskind)
{
	int mid_width = TIMER_MOD_ITEM_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = TIMER_MOD_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	if ( u8Focuskind == FOCUS )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_MOD_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_MOD_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	} else {
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		//MV_SetBrushColor( hdc, MVAPP_TEST_WHITE_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(TIMER_MOD_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(TIMER_MOD_ITEM_DX),ScalerHeigthPixel(TIMER_MOD_ITEM_HEIGHT) );
	}

	if ( u8Timer_Arrow[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_MOD_ITEM_CONT_X + TIMER_MOD_X_CAP), ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight), &MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_MOD_ITEM_X + TIMER_MOD_ITEM_DX - MV_BMP[MVBMP_RIGHT_ARROW].bmWidth ), ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight), &MV_BMP[MVBMP_RIGHT_ARROW]);
	}
	else if ( u8Timer_Arrow[esItem] == MV_NUMERIC )
		FillBoxWithBitmap(hdc,ScalerWidthPixel(TIMER_MOD_ITEM_X + TIMER_MOD_ITEM_DX - ( MV_BMP[MVBMP_Y_ENTER].bmWidth + 10 )), ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight), &MV_BMP[MVBMP_Y_ENTER]);

}

void MV_Timer_Draw_Modify_Item(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 						y_gap = TIMER_MOD_ITEM_Y + TIMER_MOD_ITEM_HEIGHT * esItem;
	RECT						TmpRect;
	char						temp_str[256];
	MV_stServiceInfo			ServiceData;
	tCS_DB_ServiceManageData 	item_data;
	U16 						CurrentService;

	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
	} else {
		if ( stGlobTimerJob.CS_Timer_Type != eCS_TIMER_Record && esItem == MV_TIMER_DURATION )
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
	}

	MV_Timer_Draw_Modify_Item_SelectBar(hdc, y_gap, esItem, u8Focuskind);

	memset( temp_str, 0x00, 256);

	if (esItem == MV_TIMER_SAVE)
	{
		TmpRect.left	=ScalerWidthPixel(TIMER_MOD_ITEM_X);
		TmpRect.right	=TmpRect.left + TIMER_MOD_ITEM_DX;
		TmpRect.top		=ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(TIMER_MOD_ITEM_HEIGHT);
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Timer_Modify_Item[esItem]), -1, &TmpRect, DT_CENTER);
	} else {
		TmpRect.left	=ScalerWidthPixel(TIMER_MOD_ITEM_X);
		TmpRect.right	=TmpRect.left + TIMER_MOD_ITEM_NAME_DX - 10;
		TmpRect.top		=ScalerHeigthPixel(y_gap+4);
		TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(TIMER_MOD_ITEM_HEIGHT);
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(Timer_Modify_Item[esItem]), -1, &TmpRect, DT_RIGHT);
	}

	TmpRect.left	=ScalerWidthPixel(TIMER_MOD_ITEM_CONT_X);
	TmpRect.right	=TmpRect.left + TIMER_MOD_ITEM_CONT_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(TIMER_MOD_ITEM_HEIGHT);

	switch ( esItem )
	{
		case MV_TIMER_NO:
			sprintf(temp_str, "%d", u8Modify_Number + 1);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_ENAVLE:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
			else
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ON));

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_TYPE:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
			{
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
			}
			else
			{
				switch ( stGlobTimerJob.CS_Timer_Type )
				{
					case eCS_TIMER_Wakeup:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_WAKE_UP));
						break;

					case eCS_TIMER_Sleep:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_SLEEP));
						break;

					case eCS_TIMER_Record:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_RECORD));
						break;

					default:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_WAKE_UP));
						break;
				}
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_TERM:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
			{
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
			}
			else
			{
				switch ( stGlobTimerJob.CS_Timer_Cycle )
				{
					case eCS_TIMER_Onetime:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ONCE));
						break;

					case eCS_TIMER_Everyday:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_EVERYDAY));
						break;

					case eCS_TIMER_Everyweek:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_EVERYWEEK));
						break;

					default:
						sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ONCE));
						break;
				}
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_CHANNEL:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
			{
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_NONE));
			}
			else
			{
				if ( stGlobTimerJob.CS_Wakeup_Service.Service_Index == 0xFFFF )
				{
					tCS_DB_ServiceListTriplet	ListTriplet;

					CS_DB_GetCurrentListTriplet(&ListTriplet);
					CurrentService = CS_DB_GetCurrentService_OrderIndex();
					CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);
					MV_DB_GetServiceDataByIndex(&ServiceData, item_data.Service_Index);

					if ( ListTriplet.sCS_DB_ServiceListType == eCS_DB_TV_LIST || ListTriplet.sCS_DB_ServiceListType == eCS_DB_FAV_TV_LIST || ListTriplet.sCS_DB_ServiceListType == eCS_DB_SAT_TV_LIST )
						stGlobTimerJob.CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_TV;
					else
						stGlobTimerJob.CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_RADIO;

					stGlobTimerJob.CS_Wakeup_Service.SList_Value = 0;
					//CurrentService = MV_DB_Get_ServiceAllList_Index(stGlobTimerJob.CS_Wakeup_Service.SList_Type, ServiceData.u16ChIndex);
					stGlobTimerJob.CS_Wakeup_Service.Service_Index = CurrentService;
				} else {
					CS_DB_GetCurrentList_ServiceData(&item_data, stGlobTimerJob.CS_Wakeup_Service.Service_Index);
					MV_DB_GetServiceDataByIndex(&ServiceData, item_data.Service_Index);
				}
				sprintf(temp_str, "%s", ServiceData.acServiceName);
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_DATE:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
			{
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				sprintf(temp_str, "00:00");
			}
			else
			{
				tTemp_YMD = CS_DT_MJDtoYMD(u16Temp_MJD);
				sprintf(temp_str, "%02d/%02d/%04d", tTemp_YMD.day, tTemp_YMD.month, tTemp_YMD.year);
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_TIME:
			if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
			{
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
				sprintf(temp_str, "00:00");
			}
			else
			{
				MV_Timer_Draw_Time(hdc, u8Focuskind, MV_TIMER_TIME);
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case MV_TIMER_DURATION:
			if ( stGlobTimerJob.CS_Timer_Type == eCS_TIMER_Record )
			{
				if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
				{
					SetTextColor(hdc,MVAPP_GRAY_COLOR);
					sprintf(temp_str, "00:00");
				}
				else
				{
					MV_Timer_Draw_Time(hdc, u8Focuskind, MV_TIMER_DURATION);
				}
			} else {
				SetTextColor(hdc,MVAPP_GRAY_COLOR);
			}
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		default:
			break;
	}
}

void MV_Timer_Draw_Modify_Itmes(HDC hdc, U8 eItem)
{
	int 			i;

	for ( i = 0 ; i < TIMER_MOD_MAX_ITEM ; i++ )
	{
		if ( i == eItem )
			MV_Timer_Draw_Modify_Item(hdc, FOCUS, i);
		else
			MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, i);
	}
}

void MV_Timer_Draw_Modify_Window(HWND hwnd, U16 Current_Item)
{
	HDC 	hdc;
	RECT	Temp_Rect;
	RECT	Dr_Rect;

	b8Timer_Window_Flag = TRUE;
	b8Timer_Save_Flag = FALSE;
	u8Modify_Item = MV_TIMER_ENAVLE;
	u8Modify_Number = Current_Item;
	u8Timer_InCnt = 0;
	CS_TIMER_GetJobInfo(Current_Item, &stGlobTimerJob);

	if ( stGlobTimerJob.CS_Begin_MDJ == 0xFFFF )
	{
		u16Temp_MJD = CS_DT_GetLocalMJD();
		u16Temp_UTC = CS_DT_GetLocalUTC();
		u16Temp_Duration_UTC = 0;

		tTemp_HM = CS_DT_UTCtoHM(u16Temp_UTC);
		sprintf( acStartHour, "%02d", tTemp_HM.hour );
		sprintf( acStartMin, "%02d", tTemp_HM.minute );

		sprintf( acDurationHour, "00");
		sprintf( acDurationMin, "00");
	}
	else
	{
		u16Temp_MJD = stGlobTimerJob.CS_Begin_MDJ;
		u16Temp_UTC = stGlobTimerJob.CS_Begin_UTC;

		tTemp_HM = CS_DT_UTCtoHM(u16Temp_UTC);
		sprintf( acStartHour, "%02d", tTemp_HM.hour );
		sprintf( acStartMin, "%02d", tTemp_HM.minute );

		u16Temp_Duration_UTC = stGlobTimerJob.CS_Duration_UTC;
		tTemp_HM = CS_DT_UTCtoHM(u16Temp_Duration_UTC);
		sprintf( acDurationHour, "%02d", tTemp_HM.hour );
		sprintf( acDurationMin, "%02d", tTemp_HM.minute );
	}

	hdc = MV_BeginPaint(hwnd);
	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(TIMER_MOD_X), ScalerHeigthPixel(TIMER_MOD_Y), ScalerWidthPixel(TIMER_MOD_DX), ScalerHeigthPixel(TIMER_MOD_DY), &btFile_cap);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_MOD_X), ScalerHeigthPixel(TIMER_MOD_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_MOD_X + TIMER_MOD_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_MOD_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_MOD_X), ScalerHeigthPixel(TIMER_MOD_Y + TIMER_MOD_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_MOD_X + TIMER_MOD_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(TIMER_MOD_Y + TIMER_MOD_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(TIMER_MOD_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(TIMER_MOD_Y),ScalerWidthPixel(TIMER_MOD_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(TIMER_MOD_DY));
	FillBox(hdc,ScalerWidthPixel(TIMER_MOD_X), ScalerHeigthPixel(TIMER_MOD_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(TIMER_MOD_DX),ScalerHeigthPixel(TIMER_MOD_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));

	Temp_Rect.top 	= TIMER_MOD_TITLE_Y;
	Temp_Rect.bottom	= TIMER_MOD_TITLE_Y + TIMER_MOD_ITEM_HEIGHT;
	Temp_Rect.left	= TIMER_MOD_TITLE_X;
	Temp_Rect.right	= Temp_Rect.left + TIMER_MOD_TITLE_DX;

	MV_Draw_PopUp_Title_Bar_ByName(hdc, &Temp_Rect, CSAPP_STR_TIMER);

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(TIMER_MOD_ITEM_X), ScalerHeigthPixel(TIMER_MOD_ITEM_Y), ScalerWidthPixel(TIMER_MOD_ITEM_DX), ScalerHeigthPixel(TIMER_MOD_ITEM_DY) );

	MV_Timer_Draw_Modify_Itmes(hdc, u8Modify_Item);

	Dr_Rect.top = TIMER_MOD_Y + TIMER_MOD_DY - TIMER_MOD_ITEM_HEIGHT;
	Dr_Rect.left = TIMER_MOD_X + 20;
	Dr_Rect.bottom = TIMER_MOD_Y + TIMER_MOD_DY;
	Dr_Rect.right = TIMER_MOD_X + TIMER_MOD_DX - 40;
	Timer_Clock_Init(&Dr_Rect, MVAPP_BACKBLUE_COLOR, MV_BAR_UNFOCUS_CHAR_COLOR);

	MV_EndPaint(hwnd,hdc);
}

BOOL MV_Check_Timer_Window_Status(void)
{
	return b8Timer_Window_Flag;
}

BOOL MV_Check_Timer_Save_Status(void)
{
	return b8Timer_Save_Flag;
}

void MV_Timer_Close_Window(HWND hwnd)
{
	HDC 	hdc;

	b8Timer_Window_Flag = FALSE;
	b8Timer_Save_Flag = FALSE;
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(TIMER_MOD_X), ScalerHeigthPixel(TIMER_MOD_Y), ScalerWidthPixel(TIMER_MOD_DX), ScalerHeigthPixel(TIMER_MOD_DY), &btFile_cap);
	MV_EndPaint(hwnd,hdc);
	Timer_Clock_Stop();
	UnloadBitmap (&btFile_cap);
}

BOOL MV_Timer_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;

	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:

			if ( stGlobTimerJob.CS_Timer_Status != eCS_TIMER_Disable )
			{
				u8Timer_InCnt = 0;

				hdc = BeginPaint(hwnd);
				MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, u8Modify_Item);

				if ( stGlobTimerJob.CS_Timer_Type != eCS_TIMER_Record && u8Modify_Item == MV_TIMER_TIME ) {
					u8Modify_Item = MV_TIMER_SAVE;
				} else {
					if ( u8Modify_Item == MV_TIMER_ITEM_MAX - 1 )
						u8Modify_Item = MV_TIMER_ENAVLE;
					else
						u8Modify_Item++;
				}

				MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
				EndPaint(hwnd,hdc);
			} else {
				hdc = BeginPaint(hwnd);
				MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, u8Modify_Item);

				if ( u8Modify_Item == MV_TIMER_SAVE )
					u8Modify_Item = MV_TIMER_ENAVLE;
				else
					u8Modify_Item = MV_TIMER_SAVE;

				MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
				EndPaint(hwnd,hdc);
			}
			break;

        case CSAPP_KEY_UP:
			if ( stGlobTimerJob.CS_Timer_Status != eCS_TIMER_Disable )
			{
				u8Timer_InCnt = 0;

				hdc = BeginPaint(hwnd);
				MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, u8Modify_Item);

				if ( stGlobTimerJob.CS_Timer_Type != eCS_TIMER_Record && u8Modify_Item == MV_TIMER_SAVE ) {
						u8Modify_Item = MV_TIMER_TIME;
				} else {
					if ( u8Modify_Item == MV_TIMER_ENAVLE )
						u8Modify_Item = MV_TIMER_ITEM_MAX - 1;
					else
						u8Modify_Item--;
				}

				MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
				EndPaint(hwnd,hdc);
			} else {
				hdc = BeginPaint(hwnd);
				MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, u8Modify_Item);

				if ( u8Modify_Item == MV_TIMER_SAVE )
					u8Modify_Item = MV_TIMER_ENAVLE;
				else
					u8Modify_Item = MV_TIMER_SAVE;

				MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
				EndPaint(hwnd,hdc);
			}
			break;

		case CSAPP_KEY_RIGHT:
			switch( u8Modify_Item )
			{
				case MV_TIMER_ENAVLE:
					if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
						stGlobTimerJob.CS_Timer_Status = eCS_TIMER_Enable;
					else
						stGlobTimerJob.CS_Timer_Status = eCS_TIMER_Disable;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Itmes(hdc, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TYPE:
					if ( stGlobTimerJob.CS_Timer_Type == eCS_TIMER_Record )
						stGlobTimerJob.CS_Timer_Type = eCS_TIMER_Wakeup;
					else
						stGlobTimerJob.CS_Timer_Type++;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, MV_TIMER_DURATION);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TERM:
					if ( stGlobTimerJob.CS_Timer_Cycle == eCS_TIMER_Everyweek )
						stGlobTimerJob.CS_Timer_Cycle = eCS_TIMER_Onetime;
					else
						stGlobTimerJob.CS_Timer_Cycle++;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_DATE:
					u16Temp_MJD++;
					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TIME:
					if( u8Timer_InCnt < 3 )
						u8Timer_InCnt++;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_DURATION:
					if( u8Timer_InCnt < 3 )
						u8Timer_InCnt++;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				default:
					break;
			}
			break;

		case CSAPP_KEY_LEFT:
			switch( u8Modify_Item )
			{
				case MV_TIMER_ENAVLE:
					if ( stGlobTimerJob.CS_Timer_Status == eCS_TIMER_Disable )
						stGlobTimerJob.CS_Timer_Status = eCS_TIMER_Enable;
					else
						stGlobTimerJob.CS_Timer_Status = eCS_TIMER_Disable;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Itmes(hdc, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TYPE:
					if ( stGlobTimerJob.CS_Timer_Type == eCS_TIMER_Wakeup )
						stGlobTimerJob.CS_Timer_Type = eCS_TIMER_Record;
					else
						stGlobTimerJob.CS_Timer_Type--;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					MV_Timer_Draw_Modify_Item(hdc, UNFOCUS, MV_TIMER_DURATION);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TERM:
					if ( stGlobTimerJob.CS_Timer_Cycle == eCS_TIMER_Onetime )
						stGlobTimerJob.CS_Timer_Cycle = eCS_TIMER_Everyweek;
					else
						stGlobTimerJob.CS_Timer_Cycle--;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_DATE:
					if ( CS_DT_GetLocalMJD() < u16Temp_MJD )
						u16Temp_MJD--;
					else
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Msg_Window(hdc, CSAPP_STR_INVALID_DATA);
						EndPaint(hwnd,hdc);

						usleep( 1000 * 1000 );

						hdc=BeginPaint(hwnd);
						Close_Msg_Window(hdc);
						EndPaint(hwnd,hdc);
					}

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_TIME:
					if( u8Timer_InCnt > 0 )
						u8Timer_InCnt--;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				case MV_TIMER_DURATION:
					if( u8Timer_InCnt > 0 )
						u8Timer_InCnt--;

					hdc = BeginPaint(hwnd);
					MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
					EndPaint(hwnd,hdc);
					break;

				default:
					break;
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
			if ( u8Modify_Item == MV_TIMER_TIME || u8Modify_Item == MV_TIMER_DURATION )
			{
				MV_Timer_Update_Time(u8Key);

				if ( u8Timer_InCnt < 3 )
					u8Timer_InCnt++;

				hdc = BeginPaint(hwnd);
				MV_Timer_Draw_Modify_Item(hdc, FOCUS, u8Modify_Item);
				EndPaint(hwnd,hdc);
			}
			break;

        case CSAPP_KEY_ENTER:
			if( u8Modify_Item == MV_TIMER_SAVE )
				b8Timer_Save_Flag = TRUE;
			break;

        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			return FALSE;

		case CSAPP_KEY_IDLE:
			hdc = BeginPaint(hwnd);
			MV_Timer_Close_Window(hdc);
			EndPaint(hwnd,hdc);
			return FALSE;
    }
	return TRUE;
}

void MV_Timer_Update_Time(WPARAM wparam)
{
	char	Temp;

	switch(wparam)
	{
		case CSAPP_KEY_0:
			Temp = '0';
			break;

		case CSAPP_KEY_1:
			Temp = '1';
			break;

		case CSAPP_KEY_2:
			Temp = '2';
			break;

		case CSAPP_KEY_3:
			Temp = '3';
			break;

		case CSAPP_KEY_4:
			Temp = '4';
			break;

		case CSAPP_KEY_5:
			Temp = '5';
			break;

		case CSAPP_KEY_6:
			Temp = '6';
			break;

		case CSAPP_KEY_7:
			Temp = '7';
			break;

		case CSAPP_KEY_8:
			Temp = '8';
			break;

		case CSAPP_KEY_9:
			Temp = '9';
			break;

		default:
			Temp = '0';
			break;
	}

	switch (u8Timer_InCnt)
	{
		case 0:
			if ( u8Modify_Item == MV_TIMER_TIME )
				acStartHour[0] = Temp;
			else
				acDurationHour[0] = Temp;
			break;

		case 1:
			if ( u8Modify_Item == MV_TIMER_TIME )
				acStartHour[1] = Temp;
			else
				acDurationHour[1] = Temp;
			break;

		case 2:
			if ( u8Modify_Item == MV_TIMER_TIME )
				acStartMin[0] = Temp;
			else
				acDurationMin[0] = Temp;
			break;

		case 3:
			if ( u8Modify_Item == MV_TIMER_TIME )
				acStartMin[1] = Temp;
			else
				acDurationMin[1] = Temp;
			break;

		default:
			break;
	}

	//acTemp_duration[u8Timer_InCnt] = Temp;
}

void MV_Timer_Draw_Time(HDC hdc, U8 u8Focuskind, U8 u8DrawItem)
{
	RECT		acRect;
	char		acTemp_Time[20];
	char		acTemp_Str[20];
	int			i;

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	acRect.top = TIMER_MOD_ITEM_Y + (TIMER_MOD_ITEM_HEIGHT * u8DrawItem);
	acRect.left = TIMER_MOD_ITEM_CONT_X + 40;
	acRect.bottom = acRect.top + TIMER_MOD_ITEM_HEIGHT;
	acRect.right = acRect.left + TIMER_MOD_ITEM_CONT_DX - 80;

	memset( acTemp_Time, 0x00, 20 );
	memset( acTemp_Str, 0x00, 20 );

	if ( u8DrawItem == MV_TIMER_TIME )
		sprintf(acTemp_Time, "%s%s", acStartHour, acStartMin);
	else if ( u8DrawItem == MV_TIMER_DURATION )
		sprintf(acTemp_Time, "%s%s", acDurationHour, acDurationMin);

//	FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));

	acRect.top += 4;
	acRect.left = acRect.left + ((TIMER_MOD_ITEM_CONT_DX - 80)/2) - 80;
	acRect.bottom = acRect.top + 22;
	acRect.right = acRect.left + 25;

	for ( i = 0 ; i < 4 ; i++ )
	{
		if ( i == 2 )
			acRect.left += 35;
		else
			acRect.left += 25;

		acRect.right = acRect.left + 25;
		sprintf(acTemp_Str, "%c", acTemp_Time[i]);

		if ( i == u8Timer_InCnt && u8Focuskind == FOCUS )
		{
			SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
		else
		{
			//SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			if ( u8Focuskind == FOCUS )
				SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
			else
				SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);

			SetBkMode(hdc,BM_TRANSPARENT);
			//FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
	}
//	CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
}

tCS_TIMER_Error MV_Timer_Upload_Data(void)
{
	tCS_DT_Time			st_StartHM;
	tCS_DT_Time			st_DurationHM;

	st_StartHM.hour = atoi(acStartHour);
	st_StartHM.minute = atoi(acStartMin);
	st_DurationHM.hour = atoi(acDurationHour);
	st_DurationHM.minute = atoi(acDurationMin);

	stGlobTimerJob.CS_Begin_MDJ = u16Temp_MJD;
	stGlobTimerJob.CS_Begin_UTC = CS_DT_HMtoUTC(st_StartHM);
	stGlobTimerJob.CS_Begin_Weekday = CS_DT_CalculateWeekDay(u16Temp_MJD);
	stGlobTimerJob.CS_Duration_UTC = CS_DT_HMtoUTC(st_DurationHM);

	return CS_TIMER_CheckandSaveJobInfo(&stGlobTimerJob, u8Modify_Number);
}

void MV_Draw_Menu_Signal(HDC hdc, RECT rRect)
{
	U8  				i = 0, u8Value = 0;
	TunerSignalState_t 	Siganl_State;

	TunerReadSignalState(Tuner_HandleId[0], &Siganl_State);

	u8Value = Siganl_State.Strength/20;

	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( i < u8Value )
		{
			if ( Siganl_State.Strength > 80 )
				SetBrushColor(hdc, MVAPP_LIGHT_GREEN_COLOR);
			else if ( Siganl_State.Strength > 50 )
				SetBrushColor(hdc, MVAPP_ORANGE_COLOR);
			else
				SetBrushColor(hdc, MVAPP_RED_COLOR);
		}
		else
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);

		FillBox(hdc,ScalerWidthPixel(rRect.left + (i*(6))), ScalerHeigthPixel(rRect.top + ( (4-i) * 4 )),ScalerWidthPixel(4),ScalerHeigthPixel(20 - ( (4-i) * 4 )));
	}

	u8Value = Siganl_State.Quality/20;

	for ( i = 0 ; i < 5 ; i++ )
	{
		if ( i < u8Value )
		{
			if ( Siganl_State.Quality > 80 )
				SetBrushColor(hdc, MVAPP_LIGHT_GREEN_COLOR);
			else if ( Siganl_State.Quality > 50 )
				SetBrushColor(hdc, MVAPP_ORANGE_COLOR);
			else
				SetBrushColor(hdc, MVAPP_RED_COLOR);
		}
		else
			SetBrushColor(hdc, MVAPP_GRAY_COLOR);

		FillBox(hdc,ScalerWidthPixel(( rRect.left + 40 ) + (i*(6))), ScalerHeigthPixel(rRect.top + ( (4-i) * 4 )),ScalerWidthPixel(4),ScalerHeigthPixel(20 - ( (4-i) * 4 )));
	}
}

