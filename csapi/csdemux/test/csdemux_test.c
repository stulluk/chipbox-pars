#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include <asm/page.h>
#include <sys/mman.h>

#include "../../open_sources/inc/minigui/common.h"
#include "../../open_sources/inc/minigui/minigui.h"
#include "../../open_sources/inc/minigui/gdi.h"
#include "../../open_sources/inc/minigui/window.h"

#include "global.h"
#include "csapi.h"
#include "../cstuner/include/cs_tuner.h"
//#include "fw_debug.h"


typedef struct CS_Service_t_
{
    unsigned int servce_id;
    unsigned int servce_pmtpid;
    unsigned int servce_vidpid;
    unsigned int servce_audpid;
    unsigned int pmt_complete;
    CSDEMUX_HANDLE filter_handle;
    CSDEMUX_HANDLE pidfilter_handle;
    unsigned int vid_type;
    unsigned int aud_type;
    char providername[20];
    char servicename[20];
} CS_Service_t;

typedef struct CS_Test_Section_t_
{
    unsigned int servce_id;
    unsigned int servce_pmtpid;
    unsigned int servce_vidpid;
    unsigned int servce_audpid;
    unsigned int pmt_complete;
    CSDEMUX_HANDLE filter_handle;
    CSDEMUX_HANDLE pidfilter_handle;
    int testfilter_index;
} CS_Test_Section_t;

#define test_pid unsigned short  

#define PAT_PID 0x00
#define PAT_TAB_ID 0x00
#define PMT_TAB_ID 0x02
#define SDT_PID 0x11
#define SDT_TAB_ID 0x42
#define SECTION_LENGTN 1024
#define MAX_SECTION_LENGTN 1024
#define CURRENT_SERVICE_NUM 16
#define  MAX_SERVICE_NUM 32

#define MAX_SECTION_FILTER_CHAR    4
#define MAX_SECTION_SIZE           4284
#if 1
#define TS_SIZE                    188
#else
#define TS_SIZE	204
#endif
static CSDEMUX_HANDLE g_demux_chl_handle = 0;
static CSDEMUX_HANDLE g_demux_video_handle = 0;
static CSDEMUX_HANDLE g_demux_audio_handle = 0;
static CSDEMUX_HANDLE g_demux_pidfilter_video_handle = 0;
static CSDEMUX_HANDLE g_demux_pidfilter_audio_handle = 0;
static CSVID_HANDLE g_video_handle = 0;
static CSAUD_HANDLE g_audio_handle = 0;
static CSTVOUT_HANDLE g_tvout_handle = 0;
static CSOSD_HANDLE g_osd_handle = 0;

unsigned char section_buf[SECTION_LENGTN];
unsigned int service_num=0;
unsigned int cur_pmt_parser_index=0;
unsigned int sdtcomplete=0;
unsigned char current_section_num=0,last_section_num=0; 
CS_Service_t service[MAX_SERVICE_NUM];
CS_Test_Section_t testfilter[MAX_SERVICE_NUM];
static unsigned char   mFilter[MAX_SECTION_FILTER_CHAR];
static unsigned char   mMask[MAX_SECTION_FILTER_CHAR];
static unsigned char   mpBuff[MAX_SECTION_SIZE];
static unsigned int    mPID;
static unsigned char   mTABID;
static unsigned int    mPos;
static unsigned int    mIsChecked;
static unsigned char   mNextCounter;


long time_start=0, time_end=0;
test_pid g_vpid=0x481,g_apid=0x482;
int g_tunerflag = 1,g_vid_decodertype = 0,g_aud_decodertype = 0;
CSVID_Rect src_rect={0,720,0,576};
CSVID_Rect dst_rect={0,720,0,576};

static CSAPI_RESULT errno_api = CSAPI_SUCCEED;

static CSTVOUT_MODE g_tvout_mode = 0;

#define OUT_CHL0_DIR_WP_ADDR    (0x41400000+(0x100<<2))
#define OUT_CHL0_DIR_RP_ADDR    (0x41400000+(0x101<<2))
#define OUT_CHL2_DIR_WP_ADDR    (0x41400000+(0x104<<2))
#define OUT_CHL2_DIR_RP_ADDR    (0x41400000+(0x105<<2))



char ts_buf[TS_SIZE];
char sec_buf[SECTION_LENGTN];
static unsigned int pid_cnt = 0;

unsigned long Byte_CRC(unsigned char IByte, unsigned long ICrc)
{
        int   i; /* Loop Counter */
        unsigned long Crc=ICrc;
        
        /* Calcurate CRC */
        Crc = Crc ^ ( (unsigned long)IByte << 24 );

        for(i = 0 ; i < 8 ; i++ )
        {
                if( ( Crc & 0x80000000 ) != 0 ) {
                        Crc = ( Crc << 1 ) ^ 0x04c11db7;
                }
                else {
                        Crc = Crc << 1;
                }
        }
        return Crc;
}

unsigned long CRC(unsigned char* buf, unsigned int len)
{
        int   i;
        unsigned long Crc=0xffffffff;
        
        for(i = 0 ; i < len ; i++ )
        {
                Crc=Byte_CRC(buf[i],Crc);
        }
        return Crc;
}
#if 1

char ts_pat_buf[TS_SIZE];
char ts_sdt_buf[TS_SIZE];

#define PAT_SECTION_LENGTH 16
static void PATDataToSection(int ts_id,int pmt_id,int service_id)
{
        char sec_ver = 0;
        int CRCW = 0;
        char sec_buf[SECTION_LENGTN];
        static unsigned int pid_pat_cnt = 0;

        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = PAT_TAB_ID;
        sec_buf[1] = 0xb0|((13>>8)&0x0f);
        sec_buf[2] = 13&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (service_id>>8)&0xff;
        sec_buf[9] = service_id&0xff;
        sec_buf[10] = (pmt_id>>8)&0x1f;
        sec_buf[11] = pmt_id&0xff;

        CRCW = CRC(sec_buf,12);

        printf("CRCW = 0x%x\n",CRCW);
        
        sec_buf[12] = (CRCW>>24)&0xff;
        sec_buf[13] = (CRCW>>16)&0xff;
        sec_buf[14] = (CRCW>>8)&0xff;
        sec_buf[15] = CRCW&0xff;

        memset(ts_pat_buf,0xff,TS_SIZE);

        ts_pat_buf[0] = 0x47;
        ts_pat_buf[1] = 0x40|((PAT_PID>>8)&0x1f);
        ts_pat_buf[2] = PAT_PID&0xff;
        ts_pat_buf[3] = 0x10|(pid_pat_cnt&0xf);
        pid_pat_cnt++;
        ts_pat_buf[4] = 0x00;
        
        memcpy(ts_pat_buf+5, sec_buf, PAT_SECTION_LENGTH);
}

static void SDTDataToSection(int ts_id,int net_id,int service_id,char* name,int name_len)
{
        char sec_ver = 0;
        int CRCW = 0;
        int des_len = 5 + name_len;
        int len = 17 + des_len;
        char sec_buf[SECTION_LENGTN];
        static unsigned int pid_sdt_cnt = 0;

        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = SDT_TAB_ID;
        sec_buf[1] = 0xb0|((len>>8)&0x0f);
        sec_buf[2] = len&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (net_id>>8)&0xff;
        sec_buf[9] = net_id&0xff;
        
        sec_buf[10] = 0;
        
        sec_buf[11] = (service_id>>8)&0xff;
        sec_buf[12] = service_id&0xff;

        sec_buf[13] = 0x1;

        sec_buf[14] = 0x50|((des_len>>8)&0x0f);
        sec_buf[15] = des_len&0xff;

        sec_buf[16] = 0x48;
        sec_buf[17] = (3+len)&0xff;

        sec_buf[18] = 0x1;

        sec_buf[19] = 0;

        sec_buf[20] = name_len;
        memcpy(sec_buf+21,name,name_len);

        CRCW = CRC(sec_buf,len+3-4);

        sec_buf[21+name_len] = (CRCW>>24)&0xff;
        sec_buf[22+name_len] = (CRCW>>16)&0xff;
        sec_buf[23+name_len] = (CRCW>>8)&0xff;
        sec_buf[24+name_len] = CRCW&0xff;

        memset(ts_sdt_buf,0xff,TS_SIZE);

        ts_sdt_buf[0] = 0x47;
        ts_sdt_buf[1] = 0x40|((SDT_PID>>8)&0x1f);
        ts_sdt_buf[2] = SDT_PID&0xff;
        ts_sdt_buf[3] = 0x10|(pid_sdt_cnt&0xf);
        pid_sdt_cnt++;
        ts_sdt_buf[4] = 0x00;

        memcpy(ts_sdt_buf+5, sec_buf, len+3);
}
#else
#define PAT_SECTION_LENGTH 16
static void PATDataToSection(int ts_id,int pmt_id,int service_id)
{
        char sec_ver = 0;
        int CRCW = 0;
        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = PAT_TAB_ID;
        sec_buf[1] = 0xb0|((13>>8)&0x0f);
        sec_buf[2] = 13&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (service_id>>8)&0xff;
        sec_buf[9] = service_id&0xff;
        sec_buf[10] = (pmt_id>>8)&0x1f;
        sec_buf[11] = pmt_id&0xff;

        CRCW = CRC(sec_buf,12);

        printf("CRCW = 0x%x\n",CRCW);
        
        sec_buf[12] = (CRCW>>24)&0xff;
        sec_buf[13] = (CRCW>>16)&0xff;
        sec_buf[14] = (CRCW>>8)&0xff;
        sec_buf[15] = CRCW&0xff;
 
}

static int SDTDataToSection(int ts_id,int net_id,int service_id,char* name,int name_len)
{
        char sec_ver = 0;
        int CRCW = 0;
        int des_len = 5 + name_len;
        int len = 17 + des_len;
 
        memset(sec_buf,0,SECTION_LENGTN);

        sec_buf[0] = SDT_TAB_ID;
        sec_buf[1] = 0xb0|((len>>8)&0x0f);
        sec_buf[2] = len&0xff;

        sec_buf[3] = (ts_id>>8)&0xff;
        sec_buf[4] = ts_id&0xff;

        sec_buf[5] = 0xc1|((sec_ver<<1)&0x3e);
        sec_buf[6] = 0;
        sec_buf[7] = 0;

        sec_buf[8] = (net_id>>8)&0xff;
        sec_buf[9] = net_id&0xff;
        
        sec_buf[10] = 0;
        
        sec_buf[11] = (service_id>>8)&0xff;
        sec_buf[12] = service_id&0xff;

        sec_buf[13] = 0x1;

        sec_buf[14] = 0x50|((des_len>>8)&0x0f);
        sec_buf[15] = des_len&0xff;
    
        sec_buf[16] = 0x48;
        sec_buf[17] = (3+len)&0xff;

        sec_buf[18] = 0x1;

        sec_buf[19] = 0;

        sec_buf[20] = name_len;
        memcpy(sec_buf+21,name,name_len);

        CRCW = CRC(sec_buf,len+3-4);
        printf("CRCW = 0x%x\n",CRCW);
        sec_buf[21+name_len] = (CRCW>>24)&0xff;
        sec_buf[22+name_len] = (CRCW>>16)&0xff;
        sec_buf[23+name_len] = (CRCW>>8)&0xff;
        sec_buf[24+name_len] = CRCW&0xff;

        return len+3;
}

static void SectionToTs(char* sec, int len,int pid)
{
        memset(ts_buf,0xff,TS_SIZE);

        ts_buf[0] = 0x47;
        ts_buf[1] = 0x40|((pid>>8)&0x1f);
        ts_buf[2] = pid&0xff;
        ts_buf[3] = 0x10|(pid_cnt&0xf);
        pid_cnt++;
        ts_buf[4] = 0x00;
        
        memcpy(ts_buf+5, sec, len);
}
#endif

/*
void audio_mixer_status(void)
{
        unsigned int reg_val = 0;
        
        ReadReg32(0x41412400,&reg_val);
        ReadReg32(0x41412404,&reg_val);
        printf("audio mailbox0 : 0x%x\t",reg_val);
        printf("audio mailbox1 : 0x%x\t",reg_val);
}
*/
static int firstin = 1;
static int firstin2 = 1;
void test_filter(void);
static int test_filter_cycle_flag = 0;
static int test_filter_flag = 0;
CSDEMUX_HANDLE test_pidfilter = NULL;
CSDEMUX_HANDLE test_filterpat = NULL;
CSDEMUX_HANDLE test_filterpmt = NULL;
CSDEMUX_HANDLE test_filterpat2 = NULL;
CSDEMUX_HANDLE test_filterpmt2 = NULL;
unsigned char pMask1[12]  = {0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMatch1[12] = {0x70,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMask2[12]  = {0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMatch2[12] = {0x73,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMask3[12]  = {0xf0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMatch3[12] = {0x50,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMask4[12]  = {0xf0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
unsigned char pMatch4[12] = {0x60,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

static int receive_data_flag = 0;
static pthread_mutex_t pvr_mutex;

pthread_t montior_thread;
void monitor_filter(void)
{
	while(1){
		pthread_mutex_lock(&pvr_mutex);\
		if(receive_data_flag){
			//pthread_mutex_unlock(&pvr_mutex);
			CSDEMUX_Filter_Disable(test_filterpat);
        		CSDEMUX_Filter_Close(test_filterpat);
				test_filterpat = NULL;
			CSDEMUX_Filter_Disable(test_filterpmt);
			CSDEMUX_Filter_Close(test_filterpmt);
				test_filterpmt = NULL;
			CSDEMUX_PIDFT_Disable(test_pidfilter);
			CSDEMUX_PIDFT_Close(test_pidfilter);
				test_pidfilter= NULL;
			test_filter();

		//pthread_mutex_lock(&pvr_mutex);
			receive_data_flag = 0;
		}
		pthread_mutex_unlock(&pvr_mutex);
	}
}

static void testdemuxcallback(CSDEMUX_HANDLE handle,CSDEMUX_SECEVENT* event)
{
        unsigned int  reg_val = 4096;
        unsigned char buf[4096];
        unsigned char prevalue = 0;
        unsigned char a[4];

  #if 1
  	printf("run callback function\n");
        memset(buf,0,4096);
        if(DEMUX_SECTION_AVAIL == *event){
            printf("handle 0x%x\n",handle);
           if(CSDEMUX_Filter_ReadSectionData(handle, buf, &reg_val)== CSAPI_SUCCEED)
           {
                if(firstin){
                        printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val);
                        firstin = 0; 
                }
                //else if((buf[67] == a[0])&&(buf[68] == a[1])&&(buf[69] == a[2])&&(buf[70] == a[3])){
                //        ;
                //}
                else{
                        printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val);
                }
	pthread_mutex_lock(&pvr_mutex);
		receive_data_flag = 1;
	pthread_mutex_unlock(&pvr_mutex);		
                a[0] = buf[67];
                a[1] = buf[68];
                a[2] = buf[69];
                a[3] = buf[70];
		
                if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                    printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
                
           }
           else{
                printf("error : %s\n",__FUNCTION__);
           }
        }
        else{
            printf("bbbbbb\n");
        }
  #else
        printf("test no respond\n");
  #endif
}

#if 0
void ecmtask(void)
{
        unsigned int wp = 0, rp = 0,in_size = 0;

        unsigned char buf[1024],buf2[1024];
        unsigned char a[4],b[4];
        unsigned int  reg_val,reg_val2 = 4096,reg_val1 = 4096;
        CSDEMUX_HANDLE ecmpidfilter = NULL;
        CSDEMUX_HANDLE ecmdata1 = NULL;
        CSDEMUX_HANDLE ecmdata2 = NULL;
        unsigned char pMask1[12]        = {0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
        unsigned char pMatch1[12] = {0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
        unsigned char pMask2[12]        = {0xff,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
        unsigned char pMatch2[12] = {0x81,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

        ecmpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID10);
        CSDEMUX_PIDFT_SetChannel(ecmpidfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(ecmpidfilter,0x1771);  
        CSDEMUX_PIDFT_Enable(ecmpidfilter);

        ecmdata1 = CSDEMUX_Filter_Open(DEMUX_FILTER_ID0);
        CSDEMUX_Filter_SetFilter(ecmdata1,pMatch1,pMask1);
        CSDEMUX_Filter_AddPID(ecmdata1,0x1771);
        CSDEMUX_Filter_SetFilterType(ecmdata1,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(ecmdata1);
        CSDEMUX_FILTER_SetSectionNotify(ecmdata1, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);
#if 1
        ecmdata2 = CSDEMUX_Filter_Open(DEMUX_FILTER_ID1);
        CSDEMUX_Filter_SetFilter(ecmdata2,pMatch2,pMask2);
        CSDEMUX_Filter_AddPID(ecmdata2,0x1771);
        CSDEMUX_Filter_SetFilterType(ecmdata2,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(ecmdata2);
        CSDEMUX_FILTER_SetSectionNotify(ecmdata2, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);
#endif
        memset(buf,0,1024);
        memset(buf2,0,1024);
        while(test_filter_flag)
        {
               if(CSDEMUX_Filter_ReadWait(ecmdata1,500) == CSAPI_SUCCEED){
                       if(CSDEMUX_Filter_ReadSectionData(ecmdata1, buf, &reg_val1)== CSAPI_SUCCEED)
                       {
                            if(firstin){
                                    printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val1);
                                    firstin = 0; 
                            }
                            else if((buf[67] == a[0])&&(buf[68] == a[1])&&(buf[69] == a[2])&&(buf[70] == a[3])){
                              ;
                            }
                            else{
                                    printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val1);
                            }
                            a[0] = buf[67];
                            a[1] = buf[68];
                            a[2] = buf[69];
                            a[3] = buf[70];
                            if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                                printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                                return;
                            }
                       }
                       else{
                            printf("filter0 read data failed\n");
                       }
               }
               else{
                        printf("filter 0 timeout\n");
#if 0
            printf("\n*******************************START**********************************\n");
            
            ReadReg32(0x41400440 + (0 << 3), &wp);
            ReadReg32(0x41400444 + (0 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter0 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400440 + (1 << 3), &wp);
            ReadReg32(0x41400444 + (1 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter1 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400544,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400548,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x4140054c,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400550,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x41400554,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400558,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x4140055c,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);

            ReadReg32(0x41400560,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x41400564,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400568,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x4140056c,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400570,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x41400574,&reg_val);
            printf("pid_match_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400578,&reg_val);
            printf("pid_match_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140057c,&reg_val);
            printf("err_tag_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400580,&reg_val);
            printf("err_tag_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400584,&reg_val);
            printf("faild0_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400588,&reg_val);
            printf("failed0_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140058c,&reg_val);
            printf("passed_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400590,&reg_val);
            printf("passed_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400594,&reg_val);
            printf("failed1_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400598,&reg_val);
            printf("failed1_cnt1 : 0x%x\n",reg_val);               

            printf("*******************************END************************************\n");
#endif
               }
#if 1
                if(CSDEMUX_Filter_ReadWait(ecmdata2,500) == CSAPI_SUCCEED){
                       if(CSDEMUX_Filter_ReadSectionData(ecmdata2, buf2, &reg_val2)== CSAPI_SUCCEED)
                       {
                            if(firstin2){
                                    printf("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[67],buf2[68],buf2[69],buf2[70],reg_val2);
                                    firstin2 = 0; 
                            }
                            else if((buf2[67] == b[0])&&(buf2[68] == b[1])&&(buf2[69] == b[2])&&(buf2[70] == b[3])){
                                    ;
                            }
                            else{
                                    printf("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[67],buf2[68],buf2[69],buf2[70],reg_val2);
                            }
                            b[0] = buf2[67];
                            b[1] = buf2[68];
                            b[2] = buf2[69];
                            b[3] = buf2[70];
                            if((buf2[0] == 0)&&(buf2[1] == 0)&&(buf2[2] == 0)){
                                printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                                return;
                            }
                       }
                }
                else{
                        printf("filter 1 timeout\n");
#if 0
            printf("\n*******************************START**********************************\n");
            
            ReadReg32(0x41400440 + (0 << 3), &wp);
            ReadReg32(0x41400444 + (0 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter0 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400440 + (1 << 3), &wp);
            ReadReg32(0x41400444 + (1 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter1 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400544,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400548,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x4140054c,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400550,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x41400554,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400558,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x4140055c,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);

            ReadReg32(0x41400560,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x41400564,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400568,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x4140056c,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400570,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x41400574,&reg_val);
            printf("pid_match_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400578,&reg_val);
            printf("pid_match_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140057c,&reg_val);
            printf("err_tag_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400580,&reg_val);
            printf("err_tag_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400584,&reg_val);
            printf("faild0_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400588,&reg_val);
            printf("failed0_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140058c,&reg_val);
            printf("passed_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400590,&reg_val);
            printf("passed_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400594,&reg_val);
            printf("failed1_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400598,&reg_val);
            printf("failed1_cnt1 : 0x%x\n",reg_val);               

            printf("*******************************END************************************\n");
#endif
                }
#endif
        }
        printf("task exit\n");
}
#else

void test_filter(void)
{
        unsigned int wp = 0, rp = 0,in_size = 0;

        unsigned char buf[4096],buf2[4096];
        unsigned char a[4],b[4];
        unsigned int  reg_val,reg_val2 = 4096,reg_val1 = 4096;
        
        //if(!test_filter_cycle_flag)
        {
            test_pidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID10);
            CSDEMUX_PIDFT_SetChannel(test_pidfilter,DEMUX_CHL_ID0);
            printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
        }
        if(test_filter_cycle_flag%2 == 0)
            CSDEMUX_PIDFT_SetPID(test_pidfilter,0x14);
        else
            CSDEMUX_PIDFT_SetPID(test_pidfilter,0x14);
        
        CSDEMUX_PIDFT_Enable(test_pidfilter);

        //if(!test_filter_cycle_flag)
            test_filterpat = CSDEMUX_Filter_Open(DEMUX_FILTER_ID14);
        if(test_filter_cycle_flag%2 == 0){
            CSDEMUX_Filter_SetFilter(test_filterpat,pMatch1,pMask1);
            CSDEMUX_Filter_AddPID(test_filterpat,0x14);
        }
        else{
            CSDEMUX_Filter_SetFilter(test_filterpat,pMatch1,pMask1);
            CSDEMUX_Filter_AddPID(test_filterpat,0x14);
        }
        CSDEMUX_Filter_SetFilterType(test_filterpat,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(test_filterpat);
        CSDEMUX_FILTER_SetSectionNotify(test_filterpat, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);

        //if(!test_filter_cycle_flag)
            test_filterpmt = CSDEMUX_Filter_Open(DEMUX_FILTER_ID15);
        if(test_filter_cycle_flag%2 == 0){
            CSDEMUX_Filter_SetFilter(test_filterpmt,pMatch2,pMask2);
            CSDEMUX_Filter_AddPID(test_filterpmt,0x14);
        }
        else{
            CSDEMUX_Filter_SetFilter(test_filterpmt,pMatch2,pMask2);
            CSDEMUX_Filter_AddPID(test_filterpmt,0x14);
        }
        CSDEMUX_Filter_SetFilterType(test_filterpmt,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(test_filterpmt);
        CSDEMUX_FILTER_SetSectionNotify(test_filterpmt, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);

	printf("test_filterpat = 0x%x, test_filterpmt = 0x%x\n",test_filterpat,test_filterpmt);
#if 0
	 //if(!test_filter_cycle_flag)
            test_filterpat2 = CSDEMUX_Filter_Open(DEMUX_FILTER_ID16);
        if(test_filter_cycle_flag%2 == 0){
            CSDEMUX_Filter_SetFilter(test_filterpat2,pMatch3,pMask3);
            CSDEMUX_Filter_AddPID(test_filterpmt,0x12);
        }
        else{
            CSDEMUX_Filter_SetFilter(test_filterpmt,pMatch3,pMask3);
            CSDEMUX_Filter_AddPID(test_filterpat2,0x12);
        }
        CSDEMUX_Filter_SetFilterType(test_filterpat2,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(test_filterpat2);
        CSDEMUX_FILTER_SetSectionNotify(test_filterpat2, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);

	 //if(!test_filter_cycle_flag)
            test_filterpmt2 = CSDEMUX_Filter_Open(DEMUX_FILTER_ID17);
        if(test_filter_cycle_flag%2 == 0){
            CSDEMUX_Filter_SetFilter(test_filterpmt2,pMatch4,pMask4);
            CSDEMUX_Filter_AddPID(test_filterpmt2,0x12);
        }
        else{
            CSDEMUX_Filter_SetFilter(test_filterpmt2,pMatch4,pMask4);
            CSDEMUX_Filter_AddPID(test_filterpmt2,0x12);
        }
        CSDEMUX_Filter_SetFilterType(test_filterpmt2,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(test_filterpmt2);
        CSDEMUX_FILTER_SetSectionNotify(test_filterpmt2, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);
#endif
        memset(buf,0,4096);
        memset(buf2,0,4096);
        while(0)
        {
               if(CSDEMUX_Filter_ReadWait(test_filterpat,500) == CSAPI_SUCCEED){
                       if(CSDEMUX_Filter_ReadSectionData(test_filterpat, buf, &reg_val1)== CSAPI_SUCCEED)
                       {
                            if(firstin){
                                    printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val1);
                                    firstin = 0; 
                            }
                            //else if((buf[67] == a[0])&&(buf[68] == a[1])&&(buf[69] == a[2])&&(buf[70] == a[3])){
                              //;
                           // }
                            else{
                                    printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[67],buf[68],buf[69],buf[70],reg_val1);
                            }
                            a[0] = buf[67];
                            a[1] = buf[68];
                            a[2] = buf[69];
                            a[3] = buf[70];
                            if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                                printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                                return;
                            }
                       }
                       else{
                            printf("filter0 read data failed\n");
                       }
               }
               else{
                        printf("filter 0 timeout\n");

                        #if 0
            printf("\n*******************************START**********************************\n");
            
            ReadReg32(0x41400440 + (0 << 3), &wp);
            ReadReg32(0x41400444 + (0 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter0 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400440 + (1 << 3), &wp);
            ReadReg32(0x41400444 + (1 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter1 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400544,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400548,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x4140054c,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400550,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x41400554,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400558,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x4140055c,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);

            ReadReg32(0x41400560,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x41400564,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400568,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x4140056c,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400570,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x41400574,&reg_val);
            printf("pid_match_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400578,&reg_val);
            printf("pid_match_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140057c,&reg_val);
            printf("err_tag_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400580,&reg_val);
            printf("err_tag_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400584,&reg_val);
            printf("faild0_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400588,&reg_val);
            printf("failed0_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140058c,&reg_val);
            printf("passed_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400590,&reg_val);
            printf("passed_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400594,&reg_val);
            printf("failed1_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400598,&reg_val);
            printf("failed1_cnt1 : 0x%x\n",reg_val);               

            printf("*******************************END************************************\n");
#endif

               }
#if 1
                if(CSDEMUX_Filter_ReadWait(test_filterpmt,500) == CSAPI_SUCCEED){
                       if(CSDEMUX_Filter_ReadSectionData(test_filterpmt, buf2, &reg_val2)== CSAPI_SUCCEED)
                       {
                            if(firstin2){
                                    printf("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[67],buf2[68],buf2[69],buf2[70],reg_val2);
                                    firstin2 = 0; 
                            }
                            //else if((buf2[67] == b[0])&&(buf2[68] == b[1])&&(buf2[69] == b[2])&&(buf2[70] == b[3])){
                            //        ;
                            //}
                            else{
                                    printf("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf2[0],buf2[1],buf2[2],buf2[3],buf2[4],buf2[67],buf2[68],buf2[69],buf2[70],reg_val2);
                            }
                            b[0] = buf2[67];
                            b[1] = buf2[68];
                            b[2] = buf2[69];
                            b[3] = buf2[70];
                            if((buf2[0] == 0)&&(buf2[1] == 0)&&(buf2[2] == 0)){
                                printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                                return;
                            }
                       }
                }
                else{
                        printf("filter 1 timeout\n");

                        #if 0
            printf("\n*******************************START**********************************\n");
            
            ReadReg32(0x41400440 + (0 << 3), &wp);
            ReadReg32(0x41400444 + (0 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter0 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400440 + (1 << 3), &wp);
            ReadReg32(0x41400444 + (1 << 3), &rp); 

            if ((wp ^ rp) & 0x80000000) in_size = (wp &0x7fffffff) - (rp & 0x7fffffff) + 0x20000;
            else in_size = (wp & 0x7fffffff) - (rp & 0x7fffffff);
            printf("filter1 rp = 0x%08x  wp = 0x%08x data_in_buffer=%d \n", rp, wp, in_size);

            ReadReg32(0x41400544,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400548,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x4140054c,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400550,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x41400554,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400558,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x4140055c,&reg_val);
            printf("filter0 : 0x%x\t",reg_val);

            ReadReg32(0x41400560,&reg_val);
            printf("filter1 : 0x%x\t",reg_val);

            ReadReg32(0x41400564,&reg_val);
            printf("filter2 : 0x%x\n",reg_val);

            ReadReg32(0x41400568,&reg_val);
            printf("mask0 : 0x%x\t",reg_val);

            ReadReg32(0x4140056c,&reg_val);
            printf("mask1 : 0x%x\t",reg_val);

            ReadReg32(0x41400570,&reg_val);
            printf("mask2 : 0x%x\n",reg_val);

            ReadReg32(0x41400574,&reg_val);
            printf("pid_match_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400578,&reg_val);
            printf("pid_match_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140057c,&reg_val);
            printf("err_tag_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400580,&reg_val);
            printf("err_tag_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400584,&reg_val);
            printf("faild0_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400588,&reg_val);
            printf("failed0_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x4140058c,&reg_val);
            printf("passed_cnt0 : 0x%x\t",reg_val);
            
            ReadReg32(0x41400590,&reg_val);
            printf("passed_cnt1 : 0x%x\n",reg_val);
             
            ReadReg32(0x41400594,&reg_val);
            printf("failed1_cnt0 : 0x%x\t",reg_val);

            ReadReg32(0x41400598,&reg_val);
            printf("failed1_cnt1 : 0x%x\n",reg_val);               

            printf("*******************************END************************************\n");
#endif

                }
#endif
        }
        printf("task exit\n");
}
#endif

void PlayIFrame(void)
{
	int read_len = 0;
	unsigned int bufsize = 0;
	unsigned char  read_buf[4096+100];
	FILE*   vid_file=NULL;
	static int breakflag = 250;

	CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_MPEG2_TS);

	CSVID_SetDecoderMode(g_video_handle, VID_FRAME_SP);
	CSVID_Play(g_video_handle);
	CSVID_PFMOpen(g_video_handle);

	vid_file = fopen("/mnt/mj/qqq123-xp","rb");//mpeg2_mpa_480p_qianlizhiwai_video1.mpv//mpeg2_mpa_1080i_video1.mpv
	if(vid_file == NULL)
	{
		printf("======>file open failed\n");
		return CSAPI_FAILED;
	}
	fseek(vid_file,0L, SEEK_SET);
	if ((read_len=fread(read_buf,1,4096,vid_file))!=4096)
	{
		fseek(vid_file,0L, SEEK_SET);
		printf("read_len = %d ,===no data \n",read_len);
	}

	while(breakflag){
		//printf_mpeg2_status();
            if(CSAPI_SUCCEED == CSVID_WritePFMData(g_video_handle, read_buf, read_len)){
                if ((read_len=fread(read_buf,1,4096,vid_file))!= 4096)
                {
                	if(feof(vid_file)){
                    		fseek(vid_file,0L, SEEK_SET);
                    		printf("read_len = %d ,===no data \n",read_len);
				breakflag--;
                	}
			else{
				printf("asdasdasd\n");
			}
                }
                CSVID_GetPFMBufferSize(g_video_handle, &bufsize);
                //printf("bufsize = %d\n",bufsize);
            }
            else{
                usleep(5*1000);
            }
	}
}


int mixerflag = 1;
void mixertask(void)
{
        FILE *mixdata=NULL;
        unsigned char src[1048544];
        int readlen=0;
        CSAUD_MixerConfig mixer_config={250,AUD_SAMPLE_RATE_48KHZ};

        mixerflag = 1;
        mixdata=fopen("/mnt/stream/mixer3.pcm", "rb");
        if(NULL == mixdata)
        {
            printf("Can't open file mixdata\n");
            return;
        }
        fseek(mixdata, 0, SEEK_SET);
        readlen=fread(src,4, 4096, mixdata);//1048543  950272
        printf("11111111111 pointer is 0x%x ,read length is %d\n",src, readlen);
        CSAUD_EnableMixer(g_audio_handle);
        CSAUD_SetMixerConfig(g_audio_handle, &mixer_config);
        CSAUD_WriteMixerBuffer(g_audio_handle, src, readlen*4);
        while(mixerflag)
        {
                static int a = 0;
                readlen=fread(src,4, 4096, mixdata);

                if(readlen != 4096){
                        fseek(mixdata, 0, SEEK_SET);
                        printf("loop!\n");

                        CSAUD_DisableMixer(g_audio_handle);
                        break;

                        if(a%2 == 1){ 
                        //CSVID_Play(g_video_handle);
                        //CSAUD_Play(g_audio_handle);
                        }
                        else{
                        //CSVID_Stop(g_video_handle);
                        //CSAUD_Stop(g_audio_handle);
                        }
                        a++;
                }
                printf("pointer is 0x%x ,read length is %d\n",src, readlen);
                CSAUD_WriteMixerBuffer(g_audio_handle, src, readlen*4);
        }
}


void testvideocallbackE(CSVID_HANDLE handle)
{
        printf("empty video call back run!\n");
}

void testvideocallbackF(CSVID_HANDLE handle)
{
        printf("full video call back run!\n");
}


static void testvideosynccallback(CSVID_HANDLE handle, signed char * sync)
{
	int aaa = 0;

        if(*sync == 1){
                CSVID_SequenceHeader hdr;

		  aaa = CSVID_GetSequenceHeader(g_video_handle, &hdr);
			while(aaa == CSAPI_FAILED){
				aaa = CSVID_GetSequenceHeader(g_video_handle, &hdr);
				printf("hdr.w = %d,hdr.h = %d,hdr.frame_rate = %d\n",hdr.w,hdr.h,hdr.frame_rate);
			}
				printf("hdr.w = %d,hdr.h = %d,hdr.frame_rate = %d\n",hdr.w,hdr.h,hdr.frame_rate);
                src_rect.right = 0;//hdr.w;
                src_rect.bottom = 0;//hdr.h;
                if(hdr.h == 576){
			if(hdr.frame_rate > 25)
                        g_tvout_mode = TVOUT_MODE_576I;
			else
				g_tvout_mode = TVOUT_MODE_576I;
                        dst_rect.right=720;
                        dst_rect.bottom=576;
                }
                else if(hdr.h == 480){
			if(hdr.frame_rate > 25)
                        g_tvout_mode = TVOUT_MODE_480P;
			else
			  g_tvout_mode = TVOUT_MODE_480I;
                        dst_rect.right=720;
                        dst_rect.bottom=480;
                }
                else if(hdr.h == 720){
			if(hdr.frame_rate > 50)
                        g_tvout_mode = TVOUT_MODE_720P60;
			else
			  g_tvout_mode = TVOUT_MODE_720P50;
                        dst_rect.right=1280;
                        dst_rect.bottom=720;
                }
                else if((hdr.h > 1069)||(hdr.h < 1091)){
			if(hdr.frame_rate > 25)
                        g_tvout_mode = TVOUT_MODE_1080I30;
			else
			   g_tvout_mode = TVOUT_MODE_1080I25;
                        dst_rect.right=1920;
                        dst_rect.bottom=1088;
                }

			//g_tvout_mode = TVOUT_MODE_1080I25;
			//dst_rect.bottom=1080;
			//dst_rect.right=1920;
			//src_rect.right=1920;
			//src_rect.bottom=1080;
/*
			{
				dst_rect.left = 90;
				dst_rect.top= 100;
				dst_rect.right= 620;
				dst_rect.bottom= 380;//380;
			}
*/
#if 0
			dst_rect.right=1920;
                     dst_rect.bottom=1080;
			dst_rect.left = 0;
			dst_rect.top = 0;
			g_tvout_mode = TVOUT_MODE_1080I30;
		  
                CSTVOUT_SetMode(g_tvout_handle,TVOUT_MODE_1080I30);
#endif
	CSAUD_DisableMute(g_audio_handle);
		CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
                CSVID_SetOutputAlpha(g_video_handle, 0xff);
		CSOSD_Disable(g_osd_handle);
		CSOSD_Enable(g_osd_handle);
               CSOSD_Flip(g_osd_handle);
		  printf("eeeeeeeeeeeeeeeeeeeeeeeee\n");

	  CSOSD_SetAlpha(g_osd_handle, 0xb0);
        }
        else{
               CSVID_SetOutputAlpha(g_video_handle, 0x80);
                CSOSD_Flip(g_osd_handle);
                CSAUD_DisableMute(g_audio_handle);
		 printf("av not sync!!!!!!\n");
		CSVID_SyncNotify(g_video_handle, testvideosynccallback, 5000000, 1);
		printf("again !!!\n");
        }
}

void testcallback(CSAUD_HANDLE handle,CSAUD_ERROR_THRESHOLD a)
{
        printf("cal back run\n");
        CSAUD_EnableMute(handle);
}

void testafdcallback(CSVID_HANDLE handle,unsigned char * afd)
{
        printf("afd = %d\n",*afd);
}

void testvideocallback(CSVID_HANDLE handle )
{
        printf("video cal back run\n");
        CSVID_SequenceHeader hdr;
        CSVID_GetSequenceHeader(g_video_handle, &hdr);
        printf("w = %d, h = %d, frame rate = %d\n", hdr.w, hdr.h,hdr.frame_rate);
}

static int _gpio_write(char *devname, char* buf, int len )
{
    int gpio_fd;
    int retval;
    char cmd='O';

    puts(devname);
    gpio_fd = open(devname,O_RDWR);
    if (gpio_fd <= 0)
    {
        printf("Error: Open %s.\n",devname);
        return -1;
    }

    retval = write(gpio_fd, &cmd , 1);
    if (retval != 1)
    {
        printf("Error: Read %s. \n",devname);
        return -1;
    }

    retval = write(gpio_fd, buf , len);
    if (retval != len)
    {
        printf("Error: Read %s. \n",devname);
        return -1;
    }

    retval = close(gpio_fd);
    return retval;
}

static int tuner_gpio_reset(void)
{
    int retval;
    char value = '0';

    retval = _gpio_write("/dev/gpio/6",&value,1);
    usleep(400000);
    value = '1';
    retval = _gpio_write("/dev/gpio/6", &value,1);
    usleep(400000);

    return 0;	
}

int tuner_lock(unsigned int tuner_frequency, unsigned int tuner_symbol_rate, unsigned int  tuner_mod)
{
    int err_code = 0;
    int tuner_fd = 0;

    TUNER_PARAMS_S tuner_param;
    TUNER_STATUS_E tuner_status = 0;
    int            icount = 0;

    while(1)
    {
        tuner_gpio_reset();

        err_code = cs_tuner_init();
        if(err_code < 0)
        {
            printf("Error: cs_tuner_init.\n");
            return err_code;
        }
        printf("1 success cs_tuner_init. \n");

        tuner_fd = cs_tuner_open(TUNER_ID_0);
        if(tuner_fd < 0)
        {
            printf("Error: cs_tuner_init.\n");
            return tuner_fd;
        }	
        printf("1 success cs_tuner_open. \n");

        memset(&tuner_param, 0, sizeof(TUNER_PARAMS_S));
        tuner_param.frequency = tuner_frequency * 1000;
        tuner_param.qam_params.symbol_rate = tuner_symbol_rate;        
        if(tuner_mod==0)
            tuner_param.qam_params.modulation = QAM_32;
        else if(tuner_mod==1)
            tuner_param.qam_params.modulation = QAM_64;
        else if(tuner_mod==2)
            tuner_param.qam_params.modulation = QAM_128;
        else if(tuner_mod==3)
            tuner_param.qam_params.modulation = QAM_256;
        else
            tuner_param.qam_params.modulation = QAM_64;

        tuner_param.inversion = INVERSION_NORMAL;

        err_code = cs_tuner_set_params(tuner_fd, &tuner_param);
        if(err_code < 0)
        {
            printf("Error: cs_tuner_set_params.\n");
            return err_code;
        }	

        for(icount = 0; icount < 8; icount++)
        {
            tuner_status = 0;
            err_code = cs_tuner_read_status(tuner_fd, &tuner_status);
            if(err_code < 0)
            {
                printf("Error: cs_tuner_read_status.\n");
                return err_code;
            }
            if(tuner_status==1)  return tuner_status;                     
            sleep(2);
        }
    }

    return 0;
}

static void init(void)
{
	CSOSD_Config osd_config = {OSD_MODE_576I,OSD_COLOR_DEPTH_16,OSD_COLOR_FORMAT_RGB565};
	//open tvout device

	if(NULL == (g_tvout_handle = CSTVOUT_Open(0)))
	{
		printf("open tvout device Failed\n");
		return;
	}
#ifdef ARCH_CSM1201
	//CSTVOUT_SetOutput(g_tvout_handle, OUTPUT_MODE_YPBPR);
	//CSTVOUT_BindVID(g_tvout_handle, TVOUT_VID_DEV_0);
	//CSTVOUT_BindGFX(g_tvout_handle, TVOUT_OSD_LAYER_0);

	//{
	//	CSTVOUT_HANDLE g_tvout1_handle = NULL;
	//	g_tvout1_handle = CSTVOUT_Open(TVOUT_DEV1);
	//	CSTVOUT_SetOutput(g_tvout1_handle, OUTPUT_MODE_YPBPR);
	//	CSTVOUT_BindVID(g_tvout1_handle, TVOUT_VID_DEV_0);
	//	g_tvout_handle = g_tvout1_handle;
	//}
#endif

    g_tvout_mode = TVOUT_MODE_576I;
    CSTVOUT_SetMode(g_tvout_handle,g_tvout_mode);

    if(NULL == (g_osd_handle = CSOSD_Open(OSD_LAYER_0)))
    {
        printf("open osd device Failed\n");
        return;
    }
    CSOSD_SetConfig(g_osd_handle, &osd_config);
    CSOSD_SetAlpha(g_osd_handle, 0xff);
    CSOSD_Enable(g_osd_handle);

//open video
    if(NULL == (g_video_handle=CSVID_Open(VID_DEV_0))) 
    {
        printf("open video error\n");
        return;
    }
    else printf("open video ok\n");

//open audio    
    if(NULL == (g_audio_handle=CSAUD_Open(AUD_DEV_0)))
    {
        printf("open audio error\n");
        return;
    }
    else printf("open audio ok\n"); 

//open demux for video out
   if(NULL == (g_demux_video_handle=CSDEMUX_VID_Open(DEMUX_VIDOUT_ID0)))
    {
         printf("open demux for video out Failed ...\n");
         return ;
    }

//open demux for audio out
    if(NULL == (g_demux_audio_handle=CSDEMUX_AUD_Open(DEMUX_AUDOUT_ID0)))
    {
         printf("open demux for audio out Failed ...\n");
         return ;
    }

//get pid filter for video
    if(NULL == (g_demux_pidfilter_video_handle=CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID62)))
    {
        printf("get pid filter for video Failed ...\n");
        return ;
    }

//get pid filter for audio
    if(NULL == (g_demux_pidfilter_audio_handle=CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID63)))
    {
        printf("get pid filter for audio Failed ...\n");
    }

    //video decoder set
    CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
    CSVID_SetOutputAlpha(g_video_handle, 0x80);

    //audio decoder set
    CSAUD_Init(g_audio_handle);
    CSAUD_SetVolume(g_audio_handle,NULL);
}

static void play(int index)
{
    g_vpid= service[index].servce_vidpid;// = 512;//= 0x753;
    g_apid= service[index].servce_audpid;// = 650;//0x903; //= 0x112;

    if(g_tunerflag){
        g_vid_decodertype = service[index].vid_type;
        g_aud_decodertype = service[index].aud_type;
    }
    else{
        g_vid_decodertype = service[index].vid_type;
        g_aud_decodertype = service[index].aud_type;
    }
    printf("Set Program Video = 0x%x , Audio = 0x%x \n", g_vpid,g_apid);
    printf("Set Program g_vid_decodertype = 0x%x , g_aud_decodertype = 0x%x \n", g_vid_decodertype,g_aud_decodertype);
//demux PID Filter Config
    CSDEMUX_PIDFT_SetChannel(g_demux_pidfilter_video_handle,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(g_demux_pidfilter_video_handle,g_vpid);  

    CSDEMUX_PIDFT_SetChannel(g_demux_pidfilter_audio_handle,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(g_demux_pidfilter_audio_handle,g_apid);  

//demux AUD Output Config
    //CSDEMUX_AUD_SetCABBuf(g_demux_audio_handle,CAB_REGION,CAB_SIZE);
    //CSDEMUX_AUD_SetPTSBuf(g_demux_audio_handle,AUD_PTS_REGION,AUD_PTS_SIZE);

    if(g_tunerflag == 1)
        CSDEMUX_AUD_SetOutputMode(g_demux_audio_handle,DEMUX_OUTPUT_MOD_NONBLOCK);
    else 
        CSDEMUX_AUD_SetOutputMode(g_demux_audio_handle,DEMUX_OUTPUT_MOD_BLOCK);

    CSDEMUX_AUD_SetPID(g_demux_audio_handle,g_apid);

//demux VID Output Config
    //CSDEMUX_VID_SetCPBBuf(g_demux_video_handle,CPB0_REGION ,CPB0_SIZE);
    //CSDEMUX_VID_SetDIRBuf(g_demux_video_handle,CPB0_DIR_REGION,CPB0_DIR_SIZE);

    if(g_tunerflag == 1)
        CSDEMUX_VID_SetOutputMode(g_demux_video_handle,DEMUX_OUTPUT_MOD_NONBLOCK);
    else
        CSDEMUX_VID_SetOutputMode(g_demux_video_handle,DEMUX_OUTPUT_MOD_BLOCK);

    CSDEMUX_VID_SetPID(g_demux_video_handle,g_vpid);
    
    //CSDEMUX_WriteReg(0x41400000+(0x101<<2),CPB0_DIR_REGION>>3);
    //CSDEMUX_WriteReg(0x41400000+(0x105<<2),CAB_REGION>>3);

    //CSDEMUX_WriteReg(OUT_CHL0_DIR_WP_ADDR,CPB0_DIR_REGION>>3);
    //CSDEMUX_WriteReg(OUT_CHL2_DIR_WP_ADDR,CAB_REGION>>3);
    //CSDEMUX_WriteReg(OUT_CHL0_DIR_RP_ADDR,CPB0_DIR_REGION>>3);
    //CSDEMUX_WriteReg(OUT_CHL2_DIR_RP_ADDR,CAB_REGION>>3);

//start play
//getchar();
//printf("4\n");
#if 1
   CSAUD_Init(g_audio_handle);
//getchar();
//printf("5\n");

    if(g_aud_decodertype == 0)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_AC3);
    else if(g_aud_decodertype == 1)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_MPA);
    else if(g_aud_decodertype == 2)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_AAC);
    else if(g_aud_decodertype == 3)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_DTS);
    else if(g_aud_decodertype == 4)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_AAC_LATM);
    else if(g_aud_decodertype == 5)
        CSAUD_SetCodecType(g_audio_handle,AUD_STREAM_TYPE_LPCM);
    else printf("invalid audio stream type %d\n",g_aud_decodertype);
#endif
    if(g_vid_decodertype == 0)
         CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_H264_TS);
    else if(g_vid_decodertype == 1)
        CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_MPEG2_TS);
    else printf("invalid video stream type %d\n",g_vid_decodertype);

    if(g_tunerflag){
        CSVID_SetInputMode(g_video_handle, VID_INPUT_DEMUX0);
	 CSAUD_SetInputMode(g_audio_handle, AUD_INPUT_NOBLOCK);
	}
	else{
       CSVID_SetInputMode(g_video_handle, VID_INPUT_STILLPIC);
	CSAUD_SetInputMode(g_audio_handle, AUD_INPUT_BLOCK);
	}

    CSVID_EnablePTSSync(g_video_handle);
    CSAUD_EnablePTSSync(g_audio_handle);
    CSDEMUX_VID_Enable(g_demux_video_handle);
    CSDEMUX_AUD_Enable(g_demux_audio_handle);
    CSDEMUX_PIDFT_Enable(g_demux_pidfilter_video_handle);
    CSDEMUX_PIDFT_Enable(g_demux_pidfilter_audio_handle);
    //CSAUD_SetSampleRate(g_audio_handle, AUD_SAMPLE_RATE_48KHZ);
    //CSAUD_SetI2SFormat(g_audio_handle, 1);
    CSVID_WaitSync(g_video_handle, 1);
    //CSVID_Force3to2PollDown(g_video_handle, 0);
    //CSVID_StartDelay(g_video_handle,2250);
    //CSAUD_SetStartDelay(g_audio_handle, 800);
    	{
    		int audio_freq = 0;
		CSAUD_GetSampleRate(g_audio_handle, &audio_freq);
		printf(" audio sample rate = %d\n",audio_freq);
    	}
{
CSAUD_Volume vol;
vol.front_left = 70;
vol.front_right = 70;
vol.rear_left = 70;
vol.rear_right = 70;
vol.center = 70;
vol.lfe  = 70;
CSAUD_SetVolume(g_audio_handle, &vol);
}
//CSVID_AFDChangeNotify (g_video_handle,testafdcallback, 1 );

CSAUD_SetOutputDevice(g_audio_handle, AUD_OUTPUT_I2S_SPDIFPCM);
//getchar();
//printf("6\n");

	CSVID_Play(g_video_handle);
	CSAUD_EnableMute(g_audio_handle);
	CSAUD_Play(g_audio_handle);

//getchar();
//printf("7\n");

#if 0
{	
	unsigned int chl = 10;
	CSAUD_GetOutputChannel(g_audio_handle,&chl);
	printf("output channel is %d\n",chl);
	CSAUD_SetOutputChannel(g_audio_handle, 0);
}//CSAUD_SetOutputChannel(g_audio_handle, AUD_PCM_CHANNEL51);
#endif
#if 0
    while(1)
    {
        int reg_val;
        ReadReg32(0x41600000 + (0x23 << 2), &reg_val);
        if (reg_val & 0x1)
            break;
    }

    {
        CSVID_SequenceHeader hdr;
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
                g_tvout_mode = TVOUT_MODE_1080I25;
                dst_rect.right=1920;
                dst_rect.bottom=1080;
        }
        CSTVOUT_SetMode(g_tvout_handle,g_tvout_mode);
        CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
        CSVID_SetOutputAlpha(g_video_handle, 0x80);
        CSOSD_Flip(g_osd_handle);
    }
#else
  CSVID_SyncNotify(g_video_handle, testvideosynccallback, 5000000, 1);

#endif
}

static void stop(void)
{
	//CSAUD_EnableMute(g_audio_handle);
	
	CSVID_SetOutputAlpha(g_video_handle, 0x0);

//getchar();
//printf("1\n");

	CSDEMUX_PIDFT_Disable(g_demux_pidfilter_video_handle);
	CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
	CSDEMUX_VID_Disable(g_demux_video_handle);
	CSDEMUX_AUD_Disable(g_demux_audio_handle);

//getchar();
//printf("2\n");

	usleep(50*1000);

	CSVID_Stop(g_video_handle);
	CSAUD_Stop(g_audio_handle);

//getchar();
//printf("3\n");

	//usleep(500*1000);
	
}

static void Pat_table_parse(unsigned char* ptr,unsigned int len)
{
	int serverid;
	int pmtpid;
	int sectionlen;

	if(len<12||(ptr[1]&0x80)==0){
                printf("exit 1\n");
                return;
        }
	if(!(ptr[5]&0x1)){ 
                printf("exit 2\n");
                return;
       }
	sectionlen=((ptr[1]&0xf)<<8|ptr[2]);

	ptr+=8;
	int i=sectionlen-9;
	while(i>=4)
	{
		serverid=(ptr[0]<<8)|ptr[1];
		pmtpid=((ptr[2]&0x1f)<<8)|ptr[3];
		printf("server id = 0x%x, pmt pid = 0x%x\n",serverid,pmtpid);
                if(service_num<MAX_SERVICE_NUM&&serverid!=0)
                {
                   service[service_num].servce_id = serverid;
                   service[service_num].servce_pmtpid = pmtpid;
                   service[service_num].servce_vidpid = 0x1fff;
                   service[service_num].servce_audpid = 0x1fff;
                   service[service_num].pmt_complete = 0;
                   service_num++;
                }
		ptr+=4;
		i-=4;
	}
}

static void  Pmt_table_parse(unsigned char* ptr,unsigned int len)
{
    int l;
    int tmp;
    int vpid = 0x1fff;
    int apid = 0x1fff;
    int audionum=0;

    if(len<12||(ptr[1]&0x80)==0)  return;
    if(!(ptr[5]&0x1)) return;

    tmp=(((ptr[10]&0xf)<<8)|ptr[11])+12;
    l=len-tmp-4;
    ptr+=tmp;

    while(l>=5)
    {
  
        if(ptr[0]==0x01||ptr[0]==0x02||ptr[0]==0x1b){
            vpid=((ptr[1]&0x1f)<<8)|ptr[2];
            service[cur_pmt_parser_index].servce_vidpid = vpid;
            if(ptr[0] == 0x1b) service[cur_pmt_parser_index].vid_type = 0;
            else service[cur_pmt_parser_index].vid_type = 1;
        }
        else if((ptr[0]==0x03||ptr[0]==0x04 ||ptr[0]==0x81 ||ptr[0]==0x6a ||ptr[0]==0x06 ||ptr[0]==0xf || ptr[0]==0x11||ptr[0]==0x80 || ptr[0]==0x86)&&(audionum == 0)){
            apid=((ptr[1]&0x1f)<<8)|ptr[2];
            service[cur_pmt_parser_index].servce_audpid = apid;
            if(ptr[0] == 0x6a || ptr[0]==0x81) service[cur_pmt_parser_index].aud_type = 0;
		else if(ptr[0]==0xf) service[cur_pmt_parser_index].aud_type = 2;
		else if(ptr[0]==0x06 ||ptr[0]==0x86) service[cur_pmt_parser_index].aud_type = 3;
		else if(ptr[0]==0x11) service[cur_pmt_parser_index].aud_type = 4;
		else if(ptr[0]==0x80) service[cur_pmt_parser_index].aud_type = 5;
            else service[cur_pmt_parser_index].aud_type = 1;
            audionum++;
        }
        else{  
            printf("=====>invalid type:%d\n",ptr[0]);
        }
        tmp=(((ptr[3]&0xf)<<8)|ptr[4])+5;
        l-=tmp;
        ptr+=tmp;
    }
    printf("video pid = 0x%08x, audio pid = 0x%08x\n",vpid,apid);
}

static void Sdt_table_parse(unsigned char* ptr,unsigned int len)
{
    int l,tmp,i,descriptorlooplen,descriptorlen,namelen,templen;
    unsigned short serviceid;
    char* aaa=NULL;
    
    printf("start parse sdt!\n");
    if(len<12||(ptr[1]&0x80)==0)  return;
    if(!(ptr[5]&0x1)) return;

    tmp=((ptr[1]<<8)&0xf00)|ptr[2];
   // printf("tmp=%d\n",tmp);

    current_section_num=ptr[6];
    last_section_num=ptr[7];
    if(current_section_num == last_section_num) sdtcomplete=1;
    else sdtcomplete=0;

    printf("current_section_num %d, last_section_num %d\n",current_section_num,last_section_num);
    l=tmp-4-8;
    ptr+=11;
    
    while(l)
    {
        //getchar();
        serviceid=ptr[0]<<8 |ptr[1];
        templen=descriptorlooplen=((ptr[3]<<8)&0xf00)|ptr[4];
        printf("serviceid %d,descriptorlooplen %d\n",serviceid,descriptorlooplen);
        aaa=ptr;
        for(i=0;i<service_num;i++){
            if(service[i].servce_id == serviceid){
                aaa+=5;
                descriptorlen=aaa[1];
                if(aaa[0] == 0x48){
                    aaa+=3;
                    namelen=aaa[0];
                    printf("111namelen =%d \n",namelen);
                    aaa++;
                    if(namelen !=0){
                            memset((char*)service[i].providername,0,20);
                            memcpy((char*)service[i].providername,(char*)aaa,namelen);
                            printf("service name %s \n",(char*)service[i].providername);
                            aaa+=namelen;
                    }
                    namelen=aaa[0];
                    printf("222namelen =%d \n",namelen);
                    aaa++;
                    if(namelen !=0){
                            memset((char*)service[i].servicename,0,20);
                            memcpy((char*)service[i].servicename,(char*)aaa,namelen);
                            printf("service name %s \n",(char*)service[i].servicename);
                            aaa+=namelen;
                    }
                    //templen=templen-descriptorlen-2;
                }
                else{
                    printf("invalid descriptor 0x%x\n",aaa[0]);
                    aaa=aaa+descriptorlen+2;
                    //descriptorlooplen=descriptorlooplen-descriptorlen-2;
                }
            }
        }
        l=l-templen-5;
        ptr=ptr+templen+5;
    }
}

static void Search_Channel(void)
{
    CSDEMUX_HANDLE hfilter;   
    CSDEMUX_HANDLE hpidfilter;   
    unsigned char filter[12];
    unsigned char mask[12];
    unsigned int  data_lengtn = 0;
    int pmt_parsed_num = 0;

    unsigned int servce_id = 0;
    unsigned int servce_pmtpid = 0;
    unsigned int servce_vidpid = 0;
    unsigned int servce_audpid = 0;

    service_num = 0;
    cur_pmt_parser_index = 0;

    memset(filter, 0 ,12);
    memset(mask, 0 ,12);

    filter[0] = 0x00;
    mask[0]   = 0xff;

    hfilter = CSDEMUX_Filter_Open(DEMUX_FILTER_ID20);
    hpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0);

    CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(hpidfilter,PAT_PID);  
    CSDEMUX_PIDFT_Enable(hpidfilter);
    
    CSDEMUX_Filter_SetFilter(hfilter,filter,mask);
    CSDEMUX_Filter_AddPID(hfilter,PAT_PID);
    CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_Enable(hfilter);
    
    while(1)
    {
       data_lengtn = 1024;
       if(CSDEMUX_Filter_ReadWait(hfilter,500) == CSAPI_SUCCEED)
       {
           if(CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn)== CSAPI_SUCCEED)
           {
              printf("data size  = %d\n", data_lengtn);
              Pat_table_parse(section_buf,data_lengtn);
              break;
           }
       }
       else printf("PAT---------ReadWait Timeout\n");
    }
    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_Filter_Close(hfilter);
    printf("PAT complete! Service_num = %d \n",service_num);

    for( cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++)
    {
        if(service[cur_pmt_parser_index].servce_id==0) continue;

        hfilter = CSDEMUX_Filter_Open(cur_pmt_parser_index);
        hpidfilter = CSDEMUX_PIDFT_Open(cur_pmt_parser_index);
        service[cur_pmt_parser_index].filter_handle = hfilter;
        service[cur_pmt_parser_index].pidfilter_handle = hpidfilter;
        filter[0] = 0x02;
        mask[0]   = 0xff;
        
        CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter,service[cur_pmt_parser_index].servce_pmtpid);  
        CSDEMUX_PIDFT_Enable(hpidfilter);

        CSDEMUX_Filter_SetFilter(hfilter,filter,mask);
        CSDEMUX_Filter_AddPID(hfilter,service[cur_pmt_parser_index].servce_pmtpid);
        CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(hfilter);
    }
    
    pmt_parsed_num = service_num;
    printf("pmt_parsed_num = %d\n",pmt_parsed_num);
    while(pmt_parsed_num)
    {
        for( cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++)
        {
            if(service[cur_pmt_parser_index].pmt_complete) continue;
            data_lengtn = 1024;
            hfilter = service[cur_pmt_parser_index].filter_handle;
            if(CSDEMUX_Filter_ReadWait(hfilter,1000) == CSAPI_SUCCEED)
            {
                if(CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn)==CSAPI_SUCCEED)
                {
                    printf("data size  = %d\n", data_lengtn);
                    Pmt_table_parse(section_buf,data_lengtn);
                    service[cur_pmt_parser_index].pmt_complete = 1;
                    printf("PMT 0x%x parse complete !\n ",service[cur_pmt_parser_index].servce_pmtpid);
                    pmt_parsed_num--;
                    break;
                }
            }
        }
    }

    for( cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++)
    {
        servce_id = service[cur_pmt_parser_index].servce_id;
        servce_pmtpid= service[cur_pmt_parser_index].servce_pmtpid;
        servce_vidpid = service[cur_pmt_parser_index].servce_vidpid;
        servce_audpid = service[cur_pmt_parser_index].servce_audpid;
        CSDEMUX_PIDFT_Disable(service[cur_pmt_parser_index].pidfilter_handle);
        CSDEMUX_PIDFT_Close(service[cur_pmt_parser_index].pidfilter_handle);
        CSDEMUX_Filter_Disable(service[cur_pmt_parser_index].filter_handle);
        CSDEMUX_Filter_Close(service[cur_pmt_parser_index].filter_handle);
        printf("servce_id = 0x%x,  servce_pmtpid = 0x%x,  servce_vidpid = 0x%x, servce_audpid = 0x%x\n",servce_id,servce_pmtpid,servce_vidpid,servce_audpid);
    }
#if 0//request and parse sdt 
    sdtcomplete = 0;
    while(1)
    {
        int timeout_flag=1;
        
        if(sdtcomplete){ 
           printf("SDT complete! Service_num = %d \n",service_num);
           break;
        }
        memset(filter, 0 ,12);
        memset(mask, 0 ,12);
        filter[0] = 0x42;
        mask[0] = 0xff;
        if(current_section_num){
            filter[6] = current_section_num+1;
            mask[6] = 0xff;
        }
        else{
            filter[6] = 0x00;
            mask[6] = 0xff;
        }
        hfilter = CSDEMUX_Filter_Open(DEMUX_FILTER_ID17);
        hpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID17);
        CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter,SDT_PID);  
        CSDEMUX_PIDFT_Enable(hpidfilter);
        CSDEMUX_Filter_SetFilter(hfilter,filter,mask);
        CSDEMUX_Filter_AddPID(hfilter,SDT_PID);
        CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(hfilter);
        data_lengtn = 1024;
        timeout_flag=1;
        while(CSDEMUX_Filter_ReadWait(hfilter,5000) == CSAPI_SUCCEED)
        {
           timeout_flag=0;
           if(CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn)== CSAPI_SUCCEED)
           {
              printf("data size  = %d\n", data_lengtn);
              Sdt_table_parse(section_buf,data_lengtn);
              break;
           }
        }
        printf("SDT#########################Disable PIDFilter and Filter\n");
        CSDEMUX_PIDFT_Disable(hpidfilter);
        CSDEMUX_PIDFT_Close(hpidfilter);
        CSDEMUX_Filter_Disable(hfilter);
        CSDEMUX_Filter_Close(hfilter);
        if(timeout_flag)
        {
            printf("SDT---------ReadWait Timeout\n");
            break;
        }
    }
#endif    
}

static void __fileplay(FILE* ts_file)
{
    unsigned char  read_buf[TS_SIZE*20];
    int i = 0, read_len = 0;
    int err_cnt=0;
    unsigned char* buf_ptr;     

    if(ts_file!=NULL)
    {

START:
    	printf("ts_file : 0x%x",ts_file);
	fseek(ts_file, 0, SEEK_SET);
        while(1)
        {
            if ((read_len=fread(read_buf,1,1,ts_file))!= (1))
            {
                fseek(ts_file,0, SEEK_SET);
                printf("===Loop \n");
                if((read_len=fread(read_buf,1,1,ts_file))!= (1))
                {
                    printf("===Exit \n");
                    break;
                }
            }
            //read_buf[9] = 0xff;
            //read_buf[0] = 0xff;
            //read_buf[2] = 0xff;
            //read_buf[7] = 0xff;
            //read_buf[1] = 0xff;
            //read_buf[8] = 0xff;

#if 1
{
		while(read_buf[0] != 0x47){
			read_len = fread(read_buf, 1,1,ts_file);
			//printf("1 read_len = %d\n",read_len);
			if (read_len != 1 ){
				fseek(ts_file, 0, SEEK_SET);
				goto START;
			}
		}

		read_len += fread(&read_buf[1], 1,TS_SIZE*7 - 1,ts_file);
		//printf("2 read_len = %d\n",read_len);
		if (read_len != (TS_SIZE*7)){
			fseek(ts_file, SEEK_SET, 0);
				goto START;
		}

}
#endif

            	if(CSDEMUX_CHL_DMA_Write(g_demux_chl_handle,read_buf,read_len)!=CSAPI_SUCCEED)
			printf("===>Write Error \n");

            /*for debug*/
            buf_ptr = read_buf;

            for(i=0;i<7;i++)
            {
	            if(buf_ptr[0]!=0x47)  printf("===>0x47\n");

	            if((buf_ptr[1]&0x1f)==0x2 && buf_ptr[2]==0)
	            {
		            if((buf_ptr[3]&0xf) != (err_cnt&0xf))   printf("===>Lost Packet\n");
		            err_cnt = buf_ptr[3];
		            err_cnt++;
		     }
	            buf_ptr +=TS_SIZE;
            }
            /*debug end*/
        }
    fclose(ts_file);
    }
}

static void  __section_parser_init(unsigned int pid, unsigned char tabid)
{
    int i;
    mPID = pid;
    mTABID = tabid;
    mPos = 0;
    memset(mFilter,0,MAX_SECTION_FILTER_CHAR);
    memset(mMask,0,MAX_SECTION_FILTER_CHAR);
    for(i=0;i<MAX_SECTION_FILTER_CHAR;i++)
    mFilter[i]&=mMask[i];
}

static unsigned int __section_parser(const unsigned char *_bf,const unsigned long _size)
{  
   unsigned int pid=((_bf[1]&0x1f)<<8)|_bf[2];

   if(pid!=mPID||_size!=TS_SIZE)   return 0;
   int len=TS_SIZE;

   if(mPos>=3&&(mPos-3)>=(((mpBuff[1]&0xf)<<8)|mpBuff[2]))
   {
           mPos=(((mpBuff[1]&0xf)<<8)|mpBuff[2]);
           mPos+=3;
           return 1;
   }
   if((_bf[1]&0xc0)==0x40)
   {  
           mPos=0;
	   mNextCounter=(_bf[3]&0xf);
	   mIsChecked=0;
   }else if(mPos==0||mNextCounter!=(_bf[3]&0xf)||(_bf[1]&0x80))
   {
	   mPos=0;
	   return 0;
   }
   mNextCounter++;
   mNextCounter&=0xf;
   len-=4;
   if(_bf[3]&0x20)
   {
	   if(_bf[4]>=183) return 0;
	   len-=_bf[4]+1;
   }
   _bf+=TS_SIZE-len;
    
   if(mPos==0)
   {
        len-=_bf[0]+1;
        _bf+=_bf[0]+1;
        if(len<=0||_bf[0]!=mTABID) return 0;
   }
   if(mPos+len>MAX_SECTION_SIZE)
   {
	   mPos=0;
	   return 0;
   }
   memcpy(mpBuff+mPos,_bf,len);
   mPos+=len;
   if(!mIsChecked&&mPos>=MAX_SECTION_FILTER_CHAR+3)
   {
	   unsigned char *ptr=mpBuff+3;
           int i=0;
	   for(i=0;i<MAX_SECTION_FILTER_CHAR;i++)
	   {
		   if((ptr[i]&mMask[i])!=mFilter[i])
		   {
			   mPos=0;
		       return 0;
		   }
	   }
	   mIsChecked=1;
   }
   return 0;
}

static void __Paser_file(FILE* ts_file)
{
    unsigned char buf[TS_SIZE];
    service_num = 0;
    cur_pmt_parser_index = 0;
    if(ts_file==NULL) return;
    
    printf("========> Paser_file <========1\n");
    
    printf("========> Paser_PAT <========2\n");
    __section_parser_init(PAT_PID, PAT_TAB_ID);
    fseek(ts_file,0L, SEEK_SET);
    while(1)
    {
         if(fread(buf,1,TS_SIZE,ts_file)!=TS_SIZE) break;
         if(__section_parser(buf,TS_SIZE)) 
        {
            Pat_table_parse(mpBuff,mPos);
            break;
        }
    }

    printf("========> Paser_PMT<========3\n");
    for(cur_pmt_parser_index =0; cur_pmt_parser_index< service_num; cur_pmt_parser_index++)
    {
        if(service[cur_pmt_parser_index].servce_id == 0 ) continue;
        __section_parser_init(service[cur_pmt_parser_index].servce_pmtpid, PMT_TAB_ID);
        //fseek(ts_file,0L, SEEK_SET);
        while(1)
        {
            if(fread(buf,1,TS_SIZE,ts_file)!=TS_SIZE) break;
    	    if(__section_parser(buf,TS_SIZE)) 
    	    {
                Pmt_table_parse(mpBuff,mPos);
    	        break;
    	    }
        }
    }

#if 0
    printf("========> Paser_SDT<========4\n");
    __section_parser_init(SDT_PID, SDT_TAB_ID);
    fseek(ts_file,0L, SEEK_SET);
    while(1)
    {
        if(fread(buf,1,188,ts_file)!=188) break;
        if(__section_parser(buf,188)) 
        {
            Sdt_table_parse(mpBuff,mPos);
            break;
        }
    }
#endif
}


pthread_t read_thread;
static void Read_Data(CS_Test_Section_t *para)
{
    unsigned int  reg_val;
    unsigned char buf[1024];
    CSDEMUX_HANDLE hfilter;

    hfilter = para->filter_handle;
    while(1)
    {
        reg_val = 1024;
        memset(buf,0,1024);
        if(CSDEMUX_Filter_ReadWait(hfilter,500) == CSAPI_SUCCEED)
        {
            if(CSDEMUX_Filter_ReadSectionData(hfilter, buf, &reg_val)== CSAPI_SUCCEED)
            {
                printf("filter index = %d\n",para->testfilter_index);
                printf("filter:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[6],buf[7],buf[8],buf[9],reg_val);
            }
        }
        else printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>filter%d:ReadWait Timeout\n",para->testfilter_index);
    }
}

static void Strong_Multi_Section(void)
{
    CSDEMUX_HANDLE  hfilter0,    hfilter1;   
    CSDEMUX_HANDLE  hpidfilter0, hpidfilter1;
    int i; 
    unsigned char filter[12]; 
    unsigned char mask[12];
    unsigned short pid=0;
    
    for(i = 0;i<CURRENT_SERVICE_NUM;i++)
    {
        hfilter0 = testfilter[i].filter_handle = CSDEMUX_Filter_Open(i);
        hpidfilter0 = testfilter[i].pidfilter_handle = CSDEMUX_PIDFT_Open(i);

        switch(i)
        {
            case 0://nit actual
                pid = 0x10;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x40;
                mask[0] = 0xff;
                break;
            case 1://nit other
                pid = 0x10;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x41;
                mask[0] = 0xff;    
                break;
            case 2://bat
                pid = 0x11;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x4a;
                mask[0] = 0xff;    
                break;
            case 3://sdt actual
                pid = 0x11;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x42;
                mask[0] = 0xff;
                break;
            case 4://sdt other
                //goto default1;
                pid = 0x11;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x46;
                mask[0] = 0xff;    
                break;
            case 5://pat
                pid = 0x00;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x00;
                mask[0] = 0xff;
                break; 
            case 6://cat
                pid = 0x01;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x01;
                mask[0] = 0xff;    
                break;
            case 7://tdt
                pid = 0x14;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x70;
                mask[0] = 0xff;
                break;
            case 8://eit p/f actual
                pid = 0x12;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x4e;
                mask[0] = 0xff;    
                break;
            case 9://eit p/f other
                pid = 0x12;//0x12;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x4f;
                mask[0] = 0xff;
                break;
            case 10://eit schedule actual
                pid = 0x12;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x50;
                mask[0] = 0xf0;    
                break;
            case 11://eit schedule other
                pid = 0x12;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x60;
                mask[0] = 0xf0;    
                break;
            case 12://pmt
                pid = 0x401;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 13://pmt
                pid = 0x3f7;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 14://pmt
                pid = 0x43d;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 15://pmt
                pid = 0x433;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 16://pmt
                pid = 0x429;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 17://pmt
                pid = 0x41f;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 18://pmt
                pid = 0x41f;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 19://pmt
                pid = 0x415;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
            case 20://pmt
                pid = 0x40b;
                memset(filter,0,12);
                memset(mask,0,12);
                filter[0] = 0x02;
                mask[0] = 0xff;    
                break;
                
            default1:
            default:
                break;
        }

        CSDEMUX_PIDFT_SetChannel(hpidfilter0,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter0,pid); 

        CSDEMUX_Filter_SetFilter(hfilter0, filter, mask);
        CSDEMUX_Filter_AddPID(hfilter0,pid);
        CSDEMUX_Filter_SetFilterType(hfilter0,DEMUX_FILTER_TYPE_SEC);

        CSDEMUX_FILTER_SetSectionNotify(hfilter0, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);
        CSDEMUX_PIDFT_Enable(hpidfilter0);
        CSDEMUX_Filter_Enable(hfilter0);


#if 0//test multi-thread
        {
            int error=0;

            testfilter[i].testfilter_index=i;
            error = pthread_create(&read_thread,NULL,(void *)Read_Data,&testfilter[i]);
            printf("error %d\n",error);
            if(error != 0)
            {
                printf("CSSI_Nit_Thread  Create Fail \n");
            }
        }
    }
    while(i){}
#else
    }

            while(1){sleep(5);}

            
    unsigned int  reg_val;
    unsigned char buf[1024];

   
    
    while(1)
    {
        for(i = 0;i<CURRENT_SERVICE_NUM;i++)
        {
            reg_val = 1024;
            memset(buf,0,1024);
            hfilter0 = testfilter[i].filter_handle;
            if(CSDEMUX_Filter_ReadWait(hfilter0,500) == CSAPI_SUCCEED)
            {
                if(CSDEMUX_Filter_ReadSectionData(hfilter0, buf, &reg_val)== CSAPI_SUCCEED)
                {
                    printf("i = %d\n",i);
                    printf("filter:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[6],buf[7],buf[8],buf[9],reg_val);
                    if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                        printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   %d   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n",i);
                        return;
                    }
                }
            }
            else printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>filter%d:ReadWait Timeout\n",i);
        }
    }

    CSDEMUX_PIDFT_Disable(hpidfilter0);
    CSDEMUX_Filter_Disable(hfilter0);
    CSDEMUX_PIDFT_Close(hpidfilter0);
    CSDEMUX_Filter_Close(hfilter0);

    CSDEMUX_PIDFT_Disable(hpidfilter1);
    CSDEMUX_Filter_Disable(hfilter1);
    CSDEMUX_PIDFT_Close(hpidfilter1);
    CSDEMUX_Filter_Close(hfilter1);
#endif
}

CSDEMUX_HANDLE  hfilter0,    hfilter1;
int test_demux_flag = 1;

static void Get_Multi_Section (void)
{
    //CSDEMUX_HANDLE  hfilter0,    hfilter1;   
    CSDEMUX_HANDLE  hpidfilter0, hpidfilter1;
   
    unsigned char filter0[12]; 
    unsigned char filter1[12];
    unsigned char mask0[12];
    unsigned char mask1[12];

    memset(filter0, 0 ,12);
    memset(mask0, 0 ,12);
    
    memset(filter1, 0 ,12);
    memset(mask1, 0 ,12);

    //filter0[0] = 0x80;
    //mask0[0]  = 0xff;
    //filter0[1] = 0x00;
    //mask0[1]   = 0xff;
    
    //filter1[0] = 0x02;
    //mask1[0]  = 0xff;


    hfilter0 = CSDEMUX_Filter_Open(3);
    hfilter1 = CSDEMUX_Filter_Open(8);

#if 1    
    hpidfilter0 = CSDEMUX_PIDFT_Open(3);
    hpidfilter1 = CSDEMUX_PIDFT_Open(8);

    CSDEMUX_PIDFT_SetChannel(hpidfilter0,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetChannel(hpidfilter1,DEMUX_CHL_ID0);

    CSDEMUX_PIDFT_SetPID(hpidfilter0,0x425);  
    CSDEMUX_PIDFT_SetPID(hpidfilter1,0x41b);  

    CSDEMUX_PIDFT_Enable(hpidfilter0);
    CSDEMUX_PIDFT_Enable(hpidfilter1);
#endif

    CSDEMUX_Filter_SetFilter(hfilter0, filter0, mask0);
    CSDEMUX_Filter_SetFilter(hfilter1, filter1, mask1);

    CSDEMUX_Filter_AddPID(hfilter0,0x425);
    CSDEMUX_Filter_AddPID(hfilter1,0x41b);

    CSDEMUX_Filter_SetFilterType(hfilter0,DEMUX_FILTER_TYPE_PES);
    CSDEMUX_Filter_SetFilterType(hfilter1,DEMUX_FILTER_TYPE_PES);

    //CSDEMUX_FILTER_SetSectionNotify(hfilter0, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);
    //CSDEMUX_FILTER_SetSectionNotify(hfilter1, testdemuxcallback, DEMUX_SECTION_AVAIL, 1);

    CSDEMUX_Filter_Enable(hfilter0);
    CSDEMUX_Filter_Enable(hfilter1);

    while(0){sleep(5);}

    printf("cccccccccccccccc\n");
    unsigned int  reg_val;
    unsigned char buf[2048];
    test_demux_flag = 1;
    
    while(test_demux_flag)
    {
    #if 1
       reg_val = 1024;
       memset(buf,0,2048);
       if(CSDEMUX_Filter_ReadWait(hfilter0,500) == CSAPI_SUCCEED)
       {
           if(CSDEMUX_Filter_ReadSectionData(hfilter0, buf, &reg_val)== CSAPI_SUCCEED)
           {
                printf("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[6],buf[7],buf[8],buf[9],reg_val);
                if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                    printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
           }
       }
       else printf("filter0:ReadWait Timeout\n");

       reg_val = 1024;
       memset(buf,0,2048);
       if(CSDEMUX_Filter_ReadWait(hfilter1,500) == CSAPI_SUCCEED)
       {
           if(CSDEMUX_Filter_ReadSectionData(hfilter1, buf+1024, &reg_val)== CSAPI_SUCCEED)
           {
                printf("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", 
                    buf[SECTION_LENGTN],buf[SECTION_LENGTN+1],buf[SECTION_LENGTN+2],buf[SECTION_LENGTN+3],buf[SECTION_LENGTN+4],
                    buf[SECTION_LENGTN+6],buf[SECTION_LENGTN+7],buf[SECTION_LENGTN+8],buf[SECTION_LENGTN+9],reg_val);
                if((buf[SECTION_LENGTN] == 0)&&(buf[SECTION_LENGTN+1] == 0)&&(buf[SECTION_LENGTN+2] == 0)){
                    printf("AAAAAAAAAAAAAAAAAAAAAAAAAA   2   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
           }
       }
       else printf("filter1:ReadWait Timeout\n");
       #else
            
       #endif
    }

    CSDEMUX_PIDFT_Disable(hpidfilter0);
    CSDEMUX_Filter_Disable(hfilter0);
    CSDEMUX_PIDFT_Close(hpidfilter0);
    CSDEMUX_Filter_Close(hfilter0);

    CSDEMUX_PIDFT_Disable(hpidfilter1);
    CSDEMUX_Filter_Disable(hfilter1);
    CSDEMUX_PIDFT_Close(hpidfilter1);
    CSDEMUX_Filter_Close(hfilter1);
    printf("filter close\n");
}

static void Get_Section(int pid_ft_id, int sec_ft_id, int pid, int tabid)
{
    CSDEMUX_HANDLE  hfilter;   
    CSDEMUX_HANDLE  hpidfilter;

    unsigned char filter[12];
    unsigned char mask[12];

    memset(filter, 0 ,12);
    memset(mask, 0 ,12);

    filter[0] = tabid&0xff;
    mask[0]  = 0xff;
    filter[3] = 0x00;
    mask[3] = 0xff;
    filter[6] = 0x00;
    mask[6] = 0xff;

    hfilter = CSDEMUX_Filter_Open(sec_ft_id);

    hpidfilter = CSDEMUX_PIDFT_Open(pid_ft_id);

    CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(hpidfilter,pid);  
    CSDEMUX_PIDFT_Enable(hpidfilter);

    CSDEMUX_Filter_SetFilter(hfilter,filter,mask);
    CSDEMUX_Filter_AddPID(hfilter,pid);
    CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_Enable(hfilter);

    unsigned int  reg_val;
    unsigned char buf[MAX_SECTION_LENGTN];
    
    while(1)
    {
       reg_val = 1024;
       if(CSDEMUX_Filter_ReadWait(hfilter,500) == CSAPI_SUCCEED)
       {
           if(CSDEMUX_Filter_ReadSectionData(hfilter, buf, &reg_val)== CSAPI_SUCCEED)
           {
                printf("section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n", buf[0],buf[1],buf[2],buf[3],buf[4],buf[6],buf[7],buf[8],buf[9],reg_val);
                if((buf[0] == 0)&&(buf[1] == 0)&&(buf[2] == 0)){
                    printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
           }
       }
       else printf("ReadWait Timeout\n");
    }
    
    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Close(hfilter);
}

#define	RECORDBUFFERSIZE		0x100000*2
#define READBUFFERSIZE		188*256
#define	TSBUFFERSIZE		READBUFFERSIZE*16
static int record_flag = 1;
static void Record_test(unsigned int service_id)
{
        FILE* record_file;
        unsigned int  reg_val,reg_val1,reg_val2,written_len;
//        unsigned char buf[RECORDBUFFERSIZE];
        CSDEMUX_HANDLE  hfilter;   
        CSDEMUX_HANDLE  hvpidfilter,hapidfilter,hpatfilter,hpmtfilter;
        int vpid = service[service_id].servce_vidpid;//0x200;//0x3fc;
        int apid = service[service_id].servce_audpid;//0x201;//0x3fd;
        char * recordbuf = NULL;

        recordbuf = malloc(RECORDBUFFERSIZE);
        if(NULL == recordbuf){
            printf("error: %s  %s\n",__FUNCTION__,strerror(errno));
            return;
        }

        record_file = fopen("/mnt/stream/record_test2.ts", "w+b");
        hfilter = CSDEMUX_Filter_Open(DEMUX_FILTER_ID2);

#if 1
        hvpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID10);
        hapidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID11);
        hpatfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID3);
        hpmtfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID4);
        
        CSDEMUX_PIDFT_SetChannel(hvpidfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetChannel(hapidfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetChannel(hpatfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetChannel(hpmtfilter,DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hvpidfilter,vpid);
        CSDEMUX_PIDFT_SetPID(hapidfilter,apid);
        CSDEMUX_PIDFT_SetPID(hpatfilter,PAT_PID);
        CSDEMUX_PIDFT_SetPID(hpmtfilter,service[service_id].servce_pmtpid);
        CSDEMUX_PIDFT_Enable(hvpidfilter);
        CSDEMUX_PIDFT_Enable(hapidfilter);
        CSDEMUX_PIDFT_Enable(hpatfilter);
        CSDEMUX_PIDFT_Enable(hpmtfilter);
#endif

        CSDEMUX_Filter_AddPID2(hfilter, vpid, 0);
        CSDEMUX_Filter_AddPID2(hfilter, apid, 1);
        CSDEMUX_Filter_AddPID2(hfilter, PAT_PID, 2);
        CSDEMUX_Filter_AddPID2(hfilter, service[service_id].servce_pmtpid, 3);
        CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_TS);
        CSDEMUX_Filter_Enable(hfilter);
        reg_val1 = 0;
        reg_val2 = 0;
        
        while(record_flag){
                #if 0
                reg_val = 188;
                reg_val1++;
                printf("reg_val1 = %d\n",reg_val1);
                if (reg_val1 == 50){
                    reg_val1 = 0;
                    reg_val2 ++;
                    printf("reg_val2 = %d\n",reg_val2);
                    if(reg_val2%2){
                        printf("11111reg_val2 = %d\n",reg_val2);
                            CSDEMUX_PIDFT_Disable(hpatfilter);
                            CSDEMUX_PIDFT_SetPID(hpatfilter,service[service_id].servce_pmtpid);
                            CSDEMUX_PIDFT_Enable(hpatfilter);
                            CSDEMUX_Filter_AddPID2(hfilter, service[service_id].servce_pmtpid, 2);
                    }
                    else{
                        printf("22222reg_val2 = %d\n",reg_val2);
                            CSDEMUX_PIDFT_Disable(hpatfilter);
                            CSDEMUX_PIDFT_SetPID(hpatfilter,PAT_PID);
                            CSDEMUX_PIDFT_Enable(hpatfilter);
                            CSDEMUX_Filter_AddPID2(hfilter, PAT_PID, 2);
                    }
                }
                #endif
                if(CSDEMUX_Filter_ReadWait(hfilter,50) == CSAPI_SUCCEED)
                {
                   if(CSDEMUX_Filter_CheckDataSize(hfilter, &reg_val) == CSAPI_SUCCEED)
                   {
                            //printf("check size %d\n",reg_val);
                         if(CSDEMUX_Filter_ReadData(hfilter, recordbuf, &reg_val) == CSAPI_SUCCEED)
                         {
                                //printf("read size %d\n",reg_val);
                                written_len = fwrite(recordbuf, 1, reg_val, record_file);
                                if(written_len != reg_val) printf("written = %d\n",written_len);
                         }
                        //printf("section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x , ReadDate Size = %d\n", buf[0],buf[1],buf[2],buf[3],buf[4],reg_val);
                   }
                }
                else printf("ReadWait Timeout\n");
        }
}

static void Get_TS(int pidft_id,int ft_id, int pid)
{
    CSDEMUX_HANDLE  hfilter;   
    CSDEMUX_HANDLE  hpidfilter;

    struct timeval start,end;
    int timeuse=0;
   
    hfilter = CSDEMUX_Filter_Open(ft_id);
    //hpidfilter = CSDEMUX_PIDFT_Open(pidft_id);

    //CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
    //CSDEMUX_PIDFT_SetPID(hpidfilter,pid);  
    //CSDEMUX_PIDFT_Enable(hpidfilter);
    
    CSDEMUX_Filter_AddPID(hfilter,pid);
    CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_TS);
    CSDEMUX_Filter_Enable(hfilter);

    unsigned int  reg_val;
    unsigned char buf[4100];
    int num1 = 0, num2 = 0;
    gettimeofday(&start, NULL);
    
    while(1)
    {
/*test CSDEMUX_CHL_SetInputMode
       if((10 == num1)||(40 == num1)||(70 == num1))   
       {
            printf("CSDEMUX_CHL_SetInputMode : DEMUX_INPUT_MOD_TUNER");
            if(CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_TUNER)) 
                printf("CSDEMUX_CHL_SetInputMode Failed! \n");
       }
       else if((20 == num1)||(50 == num1))   
       {
            printf("CSDEMUX_CHL_SetInputMode : DEMUX_INPUT_MOD_DIRECT");
            if(CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_DIRECT)) 
                printf("CSDEMUX_CHL_SetInputMode Failed! \n");
       }
       else if((30 == num1)||(60 == num1))   
       {
            printf("CSDEMUX_CHL_SetInputMode : DEMUX_INPUT_MOD_DMA");
            if(CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_DMA)) 
                printf("CSDEMUX_CHL_SetInputMode Failed! \n");
       }
*/

/*test CSDEMUX_Init,CSDEMUX_Terminate
       if(10 == num1)
       {
            printf("close demux\n");
            CSDEMUX_Terminate();
       }
       if(20 == num1)
       {
            printf("init demux\n");
            CSDEMUX_Init();
       }
*/

       //printf("num1 = %d, num2 = %d, timeuse = %d\n",num1,num2,timeuse);
       num1++;
       
       reg_val = 188*8;
       if(CSDEMUX_Filter_ReadWait(hfilter,500) == CSAPI_SUCCEED)
       {
           if(CSDEMUX_Filter_ReadData(hfilter, buf, &reg_val) == CSAPI_SUCCEED)
           {
                printf("section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x , ReadDate Size = %d\n", buf[0],buf[1],buf[2],buf[3],buf[4],reg_val);
                num2++;
           }
       }
       else printf("ReadWait Timeout\n");

        gettimeofday(&end, NULL);
        timeuse=end.tv_sec-start.tv_sec;
        if(timeuse == 60) break;

    }

    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Close(hfilter);
}


void testosd(void)
{
	char *addr;
	CSOSD_GetBaseAddr(g_osd_handle, &addr);

	int 	ts_file = NULL;
	int	read_len = 0;
	char read_buf[720*576*4];
	//ts_file = open("/mnt/mj/UTMC_720x480.argb8888",O_RDWR);
	ts_file = open("/mnt/mj/720.raw",O_RDWR);
	if (ts_file == -1)
	{
		printf( "----------Error open input device.\r\n" );
		return( -1 );
	}

	if(ts_file!= -1)
	{
		printf("ts_file : 0x%x\n",ts_file);
#if 1
		lseek(ts_file, 0, SEEK_SET);
			
		if( (read_len = read(ts_file,read_buf,720*576*4)) ==720*576*4)
		{
			printf("read OK!! lenth is : %d\n",read_len);
		}
		else
		{
			printf("read less lenth is : %d!!\n",read_len);
		}
#else
		int	i;
		read_len = 720*576*4;
		for(i=0;i<0;  )
		{
			if(i%(720*4)  <(400*4))
			{
				memset(&read_buf[i], 0xff, 1);
				memset(&read_buf[i+1], 0x0, 1);
				memset(&read_buf[i+2], 0x0, 1);
				memset(&read_buf[i+3], 0x80, 1);
			}

			i+=4;
		}
		memset(read_buf, 0xff, 720*576*4);
#endif

		memcpy(addr,read_buf,read_len);
printf("memcpy----- lenth is : %d!!\n",read_len);
	CSOSD_Flip(g_osd_handle);
CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_1080I25);

		close(ts_file);
	}
	else
	{
		printf("PVR_fileplay openfile failed\n");
		close(ts_file);
		return;
	}
}



#if 1
static int TestWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    char buf[200];
    int  i,y = 40;
    int  key_num;
    static     LOGFONT * my_logfont=NULL;
    static     int show_flag=0,mute_flag=0;
    static     RECT     prc;
    
    static     int  aud_vol = 70;
    static int alpha_local = 0;
    static     int  flag=0;
        
    if(my_logfont==NULL)
    {
        my_logfont = CreateLogFont(NULL,"song", "GB2312", 
        FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN,FONT_SETWIDTH_NORMAL,
        FONT_SPACING_CHARCELL,FONT_UNDERLINE_NONE,FONT_STRUCKOUT_NONE,
        96,0);
    }

    switch (message) {
        case MSG_CREATE:
            GetWindowRect(hWnd, &prc);
            return 0;
        case MSG_PAINT:
            hdc = BeginPaint (hWnd);
            SetBkMode(hdc,BM_TRANSPARENT);
            SetTextColor(hdc, RGB2Pixel(hdc,100,22,22)); 
            //SelectFont(hdc,my_logfont);   // GetSystemFont(SYSLOGFONT_FIXED));
            memset(buf,0,200);
            for(i=0;i<service_num;i++)
            {
                sprintf(buf, "%s, vpid: %d, apid: %d",service[i].servicename, service[i].servce_vidpid,service[i].servce_audpid);
                TextOut (hdc, 20, y , buf);
                y+=20;
            }
            EndPaint (hWnd, hdc);
            CSOSD_Flip(g_osd_handle);
	    return 0;
        case MSG_KEYDOWN:

/*
            if(wParam==SCANCODE_0)
            {
                unsigned int brightness;
                CSTVOUT_GetBrightness(tve_handle,&brightness);
                printf("=========> brightness = %d\n", brightness);
                brightness+=10;
                CSTVOUT_SetBrightness(tve_handle,brightness);
            }
            else if(wParam==SCANCODE_1)
            {
                unsigned int contrast;
                CSTVOUT_GetContrast(tve_handle,&contrast);
                printf("=========> contrast = %d\n", contrast);
                contrast+=10;
                CSTVOUT_SetContrast(tve_handle,contrast);
            }
            else if(wParam==SCANCODE_2)
            {
                unsigned int sturation;
                CSTVOUT_GetSturation(tve_handle,&sturation);
                printf("=========> sturation = %d\n", sturation);
                sturation+=10;
                CSTVOUT_SetSturation(tve_handle,sturation);
            }
            else if(wParam==SCANCODE_3)
            {
                unsigned int brightness;
                CSTVOUT_GetBrightness(tve_handle,&brightness);
                printf("=========> brightness = %d\n", brightness);
                brightness-=10;
                CSTVOUT_SetBrightness(tve_handle,brightness);
            }
            else if(wParam==SCANCODE_4)
            {
                unsigned int contrast;
                CSTVOUT_GetContrast(tve_handle,&contrast);
                printf("=========> contrast = %d\n", contrast);
                contrast-=10;
                CSTVOUT_SetContrast(tve_handle,contrast);
            }
            else if(wParam==SCANCODE_5)
            {
                unsigned int sturation;
                CSTVOUT_GetSturation(tve_handle,&sturation);
                printf("=========> sturation = %d\n", sturation);
                sturation-=10;
                CSTVOUT_SetSturation(tve_handle,sturation);
            }
*/
            if(wParam>=SCANCODE_1&&wParam<=SCANCODE_0)
            {
                if(wParam == SCANCODE_0) key_num = 0;
                else key_num = wParam - SCANCODE_1 + 1;
                printf("get input key = %d\n", key_num);
                stop();
                play(key_num);
            }
            else if(wParam == 28) //ok
            {
                if(show_flag) { 
                    errno_api = CSOSD_Enable(g_osd_handle);
                    printf("CSOSD_Enable error %d\n", errno_api);
                    //errno_api = CSOSD_Flip(g_osd_handle);
                    //printf("CSOSD_Flip error %d\n", errno_api);
                }
                else {
			int audio_freq = 0;
                    errno_api = CSOSD_Disable(g_osd_handle);
                    printf("CSOSD_Disable error %d\n", errno_api);
                    //errno_api = CSOSD_Flip(g_osd_handle);
                    //printf("CSOSD_Flip error %d\n", errno_api);
			
			CSAUD_GetSampleRate(g_audio_handle, &audio_freq);
					printf(" audio sample rate = %d\n",audio_freq);
		}
                show_flag = 1-show_flag;
            }
            else if(SCANCODE_CURSORLEFT==wParam)
            {
                static int flag = 0;
                CSAUD_Volume vol;
                if(aud_vol>=2)  aud_vol -=2;
                vol.front_left = aud_vol;
                vol.front_right = aud_vol;
                vol.rear_left = aud_vol;
                vol.rear_right = aud_vol;
                vol.center = aud_vol;
                vol.lfe  = aud_vol;
                printf("Set Volume = %d\n", aud_vol);
                CSAUD_SetVolume(g_audio_handle, &vol);
            }
            else if(SCANCODE_CURSORRIGHT==wParam)
            {
                CSAUD_Volume vol;
                if(aud_vol<90) aud_vol +=2;
                vol.front_left = aud_vol;
                vol.front_right = aud_vol;
                vol.rear_left = aud_vol;
                vol.rear_right = aud_vol;
                vol.center = aud_vol;
                vol.lfe  = aud_vol;
                printf("Set Volume = %d\n", aud_vol);
                CSAUD_SetVolume(g_audio_handle, &vol); 
            #if 1
                {
                        CSVID_SequenceHeader hdr;
                        CSVID_GetSequenceHeader(g_video_handle, &hdr);
                        printf("w = %d, h = %d, frame rate = %d\n", hdr.w, hdr.h,hdr.frame_rate);
                }
            #endif
            }
            else if(30 == wParam)//mute
            {
            #if 0
                //CSAUD_DisablePTSSync(g_audio_handle);
                CSVID_ErrNotify(g_video_handle, testvideocallback, 1, 1);
            #else
                if(mute_flag) CSAUD_EnableMute(g_audio_handle);
                else CSAUD_DisableMute(g_audio_handle);
                printf("audio mute status %d\n",mute_flag);
                mute_flag=1-mute_flag;
             #endif
            }
             else if(32 == wParam)//fav
            {
            #if 0//test CSAUD_Play and Stop
                if(mute_flag) CSAUD_Play(g_audio_handle);
                else CSAUD_Stop(g_audio_handle);
                mute_flag=1-mute_flag;
           #else//test CSVID_GetTimeCode
                unsigned int timecode = 0;
                CSVID_GetTimeCode(g_video_handle,&timecode);
                printf("user space Time Code = 0x%x\n",timecode);
                //CSAUD_EnablePTSSync(g_audio_handle);
           #endif
            }
             else if(35 == wParam)//audio
            {
            #if 1//test CSAUD_SetOutPutChannel
                flag=flag%4;
                CSAUD_SetOutputChannel(g_audio_handle, flag);
                printf("audio set output channel %d\n",flag);
                flag++;
                //CSAUD_Pause(g_audio_handle);
             #else//test CSVID_TimeCodeReportNotify
                CSVID_TimeCode  tempTC;
                tempTC.val = 0x552;
                printf("test 0x%x,0x%x,0x%x\n",tempTC.val,CSVID_Stop,g_video_handle);
                CSVID_TimeCodeReportNotify(g_video_handle, testvideocallback, tempTC, 1);
             #endif
            }
             else if(18 == wParam)//down
            {
#if 0//test equalizer
                CSAUD_EqualizerConfig   temp;
                static int eq_flag=0;//flag%5;
                CSAUD_DisableEqualizer(g_audio_handle);
                temp.equalizer_type=eq_flag;
                temp.equalizer_band_weight[0]=-12;
                temp.equalizer_band_weight[1]=12;
                temp.equalizer_band_weight[2]=-6;
                temp.equalizer_band_weight[3]=6;
                temp.equalizer_band_weight[4]=5;
                temp.equalizer_band_weight[5]=12;
                temp.equalizer_band_weight[6]=3;
                temp.equalizer_band_weight[7]=-9;
                temp.equalizer_band_weight[8]=4;
                temp.equalizer_band_weight[9]=7;
                //CSAUD_EnableEqualizer(g_audio_handle);
                CSAUD_SetEqualizerConfig(g_audio_handle,&temp);
                printf("audio set equalizer level %d\n",temp.equalizer_type);
                //if(eq_flag == 4) eq_flag =1;
                //else eq_flag++;
#else
                CSAUD_DisableMixer(g_audio_handle);
                mixerflag = 0;
                printf("CSAUD_DisableMixer\n");
#endif
            }

            else if(33 == wParam)//sat
            {
            #if 0//test start delay
                unsigned int delay_number=10;
                CSAUD_SetStartDelay(g_audio_handle,delay_number );
                printf("audio set startdelay %d\n",delay_number);
            #else//test disable equalizer
                //CSAUD_EnableEqualizer(g_audio_handle);
                CSAUD_DisableEqualizer(g_audio_handle);
                printf("audio set equalizer level\n");
            #endif
            }
            
            else if(59 == wParam)//menu
            {
            #if 1
                unsigned int streamtype=0;
                CSAUD_GetCodecType(g_audio_handle, &streamtype);
                printf("audio get streamtype is %d\n",streamtype);
            #else
                unsigned int data[64];
                data[0] = 0x1a5df032;
                data[1] = 0x1a5df023;
                data[2] = 0x1a5d0f32;
                data[3] = 0x1a5d32f0;
                data[4] = 0x1ad5f032;
                data[5] = 0xa15df032;
                data[6] = 0x5da1f032;
                data[7] = 0x12345678;
                if(CSVID_EnableStillPicture(g_video_handle)){
                    printf("%s\n",CSVID_GetErrString(g_video_handle));
                }
                if(CSVID_WriteStillPicture(g_video_handle, &data[0], 64)){
                    printf("%s\n",CSVID_GetErrString(g_video_handle));
                }
                if(CSVID_PlayStillPicture(g_video_handle)){
                    printf("%s\n",CSVID_GetErrString(g_video_handle));
                }
            #endif
            }
            else if(1 == wParam)//exit
            {
                unsigned int bitrate=0;
                CSAUD_SetSampleRate(g_audio_handle, AUD_SAMPLE_RATE_96KHZ);
                printf("audio get current bitrate is %d\n",bitrate);
            }
            else if(46 == wParam)//up
            {
            	  BITMAP bmp_bg;

		  hdc = BeginPaint (hWnd);
                if (LoadBitmap(HDC_SCREEN, &bmp_bg, "/mnt/pic/1.jpg"))
                {
                    printf("open file failed!\n");
                    return -1;
                }
                else
                    FillBoxWithBitmap (hdc, 0, 0, 720, 576, &bmp_bg);

                EndPaint (hWnd, hdc);

            }
            else if(23 == wParam)//pause
            {
#if 1//test mixer
                int error = 0;

                error = pthread_create(&read_thread,NULL,(void *)mixertask,NULL);
                printf("create pthread error %d\n",error);
                if(error != 0)
                {
                    printf("Get_Multi_Section task failed \n");
                }
#else
                FILE * testfile = NULL;
                char namebuf[] = "abcdefghigklmn";
                int len = 0;
                
                testfile = fopen("/mnt/pat.ts", "w+b");
	        if (testfile == NULL) 
		        return -1;
#if 1
                PATDataToSection(0x100, 0x100, 0x101);
                //SectionToTs(sec_buf, PAT_SECTION_LENGTH, 0x00);
                fwrite(ts_pat_buf, 1, 188, testfile);
                PATDataToSection(0x100, 0x100, 0x101);
                //SectionToTs(sec_buf, PAT_SECTION_LENGTH, 0x00);
                fwrite(ts_pat_buf, 1, 188, testfile);
                PATDataToSection(0x100, 0x100, 0x101);
                //SectionToTs(sec_buf, PAT_SECTION_LENGTH, 0x00);
                fwrite(ts_pat_buf, 1, 188, testfile);
    //            fclose(testfile);
//#else
                SDTDataToSection(0x100, 0x100, 0x101, namebuf, strlen(namebuf));
                //SectionToTs(sec_buf, len, SDT_PID);
                fwrite(ts_sdt_buf, 1, 188, testfile);
                SDTDataToSection(0x100, 0x100, 0x101, namebuf, strlen(namebuf));
                //SectionToTs(sec_buf, len, SDT_PID);
                fwrite(ts_sdt_buf, 1, 188, testfile);
                SDTDataToSection(0x100, 0x100, 0x101, namebuf, strlen(namebuf));
                //SectionToTs(sec_buf, len, SDT_PID);
                fwrite(ts_sdt_buf, 1, 188, testfile);
                fclose(testfile);
#endif
#endif
            }
            else if(50 == wParam){//guide
    #if 1
                CSAUD_ATTRIBUTE attr;

                attr.balance = - 126;
                attr.bass = 10;
                attr.mid = 10;
                attr.treble = 10;
                printf("balance %d,bass %d,mid %d,treble %d\n",attr.balance,attr.bass,attr.mid,attr.treble);
                CSAUD_SetAttributes(g_audio_handle, &attr);
                CSAUD_GetAttributes(g_audio_handle, &attr);
                printf("balance %d,bass %d,mid %d,treble %d\n",attr.balance,attr.bass,attr.mid,attr.treble);
                CSAUD_EnableEqualizer(g_audio_handle);
#else
            //CSVID_Rect rect;
            //CSVID_GetPScanCrop(g_video_handle, &rect);
           // printf("top %d,bottom %d,left %d,right %d\n",rect.top,rect.bottom,rect.left,rect.right);
            //CSDEMUX_VID_Disable(g_demux_video_handle);
            //CSDEMUX_AUD_Disable(g_demux_audio_handle);
            //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_video_handle);
            //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
            //CSVID_Stop(g_video_handle);
            //CSAUD_Stop(g_audio_handle);
            //usleep(100000);
            //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
#if 1
            int error = 0;
                test_filter_flag = 1;
		receive_data_flag = 0;

		pthread_mutex_init(&pvr_mutex, NULL);
		 error = pthread_create(&montior_thread,NULL,(void *)monitor_filter,NULL);
                printf("ecm create pthread error %d\n",error);
                if(error != 0)
                {
                    printf("ecm task failed \n");
                }
				
                error = pthread_create(&read_thread,NULL,(void *)test_filter,NULL);
                printf("ecm create pthread error %d\n",error);
                if(error != 0)
                {
                    printf("ecm task failed \n");
                }
        #endif
        
#endif
            }
            else if(49 == wParam){//info
            unsigned int wp = 0, rp = 0,in_size = 0,reg_val = 0;
            #if 0//test __csvid_get_aspectratio()
                    int aspect_ratio = -1;
                    __csvid_get_aspectratio(g_video_handle, &aspect_ratio);
                    printf("aspect ratio 0x%x\n",aspect_ratio);
            #else
                    //CSVID_Force3to2PollDown(g_video_handle, 1);
                    //CSDEMUX_VID_Enable(g_demux_video_handle);
                    //CSDEMUX_AUD_Enable(g_demux_audio_handle);
                    //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_video_handle);
                    //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_audio_handle);
                    //CSVID_Play(g_video_handle);
                    //CSAUD_Play(g_audio_handle);
                    //CSVID_SyncNotify(g_video_handle, testvideosynccallback, 5000000, 1);
                    //CSAUD_SetOutputDevice(g_audio_handle, AUD_OUTPUT_I2S);
            printf("\nprint status\n");
            //printf_h264_status();
#if 0
            {
                                #define VIDEO_REG_BASE   0x41600000  // host access
                                #define MAILBOX_9     (VIDEO_REG_BASE + (0x29<<2))
                                #define PLUTO_REG_BASE 0x41620000
                                #define PARSER_MAIL7          ( PLUTO_REG_BASE + (0xa7 << 2))
				    #define HOST_IF_VID_HOST_INT ((VIDEO_REG_BASE + (0x5<<2)))
				    #define HOST_IF_VID_HOST_MASK ((VIDEO_REG_BASE + (0x6<<2)))	
					
                                unsigned int data = 1;

					ReadReg32(0x10171408,&data);
					printf(" addr 0x10171408 IDS_PCTL_HI 0x%x\n",data);

					ReadReg32(0x1017110c,&data);
					printf(" addr 0x1017110c AUD_CLK_GEN_HPD 0x%x\n",data);

					ReadReg32(0x10171118,&data);
					printf(" addr 0x10171118 AUD_CLK_GEN_FREQ 0x%x\n",data);

					ReadReg32(0x1017111c,&data);
					printf(" addr 0x1017111c AUD_CLK_GEN_JITTER_LO 0x%x\n",data);

					ReadReg32(0x10171404,&data);
					printf(" addr 0x10171404 AUD_CLK_GEN_JITTER_HI 0x%x\n",data);



					
					
                                ReadReg32(PARSER_MAIL7, &data);
                                printf("[VIDEO] PARSER_MAIL7 (df_repeat_cnt: %d)\n",data);
                                ReadReg32(MAILBOX_9, &data);
                                printf("[VIDEO] MAILBOX_9 (cpb_full_cnt: %d)(must_output: %d)\n",data>>16, data&0xFFFF);

				    ReadReg32(PARSER_MAIL7, &data);
                                printf("HOST_IF_VID_HOST_INT: 0x%x\n",data);
                                ReadReg32(MAILBOX_9, &data);
                                printf("HOST_IF_VID_HOST_MASK: 0x%x\n",data);
            }
#endif
            printf_audio_status();
            printf_mpeg2_status();

	//audio_mixer_status();
            //printf_xport_state();
            #endif
            }
            else if(24 == wParam){//tv/radio
            #if 1
			CSVID_SequenceHeader hdr;
			static int tvmode_1 = 1;
			CSOSD_MODE osd_mode = 0;

			CSVID_GetSequenceHeader(g_video_handle, &hdr);
			printf("hdr.w = %d,hdr.h = %d,hdr.frame_rate = %d\n",hdr.w,hdr.h,hdr.frame_rate);
			src_rect.right= 0;//hdr.w;
			src_rect.bottom=0;//hdr.h;

			switch(tvmode_1){
			case TVOUT_MODE_SECAM:
			case TVOUT_MODE_PAL_CN:
			case TVOUT_MODE_PAL_M:
			case TVOUT_MODE_PAL_N:
			 case TVOUT_MODE_576I:			 	
				g_tvout_mode = TVOUT_MODE_576I;
				osd_mode = OSD_MODE_576I;
				dst_rect.bottom=576;
				dst_rect.right=720;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_576I\n");
				break;
			case TVOUT_MODE_576P:			 	
				g_tvout_mode = TVOUT_MODE_576P;
				osd_mode = OSD_MODE_576P;
				dst_rect.bottom=576;
				dst_rect.right=720;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_576P\n");
				break;
			case TVOUT_MODE_480I:			 	
				g_tvout_mode = TVOUT_MODE_480I;
				osd_mode = OSD_MODE_480I;
				dst_rect.bottom=480;
				dst_rect.right=720;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_480I\n");
				break;
			case TVOUT_MODE_480P:			 	
				g_tvout_mode = TVOUT_MODE_480P;
				osd_mode = OSD_MODE_480P;
				dst_rect.bottom=480;
				dst_rect.right=720;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_480P\n");
				break;
			case TVOUT_MODE_720P50:			 	
				g_tvout_mode = TVOUT_MODE_720P50;
				osd_mode = OSD_MODE_720P50;
				dst_rect.bottom=720;
				dst_rect.right=1280;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_720P50\n");
				break;
			case TVOUT_MODE_720P60:			 	
				g_tvout_mode = TVOUT_MODE_720P60;
				osd_mode = OSD_MODE_720P60;
				dst_rect.bottom=720;
				dst_rect.right=1280;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_720P60\n");
				break;
			case TVOUT_MODE_1080P24:
			case TVOUT_MODE_1080P25:
			case TVOUT_MODE_1080P30:
				goto ERROR;
			case TVOUT_MODE_1080I25:			 	
				g_tvout_mode = TVOUT_MODE_1080I25;
				osd_mode = OSD_MODE_1080I25;
				dst_rect.bottom=1080;
				dst_rect.right=1920;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_1080I25\n");
				break;
			case TVOUT_MODE_1080I30:			 	
				g_tvout_mode = TVOUT_MODE_1080I30;
				osd_mode = OSD_MODE_1080I30;
				dst_rect.bottom=1080;
				dst_rect.right=1920;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_MODE_1080I30\n");
				break;
			case TVOUT_RGB_640X480_60FPS:
				g_tvout_mode = TVOUT_RGB_640X480_60FPS;
				osd_mode = OSD_MODE_640X480_60FPS;
				dst_rect.bottom=480;
				dst_rect.right=640;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_640X480_60FPS\n");
				break;
			case TVOUT_RGB_800X600_60FPS:
				g_tvout_mode = TVOUT_RGB_800X600_60FPS;
				osd_mode = OSD_MODE_800X600_60FPS;
				dst_rect.bottom=600;
				dst_rect.right=800;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_800X600_60FPS\n");
				break;
			case TVOUT_RGB_800X600_72FPS:
				g_tvout_mode = TVOUT_RGB_800X600_72FPS;
				osd_mode = OSD_MODE_800X600_72FPS;
				dst_rect.bottom=600;
				dst_rect.right=800;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_800X600_72FPS\n");
				break;
			case TVOUT_RGB_1024X768_60FPS:
				g_tvout_mode = TVOUT_RGB_1024X768_60FPS;
				osd_mode = OSD_MODE_1024X768_60FPS;
				dst_rect.bottom=768;
				dst_rect.right=1024;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_1024X768_60FPS\n");
				break;
			case TVOUT_RGB_1280X1024_50FPS:
				g_tvout_mode = TVOUT_RGB_1280X1024_50FPS;
				osd_mode = OSD_MODE_1280X1024_50FPS;
				dst_rect.bottom=1024;
				dst_rect.right=1280;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_1280X1024_50FPS\n");
				break;
			case TVOUT_RGB_1600X1000_60FPS:
				g_tvout_mode = TVOUT_RGB_1600X1000_60FPS;
				osd_mode = OSD_MODE_1600X1000_60FPS;
				dst_rect.bottom=1000;
				dst_rect.right=1600;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_1600X1000_60FPS\n");
				break;
			case TVOUT_RGB_1280X1024_60FPS:
				g_tvout_mode = TVOUT_RGB_1280X1024_60FPS;
				osd_mode = OSD_MODE_1280X1024_60FPS;
				dst_rect.bottom=1024;
				dst_rect.right=1280;
				dst_rect.left = dst_rect.top = 0;
				printf("TVOUT_RGB_1280X1024_60FPS\n");
				break;
			}

			//src_rect.right = dst_rect.right;
			//src_rect.bottom = dst_rect.bottom;
//			dst_rect.left = 160;
//			dst_rect.top == 160;
//			dst_rect.right = 480;
//			dst_rect.bottom = 320;
			//CSVID_DisableOutput(g_video_handle);
			CSTVOUT_SetMode(g_tvout_handle,tvmode_1);
			CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
			CSVID_SetOutputAlpha(g_video_handle, 0xff);
			CSOSD_Flip(g_osd_handle);
ERROR:
			tvmode_1 ++;
			if(tvmode_1 == (TVOUT_RGB_1280X720_60FPS+1)){
				tvmode_1 = TVOUT_MODE_576I;
			}
#if 0
{

			CSOSD_Config osd_config = {osd_mode,OSD_COLOR_DEPTH_16,OSD_COLOR_FORMAT_RGB565};
			CSOSD_SetConfig(g_osd_handle, &osd_config);
			CSOSD_Enable(g_osd_handle);
			CSOSD_Flip(g_osd_handle);
			printf("111111111111111111111\n");
}
#endif
			//CSVID_EnableOutput(g_video_handle);
            #else
                    CSVID_Force3to2PollDown(g_video_handle, 0);
                    test_demux_flag = 0;
                    //CSDEMUX_Filter_Disable(hfilter0);
                    //printf("1\n");
                    //CSDEMUX_Filter_Disable(hfilter1);
                    //CSDEMUX_Filter_Close(hfilter0);
                    printf("2\n");
                    //CSDEMUX_Filter_Close(hfilter1);
            #endif
            } 
            else if(25 == wParam){//subtitle
            #if 1
		g_tvout_mode = TVOUT_RGB_640X480_60FPS;
		src_rect.right= 0;
		src_rect.bottom=0;
		dst_rect.bottom=480;
		dst_rect.right=640;
		dst_rect.left = dst_rect.top = src_rect.left = src_rect.top = 0;
		CSTVOUT_SetMode(g_tvout_handle,g_tvout_mode);
		CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
		CSVID_SetOutputAlpha(g_video_handle, 0xff);
	#else
       unsigned int phyaddr = 0;
	unsigned int memsize = 0;
	unsigned int addroffset = 0;
	unsigned char *buf = NULL;
	unsigned int addr = 0;
	unsigned int size = 0;
	FILE * mem_fd = NULL;
	unsigned char * virtul_reg_vid_df0_status1 = NULL;
	#define VID_DF0STA1_ADDRESS      0x41600064

	mem_fd = open("/dev/mem", O_RDWR);

        phyaddr = (VID_DF0STA1_ADDRESS >> PAGE_SHIFT) << PAGE_SHIFT;
        addroffset = VID_DF0STA1_ADDRESS - phyaddr;
        memsize = ((addroffset + 1 + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;
    
        buf = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phyaddr);
        printf("&&&&&&&&0x%x&&&&&&&\n", buf);
		printf("memsize = 0x%x, addoffset = 0x%x, phyaddr = 0x%x\n",memsize,addroffset,phyaddr);
	if (buf == MAP_FAILED) {
            return NULL;
        }
	
       virtul_reg_vid_df0_status1 = buf + addroffset;
	printf("virtul_reg_vid_df0_status1 = 0x%x \n",virtul_reg_vid_df0_status1);
	printf("virtul_reg_vid_df0_status1 = 0x%x \n",*((volatile unsigned char *)virtul_reg_vid_df0_status1));

	//size =((*(virtul_reg_vid_df0_status1+2))<<8) |(*(virtul_reg_vid_df0_status1+3));
	//printf("size = 0x%x\n",size);
            #endif

            }
            else if(37 == wParam){
			static unsigned char pause_resume = 0;
			if(pause_resume%2 == 0){
				CSVID_Pause(g_video_handle);
				printf("CSVID_Pause\n");
			}
			else if(pause_resume%2 == 1){
				CSVID_Resume(g_video_handle);
				printf("CSVID_Resume\n");
			}
			else{
				printf("pause_resmuse = %d\n", pause_resume);
			}

			pause_resume ++;
            }
            else if(38 == wParam){
                                unsigned int data = 1;
								
                                ReadReg32(0x10260030,&data);
                                printf("GPIO_PIN_MUX0: 0x%x)\n",data);
                                ReadReg32(0x10260044, &data);
                                printf("GPIO_PIN_MUX1: 0x%x)\n",data);
            }
	     else if(72 == wParam){ //chl +
	     		CSAUD_ATTRIBUTE Attr;
			static int tempaaa = 0;
			memset(&Attr,0,sizeof(CSAUD_ATTRIBUTE));
			if(tempaaa%3 == 0){
				Attr.balance = 127;
				CSAUD_SetAttributes(g_audio_handle, &Attr);
				CSAUD_EnableEqualizer(g_audio_handle);
				tempaaa= 1;
				printf("right balance = %d\n",Attr.balance);
			}
			else if(tempaaa%3 == 1){
				Attr.balance = 0;
				CSAUD_SetAttributes(g_audio_handle, &Attr);
				CSAUD_EnableEqualizer(g_audio_handle);
				tempaaa = 2;
				printf("stereo balance = %d\n",Attr.balance);
			}
			else if(tempaaa%3 == 2){
				Attr.balance = -128;
				CSAUD_SetAttributes(g_audio_handle, &Attr);
				CSAUD_EnableEqualizer(g_audio_handle);
				tempaaa = 0;
				printf("left balance = %d\n",Attr.balance);
			}
            }
            else printf("get input key = %d\n", wParam);

            return 0;
        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}


int MiniGUIMain (int argc, const char* argv[])
{
    int return_val = 0;
    pid_t   pid = 1;
    struct timeval start,end;
    FILE*   ts_file=NULL;
    int timeuse=0;
    MSG             Msg;
    HWND            hMainWnd;
    MAINWINCREATE   CreateInfo;
    CSDEMUX_HANDLE  hpidfilter0, hpidfilter1;

    CSHDMI_Init();
    InitReadWriteReg();

{
	unsigned int data = 1;

        ReadReg32(0x10260030,&data);
        printf("GPIO_PIN_MUX0: 0x%x)\n",data);
        ReadReg32(0x10260044, &data);
        printf("GPIO_PIN_MUX1: 0x%x)\n",data);
}
#if 0
    g_tunerflag = 1;
    while(tuner_lock(602, 6875, 1)<0)
    {
        printf("TEST DEMUX-----------------> lock failed!\n");
    }
#else
    g_tunerflag = 0;

	sleep(3);

    ts_file = fopen("/mnt/stream/mepg2_1080i_25fps_wrestler.ts","rb");//mepg2_1080i_25fps_wrestler.ts//TVBHD-h264-ac3.ts//h264_mpa_720i.ts//mpeg2_mpa_480p_qianlizhiwai.ts//sport_cc2.ts
    if(ts_file==NULL)
    {
       printf("======>file open failed\n");
       return -1;
    }
#endif

    if(CSDEMUX_Init()) printf("CSDEMUX_Init Failed!\n");

    g_demux_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
    if(g_demux_chl_handle == CSDEMUX_UNVALID_HANDLE)
    {
         printf("Orion Xport Channel0 Open Failed ...\n");
         CSDEMUX_Terminate();
         return return_val;
    }
    if(g_tunerflag)
        CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_TUNER);
    else
        CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_DMA);

    if(CSDEMUX_CHL_Enable(g_demux_chl_handle)) printf("CSDEMUX_CHL_Enable Failed! \n");

#if 0
    //Search_Channel();
    if(0 == fork()){
        //init();
        //play(0);
        while(1){sleep(5);}
        
#if 0//test CSDEMUX_CHL,CSDEMUX_PIDFT,CSDEMUX_VID,CSDEMUX_AUD:Enable/Disable
        gettimeofday(&start, NULL);
        int flag1=0,flag2=0;
        while(1)
        {
            gettimeofday(&end, NULL);
            timeuse=end.tv_sec-start.tv_sec;
            //printf("second : %d\n",timeuse);
            if(((timeuse%3) == 0)&&(timeuse != 0))
            {
                if(flag2 != timeuse)
                {
                    flag2=timeuse;
                    if(flag1 == 0)
                    {
                        printf(">>>>>>>>>>>>>>>>>>>>>>>>>CSDEMUX_PIDFT_Disable : %d\n",timeuse);
                        //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_video_handle);
                        //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
                        //CSDEMUX_CHL_Disable(g_demux_chl_handle);
                        CSDEMUX_VID_Disable(g_demux_video_handle);
                        CSDEMUX_AUD_Disable(g_demux_audio_handle);
                        flag1=1;
                    }
                    else if(flag1 == 1)
                    {
                        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CSDEMUX_PIDFT_Enable : %d\n",timeuse);
                        //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_video_handle);
                        //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_audio_handle);
                        //CSDEMUX_CHL_Enable(g_demux_chl_handle);
                        CSDEMUX_VID_Enable(g_demux_video_handle);
                        CSDEMUX_AUD_Enable(g_demux_audio_handle);
                        flag1=0;
                    }
                }
            }
        }
#endif
    }
    else{
        //Get_TS(DEMUX_PIDFT_ID0,DEMUX_FILTER_ID0, 0x3fc);
        //Get_Section(DEMUX_PIDFT_ID1, DEMUX_FILTER_ID1, 0x11, 0x42);
        Get_Multi_Section();
        //Strong_Multi_Section();
        //Search_Channel();
        //Record_test(0);
    }
#else
    if(g_tunerflag)//{;}
        Search_Channel();
    else{
       __Paser_file(ts_file);
    }

    init();

//	while(1){sleep(2);}

//testosd();
//return;

    if(!g_tunerflag) pid = fork();

    if(pid<0){
         printf("create processor error\n");
         return -1; 
    }
    else if(pid==0){
        __fileplay(ts_file);
    }
    else{
       #if 0
    hpidfilter0 = CSDEMUX_PIDFT_Open(3);
    hpidfilter1 = CSDEMUX_PIDFT_Open(8);

    CSDEMUX_PIDFT_SetChannel(hpidfilter0,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetChannel(hpidfilter1,DEMUX_CHL_ID0);

    CSDEMUX_PIDFT_SetPID(hpidfilter0,PAT_PID);  
    CSDEMUX_PIDFT_SetPID(hpidfilter1,0x401);  

    CSDEMUX_PIDFT_Enable(hpidfilter0);
    CSDEMUX_PIDFT_Enable(hpidfilter1);
    #endif

	//Get_Section(DEMUX_PIDFT_ID1, DEMUX_FILTER_ID0, 0x20, 0x80);
    play(0);
    //PlayIFrame();
    while(0){
      //  play(0);
     // sleep(4);
      //stop();

        //sleep(1);
        
        play(5);
        sleep(3);
        stop();
//sleep(1);
        }

        CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
        CreateInfo.dwExStyle = WS_EX_NONE;
        CreateInfo.spCaption = "Program List";
        CreateInfo.hMenu = 0;
        CreateInfo.hCursor = 0;
        CreateInfo.hIcon = 0;
        CreateInfo.MainWindowProc = TestWinProc;
        CreateInfo.lx = 80;
        CreateInfo.ty = 96;
        CreateInfo.rx = 508;//620
        CreateInfo.by = 284;//380;
        CreateInfo.iBkColor = COLOR_lightwhite;
        CreateInfo.dwAddData = 0;
        CreateInfo.hHosting = HWND_DESKTOP;
        
        hMainWnd = CreateMainWindow (&CreateInfo);
        if (hMainWnd == HWND_INVALID)
            return -1;
        ShowWindow(hMainWnd, SW_SHOWNORMAL);
        while (GetMessage(&Msg, hMainWnd)) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        MainWindowThreadCleanup (hMainWnd);
    }
#endif
    CSDEMUX_CHL_Close(g_demux_chl_handle);
    CSDEMUX_Terminate();  
    //DestroyReadWriteReg();
    return return_val;
}
#else

pthread_t fileplay_thread;
int main(void)
{
	FILE*   ts_file=NULL;
	pid_t   pid = 1;
	
#if 0
	g_tunerflag = 1;
	while(tuner_lock(698, 6875, 1)<0)
	{
	printf("TEST DEMUX-----------------> lock failed!\n");
	}
#else
	g_tunerflag = 0;
	ts_file = fopen("/opt/352a_1.ts","rb");//TVBHD-h264-ac3.ts//h264_mpa_720i.ts//mpeg2_mpa_480p_qianlizhiwai.ts//sport_cc2.ts
	if(ts_file==NULL)
	{
		printf("======>file open failed\n");
		return -1;
	}

#endif

	if(CSDEMUX_Init()) printf("CSDEMUX_Init Failed!\n");

	g_demux_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
	if(g_demux_chl_handle == CSDEMUX_UNVALID_HANDLE)
	{
		printf("Orion Xport Channel0 Open Failed ...\n");
		CSDEMUX_Terminate();
		return 1;
	}

	if(g_tunerflag)
		CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_TUNER);
	else
		CSDEMUX_CHL_SetInputMode(g_demux_chl_handle,DEMUX_INPUT_MOD_DMA);

	if(CSDEMUX_CHL_Enable(g_demux_chl_handle)) printf("CSDEMUX_CHL_Enable Failed! \n");

	if(g_tunerflag)
		Search_Channel();
	else{
		__Paser_file(ts_file);
	}

	init();
/*
	if(!g_tunerflag) pid = fork();

	if(pid<0){
		printf("create processor error\n");
		return -1; 
	}
	else if(pid==0){
		__fileplay(ts_file);
	}
	else{
*/
		int error = 0;
		test_filter_flag = 1;
		receive_data_flag = 0;

		if(!g_tunerflag){
			error = pthread_create(&fileplay_thread,NULL,(void *)__fileplay,NULL);
			printf("fileplay create pthread error %d\n",error);
			if(error != 0){
				printf("fileplay task failed \n");
			}
		}

		pthread_mutex_init(&pvr_mutex, NULL);

		error = pthread_create(&read_thread,NULL,(void *)test_filter,NULL);
		printf("test_filter create pthread error %d\n",error);
		if(error != 0){
			printf("test_filter task failed \n");
		}
		
		error = pthread_create(&montior_thread,NULL,(void *)monitor_filter,NULL);
		printf("monitor_filter create pthread error %d\n",error);
		if(error != 0){
			printf("test_filter task failed \n");
		}

		while(1){
			sleep(10);
		}

}

#endif


