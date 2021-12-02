#ifndef __DATE_TIME_H
#define __DATE_TIME_H


#ifdef __cplusplus
extern "C" {
#endif

#define kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS              6
#define ONE_DAY_IN_MINUTE                     1440  /* 24Hour * 60 = 1440  kb : Mar 25 */

typedef enum
{
	eCS_DT_MONDAY = 1,
	eCS_DT_TUESDAY,
	eCS_DT_WEDNESDAY,   
	eCS_DT_THURSDAY,
	eCS_DT_FRIDAY,
	eCS_DT_SATURDAY,
	eCS_DT_SUNDAY
}tCS_DT_DayOfWeek;

typedef enum
{
	eCS_DT_NO_ERROR = 0,
	eCS_DT_ERROR,
	eCS_DT_NOT_DONE
}tCS_DT_Error;

typedef enum
{	
	eCS_DT_NONE = 0,
	eCS_DT_MANUAL,
	eCS_DT_DAY,
	eCS_DT_HOUR,
	eCS_DT_MINUTE	
}tCS_DT_UpdateEvent;


typedef enum
{
	eCS_DT_OFFSET_NEGATIVE,
	eCS_DT_OFFSET_POSITIVE
}tCS_DT_OffsetPolarity;


typedef struct 
{
	U16 year;
	U8  month;
	U8  day;
}tCS_DT_Date;

typedef struct
{
	U8 hour;  	/* 0~23 */
	U8 minute;
}tCS_DT_Time;

typedef struct 
{	
	BOOL 					CS_DT_FoundTOTAndTDT;
	U16               		CS_DT_TDT_TOT_mjd;    		/* current date                     */	
	U16               		CS_DT_Local_mjd;      		/* Local date after applying offset */
	U16              		CS_DT_TDT_TOT_utc;   		/* current time         no second             */
	U16               		CS_DT_Local_utc;     		/* Local time after applying offset no second */
	tCS_DT_OffsetPolarity  	CS_DT_OffsetPolarity;
	U16                		CS_DT_Offset_utc;         	/* offset value         no second            */
	CS_OS_Clock_t			CS_DT_OSTime_LastUpdate;
	BOOL					CS_DT_AutoTimeEnable;
	BOOL					CS_DT_AutoOffsetEnable;
}tCS_DT_Info;

typedef void ( * tCS_DT_UpdateCallback)(tCS_DT_UpdateEvent event, U16 local_mjd, U16 local_utc);

typedef void ( * tCS_DT_OsTimerNotify)(void);

typedef struct
{
	tCS_DT_UpdateEvent 		UpdateEvent;
	tCS_DT_UpdateCallback 	NotifyFunction;
}tCS_DT_Notify;

tCS_DT_Time CS_DT_UTCtoHM(U16 utc);
tCS_DT_Date CS_DT_MJDtoYMD(U16 mjd);
U16 CS_DT_HMtoUTC(tCS_DT_Time hm);
U16 CS_DT_YMDtoMJD(tCS_DT_Date ymd);
U32 CS_DT_UTC_Add(U16 l_utc, U16 r_utc);
U32 CS_DT_UTC_Subtract(U16 l_utc, U16 r_utc);
tCS_DT_Error CS_DT_Register_DateTime_Notify(U8 *ClientId, tCS_DT_UpdateEvent UpdateEvent, tCS_DT_UpdateCallback NotifyFunction);
tCS_DT_Error CS_DT_Unregister_DateTime_Notify(U8 ClientId);
BOOL CS_DT_Init(tCS_DT_Date  start_date, tCS_DT_Time start_time);
tCS_DT_Error	CS_DT_Start(void);
tCS_DT_Error	CS_DT_Stop(void);
void CS_DT_ManualSetUTCOffset(U16  utc_offset, tCS_DT_OffsetPolarity utc_offset_type);
void CS_DT_GetUTCOffset(U16  *utc_offset, tCS_DT_OffsetPolarity *utc_offset_type);
void CS_DT_ManualSetDateAndTime(U16 mjd, U16 utc);
U16 CS_DT_Get_TDTTOT_UTC( void );
U16 CS_DT_Get_TDTTOT_MJD( void );
U16 CS_DT_GetLocalUTC( void );
U16 CS_DT_GetLocalMJD( void );
void  CS_DT_EnableAutomaticTime(BOOL  enable);
void  CS_DT_EnableAutomaticOffset(BOOL  enable);
U8  CS_DT_CalculateWeekDay(U16  mjd);
void  CS_DT_Caculate_EPG_Localtime(U16 *date_mjd,U16 *time_utc);

void CS_DT_SetOsTimerJob(U32 minute, tCS_DT_OsTimerNotify NotifyFunction);
void CS_DT_ClearOsTimerJob(void);
BOOL CS_DT_GetTDTTOTFoundStatus(void);


#ifdef __cplusplus
}
#endif

#endif 


