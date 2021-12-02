#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
 
#include "csaud.h"

pthread_t thread_pfm_play;

CSAUD_HANDLE aud_handler;
static int pfm_is_played  = 0;
static void *_fill_pfm_stream(void *param)
{
	int read_len = 0;
	unsigned int bufsize = 0;
	unsigned char  read_buf[98304];
	FILE*   aud_file=NULL;

	aud_file = fopen(param,"rb");
	if(aud_file == NULL) {
		printf("======>file open failed\n");
		return NULL;
	}

	if ((read_len=fread(read_buf,1,4096,aud_file))!=4096) {
		fseek(aud_file,0L, SEEK_SET);
		printf("read_len = %d ,===no data \n",read_len);
	}

	while(1) {
		if (!pfm_is_played)
			break;

		static unsigned long long pts_test = 0x100;
		if(CSAPI_SUCCEED == CSAUD_WritePFMDataWithPTS(aud_handler, 
							      read_buf, read_len, pts_test)){
			pts_test++;
			if ((read_len=fread(read_buf,1,4096,aud_file))!=4096) {
				//fseek(aud_file,0L, SEEK_SET);
				if (read_len == 0)
					break;
				//printf("read_len = %d ,===no data \n",read_len);
			} 
			//CSAUD_GetPFMBufferSize(aud_handler, &bufsize);
			//printf("bufsize = %d\n",bufsize);
		}
	}

	pfm_is_played = 0;
	CSAUD_Stop(aud_handler);
	CSAUD_PFMClose(aud_handler);
	CSAUD_Close(aud_handler);
	return NULL;
}
static void aud_pfm_play(int argc, char **argv)
{
	CSAUD_STREAM_TYPE type;
	CSAUD_Volume vol = { 100, 100, 100, 100, 100, 100 };

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	aud_handler = CSAUD_Open(0);
	if (aud_handler < 0) {
		printf("open audeo error\n");
		return;
	}

	//audio decoder set
	CSAUD_Init(aud_handler);
	CSAUD_SetInputMode(aud_handler,AUD_INPUT_BLOCK);
	type = atoi(argv[2]);
	CSAUD_SetCodecType(aud_handler,type);	
	CSAUD_SetVolume(aud_handler, &vol);
	CSAUD_Play(aud_handler);

	CSAUD_PFMOpen(aud_handler);

	if (0 != pthread_create(&thread_pfm_play, NULL, 
				(void *)_fill_pfm_stream, argv[1])) {
		printf("Error: pthread_create.!\n");
		return;
	}

	pfm_is_played = 1;
}

static void aud_pfm_stop(int argc, char **argv)
{
	CSAPI_RESULT result;

	if (!pfm_is_played) {
		printf("pfm is not played, run <aud_pfm_play> first!\n");
		return;
	}

	result = CSAUD_Stop(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_Stop Error.!\n");
		return;
	} 

	pfm_is_played = 0;
	if (pthread_join(thread_pfm_play, NULL) < 0) {
		printf("ERROR, pthread_join");
		return;
	}
}

static void aud_mute(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

	if (!pfm_is_played) {
		printf("pfm is not played, run <aud_pfm_play> first!\n");
		return;
	}

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnableMute(aud_handler);
	else 		result = CSAUD_DisableMute(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_Mute Operate Error.!\n");
		return;
	}
}

static void aud_volume(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSAUD_Volume vol;

	if (!pfm_is_played) {
		printf("pfm is not played, run <aud_pfm_play> first!\n");
		return;
	}

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	vol.front_left 
		= vol.front_right 
		= vol.rear_left 
		= vol.rear_right 
		= vol.center 
		= vol.lfe = atoi(argv[1]);

	result = CSAUD_SetVolume(aud_handler, &vol);
	if (result < 0) {
		printf("Error: CSAUD_SetVolume.!\n");
		return;
	}
}

static void aud_samplerate(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSAUD_SAMPLE_RATE sample_rate;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	sample_rate = atoi(argv[1]);
	result = CSAUD_SetSampleRate(aud_handler, sample_rate);
	if (result < 0) {
		printf("Error: CSAUD_SetSampleRate.!\n");
		return;
	}
}

static void aud_inputmode(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSAUD_INPUT_MODE mode;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	mode = atoi(argv[1]);
	result = CSAUD_SetInputMode(aud_handler, mode);
	if (result < 0) {
		printf("Error: CSAUD_SetInputMode.!\n");
		return;
	}
}

static void aud_sync(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnablePTSSync(aud_handler);
	else 		result = CSAUD_DisablePTSSync(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_EnablePTSSync Error.!\n");
		return;
	} 
}

static void aud_surround(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnableSurround(aud_handler);
	else 		result = CSAUD_DisableSurround(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_EnableSurround Error.!\n");
		return;
	}
}

static void aud_drc(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnableDRC(aud_handler);
	else 		result = CSAUD_DisableDRC(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_EnableDRC Error.!\n");
		return;
	} 
}

static void aud_equalizer(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnableEqualizer(aud_handler);
	else 		result = CSAUD_DisableEqualizer(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_EnableEqualizer Enable Error.!\n");
		return;
	}
}

static void aud_equalizer_conf(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSAUD_EqualizerConfig config;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	memset(&config, 0, sizeof(config));
	config.equalizer_type = atoi(argv[1]);
	config.balance = atoi(argv[2]);

	result = CSAUD_SetEqualizerConfig(aud_handler, &config);
	if (result < 0) {
		printf("Error: CSAUD_SetEqualizerConfig.!\n");
		return;
	}
}

static void aud_mixer(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSAUD_EnableMixer(aud_handler);
	else 		result = CSAUD_DisableMixer(aud_handler);
	if (result < 0) {
		printf("Error: CSAUD_EnableMixer Enable Error.!\n");
		return;
	}
}

static void aud_mixer_conf(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSAUD_MixerConfig config;

        if (!pfm_is_played) {
                printf("pfm is not played, run <aud_pfm_play> first!\n");
                return;
        }

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	config.mixer_level = atoi(argv[1]);
	config.mixer_sample_rate = atoi(argv[2]);

	result = CSAUD_SetMixerConfig(aud_handler, &config);
	if (result < 0) {
		printf("Error: CSAUD_SetMixerConfig.!\n");
		return;
	}
}

pthread_t thread_mixer_play;
static void *_fill_mixer_stream(void *param)
{
	FILE*   aud_file=NULL;
	int read_len;
	unsigned char  read_buf[4096+100];

	aud_file = fopen(param,"rb");
	if(aud_file == NULL) {
		printf("======>file open failed\n");
		return NULL;
	}

	fseek(aud_file,0L, SEEK_SET);
	if ((read_len=fread(read_buf,1,4096,aud_file))!=4096) {
		fseek(aud_file,0L, SEEK_SET);
		printf("read_len = %d ,===no data \n",read_len);
	}

	while(1){
		if(CSAPI_SUCCEED == CSAUD_WriteMixerBuffer(aud_handler, read_buf, read_len)){
			if ((read_len=fread(read_buf,1,4096,aud_file))!=4096) {
				//fseek(aud_file,0L, SEEK_SET);
				if (read_len == 0)
					break;
				printf("read_len = %d ,===no data \n",read_len);
			}
		}
	}

	CSAUD_DisableMixer(aud_handler);
	return NULL;
}
static void aud_mixer_write(int argc, char **argv)
{
	CSAPI_RESULT result;
        CSAUD_MixerConfig mixer_config={100,AUD_SAMPLE_RATE_44_100KHZ};

	if (!pfm_is_played) {
		printf("pfm is not played, run <aud_pfm_play> first!\n");
		return;
	}

        CSAUD_EnableMixer(aud_handler);
        CSAUD_SetMixerConfig(aud_handler, &mixer_config);

	if (0 != pthread_create(&thread_mixer_play, NULL, 
				(void *)_fill_mixer_stream, argv[1])) {
		printf("Error: pthread_create.!\n");
		return;
	}
}

static struct cmd_t aud_tt[] = {
 	{	
 		"aud_pfm_play",
 		NULL,
 		aud_pfm_play,
 		 "aud_pfm_play - nject some audio data into audio decoder, and play it.. \
 		\n usage: aud_pfm_play <filename> <type>  \
 		\n type:    \
 		\n 	 0 - MPA   \
 		\n 	 1 – AC3   \
 		\n 	 2 - AAC   \
 		\n 	 3 - DTS   \
 		\n 	 4 - LPCM   \
 		\n 	 5 - AAC_LATM   \
 		\n eg:aud_pfm_play  test.ts 0\n"},
 	{	
 		"aud_pfm_stop",
 		NULL,
 		aud_pfm_stop,
 		 "aud_pfm_stop -  stop PFM play. \
 		\n usage: aud_pfm_stop  \
 		\n eg:aud_pfm_stop \n"},
 	{	
 		"aud_mute",
 		NULL,
 		aud_mute,
 		 "aud_mute -  make audio output mute or unmute. \
 		\n usage: aud_mute  <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_mute 1 \n"},
 	{	
 		"aud_volume",
 		NULL,
 		aud_volume,
 		 "aud_volume -  set audio volume \
 		\n usage: aud_volume   [value]\
 		\n  	  0 < value < 100 \
 		\n eg:aud_volume 1 \n"},
 	{	
 		"aud_samplerate",
 		NULL,
 		aud_samplerate,
 		 "aud_samplerate -  set audio sample rate for gathering data. \
 		\n usage: aud_samplerate  [value]\
 		\n   value:	\
		\n	  0 - AUD_SAMPLE_RATE_96KHZ \
		\n	  1 - AUD_SAMPLE_RATE_88_200KHZ \
		\n	  2 - AUD_SAMPLE_RATE_64KHZ \
		\n	  3 - AUD_SAMPLE_RATE_48KHZ \
		\n	  4 - AUD_SAMPLE_RATE_44_100KHZ \
		\n	  5 - AUD_SAMPLE_RATE_32KHZ \
		\n	  6 - AUD_SAMPLE_RATE_24KHZ \
		\n	  7 - AUD_SAMPLE_RATE_22_050KHZ \
		\n	  8 - AUD_SAMPLE_RATE_16KHZ \
		\n	  9 - AUD_SAMPLE_RATE_12KHZ \
		\n	  10 - AUD_SAMPLE_RATE_11_025KHZ \
		\n	  11 - AUD_SAMPLE_RATE_8KHZ \
 		\n eg:aud_samplerate 1 \n"},
 	{	
 		"aud_inputmode",
 		NULL,
 		aud_inputmode,
 		 "aud_inputmode -  et audio input mode. \
 		\n usage: aud_inputmode   [modetype]\
 		\n  mode - the following modes are supported \
 		\n  	  1 - nonblock\
 		\n  	  0 - block\
 		\n eg:aud_inputmode 1 \n"},
 	{	
 		"aud_sync",
 		NULL,
 		aud_sync,
 		 "aud_sync - enable or disable audio sync method.\
 		\n usage: aud_sync   <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_sync 1 \n"},
 	{	
 		"aud_surround",
 		NULL,
 		aud_surround,
 		 "aud_surround - enable or disable audio surround.\
 		\n usage: aud_surround   <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_surround 1 \n"},
 	{	
 		"aud_drc",
 		NULL,
 		aud_drc,
 		 "aud_drc -  enable or disable audio DRC feature (only for AC3) \
 		\n usage: aud_drc   <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_drc 1 \n"},
 	{	
 		"aud_equalizer",
 		NULL,
 		aud_equalizer,
 		 "aud_equalizer -  enable or disable audio equalizer \
 		\n usage: aud_equalizer   <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_equalizer 1 \n"},
 	{	
 		"aud_equalizer_conf",
 		NULL,
 		aud_equalizer_conf,
 		 "aud_equalizer_conf -  to configure equalizer parameters \
 		\n usage: aud_equalizer_conf   <type> <blance>\
 		\n   type: the following types are supported:\
 		\n   blance: the value is from -128 to +127\
		\n	  0 - AUD_EQUALIZER_TYPE_POP, \
		\n	  1 - AUD_EQUALIZER_TYPE_THEATRE, \
		\n	  2 - AUD_EQUALIZER_TYPE_ROCK, \
		\n	  3 - AUD_EQUALIZER_TYPE_CLASSICAL, \
 		\n eg:aud_equalizer_conf 0 \n"},
 	{	
 		"aud_mixer",
 		NULL,
 		aud_mixer,
 		 "aud_mixer -  enable or disable mixer feature \
 		\n usage: aud_mixer   <enable/disable>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:aud_mixer 1 \n"},
 	{	
 		"aud_mixer_conf",
 		NULL,
 		aud_mixer_conf,
 		 "aud_mixer_conf -   to configure mixer parameters \
 		\n usage: aud_mixer_conf  <level> <samplerate>\
		\n  level: the value is from 0 to 100 \
		\n  samplerate: pls follow the below values \
		\n	  0 - AUD_SAMPLE_RATE_96KHZ \
		\n	  1 - AUD_SAMPLE_RATE_88_200KHZ \
		\n	  2 - AUD_SAMPLE_RATE_64KHZ \
		\n	  3 - AUD_SAMPLE_RATE_48KHZ \
		\n	  4 - AUD_SAMPLE_RATE_44_100KHZ \
		\n	  5 - AUD_SAMPLE_RATE_32KHZ \
		\n	  6 - AUD_SAMPLE_RATE_24KHZ \
		\n	  7 - AUD_SAMPLE_RATE_22_050KHZ \
		\n	  8 - AUD_SAMPLE_RATE_16KHZ \
		\n	  9 - AUD_SAMPLE_RATE_12KHZ \
		\n	  10 - AUD_SAMPLE_RATE_11_025KHZ \
		\n	  11 - AUD_SAMPLE_RATE_8KHZ \
 		\n eg:aud_mixer_conf 0 0 \n"},
 	{	
 		"aud_mixer_write",
 		NULL,
 		aud_mixer_write,
 		 "aud_mixer_write -   to mix the PCM data with the current audio stream \
 		\n usage: aud_mixer_write   <pcm filename>	\
 		\n eg:aud_mixer_write test.aud \n"},
};
