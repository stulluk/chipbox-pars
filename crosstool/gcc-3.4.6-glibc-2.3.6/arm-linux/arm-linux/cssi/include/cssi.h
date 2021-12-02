#ifndef __CSSI_H__
#define __CSSI_H__

#include "global.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#define SI_PIDFT_NUM 	16

typedef void* CSSI_HANDLE;

typedef struct tagSI_InitParams 
{
	unsigned int filter_num; /* to indicates that allocate how many pid filter to CSSI module. */

	CSDEMUX_HANDLE pidft_handle[SI_PIDFT_NUM]; /* a pid filter handle array. */
	CSDEMUX_HANDLE secft_handle[SI_PIDFT_NUM]; /* a section filter handler. */

	FILE *ts_fp;

} CSSI_InitParams;

typedef struct tagSI_ServiceInfo 
{
	int service_id;
	char service_name[32];
	
	int apid;
	int vpid;
	int pmtpid;

} CSSI_ServiceInfo;

CSSI_HANDLE  CSSI_Init(CSSI_InitParams *params);
CSAPI_RESULT CSSI_Term(CSSI_HANDLE handle);

CSAPI_RESULT CSSI_GetServiceInfoFromStream(CSSI_HANDLE handle, unsigned int service_id, CSSI_ServiceInfo *info);
CSAPI_RESULT CSSI_GetServiceInfoFromFile(CSSI_HANDLE handle, unsigned int service_id, CSSI_ServiceInfo *info);

#ifdef __cplusplus
}
#endif

#endif /* end of __CSSI_H__ */

