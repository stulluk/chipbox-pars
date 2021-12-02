#ifndef  _CS_APP_COMMON_H_
#define  _CS_APP_COMMON_H_

#include "linuxos.h"

#include "database.h"
#include "av_zapping.h"
#include "mwpublic.h"
#include "mwlayers.h"

#include "mv_bitmap.h"
#include "mv_gui_interface.h"
#include "mv_cfg.h"
#include "csmpr_usb.h"
#include "userdefine.h"

/* For record config remove by request : KB Kim 2012.02.06 */
// #define RECORD_CONFG_SUPORT

#define SW_VERSION_FILE				"/usr/work0/app/sw.txt"
#define	DHCP_FILE					"/tmp/ifconfig.txt"
#define	DHCP_RESOLV_FILE			"/etc/resolv.conf"
#define DNS_CFG						"/usr/share/udhcpc/default.script"
#define	AUTO_DNS_CFG				"/usr/share/udchpc/auto_default.cfg"
#define	MANU_DNS_CFG				"/usr/share/udchpc/manual_default.cfg"
#define	IP_FILE						"/etc/network/interfaces"
#define UPGRADE_CFG_FILE			"/tmp/sw_list.txt"
#define PLUGIN_INFO_FILE			"/tmp/plugin.txt" /* By KB Kim for Plugin Setting : 2011.05.07 */
#define PLUGIN_SITE_FILE            "/usr/work1/plugin_site.txt" /* For Plugin Site List by File : KB Kim 2011.09.13 */
#define ETH0_STATUS_FILE			"/sys/class/net/eth0/carrier"

#define DHCP_FILE_MAX_COL			1024
#define	IP_LENGTH					4
#define	IP_ITEM						4

#define	MV_UNFOCUS					0
#define	MV_FOCUS					1

#define	MV_STATIC					0
#define	MV_SELECT					1
#define	MV_NUMERIC					2

#define	MV_HORIZONTAL				0
#define MV_VERTICAL					1

//#define             				SCREEN_CHANGE_DARK

#define PIN_MAX_NUM					4
#define MASTER_PIN              	"3548"

#define	CSAPP_R						0x1
#define	CSAPP_G						0x1
#define	CSAPP_B						0x1

//#define 	CSAPP_BLUE_COLOR		(U16)((CSAPP_B&0x1F)<<11)|((CSAPP_G&0x07)<<8)|((CSAPP_R&0x1F)<<3)|((CSAPP_G&0x38)>>3)
//#define 	CSAPP_BLACK_COLOR		RGBA2Pixel(hdc,22,22,22,0xff)
#define CSAPP_BLACK_COLOR			(U16)((CSAPP_R&0x1F)<<11)|((CSAPP_G&0x3F)<<5)|((CSAPP_B&0x1F))
//#define CSAPP_WHITE_COLOR                  RGBA2Pixel(hdc,244,244,244,0xff)

#define MVAPP_BLUE_COLOR 			RGBA2Pixel(hdc,0x0E, 0x17, 0xDC, 0x00)
#define MVAPP_BLACK_COLOR 			RGBA2Pixel(hdc,0x09, 0x09, 0x09, 0x00)
#define MVAPP_YELLOW_COLOR 			RGBA2Pixel(hdc,0xFF, 0xF2, 0x00, 0x00)
#define MVAPP_YELLOW_COLOR_ALPHA    RGBA2Pixel(hdc,0x50, 0x50, 0x20, 0xFF)
#define MVAPP_RED_COLOR 			RGBA2Pixel(hdc,0xDC, 0x17, 0x0E, 0x00)
#define MVAPP_DARK_RED_COLOR 		RGBA2Pixel(hdc,0x55, 0x10, 0x0A, 0x00)
#define MVAPP_DARKBLUE_COLOR 		RGBA2Pixel(hdc,0x00, 0x10, 0x27, 0x00)
#define MVAPP_BACKBLUE_COLOR 		RGBA2Pixel(hdc,0x0A, 0x15, 0xE0, 0x00)
#define MVAPP_POPUP_BACK_COLOR 		RGBA2Pixel(hdc,0x05, 0x10, 0xD9, 0x00)
#define MVAPP_SCROLL_GRAY_COLOR 	RGBA2Pixel(hdc,0x99, 0x95, 0x95, 0x00)
#define MVAPP_GRAY_COLOR 			RGBA2Pixel(hdc,0x45, 0x45, 0x45, 0x00)
#define MVAPP_BLACK_COLOR_ALPHA 	RGBA2Pixel(hdc,0x09, 0x09, 0x09, 0xFF)
#define MVAPP_ORANGE_COLOR 			RGBA2Pixel(hdc,0xFB, 0x88, 0x00, 0xFF)
//#define MVAPP_LIGHT_GREEN_COLOR 	RGBA2Pixel(hdc,0x2C, 0xF5, 0x10, 0xFF) // kb : Mar 25
#define MVAPP_LIGHT_GREEN_COLOR 	RGBA2Pixel(hdc,0x21, 0xC9, 0x00, 0xFF) // kb : Mar 25
#define MVAPP_DARK_GREEN_COLOR 	    RGBA2Pixel(hdc,0x00, 0x20, 0x0a, 0xFF) // kb : Mar 25
#define MVAPP_DARK_GRAY_COLOR 	    RGBA2Pixel(hdc,0x20, 0x20, 0x20, 0xFF) // kb : Mar 25
#define MVAPP_BLACK_GRAY_COLOR 	    RGBA2Pixel(hdc,0x0c, 0x0c, 0x0c, 0xFF) // kb : Mar 25
#define MVAPP_BLACK_CYAN_COLOR 	    RGBA2Pixel(hdc,0x0a, 0x0c, 0x26, 0xFF) // kb : Mar 25
#define MVAPP_DARK_CYAN_COLOR 	    RGBA2Pixel(hdc,0x05, 0x10, 0x22, 0xFF) // kb : Mar 25
#define MVAPP_TRANSPARENTS_COLOR 	RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0x00)
#define MVAPP_TEST_WHITE_COLOR 		RGBA2Pixel(hdc,0xFF, 0xFF, 0xFF, 0xFF)

#define	CSAPP_R_W					0xfe
#define	CSAPP_G_W					0xfe
#define	CSAPP_B_W					0xfe

#define CSAPP_WHITE_COLOR       	(U16)((CSAPP_R_W&0x1F)<<11)|((CSAPP_G_W&0x3F)<<5)|((CSAPP_B_W&0x1F))

#define	param_paint_all				11

#define MV_MENU_BACK_X				90
#define MV_MENU_BACK_Y				110
#define MV_MENU_BACK_DX				(CSAPP_OSD_MAX_WIDTH - MV_MENU_BACK_X*2)
#define MV_MENU_BACK_DY				(CSAPP_OSD_MAX_HEIGHT - MV_MENU_BACK_Y - 70)
#define	MV_INSTALL_MENU_X			120
#define	MV_INSTALL_MENU_Y			140
#define	MV_INSTALL_MENU_HEIGHT		20
#define	MV_INSTALL_MENU_HEIGHT2		24
#define	MV_INSTALL_MENU_BAR_H		30
#define	MV_INSTALL_MENU_YGAP		10
#define	MV_INSTALL_SELSAT_X			230
#define	MV_INSTALL_SELSAT_Y			(MV_INSTALL_SIGNAL_Y - MV_INSTALL_MENU_BAR_H*3 - 20)
#define	MV_INSTALL_SELSAT_DX		(CSAPP_OSD_MAX_WIDTH - MV_INSTALL_SELSAT_X*2)
#define	MV_INSTALL_SELSAT_DY		(MV_INSTALL_MENU_BAR_H*3)
#define	MV_INSTALL_SIGNAL_X			300
#define	MV_INSTALL_SIGNAL_Y			520
#define	MV_INSTALL_SIGNAL_DX		700
#define	MV_INSTALL_SIGNAL_YGAP		30
#define	MV_MENU_TITLE_DX			520
#define	MV_HELP_ICON_X				130
#define	MV_HELP_ICON_Y				600
#define	MV_HELP_ICON_DX4			255
#define	MV_HELP_ICON_DX3			340

#define MV_EPG_CN_BAR_DX            720 // kb : Mar 25
#define MV_EPG_CN_PIG_DX            320 // kb : Mar 25
#define MV_EPG_CN_PIG_DY            180 // kb : Mar 25

#define MV_FONT_WIDTH               12  // kb : Mar 25
#define MV_FONT_HEIGHT              24  // kb : Mar 25

// --- Channel edit PIG ----
#define MV_PIG_LIST_LEFT			90
#define MV_PIG_LIST_TOP				162
#define MV_PIG_LIST_DX				540
#define MV_PIG_LIST_DY				320
#define MV_PIG_LEFT					650
#define MV_PIG_TOP					162
#define MV_PIG_RIGHT				1190
#define MV_PIG_BOTTOM				482
#define MV_PIG_DX					( MV_PIG_RIGHT - MV_PIG_LEFT )
#define MV_PIG_DY					( MV_PIG_BOTTOM - MV_PIG_TOP )
#define	MV_PIG_OUTGAP				10

#define	WINDOW_TOP					150
#define	WINDOW_BOTTOM				550
#define	WINDOW_LEFT					350
#define	WINDOW_RIGHT				950
#define	WINDOW_DX					( WINDOW_RIGHT - WINDOW_LEFT )
#define WINDOW_DY					( WINDOW_BOTTOM - WINDOW_TOP )
#define WINDOW_OUT_GAP				10
#define WINDOW_ITEM_X				360
#define WINDOW_ITEM_Y				200
#define	WINDOW_ITEM_DX				580
#define	WINDOW_ITEM_DY				300

#define	WARNING_TOP					250
#define	WARNING_BOTTOM				430
#define	WARNING_LEFT				350
#define	WARNING_RIGHT				950
#define	WARNING_DX					( WARNING_RIGHT - WARNING_LEFT )
#define WARNING_DY					( WARNING_BOTTOM - WARNING_TOP )
#define WARNING_OUT_GAP				10
#define WARNING_ITEM_X				360
#define WARNING_ITEM_Y				300
#define	WARNING_ITEM_DX				580
#define	WARNING_ITEM_DY				120

#define	RE_LIST1_TOP				170
#define	RE_LIST1_BOTTOM				320
#define	RE_LIST1_LEFT				120
#define	RE_LIST1_RIGHT				320
#define	RE_LIST2_TOP				170
#define	RE_LIST2_BOTTOM				320
#define	RE_LIST2_LEFT				330
#define	RE_LIST2_RIGHT				530
#define	RE_LIST3_TOP				170
#define	RE_LIST3_BOTTOM				320
#define	RE_LIST3_LEFT				540
#define	RE_LIST3_RIGHT				840
#define	RE_LIST4_TOP				170
#define	RE_LIST4_BOTTOM				320
#define	RE_LIST4_LEFT				850
#define	RE_LIST4_RIGHT				1150

#define	RE_PROG_TOP					380
#define	RE_PROG_BOTTOM				420
#define	RE_PROG_LEFT				120
#define	RE_PROG_RIGHT				1150

#define	RE_INFO_TOP					450
#define	RE_INFO_BOTTOM				600
#define	RE_INFO_LEFT				120
#define	RE_INFO_RIGHT				RE_LIST3_RIGHT
#define	RE_LIST_GAP					10

#define	MAIN_ITEM_MAX				4
#define	MAIN_SUB_ITEM_MAX			7

#define	MV_BAR_UNFOCUS_COLOR		MVAPP_BLACK_COLOR
#define MV_BAR_FOCUS_CHAR_COLOR		MVAPP_BLACK_COLOR
#define	MV_BAR_UNFOCUS_CHAR_COLOR	CSAPP_WHITE_COLOR
#define MV_BAR_DISABLE_COLOR		MVAPP_BLACK_COLOR
#define MV_BAR_ENABLE_CHAR_COLOR	CSAPP_WHITE_COLOR
#define MV_BAR_DISABLE_CHAR_COLOR	MVAPP_BLACK_COLOR

#define	CAPTURE_TOP					(CSAPP_OSD_MAX_HEIGHT/3 - 80)
#define	CAPTURE_BOTTOM				(CSAPP_OSD_MAX_HEIGHT*(2/3) - 80)
#define	CAPTURE_LEFT				(CSAPP_OSD_MAX_WIDTH/3)
#define	CAPTURE_RIGHT				(CSAPP_OSD_MAX_WIDTH*(2/3))
#define	CAPTURE_DX					(CSAPP_OSD_MAX_WIDTH/3)
#define CAPTURE_DY					(CSAPP_OSD_MAX_HEIGHT/3 - 20)

#define MAX_FIND_STR_LENGTH			16
#define MAX_CH_NAME_LENGTH			16
#define	MAX_SAT_NAME_LANGTH			16
#define MAX_KEYCODE_LENGTH			16
#define MAX_NUMERIC_LENGTH			5
#define MAX_PID_LENGTH				4

#define SCROLL_BAR_DX				20

#define	LIST_ITEM_NUM				10

#define SIGNAL_TIMER				100

/* by kb : 20100406 */
#define ASCII_LF             0x0A
#define ASCII_CR             0x0D
#define ASCII_SPACE          0x20    /* ' ' */
#define ASCII_SHARP          0x23    /* '#' */
#define ASCII_0              0x30    /* '0' */
#define ASCII_9              0x39    /* '9' */
#define ASCII_EQUAL          0x3D    /* '=' */
#define ASCII_A              0x41    /* 'A' */
#define ASCII_F              0x46    /* 'F' */
#define ASCII_N              0x4E    /* 'N' */
#define ASCII_Q              0x51    /* 'Q' */
#define ASCII_Z              0x5A    /* 'Z' */
#define ASCII_BRACKET_L      0x5B    /* '[' */
#define ASCII_BRACKET_R      0x5D    /* ']' */
#define ASCII_a              0x61    /* 'a' */
#define ASCII_f              0x66    /* 'f' */
#define ASCII_n              0x6E    /* 'n' */
#define ASCII_q              0x71    /* 'q' */
#define ASCII_z              0x7A    /* 'z' */

enum
{
	CSAPP_MAINMENU_INSTALL=0,
	CSAPP_MAINMENU_SYSTEM,
	CSAPP_MAINMENU_MEDIA,
	CSAPP_MAINMENU_TOOL,
	CSAPP_MAINMENU_CH_EDIT,
	CSAPP_MAINMENU_EPG,  // kb : Mar 25,
	CSAPP_MAINMENU_GAME,
	CSAPP_MAINMENU_CALENDAR,
	CSAPP_MAINMENU_ITEM_MAX
};

typedef enum // kb : Mar 25
{
	EN_EPG_TYPE_SIMPLE = 0,
	EN_EPG_TYPE_SCHEDULED,
	EN_EPG_TYPE_MAX
}EN_EPG_TYPE;

typedef enum
{
	EN_ITEM_FOCUS_INSTALL = 0,
	EN_ITEM_FOCUS_SATSETTING,
	EN_ITEM_FOCUS_TPSETTING,
	EN_ITEM_FOCUS_INSTALL_MAX
}EN_INSTALLA_TOP_FOCUS;

typedef enum
{
	EN_ITEM_FOCUS_LANGUAGE = 0,
	EN_ITEM_FOCUS_TIME,
	EN_ITEM_FOCUS_SYS,
	EN_ITEM_FOCUS_AV,
	EN_ITEM_FOCUS_PARENTAL,
	EN_ITEM_FOCUS_NETWORK,
	EN_ITEM_FOCUS_SYSINFO,
	EN_ITEM_FOCUS_SYSTEM_MAX
}EN_SYSTEM_TOP_FOCUS;

typedef enum
{
	EN_ITEM_FOCUS_RECORD_FILE = 0,
#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
	EN_ITEM_FOCUS_RECORD_CONF,
#endif
	EN_ITEM_FOCUS_FILE_TOOL,
	EN_ITEM_FOCUS_USB_REMOVE,
	EN_ITEM_FOCUS_STORAGE_INFO,
	EN_ITEM_FOCUS_MEDIA_PLAYER,
	EN_ITEM_FOCUS_MEDIA_MAX
}EN_MEDIA_TOP_FOCUS;

typedef enum
{
	EN_ITEM_FOCUS_UPGRADE= 0,
	EN_ITEM_FOCUS_CI,
	EN_ITEM_FOCUS_CAS,
	EN_ITEM_FOCUS_BACKUP,
	EN_ITEM_FOCUS_RESTORE,
	EN_ITEM_FOCUS_PLUG_IN,
	EN_ITEM_FOCUS_FAC_RESET,
	EN_ITEM_FOCUS_TOOLS_MAX
}EN_TOOLS_TOP_FOCUS;

typedef enum
{
	EN_ITEM_SIGNAL_LEVEL = 0,
	EN_ITEM_NORMAL_BAR_LEVEL,
	EN_ITEM_SIGNAL_LEVEL_NONAME,
	EN_ITEM_5_BAR_LEVEL,
	EN_ITEM_8_BAR_LEVEL,
	EN_ITEM_10_BAR_LEVEL,
	EN_ITEM_10_BAR_LEVEL_NONAME,
	EN_ITEM_20_BAR_LEVEL,
	EN_ITEM_PROGRESS_BAR_LEVEL,
	EN_TTEM_PROGRESS_NO_IMG,
	EN_TTEM_PROGRESS_NO_IMG_MP3,
	EN_TTEM_PROGRESS_NO_IMG_BMP,
	EN_ITEM_CHEDIT_SIGNAL_LEVEL,
	EN_ITEM_LEVEL_BAR_MAX
}EN_LEVEL_BAR_KIND;

typedef enum
{
	EN_ITEM_SAT_LIST = 0,
	EN_ITEM_CHANNEL_LIST,
	EN_ITEM_TP_LIST,
	EN_ITEM_EPG_LIST,
	EN_ITEM_LIST_MAX
}EN_LIST_KIND;

typedef struct
{
	char	Re_IP[IP_LENGTH];
} stIPStr_Struct;

typedef enum
{
	CSAPP_IP_IPADDR=0,
	CSAPP_IP_SUBNET,
	CSAPP_IP_GATEWAY,
	CSAPP_IP_DNS1,
	CSAPP_IP_DNS2,
	CSAPP_IP_DNS3,
	CSAPP_IP_MAX
} eMV_IP_FILE_Items;

typedef enum
{
	FILE_OK=0,
	FILE_NOFILE,
	FILE_READ_FAIL,
	FILE_NO_NAME
} MV_File_Return;

typedef enum CSAPP_Applet_s {
	CSApp_Applet_Exit=0,
	CSApp_Applet_Error,
	CSApp_Applet_MainMenu,

	CSApp_Applet_TVList,
	CSApp_Applet_RDList,
	CSApp_Applet_TVFAVList,
	CSApp_Applet_RADIOFAVList,
	CSApp_Applet_TVSATList,
	CSApp_Applet_RADIOSATList,
	CSapp_Applet_SystemSetting,

	CSapp_Applet_EditTV,
	CSapp_Applet_EditRadio,
	CSApp_Applet_EditTVFAV,
	CSApp_Applet_EditRADIOFAV,
	CSApp_Applet_EditTVSAT,
	CSApp_Applet_EditRADIOSAT,

	CSApp_Applet_TV_EPG,
	CSApp_Applet_Radio_EPG,
	CSApp_Applet_FAV_TV_EPG,
	CSApp_Applet_FAV_Radio_EPG,
	CSApp_Applet_SAT_TV_EPG,
	CSApp_Applet_SAT_Radio_EPG,
#ifdef DAILY_EPG
	CSApp_Applet_Daily_TV_EPG,
	CSApp_Applet_Daily_Radio_EPG,
	CSApp_Applet_Daily_FAV_TV_EPG,
	CSApp_Applet_Daily_FAV_Radio_EPG,
	CSApp_Applet_Daily_SAT_TV_EPG,
	CSApp_Applet_Daily_SAT_Radio_EPG,
#endif
	CSApp_Applet_Desktop,
	CSApp_Applet_Banner,
	CSApp_Applet_Normal,
	CSApp_Applet_Audio,
	CSAPP_Applet_Teletext,
	CSAPP_Applet_Subtitle,

	CSApp_Applet_Install,
	CSAPP_Applet_Install_Result,
	CSApp_Applet_Sat_Setting,
	CSApp_Applet_TP_Setting,
	CSAPP_Applet_D_motor,
	CSAPP_Applet_USALS,
	CSAPP_Applet_UniCable,

	CSApp_Applet_Language,
	CSApp_Applet_AVSetting,
	CSApp_Applet_TimeSetting,
	CSApp_Applet_PinSetting,
	CSApp_Applet_NetSetting,
	CSapp_Applet_SystemInfo,

	CSApp_Applet_Rec_File,
#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
	CSApp_Applet_Rec_Config,
#endif
	CSApp_Applet_File_Tool,
	CSApp_Applet_USB_Remove,
	CSApp_Applet_Storage_Info,
	CSApp_Applet_Media_Player,

	CSApp_Applet_Upgrade,
	CSApp_Applet_CI,
	CSApp_Applet_CAS,
	CSApp_Applet_Backup,
	CSApp_Applet_Restore,
	CSApp_Applet_PlugIn,
	CSApp_Applet_Reset,
	CSApp_Applet_USB,
	CSAPP_Applet_Video_Control,
	CSAPP_Applet_Finder,

	CSAPP_Applet_KeyEdit,
	CSAPP_Applet_Desk_CH_Edit,
	CSAPP_Applet_Recall,
	CSApp_Applet_ExtInfo,
	CSApp_Applet_Ext_TVList,
	CSApp_Applet_Ext_RDList,
	CSApp_Applet_Ext_TVFAVList,
	CSApp_Applet_Ext_RADIOFAVList,
	CSApp_Applet_Ext_TVSATList,
	CSApp_Applet_Ext_RADIOSATList,
	CSApp_Applet_Viewer,
	CSApp_Applet_Pvr_Record,
/*************v38..........***************/
	CSApp_Applet_Pvr_Streaming,
/*****************************************/
	CSApp_Applet_Pvr_Player,
	CSApp_Applet_Timer,
	CSApp_Applet_Push,
	CSApp_Applet_Calendar,
	CSApp_Applet_Change_Fav,
	CSApp_Applet_OSCAM_Setting,
#ifdef SMART_PHONE
	CSApp_Applet_Smart_OSD,
#endif
	CSApp_Applet_Sleep

}CSAPP_Applet_t;

//U16	MAIN_MENU_STR[MAIN_ITEM_MAX][MAIN_SUB_ITEM_MAX];

typedef struct
{
	int	x;
	int	y;
	int	dx;
	int	dy;
}tComboList_Element_Rect;

typedef struct
{
	int	x;
	int	dx;
}tComboList_Field_Rect;

typedef struct
{
	tComboList_Element_Rect	element;
	U8						field_num;
	tComboList_Field_Rect	*element_fields;
}tComboList_Element;

typedef struct
{
	U8					element_num;
	tComboList_Element	*first_element;
}tComboList;

typedef enum
{
	CSBMP_MAIN_MENU=0,
	CSBMP_EPG_MENU,
	CSBMP_TVLIST_MENU,
	CSBMP_RDLIST_MENU,
	CSBMP_BANNER
}eBitMapIndex;

typedef enum
{
	MV_WINDOW_UNKNOWN=0,
	MV_WINDOW_WARNING,
	MV_WINDOW_REPORT,
	MV_WINDOW_CERTIFY,
	MV_WINDOW_USB_CONNECT,
	MV_WINDOW_USB_DISCONNECT,
	MV_WINDOW_USB_MOUNT,
	MV_WINDOW_USB_MOUNT_FAIL,
	MV_WINDOW_USB_UNMOUNT,
	MV_WINDOW_USB_UNMOUNT_FAIL,
	MV_WINDOW_NO_FAV_CHANNEL,
	MV_WINDOW_SMART_CARD_INSERT,
	MV_WINDOW_SMART_CARD_REMOVE,
	MV_WINDOW_PLUGIN_INSTALLED,
	MV_WINDOW_STREAMING_STARTED
}ePopupIndex;

typedef struct
{
	U16			u16Ch_Index;
	char		acMac_Add[20];
	char		acCH_Name[16];
	char		acSat_Name[16];
	U16			u16Channel_Number;
	U16			u16Service_Id;
	U16			u16TP_Freq;
	U8			u8TP_Pol;
	U16			u16TP_Symbol;
	U16			u16Sat_Logitude;
	char		acSt_Time[32];
	time_t		long_StTime;
	char		acEnd_Time[32];
	U8			u8SetID;
	U8			u8Mvapp_Ver;
	U8          u8Strength; /*	For New Rating function By KB KIm 2011.08.01 */
	U8          u8Quality; /*	For New Rating function By KB KIm 2011.08.01 */
} stSend_Channel_Data;

BITMAP				Capture_bmp;	/* Capture Background UI, Use Satellite list or TP list window in Installation & Satellite Setting & TP Setting */
BITMAP				Capture_bmp2;
BITMAP				WarningCapture_bmp;	/* Capture Background UI, Use Satellite list or TP list window in Installation & Satellite Setting & TP Setting */
CSAPP_Applet_t 		b8Last_App_Status;

stIPStr_Struct		IP_Addr[IP_ITEM];
stIPStr_Struct		IP_SubNet[IP_ITEM];
stIPStr_Struct		IP_Gateway[IP_ITEM];
stIPStr_Struct		IP_DNS1[IP_ITEM];
stIPStr_Struct		IP_DNS2[IP_ITEM];
stIPStr_Struct		IP_DNS3[IP_ITEM];
U32 MvConvertTextforWindow(char *src, char **dest, U32 charPerLine, U32 dataLength); // by kb : 20100407

void CsPalyAVbyProgramIdx(U16	Current_Service);

U32	ScalerWidthPixel(U32	Pixel);
U32	ScalerHeigthPixel(U32	Pixel);

U16 get_page_count(U16 total, U8 nbItemPerPage);
U16 get_focus_line(U16 *page, U16 current, U8 nbItemPerPage);

void ComboList_Create( const tComboList_Element * first, U8 element_num );
void ComboList_UpdateAll( HWND hWnd );
void ComboList_Update_Element( HWND hWnd, U8 element );
void ComboList_Update_Field( HWND hWnd, U8 element, U8 field );
void ComboList_Destroy( void );


char Key_to_Ascii(U32 key);
void PinDlg_SetStatus(BOOL status);
BOOL PinDlg_GetStatus(void);
void PinDlg_TreatKey(HWND hWnd, U32 key, int *input_count, char * input_keys);
BOOL    Pin_Verify(char * inputs);
BOOL    Pin_CheckValid(char * inputs);

void Pin_Set_Input_Status(BOOL status);
BOOL Pin_Get_Input_Status(void);


//void VideoFormatNotify(eCS_MW_PIPE_TYPE PipeType, tCS_MW_MSG_INFO MsgInfo);

BITMAP *GetBitMapData(eBitMapIndex idx);

void APP_SetMainMenuStatus(BOOL status);
BOOL APP_GetMainMenuStatus(void);

void CS_APP_SetLastUnlockServiceIndex(U16 last_service);
U16 CS_APP_GetLastUnlockServiceIndex(void);

void CS_APP_SetFirstInDesktop(BOOL first_in);
BOOL CS_APP_GetFirstInDesktop(void);

void MV_DRAWING_MENUBACK(HWND hwnd, U8 MenuKind, U8 SubKind);
void MV_System_draw_help_banner(HDC hdc, EN_SYSTEM_TOP_FOCUS emvkind);
void MV_Media_draw_help_banner(HDC hdc, EN_MEDIA_TOP_FOCUS emvkind);
void MV_TOOLS_draw_help_banner(HDC hdc, EN_TOOLS_TOP_FOCUS emvkind);
void MV_Draw_LevelBar(HDC hdc, RECT *TmpRect, U16 LevelValue, EN_LEVEL_BAR_KIND LevelKind);
void MV_Draw_Time_Progress_Bar(HDC hdc, RECT *TmpRect, U16 LevelValue, U16 Move_Value, char *acStart, char *acEnd, time_t Pogress_Time);
void MV_Warning_Report_Window_Open( HWND hwnd, ePopupIndex MenuKind /*, U8 SubKind*/ );
void MV_Warning_Report_Window_Close( HWND hwnd );
BOOL MV_Get_Report_Window_Status(void);
void MV_Draw_PageScrollBar(HDC hdc, RECT Scroll_Rect, U16 u16Now_Point, U16 totalNumber, U16 numberInPage); // by kb : 20100407
void MV_Draw_ScrollBar(HDC hdc, RECT Scroll_Rect, U16 u8Now_Point, U16 Mv_Install_SatFocus, EN_LIST_KIND enListKind, U8 enScrollKind);
CSAPP_Applet_t get_windown_status(void);
CSAPP_Applet_t get_prev_windown_status(void);
void set_windown_status(CSAPP_Applet_t win_status);
void MainMenuSetSubtApplet(U8 Idx, U8 Subidx);

void MV_Parser_IP_Value( char *Temp, char *tempSection );
void MV_Parser_IP_DataValue( U8 u8IPIndex , char *Temp );
U8 MV_IPData_Namecheck(char *tempSection);
MV_File_Return MV_Load_IPData_File(void);
MV_File_Return MV_Save_IPData_File(void);
MV_File_Return MV_Save_DNS_File(void);
void MV_Setting_Manual_Network(void);
// BOOL Str2Hex(U8 *data, U8 *value);
CSAPP_Applet_t CSApp_Disecq_Motor(void);
CSAPP_Applet_t CSApp_Usals_Motor(void);
CSAPP_Applet_t CSApp_UniCable(void);
CSAPP_Applet_t CSApp_DeskCh_Edit(void);
CSAPP_Applet_t CSApp_Recall(void);
CSAPP_Applet_t CSApp_Upgreade(void);
CSAPP_Applet_t CSApp_Restore(void);
CSAPP_Applet_t CSApp_PlugIn(void);
CSAPP_Applet_t CSApp_Video_Set(void);
CSAPP_Applet_t CSApp_Finder(void);
CSAPP_Applet_t CSApp_KeyEdit(void);

CSAPP_Applet_t CSApp_Check_Pass(void);

int Send_ch_data_Init(void);
BOOL MV_Get_Network_Status(void);

void set_prev_windown_status(CSAPP_Applet_t win_status);
int MV_Get_USB_Info(U8 u8Partition_No, struct f_size *stfile_size);

#ifdef CHECK_CH_WATCH
int MV_Get_Current_Channel_Time(void);
void MV_ReSet_Current_Channel_Time(void);
void MV_Set_Current_Channel_Data(U16 u16Channel_Index, U16 u16Channel_Num);
void *MV_Send_Current_Channel_Data( void *param );
void MV_Set_Current_Ch_Data(U16 u16Check_index, MV_stSatInfo Temp_SatInfo, MV_stTPInfo Temp_TPInfo);
BOOL MV_Check_Current_Ch_Data(U16 u16Check_index, MV_stSatInfo Temp_SatInfo, MV_stTPInfo Temp_TPInfo);
void MV_OS_Get_Time_to_MJD_UTC_Date_Time(U16 *u16MJD, U16 *u16UTC, tCS_DT_Date *stDate, tCS_DT_Time *stTime);
#endif // #ifdef CHECK_CH_WATCH
#endif

