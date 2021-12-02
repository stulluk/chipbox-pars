#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "csaud.h"

#if 1 /*PFM*/
#include <asm/page.h>
#include <sys/mman.h>
typedef void (*call_back_emptynotify) (CSAUD_HANDLE *);
typedef struct {
	void *pVMem;
	void *pPMem;
	unsigned int MemSize;
	void *MappedAddr;
	unsigned int MappedSize;
	int FD;
	int emptynotify_size;
	unsigned char empty_enable;
	call_back_emptynotify empty_fn;
	int fullnotify_size;
	unsigned char full_enable;
	call_back_emptynotify full_fn;
} CSAUD_MMAP_t;
CSAUD_MMAP_t g_cabmem;
CSAUD_MMAP_t g_ptsmem;
#endif

#define  CSAUD_OBJ_TYPE  	'a'
#define  CSAUD_DEV_FILE0	"/dev/misc/csaudio"
#define  CSAUD_DEV_FILE1	"/dev/misc/csaudio"

#define  CSDRV_AUDIO_IOC_MAGIC  'a'
/* io commands */
#define  CSAUD_IOC_SET_DECTYPE         	_IOW(CSDRV_AUDIO_IOC_MAGIC, 1, int)
#define  CSAUD_IOC_SET_RESET          	_IOW(CSDRV_AUDIO_IOC_MAGIC, 2, int)
#define  CSAUD_IOC_SET_FREQ           	_IOW(CSDRV_AUDIO_IOC_MAGIC, 3, int)
#define  CSAUD_IOC_SET_DRC            	_IOW(CSDRV_AUDIO_IOC_MAGIC, 4, int)
#define  CSAUD_IOC_SET_MUTE           	_IOW(CSDRV_AUDIO_IOC_MAGIC, 5, int)
#define  CSAUD_IOC_SET_SURROUND       	_IOW(CSDRV_AUDIO_IOC_MAGIC, 6, int)
#define  CSAUD_IOC_SET_PTS_SYNC       	_IOW(CSDRV_AUDIO_IOC_MAGIC, 7, int)
#define  CSAUD_IOC_SET_VOLUME         	_IOW(CSDRV_AUDIO_IOC_MAGIC, 9, int)
#define  CSAUD_IOC_PLAY           	_IOW(CSDRV_AUDIO_IOC_MAGIC, 10, int)
#define  CSAUD_IOC_STOP           	_IOW(CSDRV_AUDIO_IOC_MAGIC, 11, int)
#define  CSAUD_IOC_SET_ACTIVE_MODE      _IOW(CSDRV_AUDIO_IOC_MAGIC, 12, int)
//#define  CSAUD_IOC_SET_OUTPUT_MODE      _IOW(CSDRV_AUDIO_IOC_MAGIC, 13, int)
//#define  CSAUD_IOC_SET_OUTPUT_DATA_TYPE _IOW(CSDRV_AUDIO_IOC_MAGIC, 14, int)
#define  CSAUD_IOC_SET_MIXER_STATUS     _IOW(CSDRV_AUDIO_IOC_MAGIC, 15, int)
#define  CSAUD_IOC_SET_MIXER_CONFIG     _IOW(CSDRV_AUDIO_IOC_MAGIC, 16, int)
#define  CSAUD_IOC_SET_OUTPUT_CHANNEL   _IOW(CSDRV_AUDIO_IOC_MAGIC, 17, int)
#define  CSAUD_IOC_SET_PLAYMODE      	_IOW(CSDRV_AUDIO_IOC_MAGIC, 18, int)
#define  CSAUD_IOC_SET_STARTDELAY       _IOW(CSDRV_AUDIO_IOC_MAGIC, 19, int)
#define  CSAUD_IOC_SET_DECODEDBYTES     _IOW(CSDRV_AUDIO_IOC_MAGIC, 20, int)
#define  CSAUD_IOC_SET_EQUALIZER_STATUS _IOW(CSDRV_AUDIO_IOC_MAGIC, 21, int)
#define  CSAUD_IOC_SET_EQUALIZER_CONFIG _IOW(CSDRV_AUDIO_IOC_MAGIC, 22, int)

#define  CSAUD_IOC_GET_DECTYPE        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 23, int)
#define  CSAUD_IOC_GET_BITRATE       	_IOW(CSDRV_AUDIO_IOC_MAGIC, 24, int)
#define  CSAUD_IOC_GET_SAMPLERATE       _IOW(CSDRV_AUDIO_IOC_MAGIC, 25, int)
#define  CSAUD_IOC_GET_ACMODE        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 26, int)
#define  CSAUD_IOC_GET_DECODEDBYTES     _IOW(CSDRV_AUDIO_IOC_MAGIC, 27, int)
#define  CSAUD_IOC_GET_DECODEDFRAME    	_IOW(CSDRV_AUDIO_IOC_MAGIC, 28, int)
#define  CSAUD_IOC_GET_VOLUME        	_IOW(CSDRV_AUDIO_IOC_MAGIC, 29, int)
#define  CSAUD_IOC_GET_MIXER_CONFIG     _IOW(CSDRV_AUDIO_IOC_MAGIC, 30, int)
#define  CSAUD_IOC_GET_EQUALIZER_CONFIG _IOW(CSDRV_AUDIO_IOC_MAGIC, 31, int)

#define  CSAUD_IOC_WRITE_MIXER_BUFFER   _IOW(CSDRV_AUDIO_IOC_MAGIC, 32, int)
#define  CSAUD_IOC_SET_OUTPUT_DEV       _IOW(CSDRV_AUDIO_IOC_MAGIC, 33, int)
#define  CSAUD_IOC_GET_OUTPUT_DEV       _IOW(CSDRV_AUDIO_IOC_MAGIC, 34, int)
#define  CSAUD_IOC_SET_I2SFORMAT     	_IOW(CSDRV_AUDIO_IOC_MAGIC, 35, int)
#define  CSAUD_IOC_GET_I2SFORMAT     	_IOW(CSDRV_AUDIO_IOC_MAGIC, 36, int)

#define CSAUD_IOC_SET_ERROR_LEVEL   _IOW(CSDRV_AUDIO_IOC_MAGIC, 37, int)
#define CSAUD_IOC_GET_ERROR_LEVEL   _IOW(CSDRV_AUDIO_IOC_MAGIC, 38, int)

#define CSAUD_IOC_GET_OUTPUT_CHANNEL    _IOW(CSDRV_AUDIO_IOC_MAGIC, 39, int)

#define CSAUD_IOC_PFM_GETCAB_ADDR   _IOW(CSDRV_AUDIO_IOC_MAGIC, 40, int)
#define CSAUD_IOC_PFM_GETCAB_SIZE   _IOW(CSDRV_AUDIO_IOC_MAGIC, 41, int)
#define CSAUD_IOC_PFM_GETCAB_RP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 42, int)
#define CSAUD_IOC_PFM_GETCAB_WP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 43, int)
#define CSAUD_IOC_PFM_GETCAB_LOW   _IOW(CSDRV_AUDIO_IOC_MAGIC, 44, int)
#define CSAUD_IOC_PFM_GETCAB_UP   _IOW(CSDRV_AUDIO_IOC_MAGIC, 45, int)
#define CSAUD_IOC_PFM_SETXPORT_CABWP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 46, int)
#define CSAUD_IOC_PFM_RESET      _IOW(CSDRV_AUDIO_IOC_MAGIC, 47, int)

#define CSAUD_IOC_SET_BALANCE _IOW(CSDRV_AUDIO_IOC_MAGIC, 48, int)
#define CSAUD_IOC_GET_BALANCE _IOW(CSDRV_AUDIO_IOC_MAGIC, 49, int)
/*added by wangxuewei 2008.03.12*/
/*------------------------------------------------------------------------*/
#define CSAUD_IOC_PFM_GETOFIFO_COUNT   _IOW(CSDRV_AUDIO_IOC_MAGIC, 50, int)
/*------------------------------------------------------------------------*/
#define CSAUD_IOC_PFM_GETPTS_SIZE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 51, int)
#define CSAUD_IOC_PFM_GETPTS_ADDR    _IOW(CSDRV_AUDIO_IOC_MAGIC, 52, int)
#define CSAUD_IOC_PFM_GETPTS_LOW    _IOW(CSDRV_AUDIO_IOC_MAGIC, 53, int)
#define CSAUD_IOC_PFM_GETPTS_UP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 54, int)
#define CSAUD_IOC_PFM_GETPTS_WP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 55, int)
#define CSAUD_IOC_PFM_GETPTS_RP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 56, int)
#define CSAUD_IOC_PFM_SETPTS_WP    _IOW(CSDRV_AUDIO_IOC_MAGIC, 57, int)
#define CSAUD_IOC_SET_INPUTMODE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 58, int)
#define CSAUD_IOC_GET_INPUTMODE    _IOW(CSDRV_AUDIO_IOC_MAGIC, 59, int)

pthread_t aud_callback;
pthread_mutex_t aud_mutex;
typedef void (*call_back_errornotify) (CSAUD_HANDLE *, CSAUD_ERROR_THRESHOLD);

typedef enum {
	CSDRV_AUD_DECODERTYPE = 0,
	CSDRV_AUD_BITRATE,
	CSDRV_AUD_SAMPLERATE,
	CSDRV_AUD_ACMODE,
	CSDRV_AUD_DECODEDBYTES,
	CSDRV_AUD_DECODEDFRAME,
	CSDRV_AUD_VOLUME
} CSDRV_AUD_QUERYINFO;

typedef struct tagAUD_OBJ {
	char obj_type;
	char call_back_status;
	int dev_fd;
	int errno;

	struct {
		unsigned short pts_enable:1;

		unsigned short reserved:15;
	} config_params;
	int num_notify;
	unsigned int ptsbytecounter;
	call_back_errornotify errornotify_fn;
	CSAUD_ERROR_THRESHOLD error_level;
} CSAUD_OBJ;

static CSAUD_OBJ csaud_obj;

static char *aud_errstr[] = {
	"AUDIO: no error",
	"AUDIO: open failed",
	"AUDIO: ioctl failed",
	"AUDIO: invalid arguments",
	"AUDIO: unknown device name",
	"AUDIO: busy",
	"AUDIO: invalid handle",
	"AUDIO: already initialized",
	"AUDIO: wasn't initialized",
	"AUDIO: not enough memory",
	"AUDIO: map physical memory failed"
};

CSAUD_HANDLE CSAUD_Open(CSAUD_DEV dev)
{
	UNUSED_VARIABLE(dev);

	memset(&csaud_obj, 0, sizeof(CSAUD_OBJ));

	if (csaud_obj.obj_type != CSAUD_OBJ_TYPE) {
		csaud_obj.dev_fd = open(CSAUD_DEV_FILE0, O_RDWR);

		if (csaud_obj.dev_fd < 0) {
			csaud_obj.errno = AUD_ERROR_OPEN_FAILED;
			return NULL;
		}
	}

	csaud_obj.obj_type = CSAUD_OBJ_TYPE;
	csaud_obj.num_notify = 0;
	pthread_mutex_init(&aud_mutex, NULL);

	return (CSAUD_HANDLE) & csaud_obj;
}

CSAPI_RESULT CSAUD_Close(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	CSAUD_Stop(handle);
	dev_obj->num_notify = 0;
	dev_obj->obj_type = 0;
	close(dev_obj->dev_fd);
	pthread_mutex_destroy(&aud_mutex);
	memset(&csaud_obj, 0, sizeof(CSAUD_OBJ));

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_Init(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_RESET, 0, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_Play(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_PLAY, 0, AUD);
	IOCTL(dev_obj, CSAUD_IOC_SET_PLAYMODE, 2, AUD);//mixer

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_Stop(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

        IOCTL(dev_obj, CSAUD_IOC_SET_PLAYMODE, 3, AUD);//mixer
        IOCTL(dev_obj, CSAUD_IOC_STOP, 0, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT  CSAUD_SetInputMode(CSAUD_HANDLE handle, CSAUD_INPUT_MODE input_mod)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

        IOCTL(dev_obj, CSAUD_IOC_SET_INPUTMODE, &input_mod, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT  CSAUD_GetInputMode(CSAUD_HANDLE handle, CSAUD_INPUT_MODE* input_mod)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

        IOCTL(dev_obj, CSAUD_IOC_GET_INPUTMODE, input_mod, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetSampleRate(CSAUD_HANDLE handle, CSAUD_SAMPLE_RATE audio_freq)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_FREQ, &audio_freq, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetSampleRate(CSAUD_HANDLE handle, CSAUD_SAMPLE_RATE * audio_freq)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_SAMPLERATE, audio_freq, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetVolume(CSAUD_HANDLE handle, CSAUD_Volume * vol)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_VOLUME, vol, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetVolume(CSAUD_HANDLE handle, CSAUD_Volume * vol)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_VOLUME, vol, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetCodecType(CSAUD_HANDLE handle, CSAUD_STREAM_TYPE stream_type)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_DECTYPE, &stream_type, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetCodecType(CSAUD_HANDLE handle, CSAUD_STREAM_TYPE * stream_type)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_DECTYPE, stream_type, AUD);

	switch (*stream_type) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		*stream_type = AUD_STREAM_TYPE_MPA;
		break;
	case 9:
	case 10:
		*stream_type = AUD_STREAM_TYPE_AAC;
		break;
	case 11:
		*stream_type = AUD_STREAM_TYPE_AC3;
		break;
        case 12:
                *stream_type = AUD_STREAM_TYPE_DTS;
                break;
        default:
		*stream_type = AUD_STREAM_TYPE_UNKNOWN;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnableMute(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int mute = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_MUTE, &mute, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisableMute(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int mute = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_MUTE, &mute, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnablePTSSync(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int ptssync = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_PTS_SYNC, &ptssync, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisablePTSSync(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int ptssync = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_PTS_SYNC, &ptssync, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnableDRC(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int drc = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_DRC, &drc, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisableDRC(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int drc = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_DRC, &drc, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnableSurround(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int surround = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_SURROUND, &surround, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisableSurround(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int surround = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_SURROUND, &surround, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetStartDelay(CSAUD_HANDLE handle, unsigned int ms_number)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_STARTDELAY, &ms_number, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetOutputChannel(CSAUD_HANDLE handle, CSAUD_PCM_CHANNEL channel)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_OUTPUT_CHANNEL, &channel, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetOutputChannel(CSAUD_HANDLE handle, CSAUD_PCM_CHANNEL * channel)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_OUTPUT_CHANNEL, channel, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnableEqualizer(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int equalizer = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_EQUALIZER_STATUS, &equalizer, AUD);
	sleep(1);
	IOCTL(dev_obj, CSAUD_IOC_SET_BALANCE, NULL, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisableEqualizer(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int equalizer = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_EQUALIZER_STATUS, &equalizer, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetEqualizerConfig(CSAUD_HANDLE handle, CSAUD_EqualizerConfig * equalizer_config)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_EQUALIZER_CONFIG, equalizer_config, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetEqualizerConfig(CSAUD_HANDLE handle, CSAUD_EqualizerConfig * equalizer_config)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_EQUALIZER_CONFIG, equalizer_config, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_EnableMixer(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int mixer = 1;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_MIXER_STATUS, &mixer, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_DisableMixer(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int mixer = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_MIXER_STATUS, &mixer, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetMixerConfig(CSAUD_HANDLE handle, CSAUD_MixerConfig * mixer_config)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_MIXER_CONFIG, mixer_config, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_WriteMixerBuffer(CSAUD_HANDLE handle, char *src, int size)
{
	unsigned int arg[2];
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	arg[0] = (int) src;
	arg[1] = size;

	IOCTL(dev_obj, CSAUD_IOC_WRITE_MIXER_BUFFER, &arg[0], AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetMixerConfig(CSAUD_HANDLE handle, CSAUD_MixerConfig * mixer_config)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_MIXER_CONFIG, mixer_config, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetOutputDevice(CSAUD_HANDLE handle, CSAUD_OUTPUT dev)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_OUTPUT_DEV, &dev, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetOutputDevice(CSAUD_HANDLE handle, CSAUD_OUTPUT * dev)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_OUTPUT_DEV, dev, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetI2SFormat(CSAUD_HANDLE handle, unsigned short format_cnt)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_I2SFORMAT, &format_cnt, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetI2SFormat(CSAUD_HANDLE handle, unsigned short *format_cnt)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_I2SFORMAT, format_cnt, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_Pause(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_PLAYMODE, 1, AUD);
        //IOCTL(dev_obj, CSAUD_IOC_STOP, 0, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_Resume(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_SET_PLAYMODE, 2, AUD);
        //IOCTL(dev_obj, CSAUD_IOC_PLAY, 0, AUD);
    
	return CSAPI_SUCCEED;
}

static int AudioNotify(int *param)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) param;
	int rc, fd, iCABCnt = 0;
	fd_set fds;

	fd = dev_obj->dev_fd;

	while (1) {
		if (dev_obj->errornotify_fn != NULL) {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			rc = select(fd + 1, &fds, NULL, NULL, NULL);
			if (rc < 0)
				continue;
			if (FD_ISSET(fd, &fds) ? CSAPI_SUCCEED : CSAPI_FAILED) {
				continue;
			}
			else {
				dev_obj->errornotify_fn((CSAUD_HANDLE) dev_obj, dev_obj->error_level);
			}
		}
		if(g_cabmem.full_enable == 1){
			pthread_mutex_lock(&aud_mutex);
			CSAUD_GetPFMBufferSize((CSAUD_HANDLE) dev_obj, &iCABCnt);
			if (g_cabmem.fullnotify_size >= iCABCnt) {
		                g_cabmem.full_fn((CSAUD_HANDLE) dev_obj);
			}
			pthread_mutex_unlock(&aud_mutex);
		}
		if(g_cabmem.empty_enable == 1){
			pthread_mutex_lock(&aud_mutex);
			CSAUD_GetPFMBufferSize((CSAUD_HANDLE) dev_obj, &iCABCnt);
			if(g_cabmem.emptynotify_size >= ((int)g_cabmem.MemSize - iCABCnt)){
		                g_cabmem.empty_fn((CSAUD_HANDLE) dev_obj);
			}
			pthread_mutex_unlock(&aud_mutex);
		}

		if(dev_obj->num_notify) {
			usleep(100);
		}
		else{
			sleep(1);
		}
	}
}

CSAPI_RESULT CSAUD_ErrNotify(CSAUD_HANDLE handle, void (*call_back_function) (CSAUD_HANDLE *, CSAUD_ERROR_THRESHOLD),
			     CSAUD_ERROR_THRESHOLD error_threshold, int event_enable)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	pthread_mutex_lock(&aud_mutex);

	if (event_enable) {
		IOCTL(dev_obj, CSAUD_IOC_SET_ERROR_LEVEL, error_threshold, AUD);

		if (dev_obj->call_back_status == 0) {
			int error = 0;

			dev_obj->call_back_status = 1;
			dev_obj->errornotify_fn = call_back_function;
			dev_obj->error_level = error_threshold;
			dev_obj->num_notify ++;
			error = pthread_create(&aud_callback, NULL, (void *) AudioNotify, dev_obj);
			if (error != 0) {
				IOCTL(dev_obj, CSAUD_IOC_SET_ERROR_LEVEL, 0, AUD);
				dev_obj->errornotify_fn = NULL;
		                    dev_obj->error_level = LEVEL0;
		                    dev_obj->call_back_status = 0;
				dev_obj->num_notify = 0;
			}
		}
	}
	else {
		IOCTL(dev_obj, CSAUD_IOC_SET_ERROR_LEVEL, 0, AUD);
		dev_obj->errornotify_fn = NULL;
		dev_obj->error_level = LEVEL0;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&aud_mutex);

	return CSAPI_SUCCEED;
}

CSAUD_ErrCode CSAUD_GetErrCode(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSAUD_OBJ_TYPE))
		return AUD_ERROR_INVALID_PARAMETERS;

	return dev_obj->errno;
}

char *CSAUD_GetErrString(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSAUD_OBJ_TYPE))
		return aud_errstr[AUD_ERROR_INVALID_PARAMETERS];

	return aud_errstr[dev_obj->errno];
}

CSAPI_RESULT CSAUD_SetAttributes(CSAUD_HANDLE handle, CSAUD_ATTRIBUTE * Attr)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	CSAUD_EqualizerConfig equalizer_config;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	equalizer_config.equalizer_type = AUD_EQUALIZER_TYPE_CUSTOM;
	equalizer_config.balance = Attr->balance;
	equalizer_config.equalizer_band_weight[0] = Attr->bass;
	equalizer_config.equalizer_band_weight[1] = Attr->bass;
	equalizer_config.equalizer_band_weight[2] = Attr->bass;
	equalizer_config.equalizer_band_weight[3] = (Attr->bass * 2 + Attr->mid) / 3;
	equalizer_config.equalizer_band_weight[4] = (Attr->bass + Attr->mid * 2) / 3;
	equalizer_config.equalizer_band_weight[5] = Attr->mid;
	equalizer_config.equalizer_band_weight[6] = (Attr->mid * 2 + Attr->treble) / 3;
	equalizer_config.equalizer_band_weight[7] = (Attr->mid + Attr->treble * 2) / 3;
	equalizer_config.equalizer_band_weight[8] = Attr->treble;
	equalizer_config.equalizer_band_weight[9] = Attr->treble;

	IOCTL(dev_obj, CSAUD_IOC_SET_EQUALIZER_CONFIG, &equalizer_config, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetAttributes(CSAUD_HANDLE handle, CSAUD_ATTRIBUTE * Attr)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	CSAUD_EqualizerConfig equalizer_config;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_GET_EQUALIZER_CONFIG, &equalizer_config, AUD);

	Attr->nr_mode = 0;
	Attr->balance = equalizer_config.balance;
	Attr->bass = equalizer_config.equalizer_band_weight[0];
	Attr->mid = equalizer_config.equalizer_band_weight[5];
	Attr->treble = equalizer_config.equalizer_band_weight[9];

	return CSAPI_SUCCEED;
}

#if 1	//PFM
#define MEM_SWAP32(x)	( (((x) & 0xff) << 24) | (((x) & 0xff00) <<8) |	(((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24) )
#define DMXOUT_TOGGLE_OFFSET 	31
#define DMXOUT_TOGGLE_BIT 	(1 << DMXOUT_TOGGLE_OFFSET)
#define DMXOUT_DIR_OFFSET 	3

#define AUD_REMAIN_BUF_ALIGN_SIZE  16

static int aud_remain_size = 0;
static char aud_remain_buf[AUD_REMAIN_BUF_ALIGN_SIZE];

int __distance(unsigned int x, unsigned int y, unsigned int toggle, unsigned int size)
{
	int ret_val;
	unsigned int toggle_x, toggle_y;

	toggle_x = x & toggle;
	toggle_y = y & toggle;

	x -= toggle_x;
	y -= toggle_y;

	if (toggle_x == toggle_y)
		ret_val = x - y;
	else if (x > y)
		ret_val = x - y - size;
	else
		ret_val = x + size - y;

	return ret_val;
}

CSAPI_RESULT CSAUD_PFMOpen(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int phyaddr = 0;
	unsigned int memsize = 0;
	unsigned int addroffset = 0;
	unsigned char *buf = NULL;
	unsigned int addr = 0;
	unsigned int size = 0;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_PFM_RESET, NULL, AUD);
        /*************mapping audio cab buffer start*******************/
        IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_ADDR, &addr, AUD);
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_SIZE, &size, AUD);

	memset(&g_cabmem, 0, sizeof(CSAUD_MMAP_t));

	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + size + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;

	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, dev_obj->dev_fd, phyaddr);

	if (buf == MAP_FAILED) {
		dev_obj->errno = AUD_ERROR_MEMORY_MAP;
		return CSAPI_FAILED;
	}

	g_cabmem.MappedSize = memsize;
	g_cabmem.MappedAddr = buf;
	g_cabmem.pPMem = (void *) addr;
	g_cabmem.pVMem = (void *) (buf + addroffset);
	g_cabmem.MemSize = size;
	g_cabmem.FD = dev_obj->dev_fd;
        /***************mapping audio cab buffer end******************/
        
        /***************mapping audio pts buffer start*****************/
        IOCTL(dev_obj, CSAUD_IOC_PFM_GETPTS_ADDR, &addr, AUD);
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETPTS_SIZE, &size, AUD);

        memset(&g_ptsmem, 0, sizeof(CSAUD_MMAP_t));

	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + size + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;

	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, dev_obj->dev_fd, phyaddr);

	if (buf == MAP_FAILED) {
		dev_obj->errno = AUD_ERROR_MEMORY_MAP;
		return CSAPI_FAILED;
	}

	g_ptsmem.MappedSize = memsize;
	g_ptsmem.MappedAddr = buf;
	g_ptsmem.pPMem = (void *) addr;
	g_ptsmem.pVMem = (void *) (buf + addroffset);
	g_ptsmem.MemSize = size;
	g_ptsmem.FD = dev_obj->dev_fd;
        /***************mapping audio pts buffer end******************/

	memset(aud_remain_buf, 0, AUD_REMAIN_BUF_ALIGN_SIZE);
       aud_remain_size = 0;

        dev_obj->ptsbytecounter = 0;
        
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_PFMClose(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	munmap(g_cabmem.MappedAddr, g_cabmem.MappedSize);
        munmap(g_ptsmem.MappedAddr, g_ptsmem.MappedSize);
        
	memset(&g_cabmem, 0, sizeof(CSAUD_MMAP_t));
	memset(&g_ptsmem, 0, sizeof(CSAUD_MMAP_t));
	
	memset(aud_remain_buf, 0, AUD_REMAIN_BUF_ALIGN_SIZE);
       aud_remain_size = 0;

        dev_obj->ptsbytecounter = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_PFMReset(CSAUD_HANDLE handle)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	memset(aud_remain_buf, 0, AUD_REMAIN_BUF_ALIGN_SIZE);
       aud_remain_size = 0;

        dev_obj->ptsbytecounter = 0;

	IOCTL(dev_obj, CSAUD_IOC_PFM_RESET, NULL, AUD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_GetPFMBufferSize(CSAUD_HANDLE handle, unsigned int *bufsize)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	unsigned int rp, wp, up, low, Size;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_RP, &rp, AUD);
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_WP, &wp, AUD);
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_LOW, &low, AUD);
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_UP, &up, AUD);

	Size = (up - low + 1) << 4;
	Size = Size - __distance(wp << 4, rp << 4, 0x10000000, Size);
	*bufsize = Size;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_WritePFMData(CSAUD_HANDLE handle, unsigned char *src, int size)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	int iCABCnt = 0;
	unsigned int pDst = 0;
	unsigned int pUp = 0;
	int offset = 0;
	unsigned int toggle = 0;
	unsigned int tmp_size = 0;
	unsigned int old_size = size;
	unsigned int old_iCABCnt = 0;
	unsigned char* old_src = src; 
 
	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	CSAUD_GetPFMBufferSize(handle, &iCABCnt);
      
	old_iCABCnt = iCABCnt;

        if ((size + aud_remain_size * 2) > iCABCnt) {
		dev_obj->errno = AUD_ERROR_NO_MEMORY;
		return CSAPI_FAILED;
	}
    
	size += aud_remain_size;
	tmp_size = size % AUD_REMAIN_BUF_ALIGN_SIZE;
	size -= tmp_size;

	/* writer pointer - logical address */
	offset = (int) g_cabmem.pVMem - (int) g_cabmem.pPMem;
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_WP, &pDst, AUD);

	toggle = (pDst & 0x01000000);
	pDst = (pDst & 0xFFFFFF) << 4;

	pDst = (unsigned int) ((int) pDst + offset);

	/* cab up - logical address */
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_UP, &pUp, AUD);
	pUp = (pUp + 1) << 4;
	pUp = (unsigned int) ((int) pUp + offset);

	/* copy */
	iCABCnt = pUp - pDst;
	if(iCABCnt < aud_remain_size){ /* if residual space doesn't hold remain data, toggle status */
		pDst = (unsigned int) g_cabmem.pVMem;
		toggle ^= 0x01000000;
		iCABCnt = old_iCABCnt - iCABCnt - aud_remain_size;
		
	}
	else
		iCABCnt = pUp - pDst - aud_remain_size;		

	if(aud_remain_size > 0) /* write remain audio data to cab */		
		memcpy((unsigned char *) pDst, (unsigned char *) aud_remain_buf, aud_remain_size);

	/* update r/w pointer and size */
	pDst += aud_remain_size;
	size -= aud_remain_size;
	iCABCnt = min(iCABCnt, size);

	memcpy((unsigned char *) pDst, (unsigned char *) src, iCABCnt);

	src += iCABCnt;
	pDst += iCABCnt;
	size -= iCABCnt;
	if (pDst == pUp) {
		pDst = (unsigned int) g_cabmem.pVMem;
		toggle ^= 0x01000000;
	}

	if (size > 0) {
		memcpy((unsigned char *) pDst, (unsigned char *) src, size);
		pDst += size;
	}

	/* write WP back to register */
	pDst = pDst - offset;
	pDst = pDst >> 3;	// orion1.4: wp unit is 8-bytes

	if (toggle) {
		pDst = pDst | 0x80000000;
	}

	IOCTL(dev_obj, CSAUD_IOC_PFM_SETXPORT_CABWP, &pDst, AUD);
	if(tmp_size > 0)
		memcpy((unsigned char *)aud_remain_buf, (old_src + old_size - tmp_size), tmp_size);		
	
	aud_remain_size = tmp_size;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetNotifyPFMDataEmpty(CSAUD_HANDLE handle, unsigned int *empty_threshold,
					 void (*call_back_function) (CSAUD_HANDLE *), unsigned char event_enable)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	pthread_mutex_lock(&aud_mutex);

	/* the minimum audio data size is 256B*/
	if(*empty_threshold < 256) *empty_threshold = 256;

	if (event_enable) {
		g_cabmem.emptynotify_size = *empty_threshold;
		g_cabmem.empty_enable = event_enable;
		g_cabmem.empty_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->call_back_status == 0) {
			int error = 0;

			dev_obj->call_back_status = 1;
			error = pthread_create(&aud_callback, NULL, (void *) AudioNotify, dev_obj);
			if (error != 0) {
				g_cabmem.emptynotify_size = -1;
				g_cabmem.empty_enable = 0;
				g_cabmem.empty_fn = NULL;
				dev_obj->call_back_status = 0;
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
		}
	}
	else {
		g_cabmem.emptynotify_size = -1;
		g_cabmem.empty_enable = 0;
		g_cabmem.empty_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&aud_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_SetNotifyPFMDataFull(CSAUD_HANDLE handle, unsigned int *full_threshold,
					 void (*call_back_function) (CSAUD_HANDLE *), unsigned char event_enable)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	pthread_mutex_lock(&aud_mutex);

	if (event_enable) {
		g_cabmem.fullnotify_size = *full_threshold;
		g_cabmem.full_enable = event_enable;
		g_cabmem.full_fn = call_back_function;
		dev_obj->num_notify++;
		
		if (dev_obj->call_back_status == 0) {
			int error = 0;

			dev_obj->call_back_status = 1;
			error = pthread_create(&aud_callback, NULL, (void *) AudioNotify, dev_obj);
			if (error != 0) {
				g_cabmem.fullnotify_size = -1;
				g_cabmem.full_enable = 0;
				g_cabmem.full_fn = NULL;
				dev_obj->call_back_status = 0;
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
		}
	}
	else {
		g_cabmem.fullnotify_size = -1;
		g_cabmem.full_enable = 0;
		g_cabmem.full_fn = NULL;
		dev_obj->num_notify--;
	}
	pthread_mutex_unlock(&aud_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT __csaud_writePTS(CSAUD_HANDLE handle, unsigned int count, unsigned long long pts)
{
        CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
        unsigned int pts_wp,pts_rp,pts_wp_tog,pts_rp_tog,align_wd;
        unsigned int aud_bytes_cnt = dev_obj->ptsbytecounter;
        unsigned int pts_lowlimit = (unsigned int) g_ptsmem.pPMem;
	unsigned int pts_uplimit = pts_lowlimit + g_ptsmem.MemSize;
        unsigned int offset = (int) g_ptsmem.pVMem - (int) g_ptsmem.pPMem;
	unsigned int cur_pts = 0;
	
        CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	cur_pts = pts>>1;
        if(pts!=0)
        {
                IOCTL(dev_obj, CSAUD_IOC_PFM_GETPTS_WP, &pts_wp, AUD);
                pts_wp_tog = pts_wp&0x80000000;
                pts_wp <<=3;
		  IOCTL(dev_obj, CSAUD_IOC_PFM_GETPTS_RP, &pts_rp, AUD);
                pts_rp_tog = pts_rp&0x80000000;
                pts_rp <<= 4;

		if ( !( (pts_wp == pts_rp) && (pts_wp_tog != pts_rp_tog) ) )
                {
                        align_wd = (cur_pts>>23)|(3<<17);
                        *((volatile unsigned int *) (pts_wp + offset)) = MEM_SWAP32(align_wd);

                        pts_wp +=4;
                        align_wd = cur_pts<<9;
                        *((volatile unsigned int *) (pts_wp + offset)) = MEM_SWAP32(align_wd);

                        pts_wp +=4;
                        *((volatile unsigned int *) (pts_wp + offset)) = 0;

                        pts_wp +=4;
                        align_wd = aud_bytes_cnt&0x1ffffff;
                        *((volatile unsigned int *) (pts_wp + offset)) = MEM_SWAP32(align_wd);

                        pts_wp +=4;

                        if(pts_wp>= pts_uplimit)
                        {       
                                pts_wp = pts_lowlimit;
                                pts_wp_tog = (pts_wp_tog^0x80000000);
                        }

                        pts_wp = (pts_wp >> DMXOUT_DIR_OFFSET) | pts_wp_tog;

	                IOCTL(dev_obj, CSAUD_IOC_PFM_SETPTS_WP, &pts_wp, AUD);
                }
        }
    	 dev_obj->ptsbytecounter += count;
    	 dev_obj->ptsbytecounter &= 0x1ffffff;

         return CSAPI_SUCCEED;
}

CSAPI_RESULT CSAUD_WritePFMDataWithPTS(CSAUD_HANDLE handle, unsigned char *src, int size, unsigned long long pts)
{
	CSAUD_OBJ *dev_obj = (CSAUD_OBJ *) handle;
	int iCABCnt = 0;
	unsigned int pDst = 0;
	unsigned int pUp = 0;
	int offset = 0;
	unsigned int toggle = 0;
	unsigned int tmp_size = 0;
	unsigned int old_size = size;
	unsigned int old_iCABCnt = 0;
       unsigned int pts_counter = 0;
	unsigned char* old_src = src; 

	CHECK_HANDLE_VALID(dev_obj, CSAUD_OBJ_TYPE);

	CSAUD_GetPFMBufferSize(handle, &iCABCnt);

	old_iCABCnt = iCABCnt;

        if ((size + aud_remain_size * 2) > iCABCnt) {
		dev_obj->errno = AUD_ERROR_NO_MEMORY;
		return CSAPI_FAILED;
	}
    
	size += aud_remain_size;
	tmp_size = size % AUD_REMAIN_BUF_ALIGN_SIZE;
	size -= tmp_size;

	/* writer pointer - logical address */
	offset = (int) g_cabmem.pVMem - (int) g_cabmem.pPMem;
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_WP, &pDst, AUD);

	toggle = (pDst & 0x01000000);
	pDst = (pDst & 0xFFFFFF) << 4;

	pDst = (unsigned int) ((int) pDst + offset);

	/* cab up - logical address */
	IOCTL(dev_obj, CSAUD_IOC_PFM_GETCAB_UP, &pUp, AUD);
	pUp = (pUp + 1) << 4;
	pUp = (unsigned int) ((int) pUp + offset);

	/* copy */
	iCABCnt = pUp - pDst;
	if(iCABCnt < aud_remain_size){ /* if residual space doesn't hold remain data, toggle status */
		pDst = (unsigned int) g_cabmem.pVMem;
		toggle ^= 0x01000000;
		iCABCnt = old_iCABCnt - iCABCnt - aud_remain_size;
	}
	else{
		iCABCnt = pUp - pDst - aud_remain_size;
	}
	if(aud_remain_size > 0) {/* write remain audio data to cab */		
		memcpy((unsigned char *) pDst, (unsigned char *) aud_remain_buf, aud_remain_size);
                pts_counter += aud_remain_size;
       }
	/* update r/w pointer and size */
	pDst += aud_remain_size;
	size -= aud_remain_size;
	iCABCnt = min(iCABCnt, size);

	memcpy((unsigned char *) pDst, (unsigned char *) src, iCABCnt);

        pts_counter += iCABCnt;
	src += iCABCnt;
	pDst += iCABCnt;
	size -= iCABCnt;
	if (pDst == pUp) {
		pDst = (unsigned int) g_cabmem.pVMem;
		toggle ^= 0x01000000;
	}

	if (size > 0) {
		memcpy((unsigned char *) pDst, (unsigned char *) src, size);
		pDst += size;
                pts_counter += size;
	}

        __csaud_writePTS(handle, pts_counter, pts);

	/* write WP back to register */
	pDst = pDst - offset;
	pDst = pDst >> 3;	// orion1.4: wp unit is 8-bytes

	if (toggle) {
		pDst = pDst | 0x80000000;
	}

	IOCTL(dev_obj, CSAUD_IOC_PFM_SETXPORT_CABWP, &pDst, AUD);
	if(tmp_size > 0){
		memcpy((unsigned char *)aud_remain_buf, (old_src + old_size - tmp_size), tmp_size);			
	}
	aud_remain_size = tmp_size;

	return CSAPI_SUCCEED;
}
#endif//pfm

