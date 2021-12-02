#ifndef _CS_APP_PUSH_H_
#define _CS_APP_PUSH_H_

#define MAX_LEVEL		49

#define R_BACK_COLOR	MVAPP_GRAY_COLOR
#define BOARD_COLOR		MVAPP_BACKBLUE_COLOR
#define ROUND_COLOR		MVAPP_BACKBLUE_COLOR
#define TEXT_B_COLOR	BOARD_COLOR
#define TEXT_L_COLOR	R_BACK_COLOR

#define	SPACE_BLOCK 	0
#define WALL_BLOCK 		1
#define LOCK_BLOCK 		2
#define BOX_BLOCK 		3
#define PUSH_BLOCK 		4
#define LOCKING_BLOCK 	5
#define PUSH_LOCK_BLOCK 6

CSAPP_Applet_t	CSApp_Push(void);
CSAPP_Applet_t	CSApp_Calendar(void);

#endif

