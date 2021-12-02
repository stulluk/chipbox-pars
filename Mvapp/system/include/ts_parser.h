#ifndef _TS_PARSER_H_
#define _TS_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "database.h"
#include "eit_engine.h"

typedef enum
{
	CS_PARSER_NO_ERROR = 0,
	CS_PARSER_ERROR,
	CS_PARSER_GETPF_RETURN_P,
	CS_PARSER_GETPF_RETURN_F,
}tCS_Parser_Error_t;

#if 0
unsigned char GetServiceInfo(unsigned short service_id, tCS_DB_ServiceData *service_data);
// unsigned char GetPF(U16 service_id, tCS_EIT_Event_t *eit_event_present, tCS_EIT_Event_t *eit_event_follow);
#endif

void ParserInit(void);
U8 GetPATDataFromFile(const char* filename);
U8 GetPMTDataFromFile(const char* filename);
U8 GetTDTTOTEITDataFromFile(const char* filename);
U8 GetSDTDataFromFile(const char* filename);
U8 GetPMTDataFromFileByServiceID(const char* filename, U16 service_id);

#ifdef __cplusplus
}
#endif

#endif  // _TS_PARSER_H_


