#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "global.h"
#include "../csvid/include/csvid.h"
#include "../cstvout/include/cstvout.h"
#include "../csosd/include/csosd.h"
#include "../csi2c/include/csi2c.h"

#define VIB_ENBALE

CSTVOUT_HANDLE g_tvout_handle = 0;
CSVID_HANDLE g_video_handle = NULL;
CSOSD_HANDLE g_osd_handle = NULL;
FILE*   vid_file=NULL;
static CSTVOUT_MODE g_tvout_mode = 0;
CSVID_Rect src_rect={0,720,0,576};
CSVID_Rect dst_rect={0,720,0,576};

struct timeval start,end;
//#define VIB_ENABLE
#ifdef VIB_ENABLE
typedef enum{
	VIB_PAL,
	VIB_NTSC
}VIB_MODE;

CSAPI_RESULT __VIB_InitTVP(VIB_MODE VideoMode)
{
	CSI2C_HANDLE handle;
	CSI2C_ErrCode error;
	char buff[5];
	char addr[6];

	handle = CSI2C_Open(0xb8>>1);   //chip address 
	if (handle == NULL) 
	{
		printf("Open i2c  error\n");
		return CSAPI_FAILED;
	}
	buff[0] = 0x01;
	addr[0] = 0x05;  
	error = CSI2C_Write(handle, addr[0], &buff[0], 1);  //software reset
	if (error != 0) 
	{
		printf("write[%x:%x] error no %d\n", addr[0], buff[0], error);
		return CSAPI_FAILED;
	}
	printf(" reset tvp ....... \n");

	buff[1] = 0x0;
	addr[1] = 0x05;
	error = CSI2C_Write(handle, addr[1], &buff[1], 1);  //software reset
	if (error != 0) 
	{
		printf("write[%x:%x]error no %d\n", addr[1],buff[1],error);
		return CSAPI_FAILED;
	}
	buff[2] = 0x9;
	addr[2] = 0x03;
	error = CSI2C_Write(handle, addr[2], &buff[2], 1);  //output data rate
	if (error != 0) 
	{
		printf("write[%x:%x]error no %d\n", addr[2], buff[2], error);
		return CSAPI_FAILED;
	}
	switch(VideoMode)
	{
		case VIB_PAL:
			buff[0] = 0x04;
			addr[0] = 0x28;
			CSI2C_Write(handle, addr[0], &buff[0], 1);
		break;

		case VIB_NTSC:
			buff[0] = 0x80;
			buff[1] = 0x20;
			buff[2] = 0x10;
			buff[3] = 0xc0;
			buff[4] = 0x08;

			addr[0] = 0x01;
			addr[1] = 0x02;
			addr[2] = 0x40;
			addr[3] = 0x42;
			addr[4] = 0x48;
			addr[5] = 0x4a;

			CSI2C_Write(handle, addr[0], &buff[0], 1);
			CSI2C_Write(handle, addr[1], &buff[1], 1);
			CSI2C_Write(handle, addr[2], &buff[2], 1);
			CSI2C_Write(handle, addr[3], &buff[3], 1);
			CSI2C_Write(handle, addr[4], &buff[1], 1);  
			CSI2C_Write(handle, addr[5], &buff[4], 1);
		break;

		default:
			break; 
	}

	error = CSI2C_Close(handle);
	if (error != 0) 
	{
		printf("close error one byte \n");
		return CSAPI_FAILED;
	} 
	
	return CSAPI_SUCCEED;
}
#endif

static void video_init(void)
{
	union vib_para vibpara;

	vibpara.val = 0x82d02401;
	vibpara.bits.work_mode = 0;
   //open tvout device
    if(NULL == (g_tvout_handle = CSTVOUT_Open(0)))
    {
        printf("open tvout device Failed\n");
        return;
    }
    CSTVOUT_SetMode(g_tvout_handle,TVOUT_MODE_576I);

//open video
    if(NULL == (g_video_handle=CSVID_Open(VID_DEV_0))) 
    {
        printf("open video error\n");
        return;
    }
    else printf("open video ok\n");

     if(NULL == (g_osd_handle = CSOSD_Open(OSD_LAYER_0)))
    {
        printf("open osd device Failed\n");
        return;
    }
    CSOSD_Disable(g_osd_handle);
    
    //video decoder set
    CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
    CSVID_SetOutputAlpha(g_video_handle, 0x80);
#ifdef VIB_ENABLE
CSVID_VIB_Config(g_video_handle, vibpara);
__VIB_InitTVP(VIB_PAL);
CSVID_VIB_Reset(g_video_handle);

    CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_VIB);
#else
    CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_MPEG2_TS);
#endif
    CSVID_SetInputMode(g_video_handle, VID_INPUT_STILLPIC);
CSVID_SetDecoderMode(g_video_handle, VID_FRAME_SP);
 	CSVID_WaitSync(g_video_handle, 0);
	CSVID_DisablePTSSync(g_video_handle);
}

void testvideocallbackE(CSVID_HANDLE handle)
{
        printf("empty video call back run!\n");
}

void testvideocallbackF(CSVID_HANDLE handle)
{
        printf("full video call back run!\n");
}

void testvideocallback2(CSVID_HANDLE handle, signed char * sync)
{
        printf("video call back 2  run!,sync = %d\n",*sync);
        if(*sync == 1)
            CSVID_Stop(g_video_handle);
        else sleep(1);
}

void testvideocallback_pscancrop(CSVID_HANDLE handle, CSVID_Rect * rect)
{
        printf("pscan crop offset left:%d,right:%d,top:%d,bottom:%d\n",rect->left,rect->right,rect->top,rect->bottom);
}

void testvideocallback_aspectratio(CSVID_HANDLE handle,CSVID_ASPECTRATIO * ratio)
{
    printf("ratio = %d",*ratio);
}

int main(void)
{
    int read_len = 0;
    unsigned int bufsize = 0;
    CSVID_SequenceHeader hdr;
    unsigned char  read_buf[4096+100];
    static int first_in = 1;
    unsigned int datasize = 1;

    video_init();
#ifdef VIB_ENABLE
CSVID_Play(g_video_handle);
sleep(5);
CSVID_Stop(g_video_handle);
sleep(5);
CSVID_Play(g_video_handle);

    //CSVID_SyncNotify(g_video_handle, testvideocallback2, 15, 1);
#else 
    CSVID_Play(g_video_handle);
    //CSVID_EnablePTSSync(g_video_handle);
    //CSVID_Stop(g_video_handle);
    
    CSVID_PFMOpen(g_video_handle);
    CSVID_GetPFMBufferSize(g_video_handle, &bufsize);

    //CSVID_SetNotifyPFMDataEmpty(g_video_handle, &datasize, testvideocallbackE, 1);
    //CSVID_SetNotifyPFMDataFull(g_video_handle, &datasize, testvideocallbackF, 1);
    //CSVID_PScanCropNotify(g_video_handle, testvideocallback_pscancrop, 1);
    //CSVID_AspectRatioChangeNotify(g_video_handle, testvideocallback_aspectratio, 1);
    printf("bufsize = %d\n",bufsize);

    vid_file = fopen("/mnt/stream/output.vbs","rb");//mpeg2_mpa_480p_qianlizhiwai_video1.mpv//mpeg2_mpa_1080i_video1.mpv
    if(vid_file == NULL)
    {
       printf("======>file open failed\n");
       return CSAPI_FAILED;
    }
    fseek(vid_file,0L, SEEK_SET);
    if ((read_len=fread(read_buf,1,4096,vid_file))!=1671)
    {
        fseek(vid_file,0L, SEEK_SET);
        printf("read_len = %d ,===no data \n",read_len);
    }

    gettimeofday(&start, NULL);
    //CSVID_SetPlaySpeed(g_video_handle, 0);
    //CSVID_Pause(g_video_handle);
    
    //CSVID_SetFrameRate(g_video_handle, 60);
    while(1){
            if(CSAPI_SUCCEED == CSVID_WritePFMData(g_video_handle, read_buf, read_len)){
                if ((read_len=fread(read_buf,1,4096,vid_file))!= 1671)
                {
                	if(feof(vid_file)){
                    		fseek(vid_file,0L, SEEK_SET);
                    		printf("read_len = %d ,===no data \n",read_len);
                	}
			else{
				printf("asdasdasd\n");
			}
                }
                CSVID_GetPFMBufferSize(g_video_handle, &bufsize);
                printf("bufsize = %d\n",bufsize);
            }
            else{
		printf("33333333\n");
                usleep(5*1000);
            }
#if 0
            if(first_in == 1){
                    first_in = 0;
                    //CSVID_Play(g_video_handle);
                    sleep(2);
                    CSVID_GetSequenceHeader(g_video_handle, &hdr);
                    printf("hdr.w = %d,hdr.h = %d,hdr.frame_rate = %d\n",hdr.w,hdr.h,hdr.frame_rate);
                    src_rect.right=hdr.w;
                    src_rect.bottom=hdr.h;
                    if(hdr.h == 576){
                            g_tvout_mode = TVOUT_MODE_576I;
                            dst_rect.right=720;
                            dst_rect.bottom=576;
                    }
                    else if(hdr.h == 480){
                            g_tvout_mode = TVOUT_MODE_480I;
                            dst_rect.right=720;
                            dst_rect.bottom=480;
                    }
                    else if(hdr.h == 720){
                            g_tvout_mode = TVOUT_MODE_720P50;
                            dst_rect.right=1280;
                            dst_rect.bottom=720;
                    }
                    else if((hdr.h > 1069)||(hdr.h < 1091)){
                            g_tvout_mode = (hdr.frame_rate > 25 ? TVOUT_MODE_1080I25 : TVOUT_MODE_1080I30);
                            //g_tvout_mode = TVOUT_MODE_1080I25;
                            dst_rect.right=1920;
                            dst_rect.bottom=1080;
                    }
                    CSTVOUT_SetMode(g_tvout_handle,g_tvout_mode);
                    CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
                    
            }
#endif
#if 0
            gettimeofday(&end, NULL);
            if(end.tv_sec - start.tv_sec > 10){
                      gettimeofday(&start, NULL);
                     CSVID_Freeze(g_video_handle);
                      sleep(5);
                      CSVID_Resume(g_video_handle);
            }
#endif
     
    }
    #endif
    return 0;
}
