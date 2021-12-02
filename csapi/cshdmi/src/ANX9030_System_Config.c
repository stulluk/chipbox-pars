
#include "ANX9030_System_Config.h"
#include "ANX9030_Sys9030.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/fb.h>


extern BYTE anx9030_avi_data[19];
extern  BYTE enable_invert_field_polarity ;
int isInterlace;

void ANX9030_System_Config()
{
	unsigned int current_format;
    debug_puts("Debug: In System Config!\n");

    current_format = HDMI_GetTimingMode();
    debug_printf("ANX9030_System_Config() Debug: current_format = %d\n",current_format );
    //  getchar();
    HDMI_SetTimingMode((BYTE)current_format);
    //ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
    ANX9030_System_Video_Config();
    ANX9030_System_Packets_Config();
    ANX9030_System_Audio_Config();
}

void ANX9030_System_Video_Config()
{
	 debug_puts("Set System Video is not be used!");

	 	 
}

void ANX9030_System_Packets_Config()
{
	printf("In ANX9030_System_Packets_config \n");
	if(s_anx9030_packet_config.packets_need_config & ANX9030_avi_sel)
		ANX9030_AVI_Config(0x31,source_ratio,null,null,null,null,null,null,null,null,null,null,null);

    if(s_anx9030_packet_config.packets_need_config & ANX9030_audio_sel)
        ANX9030_AUD_INFO_Config(null,null,null,null,null,null,null,null,null,null);

	ANX9030_AUD_CHStatus_Config(null,null,null,null,null,null,0x01,null,null, 0x02);
}

void ANX9030_System_Audio_Config()
{
    

}

unsigned int HDMI_GetTimingMode()	
{
	int fb_fdes =0;
	struct fb_var_screeninfo format;


	fb_fdes = open("/dev/fb/0", O_RDONLY);
	if(fb_fdes < 0)
	{
		printf("HDMI OPEN FB ERROR:\n");
		fb_fdes =0;
		return 0;
	}

	ioctl(fb_fdes, FBIOGET_VSCREENINFO, &format);
	
	debug_printf("x=%d,y=%d,vmode=%d\n",(format.left_margin + format.xres + format.right_margin),(format.upper_margin + format.yres + format.lower_margin),(format.vmode & FB_VMODE_INTERLACED));
	if(
	   (format.left_margin + format.xres + format.right_margin)   == 720 &&
	   (format.upper_margin + format.yres + format.lower_margin) == 576 ) {
	  	if(format.vmode & FB_VMODE_INTERLACED){
            close(fb_fdes);
			return anx9030_V720x576i_50Hz_4x3;
        }
        else{
            close(fb_fdes);
			return anx9030_V720x576p_50Hz_4x3;
		}
	} else if(
	   (format.left_margin + format.xres + format.right_margin)   == 720 &&
           (format.upper_margin + format.yres + format.lower_margin) == 480 ) {
		if(format.vmode & FB_VMODE_INTERLACED){
			close(fb_fdes);
            return anx9030_V720x480i_60Hz_4x3;
        }
		else{
            close(fb_fdes);
			return anx9030_V720x480p_60Hz_4x3;
        }
	} else if(
	   (format.left_margin + format.xres + format.right_margin)   == 1280 &&
	   (format.upper_margin + format.yres + format.lower_margin) ==  720 &&
           !(format.vmode & FB_VMODE_INTERLACED)) {
		if(format.hsync_len == 700){
            close(fb_fdes);
			return anx9030_V1280x720p_50Hz;
        }
		else{
            close(fb_fdes);
			return anx9030_V1280x720p_60Hz;
        }
	} else if(
	   (format.left_margin + format.xres + format.right_margin)   == 1920 &&
	   (format.upper_margin + format.yres + format.lower_margin) == 1080 &&
           format.vmode & FB_VMODE_INTERLACED) {
		if(format.hsync_len == 720){
            close(fb_fdes);
			return anx9030_V1920x1080i_50Hz;
        }
		else{
            close(fb_fdes);
			return anx9030_V1920x1080i_60Hz;
        }
    }   
    else if(
            (format.left_margin + format.xres + format.right_margin)   == 1920 &&
            (format.upper_margin + format.yres + format.lower_margin) == 1080 &&
            !(format.vmode & FB_VMODE_INTERLACED)){ 
        printf("format.hsync_len =%d \n",format.hsync_len);
		if(format.hsync_len == 830){
            close(fb_fdes);
			return anx9030_V1920x1080p_24Hz;
        }
		else if (format.hsync_len == 720){
            close(fb_fdes);
			return anx9030_V1920x1080p_25Hz;
        }
		else if (format.hsync_len == 280){
            close(fb_fdes);
			return anx9030_V1920x1080p_30Hz;
        }

    } 
	close(fb_fdes);
	return 0;
}


void HDMI_SetTimingMode(BYTE VID_FORMAT)
{

	BYTE video_id;
	BYTE input_pixel_rpt_time;

	enable_invert_field_polarity = 0;
	isInterlace = 1;

	video_id = VID_FORMAT;
	switch(video_id)
	{
		case anx9030_V720x576i_50Hz_4x3:
			anx9030_video_format_config = anx9030_YCMux422_EmbSync_Mode1;

			//Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_PAL(I576)\r\n");
			break;

		case anx9030_V720x480i_60Hz_4x3:
			anx9030_video_format_config = anx9030_YCMux422_EmbSync_Mode1;

			//	Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_NTSC\r\n");
			break;


		case anx9030_V720x480p_60Hz_4x3:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;
			input_pixel_rpt_time = 0;

#ifdef ARCH_CSM1200			
            enable_invert_field_polarity = 1;
#endif			
            debug_printf("Input Formate : VIDEO_DISPLAY_P480\r\n");
			isInterlace = 0;
			break;

		case anx9030_V720x576p_50Hz_4x3:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;
			input_pixel_rpt_time = 0;

#ifdef ARCH_CSM1200			
			enable_invert_field_polarity = 1;
#endif
			isInterlace = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_P576\r\n");
			break;


		case anx9030_V1280x720p_50Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;
			input_pixel_rpt_time = 0;

#ifdef ARCH_CSM1200						
            enable_invert_field_polarity = 1;
#endif
			isInterlace = 0;

			debug_printf("Input Formate : VIDEO_DISPLAY_P720\r\n");
			break;

		case anx9030_V1280x720p_60Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;
			input_pixel_rpt_time = 0;
			
#ifdef ARCH_CSM1200			
            enable_invert_field_polarity = 1;
#endif

			isInterlace = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_P720_30\r\n");
			break;

		case anx9030_V1920x1080i_50Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_I1080_25\r\n");
			break;

		case anx9030_V1920x1080i_60Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;

			//	Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_I1080\r\n");
			break;
		case anx9030_V1920x1080p_24Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;

#ifdef ARCH_CSM1200			
			enable_invert_field_polarity = 1;
#endif

			//	Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_P1080_24Hz\r\n");
			break;
		case anx9030_V1920x1080p_25Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;

#ifdef ARCH_CSM1200			
			enable_invert_field_polarity = 1;
#endif

			//	Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_P1080_25Hz\r\n");
			break;
		case anx9030_V1920x1080p_30Hz:
			anx9030_video_format_config = anx9030_YCrCb422_EmbSync;

#ifdef ARCH_CSM1200			
			enable_invert_field_polarity = 1;
#endif

			//	Select video time mode
			input_pixel_rpt_time = 0;
			debug_printf("Input Formate : VIDEO_DISPLAY_P1080_30Hz\r\n");
			break;

		default:
			anx9030_video_format_config = anx9030_YCMux422_EmbSync_Mode1;

			//Select video time mode
			video_id = anx9030_V720x576i_50Hz_4x3;
			input_pixel_rpt_time = 0;
			debug_printf("Default Input Formate : VIDEO_DISPLAY_PAL\r\n");
			break;
	}

	anx9030_video_timing_id = video_id;
    anx9030_in_pix_rpt = input_pixel_rpt_time;
	s_anx9030_packet_config.packets_need_config= ANX9030_avi_sel | ANX9030_audio_sel;
    //anx9030_system_config_done = 0;
    usleep(100000);
}

    
void ANX9030_AUD_CHStatus_Config(BYTE MODE,BYTE PCM_MODE,BYTE SW_CPRGT,BYTE NON_PCM,
    BYTE PROF_APP,BYTE CAT_CODE,BYTE CH_NUM,BYTE SOURCE_NUM,BYTE CLK_ACCUR,BYTE Fs)
{
    //MODE: 0x00 = PCM Audio
    //PCM_MODE: 0x00 = 2 audio channels without pre-emphasis;
                                //0x01 = 2 audio channels with 50/15 usec pre-emphasis;
    //SW_CPRGT: 0x00 = copyright is asserted;
                                // 0x01 = copyright is not asserted;
    //NON_PCM: 0x00 = Represents linear PCM
                            //0x01 = For other purposes
    //PROF_APP: 0x00 = consumer applications;
                              // 0x01 = professional applications;

    //CAT_CODE: Category code
    //CH_NUM: 0x00 = Do not take into account
                           // 0x01 = left channel for stereo channel format
                           // 0x02 = right channel for stereo channel format
    //SOURCE_NUM: source number
                                   // 0x00 = Do not take into account
                                  // 0x01 = 1; 0x02 = 2; 0x03 = 3
    //CLK_ACCUR: 0x00 = level II
                                   // 0x01 = level I
                                   // 0x02 = level III
                                   // else reserved;

    s_anx9030_audio_config.i2s_config.Channel_status1 = (MODE << 7) | (PCM_MODE << 5) |
        (SW_CPRGT << 2) | (NON_PCM << 1) | PROF_APP;
    s_anx9030_audio_config.i2s_config.Channel_status2 = CAT_CODE;
    s_anx9030_audio_config.i2s_config.Channel_status3 = (CH_NUM << 7) | SOURCE_NUM;
    s_anx9030_audio_config.i2s_config.Channel_status4 = (CLK_ACCUR << 5) | Fs;
}


/***************************************************
 *Function:ANX9030_API_AVI_Config
 ***************************************************/
void ANX9030_AVI_Config(BYTE pb1,BYTE pb2,BYTE pb3,BYTE pb4,BYTE pb5,
    BYTE pb6,BYTE pb7,BYTE pb8,BYTE pb9,BYTE pb10,BYTE pb11,BYTE pb12,BYTE pb13)
{
    s_anx9030_packet_config.avi_info.pb_byte[1] = pb1;
    s_anx9030_packet_config.avi_info.pb_byte[2] = pb2;
    s_anx9030_packet_config.avi_info.pb_byte[3] = pb3;
    s_anx9030_packet_config.avi_info.pb_byte[4] = pb4;
    s_anx9030_packet_config.avi_info.pb_byte[5] = pb5;
    s_anx9030_packet_config.avi_info.pb_byte[6] = pb6;
    s_anx9030_packet_config.avi_info.pb_byte[7] = pb7;
    s_anx9030_packet_config.avi_info.pb_byte[8] = pb8;
    s_anx9030_packet_config.avi_info.pb_byte[9] = pb9;
    s_anx9030_packet_config.avi_info.pb_byte[10] = pb10;
    s_anx9030_packet_config.avi_info.pb_byte[11] = pb11;
    s_anx9030_packet_config.avi_info.pb_byte[12] = pb12;
    s_anx9030_packet_config.avi_info.pb_byte[13] = pb13;
}


/***************************************************
 *Function:ANX9030_API_AUD_INFO_Config
 ***************************************************/
void ANX9030_AUD_INFO_Config(BYTE pb1,BYTE pb2,BYTE pb3,BYTE pb4,BYTE pb5,
    BYTE pb6,BYTE pb7,BYTE pb8,BYTE pb9,BYTE pb10)
{
    s_anx9030_packet_config.audio_info.pb_byte[1] = pb1;
    s_anx9030_packet_config.audio_info.pb_byte[2] = pb2;
    s_anx9030_packet_config.audio_info.pb_byte[3] = pb3;
    s_anx9030_packet_config.audio_info.pb_byte[4] = pb4;
    s_anx9030_packet_config.audio_info.pb_byte[5] = pb5;
    s_anx9030_packet_config.audio_info.pb_byte[6] = pb6;
    s_anx9030_packet_config.audio_info.pb_byte[7] = pb7;
    s_anx9030_packet_config.audio_info.pb_byte[8] = pb8;
    s_anx9030_packet_config.audio_info.pb_byte[9] = pb9;
    s_anx9030_packet_config.audio_info.pb_byte[10] = pb10;
}

/***************************************************
 *Function:ANX9030_API_HoldSystemConfig
 ***************************************************/
void ANX9030_HoldSystemConfig(BIT bHold_ANX9030)
{
	anx9030_app_hold_system_config = bHold_ANX9030;
}


/***************************************************
 *Function:ANX9030_API_ShutDown
 ***************************************************/
void ANX9030_ShutDown(BIT bShutDown_ANX9030)
{
    anx9030_shutdown = bShutDown_ANX9030;
}


/***************************************************
 *Function:ANX9030_API_HDCP_ONorOFF
 ***************************************************/
void ANX9030_HDCP_ONorOFF(BIT HDCP_ONorOFF)
{

    anx9030_HDCP_enable = HDCP_ONorOFF;// 1: on;  0:off
}

