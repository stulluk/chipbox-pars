#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "cssqc.h"

int SQC_VERSION(int argc, char *argv[])
{
    unsigned char version[20];
    
    memset(version, 0, sizeof(version));

    if(CSAPI_SUCCEED != CSSQC_APIINFO_SysGet(version, sizeof(version) - 1))
	printf("	*** get API version error!\n");

    printf("%s\n",version);

    return 0;
}
  
int SQC_VID_SYSGET(int argc, char *argv[])
{
    CSVID_HANDLE vid_handle;
    CSDF_HANDLE df_handle;
    CSTVOUT_HANDLE tvout_handle;
    CSSQC_VIDEO_ASPECTRATIO aspect_ratio;
    CSSQC_VIDEO_ASPECTMODE aspect_mode;

    vid_handle = CSVID_Open(VID_DEV_0);
    df_handle = CSDF_Open();
    tvout_handle = CSTVOUT_Open();

    if(CSAPI_SUCCEED != CSSQC_VIDEO_SysGet(vid_handle, df_handle, tvout_handle, &aspect_ratio, &aspect_mode)) {
	printf("	*** get sys video info error!\n");
	goto leave;
    }

    switch(aspect_ratio) {
	case RATIO_4_3_ON_16_9:
	    printf("RATIO_4_3_ON_16_9");
	    break;
	case RATIO_4_3_ON_4_3:
	    printf("RATIO_4_3_ON_4_3");
	    break;
	case RATIO_16_9_ON_4_3:
	    printf("RATIO_16_9_ON_4_3");
	    break;
	case RATIO_16_9_ON_16_9:
	    printf("RATIO_16_9_ON_16_9");
	    break;
	default:
	    printf("	*** video aspect ratio error!\n");
	    break;
    }

    switch(aspect_mode) {
	case SQC_NORMAL:
	    printf("	NORMAL\n");
	    break;
	case SQC_PAN_SCAN:
	    printf("	PAN_SCAN\n");
	    break;
	case SQC_LETTERBOX:
	    printf("	LETTERBOX\n");
	    break;
	default:
	    printf("	*** video aspect ratio mode error!\n");
	    break;
    }

leave:
/*--------------------------------------------------
*     CSVID_Close(vid_handle);
*     CSDF_Close(df_handle);
*     CSTVOUT_Close(tvout_handle);
*--------------------------------------------------*/

    return 0;
}

int SQC_VID_SYSSET(int argc, char *argv[])
{
    CSVID_HANDLE vid_handle;
    CSDF_HANDLE df_handle;
    CSTVOUT_HANDLE tvout_handle;
    CSSQC_VIDEO_OUTPUTMODE outputmode;
    CSSQC_VIDEO_ASPECTMODE aspectmode;
    int i;

    if (argc < 3) { printf(" invalid parameters, see help! \n"); return -1; }

    vid_handle = CSVID_Open(VID_DEV_0);
    df_handle = CSDF_Open();
    tvout_handle = CSTVOUT_Open();

    i = atoi(argv[1]);
    if(i == 0)
	outputmode = CSSQC_MODE_PAL;
    else if(i == 1)
	outputmode = CSSQC_MODE_NTSC;
    else if(i == 2)
	outputmode = CSSQC_MODE_576P;
    else if(i == 3)
	outputmode = CSSQC_MODE_480P;
    else if(i == 4)
	outputmode = CSSQC_MODE_720P50;
    else if(i == 5)
	outputmode = CSSQC_MODE_720P60;
    else if(i == 6)
	outputmode = CSSQC_MODE_1080I25;
    else if(i == 7)
	outputmode = CSSQC_MODE_1080I30;
    else{
	printf(" invalid parameters, see help! \n"); 
	goto leave; 
    }

    i = atoi(argv[2]);
    if(i == 0)
	aspectmode = SQC_NORMAL;
    else if(i == 1)
	aspectmode = SQC_PAN_SCAN;
    else if(i == 2)
	aspectmode = SQC_LETTERBOX;
    else{
	printf(" invalid parameters, see help! \n"); 
	goto leave; 
    }

    if(CSAPI_SUCCEED != CSSQC_VIDEO_SysSet(vid_handle, df_handle, tvout_handle, 0/*not used*/, outputmode, aspectmode)) {
	printf("	*** set sys video info error!\n");
	goto leave;
    }

leave:
/*--------------------------------------------------
*     CSVID_Close(vid_handle);
*     CSDF_Close(df_handle);
*     CSTVOUT_Close(tvout_handle);
*--------------------------------------------------*/

    return 0;
}

static struct cmd_t cssqc_tt[] = {
	{
 	 "sqc_vidset",
 	 NULL,
 	 SQC_VID_SYSSET,
 	 "sqc_vidset - set video output mode and aspect-ratio mode \
 	\n    usage: sqc_set <output mode> <aspect-ratio mode> \
 	\n\n      output mode: \
 	\n             0 - SQC_MODE_PAL \
 	\n             1 - SQC_MODE_NTSC \
 	\n             2 - SQC_MODE_576P \
 	\n             3 - SQC_MODE_480P \
 	\n             4 - SQC_MODE_720P50 \
 	\n             5 - SQC_MODE_720P60 \
 	\n             6 - SQC_MODE_1080I25 \
 	\n             7 - SQC_MODE_1080I30 \
 	\n\n      aspect-ratio mode: \
 	\n             0 - NORMAL \
 	\n             1 - PAN_SCAN \
 	\n             2 - LETTERBOX \
 	\n\n       eg: sqc_vidset 0 1 \n"},
 
 	{
 	 "sqc_vidget",
 	 NULL,
 	 SQC_VID_SYSGET,
 	 "sqc_vidget - get video aspect ratio and aspect-ratio mode \
 	\n    usage: sqc_vidget \
 	\n\n       eg: sqc_vidget \n"},
 
 	{
 	 "sqc_version",
 	 NULL,
 	 SQC_VERSION,
 	 "sqc_version - get CSAPI version information \
 	\n    usage: sqc_version \
 	\n\n       eg: sqc_version \n"},
};
