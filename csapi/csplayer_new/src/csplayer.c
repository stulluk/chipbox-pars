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
#include <string.h>
#include "global.h"
#include "csapi.h"

#include "csplayer.h"

#define  CSPLAYER_OBJ_TYPE  	'z'

#define PLAYER_LOCK(x)     	(void)pthread_mutex_lock(&x)
#define PLAYER_UNLOCK(x)   	(void)pthread_mutex_unlock(&x)

#define PLAY_BUFF_SIZE   	200*188 //1316

typedef struct tagPLAYER_OBJ 
{
	/* public */
	char obj_type;

	CSPLAYER_InitParams init_params;
	CSPLAYER_PlayParams play_params;

	CSPLAYER_STATUS play_status;

	/* private. */
	int play_pid;
	int play_fd;
	int is_stopped;
	int is_paused;

	unsigned int file_sz;
	unsigned int file_pos;

	pthread_mutex_t player_mutex;
	pthread_t player_thread;

} CSPLAYER_OBJ;

// UNDO #if 0
// UNDO int read_cb(void *buff, int sz, void *context)
// UNDO {
// UNDO 	int rd_bytes;
// UNDO 	int fd = *(int*)context;
// UNDO 
// UNDO 	rd_bytes = read(fd, buff, sz);
// UNDO 
// UNDO 	return rd_bytes;
// UNDO }
// UNDO #endif

static void *playback_proc(void *arg)
{
	int read_bytes;
	int is_half_empty = 0;
	unsigned char *st_buffer;

	CSPLAYER_OBJ *dev_obj = arg;

	while (!dev_obj->is_stopped) 
	{
		PLAYER_LOCK(dev_obj->player_mutex);

		if (dev_obj->is_paused)
		{
			usleep(20000);
		}
		else
		{
			unsigned char st_data[PLAY_BUFF_SIZE];
			if ((read_bytes = read(dev_obj->play_fd, st_data, PLAY_BUFF_SIZE)) != PLAY_BUFF_SIZE) 
			{
				if (1 == dev_obj->play_params.loop_play_flags)
				{
 					close(dev_obj->play_fd);
					dev_obj->play_fd = open(dev_obj->play_params.filename, O_RDONLY | O_LARGEFILE);
				}
				else {
					dev_obj->is_stopped = 1;
					dev_obj->play_status = PLAY_STOP;
				}
	
				PLAYER_UNLOCK(dev_obj->player_mutex);


				continue;
			}

			if (0x47 != st_data[0])
				printf(" error. != 0x47. \n");

			if (CSAPI_FAILED == CSDEMUX_CHL_DMA_Write(dev_obj->init_params.chl_handle, st_data, read_bytes))
				usleep(20000); /* xport buffer overflow. */

// UNDO #if 1
// UNDO 			CSDEMUX_CHL_DMA_GenericWrite(dev_obj->init_params.chl_handle, read_cb, PLAY_BUFF_SIZE, &dev_obj->play_fd);
// UNDO #else
// UNDO 			CSDEMUX_CHL_DMAext_IsHalfEmpty(dev_obj->init_params.chl_handle, &is_half_empty);
// UNDO 			if(0 == is_half_empty)
// UNDO 			{
// UNDO 				CSDEMUX_CHL_DMAext_GetWPtr(dev_obj->init_params.chl_handle, (void*)&st_buffer, &read_bytes);
// UNDO 
// UNDO 				read_bytes = read(dev_obj->play_fd, st_buffer, read_bytes);
// UNDO 
// UNDO 				CSDEMUX_CHL_DMAext_Exec(dev_obj->init_params.chl_handle, st_buffer, read_bytes);
// UNDO 			}
// UNDO #endif
		}

		PLAYER_UNLOCK(dev_obj->player_mutex);
	}

	return NULL;
}

CSPLAYER_HANDLE CSPLAYER_Init(CSPLAYER_InitParams *params)
{
	CSPLAYER_OBJ *dev_obj;

	if (NULL == params) return NULL;
	if (NULL == (dev_obj = malloc(sizeof(CSPLAYER_OBJ)))) return NULL;

	memset(dev_obj, 0, sizeof(CSPLAYER_OBJ));

	memcpy(&dev_obj->init_params, params, sizeof(CSPLAYER_InitParams));
	dev_obj->obj_type = CSPLAYER_OBJ_TYPE;

	return (CSPLAYER_HANDLE) dev_obj;
}

CSAPI_RESULT CSPLAYER_Term(CSPLAYER_HANDLE handle)
{
	UNUSED_VARIABLE(handle);
	return CSAPI_SUCCEED;
}

static signed int __set_pthread_prioity__(pthread_t i_thread,  signed int i_priority)
{
	signed int i_policy = 0;
	struct sched_param param;

	i_policy = SCHED_RR;
	param.sched_priority = i_priority;

	pthread_setschedparam(i_thread, i_policy, &param);            

	return 0;
}

CSAPI_RESULT CSPLAYER_Start(CSPLAYER_HANDLE handle, CSPLAYER_PlayParams *params)
{
	struct stat statbuf;
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (NULL == params) {
        printf("CSPLAYER_Start: PLAYER params is wrong !\n");
		return CSAPI_FAILED;
    }
	if ((dev_obj->play_status == PLAY_PAUSE) || (dev_obj->play_status == PLAY_RUN)) {
		printf("CSPLAYER_Start: PLAYER status is wrong !\n");
        return CSAPI_FAILED;
	}

	memcpy(&dev_obj->play_params, params, sizeof(CSPLAYER_PlayParams));

    printf("CSPLAYER: Open file:%s...\n",dev_obj->play_params.filename);
	dev_obj->play_fd = open(dev_obj->play_params.filename, O_RDONLY | O_LARGEFILE);
	if (dev_obj->play_fd < 0){
        printf("CSPLAYER_Start: Cann't open stream: error= %s!\n",strerror(errno));
		return CSAPI_FAILED;
    }
	if (fstat(dev_obj->play_fd, &statbuf) < 0)   /* need size of input file */
	{
		/* call fstat() error. */
        printf("CSPLAYER_Start: Get file size failed\n");
		return CSAPI_FAILED;
	}

	dev_obj->file_sz = statbuf.st_size;
	dev_obj->file_pos = 0;

	dev_obj->is_stopped = 0;

	pthread_mutex_init(&dev_obj->player_mutex, NULL);
	pthread_create(&dev_obj->player_thread, NULL, playback_proc, dev_obj);
	__set_pthread_prioity__(dev_obj->player_thread, 60);

	dev_obj->play_status = PLAY_RUN;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSPLAYER_Stop(CSPLAYER_HANDLE handle)
{
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if ((dev_obj->play_status == PLAY_INIT) || (dev_obj->play_status == PLAY_PAUSE)
		|| (dev_obj->play_status == PLAY_STOP)) {
		return CSAPI_FAILED;
	}

	PLAYER_LOCK(dev_obj->player_mutex);
	dev_obj->is_stopped = 1;
	dev_obj->play_status = PLAY_STOP;
	pthread_cancel(dev_obj->player_thread);
	PLAYER_UNLOCK(dev_obj->player_mutex);
	pthread_mutex_unlock(&dev_obj->player_mutex);

	close(dev_obj->play_fd);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSPLAYER_Pause(CSPLAYER_HANDLE handle)
{
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (dev_obj->play_status == PLAY_RUN)
	{
		PLAYER_LOCK(dev_obj->player_mutex);
		dev_obj->is_paused = 1;
		dev_obj->play_status = PLAY_PAUSE;
		dev_obj->file_pos = lseek(dev_obj->play_fd, 0, SEEK_CUR);
		PLAYER_UNLOCK(dev_obj->player_mutex);

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSPLAYER_Resume(CSPLAYER_HANDLE handle)
{
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (dev_obj->play_status == PLAY_PAUSE)
	{
		PLAYER_LOCK(dev_obj->player_mutex);
		dev_obj->is_paused = 0;
		dev_obj->play_status = PLAY_RUN;
		PLAYER_UNLOCK(dev_obj->player_mutex);

		return CSAPI_SUCCEED;
	}

	return CSAPI_FAILED;
}

CSAPI_RESULT CSPLAYER_Seek(CSPLAYER_HANDLE handle, CSPLAYER_PLAY_SEEK flag, unsigned int offset)
{
	int pos;
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	UNUSED_VARIABLE(flag);

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE)) {
		return CSAPI_FAILED;
	}

	if (offset > 100) offset %= 100;

	pos = (int)((float)(dev_obj->file_sz) * ((float)(offset) / (float)(100)));
	pos -= pos%188;
	lseek(dev_obj->play_fd, pos , SEEK_SET);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSPLAYER_GetStatus(CSPLAYER_HANDLE handle, CSPLAYER_Status *status)
{
	int offset = 0;
	CSPLAYER_OBJ *dev_obj = (CSPLAYER_OBJ *) handle;

	if ((dev_obj == NULL) || (dev_obj->obj_type != CSPLAYER_OBJ_TYPE) || NULL == status) {
		return CSAPI_FAILED;
	}

	offset = lseek(dev_obj->play_fd, 0, SEEK_CUR);

	status->stat = dev_obj->play_status;
	status->cur_pos = (int)((float)(offset) * (float)(100) / (float)(dev_obj->file_sz));

	return CSAPI_SUCCEED;
}
