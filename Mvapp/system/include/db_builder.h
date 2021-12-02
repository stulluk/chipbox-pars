#ifndef __DB_BUILDER_H
#define __DB_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "database.h"

#define kCS_SI_PID_PAT			0x00
#define kCS_SI_PID_CAT			0x01
#define kCS_SI_PID_NIT			0x10
#define kCS_SI_PID_SDT			0x11
#define kCS_SI_PID_EIT			0x12
#define kCS_SI_PID_RST			0x13
#define kCS_SI_PID_TDT			0x14
#define kCS_SI_PID_TOT			0x14
#define kCS_SI_PID_BAT                    	0x11

#define kCS_SI_TBID_PAT					0x00
#define kCS_SI_TBID_CAT					0x01
#define kCS_SI_TBID_PMT					0x02
#define kCS_SI_TBID_NIT_ACTUAL			0x40
#define kCS_SI_TBID_NIT_OTHER			0x41
#define kCS_SI_TBID_SDT_ACTUAL			0x42
#define kCS_SI_TBID_SDT_OTHER			0x46
#define kCS_SI_TBID_BAT					0x4a
#define kCS_SI_TBID_ECM					0x81

#define	VIDEO_STREAM_DESCRIPTOR					0x02
#define	AUDIO_STREAM_DESCRIPTOR					0x03
#define	HIERARCHY_DESCRIPTOR						0x04
#define	REGISTRATION_DESCRIPTOR					0x05
#define	DATA_STREAM_ALIGNMENT_DESCRIPTOR		0x06
#define	TARGET_BKGND_GRID_DESCRIPTOR			0x07
#define	VIDEO_WINDOW_DESCRIPTOR					0x08
#define	CA_DESCRIPTOR								0x09
#define	ISO_639_LANGUAGE_DESCRIPTOR				0x0A
#define	SYSTEM_CLOCK_DESCRIPTOR					0x0B
#define	MULTIPLEX_BUFF_UTIL_DESCRIPTOR			0x0C
#define	COPYRIGHT_DESCRIPTOR						0x0D
#define	MAXIMUM_BITRATE_DESCRIPTOR				0x0E
#define	PRIVATE_DATA_INDICAT_DESCRIPTOR			0x0F

#define NETWORK_NAME_DESCRIPTOR 					0x40
#define SERVICE_LIST_DESCRIPTOR 					0x41
#define STUFFING_DESCRIPTOR 						0x42
#define SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR 		0x43
#define CABLE_DELIVERY_SYSTEM_DESCRIPTOR 		0x44
//#define VBI_DATA_DESCRIPTOR 						0x45
//#define VBI_TELETEXT_DESCRIPTOR 					0x46
#define BOUQUET_NAME_DESCRIPTOR 					0x47
#define SERVICE_DESCRIPTOR 							0x48
#define COUNTRY_AVAILABILITY_DESCRIPTOR 			0x49
#define LINKAGE_DESCRIPTOR 							0x4a
#define NVOD_REFERENCE_DESCRIPTOR 				0x4b
#define TIME_SHIFTED_SERVICE_DESCRIPTOR 			0x4c
#define SHORT_EVENT_DESCRIPTOR 					0x4d
#define EXTENDED_EVENT_DESCRIPTOR 				0x4e
#define TIME_SHIFTED_EVENT_DESCRIPTOR 				0x4f
#define COMPONENT_DESCRIPTOR 						0x50
#define MOSAIC_DESCRIPTOR 							0x51
#define STREAM_IDENTIFIER_DESCRIPTOR 				0x52
#define CA_IDENTIFIER_DESCRIPTOR 					0x53
#define CONTENT_DESCRIPTOR 						0x54
#define PARENTAL_RATING_DESCRIPTOR 				0x55
#define TELETEXT_DESCRIPTOR 						0x56
#define TELEPHONE_DESCRIPTOR 						0x57
#define LOCAL_TIME_OFFSET_DESCRIPTOR 				0x58
#define SUBTITLING_DESCRIPTOR 						0x59
#define TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR 	0x5a
#define MULTILINGUAL_NETWORK_NAME_DESCRIPTOR 	0x5b
#define MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR 	0x5c
#define MULTILINGUAL_SERVICE_NAME_DESCRIPTOR 	0x5d
#define MULTILINGUAL_COMPONENT_DESCRIPTOR 		0x5e
#define PRIVATE_DATA_SPECIFIER_DESCRIPTOR 			0x5f
#define SERVICE_MOVE_DESCRIPTOR 					0x60
#define SHORT_SMOOTHING_BUFFER_DESCRIPTOR 		0x61
#define FREQUENCY_LIST_DESCRIPTOR 					0x62
#define PARTIAL_TRANSPORT_STREAM_DESCRIPTOR 	0x63
#define DATA_BROADCAST_DESCRIPTOR 				0x64
#define CA_SYSTEM_DESCRIPTOR 						0x65
#define DATA_BROADCAST_ID_DESCRIPTOR 				0x66
#define AUDIO_AC3_DESCRIPTOR		 				0x6a
#define LOGIC_CHANNEL_NUMBER						0x83

#define kCS_SI_MAX_WAITING_PAT				5000 	  /* PAT table waiting timeout*/
#define kCS_SI_MAX_WAITING_PMT				5000 	  /* PAT table waiting timeout*/
#define kCS_SI_MAX_WAITING_CAT				10000 	  /* PAT table waiting timeout*/
#define kCS_SI_MAX_WAITING_SDT_ACTUAL		10000     /* SDT actual table waiting timeout*/
#define kCS_SI_MAX_WAITING_NIT_ACTUAL		20000     /* NIT actual table waiting timeout*/

#define kCS_SI_MAX_MSG					50

#define kCS_SI_TRIGRAM_MAX_LENGTH 						3

#define kCS_SI_MAX_NO_OF_PROGRAM_DESCRIPTORS_PER_SERVICE		5
#define kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE				20
#define kCS_SI_MAX_NO_OF_TS_DESCRIPTORS_PER_TS					6
#define kCS_SI_MAX_NO_OF_TS_PER_NETWORK							64
#define kCS_SI_MAX_NO_OF_SERVICE_PER_TS							128

#define 	kCS_SI_MAX_NO_OF_NOTIFY_CLIENTS				3
#define 	kCS_SI_INVALID_PID								0xffff

typedef char tCS_SI_Trigram[kCS_SI_TRIGRAM_MAX_LENGTH + 1]; 

typedef  enum
{
	eCS_SI_OK = 0,
	eCS_SI_ERROR,
	eCS_SI_DISCARD,
	eCS_SI_TIMEOUT
}tCS_SI_Report;

typedef enum
{
	eCS_SI_PAT_NOTIFY = 0,
	eCS_SI_PMT_NOTIFY,	
	eCS_SI_CAT_NOTIFY,
	eCS_SI_SDT_ACTUAL_NOTIFY,	
	eCS_SI_NIT_ACTUAL_NOTIFY,
	eCS_SI_MAX_NO_OF_SI_TYPE
}tCS_SI_NotifyType;

typedef enum
{
	eCS_SI_ES_TYPE_VIDEO = 0,
	eCS_SI_ES_TYPE_AUDIO,
	eCS_SI_ES_TYPE_TELETEXT,
	eCS_SI_ES_TYPE_SUBTITLE,
	eCS_SI_ES_TYPE_AUDIO_AC3,
	eCS_SI_ES_TYPE_AUDIO_AAC, /* Add By River 06.12.2008 */
	eCS_SI_ES_TYPE_AUDIO_LATM,/* Add By River 06.12.2008 */
	eCS_SI_ES_TYPE_VIDEO_MPEG4,
	eCS_SI_ES_TYPE_UNKNOWN
}tCS_SI_ESType;

typedef enum
{
	eCS_SI_AUDIO_PCM = 0,
	eCS_SI_AUDIO_AC3
}tCS_SI_AudioType;


typedef enum
{
	eCS_SI_Teletext_Reserved = 0,
	eCS_SI_Teletext_Initial_Teletext_Page,
	eCS_SI_Teletext_Subtitle_Page,
	eCS_SI_Teletext_Additional_Info_Page,
	eCS_SI_Teletext_Program_Schedule_Page
}tCS_SI_TeletextType;

typedef enum
{
	eCS_SI_TELETEXT_SUBTITLE = 0,
	eCS_SI_DVB_SUBTITLE
}tCS_SI_SubtitleType;


typedef enum
{
	eCS_SI_UNKOWN_SERVICE = 0,
	eCS_SI_TV_SERVICE,
	eCS_SI_RADIO_SERVICE,
	eCS_SI_HDTV_SERVICE,
	eCS_SI_DATA_SERVICE
}tCS_SI_ServiceType;

typedef enum 
{
	eCS_SI_UNKNOWN_DELIVERY = 0,
	eCS_SI_SATELLITE_DELIVERY,
	eCS_SI_TERRESTRIAL_DELIVERY
}tCS_SI_DeliveryType;


typedef struct
{
	U16	ServiceID;
	U16	PMT_PID;
}tCS_SI_PATProgram;

typedef struct
{
	BOOL IsValid;
	U16	TSID;
	U8 	PAT_Version;
	U8 	ServiceNumber;
	tCS_SI_PATProgram Programs[kCS_SI_MAX_NO_OF_SERVICE_PER_TS];
}tCS_SI_PAT_Info;

typedef struct
{
	U16	CA_System_ID;
	U16	ECM_PID;
}tCS_SI_PMT_CA_System;


#ifdef SUPPORT_CI

#define kCS_SI_CAM_MAX_CAdescs		8

typedef struct 
{
	U16 					nb_desc;			/* Number of CA Descr. */
	U8					*ca_desc[kCS_SI_CAM_MAX_CAdescs];			/* Pointer on CA descr. */
}tCS_SI_CA_Desc_s;

typedef struct 
{

	U8 					stream_type;	/* stream type */
	U16					pesPID;			/* PES component PID */
	tCS_SI_CA_Desc_s	es_CAdesc;			/*PES CA descriptor*/
}tCS_SI_PMT_ES_info_s;

typedef struct
{
         BOOL       IsValid;

	U16         prog_num;
	U8          ver_num;
	U8          current_next_indicator;
	tCS_SI_CA_Desc_s		prog_CAdesc;							/*Program CA descriptor*/
	U16						nb_streams;							/* Number of PES streams */
	tCS_SI_PMT_ES_info_s		*streamInfo[kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE];	/*Stream info */

}tCS_SI_CAPMT_Desc_s;

#endif


typedef struct
{
	U8	DescType ;
	union
	{		
		tCS_SI_PMT_CA_System      	CA_System;
	}uProgramInfo;
}tCS_SI_Program_Descriptor;

typedef struct
{
	U16                	CompositionPageID;
	U16                	AncillaryPageID;
}tCS_SI_DVBSubtitleDescriptor;

typedef struct
{
	U8                	MagazineID;
	U8                	PageID;
}tCS_SI_TeletextDescriptor;

typedef struct
{
	tCS_SI_SubtitleType                 				Subtitle_Type;
	union
	{
		tCS_SI_DVBSubtitleDescriptor		DVB_Subtitle;
		tCS_SI_TeletextDescriptor			TXT_Subtitle;
	}uSubtitleSpecific;
}tCS_SI_SubtitleDescriptor;

typedef struct
{
	U16	ESType ;
	U16	ESPID ;
	tCS_SI_Trigram		Language;
	union
	{		
		tCS_SI_SubtitleDescriptor  	ESSubtitleDesc;    
		tCS_SI_TeletextDescriptor 	ESTeletextDesc;
	}uStreamSpecific;
}tCS_SI_ESDescriptor;

typedef struct
{
	BOOL    IsValid;
	U16	    ServiceID;
	U16	    PCR_PID;
        U8     Scramble;
	U8	PMT_Version;
	
	U8	ProgDesc_Number;	
	tCS_SI_Program_Descriptor 	*Prog_Descriptors[kCS_SI_MAX_NO_OF_PROGRAM_DESCRIPTORS_PER_SERVICE]; 

	U8	ESNumber;
	tCS_SI_ESDescriptor     *ESDescriptors[kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE];
}tCS_SI_PMT_Info;

#ifdef __cplusplus
}
#endif

#endif 


