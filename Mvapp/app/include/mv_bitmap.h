#ifndef __MV_BITMAP_H
#define __MV_BITMAP_H

#include "mv_gui_interface.h"

typedef enum
{
/* 000 */	MVBMP_BOOT_LOADING = 0,			/* channel list board left top round icon */
/* 001 */	MVBMP_BOARD_TOP_LEFT,			/* channel list board left top round icon */
/* 002 */	MVBMP_BOARD_TOP_RIGHT,			/* channel list board right top round icon */
/* 003 */	MVBMP_BOARD_BOT_LEFT,			/* channel list board left bottom round icon */
/* 004 */	MVBMP_BOARD_BOT_RIGHT,			/* channel list board right bottom round icon */
/* 005 */	MVBMP_MENU_TITLE_LEFT,			/* main sub menu title background bar left icon */
/* 006 */	MVBMP_MENU_TITLE_MID,			/* main sub menu title background bar body icon */
/* 007 */	MVBMP_MENU_TITLE_RIGHT,			/* main sub menu title background bar right icon */

/* 008 */	MVBMP_LEFT_ARROW,				/* menu bar left arrow icon */
/* 009 */	MVBMP_RIGHT_ARROW,				/* menu bar right arrow icon */
/* 010 */	MVBMP_BLUE_BUTTON,				/* menu bottom function blue icon */
/* 011 */	MVBMP_GREEN_BUTTON,				/* menu bottom function green icon */
/* 012 */	MVBMP_RED_BUTTON,				/* menu bottom function red icon */
/* 013 */	MVBMP_YELLOW_BUTTON,			/* menu bottom function yellow icon */
/* 014 */	MVBMP_GREEN_SIGNAL,				/* signal green bar */
/* 015 */	MVBMP_RED_SIGNAL,				/* signal red bar */
/* 016 */	MVBMP_ORANGE_SIGNAL,			/* signal orange bar */
/* 017 */	MVBMP_SD_INST1,					/* main installation menu animation first icon */
/* 018 */	MVBMP_SD_INST2,
/* 019 */	MVBMP_SD_INST3,
/* 020 */	MVBMP_SD_MEDIA1,				/* main media menu animation first icon */
/* 021 */	MVBMP_SD_MEDIA2,
/* 022 */	MVBMP_SD_MEDIA3,
/* 023 */	MVBMP_SD_SYSTEM1,				/* main system menu animation first icon */
/* 024 */	MVBMP_SD_SYSTEM2,
/* 025 */	MVBMP_SD_SYSTEM3,
/* 026 */	MVBMP_SD_TOOL1,					/* main tool menu animation first icon */
/* 027 */	MVBMP_SD_TOOL2,
/* 028 */	MVBMP_SD_TOOL3,
/* 029 */	MVBMP_YELLOW_BAR_LEFT,			/* menu select yellow bar left icon */
/* 030 */	MVBMP_YELLOW_BAR_MIDDLE,
/* 031 */	MVBMP_YELLOW_BAR_RIGHT,
/* 032 */	MVBMP_MAIN_SELECT_BAR,			// main menu sub menu select bar
/* 033 */	MVBMP_MAIN_UNSELECT_BAR,		// main menu sub menu unselect bar
#if 0
/* 034 */	MVBMP_SD_INST,					// installation menu top Icon
#endif
/* 035 */	MVBMP_Y_ENTER,					/* list bar enter Icon */
/* 036 */	MVBMP_Y_NUMBER,					/* list bar numeric Icon */
/* 037 */	MVBMP_F2_BUTTON,				/* menu bottom function f2 icon */
/* 038 */	MVBMP_TOP_MENU_BACK_TOP,		/* main menu sub menu background top */
/* 039 */	MVBMP_GRAY_SIGNAL,				/* signal background */
#if 0
/* 040 */	MVBMP_SD_SYST,					/* system menu top Icon */
/* 041 */	MVBMP_SD_MEDI,					/* media menu top Icon */
/* 042 */	MVBMP_SD_TOOL,					/* tool menu top Icon */
#endif
/* 043 */	MVBMP_UPARROW,					/* list scroll bar uparrow */
/* 044 */	MVBMP_DOWNARROW,				/* list scroll bar downarrow */
/* 045 */	MVBMP_CHLIST_SELBAR,			/* channel list select bar */
/* 046 */	MVBMP_BLACK_BUTTON,				/* menu bottom function black icon */
/* 047 */	MVBMP_GRAY_BUTTON,				/* menu bottom function gray icon */
/* 048 */	MVBMP_CHLIST_INFO_ICON,			/* channel list board bottom infomation icon */
/* 049 */	MVBMP_CHLIST_SCRAMBLE_ICON,		/* channel list scramble icon */
/* 050 */	MVBMP_CHLIST_FAVORITE_ICON,		/* channel list favorite icon */
/* 051 */	MVBMP_CHLIST_LOCK_ICON,			/* channel list lock icon */
/* 052 */	MVBMP_CHLIST_NSCRAMBLE_ICON,	/* channel list scramble normal icon */
/* 053 */	MVBMP_CHLIST_NFAVORITE_ICON,	/* channel list favorite normal icon */
/* 054 */	MVBMP_CHLIST_NLOCK_ICON,		/* channel list lock normal icon */
/* 055 */	MVBMP_UNFOCUS_KEYPAD,			/* unfocus keypad button */
/* 056 */	MVBMP_FOCUS_KEYPAD,				/* focus keypad button */
/* 057 */	MVBMP_INFO_BANNER_INFO_ICON,	/* infomation banner flat down info i icon */
/* 058 */	MVBMP_INFO_DOLBY_FO_ICON,		/* infomation banner dolby focus icon */
/* 059 */	MVBMP_INFO_DOLBY_UNFO_ICON,		/* infomation banner dolby unfocus icon */
/* 060 */	MVBMP_INFO_EPG_FO_ICON,			/* infomation banner epg focus icon */
/* 061 */	MVBMP_INFO_EPG_UNFO_ICON,		/* infomation banner epg unfocus icon */
/* 062 */	MVBMP_INFO_FAV_FO_ICON,			/* infomation banner favorite focus icon */
/* 063 */	MVBMP_INFO_FAV_UNFO_ICON,		/* infomation banner favorite unfocus icon */
/* 064 */	MVBMP_INFO_HD_FO_ICON,			/* infomation banner HD broadcast focus icon */
/* 065 */	MVBMP_INFO_HD_UNFO_ICON,		/* infomation banner HD broadcast unfocus icon */
/* 066 */	MVBMP_INFO_SCRAM_FO_ICON,		/* infomation banner scramble focus icon */
/* 067 */	MVBMP_INFO_SCRAM_UNFO_ICON,		/* infomation banner scramble unfocus icon */
/* 068 */	MVBMP_INFO_SUBT_FO_ICON,		/* infomation banner subtitle focus icon */
/* 069 */	MVBMP_INFO_SUBT_UNFO_ICON,		/* infomation banner subtitle unfocus icon */
/* 070 */	MVBMP_INFO_TTX_FO_ICON,			/* infomation banner teletext focus icon */
/* 071 */	MVBMP_INFO_TTX_UNFO_ICON,		/* infomation banner teletext unfocus icon */
/* 072 */	MVBMP_MUTE_ICON,				/* Live screen mute icon */
/* 073 */	MVBMP_VOLUME_ICON,				/* Live screen volume icon */
/* 074 */	MVBMP_PAUSE_ICON,				/* Live screen pause icon */
/* 075 */	MVBMP_0_ICON,					/* Number 0 icon */
/* 076 */	MVBMP_1_ICON,					/* Number 1 icon */
/* 077 */	MVBMP_2_ICON,					/* Number 2 icon */
/* 078 */	MVBMP_3_ICON,					/* Number 3 icon */
/* 079 */	MVBMP_4_ICON,					/* Number 4 icon */
/* 080 */	MVBMP_5_ICON,					/* Number 5 icon */
/* 081 */	MVBMP_6_ICON,					/* Number 6 icon */
/* 082 */	MVBMP_7_ICON,					/* Number 7 icon */
/* 083 */	MVBMP_8_ICON,					/* Number 8 icon */
/* 084 */	MVBMP_9_ICON,					/* Number 9 icon */
/* 085 */	MVBMP_CHLIST_DEL_ICON,			/* channel edit delete icon */
/* 086 */	MVBMP_CHLIST_NDEL_ICON,			/* channel edit delete normal icon */
/* 087 */	MVBMP_CHLIST_MOVE_ICON,			/* channel edit move icon */
/* 088 */	MVBMP_SCAN_ANI1,				/* animation no.1 on scan time */
/* 089 */	MVBMP_SCAN_ANI2,				/* animation no.2 on scan time */
/* 090 */	MVBMP_SCAN_ANI3,				/* animation no.3 on scan time */
/* 091 */	MVBMP_SCAN_ANI4,				/* animation no.4 on scan time */
/* 092 */	MVBMP_SCAN_ANI5,				/* animation no.5 on scan time */
/* 093 */	MVBMP_CHLIST_NMOVE_ICON,		/* channel edit list move normal icon */
/* 094 */	MVBMP_CHLIST_HD_ICON,			/* channel edit list HD icon */
/* 095 */	MVBMP_CHLIST_NHD_ICON,			/* channel edit list HD normal icon */
/* 096 */	MVBMP_CHLIST_CHECK_ICON,		/* channel edit list HD icon */
/* 097 */	MVBMP_CHLIST_NCHECK_ICON,		/* channel edit list HD normal icon */
/* 098 */	MVBMP_DESCTOP_MSG_PANEL,		/* desktop dispaly message ( no signal, scramble .... ) background pannel */
/* 099 */	MVBMP_OK_ICON,					/* ok button icon */
/* 100 */	MVBMP_EXIT_ICON,				/* exit button icon */
/* 101 */	MVBMP_BLUE_GROUND,				/* blue back ground */
/* 102 */	MVBMP_BLACK_GROUND,				/* black back ground */
/* 103 */ 	MVBMP_DIR_FOLDER,				/* folder icon */
/* 104 */	MVBMP_MOVIE_FILE,				/* movie and animation files icon */
/* 105 */	MVBMP_MUSIC_FILE,				/* music files icon */
/* 106 */	MVBMP_IMAGE_FILE,				/* image files icon */
/* 107 */	MVBMP_TEXT_FILE,				/* text files icon */
/* 108 */	MVBMP_TS_FILE,					/* ts recorded files icon */
/* 109 */	MVBMP_NORMAL_FILE,				/* unknown files icon */
/* 110 */	MVBMP_KEY_0_ICON,				/* Remocon Key 0 */
/* 111 */	MVBMP_KEY_1_ICON,				/* Remocon Key 1 */
/* 112 */	MVBMP_KEY_2_ICON,				/* Remocon Key 2 */
/* 113 */	MVBMP_KEY_3_ICON,				/* Remocon Key 3 */
/* 114 */	MVBMP_KEY_4_ICON,				/* Remocon Key 4 */
/* 115 */	MVBMP_KEY_5_ICON,				/* Remocon Key 5 */
/* 116 */	MVBMP_KEY_6_ICON,				/* Remocon Key 6 */
/* 117 */	MVBMP_KEY_7_ICON,				/* Remocon Key 7 */
/* 118 */	MVBMP_KEY_8_ICON,				/* Remocon Key 8 */
/* 119 */	MVBMP_KEY_9_ICON,				/* Remocon Key 9 */
/* 120 */	MVBMP_KEY_MUTE_ICON,			/* Remocon Key mute */
/* 121 */	MVBMP_KEY_PREV_ICON,			/* Remocon Key preview */
/* 122 */	MVBMP_MAIN_BACK,				/* Main Menu Background Test */
/* 123 */	MVBMP_ANI_DISC1,				/* Disc Format animation1 */
/* 124 */	MVBMP_ANI_DISC2,				/* Disc Format animation2 */
/* 125 */	MVBMP_ANI_DISC3,				/* Disc Format animation3 */

/* 126 */	MVBMP_SUBMENU_TOP_LEFT,			/* mainmenu - Submenu box left-top icon */
/* 127 */	MVBMP_SUBMENU_TOP_RIGHT,		/* mainmenu - Submenu box right-top icon */
/* 128 */	MVBMP_SUBMENU_BOT_LEFT,			/* mainmenu - Submenu box left-bottom icon */
/* 129 */	MVBMP_SUBMENU_BOT_RIGHT,		/* mainmenu - Submenu box right-bottom icon */
/* 130 */	MVBMP_SUBMENU_LEFT_LINE,		/* mainmenu - Submenu box left-line icon */
/* 131 */	MVBMP_SUBMENU_RIGHT_LINE,		/* mainmenu - Submenu box right-line icon */
/* 132 */	MVBMP_SUBMENU_TOP_LINE,			/* mainmenu - Submenu box top-line icon */
/* 133 */	MVBMP_SUBMENU_BOTTOM_LINE,		/* mainmenu - Submenu box bottom-line icon */
/* 134 */	MVBMP_INFOBAR,					/* info-bar background */
/* 135 */	MVBMP_PVR_REC,					/* PVR Recording Icon */
/* 136 */ 	MVBMP_PVR_RUN,					/* PVR Playing Icon */
/* 137 */	MVBMP_PVR_INFOBAR,				/* PVR Information Banner Background */
/* 138 */	MVBMP_PVR_DATE,					/* PVR Information Banner Calendar Icon */
/* 139 */	MVBMP_PVR_TIME,					/* PVR Information Banner clock Icon */
/* 140 */	MVBMP_PVR_RECORD,				/* PVR Information Banner Record Icon */
/* 141 */	MVBMP_PVR_PLAY,					/* PVR Information Banner Play Icon */
/* 142 */	MVBMP_HDD_WHITE,				/* PVR Storage Information white level Icon */
/* 143 */	MVBMP_HDD_BLUE,					/* PVR Storage Information blue level Icon */
/* 144 */	MVBMP_HDD_RED,					/* PVR Storage Information red level Icon */
/* 145 */ 	MVBMP_POSITION,					/* Progress Level Point Position Orange Icon */
/* 146 */ 	MVBMP_POSITION_BLUE,			/* Progress Level Point Position Blue Icon */
/* 147 */ 	MVBMP_INPUT_BACK,				/* Input channel number background */

/*********************** GAME IMAGE **************************************************/

/* 148 */ 	MVBMP_PUSH_SPACE,				/* Game Icon */
/* 149 */ 	MVBMP_PUSH_PUSH,				/* Game Icon */
/* 150 */ 	MVBMP_PUSH_WALL,				/* Game Icon */
/* 151 */ 	MVBMP_PUSH_LOCK,				/* Game Icon */
/* 152 */ 	MVBMP_PUSH_BOX,					/* Game Icon */
/* 153 */ 	MVBMP_PUSH_LOCKED,				/* Game Icon */
/* 154 */ 	MVBMP_PUSH_ONLOCK,				/* Game Icon */

/*********************** PVR IMAGE **************************************************/

/* 155 */ 	MVBMP_PVR_REC_TR,				/* PVR Recording Turckish Icon */
/* 156 */ 	MVBMP_PVR_RUN_TR,				/* PVR Playing Turckish Icon */
/* 157 */ 	MVBMP_FORWARD_2X,				/* PVR Forward 2x Fast */
/* 158 */ 	MVBMP_FORWARD_4X,				/* PVR Forward 4x Fast */
/* 159 */ 	MVBMP_FORWARD_8X,				/* PVR Forward 8x Fast */
/* 160 */ 	MVBMP_FORWARD_16X,				/* PVR Forward 16x Fast */
/* 161 */ 	MVBMP_BACKWARD_2X,				/* PVR Backward 2x Fast */
/* 162 */ 	MVBMP_BACKWARD_4X,				/* PVR Backward 4x Fast */
/* 163 */ 	MVBMP_BACKWARD_8X,				/* PVR Backward 8x Fast */
/* 164 */ 	MVBMP_BACKWARD_16X,				/* PVR Backward 16x Fast */

/*************************************************************************************/

/* 165 */ 	MVBMP_MENU_BACK,				/* PVR Backward 16x Fast */

/* --- */	MVBMP_INFO_BANNER_CIRCLE,		/* information banner middle channel number circle icon */

/* 0 */		MVBMP_INFO_CHNAME_CAP,			/* Information banner channel name background capture */
/* 0 */		MVBMP_INFO_TIME_CAP,			/* Information banner Date & Time background capture */
/* 0 */		MVBMP_INFO_EPG1_CAP,			/* Information banner Current EPG background capture */
/* 0 */		MVBMP_INFO_EPG2_CAP,			/* Information banner Next EPG background capture */
/* 0 */		MVBMP_INFO_SAT_INFO_CAP,		/* Information banner Sat & TP information background capture */
/* 0 */		MVBMP_MAIN_BACK_CAPTURE,		/* MainMenu -> MainMenu Box Capture */
/* 0 */		MVBMP_PVR_TIME_CAP,				/* PVR Record Info-bar Timer background capture */
/* 0 */		MVBMP_MAX
}	MV_Bitmap;

BITMAP	MV_BMP[MVBMP_MAX];

int MV_LoadBmp(BOOL b8Load_Kind);
int MV_LoadBmp_NoProgress(HWND hwnd);
void MV_InitBmp(void);

#endif

