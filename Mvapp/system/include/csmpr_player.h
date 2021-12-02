#ifndef _CSMPR_PLAYER_H_
#define _CSMPR_PLAYER_H_

#include "linuxos.h"
#include "global.h"
#include "csmpr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CSMPR_PLAY_INIT = 0,
    CSMPR_PLAY_STOP,
    CSMPR_PLAY_RUN,
    CSMPR_PLAY_PAUSE,
    CSMPR_PLAY_FF_2X,
    CSMPR_PLAY_FF_4X,        
    CSMPR_PLAY_FF_8X,        
    CSMPR_PLAY_FF_16X,        
    CSMPR_PLAY_REW_2X,
    CSMPR_PLAY_REW_4X,
    CSMPR_PLAY_REW_8X,
    CSMPR_PLAY_REW_16X

} CSMPR_PLAYER_STATUS;


typedef enum
{
    CSMPR_PLAY_NORMAL = 0,
    CSMPR_PLAY_TIMESHIFT,
} CSMPR_PLAYER_MODE;


typedef struct 
{
    unsigned long   duration;
    unsigned long   bitrate;
    int             total_file_no; 
    //long long		first_file_size;
    //long long		total_size;
    long long       first_file_size;
    long long       total_size;
} CSMPR_FILEINFO;


typedef struct tagPLAYER_OBJ 
{
    // Status:
    CSMPR_PLAYER_STATUS Status;    
    
    // Handles:    
    int             play_pid;
    pthread_mutex_t player_mutex;
    pthread_t       player_thread;
    CSDEMUX_HANDLE  chl_handle;
    CSVID_HANDLE    vid_handle;
    CSAUD_HANDLE    aud_handle;
    void           (*notifier)( CSMPR_PLAYER_STATUS status );        

    // File info:
    char            filename[128];
    int             play_fd;
    unsigned long   duration;
    unsigned long   bitrate;
    int             total_file_no;
    long long       total_size;
    
    // position:
    long long       curr_file_size;
    int             curr_file_no;
    long long       curr_file_pos;
    long long       position;
	long long 		temp_position;
    
} CSMPR_PLAYER_OBJ;

CSMPR_RESULT CSMPR_Player_Init( void );
CSMPR_RESULT CSMPR_Player_Term( void );
CSMPR_RESULT CSMPR_Player_Start( char* filename , long long Start_Position);
CSMPR_RESULT CSMPR_Player_Stop( void );
CSMPR_RESULT CSMPR_Player_Pause( void );
CSMPR_RESULT CSMPR_Player_Resume( void );
CSMPR_RESULT CSMPR_Player_Forward( void );
CSMPR_RESULT CSMPR_Player_Rewind( void );
CSMPR_RESULT CSMPR_Player_GetElapsedTime( time_t *pElapsedTime );
CSMPR_RESULT CSMPR_Player_GetDuration( time_t *pDuration );
CSMPR_RESULT CSMPR_Player_GetPosition( U32 *nPosition );
CSMPR_RESULT CSMPR_Player_GetFileName_Pointer( char **pFileName);
CSMPR_RESULT CSMPR_Player_GetFileName( char *pFileName);
CSMPR_RESULT CSMPR_Player_GetTrickSpeed( int *trickspeed );
CSMPR_PLAYER_STATUS CSMPR_Player_GetStatus( void );
CSMPR_RESULT CSMPR_Player_SetMode( CSMPR_PLAYER_MODE mode );
CSMPR_PLAYER_MODE CSMPR_Player_GetMode( void );
CSMPR_RESULT get_file_info( char *filename, int video_pid, CSMPR_FILEINFO *fileinfo );
CSMPR_RESULT get_file_info_simple( char *filename, CSMPR_FILEINFO *fileinfo );
CSMPR_RESULT CSMPR_Player_GetMoveTime( time_t *pMoveTime );
void CSMPR_Player_Get_Title(char *Temp);
CSMPR_RESULT CSMPR_Player_Clear_TempPosition( void );
CSMPR_RESULT CSMPR_Player_Set_Position( void );
CSMPR_RESULT CSMPR_Player_SetPoint( BOOL Jump_Kind );
#ifdef __cplusplus
}
#endif

#endif /* end of __CSPLAYER_H__ */

