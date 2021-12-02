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

#include "cssqc.h"

#define SQCABS(x)              (((x)<0) ? -(x) : (x))

#define PIC_HIGH_RANGE		50

#define SQC_PAL_HEIGHT		576
#define SQC_NTSC_HEIGHT		480
#define SQC_720P_HEIGHT		720
#define SQC_1080I_HEIGHT	1080
#define SQC_480P_HEIGHT		480
#define SQC_576P_HEIGHT		576

#define div(x) ((x) /16)
#define mod(x) ((x) % 16)

typedef enum {
    ASPECT_NONE,
    ASPECT_4_3,
    ASPECT_16_9
} SQC_ASPECT; 

static CSSQC_VIDEO_ASPECTMODE cur_mode = SQC_NORMAL;

/***********************************************
		00,		// - unknown
		23,		// - 23.97 fps
		24,		// - 24.00 fps
		25,		// - 25.00 fps
		29,		// - 29.97 fps
		30,		// - 30.00 fps
		50,		// - 50.00 fps
		59,		// - 59.94 fps
		60		// - 60.00 fps
*********************************************/

static CSAPI_RESULT  GetTVOUTModeFromVID(int vid_height, int framerate, CSTVOUT_MODE *mode)
{
    if(vid_height <= 0 || framerate <= 0)
	return CSAPI_FAILED;

    /*PAL*/
    if((SQCABS(vid_height - SQC_PAL_HEIGHT) < PIC_HIGH_RANGE) && (framerate <= 25)) {
	*mode = TVOUT_MODE_576I;
    }

    /*NTSC*/
    else if((SQCABS(vid_height - SQC_NTSC_HEIGHT) < PIC_HIGH_RANGE) && ((framerate == 29) || (framerate == 30))) {
	*mode = TVOUT_MODE_480I;
    }	

    /*1080i 30fps*/
    else if((SQCABS(vid_height - SQC_1080I_HEIGHT) < PIC_HIGH_RANGE) && ((framerate == 29) || (framerate == 30))) {
	*mode = TVOUT_MODE_1080I30;
    }	

    /*1080i 25fps*/
    else if((SQCABS(vid_height - SQC_1080I_HEIGHT) < PIC_HIGH_RANGE) && (framerate <= 25)) {
	*mode = TVOUT_MODE_1080I25;
    }	

    /*480P 30fps*/
    else if((SQCABS(vid_height - SQC_NTSC_HEIGHT) < PIC_HIGH_RANGE) && ((framerate == 59) || (framerate == 60))) {
	*mode = TVOUT_MODE_480P;
    }	

    /*576p 25fps*/
    else if((SQCABS(vid_height - SQC_PAL_HEIGHT) < PIC_HIGH_RANGE) && (framerate >= 29)) {
	*mode = TVOUT_MODE_576P;
    }	

    /*720p 30fps*/
    else if((SQCABS(vid_height - SQC_720P_HEIGHT) < PIC_HIGH_RANGE) && (framerate >= 29)) {
	*mode = TVOUT_MODE_720P60;
    }	

    /*720p 25fps*/
    else if((SQCABS(vid_height - SQC_720P_HEIGHT) < PIC_HIGH_RANGE) && (framerate <= 25)) {
	*mode = TVOUT_MODE_720P50;
    }	

    /*default*/
    else {
	return CSAPI_FAILED;
    }

    return CSAPI_SUCCEED;
}

static CSAPI_RESULT  SQCMode2TVOutMode(CSSQC_VIDEO_OUTPUTMODE sqcmode, CSTVOUT_MODE *tvoutmode)
{
    	switch(sqcmode){
	    case CSSQC_MODE_PAL:
		*tvoutmode = TVOUT_MODE_576I;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_NTSC:
		*tvoutmode = TVOUT_MODE_480I;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_576P:
		*tvoutmode = TVOUT_MODE_576P;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_480P:
		*tvoutmode = TVOUT_MODE_480P;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_720P50:
		*tvoutmode = TVOUT_MODE_720P50;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_720P60:
		*tvoutmode = TVOUT_MODE_720P60;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_1080I25:
		*tvoutmode = TVOUT_MODE_1080I25;
		return CSAPI_SUCCEED;

	    case CSSQC_MODE_1080I30:
		*tvoutmode = TVOUT_MODE_1080I30;
		return CSAPI_SUCCEED;

	    default:
		return CSAPI_FAILED;
	}
}

static SQC_ASPECT getaspect(CSTVOUT_MODE mode)
{
    switch(mode){
	case TVOUT_MODE_576I:
	case TVOUT_MODE_480I:
	case TVOUT_MODE_576P:
	case TVOUT_MODE_480P:
	    return ASPECT_4_3;

	case TVOUT_MODE_720P50:
	case TVOUT_MODE_720P60:
	case TVOUT_MODE_1080I25:
	case TVOUT_MODE_1080I30:
	    return ASPECT_16_9;

	default:
	    return ASPECT_NONE;
    }
}

static CSAPI_RESULT  SetAspectMode(CSVID_HANDLE vid_handle, CSTVOUT_HANDLE tvout_handle, \
				CSSQC_VIDEO_ASPECTMODE aspectmode, CSSQC_VIDEO_OUTPUTMODE outputmode)
{
	int tempcrop = 0;
	CSTVOUT_MODE cur_tvmode;	/* current tvout mode */
        CSTVOUT_MODE src_tvmode;	/* tvout mode in video decoded stream */
        CSTVOUT_MODE aim_tvmode;	/* tvout mode to be set */
	CSVID_Rect src, dst;
	CSVID_SequenceHeader hdr;

	if(CSAPI_SUCCEED != SQCMode2TVOutMode(outputmode, &aim_tvmode)){
        printf("qaaaaaaaaaaa\n");
        return CSAPI_FAILED;
       }
	if(CSAPI_SUCCEED != CSTVOUT_GetMode(tvout_handle, &cur_tvmode)){
        printf("bbbbbbbbbbbb\n");
        return CSAPI_FAILED;
       }

	if(CSAPI_SUCCEED != CSVID_GetSequenceHeader(vid_handle, &hdr)){
        printf("ccccccccccccc\n");
        return CSAPI_FAILED;
       }

	if(CSAPI_SUCCEED != GetTVOUTModeFromVID(hdr.h, hdr.frame_rate, &src_tvmode)){
        printf("dddddddddddddd\n");
        return CSAPI_FAILED;
       }

	if(aim_tvmode != cur_tvmode){
	    if(CSAPI_SUCCEED != CSTVOUT_SetMode(tvout_handle, aim_tvmode)){
        printf("eeeeeeeeeeeee\n");
        return CSAPI_FAILED;
       }
	}

	/* set src input rectangle */
	if((SQC_PAN_SCAN == aspectmode) && \
		getaspect(aim_tvmode) == ASPECT_4_3 && getaspect(src_tvmode) == ASPECT_16_9) {

	    /* a little wider, so cut from left and right to fit 4:3 */
	    tempcrop = (hdr.w - (hdr.h * 4 / 3)) / 2;
	    if (mod(tempcrop) > 10) {
		src.left = 16 * (div(tempcrop) + 1);
	    }
	    else {
		src.left = 16 * div(tempcrop);
	    }
	    src.right = hdr.w - src.left;
	    src.top = 0;
	    src.bottom = hdr.h;

	    cur_mode = SQC_PAN_SCAN;
	}
	else if((SQC_PAN_SCAN == aspectmode) && \
		getaspect(aim_tvmode) == ASPECT_16_9 && getaspect(src_tvmode) == ASPECT_4_3) {

	    /* a little higher, so cut from top and bottom to fit 16:9*/
	    tempcrop = (hdr.h - (hdr.w * 9 / 16)) / 2;
	    if (mod(tempcrop) > 10) {
		src.top = 16 * (div(tempcrop) + 1);
	    }
	    else {
		src.top = 16 * div(tempcrop);
	    }
	    src.bottom = hdr.h - src.top;
	    src.left = 0;
	    src.right = hdr.w;

	    cur_mode = SQC_PAN_SCAN;
	}
	else //if((SQC_LETTERBOX == aspectmode) || \
		getaspect(aim_tvmode) == getaspect(src_tvmode))  /* SQC_NORMAL(4:3 stream on 4:3 TV or 16:9 stream on 16:9 TV)  aspectmode */
	{
	    src.left = 0;
	    src.right = hdr.w;
	    src.top = 0;
	    src.bottom = hdr.h;

	    cur_mode = SQC_NORMAL;	/* restore mode, maybe modified later if SQC_LETTERBOX */
	}

	/* set dst output rectangle */
	switch (aim_tvmode) {
	    case TVOUT_MODE_480I:
	    case TVOUT_MODE_480P:
		dst.left = 0;
		dst.right = 720;
		dst.top = 0;
		dst.bottom = 480;
		if((SQC_LETTERBOX == aspectmode) && \
			getaspect(aim_tvmode) == ASPECT_4_3 && getaspect(src_tvmode) == ASPECT_16_9) {

		    tempcrop = (480 - (720 * 9 / 16)) / 2;
		    if (mod(tempcrop) > 10) {
			dst.top = 16 * (div(tempcrop) + 1);
		    }
		    else {
			dst.top = 16 * div(tempcrop);
		    }
		    dst.bottom = 480 - dst.top;

		    cur_mode = SQC_LETTERBOX;
		}

		break;

	    case TVOUT_MODE_576I:
	    case TVOUT_MODE_576P:
		dst.left = 0;
		dst.right = 720;
		dst.top = 0;
		dst.bottom = 576;
		if((SQC_LETTERBOX == aspectmode) && \
			getaspect(aim_tvmode) == ASPECT_4_3 && getaspect(src_tvmode) == ASPECT_16_9) {

		    tempcrop = (576 - (720 * 9 / 16)) / 2;
		    if (mod(tempcrop) > 10) {
			dst.top = 16 * (div(tempcrop) + 1);
		    }
		    else {
			dst.top = 16 * div(tempcrop);
		    }
		    dst.bottom = 576 - dst.top;

		    cur_mode = SQC_LETTERBOX;
		}

		break;

	    case TVOUT_MODE_720P50:
	    case TVOUT_MODE_720P60:
		dst.left = 0;
		dst.right = 1280;
		dst.top = 0;
		dst.bottom = 720;
		if((SQC_LETTERBOX == aspectmode) && \
			getaspect(aim_tvmode) == ASPECT_16_9 && getaspect(src_tvmode) == ASPECT_4_3) {

		    tempcrop = (1280 - (720 * 4 / 3)) / 2;
		    if (mod(tempcrop) > 10) {
			dst.left = 16 * (div(tempcrop) + 1);
		    }
		    else {
			dst.left = 16 * div(tempcrop);
		    }
		    dst.right = 1280 - dst.left;

		    cur_mode = SQC_LETTERBOX;
		}

		break;

	    case TVOUT_MODE_1080I25:
	    case TVOUT_MODE_1080I30:
		dst.left = 0;
		dst.right = 1920;
		dst.top = 0;
		dst.bottom = 1080;
		if((SQC_LETTERBOX == aspectmode) && \
			getaspect(aim_tvmode) == ASPECT_16_9 && getaspect(src_tvmode) == ASPECT_4_3) {

		    tempcrop = (1920 - (1080 * 4 / 3)) / 2;
		    if (mod(tempcrop) > 10) {
			dst.left = 16 * (div(tempcrop) + 1);
		    }
		    else {
			dst.left = 16 * div(tempcrop);
		    }
		    dst.right = 1920 - dst.left;

		    cur_mode = SQC_LETTERBOX;
		}

		break;

	    default:
		printf("%s tvmode [%d] not support!\n", __FUNCTION__, aim_tvmode);
		return CSAPI_FAILED;
	}

	return CSVID_SetOutputPostion(vid_handle, &src, &dst);
}


CSAPI_RESULT  CSSQC_VIDEO_SysSet(CSVID_HANDLE vid_handle, 
				CSTVOUT_HANDLE tvout_handle, 
				CSSQC_VIDEO_ASPECTRATIO aspect_ratio, 	/* not used */
				CSSQC_VIDEO_OUTPUTMODE output_mode,
				CSSQC_VIDEO_ASPECTMODE aspect_mode)
{
    	if(vid_handle == NULL /*|| df_handle == NULL*/ || tvout_handle == NULL)
        return  CSAPI_FAILED;

	return SetAspectMode(vid_handle, tvout_handle, aspect_mode, output_mode);
}

CSAPI_RESULT  CSSQC_VIDEO_SysGet(CSVID_HANDLE vid_handle,
				CSTVOUT_HANDLE tvout_handle, 
				CSSQC_VIDEO_ASPECTRATIO *aspect_ratio, 
				CSSQC_VIDEO_ASPECTMODE *aspect_mode)
{
	CSTVOUT_MODE cur_tvmode;	/* current tvout mode */
        CSTVOUT_MODE src_tvmode;	/* tvout mode in video decoded stream */
	CSVID_SequenceHeader hdr;

    	if(vid_handle == NULL /*|| df_handle == NULL*/ || tvout_handle == NULL || \
		aspect_ratio == NULL || aspect_mode == NULL){
        printf("qqqqqqqqqqqqq\n");
        return CSAPI_FAILED;
       }

	*aspect_mode = cur_mode;

	if(CSAPI_SUCCEED != CSTVOUT_GetMode(tvout_handle, &cur_tvmode))
	    return CSAPI_FAILED;

	if(CSAPI_SUCCEED != CSVID_GetSequenceHeader(vid_handle, &hdr))
	    return CSAPI_FAILED;

	if(CSAPI_SUCCEED != GetTVOUTModeFromVID(hdr.h, hdr.frame_rate, &src_tvmode))
	    return CSAPI_FAILED;

	if(getaspect(cur_tvmode) == ASPECT_16_9 && getaspect(src_tvmode) == ASPECT_4_3) {
	    *aspect_ratio = RATIO_4_3_ON_16_9;
	}
	else if(getaspect(cur_tvmode) == ASPECT_4_3 && getaspect(src_tvmode) == ASPECT_4_3) {
	    *aspect_ratio = RATIO_4_3_ON_4_3;
	}
	else if(getaspect(cur_tvmode) == ASPECT_16_9 && getaspect(src_tvmode) == ASPECT_16_9) {
	    *aspect_ratio = RATIO_16_9_ON_16_9;
	}
	else if(getaspect(cur_tvmode) == ASPECT_4_3 && getaspect(src_tvmode) == ASPECT_16_9) {
	    *aspect_ratio = RATIO_16_9_ON_4_3;
	}
	else
	    return CSAPI_FAILED;

	return CSAPI_SUCCEED;
}

static unsigned char *cur_version = "CSAPI 1.0.0";

CSAPI_RESULT  CSSQC_APIINFO_SysGet(unsigned char *version, unsigned int len)
{
    	if(version == NULL)
	    return CSAPI_FAILED;

	if(len > strlen(cur_version))
	    len = strlen(cur_version);

	memcpy((void*)version, (void*)cur_version, len);

	return CSAPI_SUCCEED;
}

