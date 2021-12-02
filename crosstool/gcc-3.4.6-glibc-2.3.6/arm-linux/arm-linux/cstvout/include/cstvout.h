
#ifndef __CSAPI_TVOUT_H__
#define __CSAPI_TVOUT_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CSTVOUT_HANDLE;

typedef enum
{
	TVOUT_CHANNEL0 = 0,
	TVOUT_CHANNEL1 = 1
}CSTVOUT_CHANNEL_ID;

typedef enum
{
        OUTPUT_MODE_CVBS_SVIDEO = 0, /* TVE0 not support */
        OUTPUT_MODE_YPBPR,
        OUTPUT_MODE_RGB/* TVE1 not support */
} CSTVOUT_OUTPUT_MODE;

typedef enum
{
        TVOUT_VID_DEV_0 = 0,
        TVOUT_VID_DEV_1, 
} CSTVOUT_VID_DEV;

typedef enum
{
        TVOUT_OSD_LAYER_0 = 0,
        TVOUT_OSD_LAYER_1, 
} CSTVOUT_OSD_LAYER;

typedef enum 
{
	TVOUT_MODE_576I = 0,
	TVOUT_MODE_480I,
	TVOUT_MODE_576P,
	TVOUT_MODE_480P,
	TVOUT_MODE_720P50,
	TVOUT_MODE_720P60,
	TVOUT_MODE_1080I25,
	TVOUT_MODE_1080I30,

	TVOUT_MODE_SECAM,
	TVOUT_MODE_PAL_M,
	TVOUT_MODE_PAL_N,
	TVOUT_MODE_PAL_CN,
	TVOUT_MODE_1080P24,
	TVOUT_MODE_1080P25,//not support
	TVOUT_MODE_1080P30,//not support

	TVOUT_RGB_640X480_60FPS,
	TVOUT_RGB_800X600_60FPS,
	TVOUT_RGB_800X600_72FPS,
	TVOUT_RGB_1024X768_60FPS,
	TVOUT_RGB_1280X1024_50FPS,
	TVOUT_RGB_1600X1000_60FPS,// not support
       TVOUT_RGB_1280X1024_60FPS,
       TVOUT_RGB_1280X720_60FPS,
	TVOUT_RGB_848X480_60FPS,
	TVOUT_RGB_800X480_60FPS,
} CSTVOUT_MODE;

typedef enum CSTVOUT_WSSSTD_ {
	VBI_WSS_PAL = 0,
	VBI_WSS_NTSC,
	VBI_WSS_END
} CSTVOUT_WSSSTD;

typedef enum CSTVOUT_WSSTYPE_ {
	TVE_VBI_WSS = 0,
	TVE_VBI_CGMS,	
	TVE_VBI_END
} CSTVOUT_WSSTYPE;

 
typedef enum CSTVOUT_ARATIO_ {
	WSS_AR_4TO3_FULL = 0,
	WSS_AR_14TO9_BOX_CENTER,	
	WSS_AR_14TO9_BOX_TOP,
	WSS_AR_16TO9_BOX_CENTER,
	WSS_AR_16TO9_BOX_TOP,
	WSS_AR_16TO9P_BOX_CENTER,
	WSS_AR_14TO9_FULL,	
	WSS_AR_16TO9_FULL,
	WSS_AR_END
} CSTVOUT_ARATIO;

typedef struct CSTVOUT_WSSINFO_ {
	CSTVOUT_WSSTYPE WssType;
	CSTVOUT_ARATIO	ARatio;
} CSTVOUT_WSSINFO;


typedef enum  {
	VBI_TXT_PALB = 0,
	VBI_TXT_NTSCB,
	VBI_TXT_END
}CSVOUT_TxtStandard_t;


#define TTX_LINESIZE          45
#define TTX_MAXLINES          32	

typedef struct
{
//        unsigned int OddEvenFlag;
    unsigned int ValidLines;                     /* bit-field lines  0..31 */
    unsigned char  Lines[TTX_MAXLINES][TTX_LINESIZE];
} CSVOUT_TxtPage_t;

typedef enum
{
	CSVOUT_COMP_YUV = 0,
	CSVOUT_COMP_RGB	

}CSVOUT_CompChannType_t;

/* CSTVOUT_OUTPUT configuration*/
typedef enum{
	OUTPUT_CONTROL_CVBS_SVIDEO=0,
    OUTPUT_CONTROL_MACROVISION	,
}CSTVOUT_OUTPUT_CMD;

typedef enum{
	DISABLE_CVBS_SVIDEO =0,
    ENABLE_CVBS_SVIDEO
}CONTROL_CVBS_SVIDEO;

typedef enum{
	DISABLE_MACROVISION =0,
    ENABLE_MACROVISION
}CONTROL_MACROVISION;

typedef struct TVOUT_MacrovisionConfig {
	unsigned char version;
	unsigned short conf_len;
#define MAX_CONFIG_SZ 	17
	unsigned char conf_data[MAX_CONFIG_SZ];
} CSTVOUT_MacrovisionConfig;

typedef enum
{
	DF_ZORDER_V0_V1_G = 0x0123,
	DF_ZORDER_V0_G_V1 = 0x1203,
	DF_ZORDER_V1_V0_G = 0x1023,
	DF_ZORDER_V1_G_V0 = 0x2130,
	DF_ZORDER_G_V0_V1 = 0x2301,
	DF_ZORDER_G_V1_V0 = 0x2310,
	DF_ZORDER_G1_V_G0 = 0x3012,
} CSTVOUT_DF_ZORDER;

typedef enum
{
        TVOUT_NO_ERROR = 0,
        TVOUT_ERROR_OPEN_FAILED,          /* open filed                 */
        TVOUT_ERROR_IOCTL_FAILED,         /* ioctl filed                */
        TVOUT_ERROR_INVALID_PARAMETERS,   /* Bad parameter passed       */
        TVOUT_ERROR_UNKNOWN_DEVICE,       /* Unknown device name        */
        TVOUT_ERROR_DEVICE_BUSY,          /* Device is currently busy   */
        TVOUT_ERROR_INVALID_HANDLE,       /* Handle is not valid        */
        TVOUT_ERROR_ALREADY_INITIALIZED,  /* Device already initialized */
        TVOUT_ERROR_NOT_INITIALIZED       /* Device not initialized     */
} CSTVOUT_ErrCode;

CSTVOUT_HANDLE CSTVOUT_Open(CSTVOUT_CHANNEL_ID id); /* CSM1200 only support OUTPUT Channel0 */
CSAPI_RESULT   CSTVOUT_Close(CSTVOUT_HANDLE handle);

CSAPI_RESULT   CSTVOUT_Enable(CSTVOUT_HANDLE handle);
CSAPI_RESULT   CSTVOUT_Disable(CSTVOUT_HANDLE handle);

CSAPI_RESULT   CSTVOUT_SetMode(CSTVOUT_HANDLE handle, CSTVOUT_MODE vid_mod);
CSAPI_RESULT   CSTVOUT_GetMode(CSTVOUT_HANDLE handle, CSTVOUT_MODE * vid_mod);

CSAPI_RESULT   CSTVOUT_SetZorder(CSTVOUT_HANDLE handle, CSTVOUT_DF_ZORDER zorder);
CSAPI_RESULT   CSTVOUT_SetBkColor(CSTVOUT_HANDLE handle, unsigned char r, unsigned char g, unsigned char b);

CSAPI_RESULT   CSTVOUT_SetBrightness(CSTVOUT_HANDLE handle, unsigned int brightness);
CSAPI_RESULT   CSTVOUT_GetBrightness(CSTVOUT_HANDLE handle, unsigned int * brightness);

CSAPI_RESULT   CSTVOUT_SetContrast(CSTVOUT_HANDLE handle, unsigned int constrast);
CSAPI_RESULT   CSTVOUT_GetContrast(CSTVOUT_HANDLE handle, unsigned int * constrast);

CSAPI_RESULT   CSTVOUT_SetSaturation(CSTVOUT_HANDLE handle, unsigned int saturation);
CSAPI_RESULT   CSTVOUT_GetSaturation(CSTVOUT_HANDLE handle, unsigned int * saturation);

CSTVOUT_ErrCode CSTVOUT_GetErrCode(CSTVOUT_HANDLE handle);
char *CSTVOUT_GetErrString(CSTVOUT_HANDLE handle);

CSAPI_RESULT CSTVOUT_SetCompChannel(CSTVOUT_HANDLE handle, CSVOUT_CompChannType_t CompChan);

CSAPI_RESULT CSTVOUT_WSS_Ctrl(CSTVOUT_HANDLE handle, char Enable);
CSAPI_RESULT CSTVOUT_WSS_SetFormat(CSTVOUT_HANDLE handle, CSTVOUT_WSSSTD WssStd);
CSAPI_RESULT CSTVOUT_WSS_SetInfo(CSTVOUT_HANDLE handle, CSTVOUT_WSSINFO *WssInfo);

CSAPI_RESULT CSTVOUT_TXT_Ctrl(CSTVOUT_HANDLE handle, char Enable);
CSAPI_RESULT CSTVOUT_TXT_SetFormat(CSTVOUT_HANDLE handle, CSVOUT_TxtStandard_t TxtStd);
CSAPI_RESULT CSTVOUT_TXT_SetInfo(CSTVOUT_HANDLE handle, CSVOUT_TxtPage_t *TxtPage);
//CSAPI_RESULT CSTVOUT_ControlOutput(CSTVOUT_HANDLE handle, CSTVOUT_OUTPUT_CMD cstvout_output_configure_cmd, unsigned int arg);

/* MCEnable  enables or disables MacroVision  on CVBS and S-Video analog outputs: */
/* MCEnable =0 (Disable), MCEnable=1(Enable)*/ 
/* Returns CSAPI_SUCCEEDED or CSAPI_FAILED */ 
CSAPI_RESULT CSTVOUT_EnableMacroVision(CSTVOUT_HANDLE handle, unsigned char MCEnable);
CSAPI_RESULT CSTVOUT_SetMacroVisionMode(CSTVOUT_HANDLE handle, unsigned char mode);
CSAPI_RESULT CSTVOUT_SetMacroVisionConfigData(CSTVOUT_HANDLE handle, CSTVOUT_MacrovisionConfig *config);
CSAPI_RESULT CSTVOUT_SetOutput(CSTVOUT_HANDLE handle, CSTVOUT_OUTPUT_MODE output_mode);

#ifdef __cplusplus
}
#endif

#endif

