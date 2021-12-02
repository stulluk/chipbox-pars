#ifndef CS_EIT_ENGINE_PRIV_H
#define CS_EIT_ENGINE_PRIV_H

#define  kCS_EIT_PRESENT_MASK 		           0x01
#define  kCS_EIT_FOLLOWING_MASK 		       0x02
#define  kCS_EIT_SC_ALL_MASK 		           0xffffffff


#define kCS_EIT_VERSION_SC_FIRST 		0
#define kCS_EIT_VERSION_SC_SECOND		1
#define kCS_EIT_NO_VERSION				0xFF
#define kCS_EIT_VERSION_INIT			0xF0
#define kCS_EIT_INVALID_LANGUAGE		0xffffffff
#define MV_EIT_NUMBER_OF_TID			0x0F   /* By kb : 2010 0404 */
#define EIT_EVENT_HEADER_LENGTH			12

#define EIT_Min(x, y)				(((S32)x < (S32)y) ? (x) : (y))
#define EIT_Max(x, y)				(((S32)x > (S32)y) ? (x) : (y))

typedef enum 
{
    eEIT_PRESENT_TABLE = 0,
    eEIT_FOLLOWING_TABLE,
    eEIT_SCHEDULE_TABLE
} eit_table_type_t;

typedef struct eit_event_s 
{
    U16                event_id;
    U16                date_mjd;
    U16                start_time_utc;
    U16                duration_utc;
    U16                extended_desc_length;
    U8                 event_name_length;      /* by kb : 20100404 */
    U8                 short_desc_length;      /* by kb : 20100404 */ 
    char              *short_description;
    char              *extended_description;
    char              *event_name;   /* by kb : 20100404 */
    U8                 running_status;  /* by kb : 20100404 */
    U8                 free_ca;    
    U8                 parental_rating;
	U8                 contentData;
    U8                 extended_desc_language[4];                    
    U8                 short_desc_language[4];
    struct eit_event_s *next;
    struct eit_event_s *prev;
} eit_event_t;

typedef struct eit_service_s 
{
    U16                	service_id;
    U8                  eit_pf_mask;
    U8                  eit_pf_version;
    U8                  eit_sc_section[MV_EIT_NUMBER_OF_TID]; /* By kb : 2010 0404 */
    U8                  eit_sc_version[MV_EIT_NUMBER_OF_TID]; /* By kb : 2010 0404 */
    U8                  eit_sc_complete[MV_EIT_NUMBER_OF_TID];
    eit_event_t        *present_event;
    eit_event_t        *following_event;
    eit_event_t        *sc_event_list;
    struct eit_service_s *next;
} eit_service_t;


typedef struct 
{
	U16     		TP_Index;
	eit_service_t  *eit_service_list;
} tCS_EIT_ServiceList;

#endif

