#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "global.h"
#include "csapi.h"

#include "csrec.h"
#include "../cstuner/include/cs_tuner.h"
#include "../cssi/include/cssi.h"

static CSTVOUT_HANDLE tve_handle;

static CSDEMUX_HANDLE demux_chl_handle;
static CSDEMUX_HANDLE demux_vid_pidft;
static CSDEMUX_HANDLE demux_aud_pidft;

static CSDEMUX_HANDLE demux_vidout;
static CSDEMUX_HANDLE demux_audout;

static CSVID_HANDLE vid_handle;
static CSAUD_HANDLE aud_handle;

int av_init(void)
{
	/* TVE initializz. */
	if (NULL == (tve_handle = CSTVOUT_Open())) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSTVOUT_SetMode(tve_handle,TVOUT_MODE_576I)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	/* DEMUX initialize. */
	CSDEMUX_Init();
	if (NULL == (demux_chl_handle = CSDEMUX_CHL_Open(DEMUX_CHL_ID0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (NULL == (demux_vid_pidft = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (NULL == (demux_aud_pidft = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID1))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (NULL == (demux_vidout = CSDEMUX_VID_Open(DEMUX_VIDOUT_ID0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (NULL == (demux_audout = CSDEMUX_AUD_Open(DEMUX_AUDOUT_ID0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	CSDEMUX_CHL_SetInputMode(demux_chl_handle, DEMUX_INPUT_MOD_TUNER);
	CSDEMUX_CHL_Enable(demux_chl_handle);

	/* CSVID/CSAUD initialize. */
	if (NULL == (vid_handle = CSVID_Open(VID_DEV_0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (NULL == (aud_handle = CSAUD_Open(AUD_DEV_0))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	return 0;
}

void enable_tv(void)
{
	if (CSAPI_FAILED == CSVID_SetOutputAlpha(vid_handle,0xff)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSAUD_DisableMute(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
}

void disable_tv(void)
{

	if (CSAPI_FAILED == CSVID_SetOutputAlpha(vid_handle,0x00)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSAUD_EnableMute(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
}

void sync_notify(CSVID_HANDLE handle, signed char *ysnc_info)
{
	CSAUD_Volume vol;
	CSVID_SequenceHeader hdr;
	CSVID_Rect src_rect;
	CSVID_Rect dst_rect;

	CSVID_GetSequenceHeader(vid_handle, &hdr);

	dst_rect.left     =  0;
	dst_rect.top      =  0;
	dst_rect.right    =  720;
	dst_rect.bottom   =  576;
	src_rect.left     =  0;
	src_rect.top      =  0;
	src_rect.right    =  hdr.w;
	src_rect.bottom   =  hdr.h;

	CSVID_SetOutputPostion(vid_handle, &src_rect, &dst_rect);
	printf("%d,%d,%d,%d \n", 0, 0, hdr.w, hdr.h);

	vol.front_left  = 90;
	vol.front_right = 90;
	vol.rear_left   = 90;
	vol.rear_right  = 90;
	vol.center      = 90;
	vol.lfe         = 90;
	CSAUD_SetVolume(aud_handle, &vol);

	enable_tv();
}

void set_program(int vid_pid, int aud_pid)
{
	disable_tv();

	if (CSAPI_FAILED == CSAUD_Stop(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSVID_Stop(vid_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSDEMUX_PIDFT_Disable(demux_vid_pidft)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_PIDFT_Disable(demux_aud_pidft)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSDEMUX_VID_Disable(demux_vidout)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_AUD_Disable(demux_audout)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	// PID Filter Config
	if (CSAPI_FAILED == CSDEMUX_PIDFT_SetChannel(demux_vid_pidft, DEMUX_CHL_ID0)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_PIDFT_SetPID(demux_vid_pidft, vid_pid)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSDEMUX_PIDFT_SetChannel(demux_aud_pidft, DEMUX_CHL_ID0)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_PIDFT_SetPID(demux_aud_pidft, aud_pid)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);


	// VID Output Config
	if (CSAPI_FAILED == CSDEMUX_VID_SetOutputMode(demux_vidout, DEMUX_OUTPUT_MOD_BLOCK)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__); /* play file mode. */
	if (CSAPI_FAILED == CSDEMUX_VID_SetPID(demux_vidout, vid_pid)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	// AUD Output Config
	if (CSAPI_FAILED == CSDEMUX_AUD_SetOutputMode(demux_audout, DEMUX_OUTPUT_MOD_BLOCK)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_AUD_SetPID(demux_audout, aud_pid)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSAUD_Init(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSAUD_SetCodecType(aud_handle, AUD_STREAM_TYPE_MPA)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSAUD_EnablePTSSync(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSVID_SetStreamType(vid_handle, VID_STREAM_TYPE_MPEG2_TS)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSVID_EnablePTSSync(vid_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSDEMUX_VID_Enable(demux_vidout)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_AUD_Enable(demux_audout)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_PIDFT_Enable(demux_vid_pidft)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSDEMUX_PIDFT_Enable(demux_aud_pidft)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSVID_Play(vid_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);
	if (CSAPI_FAILED == CSAUD_Play(aud_handle)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	if (CSAPI_FAILED == CSVID_SetInputMode(vid_handle, VID_INPUT_STILLPIC)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	CSVID_SyncNotify(vid_handle, sync_notify, 3000000, 1);

	return;
}

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

int main(void)
{
	int return_val = 0;
	CSREC_HANDLE pvr_handle = 0;
	static CSREC_InitParams initpara;

	static CSSI_ServiceInfo service_info;

	while (tuner_lock(548, 6875, 1) < 0) {
		printf("TEST DEMUX-----------------> lock failed!\n");
	}

	av_init();
#if 0
	/* test CSSI. */
	{
		int ii;
		CSSI_InitParams si_initpara;

		for (ii = 0; ii < 16; ii++) {
			si_initpara.pidft_handle[ii] = CSDEMUX_PIDFT_Open(ii+3);
			CSDEMUX_PIDFT_SetChannel(si_initpara.pidft_handle[ii], DEMUX_CHL_ID0);

			si_initpara.secft_handle[ii] = CSDEMUX_Filter_Open(ii);
		}
		si_initpara.filter_num = 16;

		CSSI_HANDLE si_handle = CSSI_Init(&si_initpara);
		CSSI_GetServiceInfoFromStream(si_handle, 0x532/*102*/, &service_info);

		for (ii = 0; ii < 16; ii++) {
			CSDEMUX_PIDFT_Close(si_initpara.pidft_handle[ii]);
			CSDEMUX_Filter_Close(si_initpara.secft_handle[ii]);
		}
	}
#endif
	/* test CSREC. */
	strcpy(initpara.filename, "/mnt/rec.ts");
	initpara.pid_list[0].pid_val = service_info.vpid = 0x200;//video pid
	initpara.pid_list[0].pid_type = PID_TYPE_VIDEOTS;
	initpara.pid_list[1].pid_val = service_info.apid = 0x201;//audio pid
	initpara.pid_list[1].pid_type = PID_TYPE_AUDIOTS;
	initpara.pid_list[2].pid_val = 0x0;//pat pid
        initpara.pid_list[2].pid_type = PID_TYPE_PRIVATE;
        initpara.pid_list[3].pid_val = 0x401;//pmt pid
        initpara.pid_list[3].pid_type = PID_TYPE_PRIVATE;
	initpara.pid_num = 4;
 
	initpara.secft_handle = CSDEMUX_Filter_Open(DEMUX_FILTER_ID15);

	initpara.pidft_handle[0] = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID61);
	initpara.pidft_handle[1] = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID60);
	initpara.pidft_handle[2] = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID59);
	initpara.pidft_handle[3] = CSDEMUX_PIDFT_Open(DEMUX_PIDFT_ID58);
	initpara.pidft_num = 4;

	CSDEMUX_PIDFT_SetChannel(initpara.pidft_handle[0], DEMUX_CHL_ID0);
	CSDEMUX_PIDFT_SetChannel(initpara.pidft_handle[1], DEMUX_CHL_ID0);
	CSDEMUX_PIDFT_SetChannel(initpara.pidft_handle[2], DEMUX_CHL_ID0);
	CSDEMUX_PIDFT_SetChannel(initpara.pidft_handle[3], DEMUX_CHL_ID0);

	pvr_handle = CSREC_Init(&initpara);
	CSREC_Start(pvr_handle);

	set_program(initpara.pid_list[0].pid_val, initpara.pid_list[1].pid_val);

	while (1) {
		sleep(5);
	}
	
	return 0;
}
