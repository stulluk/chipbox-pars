#ifndef CS_EIT_ENGINE_H
#define CS_EIT_ENGINE_H

#include "date_time.h"

#define kCS_EIT_MAX_EVENT_NAME_LENGTH		(128)//(32)
#define kCS_EIT_MAX_SHORT_DESC_LENGTH		(256)
#define kCS_EIT_MAX_EXTENDED_DESC_LENGTH	(4096)
#define kCS_EIT_MAX_TOTAL_DESC_LENGTH		(4400)

#define kCS_EIT_INVALID_MJD					0

typedef enum
{
	eCS_EIT_NO_ERROR = 0,
         eCS_EIT_ERROR,
	eCS_EIT_NOT_DONE,
	eCS_EIT_BAD_PARAMETER
}tCS_EIT_Error;

typedef enum
{
	eCS_EIT_ACTUAL_PF = 0,
	eCS_EIT_ACTUAL_SCHEDULE,
	eCS_EIT_OTHER_PF,
	eCS_EIT_OTHER_SCHEDULE,
	eCS_EIT_ACTUAL_ALL,
	eCS_EIT_OTHER_ALL,
	eCS_EIT_ATCTUAL_OTHER_ALL
}tCS_EIT_Table_Type_t;


typedef enum
{
	eCS_EIT_PRESENT = 0,
	eCS_EIT_FOLLOWING,
	eCS_EIT_SCHEDULE
}tCS_EIT_Type;


typedef void ( * tCS_EIT_NotificationCallBack)(U16  service_id, tCS_EIT_Type eit_type);

typedef struct
{
	U16				event_id;
	U8				free_ca;
	U8				running_status;
	U8				parental_rating;
	U8				contentData;
	U16				start_date_mjd;
	U16				start_time_utc;
	U16				end_date_mjd;
	U16				end_time_utc;
	U16				duration_utc;
	U32				nameLength;
	U32				descriptionLength;
	char			event_name[kCS_EIT_MAX_EVENT_NAME_LENGTH];
	char			description_data[kCS_EIT_MAX_TOTAL_DESC_LENGTH];
	U8				extended_desc_language[4];                    
	U8				short_desc_language[4];
} tCS_EIT_Event_t;

/* By KB Kim 2011.05.28 */
typedef struct tCS_EIT_Event_List_s
{
	U16		NumberOfEvent;
	U16              StartPoint;
	tCS_EIT_Event_t	*EventArray;
} tCS_EIT_Event_List_t;

BOOL CS_EIT_Init (void);
tCS_EIT_Error CS_Eit_Start(U16  tpindex, tCS_EIT_Table_Type_t type, U8 version);
tCS_EIT_Error CS_Eit_Stop(void);
tCS_EIT_Error CS_EIT_RegisterNotify(U8 * pClientID, tCS_EIT_NotificationCallBack pNotificationFunction);
tCS_EIT_Error CS_EIT_Get_Current_Parental_Rate(U16  tp_index, U16 service_id);
tCS_EIT_Error CS_EIT_Get_PF_Event(U16 tp_index, U16  service_id, tCS_EIT_Event_t *present, tCS_EIT_Event_t *follow);
// tCS_EIT_Error CS_EIT_Get_SC_Events(U16 tp_index, U16 service_id, U16 requested_date_mjd,tCS_EIT_Event_t *sc_events);
tCS_EIT_Error Mv_GetScEventList (U16 tp_index, U16 service_id, U16 dateOffset, tCS_EIT_Event_List_t *scEentList);
tCS_EIT_Error CS_EIT_Get_SC_Events_Num(U16 tp_index, U16 service_id, U16 requested_date_mjd,U16 *sc_event_num);
tCS_EIT_Error CS_EIT_Get_SC_Events_FirstMJD(U16  tp_index, U16 service_id, U16 * min_date_mjd);
void CS_EIT_FreeDatabase(void);


#endif

