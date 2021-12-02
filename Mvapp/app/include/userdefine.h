#ifndef  _CS_APP_USER_DEFINE_H_
#define  _CS_APP_USER_DEFINE_H_

#include <minigui/merihkey.h>

#define				MODEL_TYPE					"HIREMCO DVB-HD-S-0507"
#define				HW_VERSION					"CELES-HD-1201-170710-1.25"
#define				UBOOT_VERSION				"UBoot-OTE-170710-2013"
#define				KERNEL_VERSION				"Kernel-Linux-2.6"
#define				ROOTFS_VERSION				"Linux-2.6-OTE-170710-2013"
#define				MAIN_VERSION				"ELF-OTE-170710-2013"
#define				DEFAULT_DATA_VERSION		"SNT-170710-2013"
#define				DEFAULT_DB_VERSION			"DNI-170710-2013"
#define				VIDEO_BG					"BLOGO-170710-1001"
#define				AUDIO_BG					"ALOGO-170710-1001"
#define				MAC_SIZE					6

#define 			CHECK_CH_WATCH				/* Checking .. If watching CH over 30 second then send data to server at CH change time */
//#define				SPECIAL_ADD_CH_TO_DB		/* Special Only Management Creation CH_DB on MySQL Server */
#define				DAILY_EPG

#if 1

#if 0 //Move to merihkey.h in miniGUI
#define     CSAPP_KEY_POWER     0xFF
#define 	CSAPP_KEY_0			0x0
#define 	CSAPP_KEY_1			0x1
#define 	CSAPP_KEY_2			0x2
#define 	CSAPP_KEY_3			0x3
#define 	CSAPP_KEY_4			0x4
#define 	CSAPP_KEY_5			0x5
#define 	CSAPP_KEY_6			0x6
#define 	CSAPP_KEY_7			0x7
#define 	CSAPP_KEY_8			0x8
#define 	CSAPP_KEY_9			0x9
#define	    CSAPP_KEY_RECALL	0xA
#define	    CSAPP_KEY_MUTE		0xB

#define	    CSAPP_KEY_MENU		0x10
#define     CSAPP_KEY_OK        0x11
#define     CSAPP_KEY_EXIT      0x12
#define	    CSAPP_KEY_TVRADIO	0x13
#define	    CSAPP_KEY_SD	    0x14

#define 	CSAPP_KEY_RED		0x20
#define 	CSAPP_KEY_GREEN		0x21
#define 	CSAPP_KEY_YELLOW	0x22
#define 	CSAPP_KEY_BLUE		0x23
#define	    CSAPP_KEY_EPG		0x24
#define	    CSAPP_KEY_INFO		0x25

#define	    CSAPP_KEY_PG_UP		0x30
#define	    CSAPP_KEY_PG_DOWN	0x31
#define	    CSAPP_KEY_UP		0x32	
#define	    CSAPP_KEY_DOWN		0x33
#define	    CSAPP_KEY_LEFT		0x34
#define	    CSAPP_KEY_RIGHT		0x35
#define	    CSAPP_KEY_VOL_UP	0x36
#define	    CSAPP_KEY_VOL_DOWN	0x37
#define	    CSAPP_KEY_CH_UP		0x38
#define	    CSAPP_KEY_CH_DOWN	0x39

#define	    CSAPP_KEY_F1		0x40
#define	    CSAPP_KEY_F2 		0x41

#define	    CSAPP_KEY_SAT		0x50
#define	    CSAPP_KEY_TV_AV		0x51
#define	    CSAPP_KEY_FAVOLIST	0x52

#define	    CSAPP_KEY_FAV		0x60
#define	    CSAPP_KEY_PLAY		0x61
#define	    CSAPP_KEY_STOP		0x62
#define	    CSAPP_KEY_PAUSE		0x63
#define	    CSAPP_KEY_LIST		0x64
#define	    CSAPP_KEY_REW		0x65
#define	    CSAPP_KEY_FF		0x66
#define	    CSAPP_KEY_SLOW		0x67
#define	    CSAPP_KEY_SLOW_B	0x68
#define	    CSAPP_KEY_TIMER	    0x69

#define 	CSAPP_KEY_CR		CSAPP_KEY_RED
#define 	CSAPP_KEY_CG		CSAPP_KEY_GREEN
#define 	CSAPP_KEY_CY		CSAPP_KEY_YELLOW
#define 	CSAPP_KEY_CB		CSAPP_KEY_BLUE

#define     CSAPP_KEY_FIND      CSAPP_KEY_RED
#define     CSAPP_KEY_V_MODE    CSAPP_KEY_GREEN
#define     CSAPP_KEY_TTX       CSAPP_KEY_YELLOW
#define     CSAPP_KEY_AUDIO     CSAPP_KEY_BLUE

#define	    CSAPP_KEY_ENTER		CSAPP_KEY_OK
#define	    CSAPP_KEY_ESC		CSAPP_KEY_EXIT
#define	    CSAPP_KEY_IDLE		CSAPP_KEY_POWER

#define	    CSAPP_KEY_NET		CSAPP_KEY_F1
#define	    CSAPP_KEY_ZOOM  	CSAPP_KEY_F2

#define	    CSAPP_KEY_SUBTITLE	0x70

// #define	   CSAPP_KEY_LANG		CSAPP_KEY_CR
// #define	   CSAPP_KEY_AUDIO		CSAPP_KEY_CG
// #define	   CSAPP_KEY_PAUSE		CSAPP_KEY_CY                
// #define	   CSAPP_KEY_TEXT		CSAPP_KEY_CB

#define	   CSAPP_KEY_SWAP		0x26
#endif

#else
/* ------------------HS keypad --------------------------------*/
#define	CSAPP_KEY_UP		0x48	
#define	CSAPP_KEY_DOWN		0x50
#define	CSAPP_KEY_LEFT		0x4b
#define	CSAPP_KEY_RIGHT		0x4d
/* ------------------KK keypad ---------*/
#define	CSAPP_KEY_PG_UP		0x2e
#define	CSAPP_KEY_PG_DOWN	0x12
//#define	CSAPP_KEY_VOL_UP	0xf1
//#define	CSAPP_KEY_VOL_DOWN	0xfc
//#define	CSAPP_KEY_CH_UP		0xfe
//#define	CSAPP_KEY_CH_DOWN	0xf3

#define	CSAPP_KEY_MENU		0x3b
#define	CSAPP_KEY_ENTER		0x1c
#define	CSAPP_KEY_ESC			0x1
#define	CSAPP_KEY_IDLE			0x30
#define	CSAPP_KEY_MUTE		0x1e
//#define 	CSAPP_KEY_RADIO		0x10  //no use
/*----------------- BGCTV RMC rc6	-------------------*/
/*  the digit num*/ 
#define 	CSAPP_KEY_0			0xb
#define 	CSAPP_KEY_1			0x2
#define 	CSAPP_KEY_2			0x3
#define 	CSAPP_KEY_3			0x4
#define 	CSAPP_KEY_4			0x5
#define 	CSAPP_KEY_5			0x6
#define 	CSAPP_KEY_6			0x7
#define 	CSAPP_KEY_7			0x8
#define 	CSAPP_KEY_8			0x9
#define 	CSAPP_KEY_9			0xa
//#define 	CSAPP_KEY_PROGRAM	0x1B
#define 	CSAPP_KEY_FAVOLIST	0x20
#define	CSAPP_KEY_TVRADIO		0x18
#define	CSAPP_KEY_SUBTITLE	0x19

#define	CSAPP_KEY_INFO		0x31
#define	CSAPP_KEY_EPG			0x32
//#define	CSAPP_KEY_SWITCH			0x1D

//#define	CSAPP_KEY_TVLIST			0x1F
//#define	CSAPP_KEY_BCLIST			0x20
//#define	CSAPP_KEY_NVOD			0x21

//#define	CSAPP_KEY_SEARCH			0xe5	//T/TEXT

//#define	CSAPP_KEY_LANG			0xe6
//#define	CSAPP_KEY_POS				0x25	//switch
//#define	CSAPP_KEY_AUDIO			0x26


#define 	CSAPP_KEY_CR			0x22
#define 	CSAPP_KEY_CG			0x23
#define 	CSAPP_KEY_CY			0x17
#define 	CSAPP_KEY_CB			0x24

#define	CSAPP_KEY_LANG		CSAPP_KEY_CR
#define	CSAPP_KEY_AUDIO		CSAPP_KEY_CG
#define	CSAPP_KEY_PAUSE		CSAPP_KEY_CY                
#define	CSAPP_KEY_TEXT		CSAPP_KEY_CB

#define	CSAPP_KEY_SWAP		0x26

#endif

#define 	CSAPP_KEY_FnA			0x27
#define 	CSAPP_KEY_FnB			0x28
#define 	CSAPP_KEY_FnC			0x29
#define 	CSAPP_KEY_FnD			0x2A
#define 	CSAPP_KEY_FnE			0x2B
#define 	CSAPP_KEY_FnF			0x2C
#define   	CSAPP_KEY_FnS			0x31/*KEY_CODE_FnS*/
#define   	CSAPP_KEY_FnG			0x32/*KEY_CODE_FnG*/
#define   	CSAPP_KEY_FnH			0x33/*KEY_CODE_FnH*/
#define   	CSAPP_KEY_FnJ			0x34/*KEY_CODE_FnJ*/
#define   	CSAPP_KEY_FnK			0x35/*KEY_CODE_FnK*/
#define   	CSAPP_KEY_FnL			0x36/*KEY_CODE_FnL*/
#define   	CSAPP_KEY_FnZ			0x37/*KEY_CODE_FnZ*/
#define   	CSAPP_KEY_FnX			0x38/*KEY_CODE_FnX*/
#define   	CSAPP_KEY_FnV			0x39/*KEY_CODE_FnV*/
#define   	CSAPP_KEY_FnN			0x3a/*KEY_CODE_FnN*/
#define   	CSAPP_KEY_FnM			0x3b/*KEY_CODE_FnM*/

//#define 	CSAPP_KEY_NULL			0x31
#define 	CSAPP_KEY_STB_ID		0x32

/*user defined message*/
#define	MSG_UPDATEPAGE				MSG_USER+1
#define	MSG_PLAYSERVICE				MSG_USER+2
#define	MSG_PIN_INPUT				MSG_USER+3
#define	MSG_PIN_PAINT				MSG_USER+4
#define	MSG_CHECK_SERVICE_LOCK		MSG_USER+5
#define	MSG_SEARCH_COMPLETE			MSG_USER+6
#define	MSG_SET_SMALLWINDOW			MSG_USER+7

#define	MSG_VIDEO_FORFMAT_UPDATE	MSG_USER+8
#define	MSG_NOW_NEXT_UPDATE			MSG_USER+9
#define	MSG_TIME_UPDATE				MSG_USER+10

#define	MSG_EPG_SEC_UPDATE			MSG_USER+11
#define	MSG_EPG_SC_UPDATE			MSG_USER+12

#define	MSG_SERVICEINFO_UPDATE		MSG_USER+13
#define	MSG_TPINFO_UPDATE			MSG_USER+14

#define	MSG_EPG_DRAW_DESC 			MSG_USER+15

#define	MSG_UPDATE_FE				MSG_USER+16
#define	MSG_UPDATE_TTX				MSG_USER+17
#define	MSG_UPDATE_SUB				MSG_USER+18
#define	MSG_UPDATE_HD				MSG_USER+19
#define	MSG_TTX_DISPLAY				MSG_USER+20
#define	MSG_USB_MSG					MSG_USER+21
#define	MSG_NO_VIDEO				MSG_USER+22

#define	MSG_SCANINFO_UPDATE			MSG_USER+23

#ifdef SUPPORT_CI
#define	MSG_CI_MMI_UPDATE			MSG_USER+24
#endif

#define	MSG_SMART_CARD_INSERT		MSG_USER+25
#define	MSG_SMART_CARD_REMOVE		MSG_USER+26
#define	MSG_PVR_PLAY_END			MSG_USER+27
#define	MSG_DOWNLOAD_COMPLETE		MSG_USER+28

/* For Blind Scan By KB Kim 2011.02.28 */
#define	MSG_INSTALL_BLIND_UPDATED	MSG_USER+29

/* For S_CAM Menu By Jacob 14 May 2011 */
#define	MSG_S_CAM_CHANGED			MSG_USER+30

/* For Motor Control By KB Kim 2011.05.22 */
#define	MSG_MOTOR_MOVING			MSG_USER+31

/* For Motor Control By KB Kim 2011.06.02 */
#define	MSG_VIDEO_UNDERFLOW			MSG_USER+32

/*defined timer*/
#define   DESKTOP_BANNER_TIMER_ID		101
#define   DESKTOP_VOLUME_TIMER_ID		102
#define   DESKTOP_INPUT_TIMER_ID		103
#define   DESKTOP_SLEEP_TIMER_ID		104
#define   CHECK_EPG_UPDATE_ID			105
#define   CHECK_SIGNAL_TIMER_ID			106
#define   DESKTOP_VMODE_TIMER_ID		107

#define   CHANNELCHANGE_TIMER_ID        108
#define   INFO_TXT_TIMER_ID				109
#define   PVR_CHECK_TIMER_ID			110
#define   PVR_REC_BANNER_TIMER_ID		111

#define	getpage(total, num_per_page)           ( (total%num_per_page)?(total/num_per_page):(total/num_per_page-1))

#define	APP_PRINT				//printf

//#define	Screen_1080
#endif
/*   E O F  */
