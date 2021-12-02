#include "linuxos.h"

#include "database.h"
#include "sys_setup.h"
#include "mwsvc.h"
#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "mvfinder.h"
#include "mv_menu_ctr.h"

#define STR_MAX_STR					50
#define UNSELECT  					1
#define SELECT						2
#define CHAR						1
#define NUM							2

#define CL_WINDOW_X					160
#define CL_WINDOW_Y					100
#define	CL_WINDOW_ITEM_DX			21
#define	CL_WINDOW_DX				426
#define CL_WINDOW_DY				510
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

#define	SERVICES_NUM_PER_PAGE		10
#define	FIELDS_PER_LINE				6

extern tComboList_Field_Rect		SList_ComboList_First_Line[FIELDS_PER_LINE];
extern tComboList_Element			SList_ComboList_First;
extern tCS_DBU_Service				back_triplet;

static CSAPP_Applet_t				CSApp_Finder_Applets;
static U8							u8Keypad_FX;
static U8							u8Keypad_FY;
static U8							u8max_str_length = 0;
static char							acReturnbuff_Temp[STR_MAX_STR];
static char							acReturnbuff[STR_MAX_STR];
static BOOL							b8keypad_save_str;
static U8							u8Keypad_Index = 0;
BOOL								b8FindKeypad_Space_Key = FALSE;
BOOL								b8FindKeypad_Ok_Key = FALSE;
BOOL								b8FindKeypad_Cancel_Key = FALSE;
BOOL								finder_keypad_enable;
BOOL								Find_List_state = FALSE;
RECT								FindRect;
U16									u16Find_Ch_List[MV_MAX_SERVICE_COUNT];
U16									u16Toatl_FindList = 0;
U16									u16Find_Current_Service = 0;
U16									u16Find_Current_Page = 0;
U16									u16Find_Prev_Page = 0;
U16									u16Find_Current_Focus = 0;
MV_stServiceInfo					Find_service_data;
tCS_DB_ServiceManageData			Find_item_data;

static int Finder_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Close_FindKeypad( HWND hwnd )
{
	HDC		hdc;

	finder_keypad_enable = FALSE;		
	hdc = MV_BeginPaint(hwnd);
	FillBoxWithBitmap(hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTDY), &Capture_bmp);
	MV_EndPaint(hwnd,hdc);
	UnloadBitmap(&Capture_bmp);
}

void Selected_FindKey(HWND hwnd, U8 u8_NO, U8 Select_Kind)
{
	U8		a[5];
	char	temp_str[50];
	HDC		hdc;

	hdc = MV_BeginPaint(hwnd);
	if ( ( ((u8Keypad_FY-1) * COL_COUNT) + u8Keypad_FX ) > 25  && ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX < 52 ) )
	{
		a[0] = 'A' + ( (u8Keypad_FY-1) * COL_COUNT ) + u8Keypad_FX + 6;
		a[1] = '\0';
	}
	else if ( ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX < 26 ))
	{
		a[0] = 'A' + ( (u8Keypad_FY-1) * COL_COUNT ) + u8Keypad_FX;
		a[1] = '\0';
	}
	else
	{
		if ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX == 52 )
		{
			a[0] = 'S';
			a[1] = 'P';
			a[2] = '\0';
		}
		
		if ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX == 54 )
		{
			a[0] = 'O';
			a[1] = 'K';
			a[2] = '\0';
		}
		
		if ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX == 56 )
		{
			a[0] = 'E';
			a[1] = 'X';
			a[2] = 'I';
			a[3] = 'T';
			a[4] = '\0';
		}
	}
	
	if ( Select_Kind == UNSELECT )
	{
		SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		
		if ( u8_NO == CHAR )  
		{
			sprintf(temp_str, "%s", a );
			if ( ( u8Keypad_FX == 2 && u8Keypad_FY == 6 ) || ( u8Keypad_FX == 4 && u8Keypad_FY == 6 ) )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else if ( u8Keypad_FX == 6 && u8Keypad_FY == 6 ) {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			}
			
		} else {
			FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
			sprintf(temp_str, "%d", u8Keypad_FX );
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2 ), temp_str);
		}
	} else {
		SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		
		if ( u8_NO == CHAR )  
		{
			sprintf(temp_str, "%s", a );
			if ( ( u8Keypad_FX == 2 && u8Keypad_FY == 6 ) || ( u8Keypad_FX == 4 && u8Keypad_FY == 6 ) )
			{
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET) + 8),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else if ( u8Keypad_FX == 6 && u8Keypad_FY == 6 ) {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			} else {
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2), temp_str);
			}
			
		} else {
			FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
			sprintf(temp_str, "%d", u8Keypad_FX );
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (u8Keypad_FX*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (u8Keypad_FY*FINDER_KEYBOARD_KEY_OFFSET)+2 ), temp_str);
		}
	}
	MV_EndPaint(hwnd,hdc);
}

void Find_Draw_keypad(HDC hdc, U8 *item_str)
{
	U16 	i,j;
	U8		a[5];
	char	temp_str[10];
	
	for ( i = 0 ; i < FIND_ROW_COUNT ; i++ )
	{
		if ( i ==  0 )
		{
			for ( j = 0 ; j < NUM_COUNT ; j++ )
			{
				SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
				sprintf(temp_str, "%d", j );
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET)+2 ), temp_str);
			}
		}
		else
		{
			for ( j = 0 ; j < COL_COUNT ; j++ )
			{
				if ( ((i-1)*COL_COUNT) + j > 25 )
					break;
				
				if ( j == 0 && i == 1 )
					FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_FOCUS_KEYPAD]);
				else
					if ( i == 4 )
					{
						FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_BIGKEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);
					}
					else
						FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel( FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET) ), ScalerWidthPixel(FINDER_KEYBOARD_KEY_SIZE),ScalerHeigthPixel(FINDER_KEYBOARD_KEY_SIZE),&MV_BMP[MVBMP_UNFOCUS_KEYPAD]);

				if ( ( ((i-1)*COL_COUNT) + j < 26 ))
				{
					a[0] = 'A' + ( (i-1) * COL_COUNT ) + j;
					a[1] = '\0';
				}
				
				sprintf(temp_str, "%s", a );

				SetTextColor(hdc,MVAPP_DARKBLUE_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_CS_MW_TextOut( hdc,ScalerWidthPixel(FINDER_KEYBOARD_CHAR_STARTX + (j*FINDER_KEYBOARD_KEY_OFFSET)),ScalerHeigthPixel(FINDER_KEYBOARD_NUM_KEY_STARTY + (i*FINDER_KEYBOARD_KEY_OFFSET)+2 ), temp_str);
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
			MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			CS_MW_DrawText(hdc, item_str, -1, &FindRect, DT_CENTER);
			//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + 4),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4), item_str);
		}
		else
		{
			if ( ReturnV[0] != '\0' )
			{
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				CS_MW_DrawText(hdc, ReturnV, -1, &FindRect, DT_CENTER);
				//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX + 4),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY + 4), ReturnV);
			}
		}
	}
}

void MV_Draw_Finder_Keypad(HWND hwnd)
{
	HDC		hdc;
	RECT	Temp_Rect;
	char	temp_str[50];
	
	u8Keypad_FX = 0;
	u8Keypad_FY = 1;
	finder_keypad_enable = TRUE;
	u8max_str_length = STR_MAX_STR;
	acReturnbuff_Temp[0] = '\0';
	b8keypad_save_str = FALSE;
	
	FindRect.left	= FINDER_KEYBOARD_NUM_KEY_STARTX;
	FindRect.right	= FINDER_KEYBOARD_NUM_KEY_STARTX + FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6;
	FindRect.top	= FINDER_KEYBOARD_STR_STARTY + 4;
	FindRect.bottom	= FINDER_KEYBOARD_STR_STARTY + MV_INSTALL_MENU_BAR_H;

	memcpy(acReturnbuff_Temp,"\0",STR_MAX_STR);
	
	hdc = MV_BeginPaint(hwnd);

	MV_GetBitmapFromDC(hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTDY), &Capture_bmp);

	FillBoxWithBitmap (hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX + FINDER_KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX + FINDER_KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);
	
	MV_SetBrushColor( hdc, MVAPP_BACKBLUE_COLOR );
	FillBox(hdc,ScalerWidthPixel(FINDER_KEYBOARD_STARTX + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY),ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(FINDER_KEYBOARD_STARTDY));
	FillBox(hdc,ScalerWidthPixel(FINDER_KEYBOARD_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(FINDER_KEYBOARD_STARTDX),ScalerHeigthPixel(FINDER_KEYBOARD_STARTDY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	
	
	MV_SetBrushColor( hdc, MVAPP_RED_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX + WINDOW_OUT_GAP), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + WINDOW_OUT_GAP), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*2), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	Temp_Rect.top 	= FINDER_KEYBOARD_STARTY + WINDOW_OUT_GAP + 2;
	Temp_Rect.bottom	= Temp_Rect.top + MV_INSTALL_MENU_BAR_H;
	Temp_Rect.left	= FINDER_KEYBOARD_STARTX + WINDOW_OUT_GAP;
	Temp_Rect.right	= Temp_Rect.left + FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*2;
	
	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	sprintf(temp_str, "%s", CS_MW_LoadStringByIdx(CSAPP_STR_KEY_BOARD));
	CS_MW_DrawText(hdc, temp_str, -1, &Temp_Rect, DT_CENTER);	

	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_STARTX + WINDOW_OUT_GAP), ScalerHeigthPixel(WINDOW_ITEM_Y), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*2), ScalerHeigthPixel(WINDOW_ITEM_DY - 120) );

	MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
	
	Find_Draw_keypad(hdc, acReturnbuff_Temp);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - 60), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - 60),	CS_MW_LoadStringByIdx(CSAPP_STR_SPACE));
	
	FillBoxWithBitmap(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_KEY_PREV_ICON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(FINDER_KEYBOARD_STARTY + FINDER_KEYBOARD_STARTDY - 30),	CS_MW_LoadStringByIdx(CSAPP_STR_DELETE_KEY));
	
	MV_EndPaint(hwnd,hdc);
}

U8 FindPress_Key(void)
{
	U8	a=0;
#if 0
	U8 	temp_str[3];
#endif
	if ( u8Keypad_FY == 0 )
	{
#if 0
		sprintf(temp_str, "%d",(U8)u8Keypad_FX);
		a = temp_str[0];
#else
		a = '0' + u8Keypad_FX;
#endif
	} else if ( ((u8Keypad_FY-1)*COL_COUNT) + u8Keypad_FX < 26 )
	{
		a = 'A' + ( (u8Keypad_FY-1) * COL_COUNT ) + u8Keypad_FX;
	}	

	return a;
}

BOOL Find_Keypad_Proc(HWND hwnd, WPARAM u8Key)
{
	HDC		hdc;
	
	switch (u8Key)
    {
        case CSAPP_KEY_DOWN:
        	if ( u8Keypad_FY == 2 )
        	{
        		switch(u8Keypad_FX)
        		{
	        		case 6:
					case 7:
					case 8:
					case 9:
	        			Selected_FindKey(hwnd, CHAR, UNSELECT);
	        			u8Keypad_FY = 0;
	        			Selected_FindKey(hwnd, NUM, SELECT);
		        		break;
	        		
	        		default:
	        			Selected_FindKey(hwnd, CHAR, UNSELECT);
	        			u8Keypad_FY += 1;
	        			Selected_FindKey(hwnd, CHAR, SELECT);
		        		break;
	        	}
        	}
			else if (u8Keypad_FY == 3 )
			{
				Selected_FindKey(hwnd, CHAR, UNSELECT);
    			u8Keypad_FY = 0;
    			Selected_FindKey(hwnd, NUM, SELECT);
			}
        	else if (u8Keypad_FY == 0 )
        	{
				Selected_FindKey(hwnd, NUM, UNSELECT);
    			u8Keypad_FY = 1;
    			Selected_FindKey(hwnd, CHAR, SELECT);
        	}
			else
        	{
        		Selected_FindKey(hwnd, CHAR, UNSELECT);
        		u8Keypad_FY += 1;
        		Selected_FindKey(hwnd, CHAR, SELECT);
        	}
			//printf("\n====== X : %d , Y : %d =====\n", u8Keypad_FX, u8Keypad_FY);
        	break;
        case CSAPP_KEY_UP:
        	if ( u8Keypad_FY == 0 )
        	{
        		switch(u8Keypad_FX)
        		{	
        			case 6:
        			case 7:
					case 8:
					case 9:
        				Selected_FindKey(hwnd, NUM, UNSELECT);
        				u8Keypad_FY = 2;
						Selected_FindKey(hwnd, CHAR, SELECT);
        			break;
        			
        			default:
        				Selected_FindKey(hwnd, NUM, UNSELECT);
						u8Keypad_FY = 3;
						Selected_FindKey(hwnd, CHAR, SELECT);
        				break;
        		}
        	}
			else if ( u8Keypad_FY == 1 )
			{
				Selected_FindKey(hwnd, CHAR, UNSELECT);
				u8Keypad_FY = 0;
				Selected_FindKey(hwnd, NUM, SELECT);
			}
        	else
        	{
        		Selected_FindKey(hwnd, CHAR, UNSELECT);
        		u8Keypad_FY -= 1;
        		Selected_FindKey(hwnd, CHAR, SELECT);
        	}
        	break;
        case CSAPP_KEY_LEFT:
        	if ( u8Keypad_FY == 0 )
        	{
        		if ( u8Keypad_FX == 0 )
        		{
        			Selected_FindKey(hwnd, NUM, UNSELECT);
        			u8Keypad_FX = 5;
        			u8Keypad_FY = 3;
        			Selected_FindKey(hwnd, CHAR, SELECT);
        		} else {
        			Selected_FindKey(hwnd, NUM, UNSELECT);
        			u8Keypad_FX -= 1;
        			Selected_FindKey(hwnd, NUM, SELECT);
        		}
        	}
        	else
        	{
        		if ( u8Keypad_FX == 0 )
        		{
        			if ( u8Keypad_FY == 1 )
        			{
        				Selected_FindKey(hwnd, CHAR, UNSELECT);
        				u8Keypad_FX = 9;
        				u8Keypad_FY -= 1;
        				Selected_FindKey(hwnd, NUM, SELECT);
        			} else {
        				Selected_FindKey(hwnd, CHAR, UNSELECT);
        				u8Keypad_FX = 9;
        				u8Keypad_FY -= 1;
        				Selected_FindKey(hwnd, CHAR, SELECT);
        			}
        		} else {
        			Selected_FindKey(hwnd, CHAR, UNSELECT);
        			u8Keypad_FX -= 1;
        			Selected_FindKey(hwnd, CHAR, SELECT);
        		}
        	}
        	break;
        case CSAPP_KEY_RIGHT:
        	if ( u8Keypad_FY == 3 )
        	{
        		if ( u8Keypad_FX == 5 )
        		{
        			Selected_FindKey(hwnd, CHAR, UNSELECT);
        			u8Keypad_FX = 0;
        			u8Keypad_FY = 0;
        			Selected_FindKey(hwnd, NUM, SELECT);
        		} else {
        			Selected_FindKey(hwnd, CHAR, UNSELECT);
        			u8Keypad_FX += 1;
        			Selected_FindKey(hwnd, CHAR, SELECT);
        		}
        	}
        	else
        	{
        		if ( u8Keypad_FY == 0 )
        		{
        			if ( u8Keypad_FX == 9 )
	        		{
	        			Selected_FindKey(hwnd, NUM, UNSELECT);
	        			u8Keypad_FX = 0;
	        			u8Keypad_FY += 1;
	        			Selected_FindKey(hwnd, CHAR, SELECT);
	        		} else {
	        			Selected_FindKey(hwnd, NUM, UNSELECT);
	        			u8Keypad_FX += 1;
	        			Selected_FindKey(hwnd, NUM, SELECT);
	        		}
        		} else {
	        		if ( u8Keypad_FX == 9 )
	        		{
	        			Selected_FindKey(hwnd, CHAR, UNSELECT);
	        			u8Keypad_FX = 0;
	        			u8Keypad_FY += 1;
	        			Selected_FindKey(hwnd, CHAR, SELECT);
	        		} else {
	        			Selected_FindKey(hwnd, CHAR, UNSELECT);
	        			u8Keypad_FX += 1;
	        			Selected_FindKey(hwnd, CHAR, SELECT);
	        		}
	        	}
        	}
        	break;
        case CSAPP_KEY_ENTER:
			if ( u8Keypad_Index < u8max_str_length )
			{
				acReturnbuff_Temp[u8Keypad_Index] = FindPress_Key();
				//printf("\n=== %s =====\n", acReturnbuff_Temp);
				acReturnbuff_Temp[u8Keypad_Index+1] = '\0';
				hdc = MV_BeginPaint(hwnd);
				SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				CS_MW_DrawText(hdc, acReturnbuff_Temp, -1, &FindRect, DT_CENTER);
				MV_EndPaint(hwnd,hdc);
				u8Keypad_Index++;
			}
			else
			{
				FindPress_Key();
				if ( b8FindKeypad_Ok_Key == TRUE )
				{
					b8FindKeypad_Ok_Key = FALSE;
					b8keypad_save_str = TRUE;	
					MV_Close_Keypad( hwnd );
					return FALSE;				
				}
				else if ( b8FindKeypad_Cancel_Key == TRUE )
				{
					acReturnbuff_Temp[0] = '\0';
					b8FindKeypad_Cancel_Key = FALSE;
					b8keypad_save_str = FALSE;
					MV_Close_Keypad( hwnd );
					return FALSE;
				}
			}
        	break;
        case CSAPP_KEY_SWAP:
        	if ( u8Keypad_Index > 0 && u8Keypad_Index <= u8max_str_length )
        	{
	    		acReturnbuff_Temp[u8Keypad_Index-1] = '\0';
				strcpy(acReturnbuff, acReturnbuff_Temp);

				if((u8Keypad_Index-2)>=0)			
		    		acReturnbuff[u8Keypad_Index-2] = '\0';

				hdc = MV_BeginPaint(hwnd);
	        	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
				SetBkMode(hdc,BM_TRANSPARENT);
				MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
				MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
				CS_MW_DrawText(hdc, acReturnbuff_Temp, -1, &FindRect, DT_CENTER);
				MV_EndPaint(hwnd,hdc);
	        	u8Keypad_Index--;
	        }
        	break;
		case CSAPP_KEY_RED:
			acReturnbuff_Temp[u8Keypad_Index] = ' ';
			acReturnbuff_Temp[u8Keypad_Index+1] = '\0';
			hdc = MV_BeginPaint(hwnd);
			SetTextColor(hdc,MVAPP_YELLOW_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(FINDER_KEYBOARD_NUM_KEY_STARTX), ScalerHeigthPixel(FINDER_KEYBOARD_STR_STARTY ), ScalerWidthPixel(FINDER_KEYBOARD_STARTDX - WINDOW_OUT_GAP*6), ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
			CS_MW_DrawText(hdc, acReturnbuff_Temp, -1, &FindRect, DT_CENTER);
			//MV_CS_MW_TextOut( hdc,ScalerWidthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTX),ScalerHeigthPixel(KEYBOARD_INPUT_WINDOW_STRING_STARTY), Returnbuff_Temp);
			MV_EndPaint(hwnd,hdc);
			u8Keypad_Index++;
			break;
        case CSAPP_KEY_ESC:
        case CSAPP_KEY_MENU:
			b8keypad_save_str = FALSE;
			MV_Close_FindKeypad(hwnd);
			return FALSE;
            break;
    }
	return TRUE;
}

void MV_Draw_Find_ChName(HDC hdc, RECT *rRect, char *ServiceName)
{
#ifndef _TTF_SUPPORT
	int 				j, k, l;
	U8					u8Find_len;
	U8					u8point;
	char				Temp_Str[MAX_SERVICE_NAME_LENGTH];
	char				*strTemp_Str;

	memset( &Temp_Str, 0x00, MAX_SERVICE_NAME_LENGTH);
	//printf("1.%s ========= MV_Draw_Find_List \n", ServiceName);
#endif

	SetTextColor(hdc,CSAPP_WHITE_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, ServiceName, -1, rRect, DT_LEFT);
	
#ifndef _TTF_SUPPORT
	strTemp_Str = strstr(ServiceName, acReturnbuff_Temp);
	u8point = strlen(ServiceName) - strlen(strTemp_Str);

	for ( j = 0 ; j < u8point ; j++ )
		Temp_Str[j] = ' ';
	
	u8Find_len = strlen(acReturnbuff_Temp);

	l = 0;
	for ( k = j ; k < j + u8Find_len ; k++ )
		Temp_Str[k] = strTemp_Str[l++];
	
	SetTextColor(hdc,MVAPP_YELLOW_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	CS_MW_DrawText(hdc, Temp_Str, -1, rRect, DT_LEFT);
#endif
	//printf("2.%s ========= \n", Temp_Str);
}

void MV_Draw_Find_Title(HDC hdc)
{
	RECT 			TmpRect;

	if ( Find_List_state == FALSE )
	{	
		Find_List_state = TRUE;
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);

		TmpRect.left	= ScalerWidthPixel(CL_WINDOW_X) + 10;
		TmpRect.right	= TmpRect.left + ScalerWidthPixel(CL_WINDOW_DX) - 20;
		TmpRect.top		= ScalerWidthPixel(CL_WINDOW_TITLE_Y - 10) + 4;
		TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(CL_WINDOW_ITEM_DY);

		MV_Draw_PopUp_Title_Bar_ByName(hdc, &TmpRect, CSAPP_STR_FIND);
	}
}

void MV_Draw_FindList_Item(HDC hdc, int Count_index, U16 u16Focusindex, U8 FocusKind)
{
	char	buff1[50];
	RECT	rc_service_name;
	RECT 	Scroll_Rect;
	/* By KB Kim 2011.01.20 */
	U8      tvRadio;

	memset(buff1, 0, 50);
	MV_DB_GetServiceDataByIndex(&Find_service_data, u16Find_Ch_List[u16Focusindex]);

	//printf("%04d : %d : %d : %s ======\n", u16Focusindex, chlist_item_data.Service_Index, Find_service_data.u16ChIndex, Find_service_data.acServiceName);

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
#ifdef DEBUG_TEST
		//printf(" %d === %d ========= 1 ========== %d \n", u16Focusindex, chlist_item_data.Service_Index, u32Count_Loop);
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
		//CS_MW_DrawText(hdc, Find_service_data.acServiceName, -1, &rc_service_name, DT_LEFT);
		MV_Draw_Find_ChName(hdc, &rc_service_name, Find_service_data.acServiceName);

		if(Find_service_data.u8Scramble)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[2].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[2].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_SCRAMBLE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[2].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[2].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NSCRAMBLE_ICON]);
		}

		if(Find_service_data.u8Lock)
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[3].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[3].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_LOCK_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[3].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[3].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NLOCK_ICON]);
		}

		/* By KB Kim 2011.01.20 */
		if (Find_service_data.u8TvRadio == eCS_DB_RADIO_SERVICE)
		{
			tvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
		}
		else
		{
			tvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
		}
		if( MV_DB_FindFavoriteServiceBySrvIndex(tvRadio, Find_service_data.u16ChIndex) < MV_MAX_FAV_KIND )
		{
			if (  FocusKind == FOCUS )
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[4].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[4].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_FAVORITE_ICON]);
			else
				FillBoxWithBitmap (hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[4].x), ScalerHeigthPixel(SList_ComboList_First.element.y + SList_ComboList_First.element.dy * Count_index), ScalerWidthPixel(SList_ComboList_First.element_fields[4].dx), ScalerHeigthPixel(SList_ComboList_First.element.dy), &MV_BMP[MVBMP_CHLIST_NFAVORITE_ICON]);
		}

		if( Find_service_data.u8TvRadio == eCS_DB_HDTV_SERVICE )
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
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u16Find_Current_Service, u16Toatl_FindList, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_Find_List_Window(HDC hdc)
{
	if ( Find_List_state == FALSE )
	{
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight - 30), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y + CL_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight - 30), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

		SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_DY - 30));
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X), ScalerHeigthPixel(CL_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(CL_WINDOW_DX),ScalerHeigthPixel(CL_WINDOW_DY - 30 - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(CL_WINDOW_TITLE_Y),ScalerWidthPixel(CL_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY));
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	

		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_ICON_X), ScalerHeigthPixel(CL_WINDOW_ICON_Y), ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
		CS_MW_TextOut(hdc, ScalerWidthPixel(CL_WINDOW_ICON_X)+ScalerWidthPixel(MV_BMP[MVBMP_BLUE_BUTTON].bmWidth)+6, ScalerHeigthPixel(CL_WINDOW_ICON_Y), CS_MW_LoadStringByIdx(CSAPP_STR_FIND));
	}
}

void MV_Draw_Find_Full_Item(HDC hdc)
{
	int 	i;
	U16		index;
	RECT 	Scroll_Rect;

	for(i =0; i<SERVICES_NUM_PER_PAGE; i++)
	{
		index = u16Find_Current_Page * SERVICES_NUM_PER_PAGE + i;
		
		if(index < u16Toatl_FindList)
		{
			if(u16Find_Current_Focus == i)
				MV_Draw_FindList_Item(hdc, i, index, FOCUS);
			else 
				MV_Draw_FindList_Item(hdc, i, index, UNFOCUS);
		}	
		else
		{
			MV_Draw_FindList_Item(hdc, i, index, NOTFOCUS);
		}
	}	

	if (u16Toatl_FindList == 0)
	{
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		CS_MW_TextOut(hdc, ScalerWidthPixel(SList_ComboList_First.element_fields[1].x), ScalerHeigthPixel(SList_ComboList_First.element.y+4),CS_MW_LoadStringByIdx(CSAPP_STR_NO_PROGRAM));

		Scroll_Rect.top = CL_WINDOW_LIST_Y + 10;
		Scroll_Rect.left = CL_WINDOW_X + CL_WINDOW_DX - CL_WINDOW_ITEM_GAP - SCROLL_BAR_DX;
		Scroll_Rect.right = Scroll_Rect.left + SCROLL_BAR_DX;
		Scroll_Rect.bottom = CL_WINDOW_LIST_Y + CL_WINDOW_ITEM_DY * 10 + 10;
		MV_Draw_ScrollBar(hdc, Scroll_Rect, u16Find_Current_Service, u16Toatl_FindList, EN_ITEM_CHANNEL_LIST, MV_VERTICAL);
	}
}

void MV_Draw_FindCH_Info(HDC hdc)
{
	char			buff2[50];
	MV_stTPInfo		tpdata;
	MV_stSatInfo	Temp_SatData;	
	
	if (u16Toatl_FindList != 0)
	{
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(CL_WINDOW_X + CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y),ScalerWidthPixel(CL_WINDOW_DX - CL_WINDOW_ITEM_GAP * 2),ScalerHeigthPixel(CL_WINDOW_ITEM_DY * 2));	
		
		memset(buff2, 0, 50);
		MV_DB_GetServiceDataByIndex(&Find_service_data, u16Find_Ch_List[u16Find_Current_Service]);
		MV_DB_GetTPDataByIndex(&tpdata, Find_service_data.u16TransponderIndex);
		MV_GetSatelliteData_ByIndex(&Temp_SatData, MV_DB_Get_SatIndex_By_TPindex(Find_service_data.u16TransponderIndex));

		FillBoxWithBitmap (hdc, ScalerWidthPixel(CL_WINDOW_NO_X - CL_WINDOW_ITEM_GAP), ScalerHeigthPixel(CL_WINDOW_INFOR_Y + ( CL_WINDOW_ITEM_DY*2 - MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight )/2), ScalerWidthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmHeight), ScalerHeigthPixel(MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth), &MV_BMP[MVBMP_CHLIST_INFO_ICON]);
		
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		sprintf(buff2, "%s", Temp_SatData.acSatelliteName);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CL_WINDOW_NO_X + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth),ScalerHeigthPixel(CL_WINDOW_INFOR_Y + 4), buff2);

		if ( tpdata.u8Polar_H == 1 )
			sprintf(buff2, "%d / H / %d", tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		else
			sprintf(buff2, "%d / V / %d", tpdata.u16TPFrequency, tpdata.u16SymbolRate);
		CS_MW_TextOut(hdc,ScalerWidthPixel(CL_WINDOW_NO_X + MV_BMP[MVBMP_CHLIST_INFO_ICON].bmWidth),ScalerHeigthPixel(CL_WINDOW_INFOR_Y + CL_WINDOW_ITEM_DY + 4), buff2);
	}
}

void MV_Draw_FindCH_Focus(HWND hwnd, U16 u16Focusindex, U8 FocusKind)
{
	HDC		hdc;
	int 	Count_index;
	
	hdc = BeginPaint(hwnd);
	
	Count_index = u16Focusindex%SERVICES_NUM_PER_PAGE;

	if ( u16Find_Prev_Page != u16Find_Current_Page && FocusKind == FOCUS )
		MV_Draw_Find_Full_Item(hdc);
	else
		MV_Draw_FindList_Item(hdc, Count_index, u16Focusindex, FocusKind);
	
	if ( FocusKind == FOCUS )
		MV_Draw_FindCH_Info(hdc);
	
	EndPaint(hwnd,hdc);
}

void MV_Draw_Find_List(HWND hwnd)
{
	HDC			hdc;
	
	hdc = MV_BeginPaint(hwnd);

	MV_Draw_Find_List_Window(hdc);
	
	MV_Draw_Find_Title(hdc);
	MV_Draw_Find_Full_Item(hdc);
	MV_Draw_FindCH_Info(hdc);
	
	MV_EndPaint(hwnd,hdc);
}

CSAPP_Applet_t	CSApp_Finder(void)
{
    int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Finder_Applets = CSApp_Applet_Error;
	
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

	
	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "finder window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Finder_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_black;
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
	return CSApp_Finder_Applets;
    
}

static int Finder_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
    HDC 		hdc;
	U16			Prev_Service = 0;
	U16			u16Unlock_Index = 0;

	switch(message)
    {
        case MSG_CREATE:
			Find_List_state = FALSE;
			acReturnbuff_Temp[0] = '\0';
			u8Keypad_Index = 0;
			u16Toatl_FindList = 0;
			u16Find_Current_Service = 0;
			u16Find_Current_Page = 0;
			u16Find_Prev_Page = 0;
			u16Find_Current_Focus = 0;
			ComboList_Create(&SList_ComboList_First, SERVICES_NUM_PER_PAGE);
			break;
		case MSG_PAINT:
			MV_Draw_Finder_Keypad(hwnd);
			u16Toatl_FindList = MV_Get_Find_List(u16Find_Ch_List, acReturnbuff_Temp);
			MV_Draw_Find_List(hwnd);
			return 0;

		case MSG_CHECK_SERVICE_LOCK:
			MV_DB_GetServiceDataByIndex(&Find_service_data, u16Find_Ch_List[u16Find_Current_Service]);
			u16Unlock_Index = CS_DB_GetCurrent_Service_By_ServiceIndex(Find_service_data.u16ChIndex);
			//printf("======== %d : %d ===========>>>>>\n", u16Unlock_Index, CS_APP_GetLastUnlockServiceIndex());
			if ((CS_MW_GetServicesLockStatus()) && (Find_service_data.u8Lock== eCS_DB_LOCKED) \
				&& (u16Unlock_Index != CS_APP_GetLastUnlockServiceIndex()))
			{
				CS_MW_StopService(TRUE);
				//PinDlg_SetStatus(TRUE);
				//input_count = 0;
				//memset(input_keys, 0, sizeof(input_keys));
				if(wparam == 1)
					SendMessage(hwnd, MSG_PAINT, 0, 0);

				MV_Draw_Password_Window(hwnd);
			}
			else
			{
				SendMessage (hwnd, MSG_PLAYSERVICE, 0, 0);
			}

			break;

		case MSG_PLAYSERVICE:
			{	
				tCS_DBU_Service ServiceTriplet;
				U16				chlist_Current_Service;

				CS_DB_SetLastServiceTriplet();
				
				if ( MV_Get_Find_Current_Service_Order(u16Find_Ch_List[u16Find_Current_Service], &chlist_Current_Service) == eCS_DB_ERROR )
					break;

				printf("chlist_Current_Service : %d \n", chlist_Current_Service);
				CS_DB_SetCurrentService_OrderIndex(chlist_Current_Service);
				CS_DB_GetCurrentListTriplet(&(ServiceTriplet.sCS_DBU_ServiceList));
				ServiceTriplet.sCS_DBU_ServiceIndex =  chlist_Current_Service;
				CS_DBU_SaveCurrentService(ServiceTriplet);
				back_triplet = ServiceTriplet;
				FbSendFndDisplayNum((unsigned)chlist_Current_Service+1);

				CS_MW_PlayServiceByIdx(u16Find_Ch_List[u16Find_Current_Service], NOT_TUNNING);
			}
			break;

		case MSG_KEYDOWN:
			if ( finder_keypad_enable == TRUE )
			{
				Find_Keypad_Proc(hwnd, wparam);

				switch(wparam)
				{
					case CSAPP_KEY_ENTER:
						if(strlen(acReturnbuff_Temp) > 0)
						{									
							u16Toatl_FindList = 0;
							u16Find_Current_Service = 0;
							u16Find_Current_Page = 0;
							u16Find_Prev_Page = 0;
							u16Find_Current_Focus = 0;
							u16Toatl_FindList = MV_Get_Find_List(u16Find_Ch_List, acReturnbuff_Temp);
							MV_Draw_Find_List(hwnd);
						}
						break;

					case CSAPP_KEY_SWAP:						
						u16Toatl_FindList = 0;
						u16Find_Current_Service = 0;
						u16Find_Current_Page = 0;
						u16Find_Prev_Page = 0;
						u16Find_Current_Focus = 0;
						u16Toatl_FindList = MV_Get_Find_List(u16Find_Ch_List, acReturnbuff_Temp);
						MV_Draw_Find_List(hwnd);
						break;

					default:
						break;
				}
				break;
			} else if (MV_Get_Password_Flag() == TRUE)
				{
					U16		u16Unlock_Index = 0;
									
					if ( wparam != CSAPP_KEY_UP && wparam != CSAPP_KEY_DOWN )
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
									MV_Password_Set_Flag(FALSE);
									hdc = BeginPaint(hwnd);
									MV_Restore_PopUp_Window( hdc );
									EndPaint(hwnd,hdc);

									MV_DB_GetServiceDataByIndex(&Find_service_data, u16Find_Ch_List[u16Find_Current_Service]);
									u16Unlock_Index = CS_DB_GetCurrent_Service_By_ServiceIndex(Find_service_data.u16ChIndex);
									CS_APP_SetLastUnlockServiceIndex(u16Unlock_Index);
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
								}
								break;
								
							case CSAPP_KEY_ENTER:
								if(MV_Password_Retrun_Value() == TRUE)
								{
									MV_Password_Set_Flag(FALSE);
									hdc = BeginPaint(hwnd);
									MV_Restore_PopUp_Window( hdc );
									EndPaint(hwnd,hdc);

									MV_DB_GetServiceDataByIndex(&Find_service_data, u16Find_Ch_List[u16Find_Current_Service]);
									u16Unlock_Index = CS_DB_GetCurrent_Service_By_ServiceIndex(Find_service_data.u16ChIndex);
									CS_APP_SetLastUnlockServiceIndex(u16Unlock_Index);
									SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
								}
								else
									MV_Draw_Password_Window(hwnd);
								break;

							case CSAPP_KEY_ESC:
							case CSAPP_KEY_MENU:
								MV_Password_Set_Flag(FALSE);
								hdc = BeginPaint(hwnd);
								MV_Restore_PopUp_Window( hdc );
								EndPaint(hwnd,hdc);
								break;
						}
						
						if ( wparam == CSAPP_KEY_TV_AV )
						{
							ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
						}
						break;
					} else { 
						MV_Password_Set_Flag(FALSE);
						hdc = BeginPaint(hwnd);
						MV_Restore_PopUp_Window( hdc );
						EndPaint(hwnd,hdc);
						break;
					}
				}
			
			switch(wparam)
			{
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Finder_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_LEFT:
					{
						if(u16Toatl_FindList == 0)
							break;

						if(u16Find_Current_Service < SERVICES_NUM_PER_PAGE)
						{
							u16Find_Current_Service = (getpage(u16Toatl_FindList, SERVICES_NUM_PER_PAGE)+1)*SERVICES_NUM_PER_PAGE + u16Find_Current_Service-SERVICES_NUM_PER_PAGE;
						}
						else
							u16Find_Current_Service -= SERVICES_NUM_PER_PAGE;

						if(u16Find_Current_Service>u16Toatl_FindList-1)
							u16Find_Current_Service = u16Toatl_FindList-1;

						u16Find_Current_Focus = get_focus_line(&u16Find_Current_Page, u16Find_Current_Service, SERVICES_NUM_PER_PAGE);

						hdc = BeginPaint(hwnd);
						MV_Draw_Find_Full_Item(hdc);
						MV_Draw_FindCH_Info(hdc);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_RIGHT:
					{
						U8	current_page, total_page;
						
						if(u16Toatl_FindList == 0)
							break;

						current_page = getpage((u16Find_Current_Service+1), SERVICES_NUM_PER_PAGE);
						total_page = getpage(u16Toatl_FindList, SERVICES_NUM_PER_PAGE);
							
						u16Find_Current_Service += SERVICES_NUM_PER_PAGE;
						if(u16Find_Current_Service > u16Toatl_FindList-1)
						{
							if(current_page<total_page)
								u16Find_Current_Service = u16Toatl_FindList-1;
							else
								u16Find_Current_Service = 0;
						}

						u16Find_Current_Focus = get_focus_line(&u16Find_Current_Page, u16Find_Current_Service, SERVICES_NUM_PER_PAGE);

						hdc = BeginPaint(hwnd);
						MV_Draw_Find_Full_Item(hdc);
						MV_Draw_FindCH_Info(hdc);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_UP:
					{
						if(u16Toatl_FindList == 0)
							break;

						MV_Draw_FindCH_Focus(hwnd, u16Find_Current_Service, UNFOCUS);

						u16Find_Prev_Page = u16Find_Current_Page;

						if(u16Find_Current_Service == 0)
							u16Find_Current_Service = u16Toatl_FindList-1;
						else
							u16Find_Current_Service--;

						u16Find_Current_Focus = get_focus_line(&u16Find_Current_Page, u16Find_Current_Service, SERVICES_NUM_PER_PAGE);

						MV_Draw_FindCH_Focus(hwnd, u16Find_Current_Service, FOCUS);
					}
					break;
					
				case CSAPP_KEY_DOWN:
					{
						if(u16Toatl_FindList == 0)
							break;

						MV_Draw_FindCH_Focus(hwnd, u16Find_Current_Service, UNFOCUS);

						u16Find_Prev_Page = u16Find_Current_Page;
						
						if(u16Find_Current_Service == u16Toatl_FindList-1)
							u16Find_Current_Service = 0;
						else
							u16Find_Current_Service++;

						u16Find_Current_Focus = get_focus_line(&u16Find_Current_Page, u16Find_Current_Service, SERVICES_NUM_PER_PAGE);

						MV_Draw_FindCH_Focus(hwnd, u16Find_Current_Service, FOCUS);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_Finder_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_ENTER:
					{
						U16		chlist_Current_Service;
						
						if ( MV_Get_Find_Current_Service_Order(u16Find_Ch_List[u16Find_Current_Service], &chlist_Current_Service) == eCS_DB_ERROR )
							break;
						
						if ( chlist_Current_Service == CS_DB_GetCurrentService_OrderIndex() )
						{						
							CSApp_Finder_Applets = CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						} else {
							if ( chlist_Current_Service != Prev_Service )
							{
								CS_APP_SetLastUnlockServiceIndex(0xffff);
							}
							Prev_Service = chlist_Current_Service;
							SendMessage (hwnd, MSG_CHECK_SERVICE_LOCK, 0, 0);
						}
					}
					break;

				case CSAPP_KEY_RED:
					u8Keypad_Index = 0;
					MV_Draw_Finder_Keypad(hwnd);
					break;
					
				default:
					break;
			}
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

		case MSG_CLOSE:
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
			
		default:
			break;		
    }
    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


