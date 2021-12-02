#ifndef __DB_BUILDER_PRIV_H
#define __DB_BUILDER_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif


#define kCS_SI_MAX_SECTION_NUMBER_DIVIDE32			8	/*SECTION NUMBER×î´óÖµ255*/
#define kCS_SI_NO_OF_CRC_DATA_BYTES					4

typedef enum
{
	eCS_SI_SECTION,
	eCS_SI_ENABLEPAT,
	eCS_SI_ENABLEPMT,	
	eCS_SI_ENABLECAT,
	eCS_SI_ENABLESDTACTUAL,	
	eCS_SI_ENABLENITACTUAL,
	eCS_SI_DISABLEPAT,
	eCS_SI_DISABLEPMT,
	eCS_SI_DISABLECAT,
	eCS_SI_DISABLESDTACTUAL,	
	eCS_SI_DISABLENITACTUAL
}tCS_SI_MsgType;

typedef enum
{
	eCS_SI_PARSE_NO_DESCRIPTOR,	
	eCS_SI_PARSE_PAT_DESCRIPTOR,
	eCS_SI_PARSE_PMT_DESCRIPTOR,
	eCS_SI_PARSE_CAT_DESCRIPTOR,
	eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR,
	eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR,
	eCS_SI_PARSE_NIT_ACTUAL_DESCRIPTOR,
	eCS_SI_PARSE_NIT_OTHER_DESCRIPTOR,
	eCS_SI_PARSE_ES_DESCRIPTOR,
	eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR,
	eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR
}tCS_SI_DescriptorToParse;


typedef struct
{
	U8 			*buf;
	union
	{
		U32		length;
		S32		value;
	}data;
	tCS_SI_MsgType	type;
	DB_FilterHandle 	FilterHandle;
	BOOL 		IsTimeout;
}tCS_SI_Msg_t;

typedef struct
{
	U32	toFilter;
	U32	filtered;
}tCS_SI_SectionNumberMask;

typedef struct
{
	U8 *BufferBase;
	U32 *Section_StartPtr;
	U32 *Section_StopPtr;
}tCS_SI_TableBuffer;

#ifdef __cplusplus
}
#endif

#endif 

