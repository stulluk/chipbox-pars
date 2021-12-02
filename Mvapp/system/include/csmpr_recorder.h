#ifndef __CSREC_H__
#define __CSREC_H__

#include "linuxos.h"
#include "global.h"
//#include "../csdemux/include/csdemux.h"
#include "csmpr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REC_BUFF_SZ             		(0x100000 * 2)
#define PAT_PID 		            	0x00
#define SDT_PID 		            	0x11
#define TDT_PID 		            	0x14
#define EIT_PID 		            	0x12
#define CAT_PID 		            	0x01
#define TS_SIZE 		            	188
#define SECTION_LEN              		1024
#define CSREC_MAX_FILE_SIZE     		(188*10*1024*1024L)
#define CSMPR_TIMESHIFT_MAX_FILE_SIZE 	CSREC_MAX_FILE_SIZE

#define CSMPR_USB_MOUNT_DIR      		"/mnt/usb/disk1/RECORDS"

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

typedef enum
{
    CSMPR_REC_INIT = 0,
    CSMPR_REC_IDLE,
    CSMPR_REC_RUN,
    CSMPR_REC_PAUSE
} CSREC_STATUS;


typedef enum
{
    CSMPR_REC_NORMAL = 0,
    CSMPR_REC_TIMESHIFT
} CSMPR_RECORD_MODE;


typedef struct tagREC_InitParams
{
	struct
   {
		int pid_val;
		CSREC_PID_TYPE pid_type;
	} pid_list[64];

	int pid_num;

	CSDEMUX_HANDLE pidft_handle[REQUIRED_PIDFT_NUM];
	int pidft_num;
	int pmt_pid;

	CSDEMUX_HANDLE secft_handle; /* a section filter handler for PVR. */

	char filename[128]; /* a file name indicates which file will be used to save TS stream. */

} CSREC_InitParams;


typedef struct tagREC_OBJ
{
   CSREC_STATUS     record_status;
   CSREC_InitParams params;
   int              is_stopped;

   char            *record_buf;
   unsigned int     record_sec;

   unsigned long    written_cnt;  //  TODO:should be long long
   int              file_no;

   FILE             *record_fp;
   pthread_t        record_task;
} CSREC_OBJ;

CSMPR_RESULT CSMPR_Record_Init( void );
CSMPR_RESULT CSMPR_Record_Term( void );
CSMPR_RESULT CSMPR_Record_Start( void );

/************v38.......****************************/
CSMPR_RESULT CSMPR_Streaming_Start( void );

/****************************************************/

CSMPR_RESULT CSMPR_Record_Stop( void );
CSMPR_RESULT CSMPR_Record_GetFileName( char* filename );
unsigned int CSMPR_Record_GetTime( void );
CSREC_STATUS CSMPR_Record_GetStatus( void );
unsigned int CSMPR_Record_GetCurrFileSize( void );
CSMPR_RECORD_MODE CSMPR_Record_GetMode( void );
int MV_PVR_FileWrite_Time(char *PVR_filename, char *Event_name, U16 u16Ch_Index, eCS_PVR_FILE_STATUS eCS_Status);

#ifdef __cplusplus
}
#endif

#endif /* end of __CSPVR_H__ */

