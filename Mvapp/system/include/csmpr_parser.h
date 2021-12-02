#ifndef _PVR_TS_PARSER_H_
#define _PVR_TS_PARSER_H_

typedef struct 
{
	short 		pcr_pid;
	short 		audio_pid;
	short 		video_pid;
	short 		audio_type;
	short 		video_type;
} AVParam_t;


int Get_AudioVideo_PID_from_TSfile( const char *filename, AVParam_t *pAvparam );
CSMPR_RESULT CSMPR_ExtractAvParam( const char *filename, tCS_AV_PlayParams *pProgramInfo );

#endif
