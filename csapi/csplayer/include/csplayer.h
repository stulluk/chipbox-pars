#ifndef __CSPLAYER_H__
#define __CSPLAYER_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CSPLAYER_HANDLE;

typedef enum
{
        PLAY_INIT,
        PLAY_RUN,
        PLAY_PAUSE,
        PLAY_STOP,
        PLAY_FF_2X,
        PLAY_FF_4X,        
        PLAY_FF_6X,        
        PLAY_FF_8X,        
        PLAY_FF_16X,        
        PLAY_REW_2X,
        PLAY_REW_4X,
        PLAY_REW_8X,
        PLAY_REW_16X

} CSPLAYER_STATUS;

/*
typedef enum 
{
	PLAY_FF_1X,
	PLAY_FF_2X,
	PLAY_FF_4X,
	PLAY_FF_8X,
	PLAY_FF_16X

} CSPLAYER_PLAY_FF;

typedef enum 
{
	PLAY_REW_1X,
	PLAY_REW_2X,
	PLAY_REW_4X,
	PLAY_REW_8X,
	PLAY_REW_16X

} CSPLAYER_PLAY_REW;
*/

typedef enum 
{
    PLAY_SPEED_R16X= -16,
    PLAY_SPEED_R8X = -8,    
    PLAY_SPEED_R4X = -4,    
    PLAY_SPEED_R2X = -2,
	PLAY_SPEED_1X  = 1,
	PLAY_SPEED_2X  = 2,
	PLAY_SPEED_4X  = 4,
	PLAY_SPEED_8X  = 8,
	PLAY_SPEED_16X = 16
} CSPLAYER_PLAY_SPEED;

typedef enum 
{
	PLAY_SEEK_START,
	PLAY_SEEK_END,
	PLAY_SEEK_CUR
} CSPLAYER_PLAY_SEEK;

typedef struct tagPLAYER_Status
{
	CSPLAYER_STATUS stat;
	int cur_pos;
	unsigned int elapsed_time;
	unsigned int duration;
} CSPLAYER_Status;

typedef struct tagPLAYER_PlayParams 
{
	char filename[128];
	
	unsigned int play_speed;
	unsigned int start_pos;
   unsigned int duration;
   
	unsigned int loop_play_flags; /* 0 - play one time. 1 - loop play. */

	void *vid_handle;
	void *aud_handle;
	void (*notifier)( CSPLAYER_STATUS status );

} CSPLAYER_PlayParams;

typedef struct tagPLAYER_InitParams 
{
	CSDEMUX_HANDLE chl_handle;

} CSPLAYER_InitParams;

CSPLAYER_HANDLE CSPLAYER_Init(CSPLAYER_InitParams *params);
CSAPI_RESULT    CSPLAYER_Term(CSPLAYER_HANDLE handle);
CSAPI_RESULT    CSPLAYER_Start(CSPLAYER_HANDLE handle, CSPLAYER_PlayParams *params );
CSAPI_RESULT    CSPLAYER_Stop(CSPLAYER_HANDLE handle);
CSAPI_RESULT    CSPLAYER_Pause(CSPLAYER_HANDLE handle);
CSAPI_RESULT    CSPLAYER_Resume(CSPLAYER_HANDLE handle);
CSAPI_RESULT    CSPLAYER_FF(CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SPEED ff_params);
CSAPI_RESULT    CSPLAYER_REW(CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SPEED rew_params);
CSAPI_RESULT    CSPLAYER_Seek(CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SEEK flag, unsigned int offset);
CSAPI_RESULT    CSPLAYER_SetSpeed(CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SPEED speed);
CSAPI_RESULT    CSPLAYER_GetTime(CSPLAYER_HANDLE handle, unsigned int *secs);
CSAPI_RESULT    CSPLAYER_GetStatus(CSPLAYER_HANDLE handle, CSPLAYER_Status *status);

#ifdef __cplusplus
}
#endif

#endif /* end of __CSPLAYER_H__ */

