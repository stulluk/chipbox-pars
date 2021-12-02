#ifndef  _CS_RESET_H_
#define  _CS_RESET_H_

CSAPP_Applet_t	CSApp_Reset(void);

#define RESET_WARNING_X			440
#define	RESET_WARNING_Y			200
#define	RESET_WARNING_DX		400
#define	RESET_WARNING_DY		30
#define	RESET_WARNING_MSG_X		RESET_WARNING_X - 100
#define	RESET_WARNING_MSG_Y		260
#define	RESET_WARNING_MSG_DX	RESET_WARNING_DX + 200
#define	RESET_WARNING_MSG_DY	260
#define	YES_ICON_X				550
#define	YES_ICON_Y				450
#define	YES_ICON_DX				100
#define	NO_ICON_X 				680
#define NO_ICON_Y				YES_ICON_Y

typedef enum
{
	EN_RESET_MSG_RESET= 0,
	EN_RESET_MSG_OFF,
	EN_RESET_MSG_DELETE,
	EN_RESET_MSG_SAT_DELETE,
	EN_RESET_MSG_FAV_DELETE,
	EN_RESET_MSG_MAX
}EN_RESET_MSG;

typedef enum
{
	EN_RESET_LIST_SAT = 0,
	EN_RESET_LIST_FAV,
	EN_RESET_LIST_MAX
}EN_RESET_LIST_TYPE;


#endif

