#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
 
#include "csvid.h"
#include "cstvout.h"
#include "csosd.h"

CSVID_HANDLE vid_handler;
static int pfm_is_played;
pthread_t thread_vid_play;

static void *_play_stream(void *param)
{
	int read_len = 0;
	unsigned char  read_buf[4096+100];
	unsigned int bufsize = 0;
	FILE*   vid_file=NULL;

	vid_file = fopen(param,"rb");
	if(vid_file == NULL) {
		printf("======>file open failed\n");
		return NULL;
	}

	fseek(vid_file,0L, SEEK_SET);
	if ((read_len=fread(read_buf,1,4096,vid_file))!=4096) {
		fseek(vid_file,0L, SEEK_SET);
		printf("read_len = %d ,===no data \n",read_len);
	}

	while(1) {
		if (!pfm_is_played)
			break;

		if(CSAPI_SUCCEED == CSVID_WritePFMData(vid_handler, read_buf, read_len)){
			if ((read_len=fread(read_buf,1,4096,vid_file))!= 4096) {
				if (read_len == 0)
					//break;
					fseek(vid_file,0L, SEEK_SET);
					
			} 
			//CSVID_GetPFMBufferSize(vid_handler, &bufsize);
			//printf("bufsize = %d\n",bufsize);
		}
	}

	pfm_is_played = 0;
	CSVID_PFMClose(vid_handler);
	CSVID_Stop(0);
	CSVID_Open(0);
	return NULL;
}

static void vid_pfm_play(int argc, char **argv)
{
	CSTVOUT_HANDLE tvout_handler;
	CSOSD_HANDLE osd_handler;
	CSVID_Rect src_rect = { 0, 720, 0, 576 };
	CSVID_Rect dst_rect = { 0, 720, 0, 576 };
	int type;

	if (argc < 3) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	if (pfm_is_played) {
		printf("Testools: pfm_is_played already played!\n");	
		return;
	}

	//disable osd
	osd_handler = CSOSD_Open(OSD_LAYER_0);
	if(!osd_handler) {
		printf("open osd device Failed\n");
		return;
	}
	CSOSD_Disable(osd_handler);

	//set tvout mode
	tvout_handler = CSTVOUT_Open(0);
	if(!tvout_handler) {
		printf("open tvout device Failed\n");
		return;
	}
	CSTVOUT_SetMode(tvout_handler,TVOUT_MODE_576I);

	//open video
	vid_handler=CSVID_Open(VID_DEV_0);
	if(!vid_handler) {
		printf("open video error\n");
		return;
	} 

	//video decoder set
	CSVID_SetOutputPostion(vid_handler, &src_rect, &dst_rect);
	CSVID_SetOutputAlpha(vid_handler, 0x80);
	type = atoi(argv[2]);
	printf("stream type is : %d\n", type);
	CSVID_SetStreamType(vid_handler, type);
	CSVID_SetInputMode(vid_handler, VID_INPUT_STILLPIC);
	CSVID_SetDecoderMode(vid_handler, VID_FRAME_SP);
	CSVID_WaitSync(vid_handler, 0);
	CSVID_DisablePTSSync(vid_handler);

	CSVID_Play(vid_handler);
	CSVID_PFMOpen(vid_handler);

	if (0 != pthread_create(&thread_vid_play, NULL, 
				(void *)_play_stream, argv[1])) {
		printf("Error: pthread_create.!\n");
		return;
	}
	pfm_is_played = 1;
}

static void vid_pfm_stop(int argc, char **argv)
{
	CSAPI_RESULT result;

	if (!pfm_is_played) {
		printf("pfm is not played, run <vid_pfm_play> first!\n");
		return;
	}

	result = CSVID_Stop(vid_handler);
	if (result < 0) {
		printf("Error: CSAUD_Stop Error.!\n");
		return;
	} 

	pfm_is_played = 0;
	if (pthread_join(thread_vid_play, NULL) < 0) {
		printf("ERROR, pthread_join");
		return;
	}
}

static void vid_input_mode(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSVID_INPUT mode;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	mode = atoi(argv[1]);
	result = CSVID_SetInputMode(vid_handler, mode);
	if (result < 0) {
		printf("Error: CSVID_SetInputMode Enable Error.!\n");
		return;
	}
}

static void vid_decoder_mode(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSVID_DECODING_MOD mode;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	mode = atoi(argv[1]);
	result = CSVID_SetDecoderMode(vid_handler, mode);
	if (result < 0) {
		printf("Error: CSVID_SetDecoderMode Enable Error.!\n");
		return;
	}
}

static void vid_get_seqheader(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSVID_SequenceHeader hdr;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	result = CSVID_GetSequenceHeader(vid_handler, &hdr);
	if (result < 0) {
		printf("Error: CSVID_GetSequenceHeader Enable Error.!\n");
		return;
	}

	printf(" Seqheader, width: %u, height: %u, frame_rate: %u\n", 
			hdr.w, hdr.h, hdr.frame_rate);
}

static void vid_ptssync(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSVID_EnablePTSSync(vid_handler);
	else 		result = CSVID_DisablePTSSync(vid_handler);
	if (result < 0) {
		printf("Error: CSVID_EnablePTSSync Enable Error.!\n");
		return;
	}
}

static void vid_get_pts(int argc, char **argv)
{
	CSAPI_RESULT result;
	long long pts;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	result = CSVID_GetPTS(vid_handler, &pts);
	if (result < 0) {
		printf("Error: CSVID_GetPTS Enable Error.!\n");
		return;
	}

	printf(" CSVID_GetPTS: pts value is: %lld\n", pts);
}


static void vid_output(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	vid_handler = CSVID_Open(0);
	if (vid_handler < 0) {
		printf("open video error\n");
		return;
	}

	enable = atoi(argv[1]);
	if (enable) 	result = CSVID_EnableOutput(vid_handler);
	else 		result = CSVID_DisableOutput(vid_handler);
	if (result < 0) {
		printf("Error: CSVID_EnableOutput Enable Error.!\n");
		return;
	} 
}

static void vid_alpha(int argc, char **argv)
{
	CSAPI_RESULT result;
	int alpha;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	alpha = atoi(argv[1]);
	if (alpha < 0 || alpha > 255)
		printf("\n invalid parameters, see help! \n");

	result = CSVID_SetOutputAlpha(vid_handler, alpha);
	if (result < 0) {
		printf("Error: CSVID_SetOutputAlpha.!\n");
		return;
	}
}

static void vid_position(int argc, char **argv)
{

	CSVID_HANDLE vid_handler;
	CSAPI_RESULT result;
	CSVID_Rect src_rect, dst_rect;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 8) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	src_rect.left = atoi(argv[1]);
	src_rect.right = atoi(argv[2]);
	src_rect.top = atoi(argv[3]);
	src_rect.bottom = atoi(argv[4]);
	dst_rect.left = atoi(argv[5]);
	dst_rect.right = atoi(argv[6]);
	dst_rect.top = atoi(argv[7]);
	dst_rect.bottom = atoi(argv[8]);

	result = CSVID_SetOutputPostion(vid_handler, &src_rect, &dst_rect);
	if (result < 0) {
		printf("Error: CSVID_SetOutputPostion.!\n");
		return;
	}
}

static void vid_skip_error(int argc, char **argv)
{
	CSAPI_RESULT result;
	int enable;

        if (!pfm_is_played) {
                printf("pfm is not played, run <vid_pfm_play> first!\n");      
                return;
        }

	if (argc < 2) {
		printf("\n invalid parameters, see help! \n");
		return;
	}

	enable = atoi(argv[1]);
	result = CSVID_SetErrorSkipMode(vid_handler, enable);
	if (result < 0) {
		printf("Error: CSVID_SetErrorSkipMode.!\n");
		return;
	}
}

static struct cmd_t vid_tt[] = {
 	{	
 		"vid_pfm_play",
 		NULL,
 		vid_pfm_play,
 		 "vid_pfm_play - inject some video data into video decoder, and play it.. \
 		\n usage: vid_pfm_play <filename> <type> [enable PFM notify] \
 		\n eg:vid_pfm_play  test.ts 0\n"},
 	{	
 		"vid_pfm_stop",
 		NULL,
 		vid_pfm_stop,
 		 "vid_pfm_stop - stop PFM play. \
 		\n usage: vid_pfm_stop\
 		\n eg:vid_pfm_stop\n"},
 	{	
 		"vid_input_mode",
 		NULL,
 		vid_input_mode,
 		 "vid_input_mode - set input mode of video decoder. \
 		\n usage: vid_input_mode  [input mod]\
		\n 	   0 - demux \
		\n         1 - file \
 		\n eg:vid_input_mode 1\n"},
 	{	
 		"vid_decoder_mode",
 		NULL,
 		vid_decoder_mode,
 		 "vid_decoder_mode - et decoder mode .\
 		\n usage: vid_decoder_mode [decoding mod] \
 		\n	  0 - VID_FRAME_ANY /* all frames */ \
 		\n	  1 - VID_FRAME_IP     /* I- and P-frames only*/ \
 		\n	  2 - VID_FRAME_I       /* I-frames only */ \
 		\n	  3 - VID_FRAME_SP   /* still picture */ \
 		\n eg:vid_decoder_mode 0\n"},
 	{	
 		"vid_get_seqheader",
 		NULL,
 		vid_get_seqheader,
 		 "vid_get_seqheader - get sequence header (stream information), it return picture size and frame rate. \
 		\n usage: vid_get_seqheader \
 		\n eg:vid_get_seqheader\n"},
 	{	
 		"vid_ptssync",
 		NULL,
 		vid_ptssync,
 		 "vid_ptssync -  to enable / disable PTS sync \
 		\n usage: vid_ptssync <en/dis>\
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:vid_ptssync 1\n"},
 	{	
 		"vid_get_pts",
 		NULL,
 		vid_get_pts,
 		 "vid_get_pts - to get PTS from streaming \
 		\n usage: vid_get_pts \
 		\n eg:vid_get_pts\n"},
 	{	
 		"vid_output",
 		NULL,
 		vid_output,
 		 "vid_output - to get PTS from streaming \
 		\n usage: vid_output <open/close> \
 		\n  	  1 - open\
 		\n  	  0 - close \
 		\n eg:vid_output 1\n"},
 	{	
 		"vid_alpha",
 		NULL,
 		vid_alpha,
 		 "vid_alpha - to set alpha value of Vide layer\
 		\n usage: vid_alpha <val> \
 		\n eg:vid_alpha 100\n"},
 	{	
 		"vid_position",
 		NULL,
 		vid_position,
 		 "vid_position - to set source and destination window. \
 		\n usage: vid_position [src rect, dst rect]. \
		\n	  0 - left of source window \
		\n	  1 - top of source window \
		\n	  2 - right of source window \
		\n	  3 - bottom of source window \
		\n	  4 - left of destination win \
		\n	  5 - top of destination win \
		\n	  6 - right of destination win \
		\n	  7 - bottom of destination win \
 		\n eg:vid_position 0 0 320 240 0 0 720 576\n"},
 	{	
 		"vid_skip_error",
 		NULL,
 		vid_skip_error,
 		 "vid_skip_error - to enable / disable video decoder to skip error (not output). \
 		\n usage: vid_skip_error <en/dis> \
 		\n  	  1 - enable\
 		\n  	  0 - disable\
 		\n eg:vid_skip_error 1\n"},
};
