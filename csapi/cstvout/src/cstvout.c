#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "cstvout.h"

#define  CSTVOUT_OBJ_TYPE	'e'

#ifdef ARCH_CSM1201
#define  CSTVOUT_DEV0_FILE       "/dev/orion_df/tvout0"
#define  CSTVOUT_DEV1_FILE       "/dev/orion_df/tvout1"
#else
#define  CSTVOUT_DEV_FILE	     "/dev/misc/orion_tve"
#endif

#define  CSTVE_IOC_MAGIC        'e'

#define  CSTVE_IOC_SET_MODE         	_IOW(CSTVE_IOC_MAGIC, 0x01, int)
#define  CSTVE_IOC_GET_MODE         	_IOW(CSTVE_IOC_MAGIC, 0x02, int)
#define  CSTVE_IOC_SET_WHILE_LEVEL	    _IOW(CSTVE_IOC_MAGIC, 0x03, int)
#define  CSTVE_IOC_GET_WHILE_LEVEL	    _IOR(CSTVE_IOC_MAGIC, 0x04, int)
#define  CSTVE_IOC_SET_BLACK_LEVEL   	_IOW(CSTVE_IOC_MAGIC, 0x05, int)
#define  CSTVE_IOC_GET_BLACK_LEVEL	    _IOR(CSTVE_IOC_MAGIC, 0x06, int)
#define  CSTVE_IOC_SET_SATURATION_LEVEL _IOW(CSTVE_IOC_MAGIC, 0x07, int)
#define  CSTVE_IOC_GET_SATURATION_LEVEL _IOR(CSTVE_IOC_MAGIC, 0x08, int)

#define  CSTVE_IOC_ENABLE 		        _IOW(CSTVE_IOC_MAGIC, 0x0b, int)
#define  CSTVE_IOC_DISABLE	 	        _IOR(CSTVE_IOC_MAGIC, 0x0c, int)

#define CSTVE_IOC_SET_MACROVISION       _IOW(CSTVE_IOC_MAGIC, 0x0d, int)
#define CSTVE_IOC_CVBS_SVIDEO_ENABLE    _IOW(CSTVE_IOC_MAGIC, 0xe, int)

#define CSTVE_IOC_WSS_CTRL	            _IOW(CSTVE_IOC_MAGIC, 0x0f, int)
#define CSTVE_IOC_WSS_SETCONFIG		    _IOW(CSTVE_IOC_MAGIC, 0x10, int)
#define CSTVE_IOC_WSS_SETINFO	        _IOW(CSTVE_IOC_MAGIC, 0x11, int)

/* 1201 ioctl entrance */
#define CSDF_IOC_TTX_CTRL       	_IOW('x', 0x25, int)
#define CSDF_IOC_TTX_SETCONFIG       	_IOW('x', 0x26, int)
#define CSDF_IOC_TTX_SETINFO       	_IOW('x', 0x27, int)

#ifdef ARCH_CSM1201
#define CSTVE_IOC_TTX_CTRL	        CSDF_IOC_TTX_CTRL
#define CSTVE_IOC_TTX_SETCONFIG		CSDF_IOC_TTX_SETCONFIG
#define CSTVE_IOC_TTX_SETINFO	        CSDF_IOC_TTX_SETINFO
#else
#define CSTVE_IOC_TTX_CTRL	        _IOW(CSTVE_IOC_MAGIC, 0x12, int)
#define CSTVE_IOC_TTX_SETCONFIG		_IOW(CSTVE_IOC_MAGIC, 0x13, int)
#define CSTVE_IOC_TTX_SETINFO	        _IOW(CSTVE_IOC_MAGIC, 0x14, int)
#endif

#define CSTVE_IOC_SET_COMP_CHAN         _IOW(CSTVE_IOC_MAGIC, 0x15, int)
#define CSTVE_IOC_SET_DAC0_COMPOSITION  _IOW(CSTVE_IOC_MAGIC, 0x16, int)		/* select the dac0 path */

#define CSTVE_IOC_BIND_GFX		_IOW(CSTVE_IOC_MAGIC, 0x17, int)
#define CSTVE_IOC_BIND_VID		_IOW(CSTVE_IOC_MAGIC, 0x18, int)
#define CSTVE_IOC_SET_OUTPUT		_IOW(CSTVE_IOC_MAGIC, 0x19, int)
#define CSTVE_IOC_GET_BIND_INF		_IOW(CSTVE_IOC_MAGIC,0x20,int)

struct{
	int gfx_output[2];
	int vid_output[2];
	int IsTV;
	CSTVOUT_OUTPUT_MODE output_mode[2];
}tvout_layer_output;

typedef struct tagCSTVOUT_OBJ {
	char obj_type;
	int dev_fd;
	int dev_fb0;
	int dev_fb1;
	
	int errno;
	CSTVOUT_MODE pre_mode;
#ifdef ARCH_CSM1201
	CSTVOUT_CHANNEL_ID dev_id; 
#endif
} CSTVOUT_OBJ;

#ifdef ARCH_CSM1201
static CSTVOUT_OBJ cstvout_obj[2] = {{ 0, -1, -1, -1, 0, -1, -1},{ 0, -1, -1, -1, 0, -1,-1}};
#else
static CSTVOUT_OBJ cstvout_obj = { 0, -1, 0, 0 ,0,-1};
#endif

static struct fb_var_screeninfo modes_lst[] = {
	{
	 .xres = 720,
	 .yres = 576,
	 .xres_virtual = 720,
	 .yres_virtual = 576,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 74074,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 144,
	 .vsync_len = 49,
	 .vmode = FB_VMODE_INTERLACED,
	 },			/* 576i, 720*576, interlaced */
	{
	 .xres = 720,
	 .yres = 480,
	 .xres_virtual = 720,
	 .yres_virtual = 480,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 74074,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 138,
	 .vsync_len = 45,
	 .vmode = FB_VMODE_INTERLACED,
	 },			/* 480i, 720*480, interlaced */
	{
	 .xres = 720,
	 .yres = 576,
	 .xres_virtual = 720,
	 .yres_virtual = 576,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 144,
	 .vsync_len = 49,
	 .vmode = FB_VMODE_NONINTERLACED,
	 },			/* 576p, 720*576, progressive */
	{
	 .xres = 720,
	 .yres = 480,
	 .xres_virtual = 720,
	 .yres_virtual = 480,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 138,
	 .vsync_len = 45,
	 .vmode = FB_VMODE_NONINTERLACED,
	 },			/* 480p, 720*480, progressive */
	{
	 .xres = 1280,
	 .yres = 720,
	 .xres_virtual = 1280,
	 .yres_virtual = 720,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 13468,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 700,
	 .vsync_len = 30,
	 .vmode = FB_VMODE_NONINTERLACED,
	 },			/* 720p50, 1280*720, progressive, 50fps */
	{
	 .xres = 1280,
	 .yres = 720,
	 .xres_virtual = 1280,
	 .yres_virtual = 720,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 13468,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 370,
	 .vsync_len = 30,
	 .vmode = FB_VMODE_NONINTERLACED,
	 },			/* 720p60, 1280*720, progressive, 60fps */
	{
	 .xres = 1920,
	 .yres = 1080,
	 .xres_virtual = 1920,
	 .yres_virtual = 1080,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 13468,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 720,
	 .vsync_len = 45,
	 .vmode = FB_VMODE_INTERLACED,
	 },			/* 1080i25, 1980*1080, interlaced, 25fps */
	{
	 .xres = 1920,
	 .yres = 1080,
	 .xres_virtual = 1920,
	 .yres_virtual = 1080,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 13468,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 280,
	 .vsync_len = 45,
	 .vmode = FB_VMODE_INTERLACED,
	 },			/* 1080i30, 1980*1080, interlaced, 30fps */

	{
	 .xres = 720,
	 .yres = 576,
	 .xres_virtual = 720,
	 .yres_virtual = 576,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 144,
	 .vsync_len = 49,
	 .vmode = FB_VMODE_INTERLACED,
	 },		/* secam : same as pal */
	{
	 .xres = 720,
	 .yres = 480,
	 .xres_virtual = 720,
	 .yres_virtual = 480,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 138,
	 .vsync_len = 45,
	 .vmode = FB_VMODE_INTERLACED,
	 },		/* pal-m : same as pal */
	{
	 .xres = 720,
	 .yres = 576,
	 .xres_virtual = 720,
	 .yres_virtual = 576,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 144,
	 .vsync_len = 49,
	 .vmode = FB_VMODE_INTERLACED,
	 },		/* pal-n : same as pal */
	{
	 .xres = 720,
	 .yres = 576,
	 .xres_virtual = 720,
	 .yres_virtual = 576,
	 .bits_per_pixel = 16,
	 .red = {11, 5, 0},
	 .green = {5, 6, 0},
	 .blue = {0, 5, 0},
	 .height = -1,
	 .width = -1,
	 .pixclock = 37037,
	 .left_margin = 0,
	 .right_margin = 0,
	 .upper_margin = 0,
	 .lower_margin = 0,
	 .hsync_len = 144,
	 .vsync_len = 49,
	 .vmode = FB_VMODE_INTERLACED,
    },		/* pal-cn : same as pal */
    {                                                                  
        .xres = 1920,                                                     
        .yres = 1080,                                                     
        .xres_virtual = 1920,                                             
        .yres_virtual = 1080,                                             
        .bits_per_pixel = 16,                                             
        .red = {11, 5, 0},                                                
        .green = {5, 6, 0},                                               
        .blue = {0, 5, 0},                                                
        .height = -1,                                                     
        .width = -1,                                                      
        .pixclock = 13468,                                                
        .left_margin = 0,                                                 
        .right_margin = 0,                                                
        .upper_margin = 0,                                                
        .lower_margin = 0,                                                
        .hsync_len = 830,                                                 
        .vsync_len = 45,                                                  
        .vmode = FB_VMODE_NONINTERLACED,                                  
    },                     /* 1080p24, 1980*1080, progressive, 24fps */
    {
        .xres = 1920,                                                      
        .yres = 1080,                                                      
        .xres_virtual = 1920,                                              
        .yres_virtual = 1080,                                              
        .bits_per_pixel = 16,                                              
        .red = {11, 5, 0},                                                 
        .green = {5, 6, 0},                                                
        .blue = {0, 5, 0},                                                 
        .height = -1,                                                      
        .width = -1,                                                       
        .pixclock = 13468,                                                 
        .left_margin = 0,                                                  
        .right_margin = 0,                                                 
        .upper_margin = 0,                                                 
        .lower_margin = 0,                                                 
        .hsync_len = 720,                                                  
        .vsync_len = 45,                                                   
        .vmode = FB_VMODE_NONINTERLACED,                                   
    },                     /* 1080p25, 1980*1080, progressive, 25fps */
    {                                                                   
        .xres = 1920,                                                      
        .yres = 1080,                                                      
        .xres_virtual = 1920,                                              
        .yres_virtual = 1080,
        .bits_per_pixel = 16,
        .red = {11, 5, 0}, 
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 13468,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 280,
        .vsync_len = 45,
        .vmode = FB_VMODE_NONINTERLACED,  
    },                     /* 1080p30, 1980*1080, progressive, 30fps */
    {
        .xres = 640,
        .yres = 480,
        .xres_virtual = 640,
        .yres_virtual = 480,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 39730,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 160,
        .vsync_len = 45,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 640*480*60Hz*/
    {
        .xres = 800,
        .yres = 600,
        .xres_virtual = 800,
        .yres_virtual = 600,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 25000,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 256,
        .vsync_len = 28,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 800*600*60Hz*/
    {
        .xres = 800,
        .yres = 600,
        .xres_virtual = 800,
        .yres_virtual = 600,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 20000,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 240,
        .vsync_len = 66,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 800*600*72Hz*/
    {
        .xres = 1024,
        .yres = 768,
        .xres_virtual = 1024,
        .yres_virtual = 768,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 15385,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 320,
        .vsync_len = 38,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 1024*768*60Hz*/
    {
        .xres = 1280,
        .yres = 1024,
        .xres_virtual = 1280,
        .yres_virtual = 1024,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 11189,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 416,
        .vsync_len = 30,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 1280*1024*50Hz*/
    {
        .xres = 1600,
        .yres = 1000,
        .xres_virtual = 1600,
        .yres_virtual = 1000,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 7518,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 544,
        .vsync_len = 35,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 1600*1000*60Hz*/
    {
        .xres = 1280,
        .yres = 1024,
        .xres_virtual = 1280,
        .yres_virtual = 1024,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 11236,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 128,
        .vsync_len = 30,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 1280*1024*60Hz*/
    {
        .xres = 1280,
        .yres = 720,
        .xres_virtual = 1280,
        .yres_virtual = 720,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 13445,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 384,
        .vsync_len = 26,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 1280*720*60Hz*/
    {
        .xres = 848,
        .yres = 480,
        .xres_virtual = 848,
        .yres_virtual = 480,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 29630,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 240,
        .vsync_len = 37,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 848*480*60Hz*/
    {
        .xres = 800,
        .yres = 480,
        .xres_virtual = 800,
        .yres_virtual = 480,
        .bits_per_pixel = 16,
        .red = {11, 5, 0},
        .green = {5, 6, 0},
        .blue = {0, 5, 0},
        .height = -1,
        .width = -1,
        .pixclock = 33898,
        .left_margin = 0,
        .right_margin = 0,
        .upper_margin = 0,
        .lower_margin = 0,
        .hsync_len = 192,
        .vsync_len = 20,
        .vmode = FB_VMODE_NONINTERLACED,
    },                     /* VGA, 800*480*60Hz*/
};

static char *tvout_errstr[] = {
	"no error",
	"open failed",
	"ioctl failed",
	"invalid arguments",
	"unknown device name",
	"device busy",
	"invalid handle",
	"device already initialized",
	"device wasn't initialized"
};

#ifdef ARCH_CSM1201
CSTVOUT_HANDLE CSTVOUT_Open(CSTVOUT_CHANNEL_ID id)
{
	if (cstvout_obj[id].obj_type != CSTVOUT_OBJ_TYPE) {
		if(id == 0){
			cstvout_obj[id].dev_fd = open(CSTVOUT_DEV0_FILE, O_RDWR);
		}
		else if(id == 1){
			cstvout_obj[id].dev_fd = open(CSTVOUT_DEV1_FILE, O_RDWR);
		}
		if (cstvout_obj[id].dev_fd < 0) {
			cstvout_obj[id].errno = TVOUT_ERROR_OPEN_FAILED;
			return NULL;
		}
	}

	cstvout_obj[id].dev_fb0 = open("/dev/fb/0", O_RDWR);
	if (cstvout_obj[id].dev_fb0 < 0) {
		close(cstvout_obj[id].dev_fd);
		cstvout_obj[id].errno = TVOUT_ERROR_OPEN_FAILED;
		return NULL;
	}

	cstvout_obj[id].dev_fb1 = open("/dev/fb/1", O_RDWR);
	if (cstvout_obj[id].dev_fb1 < 0) {
		close(cstvout_obj[id].dev_fd);
		close(cstvout_obj[id].dev_fb0);
		cstvout_obj[id].errno = TVOUT_ERROR_OPEN_FAILED;
		return NULL;
	}
	
	cstvout_obj[id].dev_id = id;
	cstvout_obj[id].obj_type = CSTVOUT_OBJ_TYPE;

	return (CSTVOUT_HANDLE) & cstvout_obj[id];
}

CSAPI_RESULT CSTVOUT_Close(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	dev_obj->obj_type = 0;
	close(dev_obj->dev_fd);
	dev_obj->dev_fd = -1;
	
	close(dev_obj->dev_fb0);
	close(dev_obj->dev_fb1);
	dev_obj->dev_fb0 = -1;
	dev_obj->dev_fb0 = -1;
	
	return CSAPI_SUCCEED;
}

#else

CSTVOUT_HANDLE CSTVOUT_Open(CSTVOUT_CHANNEL_ID id)
{
	UNUSED_VARIABLE(id);

	if (cstvout_obj.obj_type != CSTVOUT_OBJ_TYPE) {
		cstvout_obj.dev_fd = open(CSTVOUT_DEV_FILE, O_RDWR);

		if (cstvout_obj.dev_fd < 0) {
			cstvout_obj.errno = TVOUT_ERROR_OPEN_FAILED;
			return NULL;
		}
	}

	cstvout_obj.dev_fb0 = open("/dev/fb/0", O_RDWR);
	if (cstvout_obj.dev_fb0 < 0) {
		close(cstvout_obj.dev_fd);
		cstvout_obj.errno = TVOUT_ERROR_OPEN_FAILED;
		return NULL;
	}

	cstvout_obj.dev_fb1 = open("/dev/fb/1", O_RDWR);
	if (cstvout_obj.dev_fb1 < 0) {
		close(cstvout_obj.dev_fd);
		close(cstvout_obj.dev_fb0);
		cstvout_obj.errno = TVOUT_ERROR_OPEN_FAILED;
		return NULL;
	}

	cstvout_obj.obj_type = CSTVOUT_OBJ_TYPE;

	return (CSTVOUT_HANDLE) & cstvout_obj;
}

CSAPI_RESULT CSTVOUT_Close(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	dev_obj->obj_type = 0;
	close(dev_obj->dev_fd);
	close(dev_obj->dev_fb0);
	close(dev_obj->dev_fb1);

	return CSAPI_SUCCEED;
}
#endif

CSAPI_RESULT   CSTVOUT_Enable(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_ENABLE, 0, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT   CSTVOUT_Disable(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_DISABLE, 0, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_SetMode(CSTVOUT_HANDLE handle, CSTVOUT_MODE vid_mod)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;
//	static CSTVOUT_MODE pre_mode = -1;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	if(vid_mod == TVOUT_RGB_1600X1000_60FPS){
		return CSAPI_FAILED;
	}

	if(vid_mod == dev_obj->pre_mode){
		return CSAPI_SUCCEED;
	}

	if(((vid_mod == TVOUT_MODE_1080I25)||(vid_mod == TVOUT_MODE_1080I30)||(vid_mod == TVOUT_MODE_1080P24))&&((dev_obj->pre_mode == TVOUT_MODE_720P50)||(dev_obj->pre_mode == TVOUT_MODE_720P60))){
		IOCTL(dev_obj, CSTVE_IOC_SET_MODE, TVOUT_MODE_576I, TVOUT);
	}

	dev_obj->pre_mode = vid_mod;

	IOCTL(dev_obj, CSTVE_IOC_SET_MODE, vid_mod, TVOUT);

#ifdef ARCH_CSM1201
	{ /* FOR AVOIDING A DEGIAN FAULT, DON'T REMOVE IT. */
		struct fb_var_screeninfo var;

		if (0 == ioctl(dev_obj->dev_fd, CSTVE_IOC_GET_BIND_INF, &tvout_layer_output)){
			if(tvout_layer_output.gfx_output[0] == (int)dev_obj->dev_id){
				ioctl(dev_obj->dev_fb0, FBIOGET_VSCREENINFO, &var);
				modes_lst[vid_mod].bits_per_pixel = var.bits_per_pixel;
				memcpy(&modes_lst[vid_mod].red, &var.red, sizeof(var.red));
				memcpy(&modes_lst[vid_mod].green, &var.green, sizeof(var.green));
				memcpy(&modes_lst[vid_mod].blue, &var.blue, sizeof(var.blue));
				memcpy(&modes_lst[vid_mod].transp, &var.transp, sizeof(var.transp));
				ioctl(dev_obj->dev_fb0, FBIOPUT_VSCREENINFO, &modes_lst[vid_mod]);
			}
			if(tvout_layer_output.gfx_output[1] == (int)dev_obj->dev_id){
				ioctl(dev_obj->dev_fb1, FBIOGET_VSCREENINFO, &var);
				modes_lst[vid_mod].bits_per_pixel = var.bits_per_pixel;
				memcpy(&modes_lst[vid_mod].red, &var.red, sizeof(var.red));
				memcpy(&modes_lst[vid_mod].green, &var.green, sizeof(var.green));
				memcpy(&modes_lst[vid_mod].blue, &var.blue, sizeof(var.blue));
				memcpy(&modes_lst[vid_mod].transp, &var.transp, sizeof(var.transp));
				ioctl(dev_obj->dev_fb1, FBIOPUT_VSCREENINFO, &modes_lst[vid_mod]);
			}
		}		
	}
#else
	{ /* FOR AVOIDING A DEGIAN FAULT, DON'T REMOVE IT. */
		struct fb_var_screeninfo var;

		if (0 == ioctl(dev_obj->dev_fb0, FBIOGET_VSCREENINFO, &var))
		{
			modes_lst[vid_mod].bits_per_pixel = var.bits_per_pixel;

			memcpy(&modes_lst[vid_mod].red, &var.red, sizeof(var.red));
			memcpy(&modes_lst[vid_mod].green, &var.green, sizeof(var.green));
			memcpy(&modes_lst[vid_mod].blue, &var.blue, sizeof(var.blue));
			memcpy(&modes_lst[vid_mod].transp, &var.transp, sizeof(var.transp));
		}
	}

	ioctl(dev_obj->dev_fb0, FBIOPUT_VSCREENINFO, &modes_lst[vid_mod]);

	{ /* FOR AVOIDING A DEGIAN FAULT, DON'T REMOVE IT. */
		struct fb_var_screeninfo var;

		if (0 == ioctl(dev_obj->dev_fb1, FBIOGET_VSCREENINFO, &var))
		{
			modes_lst[vid_mod].bits_per_pixel = var.bits_per_pixel;

			memcpy(&modes_lst[vid_mod].red, &var.red, sizeof(var.red));
			memcpy(&modes_lst[vid_mod].green, &var.green, sizeof(var.green));
			memcpy(&modes_lst[vid_mod].blue, &var.blue, sizeof(var.blue));
			memcpy(&modes_lst[vid_mod].transp, &var.transp, sizeof(var.transp));
		}
	}
	ioctl(dev_obj->dev_fb1, FBIOPUT_VSCREENINFO, &modes_lst[vid_mod]);
#endif
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_GetMode(CSTVOUT_HANDLE handle, CSTVOUT_MODE * vid_mod)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_GET_MODE, vid_mod, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT   CSTVOUT_SetZorder(CSTVOUT_HANDLE handle, CSTVOUT_DF_ZORDER zorder)
{
	#define FBIO_Z_ORDER _IOW('F', 0x50, u_int32_t)
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
	ioctl(dev_obj->dev_fb0, FBIO_Z_ORDER, zorder);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT   CSTVOUT_SetBkColor(CSTVOUT_HANDLE handle, unsigned char r, unsigned char g, unsigned char b)
{
#define FBIO_GFX_BGCOLOR _IOW('F', 0x54, u_int32_t)
	unsigned char y = (0.2578125*r) + (0.50390625*g) + (0.09765625*b) + 16;   
	unsigned char u = (0.4375*r) - (0.3671875*g) - (0.0703125*b) + 128;   
	unsigned char v =-(0.1484375*r) - (0.2890625*g) + (0.4375*b) + 128;   
	
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
	ioctl(dev_obj->dev_fb0, FBIO_GFX_BGCOLOR, (y<<16) || (u<<8) || (v<<0));

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_SetBrightness(CSTVOUT_HANDLE handle, unsigned int brightness)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
	if (brightness < 1 || brightness > 254) {
		dev_obj->errno = TVOUT_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	brightness = 254 + brightness * 3;

	IOCTL(dev_obj, CSTVE_IOC_SET_WHILE_LEVEL, brightness, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_GetBrightness(CSTVOUT_HANDLE handle, unsigned int *brightness)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
	IOCTL(dev_obj, CSTVE_IOC_GET_WHILE_LEVEL, (unsigned int) brightness, TVOUT);

	*brightness = (*brightness - 254) / 3;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_SetContrast(CSTVOUT_HANDLE handle, unsigned int contrast)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_SET_BLACK_LEVEL, contrast, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_GetContrast(CSTVOUT_HANDLE handle, unsigned int *contrast)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_GET_BLACK_LEVEL, (unsigned int) contrast, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_SetSaturation(CSTVOUT_HANDLE handle, unsigned int saturation)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_SET_SATURATION_LEVEL, saturation, TVOUT);

	return CSAPI_SUCCEED;
}


/* MCEnable  enables or disables MacroVision  on CVBS and S-Video analog outputs: */
/* MCEnable =0 (Disable), MCEnable=1(Enable)*/ 
/* Returns CSAPI_SUCCEEDED or CSAPI_FAILED */ 
CSAPI_RESULT CSTVOUT_EnableMacroVision(CSTVOUT_HANDLE handle, unsigned char MCEnable)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
        
	IOCTL(dev_obj, CSTVE_IOC_SET_MACROVISION, MCEnable, TVOUT);

	return CSAPI_SUCCEED;
}

#if 0  /* This function looks like not required, for CSM1200 though gpio to control CVBS/S-Video and added some special functions to control Macrovision */
       /* CSM1201 has dual TV encoder, can support output HD on a TVE, and at same time output SD on other TVE (by using HD2SD capture). */
       /* So disable CVBS/S-Video is also not required for CSM1201 */

/* configuration outputs behavior (such as DIS-(En)able CVBS and S-Video)*/
/* support commands: OUTPUT_CONTROL_CVBS_SVIDEO */
/* Returns CSAPI_SUCCEEDED or CSAPI_FAILED */ 
CSAPI_RESULT CSTVOUT_ControlOutput(CSTVOUT_HANDLE handle, CSTVOUT_OUTPUT_CMD cstvout_output_configure_cmd, unsigned int arg)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	switch (cstvout_output_configure_cmd ){
	case OUTPUT_CONTROL_CVBS_SVIDEO: /* Now we disable automatically S-Video and CVBS in HD format output. 
   They can be enabled again maually by using this function, if chip can support dual format output*/

		if (ENABLE_CVBS_SVIDEO == arg)
	  		IOCTL(dev_obj, CSTVE_IOC_CVBS_SVIDEO_ENABLE, 1, TVOUT);
	    else
			IOCTL(dev_obj, CSTVE_IOC_CVBS_SVIDEO_ENABLE, 0, TVOUT);

		break;

    case OUTPUT_CONTROL_MACROVISION:
		if (ENABLE_MACROVISION == arg)
	  		IOCTL(dev_obj, CSTVE_IOC_SET_MACROVISION, 1, TVOUT);
	    else if (DISABLE_MACROVISION == arg)
			IOCTL(dev_obj, CSTVE_IOC_SET_MACROVISION, 0, TVOUT);

        break;
	}
	return CSAPI_SUCCEED;
}
#endif

CSAPI_RESULT CSTVOUT_GetSaturation(CSTVOUT_HANDLE handle, unsigned int *saturation)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_GET_SATURATION_LEVEL, (unsigned int) saturation, TVOUT);

	return CSAPI_SUCCEED;
}

/*================================================================================*/
CSAPI_RESULT CSTVOUT_SetCompChannel(CSTVOUT_HANDLE handle, CSVOUT_CompChannType_t CompChan)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_SET_COMP_CHAN, (unsigned int)CompChan, TVOUT);
	
	return CSAPI_SUCCEED;
}

/*-----------------------------------------------------------------------------*/

CSAPI_RESULT CSTVOUT_WSS_Ctrl(CSTVOUT_HANDLE handle, char Enable)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_WSS_CTRL, (unsigned int)Enable, TVOUT);

	return CSAPI_SUCCEED;
}



CSAPI_RESULT CSTVOUT_WSS_SetFormat(CSTVOUT_HANDLE handle, CSTVOUT_WSSSTD WssStd)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_WSS_SETCONFIG, (unsigned int)WssStd, TVOUT);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSTVOUT_WSS_SetInfo(CSTVOUT_HANDLE handle, CSTVOUT_WSSINFO *WssInfo)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_WSS_SETINFO, (unsigned int)WssInfo, TVOUT);

	return CSAPI_SUCCEED;
}


/*-----------------------------------------------------------------------------*/

CSAPI_RESULT CSTVOUT_TXT_Ctrl(CSTVOUT_HANDLE handle, char Enable)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_TTX_CTRL, (unsigned int)Enable, TVOUT);

	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSTVOUT_TXT_SetFormat(CSTVOUT_HANDLE handle, CSVOUT_TxtStandard_t TxtStd)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_TTX_SETCONFIG, (unsigned int)TxtStd, TVOUT);

	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSTVOUT_TXT_SetInfo(CSTVOUT_HANDLE handle, CSVOUT_TxtPage_t *TxtPage)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);

	IOCTL(dev_obj, CSTVE_IOC_TTX_SETINFO, (unsigned int)TxtPage, TVOUT);

	return CSAPI_SUCCEED;
}



/*================================================================================*/

CSTVOUT_ErrCode CSTVOUT_GetErrCode(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSTVOUT_OBJ_TYPE))
		return TVOUT_ERROR_INVALID_PARAMETERS;

	return dev_obj->errno;
}

char *CSTVOUT_GetErrString(CSTVOUT_HANDLE handle)
{
	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSTVOUT_OBJ_TYPE))
		return tvout_errstr[TVOUT_ERROR_INVALID_PARAMETERS];

	return tvout_errstr[dev_obj->errno];
}

/**************************************DUAL OUTPUT CONFIGURATION ******************************************/
#ifdef ARCH_CSM1201
 CSAPI_RESULT CSTVOUT_SetOutput(CSTVOUT_HANDLE handle, CSTVOUT_OUTPUT_MODE output_mode)
 {
 	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;
 
 	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
 
 	if(((dev_obj->dev_id == TVOUT_CHANNEL0)&&(output_mode ==OUTPUT_MODE_CVBS_SVIDEO))
 		||((dev_obj->dev_id == TVOUT_CHANNEL1)&&(output_mode ==OUTPUT_MODE_RGB))){
 		printf("Current TVOUT device %d do not support OUTPUT Mode %d\n",dev_obj->dev_id,output_mode);
 		return CSAPI_FAILED;
 	}
 
 	IOCTL(dev_obj, CSTVE_IOC_SET_OUTPUT, output_mode, TVOUT);
 
 	return CSAPI_SUCCEED;
 }
 
 CSAPI_RESULT CSTVOUT_BindGFX(CSTVOUT_HANDLE handle,CSTVOUT_OSD_LAYER index)
 {
 	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;
 
 	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
 
 	IOCTL(dev_obj, CSTVE_IOC_BIND_GFX, index, TVOUT);
 
 	return CSAPI_SUCCEED;
 }
 
 CSAPI_RESULT CSTVOUT_BindVID(CSTVOUT_HANDLE handle,CSTVOUT_VID_DEV index)
 {
 	CSTVOUT_OBJ *dev_obj = (CSTVOUT_OBJ *) handle;
 
 	CHECK_HANDLE_VALID(dev_obj, CSTVOUT_OBJ_TYPE);
 
 	IOCTL(dev_obj, CSTVE_IOC_BIND_VID, index, TVOUT);
 
 	return CSAPI_SUCCEED;
 }

#endif

