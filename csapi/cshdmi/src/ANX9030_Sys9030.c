/*  HDMI ANX9030 Linux driver
 *   Copyright @ Celestial 2007
 *  
 * drivered by:
 *  ANALOGIX Company 
 *  ANX9030 Demo Firmware on SST
 *  Version 1.02	2006/11/06
 */

#include "hdmi_i2c.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "ANX9030_Sys9030.h"
#include "ANX9030_System_Config.h"
#include "hdmi_gpio.h"

//#ifdef ITU656
struct anx9030_video_timingtype anx9030_video_timingtype_table = 
{
    //640x480p-60hz
    {0x20/*H_RES_LOW*/, 0x03/*H_RES_HIGH*/,0x80 /*ACT_PIX_LOW*/,0x02 /*ACT_PIX_HIGH*/, 
     0x60/*HSYNC_WIDTH_LOW*/,0x00 /*HSYNC_WIDTH_HIGH*/,0x30 /*H_BP_LOW*/,0x00 /*H_BP_HIGH*/, 
     0xe0/*ACT_LINE_LOW*/, 0x01/*ACT_LINE_HIGH*/,0x02 /*VSYNC_WIDTH*/, 0x21/*V_BP_LINE*/, 
     0x0a/*V_FP_LINE*/,0x10 /*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Neg_Hsync_pol, anx9030_Neg_Vsync_pol},
    //720x480p-60hz
    {0x5a/*H_RES_LOW*/,0x03 /*H_RES_HIGH*/,0xd0/*ACT_PIX_LOW*/, 0x02/*ACT_PIX_HIGH*/, 
     0x3e/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x3c/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0xe0/*ACT_LINE_LOW*/, 0x01/*ACT_LINE_HIGH*/, 0x06/*VSYNC_WIDTH*/, 0x1e/*V_BP_LINE*/, 
     0x09/*V_FP_LINE*/, 0x10/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Neg_Hsync_pol, anx9030_Neg_Vsync_pol},
    //720p-60hz
    {0x72/*H_RES_LOW*/, 0x06/*H_RES_HIGH*/, 0x00/*ACT_PIX_LOW*/, 0x05/*ACT_PIX_HIGH*/, 
     0x28/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0xdc/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0xd0/*ACT_LINE_LOW*/, 0x02/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x14/*V_BP_LINE*/, 
     0x05/*V_FP_LINE*/, 0x6e/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //1080i-60hz
    {0x98/*H_RES_LOW*/, 0x08/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0x2c/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x94/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x0f/*V_BP_LINE*/, 
     0x02/*V_FP_LINE*/, 0x58/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Interlace, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //720x480i-60hz
    {0x5a/*H_RES_LOW*/,0x03 /*H_RES_HIGH*/,0xd0/*ACT_PIX_LOW*/, 0x02/*ACT_PIX_HIGH*/, 
     0x3e/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x39/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0xe0/*ACT_LINE_LOW*/, 0x01/*ACT_LINE_HIGH*/, 0x03/*VSYNC_WIDTH*/, 0x0f/*V_BP_LINE*/, 
     0x04/*V_FP_LINE*/, 0x13/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Interlace, anx9030_Neg_Hsync_pol, anx9030_Neg_Vsync_pol},
    //576p-50hz
    {0x60/*H_RES_LOW*/,0x03 /*H_RES_HIGH*/,0xd0 /*ACT_PIX_LOW*/, 0x02/*ACT_PIX_HIGH*/, 
     0x40/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x44/*H_BP_LOW*/,0x00 /*H_BP_HIGH*/, 
     0x40/*ACT_LINE_LOW*/, 0x02/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x27/*V_BP_LINE*/, 
     0x05/*V_FP_LINE*/, 0x0c/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Neg_Hsync_pol, anx9030_Neg_Vsync_pol},
    //720p-50hz
    {0xbc/*H_RES_LOW*/, 0x07/*H_RES_HIGH*/, 0x00/*ACT_PIX_LOW*/, 0x05/*ACT_PIX_HIGH*/, 
     0x28/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0xdc/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0xd0/*ACT_LINE_LOW*/, 0x02/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x14/*V_BP_LINE*/, 
     0x05/*V_FP_LINE*/, 0xb8/*H_FP_LOW*/, 0x01/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //1080i-50hz
    {0x50/*H_RES_LOW*/, 0x0a/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0x2c/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x94/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x0f/*V_BP_LINE*/, 
     0x02/*V_FP_LINE*/, 0x10/*H_FP_LOW*/, 0x02/*H_FP_HIGH*/, 
     anx9030_Interlace, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //576i-50hz
    {0x60/*H_RES_LOW*/,0x03 /*H_RES_HIGH*/,0xd0 /*ACT_PIX_LOW*/, 0x02/*ACT_PIX_HIGH*/, 
     0x3f/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x45/*H_BP_LOW*/,0x00 /*H_BP_HIGH*/, 
     0x40/*ACT_LINE_LOW*/,0x02 /*ACT_LINE_HIGH*/, 0x03/*VSYNC_WIDTH*/, 0x13/*V_BP_LINE*/, 
     0x02/*V_FP_LINE*/, 0x0c/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Interlace, anx9030_Neg_Hsync_pol, anx9030_Neg_Vsync_pol},
    //1080p-24hz
    {0xbe/*H_RES_LOW*/, 0x0a/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0x2c/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x94/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x24/*V_BP_LINE*/, 
     0x04/*V_FP_LINE*/, 0x7e/*H_FP_LOW*/, 0x02/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //1080p-25hz
    {0x50/*H_RES_LOW*/, 0x0a/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0x2c/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x94/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x24/*V_BP_LINE*/, 
     0x04/*V_FP_LINE*/, 0x10/*H_FP_LOW*/, 0x02/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},
    //1080p-30hz
    {0x98/*H_RES_LOW*/, 0x08/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0x2c/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0x94/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x24/*V_BP_LINE*/, 
     0x04/*V_FP_LINE*/, 0x58/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Progressive, anx9030_Pos_Hsync_pol, anx9030_Pos_Vsync_pol},

    //1080i-50hz(1250 Total)
    {0x00/*H_RES_LOW*/, 0x09/*H_RES_HIGH*/, 0x80/*ACT_PIX_LOW*/, 0x07/*ACT_PIX_HIGH*/, 
     0xa8/*HSYNC_WIDTH_LOW*/, 0x00/*HSYNC_WIDTH_HIGH*/, 0xb8/*H_BP_LOW*/, 0x00/*H_BP_HIGH*/, 
     0x38/*ACT_LINE_LOW*/, 0x04/*ACT_LINE_HIGH*/, 0x05/*VSYNC_WIDTH*/, 0x39/*V_BP_LINE*/, 
     0x17/*V_FP_LINE*/, 0x20/*H_FP_LOW*/, 0x00/*H_FP_HIGH*/, 
     anx9030_Interlace, anx9030_Pos_Hsync_pol, anx9030_Neg_Vsync_pol}

};
//#endif
struct Bist_Video_Format bist_demo[] = { 
//h_total  h_act   v_total v_act   h_fp  h_wid h_bp v_fp v_wid v_bp  h_pol v_pol    I_P  mode
    {800,    640,    525,    480,     16,     96,    48,    10,    2,    33,     1,        1,       0,      0},//640x480p@60
    {1650,   1280,  750,    720,    110,   40,    220,   5,      5,    20,     0,        0,       0,      1},//1280x720p@60
    {2200,   1920,  1125,  1080,    88,    44,    148,   2,     5,    15,     0,        0,        1,     2},//1920x1080i@60 
    {2200,   1920,  1125,  1080,    88,    44,     148,   4,    5,    36,     0,        0,        0,     2},//1920x1080p@60
    {1980,    1280,  750,    720,     440,  40,     220,   5,    5,    20,     0,        0,        0,     0},//1280x720p@50
    {2640,    1920,  1125,  1080,    528,  44,     148,   2,    5,   15,      0,        0,        1,     1},//1920x1080i@50
    {864,    720,     625,    576,    12,     64,    68,     5,     5,    39,     1,         1,       0,      0},//720x576p@50
    {864,    720,     625,    576,    12,     63,    69,     2,     3,    19,     1,         1,       1,      1},//720x576i@50
    {858,    720,     525,    480,    19,     63,    57,     4,     3,    15,     1,         1,       1,      2},//720x480i@60
};
extern BIT restart_system;
BYTE anx9030_avi_data[19];//, anx9030_avi_checksum;
BYTE ANX9030_system_state;
BYTE spdif_error_cnt = 0x00;
BYTE misc_reset_needed;
BYTE anx9030_stdaddr, anx9030_stdreg,anx9030_ext_block_num;
BYTE anx9030_svd_length,anx9030_sau_length;
BYTE anx9030_edid_dtd[18];
WORD anx9030_edid_length;
anx9030_edid_result_4_system anx9030_edid_result;

BIT anx9030_ddc_fifo_full;
BIT anx9030_ddc_progress;
BIT anx9030_hdcp_auth_en;
BIT anx9030_bksv_ready;
BIT anx9030_ksv_srm_pass;

BIT anx9030_HDCP_enable;
BYTE anx9030_hdcp_bcaps;
BYTE anx9030_hdcp_bstatus[2];
BIT anx9030_srm_checked;
BIT anx9030_hdcp_auth_pass;
BIT anx9030_avmute_enable;
BIT anx9030_send_blue_screen;
BIT anx9030_hdcp_encryption;
BIT anx9030_hdcp_init_done;
BIT anx9030_hdcp_wait_100ms_needed;
BIT anx9030_auth_fully_pass;
BIT anx9030_parse_edid_done;//060714 XY
//BIT testen;
//BYTE anx9030_avi_data[19], anx9030_avi_checksum;
BYTE anx9030_hdcp_auth_fail_counter ;

BYTE anx9030_video_format_config;
BIT  anx9030_emb_sync_mode,anx9030_de_gen_en,anx9030_demux_yc_en,anx9030_ddr_bus_mode;
BIT  anx9030_ddr_edge,anx9030_ycmux_BIT_sel;
BIT  anx9030_system_config_done;
BIT  anx9030_system_config_retry;
BYTE anx9030_RGBorYCbCr; //modified by zy 060814   
BYTE anx9030_in_pix_rpt,anx9030_tx_pix_rpt;
BYTE anx9030_in_pix_rpt_bkp,anx9030_tx_pix_rpt_bkp;
BYTE anx9030_video_timing_id=0;
BIT  anx9030_pix_rpt_set_by_sys;
BYTE anx9030_video_timing_parameter[18];
BYTE switch_value_sw_backup,switch_value_pc_backup;
BYTE switch_value,bist_switch_value_pc;
BIT anx9030_new_csc,anx9030_new_vid_id,anx9030_new_HW_interface;

audio_config_struct s_anx9030_audio_config;
config_packets s_anx9030_packet_config;
BYTE enable_invert_field_polarity = 0;		//add for 720P 10070524 by wangyu
BYTE FREQ_MCLK;	        //0X72:0X50 bit2:0
//000b:Fm = 128*Fs
//001b:Fm = 256*Fs
//010b:Fm = 384*Fs
//011b:Fm = 512*Fs
BYTE anx9030_audio_clock_edge;
BIT anx9030_app_hold_system_config;
BIT anx9030_shutdown;

static int dumpflag =0;

void ANX9030_Operation1() 
{
    BYTE c;

	
    if(ANX9030_system_state == ANX9030_INITIAL )
	{
		return;
    }

	if(ANX9030_system_state == ANX9030_WAIT_HOTPLUG)
    {
    	sleep(1);
        return;
    }
    if(ANX9030_system_state == ANX9030_READ_PARSE_EDID)
    {
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c &(~ANX9030_HDCP_CTRL0_HW_AUTHEN)));
		hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
		anx9030_hdcp_auth_en = 0;

        ANX9030_RST_DDCChannel();
        
        ANX9030_Parse_EDID();
        anx9030_parse_edid_done = 1;

        ANX9030_Set_System_State(ANX9030_WAIT_RX_SENSE);//060819
    }

    if(ANX9030_system_state == ANX9030_WAIT_RX_SENSE)
    {
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_STATE_REG, &c,1);
        if(c & ANX9030_SYS_STATE_RSV_DET)
        {
            //debug_puts("Receiver sense active.");
            ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);//060819
        }
        else
        {
            ;  //debug_puts("Receiver sense not active.");
        } 
    }
}

void ANX9030_Operation2(void)
{
    BYTE    i = 0;
    
    if(ANX9030_system_state == ANX9030_CONFIG_VIDEO)
    {
        for(i = 0; i<5; i++)
            {
                if(ANX9030_Config_Video())
                    break;
            }
    }


        if(ANX9030_system_state == ANX9030_CONFIG_AUDIO)
        {
            ANX9030_Config_Audio();
        }

        if(ANX9030_system_state == ANX9030_CONFIG_PACKETS)
        {
        	//usleep(10000);
            ANX9030_Config_Packet();
        }
}    

void ANX9030_Operation3(void)
{
 
    if(ANX9030_system_state == ANX9030_HDCP_AUTHENTICATION)
    {
        ANX9030_HDCP_Process();
        dumpflag =0;
    }
    
    if(ANX9030_system_state == ANX9030_PLAY_BACK)
    {            
        ANX9030_PLAYBACK_Process();
#ifdef HDMI_DEBUG
#ifdef DEBUG_DUMP_REGISTER
		int i;
		char ret;
		if (dumpflag ==0){

            printf("--------------------Begin of capture of HDMI registers------------------------\n ");
            for (i=0;i<0xff;i++)
                {
                    hdmi_i2c_read_byte(ANX9030_ADDR1, i, &ret);
                    printf("I2c Addr:0x72 register[0x%x]=0x%x\n",i,ret);
                }
        
            for (i=0;i<0xff;i++)
                {
                    hdmi_i2c_read_byte(ANX9030_ADDR2, i, &ret);
                    printf("I2C Addr: 0x7A register[0x%x]=0x%x\n",i,ret);
                }
            printf("------------------------End of capture of HDMI registers ----------------------\n");
			dumpflag =1;

	}
#endif
#endif
    }
}    

void ANX9030_Operation4() 
{
}



/**************************************************************
 *void ANX9030_Variable_Initial(void)
 **************************************************************/
void ANX9030_Variable_Initial(void)
{
    BYTE i;
    ANX9030_Set_System_State(ANX9030_INITIAL);
    anx9030_hdcp_auth_en = 0;
    //anx9030_bksv_ready = 0;
    anx9030_ksv_srm_pass =0;
    anx9030_srm_checked = 0;
    anx9030_hdcp_auth_pass = 0;
    anx9030_avmute_enable = 1;
    anx9030_hdcp_auth_fail_counter =0 ;
    anx9030_hdcp_encryption = 0;
    anx9030_send_blue_screen = 0;
    anx9030_hdcp_init_done = 0;
    anx9030_hdcp_wait_100ms_needed = 1;
    anx9030_auth_fully_pass = 0;
    //********************for video config**************
    anx9030_video_timing_id = 0;  
    anx9030_in_pix_rpt = 0;
    anx9030_tx_pix_rpt = 0;
    anx9030_new_csc = 0;
    anx9030_new_vid_id = 0;
    anx9030_new_HW_interface = 0;
    //********************end of video config*********

    //********************for edid parse***********
    anx9030_edid_result.is_HDMI = 0;
    anx9030_edid_result.ycbcr422_supported = 0;
    anx9030_edid_result.ycbcr444_supported = 0;
    anx9030_edid_result.supported_720p_60Hz = 0;
    anx9030_edid_result.supported_720p_50Hz = 0;
    anx9030_edid_result.supported_576p_50Hz = 0;
    anx9030_edid_result.supported_576i_50Hz = 0;
    anx9030_edid_result.supported_1080i_60Hz = 0;
    anx9030_edid_result.supported_1080i_50Hz = 0;
    anx9030_edid_result.supported_640x480p_60Hz = 0;
    anx9030_edid_result.supported_720x480p_60Hz = 0;
    anx9030_edid_result.supported_720x480i_60Hz = 0;
 
    anx9030_edid_result.supported_1080p_24Hz = 0;
    anx9030_edid_result.supported_1080p_25Hz = 0;
    anx9030_edid_result.supported_1080p_30Hz = 0;
     
    anx9030_edid_result.edid_errcode = 0;
    anx9030_edid_result.SpeakerFormat = 0;
    for(i = 0; i < 8; i ++)
    {
        anx9030_edid_result.AudioChannel[i] = 0;
        anx9030_edid_result.AudioFormat[i] = 0;
        anx9030_edid_result.AudioFs[i] = 0;
        anx9030_edid_result.AudioLength[i] = 0;
    }
    //********************end of edid**************

    s_anx9030_packet_config.packets_need_config = 0x03;   //new avi infoframe
    s_anx9030_packet_config.avi_info.type = 0x82;
    s_anx9030_packet_config.avi_info.version = 0x02;
    s_anx9030_packet_config.avi_info.length = 0x0d;
    s_anx9030_packet_config.avi_info.pb_byte[1] = 0x31;//YCbCr422
    s_anx9030_packet_config.avi_info.pb_byte[2] = 0x08;
    s_anx9030_packet_config.avi_info.pb_byte[3] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[4] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[5] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[6] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[7] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[8] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[9] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[10] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[11] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[12] = 0x00;
    s_anx9030_packet_config.avi_info.pb_byte[13] = 0x00;

    // audio infoframe
    s_anx9030_packet_config.audio_info.type = 0x84;
    s_anx9030_packet_config.audio_info.version = 0x01;
    s_anx9030_packet_config.audio_info.length = 0x0a;
    s_anx9030_packet_config.audio_info.pb_byte[1] = 0x00;  //zy 061123 for ATC
    s_anx9030_packet_config.audio_info.pb_byte[2] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[3] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[4] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[5] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[6] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[7] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[8] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[9] = 0x00;
    s_anx9030_packet_config.audio_info.pb_byte[10] = 0x00;

    ANX9030_Reset_AVI();
}


/**************************************************************
 *void ANX9030_HW_Interface_Variable_Initial(void)
 **************************************************************/
void ANX9030_HW_Interface_Variable_Initial(void)
{
    BYTE c;
    anx9030_video_format_config = ANX9030_VID_HW_INTERFACE;
    anx9030_RGBorYCbCr = ANX9030_INPUT_COLORSPACE;
    anx9030_ddr_edge = ANX9030_IDCK_EDGE_DDR;

    c = 0;
    c = (ANX9030_I2S_CH0_ENABLE << 2) | (ANX9030_I2S_CH1_ENABLE << 3) |
        (ANX9030_I2S_CH2_ENABLE << 4) | (ANX9030_I2S_CH3_ENABLE << 5);
    s_anx9030_audio_config.audio_type = ANX9030_AUD_HW_INTERFACE;     // input I2S
    s_anx9030_audio_config.down_sample = 0x00;
    s_anx9030_audio_config.i2s_config.audio_channel = c;//0x04;
    s_anx9030_audio_config.i2s_config.Channel_status1 =0x00;
    s_anx9030_audio_config.i2s_config.Channel_status1 = 0x00;
    s_anx9030_audio_config.i2s_config.Channel_status2 = 0x00;
    s_anx9030_audio_config.i2s_config.Channel_status3 = 0x00;
    //s_anx9030_audio_config.i2s_config.Channel_status4 = 0x00;//0x02;//48k
    s_anx9030_audio_config.i2s_config.Channel_status4 = 0x02;//0x02;//48k
    s_anx9030_audio_config.i2s_config.Channel_status5 = ANX9030_I2S_WORD_LENGTH;//0x0b;

    c = (ANX9030_I2S_SHIFT_CTRL << 3) | (ANX9030_I2S_DIR_CTRL << 2)  |
        (ANX9030_I2S_WS_POL << 1) | ANX9030_I2S_JUST_CTRL;
    s_anx9030_audio_config.i2s_config.i2s_format = c;//0x00;

    FREQ_MCLK = ANX9030_MCLK_Fs_RELATION;//set the relation of MCLK and WS
    anx9030_audio_clock_edge = ANX9030_AUD_CLK_EDGE;
}


void ANX9030_Hotplug_Change_Interrupt(void) 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_STATE_REG, &c,1);
    debug_printf("ANX9030_SYS_STATE_REG =0x%x\n", c);
    if (c & ANX9030_SYS_STATE_HP) 
    {
        debug_puts("ANX9030 HotPlug detected.");
        //disable audio & video & hdcp & TMDS and init    begin
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c & (~ANX9030_HDMI_AUDCTRL1_IN_EN)));

        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, (c & (~ANX9030_VID_CTRL_IN_EN)));

        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));

        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));

        ANX9030_Variable_Initial();
        //disable video & audio & hdcp & TMDS and init    end

        ANX9030_Set_System_State(ANX9030_READ_PARSE_EDID);//060819
        //Power on chip and select DVI mode
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c | 0x01)); 
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c & 0xfd)); 
        debug_puts("ANX9030 is set to DVI mode\n");
        ANX9030_RST_DDCChannel();      
    }  
    else 
    { 
        //Power down chip
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c & 0xfe));
        ANX9030_Set_System_State(ANX9030_WAIT_HOTPLUG);
        anx9030_hdcp_wait_100ms_needed = 1;
        anx9030_auth_fully_pass = 0;
    }
    // clear anx9030_parse_edid_done & anx9030_system_config_done
    anx9030_parse_edid_done = 0;
    anx9030_system_config_done = 0;
    anx9030_srm_checked = 0;
}



void ANX9030_Video_Clock_Change_Interrupt(void) 
{
    BYTE c;
    debug_puts("Video Clock Change Interrupt");
#if 0
	hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c & (~ANX9030_HDMI_AUDCTRL1_IN_EN)));

    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, (c & (~ANX9030_VID_CTRL_IN_EN)));
#endif
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));

    if((ANX9030_system_state != ANX9030_INITIAL) && (ANX9030_system_state != ANX9030_WAIT_HOTPLUG)
       &&  (ANX9030_system_state != ANX9030_READ_PARSE_EDID)
       &&  (ANX9030_system_state != ANX9030_WAIT_RX_SENSE)) 
    {
        ANX9030_Set_AVMute(); 
        //stop HDCP and reset DDC
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));
        ANX9030_RST_DDCChannel(); 
        debug_puts("after video clock change int ");
        ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
        // ANX9030_Set_System_State(ANX9030_READ_PARSE_EDID);
    }

    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, 0x00);//when clock change, clear this reg to avoid error in package config
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, 0x00);//when clock change, clear this reg to avoid error in package config
    anx9030_system_config_done = 0;//xy 11.06 when clock change, need system config again
    anx9030_hdcp_wait_100ms_needed = 1;
    anx9030_auth_fully_pass = 0;
}

void ANX9030_Video_Format_Change_Interrupt(void) 
{
    BYTE c;
    debug_puts("Video Format change interrupt processing");
#if 0
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c & (~ANX9030_HDMI_AUDCTRL1_IN_EN)));

    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, (c & (~ANX9030_VID_CTRL_IN_EN)));

#endif
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));

    if((ANX9030_system_state != ANX9030_INITIAL) 
       && (ANX9030_system_state != ANX9030_WAIT_HOTPLUG)
       &&  (ANX9030_system_state != ANX9030_READ_PARSE_EDID)
       && (ANX9030_system_state != ANX9030_WAIT_RX_SENSE)) 
    {
        debug_puts("after video format change int ");
        ANX9030_Set_AVMute();
        //stop HDCP and reset DDC
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));
        ANX9030_RST_DDCChannel(); 
        ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
        //ANX9030_Set_System_State(ANX9030_READ_PARSE_EDID);
    } 


	hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, 0x00);//when format change, clear this reg to avoid error in package config
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, 0x00);//when format change, clear this reg to avoid error in package config
    anx9030_system_config_done = 0;//xy 11.06 when format change, need system config again

    anx9030_hdcp_wait_100ms_needed = 1; //added by fxd 2008.6.3
    anx9030_auth_fully_pass = 0;


}

void ANX9030_Audio_CLK_Change_Interrupt(void) 
{

    BYTE c;
    // deleted by fxd & lixun  to avoid blue screen
    //    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
    //   hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));

    if((ANX9030_system_state != ANX9030_INITIAL) 
       && (ANX9030_system_state != ANX9030_WAIT_HOTPLUG) 
       && (ANX9030_system_state != ANX9030_READ_PARSE_EDID)
       && (ANX9030_system_state != ANX9030_WAIT_RX_SENSE))
    //   && (ANX9030_system_state != ANX9030_CONFIG_VIDEO))
    {
        debug_puts("ANX9030: audio clock changed interrupt,disable audio.");
        // disable audio 
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG,&c,1);
        c &= ~ANX9030_HDMI_AUDCTRL1_IN_EN;
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, c);
        ANX9030_Set_System_State(ANX9030_CONFIG_AUDIO);
        //ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
    }


    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, 0x00);//when format change, clear this reg to avoid error in package config
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, 0x00);//when format change, clear this reg to avoid error in package config
    anx9030_system_config_done = 0;//xy 11.06 when format change, need system config again
}

void ANX9030_Set_AVMute(void) 
{
    BYTE c;

    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_GNRL_CTRL_PKT_REG, 0x01);
    hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, (c | 0x0c));
    anx9030_avmute_enable = 1;
}

void ANX9030_Clear_AVMute(void) 
{
    BYTE c;
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_GNRL_CTRL_PKT_REG, 0x02);
    hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, (c | 0x0c));
    anx9030_avmute_enable = 0;
}

void ANX9030_Auth_Done_Interrupt() 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_STATUS_REG, &c,1);
    if(c & ANX9030_HDCP_STATUS_AUTH_PASS) 
    {
        puts("ANX9030_Authentication pass");
        ANX9030_Blue_Screen_Disable();
        anx9030_hdcp_auth_pass = 1;
        anx9030_hdcp_auth_fail_counter = 0;
    } 
    else 
    {
    	if (anx9030_hdcp_auth_pass==1)
			puts("ANX9030_Authentication failed");
        anx9030_hdcp_wait_100ms_needed = 1;
        anx9030_auth_fully_pass = 0;
        anx9030_hdcp_auth_pass = 0;
        anx9030_hdcp_auth_fail_counter ++;
        if(anx9030_hdcp_auth_fail_counter >= ANX9030_HDCP_FAIL_THRESHOLD) 
        {
            anx9030_hdcp_auth_fail_counter = 0;
            anx9030_bksv_ready = 0;
            // TODO: Reset link; 
            ANX9030_Blue_Screen_Enable();
            ANX9030_HDCP_Encryption_Disable();
            //disable audio
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, c & (~ANX9030_HDMI_AUDCTRL1_IN_EN));
        }
		usleep(100000);
    }
}

void ANX9030_Auth_Change_Interrupt() 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_STATUS_REG, &c,1);
    if(c & ANX9030_HDCP_STATUS_AUTH_PASS) 
    {
        anx9030_hdcp_auth_pass = 1;
    } 
    else 
    {
        ANX9030_Set_AVMute(); 
        debug_puts("ANX9030_Authentication failed_by_Auth_change");
        anx9030_hdcp_auth_pass = 0;
        anx9030_hdcp_wait_100ms_needed = 1;
        anx9030_auth_fully_pass = 0;
        ANX9030_HDCP_Encryption_Disable();
        if(ANX9030_system_state == ANX9030_PLAY_BACK) 
        {
            anx9030_auth_fully_pass = 0;
            ANX9030_Set_System_State(ANX9030_HDCP_AUTHENTICATION);
            //disable audio
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c & (~ANX9030_HDMI_AUDCTRL1_IN_EN)));
        }
    }
}


void ANX9030_AFIFO_Overrun_Interrupt(void)
{
    BYTE c;

	hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));

    if(ANX9030_system_state != ANX9030_INITIAL 
       && ANX9030_system_state != ANX9030_WAIT_HOTPLUG 
       && ANX9030_system_state != ANX9030_READ_PARSE_EDID
       && ANX9030_system_state != ANX9030_WAIT_RX_SENSE
       && ANX9030_system_state != ANX9030_CONFIG_VIDEO)
    {
        debug_puts("ANX9030: AFIFO overrun interrupt,disable audio.");
        // disable audio 
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG,&c,1);
        c &= ~ANX9030_HDMI_AUDCTRL1_IN_EN;
        hdmi_i2c_write(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c, 1);
        ANX9030_Set_System_State(ANX9030_CONFIG_AUDIO);
        //ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
    }
	anx9030_system_config_done = 0; //add by fxd 2008,2,4
}

void ANX9030_PllLock_Interrupt()
{
    BYTE c;

    if((ANX9030_system_state != ANX9030_INITIAL) 
       && (ANX9030_system_state != ANX9030_WAIT_HOTPLUG) 
       && (ANX9030_system_state != ANX9030_READ_PARSE_EDID)
       && (ANX9030_system_state != ANX9030_WAIT_RX_SENSE))
    {
        ANX9030_Set_AVMute();
        debug_puts("ANX9030: PLL unlock interrupt,disable audio.");
        // disable audio & video
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG,&c,1);
        c &= ~ANX9030_HDMI_AUDCTRL1_IN_EN;
        hdmi_i2c_write(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c, 1);

        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_VID_CTRL_REG,&c,1);
        c &= ~ANX9030_VID_CTRL_IN_EN;
        hdmi_i2c_write(ANX9030_ADDR1, ANX9030_VID_CTRL_REG, &c, 1);
        ANX9030_Set_System_State(ANX9030_CONFIG_VIDEO);
    }
}

#if 1  //delete by fxd
void ANX9030_SPDIF_Error_Interrupt(BYTE int1, BYTE int3)
{
    if((ANX9030_system_state == ANX9030_CONFIG_AUDIO
        || ANX9030_system_state == ANX9030_CONFIG_PACKETS
        || ANX9030_system_state == ANX9030_HDCP_AUTHENTICATION
        || ANX9030_system_state == ANX9030_PLAY_BACK ) 
       && (int3 & 0x81)) 
    {
        debug_puts("SPDIF BI Phase or Unstable error.");
        spdif_error_cnt += 0x03; 
    }
    if((ANX9030_system_state == ANX9030_CONFIG_AUDIO
        || ANX9030_system_state == ANX9030_CONFIG_PACKETS
        || ANX9030_system_state == ANX9030_HDCP_AUTHENTICATION
        || ANX9030_system_state == ANX9030_PLAY_BACK ) 
       && (int1 & ANX9030_INTR1_STATUS_SPDIF_ERR))
    {
        debug_puts("SPDIF Parity error.");
        spdif_error_cnt += 0x01; 
    }
    // adjust spdif phase
    if(spdif_error_cnt >= spdif_error_th)
    {
        BYTE freq_mclk,c1,c;
        spdif_error_cnt = 0x00;
        debug_puts("adjust mclk phase!");
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL0_REG, &c,1);
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_I2S_CTRL_REG, &c1,1);

        freq_mclk = c & 0x07; 
        switch(freq_mclk)
        {
        case ANX9030_mclk_128_Fs:   //invert 0x50[3]
            debug_puts("adjust mclk phase when 128*Fs!");
            if( c & 0x08 )    c &= 0xf7;
            else   c |= 0x08;
            hdmi_i2c_write(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL0_REG, &c, 1);
            break;
                
        case ANX9030_mclk_256_Fs:
        case ANX9030_mclk_384_Fs:
            debug_puts("adjust mclk phase when 256*Fs or 384*Fs!");
            if( c1 & 0x60 )   c1 &= 0x9f;
            else     c1 |= 0x20;
            hdmi_i2c_write(ANX9030_ADDR1, ANX9030_I2S_CTRL_REG, &c1, 1);
            break;

        case ANX9030_mclk_512_Fs:
            debug_puts("adjust mclk phase when 512*Fs!");
            if( c1 & 0x60 )   c1 &= 0x9f;
            else    c1 |= 0x40;
            hdmi_i2c_write(ANX9030_ADDR1, ANX9030_I2S_CTRL_REG, &c1, 1);
            break;
        default:
            break;

        }     
    }
}
#endif 

void ANX9030_Rx_Sense_Interrupt(void)
{
    BYTE c;

    debug_puts("Rx Sense Interrupt");
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_STATE_REG,&c,1);

    if( c & ANX9030_SYS_STATE_RSV_DET)
    {
        //xy 11.06 Power on chip
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c | 0x01));
    }
    else
    {
        // Rx is not active
        if((ANX9030_system_state != ANX9030_INITIAL) 
           && (ANX9030_system_state != ANX9030_WAIT_HOTPLUG) 
           && (ANX9030_system_state != ANX9030_READ_PARSE_EDID))
        {
            ANX9030_Set_System_State(ANX9030_WAIT_RX_SENSE);
            //stop HDCP and reset DDC when lost Rx sense
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));
            ANX9030_RST_DDCChannel(); 
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c & 0xfd));
            // mute TMDS link
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, (c & (~ANX9030_TMDS_CLKCH_MUTE)));
        }
        //Power down chip
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, (c & 0xfe));
    }
}

    

void ANX9030_Blue_Screen_Format_Config(void) 
{
    // TODO:Add ITU 601 format.(Now only ITU 709 format added)
    switch (anx9030_RGBorYCbCr) 
    {
    case ANX9030_RGB: //select RGB mode
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN0_REG, 0x10);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN1_REG, 0xeb);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN2_REG, 0x10);
        break;
    case ANX9030_YCbCr422: //select YCbCr4:2:2 mode
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN0_REG, 0x00);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN1_REG, 0xad);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN2_REG, 0x2a);
        break;
    case ANX9030_YCbCr444: //select YCbCr4:4:4 mode
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN0_REG, 0x1a);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN1_REG, 0xad);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_BLUESCREEN2_REG, 0x2a);
        break;
    default: 
        break;
    }
}

void ANX9030_Blue_Screen_Enable(void) 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL1_REG,  c | ANX9030_HDCP_CTRL1_BLUE_SCREEN_EN);
    anx9030_send_blue_screen = 1;
}    

void ANX9030_Blue_Screen_Disable(void) 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL1_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL1_REG,  (c & 0xfb));
    anx9030_send_blue_screen = 0;
}

void ANX9030_HDCP_Encryption_Enable(void) 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c | ANX9030_HDCP_CTRL0_ENC_EN));
    anx9030_hdcp_encryption = 1;
}    

void ANX9030_HDCP_Encryption_Disable(void) 
{
    BYTE c;
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c, 1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & 0xfb));
    anx9030_hdcp_encryption = 0;
}    




/**************************************************************
 *void ANX9030_Config_Video(void)
 **************************************************************/
BIT ANX9030_Config_Video(void)
{
    BYTE c,TX_is_HDMI;
    BIT cspace_y2r, y2r_sel, up_sample,range_y2r;

    cspace_y2r = 0;
    y2r_sel = 0;
    up_sample = 0;
    range_y2r = 0;
	anx9030_RGBorYCbCr = ANX9030_INPUT_COLORSPACE;				//wen
	ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, 0x00);			//wen
    if(!anx9030_system_config_done)
    {
        //printf("System has not finished config!\n");
        return(1);
    }

    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_SYS_STATE_REG, &c);
	

	if(!(c & 0x02))
    {
        debug_printf("No clock detected !\n");
        //ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL2_REG, 0x02);
		sleep(1);
        //return;
    }
 
    ANX9030_Clean_HDCP();

    debug_printf("ANX9030_Config_Video  anx9030_video_timing_id=%d, anx9030_RGBorYCbCr=%d\n", anx9030_video_timing_id,anx9030_RGBorYCbCr);
    //color space issue
    switch(anx9030_video_timing_id)
    {
    case anx9030_V1280x720p_50Hz:
    case anx9030_V1280x720p_60Hz:
    case anx9030_V1920x1080i_60Hz:
    case anx9030_V1920x1080i_50Hz:
    case anx9030_V1920x1080p_60Hz:
    case anx9030_V1920x1080p_50Hz:
    case anx9030_V1920x1080p_24Hz: 
    case anx9030_V1920x1080p_25Hz: 
    case anx9030_V1920x1080p_30Hz: 

        y2r_sel = ANX9030_CSC_BT709;
        break;
    default:
        y2r_sel = ANX9030_CSC_BT601;
        break;
    }
    //rang[0~255]/[16~235] select
    if(anx9030_video_timing_id == anx9030_V640x480p_60Hz)
        range_y2r = 1;//rang[0~255]
    else
        range_y2r = 0;//rang[16~235]

	if((anx9030_RGBorYCbCr == ANX9030_YCbCr422) && (!anx9030_edid_result.ycbcr422_supported))
    {
        up_sample = 1;
	 	if(anx9030_edid_result.ycbcr444_supported)
            cspace_y2r = 0;
	 	else
            cspace_y2r = 1;
    }
    if((anx9030_RGBorYCbCr == ANX9030_YCbCr444) && (!anx9030_edid_result.ycbcr444_supported))
    {
        cspace_y2r = 1;
    }

    up_sample =1;
    cspace_y2r = 1;
    range_y2r =1;

    //Config the embeded blue screen format according to output video format.
    ANX9030_Blue_Screen_Format_Config();

    ANX9030_Parse_Video_Format();

    if(anx9030_de_gen_en)
    {
        printf("anx9030_de_gen_en!\n");
        ANX9030_DE_Generator();
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c & (~ANX9030_VID_CAPCTRL0_DEGEN_EN));
    }
    if(anx9030_emb_sync_mode)
    {
        debug_printf("ANX9030_Embed_Sync_Decode!");
        ANX9030_Embed_Sync_Decode();
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c & (~ANX9030_VID_CAPCTRL0_EMSYNC_EN));
    }
    if(anx9030_demux_yc_en)
    {
        //printf("anx9030_demux_yc_en!");
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c | ANX9030_VID_CAPCTRL0_DEMUX_EN);
        if(anx9030_ycmux_BIT_sel)
        {
            ANX9030_i2c_read_p0_reg(ANX9030_VID_CTRL_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_VID_CTRL_REG, c | ANX9030_VID_CTRL_YCBIT_SEL);
        }
        else
        {
            ANX9030_i2c_read_p0_reg(ANX9030_VID_CTRL_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_VID_CTRL_REG, c & (~ANX9030_VID_CTRL_YCBIT_SEL));
        }
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c & (~ANX9030_VID_CAPCTRL0_DEMUX_EN));
    }
    if(anx9030_ddr_bus_mode)
    {
        //printf("anx9030_ddr_bus_mode!\n");
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c | ANX9030_VID_CAPCTRL0_DV_BUSMODE);
        if(anx9030_ddr_edge)
        {
            ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c | ANX9030_VID_CAPCTRL0_DDR_EDGE);
        }
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c & (~ANX9030_VID_CAPCTRL0_DV_BUSMODE));
        ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c & (~ANX9030_VID_CAPCTRL0_DDR_EDGE));
    }

    if(cspace_y2r)
    {
        //printf("Color space Y2R enabled********");
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c | ANX9030_VID_MODE_CSPACE_Y2R);
        if(y2r_sel)
        {
            //printf("Y2R_SEL!\n");
            ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c | ANX9030_VID_MODE_Y2R_SEL);
        }
        else
        {
            ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c & (~ANX9030_VID_MODE_Y2R_SEL));
        }
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c & (~ANX9030_VID_MODE_CSPACE_Y2R));
    }

    if(up_sample)
    {
        //printf("UP_SAMPLE!\n");
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c | ANX9030_VID_MODE_UPSAMPLE);
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c & (~ANX9030_VID_MODE_UPSAMPLE));
    }

    if(range_y2r)
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c | ANX9030_VID_MODE_RANGE_Y2R);
    }
    else
    {
        ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, c & (~ANX9030_VID_MODE_RANGE_Y2R));
    }

    if(!anx9030_pix_rpt_set_by_sys)
    {
        if((anx9030_video_timing_id == anx9030_V720x480i_60Hz_16x9)
           || (anx9030_video_timing_id == anx9030_V720x576i_50Hz_16x9)
           || (anx9030_video_timing_id == anx9030_V720x480i_60Hz_4x3)
           || (anx9030_video_timing_id == anx9030_V720x576i_50Hz_4x3))
            anx9030_tx_pix_rpt = 1;
        else
            anx9030_tx_pix_rpt = 0;
    }
    //set input pixel repeat times
    ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_MODE_REG, ((c & 0xfc) |anx9030_in_pix_rpt));
    //set link pixel repeat times
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CTRL_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CTRL_REG, ((c & 0xfc) |anx9030_tx_pix_rpt));

    if((anx9030_in_pix_rpt != anx9030_in_pix_rpt_bkp)
       ||(anx9030_tx_pix_rpt != anx9030_tx_pix_rpt_bkp) )
    {
        ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL2_REG, 0x02);
        ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL2_REG, 0x00);
        debug_printf("MISC_Reset!\n");
        anx9030_in_pix_rpt_bkp = anx9030_in_pix_rpt;
        anx9030_tx_pix_rpt_bkp = anx9030_tx_pix_rpt;
    }
    //enable video input
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CTRL_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CTRL_REG, c | ANX9030_VID_CTRL_IN_EN);

        
    //check if the video input is stable
    {
        int icount = 0;
        for(icount = 0; icount < 100; icount++)
            {
                ANX9030_i2c_read_p0_reg(ANX9030_VID_STATUS_REG, &c);
                debug_printf("ANX9030_VID_STATUS_REG=%x\n",c);
                if(!(c & ANX9030_VID_STATUS_VID_STABLE))
                    {
                        debug_printf("Video not stable!\n");
                        usleep(30000);
                    }
                else
                    break;
            }
    }

    int v_act;          
    anx9030_system_config_retry = 0; 
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_ACTVIDLINEH_REG, &c);
    v_act = c;
    v_act = v_act << 8;
          
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_ACTVIDLINEL_REG, &c);
    v_act = v_act + c;
          
    debug_printf("v_act = %d\n", v_act);
    switch(anx9030_video_timing_id) {
    case anx9030_V720x480p_60Hz_4x3:
    case anx9030_V720x480p_60Hz_16x9:
    case anx9030_V720x480i_60Hz_4x3:
    case anx9030_V720x480i_60Hz_16x9:
        {
            if(v_act != 480 && v_act != 483)  
                {
                    anx9030_system_config_retry = 1; 
                }
        }
        break;
              
    case anx9030_V1280x720p_60Hz:
    case anx9030_V1280x720p_50Hz:
        {
            if(v_act != 720)  
                {
                    anx9030_system_config_retry = 1;
                      
                }
        }
        break;
              
    case anx9030_V1920x1080i_60Hz:
    case anx9030_V1920x1080p_60Hz:
    case anx9030_V1920x1080i_50Hz:
    case anx9030_V1920x1080p_50Hz:
    case anx9030_V1920x1080p_24Hz: 
    case anx9030_V1920x1080p_25Hz: 
    case anx9030_V1920x1080p_30Hz: 
        {
            if(v_act != 1080)  
                {
                    debug_printf("v_act != 1080 = %d\n",v_act);
                    anx9030_system_config_retry = 1; 
                }
        }
        break;
                  
    case anx9030_V720x576p_50Hz_4x3:
    case anx9030_V720x576p_50Hz_16x9:
    case anx9030_V720x576i_50Hz_4x3:
    case anx9030_V720x576i_50Hz_16x9:
    default:
        {
            if(v_act != 576)  
                {
                    anx9030_system_config_retry = 1; 
                }
        }
        break;
    }
    if(anx9030_system_config_retry){
        ANX9030_i2c_write_p0_reg(ANX9030_SRST_REG, 0xff);                
        anx9030_system_config_done = 0;
        ANX9030_i2c_write_p0_reg(ANX9030_SRST_REG, 0x00);
        return(0);

    }

        
    /*Video is stable */
    if(cspace_y2r)
        anx9030_RGBorYCbCr = ANX9030_RGB;
    //Enable video CLK,Format change after config video.
    ANX9030_i2c_read_p0_reg(ANX9030_INTR1_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR1_MASK_REG, c |0x01);//3
    ANX9030_i2c_read_p0_reg(ANX9030_INTR3_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR3_MASK_REG, c | 0x40);

        
    if(anx9030_edid_result.is_HDMI)
        {
            ANX9030_i2c_read_p0_reg(ANX9030_SYS_CTRL1_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL1_REG, c | 0x02);
            //printf("ANX9030 is set to HDMI mode\n");
        }

    ANX9030_i2c_read_p0_reg(ANX9030_SYS_CTRL1_REG, &c);
    TX_is_HDMI = c & 0x02;
    //       TX_is_HDMI = 1;
    if(TX_is_HDMI == 0x02)
        {
            ANX9030_Set_AVMute();
            ANX9030_Set_System_State(ANX9030_CONFIG_AUDIO);
        }
    else
        {
            //To-Do: Config to DVI mode.
            ANX9030_Set_System_State(ANX9030_HDCP_AUTHENTICATION);		//wen_081120
            ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, &c);
            ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, (c | ANX9030_TMDS_CLKCH_MUTE));
        }
    //reset TMDS link to align 4 channels  xy 061120
    ANX9030_i2c_read_p0_reg(ANX9030_SRST_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_SRST_REG, (c | ANX9030_TX_RST));
    ANX9030_i2c_write_p0_reg(ANX9030_SRST_REG, (c & (~ANX9030_TX_RST)));
    //Enable TMDS clock output // just enable bit7, and let the other bits along to avoid overwriting.
    //    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, &c);
    //    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, (c | ANX9030_TMDS_CLKCH_MUTE));
    //    usleep(5000);		//change by xh 08-09
       
    //ANX9030_i2c_read_p0_reg(ANX9030_VID_MODE_REG, &c);  //zy 061110
    return(1);
}

//********************************************end video config*******************************

void ANX9030_Parse_Video_Format(void)
{
    switch(anx9030_video_format_config)
    {
    case anx9030_RGB_YCrCb444_SepSync:
        anx9030_emb_sync_mode = 0;
        anx9030_demux_yc_en = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        debug_puts("RGB_YCrCb444_SepSync mode!\n");
        break;
    case anx9030_YCrCb422_SepSync:
        anx9030_emb_sync_mode = 0;
        anx9030_demux_yc_en = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        debug_puts("YCrCb422_SepSync mode!\n");
        break;
    case anx9030_YCrCb422_EmbSync:
        debug_puts("YCrCb422_EmbSync mode!\n");
        anx9030_demux_yc_en = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        anx9030_emb_sync_mode = 1;
        ANX9030_Get_Video_Timing();
        break;
    case anx9030_YCMux422_SepSync_Mode1:
        debug_puts("YCMux422_SepSync_Mode1 mode!\n");
        anx9030_emb_sync_mode = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        anx9030_ycmux_BIT_sel = 0;
        anx9030_demux_yc_en = 1;
        break;
    case anx9030_YCMux422_SepSync_Mode2:
        debug_puts("YCMux422_SepSync_Mode2 mode!\n");
        anx9030_emb_sync_mode = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        anx9030_ycmux_BIT_sel = 1;
        anx9030_demux_yc_en = 1;
        break;
    case anx9030_YCMux422_EmbSync_Mode1:
        debug_puts("YCMux422_EmbSync_Mode1 mode!\n");
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        anx9030_emb_sync_mode = 1;
        anx9030_ycmux_BIT_sel = 0;
        anx9030_demux_yc_en = 1;
        ANX9030_Get_Video_Timing();
        break;
    case anx9030_YCMux422_EmbSync_Mode2:
        debug_puts("YCMux422_EmbSync_Mode2 mode!\n");
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 0;
        anx9030_emb_sync_mode = 1;
        anx9030_ycmux_BIT_sel = 1;
        anx9030_demux_yc_en = 1;
        ANX9030_Get_Video_Timing();
        break;
    case anx9030_RGB_YCrCb444_DDR_SepSync:
        debug_puts("RGB_YCrCb444_DDR_SepSync mode!\n");
        anx9030_emb_sync_mode = 0;
        anx9030_demux_yc_en = 0;
        anx9030_de_gen_en = 0;
        anx9030_ddr_bus_mode = 1;
        break;
    case anx9030_RGB_YCrCb444_DDR_EmbSync:
        debug_puts("RGB_YCrCb444_DDR_EmbSync mode!\n");
        anx9030_demux_yc_en = 0;
        anx9030_de_gen_en = 0;
        anx9030_emb_sync_mode = 1;
        anx9030_ddr_bus_mode = 1;
        ANX9030_Get_Video_Timing();
        break;
    case anx9030_RGB_YCrCb444_SepSync_No_DE:
        debug_puts("RGB_YCrCb444_SepSync_No_DE mode!\n");
        anx9030_emb_sync_mode = 0;
        anx9030_demux_yc_en = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 1;
        ANX9030_Get_Video_Timing();
        break;
    case anx9030_YCrCb422_SepSync_No_DE:
        debug_puts("YCrCb422_SepSync_No_DE mode!\n");
        anx9030_emb_sync_mode = 0;
        anx9030_demux_yc_en = 0;
        anx9030_ddr_bus_mode = 0;
        anx9030_de_gen_en = 1;
        ANX9030_Get_Video_Timing();
        break;
    default:
        break;
    }
}

void ANX9030_Get_Video_Timing()
{
    BYTE i;
    for(i = 0; i < 18; i++)
    {
        switch(anx9030_video_timing_id)
        {
        case anx9030_V640x480p_60Hz:
            debug_puts("640x480p_60Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_640x480p_60Hz[i];
            break;
        case anx9030_V720x480p_60Hz_4x3:
        case anx9030_V720x480p_60Hz_16x9:
            debug_puts("720x480p_60Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_720x480p_60Hz[i];
            break;
        case anx9030_V1280x720p_60Hz:
            debug_puts("1280x720p_60Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1280x720p_60Hz[i];
            break;
        case anx9030_V1920x1080i_60Hz:
            debug_puts("1920x1080i_60Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1920x1080i_60Hz[i];
            break;
        case anx9030_V720x480i_60Hz_4x3:
        case anx9030_V720x480i_60Hz_16x9:
            debug_puts("720x480i_60Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_720x480i_60Hz[i];
            break;
        case anx9030_V720x576p_50Hz_4x3:
        case anx9030_V720x576p_50Hz_16x9:
            debug_puts("720x576p_50Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_720x576p_50Hz[i];
            break;
        case anx9030_V1280x720p_50Hz:
            debug_puts("1280x720p_50Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1280x720p_50Hz[i];
            break;
        case anx9030_V1920x1080i_50Hz:
            debug_puts("1920x1080i_50Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1920x1080i_50Hz[i];
            break;
        case anx9030_V1920x1080p_24Hz:
            debug_puts("1920x1080p_24Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1920x1080p_24Hz[i];
            break;
        case anx9030_V1920x1080p_25Hz:
            debug_puts("1920x1080p_25Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1920x1080p_25Hz[i];
            break;
        case anx9030_V1920x1080p_30Hz:
            debug_puts("1920x1080p_30Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_1920x1080p_30Hz[i];
            break;


        case anx9030_V720x576i_50Hz_4x3:
        case anx9030_V720x576i_50Hz_16x9:
            debug_puts("720x576i_50Hz!\n");
            anx9030_video_timing_parameter[i] = anx9030_video_timingtype_table.anx9030_720x576i_50Hz[i];
            break;

        default:
            break;
        }
        debug_printf("Video_Timing_Parameter[%.2x]=%.2x\n", (WORD)i, (WORD) anx9030_video_timing_parameter[i]);
    }

}

/**************************************************************
 *void ANX9030_DE_Generator(void)
 **************************************************************/
void ANX9030_DE_Generator(void)
{
    BYTE c,video_type,hsync_pol,vsync_pol,v_fp,v_bp,vsync_width;
    BYTE hsync_width_low,hsync_width_high,v_active_low,v_active_high;
    BYTE h_active_low,h_active_high,h_res_low,h_res_high,h_bp_low,h_bp_high;
    WORD hsync_width,h_active,h_res,h_bp;

    video_type = anx9030_video_timing_parameter[15];
    hsync_pol = anx9030_video_timing_parameter[16];
    vsync_pol = anx9030_video_timing_parameter[17];
    v_fp = anx9030_video_timing_parameter[12];
    v_bp = anx9030_video_timing_parameter[11];
    vsync_width = anx9030_video_timing_parameter[10];
    hsync_width = anx9030_video_timing_parameter[5];
    hsync_width = (hsync_width << 8) + anx9030_video_timing_parameter[4];
    v_active_high = anx9030_video_timing_parameter[9];
    v_active_low = anx9030_video_timing_parameter[8];
    h_active = anx9030_video_timing_parameter[3];
    h_active = (h_active << 8) + anx9030_video_timing_parameter[2];
    h_res = anx9030_video_timing_parameter[1];
    h_res = (h_res << 8) + anx9030_video_timing_parameter[0];
    h_bp = anx9030_video_timing_parameter[7];
    h_bp = (h_bp << 8) + anx9030_video_timing_parameter[6];
    if(anx9030_demux_yc_en)
    {
        hsync_width = 2* hsync_width;
        h_active = 2 * h_active;
        h_res = 2 * h_res;
        h_bp = 2 * h_bp;
    }
    hsync_width_low = hsync_width & 0xff;
    hsync_width_high = (hsync_width >> 8) & 0xff;
    h_active_low = h_active & 0xff;
    h_active_high = (h_active >> 8) & 0xff;
    h_res_low = h_res & 0xff;
    h_res_high = (h_res >> 8) & 0xff;
    h_bp_low = h_bp & 0xff;
    h_bp_high = (h_bp >> 8) & 0xff;

    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<3);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |video_type);//set video type
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<5);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |hsync_pol);//set HSYNC polarity
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<6);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |vsync_pol);//set VSYNC polarity
    c&= ~(1<<4);
    if(enable_invert_field_polarity)
    {
		ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c|(1<<4));//enable bit for invert polarity
    }
    else
    {
		ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c );//enable bit for invert polarity
    }



    ANX9030_i2c_write_p0_reg(ANX9030_ACT_LINEL_REG, v_active_low);
    ANX9030_i2c_write_p0_reg(ANX9030_ACT_LINEH_REG, v_active_high);
    ANX9030_i2c_write_p0_reg(ANX9030_VSYNC_WID_REG, vsync_width);
    ANX9030_i2c_write_p0_reg(ANX9030_VSYNC_TAIL2VIDLINE_REG, v_bp);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_PIXL_REG, h_active_low);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_PIXH_REG, h_active_high);
    ANX9030_i2c_write_p0_reg(ANX9030_H_RESL_REG, h_res_low);
    ANX9030_i2c_write_p0_reg(ANX9030_H_RESH_REG, h_res_high);
    ANX9030_i2c_write_p0_reg(ANX9030_HSYNC_ACT_WIDTHL_REG, hsync_width_low);
    ANX9030_i2c_write_p0_reg(ANX9030_HSYNC_ACT_WIDTHH_REG, hsync_width_high);
    ANX9030_i2c_write_p0_reg(ANX9030_H_BACKPORCHL_REG, h_bp_low);
    ANX9030_i2c_write_p0_reg(ANX9030_H_BACKPORCHH_REG, h_bp_high);
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c | ANX9030_VID_CAPCTRL0_DEGEN_EN);
}


/**************************************************************
 *void ANX9030_Embed_Sync_Decode(void)
 **************************************************************/
void ANX9030_Embed_Sync_Decode(void)
{
    BYTE c,video_type,hsync_pol,vsync_pol,v_fp,vsync_width;
    BYTE h_fp_low,h_fp_high,hsync_width_low,hsync_width_high;
    WORD h_fp,hsync_width;

    video_type = anx9030_video_timing_parameter[15];
    hsync_pol = anx9030_video_timing_parameter[16];
    vsync_pol = anx9030_video_timing_parameter[17];
    v_fp = anx9030_video_timing_parameter[12];
    vsync_width = anx9030_video_timing_parameter[10];
    h_fp = anx9030_video_timing_parameter[14];
    h_fp = (h_fp << 8) + anx9030_video_timing_parameter[13];
    hsync_width = anx9030_video_timing_parameter[5];
    hsync_width = (hsync_width << 8) + anx9030_video_timing_parameter[4];
    if(anx9030_demux_yc_en)
    {
        h_fp = 2 * h_fp;
        hsync_width = 2* hsync_width;
    }
    h_fp_low = h_fp & 0xff;
    h_fp_high = (h_fp >> 8) & 0xff;
    hsync_width_low = hsync_width & 0xff;
    hsync_width_high = (hsync_width >> 8) & 0xff;

    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<3);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |video_type);//set video type
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<5);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |hsync_pol);//set HSYNC polarity
    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
	c&= ~(1<<6);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |vsync_pol);//set VSYNC polarity

    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);			//wen_081120
    c&= ~(1<<4);
    if(enable_invert_field_polarity)
    {
		debug_printf("enable_invert_field_polarity\n");
		ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c |(1<<4));//enable bit for invert polarity
    }
    else
	{
		debug_printf("no--enable_invert_field_polarity\n");
		ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL1_REG, c);//enable bit for invert polarity
	}


    ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL0_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_VID_CAPCTRL0_REG, c | ANX9030_VID_CAPCTRL0_EMSYNC_EN);
    ANX9030_i2c_write_p0_reg(ANX9030_ACT_LINE2VSYNC_REG, v_fp);
    ANX9030_i2c_write_p0_reg(ANX9030_VSYNC_WID_REG, vsync_width);
    ANX9030_i2c_write_p0_reg(ANX9030_H_FRONTPORCHL_REG, h_fp_low);
    ANX9030_i2c_write_p0_reg(ANX9030_H_FRONTPORCHH_REG, h_fp_high);
    ANX9030_i2c_write_p0_reg(ANX9030_HSYNC_ACT_WIDTHL_REG, hsync_width_low);
    ANX9030_i2c_write_p0_reg(ANX9030_HSYNC_ACT_WIDTHH_REG, hsync_width_high);
}

void ANX9030_Show_Video_Parameter()
{
    // int h_res,h_act,v_res,v_act,h_fp,hsync_width,h_bp;
    BYTE c;
	int h_res,h_act,v_res,v_act,h_fp,hsync_width,h_bp;
    puts("***********************************ANX9030 Info************************************");
    puts("   ANX9030 mode = Normal mode");
    if((anx9030_demux_yc_en == 1) && (anx9030_emb_sync_mode == 0))
        puts("   Input video format = YC_MUX");
    if((anx9030_demux_yc_en == 0) && (anx9030_emb_sync_mode == 1))
        puts("   Input video format = 656");
    if((anx9030_demux_yc_en == 1) && (anx9030_emb_sync_mode == 1))
        puts("   Input video format = YC_MUX + 656");
    if((anx9030_demux_yc_en == 0) && (anx9030_emb_sync_mode == 0))
        puts("   Input video format = Seperate Sync");
    if(anx9030_de_gen_en)
        puts("   DE generator = Enable");
    else
        puts("   DE generator = Disable");
    if(anx9030_ddr_bus_mode)
        puts("   DDR mode = Enable");
    else
        puts("   DDR mode = Disable");
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
    c = (c & 0x02);
    if(c)
    {
        puts("   Output video mode = HDMI");
        hdmi_i2c_read(ANX9030_ADDR2,0x04, &c,1);
        c = (c & 0x60) >> 5; 
        switch(c)
        {
        case ANX9030_RGB:
            puts("   Output video color format = RGB");
            break;
        case ANX9030_YCbCr422:
            puts("   Output video color format = YCbCr422");
            break;
        case ANX9030_YCbCr444:
            puts("   Output video color format = YCbCr444");
            break;
        default:
            break;
        }
    }
    else
    {
        puts("   Output video mode = DVI");
        puts("   Output video color format = RGB");
    }
    ANX9030_i2c_read_p0_reg(ANX9030_VID_STATUS_REG, &c);
    if((c & ANX9030_VID_STATUS_TYPE) == 0x04)
        printf("   Video Type = Interlace\n");
    else
        printf("   Video Type = Progressive\n");
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HRESH_REG, &c);
    h_res = c;
    h_res = h_res << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HRESL_REG, &c);
    h_res = h_res + c;
    printf("   H_resolution = %u\n",h_res);
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_PIXH_REG, &c);
    h_act = c;
    h_act = h_act << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_PIXL_REG, &c);
    h_act = h_act + c;
    printf("   H_active = %u\n",h_act);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_VRESH_REG, &c);
    v_res = c;
    v_res = v_res << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_VRESL_REG, &c);
    v_res = v_res + c;
    printf("   V_resolution = %u\n",v_res);
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_ACTVIDLINEH_REG, &c);
    v_act = c;
    v_act = v_act << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_ACTVIDLINEL_REG, &c);
    v_act = v_act + c;
    printf("   V_active = %u\n",v_act);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HFORNTPORCHH_REG, &c);
    h_fp = c;
    h_fp = h_fp << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HFORNTPORCHL_REG, &c);
    h_fp = h_fp + c;
    printf("   H_FP = %u\n",h_fp);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HBACKPORCHH_REG, &c);
    h_bp = c;
    h_bp = h_bp << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HBACKPORCHL_REG, &c);
    h_bp = h_bp + c;
    printf("   H_BP = %u\n",h_bp);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HSYNCWIDH_REG, &c);
    hsync_width = c;
    hsync_width = hsync_width << 8;
    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_HSYNCWIDL_REG, &c);
    hsync_width = hsync_width + c;
    printf("   Hsync_width = %u\n",hsync_width);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_ACTLINE2VSYNC_REG, &c);
    printf("   Vsync_FP = %u\n",c);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_VSYNCTAIL2VIDLINE_REG, &c);
    printf("   Vsync_BP = %u\n",c);

    ANX9030_i2c_read_p0_reg(ANX9030_VIDF_VSYNCWIDLINE_REG, &c);
    printf("   Vsync_width = %u\n",c);

    debug_puts("");

  
    {
        printf("   Normal mode output video format is ");
        switch(anx9030_video_timing_id)
        {
        case anx9030_V720x480p_60Hz_4x3:
        case anx9030_V720x480p_60Hz_16x9:
            printf("720x480p@60, ");
            if(anx9030_edid_result.supported_720x480p_60Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V1280x720p_60Hz:
            printf("1280x720p@60, ");
            if(anx9030_edid_result.supported_720p_60Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V1920x1080i_60Hz:
            printf("1920x1080i@60, ");
            if(anx9030_edid_result.supported_1080i_60Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V1280x720p_50Hz:
            printf("1280x720p@50, ");
            if(anx9030_edid_result.supported_720p_50Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V1920x1080i_50Hz:
            printf("1920x1080i@50, ");
            if(anx9030_edid_result.supported_1080i_50Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V720x576p_50Hz_4x3:
        case anx9030_V720x576p_50Hz_16x9:
            printf("720x576p@50, ");
            if(anx9030_edid_result.supported_576p_50Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V720x576i_50Hz_4x3:
        case anx9030_V720x576i_50Hz_16x9:
            printf("720x576i@50, ");
            if(anx9030_edid_result.supported_576i_50Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V720x480i_60Hz_4x3:
        case anx9030_V720x480i_60Hz_16x9:
            printf("720x480i@60, ");
            if(anx9030_edid_result.supported_720x480i_60Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
        case anx9030_V1920x1080p_24Hz:
            printf("1920x1080p@24, ");
            if(anx9030_edid_result.supported_1080p_24Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;
 
        case anx9030_V1920x1080p_25Hz:
            printf("1920x1080p@25, ");
            if(anx9030_edid_result.supported_1080p_25Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;

        case anx9030_V1920x1080p_30Hz:
            printf("1920x1080p@30, ");
            if(anx9030_edid_result.supported_1080p_30Hz)
                puts("and sink supports this format.");
            else
                puts("but sink does not support this format.");
            break;

        default:
            puts("unknown.");
            break;
        }
    }
    puts("**********************************ANX9030 Info*************************************");
}

//********************************************end video config*******************************



/**************************************************************
 *void ANX9030_Config_Audio(void)
 **************************************************************/
BYTE ANX9030_Config_Audio(void)
{
    BYTE exe_result = 0x00;
    BYTE c = 0x00;
    BYTE audio_layout = 0x00;
    BYTE fs = 0x00;
    WORD ACR_N = 0x0000;

	
    ANX9030_i2c_read_p0_reg(ANX9030_VID_STATUS_REG, &c);
    if((c & ANX9030_VID_STATUS_TYPE) == 0x04)
        { debug_printf("   Video Type = Interlace\n");

    	}
    else
    	{
         debug_printf("   Video Type = Progressive\n");

    	}
	ANX9030_i2c_read_p0_reg(ANX9030_VID_CAPCTRL1_REG, &c);
    if((c & 0x8) == 0x8)
        { debug_printf("   Video Type = Interlace\n");

    	}
    else
        { debug_printf("   Video Type = Progressive\n");

    	}

	
    //set audio clock edge
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG,  (c & 0xf7) | anx9030_audio_clock_edge);
    //cts get select
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
    c &=0xef;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG,  c);
    printf("audio_type = 0x%.2x\n",(WORD)s_anx9030_audio_config.audio_type);
    if (s_anx9030_audio_config.audio_type & ANX9030_i2s_input)
    {
        exe_result |= ANX9030_Config_I2s();
    }
//delete by fxd//open for guangxi 
    else
    {
        //disable I2S audio input
        //printf("ANX9030: disable I2S audio input.");
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
        c &= 0xc3;
        ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);
    }

    if (s_anx9030_audio_config.audio_type & ANX9030_spdif_input)
    {
        exe_result |= ANX9030_Config_Spdif();
    }
    else
    {
        //disable SPDIF audio input
        //printf("ANX9030: disable SPDIF audio input.");
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
        c &= ~ANX9030_HDMI_AUDCTRL1_SPDIFIN_EN;
        ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);

    }

    if (s_anx9030_audio_config.audio_type & ANX9030_super_audio_input)
    {
        exe_result |= ANX9030_Config_Super_Audio();
    }
    else
    {
        //disable super audio output
        //printf("ANX9030: disable super audio output.");
        ANX9030_i2c_write_p0_reg(ANX9030_ONEBIT_AUD_CTRL_REG, 0x00);
    }

    if ((s_anx9030_audio_config.audio_type & 0x07) == 0x00)
    {
        debug_printf("ANX9030 input no audio type.");
    }
    //delete by fxd

    //audio layout
    if (s_anx9030_audio_config.audio_type & ANX9030_i2s_input)
    {
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
        if( c & 0x38)       //BIT[5:3]
        {
            audio_layout = 0x80;
        }
    }
/*
    if (s_anx9030_audio_config.audio_type & ANX9030_super_audio_input)
    {
        ANX9030_i2c_read_p0_reg(ANX9030_ONEBIT_AUD_CTRL_REG, &c);
        if( c & 0xfc)       //BIT[5:3]
        {
            audio_layout = 0x80;
        }
    }
*/ //delete by fxd
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
    c |= audio_layout;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, c);

    if(  (s_anx9030_audio_config.audio_type & 0x07) == exe_result )
    {
        //Initial N value

        ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS4_REG, &c);
        fs = c & 0x0f;
        // set default value to N
        ACR_N = ANX9030_N_48k;
        switch(fs)
        {
        case(0x00)://44.1k
            ACR_N = ANX9030_N_44k;
            break;
        case(0x02)://48k
            ACR_N = ANX9030_N_48k;
            break;
        case(0x03)://32k
            ACR_N = ANX9030_N_32k;
            break;
        case(0x08)://88k
            ACR_N = ANX9030_N_88k;
            break;
        case(0x0a)://96k
            ACR_N = ANX9030_N_96k;
            break;
        case(0x0c)://176k
            ACR_N = ANX9030_N_176k;
            break;
        case(0x0e)://192k
            ACR_N = ANX9030_N_192k;
            break;
        default:
            //printf("note wrong fs.");
            break;
        }
        // write N(ACR) to corresponding regs
        //ACR_N = ANX9030_N_44k; //temp added by fxd
        c = ACR_N;
        ANX9030_i2c_write_p1_reg(ANX9030_ACR_N1_SW_REG, c);
        c = ACR_N>>8;
        ANX9030_i2c_write_p1_reg(ANX9030_ACR_N2_SW_REG, c);

        ANX9030_i2c_write_p1_reg(ANX9030_ACR_N3_SW_REG, 0x00);

        // set the relation of MCLK and Fs  xy 070117
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, (c & 0xf8) | FREQ_MCLK);
        printf("Audio MCLK input mode is: %.2x\n",(WORD)FREQ_MCLK);

        //Enable control of ACR
        ANX9030_i2c_read_p1_reg(ANX9030_INFO_PKTCTRL1_REG, &c);
        ANX9030_i2c_write_p1_reg(ANX9030_INFO_PKTCTRL1_REG, (c | ANX9030_INFO_PKTCTRL1_ACR_EN));

        //audio enable:
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
        c |= ANX9030_HDMI_AUDCTRL1_IN_EN;
        ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);

        ANX9030_Set_System_State(ANX9030_CONFIG_PACKETS);
    }

    return exe_result;
}

/**************************************************************
 *void ANX9030_Config_I2s(void)
 **************************************************************/
BYTE ANX9030_Config_I2s(void)
{
    BYTE exe_result = 0x00;
    BYTE c = 0x00;
    BYTE c1 = 0x00;

    printf("ANX9030 Audio: config i2s \n");

    //select SCK as source
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
    c &=  ~ANX9030_HDMI_AUDCTRL1_CLK_SEL;
    debug_printf("select SCK as source, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);
	

    //config i2s channel
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
    c1 = s_anx9030_audio_config.i2s_config.audio_channel;    // need BYTE[5:2]
    c1 &= 0x3c;
    c &= ~0x3c;
    c |= c1;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);
    debug_printf("config i2s channel, c = 0x%.2x\n",(WORD)c);

    //config i2s format
    //ANX9030_i2c_read_p0_reg(ANX9030_I2S_CTRL_REG, &c);
    c = s_anx9030_audio_config.i2s_config.i2s_format;
    ANX9030_i2c_write_p0_reg(ANX9030_I2S_CTRL_REG, c);
    debug_printf("config i2s format, c = 0x%.2x\n",(WORD)c);
	delay_ms(30);
    //map i2s fifo

    // TODO: config I2S channel map register according to system

    //ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_CTRL_REG, c);

    //swap right/left channel
    /*ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_SWCTRL_REG, &c);
      c1 = 0x00;
      c1 &= 0xf0;
      c &= ~0xf0;
      c |= c1;
      ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_SWCTRL_REG, c);
      printf("map i2s ffio, c = 0x%.2x\n",(WORD)c);*/

    //down sample
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
    c1 = s_anx9030_audio_config.down_sample;
    c1 &= 0x60;
    c &= ~0x60;
    c |= c1;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, c);
    debug_printf("down sample, c = 0x%.2x\n",(WORD)c);
	delay_ms(10);
    //config i2s channel status(5 regs)
    c = s_anx9030_audio_config.i2s_config.Channel_status1;
	debug_printf("s_anx9030_audio_config.i2s_config.Channel_status1, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS1_REG, c);
	ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS1_REG, &c);
	debug_printf("i2s_config.Channel_status1, c = 0x%.2x\n",(WORD)c);
	
	c = s_anx9030_audio_config.i2s_config.Channel_status2;
	debug_printf("s_anx9030_audio_config.i2s_config.Channel_status2, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS2_REG, c);
	ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS2_REG, &c);
	debug_printf("i2s_config.Channel_status2, c = 0x%.2x\n",(WORD)c);


    c = s_anx9030_audio_config.i2s_config.Channel_status3;
	debug_printf("s_anx9030_audio_config.i2s_config.Channel_status3, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS3_REG, c);
	ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS3_REG, &c);
	debug_printf("i2s_config.Channel_status3, c = 0x%.2x\n",(WORD)c);


    c = s_anx9030_audio_config.i2s_config.Channel_status4;
	debug_printf("s_anx9030_audio_config.i2s_config.Channel_status4, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS4_REG, c);
	ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS4_REG, &c);
	debug_printf("i2s_config.Channel_status4, c = 0x%.2x\n",(WORD)c);


    c = s_anx9030_audio_config.i2s_config.Channel_status5;
	debug_printf("config i2s channel status5, c = 0x%.2x\n",(WORD)c);
    ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS5_REG, c);
	ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS5_REG, &c);
	debug_printf("i2s_config.Channel_status5, c = 0x%.2x\n",(WORD)c);

	
	
	delay_ms(30);
    exe_result = ANX9030_i2s_input;
    //printf("return = 0x%.2x\n",(WORD)exe_result);

    // open corresponding interrupt
    ANX9030_i2c_read_p0_reg(ANX9030_INTR1_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR1_MASK_REG, (c | 0x22) );
    ANX9030_i2c_read_p0_reg(ANX9030_INTR3_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR3_MASK_REG, (c | 0x20) );

    return exe_result;
}

#if 1 //delete by fxd
/**************************************************************
 *void ANX9030_Config_Spdif(void)
 **************************************************************/
BYTE ANX9030_Config_Spdif(void)
{
    BYTE exe_result = 0x00;
    BYTE c = 0x00;
    BYTE c1 = 0x00;


    printf("ANX9030: config SPDIF audio.");


    //Select MCLK
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
    c |= ANX9030_HDMI_AUDCTRL1_CLK_SEL;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);

    //printf("ANX9030: enable SPDIF audio.");
    //Enable SPDIF
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
    c |= ANX9030_HDMI_AUDCTRL1_SPDIFIN_EN;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);

    //adjust MCLK phase in interrupt routine

    // adjust FS_FREQ   //FS_FREQ
    c1 = s_anx9030_audio_config.i2s_config.Channel_status4 & 0x0f;
    ANX9030_i2c_read_p0_reg(ANX9030_SPDIFCH_STATUS_REG, &c);
    c &= ANX9030_SPDIFCH_STATUS_FS_FREG;
    c = c >> 4;

    if( c != c1)
    {
        //printf("adjust FS_FREQ by system!");
        ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS4_REG, &c);
        c &= 0xf0;
        c |= c1;
        ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS4_REG, c);

        //enable using FS_FREQ from 0x59
        ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
        c |= 0x02;
        ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);
    }

    // down sample
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, &c);
    c1 = s_anx9030_audio_config.down_sample;
    c1 &= 0x60;
    c &= ~0x60;
    c |= c1;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL0_REG, c);

    if(s_anx9030_audio_config.down_sample)      //zy 060816
    {
        // adjust FS_FREQ by system because down sample
        //printf("adjust FS_FREQ by system because down sample!");

        c1 = s_anx9030_audio_config.i2s_config.Channel_status4 & 0x0f;;
        ANX9030_i2c_read_p0_reg(ANX9030_I2SCH_STATUS4_REG, &c);
        c &= 0xf0;
        c |= c1;
        ANX9030_i2c_write_p0_reg(ANX9030_I2SCH_STATUS4_REG, c);
    }


    // spdif is stable
    printf("config SPDIF audio done");
    exe_result = ANX9030_spdif_input;

    // open corresponding interrupt
    ANX9030_i2c_read_p0_reg(ANX9030_INTR1_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR1_MASK_REG, (c | 0x32) );
    ANX9030_i2c_read_p0_reg(ANX9030_INTR3_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR3_MASK_REG, (c | 0xa1) );
    return exe_result;
}

/**************************************************************
 *void ANX9030_Config_Super_Audio(void)
 **************************************************************/
BYTE ANX9030_Config_Super_Audio(void)
{
    BYTE exe_result = 0x00;
    BYTE c = 0x00;


    printf("ANX9030_Config_Super_Audio:::ANX9030: config one BIT audio.\n");

    // select sck as source
    ANX9030_i2c_read_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, &c);
    c &=  ~ANX9030_HDMI_AUDCTRL1_CLK_SEL;
    ANX9030_i2c_write_p0_reg(ANX9030_HDMI_AUDCTRL1_REG, c);

    // Enable stream  0x60
    c = s_anx9030_audio_config.super_audio_config.one_BIT_ctrl;
    ANX9030_i2c_write_p0_reg(ANX9030_ONEBIT_AUD_CTRL_REG, c);


    // Map stream 0x61
    // TODO: config super audio  map register according to system

    exe_result = ANX9030_super_audio_input;
    return exe_result;

}

#endif 


//*************** Config Packet ****************************
BYTE ANX9030_Config_Packet()
{
    BYTE exe_result = 0x00;     // There is no use in current solution
    BYTE info_packet_sel;
    // BYTE type_sel;
    BYTE c;

    
    info_packet_sel = s_anx9030_packet_config.packets_need_config;
    printf("info_packet_sel = 0x%.2x\n",(WORD) info_packet_sel);
    // New packet?
    if( info_packet_sel != 0x00)
    { 
        // avi infoframe
        if( info_packet_sel & ANX9030_avi_sel )
        {  
            c = s_anx9030_packet_config.avi_info.pb_byte[1];  //color space
            c &= 0x9f;
            c |= (anx9030_RGBorYCbCr << 5);
            s_anx9030_packet_config.avi_info.pb_byte[1] = c | 0x10;
            c = s_anx9030_packet_config.avi_info.pb_byte[5]; //repeat times
            c = c & 0xf0;
            c |= (anx9030_tx_pix_rpt & 0x0f);
            s_anx9030_packet_config.avi_info.pb_byte[5] = c;
            debug_puts("config avi infoframe packet."); 
            // Disable repeater
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);

            c &= ~ANX9030_INFO_PKTCTRL1_AVI_RPT;
            hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, c);

            // Enable?wait:go
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
            if(c & ANX9030_INFO_PKTCTRL1_AVI_EN)
            {
                //debug_puts("wait disable, config avi infoframe packet.");
                return exe_result;
            }

            // load packet data to regs
            ANX9030_Load_Infoframe( ANX9030_avi_infoframe,
                                    &(s_anx9030_packet_config.avi_info));
            // Enable and repeater
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
            c |= 0x30;
           	//c |= 0x38;
           	//c|=0x20;
            hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, c);

            // complete avi packet
            puts("config avi infoframe packet done.");
            s_anx9030_packet_config.packets_need_config &= ~ANX9030_avi_sel;
            
        }
       //s_anx9030_packet_config.packets_need_config &= ~ANX9030_avi_sel; 
        // audio infoframe
        if( info_packet_sel & ANX9030_audio_sel )
        {
            debug_puts("config audio infoframe packet.");

            // Disable repeater
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
            c &= ~ANX9030_INFO_PKTCTRL2_AIF_RPT;
            hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
            
            // Enable?wait:go
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
            if(c & ANX9030_INFO_PKTCTRL2_AIF_EN)
            {
                //debug_puts("wait disable, config audio infoframe packet.");
                return exe_result;
            }
            // config packet

            // load packet data to regs
            ANX9030_Load_Infoframe( ANX9030_audio_infoframe,
                                    &(s_anx9030_packet_config.audio_info));
            // Enable and repeater
            hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
            c |= 0x03;
        //	c |= 0x
            hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);

            // complete avi packet

            debug_puts("config audio infoframe packet done.");
            s_anx9030_packet_config.packets_need_config &= ~ANX9030_audio_sel;
            
        }

        // config other 4 packets
/*
        
if( info_packet_sel & 0xfc )
{
debug_puts("other packets.");

//find the current type need config
if(info_packet_sel & ANX9030_spd_sel)    type_sel = ANX9030_spd_sel;
else if(info_packet_sel & ANX9030_mpeg_sel)    type_sel = ANX9030_mpeg_sel;
else if(info_packet_sel & ANX9030_acp_sel)    type_sel = ANX9030_acp_sel;
else if(info_packet_sel & ANX9030_isrc1_sel)    type_sel = ANX9030_isrc1_sel;
else if(info_packet_sel & ANX9030_isrc2_sel)    type_sel = ANX9030_isrc2_sel;
else  type_sel = ANX9030_vendor_sel;
            

// Disable repeater
hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
c &= ~ANX9030_INFO_PKTCTRL2_AIF_RPT;
hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);

switch(type_sel)
{
 case ANX9030_spd_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL1_SPD_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, c);

     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL1_SPD_EN)
     {
         debug_puts("wait disable, config spd infoframe packet.");
         return exe_result;
     }
     break;
                    
 case ANX9030_mpeg_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL2_MPEG_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
                    
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL2_MPEG_EN)
     {
         debug_puts("wait disable, config mpeg infoframe packet.");
         return exe_result;
     }
     break;
                    
 case ANX9030_acp_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL2_UD0_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);

     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL2_UD0_EN)
     {
         debug_puts("wait disable, config mpeg infoframe packet.");
         return exe_result;
     }
     break;
                    
 case ANX9030_isrc1_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL2_UD0_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL2_UD0_EN)
     {
         debug_puts("wait disable, config isrc1 packet.");
         return exe_result;
     }
     break;
                    
 case ANX9030_isrc2_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL2_UD1_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL2_UD1_EN)
     {
         debug_puts("wait disable, config isrc2 packet.");
         return exe_result;
     }
     break;
                    
 case ANX9030_vendor_sel:
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c &= ~ANX9030_INFO_PKTCTRL2_UD1_RPT;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     if(c & ANX9030_INFO_PKTCTRL2_UD1_EN)
     {
         debug_puts("wait disable, config vendor packet.");
         return exe_result;
     }
     break;
                    
 default : break;
}   
                
      
// config packet
// TODO: config packet in top level

// load packet data to regs
 switch(type_sel)
 {
 case ANX9030_spd_sel:
     ANX9030_Load_Infoframe( ANX9030_spd_infoframe,
                             &(s_anx9030_packet_config.spd_info));
     debug_puts("config spd done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, &c,1);
     c |= 0xc0;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL1_REG, c);
     break;
                    
 case ANX9030_mpeg_sel:
     ANX9030_Load_Infoframe( ANX9030_mpeg_infoframe,
                             &(s_anx9030_packet_config.mpeg_info));
     debug_puts("config mpeg done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c |= 0x0c;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     break;
                    
 case ANX9030_acp_sel:
     ANX9030_Load_Packet( ANX9030_acp_packet,
                          &(s_anx9030_packet_config.acp_pkt));
     debug_puts("config acp done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c |= 0x30;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     break;
                    
 case ANX9030_isrc1_sel:
     ANX9030_Load_Packet( ANX9030_isrc1_packet,
                          &(s_anx9030_packet_config.acp_pkt));
     debug_puts("config isrc1 done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c |= 0x30;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     break;
                    
 case ANX9030_isrc2_sel:
     ANX9030_Load_Packet( ANX9030_isrc2_packet,
                          &(s_anx9030_packet_config.acp_pkt));
     debug_puts("config isrc2 done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c |= 0xc0;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     break;
                    
 case ANX9030_vendor_sel:
     ANX9030_Load_Infoframe( ANX9030_vendor_infoframe,
                             &(s_anx9030_packet_config.vendor_info));
     debug_puts("config vendor done.");
     hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
     c |= 0xc0;
     hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);
     break;
                    
 default : break;
 }   

 // Enable and repeater
 hdmi_i2c_read(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, &c,1);
 c |= 0x03;
 hdmi_i2c_write_byte(ANX9030_ADDR2, ANX9030_INFO_PKTCTRL2_REG, c);

 // complete config packet
 debug_puts("config other packets done.");
 s_anx9030_packet_config.packets_need_config &= ~type_sel;
            
}
*/
}


if( s_anx9030_packet_config.packets_need_config  == 0x00)
{
    debug_printf("config packets done\n");
    ANX9030_Set_System_State(ANX9030_HDCP_AUTHENTICATION);

	ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, &c);
	ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, (c | ANX9030_TMDS_CLKCH_MUTE));
}
return exe_result;
}



BYTE ANX9030_Load_Infoframe(packet_type member,
                            infoframe_struct *p)
{
    BYTE exe_result = 0x00;
    BYTE address[8] = {0x00,0x20,0x40,0x60,0x80,0x80,0xa0,0xa0};
    BYTE i;
    BYTE c;

    p->pb_byte[0] = ANX9030_Checksum(p);        

    // write infoframe to according regs
    hdmi_i2c_write_byte(ANX9030_ADDR2, address[member], p->type);
    hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+1, p->version);
    hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+2, p->length);
    
    for(i=0; i <= p->length; i++)
    {
        hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+3+i, p->pb_byte[i]);
        hdmi_i2c_read(ANX9030_ADDR2,address[member]+3+i, &c,1);
    }
    return exe_result;
}

BYTE ANX9030_Checksum(infoframe_struct *p)
{
    BYTE checksum = 0x00;
    BYTE i;

    checksum = p->type + p->length + p->version;
    for(i=1; i <= p->length; i++)
    {
        checksum += p->pb_byte[i];
    }
    checksum = ~checksum;
    checksum += 0x01;

    return checksum;
}

/*
  BYTE ANX9030_Load_Packet(packet_type member,
  infoframe_struct *p)
  {
  BYTE exe_result = 0x00;
  BYTE address[8] = {0x00,0x20,0x40,0x60,0x80,0x80,0xa0,0xa0};
  BYTE i;

  debug_printf("address  = 0x%.2x\n",(WORD) address[member]);

  // write packet to according regs
  hdmi_i2c_write_byte(ANX9030_ADDR2, address[member], p->type);

  hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+1, p->version);
    
  hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+2, p->length);
    
  for(i=0; i < 28; i++)
  {
  hdmi_i2c_write_byte(ANX9030_ADDR2, address[member]+3+i, p->pb_byte[i]);
  }
  return exe_result;
  }
*/
//***************  end of Config Packet ****************************

/**************************************************************
 * ANX9030_HDCP_Process
 **************************************************************/
void ANX9030_HDCP_Process(void)
{
    BYTE c;
    /*gerard.zhu*/
    Anx9030_DDC_Addr hdcp_aksv2;
 //   BYTE ddc_length = 5;
 //   BYTE aksv1[5],i;
//    Anx9030_DDC_Type ddc_type =DDC_Hdcp;
    hdcp_aksv2.dev_addr = 0x74;
    hdcp_aksv2.sgmt_addr = 0;
    hdcp_aksv2.offset_addr = 0x10;
    /*end*/

    if(anx9030_HDCP_enable)
    {
	    //HDCP_EN =1 means to do HDCP authentication,SWITCH4 = 0 means not to do HDCP authentication.

        if(anx9030_auth_fully_pass)
        {
            debug_printf("No need to auth, auth passed. ");
            if(anx9030_avmute_enable)
            {
                ANX9030_Clear_AVMute();
            }
            ANX9030_Set_System_State(ANX9030_PLAY_BACK);
            ANX9030_Show_Video_Parameter();
            return;
        }
        ANX9030_Set_AVMute();//before auth, set_avmute

        //ANX9030_i2c_read_p0_reg(ANX9030_HDCP_CTRL0_REG, &c);  xy 01.09
        //ANX9030_i2c_write_p0_reg(ANX9030_HDCP_CTRL0_REG, (c | 0x03)); xy 01.09

        if( !anx9030_hdcp_init_done )
        {
            if(anx9030_edid_result.is_HDMI)
            {
                ANX9030_Hardware_HDCP_Auth_Init();
            }
            else
            {   //DVI, disable 1.1 feature and enable HDCP two special point check
                hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL1_REG, &c);
                hdmi_i2c_write_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL1_REG,
                                   (c & ((~ANX9030_HDCP_CTRL1_HDCP11_EN) | ANX9030_LINK_CHK_12_EN)));
            }
            anx9030_hdcp_init_done = 1;

#if 0			
            //Write AKSV to Repeater to get repeater KSVFIFO ready
            for (i = 0; i < ddc_length; i++)
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_HDCP_AKSV1_REG + i, &aksv1[i]);
            }
            Anx9030_DDC_Write(hdcp_aksv2, aksv1,ddc_length,ddc_type);
#endif
        }

        //xy 01.09
        if(!anx9030_srm_checked)
        {
            if(!ANX9030_Check_KSV_SRM())
            {
                ANX9030_Blue_Screen_Enable();
                ANX9030_Clear_AVMute();
                anx9030_ksv_srm_pass = 0;
            }
            else
            {
                anx9030_ksv_srm_pass = 1;
            }
        }
        if(!anx9030_ksv_srm_pass)
        {
            printf("KSV SRM fail.");
            return;
        }

        // set SRM and KSVList valid.
        hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL0_REG, &c);
        hdmi_i2c_write_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL0_REG, (c | 0x03));

	  	//xy 01.09

        if(!anx9030_hdcp_auth_en)
            // enable hardware HDCP
        {
            ANX9030_RST_DDCChannel();
            hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL0_REG, &c);
            hdmi_i2c_write_byte(ANX9030_ADDR1,ANX9030_HDCP_CTRL0_REG, (c | ANX9030_HDCP_CTRL0_HW_AUTHEN));
            anx9030_hdcp_auth_en = 1;
        }
		/* xy 01.09
           if(!anx9030_bksv_ready)
           {
           printf("bksv is not ready.");
           //set srm check already done
           anx9030_srm_checked = 1;
           return;
           }

           printf("bksv is ready.");*/

        if(anx9030_hdcp_wait_100ms_needed)
        {
            anx9030_hdcp_wait_100ms_needed = 0;
            //set srm check already done
            anx9030_srm_checked = 1;
            //disable audio
            hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, c & (~ANX9030_HDMI_AUDCTRL1_IN_EN));
            //debug_printf("++++++++anx9030_hdcp_wait_100ms_needed----------+++++++++\n");
            usleep(110000);  /* must wait 100ms */
            //  return;      /* Why direct return ----changed by fxd */
        }

        if(anx9030_hdcp_auth_pass)
        {
            //Clear the SRM_Check_Pass BIT, then when reauthentication occurs, firmware can catch it.
            hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, c & 0xfc);
            //Enable HDCP Hardware encryption
            if(!anx9030_hdcp_encryption)
            {
                ANX9030_HDCP_Encryption_Enable();
            }
            if(anx9030_send_blue_screen)
            {
                ANX9030_Blue_Screen_Disable();
            }
            debug_printf("@@@@@@@@@@@@@@@@@@@@@@@anx9030_hdcp_auth_pass@@@@@@@@@@@@@@@@@@@@\n");

        }
        else
        {
            if(!anx9030_send_blue_screen)
            {
                debug_printf("++++++++++++send blue screen+++++++++++\n");
                ANX9030_Blue_Screen_Enable();
            }
            if(anx9030_hdcp_encryption)
            {
                ANX9030_HDCP_Encryption_Disable();
            }
            if(anx9030_avmute_enable)
            {
                //debug_printf("----------------------------clear avmute----------------\n");
                ANX9030_Clear_AVMute();
            }
            //disable audio
            hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c);
            hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, c & (~ANX9030_HDMI_AUDCTRL1_IN_EN));
            return;
        }

        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c);
        c |= ANX9030_HDMI_AUDCTRL1_IN_EN;
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, c);
        anx9030_auth_fully_pass = 1;

        ANX9030_Set_System_State(ANX9030_PLAY_BACK);

    }
    else
    {
        //printf("Hard HDCP authentication disabled");
        ANX9030_Set_System_State(ANX9030_PLAY_BACK);
    }
    if(anx9030_avmute_enable) //xy 11.06 move clear_avmute from playback_process to HDCP_process
        // to avoid auto clear_avmute when set_avmute in the top level
    {
        ANX9030_Clear_AVMute();
    }

    // enable audio // to avoid enable audio in playback state
    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c | ANX9030_HDMI_AUDCTRL1_IN_EN));

    ANX9030_Show_Video_Parameter();

}




//************************Play back process   **************************
void ANX9030_PLAYBACK_Process(void)
{
    //  BYTE c;

	 	//add xh 07-25
	if(anx9030_avmute_enable)
    {
        ANX9030_Clear_AVMute();
    }//end add
    
    if((s_anx9030_packet_config.packets_need_config != 0x00) && (anx9030_edid_result.is_HDMI == 1))
    {
        ANX9030_Set_System_State(ANX9030_CONFIG_PACKETS);
    }
	//usleep(100000);
	sleep(1);
//    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c,1);
//    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, (c | ANX9030_HDMI_AUDCTRL1_IN_EN));

    ///////////////////////////////////////////////
}//******************** end of Play back process ********************************

void ANX9030_RST_DDCChannel(void)
{
    BYTE c;
    //Reset the DDC channel
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL2_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL2_REG, (c | ANX9030_SYS_CTRL2_DDC_RST));
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL2_REG, (c & (~ANX9030_SYS_CTRL2_DDC_RST)));
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, 0x00); //abort current operation
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, 0x06);//reset I2C command
//Clear FIFO
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, 0x05);
}




/**************************************************************
 * ANX9030_Check_KSV_SRM
 **************************************************************/
BYTE ANX9030_Check_KSV_SRM(void)
{
    if(!ANX9030_BKSV_SRM()) //Check BKSV SRM, fail
    {
        puts("BKSV SRM fail.");
        return 0;
    }
    else // BKSV SRM pass
    {
        puts("BKSV SRM pass.");
        if(anx9030_hdcp_bcaps & 0x40)//repeater
        {
            ANX9030_IS_KSVFIFO_Ready();
            if(!(anx9030_hdcp_bcaps & 0x20))
            {
                puts("Repeater: KSVList not ready.");
                return 0;
            }

            //anx9030_srm_checked = 1;
            puts("Repeater: KSVList ready.");
            if(!ANX9030_IS_KSVList_VLD())//KSVList not valid
            {
                puts("Repeater: KSVList not valid.");
                return 0;
            }
            else//KSVList valid
            {
                puts("Repeater: KSVList valid.");
                return 1;
            }
        }
        else//not repeater
        {
            //anx9030_srm_checked = 1;
            return 1;
        }
    }
}

/**************************************************************
 * ANX9030_IS_KSVFIFO_Ready
 **************************************************************/
void ANX9030_IS_KSVFIFO_Ready(void)
{


    /*added by gerard.zhu*/

    Anx9030_DDC_Type DDC_type;
    Anx9030_DDC_Addr ddc_bcaps_address;
    BYTE BCaps_Nums;

    DDC_type = DDC_Hdcp;
    ddc_bcaps_address.dev_addr = HDCP_Dev_Addr;
    ddc_bcaps_address.sgmt_addr = 0;
    ddc_bcaps_address.offset_addr = HDCP_Bcaps_Offset;
    BCaps_Nums = 1;

    if (!Anx9030_DDC_Read(ddc_bcaps_address, &anx9030_hdcp_bcaps, BCaps_Nums, DDC_type)){
        printf ("!!!!anx9030_hdcp_bcaps = 0x%.2x\n", (WORD)anx9030_hdcp_bcaps);
    }

    /*end*/

}

/**************************************************************
 * ANX9030_BKSV_SRM
 **************************************************************/
BYTE ANX9030_BKSV_SRM(void)
{
/*    BYTE bksv[5];
      ANX9030_i2c_read_p0_reg(ANX9030_HDCP_BKSV1_REG, &bksv[0]);
      ANX9030_i2c_read_p0_reg(ANX9030_HDCP_BKSV2_REG, &bksv[1]);
      ANX9030_i2c_read_p0_reg(ANX9030_HDCP_BKSV3_REG, &bksv[2]);
      ANX9030_i2c_read_p0_reg(ANX9030_HDCP_BKSV4_REG, &bksv[3]);
      ANX9030_i2c_read_p0_reg(ANX9030_HDCP_BKSV5_REG, &bksv[4]);
      // TODO: Compare the bskv[] value to the revocation list to decide if this value is a illegal BKSV. This is system depended.
      //If illegal, return 0; legal, return 1. Now just return 1
      return 1;*/

    

    /*address by gerard.zhu*/
    BYTE i,j,bksv_ones_count,bksv_data[Bksv_Data_Nums] = {0};
    Anx9030_DDC_Addr bksv_ddc_addr;
    WORD bksv_length;
    Anx9030_DDC_Type ddc_type;

    i = 0;
    j = 0;
    bksv_ones_count = 0;
    bksv_ddc_addr.dev_addr = HDCP_Dev_Addr;
    bksv_ddc_addr.sgmt_addr = 0;
    bksv_ddc_addr.offset_addr = HDCP_Bksv_Offset;
    bksv_length = Bksv_Data_Nums;
    ddc_type = DDC_Hdcp;

    if (!Anx9030_DDC_Read(bksv_ddc_addr, bksv_data, bksv_length, ddc_type)){
        /*Judge validity for Bksv*/
        while (i < Bksv_Data_Nums){
            while (j < 8){
                if (((bksv_data[i] >> j) & 0x01) == 1){
                    bksv_ones_count++;
                }
                j++;
            }
            i++;
            j = 0;
        }
        if (bksv_ones_count != 20){
            printf ("!!!!BKSV 1s ¡Ù20\n");
            return 0;
        }
    }
    /*end*/

    puts("bksv is ready.");
    // TODO: Compare the bskv[]lue to the revocation list to decide if thislue is a illegal BKSV. This is system depended.
    //If illegal, return 0; legal, return 1. Now just return 1
    return 1;
}


/**************************************************************
 * ANX9030_IS_KSVList_VLD
 **************************************************************/
BYTE ANX9030_IS_KSVList_VLD(void)
{
    //BYTE ksvlist;

    /*added by gerard.zhu*/
    unsigned int bstatus_nums,Repeater_Ksvs_Nums,i;
    Anx9030_DDC_Type DDC_Type;
    Anx9030_DDC_Addr ddc_bstatus_address,ddc_ksv_fifo_address;
    BYTE Ksvs_Data[ksvs_data_nums] = {0};/*50 bytes by default*/
    //WORD Ksv_length;

    DDC_Type = DDC_Hdcp;
    bstatus_nums = 2;
    ddc_bstatus_address.dev_addr = HDCP_Dev_Addr;
    ddc_bstatus_address.sgmt_addr = 0;
    ddc_bstatus_address.offset_addr = HDCP_Bstatus_offset;


    Repeater_Ksvs_Nums = 0;
    ddc_ksv_fifo_address.dev_addr = HDCP_Dev_Addr;
    ddc_ksv_fifo_address.sgmt_addr = 0;
    ddc_ksv_fifo_address.offset_addr = HDCP_Ksv_Fifo_Offset;
    /*end*/

#if 0/*modify by gerard.zhu*/
    ANX9030_InitDDC_Read(0x74, 0x00, 0x41, 0x02, 0x00);
    delay_ms(5);
    ANX9030_i2c_read_p0_reg(ANX9030_DDC_FIFO_ACC_REG, &anx9030_hdcp_bstatus[0]);
    ANX9030_i2c_read_p0_reg(ANX9030_DDC_FIFO_ACC_REG, &anx9030_hdcp_bstatus[1]);
    if((anx9030_hdcp_bstatus[0] & 0x80) | (anx9030_hdcp_bstatus[1] & 0x08))
        return 0;//HDCP topology error. More than 127 RX are attached or more than seven levels of repeater are cascaded.

    Ksv_length = 5 * (anx9030_hdcp_bstatus[0] & 0x7f);

    ANX9030_InitDDC_Read(0x74,0x00, 0x43, (BYTE)(Ksv_length & 0xff), (BYTE)((Ksv_length >> 8) & 0xff));
    delay_ms(3 * Ksv_length);
    for(i = 0; i < Ksv_length; i ++)//HDCP device_count = anx9030_hdcp_bstatus[0] & 0x7f
    {
        ANX9030_i2c_read_p0_reg(ANX9030_DDC_FIFO_ACC_REG, &ksvlist);//read one byte from ddc_fifo
        printf("KSVList[%.2x] = 0x%.2x\n",(WORD)i,(WORD)ksvlist);
        // TODO: Save BKSVList to memory
        //ANX9030_i2c_write_p0_reg(ANX9030_DDC_ACC_CMD_REG, 0x01);//set DDC_ACC_CMD to read the next byte

    }
    //delay_ms(3000);
    // TODO: Compare the bskvlist[] value to the revocation list to decide if this value is a illegal BKSVList.
    //If illegal, return 0; legal, return 1. Now just return 1
#endif

    /*added by gerard.zhu*/
    /*start*/
#if 1
    if (!Anx9030_DDC_Read(ddc_bstatus_address, anx9030_hdcp_bstatus, bstatus_nums, DDC_Hdcp)){
        if((anx9030_hdcp_bstatus[0] & 0x80) | (anx9030_hdcp_bstatus[1] & 0x08)){
            return 0;
        }
    }

    Repeater_Ksvs_Nums = (anx9030_hdcp_bstatus[0] & 0x7f) * 5;/*Device_Count * 5*/

    if (!Anx9030_DDC_Read(ddc_ksv_fifo_address, Ksvs_Data, Repeater_Ksvs_Nums, DDC_Type)){
        /*report for ksvs value*/
        for (i = 0; i < Repeater_Ksvs_Nums; i++){
            printf ("!!!!KSVList[%.2d] = 0x%.2x\n", (WORD)i,(WORD)Ksvs_Data[i]);
            // TODO: Save BKSVList to memory
        }
    }
    // TODO: Compare the bskvlist[] value to the revocation list to decide if this value is a illegal BKSVList.
#endif
    /*end*/

    return 1;
}


/**************************************************************
 * ANX9030_Hardware_HDCP_Auth_Init
 **************************************************************/
void ANX9030_Hardware_HDCP_Auth_Init(void)
{
    BYTE c;
    /*added by gerard.zhu*/
#if 1
    Anx9030_DDC_Type DDC_type;
    Anx9030_DDC_Addr ddc_bcaps_address;
    BYTE BCaps_Nums;
#endif
    /*end*/

    // disable hdcp
    ANX9030_i2c_read_p0_reg(ANX9030_HDCP_CTRL0_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));

    /*deleted by gerard.zhu*/
#if 0
    // DDC reset
    ANX9030_RST_DDCChannel();

    ANX9030_InitDDC_Read(0x74, 0x00, 0x40, 0x01, 0x00);
    delay_ms(5);
    ANX9030_i2c_read_p0_reg(ANX9030_DDC_FIFO_ACC_REG, &anx9030_hdcp_bcaps);
    printf("anx9030_hdcp_bcaps = 0x%.2x\n",
           (WORD)anx9030_hdcp_bcaps);
#endif

    /*added by gerard.zhu*/
#if 1
    DDC_type = DDC_Hdcp;
    ddc_bcaps_address.dev_addr = HDCP_Dev_Addr;
    ddc_bcaps_address.sgmt_addr = 0;
    ddc_bcaps_address.offset_addr = HDCP_Bcaps_Offset;
    BCaps_Nums = 1;

	/* Set DDC to Fast */
//	ANX9030_i2c_read_p0_reg(ANX9030_SYS_CTRL1_REG, &c);
//    ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL1_REG, (c | (ANX9030_SYS_CTRL1_DDC_FAST)));


    if (!Anx9030_DDC_Read(ddc_bcaps_address, &anx9030_hdcp_bcaps, BCaps_Nums, DDC_type)){
        printf ("!!!!anx9030_hdcp_bcaps = 0x%.2x\n", (WORD)anx9030_hdcp_bcaps);
    }
#endif
    /*end*/

    if(anx9030_hdcp_bcaps & 0x02)
    {   //enable 1.1 feature
        ANX9030_i2c_read_p0_reg(ANX9030_HDCP_CTRL1_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_HDCP_CTRL1_REG, (c |ANX9030_HDCP_CTRL1_HDCP11_EN));
    }
    else
    {   //disable 1.1 feature and enable HDCP two special point check
        ANX9030_i2c_read_p0_reg(ANX9030_HDCP_CTRL1_REG, &c);
        ANX9030_i2c_write_p0_reg(ANX9030_HDCP_CTRL1_REG,
                                 (c & ((~ANX9030_HDCP_CTRL1_HDCP11_EN) | ANX9030_LINK_CHK_12_EN)));
    }
    ANX9030_RST_DDCChannel();
    anx9030_hdcp_auth_en = 0;
}



void ANX9030_Clean_HDCP(void)
{
    BYTE c;
    //mute TMDS link
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_TMDS_CLKCH_CONFIG_REG, c & (~ANX9030_TMDS_CLKCH_MUTE));

    //Disable hardware HDCP
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, (c & (~ANX9030_HDCP_CTRL0_HW_AUTHEN)));

    //Reset HDCP logic
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SRST_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SRST_REG, (c | ANX9030_SRST_HDCP_RST) );
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SRST_REG, (c & (~ANX9030_SRST_HDCP_RST)) );
    
    //Set ReAuth
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, &c,1);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, c |ANX9030_HDCP_CTRL0_RE_AUTH);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_HDCP_CTRL0_REG, c & (~ANX9030_HDCP_CTRL0_RE_AUTH));
    anx9030_hdcp_auth_en = 0;
    anx9030_bksv_ready = 0;
    anx9030_hdcp_auth_pass = 0;
    anx9030_hdcp_auth_fail_counter =0 ;
    anx9030_hdcp_encryption = 0;
    anx9030_send_blue_screen = 0;
    anx9030_hdcp_init_done = 0;
    anx9030_hdcp_wait_100ms_needed = 1;
    anx9030_auth_fully_pass = 0;
    anx9030_srm_checked = 0;
    ANX9030_RST_DDCChannel();      
}

void ANX9030_Hardware_Reset() 
{
    hdmi_gpio_reset();
}

void ANX9030_Set_System_State(BYTE ss) 
{
    ANX9030_system_state = ss;
    printf("ANX9030 To System State: ");
    switch (ss) 
    {
    case ANX9030_INITIAL:
        puts("ANX9030_INITIAL");
        break;
    case ANX9030_WAIT_HOTPLUG: 
        puts("ANX9030_WAIT_HOTPLUG");
        break;
    case ANX9030_READ_PARSE_EDID:
        puts("ANX9030_READ_PARSE_EDID");
        break;
    case ANX9030_WAIT_RX_SENSE:
        puts("ANX9030_WAIT_RX_SENSE");
        break;
    case ANX9030_CONFIG_VIDEO:
        puts("ANX9030_CONFIG_VIDEO");
        break;
    case ANX9030_CONFIG_AUDIO:
        puts("ANX9030_CONFIG_AUDIO");
        break;
    case ANX9030_CONFIG_PACKETS:
        puts("ANX9030_CONFIG_PACKETS");
        break;		
    case ANX9030_HDCP_AUTHENTICATION:
        puts("ANX9030_HDCP_AUTHENTICATION");
        break;
        ////////////////////////////////////////////////
        // System ANX9030_RESET_LINK is kept for RX clock recovery error case, not used in normal case.
    case ANX9030_RESET_LINK:
        puts("ANX9030_RESET_LINK");
        break;
        ////////////////////////////////////////////////        
    case ANX9030_PLAY_BACK:
        puts("ANX9030_PLAY_BACK");
        break;
    }	
}



/**************************************************************
 * ANX9030_Hardware_Initial
 **************************************************************/
void ANX9030_Hardware_Initial(void)
{
    BYTE c;
	unsigned int current_format;

	
    ANX9030_Hardware_Reset();
    //Power on I2C
    ANX9030_i2c_read_p0_reg(ANX9030_SYS_CTRL3_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL3_REG, (c | ANX9030_SYS_CTRL3_I2C_PWON));

    ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL2_REG, 0x00);
    ANX9030_i2c_write_p0_reg(ANX9030_SRST_REG, 0x00);
    //clear HDCP_HPD_RST
    ANX9030_i2c_read_p0_reg(ANX9030_SYS_CTRL1_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_SYS_CTRL1_REG, c & 0xbf);
    //Power on Audio capture and Video capture module clock
    ANX9030_i2c_read_p0_reg(ANX9030_SYS_PD_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_SYS_PD_REG, (c | 0x06));

    //Initial Interrupt
    // disable video/audio CLK,Format change and before config video. 060713 xy
    ANX9030_i2c_write_p0_reg(ANX9030_INTR1_MASK_REG, 0x0c);//3
    ANX9030_i2c_read_p0_reg(ANX9030_INTR1_MASK_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR2_MASK_REG, 0x9f);
    ANX9030_i2c_write_p0_reg(ANX9030_INTR3_MASK_REG, 0x1a);

    //Enable auto set clock range for video PLL
    ANX9030_i2c_read_p0_reg(ANX9030_CHIP_CTRL_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_CHIP_CTRL_REG, (c & 0xfe));

    //Set registers value of Blue Screen when HDCP authentication failed--RGB mode,green field
    ANX9030_i2c_write_p0_reg(ANX9030_HDCP_BLUESCREEN0_REG, 0x10);
    ANX9030_i2c_write_p0_reg(ANX9030_HDCP_BLUESCREEN1_REG, 0xeb);
    ANX9030_i2c_write_p0_reg(ANX9030_HDCP_BLUESCREEN2_REG, 0x10);

    //Set registers of PLL loop bit and swing when using R_BIAS(680 Ohm)
    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH0_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH0_CONFIG_REG, (c | 0x1c));

    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH1_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH1_CONFIG_REG, (c | 0x1c));

    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH2_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH2_CONFIG_REG, (c | 0x1c));

    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CLKCH_CONFIG_REG, (c | 0x1c));

    ANX9030_i2c_read_p0_reg(ANX9030_PLL_CTRL0_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_PLL_CTRL0_REG, (c & 0xf3));

/*
    //Set Ch0~2 output pre emphasis control
    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH0_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH0_CONFIG_REG, (c | 0x01));
    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH1_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH1_CONFIG_REG, (c | 0x01));
    ANX9030_i2c_read_p0_reg(ANX9030_TMDS_CH2_CONFIG_REG, &c);
    ANX9030_i2c_write_p0_reg(ANX9030_TMDS_CH2_CONFIG_REG, (c | 0x01));
*/

    //printf("ANX9030_register initial OK!");

    /*added by gerard.zhu*/
    /*start*/
    //ANX9030_i2c_read_p0_reg(ANX9030_HDCP_DBG_CTRL_REG , &c);
    //ANX9030_i2c_write_p0_reg(ANX9030_HDCP_DBG_CTRL_REG , (c & 0xfe));
   /*end*/
	current_format = HDMI_GetTimingMode();
  	debug_printf("ANX9030_System_Config() Debug: current_format = %d\n",current_format );
  	HDMI_SetTimingMode((BYTE)current_format);	
    ANX9030_Set_System_State(ANX9030_WAIT_HOTPLUG);
}


void ANX9030_Initial() 
{
    ANX9030_Variable_Initial(); 
	ANX9030_HW_Interface_Variable_Initial();
    ANX9030_Hardware_Initial();
}

void ANX9030_Interrupt_Information(BYTE c, BYTE n) 
{
    BYTE TX_is_HDMI,c1;
    static BYTE old_state1,old_state2,old_state3;
	
    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c1);
    TX_is_HDMI = c1 & 0x02;

    switch (n) 
    {
    case 1:
		
        if (c==old_state1) 
            return;
        old_state1 = c;
#ifdef HDMI_DEBUG
        if (c & ANX9030_INTR1_STATUS_CLK_CHG)
            debug_puts("ANX9030_Int: Video input clock change detected.");
        if ((c & ANX9030_INTR1_STATUS_CTS_OVRWR) && (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Audio CTS is overwrite before sending by ACR packer.");
        if (c & ANX9030_INTR1_STATUS_HP_CHG)
            debug_puts("ANX9030_Int: Hotplug change detected.");
        if (c & ANX9030_INTR1_STATUS_SW_INT)
            debug_puts("ANX9030_Int: Software induced interrupt.");
        if ((c & ANX9030_INTR1_STATUS_SPDIF_ERR)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: S/PDIF parity errors.");
        if ((c & ANX9030_INTR1_STATUS_AFIFO_OVER)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Audio FIFO is overrun.");
        if ((c & ANX9030_INTR1_STATUS_AFIFO_UNDER)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Audio FIFO is underrun.");
        if ((c & ANX9030_INTR1_STATUS_CTS_CHG)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Audio CTS changed.");
#endif
        break;
    case 2:
			
        if (c==old_state2) 
            return;
        old_state2 = c;
#ifdef HDMI_DEBUG
        if (c & ANX9030_INTR2_STATUS_AUTH_DONE)
            debug_puts("ANX9030_Int: HDCP authentication ended.");
        if (c & ANX9030_INTR2_STATUS_AUTH_CHG)
            debug_puts("ANX9030_Int: Hardware HDCP authentication state changed.");
        if (c & ANX9030_INTR2_STATUS_SHA_DONE)
            debug_puts("ANX9030_Int: Hardware HDCP computing V ended.");
        if (c & ANX9030_INTR2_STATUS_PLLLOCK_CHG)
            debug_puts("ANX9030_Int: PLL clock state changed.");
        if (c & ANX9030_INTR2_STATUS_BKSV_RDY)
            debug_puts("ANX9030_Int: BKSV ready for check.");
        if (c & ANX9030_INTR2_STATUS_HDCPENHC_CHK)
            debug_puts("ANX9030_Int: Enhanced link verification is need.");
        if (c & ANX9030_INTR2_STATUS_HDCPLINK_CHK)
            debug_puts("ANX9030_Int: Link integrity check is need.");
        if (c & ANX9030_INTR2_STATUS_ENCEN_CHG)
            debug_puts("ANX9030_Int: ENC_EN changed detected.");
#endif
        break;
    case 3:
			
        if (c==old_state3) 
            return;
        old_state3 = c;
#ifdef HDMI_DEBUG
        if ((c & ANX9030_INTR3_STATUS_SPDIF_UNSTBL)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Not find expected preamble for SPDIF input.");
        if (c & ANX9030_INTR3_STATUS_RXSEN_CHG)
            debug_puts("ANX9030_Int: Receiver active sense changed.");
        if (c & ANX9030_INTR3_STATUS_VSYNC_DET)
            debug_puts("ANX9030_Int: VSYNC active edge detected.");
        if (c & ANX9030_INTR3_STATUS_DDC_NOACK)
            debug_puts("ANX9030_Int: DDC master not detected any ACK.");
        if (c & ANX9030_INTR3_STATUS_DDCACC_ERR)
            debug_puts("ANX9030_Int: DDC channel access error.");
        if ((c & ANX9030_INTR3_STATUS_AUDCLK_CHG)&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: Audio input clock changed.");
        if (c & ANX9030_INTR3_STATUS_VIDF_CHG)
            debug_puts("ANX9030_Int: Video input format changed.");
        if ((c & ANX9030_INTR3_STATUS_SPDIFBI_ERR )&& (TX_is_HDMI == 0x02))
            debug_puts("ANX9030_Int: SPDIF bi-phase error.");
#endif
        break;
    }
}

void ANX9030_Interrupt_Process() 
{
    BYTE c,c1;
    BYTE s1, s2, s3;

    //monitor if anx9030 has been reset wrongly. 	
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_INTR1_MASK_REG,  &c,1);
    if(c == 0)
        restart_system = 1;
    
    if(ANX9030_system_state == ANX9030_INITIAL) 
        return;

    if(ANX9030_system_state == ANX9030_WAIT_HOTPLUG)
    {
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR1_STATUS_REG, &s1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_INTR1_STATUS_REG, s1);
        if(s1 & ANX9030_INTR1_STATUS_HP_CHG)
            ANX9030_Hotplug_Change_Interrupt();
    }
    else 
    {
      
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR1_STATUS_REG, &s1);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_INTR1_STATUS_REG, s1);
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR1_MASK_REG, &c1);
		ANX9030_Interrupt_Information((s1 & c1), 1);
	
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR2_STATUS_REG, &s2);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_INTR2_STATUS_REG, s2);
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR2_MASK_REG, &c1);
       	ANX9030_Interrupt_Information((s2 & c1), 2);


        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR3_STATUS_REG, &s3);
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_INTR3_STATUS_REG, s3);
        hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_INTR3_MASK_REG, &c1);
		ANX9030_Interrupt_Information((s3 & c1), 3);

        if(s1 & ANX9030_INTR1_STATUS_HP_CHG)
            ANX9030_Hotplug_Change_Interrupt();

        if(s1 & ANX9030_INTR1_STATUS_CLK_CHG){ 
            // ANX9030_Video_Clock_Change_Interrupt(); // fxd & lixun: That is a bug of ANX9030, if audio clk changed the chip will release a vid                                                                   clock changed.
            ANX9030_Audio_CLK_Change_Interrupt();
        }
        if(s3 & ANX9030_INTR3_STATUS_VIDF_CHG) 
            ANX9030_Video_Format_Change_Interrupt();

        if(s2 & ANX9030_INTR2_STATUS_BKSV_RDY)
        {
            anx9030_bksv_ready = 1;
        }
        if(s2 & ANX9030_INTR2_STATUS_AUTH_DONE) 
            ANX9030_Auth_Done_Interrupt();

        if(s2 & ANX9030_INTR2_STATUS_AUTH_CHG) 
            ANX9030_Auth_Change_Interrupt();

        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        c = c & 0x02;          // HDMI mode
        if(c == 0x02)
        {
            if(s3 & ANX9030_INTR3_STATUS_AUDCLK_CHG)
                ANX9030_Audio_CLK_Change_Interrupt();
            if(s1 & ANX9030_INTR1_STATUS_AFIFO_OVER)
                ANX9030_AFIFO_Overrun_Interrupt();

            // SPDIF error
            if((s3 & 0x81) || (s1 & ANX9030_INTR1_STATUS_SPDIF_ERR))
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_HDMI_AUDCTRL1_REG, &c1);
                if( c1 & ANX9030_HDMI_AUDCTRL1_SPDIFIN_EN)    
                {
                    ANX9030_SPDIF_Error_Interrupt(s1,s3);
                }
            }
            else
            {
                if(spdif_error_cnt > 0 && ANX9030_system_state == ANX9030_PLAY_BACK)    spdif_error_cnt --;
                if(spdif_error_cnt > 0 && ANX9030_system_state < ANX9030_CONFIG_AUDIO)    spdif_error_cnt = 0x00;
            }
        }

        if(s2 & ANX9030_INTR2_STATUS_PLLLOCK_CHG)
            ANX9030_PllLock_Interrupt();

        if(s3 & ANX9030_INTR3_STATUS_RXSEN_CHG)
        {
            ANX9030_Rx_Sense_Interrupt(); //060819
        }

    }  
    //}
}

BYTE ANX9030_Parse_EDID(void)
{

    //BYTE c,c1,j;

    ANX9030_GetEDIDLength();

    debug_printf("EDIDLength is %.u\n",  anx9030_edid_length);

 	 //ANX9030_Read_EDID();
	ANX9030_RST_DDCChannel();
#if 0 //delete by fxd
    if(!(ANX9030_Parse_EDIDHeader()))
    {	
        debug_puts("BAD EDID Header, Stop parsing \n");
        anx9030_edid_result.edid_errcode = ANX9030_EDID_BadHeader;
        return anx9030_edid_result.edid_errcode;
    }

    if(!(ANX9030_Parse_EDIDVersion()))
    {
        debug_puts("EDID does not support 861B, Stop parsing\n");
        anx9030_edid_result.edid_errcode = ANX9030_EDID_861B_not_supported;
        return anx9030_edid_result.edid_errcode;
    }

    if(ANX9030_EDID_Checksum(0) == 0)
    {
        debug_puts("EDID Block one check sum error, Stop parsing\n");
        anx9030_edid_result.edid_errcode = ANX9030_EDID_CheckSum_ERR;
        return anx9030_edid_result.edid_errcode;
    }

    //ANX9030_Parse_BasicDis();
    ANX9030_Parse_DTDinBlockONE();

    if(ANX9030_Read_EDID_BYTE(0, 0x7e) == 0)
    {
        debug_puts("No EDID extension blocks.\n");
        anx9030_edid_result.edid_errcode = ANX9030_EDID_No_ExtBlock;
        return anx9030_edid_result.edid_errcode;
    }
#endif 

    ANX9030_Parse_ExtBlock();

    if(anx9030_edid_result.edid_errcode == 0x05)
        return anx9030_edid_result.edid_errcode;

    if(anx9030_edid_result.edid_errcode == 0x03)
        return anx9030_edid_result.edid_errcode;

    ANX9030_Parse_NativeFormat();

    debug_puts("EDID parsing finished!\n");

	return 0;
}

void ANX9030_GetEDIDLength()
{
    BYTE edid_data_length;

    ANX9030_RST_DDCChannel();

    edid_data_length = ANX9030_Read_EDID_BYTE(0x00, 0x7e);
    debug_puts("Finish reading EDID block number.\n");

    anx9030_edid_length = edid_data_length * 128 + 128;

}


/**************************************************************
 * ANX9030_Read_EDID
 **************************************************************/
void ANX9030_Read_EDID(void)
{
    BYTE edid_segment,segmentpointer,k;

    ANX9030_RST_DDCChannel();

    edid_segment = anx9030_edid_length / 256;
    if(edid_segment==0)															//wen
        segmentpointer =0;
    else
        segmentpointer = edid_segment - 1;

    //segmentpointer = edid_segment - 1;												//wen

    for(k = 0; k <= segmentpointer; k ++)
    {
		debug_printf("segmentpointer=%d, k=%d",segmentpointer,k);
		ANX9030_InitDDC_Read(0xa0, k, 0x00, 0x80, 0x00);
        ANX9030_DDC_Mass_Read(128);
        ANX9030_InitDDC_Read(0xa0, k, 0x80, 0x80, 0x00);
        ANX9030_DDC_Mass_Read(128);
    }

    if((anx9030_edid_length - 256 * edid_segment) == 0)
        debug_printf("Finish reading EDID");
    else
    {
        debug_printf("Read one more block(128 bytes).........");
        ANX9030_InitDDC_Read(0xa0, segmentpointer + 1, 0x00, 0x80, 0x00);
        ANX9030_DDC_Mass_Read(128);
        debug_printf("Finish reading EDID");
    }
}


/**************************************************************
 * ANX9030_DDC_Mass_Read
 **************************************************************/
void ANX9030_DDC_Mass_Read(WORD length)
{
    int i, j;
    BYTE c, c1,ddc_empty_cnt;

    i = length;
    while (i > 0)
    {
    	debug_printf("\ni=%d\n",i);
        //check DDC FIFO statue
        hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_CHSTATUS_REG, &c);
        if(c & ANX9030_DDC_CHSTATUS_DDC_OCCUPY)
        {
            debug_printf("ANX9030 DDC channel is accessed by an external device, break!.\n");
            break;
        }
        if(c & ANX9030_DDC_CHSTATUS_FIFO_FULL)
            anx9030_ddc_fifo_full = 1;
        else
            anx9030_ddc_fifo_full = 0;
        if(c & ANX9030_DDC_CHSTATUS_INPRO)
            anx9030_ddc_progress = 1;
        else
            anx9030_ddc_progress = 0;
        if(anx9030_ddc_fifo_full)
        {
            debug_printf("DDC FIFO is full during edid reading\n");
            hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_FIFOCNT_REG, &c);
            debug_printf("FIFO counter is %.2x\n", (WORD) c);
            i = i - c;
            for(j=0; j<c; j++)
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_FIFO_ACC_REG, &c1);
                debug_printf("EDID[0x%.2x]=0x%.2x    ", (WORD)(anx9030_edid_length-(i+c-j)), (WORD) c1);
                anx9030_ddc_fifo_full = 0;
            }
            debug_printf("\n");
        }
        else if(!anx9030_ddc_progress)
        {
            debug_printf("ANX9030 DDC FIFO access finished.\n");
            hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_FIFOCNT_REG, &c);
            debug_printf("FIFO counter is %.2x\n", (WORD) c);
            if(!c)
            {
                i =0;
                break;
            }
            i = i - c;
            for(j=0; j<c; j++)
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_FIFO_ACC_REG, &c1);
                debug_printf("EDID[0x%.2x]=0x%.2x    ", (WORD)(anx9030_edid_length-(i+c-j)), (WORD) c1);
            }
            debug_printf("\ni=%d\n", i);
        }
        else
        {
            ddc_empty_cnt = 0x00;
            for(c1=0; c1<0x0a; c1++)
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1,ANX9030_DDC_CHSTATUS_REG, &c);
                debug_printf("DDC FIFO access is progressing.");
                debug_printf("DDC Channel status is 0x%.2x\n",(WORD)c);
                if(c & ANX9030_DDC_CHSTATUS_FIFO_EMPT)
                    ddc_empty_cnt++;
                delay_ms(10);
                debug_printf("ddc_empty_cnt =  0x%.2x\n",(WORD)ddc_empty_cnt);
            }
            if(ddc_empty_cnt >= 0x0a)
            {	i = 0;
            
			}
        }
    }
}

#if 0
void ANX9030_Read_EDID(void) 
{
    BYTE c, c1,ddc_empty_cnt;
    WORD i, j;
    BYTE access_number_low,access_number_high;

    access_number_low = anx9030_edid_length & 0x00ff;
    access_number_high = anx9030_edid_length >> 8;

    ANX9030_RST_DDCChannel();
/*
  hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_CHSTATUS_REG, &c,1); 
  if(c & 0x10) 
  {
  debug_puts("At the beginning of EDID read, DDC FIFO status OK");
  }  
  else 
  {
  debug_puts("At the beginning of EDID read, DDC FIFO status ERROR");
  debug_printf("ANX9030_DDC_CHSTATUS_REG = %.2x\n",(WORD)c); 
  }
*/
    
    ANX9030_InitDDC_Read(0xa0, 0x00, 0x00, access_number_low, access_number_high);

    i = anx9030_edid_length;
    while (i > 0) 
    {
        //check DDC FIFO statue
        hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_CHSTATUS_REG, &c,1);
        if(c & ANX9030_DDC_CHSTATUS_DDC_OCCUPY)
        {
            debug_puts("ANX9030 DDC channel is accessed by an external device, break!.");
            break;
        }
        if(c & ANX9030_DDC_CHSTATUS_FIFO_FULL)
            anx9030_ddc_fifo_full = 1;
        else 
            anx9030_ddc_fifo_full = 0;
        if(c & ANX9030_DDC_CHSTATUS_INPRO)
            anx9030_ddc_progress = 1;
        else
            anx9030_ddc_progress = 0;
        if(anx9030_ddc_fifo_full) 
        {
            debug_puts("DDC FIFO is full during edid reading");
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_FIFOCNT_REG, &c,1);
            debug_printf("FIFO counter is %.2x\n", (WORD) c);
            i = i - c;
            for(j=0; j<c; j++) 
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, &c1);
                debug_printf("EDID[0x%.2x]=0x%.2x    ", (WORD)(anx9030_edid_length-(i+c-j)), (WORD) c1);
                anx9030_ddc_fifo_full = 0;
            }
            debug_puts("\n");
        } 
        else if(!anx9030_ddc_progress) 
        {
            debug_puts("ANX9030 DDC FIFO access finished.");
            hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_FIFOCNT_REG, &c,1);
            debug_printf("FIFO counter is %.2x\n", (WORD) c);
            if(!c)
            {
                i =0;
                break;
            }
            i = i - c;
            for(j=0; j<c; j++) 
            {
                hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, &c1);
                debug_printf("EDID[0x%.2x]=0x%.2x    ", (WORD)(anx9030_edid_length-(i+c-j)), (WORD) c1);
            }
            debug_printf("\ni=%d\n", i);
        } 
        else 
        {
            ddc_empty_cnt = 0x00;
            for(c1=0; c1<0x0a; c1++)
            {
                hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_CHSTATUS_REG, &c,1);
                debug_puts("DDC FIFO access is progressing.");
                debug_printf("DDC Channel status is 0x%.2x\n",(WORD)c);
                if(c & ANX9030_DDC_CHSTATUS_FIFO_EMPT)
                    ddc_empty_cnt++;
                usleep(5000);
                debug_printf("ddc_empty_cnt =  0x%.2x\n",(WORD)ddc_empty_cnt);
            }
            if(ddc_empty_cnt >= 0x0a)
                i = 0;
        }
    }  
    debug_puts("Finish reading EDID.\n");
}

#endif

BYTE ANX9030_Parse_EDIDHeader(void)
{
    BYTE i,temp;
    temp = 0;
    // the EDID header should begin with 0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00
    if((ANX9030_Read_EDID_BYTE(0, 0) == 0x00) && (ANX9030_Read_EDID_BYTE(0, 7) == 0x00))
    {
        for(i = 1; i < 7; i++)
        {
            if(ANX9030_Read_EDID_BYTE(0, i) != 0xff)
            {
                temp = 0x01;
                break;
            }
        }
    }
    else
    {
        temp = 0x01;
    }
    if(temp == 0x01)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

BYTE ANX9030_Parse_EDIDVersion(void)
{
     
    if(!((ANX9030_Read_EDID_BYTE(0, 0x12) == 1) && (ANX9030_Read_EDID_BYTE(0, 0x13) >= 3) ))
    {	
        return 0;
    }
    else
    {	
        return 1;
    }
}

BYTE ANX9030_Parse_ExtBlock()
{
    BYTE i,c;

    for(i = 0; i < ANX9030_Read_EDID_BYTE(0, 0x7e); i++)    //read in blocks
    {
        c = ANX9030_Read_EDID_BYTE(i/2, 0x80 + (0x80 * i));
        if( c == 0x02)
        {
            anx9030_ext_block_num = i + 1;
            ANX9030_Parse_DTDinExtBlock();
            ANX9030_Parse_STD();
            if(!(ANX9030_EDID_Checksum(anx9030_ext_block_num)))
            {
                anx9030_edid_result.edid_errcode = ANX9030_EDID_CheckSum_ERR;
                return anx9030_edid_result.edid_errcode;
            }
        }
        else
        {
            anx9030_edid_result.edid_errcode = ANX9030_EDID_ExtBlock_NotFor_861B;
            return anx9030_edid_result.edid_errcode;
        }
    }
	return 0;	
}

void ANX9030_Parse_DTDinBlockONE()
{
    BYTE i;
    for(i = 0; i < 18; i++)
    {
        anx9030_edid_dtd[i] = ANX9030_Read_EDID_BYTE(0, (i + 0x36));
    }
    //debug_puts("Parse the first DTD in Block one:\n");
    ANX9030_Parse_DTD();

    if((ANX9030_Read_EDID_BYTE(0, 0x48) == 0) 
       && (ANX9030_Read_EDID_BYTE(0, 0x49) == 0)
       && (ANX9030_Read_EDID_BYTE(0, 0x50) == 0))
    {
        debug_puts("the second DTD in Block one is not used to descript video timing.\n");
    }
    else
    {
        for(i = 0; i < 18; i++)
        {
            anx9030_edid_dtd[i] = ANX9030_Read_EDID_BYTE(0, (i + 0x48));
        }
        ANX9030_Parse_DTD();
    }

    if((ANX9030_Read_EDID_BYTE(0,0x5a) == 0)
       && (ANX9030_Read_EDID_BYTE(0,0x5b) == 0)
       && (ANX9030_Read_EDID_BYTE(0,0x5c) == 0))
    {
        debug_puts("the third DTD in Block one is not used to descript video timing.\n");
    }
    else
    {
        for(i = 0; i < 18; i++)
        {
            anx9030_edid_dtd[i] = ANX9030_Read_EDID_BYTE(0, (i + 0x5a));
        }
        ANX9030_Parse_DTD();
    }

    if((ANX9030_Read_EDID_BYTE(0,0x6c) == 0)
       && (ANX9030_Read_EDID_BYTE(0,0x6d) == 0)
       && (ANX9030_Read_EDID_BYTE(0,0x6e) == 0))
    {
        debug_puts("the fourth DTD in Block one is not used to descript video timing.\n");
    }
    else
    {
        for(i = 0; i < 18; i++)
        {
            anx9030_edid_dtd[i] = ANX9030_Read_EDID_BYTE(0,(i + 0x6c));
        }
        ANX9030_Parse_DTD();
    }
}

void ANX9030_Parse_DTDinExtBlock()
{
    BYTE i,DTDbeginAddr;
    DTDbeginAddr = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2, (0x02 + 0x80 * anx9030_ext_block_num))
        + 0x80 * anx9030_ext_block_num;
    while(DTDbeginAddr < (0x6c + 0x80 * anx9030_ext_block_num))
    {   
        if((ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,DTDbeginAddr) == 0)
           && (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(DTDbeginAddr + 1)) == 0)
           && (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(DTDbeginAddr + 2)) == 0))
        {
            debug_puts("this DTD in Extension Block is not used to descript video timing.\n");
        }
        else
        {
            for(i = 0; i < 18; i++)
            {
                anx9030_edid_dtd[i] = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(i + DTDbeginAddr));
            }
            debug_puts("Parse the DTD in Extension Block :\n");
            ANX9030_Parse_DTD();
        }
        DTDbeginAddr = DTDbeginAddr + 18;
    }
}

void ANX9030_Parse_DTD()
{
    WORD temp;
    unsigned long temp1,temp2;
    WORD Hresolution,Vresolution,Hblanking,Vblanking;
    WORD PixelCLK,Vtotal,H_image_size,V_image_size;
    BYTE Hz;
    //float Ratio;
    
    temp = anx9030_edid_dtd[1];
    temp = temp << 8;
    PixelCLK = temp + anx9030_edid_dtd[0];
    debug_printf("Pixel clock is 10000 * %u\n",  temp);

    temp = anx9030_edid_dtd[4];
    temp = (temp << 4) & 0x0f00;
    Hresolution = temp + anx9030_edid_dtd[2];
    debug_printf("Horizontal Active is  %u\n",  Hresolution);

    temp = anx9030_edid_dtd[4];
    temp = (temp << 8) & 0x0f00;
    Hblanking = temp + anx9030_edid_dtd[3];
    debug_printf("Horizontal Blanking is  %u\n",  temp);

    temp = anx9030_edid_dtd[7];
    temp = (temp << 4) & 0x0f00;
    Vresolution = temp + anx9030_edid_dtd[5];
    debug_printf("Vertical Active is  %u\n",  Vresolution);

    temp = anx9030_edid_dtd[7];
    temp = (temp << 8) & 0x0f00;
    Vblanking = temp + anx9030_edid_dtd[6];
    debug_printf("Vertical Blanking is  %u\n",  temp);

    temp = anx9030_edid_dtd[11];
    temp = (temp << 2) & 0x0300;
    temp = temp + anx9030_edid_dtd[8];
    debug_printf("Horizontal Sync Offset is  %u\n",  temp);

    temp = anx9030_edid_dtd[11];
    temp = (temp << 4) & 0x0300;
    temp = temp + anx9030_edid_dtd[9];
    debug_printf("Horizontal Sync Pulse is  %u\n",  temp);

    temp = anx9030_edid_dtd[11];
    temp = (temp << 2) & 0x0030;
    temp = temp + (anx9030_edid_dtd[10] >> 4);
    debug_printf("Vertical Sync Offset is  %u\n",  temp);

    temp = anx9030_edid_dtd[11];
    temp = (temp << 4) & 0x0030;
    temp = temp + (anx9030_edid_dtd[8] & 0x0f);
    debug_printf("Vertical Sync Pulse is  %u\n",  temp);

    temp = anx9030_edid_dtd[14];
    temp = (temp << 4) & 0x0f00;
    H_image_size = temp + anx9030_edid_dtd[12];
    debug_printf("Horizontal Image size is  %u\n",  temp);

    temp = anx9030_edid_dtd[14];
    temp = (temp << 8) & 0x0f00;
    V_image_size = temp + anx9030_edid_dtd[13];
    debug_printf("Vertical Image size is  %u\n",  temp);

    debug_printf("Horizontal Border is  %u\n",  anx9030_edid_dtd[15]);

    debug_printf("Vertical Border is  %u\n",  anx9030_edid_dtd[16]);
    /*
      if(anx9030_edid_dtd[17] & 0x80)
      //debug_puts(" Interlaced\n");
      else
      //debug_puts("Non-interlaced\n ");
      if(anx9030_edid_dtd[17] & 0x60)
      //debug_puts("Table 3.17 for defenition\n ");
      else
      //debug_puts("Normal Display\n ");
      if(anx9030_edid_dtd[17] & 0x10)
      //debug_puts(" Digital\n");
      else
      //debug_puts(" Analog\n");*/
    temp1 = Hresolution + Hblanking;
    Vtotal = Vresolution + Vblanking;
    temp1 = temp1 * Vtotal;
    temp2 = PixelCLK;
    temp2 = temp2 * 10000;
    Hz = temp2 / temp1;
    if((Hz == 59) || (Hz == 60))
    {
        Hz = 60;
        //debug_printf("_______________Vertical Active is  %u\n",  Vresolution);
        if(Vresolution == 540)
            anx9030_edid_result.supported_1080i_60Hz = 1;
        if(Vresolution == 720)
            anx9030_edid_result.supported_720p_60Hz = 1;
        if((Hresolution == 640) && (Vresolution == 480))
            anx9030_edid_result.supported_640x480p_60Hz = 1;
        if((Hresolution == 720) && (Vresolution == 480))
            anx9030_edid_result.supported_720x480p_60Hz = 1;
        if((Hresolution == 720) && (Vresolution == 240))
            anx9030_edid_result.supported_720x480i_60Hz = 1;
    }
    if(Hz == 50)
    {
        //debug_printf("+++++++++++++++Vertical Active is  %u\n",  Vresolution);
        if(Vresolution == 540)
            anx9030_edid_result.supported_1080i_50Hz = 1;
        if(Vresolution == 720)
            anx9030_edid_result.supported_720p_50Hz = 1;
        if(Vresolution == 576)
            anx9030_edid_result.supported_576p_50Hz = 1;
        if(Vresolution == 288)
            anx9030_edid_result.supported_576i_50Hz = 1;
    }

    if (Vresolution == 1080){
        switch (Hz){
        case 24: 
            anx9030_edid_result.supported_1080p_24Hz = 1;
            break;
        case 25:
            anx9030_edid_result.supported_1080p_25Hz = 1;
            break;
        case 29:
        case 30:
            anx9030_edid_result.supported_1080p_30Hz = 1;
            break;
        }

    }
    //debug_printf("Fresh rate :% bu Hz\n", Hz);
    //Ratio = H_image_size;
    //Ratio = Ratio / V_image_size;
    //debug_printf("Picture ratio : %f \n", Ratio);
}

/*void ANX9030_Parse_BasicDis()
  {
  BYTE temp;
  temp = ANX9030_Read_EDID_BYTE(0,0x18) & 0x18;
  if(temp == 0x00)
  //debug_puts("EDID Display type: mon/gray display.\n");
  else if(temp == 0x08)
  //debug_puts("EDID Display type: RGB color display.\n");
  else if(temp == 0x10)
  //debug_puts("EDID Display type: non-RGB color display.\n");
  else 
  //debug_puts("EDID Display type: Undefined.\n");
  temp = ANX9030_Read_EDID_BYTE(0,0x18) & 0x02;
  if(temp == 0x00)
  //debug_puts("EDID Preferred_timing: not supported.\n");
  else
  //debug_puts("EDID Preferred_timing: supported.\n");
  }
*/
void ANX9030_Parse_NativeFormat()
{
    BYTE temp;
    temp = ANX9030_Read_EDID_BYTE(0,0x83) & 0xf0;
    /*if(temp & 0x80)
      ;//debug_puts("DTV supports underscan.\n");
      if(temp & 0x40)
      ;//debug_puts("DTV supports BasicAudio.\n");*/
    if(temp & 0x20)
    {
        //debug_puts("DTV supports YCbCr 4:4:4.\n");
        anx9030_edid_result.ycbcr444_supported= 1;
    }
    if(temp & 0x10)
    {
        debug_puts("DTV supports YCbCr 4:2:2.\n");
        anx9030_edid_result.ycbcr422_supported= 1;
    }
}

void ANX9030_Parse_STD()
{
    BYTE DTDbeginAddr;
    anx9030_stdaddr = 0x04 + 0x80 * anx9030_ext_block_num;
    DTDbeginAddr = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(0x02 + 0x80 * anx9030_ext_block_num));
    DTDbeginAddr = DTDbeginAddr + 0x80 * anx9030_ext_block_num;
    // debug_printf("Video DTDbeginAddr Register :%.2x\n", (WORD) DTDbeginAddr);
    while(anx9030_stdaddr < DTDbeginAddr)
    {
        anx9030_stdreg = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2, anx9030_stdaddr); 
        switch(anx9030_stdreg & 0xe0)
        {
        case 0x20:
            ANX9030_Parse_AudioSTD();
            anx9030_sau_length = anx9030_stdreg & 0x1f;
            break;
        case 0x40:
            ANX9030_Parse_VideoSTD();
            anx9030_svd_length = anx9030_stdreg & 0x1f;
            break;
        case 0x80:
            ANX9030_Parse_SpeakerSTD();
            break;
        case 0x60:
            ANX9030_Parse_VendorSTD();
            break;
        default:
            break;
        }
        anx9030_stdaddr = anx9030_stdaddr + (anx9030_stdreg & 0x1f) + 0x01;	
    }
}

void ANX9030_Parse_AudioSTD()
{
    BYTE i,AudioFormat,STDReg_tmp,STDAddr_tmp;
    STDReg_tmp = anx9030_stdreg & 0x1f;
    STDAddr_tmp = anx9030_stdaddr + 1;
    i = 0;
    while(i < STDReg_tmp)
    {
        AudioFormat = (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,STDAddr_tmp ) & 0xF8) >> 3;
        anx9030_edid_result.AudioChannel[i/3] = (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,STDAddr_tmp) & 0x07) + 1;
        anx9030_edid_result.AudioFormat[i/3] = AudioFormat;
        anx9030_edid_result.AudioFs[i/3] = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(STDAddr_tmp + 1)) & 0x7f;

        if(AudioFormat == 1)
            anx9030_edid_result.AudioLength[i/3] = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(STDAddr_tmp + 2)) & 0x07;
        else
            anx9030_edid_result.AudioLength[i/3] = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(STDAddr_tmp + 2)) << 3;

        i = i + 3;
        STDAddr_tmp = STDAddr_tmp + 3;
    }
}

void ANX9030_Parse_VideoSTD()
{
    BYTE i,STDReg_tmp,STDAddr_tmp;
    BYTE SVD_ID[34];
    STDReg_tmp = anx9030_stdreg & 0x1f;
    STDAddr_tmp = anx9030_stdaddr + 1;
    i = 0;
    while(i < STDReg_tmp)
    {
        SVD_ID[i] = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,STDAddr_tmp) & 0x7F;
        //debug_printf("anx9030_edid_result.SVD_ID[%.2x]=0x%.2x\n",(WORD)i,(WORD)anx9030_edid_result.SVD_ID[i]);
        //if(ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,STDAddr_tmp) & 0x80)
        //    debug_puts(" Native mode");
        if(SVD_ID[i] == 1)
            anx9030_edid_result.supported_640x480p_60Hz = 1;
        else if(SVD_ID[i] == 4)
            anx9030_edid_result.supported_720p_60Hz = 1;
        else if(SVD_ID[i] == 19)
            anx9030_edid_result.supported_720p_50Hz = 1;
        else if(SVD_ID[i] == 5)
            anx9030_edid_result.supported_1080i_60Hz = 1;
        else if(SVD_ID[i] == 20)
            anx9030_edid_result.supported_1080i_50Hz = 1;
        else if((SVD_ID[i] == 2) ||(SVD_ID[i] == 3))
            anx9030_edid_result.supported_720x480p_60Hz = 1;
        else if((SVD_ID[i] == 6) ||(SVD_ID[i] == 7))
            anx9030_edid_result.supported_720x480i_60Hz = 1;
        else if((SVD_ID[i] == 17) ||(SVD_ID[i] == 18))
            anx9030_edid_result.supported_576p_50Hz = 1;
        else if((SVD_ID[i] == 21) ||(SVD_ID[i] == 22))
            anx9030_edid_result.supported_576i_50Hz = 1;
        else if (SVD_ID[i] == 32)
            anx9030_edid_result.supported_1080p_24Hz = 1;
        else if (SVD_ID[i] == 33)
           anx9030_edid_result.supported_1080p_25Hz = 1;
        else if (SVD_ID[i] == 34)
           anx9030_edid_result.supported_1080p_30Hz = 1;

        i = i + 1;
        STDAddr_tmp = STDAddr_tmp + 1;
    }
}

void ANX9030_Parse_SpeakerSTD()
{
    anx9030_edid_result.SpeakerFormat = ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(anx9030_stdaddr + 1)) ;
}

void ANX9030_Parse_VendorSTD()
{
    BYTE c;
    if((ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(anx9030_stdaddr + 1)) == 0x03) 
       && (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(anx9030_stdaddr + 2)) == 0x0c) 
       && (ANX9030_Read_EDID_BYTE(anx9030_ext_block_num/2,(anx9030_stdaddr + 3)) == 0x00))
    {
        anx9030_edid_result.is_HDMI = 1;
        //hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        //hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, c |ANX9030_SYS_CTRL1_HDMI);
    }    
    else
    {
        anx9030_edid_result.is_HDMI = 0;
        //hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
        //hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, c & (~ANX9030_SYS_CTRL1_HDMI));
    }
//force to avi
    //    anx9030_edid_result.is_HDMI = 0;
    //    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, &c,1);
    //    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_SYS_CTRL1_REG, c & (~ANX9030_SYS_CTRL1_HDMI));
}

BYTE ANX9030_EDID_Checksum(BYTE block_number) 
{
    WORD i;  
    BYTE edid_block_checksum;
	
    edid_block_checksum = 0;
    for(i=(128*block_number); i<(WORD)((128*block_number)+127); i++) 
    {           
        edid_block_checksum = edid_block_checksum + ANX9030_Read_EDID_BYTE(block_number/2, i);
    }
    edid_block_checksum = (~edid_block_checksum) + 1;
    debug_printf("edid_block_checksum = 0x%.2x\n",(WORD)edid_block_checksum);
    if(ANX9030_Read_EDID_BYTE(block_number/2,((128*block_number)+127))== edid_block_checksum) 
        return 1;
    else 
	    return 0;
}    

void ANX9030_Parse_EDID_Result_For_System()
{
    BYTE i;
    //anx9030_edid_result.edid_errcode = anx9030_edid_result.edid_errcode;
    debug_printf("anx9030_edid_result.edid_errcode = 0x%.2x\n",(WORD)anx9030_edid_result.edid_errcode);
    //anx9030_edid_result.is_HDMI = anx9030_edid_result.is_HDMI;
    debug_printf("anx9030_edid_result.is_HDMI = 0x%.2x\n",(WORD)anx9030_edid_result.is_HDMI);
    //anx9030_edid_result.ycbcr422_supported = anx9030_edid_result.ycbcr422_supported;
    debug_printf("anx9030_edid_result.ycbcr422_supported = 0x%.2x\n",(WORD)anx9030_edid_result.ycbcr422_supported);
    //anx9030_edid_result.ycbcr444_supported = anx9030_edid_result.ycbcr444_supported;
    debug_printf("anx9030_edid_result.ycbcr444_supported = 0x%.2x\n",(WORD)anx9030_edid_result.ycbcr444_supported);
    //anx9030_edid_result.supported_1080i_60Hz = anx9030_edid_result.supported_1080i_60Hz;
    debug_printf("anx9030_edid_result.supported_1080i_60Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_1080i_60Hz);

    debug_printf("anx9030_edid_result.supported_1080p_24Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_1080p_24Hz);
    debug_printf("anx9030_edid_result.supported_1080p_25Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_1080p_25Hz);
    debug_printf("anx9030_edid_result.supported_1080p_30Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_1080p_30Hz);


    //anx9030_edid_result.supported_1080i_50Hz = anx9030_edid_result.supported_1080i_50Hz;
    debug_printf("anx9030_edid_result.supported_1080i_50Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_1080i_50Hz);
    //anx9030_edid_result.supported_720p_60Hz = anx9030_edid_result.supported_720p_60Hz;
    debug_printf("anx9030_edid_result.supported_720p_60Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_720p_60Hz);
    //anx9030_edid_result.supported_720p_50Hz = anx9030_edid_result.supported_720p_50Hz;
    debug_printf("anx9030_edid_result.supported_720p_50Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_720p_50Hz);
    //anx9030_edid_result.supported_640x480p_60Hz = anx9030_edid_result.supported_640x480p_60Hz;
    debug_printf("anx9030_edid_result.supported_640x480p_60Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_640x480p_60Hz); 
    //anx9030_edid_result.supported_720x480p_60Hz = anx9030_edid_result.supported_720x480p_60Hz;
    debug_printf("anx9030_edid_result.supported_720x480p_60Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_720x480p_60Hz); 
    //anx9030_edid_result.supported_720x480i_60Hz = anx9030_edid_result.supported_720x480i_60Hz;
    debug_printf("anx9030_edid_result.supported_720x480i_60Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_720x480i_60Hz);   
    //anx9030_edid_result.supported_576p_50Hz = anx9030_edid_result.supported_576p_50Hz;
    debug_printf("anx9030_edid_result.supported_576p_50Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_576p_50Hz);
    //anx9030_edid_result.supported_576i_50Hz = anx9030_edid_result.supported_576i_50Hz;
    debug_printf("anx9030_edid_result.supported_576i_50Hz = 0x%.2x\n",(WORD)anx9030_edid_result.supported_576i_50Hz);       
    if(!anx9030_edid_result.edid_errcode)
    {
        for(i = 0; i < anx9030_sau_length/3; i++)
        {
            //      anx9030_edid_result.AudioChannel[i] = anx9030_edid_result.AudioChannel[i];
            debug_printf("anx9030_edid_result.AudioChannel = 0x%.2x\n",(WORD)anx9030_edid_result.AudioChannel[i]);
            //anx9030_edid_result.AudioFormat[i] = anx9030_edid_result.AudioFormat[i];
            debug_printf("anx9030_edid_result.AudioFormat = 0x%.2x\n",(WORD)anx9030_edid_result.AudioFormat[i]);
            // anx9030_edid_result.AudioFs[i] = anx9030_edid_result.AudioFs[i];
            debug_printf("anx9030_edid_result.AudioFs = 0x%.2x\n",(WORD)anx9030_edid_result.AudioFs[i]);
            //anx9030_edid_result.AudioLength[i] = anx9030_edid_result.AudioLength[i];
            debug_printf("anx9030_edid_result.AudioLength = 0x%.2x\n",(WORD)anx9030_edid_result.AudioLength[i]);
        }
        //anx9030_edid_result.SpeakerFormat = anx9030_edid_result.SpeakerFormat;
        debug_printf("anx9030_edid_result.SpeakerFormat = 0x%.2x\n",(WORD)anx9030_edid_result.SpeakerFormat);
    }
}
void ANX9030_InitDDC_Read(BYTE devaddr, BYTE segmentpointer,
                          BYTE offset, BYTE  access_num_Low,BYTE access_num_high)
{
    //Write slave device address
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_ADDR_REG, devaddr);
//	delay_ms(1);
    // Write segment address
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_SEGADDR_REG, segmentpointer);
//	delay_ms(1);
	//Write offset
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_OFFADDR_REG, offset);
    //Write number for access
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACCNUM0_REG, access_num_Low);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACCNUM1_REG, access_num_high);
    //Clear FIFO
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, 0x05);
    //EDDC sequential Read
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, 0x04);
}

BYTE ANX9030_Read_EDID_BYTE(BYTE segmentpointer,BYTE offset)
{
    BYTE c;
    ANX9030_InitDDC_Read(0xa0, segmentpointer, offset, 0x01, 0x00);
    delay_ms(15);
    hdmi_i2c_read(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, &c,1);
    return c;
}

void ANX9030_Operation_Process () 
{
    ANX9030_Operation1(); 
    ANX9030_Operation2(); 
    ANX9030_Operation3(); 
    ANX9030_Operation4();
}




void ANX9030_Reset_AVI(void) 
{
    anx9030_avi_data[0] = 0x82; //type, HDMI defined
    anx9030_avi_data[1] = 0x02; //Version
    anx9030_avi_data[2] = 0x0d; //AVI data byte length is 13
    anx9030_avi_data[3] = 0x00; //checksum
    anx9030_avi_data[4] = 0x00; //data byte 1
    anx9030_avi_data[5] = 0x08; //data byte 2
    anx9030_avi_data[6] = 0x00; //data byte 3
    anx9030_avi_data[7] = 0x01; //data byte 4
    anx9030_avi_data[8] = 0x00; //data byte 5
    anx9030_avi_data[9] = 0x00; //data byte 6
    anx9030_avi_data[10] = 0x00; //data byte 7
    anx9030_avi_data[11] = 0x00; //data byte 8
    anx9030_avi_data[12] = 0x00; //data byte 9
    anx9030_avi_data[13] = 0x00; //data byte 10
    anx9030_avi_data[14] = 0x00; //data byte 11
    anx9030_avi_data[15] = 0x00; //data byte 12
    anx9030_avi_data[16] = 0x00; //data byte 13
    anx9030_avi_data[17] = 0x00; //for firmware use
    anx9030_avi_data[18] = 0x00; //for firmware use
}    

/*added by gerard.zhu*/
/*DDC operate start*/


/**************************************************************
 * Anx9030_DDC_Parameter_Validity
 **************************************************************/
/*Function name  :Anx9030_DDC_Parameter_Validity()*/
/*Function  :Judge the validity for input parameter*/
/*Parameter  :   Addr,length*/
/*Return  :Judge result is DDC_Data_Addr_Err,DDC_Length_Err,DDC_NO_Err*/
BYTE Anx9030_DDC_Parameter_Validity(BYTE *Addr,WORD length)
{
    if (Addr == NULL){
        return DDC_Data_Addr_Err;
    }
    else if (length > DDC_Max_Length){
        return DDC_Length_Err;
    }
    else{
        return DDC_NO_Err;
    }

}


/**************************************************************
 * Anx9030_DDC_Set_Address
 **************************************************************/
/*Function name :Anx9030_DDC_Set_Address()*/
/*Function  :Set address for DDC device*/
/*Parameter :   ddc_address,ddc_type*/
/*Return  :None*/
void Anx9030_DDC_Set_Address(Anx9030_DDC_Addr ddc_address, Anx9030_DDC_Type ddc_type)
{
    /*set DDC channel slave device address*/
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_ADDR_REG, ddc_address.dev_addr);
    /*set DDC channel slave segment address,when ddc type is edid*/
    if (ddc_type == DDC_Edid)
        hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_SEGADDR_REG, ddc_address.sgmt_addr);
    /*set DDC channel slave offset address*/
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_SLV_OFFADDR_REG, ddc_address.offset_addr);
}


/**************************************************************
 * Anx9030_DDC_Set_Number
 **************************************************************/
/*Function name :Anx9030_DDC_Set_Number()*/
/*Function  :Set number for DDC data access*/
/*Parameter :   length*/
/*Return  :None*/
void Anx9030_DDC_Set_Number(WORD length)
{
    BYTE length_low,length_high;

    printf ("DDC_data_number :%x\n",length);
    length_high = (BYTE)((length >> 8) & 0xff);
    length_low = (BYTE)(length & 0xff);
    /*set number of bytes to DDC channel*/
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACCNUM0_REG, length_low);
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACCNUM1_REG, length_high);
}


/**************************************************************
 * Anx9030_DDC_Command
 **************************************************************/
/*Function name :Anx9030_DDC_Command()*/
/*Function  :Send command to DDC*/
/*Parameter :   DDC_command*/
/*Return  :None*/
void Anx9030_DDC_Command(BYTE DDC_command)
{
    /*set DDC command*/
    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_ACC_CMD_REG, DDC_command);

}


/**************************************************************
 * Anx9030_DDC_Check_Status
 **************************************************************/
/*Function name :Anx9030_DDC_Check_Status*/
/*Function  :Check DDC status or report information of error*/
/*Parameter :   DDC_status_need_type,status_bit*/
/*Return  :always is 0 if DDC_status_need_status is "report",or return DDC status bit value
  if DDC_status_need_type is "Judge"*/
BYTE Anx9030_DDC_Check_Status(Anx9030_DDC_Status_Check_Type DDC_status_need_type,
                              BYTE status_bit)
{
    BYTE DDC_status;
	
#ifdef DEBUG
	BYTE i,j;
    BYTE *status[8] = {
        "!!!!DDC____An Error is Occurred!\n",
        "!!!!DDC____channel is accessed by an external device!\n",
        "!!!!DDC____Fifo is Full!\n",
        "!!!!DDC____Fifo is Empty!\n",
        "!!!!DDC____No Acknowledge detection!\n",
        "!!!!DDC____Fifo is being read!\n",
        "!!!!DDC____Fifo is being written!\n",
    };
#endif
    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_CHSTATUS_REG, &DDC_status);

    if (DDC_status_need_type == report){

#ifdef DEBUG
        for (i= 0,j=7; i < 8; i++,j--){
            if (DDC_status & (0x01 << i)){
                debug_printf("%s",status[j]);
            }
        }
#endif
        return 0;
    }
    else{
        return ((DDC_status >> status_bit) & 0x01 );
    }
}


/**************************************************************
 * Anx9030_DDC_Count_Compare
 **************************************************************/
/*Function name :Anx9030_DDC_Count_Compare()*/
/*Function :Check status for access count*/
/*Parameter :length*/
/*Return  :0 if check success,1 if check fail*/
BYTE Anx9030_DDC_Count_Compare(BYTE length)
{
    BYTE Fifo_Count;
    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_FIFOCNT_REG, &Fifo_Count);
    return ((Fifo_Count & 0x01f) == length ? 0 : 1);
}


/**************************************************************
 * Anx9030_DDC_Read
 **************************************************************/
/*Function name:    Anx9030_DDC_Read()*/
/*Function :    read data from DDC channel*/
/*Parameter :   ddc_address,DDC_data,length,DDC_type*/
/*Return  :Read reslut,ex.DDC_NO_Err,DDC_Status_Err*/
BYTE Anx9030_DDC_Read (Anx9030_DDC_Addr ddc_address, const BYTE *DDC_data,
                       WORD length, Anx9030_DDC_Type DDC_type)
{
    BYTE DDC_Err,fifo_read_data_compare;
    int data_cnt;
    unsigned int fifo_data_cnt;
    BYTE *Fifo_Read_P;

    DDC_Err = DDC_NO_Err;
    fifo_read_data_compare = 0;
    fifo_data_cnt = length;
    Fifo_Read_P = (BYTE *)DDC_data;

    /*Judge validity for read address and length*/
    if ((DDC_Err = Anx9030_DDC_Parameter_Validity((BYTE *)DDC_data, length))!=DDC_NO_Err)
        return (DDC_Err);

    /*set DDC address*/
    Anx9030_DDC_Set_Address(ddc_address, DDC_type);
    /*set number for DDC read*/
    Anx9030_DDC_Set_Number(length);
    /*send "clear DDC fifo" command*/
    Anx9030_DDC_Command((BYTE)Clear_DDC_Fifo);

    /*check DDC channel status*/
    if (!Anx9030_DDC_Check_Status(Judge, DDC_Error_bit) &&
        !Anx9030_DDC_Check_Status(Judge, DDC_Occup_bit) &&
        !Anx9030_DDC_Check_Status(Judge, DDC_No_Ack_bit)){
        /*send "sequential byte read"command if check success*/
        Anx9030_DDC_Command((BYTE)Sequential_Byte_Read);
        /*delay*/
        delay_ms(DDC_Read_Delay);
    }
    else{
        Anx9030_DDC_Check_Status(report, 0);
        return DDC_Status_Err;
    }

    /*read DDC fifo data*/
    do {
        /*read data from fifo if length <= DDC fifo depth*/
        if (fifo_data_cnt <= DDC_Fifo_Depth){

            fifo_read_data_compare = fifo_data_cnt;

            /*check bit DDC_Progress of DDC status,fifo count*/
            if (!Anx9030_DDC_Check_Status(Judge, DDC_Progress_bit) &&
                !Anx9030_DDC_Count_Compare(fifo_read_data_compare)){
                data_cnt = fifo_data_cnt;
                while (data_cnt--){
                    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, Fifo_Read_P++);
                }
            }
            else{
                Anx9030_DDC_Check_Status(report, 0);
                return DDC_Status_Err;
            }
        }
        /*read data from fifo if length >DDC fifo depth*/
        else{
            /*check bit DDC_Progress of DDC status,Fifo_full*/
            if (Anx9030_DDC_Check_Status(Judge, DDC_Progress_bit) &&
                Anx9030_DDC_Check_Status(Judge, DDC_Fifo_Full_bit)){
                data_cnt = DDC_Fifo_Depth;
                while(data_cnt--){
                    hdmi_i2c_read_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, Fifo_Read_P++);
                }
            }
            else{
                Anx9030_DDC_Check_Status(report, 0);
                return DDC_Status_Err;
            }

        }
        fifo_data_cnt -= DDC_Fifo_Depth;
    }while(fifo_data_cnt > 0);

    return DDC_Err;
}


/**************************************************************
 * Anx9030_DDC_Write
 **************************************************************/
/*Function name:    Anx9030_DDC_Write()*/
/*Function :    write data to DDC channel*/
/*Parameter :   ddc_address,DDC_data,length,DDC_type*/
/*Return  :write reslut,ex.DDC_NO_Err,DDC_Status_Err*/
BYTE Anx9030_DDC_Write (Anx9030_DDC_Addr ddc_address, const BYTE *DDC_data,
                        WORD length, Anx9030_DDC_Type DDC_type)
{
    BYTE DDC_Err,fifo_write_data_compare;
    int data_cnt;
    int fifo_data_cnt;
    BYTE *Fifo_Write_P;

    DDC_Err = DDC_NO_Err;
    fifo_write_data_compare = 0;
    fifo_data_cnt = length;
    Fifo_Write_P = (BYTE *)DDC_data;

    /*Judge validity for write address and length*/
    if ((DDC_Err = Anx9030_DDC_Parameter_Validity(Fifo_Write_P, length))!=DDC_NO_Err)
        return (DDC_Err);

    /*set DDC address*/
    Anx9030_DDC_Set_Address(ddc_address, DDC_type);
    /*set number for DDC write*/
    Anx9030_DDC_Set_Number(length);
    /*send "clear DDC fifo" command*/
    Anx9030_DDC_Command((BYTE)Clear_DDC_Fifo);

	delay_ms(1000);
    /*check DDC channel status*/
    if (!Anx9030_DDC_Check_Status(Judge, DDC_Error_bit) &&
        !Anx9030_DDC_Check_Status(Judge, DDC_Occup_bit) &&
        !Anx9030_DDC_Check_Status(Judge, DDC_No_Ack_bit)){

        do {
            /*write data to fifo if data length <= DDC Fifo Depth*/
            if (fifo_data_cnt <= DDC_Fifo_Depth){

                fifo_write_data_compare = fifo_data_cnt;

                while (fifo_data_cnt--){
                    /*write data to DDC fifo*/
                    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, *(Fifo_Write_P++));
                }
                /*send "sequential byte write"command after finish writing data to fifo*/
                Anx9030_DDC_Command((BYTE)Sequential_Byte_Write);
                /*delay*/
                delay_ms(DDC_Write_Delay);
                /*check DDC status when data length <= DDC Fifo Depth*/
                if (Anx9030_DDC_Check_Status(Judge, DDC_Progress_bit) ||
                    Anx9030_DDC_Count_Compare(fifo_write_data_compare)){
                    Anx9030_DDC_Check_Status(report, 0);
                    return DDC_Status_Err;
                }
            }
            /*write data to fifo if data length > DDC Fifo Depth*/
            else{
                data_cnt = DDC_Fifo_Depth;
                while(data_cnt--){
                    hdmi_i2c_write_byte(ANX9030_ADDR1, ANX9030_DDC_FIFO_ACC_REG, *(Fifo_Write_P++));
                }
                /*send "sequential byte write"command after finish writing data to fifo*/
                Anx9030_DDC_Command((BYTE)Sequential_Byte_Write);
                /*delay*/
                delay_ms(DDC_Write_Delay);
                /*check DDC status when data length > DDC Fifo Depth*/
                if (!Anx9030_DDC_Check_Status(Judge, DDC_Progress_bit) ||
                    !Anx9030_DDC_Check_Status(Judge, DDC_Fifo_Full_bit)){
                    Anx9030_DDC_Check_Status(report, 0);
                    return DDC_Status_Err;
                }

                fifo_data_cnt -= DDC_Fifo_Depth;
            }

        }while(fifo_data_cnt > 0);
    }
    else{
        Anx9030_DDC_Check_Status(report, 0);
        return DDC_Status_Err;
    }

    return DDC_Err;
}


/**************************************************************
 * delay_ms
 **************************************************************/
void delay_ms(unsigned int ms)
{
	unsigned int	i = 0;
	unsigned int	j = 0;

	for(i=0; i<ms; i++)
	{
		// if you optimization you code with level 0, please set j max 0x3000;
		// if you optimization you code with level 1, please set j max 0x7500;
		// if you optimization you code with level 2, please set j max 0x7500;
		for(j=0; j<0x3000; j++)
		{
			;
		}

	}

}




unsigned char ANX9030_i2c_write_p0_reg(unsigned char offset, unsigned char data)
{
	hdmi_i2c_write_byte(ANX9030_ADDR1, (int)offset, data);

    return 0;
}

unsigned char	ANX9030_i2c_write_p1_reg(unsigned char offset, unsigned char data)
{
	hdmi_i2c_write_byte(ANX9030_ADDR2, (int)offset, data);

    return 0;
}

unsigned char	ANX9030_i2c_read_p0_reg(unsigned char offset, unsigned char *data)
{
	hdmi_i2c_read_byte(ANX9030_ADDR1, (int)offset, (char *)data);
	return 0;
}

unsigned char	ANX9030_i2c_read_p1_reg(unsigned char offset, unsigned char *data)
{
	hdmi_i2c_read_byte(ANX9030_ADDR2, (int)offset, (char *)data);	
	return 0;
}

