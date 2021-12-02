#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "csrec.h"

#define CSREC_OBJ_TYPE 		'p'
#define	REC_BUFF_SZ		(0x100000 * 2)

#define PAT_PID 		0x00
#define SDT_PID 		0x11
#define TDT_PID 		0x14
#define EIT_PID 		0x12
#define CAT_PID 		0x01

#define TS_SIZE 		188
#define SECTION_LEN 		1024

int private_pid_list[64];

typedef struct tagREC_OBJ 
{
	/* public */
	CSREC_InitParams params;

	/* private */
	char obj_type;
	char *record_buf;
	unsigned int record_sec;
	unsigned int pause_sec;

	FILE *record_fp;
	pthread_t record_task;
	CSREC_STATUS record_status;

        int used_filter_cnt;
        int private_pid_cnt;
        int currnet_pid_order;

	int is_stopped;
	
} CSREC_OBJ;

#if 0//remux
#define TS_SIZE                    188
#define SECTION_LENGTN 1024
#define PAT_PID 0x00
#define PAT_TAB_ID 0x00
#define SDT_PID 0x11
#define SDT_TAB_ID 0x42

char ts_pat_buf[TS_SIZE];
char ts_sdt_buf[TS_SIZE];

unsigned long Byte_CRC(unsigned char IByte, unsigned long ICrc)
{
        int   i; /* Loop Counter */
        unsigned long Crc=ICrc;
        
        /* Calcurate CRC */
        Crc = Crc ^ ( (unsigned long)IByte << 24 );

        for(i = 0 ; i < 8 ; i++ )
        {
                if( ( Crc & 0x80000000 ) != 0 ) {
                        Crc = ( Crc << 1 ) ^ 0x04c11db7;
                }
                else {
                        Crc = Crc << 1;
                }
        }
        return Crc;
}

unsigned long CRC(unsigned char* buf, unsigned int len)
{
        unsigned int   i;
        unsigned long Crc=0xffffffff;
        
        for(i = 0 ; i < len ; i++ )
        {
                Crc=Byte_CRC(buf[i],Crc);
        }
        return Crc;
}
#define PAT_SECTION_LENGTH 16
static void PATDataToSection(int ts_id,int pmt_id,int service_id)
{
        char sec_ver = 0;
        int CRCW = 0;
        char sec_buf[SECTION_LENGTN];
        static unsigned int pid_pat_cnt = 0;

        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = PAT_TAB_ID;
        sec_buf[1] = 0xb0|((13>>8)&0x0f);
        sec_buf[2] = 13&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (service_id>>8)&0xff;
        sec_buf[9] = service_id&0xff;
        sec_buf[10] = (pmt_id>>8)&0x1f;
        sec_buf[11] = pmt_id&0xff;

        CRCW = CRC(sec_buf,12);

        sec_buf[12] = (CRCW>>24)&0xff;
        sec_buf[13] = (CRCW>>16)&0xff;
        sec_buf[14] = (CRCW>>8)&0xff;
        sec_buf[15] = CRCW&0xff;

        memset(ts_pat_buf,0xff,TS_SIZE);

        ts_pat_buf[0] = 0x47;
        ts_pat_buf[1] = 0x40|((PAT_PID>>8)&0x1f);
        ts_pat_buf[2] = PAT_PID&0xff;
        ts_pat_buf[3] = 0x10|(pid_pat_cnt&0xf);
        pid_pat_cnt++;
        ts_pat_buf[4] = 0x00;

        memcpy(ts_pat_buf+5, sec_buf, PAT_SECTION_LENGTH);
}

static void SDTDataToSection(int ts_id,int net_id,int service_id,char* name,int name_len)
{
        char sec_ver = 0;
        int CRCW = 0;
        int des_len = 5 + name_len;
        int len = 17 + des_len;
        char sec_buf[SECTION_LENGTN];
        static unsigned int pid_sdt_cnt = 0;

        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = SDT_TAB_ID;
        sec_buf[1] = 0xb0|((len>>8)&0x0f);
        sec_buf[2] = len&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (net_id>>8)&0xff;
        sec_buf[9] = net_id&0xff;
        
        sec_buf[10] = 0;
        
        sec_buf[11] = (service_id>>8)&0xff;
        sec_buf[12] = service_id&0xff;

        sec_buf[13] = 0x1;

        sec_buf[14] = 0x50|((des_len>>8)&0x0f);
        sec_buf[15] = des_len&0xff;

        sec_buf[16] = 0x48;
        sec_buf[17] = (3+len)&0xff;

        sec_buf[18] = 0x1;

        sec_buf[19] = 0;

        sec_buf[20] = name_len;
        memcpy(sec_buf+21,name,name_len);

        CRCW = CRC(sec_buf,len+3-4);

        sec_buf[21+name_len] = (CRCW>>24)&0xff;
        sec_buf[22+name_len] = (CRCW>>16)&0xff;
        sec_buf[23+name_len] = (CRCW>>8)&0xff;
        sec_buf[24+name_len] = CRCW&0xff;

        memset(ts_sdt_buf,0xff,TS_SIZE);

        ts_sdt_buf[0] = 0x47;
        ts_sdt_buf[1] = 0x40|((SDT_PID>>8)&0x1f);
        ts_sdt_buf[2] = SDT_PID&0xff;
        ts_sdt_buf[3] = 0x10|(pid_sdt_cnt&0xf);
        pid_sdt_cnt++;
        ts_sdt_buf[4] = 0x00;

        memcpy(ts_sdt_buf+5, sec_buf, len+3);
}
#endif

static pthread_mutex_t pvr_mutex;
static struct timeval start, end, pausetime, resumetime,changepidtime;

static int pvr_task(int *param)
{
	int read_len = 0;
	int written_len = 0;
        time_t timeout_sec = 3,starttime_sec = 0;
        
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *)param;

	pvr_obj->record_fp = fopen(pvr_obj->params.filename, "w+b");
	if (pvr_obj->record_fp == NULL) 
		return -1;

        //PATDataToSection(pvr_obj->params.ts_id, pvr_obj->params.pmtpid, pvr_obj->params.service_id);
        //SDTDataToSection(pvr_obj->params.ts_id, pvr_obj->params.net_id, pvr_obj->params.service_id, pvr_obj->params.sdt_filename, strlen(pvr_obj->params.sdt_filename));

        gettimeofday(&changepidtime, NULL);
        starttime_sec = changepidtime.tv_sec;
        
	while (!pvr_obj->is_stopped) 
	{
		pthread_mutex_lock(&pvr_mutex);
	
		if (CSDEMUX_Filter_ReadWait(pvr_obj->params.secft_handle, 50) == CSAPI_SUCCEED) {
			if (CSDEMUX_Filter_CheckDataSize(pvr_obj->params.secft_handle, &read_len) == CSAPI_SUCCEED) {
				if (CSDEMUX_Filter_ReadData(pvr_obj->params.secft_handle, pvr_obj->record_buf, &read_len) == CSAPI_SUCCEED) 
				{
					written_len = fwrite(pvr_obj->record_buf, 1, read_len, pvr_obj->record_fp);
					if (written_len != read_len)
						printf("written = %d\n", written_len);

					if(written_len % TS_SIZE){
						;
					}
					else{
                                                if(pvr_obj->params.pid_num != pvr_obj->params.pidft_num){
                                                        gettimeofday(&changepidtime, NULL);
                                                        if((changepidtime.tv_sec - starttime_sec) > timeout_sec){
                                                                CSDEMUX_PIDFT_Disable(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt]);
                                                        	CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt], pvr_obj->params.pid_list[private_pid_list[pvr_obj->currnet_pid_order]].pid_val);
                                                                CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[private_pid_list[pvr_obj->currnet_pid_order]].pid_val, pvr_obj->used_filter_cnt);
                                                                CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt]);       
                                                                pvr_obj->currnet_pid_order ++;
                                                                if(pvr_obj->currnet_pid_order == pvr_obj->private_pid_cnt){
                                                                        pvr_obj->currnet_pid_order = 0;
                                                                }
                                                                starttime_sec = changepidtime.tv_sec;
                                                        }
                                                }
                                        }
				}
			}
		}
		else
			printf("CSREC ReadWait Timeout\n");

		pthread_mutex_unlock(&pvr_mutex);
	}

	return 0;
}

CSREC_HANDLE CSREC_Init(CSREC_InitParams * params)
{
	CSREC_OBJ *pvr_obj = NULL;
        int i = 0,cnt = 0;
        if((params->pidft_num > REQUIRED_PIDFT_NUM)||(params->pid_num > 64)){
            printf("error: %s %d %s\n",__FILE__,__LINE__,__FUNCTION__);
            return NULL;
        }

        for(i =0;i < params->pid_num;i++){
                if(params->pid_list[i].pid_type != PID_TYPE_PRIVATE){
                    cnt ++;
                }
        }
        if(params->pidft_num < cnt){
                printf("error: %s %d %s\n",__FILE__,__LINE__,__FUNCTION__);
                return NULL;
        }
        
        pvr_obj = malloc(sizeof(CSREC_OBJ));
	if (NULL == pvr_obj) {
		printf("error: %s  %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	memset(pvr_obj, 0, sizeof(CSREC_OBJ));

	pvr_obj->record_buf = malloc(REC_BUFF_SZ);
	if (NULL == pvr_obj->record_buf) {
		printf("error: %s  %s\n", __FUNCTION__, strerror(errno));
		free(pvr_obj);
		return NULL;
	}

	memcpy(&(pvr_obj->params), params, sizeof(CSREC_InitParams));

	CSDEMUX_Filter_SetFilterType(pvr_obj->params.secft_handle, DEMUX_FILTER_TYPE_TS);

	pthread_mutex_init(&pvr_mutex, NULL);

	pvr_obj->record_status = REC_INIT;
	pvr_obj->obj_type = CSREC_OBJ_TYPE;

	return (CSREC_HANDLE) pvr_obj;
}

CSAPI_RESULT CSREC_Term(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (REC_PAUSE == pvr_obj->record_status)
		CSREC_Resume(handle);

	if (REC_RUN == pvr_obj->record_status)
		CSREC_Stop(handle);

	pthread_mutex_destroy(&pvr_mutex);

        free(pvr_obj->record_buf);
	memset(pvr_obj, 0, sizeof(CSREC_OBJ));
	free(pvr_obj);
        pvr_obj = NULL;
        
	return CSAPI_SUCCEED;
}

static signed int __set_pthread_prioity__(pthread_t i_thread,  signed int i_priority)
{
	signed int i_policy = 0;

	struct sched_param param;

	param.sched_priority = i_priority;
	i_policy = SCHED_RR;

	pthread_setschedparam(i_thread, i_policy, &param);            

	return 0;
}

CSAPI_RESULT CSREC_Start(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;
        int cnt = 0;
        
	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if ((pvr_obj->record_status == REC_PAUSE) || (pvr_obj->record_status == REC_RUN)) {
		return CSAPI_FAILED;
	}

        if(pvr_obj->params.pid_num == pvr_obj->params.pidft_num){
                for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++){
                        CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[cnt], pvr_obj->params.pid_list[cnt].pid_val);
                        CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, cnt);
                        CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[cnt]);
                }
        }
        else if(pvr_obj->params.pid_num > pvr_obj->params.pidft_num){
                for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++){
                        if(pvr_obj->params.pid_list[cnt].pid_type != PID_TYPE_PRIVATE){
                                CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt], pvr_obj->params.pid_list[cnt].pid_val);
                                CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, pvr_obj->used_filter_cnt);
                                CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt]);
                                pvr_obj->used_filter_cnt ++;
                        }
                        else{
                                private_pid_list[pvr_obj->private_pid_cnt] = cnt;
                                pvr_obj->private_pid_cnt ++;
                        }
                }
                pvr_obj->currnet_pid_order = 0;
        }
        else{
                printf("PVR Warning: wanted pids are less then malloced pid filters, waste pid filter!");
                for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++){
                        CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[cnt], pvr_obj->params.pid_list[cnt].pid_val);
                        CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, cnt);
                        CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[cnt]);
                }
        }

	CSDEMUX_Filter_Enable(pvr_obj->params.secft_handle);

	pvr_obj->record_sec = 0;
	pvr_obj->pause_sec = 0;
	
	pvr_obj->is_stopped = 0;
	
	gettimeofday(&start, NULL);
	pthread_create(&pvr_obj->record_task, NULL, (void *) pvr_task, pvr_obj);
	__set_pthread_prioity__(pvr_obj->record_task, 0x60);

	pvr_obj->record_status = REC_RUN;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSREC_Stop(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;
        int i = 0;
        
	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if ((pvr_obj->record_status == REC_INIT) || (pvr_obj->record_status == REC_PAUSE)
	    || (pvr_obj->record_status == REC_STOP)) {
		return CSAPI_FAILED;
	}

	pvr_obj->is_stopped = 1;

	pthread_mutex_lock(&pvr_mutex);
	pthread_cancel(pvr_obj->record_task);

	gettimeofday(&end, NULL);

	if(pvr_obj->record_fp)
		fclose(pvr_obj->record_fp);

	CSDEMUX_Filter_Disable(pvr_obj->params.secft_handle);

        for(i = 0;i < pvr_obj->params.pidft_num;i ++){
	        CSDEMUX_PIDFT_Disable(pvr_obj->params.pidft_handle[i]);
        }
	pvr_obj->record_status = REC_STOP;

	pthread_mutex_unlock(&pvr_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSREC_Pause(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (pvr_obj->record_status == REC_RUN) 
	{
		pthread_mutex_lock(&pvr_mutex);

		gettimeofday(&pausetime, NULL);
		pvr_obj->record_status = REC_PAUSE;

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSREC_Resume(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (pvr_obj->record_status == REC_PAUSE) 
	{
		pthread_mutex_unlock(&pvr_mutex);

		gettimeofday(&resumetime, NULL);
		pvr_obj->pause_sec = resumetime.tv_sec - pausetime.tv_sec + pvr_obj->pause_sec;
		pvr_obj->record_status = REC_RUN;

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSREC_GetTime(CSREC_HANDLE handle, unsigned int *secs)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	switch(pvr_obj->record_status) 
	{
		case REC_STOP:
			pvr_obj->record_sec = end.tv_sec - start.tv_sec - pvr_obj->pause_sec;
			break;

		case REC_RUN:
			gettimeofday(&end, NULL);
			pvr_obj->record_sec = end.tv_sec - start.tv_sec - pvr_obj->pause_sec;
			break;

		case REC_PAUSE:
			break;

		default:
			pvr_obj->record_sec = 0;
			break;
	}

	*secs = pvr_obj->record_sec;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSREC_GetStatus(CSREC_HANDLE handle, CSREC_STATUS * status)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	*status = pvr_obj->record_status;

	return CSAPI_SUCCEED;
}

