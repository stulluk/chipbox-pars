#ifndef __CSAPI_VID_H__
#define __CSAPI_VID_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CSVID_HANDLE;

typedef enum
{
	VID_DEV_0 = 0,
	VID_DEV_1 	
} CSVID_DEV;

typedef enum
{
	VID_INPUT_DEMUX0 = 0,
	VID_INPUT_STILLPIC
} CSVID_INPUT;

typedef enum
{
	VID_STREAM_TYPE_MPEG1_TS = 0,
	VID_STREAM_TYPE_MPEG2_TS,
	VID_STREAM_TYPE_H264_TS, 
	VID_STREAM_TYPE_VIB
} CSVID_STREAM_TYPE;

typedef enum
{
	VID_FRAME_ANY = 0, 	/* all frames */
	VID_FRAME_IP, 		/* I- and P-frames only*/
	VID_FRAME_I, 		/* I-frames only */
	VID_FRAME_SP
} CSVID_DECODING_MOD;

typedef enum
{
	VID_BUFFER_EMPTY = 0,
	VID_BUFFER_FULL = 1,
	VID_BUFFER_NORMAL = 2
} CSVID_BUFFER_STATUS;

typedef struct tagVID_SequenceHeader
{
	unsigned int w; 		/* width */
	unsigned int h; 		/* height */
	unsigned int frame_rate; 	/* in frames per 1000sec */
} CSVID_SequenceHeader;

typedef enum 
{
	VID_STATUS_STOPPED = 0,	
	VID_STATUS_RUNNING, 	
	VID_STATUS_PAUSE,
	VID_STATUS_FROZEN
}CSVID_STATUS ;

typedef struct tagVID_Rect
{
	int left;
	int right;
	int top;
	int bottom;
} CSVID_Rect;

typedef union
{
	struct {
		unsigned int pictures: 6;
		unsigned int seconds: 6;
		unsigned int marker_bit: 1;
                unsigned int minutes: 6;
		unsigned int hours: 5;
		unsigned int drop_frame_flag: 1;
                unsigned int reserved: 7;
    	}bits;

	unsigned int val;
}CSVID_TimeCode ;

typedef enum{
    VID_LEVEL0 = 0,
    VID_LEVEL1,
    VID_LEVEL2,
    VID_LEVEL3,
    VID_LEVEL4,
    VID_LEVEL5,
    VID_LEVEL6,
    VID_LEVEL7,
}CSVID_ERROR_THRESHOLD;

typedef enum {
CSVID_UNKNOWN = -1,
/* mpeg2 */
CSVID_4TO3 = 0,
CSVID_16TO9,
/* h.264 */
CSVID_1TO1,
CSVID_12TO11,
CSVID_10TO11,
CSVID_16TO11,
CSVID_40TO33,
CSVID_24TO11,
CSVID_20TO11,
CSVID_32TO11,
CSVID_80TO33,
CSVID_18TO11,
CSVID_15TO11,
CSVID_64TO33,
CSVID_160TO99,
/* mpeg1 */
CSVID_1_0,
CSVID_0_6735,
CSVID_0_7031,
CSVID_0_7615,
CSVID_0_8055,
CSVID_0_8437,
CSVID_0_8935,
CSVID_0_9157,
CSVID_0_9815,
CSVID_1_0255,
CSVID_1_0695,
CSVID_1_0950,
CSVID_1_1575,
CSVID_1_2015
}CSVID_ASPECTRATIO;

typedef enum 
{ 
	VID_NO_ERROR = 0, 
	VID_ERROR_OPEN_FAILED,		/* open filed                 */
	VID_ERROR_IOCTL_FAILED,		/* ioctl filed                */
	VID_ERROR_INVALID_PARAMETERS,	/* Bad parameter passed       */
	VID_ERROR_UNKNOWN_DEVICE,	/* Unknown device name        */
	VID_ERROR_DEVICE_BUSY,		/* Device is currently busy   */
	VID_ERROR_INVALID_HANDLE,	/* Handle is not valid        */
	VID_ERROR_ALREADY_INITIALIZED,	/* Device already initialized */
	VID_ERROR_NOT_INITIALIZED,	/* Device not initialized     */
	VID_ERROR_INVALID_STATUS,	/* invalid status	      */
	VID_ERROR_WRITE_DATA,	/* write data to demux CPB buffer error	 */
	VID_ERROR_PLAY_STILLPICTURE,	/* play still picture error	*/
	VID_ERROR_MEMORY_MAP    /*mempry map failed*/
} CSVID_ErrCode;

CSVID_HANDLE  CSVID_Open(CSVID_DEV dev);
CSAPI_RESULT  CSVID_Close(CSVID_HANDLE handle);

CSAPI_RESULT  CSVID_Play(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Stop(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Pause(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Freeze(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Resume(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Step(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_Skip(CSVID_HANDLE handle, unsigned int tm_len);

CSAPI_RESULT  CSVID_SetInputMode(CSVID_HANDLE handle, CSVID_INPUT input_mod);
CSAPI_RESULT  CSVID_GetInputMode(CSVID_HANDLE handle, CSVID_INPUT* input_mod);

CSAPI_RESULT  CSVID_SetStreamType(CSVID_HANDLE handle, CSVID_STREAM_TYPE stream_type);
CSAPI_RESULT  CSVID_GetStreamType(CSVID_HANDLE handle, CSVID_STREAM_TYPE* stream_type);

CSAPI_RESULT  CSVID_SetDecoderMode(CSVID_HANDLE handle, CSVID_DECODING_MOD decoding_mod);
CSAPI_RESULT  CSVID_GetDecoderMode(CSVID_HANDLE handle, CSVID_DECODING_MOD* decoding_mod);

CSAPI_RESULT  CSVID_SetPlaySpeed(CSVID_HANDLE handle, unsigned int speed);
CSAPI_RESULT  CSVID_GetPlaySpeed(CSVID_HANDLE handle, unsigned int* speed);

CSAPI_RESULT  CSVID_SetFrameRate(CSVID_HANDLE handle, int frame_rate);
CSAPI_RESULT  CSVID_GetFrameRate(CSVID_HANDLE handle, int *frame_rate);

CSAPI_RESULT  CSVID_GetSequenceHeader(CSVID_HANDLE handle, CSVID_SequenceHeader *hdr);
#ifdef ARCH_CSM1201
CSAPI_RESULT CSVID_GetSRCSequenceHeader(CSVID_HANDLE handle, CSVID_SequenceHeader * hdr);
#endif

CSAPI_RESULT  CSVID_EnablePTSSync(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_DisablePTSSync(CSVID_HANDLE handle);

CSAPI_RESULT CSVID_GetPTS(CSVID_HANDLE handle, long long *vid_pts);

CSAPI_RESULT  CSVID_EnableOutput(CSVID_HANDLE handle);
CSAPI_RESULT  CSVID_DisableOutput(CSVID_HANDLE handle);

CSAPI_RESULT  CSVID_SetOutputPostion(CSVID_HANDLE handle, const CSVID_Rect* const src, const CSVID_Rect* const dst);
CSAPI_RESULT  CSVID_SetOutputAlpha(CSVID_HANDLE handle, unsigned int alpha);

CSAPI_RESULT CSVID_TimeCodeReportNotify(CSVID_HANDLE handle,void (*call_back_function)(CSVID_HANDLE *),CSVID_TimeCode timecode,int event_enable);
CSAPI_RESULT CSVID_GetTimeCode(CSVID_HANDLE handle, CSVID_TimeCode *timecode);

CSAPI_RESULT CSVID_GetSourceRectangle(CSVID_HANDLE handle,int * width,int * height);

CSAPI_RESULT CSVID_ErrNotify(CSVID_HANDLE handle,void (*call_back_function)(CSVID_HANDLE*,CSVID_ERROR_THRESHOLD),CSVID_ERROR_THRESHOLD error_threshold,int event_enable ,unsigned int timeout_value);

CSAPI_RESULT CSVID_GetAspectRatio(CSVID_HANDLE handle, CSVID_ASPECTRATIO * aspect_ratio);
CSAPI_RESULT CSVID_AspectRatioChangeNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, CSVID_ASPECTRATIO *),int event_enable);
CSAPI_RESULT CSVID_GetPScanCrop(CSVID_HANDLE handle, CSVID_Rect *rect);
CSAPI_RESULT CSVID_PScanCropNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, CSVID_Rect *),int event_enable );
CSAPI_RESULT CSVID_SyncNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, signed char *),unsigned int timeout_value, int event_enable );

CSVID_ErrCode CSVID_GetErrCode(CSVID_HANDLE handle);
char*         CSVID_GetErrString(CSVID_HANDLE handle);

CSAPI_RESULT CSVID_PFMOpen(CSVID_HANDLE handle);
CSAPI_RESULT CSVID_PFMClose(CSVID_HANDLE handle);
CSAPI_RESULT CSVID_PMFReset(CSVID_HANDLE handle);
CSAPI_RESULT CSVID_GetPFMBufferSize(CSVID_HANDLE handle, unsigned int * bufsize);
CSAPI_RESULT CSVID_WritePFMData(CSVID_HANDLE handle, unsigned char *src, int size);
CSAPI_RESULT CSVID_SetNotifyPFMDataEmpty(CSVID_HANDLE handle, unsigned int *empty_threshold, void (* call_back_function)(CSVID_HANDLE *), unsigned char event_enable);
CSAPI_RESULT CSVID_SetNotifyPFMDataFull(CSVID_HANDLE handle, unsigned int *full_threshold,void (*call_back_function) (CSVID_HANDLE *), unsigned char event_enable);
CSAPI_RESULT CSVID_WritePFMDataWithPTS(CSVID_HANDLE handle, unsigned char *src, int size,  unsigned long long pts);

CSAPI_RESULT CSVID_Force3to2PollDown(CSVID_HANDLE handle, unsigned int enable_flag);
CSAPI_RESULT CSVID_WaitSync(CSVID_HANDLE handle, unsigned int enable_flag);

CSAPI_RESULT CSVID_StartDelay(CSVID_HANDLE handle, unsigned int startdelay_ms,unsigned int delay_flag);
/*
about parameters of the userdata call_back_function.
parameter 2 : userdata pointer. 
parameter 3 : number of the total userdata.
parameter 4 : number of the one userdata block.

notice : the first 32Byte userdata is a private struct. there are some differents between mpeg2 and h264. After this is the real userdata in the video stream.
user_data[0] = user data byte size // user data byte size specifies the number of remaining bytes int the real userdata. Reserved in mpeg2.
user_data[1] = user data type // Reserved in mpeg2
user_data[2] = ((U32)country_code<<16) | ((U32)country_code_extension&0xFF); // Reserved in mpeg2
user_data[3] = pts
user_data[4] = 0; // Reserved
user_data[5] = 0; // Reserved
user_data[6] = 0; // Reserved
user_data[7] = 0; // Reserved
*/
CSAPI_RESULT CSVID_UserDataNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, unsigned char *, int, int), int event_enable );

/* errorskipmode: 1 means video decoder will skip error frame output; 0 means video decoder will output all frame include error frame*/
CSAPI_RESULT CSVID_SetErrorSkipMode(CSVID_HANDLE handle, unsigned int errorskipmode);

CSAPI_RESULT CSVID_EnableTrickMode(CSVID_HANDLE handle);
CSAPI_RESULT CSVID_DisableTrickMode(CSVID_HANDLE handle);

CSAPI_RESULT CSVID_GetBufferStatus(CSVID_HANDLE handle, CSVID_BUFFER_STATUS *buffer_status);
CSAPI_RESULT CSVID_DataUnderflowNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *),int event_enable,unsigned int timeout_value);
CSAPI_RESULT CSVID_FormatChangeNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *, CSVID_SequenceHeader *),
	                               int event_enable);
CSAPI_RESULT CSVID_SRCFormatChangeNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *,CSVID_SequenceHeader *),
													int event_enable);

#define VIB_SUPPORT
#ifdef VIB_SUPPORT
union vib_para{
	struct{
		unsigned int para_enable:1;	/*1:para enable. 0:para disable*/
		unsigned int work_mode:1;		/*0:Auto detect input video source format; 1: configure video format by src_width and src_height*/
		unsigned int yuv_order:2;
		unsigned int src_height:12;		/*Video Source Height in pels*/
		unsigned int src_width:12;		/*Video Source Width in pels*/
		unsigned int src_format:1;		/*1:Video Source is Progressive(Frame). 0:Video Source is Interlaced (Field)*/
		unsigned int yc:1;				/*1:Y C Parallel(16 bit); 0:Y C Interleaved(8 bit)*/
		unsigned int uv_drop_mode:2;	/**/
	}bits;

	unsigned int val;
};

CSAPI_RESULT CSVID_VIB_Config(CSVID_HANDLE handle,union vib_para para);
CSAPI_RESULT CSVID_VIB_Reset(CSVID_HANDLE handle);
#endif

#ifdef __cplusplus
}
#endif

#endif
