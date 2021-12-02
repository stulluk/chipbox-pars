///////////////////////////////////////////////////////////////////////////////
// This is the sample program for CAT6611 driver usage.
///////////////////////////////////////////////////////////////////////////////
#include "linuxos.h"
#include "hdmitx.h"

////////////////////////////////////////////////////////////////////////////////
// EDID
////////////////////////////////////////////////////////////////////////////////
static _XDATA unsigned char EDID_Buf[128] ;
static RX_CAP _XDATA RxCapability ;
static BOOL bChangeMode = FALSE ;
_XDATA AVI_InfoFrame AviInfo;
_XDATA Audio_InfoFrame AudioInfo ;

////////////////////////////////////////////////////////////////////////////////
// Program utility.
////////////////////////////////////////////////////////////////////////////////
BOOL ParseEDID(void) ;
static BOOL ParseCEAEDID(BYTE *pCEAEDID) ;
void ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep) ;
void ConfigAudioInfoFrm(void) ;


#ifndef SUPPORT_SYNCEMB
_IDATA BYTE bInputColorMode = F_MODE_RGB444;
_IDATA BYTE bInputSignalType = 0 ;
// BYTE bInputSignalType = T_MODE_INDDR ; // for DDR mode input
#else
// BYTE bInputSignalType = T_MODE_SYNCEMB ; // for 16 bit sync embedded
_IDATA BYTE bInputColorMode = F_MODE_YUV422 ;
_IDATA BYTE bInputSignalType = T_MODE_SYNCEMB | T_MODE_CCIR656 ; // for 16 bit sync embedded
#endif // SUPPORT_SYNCEMB

_IDATA BYTE iVideoModeSelect=0 ;

_IDATA BYTE bOutputColorMode = F_MODE_YUV422; /* was F_MODE_RGB444 ; */
_XDATA ULONG VideoPixelClock ; 
_XDATA BYTE VIC ; // 480p60
_XDATA BYTE pixelrep ; // no pixelrepeating
_XDATA HDMI_Aspec aspec ;
_XDATA HDMI_Colorimetry Colorimetry ;

BOOL bHDMIMode, bAudioEnable ;

////////////////////////////////////////////////////////////////////////////////
// Function Body.
////////////////////////////////////////////////////////////////////////////////

void HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_Aspec asPecR, HDMI_OutputColorMode OutputColorMode);
void HDMITX_SetOutput(void) ;
void HDMITX_DevLoopProc(void) ;
extern void HDCP_Reset(void);
void
HDMITX_SetOutput(void)
{
	// printf("...........=======>>     HDMITX_SetOutput  !!!\n");
    #ifdef SUPPORT_SYNCEMB
    ProgramSyncEmbeddedVideoMode(VIC, bInputSignalType) ; // inf CCIR656 input
    #endif
    EnableVideoOutput(VideoPixelClock>80000000,bInputColorMode, bInputSignalType, bOutputColorMode,bHDMIMode) ;
    HDCP_Reset();//mingchih add
    if( bHDMIMode )
    {
        ConfigAVIInfoFrame(VIC, pixelrep) ;

        EnableHDCP(FALSE) ;//mingchih modify
		if( bAudioEnable )
		{
		    SetNonPCMAudio(0) ; // For LPCM audio. If AC3 or other compressed audio, please set the parameter as 1
            EnableAudioOutput(VideoPixelClock*(pixelrep+1),48000, 2, FALSE);
            ConfigAudioInfoFrm() ;
		}
    }
    SetAVMute(FALSE) ;
    bChangeMode = FALSE ;
}

void
HDMITX_DevLoopProc(void)
{
    BYTE HPD, HPDChange ;

    CheckHDMITX(&HPD,&HPDChange) ;
    if( HPDChange )
    {
		// printf("HDMITX_DevLoopProc : HPD Changed to [%d]\n", HPD);
		
        if( HPD )
        {
            ParseEDID() ;
            
            // 2008/09/19 modified by jj_tseng@chipadvanced.com
            bOutputColorMode = F_MODE_YUV422; /* was F_MODE_RGB444 ; */
            bOutputColorMode &= ~F_MODE_CLRMOD_MASK ; // color mode need to reset at first before DVI/HDMI/YUV check
			if( RxCapability.ValidHDMI )
			{
				bHDMIMode = TRUE ;
				
				if(RxCapability.VideoMode & (1<<6))
				{
					bAudioEnable = TRUE ;
				}
				
				if (RxCapability.VideoMode & (1<<4))
				{
					bOutputColorMode = F_MODE_YUV422 ;
				}
                if( RxCapability.VideoMode & (1<<5))
				{
					bOutputColorMode = F_MODE_YUV444;
				}
			}
			else
			{
				bHDMIMode = FALSE ;
				bAudioEnable = FALSE ;
				bOutputColorMode &= ~F_MODE_CLRMOD_MASK ; // color mode need to reset at first before DVI/HDMI/YUV check
				bOutputColorMode = F_MODE_RGB444 ;
			}
			//~jj_tseng@chipadvanced.com 2008/09/19
			//printf("HDMITX_DevLoopProc : bOutputColorMode [0x%02X]\n", bOutputColorMode);

            HDMITX_SetOutput() ;
#ifdef CAT6611_DRV_DEBUG_ON
            //DumpCat6611Reg();
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            
        }
        else
        {
            // unplug mode, ...
            DisableAudioOutput() ;
            DisableVideoOutput() ;
#ifdef CAT6611_DRV_DEBUG_ON
            //DumpCat6611Reg();
#endif // #ifdef CAT6611_DRV_DEBUG_ON
        }
    }
    else // no stable but need to process mode change procedure
    {
        if(bChangeMode && HPD)
        {
			// printf("HDMITX_DevLoopProc : Not HPDChange but bChangeMode\n");
            HDMITX_SetOutput() ;
#ifdef CAT6611_DRV_DEBUG_ON
            // DumpCat6611Reg();
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            
        }
    }
}


void
HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_Aspec asPecR, HDMI_OutputColorMode OutputColorMode)
{
   //HDMI_Video_Type  t=HDMI_480i60_16x9;
    switch(OutputVideoTiming)
	{
    case HDMI_640x480p60:
        VIC = 1 ;
        VideoPixelClock = 25000000 ;
        pixelrep = 0 ;
        // aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60:
        VIC = 2 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        // aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60_16x9:
        VIC = 3 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p60:
        VIC = 4 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080i60:
        VIC = 5 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_480i60:
        VIC = 6 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        // aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480i60_16x9:
        VIC = 7 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p60:
        VIC = 16 ;
        VideoPixelClock = 148500000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576p50:
        VIC = 17 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        // aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576p50_16x9:
        VIC = 18 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p50:
        VIC = 19 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080i50:
        VIC = 20 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576i50:
        VIC = 21 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        // aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576i50_16x9:
        VIC = 22 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p50:
        VIC = 31 ;
        VideoPixelClock = 148500000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p24:
        VIC = 32 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p25:
        VIC = 33 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p30:
        VIC = 34 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        // aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    default:
        bChangeMode = FALSE ;                
        return ;
    }

	aspec = asPecR ;

    if( RxCapability.ValidHDMI )
    {
        if( RxCapability.VideoMode & (1<<5))
            bOutputColorMode = F_MODE_YUV444 ;
        else if (RxCapability.VideoMode & (1<<4))
            bOutputColorMode = F_MODE_YUV422 ;
        else
            bOutputColorMode = F_MODE_YUV422; /* was F_MODE_RGB444 ; */
    /*
        switch(OutputColorMode)
        {
        case HDMI_YUV444:
            if( RxCapability.VideoMode & (1<<5))
                bOutputColorMode = F_MODE_YUV444 ;
            else if (RxCapability.VideoMode & (1<<4))
                bOutputColorMode = F_MODE_YUV422 ;
            else
                bOutputColorMode = F_MODE_RGB444 ;        
            break ;
        case HDMI_YUV422:
            if (RxCapability.VideoMode & (1<<4))
                bOutputColorMode = F_MODE_YUV422 ;
            else
                bOutputColorMode = F_MODE_RGB444 ;
            break ;
        case HDMI_RGB444:
        default:
            bOutputColorMode = F_MODE_RGB444 ;
            break ;
        }
        */
    }
    else
        bOutputColorMode = F_MODE_RGB444 ;

    if( Colorimetry == HDMI_ITU709 )
    {
        bInputColorMode |= F_VIDMODE_ITU709 ;
    }
    else
    {
        bInputColorMode &= ~F_VIDMODE_ITU709 ;
    }
    
    if( Colorimetry != HDMI_640x480p60)
    {
        bInputColorMode |= F_VIDMODE_16_235 ;
    }
    else
    {
        bInputColorMode &= ~F_VIDMODE_16_235 ;
    }

	/* For Test By KB Kim */
	// bInputColorMode |= F_VIDMODE_16_235 ;

    bChangeMode = TRUE ;
}


void
ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep)
{
//     AVI_InfoFrame AviInfo;

    AviInfo.pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80 ; 
    AviInfo.pktbyte.AVI_HB[1] = AVI_INFOFRAME_VER ; 
    AviInfo.pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN ; 
    
//     memset(&AviInfo.info,0,sizeof(AviInfo.info)) ;
// 
//     AviInfo.info.ActiveFmtInfoPresent = 1 ;
//     AviInfo.info.Scan = 0 ; // no ucData.
// 
//     AviInfo.info.BarInfo = 0 ; // no bar ucData valid
// 
//     if ( AviInfo.info.BarInfo )
//     {
//         // should give valid bar ucData.
//         AviInfo.info.Ln_End_Top = 0 ;
//         AviInfo.info.Ln_Start_Bottom = 0 ;
//         AviInfo.info.Pix_End_Left = 0 ;
//         AviInfo.info.Pix_Start_Right = 0 ;
//     }
// 
// 
    switch(bOutputColorMode)
    {
    case F_MODE_YUV444:
        // AviInfo.info.ColorMode = 2 ;
        AviInfo.pktbyte.AVI_DB[0] = (2<<5)|(1<<4) ;
        break ;
    case F_MODE_YUV422:
        // AviInfo.info.ColorMode = 1 ;
        AviInfo.pktbyte.AVI_DB[0] = (1<<5)|(1<<4) ;
        break ;
    case F_MODE_RGB444:
    default:
        // AviInfo.info.ColorMode = 0 ;
        AviInfo.pktbyte.AVI_DB[0] = (0<<5)|(1<<4) ;
        break ;
    }
// 
//     AviInfo.info.ActiveFormatAspectRatio = 8 ; // same as picture aspect ratio
    AviInfo.pktbyte.AVI_DB[1] = 8 ;
//     AviInfo.info.PictureAspectRatio = (aspec != HDMI_16x9)?1:2 ; // 4x3
    AviInfo.pktbyte.AVI_DB[1] |= (aspec != HDMI_16x9)?(1<<4):(2<<4) ; // 4:3 or 16:9
//     AviInfo.info.Colorimetry = (Colorimetry != HDMI_ITU709) ? 1:2 ; // ITU601
    AviInfo.pktbyte.AVI_DB[1] |= (Colorimetry != HDMI_ITU709)?(1<<6):(2<<6) ; // 4:3 or 16:9
//     AviInfo.info.Scaling = 0 ;
    AviInfo.pktbyte.AVI_DB[2] = 0 ;
//     AviInfo.info.VIC = VIC ;
    AviInfo.pktbyte.AVI_DB[3] = VIC ;
//     AviInfo.info.PixelRepetition = pixelrep;
    AviInfo.pktbyte.AVI_DB[4] =  pixelrep & 3 ;
    AviInfo.pktbyte.AVI_DB[5] = 0 ;
    AviInfo.pktbyte.AVI_DB[6] = 0 ;
    AviInfo.pktbyte.AVI_DB[7] = 0 ;
    AviInfo.pktbyte.AVI_DB[8] = 0 ;
    AviInfo.pktbyte.AVI_DB[9] = 0 ;
    AviInfo.pktbyte.AVI_DB[10] = 0 ;
    AviInfo.pktbyte.AVI_DB[11] = 0 ;
    AviInfo.pktbyte.AVI_DB[12] = 0 ;

    EnableAVIInfoFrame(TRUE, (unsigned char *)&AviInfo) ;
}



////////////////////////////////////////////////////////////////////////////////
// Function: ConfigAudioInfoFrm
// Parameter: NumChannel, number from 1 to 8
// Return: ER_SUCCESS for successfull.
// Remark: Evaluate. The speakerplacement is only for reference.
//         For production, the caller of SetAudioInfoFrame should program
//         Speaker placement by actual status.
// Side-Effect:
////////////////////////////////////////////////////////////////////////////////

void
ConfigAudioInfoFrm(void)
{
    int i ;
    ErrorF("ConfigAudioInfoFrm(%d)\n",2) ;
//    memset(&AudioInfo,0,sizeof(Audio_InfoFrame)) ;
//    
//    AudioInfo.info.Type = AUDIO_INFOFRAME_TYPE ;
//    AudioInfo.info.Ver = 1 ;
//    AudioInfo.info.Len = AUDIO_INFOFRAME_LEN ;

    AudioInfo.pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE ;
    AudioInfo.pktbyte.AUD_HB[1] = 1 ;
    AudioInfo.pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN ;
//    
//    // 6611 did not accept this, cannot set anything.
//    // AudioInfo.info.AudioCodingType = 1 ; // IEC60958 PCM 
//    AudioInfo.info.AudioChannelCount = 2 - 1 ;
    AudioInfo.pktbyte.AUD_DB[0] = 1 ;
    for( i = 1 ;i < AUDIO_INFOFRAME_LEN ; i++ )
    {
        AudioInfo.pktbyte.AUD_DB[i] = 0 ;
    }
//
//    /* 
//    CAT6611 does not need to fill the sample size information in audio info frame.
//    ignore the part.
//    */
// 
//    AudioInfo.info.SpeakerPlacement = 0x00 ;   //                     FR FL
//    AudioInfo.info.LevelShiftValue = 0 ;    
//    AudioInfo.info.DM_INH = 0 ;    
//
    EnableAudioInfoFrame(TRUE, (unsigned char *)&AudioInfo) ;
}



/////////////////////////////////////////////////////////////////////
// ParseEDID()
// Check EDID check sum and EDID 1.3 extended segment.
/////////////////////////////////////////////////////////////////////

BOOL
ParseEDID(void)
{
    // collect the EDID ucdata of segment 0
    BYTE CheckSum ;
    BYTE BlockCount ;
    BOOL err = TRUE;
    BOOL bValidCEA = FALSE ;
    int i ;

    RxCapability.ValidCEA  = FALSE;
    RxCapability.ValidHDMI = FALSE;    /* Add By River 12.03.2008 */
	
    GetEDIDData(0, EDID_Buf);


    for( i = 0, CheckSum = 0 ; i < 128 ; i++ )
    {
        CheckSum += EDID_Buf[i] ; CheckSum &= 0xFF ;
    }
	
			//Eep_Write(0x80, 0x80, EDID_Buf) ;
	if( CheckSum != 0 )
	{
		return FALSE ;
	}
	
	if( EDID_Buf[0] != 0x00 ||
	    EDID_Buf[1] != 0xFF ||
	    EDID_Buf[2] != 0xFF ||
	    EDID_Buf[3] != 0xFF ||
	    EDID_Buf[4] != 0xFF ||
	    EDID_Buf[5] != 0xFF ||
	    EDID_Buf[6] != 0xFF ||
	    EDID_Buf[7] != 0x00)
    {
        return FALSE ;
    }


    BlockCount = EDID_Buf[0x7E] ;

    if( BlockCount == 0 )
    {
        return TRUE ; // do nothing.
    }
    else if ( BlockCount > 4 )
    {
        BlockCount = 4 ;
    }
        	
     // read all segment for test
    for( i = 1 ; i <= BlockCount ; i++ )
    {
        err = GetEDIDData(i, EDID_Buf) ;

        if( err )
        {  
           if( !bValidCEA && EDID_Buf[0] == 0x2 && EDID_Buf[1] == 0x3 )
            {
                err = ParseCEAEDID(EDID_Buf) ;
                if( err )
                {
 
				    if(RxCapability.IEEEOUI==0x0c03)
				    {
				    	RxCapability.ValidHDMI = TRUE ;
				    	bValidCEA = TRUE ;
					}
				    else
				    {
				    	RxCapability.ValidHDMI = FALSE ;
				    }
				                   
                }
            }
        }
    }

    return err ;

}

static BOOL
ParseCEAEDID(BYTE *pCEAEDID)
{
    BYTE offset,End ;
    BYTE count ;
    BYTE tag ;
    word ParCount;

    if( pCEAEDID[0] != 0x02 || pCEAEDID[1] != 0x03 ) return ER_SUCCESS ; // not a CEA BLOCK.
    End = pCEAEDID[2]  ; // CEA description.
    RxCapability.VideoMode = pCEAEDID[3] ;

	RxCapability.VDOModeCount = 0 ;
    RxCapability.idxNativeVDOMode = 0 ;
    ParCount=0;
    for( offset = 4 ; offset < End ; )
    {
        ParCount++;
        tag = pCEAEDID[offset] >> 5 ;
        count = pCEAEDID[offset] & 0x1f ;
        switch( tag )
        {
        case 0x01: // Audio Data Block ;
            offset += count+1 ;
/*
            RxCapability.AUDDesCount = count/3 ;
            offset++ ;
            for( i = 0 ; i < RxCapability.AUDDesCount ; i++ )
            {
                RxCapability.AUDDes[i].uc[0] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[1] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[2] = pCEAEDID[offset++] ;
            }
*/
            break ;

        case 0x02: // Video Data Block ;
            offset += count+1 ;
            //RxCapability.VDOModeCount = 0 ;
            /*
            offset ++ ;
            RxCapability.idxNativeVDOMode = 0;
            for( i = 0 ; i < count ; i++ )
            {
            	BYTE VIC ;
            	VIC = pCEAEDID[offset] & (~0x80) ;
            	// if( FindModeTableEntryByVIC(VIC) != -1 )
            	{
	                RxCapability.VDOMode[RxCapability.VDOModeCount] = VIC ;
	                if( pCEAEDID[offset] & 0x80 )
	                {
	                    RxCapability.idxNativeVDOMode = (BYTE)RxCapability.VDOModeCount ;
	                    iVideoModeSelect = RxCapability.VDOModeCount ;
	                }

	                RxCapability.VDOModeCount++ ;
            	}   
                offset++;
            }  
            */
            /*
            for( i = 0 ; i < count ; i++ )
            {
                offset++;
            }
            RxCapability.VDOModeCount=0x00;
        	RxCapability.idxNativeVDOMode=0x00;
            RxCapability.VDOMode[RxCapability.VDOModeCount]=0x04;
            iVideoModeSelect = RxCapability.VDOModeCount ;
            */
            break ;

        case 0x03: // Vendor Specific Data Block ;
            offset ++ ;
            RxCapability.IEEEOUI = (ULONG)pCEAEDID[offset+2] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset+1] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset] ;
            offset += count ; // ignore the remaind.

            break ;

        case 0x04: // Speaker Data Block ;
            offset ++ ;
            RxCapability.SpeakerAllocBlk.uc[0] = pCEAEDID[offset] ;
            RxCapability.SpeakerAllocBlk.uc[1] = pCEAEDID[offset+1] ;
            RxCapability.SpeakerAllocBlk.uc[2] = pCEAEDID[offset+2] ;
            offset += 3 ;
            break ;
        case 0x05: // VESA Data Block ;
            offset += count+1 ;
            break ;
        case 0x07: // Extended Data Block ;
            offset += count+1 ; //ignore
            break ;
        default:
            offset += count+1 ; // ignore
        }
        if(ParCount>128)
            break;
    }
    RxCapability.ValidCEA = TRUE ;
    return TRUE ;
}

