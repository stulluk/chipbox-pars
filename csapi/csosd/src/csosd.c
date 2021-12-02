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
#include <sys/mman.h>

#include "csosd.h"

#define CSOSD_OBJ_TYPE		'e'

#define DEV_NUMS		2

#define CSOSD_DEV_FILE0		"/dev/fb/2"	/* FIXME@zhongkai's ugly code */
#define CSOSD_DEV_FILE1		"/dev/fb/3"	/* FIXME@zhongkai's ugly code */

#define FBIO_WAITFORVSYNC       _IOW('F', 0x20, int)
#define FBIO_GFX_ON             _IOW('F', 0x21, int)
#define FBIO_GFX_ALPHA          _IOW('F', 0x22, int)
#define FBIO_Z_ORDER            _IOW('F', 0x50, int)
#define FBIO_GFX_FLIP           _IOW('F', 0x51, gfx2d_scalor_params)
#define FBIO_GFX_COLORKEY_ON    _IOW('F', 0x52, int)
#define FBIO_GFX_COLORKEY_VAL   _IOW('F', 0x53, CSOSD_KeyColor)

typedef struct _2d_scalor_params{
	CSOSD_Rect src_rect;
	unsigned int src_width;
	unsigned int src_height;
	int src_color_format;
	unsigned int src_pitch_line;
	unsigned int src_bits_pixel;
	unsigned int src_phy_addr;

	CSOSD_Rect dst_rect;
	unsigned int dst_width;
	unsigned int dst_height;
	int dst_color_format;
	unsigned int dst_pitch_line;
	unsigned int dst_bits_pixel;
	unsigned int dst_phy_addr;

} gfx2d_scalor_params;

typedef struct tagCSOSD_OBJ {
	char obj_type;
	int dev_fd;

	char *base_addr;
	int screen_size;

	int errno;
} CSOSD_OBJ;

static CSOSD_OBJ csosd_objs[DEV_NUMS] = {
	{0, -1, NULL, 0, 0},
	{0, -1, NULL, 0, 0}
};

static char *osd_errstr[] = {
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
    },                    /* VGA, 1280*1024*60Hz*/
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
};

static CSOSD_Config osd_config = { 0, 0, 0 };
static int osd_config_ornot = 0;

CSOSD_HANDLE CSOSD_Open(CSOSD_LAYER layer)
{
	static char *devfile_list[] = {
		CSOSD_DEV_FILE0,
		CSOSD_DEV_FILE1
	};

	if (layer >= DEV_NUMS)
		return NULL;

	if (csosd_objs[layer].obj_type != CSOSD_OBJ_TYPE) {
		csosd_objs[layer].dev_fd = open(devfile_list[layer], O_RDWR);
		if (csosd_objs[layer].dev_fd < 0) {
			csosd_objs[layer].errno = OSD_ERROR_OPEN_FAILED;
			return NULL;
		}
	}

	csosd_objs[layer].obj_type = CSOSD_OBJ_TYPE;
	osd_config_ornot = 0;

	return (CSOSD_HANDLE) & csosd_objs[layer];
}

CSAPI_RESULT CSOSD_Close(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	dev_obj->obj_type = 0;
	close(dev_obj->dev_fd);

	osd_config_ornot = 0;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_Enable(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_ON, 1, OSD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_Disable(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_ON, 0, OSD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_SetConfig(CSOSD_HANDLE handle, CSOSD_Config * config)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);
	
	if(config->color_format == OSD_COLOR_FORMAT_ARGB4444) {
		modes_lst[config->mode].red.offset = 8;
		modes_lst[config->mode].red.length = 4;
		modes_lst[config->mode].red.msb_right = 0;

		modes_lst[config->mode].green.offset = 4;
		modes_lst[config->mode].green.length = 4;
		modes_lst[config->mode].green.msb_right = 0;

		modes_lst[config->mode].blue.offset = 0;
		modes_lst[config->mode].blue.length = 4;
		modes_lst[config->mode].blue.msb_right = 0;

		modes_lst[config->mode].transp.offset = 12;
		modes_lst[config->mode].transp.length = 4;
		modes_lst[config->mode].transp.msb_right = 0;

		osd_config_ornot = 1;
		osd_config.color_depth = config ->color_depth;
		osd_config.color_format = OSD_COLOR_FORMAT_ARGB4444;
		osd_config.mode = config ->mode;
	}
	else if(config->color_format == OSD_COLOR_FORMAT_RGB565) {
		modes_lst[config->mode].red.offset = 11;
		modes_lst[config->mode].red.length = 5;
		modes_lst[config->mode].red.msb_right = 0;

		modes_lst[config->mode].green.offset = 5;
		modes_lst[config->mode].green.length = 6;
		modes_lst[config->mode].green.msb_right = 0;

		modes_lst[config->mode].blue.offset = 0;
		modes_lst[config->mode].blue.length = 5;
		modes_lst[config->mode].blue.msb_right = 0;

		modes_lst[config->mode].transp.offset = 0;
		modes_lst[config->mode].transp.length = 0;
		modes_lst[config->mode].transp.msb_right = 0;

		osd_config_ornot = 1;
		osd_config.color_depth = config ->color_depth;
		osd_config.color_format = OSD_COLOR_FORMAT_RGB565;
		osd_config.mode = config ->mode;
	}
	else if(config->color_format == OSD_COLOR_FORMAT_ARGB1555) {
		printf("OSD_COLOR_FORMAT_ARGB1555\n");
		modes_lst[config->mode].red.offset = 10;
		modes_lst[config->mode].red.length = 5;
		modes_lst[config->mode].red.msb_right = 0;

		modes_lst[config->mode].green.offset = 5;
		modes_lst[config->mode].green.length = 5;
		modes_lst[config->mode].green.msb_right = 0;

		modes_lst[config->mode].blue.offset = 0;
		modes_lst[config->mode].blue.length = 5;
		modes_lst[config->mode].blue.msb_right = 0;

		modes_lst[config->mode].transp.offset = 15;
		modes_lst[config->mode].transp.length = 1;
		modes_lst[config->mode].transp.msb_right = 0;

		osd_config_ornot = 1;
		osd_config.color_depth = config ->color_depth;
		osd_config.color_format = OSD_COLOR_FORMAT_ARGB1555;
		osd_config.mode = config ->mode;
	}
#if defined(ARCH_CSM1201)
	else if(config->color_format == OSD_COLOR_FORMAT_ARGB8888) {
		printf("OSD_COLOR_FORMAT_ARGB8888\n");
		modes_lst[config->mode].transp.offset = 24;
		modes_lst[config->mode].transp.length = 8;
		modes_lst[config->mode].transp.msb_right = 0;

		modes_lst[config->mode].red.offset = 16;
		modes_lst[config->mode].red.length = 8;
		modes_lst[config->mode].red.msb_right = 0;

		modes_lst[config->mode].green.offset = 8;
		modes_lst[config->mode].green.length = 8;
		modes_lst[config->mode].green.msb_right = 0;

		modes_lst[config->mode].blue.offset = 0;
		modes_lst[config->mode].blue.length = 8;
		modes_lst[config->mode].blue.msb_right = 0;

		modes_lst[config->mode].bits_per_pixel = 32;

		osd_config_ornot = 1;
		osd_config.color_depth = config ->color_depth;
		osd_config.color_format = OSD_COLOR_FORMAT_ARGB8888;
		osd_config.mode = config ->mode;
	}
#endif
	else
		return CSAPI_FAILED;

	IOCTL(dev_obj, FBIOPUT_VSCREENINFO, &modes_lst[config->mode], OSD);

#if 1
	/* Refresh fb0 & fb1 */
	{
		struct fb_var_screeninfo var;

		int fd0 = open("/dev/fb/0", O_RDWR);
		int fd1 = open("/dev/fb/1", O_RDWR);

		ioctl(fd0, FBIOGET_VSCREENINFO, &var);
		var.activate |= FB_ACTIVATE_FORCE;
		ioctl(fd0, FBIOPUT_VSCREENINFO, &var);

		ioctl(fd1, FBIOGET_VSCREENINFO, &var);
		var.activate |= FB_ACTIVATE_FORCE;
		ioctl(fd1, FBIOPUT_VSCREENINFO, &var);

		close(fd0);
		close(fd1);
		fd0 = -1;
		fd1 = -1;
	}
#endif

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_GetConfig(CSOSD_HANDLE handle, CSOSD_Config * config)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( osd_config_ornot == 0 ){
		dev_obj->errno = OSD_ERROR_OPERATION_ERROR;
		return CSAPI_FAILED;
	}

	config->color_depth = osd_config.color_depth;
	config->color_format = osd_config.color_format;
	config->mode = osd_config.mode;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_GetBaseAddr(CSOSD_HANDLE handle, unsigned char **addr)
{
	struct fb_var_screeninfo vinfo;

	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIOGET_VSCREENINFO, &vinfo, OSD);

	dev_obj->screen_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	dev_obj->base_addr =
	    (unsigned char *) mmap(NULL, dev_obj->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_obj->dev_fd, 0);
	if (MAP_FAILED == dev_obj->base_addr) {
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	*addr = (unsigned char*)(dev_obj->base_addr);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_SetAlpha(CSOSD_HANDLE handle, int alpha)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_ALPHA, alpha, OSD);

	return CSAPI_SUCCEED;
}

// TODO CSAPI_RESULT CSOSD_GetAlpha(CSOSD_HANDLE handle, int *alpha);

CSAPI_RESULT CSOSD_EnableKeyColor(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_COLORKEY_ON, 1, OSD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_DisableKeyColor(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_COLORKEY_ON, 0, OSD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_SetKeyColor(CSOSD_HANDLE handle, CSOSD_KeyColor * key_color)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_COLORKEY_VAL, key_color, OSD);

	return CSAPI_SUCCEED;
}

// TODO CSAPI_RESULT CSOSD_GetKeyColor(CSOSD_HANDLE handle,
// TODO                                CSOSD_KeyColor * key_color);

CSAPI_RESULT CSOSD_Flip(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_GFX_FLIP, NULL, OSD);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_FlipRect(CSOSD_HANDLE handle, 
			CSOSD_Rect *src_rect, CSOSD_Rect *dst_rect)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if ((NULL == src_rect) || (NULL == dst_rect))
		IOCTL(dev_obj, FBIO_GFX_FLIP, NULL, OSD);
	else
	{
		gfx2d_scalor_params blit_conf;

		blit_conf.src_rect.left = src_rect->left;
		blit_conf.src_rect.top = src_rect->top;
		blit_conf.src_rect.right = src_rect->right;
		blit_conf.src_rect.bottom = src_rect->bottom;

		blit_conf.dst_rect.left = dst_rect->left;
		blit_conf.dst_rect.top = dst_rect->top;
		blit_conf.dst_rect.right = dst_rect->right;
		blit_conf.dst_rect.bottom = dst_rect->bottom;

		IOCTL(dev_obj, FBIO_GFX_FLIP, &blit_conf, OSD);
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_WaitVSync(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	IOCTL(dev_obj, FBIO_WAITFORVSYNC, 0, OSD);

	return CSAPI_SUCCEED;
}

CSOSD_ErrCode CSOSD_GetErrCode(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSOSD_OBJ_TYPE))
		return OSD_ERROR_INVALID_PARAMETERS;

	return dev_obj->errno;
}

char *CSOSD_GetErrString(CSOSD_HANDLE handle)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	if ((NULL == dev_obj) || (dev_obj->obj_type != CSOSD_OBJ_TYPE))
		return osd_errstr[OSD_ERROR_INVALID_PARAMETERS];

	return osd_errstr[dev_obj->errno];
}
