#ifndef  _CS_MV_MENU_CTR_H_
#define  _CS_MV_MENU_CTR_H_

#include "linuxos.h"

#include "database.h"
#include "eit_engine.h"

#include "mwsetting.h"
#include "mwsvc.h"
#include "mwlayers.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mv_gui_interface.h"
#include "ui_common.h"

//#define MANU_ITEM_6
//#define HD_OSD

#define				NOTFOCUS			2

#define				MAIN_ANI_MAX		3	/* Animation count of image kind */
#define				MAIN_ANI_COUNT		1	/* Animation roof count */

#define				INST_BMP				0
#define				SYST_BMP				1
#define				RECO_BMP				2
#define				TOOL_BMP				3

#define				ANI_COUNT				4

#define				BMP_MENU_BACK_WIDTH		MV_BMP[MVBMP_MAIN_BACK].bmWidth
#define				BMP_MENU_BACK_STARTX	181
#define				BMP_MENU_GAP			( (( CSAPP_OSD_MAX_WIDTH - BMP_MENU_BACK_STARTX*2 ) - ( MV_BMP[MVBMP_SD_INST1].bmWidth * 4 )) / 3 )

#define				MAX_DX					CSAPP_OSD_MAX_WIDTH
#define				MAX_DY					CSAPP_OSD_MAX_HEIGHT
#define				BAR_MENU_GAP			34
#define 			TXT_MENU_X				70
#define 			TXT_MENU_Y				377
#define 			TXT_MENU_Dx				200
#define 			TXT_MENU_Dy				30
#define 			BMP_MENU_X				BMP_MENU_BACK_STARTX
#define 			BMP_MENU_DY				200
#define 			BMP_MENU_Y				( MAX_DY - BMP_MENU_DY )
#define				BMP_MENU_ICON_Y			( BMP_MENU_Y + 21 )
#define				DX_WIDTH_GAP			4
#define				SUB_MENU_X				465
#define				SUB_MENU_DX				378
#define				BAR_MENU_X				BMP_MENU_X
#define				BAR_MENU_Y				BMP_MENU_Y
#define				SUB_MENU_BAR_DX			362
#define				MENU_CAP_HEIGHT			(BAR_MENU_Y - (EN_ITEM_FOCUS_TOOLS_MAX * BAR_MENU_GAP + 20 ))

#define				SEQ_UP_SLIDE			1
#define				ALL_UP_SLIDE			2
#define				LEFT_SIDE_SLIDE			3
#define				RIGHT_SIDE_SLIDE		4
#define				NO_SLIDE				5
#define				FILL_COLOR				COLOR_transparent

BITMAP       		mv_main_bmp_focus[MAIN_ITEM_MAX][MAIN_ANI_MAX];

typedef struct
{
	BITMAP 			SubBmp;
	U8				u8Focus_MAX;
	U8				u8Sub_Focus;
} stFocus_Item;

stFocus_Item		stFocus[MAIN_ITEM_MAX];

void MV_Loading_Main_Image( void );
void MV_Mainmenu_Draw( HWND hwnd, U8 u8MainFocus_Item );
#if 0
void MV_Mainmenu_Focus( /*HWND hwnd,*/ HDC	hdc, U8 u8FocusIndex);
#else
void MV_Mainmenu_Focus(HWND hwnd, U8 u8FocusIndex);
#endif
void MV_Mainmenu_UnFocus( HDC hdc, U8 u8UnFocusIndex );
void MV_Submenu_Focus( HDC hdc, U8 u8FocusIndex/*, U8 u8Key*/);
void MV_Submenu_UnFocus( HDC	hdc, U8 u8FocusIndex);
void MV_SubmenuBar_Focus( HDC	hdc, U8 u8FocusIndex, U8 u8SubIndex, U8 u8Kind );
void MV_Set_Submenu_Focus(U8 Idx, U8 u8Focus);
U8 MV_Get_Submenu_Focus(void);

#endif  // #ifndef  _CS_MV_MENU_CTR_H_

