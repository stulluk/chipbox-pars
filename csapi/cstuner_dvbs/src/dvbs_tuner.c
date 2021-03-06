#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "global.h"
#include "stv0288_regs.h"
#include "i2c_if.h"

#include "dvbs_tuner.h"

#define WARNING_PRINT	printf

#define  CSTUNER_OBJ_TYPE  	'T'

typedef struct {
	int freq_from;
	int freq_to;
	int freq_step;
	int time_out;
	int freq_cur;
	int time_used;
} Scan_Rec;

typedef struct tagTUNER_OBJ {
	unsigned char obj_type;
	int err;
	int dev_fd;

	CSTUNER_Info TunerInfo;
	CSTUNER_ThresholdList *ThresholdList;
	CSTUNER_ScanList *Scanlist;
	CSTUNER_NotifyFunc tuner_notify;
	Scan_Rec scan_rec;

	sem_t sem_tuner;
	pthread_t thread_tuner;

	int inst_cnt;
	int check_times;

	int is_stop;
} CSTUNER_OBJ;

typedef enum {
	DVBS_TUNER_MOD_NONE = 0x00,	/* Modulation unknown */
	DVBS_TUNER_MOD_ALL = 0x1FF,	/* Logical OR of all MODs */
	DVBS_TUNER_MOD_QPSK = 1,
	DVBS_TUNER_MOD_8PSK = (1 << 1),
	DVBS_TUNER_MOD_QAM = (1 << 2),
	DVBS_TUNER_MOD_16QAM = (1 << 3),
	DVBS_TUNER_MOD_32QAM = (1 << 4),
	DVBS_TUNER_MOD_64QAM = (1 << 5),
	DVBS_TUNER_MOD_128QAM = (1 << 6),
	DVBS_TUNER_MOD_256QAM = (1 << 7),
	DVBS_TUNER_MOD_BPSK = (1 << 8)
} DVBS_TUNER_Modulation_e;

static CSTUNER_OBJ cstuner_obj;

sem_t sem_pll; /* zhongkai's ugly code, FIXME */

struct diseqc_master_cmd {
	unsigned char msg[6];
	unsigned char msg_len;
};

static void _tuner_hw_reset(void);
static int stv0288_set_FEC(unsigned char fec);
static int stv0288_send_diseqc_msg(struct diseqc_master_cmd *m);
static void _tuner_set_polarization(unsigned int polarization);
static int _tuner_start_scan_(CSTUNER_HANDLE handle);
static void *tuner_scan_proc(void *param);
static int _tuner_search_list_(CSTUNER_HANDLE handle, unsigned int freq);

CSTUNER_HANDLE CSTUNER_Open(CSTUNER_DEV_TYPE dev_type)
{
	UNUSED_VARIABLE(dev_type);

	if (cstuner_obj.inst_cnt++ > 0) 
		return (CSTUNER_HANDLE) &cstuner_obj;

	memset(&cstuner_obj, 0, sizeof(CSTUNER_OBJ));

	cstuner_obj.obj_type = CSTUNER_OBJ_TYPE;

	return (CSTUNER_HANDLE) &cstuner_obj;
}

CSAPI_RESULT CSTUNER_Close(CSTUNER_HANDLE handle)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ*)handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	if (dev_obj->inst_cnt-- > 0) {
		return CSAPI_SUCCEED;
	} else {
		dev_obj->is_stop = 1;
		if (pthread_join(dev_obj->thread_tuner, NULL) < 0) {
			dev_obj->err = TUNER_ERR_UNKNOWN;
			return CSAPI_FAILED;
		}
	}
 
	dev_obj->obj_type = 0;
	dev_obj->TunerInfo.Status = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_Init(CSTUNER_HANDLE handle, CSTUNER_InitParams params)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	if (dev_obj->inst_cnt > 1) 	/* 02.05.2008 ehnus Fix bug! Modified from 0 to 1 */
		return CSAPI_SUCCEED;

	dev_obj->tuner_notify = params.notify_proc;

	sem_init(&dev_obj->sem_tuner, 0, 1);
	sem_init(&sem_pll, 0, 1);

	i2c_init();
	_tuner_hw_reset();
	stv0288_init(0);

	if (0 != pthread_create(&dev_obj->thread_tuner, NULL, (void *)tuner_scan_proc, dev_obj))
	{
		dev_obj->err = TUNER_ERR_UNKNOWN;
		return CSAPI_FAILED;
	}

	dev_obj->err = TUNER_NO_ERROR;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_SetThresholdList(CSTUNER_HANDLE handle, CSTUNER_ThresholdList * threshold_list)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	dev_obj->ThresholdList = threshold_list;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_GetThresholdList(CSTUNER_HANDLE handle, CSTUNER_ThresholdList ** threshold_list)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	*threshold_list = dev_obj->ThresholdList;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_SetScanList(CSTUNER_HANDLE handle, CSTUNER_ScanList * scan_list)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	dev_obj->Scanlist = scan_list;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_GetScanList(CSTUNER_HANDLE handle, CSTUNER_ScanList ** scan_list)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;
	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	*scan_list = dev_obj->Scanlist;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_SetFrequency(CSTUNER_HANDLE handle, CSTUNER_ScanParams params)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	dev_obj->TunerInfo.Frequency = (params.LnbFrequency > params.Frequency) ? 
					(params.LnbFrequency - params.Frequency) : (params.Frequency - params.LnbFrequency);
	dev_obj->TunerInfo.SymbolRate = params.SymbolRate;
	dev_obj->TunerInfo.FECRates = params.FECRates;
	dev_obj->TunerInfo.LnbFrequency = params.LnbFrequency;
	dev_obj->TunerInfo.Polarization = params.Polarization;
	
	_tuner_set_polarization(dev_obj->TunerInfo.Polarization);
	stv0288_set_FEC(dev_obj->TunerInfo.FECRates);
	_tuner_start_scan_(handle);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_SetFrequencyWithTimeout(CSTUNER_HANDLE handle, CSTUNER_ScanParams params, int time_out)
{
	int loop = 0;
	int timeout;
	struct timeval tv;
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	if (gettimeofday(&tv, NULL) < 0) {
		printf("gettimeofday: %s\n", strerror(errno));	
		return CSAPI_FAILED;
	}
	timeout = tv.tv_sec + time_out;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	dev_obj->TunerInfo.Frequency = (params.LnbFrequency > params.Frequency) ? 
					(params.LnbFrequency - params.Frequency) : (params.Frequency - params.LnbFrequency);
	dev_obj->TunerInfo.SymbolRate = params.SymbolRate;
	dev_obj->TunerInfo.FECRates = params.FECRates;
	dev_obj->TunerInfo.LnbFrequency = params.LnbFrequency;
	dev_obj->TunerInfo.Polarization = params.Polarization;
	
	_tuner_set_polarization(dev_obj->TunerInfo.Polarization);
	stv0288_set_FEC(dev_obj->TunerInfo.FECRates);
	_tuner_start_scan_(handle);

	/* Sun He: Add timeout at Mon Jun  2 16:52:10 CST 2008 */
	if (timeout == 0) loop = 1;
	while (dev_obj->TunerInfo.Status != S_TUNER_STATUS_LOCKED) 
	{
		usleep(1000);
		if (!loop) {
			if (gettimeofday(&tv, NULL) < 0) {
				printf("gettimeofday: %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			if (tv.tv_sec > timeout) {
				dev_obj->err = TUNER_ERROR_TIMEOUT;
				return CSAPI_FAILED;
			}
		}
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_EnableAFC(CSTUNER_HANDLE handle, unsigned char enable)
{
	UNUSED_VARIABLE(handle);
	UNUSED_VARIABLE(enable);

	// TODO: Not Needed.

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_SetDisecQ(CSTUNER_HANDLE handle, unsigned char feed_number)
{
	struct diseqc_master_cmd cmds;
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);
	
	cmds.msg[0] = 0xe0; /* send commands to device, response is not needed.*/
	cmds.msg[1] = 0x00; /* to all of devices. */
	cmds.msg[2] = 0x6b; /* go to a dedicated position. */
	cmds.msg[3] = feed_number; /* a specified postion where will go to. */
	cmds.msg_len = 4;
	
	stv0288_send_diseqc_msg(&cmds);
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_Scan(CSTUNER_HANDLE handle, int freq_from, int freq_to, int freq_step, int time_out)
{
	int timeout;
	int bad_program = 0;
/*        int findlist = 0;*/
	int loop = 0, i = 0;
	struct timeval tv;

	if (gettimeofday(&tv, NULL) < 0) {
		printf("gettimeofday: %s\n", strerror(errno));	
		return CSAPI_FAILED;
	}
	timeout = tv.tv_sec + time_out;

	CSTUNER_ScanParams ScanParams;
	CSTUNER_ScanParams *scanparam;
	CSTUNER_SignalThreshold *threshould;
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	if (!dev_obj->Scanlist) return CSAPI_FAILED;

	if (time_out == 0) loop = 1;

	dev_obj->scan_rec.freq_from = freq_from;
	dev_obj->scan_rec.freq_to = freq_to;
	dev_obj->scan_rec.freq_step = freq_step;
	dev_obj->scan_rec.freq_cur = freq_from;

	do {
/*                findlist = 0;*/
		bad_program = 0;

/*                if (dev_obj->scan_rec.freq_cur > dev_obj->scan_rec.freq_to) */
/*                {*/
/*                        dev_obj->err = TUNER_ERROR_SCAN_OUTSIDE;*/
/*                        return CSAPI_FAILED;*/
/*                }*/

		if ((i = _tuner_search_list_(handle, (unsigned int) dev_obj->scan_rec.freq_cur)) == -1) 
		{
			dev_obj->scan_rec.freq_cur += dev_obj->scan_rec.freq_step;
/*                        findlist = 1;*/

			continue;
		}

		scanparam = dev_obj->Scanlist->ScanList;

		scanparam += i;
		ScanParams.Frequency = dev_obj->scan_rec.freq_cur;
		ScanParams.SymbolRate = scanparam->SymbolRate;
		ScanParams.LnbFrequency = scanparam->LnbFrequency;
		ScanParams.Polarization = scanparam->Polarization;
		ScanParams.FECRates = scanparam->FECRates;

		CSTUNER_SetFrequency(handle, ScanParams);
		dev_obj->scan_rec.freq_cur += dev_obj->scan_rec.freq_step;

		usleep(500000);

		while (dev_obj->TunerInfo.Status != S_TUNER_STATUS_LOCKED) 
		{
			usleep(1000);
			if (!loop) {
				if (gettimeofday(&tv, NULL) < 0) {
					printf("gettimeofday: %s\n", strerror(errno));
					return CSAPI_FAILED;
				}
				if (tv.tv_sec > timeout) {
					dev_obj->err = TUNER_ERROR_TIMEOUT;
					return CSAPI_FAILED;
				}
			}
		}

		if (dev_obj->TunerInfo.Status == S_TUNER_STATUS_LOCKED) 
		{
			if ((dev_obj->ThresholdList->NumElements) != 0) 
			{
				threshould = dev_obj->ThresholdList->ThresholdList;
			
				threshould += i;
				if ((dev_obj->TunerInfo.SignalQuality < (unsigned int) threshould->SignalLow) || 
					(dev_obj->TunerInfo.SignalQuality > (unsigned int) threshould->SignalHigh))
				{
					dev_obj->err = TUNER_ERROR_QUALITY;
					return CSAPI_FAILED;
				}
			}

			return CSAPI_SUCCEED;
		}
		else {
			bad_program = 1;
		}

	} while (((dev_obj->scan_rec.freq_cur <= dev_obj->scan_rec.freq_to) && 
		(dev_obj->TunerInfo.Status != S_TUNER_STATUS_LOCKED)) || 
		(bad_program == 1)); /* || 
		(findlist == 1));*/

	dev_obj->err = TUNER_ERROR_SCAN_OUTSIDE;
	return CSAPI_FAILED;
}

CSAPI_RESULT CSTUNER_ScanContinue(CSTUNER_HANDLE handle, int time_out)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	if (dev_obj->scan_rec.freq_cur > dev_obj->scan_rec.freq_to) 
	{
		dev_obj->scan_rec.freq_cur = dev_obj->scan_rec.freq_to;
	}

	return (CSTUNER_Scan(handle, 
			dev_obj->scan_rec.freq_cur, 
			dev_obj->scan_rec.freq_to, 
			dev_obj->scan_rec.freq_step, 
			time_out));
}

CSAPI_RESULT CSTUNER_ScanAbort(CSTUNER_HANDLE handle)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	UNUSED_VARIABLE(handle);
	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	if (dev_obj->TunerInfo.Status != S_TUNER_STATUS_UNLOCKED) {

		sem_wait(&dev_obj->sem_tuner);

		/* forcibly unlock the tuner  */
		dev_obj->TunerInfo.Status = S_TUNER_STATUS_UNLOCKED;
		dev_obj->TunerInfo.SignalQuality = 0;
		dev_obj->TunerInfo.SignalLevel = 0;

		sem_post(&dev_obj->sem_tuner);
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTUNER_GetTunerInfo(CSTUNER_HANDLE handle, CSTUNER_Info* tuner_info)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	memcpy(tuner_info, &dev_obj->TunerInfo, sizeof(CSTUNER_Info));

	return CSAPI_SUCCEED;
}

static int _tuner_search_list_(CSTUNER_HANDLE handle, unsigned int freq)
{
	int i;
	CSTUNER_ScanParams *scanparem;
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTUNER_OBJ_TYPE);

	scanparem = dev_obj->Scanlist->ScanList;

	for (i = 0; i < (dev_obj->Scanlist->NumElements); i++) 
	{
		if (freq == scanparem->Frequency)
			return i;

		scanparem++;
		sleep(1);
	}

	return -1;
}

static void _tuner_hw_reset(void)
{
	gpio_write(6, 0);
	usleep(400000);

	gpio_write(6, 1);
	usleep(400000);
}

/* 
 * 0 -V, 1 - H 
 */
static void _tuner_set_polarization(unsigned int polarization)
{
	gpio_write(12, 1); /* LNB Power On/Off */
	gpio_write(13, polarization != 0 ? 1 : 0);
}

static void *tuner_scan_proc(void *param)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ*)param;

	while (!dev_obj->is_stop) 
	{
		usleep(300000);	// sleep 1s Modify By River 09.29.2007

		sem_wait(&dev_obj->sem_tuner);

		switch (dev_obj->TunerInfo.Status) 
		{
			case S_TUNER_STATUS_SCANNING:
				dev_obj->check_times = 0;

				if ((TUNER_LOCKED != check_lock_status())) 
				{
					search_false_lock(dev_obj->TunerInfo.Frequency, dev_obj->TunerInfo.SymbolRate);
				}
				if (check_lock_status() == TUNER_LOCKED) 
				{
					dev_obj->TunerInfo.Status = S_TUNER_STATUS_LOCKED;
					dev_obj->tuner_notify(TUNER_EVT_LOCKED, TUNER_SAT);
				}

				else {
					dev_obj->TunerInfo.Status = S_TUNER_STATUS_WAITLOCKED;
				}

				break;

			case S_TUNER_STATUS_WAITLOCKED:
				search_false_lock(dev_obj->TunerInfo.Frequency, dev_obj->TunerInfo.SymbolRate);

				if (check_lock_status() == TUNER_LOCKED) {
					dev_obj->check_times = 0;

					dev_obj->TunerInfo.Status = S_TUNER_STATUS_LOCKED;
					dev_obj->tuner_notify(TUNER_EVT_LOCKED, TUNER_SAT);
				}
				else {
					if (dev_obj->check_times < 5) {
						dev_obj->check_times++;
					}
					else {
						dev_obj->check_times = 0;
						dev_obj->TunerInfo.Status = S_TUNER_STATUS_UNLOCKED;
						dev_obj->tuner_notify(TUNER_EVT_UNLOCKED, TUNER_SAT);
					}
				}

				break;

			case S_TUNER_STATUS_LOCKED:
				if (check_lock_status() == TUNER_LOCKED) {
					dev_obj->TunerInfo.Status = S_TUNER_STATUS_LOCKED;
					//check_signal_strength_and_quality(&dev_obj->TunerInfo.SignalLevel,
									//  &dev_obj->TunerInfo.SignalQuality);
				}
				else {
					dev_obj->TunerInfo.Status = S_TUNER_STATUS_WAITLOCKED;
					dev_obj->TunerInfo.SignalQuality = 0;
					dev_obj->TunerInfo.SignalLevel = 0;
				}

				break;

			case S_TUNER_STATUS_UNLOCKED:
				if (check_lock_status() == TUNER_LOCKED || check_lock_status() == TUNER_CARRIER_OK) {
					dev_obj->TunerInfo.Status = S_TUNER_STATUS_WAITLOCKED;
				}
				else {
					dev_obj->TunerInfo.SignalQuality = 0;
					dev_obj->TunerInfo.SignalLevel = 0;
					dev_obj->TunerInfo.FECRates = 0;
					dev_obj->TunerInfo.Modulation = DVBS_TUNER_MOD_QPSK;
				}

				break;

			default:
				break;
		}

		sem_post(&dev_obj->sem_tuner);
	}

	return NULL;
}

static int _tuner_start_scan_(CSTUNER_HANDLE handle)
{
	CSTUNER_OBJ *dev_obj = handle;

	sem_wait(&dev_obj->sem_tuner);

	VZ0295SetFrequency(dev_obj->TunerInfo.Frequency, dev_obj->TunerInfo.SymbolRate);

	dev_obj->TunerInfo.Status = S_TUNER_STATUS_SCANNING;

	sem_post(&dev_obj->sem_tuner);

	return 0;
}

/* 
 * the implementation of DiSEqC.
 */
static int stv0288_set_FEC(unsigned char fec)
{
	switch (fec) 
	{
		case TUNER_FEC_AUTO:
			stv0288_write(0x31, 0x1f);
			break;
		case TUNER_FEC_1_2:
			stv0288_write(0x31, 0x01);
			break;
		case TUNER_FEC_2_3:
			stv0288_write(0x31, 0x02);
			break;
		case TUNER_FEC_3_4:
			stv0288_write(0x31, 0x04);
			break;
		case TUNER_FEC_5_6:
			stv0288_write(0x31, 0x08);
			break;
		case TUNER_FEC_7_8:
			stv0288_write(0x31, 0x10);
			break;
		default:
			break;
	}

	return 0;
}

// DELME static int stv0288_get_FEC(void)
// DELME {
// DELME 	static char fec_tab[] = 
// DELME 	{ 	TUNER_FEC_2_3, 
// DELME 		TUNER_FEC_3_4, 
// DELME 		TUNER_FEC_5_6,
// DELME 		TUNER_FEC_7_8, 
// DELME 		TUNER_FEC_1_2
// DELME 	};
// DELME 
// DELME 	unsigned char index;
// DELME 
// DELME 	index = stv0288_read(0x1b);
// DELME 	index &= 0x7;
// DELME 
// DELME 	if (index > 4)
// DELME 		return TUNER_FEC_AUTO;
// DELME 
// DELME 	return fec_tab[index];
// DELME }

static int stv0288_wait_diseqc_fifo(int timeout)
{
	int ii = 0;

	while (stv0288_read(0x0a) & 1) 
	{
		if (++ii > timeout) return -1;

		usleep(10000);
	};

	return 0;
}

static int stv0288_wait_diseqc_idle(int timeout)
{
	int ii = 0;

	while ((stv0288_read(0x0a) & 3) != 2) {
		if (++ii > timeout) return -1;

		usleep(10000);
	};

	return 0;
}

static int stv0288_send_diseqc_msg(struct diseqc_master_cmd *m)
{
	int i;
	unsigned char val;

	if (stv0288_wait_diseqc_idle(100) < 0)
		return -1;

	val = stv0288_read(0x08);

	stv0288_write(0x08, (val & ~0x7) | 0x6); /* DiSEqC mode */

	for (i = 0; i < m->msg_len; i++) {
		if (stv0288_wait_diseqc_fifo(100) < 0)
			return -3;

		stv0288_write(0x09, m->msg[i]);
	}

	if (stv0288_wait_diseqc_idle(100) < 0)
		return -5;

	return 0;
}

CSTUNER_ErrCode CSTUNER_GetErrCode(CSTUNER_HANDLE handle)
{
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	if (dev_obj == NULL || (dev_obj->dev_fd < 0) || dev_obj->obj_type != 'T')
		return TUNER_ERROR_INVALID_PARAMETERS;
	return (dev_obj->err);
}

char *CSTUNER_GetErrString(CSTUNER_HANDLE handle)
{
	char *errorstr;
	CSTUNER_OBJ *dev_obj = (CSTUNER_OBJ *) handle;

	if (dev_obj == NULL || dev_obj->dev_fd < 0 || dev_obj->obj_type != 'I')
		errorstr = " TUNER: Input PARAMATER is invalid\n";
	else {
		switch (dev_obj->err) {
			case TUNER_NO_ERROR:
				errorstr = " TUNER: Operation is success\n";
				break;
			case TUNER_ERROR_OPEN_FAILED:
				errorstr = " TUNER: Open  operation is failed\n";
				break;
			case TUNER_ERROR_IOCTL_FAILED:
				errorstr = " TUNER: Ioctl  operation is failed\n";
				break;
			case TUNER_ERROR_INVALID_HANDLE:
				errorstr = " TUNER: Invalid handle operation is failed\n";
				break;
			case TUNER_ERROR_INVALID_PARAMETERS:
				errorstr = " TUNER: Invalid param operation is failed\n";
				break;
			case TUNER_ERROR_UNKNOWN_DEVICE:
				errorstr = " TUNER: Unknown device  operation is failed\n";
				break;
			case TUNER_ERROR_DEVICE_BUSY:
				errorstr = " TUNER: Error device operation is failed\n";
				break;
			case TUNER_ERROR_ALREADY_INITIALIZED:
				errorstr = " TUNER: TUNER_ERROR_ALREADY_INITIALIZE evice operation is failed\n";
				break;
			case TUNER_ERROR_NOT_INITIALIZED:
				errorstr = " TUNER: TUNER_ERROR_NOT_INITIALIZED evice operation is failed\n";
				break;
			case TUNER_ERROR_INVALID_STATUS:
				errorstr = " TUNER: TUNER_ERROR_INVALID_STATU evice operation is failed\n";
				break;
			case TUNER_ERROR_NO_MEMORY:
				errorstr = " TUNER: TUNER_ERROR_NO_MEMOR evice operation is failed\n";
				break;
			case TUNER_ERROR_I2C:
				errorstr = " TUNER: TUNER_ERROR_I2C evice operation is failed\n";
				break;

			case TUNER_ERROR_TIMEOUT:
				errorstr = " TUNER: TUNER_ERROR_TIMEOUT evice operation is failed\n";
				break;
			case TUNER_ERROR_GPIO:
				errorstr = " TUNER: TUNER_ERROR_GPIO evice operation is failed\n";
				break;
			case TUNER_ERROR_QUALITY:
				errorstr = " TUNER: TUNER_ERROR_QUALIT evice operation is failed\n";
				break;
			case TUNER_ERROR_SCAN_OUTSIDE:
				errorstr = " TUNER: TUNER_ERROR_SCAN_OUTSIDE evice operation is failed\n";
				break;
			case TUNER_ERR_UNKNOWN:
				errorstr = " TUNER: TUNER_ERR_UNKNO evice operation is failed\n";
				break;
			default:
				errorstr = " TUNER: Invaild Error Type\n";
				break;
		}
	}
	return errorstr;
}
