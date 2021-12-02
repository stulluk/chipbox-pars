#ifndef  _CS_TIMER_H_
#define  _CS_TIMER_H_

#include "timer.h"

CSAPP_Applet_t	CSApp_Timer(void);

U8						u8Modify_Item;
U8						u8Modify_Number;
tCS_TIMER_JobInfo		stGlobTimerJob;

typedef enum
{
	MV_TIMER_NO = 0,							
	MV_TIMER_ENAVLE,
	MV_TIMER_TYPE,
	MV_TIMER_TERM,
	MV_TIMER_CHANNEL,
	MV_TIMER_DATE,
	MV_TIMER_TIME,
	MV_TIMER_DURATION,
	MV_TIMER_SAVE,
	MV_TIMER_ITEM_MAX
} MV_Timer_Items;

/**************************************************************************************************/

#define TIMER_WINDOW_X					280
#define TIMER_WINDOW_Y					130
#define	TIMER_WINDOW_DX					720
#define TIMER_WINDOW_DY					330
#define TIMER_WINDOW_TITLE_Y			140
#define TIMER_WINDOW_CONT_Y				180
#define TIMER_WINDOW_OUT_OFFSET			10
#define TIMER_WINDOW_ITEM_X				( TIMER_WINDOW_X + TIMER_WINDOW_OUT_OFFSET )
#define TIMER_WINDOW_ITEM_DY			30
#define TIMER_WINDOW_CONT_DX			700

#define TIMER_WINDOW_ITEM_NO_X			( TIMER_WINDOW_ITEM_X + 10 )
#define TIMER_WINDOW_ITEM_NO_DX			40
#define TIMER_WINDOW_ITEM_NAME_X		( TIMER_WINDOW_ITEM_NO_X + TIMER_WINDOW_ITEM_NO_DX )
#define TIMER_WINDOW_ITEM_NAME_DX		200
#define TIMER_WINDOW_ITEM_DATE_X		( TIMER_WINDOW_ITEM_NAME_X + TIMER_WINDOW_ITEM_NAME_DX )
#define TIMER_WINDOW_ITEM_DATE_DX		150
#define TIMER_WINDOW_ITEM_TIME_X		( TIMER_WINDOW_ITEM_DATE_X + TIMER_WINDOW_ITEM_DATE_DX )
#define TIMER_WINDOW_ITEM_TIME_DX		100
#define TIMER_WINDOW_ITEM_TYPE_X		( TIMER_WINDOW_ITEM_TIME_X + TIMER_WINDOW_ITEM_TIME_DX )
#define TIMER_WINDOW_ITEM_TYPE_DX		100

#define TIMER_DY	( TIMER_WINDOW_DY + (TIMER_WINDOW_ITEM_DY*5) )

/**************************************************************************************************/

#define TIMER_MOD_X						400
#define	TIMER_MOD_Y						200
#define TIMER_MOD_MAX_ITEM				MV_TIMER_ITEM_MAX

#define	TIMER_MOD_ITEM_HEIGHT			30
#define	TIMER_MOD_X_CAP					10
#define	TIMER_MOD_Y_CAP					10
#define	TIMER_MOD_DX					480
#define	TIMER_MOD_DY					( ( TIMER_MOD_ITEM_HEIGHT * (TIMER_MOD_MAX_ITEM + 2) ) + ( TIMER_MOD_Y_CAP * 3 ) )
#define	TIMER_MOD_ITEM_NAME_DX			170
#define TIMER_MOD_ITEM_CONT_X			( TIMER_MOD_X + TIMER_MOD_ITEM_NAME_DX + ( TIMER_MOD_X_CAP * 2 ))
#define	TIMER_MOD_ITEM_CONT_DX			280
#define	TIMER_MOD_ITEM_DX				( TIMER_MOD_ITEM_NAME_DX + TIMER_MOD_ITEM_CONT_DX + TIMER_MOD_X_CAP )
#define	TIMER_MOD_ITEM_X				( TIMER_MOD_X + TIMER_MOD_X_CAP )
#define	TIMER_MOD_ITEM_Y				( TIMER_MOD_Y + TIMER_MOD_ITEM_HEIGHT + (TIMER_MOD_Y_CAP * 2) )
#define	TIMER_MOD_ITEM_DY				( TIMER_MOD_ITEM_HEIGHT * TIMER_MOD_MAX_ITEM )
#define	TIMER_MOD_TITLE_X				( TIMER_MOD_X + TIMER_MOD_X_CAP )
#define	TIMER_MOD_TITLE_Y				( TIMER_MOD_Y + TIMER_MOD_Y_CAP )
#define	TIMER_MOD_TITLE_DX				( TIMER_MOD_DX - ( TIMER_MOD_X_CAP * 2 ) )

/**************************************************************************************************/

#endif

