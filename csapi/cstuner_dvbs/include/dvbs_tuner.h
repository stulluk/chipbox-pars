#ifndef __DVBS_TUNER_H__
#define __DVBS_TUNER_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include "csapi.h"

typedef void *CSTUNER_HANDLE;

typedef enum { 
	TUNER_SAT = 0
} CSTUNER_DEV_TYPE;

typedef enum {
	TUNER_FEC_NONE = 0,
	TUNER_FEC_1_2,
	TUNER_FEC_2_3,
	TUNER_FEC_3_4,
	TUNER_FEC_4_5,
	TUNER_FEC_5_6,
	TUNER_FEC_6_7,
	TUNER_FEC_7_8,
	TUNER_FEC_8_9,
	TUNER_FEC_AUTO
} CSTUNER_FEC_RATES;

typedef struct {
	unsigned int  Frequency;	/*MHz */
	unsigned int  SymbolRate;	/*MS/s */
	unsigned int  LnbFrequency;	/*MHz */
	unsigned int  Polarization;	/* 'V' FALSE 'H' TRUE */

	CSTUNER_FEC_RATES FECRates;
} CSTUNER_ScanParams;

typedef enum { 
	S_TUNER_STATUS_UNLOCKED, 
	S_TUNER_STATUS_SCANNING, 
	S_TUNER_STATUS_LOCKED, 
	S_TUNER_STATUS_WAITLOCKED,
	S_TUNER_STATUS_NOT_FOUND
} CS_TUNER_STATUS;

typedef enum { 
	TUNER_EVT_NO_OPERATION = 0,	/* No current scan */
	TUNER_EVT_LOCKED,		/* tuner locked    */
	TUNER_EVT_UNLOCKED,		/* tuner lost lock */
	TUNER_EVT_WAITLOCKED, 
	TUNER_EVT_SCAN_FAILED,		/* scan failed     */
	TUNER_EVT_TIMEOUT,		/* scan timed out  */
	TUNER_EVT_SIGNAL_CHANGE		/* Signal threshold has changed */
} CSTUNER_EVENT_TYPE;

typedef struct tagTUNER_Info 
{
	CSTUNER_DEV_TYPE type;
	int Status;

	unsigned int LnbFrequency;	/*khz */
	unsigned char Modulation;
	unsigned int Frequency;
	unsigned int SymbolRate;
	unsigned int Polarization;
	unsigned int SignalQuality;
	unsigned int SignalLevel;
	unsigned int LockedFreq;
	
	CSTUNER_FEC_RATES FECRates;

} CSTUNER_Info;

typedef enum { 
	TUNER_NO_ERROR = 0, 
	TUNER_ERROR_OPEN_FAILED, 
	TUNER_ERROR_IOCTL_FAILED, 
	TUNER_ERROR_INVALID_HANDLE,
	TUNER_ERROR_INVALID_PARAMETERS, 
	TUNER_ERROR_UNKNOWN_DEVICE, 
	TUNER_ERROR_DEVICE_BUSY,
	TUNER_ERROR_ALREADY_INITIALIZED, 
	TUNER_ERROR_NOT_INITIALIZED, 
	TUNER_ERROR_INVALID_STATUS,
	TUNER_ERROR_NO_MEMORY, 
	TUNER_ERROR_I2C, 
	TUNER_ERROR_TIMEOUT, 
	TUNER_ERROR_GPIO, 
	TUNER_ERROR_QUALITY,
	TUNER_ERROR_SCAN_OUTSIDE, 
	TUNER_ERR_UNKNOWN
} CSTUNER_ErrCode;

typedef void (*CSTUNER_NotifyFunc) (CSTUNER_EVENT_TYPE evt_type, CSTUNER_DEV_TYPE dev_type);

typedef struct tagTUNER_InitParams 
{
	CSTUNER_NotifyFunc notify_proc;
} CSTUNER_InitParams;

typedef struct tagTUNER_SignalThreshold 
{
	int SignalLow;
	int SignalHigh;
} CSTUNER_SignalThreshold;

typedef struct tagTUNER_ThresholdList {
	int NumElements;
	CSTUNER_SignalThreshold *ThresholdList;
} CSTUNER_ThresholdList;

typedef struct tagTUNER_ScanList {
	int NumElements;
	CSTUNER_ScanParams *ScanList;
} CSTUNER_ScanList;

CSTUNER_HANDLE CSTUNER_Open(CSTUNER_DEV_TYPE dev_type);
CSAPI_RESULT   CSTUNER_Close(CSTUNER_HANDLE handle);

CSAPI_RESULT CSTUNER_Init(CSTUNER_HANDLE handle, CSTUNER_InitParams params);
CSAPI_RESULT CSTUNER_SetFrequency(CSTUNER_HANDLE handle, CSTUNER_ScanParams params);
/* Sun He: time out version */
CSAPI_RESULT CSTUNER_SetFrequencyWithTimeout(CSTUNER_HANDLE handle, CSTUNER_ScanParams params, int time_out);
CSAPI_RESULT CSTUNER_EnableAFC(CSTUNER_HANDLE handle, unsigned char enable);
CSAPI_RESULT CSTUNER_SetDisecQ(CSTUNER_HANDLE handle, unsigned char feed_number);

CSAPI_RESULT CSTUNER_SetThresholdList(CSTUNER_HANDLE handle, CSTUNER_ThresholdList * threshold_list);
CSAPI_RESULT CSTUNER_GetThresholdList(CSTUNER_HANDLE handle, CSTUNER_ThresholdList ** threshold_list);

CSAPI_RESULT CSTUNER_SetScanList(CSTUNER_HANDLE handle, CSTUNER_ScanList * scan_list);
CSAPI_RESULT CSTUNER_GetScanList(CSTUNER_HANDLE handle, CSTUNER_ScanList ** scan_list);

CSAPI_RESULT CSTUNER_Scan(CSTUNER_HANDLE handle, int freq_from, int freq_to, int freq_step, int time_out);
CSAPI_RESULT CSTUNER_ScanContinue(CSTUNER_HANDLE handle, int time_out);
CSAPI_RESULT CSTUNER_ScanAbort(CSTUNER_HANDLE handle);

CSAPI_RESULT CSTUNER_GetTunerInfo(CSTUNER_HANDLE handle, CSTUNER_Info * tuner_info);

CSTUNER_ErrCode CSTUNER_GetErrCode(CSTUNER_HANDLE handle);
char *         CSTUNER_GetErrString(CSTUNER_HANDLE handle);

#ifdef __cplusplus
} 
#endif

#endif
