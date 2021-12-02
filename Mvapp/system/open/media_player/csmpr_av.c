#include "global.h"
#include "demux.h"
#include "database.h"
#include "av_zapping.h"
#include "csmpr_common.h"
#include "csmpr_av.h"

CSDEMUX_HANDLE xport_chl1_handle = CSDEMUX_UNVALID_HANDLE;
CSDEMUX_HANDLE xport_pid_filter2_handle = CSDEMUX_UNVALID_HANDLE;
CSDEMUX_HANDLE xport_pid_filter3_handle = CSDEMUX_UNVALID_HANDLE;

extern CSVID_HANDLE   vid_handle;
extern CSAUD_HANDLE   aud_handle;
extern CSDEMUX_HANDLE xport_vidout_handle;
extern CSDEMUX_HANDLE xport_audout_handle;
extern BOOL           video_set_show;
extern void Sycro_call_back(CSVID_HANDLE *handle, signed char * temp);


CSMPR_RESULT CSMPR_OpenDemuxCH1( void )
{
    CSMPR_DBG_TRACE;
    
	//Open Channel 1
	xport_chl1_handle = CSDEMUX_CHL_Open( DEMUX_CHL_ID1 );
	if(xport_chl1_handle == CSDEMUX_UNVALID_HANDLE)
	{
		CSMPR_DBG_ERROR("!!!Error: open Channel1 error!!!\n");
		return( CSMPR_ERROR );
	}
    
	xport_pid_filter2_handle = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID20 );
	if( xport_pid_filter2_handle == CSDEMUX_UNVALID_HANDLE)
	{
		CSMPR_DBG_ERROR("!!!Error: CSDEMUX_PIDFT_Open error!!!\n");
		return( CSMPR_ERROR );
	}
    
	xport_pid_filter3_handle = CSDEMUX_PIDFT_Open( DEMUX_PIDFT_ID21 );
	if( xport_pid_filter3_handle == CSDEMUX_UNVALID_HANDLE)
	{
		CSMPR_DBG_ERROR("!!!Error: CSDEMUX_PIDFT_Open error!!!\n");
		return( CSMPR_ERROR );
	}
    
return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_CloseDemuxCH1( void )
{
    CSMPR_DBG_TRACE;
    
	if( xport_chl1_handle != CSDEMUX_UNVALID_HANDLE )
	{
		CSDEMUX_CHL_Disable( xport_chl1_handle );
		CSDEMUX_CHL_Close( xport_chl1_handle );
	}
    
	if( xport_pid_filter2_handle != CSDEMUX_UNVALID_HANDLE )
	{
		CSDEMUX_PIDFT_Disable( xport_pid_filter2_handle );
		CSDEMUX_PIDFT_Close( xport_pid_filter2_handle );
	}
    
	if( xport_pid_filter3_handle != CSDEMUX_UNVALID_HANDLE )
	{
		CSDEMUX_PIDFT_Disable( xport_pid_filter3_handle );
		CSDEMUX_PIDFT_Close( xport_pid_filter3_handle );
	}

return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_PlayAudio( tCS_AV_PlayParams ProgramInfo )
{
    CSMPR_DBG_TRACE;
   
    // CHECK PARAMETER VALIDATION
    if( ( aud_handle == NULL ) ||
        ( xport_pid_filter3_handle == CSDEMUX_UNVALID_HANDLE ) ||
        ( xport_audout_handle == CSDEMUX_UNVALID_HANDLE ))
    {
        return( CSMPR_ERROR );
    }

    if( ProgramInfo.Audio_PID == kDB_DEMUX_INVAILD_PID )
    {
        return( CSMPR_ERROR );
    }
    
    // MUTE AND STOP AUDIO
    CSAUD_EnableMute( aud_handle );
    // Stop decoding
    CSAUD_Stop( aud_handle );
    // Stop data filtering
    CSDEMUX_PIDFT_Disable( xport_pid_filter3_handle );
    CSDEMUX_AUD_Disable( xport_audout_handle );

    // SETUP AUDIO FILTER WITH SPECIFIED PID
    // Configure pid filter
    CSDEMUX_PIDFT_SetChannel( xport_pid_filter3_handle, DEMUX_CHL_ID1 );
    CSDEMUX_PIDFT_SetPID( xport_pid_filter3_handle, ProgramInfo.Audio_PID );    
    DB_DemuxSetAudPid( xport_pid_filter3_handle, ProgramInfo.Audio_PID );   
    // Configure audio demux
    CSDEMUX_AUD_SetOutputMode( xport_audout_handle, DEMUX_OUTPUT_MOD_BLOCK );

    CSDEMUX_AUD_SetPID( xport_audout_handle, ProgramInfo.Audio_PID );
    CSDEMUX_AUD_SetSwitchMode( xport_audout_handle, DEMUX_SWITCH_MOD_CHL1 );
    // Init audio codec
    CSAUD_Init( aud_handle );
    CSAUD_EnableMute( aud_handle );
    
    // LOAD CODEC
    switch( ProgramInfo.AudioType )
    {
      case eCS_AV_AUDIO_STREAM_DTS:
        if( CS_DBU_GetDefaultAudioType() == eCS_DBU_AUDIO_AC3 )
            CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
        else
            CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
        CSAUD_SetCodecType( aud_handle, AUD_STREAM_TYPE_DTS );
      break;
      case eCS_AV_AUDIO_STREAM_AC3:
        if( CS_DBU_GetDefaultAudioType() == eCS_DBU_AUDIO_AC3 )
            CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_SPDIFAC3 );
        else
            CSAUD_SetOutputDevice( aud_handle, AUD_OUTPUT_I2S_SPDIFPCM );
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

    // ENABLE FILTERRING
    // Enable pts sync.
    CSAUD_EnablePTSSync( aud_handle );

    // Set block mode
    CSAUD_SetInputMode( aud_handle, AUD_INPUT_BLOCK );

    // Start data filterring
    CSDEMUX_AUD_Enable( xport_audout_handle );
    CSDEMUX_PIDFT_Enable( xport_pid_filter3_handle );

    // START DECODING
    CSAUD_Play( aud_handle );
    
    // AFTER...
    AV_AudioSetVolume( CS_AV_AudioGetVolume() );
    AV_AudioSetStereoMode( ProgramInfo.AudioMode );

return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_PlayVideo( tCS_AV_PlayParams ProgramInfo )
{
    int i,j;
    
    CSMPR_DBG_TRACE;
   
    // CHECK PARAMETER VALIDATION
    if( ( vid_handle == NULL ) ||
        ( xport_pid_filter2_handle == CSDEMUX_UNVALID_HANDLE ) ||
        ( xport_vidout_handle == CSDEMUX_UNVALID_HANDLE ) )
    {
        return( CSMPR_ERROR );
    }

    if( ( ProgramInfo.Video_PID == kDB_DEMUX_INVAILD_PID ) && 
        ( ProgramInfo.Audio_PID != kDB_DEMUX_INVAILD_PID ) )
    {
        Audio_SetMuteStatus( CS_AV_Audio_GetMuteStatus() );
        return( CSMPR_ERROR );
    }

    // STOP AND BLANK VIDEO
    // CSVID_SetOutputAlpha( vid_handle, 0 );

    ////CSVID_Stop( vid_handle );
    
    // Stop filterring
    CSDEMUX_PIDFT_Disable( xport_pid_filter2_handle );
    CSDEMUX_VID_Disable( xport_vidout_handle );

    //for(i=0;i<1000;i++)
    //{
    //   for(j=0;j<1000;j++)
    //   {;}
    //}    

    // SETUP VIDEO FILTERRING WITH SPECIFIED PID
    // Configure video filter pid
    CSDEMUX_PIDFT_SetChannel( xport_pid_filter2_handle, DEMUX_CHL_ID1 );
    CSDEMUX_PIDFT_SetPID( xport_pid_filter2_handle, ProgramInfo.Video_PID );    
    DB_DemuxSetVidPid( xport_pid_filter2_handle, ProgramInfo.Video_PID );
    // Configure video demux
    CSDEMUX_VID_SetOutputMode( xport_vidout_handle, DEMUX_OUTPUT_MOD_BLOCK );
    CSDEMUX_VID_SetPID( xport_vidout_handle, ProgramInfo.Video_PID );
    CSDEMUX_VID_SetSwitchMode( xport_vidout_handle, DEMUX_SWITCH_MOD_CHL1 );

    // LOAD VIDEO CODEC
    if( ProgramInfo.VideoType == eCS_AV_VIDEO_STREAM_H264 )
    {
        CSVID_SetStreamType( vid_handle, VID_STREAM_TYPE_H264_TS );
        CSVID_SetErrorSkipMode( vid_handle, 0 );
    }
    else
    {
        CSVID_SetStreamType( vid_handle, VID_STREAM_TYPE_MPEG2_TS );
    }

    // SET BLOCK MODE
    CSVID_SetInputMode( vid_handle, VID_INPUT_STILLPIC );

    // SET SYNC MODE
    CSVID_WaitSync( vid_handle, 1 );
    CSVID_EnablePTSSync( vid_handle ); 
    // ENABLE FILTERING
    CSDEMUX_VID_Enable( xport_vidout_handle );
    CSDEMUX_PIDFT_Enable( xport_pid_filter2_handle );
    
    // START DECODING
    CSVID_Play( vid_handle );
    
    // SET CALLBACK FOR SYNC    
    CSVID_SyncNotify( vid_handle, Sycro_call_back, 5000*1000, 1 );
    video_set_show = TRUE;
    
return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_StopAudio( void )
{
    CSMPR_DBG_TRACE;
   
    if( aud_handle != NULL )
    {
        CSAUD_Stop( aud_handle );       
    }

    if( xport_pid_filter3_handle != CSDEMUX_UNVALID_HANDLE )
    {
        CSDEMUX_PIDFT_Disable( xport_pid_filter3_handle );
    }

    if( xport_audout_handle != NULL )
    {
        CSDEMUX_AUD_SetSwitchMode( xport_audout_handle, DEMUX_SWITCH_MOD_CHL0 );
    }
    
return( CSMPR_SUCCESS );
}


CSMPR_RESULT CSMPR_StopVideo( void )
{
    CSMPR_DBG_TRACE;
   
    video_set_show = FALSE;
    
    if( vid_handle != NULL )
    {
        CSVID_Stop( vid_handle );       
    }

    if( xport_pid_filter2_handle != CSDEMUX_UNVALID_HANDLE )
    {
        CSDEMUX_PIDFT_Disable( xport_pid_filter2_handle );
    }
    
    if( xport_vidout_handle != NULL )
    {
        CSDEMUX_VID_SetSwitchMode( xport_vidout_handle, DEMUX_SWITCH_MOD_CHL0 );
    }    
    
return( CSMPR_SUCCESS );
}

