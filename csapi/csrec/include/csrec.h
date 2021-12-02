#ifndef __CSREC_H__
#define __CSREC_H__

#include "global.h"
#include "../csdemux/include/csdemux.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REQUIRED_PIDFT_NUM	12

typedef void* CSREC_HANDLE;

typedef enum 
{
	PID_TYPE_VIDEOTS = 1,
	PID_TYPE_AUDIOTS,
	PID_TYPE_SUBTITLE,
	PID_TYPE_TELETEXT,
	PID_TYPE_PRIVATE

} CSREC_PID_TYPE;

typedef struct tagREC_InitParams 
{
	struct {
		int pid_val;
		CSREC_PID_TYPE pid_type;
	} pid_list[64];
	int pid_num;
	
	CSDEMUX_HANDLE pidft_handle[REQUIRED_PIDFT_NUM];
	int pidft_num;
	int pmt_pid;
		
	CSDEMUX_HANDLE secft_handle; /* a section filter handler for PVR. */
	
	char filename[128]; /* a file name indicates which file will be used to save 
TS stream. */

} CSREC_InitParams;

typedef enum 
{
        REC_INIT,
        REC_RUN,
        REC_PAUSE,
        REC_STOP

} CSREC_STATUS;

CSREC_HANDLE CSREC_Init(CSREC_InitParams *params);
CSAPI_RESULT CSREC_Term(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_Start(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_Stop(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_Pause(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_Resume(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_GetTime(CSREC_HANDLE handle, unsigned int *secs);
CSAPI_RESULT CSREC_GetStatus(CSREC_HANDLE handle, CSREC_STATUS *status);

#if 0//not implment
CSAPI_RESULT CSREC_InsertTSEnable(CSREC_HANDLE handle);
CSAPI_RESULT CSREC_InsertTSData(CSREC_HANDLE handle, void *buf, int pkt_num);
CSAPI_RESULT CSREC_InsertTSDisable(CSREC_HANDLE handle);
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of __CSPVR_H__ */
