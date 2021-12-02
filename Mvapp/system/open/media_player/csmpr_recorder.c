#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "linuxos.h"
#include "global.h"
#include "csapi.h"
#include "csmpr_player.h"
#include "csmpr_recorder.h"
#include "database.h"
#include "db_builder.h"
#include "eit_engine.h"

/********************v38....*****************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

int listenfd = 0, connfd = 0;
struct sockaddr_in serv_addr;

#define MAX_BUFF 188
/***********************************************/

static CSREC_OBJ            lv_record_obj;
static pthread_mutex_t      pvr_mutex;
static struct timeval       start, end;
static unsigned char        temp_filename[128];
static CSMPR_RECORD_MODE    lv_record_mode = CSMPR_REC_NORMAL;

extern void MV_OS_Get_Time_to_MJD_UTC_Date_Time(U16 *u16MJD, U16 *u16UTC, tCS_DT_Date *stDate, tCS_DT_Time *stTime);

static unsigned long caculate_crc( unsigned char* section_data, unsigned int len )
{
	unsigned long crc;
	unsigned int i, j;

	crc = 0xffffffffL;

	for( i = 0 ; i < len ; i++ )
	{
		crc = crc ^ ( (unsigned long)section_data[i] << 24 );

		for( j = 0 ; j < 8 ; j++ )
		{
			if( ( crc & 0x80000000L) != 0 )
			{
				crc = ( crc << 1 ) ^ 0x04c11db7L;
			}
			else
			{
				crc = crc << 1;
			}
		}
	}
	return( crc );
}


static int remux_pat( unsigned char *pat_buf, int buf_len, int pmt_pid )
{
	int 			pat_size;
	unsigned long 	crcw;
	int 			j, i;
	unsigned char 	*ptr;

	j = 0;
	ptr = pat_buf;

	while( j+188 <= buf_len )
	{
		if( ptr[0] == 0x47 && ptr[1] == 0x40 &&	ptr[2] == 0x00 )
		{
			// this is a PAT:
			pat_size = ((ptr[6]&0x0f)<<8) + ptr[7] + 3+1+4;

			i = 11;
			do
			{
				i+=4;

				if( (ptr[i]&0x1f) == ((pmt_pid>>8)&0xff) && ptr[i+1] == (pmt_pid&0xff) )
				{
					ptr[13] = ptr[i-2];
					ptr[14] = ptr[i-1];
					ptr[15] = ptr[i];
					ptr[16] = ptr[i+1];

					ptr[6] = (ptr[6]&0xf0);
					ptr[7] = 21-8;

					crcw = caculate_crc( ptr, 17 );

					ptr[17] = ((crcw>>24)&0xff);
					ptr[18] = ((crcw>>16)&0xff);
					ptr[19] = ((crcw>>8)&0xff);
					ptr[20] = (crcw&0xff);

					for( i=21; i<188; i++ )
					{
						ptr[i] = 0xff;
					}
					break;
				}
			} while( i <= pat_size-4-4 );
		}

		ptr+=188;
		j+=188;
	}

	return(0);
}


static CSMPR_RESULT stop_record( void )
{
    int i;

    CSMPR_DBG_DEBUG("############ stop_record.\n");

    lv_record_obj.is_stopped = 1;

    if( lv_record_obj.record_fp != NULL )
    {
        fclose( lv_record_obj.record_fp );
    }

    CSDEMUX_Filter_Disable( lv_record_obj.params.secft_handle );

    for( i = 0; i < lv_record_obj.params.pidft_num; i++ )
    {
        CSDEMUX_PIDFT_Disable(lv_record_obj.params.pidft_handle[i]);
    }

    for( i = 0; i < lv_record_obj.params.pidft_num; i++ )
    {
        CSDEMUX_PIDFT_Close( lv_record_obj.params.pidft_handle[i] );
    }

    CSDEMUX_Filter_Close( lv_record_obj.params.secft_handle );

    lv_record_obj.record_status = CSMPR_REC_IDLE;

    return( CSMPR_SUCCESS );
}


/*static signed int set_pthread_prioity(pthread_t i_thread,  signed int i_priority)
{
   signed int i_policy = 0;

   struct sched_param param;

   param.sched_priority = i_priority;
   i_policy = SCHED_RR;

   pthread_setschedparam(i_thread, i_policy, &param);

   return 0;
}*/


static BOOL check_file_exist( char* filename )
{
	if( access( filename, 0 ) == 0 )
	{
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}


static CSMPR_RESULT generate_file_name( char *servicename, char *filename )
{
    tCS_DT_Date  CurrDate;
    tCS_DT_Time  CurrTime;
    U16          CurrMjd;
    U16          CurrUtc;
    int          i;

#if 0
    CurrMjd  = CS_DT_GetLocalMJD();
    CurrDate = CS_DT_MJDtoYMD( CurrMjd );
    CurrUtc  = CS_DT_GetLocalUTC();
    CurrTime = CS_DT_UTCtoHM( CurrUtc );
#else
	MV_OS_Get_Time_to_MJD_UTC_Date_Time(&CurrMjd, &CurrUtc, &CurrDate, &CurrTime);
#endif

    if( lv_record_mode == CSMPR_REC_TIMESHIFT )
    {
        sprintf( filename, "%s/tmsbuf.ts", CSMPR_USB_MOUNT_DIR );
    }
    else
    {
        sprintf( filename, "%s/%s_%04d%02d%02d_%02d%02d.ts",
                 CSMPR_USB_MOUNT_DIR, servicename,
                 CurrDate.year, CurrDate.month, CurrDate.day,
                 CurrTime.hour, CurrTime.minute );

        if( check_file_exist( filename ) )
        {
            i = 0;
            do
            {
                i++;
                sprintf( filename, "%s/%s_%04d%02d%02d_%02d%02d_%03d.ts",
                         CSMPR_USB_MOUNT_DIR, servicename,
                         CurrDate.year, CurrDate.month, CurrDate.day,
                         CurrTime.hour, CurrTime.minute, i );
            } while( check_file_exist( filename ) && i<999 );
        }
    }

    return( CSMPR_SUCCESS );
}


static int record_task( int *param )
{
    printf("\n*****************record_task started...\n");
    int    temp_read_len, read_len = 0;
    int    written_len, towrite_len;
    int    error_count = 0;

//    CSREC_OBJ *pvr_obj = (CSREC_OBJ *)param;

    while( !lv_record_obj.is_stopped )
    {
        //pthread_mutex_lock(&pvr_mutex);
        if( CSDEMUX_Filter_ReadWait( lv_record_obj.params.secft_handle, 50 ) == CSAPI_SUCCEED )
        {
            if( CSDEMUX_Filter_CheckDataSize( lv_record_obj.params.secft_handle, &read_len ) == CSAPI_SUCCEED )
            {
                temp_read_len = read_len;
                while( read_len > 0 )
                {
                    if( CSDEMUX_Filter_ReadData( lv_record_obj.params.secft_handle, lv_record_obj.record_buf, &read_len ) == CSAPI_SUCCEED )
                    {
                        error_count = 0;

                        if( temp_read_len != read_len )
                        {
                            CSMPR_DBG_ERROR( "####%d=%d\n", temp_read_len, read_len );
                        }

                        remux_pat( lv_record_obj.record_buf, read_len, lv_record_obj.params.pmt_pid );

                        if( read_len > 0 )
                        {
                            // Write to file. not allow bigger than MAX_FILE_SIZE.
                            if( lv_record_obj.written_cnt + read_len > CSREC_MAX_FILE_SIZE )
                                towrite_len = CSREC_MAX_FILE_SIZE - lv_record_obj.written_cnt;
                            else
                                towrite_len = read_len;

                            written_len = fwrite( lv_record_obj.record_buf, 1, towrite_len, lv_record_obj.record_fp );
                            lv_record_obj.written_cnt += written_len;

                            // Check write error:
                            if( written_len != towrite_len )
                            {
                                CSMPR_DBG_DEBUG( "Error writing file. Disk full?\n" );
								MV_PVR_FileWrite_Time(lv_record_obj.params.filename, NULL, 1, CS_PVR_STOP);
                                stop_record();
                                break;
                            }

                            if( lv_record_mode == CSMPR_REC_TIMESHIFT && lv_record_obj.written_cnt >= CSMPR_TIMESHIFT_MAX_FILE_SIZE )
                            {
                                // Rewind to the beginning
                                fseek( lv_record_obj.record_fp, 0, SEEK_SET );

                                // If there is remaining data in buffer.
                                if( read_len > written_len )
                                {
                                    towrite_len = read_len - written_len;
                                    written_len = fwrite( lv_record_obj.record_buf + written_len, 1, towrite_len, lv_record_obj.record_fp );
                                    lv_record_obj.written_cnt += written_len;

                                    if( written_len != towrite_len )
                                    {
                                        CSMPR_DBG_DEBUG( "Error writing file (tms mode). Disk full?\n" );
										MV_PVR_FileWrite_Time(lv_record_obj.params.filename, NULL, 1, CS_PVR_STOP);
                                        stop_record();
                                        break;
                                    }
                                }
                                lv_record_obj.written_cnt -= CSMPR_TIMESHIFT_MAX_FILE_SIZE;
                            }

                            // If the file bigger than MAX_FILE_SIZE, create a new file.
                            if( lv_record_obj.written_cnt >= CSREC_MAX_FILE_SIZE )
                            {
                                // Close old file.
                                fclose( lv_record_obj.record_fp );

                                lv_record_obj.written_cnt = 0;
                                lv_record_obj.file_no++;

                                if( lv_record_obj.file_no > 999 )
                                {
                                    CSMPR_DBG_DEBUG( "Error!! To many files?\n" );
									MV_PVR_FileWrite_Time(lv_record_obj.params.filename, NULL, 1, CS_PVR_STOP);
                                    stop_record();
                                    break;
                                }

                                // Create new file.
                                sprintf( temp_filename, "%s.%03d", lv_record_obj.params.filename, lv_record_obj.file_no );
                                CSMPR_DBG_DEBUG( "##### Create new file (%s).\n", temp_filename );
                                lv_record_obj.record_fp = fopen( temp_filename, "w+b" );

                                if( lv_record_obj.record_fp == NULL )
                                {
                                    CSMPR_DBG_DEBUG( "Error!! Failed to create new file. Disk full?\n" );
									MV_PVR_FileWrite_Time(lv_record_obj.params.filename, NULL, 1, CS_PVR_STOP);
                                    stop_record();
                                    break;
                                }

                                // If there is remaining data in buffer.
                                if( read_len > written_len )
                                {
                                    towrite_len = read_len - written_len;
                                    written_len = fwrite( lv_record_obj.record_buf + written_len, 1, towrite_len, lv_record_obj.record_fp );
                                    lv_record_obj.written_cnt += written_len;

                                    if( written_len != towrite_len )
                                    {
                                        CSMPR_DBG_DEBUG( "Error writing file. Disk full?\n" );
										MV_PVR_FileWrite_Time(lv_record_obj.params.filename, NULL, 1, CS_PVR_STOP);
                                        stop_record();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } //end of while
            }
        }
        else
        {
            CSMPR_DBG_DEBUG( "###CSREC ReadWait Timeout\n" );

            //if( ++error_count > 10 )
            //{
            //   lv_record_obj.is_stopped = 1;
            //}
        }
		//pthread_mutex_unlock(&pvr_mutex);
    }

    //if( lv_record_obj.record_status != CSMPR_REC_IDLE )
    //{
    //   CSMPR_DBG_DEBUG("###Stop recording\n");
    //   CSREC_Stop( (CSREC_HANDLE)pvr_obj );
    //}

    printf("\n*****************record_task stopped...\n");
    return 0;
}

static int stream_task( int *param )
{
    printf("\n*****************stream_task started...\n");

    int rc;                    /* holds return code of system calls */

	/*************************v38********/
	//int listenfd = 0, connfd = 0;
    //struct sockaddr_in serv_addr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    int yes = 1;
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
    {
	perror("setsockopt");
	return( CSMPR_ERROR );
    }


    rc =  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
      if (rc == -1) {
	fprintf(stderr, "unable to bind to socket: %s\n", strerror(errno));
	//close(listenfd);
	return( CSMPR_ERROR );
      }


    //listen(listenfd, 10);
    rc = listen(listenfd, 10);
    if (rc == -1) {
      fprintf(stderr, "listen failed: %s\n", strerror(errno));
      return( CSMPR_ERROR );
    }





    signal(SIGPIPE, SIG_IGN);


    char sendBuff[MAX_BUFF];
    int    temp_read_len, read_len = 0;

    int bytes_written=0;

  while(1){

      printf("WAITING FOR NEW CONNECTION...while(1) icindeyim...\n");

      lv_record_obj.is_stopped =0;

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    snprintf(sendBuff, MAX_BUFF,
				"HTTP/1.1 200 OK\r\nContent-type: video/ts\r\nCache-Control: no-cache\r\n\r\n");
		write(connfd, sendBuff, MAX_BUFF);


    while( !lv_record_obj.is_stopped )
    {

        //printf("while( !lv_record_obj.is_stopped ) icindeyim...\n");

        //pthread_mutex_lock(&pvr_mutex);
        if( CSDEMUX_Filter_ReadWait( lv_record_obj.params.secft_handle, 50 ) == CSAPI_SUCCEED )
        {
            if( CSDEMUX_Filter_CheckDataSize( lv_record_obj.params.secft_handle, &read_len ) == CSAPI_SUCCEED )
            {
                temp_read_len = read_len;
                while( read_len > 0 )
                {
                    //printf("while( read_len > 0 ) icindeyim...\n");

                    if( CSDEMUX_Filter_ReadData( lv_record_obj.params.secft_handle, lv_record_obj.record_buf, &read_len ) == CSAPI_SUCCEED )
                    {
                        //printf("******Filtreden okunan = %d ********\n", read_len);

                        //error_count = 0;

                        if( temp_read_len != read_len )
                        {
                            CSMPR_DBG_ERROR( "####%d=%d\n", temp_read_len, read_len );
                        }

                        remux_pat( lv_record_obj.record_buf, read_len, lv_record_obj.params.pmt_pid );

                        //printf("read_len = %d\n", read_len);

                          void *p = lv_record_obj.record_buf;
                          //printf("p is %p\n", p);

                          while (read_len > 0) {
                          bytes_written = write(connfd, p, read_len);
                          //printf("bytes_written=%d \n", bytes_written);
                          //bytes_written = fwrite(lv_record_obj.record_buf, 1, read_len, myoutfile);
                          if (bytes_written <= 0) {
                              printf("Can not write to output file anymore!!!\n");
                              //break;
                              if ( connfd || listenfd ){

                                close(connfd);
                                //close(listenfd);

                                }
                              lv_record_obj.is_stopped =1;
                              /*stop_record();

                              CSMPR_Record_Stop();
                              return -1;*/
                              read_len=0;
                              bytes_written=0;
                              break;
                          }
                          read_len -= bytes_written;
                          p += bytes_written;

                          }



                          //printf("while disinda... bytes_written=%d \n", bytes_written);

                          read_len=0;
                          bytes_written=0;





                    }
                } //end of while

            }
        }
        else
        {
            CSMPR_DBG_DEBUG( "###CSREC ReadWait Timeout\n" );

            //if( ++error_count > 10 )
            //{
            //   lv_record_obj.is_stopped = 1;
            //}
        }
		//pthread_mutex_unlock(&pvr_mutex);
    }

    //if( lv_record_obj.record_status != CSMPR_REC_IDLE )
    //{
    //   CSMPR_DBG_DEBUG("###Stop recording\n");
    //   CSREC_Stop( (CSREC_HANDLE)pvr_obj );
    //}
    printf("****Client disconnected, going back to accept block....\n");
    sleep(1);
    //break;
  }
    printf("\n*****************stream_task stopped...\n");
    return 0;

}

CSMPR_RESULT CSMPR_Record_Init( void )
{
	memset ( (void *)&lv_record_obj, 0x00, sizeof(CSREC_OBJ));

	if( lv_record_obj.record_status != CSMPR_REC_INIT )
	{
		return( CSMPR_ERROR );
	}

	memset( (void *)&lv_record_obj, 0, sizeof(CSREC_OBJ) );

	lv_record_obj.record_buf = malloc( REC_BUFF_SZ );

	if( lv_record_obj.record_buf == NULL )
	{
		CSMPR_DBG_ERROR( "%s: Failed to allocate mem.\n", __FUNCTION__ );
		return( CSMPR_ERROR );
	}

	lv_record_obj.record_status = CSMPR_REC_IDLE;

	pthread_mutex_init( &pvr_mutex, NULL );

	return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Record_Term( void )
{
    CSMPR_DBG_TRACE;

    if( lv_record_obj.record_status == CSMPR_REC_RUN || lv_record_obj.record_status == CSMPR_REC_PAUSE )
    {
        CSMPR_Record_Stop();
    }

    if( lv_record_obj.record_buf != NULL )
    {
       free( lv_record_obj.record_buf );
       lv_record_obj.record_buf = NULL;
    }

    pthread_mutex_destroy( &pvr_mutex );
	lv_record_obj.record_status = CSMPR_REC_INIT;

    return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Record_Start( void )
{
    tCS_DB_ServiceManageData 	Manage_data;
    MV_stServiceInfo       		ServiceData;
    tCS_DB_Error             	DbResult;
    int                      	nCurrentIndex = 0;
    int                      	cnt = 0;
	char						ServiceName[30];

    CSMPR_DBG_TRACE;

    if( lv_record_obj.record_status == CSMPR_REC_RUN ||
        lv_record_obj.record_status == CSMPR_REC_PAUSE )
    {
	    return( CSMPR_ERROR );
    }

	if ( check_file_exist(CSMPR_USB_MOUNT_DIR) == FALSE )
	{
		char Temp_Str[50];

		memset(Temp_Str, 0x00, 50);
		sprintf(Temp_Str, "mkdir %s", CSMPR_USB_MOUNT_DIR);
		system(Temp_Str);
	}

    pthread_mutex_lock( &pvr_mutex );

    // Get service information (video pid, audio pid, etc.)
    nCurrentIndex = CS_DB_GetCurrentService_OrderIndex();
    DbResult      = CS_DB_GetCurrentList_ServiceData( &Manage_data, nCurrentIndex );

    if( DbResult == eCS_DB_OK )
    {
        DbResult = MV_DB_GetServiceDataByIndex( &ServiceData, Manage_data.Service_Index );
    }
    else
    {
        CSMPR_DBG_ERROR( "Failed to get service info.\n" );
	    return( CSMPR_ERROR );
    }

    // Get file name:
    memset(ServiceName, 0x00, 30);
	cnt = 0;
	while(ServiceData.acServiceName[cnt])
	{
		if ( ServiceData.acServiceName[cnt] == '/' )
			ServiceName[cnt] = '-';
		else
			ServiceName[cnt] = ServiceData.acServiceName[cnt];
		cnt++;
	}

    //generate_file_name( ServiceData.acServiceName,
    //                           lv_record_obj.params.filename );
    generate_file_name( ServiceName, lv_record_obj.params.filename );

    CSMPR_DBG_DEBUG( "PVR Record Filename: %s\n", lv_record_obj.params.filename);
    CSMPR_DBG_DEBUG( "video pid = 0x%x, audio pid = 0x%x, pmt_pid = 0x%x.\r\n", ServiceData.u16VideoPid, ServiceData.u16AudioPid, ServiceData.u16PMTPid);

    // Store the pmt pid (for remux use)
    lv_record_obj.params.pmt_pid = ServiceData.u16PMTPid;
    CSMPR_DBG_DEBUG( "CSPVR_Record: pmt_pid=0x%08x\n", lv_record_obj.params.pmt_pid );

    // Open file to write:
    lv_record_obj.record_fp = fopen( lv_record_obj.params.filename, "w+b" );

    if( lv_record_obj.record_fp == NULL )
    {
        CSMPR_DBG_ERROR( "Failed to open file" );
	    return( CSMPR_ERROR );
    }

    // Configure filters:
    lv_record_obj.params.pid_list[0].pid_val  = ServiceData.u16VideoPid;
    lv_record_obj.params.pid_list[0].pid_type = PID_TYPE_VIDEOTS;
    lv_record_obj.params.pid_list[1].pid_val  = ServiceData.u16AudioPid;
    lv_record_obj.params.pid_list[1].pid_type = PID_TYPE_AUDIOTS;
    lv_record_obj.params.pid_list[2].pid_val  = PAT_PID;
    lv_record_obj.params.pid_list[2].pid_type = PID_TYPE_PRIVATE;
    lv_record_obj.params.pid_list[3].pid_val  = ServiceData.u16PMTPid;
    lv_record_obj.params.pid_list[3].pid_type = PID_TYPE_PRIVATE;
    lv_record_obj.params.pid_num   = 4;
    lv_record_obj.params.pidft_num = 4;

    lv_record_obj.params.pidft_handle[0] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID61 );
    lv_record_obj.params.pidft_handle[1] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID60 );
    lv_record_obj.params.pidft_handle[2] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID59 );
    lv_record_obj.params.pidft_handle[3] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID58 );

    lv_record_obj.params.secft_handle    = CSDEMUX_Filter_Open( DEMUX_FILTER_ID15 );

    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[0], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[1], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[2], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[3], DEMUX_CHL_ID0 );

    CSDEMUX_Filter_SetFilterType( lv_record_obj.params.secft_handle, DEMUX_FILTER_TYPE_TS );

    for( cnt = 0; cnt < lv_record_obj.params.pidft_num; cnt++ )
    {
        CSDEMUX_PIDFT_SetPID(   lv_record_obj.params.pidft_handle[cnt], lv_record_obj.params.pid_list[cnt].pid_val );
        CSDEMUX_Filter_AddPID2( lv_record_obj.params.secft_handle,      lv_record_obj.params.pid_list[cnt].pid_val, cnt );
        CSDEMUX_PIDFT_Enable(   lv_record_obj.params.pidft_handle[cnt] );
    }

    CSDEMUX_Filter_Enable( lv_record_obj.params.secft_handle );

    // Reset counters:
    lv_record_obj.record_sec  = 0;
    lv_record_obj.is_stopped  = 0;
    lv_record_obj.written_cnt = 0;
    lv_record_obj.file_no	   = 0;
    gettimeofday( &start, NULL );

    // Start:
    pthread_create( &lv_record_obj.record_task, NULL, (void *)record_task, &lv_record_obj );
    //set_pthread_prioity( lv_record_obj.record_task, 0x60);

    lv_record_obj.record_status = CSMPR_REC_RUN;

    pthread_mutex_unlock( &pvr_mutex );

    CSMPR_DBG_DEBUG( "============= PVR Record Start OK!!==============\n" );

    return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Streaming_Start( void )
{

    printf("Streaming basliyor...\n");

    tCS_DB_ServiceManageData 	Manage_data;
    MV_stServiceInfo       		ServiceData;
    tCS_DB_Error             	DbResult;
    int                      	nCurrentIndex = 0;
    int                      	cnt = 0;
	char						ServiceName[30];
	//int rc;                    /* holds return code of system calls */

	/*************************v38********/


    CSMPR_DBG_TRACE;

    if( lv_record_obj.record_status == CSMPR_REC_RUN ||
        lv_record_obj.record_status == CSMPR_REC_PAUSE )
    {
	    return( CSMPR_ERROR );
    }



    pthread_mutex_lock( &pvr_mutex );

    // Get service information (video pid, audio pid, etc.)
    nCurrentIndex = CS_DB_GetCurrentService_OrderIndex();
    DbResult      = CS_DB_GetCurrentList_ServiceData( &Manage_data, nCurrentIndex );

    if( DbResult == eCS_DB_OK )
    {
        DbResult = MV_DB_GetServiceDataByIndex( &ServiceData, Manage_data.Service_Index );
    }
    else
    {
        CSMPR_DBG_ERROR( "Failed to get service info.\n" );
	    return( CSMPR_ERROR );
    }

    CSMPR_DBG_DEBUG( "video pid = 0x%x, audio pid = 0x%x, pmt_pid = 0x%x.\r\n", ServiceData.u16VideoPid, ServiceData.u16AudioPid, ServiceData.u16PMTPid);

    // Store the pmt pid (for remux use)
    lv_record_obj.params.pmt_pid = ServiceData.u16PMTPid;
    CSMPR_DBG_DEBUG( "CSPVR_Record: pmt_pid=0x%08x\n", lv_record_obj.params.pmt_pid );

    // Configure filters:
    lv_record_obj.params.pid_list[0].pid_val  = ServiceData.u16VideoPid;
    lv_record_obj.params.pid_list[0].pid_type = PID_TYPE_VIDEOTS;
    lv_record_obj.params.pid_list[1].pid_val  = ServiceData.u16AudioPid;
    lv_record_obj.params.pid_list[1].pid_type = PID_TYPE_AUDIOTS;
    lv_record_obj.params.pid_list[2].pid_val  = PAT_PID;
    lv_record_obj.params.pid_list[2].pid_type = PID_TYPE_PRIVATE;
    lv_record_obj.params.pid_list[3].pid_val  = ServiceData.u16PMTPid;
    lv_record_obj.params.pid_list[3].pid_type = PID_TYPE_PRIVATE;
    lv_record_obj.params.pid_num   = 4;
    lv_record_obj.params.pidft_num = 4;

    lv_record_obj.params.pidft_handle[0] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID61 );
    lv_record_obj.params.pidft_handle[1] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID60 );
    lv_record_obj.params.pidft_handle[2] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID59 );
    lv_record_obj.params.pidft_handle[3] = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID58 );

    lv_record_obj.params.secft_handle    = CSDEMUX_Filter_Open( DEMUX_FILTER_ID15 );

    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[0], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[1], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[2], DEMUX_CHL_ID0 );
    CSDEMUX_PIDFT_SetChannel( lv_record_obj.params.pidft_handle[3], DEMUX_CHL_ID0 );

    CSDEMUX_Filter_SetFilterType( lv_record_obj.params.secft_handle, DEMUX_FILTER_TYPE_TS );

    for( cnt = 0; cnt < lv_record_obj.params.pidft_num; cnt++ )
    {
        CSDEMUX_PIDFT_SetPID(   lv_record_obj.params.pidft_handle[cnt], lv_record_obj.params.pid_list[cnt].pid_val );
        CSDEMUX_Filter_AddPID2( lv_record_obj.params.secft_handle,      lv_record_obj.params.pid_list[cnt].pid_val, cnt );
        CSDEMUX_PIDFT_Enable(   lv_record_obj.params.pidft_handle[cnt] );
    }

    CSDEMUX_Filter_Enable( lv_record_obj.params.secft_handle );

    // Reset counters:
    lv_record_obj.record_sec  = 0;
    lv_record_obj.is_stopped  = 0;
    lv_record_obj.written_cnt = 0;
    lv_record_obj.file_no	   = 0;
    gettimeofday( &start, NULL );

    // Start:
    pthread_create( &lv_record_obj.record_task, NULL, (void *)stream_task, &lv_record_obj );
    //set_pthread_prioity( lv_record_obj.record_task, 0x60);

    lv_record_obj.record_status = CSMPR_REC_RUN;

    pthread_mutex_unlock( &pvr_mutex );

    CSMPR_DBG_DEBUG( "============= PVR Record Start OK!!==============\n" );

    return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Record_Stop( void )
{

    if ( connfd || listenfd ){

    close(connfd);
    close(listenfd);

    }

    int  i;

    CSMPR_DBG_TRACE;

	if( lv_record_obj.record_status != CSMPR_REC_RUN &&
        lv_record_obj.record_status != CSMPR_REC_PAUSE )
	{
        return( CSMPR_ERROR );
	}
CSMPR_DBG_DEBUG("============= PVR Record Stop 1 ==============\n");
	pthread_mutex_lock( &pvr_mutex );

	lv_record_obj.is_stopped = 1;

	//pthread_cancel( &stream_task );

	pthread_cancel( lv_record_obj.record_task );
CSMPR_DBG_DEBUG("============= PVR Record Stop 2 ==============\n");
	if( lv_record_obj.record_fp != NULL )
    {
        fclose( lv_record_obj.record_fp );
    }
//CSMPR_DBG_DEBUG("============= PVR Record Stop 3 ==============\n");
	CSDEMUX_Filter_Disable( lv_record_obj.params.secft_handle );
//CSMPR_DBG_DEBUG("============= PVR Record Stop 4 ==============\n");
    for( i = 0; i < lv_record_obj.params.pidft_num; i++ )
    {
        CSDEMUX_PIDFT_Disable(lv_record_obj.params.pidft_handle[i]);
    }
//CSMPR_DBG_DEBUG("============= PVR Record Stop 5 ==============\n");
    for( i = 0; i < lv_record_obj.params.pidft_num; i++ )
    {
        CSDEMUX_PIDFT_Close( lv_record_obj.params.pidft_handle[i] );
    }
//CSMPR_DBG_DEBUG("============= PVR Record Stop 6 ==============\n");
    CSDEMUX_Filter_Close( lv_record_obj.params.secft_handle );
//CSMPR_DBG_DEBUG("============= PVR Record Stop 7 ==============\n");
	lv_record_obj.record_status = CSMPR_REC_IDLE;

	pthread_mutex_unlock( &pvr_mutex );
    CSMPR_DBG_DEBUG("============= PVR Record Stop OK ==============\n");

    return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_Record_GetFileName( char* filename )
{
    CSMPR_DBG_TRACE;

	if( lv_record_obj.record_status == CSMPR_REC_INIT )
    {
        return( CSMPR_ERROR );
    }

	if( filename == NULL )
    {
        return( CSMPR_ERROR );
    }

	strcpy( filename, lv_record_obj.params.filename );

    return( CSMPR_SUCCESS );
}


unsigned int CSMPR_Record_GetTime( void )
{
    CSMPR_DBG_TRACE;

    if( lv_record_obj.record_status == CSMPR_REC_INIT )
    {
        return( 0 );
    }

    switch( lv_record_obj.record_status )
    {
		case CSMPR_REC_IDLE:
			lv_record_obj.record_sec = 0;
			break;

		case CSMPR_REC_RUN:
			gettimeofday( &end, NULL );
			lv_record_obj.record_sec = end.tv_sec - start.tv_sec;
			break;

		case CSMPR_REC_PAUSE:
			//lv_record_obj.record_sec = 0;
			break;

		default:
			lv_record_obj.record_sec = 0;
			break;
    }

    return( lv_record_obj.record_sec );
}


CSREC_STATUS CSMPR_Record_GetStatus( void )
{
    return( lv_record_obj.record_status );
}


unsigned int CSMPR_Record_GetCurrFileSize( void )
{
    if( lv_record_obj.record_status == CSMPR_REC_RUN )
    {
       return( lv_record_obj.written_cnt );
    }
    return( 0 );
}


CSMPR_RESULT CSMPR_Record_SetMode( CSMPR_RECORD_MODE mode )
{
    lv_record_mode = mode;
    return( CSMPR_SUCCESS );
}

CSMPR_RECORD_MODE CSMPR_Record_GetMode( void )
{
    return( lv_record_mode );
}

int MV_PVR_FileWrite_Time(char *PVR_filename, char *Event_name, U16 u16Ch_Index, eCS_PVR_FILE_STATUS eCS_Status)
{
	int 				i;
	char				Temp_Str[128];
	char				File_Str[128];
	char				Time_Str[128];
	FILE* 				fp;
	int 				retval;
	U16					CurrMjd;
	tCS_DT_Date			CurrDate;
	U16					CurrUtc;
	tCS_DT_Time			CurrTime;
	MV_stSatInfo 		SatInfo;
	MV_stTPInfo 		TPInfo;
	MV_stServiceInfo	pServiceData;

	memset(Temp_Str, 0x00, 128);
	memset(File_Str, 0x00, 128);
	memset(Time_Str, 0x00, 128);

	for ( i = strlen(PVR_filename) ; i > 0 ; i-- )
	{
		if ( PVR_filename[i] == '.' )
		{
			strncpy(Temp_Str, PVR_filename, i+1 );
			break;
		}
	}

	sprintf(File_Str, "%s%s", Temp_Str, "cfg");

	if (check_file_exist( File_Str ) == TRUE )
	{
		if (!(fp = fopen(File_Str, "a+")))
		{
			printf("\nError !!!!  Addition File : %s ==\n", File_Str);
			return 0;
		}
		printf("\nAddition File : %s ==\n", File_Str);
	} else {
		if (!(fp = fopen(File_Str, "wb")))
		{
			printf("\nError !!!!  Create File : %s ==\n", File_Str);
			return 0;
		}
		printf("\nCreate File : %s ==\n", File_Str);
	}

#if 0
	CurrMjd  = CS_DT_GetLocalMJD();
	CurrDate = CS_DT_MJDtoYMD( CurrMjd );

	CurrUtc  = CS_DT_GetLocalUTC();
	CurrTime = CS_DT_UTCtoHM( CurrUtc );
#else
	MV_OS_Get_Time_to_MJD_UTC_Date_Time(&CurrMjd, &CurrUtc, &CurrDate, &CurrTime);
#endif

	sprintf( Time_Str, "%02d/%02d/%04d %02d:%02d",
	CurrDate.day, CurrDate.month, CurrDate.year,
	CurrTime.hour, CurrTime.minute );

	MV_DB_GetServiceDataByIndex(&pServiceData, u16Ch_Index);
	MV_DB_Get_SatData_By_Chindex(&SatInfo, u16Ch_Index);
	MV_DB_Get_TPdata_By_ChNum(&TPInfo, u16Ch_Index);

	if ( eCS_Status == CS_PVR_STOP )
		sprintf(Temp_Str, "ET:%s\nED:%d\nLP:0 \n", Time_Str, CS_OS_time_Get_Sec());
	else
	{
		if ( TPInfo.u8Polar_H == 1 )
			sprintf(Temp_Str, "ID:%s\nSA:%s\nTP:%d/%c/%d\nTI:%s\nST:%s\nSD:%d \n", pServiceData.acServiceName, SatInfo.acSatelliteName, TPInfo.u16TPFrequency, 'H', TPInfo.u16SymbolRate, Event_name, Time_Str, CS_OS_time_Get_Sec());
		else
			sprintf(Temp_Str, "ID:%s\nSA:%s\nTP:%d/%c/%d\nTI:%s\nST:%s\nSD:%d \n", pServiceData.acServiceName, SatInfo.acSatelliteName, TPInfo.u16TPFrequency, 'V', TPInfo.u16SymbolRate, Event_name, Time_Str, CS_OS_time_Get_Sec());
	}

	printf("MV_PVR_FileWrite_Time : Temp_Str : \n%s\n", Temp_Str);
	retval = fwrite(Temp_Str , strlen(Temp_Str), 1, fp);
	//dprintf(("fwrite actual size = %d\n", retval));
	if (retval != 1)
	{
		printf(("\nError: write database\n"));
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

