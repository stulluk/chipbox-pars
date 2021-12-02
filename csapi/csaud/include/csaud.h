#ifndef  __CSAPI_AUD_H__
#define  __CSAPI_AUD_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
 
typedef void *CSAUD_HANDLE;

typedef enum 
{
	AUD_DEV_0 = 1,
	AUD_DEV_1
} CSAUD_DEV;

typedef enum 
{ 
	AUD_OUTPUT_I2S = 0, 
	AUD_OUTPUT_SPDIFAC3,
	AUD_OUTPUT_SPDIFAAC,
	AUD_OUTPUT_SPDIFPCM,    /* not supported */
	AUD_OUTPUT_I2S_SPDIFAC3,/* not supported */
        AUD_OUTPUT_I2S_SPDIFAAC,/* not supported */
        AUD_OUTPUT_I2S_SPDIFPCM	
} CSAUD_OUTPUT;

typedef enum 
{ 
	AUD_PCM_LEFT_MONO = 0, 
	AUD_PCM_RIGHT_MONO, 
	AUD_PCM_STEREO, 
	AUD_PCM_CHANNEL51,
	AUD_PCM_INVALID
} CSAUD_PCM_CHANNEL;

typedef struct tagAUD_Volume 
{
	unsigned char front_left;
	unsigned char front_right;
	unsigned char rear_left;
	unsigned char rear_right;
	unsigned char center;
	unsigned char lfe;
} CSAUD_Volume;

typedef enum 
{ 
	AUD_STREAM_TYPE_MPA = 0, 
	AUD_STREAM_TYPE_AC3, 
	AUD_STREAM_TYPE_AAC,
	AUD_STREAM_TYPE_DTS,
	AUD_STREAM_TYPE_LPCM,
	AUD_STREAM_TYPE_AAC_LATM,
	AUD_STREAM_TYPE_AIB,
       AUD_STREAM_TYPE_UNKNOWN
} CSAUD_STREAM_TYPE;

typedef enum 
{ 
	AUD_EQUALIZER_TYPE_DISABLE = 0, 
	AUD_EQUALIZER_TYPE_POP, 
	AUD_EQUALIZER_TYPE_THEATRE,
	AUD_EQUALIZER_TYPE_ROCK, 
	AUD_EQUALIZER_TYPE_CLASSICAL,
	AUD_EQUALIZER_TYPE_CUSTOM = 7
} CSAUD_EQUALIZER_TYPE;

typedef struct tagAUD_EqualizerConfig 
{
	CSAUD_EQUALIZER_TYPE equalizer_type;
        signed char balance;
	int equalizer_band_weight[10];	/* Q27 format */
} CSAUD_EqualizerConfig;

typedef enum 
{ 
	AUD_SAMPLE_RATE_96KHZ = 0, 
	AUD_SAMPLE_RATE_88_200KHZ, 
	AUD_SAMPLE_RATE_64KHZ,
	AUD_SAMPLE_RATE_48KHZ, 
	AUD_SAMPLE_RATE_44_100KHZ,
	AUD_SAMPLE_RATE_32KHZ, 
	AUD_SAMPLE_RATE_24KHZ,
	AUD_SAMPLE_RATE_22_050KHZ, 
	AUD_SAMPLE_RATE_16KHZ,
	AUD_SAMPLE_RATE_12KHZ, 
	AUD_SAMPLE_RATE_11_025KHZ,
	AUD_SAMPLE_RATE_8KHZ
} CSAUD_SAMPLE_RATE;

typedef struct tagAUD_MixerConfig 
{
	unsigned int mixer_level; /* Q31 format */
	CSAUD_SAMPLE_RATE mixer_sample_rate;
} CSAUD_MixerConfig;

typedef enum
{
	AUD_INPUT_NOBLOCK = 0,
	AUD_INPUT_BLOCK
} CSAUD_INPUT_MODE;

typedef enum{
    LEVEL0 = 0,
    LEVEL1,
    LEVEL2,
    LEVEL3,
    LEVEL4,
    LEVEL5,
    LEVEL6,
    LEVEL7,
}CSAUD_ERROR_THRESHOLD;

typedef enum 
{ 
	AUD_NO_ERROR = 0, 
	AUD_ERROR_OPEN_FAILED,		/* open filed                 */
	AUD_ERROR_IOCTL_FAILED,		/* ioctl filed                */
	AUD_ERROR_INVALID_PARAMETERS,	/* Bad parameter passed       */
	AUD_ERROR_UNKNOWN_DEVICE,	/* Unknown device name        */
	AUD_ERROR_DEVICE_BUSY,		/* Device is currently busy   */
	AUD_ERROR_INVALID_HANDLE,	/* Handle is not valid        */
	AUD_ERROR_ALREADY_INITIALIZED,	/* Device already initialized */
	AUD_ERROR_NOT_INITIALIZED,	/* Device not initialized     */
	AUD_ERROR_NO_MEMORY,	/* Device not enough memory     */
	AUD_ERROR_MEMORY_MAP    	/* Device memory map error     */
} CSAUD_ErrCode;

typedef struct CSAUD_ATTRIBUTE_ {
unsigned char nr_mode; /* not support now !!!. Noise reduction mode: 0: NO_NOISE_REDUCTION or 1:NOISE_REDUCTION */
signed char balance; /* MAX_BALANCE = 127 MIN_BALANCE=-128 : sets balance between Left and Right channels */
signed char bass; /* Low frequency equalizer setting */
signed char mid; /* Mid frequency equalizer setting */
signed char treble; /* High frequency equalizer setting */
} CSAUD_ATTRIBUTE;

/* common interface */
CSAUD_HANDLE CSAUD_Open(CSAUD_DEV dev);
CSAPI_RESULT CSAUD_Close(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_Init(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_Play(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_Stop(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_Pause(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_Resume(CSAUD_HANDLE handle);

CSAPI_RESULT  CSAUD_SetInputMode(CSAUD_HANDLE handle, CSAUD_INPUT_MODE input_mod);
CSAPI_RESULT  CSAUD_GetInputMode(CSAUD_HANDLE handle, CSAUD_INPUT_MODE* input_mod);

CSAPI_RESULT CSAUD_SetSampleRate(CSAUD_HANDLE handle, CSAUD_SAMPLE_RATE audio_freq);
CSAPI_RESULT CSAUD_GetSampleRate(CSAUD_HANDLE handle, CSAUD_SAMPLE_RATE * audio_freq);

CSAPI_RESULT CSAUD_SetVolume(CSAUD_HANDLE handle, CSAUD_Volume * vol);
CSAPI_RESULT CSAUD_GetVolume(CSAUD_HANDLE handle, CSAUD_Volume * vol);

CSAPI_RESULT CSAUD_SetCodecType(CSAUD_HANDLE handle, CSAUD_STREAM_TYPE stream_type);
CSAPI_RESULT CSAUD_GetCodecType(CSAUD_HANDLE handle, CSAUD_STREAM_TYPE * stream_type);

CSAPI_RESULT CSAUD_SetOutputDevice(CSAUD_HANDLE handle, CSAUD_OUTPUT dev);
CSAPI_RESULT CSAUD_GetOutputDevice(CSAUD_HANDLE handle, CSAUD_OUTPUT * dev);

CSAPI_RESULT CSAUD_SetI2SFormat(CSAUD_HANDLE handle, unsigned short format_cnt);
CSAPI_RESULT CSAUD_GetI2SFormat(CSAUD_HANDLE handle, unsigned short *format_cnt);

CSAPI_RESULT CSAUD_EnableMute(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisableMute(CSAUD_HANDLE handle);

CSAPI_RESULT CSAUD_EnablePTSSync(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisablePTSSync(CSAUD_HANDLE handle);

CSAPI_RESULT CSAUD_EnableDRC(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisableDRC(CSAUD_HANDLE handle);

CSAPI_RESULT CSAUD_EnableSurround(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisableSurround(CSAUD_HANDLE handle);

CSAPI_RESULT CSAUD_SetOutputChannel(CSAUD_HANDLE handle, CSAUD_PCM_CHANNEL channel);
CSAPI_RESULT CSAUD_GetOutputChannel(CSAUD_HANDLE handle, CSAUD_PCM_CHANNEL * channel);

CSAPI_RESULT CSAUD_EnableEqualizer(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisableEqualizer(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_SetEqualizerConfig(CSAUD_HANDLE handle, CSAUD_EqualizerConfig * equalizer_config);
CSAPI_RESULT CSAUD_GetEqualizerConfig(CSAUD_HANDLE handle, CSAUD_EqualizerConfig * equalizer_config);

CSAPI_RESULT CSAUD_EnableMixer(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_DisableMixer(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_SetMixerConfig(CSAUD_HANDLE handle, CSAUD_MixerConfig * mixer_config);
CSAPI_RESULT CSAUD_GetMixerConfig(CSAUD_HANDLE handle, CSAUD_MixerConfig * mixer_config);
CSAPI_RESULT CSAUD_WriteMixerBuffer(CSAUD_HANDLE handle, char *src, int size);

CSAPI_RESULT CSAUD_SetStartDelay(CSAUD_HANDLE handle, unsigned int ms_number);

CSAPI_RESULT CSAUD_ErrNotify(CSAUD_HANDLE handle,void (*call_back_function)(CSAUD_HANDLE*,CSAUD_ERROR_THRESHOLD),CSAUD_ERROR_THRESHOLD error_threshold,int event_enable);

CSAUD_ErrCode CSAUD_GetErrCode(CSAUD_HANDLE handle);
char *CSAUD_GetErrString(CSAUD_HANDLE handle);

CSAPI_RESULT CSAUD_SetAttributes( CSAUD_HANDLE handle , CSAUD_ATTRIBUTE *Attr);
CSAPI_RESULT CSAUD_GetAttributes( CSAUD_HANDLE handle , CSAUD_ATTRIBUTE *Attr);

CSAPI_RESULT CSAUD_PFMOpen(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_PFMClose(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_PFMReset(CSAUD_HANDLE handle);
CSAPI_RESULT CSAUD_GetPFMBufferSize(CSAUD_HANDLE handle, unsigned int * bufsize);
CSAPI_RESULT CSAUD_WritePFMData(CSAUD_HANDLE handle, unsigned char *src, int size);
CSAPI_RESULT CSAUD_SetNotifyPFMDataEmpty(CSAUD_HANDLE handle, unsigned int *empty_threshold, void (* call_back_function)(CSAUD_HANDLE *), unsigned char event_enable);
CSAPI_RESULT CSAUD_SetNotifyPFMDataFull(CSAUD_HANDLE handle, unsigned int *full_threshold,void (*call_back_function) (CSAUD_HANDLE *), unsigned char event_enable);
CSAPI_RESULT CSAUD_WritePFMDataWithPTS(CSAUD_HANDLE handle, unsigned char *src, int size, unsigned long long pts);

#ifdef __cplusplus
} 
#endif 

#endif
