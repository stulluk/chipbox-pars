
#ifndef __CSAPI_OSD_H__
#define __CSAPI_OSD_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif


#define OBJECTS_NUMBER		80		/* user can modify this value */


typedef void *CSOSD_HANDLE;

typedef unsigned int PIC_ID;
typedef CSAPI_RESULT (*EXTRA_FUNC) (CSOSD_HANDLE handle);

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

typedef enum 
{
	OSD_DISTANCE = 3,
	OSD_COORDINATE
} CSOSD_MOVE_MODE;

typedef enum 
{
	OSD_LEFT = 3,
	OSD_RIGHT,
	OSD_UP,
	OSD_DOWN
} CSOSD_ROLL_DIRECTION;

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

typedef struct tagOSD_REC_Region {
	unsigned int x;			/* X coordinate of its top-left point */
	unsigned int y;			/* Y coordinate of its top-left point */
	unsigned int width;	/* width of it */
	unsigned int height;	/* height of it */
} CSOSD_REC_Region;

typedef struct tagOSD_TRI_Region {
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
	unsigned int x3;
	unsigned int y3;
} CSOSD_TRI_Region;

typedef struct tagOSD_LINE_Region {
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
} CSOSD_LINE_Region;

typedef struct tagOSD_Color {
	unsigned char r;		/* red channel */
	unsigned char g;		/* green channel */
	unsigned char b;		/* blue channel */
	unsigned char a;		/* alpha channel */
} CSOSD_Color;

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
	OSD_ERROR_OPERATION_ERROR,	/* Function invoke error */
	OSD_ERROR_DFB_INIT_ERROR,
	OSD_ERROR_DFB_GET_LAYER,
	OSD_ERROR_DFB_CREATE_SURFACE,
	OSD_ERROR_DFB_FLIP,
	OSD_ERROR_DFB_CREATE_WINDOW,
	OSD_ERROR_DFB_GET_SURFACE,
	OSD_ERROR_DFB_CREATE_IMAGE,
	OSD_ERROR_DFB_SHOW_IMAGE,
	OSD_ERROR_UNUSED_WINDOW,
	OSD_ERROR_DFB_IMAGE_ID_ERROR,
	OSD_ERROR_DFB_SET_OPACITY_WINDOW,
	OSD_ERROR_DFB_BLIT_ERROR,
	OSD_ERROR_DFB_MOVE_ERROR,
	OSD_ERROR_DFB_MALLOC_ERROR,
	OSD_ERROR_DFB_PTHREAD_ERROR,
	OSD_ERROR_DFB_CLEAR_ERROR,
	OSD_ERROR_RAISE_TO_TOP_ERROR,
	OSD_ERROR_DFB_RENDERTO_ERROR,
	OSD_ERROR_DFB_SET_BACKGROUND_ERROR,
	OSD_ERROR_FILL_REC_TANGLE,
	OSD_ERROR_FILL_TRI_ANGLE,
	OSD_ERROR_DRAW_LINE,
	OSD_ERROR_DRAW_RECTANGLE,
	OSD_ERROR_FILL_SPANS
} CSOSD_ErrCode;

CSOSD_HANDLE CSOSD_Open(CSOSD_LAYER layer);
CSAPI_RESULT CSOSD_OpenDFB(CSOSD_LAYER layer);
CSAPI_RESULT CSOSD_Close(CSOSD_HANDLE handle);

CSAPI_RESULT CSOSD_Enable(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_Disable(CSOSD_HANDLE handle);

CSAPI_RESULT CSOSD_SetConfig(CSOSD_HANDLE handle, CSOSD_Config * config);
CSAPI_RESULT CSOSD_GetConfig(CSOSD_HANDLE handle, CSOSD_Config * config);

CSAPI_RESULT CSOSD_GetBaseAddr(CSOSD_HANDLE handle, unsigned char ** addr);

/* Full Screen Set Alpha */
CSAPI_RESULT CSOSD_SetAlpha(CSOSD_HANDLE handle, int alpha);
//CSAPI_RESULT CSOSD_GetAlpha(CSOSD_HANDLE handle, int * alpha);

/* Full Screen KeyColor */
CSAPI_RESULT CSOSD_EnableKeyColor(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_DisableKeyColor(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_SetKeyColor(CSOSD_HANDLE handle, CSOSD_KeyColor * key_color);
//CSAPI_RESULT CSOSD_GetKeyColor(CSOSD_HANDLE handle, CSOSD_KeyColor * key_color);

CSAPI_RESULT CSOSD_Flip(CSOSD_HANDLE handle);
CSAPI_RESULT CSOSD_FlipRect(CSOSD_HANDLE handle, CSOSD_Rect *src_rect, CSOSD_Rect *dst_rect);

//CSAPI_RESULT CSOSD_WaitVSync(CSOSD_HANDLE handle);



/*************************************************************************************
Notice:
			If you use the following functions, please setup 
			"-L open_sources/libs  -ldirectfb -lpthread -ldl -lz -ldirect -lfusion -lm" in your app makefile !!
			
**************************************************************************************/	

CSAPI_RESULT CSOSD_GetSize(CSOSD_HANDLE handle, unsigned int *width, unsigned int *height);

//CSAPI_RESULT CSOSD_SetBackgroundImage(CSOSD_HANDLE handle, unsigned char *file_path);
CSAPI_RESULT CSOSD_SetBackgroundColor(CSOSD_HANDLE handle, CSOSD_Color *color);

/* 
	color->a: 0x0-full_transparency  0xff-full_opacity
*/
CSAPI_RESULT CSOSD_FillRectangle(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																			CSOSD_Color *color, PIC_ID *id);

CSAPI_RESULT CSOSD_FillTriangle(CSOSD_HANDLE handle, CSOSD_TRI_Region *region, 
																		CSOSD_Color *color, PIC_ID *id);

CSAPI_RESULT CSOSD_DrawLine(CSOSD_HANDLE handle, CSOSD_LINE_Region *region, 
																	CSOSD_Color *color, PIC_ID *id);

//CSAPI_RESULT CSOSD_DrawLines(CSOSD_HANDLE handle, CSOSD_LINE_Region **region, 
//																		unsigned int lines_number, 	CSOSD_Color *color);

//CSAPI_RESULT CSOSD_DrawHorizonDottedLine(CSOSD_HANDLE handle, CSOSD_LINE_Region *region, 
//																							unsigned int realline_width, unsigned int dottedline_width, 
//																							CSOSD_Color *color, PIC_ID *id);

CSAPI_RESULT CSOSD_DrawRectangle(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																					CSOSD_Color *color, PIC_ID *id);

/* 
	It is usually used in initialization process, for it would take a little time. 
	If region's members(x,y,width,height) are all set to 0, it means fullscreen.
	
	Supported image type: jpeg, gif, png, if bmp, use  CSOSD_OpenImageBuffer.
*/
CSAPI_RESULT CSOSD_OpenImage(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																			unsigned char *path, PIC_ID *id);

/* 
	It is usually used in initialization process, for it would take a little time. 
	If region's members(x,y,width,height) are all set to 0, it means fullscreen.
	Only support RGB565 for now.
*/
CSAPI_RESULT CSOSD_OpenImageBuffer(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																				CSOSD_COLOR_FORMAT format, unsigned char *data, PIC_ID *id);

CSAPI_RESULT CSOSD_ShowImage(CSOSD_HANDLE handle, PIC_ID id);

/*
	Display pic in the rolling way.
	If you want pic to roll into screen from outside, please use CSOSD_ObjectMove repeatly.

	direction: the pic display from this direction, only support OSD_DOWN for now.
	degree: the large this value is, the smoother pic display, and the slower pic display.
	func: If your directfb points to /dev/fb0 or /dev/fb1 directly, please set NULL to func, else set func with CSOSD_Flip;
*/
CSAPI_RESULT CSOSD_ShowImageRoll(CSOSD_HANDLE handle, PIC_ID id, CSOSD_ROLL_DIRECTION direction,
																					unsigned int degree,  EXTRA_FUNC func);

/* alpha: 0x0-full_transparency 0xff-full_opacity */
CSAPI_RESULT CSOSD_SetObjectAlpha(CSOSD_HANDLE handle, PIC_ID id, unsigned char alpha);

/* color->a parameter invalid*/
CSAPI_RESULT CSOSD_ObjectKeyColor(CSOSD_HANDLE handle, PIC_ID id, CSOSD_Color *color);

CSAPI_RESULT CSOSD_ObjectRaiseToTop(CSOSD_HANDLE handle, PIC_ID id);

/*
	The speed of the operation is very high.
	mode: 
				OSD_DISTANCE - (dx,dy) is relative distance, "+" towards right or down, "-" towards left or up
				OSD_COORDINATE - (dx,dy) is absolute coordinates, object will be moved to this place.
*/
CSAPI_RESULT CSOSD_ObjectMove(CSOSD_HANDLE handle, int dx, int dy, 
																		CSOSD_MOVE_MODE mode, PIC_ID id);

/*
	Special Effects: fade-in fade-out, if you don't want any more, CSOSD_ObjectFadeInOut_Stop first, then clear.
	
	func: If your directfb points to /dev/fb0 or /dev/fb1 directly, please set NULL to func, else set func with CSOSD_Flip;
*/
CSAPI_RESULT CSOSD_ObjectFadeInOut(CSOSD_HANDLE handle, PIC_ID id, EXTRA_FUNC func);

CSAPI_RESULT CSOSD_ObjectFadeInOut_Stop(CSOSD_HANDLE handle, PIC_ID id);

/* To save the limited resource, please invoke this function when the objects(eg:pic,line) is no using */
CSAPI_RESULT CSOSD_ObjectClear(CSOSD_HANDLE handle, PIC_ID id);
CSAPI_RESULT CSOSD_ObjectClearAll(CSOSD_HANDLE handle);



CSOSD_ErrCode CSOSD_GetErrCode(CSOSD_HANDLE handle);
char *CSOSD_GetErrString(CSOSD_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif

