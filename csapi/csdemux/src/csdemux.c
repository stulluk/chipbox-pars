#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../csevt/include/csevt.h"

#include "xport_defines.h"
#include "csdemux.h"

#ifdef CSDEMUX_DEBUG
#define DEBUG_PRINTF  printf
#else
#define DEBUG_PRINTF(fmt,args...)
#endif

#define XPORT_FILTER_IOC_PID0           _IOW('k', 1, int)
#define XPORT_FILTER_IOC_PID1           _IOW('k', 2, int)
#define XPORT_FILTER_IOC_PID2           _IOW('k', 3, int)
#define XPORT_FILTER_IOC_PID3           _IOW('k', 4, int)
#define XPORT_FILTER_IOC_FILTER         _IOW('k', 5, int)

#define XPORT_FILTER_IOC_TYPE           _IOW('k', 6, int)
#define XPORT_FILTER_IOC_ENABLE         _IOW('k', 8, int)

#define XPORT_FILTER_IOC_QUERY_NUM      _IOR('k', 10, int)
#define XPORT_FILTER_IOC_QUERY_SIZE     _IOR('k', 11, int)

#define XPORT_FILTER_IOC_CRC_ENABLE	_IOW('k', 12, int)

#define XPORT_PIDFT_IOC_ENABLE		_IOW('k', 13, int)
#define XPORT_PIDFT_IOC_CHANNEL		_IOW('k', 14, int)
#define XPORT_PIDFT_IOC_PIDVAL		_IOW('k', 15, int)
#define XPORT_PIDFT_IOC_DESC_ODDKEY	_IOW('k', 16, int)
#define XPORT_PIDFT_IOC_DESC_EVENKEY	_IOW('k', 17, int)
#define XPORT_PIDFT_IOC_DESC_ENABLE	_IOW('k', 18, int)

#define XPORT_CHL_IOC_ENABLE		_IOW('k', 19, int)
#define XPORT_CHL_IOC_INPUT_MODE	_IOW('k', 20, int)
#define XPORT_CHL_IOC_RESET		_IOW('k', 21, int)
#define XPORT_CHL_IOC_DMA_RESET		_IOW('k', 22, int)

#define XPORT_VID_IOC_OUTPUT_MODE	_IOW('k', 23, int)
#define XPORT_VID_IOC_RESET		_IOW('k', 24, int)
#define XPORT_VID_IOC_ENABLE		_IOW('k', 25, int)
#define XPORT_VID_IOC_PIDVAL		_IOW('k', 26, int)

#define XPORT_AUD_IOC_OUTPUT_MODE	_IOW('k', 27, int)
#define XPORT_AUD_IOC_RESET		_IOW('k', 28, int)
#define XPORT_AUD_IOC_ENABLE		_IOW('k', 29, int)
#define XPORT_AUD_IOC_PIDVAL		_IOW('k', 30, int)

#define XPORT_PCR_IOC_ENABLE		_IOW('k', 31, int)
#define XPORT_PCR_IOC_GETVAL		_IOW('k', 32, int)
#define XPORT_PCR_IOC_PIDVAL		_IOW('k', 33, int)

#define XPORT_FW_INIT			_IOW('k', 34, int)

#define XPORT_FILTER_IOC_CRC_NOTIFY_ENABLE _IOW('k', 35, int)
#define XPORT_FILTER_IOC_SAVE_ENABLE     _IOW('k', 36, int)

#define XPORT_FILTER_IOC_PID4           _IOW('k', 37, int)
#define XPORT_FILTER_IOC_PID5           _IOW('k', 38, int)
#define XPORT_FILTER_IOC_PID6           _IOW('k', 39, int)
#define XPORT_FILTER_IOC_PID7           _IOW('k', 40, int)
#define XPORT_FILTER_IOC_PID8           _IOW('k', 41, int)
#define XPORT_FILTER_IOC_PID9           _IOW('k', 42, int)
#define XPORT_FILTER_IOC_PID10           _IOW('k', 43, int)
#define XPORT_FILTER_IOC_PID11           _IOW('k', 44, int)

#define XPORT_CHL_IOC_CLEAR		_IOW('k', 45, int)
#define XPORT_CHL_IOC_TUNER_MODE	_IOW('k', 46, int)

#define XPORT_VID_IOC_SWITCH		_IOW('k', 47, int)
#define XPORT_AUD_IOC_SWITCH		_IOW('k', 48, int)
#define XPORT_FILTER_IOC_SWITCH		_IOW('k', 49, int)
#define XPORT_CHL_IOC_DES_MODE		_IOW('k', 50, int)

#if 0
#define FILTER_LOCK(x)	do { \
						DEBUG_PRINTF("thread id : %d,function: %s,line: %d, before lock\n",getpid(),__FUNCTION__,__LINE__);\
						(void)pthread_mutex_lock(&x);\
						DEBUG_PRINTF("thread id : %d,function: %s,line: %d, after lock\n",getpid(),__FUNCTION__,__LINE__); \
						}while(0)
							
#define FILTER_UNLOCK(x)	do { \
							DEBUG_PRINTF("thread id : %d,function: %s,line: %d, before unlock\n",getpid(),__FUNCTION__,__LINE__);\
							(void)pthread_mutex_unlock(&x);\
							DEBUG_PRINTF("thread id : %d,function: %s,line: %d, after unlock\n",getpid(),__FUNCTION__,__LINE__); \
							}while(0)
#else
#define FILTER_LOCK(x)							
#define FILTER_UNLOCK(x)
#endif
							
#define DEFAULT_FILTER_TIMEOUT 500;

typedef struct __ioctl_params__ {
	unsigned int pid_idx;
	unsigned int pid_en;
	unsigned int pid_val;
	unsigned int pid_chl;
	unsigned int des_idx;
	unsigned int des_en;
	unsigned int des_len;
	unsigned int des_key[6];

	unsigned int avout_idx;
	unsigned int avout_en;
	unsigned int avout_pid;
	unsigned int avout_mode;
	unsigned int avout_chl_switch;

	unsigned int filter_idx;
	unsigned int filter_en;
	unsigned int filter_crc_idx;
	unsigned int filter_crc_en;
	unsigned int filter_crc_save;
	unsigned int filter_crc_notify_en;

	unsigned int pcr_idx;
	unsigned int pcr_en;
	unsigned int pcr_pid;
	unsigned int pcr_hi_val;
	unsigned int pcr_lo_val;

} xport_ioctl_params;

static int xport_dev_fd = -1;	/* global device file descriptor */

static CSEVT_HANDLE xport_evt;	/* EVENT handler */

static CSDEMUX_DES xport_des_obj[CSDEMUX_DES_NUM];	/* descrambler objects */
static CSDEMUX_CRC xport_crc_obj[CSDEMUX_CRC_NUM];	/* CRC objects */
static CSDEMUX_PIDFT xport_pidft_obj[CSDEMUX_PIDFT_NUM];	/* PID filter objects */
static CSDEMUX_CHL xport_chl_obj[CSDEMUX_CHL_NUM];	/* input channel objects */
static CSDEMUX_VIDOUT xport_vidout_obj[CSDEMUX_VIDOUT_NUM];	/* video output objects */
static CSDEMUX_AUDOUT xport_audout_obj[CSDEMUX_AUDOUT_NUM];	/* audio output objects */
static CSDEMUX_PCR xport_pcr_obj[CSDEMUX_PCR_NUM];	/* PCR objects */
static CSDEMUX_FILTER xport_filter_obj[CSDEMUX_FILTER_NUM];	/* section filter objects */

static int __chl_dma_mmap__(void);
static int __chl_dma_ummap__(void);
static int __chl_dma_half_empty_check__(void);

static void __filter_free_crc(CSDEMUX_FILTER_ID filter_id);
static CSAPI_RESULT __filter_disable_crc(CSDEMUX_FILTER_ID filter_id, int fd);

/* 
 * FIXME@zhongkai's code, pls add downloading firmware codes to kernel space.
 */
CSAPI_RESULT CSDEMUX_Init(void)
{
	int i = 0;
	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN",getpid(),__FUNCTION__,__LINE__);
	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		xport_des_obj[i].obj_type = -1;
		xport_des_obj[i].des_id = -1;
		xport_des_obj[i].pid_filter_id = -1;
		xport_des_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_CRC_NUM; i++) {
		xport_crc_obj[i].obj_type = -1;
		xport_crc_obj[i].crc_id = -1;
		xport_crc_obj[i].filter_id = -1;
		xport_crc_obj[i].dev_fd = -1;
	}

	memset(xport_pidft_obj, 0, sizeof(xport_pidft_obj));
	memset(xport_chl_obj, 0, sizeof(xport_chl_obj));
	memset(xport_vidout_obj, 0, sizeof(xport_vidout_obj));
	memset(xport_audout_obj, 0, sizeof(xport_audout_obj));
	memset(xport_pcr_obj, 0, sizeof(xport_pcr_obj));
	memset(xport_filter_obj, 0, sizeof(xport_filter_obj));
	
	for (i = 0; i < CSDEMUX_CHL_NUM; i++) {
		xport_chl_obj[i].obj_type = -1;
		xport_chl_obj[i].chl_id = -1;
		xport_chl_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_PIDFT_NUM; i++) {
		xport_pidft_obj[i].obj_type = -1;
		xport_pidft_obj[i].pid_filter_id = -1;
		xport_pidft_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_CHL_NUM; i++) {
		xport_pcr_obj[i].obj_type = -1;
		xport_pcr_obj[i].pcr_id = -1;
		xport_pcr_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_VIDOUT_NUM; i++) {
		xport_vidout_obj[i].obj_type = -1;
		xport_vidout_obj[i].vidout_id = -1;
		xport_vidout_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_AUDOUT_NUM; i++) {
		xport_audout_obj[i].obj_type = -1;
		xport_audout_obj[i].audout_id = -1;
		xport_audout_obj[i].dev_fd = -1;
	}

	for (i = 0; i < CSDEMUX_FILTER_NUM; i++) {
		xport_filter_obj[i].obj_type = -1;
		xport_filter_obj[i].filter_id = -1;
		xport_filter_obj[i].dev_fd = -1;
		xport_filter_obj[i].filter_timeout_ms = DEFAULT_FILTER_TIMEOUT;
		pthread_mutex_init(&xport_filter_obj[i].filter_mutex, NULL);
		FILTER_LOCK(xport_filter_obj[i].filter_mutex);
	}

	xport_evt = CSEVT_Init();
	if (NULL == xport_evt){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT1\n",getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* failed to create event handler. */
	}
	xport_dev_fd = open(CSDEMUX_DEV_FILE, O_RDWR);
	if (xport_dev_fd < 0){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT2\n",getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	ioctl(xport_dev_fd, XPORT_FW_INIT, 0);	/* load firmware */

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT3\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

void CSDEMUX_Terminate(void)
{
	/* 
	 * FIXME@zhongkai's code, I think we should also close all of other devices.
	 */
	 int i = 0;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CSEVT_Term(xport_evt);

	for (i = 0; i < CSDEMUX_FILTER_NUM; i++) {
		pthread_mutex_destroy(&xport_filter_obj[i].filter_mutex);
	 }

	 close(xport_dev_fd);

	 xport_dev_fd = -1;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
}

/* 
 * pid filter interfaces 
 */
CSDEMUX_HANDLE CSDEMUX_PIDFT_Open(CSDEMUX_PIDFT_ID pid_filter_id)
{
	DEBUG_PRINTF("pidfilterid %d, thread id : %d,function: %s,line: %d IN\n",pid_filter_id,getpid(),__FUNCTION__,__LINE__);

	if (xport_dev_fd <= 0){
		DEBUG_PRINTF("pidfilterid %d, thread id : %d,function: %s,line: %d, OUT1\n",pid_filter_id,getpid(),__FUNCTION__,__LINE__);	
		return NULL;	/* must open xport device firstly */
	}
	
	if (pid_filter_id >= CSDEMUX_PIDFT_NUM){
		DEBUG_PRINTF("pidfilterid %d, thread id : %d,function: %s,line: %d, OUT2\n",pid_filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* incorrect pid filter you selected. */
	}
	if (xport_pidft_obj[pid_filter_id].dev_fd > 0){
		DEBUG_PRINTF("pidfilterid %d, thread id : %d,function: %s,line: %d, OUT3\n",pid_filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* this filter already opened. */
	}
	xport_pidft_obj[pid_filter_id].obj_type = DEMUX_OBJ_PIDFT;
	xport_pidft_obj[pid_filter_id].pid_filter_id = pid_filter_id;
	xport_pidft_obj[pid_filter_id].dev_fd = xport_dev_fd;

	DEBUG_PRINTF("pidfilterid %d, thread id : %d,function: %s,line: %d OUT4\n",pid_filter_id,getpid(),__FUNCTION__,__LINE__);
	DEBUG_PRINTF("handle = 0x%x\n",(unsigned int)&(xport_pidft_obj[pid_filter_id]));
	return &(xport_pidft_obj[pid_filter_id]);
}

CSAPI_RESULT CSDEMUX_PIDFT_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	CSDEMUX_PIDFT_DisableDES(handle);
	CSDEMUX_PIDFT_FreeDES(handle);
	CSDEMUX_PIDFT_Disable(handle);

	//memset(pidft_ptr, 0, sizeof(CSDEMUX_PIDFT));

	pidft_ptr->obj_type = -1;
	pidft_ptr->pid_filter_id = DEMUX_PIDFT_UNDEF;
	pidft_ptr->dev_fd = -1;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetChannel(CSDEMUX_HANDLE handle, CSDEMUX_CHL_ID chl_id)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	/* 
	 * write bit30 of PID_FILTERx for selecting a input channel. 
	 */
	regs_val.pid_chl = chl_id;
	regs_val.pid_idx = pidft_ptr->pid_filter_id;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_CHANNEL, &regs_val, DEMUX);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	/* write bit0~12 of PID_FILTERx for setting a PID value. */
	regs_val.pid_val = pid;
	regs_val.pid_idx = pidft_ptr->pid_filter_id;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_PIDVAL, &regs_val, DEMUX);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_GetFreeDESNumber(CSDEMUX_HANDLE handle, unsigned int *des_number)
{
	int i, des_nums = 0;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	UNUSED_VARIABLE(handle);

	if (NULL == des_number){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		if (xport_des_obj[i].dev_fd <= 0)
			des_nums++;
	}

	*des_number = des_nums;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_MallocDES(CSDEMUX_HANDLE handle)
{
	int i;
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	for (i = 0; i < CSDEMUX_DES_NUM; i++){
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id){
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;	/* already allocated a DES key for this pid filter */
		}
	}
	
	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		if (xport_des_obj[i].dev_fd <= 0) {
			break;	/* find out a empty DES slot */
		}
	}

	if (i == CSDEMUX_DES_NUM){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* no DES slot be found. */
	}
	
	xport_des_obj[i].obj_type = DEMUX_OBJ_DES;
	xport_des_obj[i].des_id = i;
	xport_des_obj[i].pid_filter_id = pidft_ptr->pid_filter_id;
	xport_des_obj[i].dev_fd = pidft_ptr->dev_fd;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_FreeDES(CSDEMUX_HANDLE handle)
{
	int i;
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		/* 
		 * find out a DES slot which was associated with PID filter 
		 */
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id) {
			xport_des_obj[i].des_id = -1;
			xport_des_obj[i].pid_filter_id = -1;
			xport_des_obj[i].dev_fd = -1;
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_SUCCEED;
		}
	}

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetDESOddKey(CSDEMUX_HANDLE handle, const unsigned char *const odd_key)
{
	unsigned int i = 0;
	xport_ioctl_params regs_val;

	CSDEMUX_PIDFT *pidft_ptr = handle;

	/* FIXME@zhongkai's ugly code. */
	struct __desc_key__ {
		unsigned int key_len;
		unsigned int key[6];
	} *const key = (struct __desc_key__ *) odd_key;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	if (NULL == odd_key){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* no odd key */
	}
	
	regs_val.des_len = key->key_len;
	memcpy(regs_val.des_key, key->key, sizeof(unsigned int) * 6);

	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id) {
			/* write a odd key to specified DES slot */
			/* wrtie this value to DESC_ODD_xx regiters. */
			/* __DESC_ODD_ADDR__(xport_des_obj[i].des_id, j) (0x41400000 + 0x280 + 4 * 12 * i + 4 * j) */

			regs_val.des_idx = i;	/* des key slot */
			IOCTL(pidft_ptr, XPORT_PIDFT_IOC_DESC_ODDKEY, &regs_val, DEMUX);
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_SUCCEED;
		}
	}

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetDESEvenKey(CSDEMUX_HANDLE handle, const unsigned char *const even_key)
{
	unsigned int i = 0;
	xport_ioctl_params regs_val;

	CSDEMUX_PIDFT *pidft_ptr = handle;

	/* FIXME@zhongkai's ugly code. */
	struct __desc_key__ {
		unsigned int key_len;
		unsigned int key[6];
	} *const key = (struct __desc_key__ *) even_key;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	if (NULL == even_key){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* no even key. */
	}
	
	regs_val.des_len = key->key_len;
	memcpy(regs_val.des_key, key->key, sizeof(unsigned int) * 6);

	for (i = 0; i < CSDEMUX_DES_NUM; i++) {
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id) {
			/* write a odd key to specified DES slot */
			/* wrtie this value to DESC_EVEN_xx regiters. */
			/* __DESC_EVEN_ADDR__(xport_des_obj[i].des_id, j) (0x41400000 + 0x298 + 4 * 12 * i + 4 * j) */

			regs_val.des_idx = i;	/* des key slot */
			IOCTL(pidft_ptr, XPORT_PIDFT_IOC_DESC_EVENKEY, &regs_val, DEMUX);
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_SUCCEED;
		}
	}

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_PIDFT_EnableDES(CSDEMUX_HANDLE handle)
{
	int i = 0;
	xport_ioctl_params regs_val;

	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	for (i = 0; i < CSDEMUX_DES_NUM; i++)
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id)
			break;	/* find out the DES slot which was associated with this pid filter. */

	if (CSDEMUX_DES_NUM == i){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* this pid filter no DES key which was associated with. */
	}
	/* write bit26~29 of PID_FILTERx. */
	/* bit29: enable / disable DES */
	/* bit26~28: DES id */
	regs_val.des_idx = i;
	regs_val.des_en = 1;
	regs_val.pid_idx = pidft_ptr->pid_filter_id;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_DESC_ENABLE, &regs_val, DEMUX);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_DisableDES(CSDEMUX_HANDLE handle)
{
	int i = 0;
	xport_ioctl_params regs_val;
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	for (i = 0; i < CSDEMUX_DES_NUM; i++)
		if (xport_des_obj[i].pid_filter_id == pidft_ptr->pid_filter_id)
			break;	/* find out the DES slot which was associated with this pid filter. */

	if (CSDEMUX_DES_NUM == i){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* this pid filter no DES key which was associated with. */
	}
	/* pls write 0 to bit26~29 of PID_FILTERx. */
	/* bit29: enable / disable DES */
	/* bit26~28: DES id */
	regs_val.des_idx = i;
	regs_val.des_en = 0;
	regs_val.pid_idx = pidft_ptr->pid_filter_id;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_DESC_ENABLE, &regs_val, DEMUX);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_Enable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	regs_val.pid_idx = pidft_ptr->pid_filter_id;
	regs_val.pid_en = 1;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_ENABLE, &regs_val, DEMUX);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_Disable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;
	CSDEMUX_PIDFT *pidft_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(pidft_ptr, DEMUX_OBJ_PIDFT);

	regs_val.pid_idx = pidft_ptr->pid_filter_id;
	regs_val.pid_en = 0;

	IOCTL(pidft_ptr, XPORT_PIDFT_IOC_ENABLE, &regs_val, DEMUX);
	
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	return CSAPI_SUCCEED;
}

/* 
 * channel interfaces
 */
CSDEMUX_HANDLE CSDEMUX_CHL_Open(CSDEMUX_CHL_ID chl_id)
{
	int dev_fd = -1;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	if (xport_dev_fd < 0){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* must open xport device firstly */
	}
	
	if (chl_id >= CSDEMUX_CHL_NUM){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* incorrect CHL you selected. */
	}
	
	if (xport_chl_obj[chl_id].dev_fd > 0){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* this CHL already opened. */
	}
	if (chl_id == 0)
		dev_fd = open(CSDEMUX_CHL0_FILE, O_RDWR);
	else
		dev_fd = open(CSDEMUX_CHL1_FILE, O_RDWR);

	if (dev_fd <= 0){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* open CHL failed. */
	}
	
	xport_chl_obj[chl_id].obj_type = DEMUX_OBJ_CHL;
	xport_chl_obj[chl_id].dev_fd = dev_fd;
	xport_chl_obj[chl_id].chl_id = chl_id;

	/*  to enable CHLx while open CHLx, write 1 to bit31 of CHLx_CFG_REGS register while you open CHLx */
	//CSDEMUX_CHL_Enable(&xport_chl_obj[chl_id]);

	__chl_dma_mmap__();

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	return &(xport_chl_obj[chl_id]);
}

CSAPI_RESULT CSDEMUX_CHL_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	/*  to disable CHLx while open CHLx, clear bit31 of CHLx_CFG_REGS register while close CHLx */
	CSDEMUX_CHL_Disable(handle);
	__chl_dma_ummap__();
	close(chl_ptr->dev_fd);

	memset(chl_ptr, 0, sizeof(CSDEMUX_CHL));

	chl_ptr->obj_type = -1;
	chl_ptr->chl_id = -1;
	chl_ptr->dev_fd = -1;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_SetDesType(CSDEMUX_HANDLE handle, CSDEMUX_DES_TYPE des_type)
{
	int chl_en;
	int regs_val = des_type;

	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);
	
	chl_en = chl_ptr->chl_en;
	
	if (1 == chl_en) CSDEMUX_CHL_Disable(handle);
	IOCTL(chl_ptr, XPORT_CHL_IOC_DES_MODE, regs_val, DEMUX);
	if (1 == chl_en) CSDEMUX_CHL_Enable(handle);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_SetTunerInputMode(CSDEMUX_HANDLE handle, CSDEMUX_TUNER_MOD input_mod)
{
	int chl_en;
	int regs_val = input_mod;

	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);
	
	chl_en = chl_ptr->chl_en;
	
	if (1 == chl_en) CSDEMUX_CHL_Disable(handle);
	IOCTL(chl_ptr, XPORT_CHL_IOC_TUNER_MODE, regs_val, DEMUX);
	if (1 == chl_en) CSDEMUX_CHL_Enable(handle);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_SetInputMode(CSDEMUX_HANDLE handle, CSDEMUX_INPUT_MOD input_mod)
{
	int regs_val = input_mod;

	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	/* FIXME@zhongkai's code, pls write input mode to XPORT_CFG_ADDR0 */
	/* 0x00000004 is for CHL0_DMA_EN, 0x00000020 is for CHL1_DMA_EN */

	/* FIXME@zhongkai's code, pls write DMA0/1 mode to bit29~30 of CHLx_CFG_ADDR. */
	/* 0x40000000 is for INPUT_MOD_DMA, 0x60000000 is for DEMUX_INPUT_MOD_DIRECT. */

	/* 
	 * enable interrupt, according the following flow:
	 * INT0_ENB_ADDR | 0x41, for DEMUX_INPUT_MOD_TUNER.
	 * INT0_ENB_ADDR | 0x41, for DMA_MOD / CHL0.
	 * INT0_ENB_ADDR | 0x82, for DMA_MOD / CHL1.
	 */

	chl_ptr->input_mode = input_mod;

	CSDEMUX_CHL_Disable(handle);
	IOCTL(chl_ptr, XPORT_CHL_IOC_INPUT_MODE, regs_val, DEMUX);
	CSDEMUX_CHL_Enable(handle);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_Reset(CSDEMUX_HANDLE handle)
{
	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	CSDEMUX_CHL_Disable(handle);
	if (chl_ptr->chl_en)
		CSDEMUX_CHL_Enable(handle);
	CSDEMUX_CHL_SetInputMode(handle, chl_ptr->input_mode);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_Enable(CSDEMUX_HANDLE handle)
{
	int regs_val = 1;
	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	/* write enable-bit to bit31 of CHLx_CFG_ADDR. */
	/* TUNER_EN | 0x00000001 is for CHL0 */
	/* TUNER_EN | 0x00000002 is for CHL1 */

	chl_ptr->chl_en = 1;

	IOCTL(chl_ptr, XPORT_CHL_IOC_ENABLE, regs_val, DEMUX);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_ClearBuffer(CSDEMUX_HANDLE handle)
{
	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	IOCTL(chl_ptr, XPORT_CHL_IOC_CLEAR, 0, DEMUX);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_Disable(CSDEMUX_HANDLE handle)
{
	int regs_val = 0;

	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);

	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	/* FIXME@zhongkai's code, pls write disable-bit to bit31 of CHLx_CFG_ADDR. */
	/* TUNER_EN & ~0x00000001 is for CHL0 */
	/* TUNER_EN & ~0x00000002 is for CHL1 */

	chl_ptr->chl_en = 0;

	IOCTL(chl_ptr, XPORT_CHL_IOC_ENABLE, regs_val, DEMUX);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_DMA_Write(CSDEMUX_HANDLE handle, const unsigned char *const buf, unsigned int len)
{
#define TS_UNIT_SZ	564

	unsigned int len_t = len;
	unsigned char *buf_t = (unsigned char *) buf;

	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);

	if (len_t == 0 || (len_t % 188) != 0){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* incorrect length of data */
	}
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	while (len_t >= TS_UNIT_SZ) {
		if (write(chl_ptr->dev_fd, buf_t, TS_UNIT_SZ) <= 0) {
			DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;
		}

		len_t -= TS_UNIT_SZ;
		buf_t += TS_UNIT_SZ;
	}

	if (len_t > 0) {
		if (write(chl_ptr->dev_fd, buf_t, len_t) <= 0) {
			DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;
		}
	}

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_DMA_Reset(CSDEMUX_HANDLE handle)
{
	CSDEMUX_CHL *chl_ptr = handle;

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, IN\n",getpid(),__FUNCTION__,__LINE__);
	
	CHECK_HANDLE_VALID(chl_ptr, DEMUX_OBJ_CHL);

	IOCTL(chl_ptr, XPORT_CHL_IOC_DMA_RESET, 0, DEMUX);

	DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);

	return CSAPI_SUCCEED;
}

/* 
 * video output interface 
 */
CSDEMUX_HANDLE CSDEMUX_VID_Open(CSDEMUX_VIDOUT_ID vid_id)
{
	if (xport_dev_fd <= 0)
		return NULL;	/* must open xport device firstly. */

	if (vid_id >= CSDEMUX_VIDOUT_NUM)
		return NULL;	/* incorrect VID_ID */

	if (xport_vidout_obj[vid_id].dev_fd > 0)
		return NULL;	/* already opened */

	xport_vidout_obj[vid_id].obj_type = DEMUX_OBJ_VIDOUT;
	xport_vidout_obj[vid_id].vidout_id = vid_id;
	xport_vidout_obj[vid_id].dev_fd = xport_dev_fd;

	return &(xport_vidout_obj[vid_id]);
}

CSAPI_RESULT CSDEMUX_VID_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	CSDEMUX_VID_Disable(handle);

	memset(vidout_ptr, 0, sizeof(CSDEMUX_VIDOUT));

	vidout_ptr->obj_type = -1;
	vidout_ptr->vidout_id = -1;
	vidout_ptr->dev_fd = -1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_SetCPBBuf(CSDEMUX_HANDLE handle, unsigned int cpb_buf_phy_addr, unsigned int cpb_buf_size)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(cpb_buf_phy_addr);
	UNUSED_VARIABLE(cpb_buf_size);

	/* write these values to MIPS registers. */

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_SetDIRBuf(CSDEMUX_HANDLE handle, unsigned int dir_buf_phy_addr, unsigned int dir_buf_size)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(dir_buf_phy_addr);
	UNUSED_VARIABLE(dir_buf_size);

	/* write these values to MIPS registers. */

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_SetOutputMode(CSDEMUX_HANDLE handle, CSDEMUX_OUTPUT_MOD block_mod)
{
	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	vidout_ptr->output_block_flags = ((block_mod == DEMUX_OUTPUT_MOD_BLOCK) ? 1 : 0);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_SetSwitchMode(CSDEMUX_HANDLE handle, CSDEMUX_SWITCH_MOD sw_mod)
{
	xport_ioctl_params regs_val;
	
	CSDEMUX_VIDOUT *vidout_ptr = handle;
	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	regs_val.avout_idx = vidout_ptr->vidout_id;
	regs_val.avout_chl_switch = sw_mod;
	
	IOCTL(vidout_ptr, XPORT_VID_IOC_SWITCH, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_SetPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	xport_ioctl_params regs_val;

	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	/* [MIPS_CHL_PID0 | (vidout_id << 8)] = pid */
	regs_val.avout_idx = vidout_ptr->vidout_id;
	regs_val.avout_pid = pid;

	vidout_ptr->vidout_pid = pid;

	IOCTL(vidout_ptr, XPORT_VID_IOC_PIDVAL, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_Reset(CSDEMUX_HANDLE handle)
{
	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	CSDEMUX_VID_Disable(vidout_ptr);
	if (vidout_ptr->vidout_pid > 0)
		CSDEMUX_VID_SetPID(vidout_ptr, vidout_ptr->vidout_pid);
	if (vidout_ptr->vidout_en)
		CSDEMUX_VID_Enable(vidout_ptr);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_Enable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;
	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	/* [MIPS_CHL_PID0 | (vidout_id << 8)] = xx */
	/* write xxx to MIPS_CHL_PID0_DATA */
	/* enable, block mode, ES output */
	/* 
	 * B31: output enable,1-enable, 0-disable
	 * B7: block mode, 1-block, 0-unblock output
	 * B2~B0: output type
	 * 0 (reserved)
	 * 1 (TS output)
	 * 2 (ES output)
	 * 3 (Section output)
	 * 4 (PES output)
	 * Other bits are reserved.
	 */

	regs_val.avout_idx = vidout_ptr->vidout_id;
	regs_val.avout_en = 1;
	regs_val.avout_mode = vidout_ptr->output_block_flags;

	IOCTL(vidout_ptr, XPORT_VID_IOC_ENABLE, &regs_val, DEMUX);

	vidout_ptr->vidout_en = 1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_VID_Disable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;

	CSDEMUX_VIDOUT *vidout_ptr = handle;

	CHECK_HANDLE_VALID(vidout_ptr, DEMUX_OBJ_VIDOUT);

	/* [MIPS_CHL_PID0 | (vidout_id << 8)] = xx */
	/* FIXME@zhongkai's code, pls write xxx to MIPS_CHL_PID0_DATA */
	/* disable, block mode (ignored), ES output */
	/* 
	 * B31: output enable,1-enable, 0-disable
	 * B7: block mode, 1-block, 0-unblock output
	 * B2~B0: output type
	 * 0 (reserved)
	 * 1 (TS output)
	 * 2 (ES output)
	 * 3 (Section output)
	 * 4 (PES output)
	 * Other bits are reserved.
	 */

	regs_val.avout_idx = vidout_ptr->vidout_id;
	regs_val.avout_en = 0;
	regs_val.avout_mode = vidout_ptr->output_block_flags;

	IOCTL(vidout_ptr, XPORT_VID_IOC_ENABLE, &regs_val, DEMUX);

	vidout_ptr->vidout_en = 0;

	return CSAPI_SUCCEED;
}

/* 
 * audio output interfaces
 */
CSDEMUX_HANDLE CSDEMUX_AUD_Open(CSDEMUX_AUDOUT_ID aud_id)
{
	if (xport_dev_fd <= 0)
		return NULL;	/* must open xport device firstly */

	if (aud_id >= CSDEMUX_AUDOUT_NUM)
		return NULL;	/* incorrect AUD id. */

	if (xport_audout_obj[aud_id].dev_fd > 0)
		return NULL;	/* already opened. */

	xport_audout_obj[aud_id].obj_type = DEMUX_OBJ_AUDOUT;
	xport_audout_obj[aud_id].audout_id = aud_id;
	xport_audout_obj[aud_id].dev_fd = xport_dev_fd;

	return &(xport_audout_obj[aud_id]);
}

CSAPI_RESULT CSDEMUX_AUD_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	CSDEMUX_AUD_Disable(handle);

	memset(audout_ptr, 0, sizeof(CSDEMUX_AUDOUT));

	audout_ptr->obj_type = -1;
	audout_ptr->audout_id = -1;
	audout_ptr->dev_fd = -1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_SetCABBuf(CSDEMUX_HANDLE handle, unsigned int cab_buf_phy_addr, unsigned int cab_buf_size)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(cab_buf_phy_addr);
	UNUSED_VARIABLE(cab_buf_size);

	/* FIXME@zhongkai's code, pls write these values to MIPS registers. */

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_SetPTSBuf(CSDEMUX_HANDLE handle, unsigned int pts_buf_phy_addr, unsigned int pts_buf_size)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(pts_buf_phy_addr);
	UNUSED_VARIABLE(pts_buf_size);

	/* FIXME@zhongkai's code, pls write these values to MIPS registers. */

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_SetOutputMode(CSDEMUX_HANDLE handle, CSDEMUX_OUTPUT_MOD block_mod)
{
	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	audout_ptr->output_block_flags = ((block_mod == DEMUX_OUTPUT_MOD_BLOCK) ? 1 : 0);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_SetSwitchMode(CSDEMUX_HANDLE handle, CSDEMUX_SWITCH_MOD sw_mod)
{
	xport_ioctl_params regs_val;
	
	CSDEMUX_AUDOUT *audout_ptr = handle;
	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	regs_val.avout_idx = audout_ptr->audout_id;
	regs_val.avout_chl_switch = sw_mod;
	
	IOCTL(audout_ptr, XPORT_AUD_IOC_SWITCH, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_SetPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	xport_ioctl_params regs_val;

	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	/*  write pid to ((2 << 8) | MIPS_CHL_PID0) */
	regs_val.avout_pid = pid;
	regs_val.avout_idx = 2;	/* 0 - vidout0, 1 - vidout1, 2 - audout0 */

	IOCTL(audout_ptr, XPORT_AUD_IOC_PIDVAL, &regs_val, DEMUX);

	audout_ptr->audout_pid = pid;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_Enable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;
	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	/* FIXME@zhongkai's code, pls write xxx to MIPS_CHL_PID0 | (2<<8) */
	/* enable, block mode, ES output */
	/* 
	 * B31: output enable,1-enable, 0-disable
	 * B7: block mode, 1-block, 0-unblock output
	 * B2~B0: output type
	 * 0 (reserved)
	 * 1 (TS output)
	 * 2 (ES output)
	 * 3 (Section output)
	 * 4 (PES output)
	 * Other bits are reserved.
	 */

	regs_val.avout_idx = 2;
	regs_val.avout_en = 1;
	regs_val.avout_mode = audout_ptr->output_block_flags;

	IOCTL(audout_ptr, XPORT_AUD_IOC_ENABLE, &regs_val, DEMUX);

	audout_ptr->audout_en = 1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_Disable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;

	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	/* write xxx to MIPS_CHL_PID0 | (2<<8)  */
	/* disable, block mode (ignored), ES output */
	/* 
	 * B31: output enable,1-enable, 0-disable
	 * B7: block mode, 1-block, 0-unblock output
	 * B2~B0: output type
	 * 0 (reserved)
	 * 1 (TS output)
	 * 2 (ES output)
	 * 3 (Section output)
	 * 4 (PES output)
	 * Other bits are reserved.
	 */

	regs_val.avout_idx = 2;
	regs_val.avout_en = 0;
	regs_val.avout_mode = audout_ptr->output_block_flags;

	IOCTL(audout_ptr, XPORT_AUD_IOC_ENABLE, &regs_val, DEMUX);

	audout_ptr->audout_en = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_AUD_Reset(CSDEMUX_HANDLE handle)
{
	CSDEMUX_AUDOUT *audout_ptr = handle;

	CHECK_HANDLE_VALID(audout_ptr, DEMUX_OBJ_AUDOUT);

	CSDEMUX_AUD_Disable(audout_ptr);
	if (audout_ptr->audout_pid > 0)
		CSDEMUX_AUD_SetPID(audout_ptr, audout_ptr->audout_pid);
	if (audout_ptr->audout_en)
		CSDEMUX_AUD_Enable(audout_ptr);

	return CSAPI_SUCCEED;
}

/* 
 * PCR interfaces
 */
CSDEMUX_HANDLE CSDEMUX_PCR_Open(CSDEMUX_PCRDEV_ID pcr_id)
{
	if (xport_dev_fd < 0)
		return NULL;	/* must open xport device firstly */

	if (pcr_id >= CSDEMUX_PCR_NUM)
		return NULL;	/* incorrect PCR id. */

	if (xport_pcr_obj[pcr_id].dev_fd > 0)
		return NULL;	/* already opened. */

	xport_pcr_obj[pcr_id].obj_type = DEMUX_OBJ_PCR;
	xport_pcr_obj[pcr_id].pcr_id = pcr_id;
	xport_pcr_obj[pcr_id].dev_fd = xport_dev_fd;

	return ((CSDEMUX_HANDLE) & (xport_pcr_obj[pcr_id]));
}

CSAPI_RESULT CSDEMUX_PCR_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_PCR *pcr_ptr = handle;

	CHECK_HANDLE_VALID(pcr_ptr, DEMUX_OBJ_PCR);

	memset(pcr_ptr, 0, sizeof(CSDEMUX_PCR));

	pcr_ptr->obj_type = -1;
	pcr_ptr->pcr_id = -1;
	pcr_ptr->dev_fd = -1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PCR_SetSwitchMode(CSDEMUX_HANDLE handle, CSDEMUX_SWITCH_MOD sw_mod)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(sw_mod);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PCR_SetPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PCR *pcr_ptr = handle;

	CHECK_HANDLE_VALID(pcr_ptr, DEMUX_OBJ_PCR);

	/* 
	 * pls do the following action:
	 * MIPS_PCR_PID = pid | 0x80000000;
	 */

	regs_val.pcr_pid = pid;
	regs_val.pcr_idx = pcr_ptr->pcr_id;

	IOCTL(pcr_ptr, XPORT_PCR_IOC_PIDVAL, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PCR_Enable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PCR *pcr_ptr = (CSDEMUX_PCR *) handle;

	CHECK_HANDLE_VALID(pcr_ptr, DEMUX_OBJ_PCR);

	/* 
	 * FIXME@zhongkai's code, pls do the following action:
	 * MIPS_PCR_PID = [MIPS_PCR_PID] | 0x80000000;
	 */

	regs_val.pcr_idx = pcr_ptr->pcr_id;
	regs_val.pcr_en = 1;

	IOCTL(pcr_ptr, XPORT_PCR_IOC_ENABLE, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PCR_Disable(CSDEMUX_HANDLE handle)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PCR *pcr_ptr = handle;

	CHECK_HANDLE_VALID(pcr_ptr, DEMUX_OBJ_PCR);

	/* 
	 * FIXME@zhongkai's code, pls do the following action:
	 * MIPS_PCR_PID = [MIPS_PCR_PID] & ~0x80000000;
	 */

	regs_val.pcr_idx = pcr_ptr->pcr_id;
	regs_val.pcr_en = 0;

	IOCTL(pcr_ptr, XPORT_PCR_IOC_ENABLE, &regs_val, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PCR_GetPCR(CSDEMUX_HANDLE handle, CSDEMUX_PCRVALUE * value)
{
	xport_ioctl_params regs_val;

	CSDEMUX_PCR *pcr_ptr = handle;

	CHECK_HANDLE_VALID(pcr_ptr, DEMUX_OBJ_PCR);

	regs_val.pcr_idx = pcr_ptr->pcr_id;
	regs_val.pcr_en = 0;

	IOCTL(pcr_ptr, XPORT_PCR_IOC_GETVAL, &regs_val, DEMUX);

	*value = 0;
	*value = regs_val.pcr_hi_val << 31;
	*value |= regs_val.pcr_lo_val;

	return CSAPI_SUCCEED;
}

/* 
 * Filter interfaces 
 */
CSDEMUX_HANDLE CSDEMUX_Filter_Open(CSDEMUX_FILTER_ID filter_id)
{
	int dev_fd = 0;
	char filter_dev_file[32];

	DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, IN\n",filter_id,getpid(),__FUNCTION__,__LINE__);
	
	if (xport_dev_fd < 0){
		DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, OUT1\n",filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* must open xport device firstly. */
	}
	
	if (filter_id >= CSDEMUX_FILTER_NUM){
		DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, OUT2\n",filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* incorrect FILTER id. */
	}
	
	if (xport_filter_obj[filter_id].dev_fd >= 0){
		DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, OUT3\n",filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* already opened. */
	}
	sprintf(filter_dev_file, "/dev/orion_xport/filter%d", filter_id);
	dev_fd = open(filter_dev_file, O_RDWR);

	if (dev_fd < 0){
		DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, OUT4\n",filter_id,getpid(),__FUNCTION__,__LINE__);
		return NULL;	/* open filter device failed. */
	}
	xport_filter_obj[filter_id].obj_type = DEMUX_OBJ_FILTER;
	xport_filter_obj[filter_id].crc_id = 0;
	xport_filter_obj[filter_id].filter_type = 0;
	xport_filter_obj[filter_id].dev_fd = dev_fd;
	xport_filter_obj[filter_id].filter_id = filter_id;
	xport_filter_obj[filter_id].filter_timeout_ms = DEFAULT_FILTER_TIMEOUT;
	
	FILTER_UNLOCK(xport_filter_obj[filter_id].filter_mutex);

	DEBUG_PRINTF("filterid %d, thread id : %d,function: %s,line: %d, OUT5\n",filter_id,getpid(),__FUNCTION__,__LINE__);
	DEBUG_PRINTF("handle = 0x%x\n",(unsigned int)&(xport_filter_obj[filter_id]));
	return &(xport_filter_obj[filter_id]);
}

CSAPI_RESULT CSDEMUX_Filter_Close(CSDEMUX_HANDLE handle)
{
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);
	
	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
       CSEVT_UnRegister(xport_evt, filter_ptr->dev_fd);
	__filter_disable_crc(filter_ptr->filter_id, filter_ptr->dev_fd);
	__filter_free_crc(filter_ptr->filter_id);
	close(filter_ptr->dev_fd);

	filter_ptr->obj_type = -1;
	filter_ptr->filter_type = -1;
	filter_ptr->filter_id = -1;
	filter_ptr->dev_fd = -1;
	filter_ptr->filter_timeout_ms = DEFAULT_FILTER_TIMEOUT;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_SetFilterType(CSDEMUX_HANDLE handle, CSDEMUX_FILTER_TYPE type)
{
	CSDEMUX_FILTER *filter_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	if (type != DEMUX_FILTER_TYPE_SEC &&
	    type != DEMUX_FILTER_TYPE_TS && type != DEMUX_FILTER_TYPE_PES && type != DEMUX_FILTER_TYPE_ES) {
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* invalid Filter type. */
	}

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	//IOCTL(filter_ptr, XPORT_FILTER_IOC_TYPE, &type, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_TYPE, &type);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_SetSwitchMode(CSDEMUX_HANDLE handle, CSDEMUX_SWITCH_MOD sw_mod)
{
	CSDEMUX_FILTER *filter_ptr = handle;
	
	IOCTL(filter_ptr, XPORT_FILTER_IOC_SWITCH, sw_mod, DEMUX);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_SetFilter(CSDEMUX_HANDLE handle, const unsigned char *const filter_data_ptr,
				      const unsigned char *const mask_data_ptr)
{
	unsigned char filter_mask[24];
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	memcpy(filter_mask, filter_data_ptr, 12);
	memcpy(filter_mask + 12, mask_data_ptr, 12);

	//IOCTL(filter_ptr, XPORT_FILTER_IOC_FILTER, filter_mask, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_FILTER, filter_mask);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_GetFreeCRCNumber(CSDEMUX_HANDLE handle, unsigned int *crc_number)
{
	int i = 0, crc_nums = 0;

	UNUSED_VARIABLE(handle);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	if (NULL == crc_number){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	for (i = 0; i < CSDEMUX_CRC_NUM; i++) {
		if (xport_crc_obj[i].dev_fd <= 0)
			crc_nums++;
	}

	*crc_number = crc_nums;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_MallocCRC(CSDEMUX_HANDLE handle)
{
	int i;
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("thread id : %d,function: %s,line: %d, OUT\n",getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	for (i = 0; i < CSDEMUX_CRC_NUM; i++)
		if (xport_crc_obj[i].filter_id == filter_ptr->filter_id){
			FILTER_UNLOCK(filter_ptr->filter_mutex);
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;	/* already allocated a CRC slot for this filter. */
		}
	for (i = 0; i < CSDEMUX_CRC_NUM; i++) {
		if (xport_crc_obj[i].dev_fd <= 0) {
			break;	/* find out a empty CRC slot for this filter. */
		}
	}

	if (i == CSDEMUX_CRC_NUM){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* no empty CRC slot. */
	}

	xport_crc_obj[i].obj_type = DEMUX_OBJ_CRC;
	xport_crc_obj[i].filter_id = filter_ptr->filter_id;
	xport_crc_obj[i].dev_fd = filter_ptr->dev_fd;
	xport_crc_obj[i].crc_id = i;

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_SUCCEED;
}

static void __filter_free_crc(CSDEMUX_FILTER_ID filter_id)
{
	int i = 0;
	
	for (i = 0; i < CSDEMUX_CRC_NUM; i++) {
		/* 
		 * find out a CRC slot which was associated with a section filter 
		 */
		if (xport_crc_obj[i].filter_id == filter_id) {
			xport_crc_obj[i].crc_id = -1;
			xport_crc_obj[i].filter_id = -1;
			xport_crc_obj[i].dev_fd = -1;
			return;
		}
	}
}

CSAPI_RESULT CSDEMUX_Filter_FreeCRC(CSDEMUX_HANDLE handle)
{
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	__filter_free_crc(filter_ptr->filter_id);

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	return CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_Filter_EnableCRC(CSDEMUX_HANDLE handle)
{
	int i;
	xport_ioctl_params regs_val;

	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	for (i = 0; i < CSDEMUX_CRC_NUM; i++)
		if (xport_crc_obj[i].filter_id == filter_ptr->filter_id)
			break;	/* find out the CRC slot which was associated with this section filter. */

	if (CSDEMUX_CRC_NUM == i){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;	/* this section filter no CRC slot which was associated with. */
	}
	regs_val.filter_crc_idx = i;
	regs_val.filter_crc_en = 1;
	regs_val.filter_crc_save = 1;
	regs_val.filter_idx = filter_ptr->filter_id;

	//IOCTL(filter_ptr, XPORT_FILTER_IOC_CRC_ENABLE, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_CRC_ENABLE, &regs_val);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

void SectionNotifyProc(void *puser, int fd, int events)
{
	CSDEMUX_SECEVENT i;
	CSDEMUX_FILTER *filter_ptr = puser;

	UNUSED_VARIABLE(fd);
	UNUSED_VARIABLE(events);

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)puser,getpid(),__FUNCTION__,__LINE__);
	if (NULL == filter_ptr){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)puser,getpid(),__FUNCTION__,__LINE__);
		return;		/* no context parameters. */
	}
	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);

	//FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		//FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)puser,getpid(),__FUNCTION__,__LINE__);
		return;
	}

	switch(events) {
		case EVT_READ:
			if (NULL != filter_ptr->call_lst[DEMUX_SECTION_AVAIL].notify_func){
				i = DEMUX_SECTION_AVAIL;
				filter_ptr->call_lst[DEMUX_SECTION_AVAIL].notify_func((CSDEMUX_HANDLE) filter_ptr, &i);
			}
			break;
		case EVT_EXCEPT:
			if (NULL != filter_ptr->call_lst[DEMUX_CRC_ERROR].notify_func){
				i = DEMUX_CRC_ERROR;
				filter_ptr->call_lst[DEMUX_CRC_ERROR].notify_func((CSDEMUX_HANDLE) filter_ptr, &i);
			}
			break;
	}

	//FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)puser,getpid(),__FUNCTION__,__LINE__);
	return;
}

CSAPI_RESULT CSDEMUX_FILTER_SetSectionNotify(CSDEMUX_HANDLE handle,
					     void (*call_back_function) (CSDEMUX_HANDLE, CSDEMUX_SECEVENT *),
					     CSDEMUX_SECEVENT demux_event, int event_enable)
{
	xport_ioctl_params regs_val;

	CSDEMUX_FILTER *filter_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if (demux_event >= CSDEMUX_FILTER_EVENT_NUM){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	filter_ptr->call_lst[demux_event].notify_func = call_back_function;

	if (demux_event == DEMUX_CRC_ERROR) {
		if (CSAPI_FAILED == CSEVT_Register(xport_evt,
						   filter_ptr->dev_fd,
						   SectionNotifyProc,
						   filter_ptr, event_enable ? EVT_EXCEPT : EVT_INVALID)){
			FILTER_UNLOCK(filter_ptr->filter_mutex);
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;
		}
		regs_val.filter_crc_notify_en = event_enable;
		regs_val.filter_idx = filter_ptr->filter_id;

		//IOCTL(filter_ptr, XPORT_FILTER_IOC_CRC_NOTIFY_ENABLE, &regs_val, DEMUX);
		ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_CRC_NOTIFY_ENABLE, &regs_val);
	}
	else {
		if (CSAPI_FAILED == CSEVT_Register(xport_evt,
						   filter_ptr->dev_fd,
						   SectionNotifyProc,
						   filter_ptr, event_enable ? EVT_READ : EVT_INVALID)){
			FILTER_UNLOCK(filter_ptr->filter_mutex);
			DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT5\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
			return CSAPI_FAILED;
		}
	}

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT6\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_FILTER_SetSaveErrSectionDataFlag(CSDEMUX_HANDLE handle, int enable)
{
	xport_ioctl_params regs_val;

	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	filter_ptr->filter_crc_save = enable;

	regs_val.filter_crc_save = enable;
	regs_val.filter_idx = filter_ptr->filter_id;

	//IOCTL(filter_ptr, XPORT_FILTER_IOC_CRC_ENABLE, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_CRC_ENABLE, &regs_val);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

static CSAPI_RESULT __filter_disable_crc(CSDEMUX_FILTER_ID filter_id, int fd)
{
	int i;
	xport_ioctl_params regs_val;

	for (i = 0; i < CSDEMUX_CRC_NUM; i++)
		if (xport_crc_obj[i].filter_id == filter_id)
			break;	/* find out the CRC slot which was associated with this section filter. */

	if (CSDEMUX_CRC_NUM == i){
		return CSAPI_FAILED;	/* this section filter no CRC slot which was associated with. */
	}
	
	regs_val.filter_crc_idx = i;
	regs_val.filter_crc_en = 0;
	regs_val.filter_idx = filter_id;

	ioctl(fd, XPORT_FILTER_IOC_CRC_ENABLE, &regs_val);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_DisableCRC(CSDEMUX_HANDLE handle)
{
	CSAPI_RESULT ret_val = CSAPI_FAILED;
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return ret_val;
	}

	ret_val = __filter_disable_crc(filter_ptr->filter_id, filter_ptr->dev_fd);

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return ret_val;
}

CSAPI_RESULT CSDEMUX_Filter_AddPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	unsigned int regs_val = pid;
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//IOCTL(filter_ptr, XPORT_FILTER_IOC_PID0, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_PID0, &regs_val);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_AddPID2(CSDEMUX_HANDLE handle, unsigned short pid, unsigned short slot)
{
	static int cmd_lst[] = {
		XPORT_FILTER_IOC_PID0,
		XPORT_FILTER_IOC_PID1,
		XPORT_FILTER_IOC_PID2,
		XPORT_FILTER_IOC_PID3,
		XPORT_FILTER_IOC_PID4,
		XPORT_FILTER_IOC_PID5,
		XPORT_FILTER_IOC_PID6,
		XPORT_FILTER_IOC_PID7,
		XPORT_FILTER_IOC_PID8,
		XPORT_FILTER_IOC_PID9,
		XPORT_FILTER_IOC_PID10,
		XPORT_FILTER_IOC_PID11,
	};

	unsigned int regs_val = pid;
	CSDEMUX_FILTER *filter_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	if (((signed short)slot < 0) || (slot > 11)){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//IOCTL(filter_ptr, cmd_lst[slot], &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, cmd_lst[slot], &regs_val);

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_ResetPID(CSDEMUX_HANDLE handle)
{
	UNUSED_VARIABLE(handle);
	return CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_Filter_ReadWait(CSDEMUX_HANDLE handle, unsigned int ms_timeout)
{
	int rc = -1;;
	int fd = -1;;

	fd_set fds;
	struct timeval ft_timeout;

	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	fd = filter_ptr->dev_fd;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	ft_timeout.tv_sec = ms_timeout / 1000;
	ft_timeout.tv_usec = ms_timeout * 1000 - ft_timeout.tv_sec * 1000000;

	rc = select(fd + 1, &fds, NULL, NULL, &ft_timeout);

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	
	if (rc < 0){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return FD_ISSET(fd, &fds) ? CSAPI_SUCCEED : CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_Filter_CheckDataSize(CSDEMUX_HANDLE handle, unsigned int *size)
{
	int regs_val = 0;
	CSDEMUX_FILTER *filter_ptr = handle;

	*size = 0;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//IOCTL(filter_ptr, XPORT_FILTER_IOC_QUERY_NUM, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_QUERY_NUM, &regs_val);
	
	*size = regs_val;

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_CheckSectionNum(CSDEMUX_HANDLE handle, unsigned int *num)
{
	return CSDEMUX_Filter_CheckDataSize(handle, num);
}

CSAPI_RESULT CSDEMUX_Filter_ReadData(CSDEMUX_HANDLE handle, unsigned char *buf, unsigned int *size)
{
	int rt_val = 0;
	unsigned int len = 0;

	int rc = -1;
	int fd = -1;
	fd_set fds;
	struct timeval ft_timeout;
	
	CSDEMUX_FILTER *filter_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	
	if (buf == NULL || size == NULL || (*size) == 0){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	len = (*size);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	fd = filter_ptr->dev_fd;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	ft_timeout.tv_sec = filter_ptr->filter_timeout_ms / 1000;
	ft_timeout.tv_usec = filter_ptr->filter_timeout_ms * 1000 - ft_timeout.tv_sec * 1000000;

	rc = select(fd + 1, &fds, NULL, NULL, &ft_timeout);

	if (rc < 0){
		rt_val = 0;
	}
	if(FD_ISSET(fd, &fds) ? CSAPI_SUCCEED : CSAPI_FAILED){
		rt_val = 0;
	}
	else{
		rt_val = read(filter_ptr->dev_fd, buf, len);
	}

	FILTER_UNLOCK(filter_ptr->filter_mutex);
	
	if (rt_val <= 0){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	*size = rt_val;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT5\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_ReadSectionData(CSDEMUX_HANDLE handle, unsigned char *buf, unsigned int *size)
{
	int rt_val = 0;
	unsigned int len = 0;

	int rc = -1;
	int fd = -1;
	fd_set fds;
	struct timeval ft_timeout;

	CSDEMUX_FILTER *filter_ptr = handle;

	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if (buf == NULL || size == NULL || (*size) == 0 || (*size) > 4096){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
       }
	
	len = (*size);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	fd = filter_ptr->dev_fd;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	ft_timeout.tv_sec = filter_ptr->filter_timeout_ms / 1000;
	ft_timeout.tv_usec = filter_ptr->filter_timeout_ms * 1000 - ft_timeout.tv_sec * 1000000;

	rc = select(fd + 1, &fds, NULL, NULL, &ft_timeout);

	if (rc < 0){
		rt_val = 0;
	}
	if(FD_ISSET(fd, &fds) ? CSAPI_SUCCEED : CSAPI_FAILED){
		rt_val = 0;
	}
	else{
		rt_val = read(filter_ptr->dev_fd, buf, len);
	}

	FILTER_UNLOCK(filter_ptr->filter_mutex);

	if (rt_val <= 0){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT4\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	*size = rt_val;
	
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT5\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_Enable(CSDEMUX_HANDLE handle)
{
	unsigned int regs_val = 1;
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}

	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//IOCTL(filter_ptr, XPORT_FILTER_IOC_ENABLE, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_ENABLE, &regs_val);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_Disable(CSDEMUX_HANDLE handle)
{
	unsigned int regs_val = 0;
	CSDEMUX_FILTER *filter_ptr = handle;

	//CHECK_HANDLE_VALID(filter_ptr, DEMUX_OBJ_FILTER);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, IN\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);

	if(filter_ptr == NULL){
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT1\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	FILTER_LOCK(filter_ptr->filter_mutex);

	if((filter_ptr == NULL)||(filter_ptr->dev_fd < 0)||(filter_ptr->obj_type !=DEMUX_OBJ_FILTER)){
		FILTER_UNLOCK(filter_ptr->filter_mutex);
		DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT2\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
		return CSAPI_FAILED;
	}
	
	//IOCTL(filter_ptr, XPORT_FILTER_IOC_ENABLE, &regs_val, DEMUX);
	ioctl(filter_ptr->dev_fd, XPORT_FILTER_IOC_ENABLE, &regs_val);
	
	FILTER_UNLOCK(filter_ptr->filter_mutex);
	DEBUG_PRINTF("handle 0x%x, thread id : %d,function: %s,line: %d, OUT3\n",(unsigned int)handle,getpid(),__FUNCTION__,__LINE__);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_EXT_GetSTC(int *hi_addr, int *lo_addr)
{
	if (xport_dev_fd <= 0)
		return CSAPI_FAILED;	/* must open xport device firstly. */

	if ((NULL == hi_addr) || (NULL == lo_addr))
		return CSAPI_FAILED;	/* NULL pointer */

	if (-1 == ioctl(xport_dev_fd, 0x41400080, hi_addr))
		return CSAPI_FAILED;
	if (-1 == ioctl(xport_dev_fd, 0x41400084, lo_addr))
		return CSAPI_FAILED;

	return CSAPI_SUCCEED;
}

/* 
 * the following implementation is for optimizing CHL DMA transfer.
 */

#include <asm/page.h>
#include <sys/mman.h>

#define CHL0_BASE_ADDR			0x07100000	// FIXME@zhongkai's code
#define XPORT_MEM_BASE			CHL0_BASE_ADDR
#define CHL0_BUFF_SZ			256000

#define XPORT_CHL0_BASE_ADDR    	(0x41400000+(0x0040*4))
#define XPORT_CHL0_CFG_ADDR     	(0x41400000+(0x0041*4))
#define XPORT_CHL0_RP_ADDR      	(0x41400000+(0x0042*4))
#define XPORT_CHL0_WP_ADDR      	(0x41400000+(0x0043*4))

#define MAIL_BOX2_ADDR              	(0x41400000+(0x0002*4))
#define MAIL_BOX3_ADDR              	(0x41400000+(0x0003*4))

#define XPORT_CHL_DMA0_WP_ADDR          (MAIL_BOX2_ADDR)
#define XPORT_CHL_DMA1_WP_ADDR          (MAIL_BOX3_ADDR)

#define XPORT_CHL0_BASE_ADDR_DEF        XPORT_MEM_BASE
#define XPORT_CHL0_UNIT_SIZE_DEF        2000
#define XPORT_CHL0_UNIT_NUM_DEF         128
#define XPORT_CHL0_CFG_DEF              ((0x60000000) | (XPORT_CHL0_UNIT_NUM_DEF<<8) | (XPORT_CHL0_UNIT_SIZE_DEF>>3))
#define XPORT_CHL0_RP_DEF               0
#define XPORT_CHL0_WP_DEF               0

#define XPORT_CHL0_MIN_SPACES           (XPORT_CHL0_UNIT_NUM_DEF >> 1)
#define  TS_SIZE        		188
#define  ML_ALIGN(x)    		((x)<<24 | (x)>>24 | ((x)>>8 & 0xff00) | ((x)<<8 & 0xff0000))

typedef struct {
	void *pVMem;
	void *pPMem;
	unsigned int MemSize;
	void *MappedAddr;
	unsigned int MappedSize;

	unsigned int chl_buf_wp;

} CSDEMUX_MMAP_t;

static CSDEMUX_MMAP_t g_chl_mem;

static int __chl_dma_mmap__(void)
{
	unsigned char *buf = NULL;
	unsigned int addr = 0, phyaddr = 0, addroffset = 0, memsize = 0;

	addr = CHL0_BASE_ADDR;
	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + CHL0_BUFF_SZ + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;

	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, xport_dev_fd, phyaddr);
	if (buf == MAP_FAILED)
		return CSAPI_FAILED;

	g_chl_mem.MappedSize = memsize;
	g_chl_mem.MappedAddr = buf;
	g_chl_mem.pPMem = (void *) addr;
	g_chl_mem.pVMem = (void *) (buf + addroffset);
	g_chl_mem.MemSize = CHL0_BUFF_SZ;

	g_chl_mem.chl_buf_wp = 0;

	return CSAPI_SUCCEED;
}

static int __chl_dma_ummap__(void)
{
	munmap(g_chl_mem.MappedAddr, g_chl_mem.MappedSize);
}

CSAPI_RESULT CSDEMUX_CHL_DMA_GenericWrite(CSDEMUX_HANDLE handle, int (*func) (void *buff, int sz, void *context),
					  unsigned int len, void *context)
{
	char *write_addr;
	char *base_addr;

	unsigned int tmp_addr = 0;
	unsigned int cfg_addr, wp_addr, rd_addr, baseaddr_addr;

	unsigned int chl_buf_unit_num, chl_buf_unit_size, chl_buf_type;
	unsigned int free_block_num, chl_buf_wp, chl_buf_rp;
	unsigned int regs_val = 0, rt_val;
	unsigned int write_len;

	UNUSED_VARIABLE(handle);

	if (NULL == func)
		return CSAPI_FAILED;

	if (0 != __chl_dma_half_empty_check__())
		return CSAPI_FAILED;

	cfg_addr = XPORT_CHL0_CFG_ADDR;
	wp_addr = XPORT_CHL_DMA0_WP_ADDR;
	rd_addr = XPORT_CHL0_RP_ADDR;
	baseaddr_addr = XPORT_CHL0_BASE_ADDR;

	ioctl(xport_dev_fd, cfg_addr, &regs_val);
	chl_buf_type = ((regs_val >> 29) & 0x3);
	chl_buf_unit_num = (regs_val >> 8) & 0xfff;
	chl_buf_unit_size = (regs_val & 0xff) << 3;

	ioctl(xport_dev_fd, baseaddr_addr, &tmp_addr);
	tmp_addr <<= 3;
	if ((chl_buf_type != 3) || (tmp_addr != XPORT_CHL0_BASE_ADDR_DEF)) {
		return CSAPI_FAILED;
	}

	tmp_addr -= XPORT_MEM_BASE;
	base_addr = (char*)((unsigned int) g_chl_mem.pVMem + tmp_addr);

	ioctl(xport_dev_fd, wp_addr, &chl_buf_wp);
	ioctl(xport_dev_fd, rd_addr, &chl_buf_rp);

	if ((chl_buf_wp ^ chl_buf_rp) >> 31)
		free_block_num = (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);
	else
		free_block_num = chl_buf_unit_num + (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);

	rt_val = 0;

	while (((int) len >= TS_SIZE) && ((int) free_block_num > 0) && (free_block_num <= chl_buf_unit_num)) {

		write_addr = base_addr + ((chl_buf_wp & 0xfff) * chl_buf_unit_size);
		write_len = 0;

		while ((len >= TS_SIZE) && (write_len + TS_SIZE + 8 <= chl_buf_unit_size)) {
			write_len += TS_SIZE;
			len -= TS_SIZE;
		}

		if (write_len == 0) {
			return CSAPI_FAILED;
		}

		write_len = func((write_addr + 8), write_len, context);

		write_addr[0] = 0;
		write_addr[1] = 0;
		write_addr[2] = 0;
		write_addr[3] = 0;
		write_addr[4] = 0;
		write_addr[5] = 0;
		write_addr[6] = (write_len >> 8) & 0xff;
		write_addr[7] = write_len & 0xff;

		write_addr += 8;
		write_addr += write_len;
		rt_val += write_len;

		/* update wp pointer */
		chl_buf_wp++;
		if ((chl_buf_wp & 0xfff) >= chl_buf_unit_num)
			chl_buf_wp = (~chl_buf_wp) & 0x80000000;

		free_block_num--;
	}

	ioctl(xport_dev_fd, wp_addr | 0x80000000, chl_buf_wp);

	return CSAPI_SUCCEED;
}

static int __chl_dma_half_empty_check__(void)
{
	int rt_val = -1;

	unsigned int chl_wp, chl_rp, chl_wp_addr, chl_rp_addr;
	unsigned int space_cnt = 0;
	unsigned int tmp;

	chl_wp_addr = XPORT_CHL0_WP_ADDR;
	chl_rp_addr = XPORT_CHL0_RP_ADDR;
	ioctl(xport_dev_fd, XPORT_CHL0_CFG_ADDR, &tmp);
	tmp = (tmp >> 29) & 0x3;

	if (tmp == 3)
		chl_wp_addr = XPORT_CHL_DMA0_WP_ADDR;

	ioctl(xport_dev_fd, chl_wp_addr, &chl_wp);
	ioctl(xport_dev_fd, chl_rp_addr, &chl_rp);

	if ((chl_wp ^ chl_rp) >> 31)
		space_cnt = (chl_rp & 0xfff) - (chl_wp & 0xfff);
	else
		space_cnt = XPORT_CHL0_UNIT_NUM_DEF + (chl_rp & 0xfff) - (chl_wp & 0xfff);

	if (space_cnt > XPORT_CHL0_MIN_SPACES)
		rt_val = 0;

	return rt_val;
}

CSAPI_RESULT CSDEMUX_CHL_DMAext_IsHalfEmpty(CSDEMUX_HANDLE handle, int *is_empty)
{
	UNUSED_VARIABLE(handle);

	if (NULL == is_empty)
		return CSAPI_FAILED;

	*is_empty = __chl_dma_half_empty_check__();

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_DMAext_Exec(CSDEMUX_HANDLE handle, void *buff_addr, unsigned int sz)
{
	char *write_addr = (char *) buff_addr;

	UNUSED_VARIABLE(handle);

	write_addr[-8] = 0;
	write_addr[-7] = 0;
	write_addr[-6] = 0;
	write_addr[-5] = 0;
	write_addr[-4] = 0;
	write_addr[-3] = 0;
	write_addr[-2] = (sz >> 8) & 0xff;
	write_addr[-1] = sz & 0xff;

	ioctl(xport_dev_fd, XPORT_CHL_DMA0_WP_ADDR | 0x80000000, g_chl_mem.chl_buf_wp);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_CHL_DMAext_GetWPtr(CSDEMUX_HANDLE handle, void **buff_addr, unsigned int *sz)
{
	char *write_addr = NULL;
	char *base_addr = NULL;

	unsigned int tmp_addr = 0;
	unsigned int cfg_addr, wp_addr, rd_addr, baseaddr_addr;

	unsigned int chl_buf_unit_num, chl_buf_unit_size, chl_buf_type;
	unsigned int free_block_num, chl_buf_wp, chl_buf_rp;
	unsigned int regs_val = 0, rt_val;
	unsigned int write_len = 0;

	UNUSED_VARIABLE(handle);

	cfg_addr = XPORT_CHL0_CFG_ADDR;
	wp_addr = XPORT_CHL_DMA0_WP_ADDR;
	rd_addr = XPORT_CHL0_RP_ADDR;
	baseaddr_addr = XPORT_CHL0_BASE_ADDR;

	ioctl(xport_dev_fd, cfg_addr, &regs_val);
	chl_buf_type = ((regs_val >> 29) & 0x3);
	chl_buf_unit_num = (regs_val >> 8) & 0xfff;
	chl_buf_unit_size = (regs_val & 0xff) << 3;

	ioctl(xport_dev_fd, baseaddr_addr, &tmp_addr);
	tmp_addr <<= 3;
	if ((chl_buf_type != 3) || (tmp_addr != XPORT_CHL0_BASE_ADDR_DEF)) {
		return CSAPI_FAILED;
	}

	tmp_addr -= XPORT_MEM_BASE;
	base_addr = (char*)((unsigned int) g_chl_mem.pVMem + tmp_addr);

	ioctl(xport_dev_fd, wp_addr, &chl_buf_wp);
	ioctl(xport_dev_fd, rd_addr, &chl_buf_rp);

	if ((chl_buf_wp ^ chl_buf_rp) >> 31)
		free_block_num = (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);
	else
		free_block_num = chl_buf_unit_num + (chl_buf_rp & 0xfff) - (chl_buf_wp & 0xfff);

	rt_val = 0;

	if (((int) free_block_num > 0) && (free_block_num <= chl_buf_unit_num)) {

		write_addr = base_addr + ((chl_buf_wp & 0xfff) * chl_buf_unit_size);
		write_len = 0;

		while (write_len + TS_SIZE + 8 <= chl_buf_unit_size) {
			write_len += TS_SIZE;
		}

		if (write_len == 0) {
			*sz = 0;
			return CSAPI_FAILED;
		}

		/* update wp pointer */
		chl_buf_wp++;
		if ((chl_buf_wp & 0xfff) >= chl_buf_unit_num)
			chl_buf_wp = (~chl_buf_wp) & 0x80000000;
	}

	*buff_addr = write_addr + 8;
	*sz = write_len;

	g_chl_mem.chl_buf_wp = chl_buf_wp;

	return CSAPI_SUCCEED;
}
