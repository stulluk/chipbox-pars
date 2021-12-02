#ifndef  _CS_APP_CI_MMI_H_
#define  _CS_APP_CI_MMI_H_

#define 	MV_CI_MENU_X			340
#define		MV_CI_MENU_Y			300
#define		MV_CI_MENU_Y_GAP		30
#define		MV_CI_MENU_DX			600
#define		MV_CI_MENU_SUBTITLE		( MV_CI_MENU_Y - ( MV_CI_MENU_Y_GAP * 4 ) + 20 )
#define		MV_CI_MENU_STATE_Y		MV_CI_MENU_SUBTITLE

typedef struct{
	char text[100];
	int length;
} ci_text_t;

CSAPP_Applet_t CSApp_CI(void);

#endif
/*   E O F  */



