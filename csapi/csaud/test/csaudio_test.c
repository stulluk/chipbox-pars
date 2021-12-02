#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "../csaud/include/csaud.h"

CSAUD_HANDLE g_audio_handle = NULL;
FILE*   aud_file=NULL;
unsigned char  read_buf[98304];
int read_len = 0;

void testaudiocallbackE(CSAUD_HANDLE handle)
{
	unsigned int bufsize = 0;

	printf(" empty audio call back run!\n");
	
	if(CSAPI_SUCCEED == CSAUD_WritePFMData(g_audio_handle, read_buf, read_len)){
		if ((read_len=fread(read_buf,1,4096,aud_file))!=4096)
		{
			fseek(aud_file,0L, SEEK_SET);
			printf("read_len = %d ,===no data \n",read_len);
		}
		CSAUD_GetPFMBufferSize(g_audio_handle, &bufsize);
		printf("bufsize = %d\n",bufsize);
	}
}

void testaudiocallbackF(CSAUD_HANDLE handle)
{
        printf(" full audio call back run!\n");
}


int main(void){
    unsigned int bufsize = 0;
    unsigned int datasize = 50;
    int read_len = 0;

    CSAUD_Volume vol;

    vol.front_left = 100;
    vol.front_right = 100;
    vol.rear_left = 100;
    vol.rear_right = 100;
    vol.center = 100;
    vol.lfe  = 100;

   //open audio    
    if(NULL == (g_audio_handle=CSAUD_Open(AUD_DEV_0)))
    {
        printf("open audio error\n");
        return CSAPI_FAILED;
    }
    else printf("open audio ok\n"); 

    //audio decoder set
    CSAUD_Init(g_audio_handle);
    CSAUD_SetInputMode(g_audio_handle,AUD_INPUT_BLOCK);
    //CSAUD_SetVolume(g_audio_handle,NULL);
    CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_MPA);
    //CSAUD_EnablePTSSync(g_audio_handle);
    CSAUD_SetVolume(g_audio_handle, &vol);
    CSAUD_Play(g_audio_handle);
    //CSAUD_Stop(g_audio_handle);

    CSAUD_PFMOpen(g_audio_handle);
    CSAUD_GetPFMBufferSize(g_audio_handle, &bufsize);
    printf("bufsize = %d\n",bufsize);

    aud_file = fopen("/mnt/stream/weicheng.mp3","rb");
    if(aud_file == NULL)
    {
       printf("======>file open failed\n");
       return CSAPI_FAILED;
    }

#if 0
    read_len = fread(read_buf,1,4096,aud_file);
#else
    if ((read_len=fread(read_buf,1,4096,aud_file))!=4096)
    {
        fseek(aud_file,0L, SEEK_SET);
        printf("read_len = %d ,===no data \n",read_len);
    }
#endif

	CSAUD_SetNotifyPFMDataEmpty(g_audio_handle, &datasize, testaudiocallbackE, 1);
	CSAUD_SetNotifyPFMDataFull(g_audio_handle, &datasize, testaudiocallbackF, 1);
while(1){ 
	printf("contiune\n");
	sleep(20);
}

    while(0){
#if 1      
            static unsigned long long pts_test = 0x100;
            if(CSAPI_SUCCEED == CSAUD_WritePFMDataWithPTS(g_audio_handle, read_buf, read_len, pts_test)){
                    pts_test++;
			//if(pts_test == 0x110)
				//break;
			//usleep(50000);
#else
            if(CSAPI_SUCCEED == CSAUD_WritePFMData(g_audio_handle, read_buf, read_len)){
#endif
                if ((read_len=fread(read_buf,1,4096,aud_file))!=4096)
                {
                    fseek(aud_file,0L, SEEK_SET);
                    printf("read_len = %d ,===no data \n",read_len);
                }
               	CSAUD_GetPFMBufferSize(g_audio_handle, &bufsize);
    			printf("bufsize = %d\n",bufsize);
			if(bufsize == 0){	CSAUD_Play(g_audio_handle);}
            }
    }

    return CSAPI_SUCCEED;
}
    
