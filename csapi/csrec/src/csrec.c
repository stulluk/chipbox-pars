#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "csrec.h"

#define  CSREC_OBJ_TYPE       'p'
#define	REC_BUFF_SZ          (0x100000 * 2)

#define PAT_PID 		         0x00
#define SDT_PID 		         0x11
#define TDT_PID 		         0x14
#define EIT_PID 		         0x12
#define CAT_PID 		         0x01

#define TS_SIZE 		         188
#define SECTION_LEN           1024

#define CSREC_MAX_FILE_SIZE   (188*20*1024*1024L) //(188*100*1024L) //(188*10*1024*1024L)

int private_pid_list[64];

//#define __TEST__
#ifdef  __TEST__

#define  CIRCULAR_BUFFER_SIZE (2*1024*1024)
#define  SECTOR_SIZE          (64*1024)
unsigned char acCircularBuffer[CIRCULAR_BUFFER_SIZE];

volatile unsigned char bFull  = 0;
volatile unsigned char bEmpty = 1;
volatile int  nWritePointer   = 0;
volatile int  nReadPointer    = 0;
volatile int  nFreeSpace;

#endif

typedef struct tagREC_OBJ 
{
   CSREC_InitParams params;

   char obj_type;
   char *record_buf;
   unsigned int record_sec;
   unsigned int pause_sec;

   FILE *record_fp;
   pthread_t record_task;
   pthread_t write_task; ////
   CSREC_STATUS record_status;

   int used_filter_cnt;
   int private_pid_cnt;
   int currnet_pid_order;

   int is_stopped;

   unsigned long written_cnt;  // TODO:should be long long 
   int file_no;  

} CSREC_OBJ;

unsigned char temp_filename[128];

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
   int pat_size;
   unsigned long crcw;
   int j, i;
   unsigned char *ptr;
	
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


static CSAPI_RESULT CSREC_Quit( CSREC_OBJ *pvr_obj )
{
   int i;

   printf("############ CSREC_Quit.\n");
   
	pvr_obj->is_stopped = 1;

#ifdef __TEST__
	pthread_cancel(pvr_obj->write_task);	
#endif

	if( pvr_obj->record_fp )
   {  
		fclose( pvr_obj->record_fp );
   }
   
	CSDEMUX_Filter_Disable( pvr_obj->params.secft_handle );

   for( i = 0;i < pvr_obj->params.pidft_num; i++ )
   {
      CSDEMUX_PIDFT_Disable(pvr_obj->params.pidft_handle[i]);
   }
	pvr_obj->record_status = REC_STOP;

return CSAPI_SUCCEED;
}

static pthread_mutex_t pvr_mutex;
static struct timeval start, end, pausetime, resumetime,changepidtime;

static int pvr_task(int *param)
{
   int    temp_read_len, read_len = 0;
   int    written_len, towrite_len;
   //time_t timeout_sec = 3;
   time_t starttime_sec = 0;
   int    error_count = 0;

   CSREC_OBJ *pvr_obj = (CSREC_OBJ *)param;

	pvr_obj->record_fp = fopen( pvr_obj->params.filename, "w+b");

   if (pvr_obj->record_fp == NULL) 
   {  
		return -1;
   }

   gettimeofday(&changepidtime, NULL);
   starttime_sec = changepidtime.tv_sec;
        
   while( !pvr_obj->is_stopped ) 
   {
      //pthread_mutex_lock(&pvr_mutex);
      if (CSDEMUX_Filter_ReadWait(pvr_obj->params.secft_handle, 50) == CSAPI_SUCCEED) 
      {
         if (CSDEMUX_Filter_CheckDataSize( pvr_obj->params.secft_handle, &read_len ) == CSAPI_SUCCEED) 
         {
            temp_read_len = read_len;
          while( read_len > 0 )
          {
            if( CSDEMUX_Filter_ReadData( pvr_obj->params.secft_handle, pvr_obj->record_buf, &read_len ) == CSAPI_SUCCEED ) 
            {
               error_count = 0;
               
               if( temp_read_len != read_len )
               {  
                  printf( "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d=%d\n", temp_read_len, read_len );
               }                    

               remux_pat( pvr_obj->record_buf, read_len, pvr_obj->params.pmt_pid );

               if( read_len > 0 )
               {
                  // Write to file. not allow bigger than MAX_FILE_SIZE.
                  if( pvr_obj->written_cnt + read_len > CSREC_MAX_FILE_SIZE )
                     towrite_len = CSREC_MAX_FILE_SIZE - pvr_obj->written_cnt;
                  else 
                     towrite_len = read_len;

                  written_len = fwrite( pvr_obj->record_buf, 1, towrite_len, pvr_obj->record_fp );
                  pvr_obj->written_cnt += written_len;
                  
                  // Check write error:
                  if( written_len != towrite_len )
                  {
                     printf( "Error writing file. Disk full?\n" );
                     CSREC_Quit( pvr_obj );
                     break;
                  }

                  // If the file bigger than MAX_FILE_SIZE, create a new file.
                  if( pvr_obj->written_cnt >= CSREC_MAX_FILE_SIZE )
                  {
                     // Close old file.
                     fclose( pvr_obj->record_fp );

                     pvr_obj->written_cnt = 0;
                     pvr_obj->file_no++;
                     
                     if( pvr_obj->file_no > 999 )
                     {
                        printf( "Error!! To many files?\n" );
                        CSREC_Quit( pvr_obj );
                        break;                        
                     }
                     
                     // Create new file.
                     sprintf( temp_filename, "%s.%03d", pvr_obj->params.filename, pvr_obj->file_no );
                     printf("##### Create new file (%s).\n", temp_filename);
                     pvr_obj->record_fp = fopen( temp_filename, "w+b" );

                     if( pvr_obj->record_fp == NULL )
                     {  
                        printf( "Error!! Failed to create new file. Disk full?\n" );
                        CSREC_Quit( pvr_obj );
                        break;
                     }

                     // If there is remaining data in buffer.
                     if( read_len > written_len )
                     {
                        towrite_len = read_len - written_len;
                        written_len = fwrite( pvr_obj->record_buf + written_len, 1, towrite_len, pvr_obj->record_fp );
                        pvr_obj->written_cnt += written_len;
                        
                        if( written_len != towrite_len )
                        {
                           printf( "Error writing file. Disk full?\n" );
                           CSREC_Quit( pvr_obj );
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
         printf("###CSREC ReadWait Timeout\n");        
         
         //if( ++error_count > 10 )
         //{
         //   pvr_obj->is_stopped = 1;
         //}
      }
		//pthread_mutex_unlock(&pvr_mutex);
	}

    //if( pvr_obj->record_status != REC_STOP )
    //{
    //   printf("###Stop recording\n");        
    //   CSREC_Stop( (CSREC_HANDLE)pvr_obj );
    //}    
	return 0;
}


#ifdef __TEST__
static int write_task(int *param)
{
   int        written_len = 0;        
   CSREC_OBJ *pvr_obj = (CSREC_OBJ *)param;

   printf("write_task: wait...\n");    
    
   while( pvr_obj->record_fp == NULL ) 
   { ;
   }
    
   while (!pvr_obj->is_stopped) 
   {   
		// Read:                   
		while( !bEmpty && ((nReadPointer + SECTOR_SIZE <= nWritePointer) || (nReadPointer >= nWritePointer) ) )
		{                    	
         //printf("write\n");
         written_len = fwrite( acCircularBuffer+nReadPointer, 1, SECTOR_SIZE, pvr_obj->record_fp );

         nReadPointer += SECTOR_SIZE;
         if( nReadPointer >= CIRCULAR_BUFFER_SIZE )
         {  nReadPointer = 0;
         }                         

         if( nReadPointer == nWritePointer )
         {  printf("Queue empty.\n");
            bEmpty = 1;
         }                        
         bFull = 0;
		}                    
	}

	return 0;
}
#endif


CSREC_HANDLE CSREC_Init(CSREC_InitParams * params)
{
	CSREC_OBJ *pvr_obj = NULL;
   int i = 0,cnt = 0;

   if((params->pidft_num > REQUIRED_PIDFT_NUM)||(params->pid_num > 64))
   {
      printf("error: %s %d %s\n",__FILE__,__LINE__,__FUNCTION__);
      return NULL;
   }

   for(i =0;i < params->pid_num;i++)
   {
      if(params->pid_list[i].pid_type != PID_TYPE_PRIVATE)
      {
         cnt ++;
      }
   }

   if(params->pidft_num < cnt)
   {
      printf("error: %s %d %s\n",__FILE__,__LINE__,__FUNCTION__);
      return NULL;
   }

   pvr_obj = malloc(sizeof(CSREC_OBJ));
	if (NULL == pvr_obj) 
   {
		printf("error: %s  %s\n", __FUNCTION__, strerror(errno));
		return NULL;
	}

	memset(pvr_obj, 0, sizeof(CSREC_OBJ));

	pvr_obj->record_buf = malloc(REC_BUFF_SZ);
	if (NULL == pvr_obj->record_buf) 
   {
		printf("error: %s  %s\n", __FUNCTION__, strerror(errno));
		free(pvr_obj);
		return NULL;
	}

	memcpy(&(pvr_obj->params), params, sizeof(CSREC_InitParams));

	CSDEMUX_Filter_SetFilterType(pvr_obj->params.secft_handle, DEMUX_FILTER_TYPE_TS);

	pthread_mutex_init(&pvr_mutex, NULL);

	pvr_obj->record_status = REC_INIT;
	pvr_obj->obj_type = CSREC_OBJ_TYPE;

	return (CSREC_HANDLE) pvr_obj;
}

CSAPI_RESULT CSREC_Term(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
		return CSAPI_FAILED;
	}

	if (REC_PAUSE == pvr_obj->record_status)
	{	
	   CSREC_Resume(handle);
   }
   
	if (REC_RUN == pvr_obj->record_status)
   {  
		CSREC_Stop(handle);
   }

   pthread_mutex_destroy(&pvr_mutex);

   free(pvr_obj->record_buf);
   memset(pvr_obj, 0, sizeof(CSREC_OBJ));
   free(pvr_obj);
   pvr_obj = NULL;

   return CSAPI_SUCCEED;
}


static signed int __set_pthread_prioity__(pthread_t i_thread,  signed int i_priority)
{
   signed int i_policy = 0;

   struct sched_param param;

   param.sched_priority = i_priority;
   i_policy = SCHED_RR;

   pthread_setschedparam(i_thread, i_policy, &param);            

   return 0;
}


CSAPI_RESULT CSREC_Start(CSREC_HANDLE handle)
{
   CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;
   int cnt = 0;
        
	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
		return CSAPI_FAILED;
	}

	if ((pvr_obj->record_status == REC_PAUSE) || (pvr_obj->record_status == REC_RUN)) 
   {
		return CSAPI_FAILED;
	}

   if(pvr_obj->params.pid_num == pvr_obj->params.pidft_num)
   {
      for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++)
      {
         CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[cnt], pvr_obj->params.pid_list[cnt].pid_val);
         CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, cnt);
         CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[cnt]);
      }
   }
   else if(pvr_obj->params.pid_num > pvr_obj->params.pidft_num)
   {
      for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++)
      {
         if(pvr_obj->params.pid_list[cnt].pid_type != PID_TYPE_PRIVATE)
         {
            CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt], pvr_obj->params.pid_list[cnt].pid_val);
            CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, pvr_obj->used_filter_cnt);
            CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[pvr_obj->used_filter_cnt]);
            pvr_obj->used_filter_cnt ++;
         }
         else
         {
            private_pid_list[pvr_obj->private_pid_cnt] = cnt;
            pvr_obj->private_pid_cnt ++;
         }
      }
      pvr_obj->currnet_pid_order = 0;
   }
   else
   {
      printf("PVR Warning: wanted pids are less then malloced pid filters, waste pid filter!");
      for(cnt = 0;cnt < pvr_obj->params.pid_num;cnt ++)
      {
         CSDEMUX_PIDFT_SetPID(pvr_obj->params.pidft_handle[cnt], pvr_obj->params.pid_list[cnt].pid_val);
         CSDEMUX_Filter_AddPID2(pvr_obj->params.secft_handle, pvr_obj->params.pid_list[cnt].pid_val, cnt);
         CSDEMUX_PIDFT_Enable(pvr_obj->params.pidft_handle[cnt]);
      }
   }

	CSDEMUX_Filter_Enable(pvr_obj->params.secft_handle);

	pvr_obj->record_sec  = 0;
	pvr_obj->pause_sec   = 0;	
	pvr_obj->is_stopped  = 0;
   
   pvr_obj->written_cnt = 0;
   pvr_obj->file_no	   = 0;
   
	gettimeofday(&start, NULL);
	pthread_create(&pvr_obj->record_task, NULL, (void *) pvr_task, pvr_obj);
	__set_pthread_prioity__(pvr_obj->record_task, 0x60);

#ifdef __TEST__
	pthread_create(&pvr_obj->write_task, NULL, (void *) write_task, pvr_obj );
	//__set_pthread_prioity__(pvr_obj->write_task, 0x60);
#endif

	pvr_obj->record_status = REC_RUN;

	return CSAPI_SUCCEED;
}



CSAPI_RESULT CSREC_Stop(CSREC_HANDLE handle)
{
    CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;
    int i = 0;
        
    if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
    {
		return CSAPI_FAILED;
	}

	if ((pvr_obj->record_status == REC_INIT) || (pvr_obj->record_status == REC_PAUSE)
	    || (pvr_obj->record_status == REC_STOP)) 
	{
		return CSAPI_FAILED;
	}

	pvr_obj->is_stopped = 1;

	pthread_mutex_lock(&pvr_mutex);
	pthread_cancel(pvr_obj->record_task);

#ifdef __TEST__
	pthread_cancel(pvr_obj->write_task);	
#endif

	gettimeofday(&end, NULL);

	if(pvr_obj->record_fp)
    {  
        fclose(pvr_obj->record_fp);
    }
   
	CSDEMUX_Filter_Disable(pvr_obj->params.secft_handle);

    for( i = 0;i < pvr_obj->params.pidft_num; i++ )
    {
        CSDEMUX_PIDFT_Disable(pvr_obj->params.pidft_handle[i]);
    }
	pvr_obj->record_status = REC_STOP;

	pthread_mutex_unlock(&pvr_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSREC_Pause(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
      return CSAPI_FAILED;
	}

	if (pvr_obj->record_status == REC_RUN) 
	{
		pthread_mutex_lock(&pvr_mutex);

		gettimeofday(&pausetime, NULL);
		pvr_obj->record_status = REC_PAUSE;

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSREC_Resume(CSREC_HANDLE handle)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
      return CSAPI_FAILED;
	}

	if (pvr_obj->record_status == REC_PAUSE) 
	{
		pthread_mutex_unlock(&pvr_mutex);

		gettimeofday(&resumetime, NULL);
		pvr_obj->pause_sec = resumetime.tv_sec - pausetime.tv_sec + pvr_obj->pause_sec;
		pvr_obj->record_status = REC_RUN;

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSREC_GetTime(CSREC_HANDLE handle, unsigned int *secs)
{
	CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

	if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
      return CSAPI_FAILED;
	}

   switch(pvr_obj->record_status) 
   {
    case REC_STOP:
      pvr_obj->record_sec = end.tv_sec - start.tv_sec - pvr_obj->pause_sec;
    break;

    case REC_RUN:
      gettimeofday(&end, NULL);
      pvr_obj->record_sec = end.tv_sec - start.tv_sec - pvr_obj->pause_sec;
    break;

    case REC_PAUSE:
    break;

    default:
      pvr_obj->record_sec = 0;
    break;
   }

   *secs = pvr_obj->record_sec;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSREC_GetStatus(CSREC_HANDLE handle, CSREC_STATUS * status)
{
   CSREC_OBJ *pvr_obj = (CSREC_OBJ *) handle;

   if ((pvr_obj == NULL) || (pvr_obj->obj_type != CSREC_OBJ_TYPE)) 
   {
      return CSAPI_FAILED;
   }

   *status = pvr_obj->record_status;

   return CSAPI_SUCCEED;
}

