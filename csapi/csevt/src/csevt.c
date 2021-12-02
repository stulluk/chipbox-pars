
#include <errno.h>     
#include <string.h>    
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h> 
#include <unistd.h>     
#include <assert.h>

#include <pthread.h>

#include "csevt.h"

#ifdef CSEVT_DEBUG
#define DEBUG_PRINTF  printf
#else
#define DEBUG_PRINTF(fmt,args...)
#endif

#define MAX_NUM_FDS      	1024
#define MAX_NUM_EVT_DEPTH	8

#define FDS_LOCK(x)	do { \
						DEBUG_PRINTF("thread id : %d,function: %s,line: %d, before lock\n",getpid(),__FUNCTION__,__LINE__);\
						(void)pthread_mutex_lock(&x);\
						DEBUG_PRINTF("thread id : %d,function: %s,line: %d, after lock\n",getpid(),__FUNCTION__,__LINE__);\
					}while(0)
						
#define FDS_UNLOCK(x)	do { \
							DEBUG_PRINTF("thread id : %d,function: %s,line: %d, before unlock\n",getpid(),__FUNCTION__,__LINE__);\
							(void)pthread_mutex_unlock(&x);\
							DEBUG_PRINTF("thread id : %d,function: %s,line: %d, after unlock\n",getpid(),__FUNCTION__,__LINE__);\
						}while(0)

struct fds_data
{
   	int fd;
   	int events;
	int dispatch_flags;

   	event_proc p_callback;
   	void *p_user;
};

typedef struct tagEVT_OBJECT 
{
   	int max_fd;
	int is_stopped;
	pthread_t evt_thread;
	pthread_mutex_t evt_mutex;

	struct fds_data flist[MAX_NUM_FDS];
} CSEVT_OBJECT;

static void *notify_proc(void *arg);

static void update_max_fd(CSEVT_OBJECT *evt_obj);
static void fds_free(CSEVT_OBJECT *evt_obj, struct fds_data *pf);
static struct fds_data *fds_find(CSEVT_OBJECT *evt_obj, int fd);

CSEVT_HANDLE CSEVT_Init(void)
{
	int ii;
	CSEVT_OBJECT *evt_obj;
	
	if (NULL == (evt_obj = malloc(sizeof(CSEVT_OBJECT))))
		return NULL;

	memset(evt_obj, 0, sizeof(CSEVT_OBJECT));
	
	for (ii = 0; ii < MAX_NUM_FDS; ii++)
		evt_obj->flist[ii].fd = -1;

	pthread_mutex_init(&evt_obj->evt_mutex, NULL);
	pthread_create(&evt_obj->evt_thread, NULL, notify_proc, evt_obj);
	
	return evt_obj;
}

CSAPI_RESULT CSEVT_Term(CSEVT_HANDLE handle)
{
	CSEVT_OBJECT *evt_obj = handle;

	if (NULL == evt_obj)
		return CSAPI_FAILED;
	
	evt_obj->is_stopped = 1;
	
	pthread_join (evt_obj->evt_thread, NULL);
	pthread_mutex_destroy(&evt_obj->evt_mutex);

	free(evt_obj);
	evt_obj = NULL;
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSEVT_Register(CSEVT_HANDLE handle, int fd, event_proc func, void *puser, int events)
{
	struct fds_data *pf;
	CSAPI_RESULT retval = CSAPI_FAILED;

	CSEVT_OBJECT *evt_obj = handle;
	
	if (NULL == evt_obj) 
		return CSAPI_FAILED;

	if ((fd >= 0) && (fd < MAX_NUM_FDS))
	{
		FDS_LOCK(evt_obj->evt_mutex);

		/* Get existed or new entry */
		pf = fds_find(evt_obj, fd);

		if (pf == NULL)
		{
			return CSAPI_FAILED; /* no available entry be found. */
		}

		/* Update/populate the data */
		if (pf != NULL)
		{
			pf->fd         = fd;
			pf->events     = events;
			pf->p_callback = func;
			pf->p_user     = puser;

			pf->dispatch_flags = 0;

			/* update the max fd */
			if (fd >= evt_obj->max_fd)
			{
				evt_obj->max_fd = fd;
			}

			retval = CSAPI_SUCCEED;
		}

		FDS_UNLOCK(evt_obj->evt_mutex);
	}

	return retval;
}


CSAPI_RESULT CSEVT_UnRegister(CSEVT_HANDLE handle, int fd)
{
	struct fds_data *pf;
	CSAPI_RESULT retval = CSAPI_FAILED;

	CSEVT_OBJECT *evt_obj = handle;

	if (NULL == evt_obj) 
		return CSAPI_FAILED;

	if ((fd >= 0) && (fd < MAX_NUM_FDS))
	{
		FDS_LOCK(evt_obj->evt_mutex);

		pf = fds_find(evt_obj, fd);
		if (pf != NULL)
		{
			fds_free(evt_obj, pf);
                        pf = NULL;
                        
			/* If this was the max, find the new max */
			if (evt_obj->max_fd == fd)
			{
				update_max_fd(evt_obj);
			}

			retval = CSAPI_SUCCEED;
		}

		FDS_UNLOCK(evt_obj->evt_mutex);
	}

	return(retval);
}

CSAPI_RESULT CSEVT_ReportEvent(CSEVT_HANDLE handle, int fd, int events, void *context)
{
	struct fds_data *pf;

	CSEVT_OBJECT *evt_obj = handle;
	if (NULL == evt_obj) 
		return CSAPI_FAILED;

        UNUSED_VARIABLE(events);
        
	if ((fd >= 0) && (fd < MAX_NUM_FDS))
	{
		FDS_LOCK(evt_obj->evt_mutex);

		/* Get existing or new entry */
		pf = fds_find(evt_obj, fd);

		if (pf != NULL)
		{
			pf->p_user = context;
			pf->dispatch_flags = 1;
		}

		FDS_UNLOCK(evt_obj->evt_mutex);
	}

	return CSAPI_SUCCEED;
}

static struct fds_data *fds_find(CSEVT_OBJECT *evt_obj, int fd)
{
	int ii;

	/* find an existed entry */
	for (ii = 0; ii < MAX_NUM_FDS; ii++)
	{
		if (fd == evt_obj->flist[ii].fd)
			return &(evt_obj->flist[ii]);
	}

	/* find the first empty entry */
	for (ii = 0; ii < MAX_NUM_FDS; ii++)
	{
		if (-1 == evt_obj->flist[ii].fd)
			return &(evt_obj->flist[ii]);
	}
	
	return(NULL);
}

static void fds_free(CSEVT_OBJECT *evt_obj, struct fds_data *pf)
{
	UNUSED_VARIABLE(evt_obj);

        memset(pf,0,sizeof(struct fds_data));
	pf->fd = -1;
}

static void update_max_fd(CSEVT_OBJECT *evt_obj)
{
	int ii;

	for (ii = 0; ii < MAX_NUM_FDS; ii++)
	{
		if (evt_obj->flist[ii].fd > evt_obj->max_fd)
			evt_obj->max_fd = evt_obj->flist[ii].fd;
	}
}

static void *notify_proc(void *arg)
{
	int sel_ret, ii;
	int tmp_event, ms_wait = 10;

	struct fds_data *pf;

	struct timeval sto;
	fd_set rd_set, wr_set, ex_set;

	CSEVT_OBJECT *evt_obj = arg;

	while (!evt_obj->is_stopped) 
	{
		for (ii = 0; ii < MAX_NUM_FDS; ii++)
		{
			if (evt_obj->flist[ii].fd >= 0)
				break;
		}

		if (MAX_NUM_FDS == ii)
		{
			/* Just wait - no fds for select() */
			usleep(ms_wait * 1000);
		}
		else
		{
			FD_ZERO(&rd_set);
			FD_ZERO(&wr_set);
			FD_ZERO(&ex_set);

			FDS_LOCK(evt_obj->evt_mutex);

			/* Loop through all the entries and build the fd_sets */
			for (ii = 0; ii < MAX_NUM_FDS; ii++)
			{
				if (evt_obj->flist[ii].fd >= 0)
				{
					pf = &evt_obj->flist[ii];
					
					if (pf->events & EVT_READ)
					{
						FD_SET(pf->fd, &rd_set);
					}

					if (pf->events & EVT_WRITE)
					{
						FD_SET(pf->fd, &wr_set);
					}

					if (pf->events & EVT_EXCEPT)
					{
						FD_SET(pf->fd, &ex_set);
					}

					if (pf->events & EVT_USER)
					{
						if (1 == pf->dispatch_flags) 
						{
							if (pf->p_callback != NULL) 
							{
								FDS_UNLOCK(evt_obj->evt_mutex);
								pf->p_callback(pf->p_user, pf->fd, EVT_USER);
								FDS_LOCK(evt_obj->evt_mutex);
							}
							
							pf->dispatch_flags = 0;
						}

					}
				}
			}

			sto.tv_sec  = ms_wait / 1000;
			sto.tv_usec = (ms_wait % 1000) * 1000;

			/* Do the select */
			sel_ret = select(evt_obj->max_fd + 1, &rd_set, &wr_set, &ex_set, &sto);
			if (sel_ret == 0)
			{
				/* no fd events before the timeout, never happened shoule be */
			}
			else if (sel_ret > 0)
			{
				/* Dispatch the events */
				for (ii = 0; ii < MAX_NUM_FDS; ii++)
				{
					if (evt_obj->flist[ii].fd >= 0)
					{
						pf = &evt_obj->flist[ii];
						
						tmp_event  = FD_ISSET(pf->fd, &rd_set) ? EVT_READ : 0;
						tmp_event |= FD_ISSET(pf->fd, &wr_set) ? EVT_WRITE : 0;
						tmp_event |= FD_ISSET(pf->fd, &ex_set) ? EVT_EXCEPT : 0;

						if ((tmp_event != 0) && (pf->p_callback != NULL) && (pf->fd >= 0))
						{
							FDS_UNLOCK(evt_obj->evt_mutex);
							pf->p_callback(pf->p_user, pf->fd, tmp_event);
							FDS_LOCK(evt_obj->evt_mutex);
						}
					}
				}
			}
			else
			{
				/* A FD is bad - find the offending fd and do a callback */

				int last_errno = errno;

				if (last_errno == EBADF)
				{
					/* Poll each fd entry to try to find the bad one(s) */
					for (ii = 0; ii < MAX_NUM_FDS; ii++)
					{
						if (evt_obj->flist[ii].fd >= 0)
						{
							pf = &evt_obj->flist[ii];

							FD_ZERO(&rd_set);
							FD_SET(pf->fd, &rd_set);

							sto.tv_sec  = 0;
							sto.tv_usec = 0;

							sel_ret = select(pf->fd + 1, &rd_set, NULL, NULL, &sto);
							if (sel_ret < 0)
							{
								if ((pf->p_callback != NULL) && (pf->fd >= 0))
								{
									FDS_UNLOCK(evt_obj->evt_mutex);
									pf->p_callback(pf->p_user, pf->fd, EVT_INVALID);
									FDS_LOCK(evt_obj->evt_mutex);
								}
								fds_free(evt_obj, pf);

								pf = NULL;            
							}
						}
					}
				}
			}

			FDS_UNLOCK(evt_obj->evt_mutex);
		}

		usleep(10000);

	} /* end of while(1) */

	return NULL;
}

