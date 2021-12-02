#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "linuxos.h"
#include "global.h"
#include "csapi.h"
#include "av_zapping.h"
#include "database.h"
#include "csmpr_recorder.h"
#include "csmpr_common.h"  
#include "csmpr_parser.h"
#include "csmpr_player.h"
#include "userdefine.h"
#include "csmpr_av.h"
#include "csmpr_pts.h"
#include "mwsvc.h"
#include "ui_common.h"

#define PLAY_BUFF_SIZE   	   			(256*188)
#define	CSPLAY_MAX_FILE_SIZE			CSREC_MAX_FILE_SIZE

static unsigned char					st_data[PLAY_BUFF_SIZE];
static unsigned char					temp_file_name[128];
static tCS_AV_PlayParams				ProgramInfo;
static CSMPR_PLAYER_OBJ					lv_player_obj;
static CSMPR_PLAYER_MODE				lv_player_mode = CSMPR_PLAY_NORMAL;
 
extern CSVID_HANDLE						vid_handle;
extern CSAUD_HANDLE						aud_handle;
extern CSDEMUX_HANDLE					xport_chl1_handle;
extern CSDEMUX_HANDLE					xport_pid_filter2_handle;
extern CSDEMUX_HANDLE					xport_pid_filter3_handle;
extern CSDEMUX_HANDLE					xport_vidout_handle;
extern CSDEMUX_HANDLE					xport_audout_handle;

static CSOS_Semaphore_t 				*sem_Pvr_Player_Access = NULL;

static void *player_task( void *arg )
{
	int            read_bytes;
	long long      SeekOffset = 0;
	static int     counter       = 0;
//	int            SeekSize      = 100;
	int            LoadSize      = 8;
	unsigned char  bHookNotifier = 0;

//	strcpy(st_buffer, (char *)arg);
	
	while( lv_player_obj.Status != CSMPR_PLAY_STOP ) 
	{
		//printf(".");
		if( lv_player_obj.Status == CSMPR_PLAY_PAUSE )
		{
			usleep(20000);
		}
		else
		{
			if( (counter++ % LoadSize) == 0 ) 
			{
				switch( lv_player_obj.Status )
				{
					case CSMPR_PLAY_RUN:
						LoadSize   = 8;
						SeekOffset = 0;
						break;
						
					case CSMPR_PLAY_FF_2X:
						LoadSize   = 8;
						SeekOffset = 0;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 360000 );
						} else {
							usleep( 60000 );
						}
						break;
						
					case CSMPR_PLAY_FF_4X:
						LoadSize   = 8;				   	
						SeekOffset = 0;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 100000 );
						} else {
							usleep( 10000 );
						}
						break;
						
					case CSMPR_PLAY_FF_8X:
						LoadSize   = 32;				   	
						SeekOffset =  LoadSize * 2* PLAY_BUFF_SIZE;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 50000 );
						} else {
							usleep( 10 );
						}
						break;
						
					case CSMPR_PLAY_FF_16X:
						LoadSize   = 32;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							SeekOffset =  LoadSize * 2* PLAY_BUFF_SIZE;
						} else {
							SeekOffset =  LoadSize * 4* PLAY_BUFF_SIZE;
						}		      
						break;
						
					case CSMPR_PLAY_REW_2X:
						LoadSize   = 32;
						SeekOffset = -LoadSize * 2* PLAY_BUFF_SIZE;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 210000 );
						} else {
							usleep( 90000 );
						}
						break;
						
					case CSMPR_PLAY_REW_4X:
						LoadSize   = 32;				   	
						SeekOffset = - LoadSize * 2 * PLAY_BUFF_SIZE;
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 90000 );
						} else {
							usleep( 30000 );
						}				  
						break;
						
					case CSMPR_PLAY_REW_8X:
						LoadSize   = 32;
						SeekOffset = - LoadSize * 2 * PLAY_BUFF_SIZE;                    
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 30000 );
						} else {
							usleep( 10000 );
						}				  
						break;
						
					case CSMPR_PLAY_REW_16X:
						LoadSize   = 32;
						SeekOffset = - LoadSize * 2 * PLAY_BUFF_SIZE;                    
						if( lv_player_obj.bitrate < 7*1024*1024L )
						{
							usleep( 3000 );
						} else {
							usleep( 1000 );
						}				  
						break;
						
					default:
						SeekOffset = 0;
						break;
				}

				if( SeekOffset != 0 )
				{
					CSOS_WaitSemaphore(sem_Pvr_Player_Access);
					lv_player_obj.position += SeekOffset;
					CSOS_SignalSemaphore(sem_Pvr_Player_Access);
				}
			}

			if( (lv_player_obj.Status >= CSMPR_PLAY_REW_2X && lv_player_obj.Status <= CSMPR_PLAY_REW_16X ) && (lv_player_obj.position < LoadSize * 2 * PLAY_BUFF_SIZE ) )
			{
				// fast rewinds to the beginning, resume to playback
				CSVID_DisableTrickMode( lv_player_obj.vid_handle );
				CSAUD_DisableMute( lv_player_obj.aud_handle );

				lseek( lv_player_obj.play_fd, 0, SEEK_SET );
			
				CSOS_WaitSemaphore(sem_Pvr_Player_Access);
				lv_player_obj.position = 0;
				CSOS_SignalSemaphore(sem_Pvr_Player_Access);

				lv_player_obj.Status = CSMPR_PLAY_RUN;
				bHookNotifier = 1;	
			}          

			if( lv_player_mode == CSMPR_PLAY_TIMESHIFT && lv_player_obj.position >= (long long)CSMPR_TIMESHIFT_MAX_FILE_SIZE )
			{
				// Rewind to the beginning
				CSMPR_DBG_DEBUG("TIME SHIFT: Rewind to the beginning.\n");
				
				CSOS_WaitSemaphore(sem_Pvr_Player_Access);
				lv_player_obj.position -= CSMPR_TIMESHIFT_MAX_FILE_SIZE;
				CSOS_SignalSemaphore(sem_Pvr_Player_Access);
				
				lseek( lv_player_obj.play_fd, lv_player_obj.position, SEEK_SET );
			}

			// printf( "position = %lld ", lv_player_obj.position);
			//printf( "CSPLAY_MAX_FILE_SIZE=%ld, curr_file_no=%d\n", CSPLAY_MAX_FILE_SIZE, lv_player_obj.curr_file_no );
			if( ( lv_player_obj.total_file_no > 1 ) && (lv_player_obj.position / CSPLAY_MAX_FILE_SIZE) != lv_player_obj.curr_file_no )
			{
				// printf( "close(%d)\n", lv_player_obj.play_fd );
				close( lv_player_obj.play_fd );

				lv_player_obj.curr_file_no = (lv_player_obj.position / CSPLAY_MAX_FILE_SIZE);

				if( lv_player_obj.curr_file_no == 0 )
					sprintf( temp_file_name, "%s", lv_player_obj.filename );
				else
					sprintf( temp_file_name, "%s.%03d", lv_player_obj.filename, lv_player_obj.curr_file_no );

				lv_player_obj.play_fd = open( temp_file_name, O_RDONLY );
				// printf( "open(%s) = %d\n", temp_file_name, lv_player_obj.play_fd );
				if( lv_player_obj.play_fd < 0 )
				{
					// Error open file. quit playback mode.
					CSMPR_DBG_ERROR("!!!!Error open next file. Quit.\n");
					lv_player_obj.position = 0;
					lv_player_obj.Status = CSMPR_PLAY_STOP;
					bHookNotifier = 1;
				}
				lv_player_obj.curr_file_pos = (lv_player_obj.position % CSPLAY_MAX_FILE_SIZE);
				lseek( lv_player_obj.play_fd, lv_player_obj.curr_file_pos, SEEK_SET );
				// printf( "lseek(%lld/%lld)\n", lv_player_obj.curr_file_pos, lv_player_obj.position );                
			}
			else 
			{
				/* For Second file play problem By KB Kim 2011.09.28 */
				// if( SeekOffset != 0 )
				{
					if (lv_player_obj.total_file_no > 1)
					{
						lv_player_obj.curr_file_pos = (lv_player_obj.position % CSPLAY_MAX_FILE_SIZE);
						lseek( lv_player_obj.play_fd, lv_player_obj.curr_file_pos, SEEK_SET );
					}
					else
					{
						lseek( lv_player_obj.play_fd, lv_player_obj.position, SEEK_SET );
					}

					SeekOffset = 0;
				}
			}

			read_bytes = read( lv_player_obj.play_fd, st_data, PLAY_BUFF_SIZE );
			// printf( "read_bytes = %d\n", read_bytes );

			/*  By KB Kim 2011.09.24
			CSOS_WaitSemaphore(sem_Pvr_Player_Access);
			lv_player_obj.curr_file_pos += read_bytes;
			lv_player_obj.position      += read_bytes;
			CSOS_SignalSemaphore(sem_Pvr_Player_Access);
			*/

			if( read_bytes != PLAY_BUFF_SIZE )
			{
				// close( lv_player_obj.play_fd );

				// Error open file. quit playback mode.
				// printf( "read_bytes = %d\n", read_bytes );
				// CSMPR_DBG_ERROR("!!!!Error read file. Quit.\n");

				CSOS_WaitSemaphore(sem_Pvr_Player_Access);
				/*  By KB Kim 2011.09.24 */
				// lv_player_obj.curr_file_pos -= read_bytes;
				// lv_player_obj.position      -= read_bytes;
				
				lv_player_obj.curr_file_pos += PLAY_BUFF_SIZE;
				lv_player_obj.position      += PLAY_BUFF_SIZE;
				CSOS_SignalSemaphore(sem_Pvr_Player_Access);

				if ( lv_player_obj.position > lv_player_obj.total_size )
				{
					CSMPR_DBG_ERROR("!!!!Error read file. Quit. Maybe File End \n");
					lv_player_obj.Status = CSMPR_PLAY_STOP;
					bHookNotifier = 1;
				}
			}
			else
			{
				/*  By KB Kim 2011.09.24 */
				CSOS_WaitSemaphore(sem_Pvr_Player_Access);
				lv_player_obj.curr_file_pos += read_bytes;
				lv_player_obj.position      += read_bytes;
				CSOS_SignalSemaphore(sem_Pvr_Player_Access);

				if ( lv_player_obj.position > lv_player_obj.total_size )
				{
					CSMPR_DBG_ERROR("!!!!Error read file. Quit. Maybe File End \n");
					lv_player_obj.Status = CSMPR_PLAY_STOP;
					bHookNotifier = 1;
				}
				
				if( 0x47 != st_data[0] )
				{   
					CSMPR_DBG_ERROR(" error. != 0x47. \n");
				}

				if( CSAPI_FAILED == CSDEMUX_CHL_DMA_Write(lv_player_obj.chl_handle, st_data, read_bytes) )
				{	
					// printf("======== CSDEMUX_CHL_DMA_Write FAIL =======\n");
					usleep(20000); 
				}
			}
		}

		if( bHookNotifier && lv_player_obj.notifier != NULL )
		{
			lv_player_obj.notifier( lv_player_obj.Status );
		}
		bHookNotifier = 0;
	}

	return NULL;
}

CSMPR_RESULT get_file_info( char *filename, int video_pid, CSMPR_FILEINFO *fileinfo )
{
	struct stat   statbuf;
	int           ii;

	CSMPR_DBG_TRACE;

	// Get duration according PTS.
#if 1
	CSMPR_DBG_DEBUG( "Get file duration.\n" );
	fileinfo->duration = PVR_get_file_duration( filename, video_pid );
#else // FOR TEST
	lv_player_obj.duration = 300;
#endif
	CSMPR_DBG_DEBUG( "duration = %ld.\n", fileinfo->duration );    

	// Get file size: 
	if( stat( filename, &statbuf ) < 0 )
	{
		return( CSMPR_ERROR );
	}
	
	fileinfo->first_file_size = statbuf.st_size;

	if( fileinfo->duration > 0 )
	{  
		fileinfo->bitrate = fileinfo->first_file_size * 8 / fileinfo->duration;
	} 
	else 
	{
		fileinfo->bitrate = 0;
	}

	// Count how total file number and total file size.
	fileinfo->total_file_no = 1;
	fileinfo->total_size    = fileinfo->first_file_size;

	ii = 1;  // count from one.
	do
	{
		sprintf( temp_file_name, "%s.%03d", filename, ii );
		CSMPR_DBG_DEBUG("####Check file exist %s\n", temp_file_name );
		
		if( access( temp_file_name, 0 ) == 0 )
		{
			fileinfo->total_file_no++;
			if( stat( temp_file_name, &statbuf ) == 0 )
			{
				fileinfo->total_size += statbuf.st_size;
			}
		} 
		else
		{
			break;
		}
		
		ii++;
	} while(1);

	fileinfo->duration = fileinfo->duration * fileinfo->total_size / fileinfo->first_file_size;

	CSMPR_DBG_DEBUG( "bitrate=%ld\n", fileinfo->bitrate );
	CSMPR_DBG_DEBUG( "first file size = %lld\n", fileinfo->first_file_size );
	CSMPR_DBG_DEBUG( "Total duration = %ld\n", fileinfo->duration );
	CSMPR_DBG_DEBUG( "Total file number = %d\n", fileinfo->total_file_no );
	CSMPR_DBG_DEBUG( "Total file size = %lld\n", fileinfo->total_size );

	return( CSMPR_SUCCESS );
}

CSMPR_RESULT get_file_info_simple( char *filename, CSMPR_FILEINFO *fileinfo )
{
	struct stat   statbuf;
	int           ii;

	if( stat( filename, &statbuf ) < 0 )
	{
		return( CSMPR_ERROR );
	}
	
	fileinfo->first_file_size = statbuf.st_size;
	fileinfo->total_file_no = 1;
	fileinfo->total_size    = fileinfo->first_file_size;

	ii = 1;  // count from one.

	do
	{
		sprintf( temp_file_name, "%s.%03d", filename, ii );
		
		if( access( temp_file_name, 0 ) == 0 )
		{
			fileinfo->total_file_no++;
			if( stat( temp_file_name, &statbuf ) == 0 )
			{
				fileinfo->total_size += statbuf.st_size;
			}
		} 
		else
		{
			break;
		}
		
		ii++;
	} while(1);

	return( CSMPR_SUCCESS );
}

static void player_notifier( CSMPR_PLAYER_STATUS status )
{
//    int   Result;
//    CSVID_BUFFER_STATUS buffer_status;

    CSMPR_DBG_TRACE;

    if( status == CSMPR_PLAY_STOP )
    {	
        /*do
        {
            Result = CSVID_GetBufferStatus( vid_handle, &buffer_status );
            CSMPR_DBG_DEBUG( "Result=%d, buffer_status=%d.\n ", Result, buffer_status );
            sleep(1);
        } while( buffer_status != VID_BUFFER_EMPTY && Result == CSAPI_SUCCEED );*/

        lv_player_obj.Status = CSMPR_PLAY_STOP;
        //pthread_cancel( lv_player_obj.player_thread );
        close( lv_player_obj.play_fd );

        CSMPR_StopAudio();
        CSMPR_StopVideo();
        CSMPR_CloseDemuxCH1();

		CS_MW_StopService(TRUE);
		BroadcastMessage (MSG_PVR_PLAY_END, 0, 0);
		MV_MW_StartService(CS_DB_GetCurrentServiceIndex());
        // Notify UI.
        //if( lv_player_obj.status_notifier != NULL )
        //{
        //    lv_player_obj.status_notifier( CSMPR_PLAY_STOP );
        //}
    }
    else if( status == CSMPR_PLAY_RUN )
    {
        lv_player_obj.Status = CSMPR_PLAY_RUN;
        
        CSAUD_Resume( aud_handle );
        CSVID_Resume( vid_handle );

        CSDEMUX_PIDFT_SetPID( xport_pid_filter3_handle, ProgramInfo.Audio_PID );
        CSDEMUX_AUD_SetPID( xport_audout_handle, ProgramInfo.Audio_PID );

        CSVID_DisableTrickMode( vid_handle );
        CSAUD_DisableMute( aud_handle );

		ProgramInfo.Audio_Volume = CS_AV_AudioGetVolume();
        CS_AV_ProgramPlay( ProgramInfo );

        // Notify UI.
        //if( lv_player_obj.status_notifier != NULL )
        //{
        //    lv_player_obj.status_notifier( CSMPR_PLAY_RUN );
        //}
    }
}


CSMPR_RESULT CSMPR_Player_Init( void )
{
	sem_Pvr_Player_Access  = CSOS_CreateSemaphoreFifo (NULL, 1 );
	
	memset ( (void *)&lv_player_obj, 0x00, sizeof(CSMPR_PLAYER_OBJ));
    lv_player_obj.Status = CSMPR_PLAY_STOP;
    pthread_mutex_init( &lv_player_obj.player_mutex, NULL );

	return ( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_Term( void )
{
	if( lv_player_obj.Status >= CSMPR_PLAY_RUN )
    {
        CSMPR_Player_Stop();
        return( CSMPR_ERROR );
    }
    
	lv_player_obj.Status = CSMPR_PLAY_INIT;

    pthread_mutex_destroy( &lv_player_obj.player_mutex );
    
    return( CSMPR_SUCCESS );       
}


CSMPR_RESULT CSMPR_Player_Start( char* filename , long long Start_Position)
{
	CSMPR_FILEINFO  fileInfo;

	CSMPR_DBG_TRACE;
	CSMPR_DBG_DEBUG( "File name:%s\r\n", filename );

	if( lv_player_obj.Status != CSMPR_PLAY_STOP )
	{
		return( CSMPR_ERROR );
	}

	// Extract audio/video pid and type information:
	if( CSMPR_ExtractAvParam( filename, &ProgramInfo ) != 0 )
	{
		CSMPR_DBG_ERROR( "Failed to parse ts file!!!\n" );
		return( CSMPR_ERROR );
	}
	
	CSMPR_DBG_DEBUG( "video pid = %d type = %d.\r\n", ProgramInfo.Video_PID, ProgramInfo.VideoType );
	CSMPR_DBG_DEBUG( "audio pid = %d type = %d.\r\n", ProgramInfo.Audio_PID, ProgramInfo.AudioType );

	if( lv_player_mode != CSMPR_PLAY_TIMESHIFT )
	{
		if( get_file_info( filename, ProgramInfo.Video_PID, &fileInfo ) != CSMPR_SUCCESS )
		{
			CSMPR_DBG_ERROR( "Failed to get file info!!!\n" );
			return( CSMPR_ERROR );
		}

		lv_player_obj.duration        = fileInfo.duration;
		lv_player_obj.bitrate         = fileInfo.bitrate;
		lv_player_obj.total_file_no   = fileInfo.total_file_no; 
		lv_player_obj.total_size      = fileInfo.total_size;
		lv_player_obj.curr_file_size  = fileInfo.first_file_size;       
	}
	else
	{
		// In timeshift mode:
		lv_player_obj.duration        = CSMPR_Record_GetTime();
		lv_player_obj.total_file_no   = 1;       
		lv_player_obj.curr_file_size  = CSMPR_Record_GetCurrFileSize();
		lv_player_obj.total_size      = lv_player_obj.curr_file_size;

		if( lv_player_obj.duration != 0 )
		{   
			lv_player_obj.bitrate = lv_player_obj.curr_file_size / lv_player_obj.duration;
		}

		printf("timeshift playback: duration=%ld, filesize=%lld, bitrate=%ld.\n", 
		lv_player_obj.duration, lv_player_obj.curr_file_size, lv_player_obj.bitrate );
	}

	memset( lv_player_obj.filename, 0, 128 );
	strncpy( lv_player_obj.filename, filename, 127 );

	// Reset handles and status:
	//lv_player_obj.chl_handle      = xport_chl1_handle;
	lv_player_obj.vid_handle      = vid_handle;
	lv_player_obj.aud_handle      = aud_handle;
	lv_player_obj.notifier        = player_notifier;

//	printf("Start_Position : %ld\n", Start_Position);
	lv_player_obj.position        = Start_Position;   
	lv_player_obj.curr_file_no    = 0;
	lv_player_obj.curr_file_pos   = Start_Position;

//	printf( "%s\n", lv_player_obj.filename );
	if( (lv_player_obj.play_fd = open( lv_player_obj.filename, O_RDONLY )) < 0 )
	{   
		CSMPR_DBG_ERROR( "Failed to open file!!!\n" );
		return( CSMPR_ERROR );
	}

#if 0
	{
		unsigned char buf[188];
//		int           ii;

		if( read( lv_player_obj.play_fd, buf, 188 ) == 188 )
		{
/*			
			for( ii = 0; ii < 188; ii++ )
			{
				printf( "%02x ", buf[ii] );
			}
*/
		} else {
			printf("#### Error reading file.\n");
		}
	}
#endif

	// Start...
	CSMPR_OpenDemuxCH1();
	// update chl_handle
	lv_player_obj.chl_handle  = xport_chl1_handle;

	CSDEMUX_CHL_SetInputMode( xport_chl1_handle, DEMUX_INPUT_MOD_DMA );
	CSDEMUX_CHL_Enable( xport_chl1_handle );

	CSMPR_PlayAudio( ProgramInfo );
	CSMPR_PlayVideo( ProgramInfo );

	lv_player_obj.Status = CSMPR_PLAY_RUN;  // Set this before launch the task.

	// Create thread, go...
	pthread_create( &lv_player_obj.player_thread, NULL, player_task, &lv_player_obj );
	
//	printf("=== Position : %lld\n", lv_player_obj.position);
//	printf("=== curr_file_pos : %lld\n", lv_player_obj.curr_file_pos);

	if ( lv_player_obj.position > 0 )
	{
		lv_player_obj.curr_file_pos = (lv_player_obj.position % CSPLAY_MAX_FILE_SIZE);
		lseek( lv_player_obj.play_fd, lv_player_obj.curr_file_pos, SEEK_SET );
	} else {
		lseek( lv_player_obj.play_fd, lv_player_obj.position, SEEK_SET );
	}
	//set_pthread_prioity( lv_player_obj.player_thread, 60 );
	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_Stop( void )
{
	char	TempStr[128];
	char	TempStr2[128];
	
    CSMPR_DBG_TRACE;

    if( lv_player_obj.Status == CSMPR_PLAY_STOP || lv_player_obj.Status == CSMPR_PLAY_INIT )
    {
        return( CSMPR_ERROR );
    }

	memset(TempStr, 0x00, 128);
	
	strncpy(TempStr, lv_player_obj.filename, strlen(lv_player_obj.filename) - 3);
	sprintf(TempStr2, "%s.cfg", TempStr);
/*
	printf("== TempStr : %s \n", TempStr);
	printf("== TempStr2 : %s \n", TempStr2);
	printf("== position : %lld \n", lv_player_obj.position);
*/
	if ( lv_player_obj.position >= lv_player_obj.total_size )
		MV_PVR_CFG_File_Set_LastPosition(TempStr2, 0);
	else 
		MV_PVR_CFG_File_Set_LastPosition(TempStr2, lv_player_obj.position);
		
    lv_player_obj.Status = CSMPR_PLAY_STOP;
    pthread_cancel( lv_player_obj.player_thread );

    close( lv_player_obj.play_fd );

    CSMPR_StopAudio();
    CSMPR_StopVideo();
    CSMPR_CloseDemuxCH1();

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_Pause( void )
{
    CSMPR_DBG_TRACE;

    if( lv_player_obj.Status != CSMPR_PLAY_RUN )
    {
        CSMPR_DBG_ERROR( "CSMPR_Player_Pause(): Error Status = %d.\n", lv_player_obj.Status );
        return( CSMPR_ERROR );
    }

    lv_player_obj.Status   = CSMPR_PLAY_PAUSE;
    lv_player_obj.curr_file_pos = lseek( lv_player_obj.play_fd, 0, SEEK_CUR );
    
	CSAUD_Pause( aud_handle );
	CSVID_Pause( vid_handle );
		
	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_Resume( void )
{
	CSMPR_DBG_TRACE;

	if( lv_player_obj.Status <= CSMPR_PLAY_RUN )
	{
		CSMPR_DBG_ERROR( "[%s]: Error Status = %d.\n", __FUNCTION__, lv_player_obj.Status );
		return( CSMPR_ERROR );
	}

	if( lv_player_obj.Status == CSMPR_PLAY_PAUSE )
	{
		CSMPR_DBG_DEBUG( "[%s]:Resume from Pause\n", __FUNCTION__ );

		CSAUD_Play( aud_handle );
		CSVID_Play( vid_handle );

		lv_player_obj.Status = CSMPR_PLAY_RUN;
	} 
	else if( (lv_player_obj.Status >= CSMPR_PLAY_FF_2X && lv_player_obj.Status <= CSMPR_PLAY_FF_16X ) || 
		(lv_player_obj.Status >= CSMPR_PLAY_REW_2X && lv_player_obj.Status <= CSMPR_PLAY_REW_16X ) )
	{
		CSMPR_DBG_DEBUG( "[%s]:Resume from FF/REW\n", __FUNCTION__ );

		lv_player_obj.Status = CSMPR_PLAY_RUN;

#if 0
		CSAUD_Resume( aud_handle );
		CSVID_Resume( vid_handle );

		CSDEMUX_PIDFT_SetPID( xport_pid_filter3_handle, ProgramInfo.Audio_PID );
		CSDEMUX_AUD_SetPID( xport_audout_handle, ProgramInfo.Audio_PID );

		CSVID_DisableTrickMode( vid_handle );
		CSAUD_DisableMute( aud_handle );

		ProgramInfo.Audio_Volume = CS_AV_AudioGetVolume();
		CS_AV_ProgramPlay( ProgramInfo );
#else
		CSDEMUX_CHL_ClearBuffer(lv_player_obj.chl_handle);

		CSDEMUX_PIDFT_SetPID( xport_pid_filter3_handle, ProgramInfo.Audio_PID );
		CSDEMUX_AUD_SetPID( xport_audout_handle, ProgramInfo.Audio_PID );

		CSVID_DisableTrickMode( vid_handle );
		CSAUD_DisableMute( aud_handle );

		CSMPR_PlayAudio( ProgramInfo );
		CSMPR_PlayVideo( ProgramInfo );
#endif
	}

	return( CSMPR_SUCCESS );   
}



CSMPR_RESULT CSMPR_Player_Forward( void )
{	
	CSMPR_DBG_TRACE;

	switch( lv_player_obj.Status )
	{
		case CSMPR_PLAY_RUN:
			lv_player_obj.Status = CSMPR_PLAY_FF_2X;
			break;
		case CSMPR_PLAY_FF_2X:
			lv_player_obj.Status = CSMPR_PLAY_FF_4X;
			break;
		case CSMPR_PLAY_FF_4X:
			lv_player_obj.Status = CSMPR_PLAY_FF_8X;
			break;
		case CSMPR_PLAY_FF_8X:
			lv_player_obj.Status = CSMPR_PLAY_FF_16X;
			break;
		default:
			lv_player_obj.Status = CSMPR_PLAY_FF_2X;
			break;      
	}

	CSAUD_Stop( aud_handle );
	CSDEMUX_AUD_Disable( xport_audout_handle );
	CSDEMUX_PIDFT_Disable( xport_pid_filter3_handle );
	CSVID_EnableTrickMode( vid_handle );

	return( CSMPR_SUCCESS );
}



CSMPR_RESULT CSMPR_Player_Rewind( void )
{
	CSMPR_DBG_TRACE;

	switch( lv_player_obj.Status )
	{
		case CSMPR_PLAY_RUN:
			lv_player_obj.Status = CSMPR_PLAY_REW_2X;
			break;
		case CSMPR_PLAY_REW_2X:
			lv_player_obj.Status = CSMPR_PLAY_REW_4X;
			break;
		case CSMPR_PLAY_REW_4X:
			lv_player_obj.Status = CSMPR_PLAY_REW_8X;
			break;
		case CSMPR_PLAY_REW_8X:
			lv_player_obj.Status = CSMPR_PLAY_REW_16X;
			break;
		default:
			lv_player_obj.Status = CSMPR_PLAY_REW_2X;
			break;
	}

	CSAUD_Stop( aud_handle );
	CSDEMUX_AUD_Disable( xport_audout_handle );
	CSDEMUX_PIDFT_Disable( xport_pid_filter3_handle );
	CSVID_EnableTrickMode( vid_handle );

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_GetElapsedTime( time_t *pElapsedTime )
{		
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }

	if( pElapsedTime == NULL ) 
	{   
		return( CSMPR_ERROR );
	}    
	
	*pElapsedTime = lv_player_obj.duration * lv_player_obj.position / lv_player_obj.total_size;

return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_GetDuration( time_t *pDuration )
{		
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }

	if( pDuration == NULL ) 
	{   
		return( CSMPR_ERROR );
	}    
	
	*pDuration = lv_player_obj.duration;

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_GetPosition( U32 *nPosition )
{
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }

	if( nPosition == NULL ) 
	{   
		return( CSMPR_ERROR );
	}    
	
	*nPosition = lv_player_obj.position * 100 / lv_player_obj.total_size;

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Player_GetFileName_Pointer( char **pFileName)
{
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }

	if( pFileName == NULL ) 
	{   
		return( CSMPR_ERROR );
	}    
	
	*pFileName = lv_player_obj.filename;

return( CSMPR_SUCCESS );
}

CSMPR_RESULT CSMPR_Player_GetFileName( char *pFileName)
{
	if( lv_player_obj.Status == CSMPR_PLAY_INIT )
	{   
		return( CSMPR_ERROR );
	}

	if( pFileName == NULL ) 
	{   
		return( CSMPR_ERROR );
	}    

	strncpy(pFileName, lv_player_obj.filename, strlen(lv_player_obj.filename));

	return( CSMPR_SUCCESS );
}

CSMPR_RESULT CSMPR_Player_GetTrickSpeed( int *trickspeed )
{
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }

	if( trickspeed == NULL ) 
	{   
		return( CSMPR_ERROR );
	}

    switch( lv_player_obj.Status )
    {
		case CSMPR_PLAY_FF_2X:
			*trickspeed = 2;
			break;
			
		case CSMPR_PLAY_FF_4X:        
			*trickspeed = 4;
			break;
			
		case CSMPR_PLAY_FF_8X:        
			*trickspeed = 8;
			break;
			
		case CSMPR_PLAY_FF_16X:        
			*trickspeed = 16;
			break;
			
		case CSMPR_PLAY_REW_2X:
			*trickspeed = 2;
			break;
			
		case CSMPR_PLAY_REW_4X:
			*trickspeed = 4;
			break;
			
		case CSMPR_PLAY_REW_8X:
			*trickspeed = 8;
			break;
			
		case CSMPR_PLAY_REW_16X:
			*trickspeed = 16;
			break;
			
		default:
			*trickspeed = 1;
			break;
    }

    return( CSMPR_SUCCESS );
}


CSMPR_PLAYER_STATUS CSMPR_Player_GetStatus( void )
{
    return( lv_player_obj.Status );
}


CSMPR_RESULT CSMPR_Player_SetMode( CSMPR_PLAYER_MODE mode )
{
    lv_player_mode = mode;
    return( CSMPR_SUCCESS );
}


CSMPR_PLAYER_MODE CSMPR_Player_GetMode( void )
{
    return( lv_player_mode );    
}

CSMPR_RESULT CSMPR_Player_SetPoint( BOOL Jump_Kind )
{
	long long 			lJump_Buffer_size = lv_player_obj.total_size/10;

#if 1
	//printf("1 %lld , %lld==========>\n", lJump_Buffer_size, lv_player_obj.total_size);
	if( lJump_Buffer_size%PLAY_BUFF_SIZE != 0 )
	{
		lJump_Buffer_size += (PLAY_BUFF_SIZE - lJump_Buffer_size%PLAY_BUFF_SIZE);
	}
	//printf("2 %lld , %lld ==========>\n", lJump_Buffer_size, lv_player_obj.total_size);

	if ( Jump_Kind == TRUE )
	{
		if ( lv_player_obj.temp_position == 0 )
			lv_player_obj.temp_position = lv_player_obj.position;
		
		if ( ( lv_player_obj.temp_position + lJump_Buffer_size ) < lv_player_obj.total_size )
		{
			//printf("====== RIGHT : %lld =======\n", lv_player_obj.temp_position);	
	    	lv_player_obj.temp_position += lJump_Buffer_size;
			//printf("====== RIGHT : %lld =======\n", lv_player_obj.temp_position);
		}
	}
	else
	{
		if ( lv_player_obj.temp_position == 0 )
			lv_player_obj.temp_position = lv_player_obj.position;
		
		if ( lv_player_obj.temp_position > lJump_Buffer_size )
		{
			//printf("====== LEFT : %lld =======\n", lv_player_obj.temp_position);
			lv_player_obj.temp_position -= lJump_Buffer_size;
			//printf("====== LEFT : %lld =======\n", lv_player_obj.temp_position);
		} else {
			lv_player_obj.temp_position = 0;
		}
	}
#else
	if ( Jump_Kind == TRUE )
	{
		if ( lv_player_obj.temp_position == 0 )
			lv_player_obj.temp_position = lv_player_obj.position;
		
		if ( ( lv_player_obj.temp_position + 1024 * 2 * PLAY_BUFF_SIZE ) < lv_player_obj.total_size )
		{
			//printf("====== RIGHT : %lld =======\n", lv_player_obj.temp_position);	
	    	lv_player_obj.temp_position += 1024 * 2 * PLAY_BUFF_SIZE;
			//printf("====== RIGHT : %lld =======\n", lv_player_obj.temp_position);
		}
	}
	else
	{
		if ( lv_player_obj.temp_position == 0 )
			lv_player_obj.temp_position = lv_player_obj.position;
		
		if ( lv_player_obj.temp_position > ( 1024 * 2 * PLAY_BUFF_SIZE ) )
		{
			//printf("====== LEFT : %lld =======\n", lv_player_obj.temp_position);
			lv_player_obj.temp_position -= 1024 * 2 * PLAY_BUFF_SIZE;
			//printf("====== LEFT : %lld =======\n", lv_player_obj.temp_position);
		} else {
			lv_player_obj.temp_position = 0;
		}
	}
#endif
	return( CSMPR_SUCCESS );
}

CSMPR_RESULT CSMPR_Player_Set_Position( void )
{
	CSMPR_StopAudio();
    CSMPR_StopVideo();
	
	CSDEMUX_CHL_ClearBuffer(lv_player_obj.chl_handle);
	
	CSOS_WaitSemaphore(sem_Pvr_Player_Access);
	
	lv_player_obj.position = lv_player_obj.temp_position;
	if ( lv_player_obj.total_file_no > 1 )
	{
		if ((lv_player_obj.position / CSPLAY_MAX_FILE_SIZE) != lv_player_obj.curr_file_no)
		{
			// printf( "CSMPR_Player_Set_Position : close(%d)\n", lv_player_obj.play_fd );
			close( lv_player_obj.play_fd );
			lv_player_obj.curr_file_no = (lv_player_obj.position / CSPLAY_MAX_FILE_SIZE);

			if( lv_player_obj.curr_file_no == 0 )
				sprintf( temp_file_name, "%s", lv_player_obj.filename );
			else
				sprintf( temp_file_name, "%s.%03d", lv_player_obj.filename, lv_player_obj.curr_file_no );
			lv_player_obj.play_fd = open( temp_file_name, O_RDONLY );
			// printf( "CSMPR_Player_Set_Position : open(%s) = %d\n", temp_file_name, lv_player_obj.play_fd );
		}
		
		lv_player_obj.curr_file_pos = lv_player_obj.position % CSPLAY_MAX_FILE_SIZE;	
		// printf( "CSMPR_Player_Set_Position : position(%lld/%lld)\n", lv_player_obj.curr_file_pos, lv_player_obj.position);
		lseek( lv_player_obj.play_fd, lv_player_obj.curr_file_pos, SEEK_SET );
	} else {
		// printf( "CSMPR_Player_Set_Position : position(%lld)\n", lv_player_obj.position);
		lseek( lv_player_obj.play_fd, lv_player_obj.position, SEEK_SET );
	}
	
	lv_player_obj.temp_position = 0;
	
	CSOS_SignalSemaphore(sem_Pvr_Player_Access);

	CSMPR_PlayAudio( ProgramInfo );
	CSMPR_PlayVideo( ProgramInfo );
		
	return( CSMPR_SUCCESS );
}

CSMPR_RESULT CSMPR_Player_Clear_TempPosition( void )
{
	lv_player_obj.temp_position = 0;
		
	return( CSMPR_SUCCESS );
}

CSMPR_RESULT CSMPR_Player_GetMoveTime( time_t *pMoveTime )
{		
    if( lv_player_obj.Status == CSMPR_PLAY_INIT )
    {   
		return( CSMPR_ERROR );
    }
	
	*pMoveTime = lv_player_obj.duration * lv_player_obj.temp_position / lv_player_obj.total_size;

	return( CSMPR_SUCCESS );
}

void CSMPR_Player_Get_Title(char *Temp)
{
	int 	i;
	char	*Temp_Str;

	
	for ( i = strlen(lv_player_obj.filename) ; i > 0 ; i-- )
		if ( lv_player_obj.filename[i] == '/' )
			break;

	Temp_Str = lv_player_obj.filename + i + 1;
	
	strcpy(Temp, Temp_Str);
}

