#ifndef _ANX9030_Sys_Config_H
#define _ANX9030_Sys_Config_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ANX9030_Sys9030.h"

//user interface define begins

//select video hardware interface
#define ANX9030_VID_HW_INTERFACE 0x05//0x00:RGB and YcbCr 4:4:4 Formats with Separate Syncs (24-bpp mode)
                                                                 //0x01:YCbCr 4:2:2 Formats with Separate Syncs(16-bbp)
                                                                 //0x02:YCbCr 4:2:2 Formats with Embedded Syncs(No HS/VS/DE)
                                                                 //0x03:YC Mux 4:2:2 Formats with Separate Sync Mode1(bit15:8 and bit 3:0 are used)
                                                                 //0x04:YC Mux 4:2:2 Formats with Separate Sync Mode2(bit11:0 are used)
                                                                 //0x05:YC Mux 4:2:2 Formats with Embedded Sync Mode1(bit15:8 and bit 3:0 are used)
                                                                 //0x06:YC Mux 4:2:2 Formats with Embedded Sync Mode2(bit11:0 are used)
                                                                 //0x07:RGB and YcbCr 4:4:4 DDR Formats with Separate Syncs
                                                                 //0x08:RGB and YcbCr 4:4:4 DDR Formats with Embedded Syncs
                                                                 //0x09:RGB and YcbCr 4:4:4 Formats with Separate Syncs but no DE
                                                                 //0x0a:YCbCr 4:2:2 Formats with Separate Syncs but no DE
//select input color space
#define ANX9030_INPUT_COLORSPACE 0x01//0x00: input color space is RGB
                                                                //0x01: input color space is YCbCr422
                                                                //0x02: input color space is YCbCr444
//select input pixel clock edge for DDR mode
#define ANX9030_IDCK_EDGE_DDR 0x01  //0x00:use rising edge to latch even numbered pixel data
                                                                //0x01:use falling edge to latch even numbered pixel data 

//select audio hardware interface
#define ANX9030_AUD_HW_INTERFACE 0x01//0x01:audio input comes from I2S
                                                                  //0x02:audio input comes from SPDIF
                                                                  //0x04:audio input comes from one bit audio
//select MCLK and Fs relationship if audio HW interface is I2S
#define ANX9030_MCLK_Fs_RELATION 0x01		//0x00:MCLK = 128 * Fs
                                            //0x01:MCLK = 256 * Fs
                                            //0x03:MCLK = 384 * Fs
                                            //0x04:MCLK = 512 * Fs

#define ANX9030_AUD_CLK_EDGE 0x00  //0x00:use MCLK and SCK rising edge to latch audio data
                                                                //0x80:use MCLK and SCK falling edge to latch audio data
//select I2S channel numbers if audio HW interface is I2S
#define ANX9030_I2S_CH0_ENABLE 0x01 //0x01:enable channel 0 input; 0x00: disable
#define ANX9030_I2S_CH1_ENABLE 0x00 //0x01:enable channel 0 input; 0x00: disable
#define ANX9030_I2S_CH2_ENABLE 0x00 //0x01:enable channel 0 input; 0x00: disable
#define ANX9030_I2S_CH3_ENABLE 0x00 //0x01:enable channel 0 input; 0x00: disable
//select I2S word length if audio HW interface is I2S
#define ANX9030_I2S_WORD_LENGTH 0x0b
                                        //0x02 = 16bits; 0x04 = 18 bits; 0x08 = 19 bits; 0x0a = 20 bits(maximal word length is 20bits); 0x0c = 17 bits;
                                        // 0x03 = 20bits(maximal word length is 24bits); 0x05 = 22 bits; 0x09 = 23 bits; 0x0b = 24 bits; 0x0d = 21 bits;

//select I2S format if audio HW interface is I2S
#define ANX9030_I2S_SHIFT_CTRL 0x00//0x00: fist bit shift(philips spec)
                                                                //0x01:no shift
#define ANX9030_I2S_DIR_CTRL 0x00//0x00:SD data MSB first
                                                            //0x01:LSB first
#define ANX9030_I2S_WS_POL 0x00//0x00:left polarity when word select is low
                                                        //0x01:left polarity when word select is high
#define ANX9030_I2S_JUST_CTRL 0x00//0x00:data is left justified
                                                             //0x01:data is right justified

//user interface define ends

extern BYTE anx9030_new_HW_interface_parameter;

void ANX9030_System_Config(void);
void ANX9030_System_Video_Config(void);
void ANX9030_System_Packets_Config(void);
void ANX9030_System_Audio_Config(void);
unsigned int HDMI_GetTimingMode(void);

    void HDMI_SetTimingMode(BYTE VID_FORMAT);
#ifdef __cplusplus
}
#endif

#endif
