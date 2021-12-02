#ifndef _PARSER_H_
#define _PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SECTION_SIZE		4284
#define TS_PACKET_SIZE			188
#define MAX_SECTION_NUM			255
#define MAX_PROGRAM_NUM			1000

#define NIT_PID					0x10
#define NIT_TABLE_ID_ACTUAL		0x40
#define NIT_TABLE_ID_OTHER		0x41
#define PAT_PID					0x00
#define PAT_TABLE_ID			0x00
#define PMT_TABLE_ID			0x02
#define TDT_PID					0x14
#define TDT_TABLE_ID			0x70
#define TOT_PID					0x14
#define TOT_TABLE_ID			0x73
#define SDT_PID					0x11
#define SDT_TABLE_ID_ACTUAL		0x42
#define SDT_TABLE_ID_OTHER		0x46
#define EIT_PID					0x12
#define	EIT_TABLE_ID_ACTUAL		0x4e
#define EIT_TABLE_ID_OTHER		0x4f
#define EIT_SCHEDUAL_TABLE_ID_ACTUAL_FIRST	0x50
#define EIT_SCHEDUAL_TABLE_ID_ACTUAL_LAST	0x5f
#define EIT_SCHEDUAL_TABLE_ID_OTHER_FIRST	0x60
#define EIT_SCHEDUAL_TABLE_ID_OTHER_LAST	0x6f

#define SHORT_EVENT_DESCRIPTOR_TAG	0x4d
#define SERVICE_DESCRIPTOR_TAG		0x48

#define MAX_SERVICE_DESCRIPTOR_NUM 20
#define MAX_MULTI_SERVICE_NAME_DESCRIPTOR_NUM 20
#define MAX_EXTENDED_EVENT_DESCRIPTOR_NUM 20
#define MAX_EIT_INFO_NUM 20
#define MAX_SHORT_EVENT_DESCRIPTOR_NUM 20
#define MAX_EXTENDED_EVENT_DESCRIPTOR_NUM 20
#define MAX_LOCAL_TIME_OFFSET_DESCRIPTOR_NUM 5
#define MAX_LOCAL_TIME_OFFSET_DESCRIPTOR_INFO_NUM 5

// descriptors
typedef struct _short_event_descriptor
{
	U8 descriptor_tag;
	U8 descriptor_length;
	U16 ISO_639_2_language_code1;
	U8 ISO_639_2_language_code2;
	U8 event_name_length;
	U8 event_name_char[100];
	U8 text_length;
	U8 text_char[1000];
}st_short_event_descriptor;

typedef struct _extended_event_descriptor_info
{
    U8 item_description_length;
    U8 *item_description_char;
    U8 item_length;
    U8 *item_char;
    U8 text_length;
    U8 *text_char;
}st_extended_event_descriptor_info;

typedef struct _extended_event_descriptor
{
    U8 descriptor_tag;
    U8 descriptor_length;
    U8 descriptor_number : 4;
    U8 last_descriptor_number : 4;
    U16 ISO_639_2_language_code1;
    U8 ISO_639_2_language_code2;
    U8 length_of_items;

    U8 extended_event_descriptor_num;
    st_extended_event_descriptor_info *extended_event_descriptor[MAX_EXTENDED_EVENT_DESCRIPTOR_NUM];
}st_extended_event_descriptor;

typedef struct _service_descriptor
{
	U8 descriptor_tag;
	U8 descriptor_length;
	U8 service_type;
	U8 service_provider_length;
	U8 service_provider_char[100];
	U8 service_name_length;
	U8 service_name_char[100];
}st_service_descriptor;

typedef struct _multilingual_service_name_descriptor_info
{
	U16 ISO_639_2_language_code1;
	U8 ISO_639_2_language_code2;
	U8 service_provider_name_length;
	U8 service_provider_name_char[100];
	U8 service_name_length;
	U8 service_name_char[100];
}st_multilingual_service_name_descriptor_info;

typedef struct _multilingual_service_name_descriptor
{
	U8 descriptor_tag;
	U8 descriptor_length;

    U8 multi_service_name_des_info_num;
    st_multilingual_service_name_descriptor_info *multi_service_name_des_info[MAX_MULTI_SERVICE_NAME_DESCRIPTOR_NUM];
}st_multilingual_service_name_descriptor;

typedef struct _local_time_offset_descriptor_info
{
    U16 country_code1;
    U8 country_code2;
    U8 country_region_id : 6;
    U8 reserved : 1;
    U8 local_time_offset_polarity : 1;
    U16 local_time_offset;
    U16 time_of_change_date;
    U16 time_of_change_time1;
    U8 time_of_change_time2;
    U16 next_time_offset;
}st_local_time_offset_descriptor_info;

typedef struct _local_time_offset_descriptor
{
    U8 descriptor_tag;
    U8 descriptor_length;

    U8 local_time_offset_descriptor_info_num;
    st_local_time_offset_descriptor_info * local_time_offset_descriptor_info[MAX_LOCAL_TIME_OFFSET_DESCRIPTOR_INFO_NUM];
}st_local_time_offset_descriptor;

//TSPACKET
//One ts packet is contained by TSPacketHeader, adaption field, payload data.
//If there is adaption filed, adaption field length is specified by the first byte after TSPacketHeader.
//Payload bytes length is the remained bytes.

typedef struct _TSPACKETHEAD //4 BYTES
{
	U8 ucSyncByte;       // First byte
	U8 PID_hibits : 5;      // Second byte - Notice the reverse declaration 
	U8  transport_priority : 1;
	U8  payload_unit_start_indicator : 1;
	U8 transport_error_indicator : 1;
	U8 PID_lobits;       // Third byte
	U8 continuity_counter : 4;    // Fourth byte
	U8 _payload_present_flag : 1;
	U8 _adaptation_field_present_flag : 1;
	U8 transport_scrambling_control : 2;
} stTSPACKETHEAD, *pstTSPACKETHEAD;

typedef struct _TS_NIT
{
	U8 table_id;
	U16 section_syntax_indicator : 1;
	U16 reserved_future_use_1 : 1;
	U16 reserved_1 : 2;
	U16 section_length : 12;
	U16 network_id;
	U8 reserved_2 : 2;
	U8 version_number : 5;
	U8 current_next_indicator : 1;
	U8 section_number;
	U8 last_section_number;
	U16 reserved_future_use_2 : 4;
	U16 network_descriptors_length : 12;
	U16 reserved_future_use_3 : 4;
	U16 transport_stream_loop_length : 12;
	U32 CRC_32;
}stTS_NIT;

typedef struct _TRANSPORT_STREAM_INFO
{
	U16 transport_stream_id;
	U16 original_network_id;
	U16 reserved_future_use : 4;
	U16 transport_descriptors_length : 12;
}stTRANSPORT_STREAM_INFO;

// Program Association Table
typedef struct _TS_PAT
{
    U8 table_id : 8;
    U16 section_syntax_indicator : 1;
    U16 zero : 1;
    U16 reserved_1 : 2;
    U16 section_length : 12;
    U16 transport_stream_id : 16;
    U8 reserved_2 : 2;
    U8 version_number : 5;
    U8 current_next_indicator : 1;
    U8 section_number;
    U8 last_section_number;
    U32 CRC_32;
} stTS_PAT; 

typedef struct _ProgramInfoInPAT// 4 BYTES
{
	U16 program_number;
	U16 reserved : 3;
	U16 PMT_PID : 13;
}ProgramInfoInPAT;

// Program Map Table
typedef struct _TS_PMT
{
    U8 table_id;
    U16 section_syntax_indicator : 1;
    U16 zero : 1;
    U16 reserved_1 : 2;
    U16 section_length : 12;
    U16 program_number;
    U8 reserved_2 : 2;
    U8 version_number : 5;
    U8 current_next_indicator : 1;
    U8 section_number;
    U8 last_section_number;
    U16 reserved_3 : 3;
    U16 PCR_PID : 13;
    U16 reserved_4 : 4;
    U16 program_info_length : 12;
	U32 CRC_32; 
} stTS_PMT; 

typedef struct _ES_INFO
{
	unsigned stream_type                    : 8;
    unsigned reserved1                      : 3;
    unsigned elementary_PID                 : 13;
    unsigned reserved2                      : 4;
    unsigned ES_info_length                 : 12; 
}stES_INFO;

typedef struct _DT_Date
{
	U16 year;
	U8  month;
	U8  day;
}stDT_Date;

typedef struct _DT_Time
{
	U8 hour;  	// 0 ~ 23
	U8 minute;
	U8 second;
}stDT_Time;

typedef struct _TS_TDT
{
	U8 table_id;
	U16 section_syntax_indicator : 1;
	U16 reserved_future_use : 1;
	U16 reserved : 2;
	U16 section_length : 12;
	U16 UTC_time_date;
	U16 UTC_time_time1;
    U8 UTC_time_time2;
}stTS_TDT;

typedef struct _TS_TOT
{
	U8 table_id;
	U16 section_syntax_indicator : 1;
	U16 reserved_future_use : 1;
	U16 reserved_1 : 2;
	U16 section_length : 12;
	U16 UTC_time_date;
	U16 UTC_time_time1;
	U8	UTC_time_time2;
	U16 reserved_2 : 4;
	U16 descriptors_loop_length : 12;
	U32 CRC_32;

    U8 local_time_offset_descriptor_num;
    st_local_time_offset_descriptor* local_time_offset_descriptor[MAX_LOCAL_TIME_OFFSET_DESCRIPTOR_NUM];
}stTS_TOT;

typedef struct _TS_SDT
{
	U8 table_id;
	U16 section_syntax_indicator : 1;
	U16 reserved_future_use_1 : 1;
	U16 reserved_1 : 2;
	U16 section_length : 12;
	U16 transport_stream_id;
	U8 reserved_2 : 2;
	U8 version_number : 5;
	U8 current_next_indicator : 1;
	U8 section_number;
	U8 last_section_number;
	U16 original_network_id;
	U8 reserved_future_use_2;
	U32 CRC_32;
}stTS_SDT;

typedef struct _SDT_INFO
{
	U16 service_id;
	U8 reserved_future_use : 6;
	U8 EIT_schedule_flag : 1;
	U8 EIT_present_following_flag : 1;
	U16 running_status : 3;
	U16 free_CA_mode : 1;
	U16 descriptors_loop_length : 12;

    U8 service_des_num;
    st_service_descriptor* service_descriptor[MAX_SERVICE_DESCRIPTOR_NUM];

    U8 multi_service_name_des_num;
    st_multilingual_service_name_descriptor *multi_service_name_descriptor[MAX_MULTI_SERVICE_NAME_DESCRIPTOR_NUM];
}stSDT_INFO;

typedef struct _EIT_INFO
{
	U16 event_id : 16;
	U16 start_time_date;
	U16 start_time_time1;
	U8 start_time_time2;
	U16 duration1;
	U8 duration2;
	U16 running_status : 3;
	U16 free_CA_mode : 1;
	U16 descriptors_loop_length : 12;

    U8 short_event_descriptor_num;
    st_short_event_descriptor *short_event_descriptor[MAX_SHORT_EVENT_DESCRIPTOR_NUM];

    U8 extended_event_descriptor_num;
    st_extended_event_descriptor *extended_event_descriptor[MAX_EXTENDED_EVENT_DESCRIPTOR_NUM];
}stEIT_INFO;

typedef struct _TS_EIT
{
	U8 table_id;
	U16 section_syntax_indicator : 1;
	U16 reserved_future_use : 1;
	U16 reserved_1 : 2;
	U16 section_length : 12;
	U16 service_id;
	U8 reserved_2 : 2;
	U8 version_number : 5;
	U8 current_next_indicator : 1;
	U8 section_number;
	U8 last_section_number;
	U16 transport_stream_id;
	U16 original_network_id;
	U8 segment_last_section_number;
	U8 last_table_id;
	U32 CRC_32;

    U16 eit_info_num;
    stEIT_INFO *eit_info[MAX_EIT_INFO_NUM];
}stTS_EIT;


// functions
void  section_parser_init(U32 pid, U8 tabid) ;
U32 section_parser(const U8 *_bf,const unsigned long _size) ;

void ParseNITSection(stTS_NIT *NIT_section, const U8 *buffer);
void ParsePATSection (stTS_PAT *PAT_section, const U8 *buffer);
void ParsePMTSection (stTS_PMT *PMT_section, const U8 *buffer);
stDT_Date MJD2YMD(U16 mjd);
stDT_Time UTC2HM(U32 utc);
void ParseTDTSection(stTS_TDT *TDT_section, const U8 *buffer);
void ParseTOTSection(stTS_TOT *TOT_section, const U8 *buffer);
void ParseSDTSection(stTS_SDT *SDT_section, const U8 *buffer);
void ParseEITSection(stTS_EIT *EIT_section, const U8 *buffer);

int parse_short_event_descriptor(st_short_event_descriptor *short_event, const U8 *buffer);
int parse_extended_event_descriptor(st_extended_event_descriptor *extended_event, const U8 *buffer);
int parse_service_descriptor(st_service_descriptor *service_des, const U8 *buffer);
int parse_multilingual_service_name_descriptor(st_multilingual_service_name_descriptor *msn_des, const U8 *buffer);
int parse_local_time_offset_descriptor(st_local_time_offset_descriptor *local_time_offset, const U8 *buffer);

#ifdef __cplusplus
}
#endif

#endif // _PARSER_H_
