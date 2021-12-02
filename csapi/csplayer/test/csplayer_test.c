#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "global.h"
#include "csapi.h"
#include "csplayer.h"

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

	CSDEMUX_CHL_SetInputMode(demux_chl_handle, DEMUX_INPUT_MOD_DMA);
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

int main(void)
{
	CSPLAYER_HANDLE player_handle;
	CSPLAYER_InitParams init_params;
	CSPLAYER_PlayParams play_params;
   
	av_init();
	printf("av_init() is OK ...... \n");

	init_params.chl_handle = demux_chl_handle;
	if (NULL == (player_handle =CSPLAYER_Init(&init_params))) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	strcpy(play_params.filename, "/mnt/rec.ts");//stream/Panasonic_fanbingbing_mpeg2.ts");
	play_params.loop_play_flags = 1;
	if (CSAPI_FAILED == CSPLAYER_Start(player_handle, &play_params)) printf("Error: %s,%d .............\n", __FUNCTION__, __LINE__);

	set_program(0x3fc, 0x3fd); //(0x200, 0x1fff); //(0x3fc, 0x3fd);

	while(1){sleep(5);}

	return 0;
}
