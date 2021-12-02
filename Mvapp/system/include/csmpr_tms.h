#ifndef __CSMPR_TMS_H_
#define __CSMPR_TMS_H_

#include "csmpr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CSMPR_TMS_IDLE = 0,
    CSMPR_TMS_PAUSE,
    CSMPR_TMS_RUN,
    CSMPR_TMS_REPAUSE,
    CSMPR_TMS_FF,
    CSMPR_TMS_REW
} CSMPR_TMS_STATUS;


CSMPR_RESULT CSMPR_TMS_Start( void );
CSMPR_RESULT CSMPR_TMS_Play( void );
CSMPR_RESULT CSMPR_TMS_Pause( void );
CSMPR_RESULT CSMPR_TMS_Resume( void );
CSMPR_RESULT CSMPR_TMS_Forward( void );
CSMPR_RESULT CSMPR_TMS_Rewind( void );
CSMPR_RESULT CSMPR_TMS_Stop( void );
CSMPR_TMS_STATUS CSMPR_TMS_GetStatus( void );

#ifdef __cplusplus
}
#endif

#endif

