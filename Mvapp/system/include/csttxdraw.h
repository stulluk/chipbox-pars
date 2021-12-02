#ifndef _CS_TTX_DRAW_H_
#define _CS_TTX_DRAW_H_

#ifdef __cplusplus
extern "C" {
#endif



//ttx draw task
#define CSTTX_DRAW_TASK_STACK_SIZE      1024*3
#define CSTTX_DRAW_TASK_PRIORITY		  15

#define   CSTTX_MAXPAGEHISTORY     64

//#define 	INT32		int

enum
{
	CS_TTX_SUCESS=0,
	CS_TTX_FAILURE=-1,
	CS_TTX_BAD_PARA=-2,
	CS_TTX_NO_MEMRY=-5,
};

enum
{
	CS_TTX_KEY_MSG_0=0x10,
	CS_TTX_KEY_MSG_1=0x11,
	CS_TTX_KEY_MSG_2=0x12,
	CS_TTX_KEY_MSG_3=0x13,
	CS_TTX_KEY_MSG_4=0x14,
	CS_TTX_KEY_MSG_5=0x15,
	CS_TTX_KEY_MSG_6=0x16,
	CS_TTX_KEY_MSG_7=0x17,
	CS_TTX_KEY_MSG_8=0x18,
	CS_TTX_KEY_MSG_9=0x19,

	CS_TTX_KEY_MSG_OK=0x20,
	CS_TTX_KEY_MSG_PAGE_UP=0x21,
	CS_TTX_KEY_MSG_PAGE_DOWN=0x22,
	CS_TTX_KEY_MSG_SUBPAGE_UP=0x23,
	CS_TTX_KEY_MSG_SUBPAGE_DOWN=0x24,
	CS_TTX_KEY_MSG_PAGE10_UP=0x25,
	CS_TTX_KEY_MSG_PAGE10_DOWN=0x26,

	CS_TTX_KEY_MSG_RED=0x51,
	CS_TTX_KEY_MSG_BLUE=0x52,
	CS_TTX_KEY_MSG_GREEN=0x53,
	CS_TTX_KEY_MSG_YELLOW=0x54,
};

typedef enum
{
	CS_TTX_DEFAULT_MODE=0,
	CS_TTX_720_MODE=1,
	CS_TTX_1080_MODE=2
}tTTXDixPlayMode;

typedef enum
{
	CS_TTX_COLOR_16BITS=0,
	CS_TTX_COLOR_32BITS=1,
}tTTXColorMode;

typedef struct TTXInit_Params_S
{
	CSDEMUX_HANDLE			DMXHandle;
	CSDEMUX_HANDLE			BlitHandle;
	CSDEMUX_HANDLE			DISP_handle;
	CSOS_MessageQueue_t*	pMessage;
	tTTXDixPlayMode			Displaymode;
	tTTXColorMode			Colormode;
}TTXInit_Params;

typedef struct TTXInputNumber_S
{
	U8		String[3];
	U16		PageHex;
	U8		Position;
}TTXInputNumber;

typedef struct TTXHistoryPage_S
{
	U16		PageHistoryHead;
	U16		PageHistory[CSTTX_MAXPAGEHISTORY];
}TTXHistoryPage;

#ifdef NEW_GUI
struct _BITMAP
{
    /**
     * Bitmap types, can be OR'ed by the following values:
     *  - BMP_TYPE_NORMAL\n
     *    A nomal bitmap, without alpha and color key.
     *  - BMP_TYPE_RLE\n
     *    A RLE encoded bitmap, not used so far.
     *  - BMP_TYPE_ALPHA\n
     *    Per-pixel alpha in the bitmap.
     *  - BMP_TYPE_ALPHACHANNEL\n
     *    The \a bmAlpha is a valid alpha channel value.
     *  - BMP_TYPE_COLORKEY\n
     *    The \a bmColorKey is a valid color key value.
     *  - BMP_TYPE_PRIV_PIXEL\n
     *    The bitmap have a private pixel format.
     */
    U8   bmType;
    /** The bits per piexel. */
    U8   bmBitsPerPixel;
    /** The bytes per piexel. */
    U8   bmBytesPerPixel;
    /** The alpha channel value. */
    U8   bmAlpha;
    /** The color key value. */
    U32  bmColorKey;

    /** The width of the bitmap */
    U32  bmWidth;
    /** The height of the bitmap */
    U32  bmHeight;
    /** The pitch of the bitmap */
    U32  bmPitch;
    /** The bits of the bitmap */
    U8*  bmBits;

    /** The private pixel format */
    void*   bmAlphaPixelFormat;
};

typedef struct _BITMAP BITMAP;
#endif



typedef struct TTXDisplayBitmap_S
{
	U32 		x;
	U32		y;
	BITMAP	TtxBitmap;
	U8		Update;

} TTXDisplayBitmap;
	

typedef enum
{	
	CS_TTX_DEFAULT=0,
	CS_TTX_SUBTITLE=1,
	CS_TTX_VBI=2,
}TTXMode;

typedef void ( * TTX_NotificationCallBack)(U32 pBitmap,U32 lpara);

INT32 InitialTeletext(TTXInit_Params Params);
INT32 CreateTeletext(U16 Ttxpid,U16 PageHex,TTXMode Mode,U8 *Lang,TTX_NotificationCallBack NotifyFunction);
INT32 DestroyTeletext(void);
INT32 PauseTeletext(BOOL cmd);
CSDEMUX_HANDLE GetTeletextDmxHandle(void);

#ifdef __cplusplus
}
#endif

#endif
