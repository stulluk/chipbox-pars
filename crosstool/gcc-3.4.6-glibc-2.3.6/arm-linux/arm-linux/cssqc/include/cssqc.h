#ifndef __CSAPI_SQC_H__
#define __CSAPI_SQC_H__

#include "global.h"
#include "../../cstvout/include/cstvout.h"
#include "../../csvid/include/csvid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RATIO_16_9_ON_4_3 = 0,
	RATIO_4_3_ON_4_3,
	RATIO_16_9_ON_16_9,
	RATIO_4_3_ON_16_9
} CSSQC_VIDEO_ASPECTRATIO;

typedef enum {
	SQC_NORMAL = 0,
	SQC_PAN_SCAN,
	SQC_LETTERBOX
} CSSQC_VIDEO_ASPECTMODE;

typedef enum {
	CSSQC_MODE_PAL = 0,
	CSSQC_MODE_NTSC,
	CSSQC_MODE_576P,
	CSSQC_MODE_480P,
	CSSQC_MODE_720P50,
	CSSQC_MODE_720P60,
	CSSQC_MODE_1080I25,
	CSSQC_MODE_1080I30 
} CSSQC_VIDEO_OUTPUTMODE;


CSAPI_RESULT  CSSQC_VIDEO_SysSet(CSVID_HANDLE video_handle, 
				CSTVOUT_HANDLE tvout_handle, 
				CSSQC_VIDEO_ASPECTRATIO aspect_ratio, 	/* not used */
				CSSQC_VIDEO_OUTPUTMODE output_mode,
				CSSQC_VIDEO_ASPECTMODE aspect_mode);

CSAPI_RESULT  CSSQC_VIDEO_SysGet(CSVID_HANDLE video_handle,
				CSTVOUT_HANDLE tvout_handle, 
				CSSQC_VIDEO_ASPECTRATIO *aspect_ratio, 
				CSSQC_VIDEO_ASPECTMODE *aspect_mode);

CSAPI_RESULT  CSSQC_APIINFO_SysGet(unsigned char *version, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
