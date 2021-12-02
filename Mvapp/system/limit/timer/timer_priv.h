#ifndef __TIMER_PRIV_H
#define __TIMER_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	eCS_TIMER_EVT_MANUAL_TIME,
	eCS_TIMER_EVT_JOB_BEGIN,	
	eCS_TIMER_EVT_JOB_END
}tTIMER_MsgType;


typedef enum
{
	eCS_TIMER_Begin = 0,
	eCS_TIMER_End
}tTIMER_ActionType;

typedef struct
{
	U32		Begin_Time;
	U32		End_Time;
	tTIMER_ActionType	Recent_Action;
}tTIMER_JobAction;

typedef struct
{
	U8					Job_Index;
	tTIMER_JobAction	Recent_Job;
}tTIMER_RecentJobAction;

typedef struct
{
	tTIMER_MsgType  	Message;
	U8				Job_Index;
}tTIMER_Msg_t;

#ifdef __cplusplus
}
#endif

#endif 

