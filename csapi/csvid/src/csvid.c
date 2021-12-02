#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "csvid.h"
#include "../csi2c/include/csi2c.h"
#if 1				/*PMF used */
#include <asm/page.h>
#include <sys/mman.h>
#define VIDOUT_ONE_DIR_SIZE      (16)
#define VIDOUT_GET_DIR_NUM(start, end) 	((end - start) / VIDOUT_ONE_DIR_SIZE)
#define MEM_SWAP32(x)	( (((x) & 0xff) << 24) | (((x) & 0xff00) <<8) |	(((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24) )
#define DMXOUT_TOGGLE_OFFSET 	(31)
#define DMXOUT_TOGGLE_BIT 	(1 << DMXOUT_TOGGLE_OFFSET)
#define DMXOUT_DIR_OFFSET 	(3)
#define USER_DATA_MAX_BLOCK_NUM (8)
#define USER_DATA_BLOCK_SIZE (0x800) // 2k

typedef void (*call_back_emptynotify) (CSVID_HANDLE *);
typedef struct {
	void *pVMem;
	void *pPMem;
	unsigned int MemSize;
	void *MappedAddr;
	unsigned int MappedSize;
	int FD;
	int emptynotify_size;
	unsigned char empty_enable;
	call_back_emptynotify empty_fn;
	int fullnotify_size;
	unsigned char full_enable;
	call_back_emptynotify full_fn;
} CSVID_MMAP_t;
CSVID_MMAP_t g_cpbmem;
CSVID_MMAP_t g_dirmem;
static unsigned int g_vid_cpb_wptr = 0;
#endif
CSVID_MMAP_t g_userdatamem;
static int userdata_rptr = 0;
//static unsigned char last_afd = 0;
unsigned char userdata_buf[USER_DATA_BLOCK_SIZE * USER_DATA_MAX_BLOCK_NUM];

#define CSVID_OBJ_TYPE  		'z'
#define CSVID_DEV_FILE			"/dev/misc/orion_video"

#define CSVID_IOC_INPUT_MODE            _IOW('z', 0x01, int)
#define CSVID_IOC_STREAM_TYPE           _IOW('z', 0x02, int)
#define CSVID_IOC_DECODER_MODE          _IOW('z', 0x03, int)
#define CSVID_IOC_PLAY_SPEED            _IOW('z', 0x04, int)
#define CSVID_IOC_FRAME_RATE            _IOW('z', 0x05, int)
#define CSVID_IOC_PLAY                  _IOW('z', 0x06, int)
#define CSVID_IOC_STOP                  _IOW('z', 0x07, int)
#define CSVID_IOC_PAUSE                 _IOW('z', 0x08, int)
#define CSVID_IOC_FREEZE                _IOW('z', 0x09, int)
#define CSVID_IOC_STEP                  _IOW('z', 0x0a, int)
#define CSVID_IOC_RESUME                _IOW('z', 0x0b, int)
#define CSVID_IOC_SKIP                  _IOW('z', 0x0c, int)

#define CSVID_IOC_PTS_ENABLE            _IOW('z', 0x11, int)
#define CSVID_IOC_PTS_DISABLE           _IOW('z', 0x12, int)

#define CSVID_IOC_DISP_ON               _IOW('z', 0x13, int)
#define CSVID_IOC_DISP_OFF              _IOW('z', 0x14, int)
#define CSVID_IOC_DISP_POS              _IOW('z', 0x15, struct vid_output_pos)
#define CSVID_IOC_DISP_ALPHA            _IOW('z', 0x16, int)
#define CSVID_IOC_DISP_BLANK            _IOW('z', 0x17, int)

#define CSVID_IOC_GET_VIDEOFMT          _IOW('z', 0x18, int)

#define CSVID_IOC_GET_TIMECODE		_IOW('z', 0x19, int)
#define CSVID_IOC_SET_TIMECODE		_IOW('z', 0x1a, int)

#define CSVID_IOC_GET_NOTIFY_TYPE   _IOW('z',0x1c,int)
#define CSVID_IOC_SET_ERROR_LEVEL   _IOW('z',0x1d,int)

#define CSVID_IOC_PMF_RESET     _IOW('z',0x1e,int)
#define CSVID_IOC_PMF_GETCPB_ADDR       _IOW('z',0x1f,int)
#define CSVID_IOC_PMF_GETCPB_SIZE       _IOW('z',0x20,int)
#define CSVID_IOC_PMF_GETDIR_ADDR       _IOW('z',0x21,int)
#define CSVID_IOC_PMF_GETDIR_SIZE       _IOW('z',0x22,int)
#define CSVID_IOC_PMF_SET_DIR_WP        _IOW('z',0x23,int)
#define CSVID_IOC_PMF_GET_DIR_WP        _IOW('z',0x24,int)
#define CSVID_IOC_PMF_SET_DIR_RP         _IOW('z',0x25,int)
#define CSVID_IOC_PMF_GET_DIR_RP         _IOW('z',0x26,int)
#define CSVID_IOC_PMF_SET_CPBDIRU0        _IOW('z',0x27,int)
#define CSVID_IOC_PMF_GET_CPBDIRU0        _IOW('z',0x28,int)
#define CSVID_IOC_PMF_SET_CPBDIRL0         _IOW('z',0x29,int)
#define CSVID_IOC_PMF_GET_CPBDIRL0         _IOW('z',0x2a,int)

#define CSVID_IOC_SET_ASPECTRATIO_STATUS         _IOW('z',0x2b,int)
#define CSVID_IOC_SET_PSCANCROP_STATUS      _IOW('z',0x2c,int)
#define CSVID_IOC_GET_PSCANCROP     _IOW('z',0x2d,int)
#define CSVID_IOC_SET_SYNC_STATUS       _IOW('z',0x2e,int)
#define CSVID_IOC_GET_ASPECTRATIO       _IOW('z',0x2f,int)
#define CSVID_IOC_TRICK_MDOE        _IOW('z',0x30,int)

#define CSVID_IOC_GET_PTS		_IOW('z', 0x35, long long)
#define CSVID_IOC_SET_FORCE_3TO2_POLLDOW        _IOW('z', 0x36, int)
#define CSVID_IOC_SET_NOWAIT_SYNC        _IOW('z', 0x37, int)
#define CSVID_IOC_SET_STARTDELAY        _IOW('z', 0x38, int)

#ifdef VIB_SUPPORT
#define CSVID_IOC_VIB_CONFIG        _IOW('z', 0x39, int)
#define CSVID_IOC_VIB_RESET        _IOW('z', 0x3a, int)
#endif
#define CSVID_IOC_SET_USER_DATA_STATUS        _IOW('z', 0x3b, int)
#define CSVID_IOC_GET_USERDATA_ADDR		_IOW('z', 0x3c, int)
#define CSVID_IOC_GET_USERDATA_SIZE	_IOW('z', 0x3d, int)
#define CSVID_IOC_SET_USERDATA_BLOCK_SIZE	_IOW('z', 0x3e, int)

#define CSVID_IOC_SET_ERROR_SKIP_MODE	_IOW('z', 0x3f, int)

#define CSVID_IOC_DISP_POS_BEIYANG		_IOW('z', 0x40, int)
#define CSVID_IOC_DISP_ALPHA_BEIYANG		_IOW('z', 0x41, int)
#define	CSVID_IOC_GET_SRC_VIDEOFMT		_IOW('z', 0x42, int)

#define CSVID_IOC_SET_UNDERFLOW_REPORT		_IOW('z', 0x43, int)
#define CSVID_IOC_SET_FORMATCHANGE_REPORT		_IOW('z', 0x44, int)
#define CSVID_IOC_SET_SRCFORMATCHANGE_REPORT		_IOW('z', 0x45, int)

pthread_t vid_callback;
pthread_mutex_t vid_mutex;
typedef void (*call_back_timecodenotify) (CSVID_HANDLE *);
typedef void (*call_back_errornotify) (CSVID_HANDLE *, CSVID_ERROR_THRESHOLD);
typedef void (*call_back_aspectrationotify)(CSVID_HANDLE *, CSVID_ASPECTRATIO *);
typedef void (* call_back_pscancropnotify)(CSVID_HANDLE *, CSVID_Rect *);
typedef void (* call_back_sync)(CSVID_HANDLE *,  signed char *);
typedef void (* call_back_userdatanotify)(CSVID_HANDLE *, unsigned char *, int, int);
typedef void (* call_back_underflow_notify)(CSVID_HANDLE *);
typedef void (* call_back_changeformat_notify)(CSVID_HANDLE *,CSVID_SequenceHeader *);

struct vid_rect {
	int left;
	int right;
	int top;
	int bottom;
};

struct vid_output_pos {
	struct vid_rect src;
	struct vid_rect dst;
} disp_pos;

typedef struct tagVID_OBJ {
	char obj_type;
	int dev_fd;

	int errno;
	int run_status;

	struct {
		unsigned int pts_enable:1;
		unsigned int trick_mode:1;
		unsigned int decode_mode:2;
		unsigned int channel_num:4;
		unsigned int frame_rate:4;
		unsigned int play_speed:5;

		unsigned int stillpicture_flag:1;
		unsigned int call_back_status:1;
		unsigned int time_code:1;
		unsigned int error_level:1;
		unsigned int afd_info:1;
		unsigned int reserved:9;
	} config_params;

	struct {
		unsigned int pic_width:14;
		unsigned int pic_height:14;
		unsigned int src_frame_rate:4;
	} format_params;

	call_back_timecodenotify tc_fn;
	call_back_errornotify error_fn;
	call_back_aspectrationotify aspectratio_fn;
	call_back_pscancropnotify pscancrop_fn;
	call_back_sync sync_fn;
	call_back_userdatanotify afd_fn;
	call_back_underflow_notify underflow_fn;
	call_back_changeformat_notify changeformat_fn;
	call_back_changeformat_notify changesrcformat_fn;

	unsigned int sync_timeout;
	struct timeval start;

	int input_mode;		/* 0 - non-file, 1 - file. */
	int stream_type;	/*  MPEG2 , H.264  and VIB are supported. */
	int decoding_mode;	/* any / only I / only IP. */
	int play_speed;

	int instance_counter;
	int num_notify;
} CSVID_OBJ;

static CSVID_OBJ csvid_obj;

static char *vid_errstr[] = {
	"VIDEO:no error",
	"VIDEO:open failed",
	"VIDEO:ioctl failed",
	"VIDEO:invalid arguments",
	"VIDEO:unknown device name",
	"VIDEO:device busy",
	"VIDEO:invalid handle",
	"VIDEO:device already initialized",
	"VIDEO:device wasn't initialized",
	"VIDEO:invalid status",
	"VIDEO:write still picture data to driver error",
	"VIDEO:play still picture error",
	"VIDEO: memory map failed"
};

static int CallBackNotify(int *param);
static int __csvid_dir_is_empty(int dir_num);

int __csvid_get_aspectratio(CSVID_HANDLE handle, CSVID_ASPECTRATIO * aspect_ratio)
{
        CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_ASPECTRATIO, aspect_ratio, VID);

        if(((*aspect_ratio) >> 28) == 1){
                *aspect_ratio&=0xff;
                if(*aspect_ratio == 2){
                        *aspect_ratio = CSVID_4TO3;
                }
                else if(*aspect_ratio == 3){
                        *aspect_ratio = CSVID_16TO9;
                }
                else{
                        *aspect_ratio = CSVID_UNKNOWN;
                }
        }
        else if(((*aspect_ratio) >> 28) == 2){
                    *aspect_ratio&=0xfffffff;
                    switch(*aspect_ratio){
                        case 1:
                            *aspect_ratio = CSVID_1TO1;
                            break;
                        case 2:
                            *aspect_ratio = CSVID_12TO11;
                            break;
                        case 3:
                            *aspect_ratio = CSVID_10TO11;
                            break;
                        case 4:
                            *aspect_ratio = CSVID_16TO11;
                            break;
                        case 5:
                            *aspect_ratio = CSVID_40TO33;
                            break;
                        case 6:
                            *aspect_ratio = CSVID_24TO11;
                            break;
                        case 7:
                            *aspect_ratio = CSVID_20TO11;
                            break;
                        case 8:
                            *aspect_ratio = CSVID_32TO11;
                            break;
                        case 9:
                            *aspect_ratio = CSVID_80TO33;
                            break;
                        case 10:
                            *aspect_ratio = CSVID_18TO11;
                            break;
                        case 11:
                            *aspect_ratio = CSVID_15TO11;
                            break;
                        case 12:
                            *aspect_ratio = CSVID_64TO33;
                            break;
                        case 13:
                            *aspect_ratio = CSVID_160TO99;
                            break;
                        default:
                            if((*aspect_ratio)>>16)
                                    *aspect_ratio = *aspect_ratio;
                            else
                                    *aspect_ratio = CSVID_UNKNOWN;
                            break;
               }
        }
	else if(((*aspect_ratio) >> 28) == 3){
		*aspect_ratio&=0xff;
		switch(*aspect_ratio){
			case 1:
				*aspect_ratio = CSVID_1_0;
				break;
			case 2:
				*aspect_ratio = CSVID_0_6735;
				break;
			case 3:
				*aspect_ratio = CSVID_0_7031;
				break;
			case 4:
				*aspect_ratio = CSVID_0_7615;
				break;
			case 5:
				*aspect_ratio = CSVID_0_8055;
				break;
			case 6:
				*aspect_ratio = CSVID_0_8437;
				break;
			case 7:
				*aspect_ratio = CSVID_0_8935;
				break;
			case 8:
				*aspect_ratio = CSVID_0_9157;
				break;
			case 9:
				*aspect_ratio = CSVID_0_9815;
				break;
			case 10:
				*aspect_ratio = CSVID_1_0255;
				break;
			case 11:
				*aspect_ratio = CSVID_1_0695;
				break;
			case 12:
				*aspect_ratio = CSVID_1_0950;
				break;
			case 13:
				*aspect_ratio = CSVID_1_1575;
				break;
			case 14:
				*aspect_ratio = CSVID_1_2015;
				break;
			default:
				*aspect_ratio = CSVID_UNKNOWN;
				break;
		}
	}
	return CSAPI_SUCCEED;
}

CSVID_HANDLE CSVID_Open(CSVID_DEV dev)
{
	unsigned int phyaddr = 0;
	unsigned int memsize = 0;
	unsigned int addroffset = 0;
	unsigned char *buf = NULL;
	unsigned int addr = 0;
	unsigned int size = 0;

	UNUSED_VARIABLE(dev);

	if(csvid_obj.instance_counter == 0){
		memset(&csvid_obj, 0, sizeof(CSVID_OBJ));
	}

	if (csvid_obj.obj_type != CSVID_OBJ_TYPE) {
		csvid_obj.dev_fd = open(CSVID_DEV_FILE, O_RDWR);

		if (csvid_obj.dev_fd < 0) {
			csvid_obj.errno = VID_ERROR_OPEN_FAILED;
			return NULL;
		}
	}
	else goto open_exit;

	ioctl(csvid_obj.dev_fd, CSVID_IOC_DISP_ON, 0);	/* FIXME@zhongkai's ugly code */

	csvid_obj.obj_type = CSVID_OBJ_TYPE;

	memset(&g_cpbmem, 0, sizeof(CSVID_MMAP_t));
	memset(&g_dirmem, 0, sizeof(CSVID_MMAP_t));
	memset(&g_userdatamem, 0, sizeof(CSVID_MMAP_t));

	ioctl(csvid_obj.dev_fd, CSVID_IOC_GET_USERDATA_ADDR, &addr);
	ioctl(csvid_obj.dev_fd, CSVID_IOC_GET_USERDATA_SIZE, &size);
	
	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + size + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;
	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, csvid_obj.dev_fd, phyaddr);
	if (buf == MAP_FAILED) {
		csvid_obj.errno = VID_ERROR_MEMORY_MAP;
		return NULL;
	}
	g_userdatamem.MappedSize = memsize;
	g_userdatamem.MappedAddr = buf;
	g_userdatamem.pPMem = (void *) addr;
	g_userdatamem.pVMem = (void *) (buf + addroffset);
	g_userdatamem.MemSize = size;
	g_userdatamem.FD = csvid_obj.dev_fd;
	userdata_rptr = addr;
	memset(userdata_buf,0,USER_DATA_BLOCK_SIZE);
	size = USER_DATA_BLOCK_SIZE;
	ioctl(csvid_obj.dev_fd, CSVID_IOC_SET_USERDATA_BLOCK_SIZE, &size);

	csvid_obj.num_notify = 0;

	pthread_mutex_init(&vid_mutex, NULL);

open_exit:
	csvid_obj.instance_counter ++;

	return (CSVID_HANDLE) & csvid_obj;
}

CSAPI_RESULT CSVID_Close(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	dev_obj->instance_counter--;
	if (dev_obj->instance_counter == 0) 
	{
		CSVID_Stop(handle);

		userdata_rptr = 0;
		munmap(g_userdatamem.MappedAddr, g_userdatamem.MappedSize);
		memset(&g_userdatamem,0,sizeof(CSVID_MMAP_t));
		memset(userdata_buf,0,USER_DATA_BLOCK_SIZE);

		IOCTL(dev_obj, CSVID_IOC_DISP_OFF, 0, VID);

		dev_obj->obj_type = 0;
		close(dev_obj->dev_fd);

		memset(&csvid_obj, 0, sizeof(CSVID_OBJ));

		pthread_mutex_destroy(&vid_mutex);
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Play(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PLAY, 0, VID);
	dev_obj->run_status = VID_STATUS_RUNNING;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Stop(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_STOP, 0, VID);
	dev_obj->run_status = VID_STATUS_STOPPED;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Pause(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

        if ((dev_obj->run_status == VID_STATUS_PAUSE)
	    || (dev_obj->run_status == VID_STATUS_FROZEN)) {
		dev_obj->errno = VID_ERROR_INVALID_STATUS;
		return CSAPI_FAILED;
	}

	
	IOCTL(dev_obj, CSVID_IOC_PAUSE, 0, VID);
	dev_obj->run_status = VID_STATUS_PAUSE;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Freeze(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

        if ((dev_obj->run_status == VID_STATUS_PAUSE)
	    || (dev_obj->run_status == VID_STATUS_FROZEN)) {
		dev_obj->errno = VID_ERROR_INVALID_STATUS;
		return CSAPI_FAILED;
	}
	// printf("CSVID_Freeze\n");
	IOCTL(dev_obj, CSVID_IOC_FREEZE, 0, VID);
	dev_obj->run_status = VID_STATUS_FROZEN;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Resume(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	if ((dev_obj->run_status != VID_STATUS_PAUSE)
	    && (dev_obj->run_status != VID_STATUS_FROZEN)) {
		dev_obj->errno = VID_ERROR_INVALID_STATUS;
		return CSAPI_FAILED;
	}

	// printf("CSVID_Resume\n");
	IOCTL(dev_obj, CSVID_IOC_RESUME, 0, VID);
	dev_obj->run_status = VID_STATUS_RUNNING;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Step(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	if ((dev_obj->run_status != VID_STATUS_PAUSE)
	    && (dev_obj->run_status != VID_STATUS_FROZEN)) {
		dev_obj->errno = VID_ERROR_INVALID_STATUS;
		return CSAPI_FAILED;
	}
	IOCTL(dev_obj, CSVID_IOC_STEP, 0, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Skip(CSVID_HANDLE handle, unsigned int ms_len)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	if (dev_obj->run_status != VID_STATUS_RUNNING) {
		dev_obj->errno = VID_ERROR_INVALID_STATUS;
		return CSAPI_FAILED;
	}
	IOCTL(dev_obj, CSVID_IOC_SKIP, ms_len, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetInputMode(CSVID_HANDLE handle, CSVID_INPUT input_mod)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_INPUT_MODE, input_mod, VID);

	dev_obj->input_mode = input_mod;

	return CSAPI_SUCCEED;
}

/* adde by wangxuewewei 2008.03.12*/
/*--------------------------------------------------------*/

CSAPI_RESULT CSVID_GetPTS(CSVID_HANDLE handle, long long *vid_pts)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_PTS, vid_pts, VID);

	return CSAPI_SUCCEED;
}
/*--------------------------------------------------------*/
CSAPI_RESULT CSVID_GetInputMode(CSVID_HANDLE handle, CSVID_INPUT * input_mod)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	*input_mod = dev_obj->input_mode;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetStreamType(CSVID_HANDLE handle, CSVID_STREAM_TYPE stream_type)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_STREAM_TYPE, stream_type, VID);

	dev_obj->stream_type = stream_type;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetStreamType(CSVID_HANDLE handle, CSVID_STREAM_TYPE * stream_type)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	*stream_type = dev_obj->stream_type;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetDecoderMode(CSVID_HANDLE handle, CSVID_DECODING_MOD decoding_mod)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_DECODER_MODE, decoding_mod, VID);

	dev_obj->decoding_mode = decoding_mod;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetDecoderMode(CSVID_HANDLE handle, CSVID_DECODING_MOD * decoding_mod)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	*decoding_mod = dev_obj->decoding_mode;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetPlaySpeed(CSVID_HANDLE handle, unsigned int speed)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PLAY_SPEED, speed, VID);

	dev_obj->play_speed = speed;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetPlaySpeed(CSVID_HANDLE handle, unsigned int *speed)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	*speed = dev_obj->play_speed;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetFrameRate(CSVID_HANDLE handle, int frame_rate)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_FRAME_RATE, frame_rate, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetFrameRate(CSVID_HANDLE handle, int *frame_rate)
{
	CSVID_SequenceHeader hdr;

	if (CSAPI_FAILED == CSVID_GetSequenceHeader(handle, &hdr))
		return CSAPI_FAILED;

	*frame_rate = hdr.frame_rate;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetSequenceHeader(CSVID_HANDLE handle, CSVID_SequenceHeader * hdr)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	int raw_framerates[] = {
		00,		// - unknown
		23,		// - 23.97 fps
		24,		// - 24.00 fps
		25,		// - 25.00 fps
		29,		// - 29.97 fps
		30,		// - 30.00 fps
		50,		// - 50.00 fps
		59,		// - 59.94 fps
		60		// - 60.00 fps
	};

	union video_format_params {
		struct {
			unsigned int pic_width:14;
			unsigned int pic_height:14;
			unsigned int src_frame_rate:4;
		} bits;

		unsigned int val;
	} vid_fmts;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_VIDEOFMT, &vid_fmts, VID);

	hdr->w = vid_fmts.bits.pic_width;
	hdr->h = vid_fmts.bits.pic_height;

	hdr->frame_rate = vid_fmts.bits.src_frame_rate;

	if (hdr->frame_rate <= 8)
		hdr->frame_rate = raw_framerates[hdr->frame_rate];
	else
		hdr->frame_rate = 0;

	if((hdr->w == 0)||(hdr->h == 0))
		return CSAPI_FAILED;
	else
		return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetSRCSequenceHeader(CSVID_HANDLE handle, CSVID_SequenceHeader * hdr)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	int raw_framerates[] = {
		00,		// - unknown
		23,		// - 23.97 fps
		24,		// - 24.00 fps
		25,		// - 25.00 fps
		29,		// - 29.97 fps
		30,		// - 30.00 fps
		50,		// - 50.00 fps
		59,		// - 59.94 fps
		60		// - 60.00 fps
	};

	union video_format_params {
		struct {
			unsigned int pic_width:14;
			unsigned int pic_height:14;
			unsigned int src_frame_rate:4;
		} bits;

		unsigned int val;
	} vid_fmts;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_SRC_VIDEOFMT, &vid_fmts, VID);

	hdr->w = vid_fmts.bits.pic_width;
	hdr->h = vid_fmts.bits.pic_height;

	hdr->frame_rate = vid_fmts.bits.src_frame_rate;

	if (hdr->frame_rate <= 8)
		hdr->frame_rate = raw_framerates[hdr->frame_rate];
	else
		hdr->frame_rate = 0;

	if((hdr->w == 0)||(hdr->h == 0))
		return CSAPI_FAILED;
	else
		return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_EnablePTSSync(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PTS_ENABLE, 0, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_DisablePTSSync(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PTS_DISABLE, 0, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_EnableOutput(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_DISP_ON, 0, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_DisableOutput(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_DISP_OFF, 0, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetOutputPostion(CSVID_HANDLE handle, const CSVID_Rect * const src, const CSVID_Rect * const dst)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	memcpy(&disp_pos.src, src, sizeof(CSVID_Rect));
	memcpy(&disp_pos.dst, dst, sizeof(CSVID_Rect));

	/*right to width, bottom to height*/
	disp_pos.src.right -= disp_pos.src.left;
	disp_pos.src.bottom -= disp_pos.src.top;

	IOCTL(dev_obj, CSVID_IOC_DISP_POS, &disp_pos, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetOutputAlpha(CSVID_HANDLE handle, unsigned int alpha)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_DISP_ALPHA, alpha, VID);

	return CSAPI_SUCCEED;
}

#if 0
static unsigned char __parse_AFD(unsigned char * buf)
{
	unsigned char flag = 0;
	unsigned int *temp_buf = (unsigned int *)buf;
	unsigned int i = 0;
	unsigned int maxloop = 0;
	unsigned char afd_val = 0;
	
	if((*buf == 0) && (*(buf+1) == 0)) {
		flag = 0;// mpeg2
		maxloop = USER_DATA_BLOCK_SIZE -4;
	}
	else {
		flag = 1;// h.264
		maxloop = MEM_SWAP32(*temp_buf);
	}

	if(maxloop > USER_DATA_BLOCK_SIZE){
		printf("error : userdata size = %d, max size = %d\n",maxloop,USER_DATA_BLOCK_SIZE);
		return 0;
	}

	temp_buf += 8;
	buf +=32;

	for(i = 0; i < maxloop; i ++){// afd_identifier:0x44544731.
		if(((*(temp_buf+i)) == MEM_SWAP32(0x44544731)) && ((*(unsigned char*)(temp_buf + 4 + i*4))&0x40)){
			afd_val = *(unsigned char*)(temp_buf + 5 + i*4);
			return (afd_val&0xf);
		}
	}

	return afd_val;
}
#endif

int __check_AFD(unsigned char * afd, int * ud_total_num)
{
	unsigned int counter = 0;
	unsigned int rtog = 0, wtog= 0;
	unsigned int temp_userdata_wptr = 0, temp_userdata_rptr = 0;
	unsigned int userdata_lowlimit = ((unsigned int) g_userdatamem.pPMem) + USER_DATA_BLOCK_SIZE;
	unsigned int userdata_uplimit = userdata_lowlimit + g_userdatamem.MemSize - USER_DATA_BLOCK_SIZE;
	unsigned int offset = (unsigned int) g_userdatamem.pVMem - (unsigned int) g_userdatamem.pPMem;
	CSVID_OBJ *handle = &csvid_obj;

	UNUSED_VARIABLE(handle);
     
	temp_userdata_rptr = *((volatile unsigned int *) (g_userdatamem.pVMem));
	temp_userdata_rptr = MEM_SWAP32(temp_userdata_rptr);

	rtog = (DMXOUT_TOGGLE_BIT & temp_userdata_rptr);
	temp_userdata_rptr = (~DMXOUT_TOGGLE_BIT)&temp_userdata_rptr;

	temp_userdata_wptr = *(((volatile unsigned int *) g_userdatamem.pVMem) + 1);
	temp_userdata_wptr = MEM_SWAP32(temp_userdata_wptr);

	wtog = (DMXOUT_TOGGLE_BIT & temp_userdata_wptr);
	temp_userdata_wptr = (~DMXOUT_TOGGLE_BIT)&temp_userdata_wptr;

	if((temp_userdata_rptr > userdata_uplimit) ||(temp_userdata_rptr < userdata_lowlimit)) return 0;
	if((temp_userdata_wptr > userdata_uplimit)||(temp_userdata_wptr < userdata_lowlimit)) return 0;

	if((rtog == wtog)&&(temp_userdata_wptr > temp_userdata_rptr)){
		counter = (temp_userdata_wptr - temp_userdata_rptr)/USER_DATA_BLOCK_SIZE;
		counter = min(counter, USER_DATA_MAX_BLOCK_NUM);

	   	memcpy(userdata_buf, (unsigned int *)(temp_userdata_rptr + offset), USER_DATA_BLOCK_SIZE * counter);
	  	memset((unsigned int *)(temp_userdata_rptr + offset), 0, USER_DATA_BLOCK_SIZE * counter);
	  	temp_userdata_rptr += (USER_DATA_BLOCK_SIZE * counter);
	  	*((volatile unsigned int *) (g_userdatamem.pVMem)) = MEM_SWAP32(temp_userdata_rptr|rtog);
	  	afd = userdata_buf;
	  	*ud_total_num = counter;
  		return 1;
  
	}
	else if((rtog != wtog)&&(temp_userdata_wptr < temp_userdata_rptr)){
		int counter1 = 0, counter2 = 0;

		counter1 = (userdata_uplimit - temp_userdata_rptr)/USER_DATA_BLOCK_SIZE;
		counter2 = (temp_userdata_wptr - userdata_lowlimit)/USER_DATA_BLOCK_SIZE;
		
		counter1 = min(counter1, USER_DATA_MAX_BLOCK_NUM);
		memcpy(userdata_buf, (unsigned int *)(temp_userdata_rptr + offset), USER_DATA_BLOCK_SIZE * counter1);
		memset((unsigned int *)(temp_userdata_rptr + offset), 0, USER_DATA_BLOCK_SIZE * counter1);
	  
		temp_userdata_rptr += (USER_DATA_BLOCK_SIZE * counter1);
		if (temp_userdata_rptr == userdata_uplimit)
		{
			temp_userdata_rptr = userdata_lowlimit;
			rtog = wtog;
		}
		*ud_total_num = counter1;

		counter2 = min(USER_DATA_MAX_BLOCK_NUM - counter1, counter2);
		if (counter2 > 0)
		{
			memcpy(userdata_buf+(USER_DATA_BLOCK_SIZE * counter1)
			, (unsigned int *)(temp_userdata_rptr + offset), USER_DATA_BLOCK_SIZE * counter2);
			memset((unsigned int *)(temp_userdata_rptr + offset), 0, USER_DATA_BLOCK_SIZE * counter2);

			*ud_total_num += counter2;
			temp_userdata_rptr += (USER_DATA_BLOCK_SIZE * counter2);
		}

		afd = userdata_buf;
		*((volatile unsigned int *) (g_userdatamem.pVMem)) = MEM_SWAP32(temp_userdata_rptr|rtog);

		return 1;		
	}

	*afd = 0;
	*ud_total_num = 0;
	return 0;
}

CSAPI_RESULT CSVID_TimeCodeReportNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *),
					CSVID_TimeCode timecode, int event_enable)
{
	CSVID_TimeCode tempTC;
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		tempTC.val = timecode.val;
		tempTC.bits.reserved = 0x40;
		IOCTL(dev_obj, CSVID_IOC_SET_TIMECODE, &tempTC, VID);

		dev_obj->config_params.time_code = 1;
		dev_obj->tc_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->config_params.time_code = 0;
				tempTC.val = timecode.val;
                		tempTC.bits.reserved = 0x0;
                		IOCTL(dev_obj, CSVID_IOC_SET_TIMECODE, &tempTC, VID);
                		dev_obj->tc_fn = NULL;
				dev_obj->num_notify = 0;
                                return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		} 
	}
	else {
		tempTC.val = timecode.val;
		tempTC.bits.reserved = 0x0;
		IOCTL(dev_obj, CSVID_IOC_SET_TIMECODE, &tempTC, VID);
		dev_obj->tc_fn = NULL;
		dev_obj->num_notify --;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetTimeCode(CSVID_HANDLE handle, CSVID_TimeCode * timecode)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_TIMECODE, timecode, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetSourceRectangle(CSVID_HANDLE handle, int *width, int *height)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	union video_format_params {
		struct {
			unsigned int pic_width:14;
			unsigned int pic_height:14;
			unsigned int src_frame_rate:4;
		} bits;

		unsigned int val;
	} vid_fmts;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_VIDEOFMT, &vid_fmts, VID);

	*width = vid_fmts.bits.pic_width;
	*height = vid_fmts.bits.pic_height;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_ErrNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *, CSVID_ERROR_THRESHOLD),
			     CSVID_ERROR_THRESHOLD error_threshold, int event_enable, unsigned int timeout_value)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);
	UNUSED_VARIABLE(timeout_value)
	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_ERROR_LEVEL, error_threshold, VID);

		dev_obj->error_fn = call_back_function;
		dev_obj->config_params.error_level = 1;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->error_fn = NULL;
				dev_obj->config_params.error_level = 0;
				IOCTL(dev_obj, CSVID_IOC_SET_ERROR_LEVEL, 0, VID);
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		IOCTL(dev_obj, CSVID_IOC_SET_ERROR_LEVEL, 0, VID);
		dev_obj->error_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_DataUnderflowNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *),
	                               int event_enable, unsigned int timeout_value)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);
	UNUSED_VARIABLE(timeout_value)	    
	pthread_mutex_lock(&vid_mutex);
	    
	if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_UNDERFLOW_REPORT, event_enable, VID);

		dev_obj->underflow_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->underflow_fn = NULL;
				IOCTL(dev_obj, CSVID_IOC_SET_UNDERFLOW_REPORT, 0, VID);
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		IOCTL(dev_obj, CSVID_IOC_SET_UNDERFLOW_REPORT, 0, VID);
		dev_obj->underflow_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}


CSVID_ErrCode CSVID_GetErrCode(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSVID_OBJ_TYPE))
		return VID_ERROR_INVALID_PARAMETERS;

	return dev_obj->errno;
}

char *CSVID_GetErrString(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSVID_OBJ_TYPE))
		return vid_errstr[VID_ERROR_INVALID_PARAMETERS];

	return vid_errstr[dev_obj->errno];
}

CSAPI_RESULT CSVID_GetAspectRatio(CSVID_HANDLE handle, CSVID_ASPECTRATIO * aspect_ratio)
{
        return __csvid_get_aspectratio(handle, aspect_ratio);
}

CSAPI_RESULT CSVID_AspectRatioChangeNotify ( CSVID_HANDLE handle,
void (* call_back_function)(CSVID_HANDLE *, CSVID_ASPECTRATIO *),
int event_enable )
{
        CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_ASPECTRATIO_STATUS, 1, VID);

		dev_obj->aspectratio_fn = call_back_function;
		dev_obj->num_notify++;

		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->aspectratio_fn = NULL;
				IOCTL(dev_obj, CSVID_IOC_SET_ASPECTRATIO_STATUS, 0, VID);
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		IOCTL(dev_obj, CSVID_IOC_SET_ASPECTRATIO_STATUS, 0, VID);
		dev_obj->aspectratio_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_PScanCropNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, CSVID_Rect *),int event_enable )
{
        CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_PSCANCROP_STATUS, 1, VID);

		dev_obj->pscancrop_fn = call_back_function;
		dev_obj->num_notify++;                
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->pscancrop_fn = NULL;
				IOCTL(dev_obj, CSVID_IOC_SET_PSCANCROP_STATUS, 0, VID);
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		IOCTL(dev_obj, CSVID_IOC_SET_PSCANCROP_STATUS, 0, VID);
		dev_obj->pscancrop_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetPScanCrop(CSVID_HANDLE handle, CSVID_Rect *rect)
{
        CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_GET_PSCANCROP, rect, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SyncNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, signed char *),unsigned int timeout_value, int event_enable )
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
	struct timeval start;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) 
	{
		IOCTL(dev_obj, CSVID_IOC_SET_SYNC_STATUS, 1, VID);

		dev_obj->sync_fn = call_back_function;
		dev_obj->sync_timeout = timeout_value;
		gettimeofday(&start, NULL);
		memcpy(&(dev_obj->start), &start, sizeof(struct timeval));
		dev_obj->start.tv_sec += timeout_value/1000000;
		dev_obj->start.tv_usec += timeout_value%1000000;
		dev_obj->num_notify++;

		if (dev_obj->config_params.call_back_status == 0) 
		{
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);

			if (error != 0) 
			{
				printf("video call back function error!\n");
				dev_obj->sync_fn = NULL;
				dev_obj->sync_timeout = 0;
				IOCTL(dev_obj, CSVID_IOC_SET_SYNC_STATUS, 0, VID);
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}

			dev_obj->config_params.call_back_status = 1;
		}
	}
	else 
	{
		IOCTL(dev_obj, CSVID_IOC_SET_SYNC_STATUS, 0, VID);
		dev_obj->sync_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_Force3to2PollDown(CSVID_HANDLE handle, unsigned int enable_flag)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_SET_FORCE_3TO2_POLLDOW, enable_flag, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_WaitSync(CSVID_HANDLE handle, unsigned int enable_flag)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_SET_NOWAIT_SYNC, enable_flag, VID);

	return CSAPI_SUCCEED;
}

/*
function : CSVID_StartDelay
startdelay_ms : unit is ms, range 0~65535ms.
delay_flag : 0 - video follow the audio; 1 - video ecxeed the audio
*/
CSAPI_RESULT CSVID_StartDelay(CSVID_HANDLE handle, unsigned int startdelay_ms,unsigned int delay_flag)
{
        CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	startdelay_ms |= (delay_flag <<16);
	IOCTL(dev_obj, CSVID_IOC_SET_STARTDELAY, startdelay_ms, VID);

	return CSAPI_SUCCEED;
}

#if 1	//PFM
#define VID_REMAIN_BUF_ALIGN_SIZE 32

static int vid_remain_size = 0;
static char vid_remain_buf[VID_REMAIN_BUF_ALIGN_SIZE];
unsigned int first_in= 1;
unsigned int pre_buf_size = 0;
unsigned int pre_cpb_wptr = 0;

static unsigned int __dump_data(unsigned char* bufaddr, unsigned int len, unsigned int offset)
{
	FILE*	ts_file=NULL;
	static int count = 1;
	char filename[20];
	unsigned int written_len = 0;

	if(count > 5) return 0;

	sprintf(filename,"/mnt/dump_data%d.es",count);
	printf("create file %s\n",filename);
	printf("cpb addr 0x%x, size 0x%x\n",((unsigned int)bufaddr - offset),len);

	ts_file = fopen(filename,"w+b");
	if(ts_file==NULL)
	{
		printf("======>file open failed\n");
		return -1;
	}

	written_len = fwrite(bufaddr, 1, len, ts_file);
	if (written_len != len)
		printf("written = %d\n", written_len);

	fclose(ts_file);
	ts_file = NULL;

	count++;
	return written_len;
}

static unsigned int __csvid_get_cpb_wptr(void)
{
	return g_vid_cpb_wptr;
}

static unsigned int __csvid_get_cpb_rptr(void)
{
	unsigned int vid_dir_rptr = 0;
	unsigned int buf_addr = 0;
	unsigned int buf_size = 0;
	int cpb_lowlimit = (int) g_cpbmem.pPMem;
	int cpb_uplimit = cpb_lowlimit + g_cpbmem.MemSize;
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
	CSVID_OBJ *handle = &csvid_obj;

	if (first_in)
	    return cpb_lowlimit;
	
	IOCTL(handle, CSVID_IOC_PMF_GET_DIR_RP, &vid_dir_rptr, VID);
	vid_dir_rptr = vid_dir_rptr << DMXOUT_DIR_OFFSET;
	if ((vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) > (unsigned int) dir_uplimit
	    || (vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) < (unsigned int) dir_lowlimit) {
		vid_dir_rptr = dir_lowlimit >> DMXOUT_DIR_OFFSET;
		IOCTL(handle, CSVID_IOC_PMF_SET_DIR_RP, &vid_dir_rptr, VID);
		vid_dir_rptr = vid_dir_rptr << DMXOUT_DIR_OFFSET;
	}

	vid_dir_rptr += 0;	/* reserved */
	vid_dir_rptr += 4;	/* pts */

	vid_dir_rptr += 4;	/* buf_start */
	buf_addr = *((volatile unsigned int *) ((int) g_dirmem.pVMem + (vid_dir_rptr - (int) g_dirmem.pPMem)));
	buf_addr = MEM_SWAP32(buf_addr);

	vid_dir_rptr += 4;	/* buf_size */
	buf_size = *((volatile unsigned int *) ((int) g_dirmem.pVMem + (vid_dir_rptr - (int) g_dirmem.pPMem)));
	buf_size = MEM_SWAP32(buf_size);

	if (buf_addr < (unsigned int) cpb_lowlimit || buf_addr > (unsigned int) cpb_uplimit) {
		buf_addr = cpb_lowlimit;
		buf_size = 0;
	}

	return buf_addr;
}

static int __csvid_cpb_is_full(int Buf_size)
{
	unsigned int cpb_rptr = 0;
	unsigned int cpb_wptr = 0;
	int cpb_lowlimit = (int) g_cpbmem.pPMem;
	int cpb_uplimit = cpb_lowlimit + g_cpbmem.MemSize;

	if (__csvid_dir_is_empty(0) < 0)    // for empty case
	{
	    if (g_cpbmem.MemSize - pre_buf_size < (unsigned int)Buf_size)
		return -1;
	    else
		return 0;
	}
	
	cpb_rptr = __csvid_get_cpb_rptr();
	cpb_wptr = __csvid_get_cpb_wptr();

	if (((cpb_wptr > cpb_rptr) && ((cpb_wptr + Buf_size) > (unsigned int) cpb_uplimit)
	     && ((cpb_rptr - cpb_lowlimit) < (unsigned int) Buf_size)) || ((cpb_rptr > cpb_wptr)
									   && ((cpb_wptr + Buf_size) > cpb_rptr))) {
		return -1;
	}

	return 0;
}

static int __csvid_correct_dir_limit(void)
{
	unsigned int vid_dir_uplimit = 0;
	unsigned int vid_dir_lowlimit = 0;
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
	CSVID_OBJ *handle = &csvid_obj;

	/* get the upper directory, if error, correct it. */
	IOCTL(handle, CSVID_IOC_PMF_GET_CPBDIRU0, &vid_dir_uplimit, VID);
	vid_dir_uplimit = (vid_dir_uplimit >> DMXOUT_DIR_OFFSET) << DMXOUT_DIR_OFFSET;
	if (vid_dir_uplimit != (unsigned int) dir_uplimit) {
		vid_dir_uplimit = (dir_uplimit >> DMXOUT_DIR_OFFSET) << DMXOUT_DIR_OFFSET;
		IOCTL(handle, CSVID_IOC_PMF_SET_CPBDIRU0, &vid_dir_uplimit, VID);
	}

	/* get the low directory, if error, correct it. */
	IOCTL(handle, CSVID_IOC_PMF_GET_CPBDIRL0, &vid_dir_lowlimit, VID);
	vid_dir_lowlimit = (vid_dir_lowlimit >> DMXOUT_DIR_OFFSET) << DMXOUT_DIR_OFFSET;
	if (vid_dir_lowlimit != (unsigned int) dir_lowlimit) {
		vid_dir_lowlimit = (dir_lowlimit >> DMXOUT_DIR_OFFSET) << DMXOUT_DIR_OFFSET;
		IOCTL(handle, CSVID_IOC_PMF_SET_CPBDIRL0, &vid_dir_lowlimit, VID);
	}

	return CSAPI_SUCCEED;
}

static unsigned int __csvid_get_dir_rptr(unsigned int *vid_dir_rptr, unsigned int *vid_dir_rtog)
{
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
	CSVID_OBJ *handle = &csvid_obj;

	/* get the lower/upper directory, if error, correct it. */
	__csvid_correct_dir_limit();

	/* get the read pointer of directory, if error, correct it. */
	IOCTL(handle, CSVID_IOC_PMF_GET_DIR_RP, vid_dir_rptr, VID);

	*vid_dir_rtog = (DMXOUT_TOGGLE_BIT & (*vid_dir_rptr)) >> (DMXOUT_TOGGLE_OFFSET);
	*vid_dir_rptr = (*vid_dir_rptr) << DMXOUT_DIR_OFFSET;

	if (((*vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) > (unsigned int) dir_uplimit)
	    || ((*vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) < (unsigned int) dir_lowlimit)) {
		*vid_dir_rptr = dir_lowlimit >> DMXOUT_DIR_OFFSET;
		IOCTL(handle, CSVID_IOC_PMF_SET_DIR_RP, vid_dir_rptr, VID);
		*vid_dir_rptr = dir_lowlimit;
	}

	return 0;
}

static unsigned int __csvid_get_dir_wptr(unsigned int *vid_dir_wptr, unsigned int *vid_dir_wtog)
{
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
	CSVID_OBJ *handle = &csvid_obj;

	/* get the lower/upper directory, if error, correct it. */
	__csvid_correct_dir_limit();

	/* get the writer pointer of directory, if error, correct it. */
	IOCTL(handle, CSVID_IOC_PMF_GET_DIR_WP, vid_dir_wptr, VID);

	*vid_dir_wtog = (DMXOUT_TOGGLE_BIT & (*vid_dir_wptr)) >> (DMXOUT_TOGGLE_OFFSET);
	*vid_dir_wptr = (*vid_dir_wptr) << DMXOUT_DIR_OFFSET;

	if (((*vid_dir_wptr) & (~DMXOUT_TOGGLE_BIT)) > (unsigned int) dir_uplimit
	    || ((*vid_dir_wptr) & (~DMXOUT_TOGGLE_BIT)) < (unsigned int) dir_lowlimit) {
		*vid_dir_wptr = dir_lowlimit >> DMXOUT_DIR_OFFSET;
		IOCTL(handle, CSVID_IOC_PMF_SET_DIR_WP, vid_dir_wptr, VID);
		*vid_dir_wptr = dir_lowlimit;
	}

	return 0;
}

static int __csvid_dir_is_empty(int dir_num)
{
	unsigned int vid_dir_rtog = 0;
	unsigned int vid_dir_wtog = 0;
	unsigned int vid_dir_wptr = 0;
	unsigned int vid_dir_rptr = 0;
	int dir_is_full = 0;
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;

	__csvid_get_dir_rptr(&vid_dir_rptr, &vid_dir_rtog);
	__csvid_get_dir_wptr(&vid_dir_wptr, &vid_dir_wtog);

	if (vid_dir_rtog == vid_dir_wtog) {
		if ((VIDOUT_GET_DIR_NUM(vid_dir_rptr, vid_dir_wptr)) <= (unsigned int) dir_num){
			dir_is_full = -1;
                }
	}
	else {
		if (vid_dir_wptr > vid_dir_rptr) {
			if ((VIDOUT_GET_DIR_NUM((vid_dir_wptr - vid_dir_rptr), (dir_uplimit - dir_lowlimit))) <=
			    (unsigned int) dir_num){
				dir_is_full = -1;
                        }
		}
		else {
			if ((VIDOUT_GET_DIR_NUM((vid_dir_rptr - vid_dir_wptr), (dir_uplimit - dir_lowlimit))) <=
			    (unsigned int) dir_num){
				dir_is_full = -1;
                        }
		}
	}

	return dir_is_full;
}

static int __csvid_dir_is_full(int dir_num)
{
	unsigned int vid_dir_rtog = 0;
	unsigned int vid_dir_wtog = 0;
	unsigned int vid_dir_wptr = 0;
	unsigned int vid_dir_rptr = 0;
	int dir_is_full = 0;
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;

	__csvid_get_dir_rptr(&vid_dir_rptr, &vid_dir_rtog);
	__csvid_get_dir_wptr(&vid_dir_wptr, &vid_dir_wtog);

	if (vid_dir_rtog == vid_dir_wtog) {
		if ((VIDOUT_GET_DIR_NUM(dir_lowlimit, dir_uplimit) -
		     (VIDOUT_GET_DIR_NUM(vid_dir_rptr, vid_dir_wptr))) <= (unsigned int) dir_num){
			dir_is_full = -1;
                }
	}
	else {
		if (vid_dir_wptr > vid_dir_rptr) {
			if ((VIDOUT_GET_DIR_NUM(dir_lowlimit, dir_uplimit) -
			     (VIDOUT_GET_DIR_NUM((vid_dir_wptr - vid_dir_rptr), (dir_uplimit - dir_lowlimit)))) <=
			    (unsigned int) dir_num){
				dir_is_full = -1;
                        }
		}
		else {
			if ((VIDOUT_GET_DIR_NUM(dir_lowlimit, dir_uplimit) -
			     (VIDOUT_GET_DIR_NUM((vid_dir_rptr - vid_dir_wptr), (dir_uplimit - dir_lowlimit)))) <=
			    (unsigned int) dir_num){
				dir_is_full = -1;
                        }
		}
	}

	return dir_is_full;
}

/*
	Buf_size is the multiple of 4
*/
static int __csvid_write_block_to_cpb(unsigned char *pBuf, int Buf_size)
{
	unsigned int cpb_wptr = 0;
	int cpb_lowlimit = (int) g_cpbmem.pPMem;
	int cpb_uplimit = cpb_lowlimit + g_cpbmem.MemSize;
	unsigned int offset = 0;

	/* get cpb w ptr */
	cpb_wptr = __csvid_get_cpb_wptr();

	/* correct wptr */
	if ((cpb_wptr + Buf_size) > (unsigned int) cpb_uplimit)
		cpb_wptr = cpb_lowlimit;

	g_vid_cpb_wptr = cpb_wptr + Buf_size;
	if (g_vid_cpb_wptr > (unsigned int) cpb_uplimit){
		g_vid_cpb_wptr = cpb_lowlimit;
        }
	offset = (int) g_cpbmem.pVMem - (int) g_cpbmem.pPMem;
	cpb_wptr += offset;

	if(vid_remain_size > 0)
		memcpy((unsigned char *) cpb_wptr, (unsigned char *) vid_remain_buf, vid_remain_size);
	
	memcpy((unsigned char *) (cpb_wptr + vid_remain_size), (unsigned char *) pBuf, (Buf_size - vid_remain_size));

//	__dump_data((unsigned char *)cpb_wptr, Buf_size, offset);
	return 0;
}

/*
	directory struct
	{
		reserved : 32;
		pts :          32;
		wpointer :  32;
		buf_len:    32;
	}
*/
static int __csvid_write_dir(unsigned int Buf_Addr, int Buf_size,  unsigned long long pts)
{
	int err_code = 0;
	unsigned int cur_pts = 0;
	unsigned int vid_dir_wtog = 0;
	unsigned int vid_dir_wptr = 0;
	unsigned int vid_cpb_wptr = 0;
	int dir_lowlimit = (int) g_dirmem.pPMem;
	int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
	CSVID_OBJ *handle = &csvid_obj;
	unsigned int offset = (int) g_dirmem.pVMem - (int) g_dirmem.pPMem;

	if (Buf_Addr == 0 || Buf_size <= 0) {
		return -1;
	}

	__csvid_get_dir_wptr(&vid_dir_wptr, &vid_dir_wtog);

	vid_cpb_wptr = __csvid_get_cpb_wptr();

	if ((vid_dir_wptr + VIDOUT_ONE_DIR_SIZE) > (unsigned int) dir_uplimit) {
		vid_dir_wptr = dir_lowlimit;
		vid_dir_wtog = (~vid_dir_wtog) & 0x1;
	}

	cur_pts = pts >> 1;
	/* update the directory */
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = 0;

	vid_dir_wptr += 4;
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = MEM_SWAP32(cur_pts);

if(first_in == 1){
	first_in = 0;

	pre_cpb_wptr = vid_cpb_wptr;
	pre_buf_size = Buf_size;

	vid_dir_wptr += 4;
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = MEM_SWAP32(pre_cpb_wptr - pre_buf_size);

	vid_dir_wptr += 4;
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = 0;
}
else{
	vid_dir_wptr += 4;
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = MEM_SWAP32(pre_cpb_wptr - pre_buf_size);

	vid_dir_wptr += 4;
	*((volatile unsigned int *) (vid_dir_wptr + offset)) = MEM_SWAP32((unsigned int) pre_buf_size);

	pre_cpb_wptr = vid_cpb_wptr;
	pre_buf_size = Buf_size;
}
	vid_dir_wptr += 4;
	vid_dir_wptr = (vid_dir_wptr >> DMXOUT_DIR_OFFSET) | (vid_dir_wtog << DMXOUT_TOGGLE_OFFSET);

	IOCTL(handle, CSVID_IOC_PMF_SET_DIR_WP, &vid_dir_wptr, VID);

	return err_code;
}

CSAPI_RESULT CSVID_PFMOpen(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
	unsigned int phyaddr = 0;
	unsigned int memsize = 0;
	unsigned int addroffset = 0;
	unsigned char *buf = NULL;
	unsigned int addr = 0;
	unsigned int size = 0;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PMF_RESET, NULL, VID);

	/*map cpb memory */
	IOCTL(dev_obj, CSVID_IOC_PMF_GETCPB_ADDR, &addr, VID);
	IOCTL(dev_obj, CSVID_IOC_PMF_GETCPB_SIZE, &size, VID);

	memset(&g_cpbmem, 0, sizeof(CSVID_MMAP_t));
	memset(vid_remain_buf, 0, VID_REMAIN_BUF_ALIGN_SIZE);
	vid_remain_size = 0;
	
	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + size + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;

	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, dev_obj->dev_fd, phyaddr);
	if (buf == MAP_FAILED) {
		dev_obj->errno = VID_ERROR_MEMORY_MAP;
		return CSAPI_FAILED;
	}

	g_cpbmem.MappedSize = memsize;
	g_cpbmem.MappedAddr = buf;
	g_cpbmem.pPMem = (void *) addr;
	g_cpbmem.pVMem = (void *) (buf + addroffset);
	g_cpbmem.MemSize = size;
	g_cpbmem.FD = dev_obj->dev_fd;

	/*map dir memory */
	IOCTL(dev_obj, CSVID_IOC_PMF_GETDIR_ADDR, &addr, VID);
	IOCTL(dev_obj, CSVID_IOC_PMF_GETDIR_SIZE, &size, VID);

	memset(&g_dirmem, 0, sizeof(CSVID_MMAP_t));

	phyaddr = (addr >> PAGE_SHIFT) << PAGE_SHIFT;
	addroffset = addr - phyaddr;
	memsize = ((addroffset + size + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;

	buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, dev_obj->dev_fd, phyaddr);
	if (buf == MAP_FAILED) {
		dev_obj->errno = VID_ERROR_MEMORY_MAP;
		return CSAPI_FAILED;
	}

	g_dirmem.MappedSize = memsize;
	g_dirmem.MappedAddr = buf;
	g_dirmem.pPMem = (void *) addr;
	g_dirmem.pVMem = (void *) (buf + addroffset);
	g_dirmem.MemSize = size;
	g_dirmem.FD = dev_obj->dev_fd;

	g_vid_cpb_wptr = (unsigned int) g_cpbmem.pPMem;
	first_in = 1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_PFMClose(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PMF_RESET, NULL, VID);

	munmap(g_cpbmem.MappedAddr, g_cpbmem.MappedSize);
	munmap(g_dirmem.MappedAddr, g_dirmem.MappedSize);

	memset(&g_cpbmem, 0, sizeof(CSVID_MMAP_t));
	memset(&g_dirmem, 0, sizeof(CSVID_MMAP_t));
	memset(vid_remain_buf, 0, VID_REMAIN_BUF_ALIGN_SIZE);

	vid_remain_size = 0;
	g_vid_cpb_wptr = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_PMFReset(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PMF_RESET, NULL, VID);

	g_vid_cpb_wptr = (unsigned int) g_cpbmem.pPMem;

	memset(vid_remain_buf, 0, VID_REMAIN_BUF_ALIGN_SIZE);	
	vid_remain_size = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetPFMBufferSize(CSVID_HANDLE handle, unsigned int *bufsize)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
        unsigned int vid_dir_rtog = 0;
	unsigned int vid_dir_wtog = 0;
	unsigned int vid_dir_wptr = 0;
	unsigned int vid_dir_rptr = 0;
        unsigned int offset = (int) g_dirmem.pVMem - (int) g_dirmem.pPMem;
        unsigned int dir_lowlimit = (int) g_dirmem.pPMem;
	unsigned int dir_uplimit = dir_lowlimit + g_dirmem.MemSize;
        int current_dir_num = 0;
        int total_dir_num = VIDOUT_GET_DIR_NUM(dir_lowlimit, dir_uplimit);
    
	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

        *bufsize = 0;
	/* get cpb r/w ptr */
        __csvid_get_dir_rptr(&vid_dir_rptr, &vid_dir_rtog);
	__csvid_get_dir_wptr(&vid_dir_wptr, &vid_dir_wtog);
#if 1
	//printf("dir_wp = 0x%08x  dir_rp = 0x%08x  wtog= 0x%08x  rtog = 0x%08x\n", vid_dir_wptr, vid_dir_rptr, vid_dir_wtog, vid_dir_rtog);
	if (vid_dir_rtog == vid_dir_wtog) {
	    current_dir_num = VIDOUT_GET_DIR_NUM(vid_dir_rptr, vid_dir_wptr);
	}
	else{
	    current_dir_num = VIDOUT_GET_DIR_NUM((vid_dir_rptr - vid_dir_wptr), (dir_uplimit - dir_lowlimit));
	}

	if ((total_dir_num -current_dir_num) <= 1){
	    *bufsize = 0;
	}
	else if (current_dir_num == 0)
	{
	    *bufsize = g_cpbmem.MemSize - pre_buf_size;
	}
	else
	{
	    unsigned int cpb_wp, cpb_rp, cpb_up, cpb_low;
	    
	    cpb_wp = __csvid_get_cpb_wptr();
		
	    cpb_rp = *((volatile unsigned int *)(vid_dir_rptr + offset + 8));
	    cpb_rp = MEM_SWAP32(cpb_rp);

	    //printf("  cpb_rp = 0x%08x cpb_low = 0x%08x  cpb_up = 0x%08x\n", cpb_rp, (unsigned int)g_cpbmem.pPMem, (unsigned int)g_cpbmem.pPMem + g_cpbmem.MemSize);
	    
	    if (cpb_rp < cpb_wp)    // one direction
	    {
		cpb_low = (unsigned int)g_cpbmem.pPMem;
		cpb_up = cpb_low + g_cpbmem.MemSize;
		*bufsize = cpb_up - cpb_wp;
		if ((*bufsize < (cpb_rp - cpb_low)) /* && (*bufsize < 4096)*/)   // in order to save memory space, we leave max gap of 4096byte when toggle happens
		    *bufsize = cpb_rp - cpb_low;
	    }
	    else if (cpb_rp >= cpb_wp)   // toggle happens. cpb_rp ==  cpb_wp only when cpb is full
	    {
		*bufsize = cpb_rp - cpb_wp;
	    }

	    //printf("bufsize = 0x%08x\n", *bufsize);

	    //printf("0x0b1f7108 = 0x%08x 0x0b1f710c = 0x%08x\n", *((volatile int *)(0x0b1f7108 + offset)), *((volatile int *)(0x0b1f710c + offset)));
	}

	*bufsize &=  ~(VID_REMAIN_BUF_ALIGN_SIZE-1);	// buffer siz must be aligned by 32.
#else	    
	if (vid_dir_rtog == vid_dir_wtog) {
                current_dir_num = VIDOUT_GET_DIR_NUM(vid_dir_rptr, vid_dir_wptr);
                if ((total_dir_num -current_dir_num) <= 1){
                       *bufsize = 0;
                }
                else{
                        while(current_dir_num){
                                used_size = *((volatile unsigned int *) (vid_dir_rptr + offset+12));
                                *bufsize += MEM_SWAP32((unsigned int) used_size);
                                vid_dir_rptr += VIDOUT_ONE_DIR_SIZE;
                                current_dir_num--;
                        }
                        *bufsize = g_cpbmem.MemSize - (*bufsize);
                }
	}
	else {
		if (vid_dir_wptr > vid_dir_rptr) {
			printf("warning!!!!!!! buffer wp exceed rp ! something wrong !\n");
#if 0			
                        current_dir_num = VIDOUT_GET_DIR_NUM((vid_dir_wptr - vid_dir_rptr), (dir_uplimit - dir_lowlimit));
			if ((total_dir_num -current_dir_num) <= 1){
				*bufsize = 0;
                        }
                        else{
                                while(current_dir_num){
                                        used_size = *((volatile unsigned int *) (vid_dir_rptr + offset + 12));
                                        *bufsize += MEM_SWAP32((unsigned int) used_size);
                                        vid_dir_rptr += VIDOUT_ONE_DIR_SIZE;
                                        current_dir_num--;
                                }
                                *bufsize = g_cpbmem.MemSize - (*bufsize);
                        }
#endif			
		}
		else {
                        current_dir_num = VIDOUT_GET_DIR_NUM((vid_dir_rptr - vid_dir_wptr), (dir_uplimit - dir_lowlimit));
			if ((total_dir_num -current_dir_num) <= 1){
                                *bufsize = 0;
                        }
                        else{
                                while(current_dir_num){
                                        if((vid_dir_rptr + VIDOUT_ONE_DIR_SIZE) > dir_uplimit){
                                                vid_dir_rptr = dir_lowlimit + 12;
                                        }
                                        used_size= *((volatile unsigned int *) (vid_dir_rptr + offset + 12));
                                        *bufsize += MEM_SWAP32((unsigned int) used_size);
					vid_dir_rptr += VIDOUT_ONE_DIR_SIZE;
                                        current_dir_num--;
                                }
                                *bufsize = g_cpbmem.MemSize - (*bufsize);
                        }
		}
	}
#endif
	
	return CSAPI_SUCCEED;
}

#define DEFAULT_VIDEO_BLOCK 32
CSAPI_RESULT CSVID_WritePFMData(CSVID_HANDLE handle, unsigned char *src, int size)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
	int err_code = 0;
	int block_size = 0;
	int dir_count = 0;	/* the counter of directory */
	int dir_num = 0;	/* the sum of directory */
	unsigned char *p_buf = 0;	/* the remains of buffer size */
	unsigned int pts = 0;
        int tempsize = 0;
        int fix_bug_var = 0;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	if (src == NULL || size < 0) {
		return CSAPI_FAILED;
	}

	/* modify buffer and size */
	fix_bug_var = size;
	size += vid_remain_size;
	tempsize = size % VID_REMAIN_BUF_ALIGN_SIZE;		
	size -= tempsize;

	if(size > 0)
	{
        	dir_num = (size / size);
        	err_code = __csvid_dir_is_full(dir_num);
        	if (err_code < 0) {
			//printf("csvid dir is full\n");
			return CSAPI_FAILED;
        	}
        	err_code = __csvid_cpb_is_full(size);
        	if (err_code < 0) {
			//printf("csvid cpb is full\n");
			return CSAPI_FAILED;
        	}

        	p_buf = src;
        	block_size = size;
        	while (dir_count < dir_num) {
			__csvid_write_block_to_cpb(p_buf, block_size);
			__csvid_write_dir((unsigned int) p_buf, block_size, pts);
        		p_buf = p_buf + block_size;
        		dir_count++;
        	}
	}
	if(tempsize > 0){
		if(size > 0)
			memcpy((unsigned char *)&vid_remain_buf, src + (size - vid_remain_size),  tempsize);
		else
			memcpy((unsigned char *)&vid_remain_buf + vid_remain_size, src, fix_bug_var);
	}
	
	vid_remain_size = tempsize;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetNotifyPFMDataEmpty(CSVID_HANDLE handle, unsigned int *empty_threshold,
					 void (*call_back_function) (CSVID_HANDLE *), unsigned char event_enable)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		g_cpbmem.emptynotify_size = *empty_threshold;
		g_cpbmem.empty_enable = event_enable;
		g_cpbmem.empty_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				g_cpbmem.emptynotify_size = -1;
				g_cpbmem.empty_enable = 0;
				g_cpbmem.empty_fn = NULL;
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		g_cpbmem.emptynotify_size = -1;
		g_cpbmem.empty_enable = 0;
		g_cpbmem.empty_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetNotifyPFMDataFull(CSVID_HANDLE handle, unsigned int *full_threshold,
					 void (*call_back_function) (CSVID_HANDLE *), unsigned char event_enable)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		g_cpbmem.fullnotify_size = *full_threshold;
		g_cpbmem.full_enable = event_enable;
		g_cpbmem.full_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;

			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				g_cpbmem.fullnotify_size = -1;
				g_cpbmem.full_enable = 0;
				g_cpbmem.full_fn = NULL;
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}
	}
	else {
		g_cpbmem.fullnotify_size = -1;
		g_cpbmem.full_enable = 0;
		g_cpbmem.full_fn = NULL;
		dev_obj->num_notify--;
	}
	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_WritePFMDataWithPTS(CSVID_HANDLE handle, unsigned char *src, int size, unsigned long long pts)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
	int err_code = 0;
	int block_size = 0;
	int dir_count = 0;	/* the counter of directory */
	int dir_num = 0;	/* the sum of directory */
	unsigned char *p_buf = 0;	/* the remains of buffer size */
       int tempsize = 0;
       int fix_bug_var = 0;
	
	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	if (src == NULL || size < 0) {
		return CSAPI_FAILED;
	}

	/* modify buffer and size */
	fix_bug_var = size;
	size += vid_remain_size;
	tempsize = size % VID_REMAIN_BUF_ALIGN_SIZE;		
	size -= tempsize;
	
	if(size > 0)
	{
        	dir_num = (size / size);

        	err_code = __csvid_dir_is_full(dir_num);
        	if (err_code < 0) {
        		return CSAPI_FAILED;
        	}

        	err_code = __csvid_cpb_is_full(size);
        	if (err_code < 0) {
        		return CSAPI_FAILED;
        	}

        	p_buf = src;
        	block_size = size;
        	while (dir_count < dir_num) {
        		__csvid_write_block_to_cpb(p_buf, block_size);
        		__csvid_write_dir((unsigned int) p_buf, block_size, pts);

        		p_buf = p_buf + block_size;
        		dir_count++;
        	}
	}

	if(tempsize > 0){
		if(size > 0)
			memcpy((unsigned char *)&vid_remain_buf, src + (size - vid_remain_size),  tempsize);
		else
			memcpy((unsigned char *)&vid_remain_buf + vid_remain_size, src, fix_bug_var);
	}

	vid_remain_size = tempsize;
	
	return CSAPI_SUCCEED;
}
#endif

#ifdef VIB_SUPPORT
CSAPI_RESULT CSVID_VIB_Config(CSVID_HANDLE handle,union vib_para para)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);
	IOCTL(dev_obj, CSVID_IOC_VIB_CONFIG, &para, VID);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_VIB_Reset(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_VIB_RESET, NULL, VID);

	return CSAPI_SUCCEED;
}
#endif

CSAPI_RESULT CSVID_UserDataNotify ( CSVID_HANDLE handle,void (* call_back_function)(CSVID_HANDLE *, unsigned char *, int, int), int event_enable )
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	pthread_mutex_lock(&vid_mutex);

	if (event_enable) {
		if (dev_obj->config_params.call_back_status == 0) {
			int error = 0;
			
			dev_obj->num_notify++;
			error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
			if (error != 0) {
				printf("video call back function error!\n");
				dev_obj->num_notify = 0;
				return CSAPI_FAILED;
			}
			dev_obj->config_params.call_back_status = 1;
		}

		dev_obj->config_params.afd_info = 1;
              dev_obj->afd_fn = call_back_function;
		IOCTL(dev_obj, CSVID_IOC_SET_USER_DATA_STATUS, 1, VID);
	}
	else {
		IOCTL(dev_obj, CSVID_IOC_SET_USER_DATA_STATUS, 0, VID);
		dev_obj->config_params.afd_info = 0;
		dev_obj->afd_fn = NULL;
		dev_obj->num_notify--;
	}

	pthread_mutex_unlock(&vid_mutex);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_FormatChangeNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *, CSVID_SequenceHeader *),
	                               int event_enable)
{
            CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	    CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);
	    
	    pthread_mutex_lock(&vid_mutex);
	    
	    if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_FORMATCHANGE_REPORT, event_enable, VID);

		dev_obj->changeformat_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
		    int error = 0;
		    
		    error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
		    if (error != 0) {
			printf("video call back function error!\n");
			dev_obj->changeformat_fn = NULL;
			IOCTL(dev_obj, CSVID_IOC_SET_FORMATCHANGE_REPORT, 0, VID);
			dev_obj->num_notify = 0;
			return CSAPI_FAILED;
		    }
		    dev_obj->config_params.call_back_status = 1;
		}
	    }
	    else {
		IOCTL(dev_obj, CSVID_IOC_SET_FORMATCHANGE_REPORT, 0, VID);
		dev_obj->changeformat_fn = NULL;
		dev_obj->num_notify--;
	    }

	    pthread_mutex_unlock(&vid_mutex);
	    
	    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SRCFormatChangeNotify(CSVID_HANDLE handle, void (*call_back_function) (CSVID_HANDLE *,CSVID_SequenceHeader *),
	                               int event_enable)
{
            CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	    CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	    pthread_mutex_lock(&vid_mutex);

	    if (event_enable) {
		IOCTL(dev_obj, CSVID_IOC_SET_SRCFORMATCHANGE_REPORT, event_enable, VID);

		dev_obj->changesrcformat_fn = call_back_function;
		dev_obj->num_notify++;
		if (dev_obj->config_params.call_back_status == 0) {
		    int error = 0;
		    
		    error = pthread_create(&vid_callback, NULL, (void *) CallBackNotify, dev_obj);
		    if (error != 0) {
			printf("video call back function error!\n");
			dev_obj->changesrcformat_fn = NULL;
			IOCTL(dev_obj, CSVID_IOC_SET_SRCFORMATCHANGE_REPORT, 0, VID);
			dev_obj->num_notify = 0;
			return CSAPI_FAILED;
		    }
		    dev_obj->config_params.call_back_status = 1;
		}
	    }
	    else {
		IOCTL(dev_obj, CSVID_IOC_SET_SRCFORMATCHANGE_REPORT, 0, VID);
		dev_obj->changesrcformat_fn = NULL;
		dev_obj->num_notify--;
	    }

	    pthread_mutex_unlock(&vid_mutex);
	    
	    return CSAPI_SUCCEED;
}


CSAPI_RESULT CSVID_SetErrorSkipMode(CSVID_HANDLE handle, unsigned int errorskipmode)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_SET_ERROR_SKIP_MODE, errorskipmode, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_EnableTrickMode(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_TRICK_MDOE, 1, VID);
	dev_obj->config_params.trick_mode = 1;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_DisableTrickMode(CSVID_HANDLE handle)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_TRICK_MDOE, 0, VID);
	dev_obj->config_params.trick_mode = 0;

	return CSAPI_SUCCEED;
}

static int CallBackNotify(int *param)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) param;
	int rc, fd,ms_wait = 100;
	fd_set fds;
	struct timeval end,timeout;
	signed char in_sync = -1;      
	unsigned char afd= 0 ;

	fd = dev_obj->dev_fd;
	timeout.tv_sec  = ms_wait / 1000;
	timeout.tv_usec = (ms_wait % 1000) * 1000;

	while (1) 
	{
		if(dev_obj->sync_fn)
		{
			gettimeofday(&end, NULL);
			if(timercmp(&dev_obj->start,&end,<))
			{
				in_sync = -1;
				memset(&(dev_obj->start),0,sizeof(struct timeval));
				dev_obj->sync_fn((CSVID_HANDLE) dev_obj, &in_sync);
				if((dev_obj->start.tv_usec == 0)&&(dev_obj->start.tv_sec == 0))
				{ 
					dev_obj->sync_fn = NULL;
				}
			}
		}

		if ((dev_obj->config_params.time_code) || (dev_obj->config_params.error_level) ||(dev_obj->aspectratio_fn) ||
			(dev_obj->pscancrop_fn) ||(dev_obj->sync_fn) || (dev_obj->underflow_fn) ||(dev_obj->changeformat_fn)) 
		{
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			rc = select(fd + 1, &fds, NULL, NULL, &timeout);
			if (rc < 0)
			{
				usleep(100);
				continue;
			}
			if (FD_ISSET(fd, &fds) ? CSAPI_SUCCEED : CSAPI_FAILED) 
			{
				usleep(100);
				continue;
			}
			else 
			{
				IOCTL(dev_obj, CSVID_IOC_GET_NOTIFY_TYPE, &rc, VID);
				if ((rc & 0x1) && (dev_obj->tc_fn)) 
				{
					dev_obj->tc_fn((CSVID_HANDLE) dev_obj);
					dev_obj->config_params.time_code = 0;
					dev_obj->tc_fn = NULL;
				}
				
				if ((rc & 0x2) && (dev_obj->error_fn)) 
				{
					dev_obj->error_fn((CSVID_HANDLE) dev_obj, dev_obj->config_params.error_level);
				}
				
				if ((rc & 0x4) && (dev_obj->aspectratio_fn)) 
				{
					CSVID_ASPECTRATIO aspect_ratio = CSVID_UNKNOWN;
					__csvid_get_aspectratio((CSVID_HANDLE) dev_obj, &aspect_ratio);
					dev_obj->aspectratio_fn((CSVID_HANDLE) dev_obj, &aspect_ratio);
				}
				
				if ((rc & 0x8) && (dev_obj->pscancrop_fn)) 
				{
					CSVID_Rect rect;
					CSVID_GetPScanCrop((CSVID_HANDLE) dev_obj, &rect);
					dev_obj->pscancrop_fn((CSVID_HANDLE) dev_obj, &rect);
				}
				
#ifdef ARCH_CSM1201
				if((rc & 0x20) && (dev_obj->sync_fn)) 
				{
					in_sync = 2;
					dev_obj->sync_fn((CSVID_HANDLE) dev_obj, &in_sync);
				}
#endif

				if((rc & 0x10) && (dev_obj->sync_fn)) 
				{
					in_sync = 1;
					dev_obj->sync_fn((CSVID_HANDLE) dev_obj, &in_sync);
					dev_obj->sync_fn = NULL;
					memset(&(dev_obj->start),0,sizeof(struct timeval));
				}

				if ((rc & 0x40) && (dev_obj->underflow_fn)) 
				{
					dev_obj->underflow_fn((CSVID_HANDLE)dev_obj);
				}

				if ((rc & 0x80) && (dev_obj->changeformat_fn)) 
				{
					CSVID_SequenceHeader hdr;
					CSVID_GetSequenceHeader(dev_obj, &hdr);
					dev_obj->changeformat_fn((CSVID_HANDLE)dev_obj,&hdr);
				}

				if ((rc & 0x100) && (dev_obj->changesrcformat_fn)) 
				{
					CSVID_SequenceHeader hdr;
					CSVID_GetSRCSequenceHeader(dev_obj, &hdr);
					dev_obj->changesrcformat_fn((CSVID_HANDLE)dev_obj,&hdr);
				}
			}
		}

		if(dev_obj->config_params.afd_info)
		{
			int num = 0;
			if(__check_AFD(&afd,&num))
			{
				dev_obj->afd_fn((CSVID_HANDLE) dev_obj, userdata_buf, num, USER_DATA_BLOCK_SIZE);
				memset(userdata_buf, 0, USER_DATA_BLOCK_SIZE * num);
			}
		}

		pthread_mutex_lock(&vid_mutex);

		if (g_cpbmem.full_enable == 1) 
		{
			int size = 0;
			size = __csvid_dir_is_full(1);
			if (size < 0) 
			{
				g_cpbmem.full_fn((CSVID_HANDLE) dev_obj);
			}
			else{
				size = 0;
				CSVID_GetPFMBufferSize((CSVID_HANDLE) dev_obj, &size);
				if(g_cpbmem.fullnotify_size >= size)
				{
					g_cpbmem.full_fn((CSVID_HANDLE) dev_obj);
				}
			}
		}

		if (g_cpbmem.empty_enable == 1) 
		{
			int size = 0;
			size = __csvid_dir_is_empty(2);
			if (size < 0) 
			{
				g_cpbmem.empty_fn((CSVID_HANDLE) dev_obj);
			}
			else
			{
				size = 0;
				CSVID_GetPFMBufferSize((CSVID_HANDLE) dev_obj, &size);
				if(g_cpbmem.emptynotify_size >= ((int)g_cpbmem.MemSize - size))
				{
					g_cpbmem.empty_fn((CSVID_HANDLE) dev_obj);
				}
			}
		}

		pthread_mutex_unlock(&vid_mutex);

		if(dev_obj->num_notify)
		{
			usleep(10);
		}
		else
		{
			sleep(1);
		}

		usleep(100);
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_GetBufferStatus(CSVID_HANDLE handle, CSVID_BUFFER_STATUS *buffer_status)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;
	int current_dir_num = 0;
	unsigned int addr = 0;
	unsigned int size = 0;
	unsigned int dir_lowlimit = 0;
	unsigned int dir_uplimit = 0;
	unsigned int vid_dir_rtog = 0;
	unsigned int vid_dir_wtog = 0;
	unsigned int vid_dir_wptr = 0;
	unsigned int vid_dir_rptr = 0;
	int total_dir_num = 0;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_PMF_GETDIR_ADDR, &addr, VID);
	IOCTL(dev_obj, CSVID_IOC_PMF_GETDIR_SIZE, &size, VID);

	dir_lowlimit = addr;
	dir_uplimit = addr + size;
	total_dir_num = VIDOUT_GET_DIR_NUM(dir_lowlimit, dir_uplimit);
	
	IOCTL(dev_obj, CSVID_IOC_PMF_GET_DIR_RP, &vid_dir_rptr, VID);
	vid_dir_rtog = (DMXOUT_TOGGLE_BIT & (vid_dir_rptr)) >> (DMXOUT_TOGGLE_OFFSET);
	vid_dir_rptr = (vid_dir_rptr) << DMXOUT_DIR_OFFSET;
	if (((vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) > (unsigned int) dir_uplimit)
	    || ((vid_dir_rptr & (~DMXOUT_TOGGLE_BIT)) < (unsigned int) dir_lowlimit)) {
		vid_dir_rptr = dir_lowlimit >> DMXOUT_DIR_OFFSET;
		IOCTL(dev_obj, CSVID_IOC_PMF_SET_DIR_RP, &vid_dir_rptr, VID);
	}

	IOCTL(dev_obj, CSVID_IOC_PMF_GET_DIR_WP, &vid_dir_wptr, VID);
	vid_dir_wtog = (DMXOUT_TOGGLE_BIT & (vid_dir_wptr)) >> (DMXOUT_TOGGLE_OFFSET);
	vid_dir_wptr = (vid_dir_wptr) << DMXOUT_DIR_OFFSET;
	if (((vid_dir_wptr) & (~DMXOUT_TOGGLE_BIT)) > (unsigned int) dir_uplimit
	    || ((vid_dir_wptr) & (~DMXOUT_TOGGLE_BIT)) < (unsigned int) dir_lowlimit) {
		vid_dir_wptr = dir_lowlimit >> DMXOUT_DIR_OFFSET;
		IOCTL(dev_obj, CSVID_IOC_PMF_SET_DIR_WP, &vid_dir_wptr, VID);
	}

	if (vid_dir_rtog == vid_dir_wtog) {
		current_dir_num = VIDOUT_GET_DIR_NUM(vid_dir_rptr, vid_dir_wptr);
		if (current_dir_num <= 3){
			*buffer_status = VID_BUFFER_EMPTY;
		}
		else if(current_dir_num >= total_dir_num){
			*buffer_status = VID_BUFFER_FULL;
		}
		else{
			*buffer_status = VID_BUFFER_NORMAL;
		}
	}
	else{
		current_dir_num = VIDOUT_GET_DIR_NUM(vid_dir_wptr, vid_dir_rptr);
		if ((total_dir_num - current_dir_num) <= 3)
			*buffer_status = VID_BUFFER_EMPTY;
		else if(current_dir_num <= 3){
			*buffer_status = VID_BUFFER_FULL;
		}
		else{
			*buffer_status = VID_BUFFER_NORMAL;
		}
	}

	return CSAPI_SUCCEED;
}

/*beiyang*/
CSAPI_RESULT CSVID_SetOutputPostion_Layer1(CSVID_HANDLE handle, const CSVID_Rect * const src, const CSVID_Rect * const dst)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	memcpy(&disp_pos.src, src, sizeof(CSVID_Rect));
	memcpy(&disp_pos.dst, dst, sizeof(CSVID_Rect));

	IOCTL(dev_obj, CSVID_IOC_DISP_POS_BEIYANG, &disp_pos, VID);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSVID_SetOutputAlpha_Layer1(CSVID_HANDLE handle, unsigned int alpha)
{
	CSVID_OBJ *dev_obj = (CSVID_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSVID_OBJ_TYPE);

	IOCTL(dev_obj, CSVID_IOC_DISP_ALPHA_BEIYANG, alpha, VID);

	return CSAPI_SUCCEED;
}

