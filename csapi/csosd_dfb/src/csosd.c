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
#include <pthread.h>
#include <math.h>

#include "csosd.h"
#include "directfb.h"

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

#define CSOSD_LOCK(x)     	(void)pthread_mutex_lock(&x)
#define CSOSD_UNLOCK(x)   	(void)pthread_mutex_unlock(&x)

#define MAX(x,y)	((x>y)?(x):(y))
#define MIN(x,y)	((x>y)?(y):(x))

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

	IDirectFB              *dfb;
	IDirectFBDisplayLayer	*layer;
	IDirectFBSurface				*bgsurface;
	IDirectFBWindow				*windows[OBJECTS_NUMBER];
	IDirectFBSurface				*surfaces[OBJECTS_NUMBER];
	pthread_t 						magic_task[OBJECTS_NUMBER];
	unsigned	int					usednumb;
	unsigned	int					scr_width;
	unsigned	int					scr_height;
	pthread_mutex_t 			osd_mutex;

	int errno;
} CSOSD_OBJ;

typedef struct _magic_task_params{
	CSOSD_OBJ *p_obj;
	PIC_ID			id;
	EXTRA_FUNC	func;
} magic_task_params;

static CSOSD_OBJ csosd_objs[DEV_NUMS] = {
	{
		.obj_type = 0, 
		.dev_fd = -1,
		.base_addr = NULL,
		.screen_size = 0,
		.errno = 0
	},
	{
		.obj_type = 0, 
		.dev_fd = -1,
		.base_addr = NULL,
		.screen_size = 0,
		.errno = 0
	}
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
	"device wasn't initialized",
	"directfb init error",
	"directfb getdisplaylayer error",
	"directfb createsurface error",
	"directfb flip error",
	"directfb creat window error",
	"directfb get surface error",
	"directfb creat image provider error",
	"directfb show image id parameter error",
	"directfb not enough unused window, please image_clear()",
	"directfb image id parameter error",
	"directfb set opacity error",
	"directfb blit error",
	"directfb move error",
	"directfb malloc error",
	"directfb pthread creat error",
	"directfb surface clear error",
	"directfb raise to top error",
	"directfb render to error",
	"directfb set background image error",
	"directfb fill rectangle error",
	"directfb fill triangle error",
	"directfb draw line error",
	"directfb draw rectangle error",
	"directfb fill spans error"
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

static DFBResult _directfb_init (CSOSD_OBJ *handle);
static int _find_used_windownumb (CSOSD_OBJ *handle, unsigned int startnumber);
static int _find_unused_windownumb (CSOSD_OBJ *handle);
static inline long _myclock(void);
static int _fadeinout_task(void *param);


static DFBResult _directfb_init (CSOSD_OBJ *handle)
{
	DFBResult	dfbret;
	DFBSurfaceDescription 				desc;
	DFBGraphicsDeviceDescription  gdesc;
	DFBDisplayLayerConfig         	layer_config;
	
	handle->dfb = NULL;
	handle->layer = NULL;
	handle->bgsurface = NULL;
	handle->usednumb = 0;
	handle->scr_width = 0;
	handle->scr_height = 0;
	pthread_mutex_init(&handle->osd_mutex, NULL);
	memset( handle->windows, 0x0, sizeof(handle->windows) );
	memset( handle->surfaces, 0x0, sizeof(handle->surfaces) );
	memset( handle->magic_task, 0x0, sizeof(handle->magic_task) );

	dfbret = DirectFBInit( 0, 0 );
	dfbret |= DirectFBCreate( &handle->dfb );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Init Create error\n", __FUNCTION__);
		return dfbret;
	}
	
	printf("+++++++++++Test DFB+++++++++++++++++++++\n");
	
	dfbret = handle->dfb->GetDisplayLayer( handle->dfb, DLID_PRIMARY, &handle->layer );
	if(dfbret != DFB_OK){
		printf("%s: GetDisplayLayer error\n", __FUNCTION__);
		return dfbret;
	}
	printf("test dfk ok!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	handle->layer->SetCooperativeLevel( handle->layer, DLSCL_ADMINISTRATIVE );

	handle->dfb->GetDeviceDescription( handle->dfb, &gdesc );
	if (!((gdesc.blitting_flags & DSBLIT_BLEND_ALPHACHANNEL) && (gdesc.blitting_flags & DSBLIT_BLEND_COLORALPHA))){
		layer_config.flags = DLCONF_BUFFERMODE;
		layer_config.buffermode = DLBM_BACKSYSTEM;
		handle->layer->SetConfiguration( handle->layer, &layer_config );
	}

	handle->layer->GetConfiguration( handle->layer, &layer_config );
	memset( &desc, 0, sizeof(DFBSurfaceDescription) );
	desc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS;

	/*desc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_CAPS | DSDESC_PIXELFORMAT;
	desc.pixelformat = DSPF_ARGB;*/
	
	desc.width = layer_config.width;
	desc.height = layer_config.height;
	desc.caps = DSCAPS_PRIMARY | DSCAPS_DOUBLE;
	dfbret = handle->dfb->CreateSurface( handle->dfb, &desc, &handle->bgsurface );
	if(dfbret != DFB_OK){
		printf("%s: CreateSurface error\n", __FUNCTION__);
		return dfbret;
	}

	handle->scr_width = layer_config.width;
	handle->scr_height = layer_config.height;

	handle->bgsurface->SetDrawingFlags ( handle->bgsurface, DSDRAW_BLEND );
	handle->bgsurface->SetColor ( handle->bgsurface, 0x0, 0x0, 0x0, 0x0 );
	handle->bgsurface->FillRectangle ( handle->bgsurface, 0, 0, desc.width, desc.height );
	dfbret = handle->bgsurface->Flip ( handle->bgsurface, NULL, 0 );
	handle->bgsurface->SetDrawingFlags ( handle->bgsurface, DSDRAW_BLEND );
	handle->bgsurface->SetColor ( handle->bgsurface, 0x0, 0x0, 0x0, 0x0 );
	handle->bgsurface->FillRectangle ( handle->bgsurface, 0, 0, desc.width, desc.height );
	dfbret |= handle->bgsurface->Flip ( handle->bgsurface, NULL, 0 );
	if(dfbret != DFB_OK){
		printf("%s: Flip error\n", __FUNCTION__);
		return dfbret;
	}

printf("Test over!!!!!!!!!!!!\n");
	return dfbret;     
}

static int _find_used_windownumb (CSOSD_OBJ *handle, unsigned int startnumber)
{
	int	i;

	CSOSD_LOCK(handle->osd_mutex);
	
	if( 0 == handle->usednumb ){
		CSOSD_UNLOCK(handle->osd_mutex);
		return		-1;
	}

	for( i = startnumber; i < OBJECTS_NUMBER; i++ ){
		if( (handle->windows[i] != 0x0) && (handle->surfaces[i] != 0x0) ){
			CSOSD_UNLOCK(handle->osd_mutex);
			return		i;
		}
	}

	CSOSD_UNLOCK(handle->osd_mutex);
	return		-1;
}

static int _find_unused_windownumb (CSOSD_OBJ *handle)
{
	int	i;
	
	CSOSD_LOCK(handle->osd_mutex);
	
	if( OBJECTS_NUMBER == handle->usednumb ){
		CSOSD_UNLOCK(handle->osd_mutex);
		return		-1;
	}

	for( i = 0; i < OBJECTS_NUMBER; i++ ){
		if( (handle->windows[i] == 0x0) && (handle->surfaces[i] == 0x0) ){
			CSOSD_UNLOCK(handle->osd_mutex);
			return		i;
		}
	}

	CSOSD_UNLOCK(handle->osd_mutex);
	return		-1;
}

static inline long _myclock(void)
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static int _fadeinout_task(void *param)
{
	PIC_ID			id;
	CSOSD_OBJ *dev_obj;
	EXTRA_FUNC	func = NULL;
	DFBResult		dfbret;

	id = ((magic_task_params *)param)->id;
	func = ((magic_task_params *)param)->func;
	dev_obj = ((magic_task_params *)param)->p_obj;

	free(param);
	
	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) || (0 == dev_obj->magic_task[id]) ){
		printf("%s: DirectFB id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return -1;
	}

	while(1)
	{
		dfbret = dev_obj->windows[id]->SetOpacity( dev_obj->windows[id], (sin( _myclock()/300.0 ) * 85) + 170 );
		if(dfbret != DFB_OK){
			printf("%s: DirectFB SetOpacity error\n", __FUNCTION__);
			dev_obj->errno = OSD_ERROR_DFB_SET_OPACITY_WINDOW;
			return -1;
		}

		if(func)
			(*func)((CSOSD_HANDLE)dev_obj);

		usleep(50*1000);
	}	
}


CSOSD_HANDLE CSOSD_Open(CSOSD_LAYER layer)
{
	//DFBResult	dfbret;
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
		
		/*if(layer == 0){			
			dfbret = _directfb_init(&csosd_objs[layer]);
			if(dfbret != DFB_OK)
				return NULL;
		}*/
	}

	csosd_objs[layer].obj_type = CSOSD_OBJ_TYPE;
	osd_config_ornot = 0;

	return (CSOSD_HANDLE) & csosd_objs[layer];
}

CSAPI_RESULT CSOSD_OpenDFB(CSOSD_LAYER layer)
{
	DFBResult	dfbret;

	if (layer >= (DEV_NUMS-1))
		return CSAPI_FAILED;

	if(layer == 0){			
		dfbret = _directfb_init(&csosd_objs[layer]);
		if(dfbret != DFB_OK)
			return CSAPI_FAILED;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_Close(CSOSD_HANDLE handle)
{
	int	j;
	unsigned int i;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( dev_obj->dfb != NULL ){
		for( i = 0, j = -1; i < dev_obj->usednumb; ){
			j = _find_used_windownumb(handle, j+1);
			if( -1 == j )
				break;
			if( dev_obj->magic_task[j] ){
				pthread_cancel( dev_obj->magic_task[j] );
				dev_obj->magic_task[j] = 0x0;
			}
			dev_obj->surfaces[j]->Release( dev_obj->surfaces[j] );
     		dev_obj->windows[j]->Release( dev_obj->windows[j] );
			dev_obj->surfaces[j] = 0x0;
			dev_obj->windows[j] = 0x0;
			dev_obj->usednumb--;
		}
		dev_obj->bgsurface->Release( dev_obj->bgsurface );
	  	dev_obj->layer->Release( dev_obj->layer );
     	dev_obj->dfb->Release( dev_obj->dfb );
		dev_obj->bgsurface = 0x0;
	  	dev_obj->layer = 0x0;
     	dev_obj->dfb = 0x0;
	}

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
		modes_lst[config->mode].red.offset = 11;
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


CSAPI_RESULT CSOSD_GetSize(CSOSD_HANDLE handle, unsigned int *width, unsigned int *height)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == width) || (NULL == height) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	*width = dev_obj->scr_width;
	*height = dev_obj->scr_height;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_FillRectangle(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																			CSOSD_Color *color, PIC_ID *id)
{
	int numb;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == color) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;    
	if( (region->x == 0) && (region->y == 0) && (region->width == 0) && (region->height == 0)){
		desc.posx   = 0;
		desc.posy   = 0;
		desc.width  = dev_obj->scr_width;
		desc.height = dev_obj->scr_height;
	}
	else{
		desc.posx   = region->x;
		desc.posy   = region->y;
		desc.width  = region->width;
		desc.height = region->height;
	}
	//desc.caps = DWCAPS_ALPHACHANNEL;

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

   dev_obj->surfaces[numb]->SetColor (dev_obj->surfaces[numb], color->r, color->g, color->b, color->a);

	dfbret = dev_obj->surfaces[numb]->FillRectangle(dev_obj->surfaces[numb], 0, 0, region->width, region->height);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB FillRectangle error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_FILL_REC_TANGLE;
		return CSAPI_FAILED;
	}

	dev_obj->windows[numb]->SetOpacity( dev_obj->windows[numb], 0xFF );
	dev_obj->windows[numb]->RaiseToTop( dev_obj->windows[numb] );
	
	dfbret = dev_obj->surfaces[numb]->Flip(dev_obj->surfaces[numb], NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	*id = numb;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_FillTriangle(CSOSD_HANDLE handle, CSOSD_TRI_Region *region, CSOSD_Color *color, PIC_ID *id)
{
	int numb, x, y, width, height;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == color) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	x = MIN(MIN(region->x1,region->x2),region->x3);
	y = MIN(MIN(region->y1,region->y2),region->y3);
	width = MAX(MAX(region->x1,region->x2),region->x3) - x;
	height = MAX(MAX(region->y1,region->y2),region->y3) - y;
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;    
	desc.posx   = x;
	desc.posy   = y;
	desc.width  = width;
	desc.height = height;
	//desc.caps = DWCAPS_ALPHACHANNEL;

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

   dev_obj->surfaces[numb]->SetColor (dev_obj->surfaces[numb], color->r, color->g, color->b, color->a);

	dfbret = dev_obj->surfaces[numb]->FillTriangle(dev_obj->surfaces[numb], region->x1-x, region->y1-y, region->x2-x, region->y2-y, region->x3-x, region->y3-y);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB FillTriangle error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_FILL_TRI_ANGLE;
		return CSAPI_FAILED;
	}

	dev_obj->windows[numb]->SetOpacity( dev_obj->windows[numb], 0xFF );
	dev_obj->windows[numb]->RaiseToTop( dev_obj->windows[numb] );
	
	dfbret = dev_obj->surfaces[numb]->Flip(dev_obj->surfaces[numb], NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	*id = numb;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_DrawLine(CSOSD_HANDLE handle, CSOSD_LINE_Region *region, CSOSD_Color *color, PIC_ID *id)
{
	int numb, x, y, width, height;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == color) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	x = MIN(region->x1,region->x2);
	y = MIN(region->y1,region->y2);
	width = MAX(region->x1,region->x2) - x;
	height = MAX(region->y1,region->y2) - y;
	width = (width)? width:1;
	height = (height)? height:1;
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;    
	desc.posx   = x;
	desc.posy   = y;
	desc.width  = width;
	desc.height = height;
	//desc.caps = DWCAPS_ALPHACHANNEL;

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

   dev_obj->surfaces[numb]->SetColor (dev_obj->surfaces[numb], color->r, color->g, color->b, color->a);

	dfbret = dev_obj->surfaces[numb]->DrawLine(dev_obj->surfaces[numb], region->x1-x, region->y1-y, region->x2-x, region->y2-y);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB DrawLine error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DRAW_LINE;
		return CSAPI_FAILED;
	}

	dev_obj->windows[numb]->SetOpacity( dev_obj->windows[numb], 0xFF );
	dev_obj->windows[numb]->RaiseToTop( dev_obj->windows[numb] );
	
	dfbret = dev_obj->surfaces[numb]->Flip(dev_obj->surfaces[numb], NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	*id = numb;

	return CSAPI_SUCCEED;
}

/*CSAPI_RESULT CSOSD_DrawLines(CSOSD_HANDLE handle, CSOSD_LINE_Region **region, 
																		unsigned int lines_number, 	CSOSD_Color *color)
{
	DFBResult	dfbret;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	return CSAPI_SUCCEED;
}*/

CSAPI_RESULT CSOSD_DrawRectangle(CSOSD_HANDLE handle, CSOSD_REC_Region *region, CSOSD_Color *color, PIC_ID *id)
{
	int numb;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == color) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;    
	if( (region->x == 0) && (region->y == 0) && (region->width == 0) && (region->height == 0)){
		desc.posx   = 0;
		desc.posy   = 0;
		desc.width  = dev_obj->scr_width;
		desc.height = dev_obj->scr_height;
	}
	else{
		desc.posx   = region->x;
		desc.posy   = region->y;
		desc.width  = region->width;
		desc.height = region->height;
	}
	//desc.caps = DWCAPS_ALPHACHANNEL;

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

   dev_obj->surfaces[numb]->SetColor (dev_obj->surfaces[numb], color->r, color->g, color->b, color->a);

	dfbret = dev_obj->surfaces[numb]->DrawRectangle(dev_obj->surfaces[numb], 0, 0, region->width, region->height);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB DrawRectangle error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DRAW_RECTANGLE;
		return CSAPI_FAILED;
	}

	dev_obj->windows[numb]->SetOpacity( dev_obj->windows[numb], 0xFF );
	dev_obj->windows[numb]->RaiseToTop( dev_obj->windows[numb] );
	
	dfbret = dev_obj->surfaces[numb]->Flip(dev_obj->surfaces[numb], NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	*id = numb;
	
	return CSAPI_SUCCEED;
}

/*CSAPI_RESULT CSOSD_DrawHorizonDottedLine(CSOSD_HANDLE handle, CSOSD_LINE_Region *region, 
																							unsigned int realline_width, unsigned int dottedline_width, 
																							CSOSD_Color *color, PIC_ID *id)
{
	int i, numb, segmts, x, y, width, height, dx, ddx;
	DFBSpan *spans;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;
DFBSpan spansd[10];

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == color) || (NULL == id) || (region->y1 != region->y2) || \
		(0 == realline_width) || (0 == dottedline_width) || (region->x1 == region->x2) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	x = MIN(region->x1,region->x2);
	y = region->y1;
	width = MAX(region->x1,region->x2) - x;
	height = 1;
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	desc.flags |= DWDESC_CAPS;    
	desc.posx   = x;
	desc.posy   = y;
	desc.width  = width;
	desc.height = height;
	desc.caps = DWCAPS_ALPHACHANNEL;

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

   dev_obj->surfaces[numb]->SetColor (dev_obj->surfaces[numb], color->r, color->g, color->b, color->a);

	dx = abs(region->x2 - region->x1);
	ddx = realline_width + dottedline_width;
	segmts =  dx / ddx;
	
	if( dx - ddx*segmts){
		segmts += 1;
		spans = (DFBSpan *)malloc(sizeof(DFBSpan)*segmts);
		if( NULL == spans ){
			dev_obj->errno = OSD_ERROR_DFB_MALLOC_ERROR;
			return CSAPI_FAILED;
		}
		
		for( i = 0; i < (segmts-1); i++ ){
			spans[i].w = realline_width;
			spans[i].x = 0 + i*ddx;
		}
		spans[segmts-1].w = ((dx - ddx*segmts) > (int)realline_width)? (int)realline_width:(dx - ddx*(segmts-1));
		spans[segmts-1].x = 0 + i*ddx;
	}
	else{
		spans = (DFBSpan *)malloc(sizeof(DFBSpan)*segmts);
		if( NULL == spans ){
			dev_obj->errno = OSD_ERROR_DFB_MALLOC_ERROR;
			return CSAPI_FAILED;
		}
		
		for( i = 0; i < segmts; i++ ){
			spans[i].w = realline_width;
			spans[i].x = 0 + i*ddx;
		}
	}

	dfbret = dev_obj->surfaces[numb]->FillSpans (dev_obj->surfaces[numb], 0, spans, segmts);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB FillSpans error\n", __FUNCTION__);
		free(spans);
		dev_obj->errno = OSD_ERROR_FILL_SPANS;
		return CSAPI_FAILED;
	}

	free(spans);

	dev_obj->windows[numb]->SetOpacity( dev_obj->windows[numb], 0xFF );
	dev_obj->windows[numb]->RaiseToTop( dev_obj->windows[numb] );
	
	dfbret = dev_obj->surfaces[numb]->Flip(dev_obj->surfaces[numb], NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	*id = numb;

	return CSAPI_SUCCEED;
}*/

/*CSAPI_RESULT CSOSD_SetBackgroundImage(CSOSD_HANDLE handle, unsigned char *file_path)
{
	DFBResult	dfbret;
	IDirectFBImageProvider *provider;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	dfbret = dev_obj->dfb->CreateImageProvider( dev_obj->dfb, file_path, &provider );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB SetBackgroundImage error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_IMAGE;
		return CSAPI_FAILED;
	}

	dfbret = provider->RenderTo( provider, dev_obj->bgsurface, NULL );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB RenderTo error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_RENDERTO_ERROR;
		return CSAPI_FAILED;
	}
	provider->Release( provider );

    dfbret = dev_obj->layer->SetBackgroundImage( dev_obj->layer, dev_obj->bgsurface );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB SetBackgroundImage error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_SET_BACKGROUND_ERROR;
		return CSAPI_FAILED;
	}
    dev_obj->layer->SetBackgroundMode( dev_obj->layer, DLBM_IMAGE );
	
	return CSAPI_SUCCEED;
	
}*/

CSAPI_RESULT CSOSD_SetBackgroundColor(CSOSD_HANDLE handle, CSOSD_Color *color)
{
	DFBResult	dfbret;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( 0x0 == dev_obj->bgsurface){
		printf("%s: DirectFB parameters error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;		
	}

	if( NULL == color ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	dev_obj->bgsurface->SetColor (dev_obj->bgsurface, color->r, color->g, color->b, color->a);

	dfbret = dev_obj->bgsurface->FillRectangle(dev_obj->bgsurface, 0, 0, dev_obj->scr_width, dev_obj->scr_height);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB FillRectangle error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_FILL_REC_TANGLE;
		return CSAPI_FAILED;
	}

	dfbret = dev_obj->bgsurface->Flip(dev_obj->bgsurface, NULL, DSFLIP_WAITFORSYNC);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_OpenImage(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																			unsigned char *path, PIC_ID *id)
{
	int numb;
	DFBResult	dfbret;
	DFBWindowDescription  desc;
	DFBSurfaceDescription	sdes;
	IDirectFBImageProvider *provider;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (NULL == region) || (NULL == path) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	if( (region->x == 0) && (region->y == 0) && (region->width == 0) && (region->height == 0)){
		desc.posx   = 0;
		desc.posy   = 0;
		desc.width  = dev_obj->scr_width;
		desc.height = dev_obj->scr_height;
	}
	else{
		desc.posx   = region->x;
		desc.posy   = region->y;
		desc.width  = region->width;
		desc.height = region->height;
	}
	
	dfbret = dev_obj->dfb->CreateImageProvider( dev_obj->dfb, path, &provider );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateImageProvider error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_IMAGE;
		return CSAPI_FAILED;
	}
	
	provider->GetSurfaceDescription ( provider, &sdes );
	if( sdes.pixelformat == DSPF_RGB16 ){
		desc.flags |= DWDESC_PIXELFORMAT;
		desc.pixelformat = DSPF_RGB16;
	}
	if( sdes.pixelformat == DSPF_RGB32 ){
		desc.flags |= DWDESC_PIXELFORMAT;
		desc.pixelformat = DSPF_RGB32;
	}
	if( sdes.pixelformat == DSPF_ARGB ){
		desc.flags |= DWDESC_PIXELFORMAT/* | DWDESC_CAPS*/; 
		desc.pixelformat = DSPF_ARGB;
		//desc.caps = DWCAPS_ALPHACHANNEL;
	}

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

	/*{
		DFBImageDescription ides;
		DFBSurfaceDescription sdes;
		DFBWindowOptions woption;
		DFBSurfacePixelFormat	pixelformat;
		provider->GetImageDescription ( provider, &ides );
		printf("[Image-Description] caps: 0x%x   colorkey_r:%d  colorkey_g:%d  colorkey_b:%d\n", ides.caps, ides.colorkey_r, ides.colorkey_g, ides.colorkey_b);
		provider->GetSurfaceDescription ( provider, &sdes );
		printf("[Surface-Should-Description] caps: 0x%x   flags: 0x%x   height: %d   width: %d   pixelformat: 0x%x(DSPF_RGB16:0x%x  DSPF_RGB32:0x%x  DSPF_ARGB:0x%x)   preallocated.data: 0x%x   preallocated.pitch: %d\n", sdes.caps, sdes.flags, sdes.height, sdes.width, sdes.pixelformat, DSPF_RGB16, DSPF_RGB32, DSPF_ARGB, sdes.preallocated[0].data, sdes.preallocated[0].pitch);
		dev_obj->windows[numb]->GetOptions ( dev_obj->windows[numb], &woption );
		printf("[Window-Options] : 0x%x\n", woption);
		dev_obj->surfaces[numb]->GetPixelFormat  ( dev_obj->surfaces[numb], &pixelformat );
		printf("[Surface-Pixelformat] : 0x%x\n", pixelformat);
		dev_obj->bgsurface->GetPixelFormat  ( dev_obj->bgsurface, &pixelformat );
		printf("[bgsurface-Pixelformat] : 0x%x\n", pixelformat);
	}*/
	
	dfbret = provider->RenderTo( provider, dev_obj->surfaces[numb], NULL );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB RenderTo error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_RENDERTO_ERROR;
		return CSAPI_FAILED;
	}
	provider->Release( provider );

	*id = numb;

	return CSAPI_SUCCEED;

}

CSAPI_RESULT CSOSD_OpenImageBuffer(CSOSD_HANDLE handle, CSOSD_REC_Region *region, 
																				CSOSD_COLOR_FORMAT format, unsigned char *data, PIC_ID *id)
{	
	DFBResult	dfbret;
	void *point = NULL;
	DFBWindowDescription  desc;
	int i, pitch, numb, width, height;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);	

	if( (NULL == region) || (NULL == data) || (NULL == id) ){
		printf("%s: parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	/*if( format != OSD_COLOR_FORMAT_RGB565 ){
		printf("%s: only support RGB565 for now\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}*/
	
	desc.flags = ( DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT );
	
	switch(format){

		case	OSD_COLOR_FORMAT_RGB565:
			desc.flags |= DWDESC_PIXELFORMAT;
			desc.pixelformat = DSPF_RGB16;
			break;

		case	OSD_COLOR_FORMAT_ARGB4444:
			desc.flags |= DWDESC_PIXELFORMAT/* | DWDESC_CAPS*/;
			//desc.caps   = DWCAPS_ALPHACHANNEL;
			desc.pixelformat = DSPF_ARGB4444;
			break;
			
		case	OSD_COLOR_FORMAT_ARGB1555:
			desc.flags |= DWDESC_PIXELFORMAT/* | DWDESC_CAPS*/;
			//desc.caps   = DWCAPS_ALPHACHANNEL;
			desc.pixelformat = DSPF_ARGB1555;
			break;

		#if defined(ARCH_CSM1201)
		case	OSD_COLOR_FORMAT_ARGB8888:
			desc.flags |= DWDESC_PIXELFORMAT/* | DWDESC_CAPS*/;
			//desc.caps   = DWCAPS_ALPHACHANNEL;
			desc.pixelformat = DSPF_ARGB;
			break;
		#endif

		default:	
			printf("%s: format parameter error\n", __FUNCTION__);
			dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
			return CSAPI_FAILED;
			break;

	}
	
	if( (region->x == 0) && (region->y == 0) && (region->width == 0) && (region->height == 0)){
		desc.posx   = 0;
		desc.posy   = 0;
		desc.width  = dev_obj->scr_width;
		desc.height = dev_obj->scr_height;
	}
	else{
		desc.posx   = region->x;
		desc.posy   = region->y;
		desc.width  = region->width;
		desc.height = region->height;
	}

	numb = _find_unused_windownumb(dev_obj);
	if(numb == -1){
		printf("%s: find unused window error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_UNUSED_WINDOW;
		return CSAPI_FAILED;
	}
	dev_obj->usednumb++;
	
    dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &dev_obj->windows[numb]);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		dev_obj->windows[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}
    dfbret = dev_obj->windows[numb]->GetSurface( dev_obj->windows[numb], &dev_obj->surfaces[numb] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_GET_SURFACE;
		dev_obj->surfaces[numb] = 0x0;
		dev_obj->usednumb--;
		return CSAPI_FAILED;
	}

	dev_obj->surfaces[numb]->GetSize(dev_obj->surfaces[numb], &width, &height);

	dev_obj->surfaces[numb]->Lock(dev_obj->surfaces[numb],DSLF_WRITE, &point,&pitch);
	//printf("point: 0x%x  pitch: %d\n", point, pitch);
	for(i=0; i < desc.height; i++){
		memcpy((unsigned char *)point + (pitch*i), data + (pitch*i), pitch);
	} 
	//printf("memcpy over, ready to unlock....\n");getchar();
	dev_obj->surfaces[numb]->Unlock( dev_obj->surfaces[numb] );

	*id = numb;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ShowImage(CSOSD_HANDLE handle, PIC_ID id)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	dev_obj->surfaces[id]->Flip( dev_obj->surfaces[id], NULL, 0 );
	dev_obj->windows[id]->SetOpacity( dev_obj->windows[id], 0xFF );
	dev_obj->windows[id]->RaiseToTop( dev_obj->windows[id] );

	//dev_obj->bgsurface->Dump(dev_obj->bgsurface, "/mnt", "backsurface");
	//dev_obj->surfaces[id]->Dump(dev_obj->surfaces[id], "/tmp", "2surface");

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ShowImageRoll(CSOSD_HANDLE handle, PIC_ID id, CSOSD_ROLL_DIRECTION direction,
																					unsigned int degree, EXTRA_FUNC func)
{
	unsigned int					i;
	DFBResult					dfbret;
	DFBRectangle    			rectangle;
	IDirectFBSurface       *tempsurface;
	IDirectFBWindow			*tempwindow;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	if( (0 == degree) || (direction!=OSD_DOWN) ){
//		((direction!=OSD_LEFT)&&(direction!=OSD_RIGHT)&&(direction!=OSD_UP)&&(direction!=OSD_DOWN)) ){
			printf("%s: DirectFB parameters error\n", __FUNCTION__);
			dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
			return CSAPI_FAILED;		
	}

	desc.flags = ( DWDESC_POSX | DWDESC_POSY |DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;               
	//desc.caps = DWCAPS_ALPHACHANNEL;
	desc.posx   = 0;
	desc.posy   = 0;
	dev_obj->windows[id]->GetSize( dev_obj->windows[id], &desc.width, &desc.height );

	dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &tempwindow );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		return CSAPI_FAILED;
	}
	
	dfbret = tempwindow->GetSurface( tempwindow, &tempsurface );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_SURFACE;
		return CSAPI_FAILED;
	}

	dfbret = tempsurface->Blit(tempsurface, dev_obj->surfaces[id], NULL, 0, 0);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Blit error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_BLIT_ERROR;
		return CSAPI_FAILED;
	}

	dfbret = dev_obj->surfaces[id]->Clear(dev_obj->surfaces[id], 0, 0, 0, 0x0);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Clear error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CLEAR_ERROR;
		return CSAPI_FAILED;
	}
	dev_obj->windows[id]->SetOpacity( dev_obj->windows[id], 0xFF );
	dev_obj->windows[id]->RaiseToTop( dev_obj->windows[id] );

	switch(direction){

		case OSD_DOWN:

			if( (int)degree > desc.height ){
				printf("%s: DirectFB degree is too large\n", __FUNCTION__);
				dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
				return CSAPI_FAILED;		
			}
			
			rectangle.x = 0;
			rectangle.y = 0;
			rectangle.w = desc.width;		
			for( i=0; i < degree; i++){
				rectangle.h = desc.height/degree*(i+1);									
				dev_obj->surfaces[id]->Blit(dev_obj->surfaces[id], tempsurface, &rectangle, 0, desc.height-(desc.height/degree*(i+1)));
				dev_obj->surfaces[id]->Flip( dev_obj->surfaces[id], NULL, 0 );
				if(func)
					(*func)((CSOSD_HANDLE)dev_obj);
			}

			break;

		case OSD_UP:
			break;
		case OSD_LEFT:
			break;
		case OSD_RIGHT:
			break;

		default:	break;
	}
	
	tempsurface->Release( tempsurface );
	tempwindow->Release( tempwindow );
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_SetObjectAlpha(CSOSD_HANDLE handle, PIC_ID id, unsigned char alpha)
{
	DFBResult					dfbret;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	dfbret = dev_obj->windows[id]->SetOpacity( dev_obj->windows[id], alpha );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB SetOpacity error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_SET_OPACITY_WINDOW;
		return CSAPI_FAILED;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectKeyColor(CSOSD_HANDLE handle, PIC_ID id, CSOSD_Color *color)
{
	DFBResult					dfbret;
	IDirectFBWindow			*tempwindow;
	IDirectFBSurface			*tempsurface;
	DFBWindowDescription  desc;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	desc.flags = ( DWDESC_POSX | DWDESC_POSY |DWDESC_WIDTH | DWDESC_HEIGHT );
	//desc.flags |= DWDESC_CAPS;               
	//desc.caps = DWCAPS_ALPHACHANNEL;
	desc.posx   = 0;
	desc.posy   = 0;
	dev_obj->windows[id]->GetSize( dev_obj->windows[id], &desc.width, &desc.height );

	dfbret = dev_obj->layer->CreateWindow( dev_obj->layer, &desc, &tempwindow );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB CreateWindow error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_WINDOW;
		return CSAPI_FAILED;
	}
	
	dfbret = tempwindow->GetSurface( tempwindow, &tempsurface );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB GetSurface error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_CREATE_SURFACE;
		return CSAPI_FAILED;
	}
	
	tempsurface->SetBlittingFlags( tempsurface, DSBLIT_SRC_COLORKEY );
	dev_obj->surfaces[id]->SetBlittingFlags( dev_obj->surfaces[id], DSBLIT_SRC_COLORKEY );
	tempsurface->SetSrcColorKey( tempsurface, color->r, color->g, color->b );
	dev_obj->surfaces[id]->SetSrcColorKey( dev_obj->surfaces[id], color->r, color->g, color->b );
	
	dfbret = tempsurface->Blit(tempsurface, dev_obj->surfaces[id], NULL, 0, 0);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Blit error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_BLIT_ERROR;
		return CSAPI_FAILED;
	}
	
	dfbret = dev_obj->surfaces[id]->Blit(dev_obj->surfaces[id], tempsurface, NULL, 0, 0);
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Blit error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_BLIT_ERROR;
		return CSAPI_FAILED;
	}
	
	dfbret = dev_obj->surfaces[id]->Flip( dev_obj->surfaces[id], NULL, DSFLIP_WAITFORSYNC );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Flip error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_FLIP;
		return CSAPI_FAILED;
	}
	
	tempsurface->Release( tempsurface );
	tempwindow->Release( tempwindow );

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectRaiseToTop(CSOSD_HANDLE handle, PIC_ID id)
{
	DFBResult					dfbret;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB ObjectRaiseToTop id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	dfbret = dev_obj->windows[id]->RaiseToTop( dev_obj->windows[id] );
	if(dfbret != DFB_OK){
		printf("%s: DirectFB Object Raise To Top error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_RAISE_TO_TOP_ERROR;
		return CSAPI_FAILED;
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectMove(CSOSD_HANDLE handle, int dx, int dy, CSOSD_MOVE_MODE mode, PIC_ID id)
{
	//int x, y;
	DFBResult		dfbret;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB ObjectMove id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	//dev_obj->windows[id]->GetPosition( dev_obj->windows[id], &x, &y );

	if( OSD_DISTANCE == mode ){
		dfbret = dev_obj->windows[id]->Move( dev_obj->windows[id], dx, dy );
		if(dfbret != DFB_OK){
			printf("%s: DirectFB Move error\n", __FUNCTION__);
			dev_obj->errno = OSD_ERROR_DFB_MOVE_ERROR;
			return CSAPI_FAILED;
		}
		return CSAPI_SUCCEED;
	}

	if( OSD_COORDINATE == mode ){
		dfbret = dev_obj->windows[id]->MoveTo( dev_obj->windows[id], dx, dy );
		if(dfbret != DFB_OK){
			printf("%s: DirectFB MoveTo error\n", __FUNCTION__);
			dev_obj->errno = OSD_ERROR_DFB_MOVE_ERROR;
			return CSAPI_FAILED;
		}
		return CSAPI_SUCCEED;
	}

	dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
	return CSAPI_FAILED;
}

CSAPI_RESULT CSOSD_ObjectFadeInOut(CSOSD_HANDLE handle, PIC_ID id, EXTRA_FUNC func)
{
	int	ret;
	void *parameter;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) || (dev_obj->magic_task[id] != 0) ){
		printf("%s: DirectFB FadeInOut id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	parameter = (void *)malloc( sizeof(magic_task_params) );
	if( NULL == parameter ){
		dev_obj->errno = OSD_ERROR_DFB_MALLOC_ERROR;
		return CSAPI_FAILED;
	}
	((magic_task_params *)parameter)->p_obj = dev_obj;
	((magic_task_params *)parameter)->id = id;
	((magic_task_params *)parameter)->func = func;
	
	ret = pthread_create(&dev_obj->magic_task[id], NULL, (void *) _fadeinout_task, parameter);
	if( ret != 0 ){
		free(parameter);
		dev_obj->errno = OSD_ERROR_DFB_PTHREAD_ERROR;
		return CSAPI_FAILED;
	}
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectFadeInOut_Stop(CSOSD_HANDLE handle, PIC_ID id)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) || ( 0 == dev_obj->magic_task[id]) ){
		printf("%s: DirectFB Object FadeInOut_Stop id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}

	pthread_cancel( dev_obj->magic_task[id] );
	dev_obj->magic_task[id] = 0x0;
	
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectClear(CSOSD_HANDLE handle, PIC_ID id)
{
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);

	if( (0 == dev_obj->surfaces[id]) || (0 == dev_obj->windows[id]) ){
		printf("%s: DirectFB ObjectClear id error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_DFB_IMAGE_ID_ERROR;
		return CSAPI_FAILED;
	}
	
	dev_obj->windows[id]->SetOpacity( dev_obj->windows[id], 0x00 );
	dev_obj->windows[id]->LowerToBottom( dev_obj->windows[id] );

	dev_obj->surfaces[id]->Release(dev_obj->surfaces[id]);
	dev_obj->windows[id]->Release(dev_obj->windows[id]);

	dev_obj->surfaces[id] = 0;
	dev_obj->windows[id] = 0;
	dev_obj->usednumb--;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSOSD_ObjectClearAll(CSOSD_HANDLE handle)
{
	int	j;
	unsigned int i;
	CSOSD_OBJ *dev_obj = (CSOSD_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSOSD_OBJ_TYPE);
	
	if( dev_obj->dfb != NULL ){
		for( i = 0, j = -1; i < dev_obj->usednumb; ){
			j = _find_used_windownumb(handle, j+1);
			if( -1 == j )
				break;
			if( dev_obj->magic_task[j] ){
				pthread_cancel( dev_obj->magic_task[j] );
				dev_obj->magic_task[j] = 0x0;
			}
			dev_obj->windows[j]->SetOpacity( dev_obj->windows[j], 0x00 );
			dev_obj->windows[j]->LowerToBottom( dev_obj->windows[j] );
			dev_obj->surfaces[j]->Release( dev_obj->surfaces[j] );
     		dev_obj->windows[j]->Release( dev_obj->windows[j] );
			dev_obj->surfaces[j] = 0x0;
			dev_obj->windows[j] = 0x0;
			dev_obj->usednumb--;
		}
	}
	else{
		printf("%s: DirectFB ObjectClearAll parameter error\n", __FUNCTION__);
		dev_obj->errno = OSD_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

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
