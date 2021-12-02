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

#include "global.h"
#include "csapi.h"

#include "csplayer.h"

#define  CSPLAYER_OBJ_TYPE  	'z'

#define PLAYER_LOCK(x)     	(void)pthread_mutex_lock(&x)
#define PLAYER_UNLOCK(x)   	(void)pthread_mutex_unlock(&x)

#define PLAY_BUFF_SIZE   	   (256*188)
#define CSPLAY_MAX_FILE_SIZE   (188*20*1024*1024L) //(188*100*1024L) //(188*10*1024*1024L)  //SAME AS CSREC_MAX_FILE_SIZE

static unsigned char st_data[PLAY_BUFF_SIZE];
static unsigned char temp_file_name[128];

typedef struct tagPLAYER_OBJ 
{
   char obj_type;

   CSPLAYER_InitParams init_params;
   CSPLAYER_PlayParams play_params;

   CSPLAYER_STATUS play_status;

   int             play_pid;
   int             play_fd;
   int             is_stopped;
   int             is_paused;
   unsigned long   duration;
   unsigned long   bitrate;

   long long       curr_file_size;
   long long       curr_file_pos;
   long long       position;
   long long       total_size;
   int             curr_file_no;
   int             total_file_no;

   pthread_mutex_t player_mutex;
   pthread_t       player_thread;

   CSVID_HANDLE    vid_handle;                            ////
   CSAUD_HANDLE    aud_handle;                            ////	
   void           (*notifier)( CSPLAYER_STATUS status ); ////
} CSPLAYER_OBJ;


static signed int __set_pthread_prioity__( pthread_t i_thread,  signed int i_priority )
{
   signed int i_policy = 0;
   struct sched_param param;

   i_policy = SCHED_RR;
   param.sched_priority = i_priority;

   pthread_setschedparam(i_thread, i_policy, &param);            

   return 0;
}


static void *playback_proc( void *arg )
{
   int            read_bytes;
   unsigned char *st_buffer;
   static int     counter = 0;
   int            SeekSize = 100;
   int            LoadSize = 10;
   long long      SeekOffset;
   unsigned char  bHookNotifier = 0;


   CSPLAYER_OBJ *dev_obj = arg;

   while( !dev_obj->is_stopped ) 
   {
      //PLAYER_LOCK(dev_obj->player_mutex);

      //printf("status = %d\n", dev_obj->play_status );

      if( dev_obj->is_paused )
      {
         usleep(20000);
      }
      else
      {
#if 1
         if( (counter++ % LoadSize) == 0 ) 
         {
            switch( dev_obj->play_status )
            {
             case PLAY_RUN:
             case PLAY_PAUSE:
             case PLAY_STOP:
               LoadSize   = 10;
               SeekOffset = 0;								   	
             break;					   
             case PLAY_FF_2X:
               LoadSize    = 10;
               SeekOffset = 0;
               
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 360000 );
               } else {
               	  usleep( 60000 );
               }
               //usleep( 100 );
             break;
             case PLAY_FF_4X:
               LoadSize = 10;				   	
               SeekOffset = 0;
               
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 100000 );
               } else {
               	  usleep( 10000 );
               }

               //usleep( 100 );				      
             break;
             case PLAY_FF_8X:
               LoadSize = 30;				   	
               SeekOffset =  LoadSize * 2* PLAY_BUFF_SIZE;
               //usleep( 40000 );
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 50000 );
               } else {
               	  usleep( 10 );
               }

             break;
             case PLAY_FF_16X:
               LoadSize = 30;				   	
               //SeekOffset =  0;
               
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  SeekOffset =  LoadSize * 2* PLAY_BUFF_SIZE;
               } else {
               	  SeekOffset =  LoadSize * 4* PLAY_BUFF_SIZE;
               }		      
             break;
             case PLAY_REW_2X:
               LoadSize = 30;
               SeekOffset =  -LoadSize * 2* PLAY_BUFF_SIZE;
               
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 210000 );
               } else {
               	  usleep( 90000 );
               }
             break;
             case PLAY_REW_4X:
               LoadSize = 30;				   	
               SeekOffset =  - LoadSize * 2 * PLAY_BUFF_SIZE;
               
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 90000 );
               } else {
               	  usleep( 30000 );
               }				  
             break;
             case PLAY_REW_8X:
               LoadSize = 30;
               SeekOffset =  - LoadSize * 2 * PLAY_BUFF_SIZE;
               if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 30000 );
               } else {
               	  usleep( 10000 );
               }				  
             break;
             case PLAY_REW_16X:
               LoadSize = 30;
               SeekOffset =  - LoadSize * 2 * PLAY_BUFF_SIZE;
                if( dev_obj->bitrate < 7*1024*1024L )
               {
                  usleep( 3000 );
               } else {
               	  usleep( 1000 );
               }				  
             break;						   
             default:
               SeekOffset = 0;
            }

            if( SeekOffset != 0 )
            {
               dev_obj->position += SeekOffset;
            }
         }

#endif	
		if( (dev_obj->play_status == PLAY_REW_2X || dev_obj->play_status == PLAY_REW_4X ||
		     dev_obj->play_status == PLAY_REW_8X || dev_obj->play_status == PLAY_REW_16X ) &&
		    (dev_obj->position < LoadSize * 2 * PLAY_BUFF_SIZE ) )
		{
		   CSVID_DisableTrickMode( dev_obj->vid_handle );
		   CSAUD_DisableMute( dev_obj->aud_handle );
		
		   lseek( dev_obj->play_fd,  0, SEEK_SET );	
		   dev_obj->position = 0;
		
		   dev_obj->play_status = PLAY_RUN;
		   bHookNotifier = 1;	
		}          

         if( (dev_obj->position / CSPLAY_MAX_FILE_SIZE) != dev_obj->curr_file_no )
         {
            close( dev_obj->play_fd );
			dev_obj->play_fd = NULL;
            
            dev_obj->curr_file_no = (dev_obj->position / CSPLAY_MAX_FILE_SIZE);
            
            if( dev_obj->curr_file_no == 0 )
               sprintf( temp_file_name, "%s", dev_obj->play_params.filename );
            else
               sprintf( temp_file_name, "%s.%03d", dev_obj->play_params.filename, dev_obj->curr_file_no );

            dev_obj->play_fd = open( temp_file_name, O_RDONLY | O_LARGEFILE ); 

            if( dev_obj->play_fd < 0 )
            {
               // Error open file. quit playback mode.
               printf("!!!!Error open next file. Quit.\n");
               dev_obj->is_stopped  = 1;
               dev_obj->play_status = PLAY_STOP;
               bHookNotifier = 1;
            }
            dev_obj->curr_file_pos = (dev_obj->position % CSPLAY_MAX_FILE_SIZE);            
            lseek( dev_obj->play_fd, dev_obj->curr_file_pos, SEEK_SET );            
         }
         else 
         {
            if( SeekOffset != 0 )
            {
               dev_obj->curr_file_pos = (dev_obj->position % CSPLAY_MAX_FILE_SIZE);         
               lseek( dev_obj->play_fd, dev_obj->curr_file_pos, SEEK_SET );
            }
         }


         read_bytes = read( dev_obj->play_fd, st_data, PLAY_BUFF_SIZE );
         
         dev_obj->curr_file_pos += read_bytes;
         dev_obj->position      += read_bytes;
                  
         if( read_bytes != PLAY_BUFF_SIZE )
         {
            close( dev_obj->play_fd );
			dev_obj->play_fd = NULL;

            // Error open file. quit playback mode.
            printf("!!!!Error read file. Quit.\n");
            dev_obj->is_stopped = 1;
            dev_obj->play_status = PLAY_STOP;
            bHookNotifier = 1;
	
				// PLAYER_UNLOCK(dev_obj->player_mutex);
				// continue;
         }

         if( 0x47 != st_data[0] )
         {   printf(" error. != 0x47. \n");
         }
         
         if( CSAPI_FAILED == CSDEMUX_CHL_DMA_Write(dev_obj->init_params.chl_handle, st_data, read_bytes) )
         {  
         	usleep(20000); 
         }
		}

        //PLAYER_UNLOCK(dev_obj->player_mutex);

        if( bHookNotifier && dev_obj->notifier != NULL )
        {
          dev_obj->notifier( dev_obj->play_status );
        }
        bHookNotifier = 0;
	}
    
	return NULL;
}


CSPLAYER_HANDLE CSPLAYER_Init(CSPLAYER_InitParams *params)
{
   CSPLAYER_OBJ *dev_obj;

   if( NULL == params ) 
      return NULL;
   if( NULL == (dev_obj = malloc(sizeof(CSPLAYER_OBJ))) )
      return NULL;

   memset(dev_obj, 0, sizeof(CSPLAYER_OBJ));

   memcpy(&dev_obj->init_params, params, sizeof(CSPLAYER_InitParams));
   dev_obj->obj_type = CSPLAYER_OBJ_TYPE;

   return (CSPLAYER_HANDLE) dev_obj;
}

CSAPI_RESULT CSPLAYER_Term( CSPLAYER_HANDLE handle )
{
   UNUSED_VARIABLE(handle);
   return CSAPI_SUCCEED;
}

CSAPI_RESULT CSPLAYER_Start( CSPLAYER_HANDLE handle, CSPLAYER_PlayParams *params )
{
   struct stat   statbuf;
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;
   int           ii;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) )
   {
      return CSAPI_FAILED;
   }

   if (NULL == params) 
   {
      return CSAPI_FAILED;
   }

   if( (dev_obj->play_status == PLAY_PAUSE) || (dev_obj->play_status == PLAY_RUN) ) 
   {
      return CSAPI_FAILED;
   }

   memcpy( &dev_obj->play_params, params, sizeof(CSPLAYER_PlayParams) );

   dev_obj->play_fd = open( dev_obj->play_params.filename, O_RDONLY | O_LARGEFILE );
   if( dev_obj->play_fd < 0 )
   {   
      return CSAPI_FAILED;
   }

   if( fstat(dev_obj->play_fd, &statbuf) < 0 )
   {
      return CSAPI_FAILED;
   }
   
   dev_obj->is_stopped     = 0;
   //
   dev_obj->vid_handle     = params->vid_handle;
   dev_obj->aud_handle     = params->aud_handle;	
   dev_obj->notifier       = params->notifier;	
   //

   dev_obj->duration       = params->duration;   // TODO: to be fixed.
   dev_obj->curr_file_size = statbuf.st_size;
   dev_obj->curr_file_pos  = 0;
   dev_obj->position       = 0;   
   dev_obj->curr_file_no   = 0;
   
   if( dev_obj->duration > 0 )
   {  
      dev_obj->bitrate = dev_obj->curr_file_size * 8 / dev_obj->duration;
   } else {
      dev_obj->bitrate = 0;
   }
   printf( "dev_obj->bitrate=%d\n", dev_obj->bitrate );

   // count how total file number and total file size.
   dev_obj->total_file_no  = 1;
   dev_obj->total_size     = dev_obj->curr_file_size;
   ii = 1;  // count from one.
   
   do
   {
      sprintf( temp_file_name, "%s.%03d", dev_obj->play_params.filename, ii );
      printf("####Check file exist %s\n", temp_file_name );
      if( access( temp_file_name, 0 ) == 0 )
      {
         dev_obj->total_file_no++;
         if( stat( temp_file_name, &statbuf ) == 0 )
         {
            dev_obj->total_size += statbuf.st_size;
         }
      } 
      else
      {
         break;
      }
      ii++;
      
   } while(1);
   
   dev_obj->duration = dev_obj->duration * dev_obj->curr_file_size / dev_obj->total_size;
   printf( "Total Duration = %d\n", dev_obj->duration );

   printf("Total file number = %d, total file size = %d.\n", dev_obj->total_file_no, dev_obj->total_size );

   pthread_mutex_init( &dev_obj->player_mutex, NULL );
   pthread_create( &dev_obj->player_thread, NULL, playback_proc, dev_obj );
   __set_pthread_prioity__( dev_obj->player_thread, 60 );

   dev_obj->play_status = PLAY_RUN;

   return CSAPI_SUCCEED;
}


CSAPI_RESULT CSPLAYER_Stop( CSPLAYER_HANDLE handle )
{
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) ) 
   {
      printf("CSPLAYER_Stop().Error dev_obj.\n");
      return CSAPI_FAILED;
   }

   if( (dev_obj->play_status == PLAY_INIT) ||
       (dev_obj->play_status == PLAY_STOP) ) 
   {
      printf("Warning CSPLAYER_Stop(). play_status = %d.\n", dev_obj->play_status );   
      //return CSAPI_SUCCEED;
   }

   PLAYER_LOCK(dev_obj->player_mutex);
   dev_obj->is_paused = 0;
   dev_obj->is_stopped = 1;
   dev_obj->play_status = PLAY_STOP;
   pthread_cancel(dev_obj->player_thread);
   PLAYER_UNLOCK(dev_obj->player_mutex);	
   //pthread_mutex_unlock(&dev_obj->player_mutex);

   if( dev_obj->play_fd != NULL )
   {
      close(dev_obj->play_fd);
      dev_obj->play_fd = NULL;
   }

   return CSAPI_SUCCEED;
}


CSAPI_RESULT CSPLAYER_Pause( CSPLAYER_HANDLE handle )
{
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) 
   {
      return CSAPI_FAILED;
   }

   if( dev_obj->play_status == PLAY_RUN )
   {
      PLAYER_LOCK(dev_obj->player_mutex);
      dev_obj->is_paused = 1;
      dev_obj->play_status = PLAY_PAUSE;
      dev_obj->curr_file_pos = lseek(dev_obj->play_fd, 0, SEEK_CUR);
      PLAYER_UNLOCK(dev_obj->player_mutex);

      return CSAPI_SUCCEED;
   }

   return CSAPI_FAILED;
}


CSAPI_RESULT CSPLAYER_Resume( CSPLAYER_HANDLE handle )
{
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) ) 
   {
      return CSAPI_FAILED;
   }

   if(  dev_obj->play_status == PLAY_PAUSE 
      ||dev_obj->play_status == PLAY_FF_2X
      ||dev_obj->play_status == PLAY_FF_4X
      ||dev_obj->play_status == PLAY_FF_8X
      ||dev_obj->play_status == PLAY_FF_16X
      ||dev_obj->play_status == PLAY_REW_2X
      ||dev_obj->play_status == PLAY_REW_4X
      ||dev_obj->play_status == PLAY_REW_8X
      ||dev_obj->play_status == PLAY_REW_16X )
   {
      PLAYER_LOCK( dev_obj->player_mutex );
      dev_obj->is_paused = 0;
      dev_obj->play_status = PLAY_RUN;
      PLAYER_UNLOCK( dev_obj->player_mutex );

      return CSAPI_SUCCEED;
   }
   return CSAPI_FAILED;
}


CSAPI_RESULT CSPLAYER_Seek( CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SEEK flag, unsigned int offset )
{
   long long     pos;
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   UNUSED_VARIABLE(flag);

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) ) 
   {
      return CSAPI_FAILED;
   }

   if( offset > 100 ) 
      offset %= 100;

   pos  = dev_obj->curr_file_size * offset / 100;
   pos -= pos%188;
   lseek( dev_obj->play_fd, pos, SEEK_SET );

   return CSAPI_SUCCEED;
}


CSAPI_RESULT CSPLAYER_FF( CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SPEED ff_params )
{
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) )
   {
      return CSAPI_FAILED;
   }

   switch( ff_params )
   {
    case PLAY_SPEED_2X:
      dev_obj->play_status = PLAY_FF_2X;
    break;
    case PLAY_SPEED_4X:
      dev_obj->play_status = PLAY_FF_4X;
    break;
    case PLAY_SPEED_8X:
      dev_obj->play_status = PLAY_FF_8X;
    break;
    case PLAY_SPEED_16X:
      dev_obj->play_status = PLAY_FF_16X;
    break;
    default:
      dev_obj->play_status = PLAY_RUN;	
    break;  
   }
   
   printf("######CSPLAYER_FF called, speed = %d.\n", ff_params );

   return CSAPI_SUCCEED;
}


CSAPI_RESULT CSPLAYER_REW( CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SPEED rew_params )
{
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) )
   {
      return CSAPI_FAILED;
   }
	
   switch( rew_params )
   {
    case PLAY_SPEED_R2X:
      dev_obj->play_status = PLAY_REW_2X;
    break;
    case PLAY_SPEED_R4X:
      dev_obj->play_status = PLAY_REW_4X;
    break;
    case PLAY_SPEED_R8X:
      dev_obj->play_status = PLAY_REW_8X;
    break;
    case PLAY_SPEED_R16X:
      dev_obj->play_status = PLAY_REW_16X;
    break;
    default:
      dev_obj->play_status = PLAY_RUN;	
    break;  
   }

   printf("######CSPLAYER_REW called, speed = %d.\n", rew_params );
	
	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSPLAYER_GetStatus( CSPLAYER_HANDLE handle, CSPLAYER_Status *status )
{
   //long long     position;
   CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

   if( (dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) || NULL == status ) 
   {
      return CSAPI_FAILED;
   }

   //dev_obj->curr_file_pos = lseek( dev_obj->play_fd, 0, SEEK_CUR );
   //position = dev_obj->curr_file_pos;

   status->stat     = dev_obj->play_status;
   status->duration = dev_obj->duration;
   
   if( dev_obj->total_size > 0 )
   {
      status->cur_pos  = dev_obj->position * 100 / dev_obj->total_size;
      status->elapsed_time = dev_obj->position * dev_obj->duration / dev_obj->total_size;
      return CSAPI_SUCCEED;
   } 
   else 
   {
      status->cur_pos  = 0;
      status->elapsed_time = 0;
      return CSAPI_FAILED;      
   }     
}

