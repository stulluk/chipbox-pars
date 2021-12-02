/****************************************************************
*
* FILENAME
*	merihkey.h
*
* PURPOSE 
*	Header for MerihVideo Front Key Input
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.02.21           KB
*
****************************************************************/
#ifndef MERIH_KEY_H
#define MERIH_KEY_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/

/****************************************************************
*	                    Define Values                           *
*****************************************************************/

#define MAX_USER_KEY                  60
#define MAX_FRONT_SUPPORT_RCU         5

#if 0
/* Key Define for application */
#define     CSAPP_KEY_NULL      0xFF
#define     CSAPP_KEY_POWER     0xF
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
#define	    CSAPP_KEY_SD_HD	    0x14

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
#define	    CSAPP_KEY_CH_UP		0x39
#define	    CSAPP_KEY_CH_DOWN	0x3a

#define	    CSAPP_KEY_F1		0x40
#define	    CSAPP_KEY_F2 		0x41

#define	    CSAPP_KEY_SAT		0x50
#define	    CSAPP_KEY_TV_AV		0x51
#define	    CSAPP_KEY_FAVOLIST	0x52

#define	    CSAPP_KEY_REC		0x60
#define	    CSAPP_KEY_PLAY		0x61
#define	    CSAPP_KEY_STOP		0x62
#define	    CSAPP_KEY_PAUSE		0x63
#define	    CSAPP_KEY_LIST		0x64
#define	    CSAPP_KEY_REW		0x65
#define	    CSAPP_KEY_FF		0x66
#define	    CSAPP_KEY_SLOW		0x67
#define	    CSAPP_KEY_SLOW_B	0x68
#define     CSAPP_KEY_SLEEP     0x69

#define	    CSAPP_KEY_SUBTITLE	0x70
#define	    CSAPP_KEY_NET_CFG	0x71
#define	    CSAPP_KEY_ZOOM  	0x72
#define	    CSAPP_KEY_LANG		0x73
#define     CSAPP_KEY_FIND      0x74
#define     CSAPP_KEY_V_MODE    0x75
#define     CSAPP_KEY_TEXT      0x76
#define     CSAPP_KEY_AUDIO     0x77

#define     CSAPP_KEY_HOLD      0x80
#define     CSAPP_KEY_UPDATE    0x81
#define     CSAPP_KEY_DIAG      0x83

#else

#define     CSAPP_KEY_NULL      0xFF

#define		CSAPP_KEY_EXIT		0x1
#define 	CSAPP_KEY_1			0x2
#define 	CSAPP_KEY_2			0x3
#define 	CSAPP_KEY_3			0x4
#define 	CSAPP_KEY_4			0x5
#define 	CSAPP_KEY_5			0x6
#define 	CSAPP_KEY_6			0x7
#define 	CSAPP_KEY_7			0x8
#define 	CSAPP_KEY_8			0x9
#define 	CSAPP_KEY_9			0xa
#define 	CSAPP_KEY_0			0xb

#define		CSAPP_KEY_PG_DOWN	0x12
#define	    CSAPP_KEY_SD_HD	    0x14
#define     CSAPP_KEY_PAUSE		0x17                
#define		CSAPP_KEY_TVRADIO   0x18
#define		CSAPP_KEY_SUBTITLE  0x19
#define		CSAPP_KEY_OK		0x1c
#define		CSAPP_KEY_MUTE		0x1e
#define 	CSAPP_KEY_FAVOLIST	0x20

#define		CSAPP_KEY_LANG      0x22
#define		CSAPP_KEY_AUDIO	    0x23
#define		CSAPP_KEY_TEXT      0x24
#define     CSAPP_KEY_RECALL	0x26
#define 	CSAPP_KEY_RED		0x27
#define 	CSAPP_KEY_GREEN		0x28
#define 	CSAPP_KEY_YELLOW	0x29
#define 	CSAPP_KEY_BLUE		0x2a
#define		CSAPP_KEY_PG_UP		0x2e

#define		CSAPP_KEY_POWER		0x30
#define		CSAPP_KEY_INFO		0x31
#define		CSAPP_KEY_EPG		0x32
#define		CSAPP_KEY_MENU		0x3b
#define	    CSAPP_KEY_F1		0x3c
#define	    CSAPP_KEY_F2 		0x3d
#define	    CSAPP_KEY_F3		0x3e
#define	    CSAPP_KEY_F4 		0x3f

#define	    CSAPP_KEY_VOL_UP	0x40
#define	    CSAPP_KEY_VOL_DOWN	0x41
#define	    CSAPP_KEY_CH_UP		0x42
#define	    CSAPP_KEY_CH_DOWN	0x43
#define		CSAPP_KEY_UP		0x48	
#define		CSAPP_KEY_LEFT		0x4b
#define		CSAPP_KEY_RIGHT		0x4d
#define		CSAPP_KEY_DOWN		0x50

#define	    CSAPP_KEY_SAT		0x51
#define	    CSAPP_KEY_TV_AV		0x52

#define	    CSAPP_KEY_REC		0x60
#define	    CSAPP_KEY_PLAY		0x61
#define	    CSAPP_KEY_STOP		0x62
// #define	    CSAPP_KEY_LIST		0x64  /* Problem with MiniGUI main */
#define	    CSAPP_KEY_REW		0x65
#define	    CSAPP_KEY_FF		0x66
#define	    CSAPP_KEY_SLOW		0x67
#define	    CSAPP_KEY_SLOW_B	0x68
#define     CSAPP_KEY_SLEEP     0x69
#define	    CSAPP_KEY_LIST		0x6A

#define	    CSAPP_KEY_NET_CFG	0x71
#define	    CSAPP_KEY_ZOOM  	0x72
#define     CSAPP_KEY_FIND      0x74
#define     CSAPP_KEY_V_MODE    0x75

#define     CSAPP_KEY_HOLD      0x80
#define     CSAPP_KEY_UPDATE    0x81
#define     CSAPP_KEY_DIAG      0x83

#endif

#define 	CSAPP_KEY_CR		CSAPP_KEY_RED
#define 	CSAPP_KEY_CG		CSAPP_KEY_GREEN
#define 	CSAPP_KEY_CY		CSAPP_KEY_YELLOW
#define 	CSAPP_KEY_CB		CSAPP_KEY_BLUE


#define	    CSAPP_KEY_ENTER		CSAPP_KEY_OK
#define	    CSAPP_KEY_ESC		CSAPP_KEY_EXIT
#define	    CSAPP_KEY_IDLE		CSAPP_KEY_POWER

#define	    CSAPP_KEY_SWAP		CSAPP_KEY_RECALL


/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
unsigned char GetRcuPowerKeyCode(unsigned char *keyData);
#endif // #ifndef MERIH_KEY_H

