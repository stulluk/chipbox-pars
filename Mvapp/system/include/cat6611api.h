#ifndef  _CS_SYS_CAT6611_API_H_
#define  _CS_SYS_CAT6611_API_H_

typedef enum TagSYSHDMIType
{
    SYS_HDMI_576I=0,
    SYS_HDMI_480I,
    SYS_HDMI_576P,
    SYS_HDMI_720P50,
    SYS_HDMI_1080I25,
    SYS_HDMI_1080I30,
    SYS_HDMI_720P60
}eSYSHDMIType;


int Cat6611_init(void);
void Cat6611_SetAspecRatio(U8 on16_9);
int Cat6611_SetOutputMode( eSYSHDMIType Mode );
void Cat6611ClearVideoState(void);
#endif

