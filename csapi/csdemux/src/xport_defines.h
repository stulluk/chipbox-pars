#ifndef __CSAPI_DEMUX_DEFINE_H__
#define __CSAPI_DEMUX_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "csdemux.h"

#define  CSDEMUX_DEV_FILE              "/dev/orion_xport/orion_xport"
#define  CSDEMUX_CHL0_FILE             "/dev/orion_xport/video0"
#define  CSDEMUX_CHL1_FILE             "/dev/orion_xport/video1"

/* Object and Handle define */
typedef enum
{
	DEMUX_OBJ_DES = 0,
	DEMUX_OBJ_PIDFT,
	DEMUX_OBJ_CRC,
	DEMUX_OBJ_CHL,
	DEMUX_OBJ_VIDOUT,
	DEMUX_OBJ_AUDOUT,
	DEMUX_OBJ_PCR,
	DEMUX_OBJ_FILTER
} CSDEMUX_OBJ_TYPE;

typedef struct tagCSDEMUX_DES
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_DES_ID        des_id;
        CSDEMUX_PIDFT_ID      pid_filter_id;
        int                   dev_fd;       
	int                   errno;
} CSDEMUX_DES;

typedef struct tagCSDEMUX_PIDFT
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_PIDFT_ID      pid_filter_id;
        int                   dev_fd;
	int                   errno;
} CSDEMUX_PIDFT;

typedef struct tagCSDEMUX_CHL
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_CHL_ID        chl_id;

	int                   input_mode;
	int                   chl_en;
        int                   dev_fd;       
	int                   errno;
} CSDEMUX_CHL;

typedef struct tagCSDEMUX_VIDOUT
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_VIDOUT_ID     vidout_id;
	unsigned int          vidout_pid;
	unsigned int          vidout_en;
        unsigned int          output_block_flags;
        int                   dev_fd;
	int                   errno;
} CSDEMUX_VIDOUT;

typedef struct tagCSDEMUX_AUDOUT
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_AUDOUT_ID     audout_id;
	unsigned int          audout_pid;
	unsigned int          audout_en;
        unsigned int          output_block_flags;
        int                   dev_fd;
	int                   errno;
} CSDEMUX_AUDOUT;

typedef struct tagCSDEMUX_PCR
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_PCRDEV_ID     pcr_id;
        int                   dev_fd;
	int                   errno;
} CSDEMUX_PCR;

typedef struct tagCSDEMUX_CRC
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_CRC_ID        crc_id;
	CSDEMUX_FILTER_ID     filter_id;
        int                   dev_fd;     
	int                   errno;
} CSDEMUX_CRC;

typedef struct tagCSDEMUX_FILTER
{
	CSDEMUX_OBJ_TYPE      obj_type; 
	CSDEMUX_FILTER_TYPE   filter_type; 
	CSDEMUX_FILTER_ID     filter_id;
	CSDEMUX_CRC_ID        crc_id;
	char                   filter_crc_save; /* it's only useful for DEMUX_SECTION_AVAIL event. */
       char                   filter_crc_enable;
	pthread_mutex_t filter_mutex;
	int filter_timeout_ms;
	
	struct 
	{
		void (*notify_func) (CSDEMUX_HANDLE, CSDEMUX_SECEVENT *);

	} call_lst[CSDEMUX_FILTER_EVENT_NUM];

        int                   dev_fd;
	int                   errno;
} CSDEMUX_FILTER;

#ifdef __cplusplus
}
#endif

#endif

