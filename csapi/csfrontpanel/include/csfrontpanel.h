#ifndef __CSAPI_FRONTPANEL_H__
#define __CSAPI_FRONTPANEL_H__

#include "../include/global.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************************************/

typedef enum 
{ 
	CSFP_NO_ERROR = 0, 
	CSFP_ERROR_OPEN_FPC_FAILED,		         /* open fpc device filed          */
	CSFP_ERROR_OPEN_INPUT_FAILED,		     /* open fpc device filed          */
	CSFP_ERROR_OPEN_SERIAL_FAILED,		     /* open fpc device filed          */
	CSFP_ERROR_IOCTL_SETSYSTEMCODE_FAILED,   /* ioctl set systemcode failed   */
	CSFP_ERROR_IOCTL_SETKEYSCAN_FAILED,      /* ioctl enable/disable keyscan failed*/
	CSFP_ERROR_IOCTL_SETRC_FAILED,           /* ioctl enable/disable rc failed*/
	CSFP_ERROR_IOCTL_ENABLESERIALIN_FAILED,  /* ioctl enable serial port  failed */
	CSFP_ERROR_IOCTL_DISABLESERIALIN_FAILED, /* ioctl disable serial port failed */
	CSFP_ERROR_IOCTL_RESETSERIALIN_FAILED,   /* ioctl reset serial port failed*/
	CSFP_ERROR_IOCTL_SETSERIALIN_FAILED,     /* ioctl set attribute of serial port failed*/
	CSFP_ERROR_IOCTL_SETBitTimeCnt_FAILED,   /* ioctl set bit time count failed */
	CSFP_ERROR_IOCTL_GETBitTimeCnt_FAILED,   /* ioctl get bit time count failed */ 
	CSFP_ERROR_NOTOPEN_SERIALIN,             /* serial port is not opened */
	CSFP_ERROR_INVALID_PARAMETERS,	         /* Bad parameter passed       */
	CSFP_ERROR_UNKNOWN_DEVICE,	             /* Unknown device name            */
	CSFP_ERROR_DEVICE_BUSY,		             /* Device is currently busy       */
	CSFP_ERROR_INVALID_HANDLE,	             /* Handle is not valid            */
	CSFP_ERROR_ALREADY_INITIALIZED,	         /* Device already initialized */
	CSFP_ERROR_CLOSE_FPC,                    /* close front panel device is failed */
	CSFP_ERROR_CLOSE_SERIAL,				 /* close serial device failed */
	CSFP_ERROR_CLOSE_INPUT,					 /* close input device failed */
	CSFP_ERROR_REGISTER_CALLBACK			 /* register call back failed */
} CSFP_ErrCode;


typedef void* CSFP_HANDLE;


/**************Structure*************/
typedef enum 
{
	CSFP_LEDDISP_OFF = 0,
	CSFP_LEDDISP_BLINK = 1,
	CSFP_LEDDISP_ON = 2 
}CSFP_LEDDISPMODE;
typedef enum  
{
	CSFP_LEDDISP_ON_BRIGHT = 0,
	CSFP_LEDDISP_ON_DIM = 1,
	CSFP_LEDDISP_FAST_BRIGHT = 2,
	CSFP_LEDDISP_FAST_DIM = 3,
	CSFP_LEDDISP_SLOW_BRIGHT = 4,
	CSFP_LEDDISP_SLOW_DIM = 5 
}CSFP_LEDDISPATTR ;


typedef void* CSFP_LEDDISP_HANDLE;
typedef enum 
{
	CSFP_KEYSCAN_IN = 0,
    CSFP_RCBYTE_IN = 2,
	
}CSFP_NOTIFYEVENT;

typedef enum
{
	CSFP_RC_RC5 =0,
	CSFP_RC_NEC =1
}CSFP_RC_TYPE;

CSFP_HANDLE  CSFP_Open(void);
CSAPI_RESULT CSFP_Close(CSFP_HANDLE handle);

CSAPI_RESULT CSFP_EnableKeyScan (CSFP_HANDLE handle);
CSAPI_RESULT CSFP_DisableKeyScan(CSFP_HANDLE handle);
CSAPI_RESULT CSFP_SetKeyScanNumKeys(CSFP_HANDLE handle, unsigned char num_keys);
CSAPI_RESULT CSFP_SetKeyScanMap(CSFP_HANDLE handle, unsigned char raw_input, unsigned char sys_code);

CSAPI_RESULT CSFP_EnableSystemCode(CSFP_HANDLE handle);
CSAPI_RESULT CSFP_DisableSystemCode(CSFP_HANDLE handle);
CSAPI_RESULT CSFP_SetSystemCode(CSFP_HANDLE handle, int system_code);
CSAPI_RESULT CSFP_EnableRemoteController (CSFP_HANDLE handle); 
CSAPI_RESULT CSFP_DisableRemoteController (CSFP_HANDLE handle);
CSAPI_RESULT CSFP_SetRemoteControllerType(CSFP_HANDLE handle, CSFP_RC_TYPE rctype); 

CSAPI_RESULT CSFP_SetBitTimeCnt(CSFP_HANDLE handle, unsigned short bittime);
CSAPI_RESULT CSFP_GetBitTimeCnt(CSFP_HANDLE handle, unsigned short *bittime);

CSAPI_RESULT CSFP_SetLEDDisplay(CSFP_HANDLE handle, const char const* num_str);
CSAPI_RESULT CSFP_GetLEDDisplay(CSFP_HANDLE handle, unsigned char *str_display );

CSAPI_RESULT CSFP_SetLEDDisplayChar(CSFP_HANDLE handle, unsigned char *char_value);
CSAPI_RESULT CSFP_SetLEDDisplayMode (CSFP_HANDLE handle, CSFP_LEDDISPMODE mode );
CSAPI_RESULT CSFP_SetLEDDisplayAttr (CSFP_HANDLE handle, CSFP_LEDDISPATTR attr );
CSAPI_RESULT CSFP_SetLEDDisplayRaw(CSFP_HANDLE handle, unsigned char *raw_value, int raw_len);

CSAPI_RESULT CSFP_GetLEDDisplayChar(CSFP_HANDLE handle, unsigned char *char_value);
CSAPI_RESULT CSFP_GetLEDDisplayMode(CSFP_HANDLE handle, CSFP_LEDDISPMODE *mode );
CSAPI_RESULT CSFP_GetLEDDisplayAttr(CSFP_HANDLE handle, CSFP_LEDDISPATTR *attr);
CSAPI_RESULT CSFP_GetLEDDisplayRaw(CSFP_HANDLE handle, unsigned char *raw_value);

CSAPI_RESULT CSFP_SetLEDDisplayPos(CSFP_HANDLE handle, unsigned char row, unsigned char column);
/*row is not used now, column can be set 0-3 to display, other cloumn value will display the character on
 all LED */

CSAPI_RESULT CSFP_SetNotify(CSFP_HANDLE handle, void (*call_back_function)(CSFP_HANDLE, CSFP_NOTIFYEVENT *, unsigned char *), 
										CSFP_NOTIFYEVENT fp_event, unsigned char event_enable);

CSFP_ErrCode CSFP_GetErrCode(CSFP_HANDLE handle);


char *CSFP_GetErrString(CSFP_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /*  __CSAPI_FRONTPANEL_H__ */
