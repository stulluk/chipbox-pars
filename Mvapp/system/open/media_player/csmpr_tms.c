#include "linuxos.h"
#include "global.h"
#include "database.h"
#include "db_builder.h"
#include "demux.h"
#include "eit_engine.h"
#include "av_zapping.h"
#include "fe_mngr.h"

#include "csmpr_player.h"
#include "csmpr_recorder.h"
#include "csmpr_parser.h"
#include "csmpr_av.h"
#include "csmpr_tms.h"

CSMPR_RESULT CSMPR_Record_SetMode( CSMPR_RECORD_MODE mode );
CSMPR_FILEINFO  TmsFileInfo;


CSMPR_RESULT CSMPR_TMS_Start( void )
{
	CSMPR_DBG_TRACE; 
	CSMPR_Record_SetMode( CSMPR_REC_TIMESHIFT );   
	CSMPR_Record_Start();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Play( void )
{
	/* By KB Kim for Timeshift play 2011.01.24 */
	char filename[256];

	CSMPR_DBG_TRACE;
	CSMPR_Player_SetMode( CSMPR_PLAY_TIMESHIFT );      
	/* By KB Kim for Timeshift play 2011.01.24 */
	sprintf(filename, "%s/tmsbuf.ts", CSMPR_USB_MOUNT_DIR );
	CSMPR_Player_Start(filename, 0);

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Pause( void )
{
	CSMPR_DBG_TRACE;
	CSMPR_Player_Pause();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Resume( void )
{
	CSMPR_DBG_TRACE;
	CSMPR_Player_Resume();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Forward( void )
{
	CSMPR_DBG_TRACE;
	CSMPR_Player_Forward();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Rewind( void )
{
	CSMPR_DBG_TRACE;
	CSMPR_Player_Rewind();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_TMS_Stop( void )
{
	CSMPR_DBG_TRACE;
	CSMPR_Player_SetMode( CSMPR_PLAY_NORMAL );      
	CSMPR_Record_SetMode( CSMPR_REC_NORMAL );         
	CSMPR_Player_Stop();
	CSMPR_Record_Stop();

	return( CSMPR_SUCCESS );
}

CSMPR_TMS_STATUS CSMPR_TMS_GetStatus( void )
{
	CSMPR_TMS_STATUS Status;

	if( CSMPR_Record_GetMode() == CSMPR_REC_TIMESHIFT && CSMPR_Record_GetStatus() >= CSMPR_REC_RUN )      
	{
		if( CSMPR_Player_GetMode() == CSMPR_PLAY_TIMESHIFT )
		{
			switch( CSMPR_Player_GetStatus() )
			{
				case CSMPR_PLAY_RUN:
					Status = CSMPR_TMS_RUN;
					break;
				case CSMPR_PLAY_INIT:
				case CSMPR_PLAY_STOP:
					Status = CSMPR_TMS_PAUSE;
					break;
				case CSMPR_PLAY_PAUSE:
					Status = CSMPR_TMS_REPAUSE;
					break;
				case CSMPR_PLAY_FF_2X:
				case CSMPR_PLAY_FF_4X:
				case CSMPR_PLAY_FF_8X:
				case CSMPR_PLAY_FF_16X:            
					Status = CSMPR_TMS_FF;
					break;
				case CSMPR_PLAY_REW_2X:
				case CSMPR_PLAY_REW_4X:
				case CSMPR_PLAY_REW_8X:
				case CSMPR_PLAY_REW_16X:            
					Status = CSMPR_TMS_REW;
					break;
				default:  
					printf("Error! illegal status.\n");
					Status = CSMPR_TMS_IDLE;
			}         
		} 
		else
		{  
			Status = CSMPR_TMS_PAUSE;
		}      
	} 
	else 
	{
		if( CSMPR_Player_GetMode() != CSMPR_PLAY_TIMESHIFT )      
		{
			Status = CSMPR_TMS_IDLE;
		}
		else 
		{
			//This can't happen.
			printf("Error! illegal status.\n");
			Status = CSMPR_TMS_IDLE;         
		}
	}

	return( Status );
}

