#ifndef __UI_COM_H
#define __UI_COM_H

#include 	"cs_app_common.h"
#include 	"linuxos.h"
#include 	"database.h"
#include 	"userdefine.h"
#include 	"cs_app_common.h"
#include 	"cs_app_main.h"
#include 	"mvosapi.h"
#include    "scart.h"   /* By KB Kim : 2010_08_31 for Scart Control */

#define		DR_TOP									0x00000001
#define		DR_LEFT									0x00000002
#define		DR_BOTTOM								0x00000004
#define		DR_RIGHT								0x00000008

#define 	WARNING_WINDOW_X						364
#define		WARNING_WINDOW_Y						214
#define		WARNING_WINDOW_DX						512
#define		WARNING_WINDOW_DY						162
#define		WARNING_WINDOW_TITLE_X					370
#define		WARNING_WINDOW_TITLE_Y					220
#define		WARNING_WINDOW_TITLE_DX					500
#define		WARNING_WINDOW_TITLE_DY					30
#define		WARNING_WINDOW_CONTENT_X				370
#define		WARNING_WINDOW_CONTENT_Y				254
#define		WARNING_WINDOW_CONTENT_DX				500
#define		WARNING_WINDOW_CONTENT_DY				116
#define		WARNING_WINDOW_CONTENT_ITEM_DY			30
#define		WARNING_WINDOW_BUTTON_X1				560
#define		WARNING_WINDOW_BUTTON_X2				630
#define		WARNING_WINDOW_BUTTON_DX				50
#define		WARNING_WINDOW_BUTTON_Y					330

#define		JUMP_WINDOW_STARTX						540
#define		JUMP_WINDOW_STARTY						210
#define		JUMP_WINDOW_STARTDX						200
#define		JUMP_WINDOW_STARTDY						100
#define 	JUMP_WINDOW_OUTGAP						6
#define 	JUMP_WINDOW_ITEM_DY						30

#define		PASSWORD_WINDOW_STARTX					500
#define		PASSWORD_WINDOW_STARTY					210
#define		PASSWORD_WINDOW_STARTDX					280
#define		PASSWORD_WINDOW_STARTDY					150
#define		PASSWORD_WINDOW_STARTX_CI				400
#define		PASSWORD_WINDOW_STARTDX_CI				480
#define 	PASSWORD_WINDOW_OUTGAP					6
#define 	PASSWORD_WINDOW_ITEM_DY					30

#define 	KEYBOARD_STARTX							450
#define 	KEYBOARD_STARTY							WINDOW_TOP - 10
#define 	KEYBOARD_STARTDX						360
#define 	KEYBOARD_STARTDY						WINDOW_DY//16*16
#define 	KEYBOARD_KEY_OFFSET 					30
#define 	KEYBOARD_NUM_KEY_STARTX					KEYBOARD_STARTX + KEYBOARD_KEY_OFFSET
#define 	KEYBOARD_CHAR_KEY_STARTX				KEYBOARD_NUM_KEY_STARTX
#define 	KEYBOARD_NUM_KEY_STARTY					WINDOW_ITEM_Y + KEYBOARD_KEY_OFFSET + 20
#define 	KEYBOARD_CHAR_KEY_STARTY				KEYBOARD_NUM_KEY_STARTY
#define 	KEYBOARD_NUM_STARTX	 					KEYBOARD_NUM_KEY_STARTX + 10
#define 	KEYBOARD_CHAR_STARTX 					KEYBOARD_CHAR_KEY_STARTX + 8
#define		KEYBOARD_KEY_SIZE						MV_BMP[MVBMP_UNFOCUS_KEYPAD].bmWidth
#define		KEYBOARD_BIGKEY_SIZE					56

#define		NUM_KEYPAD_STARTX						460
#define		NUM_KEYPAD_STARTY						WINDOW_TOP + 50
#define		NUM_KEYPAD_STARTDX						340
#define		NUM_KEYPAD_STARTDY						240
#define		NUM_KEYPAD_IMG_SIZE						MV_BMP[MVBMP_UNFOCUS_KEYPAD].bmWidth
#define		NUM_KEYPAD_XOFFSET						30

#define		HEXAKEYPAD_STARTX						50
#define		HEXAKEYPAD_STARTY						300
#define		HEXAKEYPAD_STARTDX						512
#define		HEXAKEYPAD_STARTDY						90
#define		HEXAKEYPAD_IMG_SIZE						16
#define		HEXAKEYPAD_XOFFSET						29

#define		ROW_COUNT								7
#define		FIND_ROW_COUNT							5
#define		COL_COUNT								10
#define		NUM_COUNT								10

#define		KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTX	KEYBOARD_NUM_KEY_STARTX
#define		KEYBOARD_INPUT_WINDOW_SHOWBAR_DX		KEYBOARD_KEY_OFFSET * 11
#define		KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTY	WINDOW_ITEM_Y
#define		KEYBOARD_INPUT_WINDOW_STRING_STARTX		KEYBOARD_INPUT_WINDOW_SHOWBAR_STARTX
#define		KEYBOARD_INPUT_WINDOW_STRING_STARTY		WINDOW_ITEM_Y + 10

#define		HEXA_KEYPAD_STR_BOARDX					350
#define 	HEXA_KEYPAD_STR_BOARDY					NUM_KEYPAD_STARTY - 50
#define		HEXA_KEYPAD_STR_BOARDDX					580
#define		HEXA_KEYPAD_STR_BOARDDY					(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight * 2) + KEYBOARD_KEY_OFFSET
#define		HEXA_KEYPAD_OUTLINE_OFFSET				MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight
#define 	HEXA_KEYPAD_KEY_BOARDX					550
#define 	HEXA_KEYPAD_KEY_BOARDY					HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY + KEYBOARD_KEY_OFFSET
#define 	HEXA_KEYPAD_KEY_BOARDDX					180
#define 	HEXA_KEYPAD_KEY_BOARDDY					(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight * 2) + 220
#define		HEXA_KEYPAD_HELP_X						HEXA_KEYPAD_STR_BOARDX
#define		HEXA_KEYPAD_HELP_Y						HEXA_KEYPAD_KEY_BOARDY + HEXA_KEYPAD_KEY_BOARDDY + KEYBOARD_KEY_OFFSET
#define		HEXA_KEYPAD_HELP_DX						HEXA_KEYPAD_STR_BOARDDX
#define		HEXA_KEYPAD_HELP_DY						HEXA_KEYPAD_STR_BOARDDY
#define		HEXA_KEYPAD_WINDOWDY					HEXA_KEYPAD_STR_BOARDDY + HEXA_KEYPAD_KEY_BOARDDY + HEXA_KEYPAD_HELP_DY + ( KEYBOARD_KEY_OFFSET * 3 )

#define 	STRING_KEYPAD_KEY_BOARDX				470 /* was 500 */
#define 	STRING_KEYPAD_KEY_BOARDY				HEXA_KEYPAD_STR_BOARDY + HEXA_KEYPAD_STR_BOARDDY + KEYBOARD_KEY_OFFSET - 10
#define 	STRING_KEYPAD_KEY_BOARDDX				340 /* was 280 */
#define 	STRING_KEYPAD_KEY_BOARDDY				(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight * 2) + 240

#define		INPUT_WINDOW_NUM_STARTX					NUM_KEYPAD_STARTX + 20
#define		INPUT_WINDOW_NUM_STARTY					NUM_KEYPAD_STARTY + 60
#define		INPUT_WINDOW_NUM_STARTDX				NUM_KEYPAD_STARTDX - 40
#define		INPUT_WINDOW_NUM_STARTDY				KEYBOARD_KEY_OFFSET
#define 	INPUT_WINDOW_NUM_ITEMY					INPUT_WINDOW_NUM_STARTY + 40

/* ============== File Window Start =========================== */
#define 	FILE_LIST_MAX_ITEM						10
#define		FILE_MAX_NAME_LENGTH					255

#define		FILE_WINDOW_ITEM_HEIGHT					30
#define		FILE_WINDOW_X_CAP						10 //10
#define		FILE_WINDOW_Y_CAP						10
#define		FILE_WINDOW_X							300 //100 //300
#define		FILE_WINDOW_Y							150
#define		FILE_WINDOW_DX							680
#define		FILE_WINDOW_DY							( ( FILE_WINDOW_ITEM_HEIGHT * (FILE_LIST_MAX_ITEM + 1) ) + ( FILE_WINDOW_Y_CAP * 3 ) )
#define		FILE_WINDOW_ITEM_NAME_DX				340
#define		FILE_WINDOW_ITEM_SIZE_DX				130
#define		FILE_WINDOW_ITEM_DATE_DX				190
#define		FILE_WINDOW_ITEM_DX						( FILE_WINDOW_ITEM_NAME_DX + FILE_WINDOW_ITEM_SIZE_DX + FILE_WINDOW_ITEM_DATE_DX )
#define		FILE_WINDOW_ITEM_X						( FILE_WINDOW_X + FILE_WINDOW_X_CAP )
#define		FILE_WINDOW_ITEM_Y						( FILE_WINDOW_Y + FILE_WINDOW_ITEM_HEIGHT + (FILE_WINDOW_Y_CAP * 2) )
#define		FILE_WINDOW_ITEM_DY						( FILE_WINDOW_ITEM_HEIGHT * FILE_LIST_MAX_ITEM )
#define		FILE_WINDOW_TITLE_X						( FILE_WINDOW_X + FILE_WINDOW_X_CAP )
#define		FILE_WINDOW_TITLE_Y						( FILE_WINDOW_Y + FILE_WINDOW_Y_CAP )
#define		FILE_WINDOW_TITLE_DX					( FILE_WINDOW_DX - ( FILE_WINDOW_X_CAP * 2 ) )
#define		FILE_WINDOW_NAME_X						FILE_WINDOW_ITEM_X
#define		FILE_WINDOW_SIZE_X						( FILE_WINDOW_NAME_X + FILE_WINDOW_ITEM_NAME_DX )
#define		FILE_WINDOW_DATE_X						( FILE_WINDOW_SIZE_X + FILE_WINDOW_ITEM_SIZE_DX )
/* ============== File Window End ============================= */

/* For Plugin Site List by File : KB Kim 2011.09.13 */
#define		SITE_WINDOW_DY							(FILE_WINDOW_DY +  FILE_WINDOW_ITEM_HEIGHT + FILE_WINDOW_Y_CAP)
#define     SITE_HELP_DX                            (FILE_WINDOW_TITLE_DX / 4)
#define     SITE_HELP_Y                             (FILE_WINDOW_Y + FILE_WINDOW_DY + 5)
#define     SITE_HELP_X                             (FILE_WINDOW_TITLE_X + FILE_WINDOW_X_CAP)
#define     SITE_HELP_X1                            (SITE_HELP_X + SITE_HELP_DX)
#define     SITE_HELP_X2                            (SITE_HELP_X1 + SITE_HELP_DX)
#define     SITE_HELP_X3                            (SITE_HELP_X2 + SITE_HELP_DX)

/* By KB Kim for Plugin Setting : 2011.05.07 */
#define	TMP_UPGRADE_FILE			"/tmp/www/AllFlash_32M.tar"
#define	TMP_PLUGIN_FILE				"/tmp/plugin/plugin.tar"
#define	TMP_PLUGIN_FOLDER			"/tmp/plugin"

typedef enum
{
    eMV_TITLE_SORT = 0,
    eMV_TITLE_SAT,
    eMV_TITLE_FAV,
    eMV_TITLE_SAT_FAV,
    eMV_TITLE_ATTENTION,
    eMV_TITLE_WARNING,
    eMV_NO_TITLE,
    eMV_TITLE_MAX
} tWindow_Title;

typedef struct
{
	RECT			Window_Rect;
	U16				u16Total;
	U16				u16Current_Pos;
	U8				u8Item_Num;
	BOOL			b8Scroll_OnOff;
	BOOL			b8YesNo_Button;
	U8				u8Help_Kind;
	tWindow_Title	tTitle;
} stPopUp_Window;

typedef struct
{
	char			Contents[200][100];
	U8				u8TotalCount;
	U8				u8Focus_Position;
} stPopUp_Window_Contents;

#define RECfile 			"/mnt/usb/disk1/RECORDS"
#define PVR_DATA_LEN		3

typedef struct
{
	char		PVR_Ch_Index[128];
	char		PVR_Sat_Name[128];
	char		PVR_TP_Info[128];
	char		PVR_Title[128];
	char		PVR_Start_UTC[128];
	time_t		PVR_Start_OS;
	char		PVR_End_UTC[128];
	time_t		PVR_End_OS;
	long long	PVR_Last_Posion;
}st_pvr_data;

typedef enum
{
	PVR_REC_ITEM_CHINDEX = 0,	/* Channel name */
	PVR_REC_ITEM_SAT,			/* Satellite Name */
	PVR_REC_ITEM_TP,			/* TP Information */
	PVR_REC_ITEM_TITLE,			/* Event Title */
	PVR_REC_ITEM_ST,			/* Record Start UTC Time */
	PVR_REC_ITEM_SD,			/* Record Start OS Time For Sec Count */
	PVR_REC_ITEM_ET,			/* Record End UTC Time */
	PVR_REC_ITEM_ED,			/* Record End OS Time For Sec Count */
	PVR_REC_ITEM_LP,			/* Play Last Position */
	PVR_DATA_KIND
} eMV_PVR_REC_Items;

typedef struct
{
	char 	acFileName[100];
	char	acFileExt[10];
} stFile_db_temp;

/* By KB Kim for Plugin Setting : 2011.05.07 */
/* For Plugin Site List by File : KB Kim 2011.09.13 */
typedef enum
{
	NET_UPGRADE_MAIN = 0,	/* Upgrade Main Flash */
	NET_UPGRADE_PLUGIN,		/* Upgrade Plugin     */
	NET_PLUGIN_SITE,        /* Plugin site List   */
	NET_UPGRADE_KIND
} eMV_NET_Upgrade_Items;
#define PLUGIN_MENU

#ifdef PLUGIN_MENU
typedef struct
{
	U8		u8Set_Version;
	U8 		u8SW_Version;
	char	acFile_Location[1024];
	char	acFile_Name[256];
	char	acFile_size[32];
	char	acFile_date[32];
} stMV_Upgrade_Info;

typedef struct
{
	char	pFile_Detail[100];
	char	pFile_Name[256];
	char	pFile_size[32];
} stMV_Plugin_Info;
#endif

char 		ReturnV[50];
BOOL		keypad_enable;
BOOL		num_keypad_enable;
BOOL		hexa_keypad_enable;
BOOL		String_keypad_enable;
U8			b8CapsLock;
BITMAP		btMsg_cap;
BITMAP		btMsg_Con_cap;
BITMAP		btFile_cap;

/* For Key Emulation By KB Kim 2011.05.13 */
void Mv_BroadcastMessage(int msg, U32 param, U32 subParam);

BOOL MV_Get_Save_Flag(void);
void MV_Set_Save_Flag(BOOL b8TureOrFalse);
BOOL MV_Get_Attention_Flag(void);

BOOL MV_Get_PopUp_Window_Status(void);
U8 MV_Get_PopUp_Window_Result(void);
tWindow_Title MV_Get_PopUp_Window_Kind(void);
void MV_Capture_PopUp_Window(HDC hdc);
void MV_Restore_PopUp_Window(HDC hdc);
void MV_Draw_PopUp_Title_Bar(HDC hdc, RECT *Title_Rect, tWindow_Title title_i);
void MV_Draw_PopUp_Window(HWND hwnd, stPopUp_Window stWindow, stPopUp_Window_Contents *stContents);
void MV_Draw_PopUp_Item(HDC hdc, U8 u8Item_Num, U8 u8Focus_Kind);
BOOL MV_PopUp_Proc(HWND hwnd, WPARAM u8Key);

void MV_Draw_Satlist_Window(HWND hwnd);
void MV_Draw_SatFavlist_Window(HWND hwnd, U8 u8TVRadio);
void MV_Draw_Favlist_Window(HWND hwnd, U8 u8TVRadio, BOOL b8TrueORFalse);
void MV_Draw_NoName_Window(HWND hwnd, RECT *reRect, stPopUp_Window_Contents *stContents);

void MV_Draw_Keypad(HWND hwnd, U8 *item_str, U8 max_string_length);
void MV_Draw_NumKeypad(HWND hwnd, U16 u16Num, U16 u16Input_Key, U8 max_string_length);
void MV_Close_Keypad( HWND hwnd );
void Draw_keypad(HDC hdc, U8 *item_str);
void Draw_Numkeypad(HDC hdc, U8 *item_str);
BOOL UI_Keypad_Proc(HWND hwnd, WPARAM u8Key);
BOOL UI_NumKeypad_Proc(HWND hwnd, WPARAM u8Key);
void Get_Save_Str(char *Return_Value);
void Selected_Key(HWND hwnd, U8 u8_NO, U8 Select_Kind);
void NumSelected_Key(HWND hwnd, U8 Select_Kind);
U8 Press_Key(void);
BOOL Get_Keypad_Status(void);
BOOL Get_NumKeypad_Status(void);
BOOL Get_Keypad_is_Save(void);
void Draw_Loading_Progress_Bar(HDC hdc, U8 u8ProgressValue, char *char_str);
void MV_Scroll_Window(HDC hdc, char *char_str);
void MV_Draw_PopUp_Title_Bar_ByName(HDC hdc, RECT *Title_Rect, U32 title_i);
void MV_Draw_PopUp_Title_Bar_ByStr(HDC hdc, RECT *Title_Rect, char *title_str);
void MV_Jump_Retrun_Value(char *Temp);
void MV_Jump_Value_Display(HWND hwnd);
void MV_Draw_Jump_Window(HWND hwnd, RECT *reRect);
BOOL MV_Jump_Proc(HWND hwnd, WPARAM u8Key);
void Close_Msg_Window(HDC hdc);
void MV_Draw_Msg_Window(HDC hdc, U32 u32Str);
BOOL MV_Get_Jump_Flag(void);

void MV_Change_Confirm(void);
void MV_Set_Confirm(BOOL b8Confirm);
BOOL MV_Check_Confirm_Window(void);
BOOL MV_Check_YesNo(void);
void MV_Draw_Confirm_Button(HDC hdc);
void MV_Draw_Progress_Window(HWND hwnd);
void Restore_Progress_Window(HWND hwnd);
void MV_Draw_Progress_status(HWND hwnd, MV_stTPInfo Temp_TPData, U8 u8Max_Count, U8 Now_Count);
void MV_Draw_Confirm_Window(HWND hwnd, U16 u16Message_index);
void Restore_Confirm_Window(HDC hdc);
BOOL MV_Confirm_Proc(HWND hwnd, WPARAM u8Key);

/* By KB Kim for Plugin Setting : 2011.05.07 */
#ifdef PLUGIN_MENU
void MV_SetNetUpgradeMode(eMV_NET_Upgrade_Items mode);
eMV_NET_Upgrade_Items MV_GetNetUpgradeMode(void);
void MV_Draw_Msg_Download(HWND hwnd, U32 u32Message);
void MV_Close_Msg_Download(HWND hwnd);
BOOL MV_CheckDownLoadStatus(void);
int Download_Init(void);
void *Download_Task( void *param );
void Download_Stop(void);
BOOL MV_NetCheck_Confirm_Window(void);
BOOL MV_NetCheck_YesNo(void);
void Restore_NetDown_Confirm_Window(HDC hdc);
void MV_Draw_NetDown_Confirm(HWND hwnd);
BOOL MV_NetDown_Confirm_Proc(HWND hwnd, WPARAM u8Key);
BOOL MV_CheckWgetListStatus(void);
void MV_Close_Wget_Window(HDC hdc);
MV_CFG_RETURN MV_Upgrade_Get_Wget_File(void);
MV_CFG_RETURN MV_Plugin_Get_Wget_File(void);
void MV_Draw_Wget_FileList(HWND hwnd);
BOOL MV_WgetList_Proc(HWND hwnd, WPARAM u8Key);

/* For Plugin Site List by File : KB Kim 2011.09.13 */
char *MV_GetPluginSite(void);
BOOL MV_CheckSiteListStatus(void);
U16 MV_GetPluginSiteCount(void);
void MV_Close_SiteList_Window(HDC hdc);
MV_CFG_RETURN MV_Plugin_Save_SiteList(void);
MV_CFG_RETURN MV_Plugin_Set_DefaultSiteList(void);
void MV_Delete_PluginSite(HWND hwnd);
void MV_Add_PluginSite(char *siteData);
void MV_Edit_PluginSite(char *siteData);
MV_CFG_RETURN MV_Plugin_Get_SiteList(void);
void MV_Draw_PlugIn_Site_List(HWND hwnd);
#endif

void MV_Parser_PVR_Value( char *Temp, char *tempSection );
void MV_PVR_CFG_File_Parser(st_pvr_data *PVR_Rec_Data, char *PVR_FileName);
MV_CFG_RETURN MV_PVR_CFG_File_Set_LastPosition(char *PVR_FileName, long long Last_Position);
void MV_Draw_HexaKeypad(HWND hwnd, U8 *item_str, U8 max_string_length);
void Draw_Hexa_keypad( HDC hdc );
void Draw_Hexa_Keypad_Help(HDC hdc);
void MV_HexaKeypad_Proc(HWND hwnd, WPARAM u8Key);
char *MV_Get_HexaEdited_String(void);
BOOL MV_Get_HexaKeypad_Status(void);
BOOL MV_Get_Password_Flag(void);
BOOL MV_Password_Proc(HWND hwnd, WPARAM u8Key);
void MV_Password_Set_Flag(BOOL bflag);
BOOL MV_Password_Retrun_Value(void);
void MV_Draw_Password_Window(HWND hwnd);
void MV_Draw_PIN_Window(HWND hwnd, char *Str_Temp);
void MV_Draw_PIN_Window_For_CI(HWND hwnd, char *Str_Temp);
char *MV_Password_Return_Str(void);
BOOL MV_Get_StringKeypad_Status(void);
void MV_Set_StringKeypad_Status(BOOL bStatus);
char *MV_Get_StringEdited_String(void);
void MV_Close_StringKeypad(HWND hwnd);
void MV_Draw_StringEdit_String(HDC hdc, RECT *Temp_Rect);
void Draw_String_keypad( HDC hdc );
void Draw_String_Keypad_Help(HDC hdc);
void MV_Draw_StringKeypad(HWND hwnd, U8 *item_str, U8 max_string_length);
void MV_StringKeypad_Proc(HWND hwnd, WPARAM u8Key);

BOOL MV_Get_NumEdit_Flag(void);
void MV_NumEdit_Retrun_Value(char *Temp);
void MV_NumEdit_Draw_Value(HDC hdc, RECT *lrect);
void MV_Draw_Only_Number_Edit(HWND hwnd, RECT *reRect, S16 acEdit_Number, U8 u8MaxLength, U16 u16String_Index);
void MV_NumEdit_Update_Value(WPARAM wparam);
BOOL MV_NumEdit_Proc(HWND hwnd, WPARAM u8Key);

void MV_Draw_File_Window(HWND hwnd);
BOOL MV_Check_File_Window(void);
void MV_Close_File_Window(HDC hdc);
BOOL MV_File_Check_Enter(void);
char *MV_Get_FileName(void);
char *MV_Get_FileExt(void);
BOOL MV_FileList_Proc(HWND hwnd, WPARAM u8Key);
void MV_Draw_Box(HDC hdc, RECT *rRect, gal_pixel Line_color, UINT nFormat);
void *Disk_Ani_Task( void *param );
int Disc_Ani_Init(void);
void Disc_Ani_Stop(void);
void *Main_Ani_Task( void *param );
int MainMenu_Ani_Init(U8 MainMenu_Focus_Item);
void MainMenu_Ani_Stop(void);
MV_File_Return MV_Load_TS_fileData(stFile_db *stFileDB);

void *Timer_Clock_Task( void *param );
int Timer_Clock_Init(RECT *Dr_Rect, DWORD Clock_Back, DWORD Color_Font);
void Timer_Clock_Stop(void);

BOOL Motor_Moving_State(void);
void Motor_Moving_Stop(void);
void Draw_Motor_Moving_Massage(HDC hdc);
void Clear_Motor_Moving_Massage(HDC hdc);
void *Motor_Moving_Task( void *param );
int Motor_Moving_Start (U16 u16First_Longitude, U16 u16Second_Longitude);
int Mv_MotorMovingDisplay (void);

void MV_Timer_Draw_Modify_Item_SelectBar(HDC hdc, int y_gap, U8 esItem, U8 u8Focuskind);
void MV_Timer_Draw_Modify_Item(HDC hdc, U8 u8Focuskind, U8 esItem);
void MV_Timer_Draw_Modify_Itmes(HDC hdc, U8 eItem);
void MV_Timer_Draw_Modify_Window(HWND hwnd, U16 Current_Item);
BOOL MV_Check_Timer_Window_Status(void);
void MV_Timer_Close_Window(HWND hwnd);
BOOL MV_Timer_Proc(HWND hwnd, WPARAM u8Key);
void MV_Timer_Update_Time(WPARAM wparam);
void MV_Timer_Draw_Time(HDC hdc, U8 u8Focuskind, U8 u8DrawItem);
tCS_TIMER_Error MV_Timer_Upload_Data(void);
BOOL MV_Check_Timer_Save_Status(void);
void MV_Draw_Menu_Signal(HDC hdc, RECT rRect);

#endif //#ifndef __UI_COM_H

