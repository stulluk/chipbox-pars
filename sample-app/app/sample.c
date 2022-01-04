#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "csapi.h"

typedef struct CS_Service_t_ {
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

typedef struct CS_Test_Section_t_ {
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
#define SDT_PID 0x11
#define SECTION_LENGTN 1024
#define MAX_SECTION_LENGTN 1024
#define CURRENT_SERVICE_NUM 16
#define  MAX_SERVICE_NUM 32

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
unsigned int service_num = 0;
unsigned int cur_pmt_parser_index = 0;
unsigned int sdtcomplete = 0;
unsigned char current_section_num = 0, last_section_num = 0;
CS_Service_t service[MAX_SERVICE_NUM];
CS_Test_Section_t testfilter[MAX_SERVICE_NUM];

long time_start = 0, time_end = 0;
test_pid g_vpid = 0x481, g_apid = 0x482;
int g_tunerflag = 1, g_vid_decodertype = 0, g_aud_decodertype = 0;
CSVID_Rect src_rect = { 0, 720, 0, 576 };
CSVID_Rect dst_rect = { 0, 720, 0, 576 };

static CSAPI_RESULT errno = CSAPI_SUCCEED;

#define OUT_CHL0_DIR_WP_ADDR    (0x41400000+(0x100<<2))
#define OUT_CHL0_DIR_RP_ADDR    (0x41400000+(0x101<<2))
#define OUT_CHL2_DIR_WP_ADDR    (0x41400000+(0x104<<2))
#define OUT_CHL2_DIR_RP_ADDR    (0x41400000+(0x105<<2))

static int _gpio_write(char *devname, char *buf, int len)
{
    int gpio_fd;
    int retval;
    char cmd = 'O';

    puts(devname);
    gpio_fd = open(devname, O_RDWR);
    if (gpio_fd <= 0) {
        printf("Error: Open %s.\n", devname);
        return -1;
    }

    retval = write(gpio_fd, &cmd, 1);
    if (retval != 1) {
        printf("Error: Read %s. \n", devname);
        return -1;
    }

    retval = write(gpio_fd, buf, len);
    if (retval != len) {
        printf("Error: Read %s. \n", devname);
        return -1;
    }

    retval = close(gpio_fd);
    return retval;
}

static int tuner_gpio_reset(void)
{
    int retval;
    char value = '0';

    retval = _gpio_write("/dev/gpio/6", &value, 1);
    usleep(400000);
    value = '1';
    retval = _gpio_write("/dev/gpio/6", &value, 1);
    usleep(400000);

    return 0;
}

int tuner_lock(unsigned int tuner_frequency, unsigned int tuner_symbol_rate, unsigned int tuner_mod)
{
    int err_code = 0;
    int tuner_fd = 0;

    TUNER_PARAMS_S tuner_param;
    TUNER_STATUS_E tuner_status = 0;
    int icount = 0;

    while (1) {
        tuner_gpio_reset();

        err_code = cs_tuner_init();
        if (err_code < 0) {
            printf("Error: cs_tuner_init.\n");
            return err_code;
        }
        printf("1 success cs_tuner_init. \n");

        tuner_fd = cs_tuner_open(TUNER_ID_0);
        if (tuner_fd < 0) {
            printf("Error: cs_tuner_init.\n");
            return tuner_fd;
        }
        printf("1 success cs_tuner_open. \n");

        memset(&tuner_param, 0, sizeof(TUNER_PARAMS_S));
        tuner_param.frequency = tuner_frequency * 1000;
        tuner_param.qam_params.symbol_rate = tuner_symbol_rate;
        if (tuner_mod == 0)
            tuner_param.qam_params.modulation = QAM_32;
        else if (tuner_mod == 1)
            tuner_param.qam_params.modulation = QAM_64;
        else if (tuner_mod == 2)
            tuner_param.qam_params.modulation = QAM_128;
        else if (tuner_mod == 3)
            tuner_param.qam_params.modulation = QAM_256;
        else
            tuner_param.qam_params.modulation = QAM_64;

        tuner_param.inversion = INVERSION_NORMAL;

        err_code = cs_tuner_set_params(tuner_fd, &tuner_param);
        if (err_code < 0) {
            printf("Error: cs_tuner_set_params.\n");
            return err_code;
        }

        for (icount = 0; icount < 8; icount++) {
            tuner_status = 0;
            err_code = cs_tuner_read_status(tuner_fd, &tuner_status);
            if (err_code < 0) {
                printf("Error: cs_tuner_read_status.\n");
                return err_code;
            }
            if (tuner_status == 1)
                return tuner_status;
            sleep(2);
        }
    }

    return 0;
}

static void init(void)
{
    CSOSD_Config osd_config = { OSD_MODE_576, OSD_COLOR_DEPTH_16, OSD_COLOR_FORMAT_RGB565 };
//open tvout device
    if (NULL == (g_tvout_handle = CSTVOUT_Open())) {
        printf("open tvout device Failed\n");
        return;
    }
    CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_576I);

#if 1
    if (NULL == (g_osd_handle = CSOSD_Open(OSD_LAYER_0))) {
        printf("open osd device Failed\n");
        return;
    }
    CSOSD_SetConfig(g_osd_handle, &osd_config);
    CSOSD_Enable(g_osd_handle);
#endif
//open video
    if (NULL == (g_video_handle = CSVID_Open(VID_DEV_0))) {
        printf("open video error\n");
        return;
    }
    else
        printf("open video ok\n");

//open audio    
    if (NULL == (g_audio_handle = CSAUD_Open(AUD_DEV_0))) {
        printf("open audio error\n");
        return;
    }
    else
        printf("open audio ok\n");

//open demux for video out
    if (NULL == (g_demux_video_handle = CSDEMUX_VID_Open(DEMUX_VIDOUT_ID0))) {
        printf("open demux for video out Failed ...\n");
        return;
    }

//open demux for audio out
    if (NULL == (g_demux_audio_handle = CSDEMUX_AUD_Open(DEMUX_AUDOUT_ID0))) {
        printf("open demux for audio out Failed ...\n");
        return;
    }

//get pid filter for video
    if (NULL == (g_demux_pidfilter_video_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID62))) {
        printf("get pid filter for video Failed ...\n");
        return;
    }

//get pid filter for audio
    if (NULL == (g_demux_pidfilter_audio_handle = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID63))) {
        printf("get pid filter for audio Failed ...\n");
    }

    //video decoder set
    CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
    CSVID_SetOutputAlpha(g_video_handle, 0x80);

    //audio decoder set
    CSAUD_Init(g_audio_handle, NULL);
    CSAUD_SetVolume(g_audio_handle, NULL);
}

static void play(int index)
{
    g_vpid = service[index].servce_vidpid;
    g_apid = service[index].servce_audpid;
    g_vid_decodertype = service[index].vid_type;
    g_aud_decodertype = service[index].aud_type;
    printf("Set Program Video = 0x%x , Audio = 0x%x \n", g_vpid, g_apid);
    printf("Set Program g_vid_decodertype = 0x%x , g_aud_decodertype = 0x%x \n", g_vid_decodertype,
           g_aud_decodertype);
//demux PID Filter Config
    CSDEMUX_PIDFT_SetChannel(g_demux_pidfilter_video_handle, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(g_demux_pidfilter_video_handle, g_vpid);

    CSDEMUX_PIDFT_SetChannel(g_demux_pidfilter_audio_handle, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(g_demux_pidfilter_audio_handle, g_apid);

//demux AUD Output Config
    CSDEMUX_AUD_SetCABBuf(g_demux_audio_handle, CAB_REGION, CAB_SIZE);
    CSDEMUX_AUD_SetPTSBuf(g_demux_audio_handle, AUD_PTS_REGION, AUD_PTS_SIZE);

    if (g_tunerflag == 1)
        CSDEMUX_AUD_SetOutputMode(g_demux_audio_handle, DEMUX_OUTPUT_MOD_NONBLOCK);
    else
        CSDEMUX_AUD_SetOutputMode(g_demux_audio_handle, DEMUX_OUTPUT_MOD_BLOCK);

    CSDEMUX_AUD_SetPID(g_demux_audio_handle, g_apid);

//demux VID Output Config
    CSDEMUX_VID_SetCPBBuf(g_demux_video_handle, CPB0_REGION, CPB0_SIZE);
    CSDEMUX_VID_SetDIRBuf(g_demux_video_handle, CPB0_DIR_REGION, CPB0_DIR_SIZE);

    if (g_tunerflag == 1)
        CSDEMUX_VID_SetOutputMode(g_demux_video_handle, DEMUX_OUTPUT_MOD_NONBLOCK);
    else
        CSDEMUX_VID_SetOutputMode(g_demux_video_handle, DEMUX_OUTPUT_MOD_BLOCK);

    CSDEMUX_VID_SetPID(g_demux_video_handle, g_vpid);

//start play
    CSAUD_Init(g_audio_handle, NULL);
    if (g_aud_decodertype == 0)
        CSAUD_SetCodecType(g_audio_handle, AUD_STREAM_TYPE_AC3);
    else if (g_aud_decodertype == 1)
        CSAUD_SetCodecType(g_audio_handle, AUD_STREAM_TYPE_MPA);
    else
        printf("invalid audio stream type %d\n", g_aud_decodertype);

    if (g_vid_decodertype == 0)
        CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_H264_TS);
    else if (g_vid_decodertype == 1)
        CSVID_SetStreamType(g_video_handle, VID_STREAM_TYPE_MPEG2_TS);
    else
        printf("invalid video stream type %d\n", g_vid_decodertype);

    CSVID_EnablePTSSync(g_video_handle);
    CSAUD_EnablePTSSync(g_audio_handle);
    CSDEMUX_VID_Enable(g_demux_video_handle);
    CSDEMUX_AUD_Enable(g_demux_audio_handle);
    CSDEMUX_PIDFT_Enable(g_demux_pidfilter_video_handle);
    CSDEMUX_PIDFT_Enable(g_demux_pidfilter_audio_handle);
    CSAUD_SetSampleRate(g_audio_handle, 48000);
    CSVID_Play(g_video_handle);
    CSAUD_Play(g_audio_handle);
    sleep(1);
    {
        CSVID_SequenceHeader hdr;
        CSVID_GetSequenceHeader(g_video_handle, &hdr);
        printf("hdr.w = %d,hdr.h = %d\n", hdr.w, hdr.h);
        src_rect.right = hdr.w;
        src_rect.bottom = hdr.h;
        if (hdr.h == 576) {
            CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_576I);
            dst_rect.right = 720;
            dst_rect.bottom = 576;
        }
        else if (hdr.h == 480) {
            CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_480I);
            dst_rect.right = 720;
            dst_rect.bottom = 480;
        }
        else if (hdr.h == 720) {
            CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_720P50);
            dst_rect.right = 1280;
            dst_rect.bottom = 720;
        }
        else if ((hdr.h > 1069) || (hdr.h < 1091)) {
            CSTVOUT_SetMode(g_tvout_handle, TVOUT_MODE_1080I25);
            dst_rect.right = 1920;
            dst_rect.bottom = 1080;
        }
        errno = CSVID_SetOutputPostion(g_video_handle, &src_rect, &dst_rect);
        printf("CSVID_SetOutputPostion errno %d\n", errno);
    }
    errno = CSVID_SetOutputAlpha(g_video_handle, 0x80);
    printf("CSVID_SetOutputAlpha errno %d\n", errno);
}

static void stop(void)
{
    CSVID_SetOutputAlpha(g_video_handle, 0x0);

    CSVID_Stop(g_video_handle);
    CSAUD_Stop(g_audio_handle);
    CSDEMUX_VID_Disable(g_demux_video_handle);
    CSDEMUX_AUD_Disable(g_demux_audio_handle);
    CSDEMUX_PIDFT_Disable(g_demux_pidfilter_video_handle);
    CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
}

static void Pat_table_parse(unsigned char *ptr, unsigned int len)
{
    int serverid;
    int pmtpid;
    int sectionlen;

    if (len < 12 || (ptr[1] & 0x80) == 0)
        return;
    if (!(ptr[5] & 0x1))
        return;
    sectionlen = ((ptr[1] & 0xf) << 8 | ptr[2]);

    ptr += 8;
    int i = sectionlen - 9;
    while (i >= 4) {
        serverid = (ptr[0] << 8) | ptr[1];
        pmtpid = ((ptr[2] & 0x1f) << 8) | ptr[3];
        printf("server id = 0x%x, pmt pid = 0x%x\n", serverid, pmtpid);
        if (service_num < MAX_SERVICE_NUM && serverid != 0) {
            service[service_num].servce_id = serverid;
            service[service_num].servce_pmtpid = pmtpid;
            service[service_num].servce_vidpid = 0x1fff;
            service[service_num].servce_audpid = 0x1fff;
            service[service_num].pmt_complete = 0;
            service_num++;
        }
        ptr += 4;
        i -= 4;
    }
}

static void Pmt_table_parse(unsigned char *ptr, unsigned int len)
{
    int l;
    int tmp;
    int vpid = 0x1fff;
    int apid = 0x1fff;
    int audionum = 0;

    if (len < 12 || (ptr[1] & 0x80) == 0)
        return;
    if (!(ptr[5] & 0x1))
        return;

    tmp = (((ptr[10] & 0xf) << 8) | ptr[11]) + 12;
    l = len - tmp - 4;
    ptr += tmp;

    while (l >= 5) {

        if (ptr[0] == 0x01 || ptr[0] == 0x02 || ptr[0] == 0x1b) {
            vpid = ((ptr[1] & 0x1f) << 8) | ptr[2];
            service[cur_pmt_parser_index].servce_vidpid = vpid;
            if (ptr[0] == 0x1b)
                service[cur_pmt_parser_index].vid_type = 0;
            else
                service[cur_pmt_parser_index].vid_type = 1;
        }
        else if ((ptr[0] == 0x03 || ptr[0] == 0x04 || ptr[0] == 0x81 || ptr[0] == 0x6a) && (audionum == 0)) {
            apid = ((ptr[1] & 0x1f) << 8) | ptr[2];
            service[cur_pmt_parser_index].servce_audpid = apid;
            if (ptr[0] == 0x6a || ptr[0] == 0x81)
                service[cur_pmt_parser_index].aud_type = 0;
            else
                service[cur_pmt_parser_index].aud_type = 1;
            audionum++;
        }
        else {
            printf("=====>invalid type:%d\n", ptr[0]);
        }
        tmp = (((ptr[3] & 0xf) << 8) | ptr[4]) + 5;
        l -= tmp;
        ptr += tmp;
    }
    printf("video pid = 0x%08x, audio pid = 0x%08x\n", vpid, apid);
}

static void Sdt_table_parse(unsigned char *ptr, unsigned int len)
{
    int l, tmp, i, descriptorlooplen, descriptorlen, namelen, templen;
    unsigned short serviceid;
    char *aaa = NULL;

    //printf("start parse sdt!\n");
    if (len < 12 || (ptr[1] & 0x80) == 0)
        return;
    if (!(ptr[5] & 0x1))
        return;

    tmp = ((ptr[1] << 8) & 0xf00) | ptr[2];
    // printf("tmp=%d\n",tmp);

    current_section_num = ptr[6];
    last_section_num = ptr[7];
    if (current_section_num == last_section_num)
        sdtcomplete = 1;
    else
        sdtcomplete = 0;

    //printf("current_section_num %d, last_section_num %d\n",current_section_num,last_section_num);
    l = tmp - 4 - 8;
    ptr += 11;

    while (l) {
        //getchar();
        serviceid = ptr[0] << 8 | ptr[1];
        templen = descriptorlooplen = ((ptr[3] << 8) & 0xf00) | ptr[4];
        //printf("serviceid %d,descriptorlooplen %d\n",serviceid,descriptorlooplen);
        aaa = ptr;
        for (i = 0; i < service_num; i++) {
            if (service[i].servce_id == serviceid) {
                aaa += 5;
                descriptorlen = aaa[1];
                if (aaa[0] == 0x48) {
                    aaa += 3;
                    namelen = aaa[0];
                    //printf("namelen =%d \n",namelen);
                    aaa++;
                    memset((char *) service[i].providername, 0, 20);
                    memcpy((char *) service[i].providername, (char *) aaa, namelen);
                    //printf("service name %s \n",(char*)service[i].providername);
                    aaa += namelen;
                    namelen = aaa[0];
                    //printf("namelen =%d \n",namelen);
                    aaa++;
                    memset((char *) service[i].servicename, 0, 20);
                    memcpy((char *) service[i].servicename, (char *) aaa, namelen);
                    //printf("service name %s \n",(char*)service[i].servicename);
                    aaa += namelen;
                    //templen=templen-descriptorlen-2;
                }
                else {
                    //printf("invalid descriptor 0x%x\n",aaa[0]);
                    aaa = aaa + descriptorlen + 2;
                    //descriptorlooplen=descriptorlooplen-descriptorlen-2;
                }
            }
        }
        l = l - templen - 5;
        ptr = ptr + templen + 5;
    }
}

static void Search_Channel(void)
{
    CSDEMUX_HANDLE hfilter;
    CSDEMUX_HANDLE hpidfilter;
    unsigned char filter[12];
    unsigned char mask[12];
    unsigned int data_lengtn = 0;
    int pmt_parsed_num = 0;

    unsigned int servce_id = 0;
    unsigned int servce_pmtpid = 0;
    unsigned int servce_vidpid = 0;
    unsigned int servce_audpid = 0;

    service_num = 0;
    cur_pmt_parser_index = 0;

    memset(filter, 0, 12);
    memset(mask, 0, 12);

    filter[0] = 0x00;
    mask[0] = 0xff;

    hfilter = CSDEMUX_Filter_Open(DEMUX_FILTER_ID0);
    hpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0);

    CSDEMUX_PIDFT_SetChannel(hpidfilter, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(hpidfilter, PAT_PID);
    CSDEMUX_PIDFT_Enable(hpidfilter);

    CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
    CSDEMUX_Filter_AddPID(hfilter, PAT_PID);
    CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_Enable(hfilter);

    while (1) {
        data_lengtn = 1024;
        if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn) == CSAPI_SUCCEED) {
                printf("data size  = %d\n", data_lengtn);
                Pat_table_parse(section_buf, data_lengtn);
                break;
            }
        }
        else
            printf("PAT---------ReadWait Timeout\n");
    }
    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_Filter_Close(hfilter);
    printf("PAT complete! Service_num = %d \n", service_num);

    for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) {
        if (service[cur_pmt_parser_index].servce_id == 0)
            continue;

        hfilter = CSDEMUX_Filter_Open(cur_pmt_parser_index);
        hpidfilter = CSDEMUX_PIDFT_Open(cur_pmt_parser_index);
        service[cur_pmt_parser_index].filter_handle = hfilter;
        service[cur_pmt_parser_index].pidfilter_handle = hpidfilter;
        filter[0] = 0x02;
        mask[0] = 0xff;

        CSDEMUX_PIDFT_SetChannel(hpidfilter, DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter, service[cur_pmt_parser_index].servce_pmtpid);
        CSDEMUX_PIDFT_Enable(hpidfilter);

        CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
        CSDEMUX_Filter_AddPID(hfilter, service[cur_pmt_parser_index].servce_pmtpid);
        CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(hfilter);
    }

    pmt_parsed_num = service_num;
    printf("pmt_parsed_num = %d\n", pmt_parsed_num);
    while (pmt_parsed_num) {
        for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) {
            if (service[cur_pmt_parser_index].pmt_complete)
                continue;
            data_lengtn = 1024;
            hfilter = service[cur_pmt_parser_index].filter_handle;
            if (CSDEMUX_Filter_ReadWait(hfilter, 1000) == CSAPI_SUCCEED) {
                if (CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn) == CSAPI_SUCCEED) {
                    printf("data size  = %d\n", data_lengtn);
                    Pmt_table_parse(section_buf, data_lengtn);
                    service[cur_pmt_parser_index].pmt_complete = 1;
                    printf("PMT 0x%x parse complete !\n ",
                           service[cur_pmt_parser_index].servce_pmtpid);
                    pmt_parsed_num--;
                    break;
                }
            }
        }
    }

    for (cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++) {
        servce_id = service[cur_pmt_parser_index].servce_id;
        servce_pmtpid = service[cur_pmt_parser_index].servce_pmtpid;
        servce_vidpid = service[cur_pmt_parser_index].servce_vidpid;
        servce_audpid = service[cur_pmt_parser_index].servce_audpid;
        CSDEMUX_PIDFT_Disable(service[cur_pmt_parser_index].pidfilter_handle);
        CSDEMUX_PIDFT_Close(service[cur_pmt_parser_index].pidfilter_handle);
        CSDEMUX_Filter_Disable(service[cur_pmt_parser_index].filter_handle);
        CSDEMUX_Filter_Close(service[cur_pmt_parser_index].filter_handle);
        printf("servce_id = 0x%x,  servce_pmtpid = 0x%x,  servce_vidpid = 0x%x, servce_audpid = 0x%x\n",
               servce_id, servce_pmtpid, servce_vidpid, servce_audpid);
    }
#if 1                //request and parse sdt
    while (1) {
        int timeout_flag = 1;

        if (sdtcomplete) {
            printf("SDT complete! Service_num = %d \n", service_num);
            break;
        }
        memset(filter, 0, 12);
        memset(mask, 0, 12);
        filter[0] = 0x42;
        mask[0] = 0xff;
        if (current_section_num) {
            filter[6] = current_section_num + 1;
            mask[6] = 0xff;
        }
        else {
            filter[6] = 0x00;
            mask[6] = 0xff;
        }
        hfilter = CSDEMUX_Filter_Open(DEMUX_FILTER_ID0);
        hpidfilter = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0);
        CSDEMUX_PIDFT_SetChannel(hpidfilter, DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter, SDT_PID);
        CSDEMUX_PIDFT_Enable(hpidfilter);
        CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
        CSDEMUX_Filter_AddPID(hfilter, SDT_PID);
        CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
        CSDEMUX_Filter_Enable(hfilter);
        data_lengtn = 1024;
        timeout_flag = 1;
        while (CSDEMUX_Filter_ReadWait(hfilter, 5000) == CSAPI_SUCCEED) {
            timeout_flag = 0;
            if (CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn) == CSAPI_SUCCEED) {
                printf("data size  = %d\n", data_lengtn);
                Sdt_table_parse(section_buf, data_lengtn);
                break;
            }
        }
        printf("SDT#########################Disable PIDFilter and Filter\n");
        CSDEMUX_PIDFT_Disable(hpidfilter);
        CSDEMUX_PIDFT_Close(hpidfilter);
        CSDEMUX_Filter_Disable(hfilter);
        CSDEMUX_Filter_Close(hfilter);
        if (timeout_flag) {
            printf("SDT---------ReadWait Timeout\n");
            break;
        }
    }
#endif
/*    
    for( cur_pmt_parser_index = 0; cur_pmt_parser_index < service_num; cur_pmt_parser_index++)
    {
         if(service[cur_pmt_parser_index].servce_id==0) continue;

         filter[0] = 0x02;
         mask[0]   = 0xff;
         CSDEMUX_PIDFT_SetChannel(hpidfilter,DEMUX_CHL_ID0);
         CSDEMUX_PIDFT_SetPID(hpidfilter,service[cur_pmt_parser_index].servce_pmtpid);  
         CSDEMUX_PIDFT_Enable(hpidfilter);
    
         CSDEMUX_Filter_SetFilter(hfilter,filter,mask);
         CSDEMUX_Filter_AddPID(hfilter,service[cur_pmt_parser_index].servce_pmtpid);
         CSDEMUX_Filter_SetFilterType(hfilter,DEMUX_FILTER_TYPE_SEC);
         CSDEMUX_Filter_Enable(hfilter);
      {  
            if(CSDEMUX_Filter_ReadWait(hfilter,1000) == CSAPI_SUCCEED)
            {
                data_lengtn = 1024;
                if(CSDEMUX_Filter_ReadSectionData(hfilter, section_buf, &data_lengtn)==CSAPI_SUCCEED)
                {
                    printf("data size  = %d\n", data_lengtn);
                    Pmt_table_parse(section_buf,data_lengtn);
                    break;
                }
                else printf("PMT------------ReadWait timeout\n");
            }
        } 
        while(1)
        
    }
 */

}

pthread_t read_thread;
static void Read_Data(CS_Test_Section_t * para)
{
    unsigned int reg_val;
    unsigned char buf[1024];
    CSDEMUX_HANDLE hfilter;

    hfilter = para->filter_handle;
    while (1) {
        reg_val = 1024;
        memset(buf, 0, 1024);
        if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadSectionData(hfilter, buf, &reg_val) == CSAPI_SUCCEED) {
                printf("filter index = %d\n", para->testfilter_index);
                printf
                    ("filter:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n",
                     buf[0], buf[1], buf[2], buf[3], buf[4], buf[6], buf[7], buf[8], buf[9], reg_val);
            }
        }
        else
            printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>filter%d:ReadWait Timeout\n", para->testfilter_index);
    }
}

static void Strong_Multi_Section(void)
{
    CSDEMUX_HANDLE hfilter0, hfilter1;
    CSDEMUX_HANDLE hpidfilter0, hpidfilter1;
    int i;
    unsigned char filter[12];
    unsigned char mask[12];
    unsigned short pid = 0;

    for (i = 0; i < CURRENT_SERVICE_NUM; i++) {
        hfilter0 = testfilter[i].filter_handle = CSDEMUX_Filter_Open(i);
        hpidfilter0 = testfilter[i].pidfilter_handle = CSDEMUX_PIDFT_Open(i);

        switch (i) {
        case 0:    //nit actual
            pid = 0x10;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x40;
            mask[0] = 0xff;
            break;
        case 1:    //nit other
            pid = 0x10;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x41;
            mask[0] = 0xff;
            break;
        case 2:    //bat
            pid = 0x11;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x4a;
            mask[0] = 0xff;
            break;
        case 3:    //sdt actual
            pid = 0x11;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x42;
            mask[0] = 0xff;
            break;
        case 4:    //sdt other
            //goto default1;
            pid = 0x11;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x46;
            mask[0] = 0xff;
            break;
        case 5:    //pat
            pid = 0x00;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x00;
            mask[0] = 0xff;
            break;
        case 6:    //cat
            pid = 0x01;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x01;
            mask[0] = 0xff;
            break;
        case 7:    //tdt
            pid = 0x14;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x70;
            mask[0] = 0xff;
            break;
        case 8:    //eit p/f actual
            pid = 0x12;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x4e;
            mask[0] = 0xff;
            break;
        case 9:    //eit p/f other
            pid = 0x12;    //0x12;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x4f;
            mask[0] = 0xff;
            break;
        case 10:    //eit schedule actual
            pid = 0x12;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x50;
            mask[0] = 0xf0;
            break;
        case 11:    //eit schedule other
            pid = 0x12;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x60;
            mask[0] = 0xf0;
            break;
        case 12:    //pmt
            pid = 0x401;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 13:    //pmt
            pid = 0x3f7;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 14:    //pmt
            pid = 0x43d;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 15:    //pmt
            pid = 0x433;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 16:    //pmt
            pid = 0x429;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 17:    //pmt
            pid = 0x41f;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 18:    //pmt
            pid = 0x41f;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 19:    //pmt
            pid = 0x415;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;
        case 20:    //pmt
            pid = 0x40b;
            memset(filter, 0, 12);
            memset(mask, 0, 12);
            filter[0] = 0x02;
            mask[0] = 0xff;
            break;

              default1:
        default:
            break;
        }

        CSDEMUX_PIDFT_SetChannel(hpidfilter0, DEMUX_CHL_ID0);
        CSDEMUX_PIDFT_SetPID(hpidfilter0, pid);

        CSDEMUX_Filter_SetFilter(hfilter0, filter, mask);
        CSDEMUX_Filter_AddPID(hfilter0, pid);
        CSDEMUX_Filter_SetFilterType(hfilter0, DEMUX_FILTER_TYPE_SEC);

        CSDEMUX_PIDFT_Enable(hpidfilter0);
        CSDEMUX_Filter_Enable(hfilter0);

#if 0                //test multi-thread
        {
            int error = 0;

            testfilter[i].testfilter_index = i;
            error = pthread_create(&read_thread, NULL, (void *) Read_Data, &testfilter[i]);
            printf("error %d\n", error);
            if (error != 0) {
                printf("CSSI_Nit_Thread  Create Fail \n");
            }
        }
    }
    while (i) {
    }
#else
    }
    unsigned int reg_val;
    unsigned char buf[1024];

    while (1) {
        for (i = 0; i < CURRENT_SERVICE_NUM; i++) {
            reg_val = 1024;
            memset(buf, 0, 1024);
            hfilter0 = testfilter[i].filter_handle;
            if (CSDEMUX_Filter_ReadWait(hfilter0, 500) == CSAPI_SUCCEED) {
                if (CSDEMUX_Filter_ReadSectionData(hfilter0, buf, &reg_val) == CSAPI_SUCCEED) {
                    printf("i = %d\n", i);
                    printf
                        ("filter:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n",
                         buf[0], buf[1], buf[2], buf[3], buf[4], buf[6], buf[7], buf[8], buf[9],
                         reg_val);
                    if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0)) {
                        printf
                            ("AAAAAAAAAAAAAAAAAAAAAAAAAA   %d   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n",
                             i);
                        return;
                    }
                }
            }
            else
                printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>filter%d:ReadWait Timeout\n", i);
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

static void Get_Multi_Section(void)
{
    CSDEMUX_HANDLE hfilter0, hfilter1;
    CSDEMUX_HANDLE hpidfilter0, hpidfilter1;

    unsigned char filter0[12];
    unsigned char filter1[12];
    unsigned char mask0[12];
    unsigned char mask1[12];

    memset(filter0, 0, 12);
    memset(mask0, 0, 12);

    memset(filter1, 0, 12);
    memset(mask1, 0, 12);

    filter0[0] = 0x42;
    mask0[0] = 0xff;
    //filter0[6] = 0x00;
    //mask0[6]   = 0xff;

    filter1[0] = 0x40;
    mask1[0] = 0xff;

    hfilter0 = CSDEMUX_Filter_Open(3);
    hfilter1 = CSDEMUX_Filter_Open(8);

    hpidfilter0 = CSDEMUX_PIDFT_Open(3);
    hpidfilter1 = CSDEMUX_PIDFT_Open(8);

    CSDEMUX_PIDFT_SetChannel(hpidfilter0, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetChannel(hpidfilter1, DEMUX_CHL_ID0);

    CSDEMUX_PIDFT_SetPID(hpidfilter0, 0x11);
    CSDEMUX_PIDFT_SetPID(hpidfilter1, 0x10);

    CSDEMUX_PIDFT_Enable(hpidfilter0);
    CSDEMUX_PIDFT_Enable(hpidfilter1);

    CSDEMUX_Filter_SetFilter(hfilter0, filter0, mask0);
    CSDEMUX_Filter_SetFilter(hfilter1, filter1, mask1);

    CSDEMUX_Filter_AddPID(hfilter0, 0x11);
    CSDEMUX_Filter_AddPID(hfilter1, 0x10);

    CSDEMUX_Filter_SetFilterType(hfilter0, DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_SetFilterType(hfilter1, DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_Enable(hfilter0);
    CSDEMUX_Filter_Enable(hfilter1);

    unsigned int reg_val;
    unsigned char buf[2048];

    while (1) {
        reg_val = 1024;
        memset(buf, 0, 2048);
        if (CSDEMUX_Filter_ReadWait(hfilter0, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadSectionData(hfilter0, buf, &reg_val) == CSAPI_SUCCEED) {
                printf
                    ("filter0:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n",
                     buf[0], buf[1], buf[2], buf[3], buf[4], buf[6], buf[7], buf[8], buf[9], reg_val);
                if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0)) {
                    printf
                        ("AAAAAAAAAAAAAAAAAAAAAAAAAA   1   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
            }
        }
        else
            printf("filter0:ReadWait Timeout\n");

        reg_val = 1024;
        memset(buf, 0, 2048);
        if (CSDEMUX_Filter_ReadWait(hfilter1, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadSectionData(hfilter1, buf + 1024, &reg_val) == CSAPI_SUCCEED) {
                printf
                    ("filter1:section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n",
                     buf[SECTION_LENGTN], buf[SECTION_LENGTN + 1], buf[SECTION_LENGTN + 2],
                     buf[SECTION_LENGTN + 3], buf[SECTION_LENGTN + 4], buf[SECTION_LENGTN + 6],
                     buf[SECTION_LENGTN + 7], buf[SECTION_LENGTN + 8], buf[SECTION_LENGTN + 9],
                     reg_val);
                if ((buf[SECTION_LENGTN] == 0) && (buf[SECTION_LENGTN + 1] == 0)
                    && (buf[SECTION_LENGTN + 2] == 0)) {
                    printf
                        ("AAAAAAAAAAAAAAAAAAAAAAAAAA   2   AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
            }
        }
        else
            printf("filter1:ReadWait Timeout\n");
    }

    CSDEMUX_PIDFT_Disable(hpidfilter0);
    CSDEMUX_Filter_Disable(hfilter0);
    CSDEMUX_PIDFT_Close(hpidfilter0);
    CSDEMUX_Filter_Close(hfilter0);

    CSDEMUX_PIDFT_Disable(hpidfilter1);
    CSDEMUX_Filter_Disable(hfilter1);
    CSDEMUX_PIDFT_Close(hpidfilter1);
    CSDEMUX_Filter_Close(hfilter1);
}

static void Get_Section(int pid_ft_id, int sec_ft_id, int pid, int tabid)
{
    CSDEMUX_HANDLE hfilter;
    CSDEMUX_HANDLE hpidfilter;

    unsigned char filter[12];
    unsigned char mask[12];

    memset(filter, 0, 12);
    memset(mask, 0, 12);

    filter[0] = tabid & 0xff;
    mask[0] = 0xff;
    // filter[6] = 0x00;
    // mask[6] = 0xff;

    hfilter = CSDEMUX_Filter_Open(sec_ft_id);

    hpidfilter = CSDEMUX_PIDFT_Open(pid_ft_id);

    CSDEMUX_PIDFT_SetChannel(hpidfilter, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(hpidfilter, pid);
    CSDEMUX_PIDFT_Enable(hpidfilter);

    CSDEMUX_Filter_SetFilter(hfilter, filter, mask);
    CSDEMUX_Filter_AddPID(hfilter, pid);
    CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_SEC);
    CSDEMUX_Filter_Enable(hfilter);

    unsigned int reg_val;
    unsigned char buf[MAX_SECTION_LENGTN];

    while (1) {
        reg_val = 1024;
        if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadSectionData(hfilter, buf, &reg_val) == CSAPI_SUCCEED) {
                printf
                    ("section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x, 0x%02x,0x%02x, 0x%02x, 0x%02x,  ReadDate length=%d \n",
                     buf[0], buf[1], buf[2], buf[3], buf[4], buf[6], buf[7], buf[8], buf[9], reg_val);
                if ((buf[0] == 0) && (buf[1] == 0) && (buf[2] == 0)) {
                    printf
                        ("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
                    return;
                }
            }
        }
        else
            printf("ReadWait Timeout\n");
    }

    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Close(hfilter);
}

static void Get_TS(int pidft_id, int ft_id, int pid)
{
    CSDEMUX_HANDLE hfilter;
    CSDEMUX_HANDLE hpidfilter;

    struct timeval start, end;
    int timeuse = 0;

    hfilter = CSDEMUX_Filter_Open(ft_id);
    hpidfilter = CSDEMUX_PIDFT_Open(pidft_id);

    CSDEMUX_PIDFT_SetChannel(hpidfilter, DEMUX_CHL_ID0);
    CSDEMUX_PIDFT_SetPID(hpidfilter, pid);
    CSDEMUX_PIDFT_Enable(hpidfilter);

    CSDEMUX_Filter_AddPID(hfilter, pid);
    CSDEMUX_Filter_SetFilterType(hfilter, DEMUX_FILTER_TYPE_PES);
    CSDEMUX_Filter_Enable(hfilter);

    unsigned int reg_val;
    unsigned char buf[4100];
    int num1 = 0, num2 = 0;
    gettimeofday(&start, NULL);

    while (1) {
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

        printf("num1 = %d, num2 = %d, timeuse = %d\n", num1, num2, timeuse);
        num1++;

        reg_val = 188 * 30;
        if (CSDEMUX_Filter_ReadWait(hfilter, 500) == CSAPI_SUCCEED) {
            if (CSDEMUX_Filter_ReadData(hfilter, buf, &reg_val) == CSAPI_SUCCEED) {
                printf("section  = 0x%02x, 0x%02x, 0x%02x,  0x%02x, 0x%02x , ReadDate Size = %d\n",
                       buf[0], buf[1], buf[2], buf[3], buf[4], reg_val);
                num2++;
            }
        }
        else
            printf("ReadWait Timeout\n");

        gettimeofday(&end, NULL);
        timeuse = end.tv_sec - start.tv_sec;
        if (timeuse == 60)
            break;

    }

    CSDEMUX_PIDFT_Disable(hpidfilter);
    CSDEMUX_Filter_Disable(hfilter);
    CSDEMUX_PIDFT_Close(hpidfilter);
    CSDEMUX_Filter_Close(hfilter);
}

static int TestWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    char buf[200];
    int i, y = 40;
    int key_num;
    static LOGFONT *my_logfont = NULL;
    static int show_flag = 0, mute_flag = 0;
    static RECT prc;

    static int aud_vol = 70;
    static int alpha_local = 0;
    static int flag = 0;

    if (my_logfont == NULL) {
        my_logfont = CreateLogFont(NULL, "song", "GB2312",
                       FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
                       FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 96, 0);
    }

    switch (message) {
    case MSG_CREATE:
        GetWindowRect(hWnd, &prc);
        return 0;
    case MSG_PAINT:
        hdc = BeginPaint(hWnd);
        SetBkMode(hdc, BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 100, 22, 22));
        //SelectFont(hdc,my_logfont);   // GetSystemFont(SYSLOGFONT_FIXED));
        memset(buf, 0, 200);
        for (i = 0; i < service_num; i++) {
            sprintf(buf, "%s, vpid: %d, apid: %d", service[i].servicename, service[i].servce_vidpid,
                service[i].servce_audpid);
            TextOut(hdc, 20, y, buf);
            y += 20;
        }
        EndPaint(hWnd, hdc);
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
                unsigned int saturation;
                CSTVOUT_Get (tve_handle,&saturation);
                printf("=========> saturation = %d\n", saturation);
                saturation+=10;
                CSTVOUT_SetSaturation(tve_handle,saturation);
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
                unsigned int saturation;
                CSTVOUT_GetSaturation(tve_handle,&saturation);
                printf("=========> saturation = %d\n", saturation);
                saturation-=10;
                CSTVOUT_SetSaturation(tve_handle,saturation);
            }
*/
        if (wParam >= SCANCODE_1 && wParam <= SCANCODE_0) {
            if (wParam == SCANCODE_0)
                key_num = 0;
            else
                key_num = wParam - SCANCODE_1 + 1;
            printf("get input key = %d\n", key_num);
            stop();
            play(key_num);
        }
        else if (wParam == 28) {
            if (show_flag) {
                errno = CSOSD_Enable(g_osd_handle);
                printf("CSOSD_Enable error %d\n", errno);
                //errno = CSOSD_Flip(g_osd_handle);
                //printf("CSOSD_Flip error %d\n", errno);
            }
            else {
                errno = CSOSD_Disable(g_osd_handle);
                printf("CSOSD_Disable error %d\n", errno);
                //errno = CSOSD_Flip(g_osd_handle);
                //printf("CSOSD_Flip error %d\n", errno);
            }
            show_flag = 1 - show_flag;
        }
        else if (SCANCODE_CURSORLEFT == wParam) {
            CSAUD_Volume vol;
            if (aud_vol >= 2)
                aud_vol -= 2;
            vol.front_left = aud_vol;
            vol.front_right = aud_vol;
            vol.rear_left = aud_vol;
            vol.rear_right = aud_vol;
            vol.center = aud_vol;
            vol.lfe = aud_vol;
            printf("Set Volume = %d\n", aud_vol);
            CSAUD_SetVolume(g_audio_handle, &vol);
        }
        else if (SCANCODE_CURSORRIGHT == wParam) {
            CSAUD_Volume vol;
            if (aud_vol < 90)
                aud_vol += 2;
            vol.front_left = aud_vol;
            vol.front_right = aud_vol;
            vol.rear_left = aud_vol;
            vol.rear_right = aud_vol;
            vol.center = aud_vol;
            vol.lfe = aud_vol;
            printf("Set Volume = %d\n", aud_vol);
            CSAUD_SetVolume(g_audio_handle, &vol);
            /*{
               CSVID_SequenceHeader hdr;
               CSVID_GetSequenceHeader(g_video_handle, &hdr);
               printf("w = %d, h = %d \n", hdr.w, hdr.h);
               } */
        }
        else if (30 == wParam)    //mute
        {
            if (mute_flag)
                CSAUD_EnableMute(g_audio_handle);
            else
                CSAUD_DisableMute(g_audio_handle);
            printf("audio mute status %d\n", mute_flag);
            mute_flag = 1 - mute_flag;
        }
        else if (32 == wParam)    //fav
        {
            if (mute_flag)
                CSAUD_Play(g_audio_handle);
            else
                CSAUD_Stop(g_audio_handle);
            mute_flag = 1 - mute_flag;
        }
        else if (35 == wParam)    //audio
        {
            flag = flag % 5;
            CSAUD_SetOutputChannel(g_audio_handle, flag);
            printf("audio set output channel %d\n", flag);
            flag++;
        }
        else if (18 == wParam)    //down
        {
#if 1                //test equalizer
            CSAUD_EqualizerConfig temp;
            flag = 3;    //flag%5;
            temp.equalizer_type = flag;
            temp.equalizer_band_weight[0] = 11;
            temp.equalizer_band_weight[1] = 11;
            temp.equalizer_band_weight[2] = 11;
            temp.equalizer_band_weight[3] = 11;
            temp.equalizer_band_weight[4] = 11;
            temp.equalizer_band_weight[5] = 11;
            temp.equalizer_band_weight[6] = 11;
            temp.equalizer_band_weight[7] = 11;
            temp.equalizer_band_weight[8] = 11;
            temp.equalizer_band_weight[9] = 11;
            CSAUD_EnableEqualizer(g_audio_handle);
            CSAUD_SetEqualizer(g_audio_handle, &temp);
            printf("audio set equalizer level %d\n", temp.equalizer_type);
            //flag++;
#else
            CSAUD_DisableMixer(g_audio_handle);
#endif
        }
        else if (33 == wParam)    //sat
        {
#if 0                //test start delay
            unsigned int delay_number = 10;
            CSAUD_SetStartDelay(g_audio_handle, delay_number);
            printf("audio set startdelay %d\n", delay_number);
#else                //test disable equalizer
            CSAUD_DisableEqualizer(g_audio_handle);
            printf("audio set equalizer level\n");
#endif

        }
        else if (59 == wParam)    //menu
        {
            unsigned int streamtype = 0;
            CSAUD_GetCodecType(g_audio_handle, &streamtype);
            printf("audio get streamtype is %d\n", streamtype);
        }
        else if (1 == wParam)    //exit
        {
            unsigned int bitrate = 0;
            CSAUD_SetSampleRate(g_audio_handle, 96000);
            printf("audio get current bitrate is %d\n", bitrate);
        }
        else if (46 == wParam)    //up
        {
            unsigned int samrate = 0;
            CSAUD_GetSampleRate(g_audio_handle, &samrate);
            printf("audio get current samrate is %d\n", samrate);
        }
        else if (23 == wParam)    //pause
        {
#if 0                //test interface CSAUD_GetVolume();
            CSAUD_Volume tempvol;
            tempvol.center = 16;
            tempvol.front_left = 55;
            CSAUD_GetVolume(g_audio_handle, &tempvol);
            printf("%d %d\n", tempvol.center, tempvol.front_left);
#else                //test interface CSAUD_WriteMixerBuffer();
            FILE *mixdata = NULL;
            unsigned char src[65504];
            int readlen = 0;
            CSAUD_MixerConfig mixer_config = { 250, AUD_SAMPLE_RATE_48KHZ };

            mixdata = fopen("/mnt/mj/mixdata", "rb");
            if (NULL == mixdata) {
                printf("Can't open file mixdata\n");
            }
            fseek(mixdata, 0, SEEK_SET);
            readlen = fread(src, 1, 65504, mixdata);
            printf("pointer is 0x%x ,read length is %d\n", src, readlen);
            CSAUD_WriteMixerBuffer(g_audio_handle, src, readlen);

            CSAUD_EnableMixer(g_audio_handle);
            CSAUD_SetMixer(g_audio_handle, &mixer_config);

            while (1) {
                readlen = fread(src, 1, 65500, mixdata);
                printf("pointer is 0x%x ,read length is %d\n", src, readlen);
                CSAUD_WriteMixerBuffer(g_audio_handle, src, readlen);
            }
#endif
        }
        else
            printf("get input key = %d\n", wParam);

        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        PostQuitMessage(hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain(int argc, const char *argv[])    //int main(void)
{
    int return_val = 0;
    struct timeval start, end;
    int timeuse = 0;
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    CSHDMI_Init();
#if 1
    while (tuner_lock(548, 6875, 1) < 0) {
        printf("TEST DEMUX-----------------> lock failed!\n");
    }
#endif

    if (CSDEMUX_Init())
        printf("CSDEMUX_Init Failed!\n");

    g_demux_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0);
    if (g_demux_chl_handle == CSDEMUX_UNVALID_HANDLE) {
        printf("Orion Xport Channel0 Open Failed ...\n");
        CSDEMUX_Terminate();
        return return_val;
    }

    if (CSDEMUX_CHL_SetInputMode(g_demux_chl_handle, DEMUX_INPUT_MOD_TUNER))
        printf("CSDEMUX_CHL_SetInputMode Failed! \n");    //DEMUX_INPUT_MOD_TUNER,DEMUX_INPUT_MOD_DMA,DEMUX_INPUT_MOD_DIRECT
    if (CSDEMUX_CHL_Enable(g_demux_chl_handle))
        printf("CSDEMUX_CHL_Enable Failed! \n");

#if 0
    Search_Channel();
    if (0 == fork()) {
        init();
        play(0);
#if 0                //test CSDEMUX_CHL,CSDEMUX_PIDFT,CSDEMUX_VID,CSDEMUX_AUD:Enable/Disable
        gettimeofday(&start, NULL);
        int flag1 = 0, flag2 = 0;
        while (1) {
            gettimeofday(&end, NULL);
            timeuse = end.tv_sec - start.tv_sec;
            //printf("second : %d\n",timeuse);
            if (((timeuse % 3) == 0) && (timeuse != 0)) {
                if (flag2 != timeuse) {
                    flag2 = timeuse;
                    if (flag1 == 0) {
                        printf(">>>>>>>>>>>>>>>>>>>>>>>>>CSDEMUX_PIDFT_Disable : %d\n",
                               timeuse);
                        //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_video_handle);
                        //CSDEMUX_PIDFT_Disable(g_demux_pidfilter_audio_handle);
                        //CSDEMUX_CHL_Disable(g_demux_chl_handle);
                        CSDEMUX_VID_Disable(g_demux_video_handle);
                        CSDEMUX_AUD_Disable(g_demux_audio_handle);
                        flag1 = 1;
                    }
                    else if (flag1 == 1) {
                        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<CSDEMUX_PIDFT_Enable : %d\n",
                               timeuse);
                        //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_video_handle);
                        //CSDEMUX_PIDFT_Enable(g_demux_pidfilter_audio_handle);
                        //CSDEMUX_CHL_Enable(g_demux_chl_handle);
                        CSDEMUX_VID_Enable(g_demux_video_handle);
                        CSDEMUX_AUD_Enable(g_demux_audio_handle);
                        flag1 = 0;
                    }
                }
            }
        }
#endif
    }
    else {
        //Get_TS(DEMUX_PIDFT_ID0,DEMUX_FILTER_ID0, 0x3fc);
        //Get_Section(DEMUX_PIDFT_ID1, DEMUX_FILTER_ID1, 0x11, 0x42);
        //Get_Multi_Section();
        Strong_Multi_Section();
        //Search_Channel();
    }
#else
    Search_Channel();
    init();
    play(0);

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Program List";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = 0;
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = TestWinProc;
    CreateInfo.lx = 90;
    CreateInfo.ty = 100;
    CreateInfo.rx = 620;
    CreateInfo.by = 380;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow(&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;
    ShowWindow(hMainWnd, SW_SHOWNORMAL);
    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    MainWindowThreadCleanup(hMainWnd);
#endif

    CSDEMUX_CHL_Close(g_demux_chl_handle);
    CSDEMUX_Terminate();
    return return_val;
}
