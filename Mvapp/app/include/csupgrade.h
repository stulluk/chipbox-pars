#ifndef  _CS_UPGRADE_H_
#define  _CS_UPGRADE_H_

CSAPP_Applet_t	CSApp_Upgreade(void);

#define UPGRADE_TITLE_X				440
#define UPGRADE_TITLE_Y				200
#define	UPGRADE_TITLE_DX			400
#define	UPGRADE_TITLE_DY			30

#ifdef USE_TWO_STEP
#define UPGRADE_CATEGORY_X			230
#else
#define UPGRADE_CATEGORY_X			460
#endif

#define	UPGRADE_CATEGORY_Y			260
#define	UPGRADE_CATEGORY_DX			370
#define	UPGRADE_CATEGORY_DY			260
#define UPGRADE_LEFT_LINE_X			( UPGRADE_CATEGORY_X + UPGRADE_CATEGORY_DX + 50 )
#define UPGRADE_LEFT_LINE_DX		50
#define UPGRADE_LEFT_LINE_Y1		( UPGRADE_CATEGORY_Y + 15 )
#define UPGRADE_LEFT_LINE_Y2		( UPGRADE_LEFT_LINE_Y1 + MV_INSTALL_MENU_BAR_H )
#define UPGRADE_LEFT_LINE_Y3		( UPGRADE_LEFT_LINE_Y2 + MV_INSTALL_MENU_BAR_H )
#define UPGRADE_MID_LINE_X			( UPGRADE_LEFT_LINE_X + UPGRADE_LEFT_LINE_DX )
#define UPGRADE_RIGHT_LINE_X		( UPGRADE_MID_LINE_X + UPGRADE_LINE_OFFSET )
#define UPGRADE_RIGHT_LINE_Y4		( UPGRADE_LEFT_LINE_Y3 + MV_INSTALL_MENU_BAR_H )
#define UPGRADE_RIGHT_LINE_Y5		( UPGRADE_RIGHT_LINE_Y4 + MV_INSTALL_MENU_BAR_H )
#define UPGRADE_LINE_OFFSET			4
#define UPGRADE_KIND_X				( UPGRADE_CATEGORY_X + 500 )
#define	UPGRADE_KIND_Y				( UPGRADE_CATEGORY_Y )
#define	UPGRADE_KIND_DX				( UPGRADE_CATEGORY_DX )
#define	UPGRADE_KIND_DY				( UPGRADE_CATEGORY_DY )

#define	UPGRADE_WARNING_TOP			250
#define	UPGRADE_WARNING_BOTTOM		430
#define	UPGRADE_WARNING_LEFT		400
#define	UPGRADE_WARNING_RIGHT		900
#define	UPGRADE_WARNING_DX			( UPGRADE_WARNING_RIGHT - UPGRADE_WARNING_LEFT )
#define UPGRADE_WARNING_DY			( UPGRADE_WARNING_BOTTOM - UPGRADE_WARNING_TOP )
#define UPGRADE_WARNING_OUT_GAP		10
#define UPGRADE_WARNING_ITEM_X		410
#define UPGRADE_WARNING_ITEM_Y		300
#define	UPGRADE_WARNING_ITEM_DX		480
#define	UPGRADE_WARNING_ITEM_DY		120

#define UPGRADE_MESSAGE_X			300
#define UPGRADE_MESSAGE_Y			500
#define UPGRADE_MESSAGE_Y2			( UPGRADE_MESSAGE_Y - ( 7 * MV_INSTALL_MENU_BAR_H ))
#define UPGRADE_MESSAGE_DX			700
#define UPGRADE_MESSAGE_DY			120
#define UPGRADE_MESSAGE_DY2			( UPGRADE_MESSAGE_DY + ( 7 * MV_INSTALL_MENU_BAR_H ))

typedef enum
{
	EN_UPGRADE_KIND_FULL_SYSTEM = 0,
	EN_UPGRADE_KIND_DATABASE,
	EN_UPGRADE_KIND_DEFAULT_DATA,
	EN_UPGRADE_KIND_MAX
}EN_UPGRADE_KIND;

typedef enum
{
	EN_UPGRADE_METHOD_USB,
	EN_UPGRADE_METHOD_NETWORK,
	EN_UPGRADE_METHOD_MAX
}EN_UPGRADE_METHOD;

/* By KB Kim for Plugin Setting : 2011.05.07 */
#ifndef PLUGIN_MENU
typedef struct
{
	U8		u8Set_Version;
	U8 		u8SW_Version;
	char	acFile_Location[1024];
	char	acFile_Name[256];
	char	acFile_size[32];
	char	acFile_date[32];
} stMV_Upgrade_Info;

void MV_Draw_Msg_Download(HWND hwnd, U32 u32Message);
void MV_Close_Msg_Download(HWND hwnd);
int Download_Init(void);
void *Download_Task( void *param );
void Download_Stop(void);
#endif
#endif


