#ifndef  _CS_APP_INSTALL_H_
#define  _CS_APP_INSTALL_H_

#include "mv_motor.h"

#define	UNFOCUS			0
#define	FOCUS			1
#define	MAX_MULTI_SAT	9

#define	D_WIN_X			200
#define	D_WIN_Y			140
#define	D_WIN_DX		880
#define	D_WIN_DY		440
#define D_WIN_GAP		10
#define D_WIN_ITEM_X	210
#define D_WIN_ITEM_Y	190
#define D_WIN_ITEM_DX	860
#define D_WIN_ITEM_DY	370

typedef struct
{
	eScanMode		ScanMode;
	U32				StrIdx;
	U32 			Freqncy;
	U8				FreqStr[12];
	U32				Bandwidth;	
	U8				BandStr[6];
}tScanData;

typedef enum
{
	SCAN_ITEM_CONNECTION=0,
	SCAN_ITEM_SATELLITE,
	SCAN_ITEM_TP_SELECT,
	SCAN_ITEM_LNB_TYPE,
	SCAN_ITEM_HIGH_FREQ,
	SCAN_ITEM_LOW_FREQ,
	SCAN_ITEM_DISECQ,
	SCAN_ITEM_SCAN_TYPE,
	SCAN_ITEM_MAX
}eScanItemID;

typedef enum
{
	SCAN_CON_ALL = 0,
	SCAN_CON_ALL_NIT,
	SCAN_CON_FTA,
	SCAN_CON_FTA_NIT,
	SCAN_CON_ALL_BLIND,
	SCAN_CON_FTA_BLIND,
	SCAN_CON_MAX
}eScanConditionID;

typedef enum
{
	SAT_ITEM_SATELLITE=0,
	SAT_ITEM_TP_SELECT,
	SAT_ITEM_LNB_TYPE,
	SAT_ITEM_HIGH_FREQ,
	SAT_ITEM_LOW_FREQ,
	SAT_ITEM_22K,
	SAT_ITEM_DISECQ,
	SAT_ITEM_TONEBURST,
	SAT_ITEM_SCAN_TYPE,
	SAT_ITEM_MAX
}eSatItemID;

typedef enum
{
	TP_ITEM_SATELLITE=0,
	TP_ITEM_TP_SELECT,
	TP_ITEM_FREQ,
	TP_ITEM_SYMBOL,
	TP_ITEM_POL,
	TP_ITEM_DEL_TP,
	TP_ITEM_TP_ADD,
	TP_ITEM_MAN_CHADD,
	TP_ITEM_MAX
}eTPItemID;

typedef enum
{
	TPADD_ITEM_SATELLITE=0,
	TPADD_ITEM_FREQ,
	TPADD_ITEM_SYMBOL,
	TPADD_ITEM_POL,
	TPADD_ITEM_MAX
}eTPADDItemID;

typedef enum
{
	CHADD_ITEM_SATELLITE=0,
	CHADD_ITEM_TP,
	CHADD_ITEM_NAME,
	CHADD_ITEM_VPID,
	CHADD_ITEM_APID,
	CHADD_ITEM_PPID,
	CHADD_ITEM_TVRADIO,
	CHADD_ITEM_MAX
}eCHADDItemID;

typedef enum
{
	MOTOR_ITEM_SATELLITE=0,
	MOTOR_ITEM_TP,
	MOTOR_ITEM_POSITION,
	MOTOR_ITEM_GOTOX,
	MOTOR_ITEM_STEPMOVE,
	MOTOR_ITEM_AUTOMOVE,
	MOTOR_ITEM_LIMIT_SET,
	MOTOR_ITEM_WEST_LIMIT,
	MOTOR_ITEM_EAST_LIMIT,
	MOTOR_ITEM_REF,
	MOTOR_ITEM_RECALC,
	MOTOR_ITEM_SAVE,
	MOTOR_ITEM_MAX
}eMotorItemID;

typedef enum
{
	USALS_ITEM_SATELLITE=0,
	USALS_ITEM_TP,
	USALS_ITEM_SAT_LONGITUDE,
	USALS_ITEM_LONGITUDE,
	USALS_ITEM_LATITUDE,
	USALS_ITEM_LIMIT_SET,
	USALS_ITEM_WEST_LIMIT,
	USALS_ITEM_EAST_LIMIT,
	USALS_ITEM_REF,
	USALS_ITEM_MAX
}eUsalsItemID;

typedef enum
{
	UNICABLE_ITEM_SEL=0,
	UNICABLE_ITEM_POS,
	UNICABLE_ITEM_CHAN,
	UNICABLE_ITEM_FREQ,
	UNICABLE_ITEM_MAX
}eUniCableItemID;

typedef enum
{
	INSTALL_SEARCH=0,
	SATELLITE_SETTING,
	TP_SETTING,
	MOTOR_SETTING,
	USALS_SETTING,
	UNICABLE_SETTING,
	MAX_MENU
}	MV_Menu_Kind;

typedef enum
{
	C_BAND_5150=0,
	C_BAND_5750,
	KU_BAND_9750,
	KU_BAND_10000,
	KU_BAND_10600,
	KU_BAND_10678,
	KU_BAND_10700,
	KU_BAND_10750,
	KU_BAND_11000,
	KU_BAND_11200,
	KU_BAND_11250,
	KU_BAND_11300,
	UNIVERSAL_5150_5750,
	UNIVERSAL_5750_5150,
	UNIVERSAL_9750_10600,
	UNIVERSAL_9750_10700,
	UNIVERSAL_9750_10750,
	DIGITURK_1,
	DIGITURK_2,
	DIGITURK_3,
	DIGITURK_4,
	MAX_LNBTYPE
}	MV_LNB_TYPE;

typedef enum
{
	D1_1=0,
	D1_4,
	D2_4,
	D3_4,
	D4_4,
	D1_16,
	D2_16,
	D3_16,
	D4_16,
	D5_16,
	D6_16,
	D7_16,
	D8_16,
	D9_16,
	D10_16,
	D11_16,
	D12_16,
	D13_16,
	D14_16,
	D15_16,
	D16_16,
	MAX_DISECQ_PORT
}	MV_DISECQ_PORT;

/* For Tone Burst Control : By KB Kim 2011.02.28 */
typedef enum
{
	TONE_OFF=0,
	TONE_SAT_A,
	TONE_SAT_B
} MV_TONE_BURST;

typedef enum
{
	ONE_SAT=0,
	MULTI_SAT,
	MAX_SCAN_TYPE
}	MV_SCAN_TYPE;

//search result
enum
{
	CSAPP_SEARCH_RESULT_ITEM_BASE=0,
	
	CSAPP_SEARCH_RESULT_MODE,
	CSAPP_SEARCH_RESULT_PROGRESS,
	CSAPP_SEARCH_RESULT_TV,
	CSAPP_SEARCH_RESULT_RD,
	CSAPP_SEARCH_RESULT_TV_NUM,
	CSAPP_SEARCH_RESULT_RD_NUM,

	CSAPP_SEARCH_RESULT_ITEM_MAX
};

BOOL					Search_Condition_Status;
U8						Search_Condition_Sat_Index[MAX_MULTI_SAT];
/* For Tone Burst Control : By KB Kim 2011.02.28 */
U8                      BlindLnbMode[MAX_MULTI_SAT];
U8						u8Multi_Select_Sat;
U8						u8TPScan_Select_TP;
BOOL					Warning_Window_Status;
U8						u8Glob_Sat_Focus;
U8						u8Glob_TP_Focus;

tScanData				SearchData;

CSAPP_Applet_t CSApp_Install(void);
CSAPP_Applet_t CSApp_SearchResult(void);
CSAPP_Applet_t CSApp_Sat_Setting(void);
CSAPP_Applet_t CSApp_TP_Setting(void);

void MV_Draw_SatSelectBar(HDC hdc, int y_gap);
void MV_SatList_Bar_Draw(HDC hdc, int iCount, U8 u8Start_Point, MV_stSatInfo *Temp_SatData, U8 FocusKind);
void MV_TPList_Bar_Draw(HDC hdc, int iCount, U8 u8Start_Point, MV_stTPInfo *Temp_TPData, U8 FocusKind);
void MV_Install_Satlist_Item_Draw(HDC hdc, U8 u8Start_Point, U8 u8End_Point);
void MV_Install_List_Window( HWND hwnd );
void MV_Install_List_Window_Close( HWND hwnd );
void MV_Install_draw_help_banner(HDC hdc, MV_Menu_Kind mv_list_kind);
void MV_Draw_SelectBar(HDC hdc, int y_gap, eScanItemID esItem);
void MV_Draw_MenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem);
void MV_Draw_InstallMenuBar(HDC hdc);
int Sat_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
void MV_Draw_SatMenuList(HDC hdc);
void MV_Draw_SatMenuBar(HDC hdc, U8 u8Focuskind, eSatItemID esItem);
void MV_Draw_SatSetingSelectBar(HDC hdc, int y_gap, eSatItemID esItem);
int TP_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);
void MV_Draw_TPMenuList(HDC hdc);
void MV_Draw_TPMenuBar(HDC hdc, U8 u8Focuskind, eTPItemID esItem);
void MV_Draw_TPSetingSelectBar(HDC hdc, int y_gap, eTPItemID esItem);
void Draw_Progress_Bar(HDC hdc, U8 u8ProgressValue);
void Show_Signal(HDC hdc);
void MV_TP_Satlist_Item_Draw(HDC hdc, U8 u8Start_Point, U8 u8End_Point);
void MV_TP_List_Window( HWND hwnd );
void MV_TP_ADD_Window( HWND hwnd, U8 AddKind );
void MV_Add_FullItem_Draw(HDC hdc, U8 AddKind);
void MV_TP_List_Window_Close( HWND hwnd );
void MV_Addition_Window_Close( HWND hwnd );
BOOL CH_Add_Proc(HWND hwnd, WPARAM u8Key);
BOOL TP_Add_Proc(HWND hwnd, WPARAM u8Key);
void Install_Draw_Warning(HWND hwnd, U16 u16Message_index);
void MV_Install_Warning_Window_Close( HWND hwnd );
void MV_Search_Condition( HWND hwnd, U8 mode);
void MV_SearchCondition_Bar_Draw(HDC hdc, int iCount, U8 FocusKind);
#endif
/*   E O F  */


