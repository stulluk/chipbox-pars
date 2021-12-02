#ifndef __SYS_SETUP_H
#define __SYS_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif


//extern FLASH_Handle_t 	Flash_handle;
//extern FEND_Handle_t 	FEND_handle;
//extern BLIT_Handle_t	BLIT_handle;

extern U32 g_screen_width;
extern U32 g_screen_height;

typedef enum
{
	CS_SYS_XPORT_HANDLE=0,
	CS_SYS_BLIT_HANDLE,
	CS_SYS_DISP_HANDLE,
	CS_SYS_FLASH_HANDLE,
	CS_SYS_E2P_HANDLE
}eSysHandleType;

typedef struct CS_BSPConfig_t_
{
    unsigned char in_file[200];
    unsigned int  app_type;
    unsigned char net_addr[50];
    unsigned int  port;   
   
    unsigned int  video_codec_type;
    unsigned int  audio_codec_type;
    
    unsigned int  tuner_frequency;
    unsigned int  tuner_symbol_rate;
    unsigned int  tuner_mod;

    unsigned int  display_mode; 
    unsigned int  vid0_z_order;
    unsigned int  vid1_z_order;
    unsigned int  gfx0_z_order;
    unsigned int  gfx1_z_order;
    unsigned int  df0_x_start;
    unsigned int  df0_x_end;
    unsigned int  df0_y_start;
    unsigned int  df0_y_end;
    unsigned int  df0_cropx_start;
    unsigned int  df0_cropx_size;
    unsigned int  df0_cropy_start;
    unsigned int  df0_cropy_size;
    unsigned int  df0_alpha;
    unsigned int  gfx1_ena;
    unsigned int  gfx2_ena;

    unsigned int  parser_pid;

    unsigned int  def_video_pid; 
    unsigned int  def_audio_pid;
    unsigned int  def_pcr_pid; 
   
} CS_BSPConfig_t;
int CS_DRV_Init(void);
int CS_DRV_Open(void);
BOOL CS_SYS_Init(void);
U32 CS_SYS_GetHandle(eSysHandleType type);


#ifdef __cplusplus
}
#endif

#endif 


