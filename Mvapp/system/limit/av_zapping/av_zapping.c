#include "linuxos.h"

#include "database.h"
#include "demux.h"
#include "av_zapping.h"
#include "csmpr_player.h"
#include "cat6611api.h"
#include "scart.h"   /* By KB Kim : 2010_08_31 for Scart Control */
#include "userdefine.h"
#include "mvosapi.h"  /* By KB Kim 2011.06.02 */

/* By KB Kim : 2010_08_31 for RGB Control */
tCS_DBU_VideoOutput    			CurrentTvOutput = eCS_DBU_OUTPUT_MAX;
tCS_AV_VideoDefinition 			CurrentDefinition = eCS_AV_VIDEO_FORMAT_UNKNOWN;

#define MVAPP_OSD_MAX_WIDTH   	1280
#define MVAPP_OSD_MAX_HEIGHT  	720

CSVID_HANDLE					vid_handle = NULL;
CSAUD_HANDLE					aud_handle = NULL;
static CSTVOUT_HANDLE			tve_handle = NULL;
static CSOSD_HANDLE				osd_handle = NULL;
//static  CSDF_HANDLE			df_handle = NULL;

//CSDEMUX_HANDLE xport_chl_handle;
CSDEMUX_HANDLE 					xport_pid_filter0_handle = CSDEMUX_UNVALID_HANDLE;;
CSDEMUX_HANDLE 					xport_pid_filter1_handle = CSDEMUX_UNVALID_HANDLE;;

CSDEMUX_HANDLE 					xport_vidout_handle = CSDEMUX_UNVALID_HANDLE;
CSDEMUX_HANDLE 					xport_audout_handle = CSDEMUX_UNVALID_HANDLE;

static CSOS_Semaphore_t			*sem_AVAccess = NULL;
//static CSOS_MessageQueue_t 	*CS_AV_MsgQid = NULL;

#define	kCS_AV_MAX_MSG			20
#define	AV_TRACK_TASK_PRIORITY	14
#define	AV_TRACK_STACK_SIZE		1024*2

CSOS_TaskFlag_t					AV_TRACK_TASK_FLAG;
U8								AV_TRACK_TASK_STACK[AV_TRACK_STACK_SIZE];
CSOS_Task_Handle_t				AV_TRACK_TASK_HANDLE;
CSOS_TaskDesc_t					*AV_TRACK_TASK_DESC;

BOOL							video_set_show = FALSE;
static CSVID_Rect				*video_window_rect = NULL;
static CSVID_Rect				rect_buf;
static CSVID_ASPECTRATIO		current_video_aspect = CSVID_UNKNOWN;


#define VideoSizeOffset			20
#define VideoSizeRange(x, y)	(((x >= y - VideoSizeOffset) && (x <= y + VideoSizeOffset)) ? 1:0)

#define VideoRateOffset			3
#define VideoRateRange(x, y)	(((x >= y - VideoRateOffset) && (x <= y + VideoRateOffset)) ? 1:0)


#define MAX_VOLUME_VALUE		80
#define MIN_VOLUME_VALUE		1

#define ORION_XPORT_REG_BASE	0x41400000
#define DDR_RW_DATA_ADDR0		( ORION_XPORT_REG_BASE + (0x01b0<<2))
#define DDR_RW_DATA_ADDR4       ( ORION_XPORT_REG_BASE + (0x01b4<<2))
#define DDR_RW_DATA_ADDR8		( ORION_XPORT_REG_BASE + (0x01b8<<2))
#define DDR_RW_DATA_ADDR12		( ORION_XPORT_REG_BASE + (0x01bc<<2))
#define DDR_RW_DATA_ADDR16		( ORION_XPORT_REG_BASE + (0x01c0<<2))
#define DDR_RW_DATA_ADDR20		( ORION_XPORT_REG_BASE + (0x01c4<<2))
#define DDR_RW_DATA_ADDR24		( ORION_XPORT_REG_BASE + (0x01c8<<2))
#define DDR_RW_DATA_ADDR28		( ORION_XPORT_REG_BASE + (0x01cc<<2))
#define MIPS_RW_ADDR_ADDR		( ORION_XPORT_REG_BASE + (0x01a0<<2))
#define MIPS_RW_REQ_ADDR		( ORION_XPORT_REG_BASE + (0x01a1<<2))

#define OUT_CHL0_DIR_WP_ADDR	(0x41400000+(0x100<<2))
#define OUT_CHL0_DIR_RP_ADDR	(0x41400000+(0x101<<2))
#define OUT_CHL2_DIR_WP_ADDR	(0x41400000+(0x104<<2))
#define OUT_CHL2_DIR_RP_ADDR	(0x41400000+(0x105<<2))

#define div(x) (x /16)
#define mod(x) (x % 16)

typedef struct DesKeyData_s
{
	unsigned int key_len;
	unsigned int key[6];
}DesKeyData_t;

// extern void MV_VideoData_UnderFlow_Callback(CSVID_HANDLE *handle );
extern CSAPI_RESULT CSTVOUT_SetOutput(CSTVOUT_HANDLE handle, CSTVOUT_OUTPUT_MODE output_mode);

/* By KB Kim 2011.06.02 */
#define MAX_NO_VIDEO_COUNT      10
#define MAX_VIDEO_STATUS_COUNT  10
U32 VideoStatusAccessSem;
U32 VideoSatusUnderFlow = 0;
U8  NoVideoOn = 0;

#if 0
#include <sys/mman.h>
#define CACHED(x)  ( ( (x) >= (pa) ) && ( (x) < (pa + page_size) ) )
static unsigned int page_size, page_size_shift, pa;
static int fd = 0;
static unsigned char * va = NULL;
static pthread_mutex_t mutex_register = PTHREAD_MUTEX_INITIALIZER;

#define CAB_SIZE    0x180008

void video_print_CAB(void)

{

       U32 CAB_RP,CAB_WP, data;

       ReadReg32(0x41211014, &(U32)data); CAB_RP = data;

      printf("[AUDIO]audio CAB RP 0x41211014 0x%08x\n", data);

      ReadReg32(0x41211058, &(U32)data); CAB_WP = data;

      printf("[AUDIO]audio CAB WP 0x41211058 0x%08x\n", data);



      {

             U32 cab_depth = CAB_SIZE;

               U32 CAB_EMPTY_SIZE;

               U32 rtog,wtog;

               U32 rp,wp;

               U32 len = 0;

               U32 i;



               cab_depth = CAB_SIZE;



               rtog = (CAB_RP>>24) & 0x1;

               rp = CAB_RP & 0xffffff;

               wtog = (CAB_WP>>24) & 0x1;

               wp = CAB_WP & 0xffffff;



               if( (wp > rp) && (rtog == wtog) )

               {

                      CAB_EMPTY_SIZE = cab_depth - (wp - rp);

               }

               else if( (wp < rp) && (rtog != wtog) )

               {

                      CAB_EMPTY_SIZE = rp - wp;

               }

               else if(rtog == wtog)

               {

                      CAB_EMPTY_SIZE = cab_depth;

               }

               else if(rtog != wtog)

               {

                      CAB_EMPTY_SIZE = 0;

               }

               else

               {

                      printf("[AUDIO] CPB pointer error !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

               }



               len = 78 * CAB_EMPTY_SIZE / cab_depth;

               printf("[AUDIO] CAB status: ");

               for(i=0; i<len; i++) printf(" ");

               printf("*\n");

               printf("[AUDIO] CAB status:                    |                    |                    |                    |\n");

      }

}



#define VIDEO_REG_BASE     0x41600000        // host access

#define VID_CPBU0           (VIDEO_REG_BASE + (0x07<<2) )

#define VID_CPBL0           (VIDEO_REG_BASE + (0x08<<2) )

#define VID_PPBL0_ADDR           (VIDEO_REG_BASE + (0x3d<<2) )

#define VID_PPBU0_ADDR          (VIDEO_REG_BASE + (0x3e<<2) )

#define MAILBOX_0                  (VIDEO_REG_BASE + (0x20<<2) )

#define MAILBOX_1                  (VIDEO_REG_BASE + (0x21<<2) )

#define MAILBOX_2                  (VIDEO_REG_BASE + (0x22<<2) )

#define MAILBOX_3                  (VIDEO_REG_BASE + (0x23<<2) )

#define MAILBOX_4                  (VIDEO_REG_BASE + (0x24<<2) )

#define MAILBOX_5                  (VIDEO_REG_BASE + (0x25<<2) )

#define MAILBOX_6                  (VIDEO_REG_BASE + (0x26<<2) )

#define MAILBOX_7                  (VIDEO_REG_BASE + (0x27<<2) )

#define MAILBOX_8                  (VIDEO_REG_BASE + (0x28<<2) )

#define MAILBOX_9                  (VIDEO_REG_BASE + (0x29<<2) )

#define MAILBOX_10                (VIDEO_REG_BASE + (0x2a<<2) )

#define MAILBOX_11                (VIDEO_REG_BASE + (0x2b<<2) )

#define MAILBOX_12                (VIDEO_REG_BASE + (0x2c<<2) )

#define MAILBOX_13                (VIDEO_REG_BASE + (0x2d<<2) )

#define MAILBOX_14                (VIDEO_REG_BASE + (0x2e<<2) )

#define MAILBOX_15                (VIDEO_REG_BASE + (0x2f<<2) )

#define MIPS_STA0       (VIDEO_REG_BASE + (0x30<<2) )

#define MIPS_STA1       (VIDEO_REG_BASE + (0x31<<2) )

#define MIPS_STA2       (VIDEO_REG_BASE + (0x32<<2) )



#define PLUTO_REG_BASE              0x41620000

#define VID_PPBRP0_ADDR         (PLUTO_REG_BASE + (0x89<<2) )

#define VID_PPBWP0_ADDR        (PLUTO_REG_BASE + (0x8b<<2) )



#define PARSER_MAIL0        ( PLUTO_REG_BASE + (0xa0 << 2))

#define PARSER_MAIL1        ( PLUTO_REG_BASE + (0xa1 << 2))

#define PARSER_MAIL2        ( PLUTO_REG_BASE + (0xa2 << 2))

#define PARSER_MAIL3        ( PLUTO_REG_BASE + (0xa3 << 2))

#define PARSER_MAIL4        ( PLUTO_REG_BASE + (0xa4 << 2))

#define PARSER_MAIL5        ( PLUTO_REG_BASE + (0xa5 << 2))

#define PARSER_MAIL6        ( PLUTO_REG_BASE + (0xa6 << 2))

#define PARSER_MAIL7        ( PLUTO_REG_BASE + (0xa7 << 2))







void printf_h264_status(void)

{

    U32 CPB_DIR_RP,CPB_DIR_WP;

       U32 data;

       U32 audio_diff, video_diff;

       U32 OneFieldOnlyFlag,StorePicMode,BotFieldFirst,DisplayFrmFldMode,FrameFetch;

       float FrameRate;



       {

       U32 CodeType,Year,Month,Day,Time;

       U32 data1;

       ReadReg32(MIPS_STA2, &(U32)data);     data1 = data;

       Time = data&0xF; data = data>>4;

       Day = data&0xFF; data = data>>8;

       Month = data&0xFF; data = data>>8;

       Year = data&0xFF; data = data>>8;

       CodeType = data&0xF;

       if(CodeType == 5)

              printf("[VIDEO] video firmware version:    H264-20%02d-%02d-%02d-%02d\n",Year,Month,Day,Time);

       else if(CodeType == 3)

              printf("[VIDEO] video firmware version: MPEG2-20%02d-%02d-%02d-%02d\n",Year,Month,Day,Time);

       else

              printf("[VIDEO] video firmware version: error MIPS_STA2 = %08x\n",data1);

       }



       ReadReg32(MIPS_STA1, &(U32)data);

       printf("[VIDEO] Host configure: 0x%08x\n",data);

       ReadReg32(MAILBOX_2, &(U32)data);

       printf("[VIDEO] Host command:               0x%08x\n",data);

       ReadReg32(MAILBOX_12, &(U32)data);

       printf("[VIDEO] FW_Running_Sta:    0x%08x\n",data);

       ReadReg32(MAILBOX_14, &(U32)data);

       printf("[VIDEO] FW_Interrupt:           0x%08x\n",data);

       ReadReg32(0x416000c0, &(U32)data);

       printf("[VIDEO] MIPS_STATUS:              0x%08x\n",data);

       ReadReg32(MAILBOX_3, &(U32)data);

       printf("[VIDEO] Has_Send_DfCmd: 0x%08x\n",data);



       video_print_CAB();



       ReadReg32(0x41600024, &(U32)CPB_DIR_RP);

       ReadReg32(0x41600028, &(U32)CPB_DIR_WP);

       printf("[VIDEO] CPB_DIR RP 0x%08x       CPB_DIR WP 0x%08x\n", CPB_DIR_RP, CPB_DIR_WP);

       printf("[VIDEO] CPB_DIR RP 0x%08x\n", CPB_DIR_RP);

       printf("[VIDEO] CPB_DIR WP 0x%08x\n", CPB_DIR_WP);



       // printf CPB status

       {

              U32 CPBU, CPBL;

              U32 CPB_SIZE;

              U32 CPB_EMPTY_SIZE;

              U32 rtog,wtog;

              U32 rp,wp;

              U32 len = 0;

              U32 i;



              ReadReg32(VID_CPBU0, &CPBU);

              ReadReg32(VID_CPBL0, &CPBL);

              CPB_SIZE = CPBU - CPBL;



              rtog = (CPB_DIR_RP>>28) & 0x1;

              rp = CPB_DIR_RP & 0xfffffff;

              wtog = (CPB_DIR_WP>>28) & 0x1;

              wp = CPB_DIR_WP & 0xfffffff;



              if( (wp > rp) && (rtog == wtog) )

              {

                     CPB_EMPTY_SIZE = CPB_SIZE - (wp - rp);

              }

              else if( (wp < rp) && (rtog != wtog) )

              {

                     CPB_EMPTY_SIZE = rp - wp;

              }

              else if(rtog == wtog)

              {

                     CPB_EMPTY_SIZE = CPB_SIZE;

              }

              else if(rtog != wtog)

              {

                     CPB_EMPTY_SIZE = 0;

              }

              else

              {

                     printf("[VIDEO] CPB pointer error !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

              }



              len = 78 * CPB_EMPTY_SIZE / CPB_SIZE;

              printf("[VIDEO] CPB status: ");

              for(i=0; i<len; i++) printf(" ");

              printf("@\n");

              printf("[VIDEO] CPB status:                    |                   |                   |                   |\n");

       }



       // printf PPB status

       {

              U32 PPBU, PPBL ,PPB_DIR_RP, PPB_DIR_WP;

              U32 PPB_SIZE;

              U32 PPB_EMPTY_SIZE;

              U32 rtog,wtog;

              U32 rp,wp;

              U32 len = 0;

              U32 i;



              ReadReg32(VID_PPBU0_ADDR, &PPBU);

              ReadReg32(VID_PPBL0_ADDR, &PPBL);

              PPB_SIZE = PPBU - PPBL;



              ReadReg32(VID_PPBRP0_ADDR, &PPB_DIR_RP);

              ReadReg32(VID_PPBWP0_ADDR, &PPB_DIR_WP);

              printf("[VIDEO] PPB_DIR RP 0x%08x\n", PPB_DIR_RP);

              printf("[VIDEO] PPB_DIR WP 0x%08x\n", PPB_DIR_WP);

              rtog = (PPB_DIR_RP>>28) & 0x1;

              rp = PPB_DIR_RP & 0xfffffff;

              wtog = (PPB_DIR_WP>>28) & 0x1;

              wp = PPB_DIR_WP & 0xfffffff;



              if( (wp > rp) && (rtog == wtog) )

              {

                     PPB_EMPTY_SIZE = PPB_SIZE - (wp - rp);

              }

              else if( (wp < rp) && (rtog != wtog) )

              {

                     PPB_EMPTY_SIZE = rp - wp;

              }

              else if(rtog == wtog)

              {

                     PPB_EMPTY_SIZE = PPB_SIZE;

              }

              else if(rtog != wtog)

              {

                     PPB_EMPTY_SIZE = 0;

              }

              else

              {

                     printf("[VIDEO] PPB pointer error !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

              }



              len = 78* PPB_EMPTY_SIZE / PPB_SIZE;

              printf("[VIDEO] PPB status: ");

              for(i=0; i<len; i++) printf(" ");

              printf("#\n");

              printf("[VIDEO] CPB status:                    |                   |                   |                   |\n");

       }



       ReadReg32(MAILBOX_9, &(U32)data);

       printf("[VIDEO] First at STC:              0x%08x\n",data);

       ReadReg32(PARSER_MAIL5, &(U32)data);

       printf("[VIDEO] First at FrameOrder:         %d\n",data);



       ReadReg32(MAILBOX_15, &(U32)data);

       printf("[VIDEO] Cur PTS:             0x%08x\n",data);



       ReadReg32(MAILBOX_5, &(U32)data);

       printf("[VIDEO] Cur PTS:             0x%08x\n",data);

       ReadReg32(MAILBOX_6, &(U32)data);

       printf("[VIDEO] Cur STC:             0x%08x\n",data);

       ReadReg32(MAILBOX_7, &(U32)audio_diff);

       printf("[VIDEO] Audio diff: 0x%08x\n",audio_diff);

       ReadReg32(MAILBOX_8, &(U32)video_diff);

       printf("[VIDEO] Video diff: 0x%08x\n",video_diff);

       printf("[VIDEO] Video - Audio:    0x%08x (%d ms)--(A/V asynchronism time)\n",(video_diff - audio_diff), ((signed int)(video_diff - audio_diff))/45);

       ReadReg32(MAILBOX_10, &(U32)data);

       printf("[VIDEO] FrameSyncOrder: %d\n",data&0xffff);

       printf("[VIDEO] pts ctrl bit:   %d\n",data>>16);

       ReadReg32(MAILBOX_4, &(U32)data);

       OneFieldOnlyFlag = data>>24;

       StorePicMode = (data>>16)&0xf;

       BotFieldFirst = (data>>8)&0xf;

       DisplayFrmFldMode = (data>>4)&0xf;

       FrameFetch = data & 0xf;

       printf("[VIDEO] OneFieldFlag = %d, StorePicMode= %d, BotFieldFirst= %d, DisplayFrmFldMode = %d, FrameFetch = %d\n",OneFieldOnlyFlag,StorePicMode,BotFieldFirst,DisplayFrmFldMode,FrameFetch);

       ReadReg32(MAILBOX_11, &(U32)data);

       printf("[VIDEO] First Diff:     0x%08x (%d ms)\n",(data), ((signed int)(data))/45);



       {

              U32 skip_num,wait_num;

              static U32 prev_skip_num=0;

              static U32 prev_wait_num=0;

              U32 delta_skip_num,delta_wait_num;

              U32 len = 0;

              U32 i;

              ReadReg32(PARSER_MAIL7, &(U32)data);

              skip_num = data>>16;

              wait_num = data&0xFFFF;

              if(skip_num < prev_skip_num)

                     delta_skip_num = 65536 + skip_num - prev_skip_num;

              else

                     delta_skip_num = skip_num - prev_skip_num;

              if(delta_skip_num>=40)   delta_skip_num = 40;

              if(wait_num < prev_wait_num)

                     delta_wait_num = 65536 + wait_num - prev_wait_num;

              else

                     delta_wait_num = wait_num - prev_wait_num;

              if(delta_wait_num>=40)   delta_wait_num = 40;



              printf("[VIDEO] SYNC Skip: %d",skip_num);

              len = 75 * delta_skip_num / 40;

              for(i=0; i<len; i++)printf(" ");

              printf("S\n");

              printf("[VIDEO] SYNC Wait:       %d",wait_num);

              len = 75 * delta_wait_num / 40;

              for(i=0; i<len; i++)printf(" ");

              printf("W\n");



              prev_skip_num = skip_num;

              prev_wait_num = wait_num;

       }



//     ReadReg32(MAILBOX_14, &(U32)data);

//     printf("[VIDEO] CPB PTS NUM: %d  PARSER PTS NUM: %d\n",data>>16, data&0xFFFF);

//     printf("[VIDEO] MAX DELAY TIME: %d       EXCEED 30ms CNT:       %d\n",(data>>16)/45, data&0xFFFF);

//     printf("[VIDEO] CPB OVERFLOW CNT: %d  \n", data);

//     printf("[VIDEO] MustOutputNum:      %d  \n", data);

//     printf("[VIDEO] First PTS:     0x%08x \n", data);

//     printf("[VIDEO] ReSendDFNUM:      %d  DFRepeatNum: %d\n",data>>16, data&0xFFFF);

//     printf("[VIDEO] PPB OVERFLOW:   %d  CPB OVERFLOW: %d\n",data>>16, data&0xFFFF);

//     printf("[VIDEO] Pipe Err: (%d)     Pipe Tout: (%d) Parser Err: (%d)    Parser Tout: (%d)\n",(data>>24)&0xFF,(data>>16)&0xFF,(data>>8)&0xFF,data&0xFF );

//     printf("[VIDEO] flag: (%d)     BufIdx: (%d)      IsIPic: (%d)  DfFlag: (%d)\n",(data>>24)&0xFF,(data>>16)&0xFF,(data>>8)&0xFF,data&0xFF );

//     printf("[VIDEO] DF Size:       %d  x     %d\n",data>>16, data&0xFFFF);



//     ReadReg32(MAILBOX_15, &(U32)data);

//     parser_timout = data & 0xFF;

//     parser_err = (data>>8) & 0xFF;

//     pipe_timeout = (data>>16) & 0xFF;

//     pipe_err = (data>>24) & 0xFF;

//     printf("[VIDEO] parser_timout:     %d  parser_err: %d     pipe_timeout:      %d  pipe_err:       %d\n",parser_timout,parser_err,pipe_timeout,pipe_err);

//     printf("[VIDEO] ref miss num:      %d  slice miss num: %d\n",data>>16, data&0xffff);



       {

              //U32 parser_timout,parser_err;

              //U32 pipe_timeout,pipe_err;

              U32 pic_width, pic_heigth, SrcFrameRate;

              ReadReg32(MAILBOX_13, &(U32)data);

              pic_heigth = data & 0x3fff;

              data = data >> 14;

              pic_width = data & 0x3fff;

              SrcFrameRate = data >> 14;

        printf("[VIDEO] video source format is : (%d x %d) @ (%d)\n",pic_width,pic_heigth,SrcFrameRate );

              ReadReg32(MAILBOX_0, &(U32)data);

              printf("[VIDEO] Aspect Ratio 0: (%d, %d)\n", (data>>14)&0x3fff, data&0x3fff);

              ReadReg32(MAILBOX_1, &(U32)data);

              printf("[VIDEO] Aspect Ratio 1: (%d, %d)\n", (data>>14)&0x3fff, data&0x3fff);

       }



/*

       ReadReg32(PARSER_MAIL0, &(U32)data);

       printf("[VIDEO] PARSER_MAIL0:    %08x\n",data);

       ReadReg32(PARSER_MAIL1, &(U32)data);

       printf("[VIDEO] PARSER_MAIL1:    %08x\n",data);



       ReadReg32(PARSER_MAIL4, &(U32)data);

       printf("[VIDEO] PARSER_MAIL4:    %08x\n",data);

       ReadReg32(PARSER_MAIL5, &(U32)data);

       printf("[VIDEO] PARSER_MAIL5:    %08x\n",data);



       ReadReg32(PARSER_MAIL2, &(U32)data);

       printf("[VIDEO] PARSER_MAIL2 (pts=0):      %08x\n",data);

*/

       ReadReg32(PARSER_MAIL4, &(U32)data);

       printf("[VIDEO] PARSER_MAIL4:    (%d, %d)\n",data>>16, data&0xffff);



       ReadReg32(PARSER_MAIL3, &(U32)data);

       printf("[VIDEO] PARSER_MAIL3 (no pts cnt):      %08x\n",data);

       ReadReg32(PARSER_MAIL6, &(U32)data);

       printf("[VIDEO] PARSER_MAIL6:    %08x\n",data);



       ReadReg32(PARSER_MAIL7, &(U32)data);

       printf("[VIDEO] PARSER_MAIL7:    %08x\n",data);



       {

              float Ndiv,Mmtu,freq;

              U32 od;

              ReadReg32(0x10171100, &(U32)data);

              Mmtu = data&0xff;

              Ndiv = (data>>9)&0x1f;

              od   = (data>>14)&0x3;

              freq = 27 * Mmtu / Ndiv /(1<<od);

              printf("[VIDEO] DDR Freq:   %6.2f MHz (%08x)\n",freq,data);

              ReadReg32(0x10171408, &(U32)data);

              Mmtu = data&0xff;

              Ndiv = (data>>9)&0x1f;

              od   = (data>>14)&0x3;

              freq = 27 * Mmtu / Ndiv / (1<<od) / 2;

              printf("[VIDEO] VID Freq:    %6.2f MHz (%08x)\n",freq,data);

       }



//     printf("[VIDEO] CPB0_REGION = 0x%08x (size = %08x)\n", CPB0_REGION, CPB0_SIZE);

//     printf("[VIDEO] CPB0_DIR_REGION = 0x%08x (size = %08x)\n", CPB0_DIR_REGION, CPB0_DIR_SIZE);

//     printf("[VIDEO] DPB0_REGION = 0x%08x (size = %08x)\n", DPB0_REGION, DPB0_SIZE);

       //printf("[VIDEO] AUD_PTS_REGION = 0x%08x \n", AUD_PTS_REGION);

}



void printf_mpge2_status(void)

{

    U32 data,data2;

       U32 CPB_DIR_RP,CPB_DIR_WP;



       {

       U32 CodeType,Year,Month,Day,Time;

       U32 data1;

       ReadReg32(MIPS_STA2, &(U32)data);     data1 = data;

       Time = data&0xF; data = data>>4;

       Day = data&0xFF; data = data>>8;

       Month = data&0xFF; data = data>>8;

       Year = data&0xFF; data = data>>8;

       CodeType = data&0xF;

       if(CodeType == 5)

              printf("[VIDEO] video firmware version:    H264-20%02d-%02d-%02d-%02d\n",Year,Month,Day,Time);

       else if(CodeType == 3)

              printf("[VIDEO] video firmware version: MPEG2-20%02d-%02d-%02d-%02d\n",Year,Month,Day,Time);

       else

              printf("[VIDEO] video firmware version: error MIPS_STA2 = %08x\n",data1);

       }

       ReadReg32(0x41600024, &(U32)CPB_DIR_RP);

       ReadReg32(0x41600028, &(U32)CPB_DIR_WP);

       printf("[VIDEO] CPB_DIR RP            0x%08x\n", CPB_DIR_RP);

       printf("[VIDEO] CPB_DIR WP           0x%08x\n", CPB_DIR_WP);

       ReadReg32(0x416000c0,&(U32)data);

       printf("[VIDEO] MIPS_STATUS: %08x \n", data);



       ReadReg32(0x416000c4,&(U32)data);

       printf("[VIDEO] Host_config:       %08x \n", data);

       // printf CPB status

       {

              U32 CPBU, CPBL;

              U32 CPB_SIZE;

              U32 CPB_EMPTY_SIZE;

              U32 rtog,wtog;

              U32 rp,wp;

              U32 len = 0;

              U32 i;



              ReadReg32(VID_CPBU0, &CPBU);

              ReadReg32(VID_CPBL0, &CPBL);

              CPB_SIZE = CPBU - CPBL;



              rtog = (CPB_DIR_RP>>28) & 0x1;

              rp = CPB_DIR_RP & 0xfffffff;

              wtog = (CPB_DIR_WP>>28) & 0x1;

              wp = CPB_DIR_WP & 0xfffffff;



              if( (wp > rp) && (rtog == wtog) )

              {

                     CPB_EMPTY_SIZE = CPB_SIZE - (wp - rp);

              }

              else if( (wp < rp) && (rtog != wtog) )

              {

                     CPB_EMPTY_SIZE = rp - wp;

              }

              else if(rtog == wtog)

              {

                     CPB_EMPTY_SIZE = CPB_SIZE;

              }

              else if(rtog != wtog)

              {

                     CPB_EMPTY_SIZE = 0;

              }

              else

              {

                     printf("[VIDEO] CPB pointer error !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

              }



              len = 78 * CPB_EMPTY_SIZE / CPB_SIZE;

              printf("[VIDEO] CPB status: ");

              for(i=0; i<len; i++) printf(" ");

              printf("@\n");

              printf("[VIDEO] CPB status:                    |                   |                   |                   |\n");

       }



       ReadReg32(MAILBOX_0,&(U32)data);

       printf("[VIDEO] mailbox_0 (firmware version): %d-%d-%d-%d\n", (data>>16)&0xf,(data>>12)&0xf,(data>>4)&0x1f,data&0xf);

       ReadReg32(MAILBOX_13,&(U32)data);  //3

       printf("[VIDEO] mailbox_13 (video format):     %d x %d @ %d\n", (data>>14)&0x3fff,data&0x3fff, data>>28);

       ReadReg32(MAILBOX_2,&(U32)data);    //3

       printf("[VIDEO] mailbox_2 (host command):    %08x \n", data);

       ReadReg32(MAILBOX_3,&(U32)data);    //3

       printf("[VIDEO] mailbox_3 (has send DF): %08x \n", data);



       ReadReg32(MAILBOX_5,&(U32)data);    //5

       printf("[VIDEO] mailbox_5 (M2VD IRQ): %08x \n", data);

       ReadReg32(MAILBOX_6,&(U32)data);    //6

       printf("[VIDEO] mailbox_6 (m2vd VBV_RP):  %08x \n", data);

       ReadReg32(MAILBOX_7,&(U32)data);    //7

       printf("[VIDEO] mailbox_7 (m2vd VBV_WP): %08x \n", data);



       ReadReg32(MAILBOX_4,&(U32)data);    //3

       printf("[VIDEO] mailbox_4 (parser pic num):    %d \n", data);



       ReadReg32(MAILBOX_8,&(U32)data);    //8

       printf("[VIDEO] Audio Diff:  %08x \n", data);

       ReadReg32(MAILBOX_9,&(U32)data2);  //9

       printf("[VIDEO] Video Diff:  %08x \n", data2);

       printf("[VIDEO] Video - Audio:    %08x (%d)\n", (data2-data), (signed int)(data2-data)/45);

       ReadReg32(MAILBOX_10,&(U32)data);  //10

//     printf("[VIDEO] mailbox_10 (error number):     %d \n", data);

       printf("[VIDEO] STC - PTS:   %08x (%d) \n", data, (signed int)data/45);



       ReadReg32(MAILBOX_11,&(U32)data);  //11

       printf("[VIDEO] mailbox_11 (timeout number): %d \n", data);

       ReadReg32(MAILBOX_15,&(U32)data);  //3

       printf("[VIDEO] MAILBOX_15 (current PTS):              %08x \n", data);



       ReadReg32(MAILBOX_12,&(U32)data);  //11

       printf("[VIDEO] CPB PTS: %d     FIFO PTS %d\n", data>>16, data&0xFFFF);



       //printf("[VIDEO] CPB0_REGION = 0x%08x (size = %08x)\n", CPB0_REGION, CPB0_SIZE);

       //printf("[VIDEO] CPB0_DIR_REGION = 0x%08x (size = %08x)\n", CPB0_DIR_REGION, CPB0_DIR_SIZE);



}


#if 0
void printf_video_status(U32 uiVideoCodec)
{

    U32 type;



    type = (uiVideoCodec & VIDEO_CODEC_MASK) >> VIDEO_CODEC_OFFSET;



    if (type == VIDEO_CODEC_H264)

        printf_h264_status();

    else if (type == VIDEO_CODEC_MPEG2)

        printf_mpge2_status();



    return;

}
#endif




int  InitReadWriteReg(void)
{
    fd = open("/dev/mem", O_RDWR);
    if (fd == -1)
    {
        printf("can not open mem device map\n");
        return CSAPI_FAILED;
    }

    page_size = getpagesize();
    page_size_shift = 0;
    while ((page_size >> page_size_shift) != 0)
        page_size_shift++;
    page_size_shift--;

    pa = 0xffffffff << page_size_shift;
    va = (unsigned char *)mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa);
    if (va == MAP_FAILED)
    {
        printf("map physical memory failed\n");
        close(fd);
        fd = 0;
        return CSAPI_FAILED;
    }

    printf("page_size %d shift %d pa 0x%08x  va 0x%08x\n", page_size, page_size_shift, pa, (unsigned int)va);
    return CSAPI_SUCCEED;
}

int	ReadReg32( unsigned int address, unsigned int * pVal32 )
{
    pthread_mutex_lock(&mutex_register);

    if (fd == 0)
	InitReadWriteReg();

    if (!CACHED(address))
    {
        munmap(va, page_size);
        pa = address & (0xffffffff << page_size_shift);
        va = (unsigned char *)mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa);
        if (va == MAP_FAILED)
        {
            printf("map physical memory failed: shift %d  address 0x%08x  pa 0x%08x page_size %d  va 0x%08x\n", page_size_shift, address, pa, page_size, (unsigned int)va);
            pthread_mutex_unlock(&mutex_register);
            return CSAPI_FAILED;
        }
    }

    *pVal32 = *( (volatile unsigned int *)(va + (address & (page_size - 1))) );

    pthread_mutex_unlock(&mutex_register);

    return  CSAPI_SUCCEED;
}

int	WriteReg32( unsigned int address, unsigned int  Val32 )
{
    pthread_mutex_lock(&mutex_register);

    if (fd == 0)
	InitReadWriteReg();

    if (!CACHED(address))
    {
        munmap(va, page_size);
        pa = address & (0xffffffff << page_size_shift);
        va = (unsigned char *)mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa);
        if (va == MAP_FAILED)
        {
            printf("map physical memory failed: shift %d  address 0x%08x  pa 0x%08x page_size %d  va 0x%08x\n", page_size_shift, address, pa, page_size, (unsigned int)va);
            pthread_mutex_unlock(&mutex_register);
            return CSAPI_FAILED;
        }
    }

    *( (volatile unsigned int *)(va + (address & (page_size - 1))) ) = Val32;

    pthread_mutex_unlock(&mutex_register);

    return  CSAPI_SUCCEED;
}

S32 ClearChipMem( U32 address, U32 size )
{

    INT32 fd, i;

    U32 page_size, page_size_shift = 0;

    PU8 buf;

    U32 phy_addr_page, phy_addr_offset, map_mem_size;



    fd = open("/dev/mem", O_RDWR);

    if (fd == -1)
    {

        printf("can not open mem device map\n");

        return 0;

    }



    /* if there is initialization part for this operation,

       we can move following part to there */

    page_size = getpagesize();

    while ((page_size >> page_size_shift) != 0)

        page_size_shift++;

    page_size_shift --;



    phy_addr_page = (address >> page_size_shift) << page_size_shift;

    phy_addr_offset = address - phy_addr_page;

    map_mem_size = ( ( size + page_size - 1 ) >> page_size_shift ) <<page_size_shift;

    /* map page_size physical memory to logical memory space */

    buf = (PU8)mmap(NULL, map_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_page);

    if (buf == MAP_FAILED)
    {

        printf("map physical memory failed phy_addr_page=0x%08x map_mem_size=0x%08x\n",phy_addr_page, map_mem_size );

        close(fd);

        return 0;

    }


    /* memory copy */
    for (i = 0; i < size; i++)
    {

        *((volatile PU8)(buf + phy_addr_offset + i)) = 0;

    }


    munmap(buf, map_mem_size);

    close(fd);

    return 1;

}

static void EXAPI_SetAspectRatio(tCS_AV_VideoAspect ratio)
{
	int tempcrop = 0;
	CSTVOUT_MODE tvmode = TVOUT_MODE_576I;
	CSVID_Rect src, dst;
	CSVID_SequenceHeader hdr;

	CSTVOUT_GetMode(tve_handle, &tvmode);

	CSVID_GetSequenceHeader(vid_handle, &hdr);

	if (eCS_AV_VIDEO_ASPECT_4_3 == ratio) {
		switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
			tempcrop = (720 - (480 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 720 - dst.left;
			dst.top = 0;
			dst.bottom = 480;
			break;

		case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
			tempcrop = (576 - (720 * 3 / 4)) / 2;
			if (mod(tempcrop) > 10) {
				dst.top = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.top = 16 * div(tempcrop);
			}
			dst.bottom = 576 - dst.top;
			dst.left = 0;
			dst.right = 720;
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			tempcrop = (1280 - (720 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 1280 - dst.left;
			dst.top = 0;
			dst.bottom = 720;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			tempcrop = (1920 - (1080 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 1920 - dst.left;
			dst.top = 0;
			dst.bottom = 1080;
			break;

		default:
			return;
		}
	}
	else if (eCS_AV_VIDEO_ASPECT_16_9 == ratio) {
		switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
			tempcrop = (480 - (720 * 9 / 16)) / 2;
			if (mod(tempcrop) > 10) {
				dst.top = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.top = 16 * div(tempcrop);
			}
			dst.bottom = 480 - dst.top;
			dst.left = 0;
			dst.right = 720;
			break;

		case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
			tempcrop = (576 - (720 * 9 / 16)) / 2;
			if (mod(tempcrop) > 10) {
				dst.top = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.top = 16 * div(tempcrop);
			}
			dst.bottom = 576 - dst.top;
			dst.left = 0;
			dst.right = 720;
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			dst.left = dst.top = 0;
			dst.right = 1280;
			dst.bottom = 720;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			dst.left = dst.top = 0;
			dst.right = 1920;
			dst.bottom = 1080;
			break;

		default:
			return;
		}
	}
	else {
		return;
	}
	src.left = src.top = 0;
	src.right = hdr.w;
	src.bottom = hdr.h;

	CSVID_SetOutputPostion(vid_handle, &src, &dst);
}
#endif

tCS_AV_Error Audio_SetMuteStatus( BOOL enable )
{
    tCS_AV_Error    err = eCS_AV_OK;

    if( aud_handle == NULL )
        {
                return eCS_AV_ERROR;
        }

    if( enable )
        {
            CSAUD_EnableMute( aud_handle );
        }
        else
        {
            CSAUD_DisableMute( aud_handle );
        }

    return(err);
}
#if 1

#if 1
void AspectRatio_Resize(CSVID_Rect* psrc,CSVID_Rect* pdst,tCS_DBU_VideoAspectRatio dst_aspect_ratio, CSVID_ASPECTRATIO src_vid_aspect,CSSQC_VIDEO_ASPECTMODE aspectmode,CSTVOUT_MODE tvmode)
{
	int 			tempcrop = 0;
    unsigned int 	src_width;
    unsigned int 	src_height;
    unsigned int 	dst_width;
    unsigned int 	dst_height;
    CSVID_Rect 		src, dst;



	memcpy(&src,psrc,sizeof(CSVID_Rect));

	if (src.bottom > 1080)
	{
		/* For Error in Video Driver */
		src.bottom = 1080;
	}
	/* set dst output rectangle */
	switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
			dst.left = 0;
			dst.right = 720;
			dst.top = 0;
			dst.bottom = 480;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;
			break;

		case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
		case TVOUT_MODE_SECAM:
			dst.left = 0;
			dst.right = 720;
			dst.top = 0;
			dst.bottom = 576;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			dst.left = 0;
			dst.right = 1280;
			dst.top = 0;
			dst.bottom = 720;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			dst.left = 0;
			dst.right = 1920;
			dst.top = 0;
			dst.bottom = 1080;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;
			break;

		default:
			printf("%s tvmode [%d] not support!\n", __FUNCTION__, tvmode);
			/* Set to 1080 Mode for default */
			dst.left = 0;
			dst.right = 1920;
			dst.top = 0;
			dst.bottom = 1080;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;
			break;
	}

	if (dst.bottom != src.bottom)
	{
		if ((dst.bottom == 720) && (dst.bottom < src.bottom))
		{
			src.bottom = src.bottom;
		}
		else
		{
			src_width  = (src.right * dst.bottom) / src.bottom;
			src.bottom = dst.bottom;
			src.right  = src_width;
			src.left   = 0;
		}
	}

	src_width = src.right - src.left;
	src_height = src.bottom - src.top;

	/* set src input rectangle */
	if(aspectmode == SQC_PAN_SCAN)
	{
		if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)
		{
			/* a little wider, so cut from left and right to fit 4:3 */
			switch(src_vid_aspect)
			{
	            // by KB Kim 2010.08.31 for AV Setting
				case CSVID_1TO1:
				case CSVID_16TO9:
					{
						tempcrop = (src_width - (src_width* (4 *9)/ (3*16))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}
						src.left += tempcrop;
						src.right -= tempcrop;
					}
					break;
				case CSVID_16TO11:
					if(src_width > (src_height* (4 *11)/ (3*16)))
					{
						tempcrop = (src_width- (src_height* (4 *11)/ (3*16))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}
						src.left += tempcrop;
						src.right -= tempcrop;
					}
					break;

				case    CSVID_4TO3:
				default:
					break;
			}
		}
		else if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)
		{
			switch(src_vid_aspect)
			{
				case    CSVID_4TO3:
					{
						tempcrop = (src_height - (src_height * (4 *9)/ (3*16))) / 2;
						//   printf("src_height=%d tempcrop=%d\n",tempcrop,src_height);
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}
						src.top += tempcrop;
						src.bottom -= tempcrop;
					}
				break;
				case    CSVID_16TO11:
					if(src_width > (src_height* (16 *11)/ (9*16)))
					{
						tempcrop = (src_width- (src_height* (16 *11)/ (9*16))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}
						src.left += tempcrop;
						src.right -= tempcrop;
					}
				break;

				case    CSVID_16TO9:
				default:
					break;
			}
		}
	}
	else if (aspectmode == SQC_LETTERBOX)
	{
		src.left   = 0;
		src.right  = 0;
		src.top    = 0;
		src.bottom = 0;

		if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)
		{
			switch(src_vid_aspect)
			{
				case    CSVID_1TO1:
				case    CSVID_16TO9:
					{

						tempcrop = (dst_height - (dst_height* (4 *9)/ (3*16))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}

						dst.top += tempcrop;
						dst.bottom -= tempcrop;

					}
					break;
				case    CSVID_16TO11:
					if(dst_height > (dst_height* src_height* (4 *11)/ (3*16*src_width)))
					{
						tempcrop = (dst_height - (dst_height* src_height* (4 *11)/ (3*16*src_width))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}

						dst.top += tempcrop;
						dst.bottom -= tempcrop;
					}
					break;

				case    CSVID_4TO3:
				default:
					break;
			}
		}
		else if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)
		{
			switch(src_vid_aspect)
			{
				case    CSVID_4TO3:
					{
						tempcrop = (dst_width - (dst_width* (4 *9)/ (3*16))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}
						dst.left += tempcrop;
						dst.right -= tempcrop;

					}
					break;
				case    CSVID_16TO11:
					if(dst_width > (dst_width* src_width* (16 *9)/ (11*16*src_height)))
					{
						tempcrop = (dst_width - (dst_width* src_width* (16 *9)/ (11*16*src_height))) / 2;
						if (mod(tempcrop) > 10) {
							tempcrop = 16 * (div(tempcrop) + 1);
						}
						else {
							tempcrop = 16 * div(tempcrop);
						}

						dst.left += tempcrop;
						dst.right -= tempcrop;
					}
					break;

				case    CSVID_16TO9:
				default:
					break;
			}
		}
	}
	else
	{
		src.left   = 0;
		src.right  = 0;
		src.top    = 0;
		src.bottom = 0;
	}

	memcpy(psrc,&src,sizeof(CSVID_Rect));
	memcpy(pdst,&dst,sizeof(CSVID_Rect));

}
#else
void AspectRatio_Resize(CSVID_Rect* psrc,CSVID_Rect* pdst,tCS_DBU_VideoAspectRatio dst_aspect_ratio, CSVID_ASPECTRATIO src_vid_aspect,CSSQC_VIDEO_ASPECTMODE aspectmode,CSTVOUT_MODE tvmode)
{
	int 			tempcrop = 0;
    unsigned int 	src_width;
    unsigned int 	src_height;
    unsigned int 	dst_width;
    unsigned int 	dst_height;
    CSVID_Rect 		src, dst;



	memcpy(&src,psrc,sizeof(CSVID_Rect));
	memcpy(&dst,pdst,sizeof(CSVID_Rect));

	src_width = src.right - src.left;
	src_height = src.bottom - src.top;

	/* set src input rectangle */
	if((SQC_PAN_SCAN == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)) {

		/* a little wider, so cut from left and right to fit 4:3 */
		switch(src_vid_aspect)
		{
                        // by KB Kim 2010.08.31 for AV Setting
			case CSVID_1TO1:
			case CSVID_16TO9:
				{
					tempcrop = (src_width - (src_width* (4 *9)/ (3*16))) / 2;
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.left += tempcrop;
					src.right -= tempcrop;
				}
				break;
/*
			case CSVID_1TO1:
				if(src_width > (src_height* (4 *1)/ (3*1)))
				{
					tempcrop = (src_width- (src_height* (4 *1)/ (3*1))) / 2;
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.left += tempcrop;
					src.right -= tempcrop;
				}
				break;
*/
			case CSVID_16TO11:
				if(src_width > (src_height* (4 *11)/ (3*16)))
				{
					tempcrop = (src_width- (src_height* (4 *11)/ (3*16))) / 2;
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.left += tempcrop;
					src.right -= tempcrop;
				}
				break;

			case    CSVID_4TO3:
			default:
				break;
		}
	} else if((SQC_PAN_SCAN == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)) {

		/* a little higher, so cut from top and bottom to fit 16:9*/
		switch(src_vid_aspect)
		{
			case    CSVID_4TO3:
				{
					tempcrop = (src_height - (src_height * (4 *9)/ (3*16))) / 2;
					//   printf("src_height=%d tempcrop=%d\n",tempcrop,src_height);
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.top += tempcrop;
					src.bottom -= tempcrop;
				}
			break;
/*
			case    CSVID_1TO1:
				if(src_width > (src_height* (16 *1)/ (9*1)))
				{
					tempcrop = (src_width- (src_height* (16 *1)/ (9*1))) / 2;
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.left += tempcrop;
					src.right -= tempcrop;
				}
			break;
*/
			case    CSVID_16TO11:
				if(src_width > (src_height* (16 *11)/ (9*16)))
				{
					tempcrop = (src_width- (src_height* (16 *11)/ (9*16))) / 2;
					if (mod(tempcrop) > 10) {
						tempcrop = 16 * (div(tempcrop) + 1);
					}
					else {
						tempcrop = 16 * div(tempcrop);
					}
					src.left += tempcrop;
					src.right -= tempcrop;
				}
			break;

			case    CSVID_16TO9:
			default:
				break;
		}

		//cur_mode = SQC_PAN_SCAN;
	}

	/* set dst output rectangle */
	switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
			dst.left = 0;
			dst.right = 720;
			dst.top = 0;
			dst.bottom = 480;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;

			if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)) {

				switch(src_vid_aspect)
				{
					case    CSVID_1TO1:
					case    CSVID_16TO9:
						{

							tempcrop = (dst_height - (dst_height* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_height > (dst_height* src_height* (4 *1)/ (3*1*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *1)/ (3*1*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_height > (dst_height* src_height* (4 *11)/ (3*16*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *11)/ (3*16*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;

					case    CSVID_4TO3:
					default:
						break;
				}
			}
			else if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9))
			{
				switch(src_vid_aspect)
				{
					case    CSVID_4TO3:
						{
							tempcrop = (dst_width - (dst_width* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}
							dst.left += tempcrop;
							dst.right -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_width > (dst_width* src_width* (1 *9)/ (1*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (1 *9)/ (1*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_width > (dst_width* src_width* (16 *9)/ (11*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (16 *9)/ (11*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;

					case    CSVID_16TO9:
					default:
						break;
				}

			}

			break;

		case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
		case TVOUT_MODE_SECAM:
			dst.left = 0;
			dst.right = 720;
			dst.top = 0;
			dst.bottom = 576;

			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;

			if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)) {

				switch(src_vid_aspect)
				{
					case    CSVID_1TO1:
					case    CSVID_16TO9:
						{

							tempcrop = (dst_height - (dst_height* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_height > (dst_height* src_height* (4 *1)/ (3*1*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *1)/ (3*1*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_height > (dst_height* src_height* (4 *11)/ (3*16*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *11)/ (3*16*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;

					case    CSVID_4TO3:
					default:
						break;
				}
			}
			else if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9))
			{
				switch(src_vid_aspect)
				{
					case    CSVID_4TO3:
						{
							tempcrop = (dst_width - (dst_width* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}
							dst.left += tempcrop;
							dst.right -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_width > (dst_width* src_width* (1 *9)/ (1*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (1 *9)/ (1*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_width > (dst_width* src_width* (16 *9)/ (11*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (16 *9)/ (11*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;

					case    CSVID_16TO9:
					default:
						break;
				}

			}
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			dst.left = 0;
			dst.right = 1280;
			dst.top = 0;
			dst.bottom = 720;

			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;

			if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)) {

				switch(src_vid_aspect)
				{
					case    CSVID_1TO1:
					case    CSVID_16TO9:
						{

							tempcrop = (dst_height - (dst_height* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_height > (dst_height* src_height* (4 *1)/ (3*1*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *1)/ (3*1*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_height > (dst_height* src_height* (4 *11)/ (3*16*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *11)/ (3*16*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;

					case    CSVID_4TO3:
					default:
						break;
				}
			}
			else if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9))
			{
				switch(src_vid_aspect)
				{
					case    CSVID_4TO3:
						{
							tempcrop = (dst_width - (dst_width* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}
							dst.left += tempcrop;
							dst.right -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_width > (dst_width* src_width* (1 *9)/ (1*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (1 *9)/ (1*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_width > (dst_width* src_width* (16 *9)/ (11*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (16 *9)/ (11*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;

					case    CSVID_16TO9:
					default:
						break;
				}

			}
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			dst.left = 0;
			dst.right = 1920;
			dst.top = 0;
			dst.bottom = 1080;
			dst_width = dst.right - dst.left;
			dst_height = dst.bottom - dst.top;

			if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)) {

				switch(src_vid_aspect)
				{
					case    CSVID_1TO1:
					case    CSVID_16TO9:
						{

							tempcrop = (dst_height - (dst_height* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_height > (dst_height* src_height* (4 *1)/ (3*1*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *1)/ (3*1*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
					if(dst_height > (dst_height* src_height* (4 *11)/ (3*16*src_width)))
						{
							tempcrop = (dst_height - (dst_height* src_height* (4 *11)/ (3*16*src_width))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.top += tempcrop;
							dst.bottom -= tempcrop;
						}
						break;

					case    CSVID_4TO3:
					default:
						break;
				}
			}
			else if((SQC_LETTERBOX == aspectmode) && (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9))
			{
				switch(src_vid_aspect)
				{
					case    CSVID_4TO3:
						{
							tempcrop = (dst_width - (dst_width* (4 *9)/ (3*16))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}
							dst.left += tempcrop;
							dst.right -= tempcrop;

						}
						break;
/*
					case    CSVID_1TO1:
						if(dst_width > (dst_width* src_width* (1 *9)/ (1*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (1 *9)/ (1*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;
*/
					case    CSVID_16TO11:
						if(dst_width > (dst_width* src_width* (16 *9)/ (11*16*src_height)))
						{
							tempcrop = (dst_width - (dst_width* src_width* (16 *9)/ (11*16*src_height))) / 2;
							if (mod(tempcrop) > 10) {
								tempcrop = 16 * (div(tempcrop) + 1);
							}
							else {
								tempcrop = 16 * div(tempcrop);
							}

							dst.left += tempcrop;
							dst.right -= tempcrop;
						}
						break;

					case    CSVID_16TO9:
					default:
						break;
				}

			}
			break;

		default:
			printf("%s tvmode [%d] not support!\n", __FUNCTION__, tvmode);
			break;
	}

	memcpy(psrc,&src,sizeof(CSVID_Rect));
	memcpy(pdst,&dst,sizeof(CSVID_Rect));

}
#endif

static CSVID_Rect lastsrc, lastdst;
tCS_AV_Error SetVideoScalor(CSVID_ASPECTRATIO src_vid_aspect, tCS_DBU_VideoAspectRatio dst_aspect_ratio, CSSQC_VIDEO_ASPECTMODE aspectmode, CSVID_Rect * window_rect)
{
	CSVID_SequenceHeader 	hdr;
	CSVID_Rect 				src, dst;
	CSTVOUT_MODE 			tvmode = TVOUT_MODE_576I;
	unsigned int 			src_width;
	unsigned int 			src_height;

	if( vid_handle == NULL )
	{
		return eCS_AV_ERROR;
	}

	/* By KB Kim : 2010_08_31 for Scart Control */
	if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_4_3)
	{
		ScartAspecChange(0);
#ifdef USE_HDMI_CAT6611
		Cat6611_SetAspecRatio(0);
#endif //#ifdef USE_HDMI_CAT6611
	}
	else if (dst_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)
	{
		ScartAspecChange(1);
#ifdef USE_HDMI_CAT6611
		Cat6611_SetAspecRatio(1);
#endif //#ifdef USE_HDMI_CAT6611
	}
	else
	{
		if ((src_vid_aspect == CSVID_16TO9) || (src_vid_aspect == CSVID_1TO1))
		{
			/* 16:9 format */
			ScartAspecChange(1);
#ifdef USE_HDMI_CAT6611
			Cat6611_SetAspecRatio(1);
#endif //#ifdef USE_HDMI_CAT6611
		}
		else
		{
			/* 4:3 format */
			ScartAspecChange(0);
#ifdef USE_HDMI_CAT6611
			Cat6611_SetAspecRatio(0);
#endif //#ifdef USE_HDMI_CAT6611
		}
	}

#if 0
	CSVID_GetSequenceHeader(vid_handle, &hdr);

    src.top = 0;
	src.left  = 0;
	src.right = hdr.w;
	src.bottom = hdr.h;

	src_width = src.right - src.left;
	src_height = src.bottom - src.top;

	if((src_width ==0)||(src_height ==0))
		return eCS_AV_ERROR;

	CSTVOUT_GetMode(tve_handle, &tvmode);

	if(window_rect == NULL)
	{
		AspectRatio_Resize(&src,&dst,dst_aspect_ratio,src_vid_aspect,aspectmode,tvmode);

		if((dst.bottom==lastdst.bottom)&&(dst.left==lastdst.left)&&(dst.right==lastdst.right)&&(dst.top==lastdst.top))
		{
			return eCS_AV_OK;;
		}
		else
		{
			memset( &src, 0, sizeof(CSVID_Rect));
				// printf("****************src[%d, %d, %d, %d]**************\n", src.left, src.right, src.top, src.bottom);
    				// printf("****23232************dst[%d, %d, %d, %d]**************\n", dst.left, dst.right, dst.top, dst.bottom);
			CSVID_SetOutputPostion(vid_handle, &src, &dst);
		}
	}
#endif

    CSVID_GetSRCSequenceHeader(vid_handle, &hdr);
    // CSVID_GetSequenceHeader(vid_handle, &hdr);
	if((hdr.w ==0)||(hdr.h ==0))
		return eCS_AV_ERROR;

    src.top = 0;
	src.left  = 0;
	src.right = hdr.w;
	src.bottom = hdr.h;
	src_width = src.right - src.left;
	src_height = src.bottom - src.top;

	CSTVOUT_GetMode(tve_handle, &tvmode);

	if(window_rect == NULL)
	{
		// printf("0. SRC.top=%d  SRC.bottom=%d left=%d right=%d Src aspect=%d\n",src.top,src.bottom,src.left, src.right,src_vid_aspect);
		AspectRatio_Resize( &src, &dst, dst_aspect_ratio, src_vid_aspect, aspectmode, tvmode);
		// printf("1. SRC.top=%d  SRC.bottom=%d left=%d right=%d Src aspect=%d\n",src.top,src.bottom,src.left, src.right,src_vid_aspect);
		// printf("0. DST.top=%d  DST.bottom=%d left=%d right=%d DST aspect=%d\n",dst.top,dst.bottom,dst.left,dst.right,dst_aspect_ratio);
		/*
			if((dst.bottom==lastdst.bottom)&&(dst.left==lastdst.left)&&(dst.right==lastdst.right)&&(dst.top==lastdst.top))
			{
				 return eCS_AV_OK;;
			}
			*/
			// memset( &src, 0, sizeof(CSVID_Rect));
			/*
			if((src.top+src.bottom)>576)
		{
			src.bottom = src.bottom - ( (src.top + src.bottom) - 1080 );
		}
				*/

		//printf("3. SRC.top=%d  SRC.bottom=%d Src aspect=%d\n",src.top,src.bottom,src_vid_aspect);
	}
	else
	{
		U16    uleft, uright, utop, ubottom;

		switch (tvmode)
		{
			case TVOUT_MODE_480I:
			case TVOUT_MODE_480P:
                        uleft = window_rect->left*720/MVAPP_OSD_MAX_WIDTH;
                        uright = window_rect->right*720/MVAPP_OSD_MAX_WIDTH;
                        utop = window_rect->top*480/MVAPP_OSD_MAX_HEIGHT;
                        ubottom = window_rect->bottom*480/MVAPP_OSD_MAX_HEIGHT;
				break;

			case TVOUT_MODE_720P50:
			case TVOUT_MODE_720P60:
                        uleft = window_rect->left*1280/MVAPP_OSD_MAX_WIDTH;
                        uright = window_rect->right*1280/MVAPP_OSD_MAX_WIDTH;
                        utop = window_rect->top*720/MVAPP_OSD_MAX_HEIGHT;
                        ubottom = window_rect->bottom*720/MVAPP_OSD_MAX_HEIGHT;
				break;

			case TVOUT_MODE_1080I25:
			case TVOUT_MODE_1080I30:
                        uleft = window_rect->left*1920/MVAPP_OSD_MAX_WIDTH;
                        uright = window_rect->right*1920/MVAPP_OSD_MAX_WIDTH;
                        utop = window_rect->top*1080/MVAPP_OSD_MAX_HEIGHT;
                        ubottom = window_rect->bottom*1080/MVAPP_OSD_MAX_HEIGHT;
				break;

			case TVOUT_MODE_576I:
			case TVOUT_MODE_576P:
			case TVOUT_MODE_SECAM:
			default:
                        uleft = window_rect->left*720/MVAPP_OSD_MAX_WIDTH;
                        uright = window_rect->right*720/MVAPP_OSD_MAX_WIDTH;
                        utop = window_rect->top*576/MVAPP_OSD_MAX_HEIGHT;
                        ubottom = window_rect->bottom*576/MVAPP_OSD_MAX_HEIGHT;
				break;
		}

		if (mod(uleft) > 10) {
			uleft = 16 * (div(uleft) + 1);
		}
		else {
			uleft = 16 * div(uleft);
		}

		if (mod(uright) > 10) {
			uright = 16 * (div(uright) + 1);
		}
		else {
			uright = 16 * div(uright);
		}

		if (mod(utop) > 10) {
			utop = 16 * (div(utop) + 1);
		}
		else {
			utop = 16 * div(utop);
		}

		if (mod(ubottom) > 10) {
			ubottom = 16 * (div(ubottom) + 1);
		}
		else {
			ubottom = 16 * div(ubottom);
		}

		dst.top = utop;
		dst.bottom = ubottom;
		dst.left = uleft;
		dst.right = uright;
	}

	//printf("****************src[%d, %d, %d, %d]**************\n", src.left, src.right, src.top, src.bottom);
	//printf("****************dst[%d, %d, %d, %d]**************\n", dst.left, dst.right, dst.top, dst.bottom);

	if((src.bottom==lastsrc.bottom)&&(src.left==lastsrc.left)&&(src.right==lastsrc.right)&&(src.top==lastsrc.top)&&\
	(dst.bottom==lastdst.bottom)&&(dst.left==lastdst.left)&&(dst.right==lastdst.right)&&(dst.top==lastdst.top))
	{
		//printf("*********1*******src[%d, %d, %d, %d]**************\n", src.left, src.right, src.top, src.bottom);
		//printf("**********1******dst[%d, %d, %d, %d]**************\n", dst.left, dst.right, dst.top, dst.bottom);
	}
	else
	{
		// memset( &src, 0, sizeof(CSVID_Rect));
		CSVID_SetOutputPostion(vid_handle, &src, &dst);
		lastsrc=src;
		lastdst=dst;
		// printf("3. ****************Video Width[%d], Height[%d]**************\n", hdr.w, hdr.h);
		// printf("4. ****************src[%d, %d, %d, %d]**************\n", src.left, src.right, src.top, src.bottom);
		// printf("5. ****************dst[%d, %d, %d, %d]**************\n", dst.left, dst.right, dst.top, dst.bottom);
		//CSVID_SetOutputPostion(vid_handle, &src, &dst);
	}
	return eCS_AV_OK;
}

#else

tCS_AV_Error    SetVideoScalor(tCS_AV_VideoAspect ratio, CSVID_Rect * window_rect)
{
    int tempcrop = 0;
    CSVID_SequenceHeader hdr;
    CSVID_Rect src, dst;

    if( vid_handle == NULL )
    {
            return eCS_AV_ERROR;
    }

    CSVID_GetSequenceHeader(vid_handle, &hdr);

    src.left = src.top = 0;
    src.right = hdr.w;
    src.bottom = hdr.h;

    if(window_rect == NULL)
        {

	CSTVOUT_MODE tvmode = TVOUT_MODE_576I;
	CSVID_SequenceHeader hdr;

	CSTVOUT_GetMode(tve_handle, &tvmode);

	if (eCS_AV_VIDEO_ASPECT_4_3 == ratio) {
		switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
                        #if 1
			tempcrop = (720 - (480 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 720 - dst.left;
			dst.top = 0;
			dst.bottom = 480;
                        #else
                            dst.top = 0;
                            dst.bottom = 480;
                             dst.left = 0;
                             dst.right = 720;
                        #endif
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			tempcrop = (1280 - (720 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 1280 - dst.left;
			dst.top = 0;
			dst.bottom = 720;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			tempcrop = (1920 - (1080 * 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				dst.left = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.left = 16 * div(tempcrop);
			}
			dst.right = 1920 - dst.left;
			dst.top = 0;
			dst.bottom = 1080;
			break;


                case TVOUT_MODE_576I:
                case TVOUT_MODE_576P:
                default:
            #if 1
                        tempcrop = (576 - (720 * 3 / 4)) / 2;
                        if (mod(tempcrop) > 10) {
                            dst.top = 16 * (div(tempcrop) + 1);
                        }
                        else {
                            dst.top = 16 * div(tempcrop);
                        }
                        dst.bottom = 576 - dst.top;
                        dst.left = 0;
                        dst.right = 720;
            #else
                        dst.top = 0;
                        dst.bottom = 576;
                         dst.left = 0;
                         dst.right = 720;
            #endif
                        break;
		}
	}
	else if (eCS_AV_VIDEO_ASPECT_16_9 == ratio) {
		switch (tvmode) {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
			tempcrop = (480 - (720 * 9 / 16)) / 2;
			if (mod(tempcrop) > 10) {
				dst.top = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.top = 16 * div(tempcrop);
			}
			dst.bottom = 480 - dst.top;
			dst.left = 0;
			dst.right = 720;
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
			dst.left = dst.top = 0;
			dst.right = 1280;
			dst.bottom = 720;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
			dst.left = dst.top = 0;
			dst.right = 1920;
			dst.bottom = 1080;
			break;

                  case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
                  default:
			tempcrop = (576 - (720 * 9 / 16)) / 2;
			if (mod(tempcrop) > 10) {
				dst.top = 16 * (div(tempcrop) + 1);
			}
			else {
				dst.top = 16 * div(tempcrop);
			}
			dst.bottom = 576 - dst.top;
			dst.left = 0;
			dst.right = 720;
			break;
		}
	}
	else {
		return eCS_AV_ERROR;
	}

        }
    else
        {
            CSTVOUT_MODE tvmode = TVOUT_MODE_576I;
        	    CSVID_SequenceHeader hdr;
             U16    uleft, uright, utop, ubottom;

        	    CSTVOUT_GetMode(tve_handle, &tvmode);

            switch (tvmode)
                {
		case TVOUT_MODE_480I:
		case TVOUT_MODE_480P:
                        uleft = window_rect->left;
                        uright = window_rect->right;
                        utop = window_rect->top*480/576;
                        ubottom = window_rect->bottom*480/576;
			break;

		case TVOUT_MODE_720P50:
		case TVOUT_MODE_720P60:
                        uleft = window_rect->left*1280/720;
                        uright = window_rect->right*1280/720;
                        utop = window_rect->top*720/576;
                        ubottom = window_rect->bottom*720/576;
			break;

		case TVOUT_MODE_1080I25:
		case TVOUT_MODE_1080I30:
                        uleft = window_rect->left*1920/720;
                        uright = window_rect->right*1920/720;
                        utop = window_rect->top*1080/576;
                        ubottom = window_rect->bottom*1080/576;
			break;

                  case TVOUT_MODE_576I:
		case TVOUT_MODE_576P:
		default:
                        uleft = window_rect->left;
                        uright = window_rect->right;
                        utop = window_rect->top;
                        ubottom = window_rect->bottom;
			break;
		}

            if (eCS_AV_VIDEO_ASPECT_4_3 == ratio)
                {
                    tempcrop = ((uright - uleft) - ((ubottom - utop)* 4 / 3)) / 2;
			if (mod(tempcrop) > 10) {
				tempcrop = 16 * (div(tempcrop) + 1);
			}
			else {
				tempcrop = 16 * div(tempcrop);
			}

			dst.top = utop;
			dst.bottom = ubottom;
                           dst.left = uleft + tempcrop;
			dst.right = uright  - tempcrop;
                }
            else if (eCS_AV_VIDEO_ASPECT_16_9 == ratio)
                {
                        tempcrop = ((ubottom - utop) - ((uright - uleft) * 9 / 16)) / 2;
			if (mod(tempcrop) > 10) {
				tempcrop = 16 * (div(tempcrop) + 1);
			}
			else {
				tempcrop = 16 * div(tempcrop);
			}
                           dst.top = utop + tempcrop;
			dst.bottom = ubottom - tempcrop;
			dst.left = uleft;
			dst.right = uright;
                }
            else
                {
		        return eCS_AV_ERROR;
	        }
        }

        CSVID_SetOutputPostion(vid_handle, &src, &dst);

        return eCS_AV_OK;
}

#endif

tCS_AV_Error GetVideoOriginalInfo( tCS_AV_VideoOriginalInfo* pOri_Size )
{
    CSVID_SequenceHeader VideoMode;

    if( vid_handle == NULL )
    {
    	     printf("vid_handle error...\n");
            return eCS_AV_ERROR;
    }

    memset( &VideoMode, 0, sizeof(CSVID_SequenceHeader));
    CSVID_GetSequenceHeader( vid_handle, &VideoMode );

    //printf("GetVideoOriginalInfo OK\n");

    pOri_Size->Width                = VideoMode.w;
    pOri_Size->Height               = VideoMode.h;
    pOri_Size->FrameRate    = VideoMode.frame_rate;

    if(VideoSizeRange( pOri_Size->Width, 720 ) && VideoSizeRange( pOri_Size->Height, 576 ))
    {
        if( VideoRateRange(pOri_Size->FrameRate , 25))
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_PAL;
        }
        else
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_576P50;
        }
    }
    else if(VideoSizeRange( pOri_Size->Width, 720 ) && VideoSizeRange( pOri_Size->Height, 480 ))
    {
        if( VideoRateRange(pOri_Size->FrameRate , 30))
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_NTSC;
        }
        else
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_480P60;
        }
    }
    else if(VideoSizeRange( pOri_Size->Width, 1280 ) && VideoSizeRange( pOri_Size->Height, 720 ))
    {
        if( VideoRateRange(pOri_Size->FrameRate , 50))
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_720P50;
        }
        else
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_720P60;
        }
    }
    else if(VideoSizeRange( pOri_Size->Width, 1920 ) &&VideoSizeRange(  pOri_Size->Height,1080 ))
    {
        if( VideoRateRange(pOri_Size->FrameRate , 25))
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_1080I25;
        }
        else
        {
            pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_1080I30;
        }
    }
    else
    {
         pOri_Size->Video_Definition = eCS_AV_VIDEO_FORMAT_UNKNOWN;
         //return FALSE;
    }

    /*printf( "Current Video Format = %d FrameRate = %d \n",
                     pOri_Size->Video_Definition,
                     pOri_Size->FrameRate );
    printf( "              Width  = %d Height = %d\n",
                     pOri_Size->Width,
                     pOri_Size->Height );*/

     return eCS_AV_OK;
}

void AdjustVideoWindows(void)
{
	CSSQC_VIDEO_OUTPUTMODE 		output_mode;
	CSSQC_VIDEO_ASPECTMODE 		aspect_mode;

	tCS_DBU_AspectRatioMode		current_aspect_mode;
	tCS_DBU_VideoDefinition		current_output_mode;
	tCS_DBU_VideoAspectRatio	current_aspect_ratio;

/*
	if (IsNewProgram())
	{
		return;
	}
*/
	current_aspect_ratio = CS_DBU_GetVideoAspectRatio();
	current_aspect_mode = CS_DBU_GetAspectRatioMode();
	switch(current_aspect_mode)
	{
		case eCS_DBU_ARM_PANSCAN:
			aspect_mode = SQC_PAN_SCAN;
			break;
		case eCS_DBU_ARM_LETTER_BOX:
			aspect_mode = SQC_LETTERBOX;
			break;
		case eCS_DBU_ARM_COMBINED:
		default:
			aspect_mode = SQC_NORMAL;
			break;
	}

	current_output_mode = CS_DBU_GetVideoDefinition();
	switch(current_output_mode)
	{
		case eCS_DBU_DEFINITION_480I:
			output_mode = CSSQC_MODE_NTSC;
			break;
		case eCS_DBU_DEFINITION_576P:
			output_mode = CSSQC_MODE_576P;
			break;
		case eCS_DBU_DEFINITION_720P:
			output_mode = CSSQC_MODE_720P50;
			break;
		case eCS_DBU_DEFINITION_1080I:
			output_mode = CSSQC_MODE_1080I25;
			break;
		case eCS_DBU_DEFINITION_576I:
		default:
			output_mode = CSSQC_MODE_PAL;
			break;
	}

	//  printf("*******************aspect_ratio = %d, output_mode = %d, aspect_mode = %d**************************\n", aspect_ratio, output_mode, aspect_mode);
	//  printf("Set Aspec [%d], DisplayMode[%d], Video A/S[%d] \n", (U32)current_aspect_ratio, (U32)aspect_mode, (U32)current_video_aspect);
	SetVideoScalor(current_video_aspect, current_aspect_ratio, aspect_mode, video_window_rect);
}


BOOL					Audio_errFlag=FALSE;
U8						Audio_errTime=0;
CSVID_SequenceHeader	G_hdr;

CSVID_SequenceHeader MV_Get_Seq_Header(void)
{
	return G_hdr;
}

/* By KB Kim 2011.06.02 */
void VideoStatusAccessLock(void)
{
	if (VideoStatusAccessSem != NULL)
	{
		OsSemaphoreSignal(VideoStatusAccessSem);
	}
}

/* By KB Kim 2011.06.02 */
void VideoStausAccessRelease(void)
{
	if (VideoStatusAccessSem != NULL)
	{
		OsSemaphoreWait (VideoStatusAccessSem, TIMEOUT_FOREVER);
	}
}

/* By KB Kim 2011.06.02 */
void SetVideoUnderflowStatus(U32 status)
{
	VideoStatusAccessLock();
	VideoSatusUnderFlow = status;
	VideoStausAccessRelease();
}

/* By KB Kim 2011.06.02 */
void AddVideoUnderflowStatus(void)
{
	VideoStatusAccessLock();
	VideoSatusUnderFlow++;
	VideoStausAccessRelease();
}

/* By KB Kim 2011.06.02 */
U32 GetVideoUnderflowStatus(void)
{
	U32 status;

	VideoStatusAccessLock();
	status = VideoSatusUnderFlow;
	VideoStausAccessRelease();

	return status;
}

void MV_VideoData_UnderFlow_Callback(CSVID_HANDLE *handle )
{
	CSVID_HANDLE	Temp_warning;
	Temp_warning = handle;

	// printf("\n====== MV_VideoData_UnderFlow_Callback =======\n\n");
	/* By KB Kim 2011.06.02 */
	SetVideoUnderflowStatus(1);
	// BroadcastMessage (MSG_VIDEO_UNDERFLOW, 0, 0);
}

/*
void ClearNoVideoOn(void)
{
	VideoStatusAccessLock();
	NoVideoOn = 0;
	VideoStausAccessRelease();
}
*/

void AV_TrackTask(void)
{
	tCS_DBU_VideoDefinition		current_output_mode;
	int							No_Video_Count = 0;
	int 						VideoStatusCount = 0;;/* By KB Kim 2011.06.02 */
	U32                         previousH = 720;

	FILE* OscamAutoRestartVarmi;


	NoVideoOn = 0;

	while(1)
	{

		CSOS_DelayTaskMs(150);

		CSVID_GetSRCSequenceHeader(vid_handle, &G_hdr);

#if 0 /* By KB Kim 2011.06.02 */
		// printf("==== h : %d , w : %d =====\n", G_hdr.h , G_hdr.w );

		if ( G_hdr.h == 0 )
		{
			No_Video_Count++;
		}
		else
		{
			No_Video_Count = 0;
			if (noVideoEnable)
			{
				BroadcastMessage (MSG_NO_VIDEO, FALSE, 0);
				noVideoEnable = 0;
			}
/*
			if (previousH != G_hdr.h)
			{
				CS_AV_VideoBlank();
				previousH = G_hdr.h;
			}
*/
		}

		if ( No_Video_Count > 100 )
		{
			BroadcastMessage (MSG_NO_VIDEO, TRUE, 0);
			No_Video_Count = 0;
			noVideoEnable  = 1;
		}
#else
		/*
		if ( G_hdr.h > 0 )
		{
			if (previousH != G_hdr.h)
			{
				CS_AV_VideoBlank();
				previousH = G_hdr.h;
			}
		}
		*/

		//printf("No_video_count=%d VideoStatusCount = %d \n", No_Video_Count, VideoStatusCount);

		if (GetVideoUnderflowStatus())
		{
			No_Video_Count++;
			VideoStatusCount = 0;
			SetVideoUnderflowStatus(0);
			// printf("Video UnderFlow : %d \n", No_Video_Count);
		}
		else
		{
			if (No_Video_Count > 0)
			{
				VideoStatusCount++;
				// printf("Video Status : %d\n", VideoStatusCount);
				if (VideoStatusCount > MAX_VIDEO_STATUS_COUNT)
				{
					// printf("Video Started\n");
					if (NoVideoOn)
					{
						NoVideoOn = 0;
						BroadcastMessage (MSG_NO_VIDEO, FALSE, 0);
					}
					VideoStatusCount = 0;
					No_Video_Count = 0;
				}
			}
		}

		if ( No_Video_Count > MAX_NO_VIDEO_COUNT )
		{
			No_Video_Count = 1;
			if (NoVideoOn == 0)
			{
			    CS_AV_VideoBlank();
			    if (eCS_AV_ERROR == CS_AV_Play_IFrame2("/usr/work0/app/black.mpg"))
                                    printf("black.mpg error!!!\n");  //vdeneme sertac 16.05.2013--- ilk defa buraya ald?m*/
                printf("Black.mpg played...\n");

                //sleep(3);
                CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
                printf("Retune edildi!!\n");

				BroadcastMessage (MSG_NO_VIDEO, TRUE, 0);
				NoVideoOn  = 1;

				OscamAutoRestartVarmi = fopen("/usr/work1/OscamAutoRestart", "r");
				if(OscamAutoRestartVarmi){
				    system("/usr/work1/S70oscam restart");
                    printf("oscam yeniden basliyor...!\n");
                CS_MW_PlayServiceByIdx(CS_MW_GetCurrentPlayProgram(), RE_TUNNING);
                printf("Retune edildi!!\n");
				fclose(OscamAutoRestartVarmi);

				}



			}

		}
#endif
		//printf("%d\n", CS_OS_time_now());
#if 1
		CSOS_WaitSemaphore(sem_AVAccess);

		if(Audio_errFlag==TRUE)
		{
			CSAUD_EnableMute( aud_handle );
			Audio_errTime++;
			if(Audio_errTime>=8)
			Audio_errFlag=FALSE;
		}
		else
		{
			tCS_DBU_Status  status;

			status = CS_DBU_GetMuteStatus();

			if( status == eCS_DBU_ON )
			{
				CSAUD_EnableMute( aud_handle );
			}
			else
			{
				CSAUD_DisableMute( aud_handle );
			}
			Audio_errTime=0;
		}

		if(!video_set_show)
			CSVID_SetOutputAlpha( vid_handle, 0 );

		AdjustVideoWindows();
		/*
		{CSVID_SequenceHeader hdr;

		CSVID_GetSequenceHeader(vid_handle, &hdr);

		printf("------h=%d------w=%d---------\n",hdr.h,hdr.w);
		}
		*/
		CSOS_SignalSemaphore(sem_AVAccess);

		/* By KB Kim 2010.08.31 for Video Format Auto Mode */
		current_output_mode = CS_DBU_GetVideoDefinition();

		if (current_output_mode == eCS_DBU_DEFINITION_AUTOMATIC)
		{
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_AUTO);
		}

#endif
	}
}

void Sycro_call_back(CSVID_HANDLE *handle, signed char * temp)
{
	// printf("Sycro_call_back %d, time = %d\n", (* temp), CS_OS_time_now());
	if( video_set_show && (*temp==1) )
	{
		CS_AV_VideoUnblank();
		Audio_SetMuteStatus(CS_AV_Audio_GetMuteStatus());
	}
	else
	{
		CSVID_SyncNotify(vid_handle, Sycro_call_back, 5000*1000, 1);
	}
}


void AspectRatio_call_back(CSVID_HANDLE *handle, CSVID_ASPECTRATIO * ratio)
{
    //tCS_AV_VideoOriginalInfo    pOri_Size;
    // printf("AspectRatio = %d\n", * ratio);

    current_video_aspect = * ratio;

    CSOS_WaitSemaphore(sem_AVAccess);

    AdjustVideoWindows();

    CSOS_SignalSemaphore(sem_AVAccess);

    //GetVideoOriginalInfo( &pOri_Size );
    #if 0
    {
		CSSQC_VIDEO_ASPECTRATIO aspect_ratio = RATIO_4_3_ON_4_3;
		CSSQC_VIDEO_OUTPUTMODE output_mode;
		CSSQC_VIDEO_ASPECTMODE aspect_mode;

		tCS_DBU_AspectRatioMode         current_aspect_mode;
		tCS_DBU_VideoDefinition             current_output_mode;
		tCS_DBU_VideoAspectRatio        current_aspect_ratio;

		current_aspect_ratio = CS_DBU_GetVideoAspectRatio();

		switch(* ratio)
		{
			case    CSVID_16TO9:
				if(current_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)
				{
					aspect_ratio == RATIO_16_9_ON_16_9;
				}
				else
				{
					aspect_ratio == RATIO_16_9_ON_4_3;
				}
				break;
			case    CSVID_4TO3:
			default:
				if(current_aspect_ratio == eCS_DBU_ASPECT_RATIO_16_9)
				{
					aspect_ratio == RATIO_4_3_ON_16_9;
				}
				else
				{
					aspect_ratio == RATIO_4_3_ON_4_3;
				}
				break;
		}

		current_aspect_mode = CS_DBU_GetAspectRatioMode();
		switch(current_aspect_mode)
		{
			case eCS_DBU_ARM_PANSCAN:
				aspect_mode = SQC_PAN_SCAN;
				break;
			case eCS_DBU_ARM_LETTER_BOX:
				aspect_mode = SQC_LETTERBOX;
				break;
			case eCS_DBU_ARM_COMBINED:
			default:
				aspect_mode = SQC_NORMAL;
				break;
		}

		current_output_mode = CS_DBU_GetVideoDefinition();
		switch(current_output_mode)
		{
			case eCS_DBU_DEFINITION_480P:
				output_mode = CSSQC_MODE_480P;
				break;
			case eCS_DBU_DEFINITION_576P:
				output_mode = CSSQC_MODE_576P;
				break;
			case eCS_DBU_DEFINITION_720P:
				output_mode = CSSQC_MODE_720P50;
				break;
			case eCS_DBU_DEFINITION_1080I:
				output_mode = CSSQC_MODE_1080I25;
				break;
			case eCS_DBU_DEFINITION_576I:
			default:
				output_mode = CSSQC_MODE_PAL;
				break;
		}

		if( vid_handle != NULL )
			CSVID_SetOutputAlpha( vid_handle, 0xff );

		printf("*******************aspect_ratio = %d, output_mode = %d, aspect_mode = %d**************************\n", aspect_ratio, output_mode, aspect_mode);

		CSSQC_VIDEO_SysSet(vid_handle, df_handle, tve_handle, aspect_ratio, output_mode, aspect_mode);
	}
    #endif
}

BOOL MV_AvSetCw(U8 dmx, U8 mode, U8 *evenKey, U8 *oddKey)
{
	DesKeyData_t 	desKey;
	CSDEMUX_HANDLE 	currentHandle;
	/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
	CSDEMUX_HANDLE 	pes1;
	CSDEMUX_HANDLE 	pes2;

	//printf("Mode : 0x%02X  == dmx : %d\n", mode, dmx);

	/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
	pes1 = CSDEMUX_UNVALID_HANDLE;
	pes2 = CSDEMUX_UNVALID_HANDLE;

	if (mode & 0x0F)
	{
		currentHandle = xport_pid_filter1_handle;
	}
	else
	{
		currentHandle = xport_pid_filter0_handle;

		/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
		pes1 = CS_GetPesDemuxInfo(8); /* reserved slot for Subtitle and Teletext are 8, 9 */
		pes2 = CS_GetPesDemuxInfo(9); /* reserved slot for Subtitle and Teletext are 8, 9 */
	}

	if (mode & 0x20)
	{
		desKey.key[0] = Array2Word(oddKey + 4, 4);
		desKey.key[1] = Array2Word(oddKey, 4);
		desKey.key_len = 2;
		// printf("OddKey : 0x%08X , 0x%08x\n", desKey.key[0], desKey.key[1]);
		CSDEMUX_PIDFT_SetDESOddKey(currentHandle, (unsigned char *)&desKey);

		/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
		if (pes1 != CSDEMUX_UNVALID_HANDLE)
		{
			CSDEMUX_PIDFT_SetDESOddKey(pes1, (unsigned char *)&desKey);
			// printf("Set Slot[8] Odd Key\n");
		}
		if (pes2 != CSDEMUX_UNVALID_HANDLE)
		{
			CSDEMUX_PIDFT_SetDESOddKey(pes2, (unsigned char *)&desKey);
			// printf("Set Slot[9] Odd Key\n");
		}
	}

	if (mode & 0x10)
	{
		desKey.key[0] = Array2Word(evenKey + 4, 4);
		desKey.key[1] = Array2Word(evenKey, 4);
		desKey.key_len = 2;
		//printf("EvenKey : 0x%08X , 0x%08x\n", desKey.key[0], desKey.key[1]);
		CSDEMUX_PIDFT_SetDESEvenKey(currentHandle, (unsigned char *)&desKey);

		/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
		if (pes1 != CSDEMUX_UNVALID_HANDLE)
		{
			CSDEMUX_PIDFT_SetDESEvenKey(pes1, (unsigned char *)&desKey);
			// printf("Set Slot[8] Even Key\n");
		}
		if (pes2 != CSDEMUX_UNVALID_HANDLE)
		{
			CSDEMUX_PIDFT_SetDESEvenKey(pes2, (unsigned char *)&desKey);
			// printf("Set Slot[9] Even Key\n");
		}
	}

	CSDEMUX_PIDFT_EnableDES(currentHandle);

	/* For Scambled SubTitle and Teletext by KB Kim 2011.03.26 */
	if (pes1 != CSDEMUX_UNVALID_HANDLE)
	{
		CSDEMUX_PIDFT_EnableDES(pes1);
		// printf("Slot[8] DES Key enable\n");
	}
	if (pes2 != CSDEMUX_UNVALID_HANDLE)
	{
		CSDEMUX_PIDFT_EnableDES(pes2);
		// printf("Slot[9] DES Key enable\n");
	}

	return TRUE;   // Just Warning message
}

BOOL CS_AV_Init2(void){

CSOSD_Config osd_config = {
					OSD_MODE_1080I25,
					OSD_COLOR_DEPTH_32,
					OSD_COLOR_FORMAT_ARGB8888
				};
	sem_AVAccess  = CSOS_CreateSemaphoreFifo (NULL, 1 );

	/* By KB Kim 2011.06.02 */
	VideoSatusUnderFlow = 0;
	if (OsCreateSemaphore(&VideoStatusAccessSem, 1) != OS_NO_ERROR)
	{
		printf("Cannot Create VideoStatusAccessSem\n");
	}



	tve_handle = CSTVOUT_Open(TVOUT_CHANNEL0);

	if( tve_handle == NULL )
	{
		printf("CSTVOUT_Open error\n");
		return FALSE;
	}

	CSTVOUT_Enable(tve_handle);

	osd_handle = CSOSD_Open(OSD_LAYER_0);

	if( osd_handle == NULL )
	{
		printf("###########################   <<<<<   CSOSD_Open error\n");
		return FALSE;
	}
	printf("\n\n########### OSD_INIT ############\n\n");

	CSOSD_SetConfig(osd_handle, &osd_config);
	CSOSD_Enable( osd_handle );

	vid_handle = CSVID_Open(VID_DEV_0);

	if( vid_handle == NULL )
	{
		printf("CSVID_Open error\n");
		return FALSE;
	}

	aud_handle = CSAUD_Open(AUD_DEV_0);

	if( aud_handle == NULL )
	{
		printf("CSAUD_Open error\n");
		return FALSE;
	}



	xport_pid_filter0_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0);

	if( xport_pid_filter0_handle == CSDEMUX_UNVALID_HANDLE)
	{
	    printf("CSDEMUX_PIDFT_ID0 error\n");
		return FALSE;
	}

	if (CSDEMUX_PIDFT_MallocDES(xport_pid_filter0_handle) == CSAPI_FAILED)
	{
		printf("CSDEMUX_PIDFT_MallocDES for Video error\n");
		// return FALSE;
	}


	xport_pid_filter1_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID1);

	if( xport_pid_filter1_handle == CSDEMUX_UNVALID_HANDLE)
	{
	    printf("CSDEMUX_PIDFT_ID1 error\n");
		return FALSE;
	}

	if (CSDEMUX_PIDFT_MallocDES(xport_pid_filter1_handle) == CSAPI_FAILED)
	{
		printf("CSDEMUX_PIDFT_MallocDES for Audio error\n");
		// return FALSE;
	}


	xport_vidout_handle = CSDEMUX_VID_Open(DEMUX_VIDOUT_ID0);

	if( xport_vidout_handle == CSDEMUX_UNVALID_HANDLE )
	{
	    printf("DEMUX_VIDOUT_ID0 error\n");
		return FALSE;
	}



	xport_audout_handle = CSDEMUX_AUD_Open(DEMUX_AUDOUT_ID0);

	if( xport_audout_handle == CSDEMUX_UNVALID_HANDLE )
	{
	    printf("DEMUX_AUDOUT_ID0 error\n");
		return FALSE;
	}

	CSVID_AspectRatioChangeNotify(vid_handle, AspectRatio_call_back, 1);

#if 1
	if (CSOS_CreateTask(	AV_TrackTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						AV_TRACK_STACK_SIZE,				/* size of stack in bytes */
						AV_TRACK_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&AV_TRACK_TASK_HANDLE,			/* return thread handle */
						&AV_TRACK_TASK_DESC, 			/* space to store thread data */
						AV_TRACK_TASK_PRIORITY,
						"av_tracking", 				/* name of thread */
						AV_TRACK_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the av_tracking \n" );
		return(FALSE);
	}

	CSOS_StartTask(AV_TRACK_TASK_HANDLE);
#endif
    printf("end of cs_av_init2!!!\n");
	return TRUE;




}


BOOL CS_AV_Init(void)
{
	CSOSD_Config osd_config = {
					OSD_MODE_1080I25,
					OSD_COLOR_DEPTH_32,
					OSD_COLOR_FORMAT_ARGB8888
				};
	sem_AVAccess  = CSOS_CreateSemaphoreFifo (NULL, 1 );

	/* By KB Kim 2011.06.02 */
	VideoSatusUnderFlow = 0;
	if (OsCreateSemaphore(&VideoStatusAccessSem, 1) != OS_NO_ERROR)
	{
		printf("Cannot Create VideoStatusAccessSem\n");
	}

	/*if((CS_AV_MsgQid = CSOS_CreateMessageQueue("/AV_TRACK_MsgQid",sizeof(tCS_AV_Msg_t), kCS_AV_MAX_MSG )) == NULL)
	{
		printf("create CS_AV_MsgQid error\n");
		return(eCS_AV_ERROR);
	}*/

	tve_handle = CSTVOUT_Open(TVOUT_CHANNEL0);

	if( tve_handle == NULL )
	{
		printf("CSTVOUT_Open error\n");
		return FALSE;
	}

	CSTVOUT_Enable(tve_handle);

	osd_handle = CSOSD_Open(OSD_LAYER_0);

	if( osd_handle == NULL )
	{
		printf("###########################   <<<<<   CSOSD_Open error\n");
		return FALSE;
	}
	printf("\n\n########### OSD_INIT ############\n\n");

	CSOSD_SetConfig(osd_handle, &osd_config);
	CSOSD_Enable( osd_handle );

	vid_handle = CSVID_Open(VID_DEV_0);

	if( vid_handle == NULL )
	{
		printf("CSVID_Open error\n");
		return FALSE;
	}

	aud_handle = CSAUD_Open(AUD_DEV_0);

	if( aud_handle == NULL )
	{
		printf("CSAUD_Open error\n");
		return FALSE;
	}

	/*df_handle = CSDF_Open();
	if( df_handle == NULL )
	{
	return FALSE;
	}

	CSDF_Enable(df_handle);*/
	/*if ( CSAPI_FAILED == CSDEMUX_PIDFT_Close(xport_pid_filter0_handle))
        printf("CSDEMUX_PIDFT_Close(DEMUX_PIDFT_ID0) Failed!!!\n");*/

	xport_pid_filter0_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0);

	if( xport_pid_filter0_handle == CSDEMUX_UNVALID_HANDLE)
	{
	    printf("CSDEMUX_PIDFT_ID0 error\n");
		return FALSE;
	}

	if (CSDEMUX_PIDFT_MallocDES(xport_pid_filter0_handle) == CSAPI_FAILED)
	{
		printf("CSDEMUX_PIDFT_MallocDES for Video error\n");
		// return FALSE;
	}

	/*if ( CSAPI_FAILED == CSDEMUX_PIDFT_Close(xport_pid_filter1_handle))
        printf("CSDEMUX_PIDFT_Close(DEMUX_PIDFT_ID1) Failed!!!\n");*/

	xport_pid_filter1_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID1);

	if( xport_pid_filter1_handle == CSDEMUX_UNVALID_HANDLE)
	{
	    printf("CSDEMUX_PIDFT_ID1 error\n");
		return FALSE;
	}

	if (CSDEMUX_PIDFT_MallocDES(xport_pid_filter1_handle) == CSAPI_FAILED)
	{
		printf("CSDEMUX_PIDFT_MallocDES for Audio error\n");
		// return FALSE;
	}

	/*if ( CSAPI_FAILED == CSDEMUX_VID_Close(xport_vidout_handle))
        printf("CSDEMUX_VID_Close(xport_vidout_handle) Failed!!!\n");*/

	xport_vidout_handle = CSDEMUX_VID_Open(DEMUX_VIDOUT_ID0);

	if( xport_vidout_handle == CSDEMUX_UNVALID_HANDLE )
	{
	    printf("DEMUX_VIDOUT_ID0 error\n");
		return FALSE;
	}

	/*if ( CSAPI_FAILED == CSDEMUX_AUD_Close(xport_audout_handle))
        printf("CSDEMUX_AUD_Close(xport_audout_handle) Failed!!!\n");*/

	xport_audout_handle = CSDEMUX_AUD_Open(DEMUX_AUDOUT_ID0);

	if( xport_audout_handle == CSDEMUX_UNVALID_HANDLE )
	{
	    printf("DEMUX_AUDOUT_ID0 error\n");
		return FALSE;
	}

	CSVID_AspectRatioChangeNotify(vid_handle, AspectRatio_call_back, 1);

#if 1
	if (CSOS_CreateTask(	AV_TrackTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						AV_TRACK_STACK_SIZE,				/* size of stack in bytes */
						AV_TRACK_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&AV_TRACK_TASK_HANDLE,			/* return thread handle */
						&AV_TRACK_TASK_DESC, 			/* space to store thread data */
						AV_TRACK_TASK_PRIORITY,
						"av_tracking", 				/* name of thread */
						AV_TRACK_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the av_tracking \n" );
		return(FALSE);
	}

	CSOS_StartTask(AV_TRACK_TASK_HANDLE);
#endif
    printf("end of cs_av_init!!!\n");
	return TRUE;
}

tCS_AV_Error CS_AV_Close(void)
{

        if ( CSAPI_FAILED == CSDEMUX_PIDFT_Close(xport_pid_filter0_handle))
        printf("CSDEMUX_PIDFT_Close(DEMUX_PIDFT_ID0) Failed!!!\n");

        if ( CSAPI_FAILED == CSDEMUX_PIDFT_Close(xport_pid_filter1_handle))
        printf("CSDEMUX_PIDFT_Close(DEMUX_PIDFT_ID1) Failed!!!\n");

        if ( CSAPI_FAILED == CSDEMUX_VID_Close(xport_vidout_handle))
        printf("CSDEMUX_VID_Close(xport_vidout_handle) Failed!!!\n");

        if ( CSAPI_FAILED == CSDEMUX_AUD_Close(xport_audout_handle))
        printf("CSDEMUX_AUD_Close(xport_audout_handle) Failed!!!\n");

        if( vid_handle != NULL )
        {
            if ( CSAPI_FAILED == CSVID_Close( vid_handle ))
            printf("CSVID_Close Failed!!!\n");
        }
        if( aud_handle != NULL )
        {
            if ( CSAPI_FAILED == CSAUD_Close( aud_handle ))
            printf("CSAUD_Close Failed!!!\n");
        }
        if( tve_handle != NULL )
        {
            if ( CSAPI_FAILED == CSTVOUT_Close( tve_handle ))
            printf("CSTVOUT_Close Failed!!!\n");
        }
        if( osd_handle != NULL )
        {
            if ( CSAPI_FAILED == CSOSD_Close  ( osd_handle ))
            printf("CSOSD_Close Failed!!!\n");
        }
    return eCS_AV_OK;
}


tCS_AV_Error CS_Video_Close(void)
{
        if( vid_handle != NULL )
        {
            if ( CSAPI_FAILED == CSVID_Close  ( vid_handle )){
                printf("CS_Video_Close(void) Failed!!!\n");
                return eCS_AV_ERROR;
            }
            else {
                printf("CS_Video_Close(void) Succesfull!!!\n");
                return eCS_AV_OK;
            }

        }

        else{
        printf("vid_handle == NULL, Video already closed!!\n");
        return eCS_AV_OK;
        }

}

tCS_AV_Error CS_Audio_Close(void)
{
        if( aud_handle != NULL )
        {
            if ( CSAPI_FAILED == CSAUD_Close  ( aud_handle )){
                printf("CSAUD_Close  ( aud_handle ) Failed!!!\n");
                return eCS_AV_ERROR;
            }
            else {
                printf("CSAUD_Close  ( aud_handle ) Succesfull!!!\n");
                return eCS_AV_OK;
            }

        }

        else{
        printf("aud_handle == NULL, Audio already closed!!\n");
        return eCS_AV_OK;
        }

}

tCS_AV_Error CS_TVE_Close(void)
{
        if( tve_handle != NULL )
        {
            if ( CSAPI_FAILED == CSTVOUT_Close  ( tve_handle )){
                printf("CSTVOUTClose  ( tve_handle ) Failed!!!\n");
                return eCS_AV_ERROR;
            }
            else {
                printf("CSTVOUT_Close  ( tve_handle ) Succesfull!!!\n");
                return eCS_AV_OK;
            }

        }

        else{
        printf("tve_handle == NULL, TVOUT already closed!!\n");
        return eCS_AV_OK;
        }

}

tCS_AV_Error CS_OSD_Close(void)
{
        if( osd_handle != NULL )
        {
            if ( CSAPI_FAILED == CSOSD_Close  ( osd_handle )){
                printf("CSOSD_Close  ( osd_handle ) Failed!!!\n");
                return eCS_AV_ERROR;
            }
            else {
                printf("CSOSD_Close  ( osd_handle ) Succesfull!!!\n");
                return eCS_AV_OK;
            }

        }

        else{
        printf("osd_handle == NULL, OSD already closed!!\n");
        return eCS_AV_OK;
        }

}

tCS_AV_Error CS_AV_Audio_SetMuteStatus( BOOL enable )
{
        tCS_AV_Error    err = eCS_AV_OK;

        CSOS_WaitSemaphore(sem_AVAccess);

        if( enable )
        {
            CS_DBU_SetMuteStatus( eCS_DBU_ON );
        }
        else
        {
            CS_DBU_SetMuteStatus( eCS_DBU_OFF );
        }

        err = Audio_SetMuteStatus(enable);

        CSOS_SignalSemaphore(sem_AVAccess);

        return(err);
}


BOOL CS_AV_Audio_GetMuteStatus(void)
{
    tCS_DBU_Status  status;

    status = CS_DBU_GetMuteStatus();
    if( status == eCS_DBU_ON )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

tCS_AV_Error AV_AudioSetVolume( U8 Volume )
{
    tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT 	csResult = CSAPI_SUCCEED;
    CSAUD_Volume 	aud_Volume;
    U8      		set_value = 0;
    float   		temp;

    if( aud_handle == NULL )
    {
        return eCS_AV_ERROR;
    }

    //set_value = Volume * 3 / 2;

    temp = ( MAX_VOLUME_VALUE - MIN_VOLUME_VALUE )
		   * log10f( Volume + 1 ) / log10f( kCS_DBU_MAX_VOLUME + 1 );
    /*temp = ( MAX_VOLUME_VALUE - MIN_VOLUME_VALUE )
		   * ( Volume + 1 ) / ( kCS_DBU_MAX_VOLUME + 1 );*/
    set_value = temp;

    aud_Volume.front_left  = set_value;
    aud_Volume.front_right = set_value;
    aud_Volume.rear_left   = set_value;
    aud_Volume.rear_right  = set_value;
    aud_Volume.center      = set_value;
    aud_Volume.lfe         = set_value;
    csResult = CSAUD_SetVolume( aud_handle, &aud_Volume );

    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    return(err);

}

tCS_AV_Error CS_AV_AudioSetVolume( U8 Volume )
{
    tCS_AV_Error    err = eCS_AV_OK;

	CSOS_WaitSemaphore(sem_AVAccess);

    //CS_DBU_SetVolume( Volume );
    err = AV_AudioSetVolume(Volume);

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);
}


U8 CS_AV_AudioGetVolume(void)
{
    U8 csDBVolume = 0;

    csDBVolume = (U8)CS_DBU_GetVolume();

    return csDBVolume;
}

tCS_AV_Error AV_AudioSetStereoMode( tCS_AV_StereoMode StereoMode )
{
    tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;
    CSAUD_PCM_CHANNEL audStereoMode;

    if( aud_handle == NULL )
        {
                return eCS_AV_ERROR;
        }

    switch( StereoMode )
    {
        case eCS_AV_STEREO_MODE_STEREO:
            audStereoMode = AUD_PCM_STEREO;
            break;

        case eCS_AV_STEREO_MODE_LEFT:
            audStereoMode = AUD_PCM_LEFT_MONO;
            break;

        case eCS_AV_STEREO_MODE_RIGHT:
            audStereoMode = AUD_PCM_RIGHT_MONO;
            break;

        default:
            audStereoMode = AUD_PCM_STEREO;
            break;
    }

    csResult = CSAUD_SetOutputChannel( aud_handle, audStereoMode );
    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    return(err);

}


tCS_AV_Error CS_AV_AudioSetStereoMode( tCS_AV_StereoMode StereoMode )
{
    tCS_AV_Error    err = eCS_AV_OK;

    CSOS_WaitSemaphore(sem_AVAccess);
    err = AV_AudioSetStereoMode(StereoMode);

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);

}

static void Audio_Notify(CSAUD_HANDLE *Handle,CSAUD_ERROR_THRESHOLD Value)
{
 	printf("Audio frame error:%d, %d\n", Value, (int)Handle);
	Audio_errFlag=TRUE;
	Audio_errTime=0;
}

void Audio_ErrMoniter(void)
{
	CSAUD_ErrNotify(aud_handle,Audio_Notify,LEVEL2,1);
}

tCS_AV_Error CS_AV_AudioFreeze(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;

	CSOS_WaitSemaphore(sem_AVAccess);
	if( aud_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

    csResult = CSAUD_Pause( aud_handle );
    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);
}

tCS_AV_Error CS_AV_VideoFreeze(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;

	CSOS_WaitSemaphore(sem_AVAccess);
	if( vid_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

    // csResult = CSVID_Pause( vid_handle );
    csResult = CSVID_Freeze( vid_handle );
	// csResult = CSVID_Stop(vid_handle);

    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);
}

tCS_AV_Error CS_AV_AudioUnfreeze(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
	CSAPI_RESULT csResult = CSAPI_SUCCEED;

	CSOS_WaitSemaphore(sem_AVAccess);
	if( aud_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

    csResult = CSAUD_Resume( aud_handle );
    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}


tCS_AV_Error CS_AV_VideoUnfreeze(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;

	CSOS_WaitSemaphore(sem_AVAccess);
	if( vid_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

	// printf("CS_AV_VideoUnfreeze\n");

    csResult = CSVID_Resume( vid_handle );
    if( csResult != CSAPI_SUCCEED )
    {
		printf("CS_AV_VideoUnfreeze Error:%d \n", csResult);
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}

tCS_AV_Error CS_AV_VideoBlank(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;

	// printf("================>   CS_AV_VideoBlank!!   <================\n");
	CSOS_WaitSemaphore(sem_AVAccess);
	if( vid_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

    //CSVID_DisableTV(vid_handle);
    csResult = CSVID_SetOutputAlpha( vid_handle, 0 );
    if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);
}

tCS_AV_Error CS_AV_VideoUnblank(void)
{
	tCS_AV_Error    err = eCS_AV_OK;
	CSAPI_RESULT csResult = CSAPI_SUCCEED;

	// printf("================>  CS_AV_VideoUnBlank!!  <================\n");
	CSOS_WaitSemaphore(sem_AVAccess);
	if( vid_handle == NULL )
    {
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

    csResult = CSVID_SetOutputAlpha( vid_handle, 0xff );
	if( csResult != CSAPI_SUCCEED )
    {
        err = eCS_AV_ERROR;
    }

    CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}

tCS_AV_Error CS_AV_SetOSDAlpha( int Alpha )
{
	tCS_AV_Error    err = eCS_AV_OK;
	CSAPI_RESULT csResult = CSAPI_SUCCEED;
#if 1
	//CSOS_WaitSemaphore(sem_AVAccess);
	if( osd_handle == NULL )
    {
		//CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
    }

	csResult = CSOSD_SetAlpha( osd_handle, Alpha );
	if( csResult != CSAPI_SUCCEED )
	{
		err = eCS_AV_ERROR;
	}
#endif
    //CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}

tCS_AV_Error CS_AV_GetVideoOriginalInfo( tCS_AV_VideoOriginalInfo* pOri_Size )
{
	tCS_AV_Error    err = eCS_AV_OK;

    CSOS_WaitSemaphore(sem_AVAccess);

    err = GetVideoOriginalInfo(pOri_Size);

    CSOS_SignalSemaphore(sem_AVAccess);

    return(err);
}

void ClearOSD(void)
{
	CSOSD_Flip(osd_handle);
}

void AV_SetOutputPostion(const CSVID_Rect * const src, const CSVID_Rect * const dst)

{

	CSVID_SetOutputPostion(vid_handle, src, dst);
}

tCS_AV_Error CS_AV_VideoScalor( tCS_AV_VideoRect * vid_rect)
{
	tCS_AV_Error    err = eCS_AV_OK;
	//printf("CS_AV_VideoScalor\n");

	CSOS_WaitSemaphore(sem_AVAccess);

	if(vid_rect == NULL)
	{
		video_window_rect = NULL;
	}
	else
	{
		rect_buf.left		= vid_rect->x;
		rect_buf.right		= vid_rect->x + vid_rect->w;
		rect_buf.top		= vid_rect->y;
		rect_buf.bottom		= vid_rect->y + vid_rect->h;

		video_window_rect	= &rect_buf;
	}

	AdjustVideoWindows();

	CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}

/* By KB Kim 2010.08.31 for RGB Control */
tCS_AV_Error CS_AV_SetTVOutput( void )
{
	tCS_DBU_VideoOutput outputMode;

	CSOS_WaitSemaphore(sem_AVAccess);
	outputMode = CS_DBU_GetVideoOutput();

	if( tve_handle == NULL )
	{
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
	}

	if (CurrentTvOutput != outputMode)
	{
		switch(outputMode)
		{
		case eCS_DBU_OUTPUT_RGB   :
			// printf("-------------------> Set RGB Mode !\n");
			// CSTVOUT_SetOutput(tve_handle, OUTPUT_MODE_RGB);
			CSTVOUT_SetCompChannel(tve_handle, CSVOUT_COMP_RGB);
			ScartFbOnOff(1);
			break;
#if 0
		case eCS_DBU_OUTPUT_YC    :
			CSTVOUT_SetCompChannel(tve_handle, CSVOUT_COMP_YUV);
			ScartFbOnOff(0);
			break;
#endif
		case eCS_DBU_OUTPUT_YPBPR :
		default                   :
			// printf("-------------------> Set YPbPr Mode !\n");
			outputMode = eCS_DBU_OUTPUT_YPBPR;
			CSTVOUT_SetOutput(tve_handle, OUTPUT_MODE_YPBPR);
			// CSTVOUT_SetCompChannel(tve_handle, CSVOUT_COMP_YUV);
			ScartFbOnOff(0);
			break;
		}
		CurrentTvOutput = outputMode;
	}
	CSOS_SignalSemaphore(sem_AVAccess);

	return eCS_AV_OK;
}

/* By KB Kim 2010.08.31 for Video Format Auto Mode */
tCS_AV_Error CS_AV_SetTVOutDefinition( tCS_AV_VideoDefinition vFormat )
{
	tCS_AV_Error                  err = eCS_AV_OK;
	CSVID_SequenceHeader 	      hdr;
	tCS_AV_VideoDefinition definition;

	CSOS_WaitSemaphore(sem_AVAccess);
	if( tve_handle == NULL )
	{
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
	}

	// memset(&lastsrc,0,sizeof(lastsrc));
	if (vFormat == eCS_AV_VIDEO_FORMAT_AUTO)
	{
		hdr = MV_Get_Seq_Header();

		if (hdr.h <= 240)
		{
			definition = CurrentDefinition;
		}
		else if (hdr.h <= 480)
		{
			definition = eCS_AV_VIDEO_FORMAT_NTSC;
		}
		else if (hdr.h <= 576)
		{
			definition = eCS_AV_VIDEO_FORMAT_PAL;
		}
		else if (hdr.h <= 720)
		{
			if (hdr.frame_rate >= 29)
			{
				definition = eCS_AV_VIDEO_FORMAT_720P60;
			}
			else
			{
				definition = eCS_AV_VIDEO_FORMAT_720P50;
			}
		}
		else
		{
			if (hdr.frame_rate >= 29)
			{
				definition = eCS_AV_VIDEO_FORMAT_1080I30;
			}
			else
			{
				definition = eCS_AV_VIDEO_FORMAT_1080I25;
			}
		}
	}
	else
	{
		definition = vFormat;
	}

	if (CurrentDefinition == definition)
	{
		/* No change */

		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_OK;
	}

	/*
	if (vFormat == eCS_AV_VIDEO_FORMAT_AUTO)
	{
		printf("Picture H[%d] W[%d], Rate[%d]\n", hdr.h, hdr.w, hdr.frame_rate);
	}
	*/

	memset(&lastsrc,0,sizeof(lastsrc));

#ifdef USE_HDMI_CAT6611
	//usleep(10000);
	switch( definition )
	{
		case eCS_AV_VIDEO_FORMAT_NTSC:
			Cat6611_SetOutputMode(SYS_HDMI_480I);
			break;

		case eCS_AV_VIDEO_FORMAT_PAL:
			Cat6611_SetOutputMode(SYS_HDMI_576I);
			break;

		case eCS_AV_VIDEO_FORMAT_576P50:
			//       case eCS_AV_VIDEO_FORMAT_576P60:
			Cat6611_SetOutputMode(SYS_HDMI_576P);
			break;

		case eCS_AV_VIDEO_FORMAT_720P50:
			Cat6611_SetOutputMode(SYS_HDMI_720P50);
			break;
		case eCS_AV_VIDEO_FORMAT_720P60:
			Cat6611_SetOutputMode(SYS_HDMI_720P60);
			break;

		case eCS_AV_VIDEO_FORMAT_1080I25:
			Cat6611_SetOutputMode(SYS_HDMI_1080I25);
			break;
		case eCS_AV_VIDEO_FORMAT_1080I30:
			Cat6611_SetOutputMode(SYS_HDMI_1080I30);
			break;

		default:
			Cat6611_SetOutputMode(SYS_HDMI_576I);
			break;
	}
    // printf("CAT6611 Setup definition =%d!!!\n",definition);
#endif
	switch( definition )
	{
		case eCS_AV_VIDEO_FORMAT_NTSC:        /* NTSC 480I 60Hz */
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_480I );
			//CSTVOUT_SetMode( tve_handle, TVOUT_SECAM);
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,1);
			break;

		case eCS_AV_VIDEO_FORMAT_PAL:         /* PAL 576I 50Hz*/
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_576I );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,1);
#if 0
			//CSTVOUT_SetCompChannel(tve_handle,CSVOUT_COMP_RGB);
			CSTVOUT_SetCompChannel(tve_handle,CSVOUT_COMP_YVU);
			{
			CSTVOUT_WSSINFO  Info;
			Info.ARatio=WSS_AR_14TO9_FULL;
			Info.WssType=TVE_VBI_WSS;

			CSTVOUT_WSS_SetFormat(tve_handle,VBI_WSS_PAL);
			CSTVOUT_WSS_SetInfo(tve_handle,&Info);
			CSTVOUT_WSS_Ctrl(tve_handle,1);
			}
#endif

			break;

		case eCS_AV_VIDEO_FORMAT_480P60:      /* 480p 60HZ*/
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_480P );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			//CSTVOUT_SetMode( tve_handle, TVOUT_SECAM);
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,1);
			break;

		case eCS_AV_VIDEO_FORMAT_576P50:      /* 576P 50Hz */
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_576P );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			break;

		case eCS_AV_VIDEO_FORMAT_720P50:      /* 720P 50Hz */
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_720P50 );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			break;

		case eCS_AV_VIDEO_FORMAT_720P60:      /* 720P 60Hz */
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_720P60 );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			break;

		case eCS_AV_VIDEO_FORMAT_1080I25:     /* 1080I 25HZ*/
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_1080I25);
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			break;

		case eCS_AV_VIDEO_FORMAT_1080I30:     /* 1080I 30HZ*/
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_1080I30 );
			//CSTVOUT_DAC0_SetPathComposition(tve_handle,0);
			break;

		case eCS_AV_VIDEO_FORMAT_AUTO:
			break;
		case eCS_AV_VIDEO_FORMAT_UNKNOWN:
		default:
			CSTVOUT_SetMode( tve_handle, TVOUT_MODE_576I );
			break;

	}

	/* By KB Kim : 2010_08_31 for RGB Control */
	CurrentDefinition = definition;

	CSOSD_Flip(osd_handle);

	CSOS_SignalSemaphore(sem_AVAccess);

	if (CurrentTvOutput == eCS_DBU_OUTPUT_RGB)
	{
		CurrentTvOutput = eCS_DBU_OUTPUT_MAX;
		CS_AV_SetTVOutput();
	}

	return(err);

}

BOOL CS_AV_SetVBIPage(CSVOUT_TxtPage_t *pPage)
{

	CSTVOUT_TXT_SetInfo(tve_handle,pPage);
	return TRUE;
}

BOOL CS_AV_SetVBI_Ctrl(char Enable)
{
	CSTVOUT_TXT_Ctrl(tve_handle,Enable);
	return TRUE;
}

BOOL CS_AV_SetVBIFmt(CSVOUT_TxtStandard_t TxtStd)
{
	CSTVOUT_TXT_SetFormat(tve_handle,TxtStd);
	return TRUE;
}

BOOL CS_AV_VideoSwitchCVBS( BOOL CVBSEnable )
{
     return  CVBSEnable;
}

tCS_AV_Error CS_AV_ProgramStop(void)
{
     tCS_AV_Error    err = eCS_AV_OK;

     CSOS_WaitSemaphore(sem_AVAccess);
    // video_set_show = FALSE;
    if( vid_handle != NULL )
    {
        CSVID_Stop(vid_handle);
        //CSVID_Close(vid_handle);
        //vid_handle = NULL;
    }

    if( aud_handle != NULL )
    {
        CSAUD_Stop(aud_handle);
        //CSAUD_Close(aud_handle);
        //aud_handle = NULL;
    }

    if( xport_pid_filter0_handle != CSDEMUX_UNVALID_HANDLE )
        {
            CSDEMUX_PIDFT_Disable(xport_pid_filter0_handle);
        }

    if( xport_pid_filter1_handle != CSDEMUX_UNVALID_HANDLE )
        {
            CSDEMUX_PIDFT_Disable(xport_pid_filter1_handle);
        }
#if 0

    if( xport_vidout_handle != CSDEMUX_UNVALID_HANDLE )
    {
       CSDEMUX_VID_Disable(xport_vidout_handle);
       //CSDEMUX_VID_Close(xport_vidout_handle);
    }

    if( xport_audout_handle != CSDEMUX_UNVALID_HANDLE )
    {
       CSDEMUX_AUD_Disable(xport_audout_handle);
       //CSDEMUX_AUD_Close(xport_audout_handle);
    }

     DB_DemuxResetChannel();
#endif

    CSOS_SignalSemaphore(sem_AVAccess);

	return(err);

}

tCS_AV_Error Play_Video(tCS_AV_PlayParams ProgramInfo)
{
    tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT    sdk_err;

    if(( vid_handle == NULL )||( xport_pid_filter0_handle == CSDEMUX_UNVALID_HANDLE )||( xport_vidout_handle == CSDEMUX_UNVALID_HANDLE ))
	{
		return eCS_AV_ERROR;
	}

   // if( ProgramInfo.Video_PID == kDB_DEMUX_INVAILD_PID && ProgramInfo.Audio_PID != kDB_DEMUX_INVAILD_PID )
	if( ProgramInfo.Video_PID == kDB_DEMUX_INVAILD_PID )
    {
		if ( ProgramInfo.Audio_PID == kDB_DEMUX_INVAILD_PID )
	       	err = Audio_SetMuteStatus(CS_AV_Audio_GetMuteStatus());

		// printf("== Audio_SetMuteStatus : %d , CS_AV_Audio_GetMuteStatus() : %d == \n", err, CS_AV_Audio_GetMuteStatus());
		return eCS_AV_ERROR;
    }

	/*  kb : 20100403
    CSVID_SetOutputAlpha( vid_handle, 0 );
    CSVID_Stop(vid_handle);

    CSDEMUX_PIDFT_Disable(xport_pid_filter0_handle);
    */

    CSDEMUX_VID_Disable(xport_vidout_handle);

    CSDEMUX_PIDFT_SetChannel(xport_pid_filter0_handle,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(xport_pid_filter0_handle,ProgramInfo.Video_PID);

    DB_DemuxSetVidPid(xport_pid_filter0_handle, ProgramInfo.Video_PID);

    if( CSMPR_Player_GetStatus() == CSMPR_PLAY_RUN )
	{
		CSVID_SetInputMode( vid_handle, VID_INPUT_STILLPIC );
		CSDEMUX_VID_SetOutputMode( xport_vidout_handle, DEMUX_OUTPUT_MOD_BLOCK );
		// printf("CSDEMUX_VID_SetOutputMode: DEMUX_OUTPUT_MOD_BLOCK\n");
	}
	else
	{
		CSVID_SetInputMode( vid_handle, VID_INPUT_DEMUX0 );
		CSDEMUX_VID_SetOutputMode( xport_vidout_handle, DEMUX_OUTPUT_MOD_NONBLOCK );
                  // printf("CSDEMUX_VID_SetOutputMode: DEMUX_OUTPUT_MOD_NONBLOCK\n");
	}

    CSDEMUX_VID_SetPID(xport_vidout_handle,ProgramInfo.Video_PID);
    if( ProgramInfo.VideoType == eCS_AV_VIDEO_STREAM_H264 )
    {
		//printf("\nH.264 ########################################\n");
        CSVID_SetStreamType( vid_handle, VID_STREAM_TYPE_H264_TS );
    }
    else
    {
		//printf("\nMPEG2 ########################################\n");
        CSVID_SetStreamType( vid_handle, VID_STREAM_TYPE_MPEG2_TS );
    }

    CSVID_WaitSync(vid_handle, 1);

    // CSVID_EnablePTSSync(vid_handle);

    // CSVID_StartDelay(vid_handle, 2300);
    CSDEMUX_VID_Enable(xport_vidout_handle);
    CSDEMUX_PIDFT_Enable(xport_pid_filter0_handle);

    sdk_err = CSVID_Play(vid_handle);
	CSVID_EnablePTSSync(vid_handle);

	/* By KB Kim 2011.06.02 */
	SetVideoUnderflowStatus(0);

	CSVID_DataUnderflowNotify(vid_handle, MV_VideoData_UnderFlow_Callback, 1, 0);
    CSVID_SyncNotify(vid_handle, Sycro_call_back, 5000*1000, 1);
    //printf("CSVID_Play err = %d\n", sdk_err);

    //printf("begin CSVID_Play, time = %d\n", CS_OS_time_now());

    video_set_show = TRUE;

    return (err);

}

tCS_AV_Error Play_Audio(tCS_AV_PlayParams ProgramInfo)
{
	tCS_AV_Error    err = eCS_AV_OK;

	if(( aud_handle == NULL )||( xport_pid_filter1_handle == CSDEMUX_UNVALID_HANDLE )||( xport_audout_handle == CSDEMUX_UNVALID_HANDLE ))
	{
		printf("\n1 === %d , %d , %d =======\n", (int)aud_handle, (int)xport_pid_filter1_handle, (int)xport_audout_handle);
		return eCS_AV_ERROR;
	}

	if(ProgramInfo.Audio_PID == kDB_DEMUX_INVAILD_PID)
	{
		printf("\n2 === %d =======\n", ProgramInfo.Audio_PID);
		return eCS_AV_ERROR;
	}

	CSAUD_EnableMute( aud_handle );

	CSAUD_Stop(aud_handle);

	//usleep(100*1000);

	CSDEMUX_PIDFT_Disable(xport_pid_filter1_handle);

	CSDEMUX_AUD_Disable(xport_audout_handle);

	CSDEMUX_PIDFT_SetChannel(xport_pid_filter1_handle,DEMUX_CHL_ID0);
	CSDEMUX_PIDFT_SetPID(xport_pid_filter1_handle,ProgramInfo.Audio_PID);

	DB_DemuxSetAudPid(xport_pid_filter1_handle, ProgramInfo.Audio_PID);

	if( CSMPR_Player_GetStatus() == CSMPR_PLAY_RUN )
	{
		CSDEMUX_AUD_SetOutputMode( xport_audout_handle, DEMUX_OUTPUT_MOD_BLOCK );
	}
	else
	{
		CSDEMUX_AUD_SetOutputMode( xport_audout_handle, DEMUX_OUTPUT_MOD_NONBLOCK );
	}

	CSDEMUX_AUD_SetPID(xport_audout_handle,ProgramInfo.Audio_PID);

	CSAUD_Init( aud_handle );
	//CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
	CSAUD_EnableMute( aud_handle );


	/* Modify By River 06.12.2008 */
	switch( ProgramInfo.AudioType )
	{
		case eCS_AV_AUDIO_STREAM_AC3:
			if(CS_DBU_GetDefaultAudioType() == eCS_DBU_AUDIO_AC3)
			{
				CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
				printf("@@@eCS_AV_AUDIO_STREAM_AC3@ @@@\n");
			}
			else
			{
				CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
				// printf("@@@eAUD_OUTPUT_I2S_SPDIFPCM@@@@\n");
			}
			CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AC3 );
			break;

		case eCS_AV_AUDIO_STREAM_AAC:
			CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
			CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AAC );
			// printf("@@@eAUD_OUTPUT_I2S_SPDIFPCM@@@@\n");
			break;

		case eCS_AV_AUDIO_STREAM_LATM:
			CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
			CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AAC_LATM );
			// printf("@@@eAUD_OUTPUT_I2S_SPDIFPCM@@@@\n");
			break;

		case eCS_AV_AUDIO_STREAM_MPEG2:
		default:
			CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
			CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_MPA );
			// printf("@@@eAUD_OUTPUT_I2S_SPDIFPCM@@@@\n");
			break;

	}

	// CSAUD_EnablePTSSync(aud_handle);

	// CSAUD_SetStartDelay(aud_handle, 300);
	// printf("Play_Audio : Delay 1500\n");

	CSDEMUX_AUD_Enable(xport_audout_handle);
	CSDEMUX_PIDFT_Enable(xport_pid_filter1_handle);

	CSAUD_Play(aud_handle);
	CSAUD_EnablePTSSync(aud_handle);

	err = AV_AudioSetVolume(ProgramInfo.Audio_Volume);
	err = AV_AudioSetStereoMode(ProgramInfo.AudioMode);

	//Audio_ErrMoniter();

	return (err);
}

#if 0
#define CHECK_NULL(x) do { \
	if (NULL == (x)) printf(" NULL pointer returned, at %s:%d \n", __FUNCTION__, __LINE__); \
} while(0);

#define CHECK_CSAPI_RET(x) do { \
	if (CSAPI_FAILED == (x)) printf(" CSAPI_FAILED returned, at %s:%d \n", __FUNCTION__, __LINE__); \
} while(0);

#define CHECK_ETC_RET(x) do { \
	if (0 != (x)) printf(" ETC_FAILED returned, at %s:%d \n", __FUNCTION__, __LINE__); \
} while(0);

static CSDEMUX_HANDLE demux_vid_pidft;
static CSDEMUX_HANDLE demux_aud_pidft;

static CSDEMUX_HANDLE demux_vidout;
static CSDEMUX_HANDLE demux_audout;

void set_program(int vid_pid, int aud_pid, int v_type, int a_type, int vin_mode, int demux_out_mode)
{
        demux_vid_pidft     = xport_pid_filter0_handle;
        demux_aud_pidft   = xport_pid_filter1_handle;
        demux_vidout = xport_vidout_handle;
        demux_audout = xport_audout_handle;

	CHECK_CSAPI_RET(CSVID_SetOutputAlpha(vid_handle, 0x00));
	CHECK_CSAPI_RET(CSAUD_EnableMute(aud_handle));

	CHECK_CSAPI_RET(CSAUD_Stop(aud_handle));
	CHECK_CSAPI_RET(CSVID_Stop(vid_handle));

	CHECK_CSAPI_RET(CSDEMUX_PIDFT_Disable(demux_vid_pidft));
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_Disable(demux_aud_pidft));

	CHECK_CSAPI_RET(CSDEMUX_VID_Disable(demux_vidout));
	CHECK_CSAPI_RET(CSDEMUX_AUD_Disable(demux_audout));

	// PID Filter Config
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_SetChannel(demux_vid_pidft, DEMUX_CHL_ID0));
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_SetPID(demux_vid_pidft, vid_pid));

	CHECK_CSAPI_RET(CSDEMUX_PIDFT_SetChannel(demux_aud_pidft, DEMUX_CHL_ID0));
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_SetPID(demux_aud_pidft, aud_pid));

	// VID Output Config
	CHECK_CSAPI_RET(CSDEMUX_VID_SetOutputMode(demux_vidout, demux_out_mode));	/* play file mode. */
	CHECK_CSAPI_RET(CSDEMUX_VID_SetPID(demux_vidout, vid_pid));

	// AUD Output Config
	CHECK_CSAPI_RET(CSDEMUX_AUD_SetOutputMode(demux_audout, demux_out_mode));
	CHECK_CSAPI_RET(CSDEMUX_AUD_SetPID(demux_audout, aud_pid));

	CHECK_CSAPI_RET(CSAUD_Init(aud_handle));
	CHECK_CSAPI_RET(CSAUD_SetCodecType(aud_handle, a_type));

	CHECK_CSAPI_RET(CSAUD_EnablePTSSync(aud_handle));

	CHECK_CSAPI_RET(CSVID_SetStreamType(vid_handle, v_type));
	CHECK_CSAPI_RET(CSVID_EnablePTSSync(vid_handle));

	CHECK_CSAPI_RET(CSDEMUX_VID_Enable(demux_vidout));
	CHECK_CSAPI_RET(CSDEMUX_AUD_Enable(demux_audout));
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_Enable(demux_vid_pidft));
	CHECK_CSAPI_RET(CSDEMUX_PIDFT_Enable(demux_aud_pidft));

	CHECK_CSAPI_RET(CSVID_Play(vid_handle));
	CHECK_CSAPI_RET(CSAUD_Play(aud_handle));

	CHECK_CSAPI_RET(CSVID_SetInputMode(vid_handle, vin_mode));

    video_set_show = TRUE;

        // CHECK_CSAPI_RET(CSVID_SyncNotify(vid_handle, sync_notify, 3000000, 1));
        usleep(800000);
        CHECK_CSAPI_RET(CSVID_SetOutputAlpha(vid_handle, 0xff));
	CHECK_CSAPI_RET(CSAUD_DisableMute(aud_handle));/* this is a bug, because H.264 does not support Sync notifiacation interrupt. */

	return;
}
#endif

tCS_AV_Error Mv_VideoRestart(tCS_AV_PlayParams ProgramInfo)
{
	tCS_AV_Error    err = eCS_AV_OK;
    CSAPI_RESULT csResult = CSAPI_SUCCEED;

	// printf("\n====== CS_AV_VideoRestart =======\n\n");

	CSOS_WaitSemaphore(sem_AVAccess);
    // video_set_show = FALSE;
    if( vid_handle != NULL )
    {
        CSVID_Stop(vid_handle);
        //CSVID_Close(vid_handle);
        //vid_handle = NULL;
    }

    if( xport_pid_filter0_handle != CSDEMUX_UNVALID_HANDLE )
        {
            CSDEMUX_PIDFT_Disable(xport_pid_filter0_handle);
        }

	err = Play_Video(ProgramInfo);

    CSOS_SignalSemaphore(sem_AVAccess);

	return(err);
}

tCS_AV_Error CS_AV_ProgramPlay( tCS_AV_PlayParams ProgramInfo )
{
	tCS_AV_Error    err = eCS_AV_OK;
#if 0
	printf( "*******************cs_middleware*************************\n" );
	printf( "Play Program Video = 0x%x , VideoType = %s\n",
	       ProgramInfo.Video_PID,
	     ( ProgramInfo.VideoType > 0 )? "H.264": "MPEG2" );
	printf( "             Audio = 0x%x , AudioType = %d\n", ProgramInfo.Audio_PID, ProgramInfo.AudioType);
	printf( "*********************************************************\n" );
#endif


	CSOS_WaitSemaphore(sem_AVAccess);

	memset(&lastsrc,0,sizeof(lastsrc));

	err = Play_Audio(ProgramInfo);
	//printf("--@@@--- Play_Audio --- %d , %d ---@@@@@@@@----\n", err, ProgramInfo.Audio_Volume);
	err = Play_Video(ProgramInfo);
	//printf("--@@@--- Play_Video --- %d ---@@@@@@@@----\n", err);
	//set_program(ProgramInfo.Video_PID, ProgramInfo.Audio_PID, VID_STREAM_TYPE_H264_TS, AUD_STREAM_TYPE_AC3, VID_INPUT_DEMUX0, DEMUX_OUTPUT_MOD_NONBLOCK);

	CSOS_SignalSemaphore(sem_AVAccess);

	//printf("--@@@---CS_AV_ProgramPlay---end---@@@@@@@@----\n");

	return (err);
}



tCS_AV_Error CS_AV_ChangeAudioPid( U16 AudioPID, tCS_AV_AudioType AudioType )
{
         tCS_AV_Error    err = eCS_AV_OK;

         CSOS_WaitSemaphore(sem_AVAccess);

	if( aud_handle == NULL )
	{
		CSOS_SignalSemaphore(sem_AVAccess);
		return eCS_AV_ERROR;
	}

    CSAUD_EnableMute( aud_handle );


        CSAUD_Stop(aud_handle);

        //usleep(100*1000);
        //CSAUD_Close(aud_handle);
        //aud_handle = NULL;

    CSDEMUX_PIDFT_Disable(xport_pid_filter1_handle);

    if( xport_audout_handle != CSDEMUX_UNVALID_HANDLE )
    {
       CSDEMUX_AUD_Disable(xport_audout_handle);
       //CSDEMUX_AUD_Close(xport_audout_handle);
    }

    //PID Filter Config
    CSDEMUX_PIDFT_SetChannel(xport_pid_filter1_handle,DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(xport_pid_filter1_handle, AudioPID);

    //AUD Output Config
    //CSDEMUX_AUD_SetCABBuf(xport_audout_handle,CAB_REGION,CAB_SIZE);
    //CSDEMUX_AUD_SetPTSBuf(xport_audout_handle,AUD_PTS_REGION,AUD_PTS_SIZE);

    CSDEMUX_AUD_SetOutputMode(xport_audout_handle,DEMUX_OUTPUT_MOD_NONBLOCK);

    //CSDEMUX_AUD_SetSWMode(xport_audout_handle,xport_chl_handle);
    CSDEMUX_AUD_SetPID( xport_audout_handle, AudioPID );

    CSAUD_Init( aud_handle );
    CSAUD_EnableMute( aud_handle );

#if 0

    if( AudioType == eCS_AV_AUDIO_STREAM_MPEG2 )
    {
        CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
        CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_MPA );
    }
    else
    {
            if(CS_DBU_GetDefaultAudioType() == eCS_DBU_AUDIO_AC3)
                {
                    CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
                }
            else
                {
                    CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
                }

        CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AC3 );
    }
#else
	switch( AudioType )
        {
            case eCS_AV_AUDIO_STREAM_AC3:
                if(CS_DBU_GetDefaultAudioType() == eCS_DBU_AUDIO_AC3)
                    {
                        CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
                    }
                else
                    {
                        CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
                    }
                CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AC3 );
                break;

            case eCS_AV_AUDIO_STREAM_AAC:
                CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
                CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AAC );
                break;

            case eCS_AV_AUDIO_STREAM_LATM:
                CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
                CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_AAC_LATM );
                break;

            case eCS_AV_AUDIO_STREAM_MPEG2:
            default:
                CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
                CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_MPA );
                break;

        }
#endif

    // CSAUD_EnablePTSSync(aud_handle);
	// CSAUD_SetStartDelay(aud_handle, 1500);
	// printf("CS_AV_ChangeAudioPid : Delay 1500\n");

    CSDEMUX_AUD_Enable(xport_audout_handle);
    CSDEMUX_PIDFT_Enable(xport_pid_filter1_handle);


    CSAUD_Play(aud_handle);
    CSAUD_EnablePTSSync(aud_handle);

    CSOS_DelayTaskMs(200);

    err = AV_AudioSetVolume(CS_AV_AudioGetVolume());
    err = Audio_SetMuteStatus(CS_AV_Audio_GetMuteStatus());

    CSOS_SignalSemaphore(sem_AVAccess);


	return (err);

}

tCS_AV_Error CS_AV_EnableTVOut(void)
{
    if(CSTVOUT_Enable(tve_handle) == CSAPI_SUCCEED)
        return eCS_AV_OK;
    else
        return eCS_AV_ERROR;
}


tCS_AV_Error CS_AV_DisableTVOut(void)
{
    if(CSTVOUT_Disable(tve_handle) == CSAPI_SUCCEED)
        return eCS_AV_OK;
    else
        return eCS_AV_ERROR;
}

void testac3(void)
{
    if( aud_handle == NULL )
	{
		return;
	}
    printf("testac3\n");
    CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
}

FILE*   vid_file=NULL;

tCS_AV_Error CS_AV_Play_IFrame(const char* file_path)
{
    int 			read_len = 0;
    unsigned int 	bufsize = 0;
    unsigned char  	read_buf[4096+100];
    U8  			i = 0;
	U8 				read_count = 0;

    memset(read_buf, 0, 4096+100);

    CSVID_PFMClose(vid_handle);

    //CSVID_SetOutputPostion(vid_handle, &src_rect, &dst_rect);
    CSVID_SetOutputAlpha(vid_handle, 0);
    CSVID_SetStreamType(vid_handle, VID_STREAM_TYPE_MPEG2_TS);
    CSVID_SetInputMode(vid_handle, VID_INPUT_STILLPIC);
    CSVID_SetDecoderMode(vid_handle, 3);
    //CSVID_SyncNotify(vid_handle, testvideocallback2, 15, 1);
    CSVID_Play(vid_handle);
    video_set_show = TRUE;
    //CSVID_EnablePTSSync(vid_handle);
    CSVID_PFMOpen(vid_handle);
    CSVID_GetPFMBufferSize(vid_handle, &bufsize);

    //CSVID_SetNotifyPFMDataEmpty(vid_handle, &datasize, testvideocallback, 1);
    //CSVID_PScanCropNotify(vid_handle, testvideocallback_pscancrop, 1);
    //CSVID_AspectRatioChangeNotify(vid_handle, testvideocallback_aspectratio, 1);
    //printf("bufsize = %d\n",bufsize);

	//printf("\n====>>>>>>>>> %s \n", file_path);
    vid_file = fopen(file_path,"rb");//mpeg2_mpa_480p_qianlizhiwai_video1.mpv//mpeg2_mpa_1080i_video1.mpv
    if(vid_file == NULL)
    {
       printf("======>file open failed\n");
       return eCS_AV_ERROR;
    }

    fseek(vid_file,0L, SEEK_SET);
    if ((read_len=fread(read_buf,1,4096,vid_file))!=4096)
    {
        fseek(vid_file,0L, SEEK_SET);
        //printf("1 read_len = %d ,===no data \n",read_len);
    }

    for(i=0;i<100;i++){
        if(CSAPI_SUCCEED == CSVID_WritePFMData(vid_handle, read_buf, read_len)){
            if ((read_len=fread(read_buf,1,4096,vid_file))!=4096)
            {
                fseek(vid_file,0L, SEEK_SET);
                //printf("2 read_len = %d : %d ,===no data \n", i, read_len);

				if ( read_count > 1 )
					break;
				read_count++;
            }
        }
    }

     usleep(300000);
     CSVID_SetOutputAlpha(vid_handle, 0xff);

     //CSVID_PFMClose(vid_handle);

    return eCS_AV_OK;
}

/*Sertac ozel Play I Frame fonksiyonu...*/
tCS_AV_Error CS_AV_Play_IFrame2(const char* file_path)
{
    int 			read_len = 0;
    unsigned int 	bufsize = 0;
    unsigned char  	read_buf[4096+100];
    U8  			i = 0;
	U8 				read_count = 0;

    memset(read_buf, 0, 4096+100);

    CSVID_PFMClose(vid_handle);

    CSVID_STREAM_TYPE oldstreamtype;

    //CSVID_GetStreamType(vid_handle, &oldstreamtype);
    //printf("oldstreamtype=%d\n",oldstreamtype);

    //CSVID_SetOutputPostion(vid_handle, &src_rect, &dst_rect);
    CSVID_SetOutputAlpha(vid_handle, 0);
    CSVID_SetStreamType(vid_handle, VID_STREAM_TYPE_MPEG2_TS);
    CSVID_SetInputMode(vid_handle, VID_INPUT_STILLPIC);
    CSVID_SetDecoderMode(vid_handle, 3);
    //CSVID_SyncNotify(vid_handle, testvideocallback2, 15, 1);
    CSVID_Play(vid_handle);
    video_set_show = TRUE;
    //CSVID_EnablePTSSync(vid_handle);
    CSVID_PFMOpen(vid_handle);
    CSVID_GetPFMBufferSize(vid_handle, &bufsize);

    //CSVID_SetNotifyPFMDataEmpty(vid_handle, &datasize, testvideocallback, 1);
    //CSVID_PScanCropNotify(vid_handle, testvideocallback_pscancrop, 1);
    //CSVID_AspectRatioChangeNotify(vid_handle, testvideocallback_aspectratio, 1);
    //printf("bufsize = %d\n",bufsize);

	//printf("\n====>>>>>>>>> %s \n", file_path);
    vid_file = fopen(file_path,"rb");//mpeg2_mpa_480p_qianlizhiwai_video1.mpv//mpeg2_mpa_1080i_video1.mpv
    if(vid_file == NULL)
    {
       printf("======>file open failed\n");
       return eCS_AV_ERROR;
    }

    fseek(vid_file,0L, SEEK_SET);
    if ((read_len=fread(read_buf,1,4096,vid_file))!=4096)
    {
        fseek(vid_file,0L, SEEK_SET);
        //printf("1 read_len = %d ,===no data \n",read_len);
    }

    for(i=0;i<100;i++){
        if(CSAPI_SUCCEED == CSVID_WritePFMData(vid_handle, read_buf, read_len)){
            if ((read_len=fread(read_buf,1,4096,vid_file))!=4096)
            {
                fseek(vid_file,0L, SEEK_SET);
                //printf("2 read_len = %d : %d ,===no data \n", i, read_len);

				if ( read_count > 1 )
					break;
				read_count++;
            }
        }
    }

     usleep(300000);

     CSVID_SetOutputAlpha(vid_handle, 0);
     //CSVID_SetInputMode(vid_handle, VID_INPUT_DEMUX0);
     //CSVID_SetDecoderMode(vid_handle, 0);
     //CSVID_SetStreamType(vid_handle, oldstreamtype);

     //CSVID_PFMClose(vid_handle);

    return eCS_AV_OK;
}

/*Yukar?s? deneme i?in*/

tCS_AV_Error CS_AV_VID_GetPTS(long long * VideoPts)
{
    CSVID_GetPTS(vid_handle, VideoPts);

    return eCS_AV_OK;
}

tCS_AV_Error MV_GetVideoSizeWH(int *S_W, int *S_H)
{
	CSVID_SequenceHeader 	hdr;

	if( vid_handle == NULL )
	{
		return eCS_AV_ERROR;
	}

    CSVID_GetSequenceHeader(vid_handle, &hdr);

	//printf("MV_GetVideoSize :: W : %d , H : %d\n", hdr.w, hdr.h);

	*S_W = hdr.w;
	*S_H = hdr.h;
	return eCS_AV_OK;
}

tCS_AV_Error CS_AV_VID_SetContrast(unsigned int contrast)
{
	printf("= SET CS_AV_VID_SetContrast : %d =>", contrast);

	contrast = contrast * 256/10;
	if ( contrast > 254)
		contrast = 254;
	else if ( contrast < 1 )
		contrast = 1;

	printf(" %d \n", contrast);

	if (CSTVOUT_SetContrast(tve_handle, contrast) == CSAPI_FAILED)
	{
		printf("\n ERROR ==> CSTVOUT_SetContrast \n");
		return eCS_AV_ERROR;
	}
	else
		return eCS_AV_OK;
}

tCS_AV_Error CS_AV_VID_GetContrast(unsigned int *contrast)
{
    CSTVOUT_GetContrast(tve_handle, contrast);
	printf("= GET CS_AV_VID_GetContrast : %d =>", *contrast);
	*contrast = *contrast / ( 256/10 );
	printf(" %d \n", *contrast);

	return eCS_AV_OK;  // Just Warning message
}

tCS_AV_Error CS_AV_VID_SetBrightness(unsigned int brightness)
{
	printf("= SET CS_AV_VID_SetBrightness : %d =>", brightness);

	brightness = brightness * 256/10;
	if ( brightness > 254)
		brightness = 254;
	else if ( brightness < 1 )
		brightness = 1;

	printf(" %d \n", brightness);

    if (CSTVOUT_SetBrightness(tve_handle, brightness) == CSAPI_FAILED)
    {
		printf("\n ERROR ==> CSTVOUT_SetBrightness \n");
		return eCS_AV_ERROR;
    }
	else
		return eCS_AV_OK;
}

tCS_AV_Error CS_AV_VID_GetBrightness(unsigned int *brightness)
{
    CSTVOUT_GetBrightness(tve_handle, brightness);
	printf("= GET CS_AV_VID_GetBrightness : %d =>", *brightness);
	*brightness = *brightness / ( 256/10 );
	printf(" %d \n", *brightness);

	return eCS_AV_OK;  // Just Warning message
}

tCS_AV_Error CS_AV_VID_SetSaturation(unsigned int saturation)
{
	printf("= SET CS_AV_VID_SetSaturation : %d =>", saturation);

	saturation = saturation * 256/10;
	if ( saturation > 254)
		saturation = 254;
	else if ( saturation < 1 )
		saturation = 1;

	printf(" %d \n", saturation);

	if (CSTVOUT_SetSaturation(tve_handle, saturation) == CSAPI_FAILED)
	{
		printf("\n ERROR ==> CSTVOUT_SetSaturation \n");
		return eCS_AV_ERROR;
	}
	else
		return eCS_AV_OK;
}

tCS_AV_Error CS_AV_VID_GetSaturation(unsigned int *saturation)
{
    CSTVOUT_GetSaturation(tve_handle, saturation);
	printf("= GET CS_AV_VID_GetSaturation : %d =>", *saturation);
	*saturation = *saturation / ( 256/10 );
	printf(" %d \n", *saturation);

	return eCS_AV_OK;  // Just Warning message
}

