
#ifndef __CSAPI_OSD_H__
#define __CSAPI_OSD_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CSOSD_HANDLE;

typedef enum 
{
	OSD_MODE_576I = 0,
	OSD_MODE_480I,
	OSD_MODE_576P,
	OSD_MODE_480P,
	OSD_MODE_720P50,
	OSD_MODE_720P60,
	OSD_MODE_1080I25,
	OSD_MODE_1080I30,

	OSD_MODE_SECAM,
	OSD_MODE_PAL_M,
	OSD_MODE_PAL_N,
	OSD_MODE_PAL_CN,
	OSD_MODE_1080P24,
	OSD_MODE_1080P25,
	OSD_MODE_1080P30,

	OSD_MODE_640X480_60FPS,// not support
	OSD_MODE_800X600_60FPS,
	OSD_MODE_800X600_72FPS,
	OSD_MODE_1024X768_60FPS,
	OSD_MODE_1280X1024_50FPS,
	OSD_MODE_1600X1000_60FPS,// not support
       OSD_MODE_1280X1024_60FPS,
} CSOSD_MODE;

typedef enum 
{
	OSD_LAYER_0 = 0,
	OSD_LAYER_1
} CSOSD_LAYER;

typedef enum 
{
	OSD_COLOR_DEPTH_16 = 0,
#if defined(ARCH_CSM1201)
	OSD_COLOR_DEPTH_32,
#endif
} CSOSD_COLOR_DEPTH;

typedef enum 
{
#if defined(ARCH_CSM1201)
	OSD_COLOR_FORMAT_RGB565    = 2,
	OSD_COLOR_FORMAT_ARGB4444  = 3,
	OSD_COLOR_FORMAT_ARGB1555  = 5,
	OSD_COLOR_FORMAT_ARGB8888  = 6,
#else
	OSD_COLOR_FORMAT_RGB565 = 0,
	OSD_COLOR_FORMAT_ARGB4444,
	OSD_COLOR_FORMAT_ARGB1555,
#endif
} CSOSD_COLOR_FORMAT;

typedef struct tagOSD_Config {
	CSOSD_MODE mode;

	CSOSD_COLOR_DEPTH color_depth;
	CSOSD_COLOR_FORMAT color_format;
} CSOSD_Config;

typedef struct tagOSD_KeyColor {
	char r_min;
	char r_max;

	char g_min;
	char g_max;

	char b_min;
	char b_max;
} CSOSD_KeyColor;

typedef struct tagOSD_Rect {
	unsigned int left;
	unsigned int top;
	unsigned int right;
	unsigned int bottom;
} CSOSD_Rect;

typedef enum 
{ 
	OSD_NO_ERROR = 0, 
	OSD_ERROR_OPEN_FAILED,		/* open filed                 */
	OSD_ERROR_IOCTL_FAILED,		/* ioctl filed                */
	OSD_ERROR_INVALID_PARAMETERS,	/* Bad parameter passed       */
	OSD_ERROR_UNKNOWN_DEVICE,	/* Unknown device name        */
	OSD_ERROR_DEVICE_BUSY,		/* Device is currently busy   */
	OSD_ERROR_INVALID_HANDLE,	/* Handle is not valid        */
	OSD_ERROR_ALREADY_INITIALIZED,	/* Device already initialized */
	OSD_ERROR_NOT_INITIALIZED,	/* Device not initialized     */
	OSD_ERROR_OPERATION_ERROR	/* Function invoke error */
} CSOSD_ErrCode;

CSOSD_HANDLE CSOSD_Open(CSOSD_LAYER layer);
CSAPI_RESULT CSOSD_Close(CSOSD_HANDLE handle);

CSAPI_RESULT CSOSD_Enable(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_Disable(CSOSD_HANDLE handle);

CSAPI_RESULT CSOSD_SetConfig(CSOSD_HANDLE handle, CSOSD_Config * config);
CSAPI_RESULT CSOSD_GetConfig(CSOSD_HANDLE handle, CSOSD_Config * config);

CSAPI_RESULT CSOSD_GetBaseAddr(CSOSD_HANDLE handle, unsigned char ** addr);

CSAPI_RESULT CSOSD_SetAlpha(CSOSD_HANDLE handle, int alpha);
CSAPI_RESULT CSOSD_GetAlpha(CSOSD_HANDLE handle, int * alpha);

CSAPI_RESULT CSOSD_EnableKeyColor(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_DisableKeyColor(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_SetKeyColor(CSOSD_HANDLE handle, CSOSD_KeyColor * key_color);
CSAPI_RESULT CSOSD_GetKeyColor(CSOSD_HANDLE handle, CSOSD_KeyColor * key_color);

CSAPI_RESULT CSOSD_Flip(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_FlipRect(CSOSD_HANDLE handle, 
			CSOSD_Rect *src_rect, CSOSD_Rect *dst_rect);

CSAPI_RESULT CSOSD_WaitVSync(CSOSD_HANDLE handle);

CSOSD_ErrCode CSOSD_GetErrCode(CSOSD_HANDLE handle);
char *CSOSD_GetErrString(CSOSD_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif

