#ifndef __AV_ZAPPING_H
#define __AV_ZAPPING_H

#include "csapi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef  enum
{
	eCS_AV_OK,
	eCS_AV_ERROR
}tCS_AV_Error;

typedef enum
{
    eCS_AV_AUDIO_STREAM_MPEG2,
    eCS_AV_AUDIO_STREAM_AC3,
    /* By KB Kim for DTS Audio */
    eCS_AV_AUDIO_STREAM_DTS,
    /* Add By River 06.12.2008 */
    eCS_AV_AUDIO_STREAM_AAC,
    eCS_AV_AUDIO_STREAM_LATM,
    eCS_AV_AUDIO_STREAM_MP3

}tCS_AV_AudioType;

typedef enum
{
    eCS_AV_VIDEO_STREAM_MPEG2,
    eCS_AV_VIDEO_STREAM_H264,
    eCS_AV_VIDEO_STREAM_MPEG4,
    eCS_AV_VIDEO_STREAM_VC1,
    eCS_AV_VIDEO_STREAM_AVS

}tCS_AV_VideoType;

typedef enum
{
    eCS_AV_STEREO_MODE_STEREO,
    eCS_AV_STEREO_MODE_LEFT,
    eCS_AV_STEREO_MODE_RIGHT,

}tCS_AV_StereoMode;

typedef enum
{
    eCS_AV_VIDEO_ASPECT_4_3,
    eCS_AV_VIDEO_ASPECT_16_9,
    eCS_AV_VIDEO_ASPECT_AUTO
}tCS_AV_VideoAspect;

typedef enum
{
	eCS_AV_ASPECT_COMBINED = 0,
	eCS_AV_ASPECT_LETTER_BOX,
	eCS_AV_ASPECT_PANSCAN
}tCS_AV_AspectRatioMode;


typedef enum
{
    eCS_AV_VIDEO_FORMAT_UNKNOWN,
    eCS_AV_VIDEO_FORMAT_NTSC,        /* NTSC 480I 60Hz */
    eCS_AV_VIDEO_FORMAT_PAL,         /* PAL 576I 50Hz*/
    eCS_AV_VIDEO_FORMAT_480P60,      /* 480p 60HZ*/
    eCS_AV_VIDEO_FORMAT_576P50,      /* 576P 50Hz */
    eCS_AV_VIDEO_FORMAT_720P50,	    /* 720P 50Hz */
    eCS_AV_VIDEO_FORMAT_720P60,      /* 720P 60Hz */
    eCS_AV_VIDEO_FORMAT_1080I25,     /* 1080I 50HZ*/
    eCS_AV_VIDEO_FORMAT_1080I30,     /* 1080I 60HZ*/
    eCS_AV_VIDEO_FORMAT_AUTO

}tCS_AV_VideoDefinition;

typedef struct
{

    U16 Width;
    U16 Height;
    U8  FrameRate;
    tCS_AV_VideoDefinition Video_Definition;

}tCS_AV_VideoOriginalInfo;


typedef struct
{
	U8	Audio_Volume;
    U16 Video_PID;
    U16 Audio_PID;
    U16 PCR_PID;
    tCS_AV_VideoType        VideoType;
    tCS_AV_AudioType        AudioType;
    tCS_AV_StereoMode     AudioMode;
}tCS_AV_PlayParams;

typedef struct
{
        tCS_AV_VideoAspect              Video_Aspect;
        tCS_AV_AspectRatioMode      Video_Ratio;
        tCS_AV_VideoDefinition           Video_Format;
}tCS_AV_VideoFormat;

typedef struct
{
	U16 x;
	U16 y;
	U16 w;
	U16 h;
}tCS_AV_VideoRect;


/*typedef struct
{
         U8               Set_Status;
}tCS_AV_Msg_t;*/

BOOL CS_AV_Init2(void);
BOOL CS_AV_Init(void);
tCS_AV_Error CS_AV_Close(void);
tCS_AV_Error CS_Video_Close(void);
tCS_AV_Error CS_Audio_Close(void);
tCS_AV_Error CS_TVE_Close(void);
tCS_AV_Error CS_OSD_Close(void);
tCS_AV_Error CS_AV_Audio_SetMuteStatus( BOOL enable );
BOOL CS_AV_Audio_GetMuteStatus(void);
tCS_AV_Error CS_AV_AudioSetVolume( U8 Volume );
U8 CS_AV_AudioGetVolume(void);
tCS_AV_Error CS_AV_AudioSetStereoMode( tCS_AV_StereoMode StereoMode );
tCS_AV_Error CS_AV_VideoFreeze(void);
tCS_AV_Error CS_AV_VideoUnfreeze(void);
tCS_AV_Error CS_AV_AudioFreeze(void);
tCS_AV_Error CS_AV_AudioUnfreeze(void);
tCS_AV_Error CS_AV_VideoBlank(void);
tCS_AV_Error CS_AV_VideoUnblank(void);
tCS_AV_Error CS_AV_SetOSDAlpha( int Alpha );
tCS_AV_Error CS_AV_GetVideoOriginalInfo( tCS_AV_VideoOriginalInfo* pOri_Size );
tCS_AV_Error CS_AV_VideoScalor( tCS_AV_VideoRect * vid_rect);
/* By KB Kim 2010.08.31 for RGB Control */
tCS_AV_Error CS_AV_SetTVOutput( void );
tCS_AV_Error CS_AV_SetTVOutDefinition( tCS_AV_VideoDefinition definition )	;
BOOL CS_AV_VideoSwitchCVBS( BOOL CVBSEnable );
tCS_AV_Error CS_AV_ProgramStop(void);
tCS_AV_Error Mv_VideoRestart(tCS_AV_PlayParams ProgramInfo);
tCS_AV_Error CS_AV_ProgramPlay( tCS_AV_PlayParams ProgramInfo );
tCS_AV_Error CS_AV_ChangeAudioPid( U16 AudioPID, tCS_AV_AudioType AudioType );

tCS_AV_Error CS_AV_EnableTVOut(void);
tCS_AV_Error CS_AV_DisableTVOut(void);

tCS_AV_Error CS_AV_Play_IFrame(const char* file_path);
tCS_AV_Error CS_AV_Play_IFrame2(const char* file_path);
tCS_AV_Error CS_AV_VID_GetPTS(long long * VideoPts);

BOOL CS_AV_SetVBIPage(CSVOUT_TxtPage_t *pPage);
BOOL CS_AV_SetVBI_Ctrl(char Enable);
BOOL CS_AV_SetVBIFmt(CSVOUT_TxtStandard_t TxtStd);
void ClearOSD(void);

tCS_AV_Error CS_AV_VID_SetContrast(unsigned int contrast);
tCS_AV_Error CS_AV_VID_GetContrast(unsigned int *contrast);
tCS_AV_Error CS_AV_VID_SetBrightness(unsigned int brightness);
tCS_AV_Error CS_AV_VID_GetBrightness(unsigned int *brightness);
tCS_AV_Error CS_AV_VID_SetSaturation(unsigned int saturation);
tCS_AV_Error CS_AV_VID_GetSaturation(unsigned int *saturation);

CSVID_SequenceHeader MV_Get_Seq_Header(void);

#ifdef __cplusplus
}
#endif

#endif


