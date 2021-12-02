#ifndef _CS_SUBTITLE_H_
#define _CS_SUBTITLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "csttxdraw.h"

typedef enum
{
	CS_SUB_DEFAULT_MODE=0,
	CS_SUB_720_MODE=1,
	CS_SUB_1080_MODE=2
}tSUBDixPlayMode;

typedef enum
{
	CS_SUB_COLOR_16BITS=0,
	CS_SUB_COLOR_32BITS=1,
}tSUBColorMode;

typedef struct SUBInit_Params_S
{
	CSDEMUX_HANDLE			DMXHandle;
	CSDEMUX_HANDLE			BlitHandle;
	CSDEMUX_HANDLE			DISP_handle;
	tTTXDixPlayMode			Displaymode;
	tTTXColorMode			Colormode;
}SUBInit_Params;

typedef struct SubDisplayBitmap_S
{
	U32 		x;
	U32		y;
	BITMAP	SubBitmap;
	U8		Update;

} SubDisplayBitmap;


typedef void (* Sub_NotificationCallBack)(U32 pBitmap,U32 lpara);


int InitialSubtitle(SUBInit_Params Initpara);
void OpenSubtitle(U16 Pid,Sub_NotificationCallBack NotifyFunction);
void CloseSubtitle(void);
CSDEMUX_HANDLE GetSubtitleDmxHandle(void);

#ifdef __cplusplus
}
#endif

#endif
