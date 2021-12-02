#ifndef _CSMPR_AV_H_
#define _CSMPR_AV_H_

#include "av_zapping.h"

CSMPR_RESULT CSMPR_OpenDemuxCH1( void );
CSMPR_RESULT CSMPR_CloseDemuxCH1( void );
CSMPR_RESULT CSMPR_PlayAudio( tCS_AV_PlayParams ProgramInfo );
CSMPR_RESULT CSMPR_PlayVideo( tCS_AV_PlayParams ProgramInfo );
CSMPR_RESULT CSMPR_StopAudio( void );
CSMPR_RESULT CSMPR_StopVideo( void );

#endif
