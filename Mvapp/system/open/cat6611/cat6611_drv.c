#include "linuxos.h"
#include "hdmitx.h"


#define TIMEOUT_WAIT_AUTH MS(2000)

BYTE I2CADR = 0x98 ;
BYTE I2CDEV = 0 ;


////////////////////////////////////////////////////////////////////////////////
// General global variables
////////////////////////////////////////////////////////////////////////////////

_XDATA BYTE VideoFormat ;
//_XDATA BYTE bVideoInputType = 0 /* | T_MODE_CCIR656 | T_MODE_SYNCEMB | T_MODE_INDDR */ ; // for Sync Embedded, CCIR656, InputDDR


// _XDATA ULONG AudioSampleFreq = 48000 ;
// _XDATA BYTE AudioSWL = 24 ;
// _XDATA BYTE AudioChannel = 2 ;
_XDATA BYTE bOutputAudioMode = 0x41 ; // use full packet as default 
_XDATA BYTE bAudioChannelSwap = 0 ;
_XDATA BYTE bIntType = 0 ;
_XDATA BYTE bAudioEnableSetting = 0 ;
_XDATA BYTE bAudioPendingCounter = 0 ;
_XDATA ULONG currVideoPixelClock ;
// BOOL bHDMIMode = FALSE;
// BOOL bAudioEnable = FALSE ;
// BOOL bCheckHDCP = FALSE ;
// BOOL bEnableEncryption = FALSE ;
// BOOL bGetEDID = FALSE ;
static BOOL bNLPCM = FALSE ;
static BOOL bAuthenticated = FALSE ;
static BOOL bManualCheckRi = FALSE ;
static BOOL bPendingAudio = FALSE ;
static BOOL bPendingAudioAdjust = FALSE ;

_XDATA unsigned short ManualCheckRiCounter = 0 ;

_XDATA TXDEVINFO TXDev_Info[TXDEVCOUNT] ;

// Y,C,RGB offset
// for register 73~75
_CODE BYTE bCSCOffset_16_235[] =
{
    0x00, 0x80, 0x00
};

_CODE BYTE bCSCOffset_0_255[] =
{
    0x10, 0x80, 0x10
};

#ifdef SUPPORT_INPUTRGB
    _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] =
    {
        0xB2,0x04,0x64,0x02,0xE9,0x00,
        0x93,0x3C,0x18,0x04,0x56,0x3F,
        0x49,0x3D,0x9F,0x3E,0x18,0x04
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] =
    {
        0x09,0x04,0x0E,0x02,0xC8,0x00,
        0x0E,0x3D,0x84,0x03,0x6E,0x3F,
        0xAC,0x3D,0xD0,0x3E,0x84,0x03
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] =
    {
        0xB8,0x05,0xB4,0x01,0x93,0x00,
        0x49,0x3C,0x18,0x04,0x9F,0x3F,
        0xD9,0x3C,0x10,0x3F,0x18,0x04
    } ;

    _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] =
    {
        0xE5,0x04,0x78,0x01,0x81,0x00,
        0xCE,0x3C,0x84,0x03,0xAE,0x3F,
        0x49,0x3D,0x33,0x3F,0x84,0x03
    } ;
#endif

#ifdef SUPPORT_INPUTYUV

    _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] =
    {
        0x00,0x08,0x6A,0x3A,0x4F,0x3D,
        0x00,0x08,0xF7,0x0A,0x00,0x00,
        0x00,0x08,0x00,0x00,0xDB,0x0D
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] =
    {
        0x4F,0x09,0x81,0x39,0xDF,0x3C,
        0x4F,0x09,0xC2,0x0C,0x00,0x00,
        0x4F,0x09,0x00,0x00,0x1E,0x10
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] =
    {
        0x00,0x08,0x53,0x3C,0x89,0x3E,
        0x00,0x08,0x51,0x0C,0x00,0x00,
        0x00,0x08,0x00,0x00,0x87,0x0E
    } ;

    _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] =
    {
        0x4F,0x09,0xBA,0x3B,0x4B,0x3E,
        0x4F,0x09,0x56,0x0E,0x00,0x00,
        0x4F,0x09,0x00,0x00,0xE7,0x10
    } ;
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Prototype
////////////////////////////////////////////////////////////////////////////////
void InitCAT6611(void);
BOOL EnableVideoOutput(BOOL HiFreq, unsigned char inputColorMode, unsigned char inputVideoType, unsigned char outputColorMode, BOOL bHDMI) ;
BOOL EnableAudioOutput(unsigned long VideoPixelClock, unsigned long AudioSampleClock, int ChannelNumber, BOOL bSPDIF);
void DisableVideoOutput(void) ;
void DisableAudioOutput(void) ;
BOOL GetEDIDData(int EDIDBlockID, unsigned char *pEDIDData);
BOOL CheckHDMITX(BYTE *pHPD, BYTE *pHPDChange) ;
BOOL EnableAVIInfoFrame(BOOL bEnable, unsigned char *pAVIInfoFrame);
BOOL EnableAudioInfoFrame(BOOL bEnable, unsigned char *pAudioInfoFrame);
void SetAVMute(BOOL bEnable) ;
BOOL ProgramSyncEmbeddedVideoMode(BYTE VIC, BYTE bVideoInputType) ;
///////////////////////////////////////////////////////////
// Video Function
///////////////////////////////////////////////////////////
static void SetInputMode(BYTE InputColorMode, BYTE InputSignalType);
static void SetCSCScale(BYTE bInputMode,BYTE bOutputMode);
static void SetupAFE(BOOL HighFreqMode);
static void FireAFE(void);
///////////////////////////////////////////////////////////
// Audio Function
///////////////////////////////////////////////////////////
static SYS_STATUS SetAudioFormat(BYTE NumChannel, BYTE AudioEnable, ULONG SampleFreq, BYTE AudSWL, BYTE AudioCatCode);
static SYS_STATUS SetNCTS(ULONG PCLK, ULONG Fs);
static void SetAudioChannel(void) ;
static void AdjustAudioSampleClock(void) ;

///////////////////////////////////////////////////////////
// DDC
///////////////////////////////////////////////////////////
static void ClearDDCFIFO(void);
static void GenerateDDCSCLK(void) ;
static void AbortDDC(void);
///////////////////////////////////////////////////////////
// EDID
///////////////////////////////////////////////////////////
static SYS_STATUS ReadEDID(BYTE *pData, BYTE bSegment, BYTE offset, SHORT Count);
///////////////////////////////////////////////////////////
// InfoFrame
///////////////////////////////////////////////////////////
static SYS_STATUS SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame);
static SYS_STATUS SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);
///////////////////////////////////////////////////////////
// HDCP Authentication
///////////////////////////////////////////////////////////
static SYS_STATUS HDCP_EnableEncryption(void);
static void HDCP_Auth_Fire(void);
static void HDCP_StartAnCipher(void);
static void HDCP_GenerateAn(void);
static SYS_STATUS HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus);
static SYS_STATUS HDCP_GetBKSV(BYTE *pBKSV);
static SYS_STATUS HDCP_Authenticate(void);
static void HDCP_ResumeAuthentication(void);
static void HDCP_ReadRi(void) ;

static void HDCP_ResetAuthenticate(void) ;
static void HDCP_StartAnCipher(void) ;
static void HDCP_StopAnCipher(void) ;
static SYS_STATUS HDCP_GetVr(BYTE *pVr) ;
static SYS_STATUS HDCP_Authenticate_Repeater(void) ;
static SYS_STATUS HDCP_VerifyIntegration(void) ;
static SYS_STATUS HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream) ;
static SYS_STATUS HDCP_CheckSHA(BYTE M0[],USHORT BStatus,BYTE KSVList[],int devno,BYTE Vr[]) ;
void HDCP_Reset(void) ;


#ifdef CAT6611_DRV_DEBUG_ON
void DumpCat6611Reg(void) ;
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Body.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// External Interface                                                         //
////////////////////////////////////////////////////////////////////////////////

void InitCAT6611(void)
{
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("InitCAT6611():\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON

    CAT6611_I2CInit(I2CADR);
	
    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_INT_CTRL,B_INTPOL_ACTL | B_INTPOL_ACTH) ; // depends on the interrupt type
    // bIntPOL = (bIntType&B_INTPOL_ACTH)?TRUE:FALSE ;

    HDMITX_WriteI2C_Byte(REG_SW_RST, B_REF_RST|B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;
    DelayMS(10) ;
    HDMITX_WriteI2C_Byte(REG_SW_RST, B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;

    // Avoid power loading in un play status.
    HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,B_AFE_DRV_RST/*|B_AFE_DRV_PWD*/) ;

    // set interrupt mask, mask value 0 is interrupt available.
    HDMITX_WriteI2C_Byte(REG_INT_MASK1, 0xF8) ;
    HDMITX_WriteI2C_Byte(REG_INT_MASK2, 0xF8) ;
    HDMITX_WriteI2C_Byte(REG_INT_MASK3, 0xF7) ;

    
    DISABLE_NULL_PKT();
    DISABLE_ACP_PKT();
    DISABLE_ISRC1_PKT();
    DISABLE_ISRC2_PKT();
    DISABLE_AVI_INFOFRM_PKT();
    DISABLE_AUD_INFOFRM_PKT();
    DISABLE_SPD_INFOFRM_PKT();
    DISABLE_MPG_INFOFRM_PKT();
}

#ifdef SUPPORT_SYNCEMB

// sync embedded table setting, defined as comment.
struct SyncEmbeddedSetting {
    BYTE fmt ;
    BYTE RegHVPol ; // Reg90
	BYTE RegHfPixel ; // Reg91
    BYTE RegHSSL ; // Reg95
    BYTE RegHSEL ; // Reg96
    BYTE RegHSH ; // Reg97
    BYTE RegVSS1 ; // RegA0
    BYTE RegVSE1 ; // RegA1
    BYTE RegVSS2 ; // RegA2
    BYTE RegVSE2 ; // RegA3
	BYTE REGHVPol656; // Reg90 when CCIR656
	BYTE REGHfPixel656; //Reg91 when CCIR656
	BYTE RegHSSL656 ; // Reg95 when CCIR656
    BYTE RegHSEL656 ; // Reg96 when CCIR656
    BYTE RegHSH656 ; // Reg97 when CCIR656
    unsigned long PCLK ;
    BYTE VFreq ;
} ;

static _CODE struct SyncEmbeddedSetting SyncEmbTable[] = {

#if 0
// {FMT,0x90, 0x91,
 //   VIC ,0x90, 0x91, 0x95, 0x96,0x97, 0xA0, 0xA1, 0xA2, 0xA3, 0x90_CCIR8,0x91_CCIR8    ,0x95_CCIR8,0x960_CCIR8   ,0x97_CCIR8  ,  PCLK    ,VFREQ},
    {   1 ,0xF0, 0x31, 0x0E, 0x6E,0x00, 0x0A, 0xC0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0xDD          ,0X00            , 25175000,60},
    {   2 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   3 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   4 ,0x76, 0x33, 0x6c, 0x94,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x76      ,0x33          ,0xDA      ,0x2A          ,0x10            , 74175000,60},
    {   5 ,0x26, 0x4A, 0x56, 0x82,0x00, 0x02, 0x70, 0x34, 0x92, 0x46      ,0x94          ,0x57      ,0x83          ,0x00            , 74175000,60},
    {   6 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   7 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   8 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   9 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {  10 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  11 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  12 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  13 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  14 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
    {  15 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
    {  16 ,0x06, 0xff, 0x56, 0x82,0x00, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xff          ,0xFF      ,0xFF          ,0xFF            ,148350000,60},
    {  17 ,0xA0, 0x1B, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  18 ,0x00, 0xff, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  19 ,0x46, 0x59, 0xB6, 0xDE,0x11, 0x05, 0xA0, 0xFF, 0xFF, 0x06      ,0xFF          ,0x6D      ,0xBD          ,0x33            , 74250000,50},
    {  20 ,0x66, 0x73, 0x0e, 0x3A,0x22, 0x02, 0x70, 0x34, 0x92, 0xC6      ,0xE6          ,0x1D      ,0x75          ,0x44            , 74250000,50},
    {  21 ,0xA0, 0x1B, 0x0A, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  22 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  23 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00         , 27000000,50},
    {  24 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00         , 27000000,50},
    {  25 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00         , 54000000,50},
    {  26 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00         , 54000000,50},
    {  27 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00         , 54000000,50},
    {  28 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00         , 54000000,50},
    {  29 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10         , 54000000,50},
    {  30 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10         , 54000000,50},
    {  31 ,0x06, 0xff, 0x0e, 0x3a,0x22, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xFF          ,0xFF      ,0xFF          ,0xFF         ,148500000,50},
    { 0xFF,0xFF, 0xff, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF      ,0xFF          ,0xFF      ,0xFF          ,0xFF            , 0       , 0}

#else
	
 // {FMT,0x90, 0x91,
 //   VIC ,0x90, 0x91, 0x95, 0x96,0x97, 0xA0, 0xA1, 0xA2, 0xA3, 0x90_CCIR8,0x91_CCIR8    ,0x95_CCIR8,0x960_CCIR8   ,0x97_CCIR8  ,  PCLK    ,VFREQ},
    {   1 ,0xF0, 0x31, 0x0E, 0x6E,0x00, 0x0A, 0xC0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0xDD          ,0X00            , 25175000,60},
//    {   2 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
//    {   3 ,0xF0, 0x31, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
//    {   4 ,0x76, 0x33, 0x6c, 0x94,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x76      ,0x33          ,0xDA      ,0x2A          ,0x10            , 74175000,60},
    {   2 ,0xE0, 0x00, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0x09, 0xF0, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   3 ,0xE0, 0x00, 0x0E, 0x4c,0x00, 0x09, 0xF0, 0x09, 0xF0, 0xF0      ,0x31          ,0x1D      ,0x99          ,0x00            , 27000000,60},
    {   4 ,0xc6, 0x06, 0x6c, 0x94,0x00, 0x05, 0xA0, 0x05, 0xA0, 0x76      ,0x33          ,0xDA      ,0x2A          ,0x10            , 74175000,60},
    {   5 ,0x26, 0x4A, 0x56, 0x82,0x00, 0x02, 0x70, 0x34, 0x92, 0x46      ,0x94          ,0x57      ,0x83          ,0x00            , 74175000,60},
    {   6 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   7 ,0xE0, 0x1B, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   8 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {   9 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xFF          ,0x23      ,0x9F          ,0x00            , 27000000,60},
    {  10 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  11 ,0xe0, 0x1b, 0x11, 0x4F,0x00, 0x04, 0x70, 0x0A, 0xD1, 0xE0      ,0x37          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  12 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  13 ,0x00, 0xff, 0x11, 0x4F,0x00, 0x04, 0x70, 0xFF, 0xFF, 0x00      ,0xff          ,0x23      ,0x9F          ,0x00            , 54000000,60},
    {  14 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
    {  15 ,0x00, 0xff, 0x1e, 0x9A,0x00, 0x09, 0xF0, 0xFF, 0xFF, 0x00      ,0xff          ,0x3D      ,0x35          ,0x10            , 54000000,60},
    {  16 ,0x06, 0xff, 0x56, 0x82,0x00, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xff          ,0xFF      ,0xFF          ,0xFF            ,148350000,60},
//    {  17 ,0xA0, 0x1B, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
//    {  18 ,0x00, 0xff, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x95          ,0x00            , 27000000,50},
//    {  19 ,0x46, 0x59, 0xB6, 0xDE,0x11, 0x05, 0xA0, 0xFF, 0xFF, 0x06      ,0xFF          ,0x6D      ,0xBD          ,0x33            , 74250000,50},
    {  17 ,0xA0, 0x00, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0x05, 0xA0, 0x50      ,0x01          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  18 ,0xA0, 0x00, 0x0a, 0x4A,0x00, 0x05, 0xA0, 0x05, 0xA0, 0x50      ,0x01          ,0x15      ,0x95          ,0x00            , 27000000,50},
    {  19 ,0x66, 0x1B, 0xB6, 0xDE,0x11, 0x05, 0xA0, 0x05, 0xA0, 0xD6      ,0x36          ,0x6D      ,0xBD          ,0x33            , 74250000,50},
    
    {  20 ,0x66, 0x73, 0x0e, 0x3A,0x22, 0x02, 0x70, 0x34, 0x92, 0xC6      ,0xE6          ,0x1D      ,0x75          ,0x44            , 74250000,50},
    {  21 ,0xA0, 0x1B, 0x0A, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  22 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00            , 27000000,50},
    {  23 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00	        , 27000000,50},
    {  24 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00	        , 27000000,50},
    {  25 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00	        , 54000000,50},
    {  26 ,0xA0, 0x1B, 0x0a, 0x49,0x00, 0x02, 0x50, 0x3A, 0xD1, 0x50      ,0x37          ,0x15      ,0x93          ,0x00	        , 54000000,50},
    {  27 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00	        , 54000000,50},
    {  28 ,0x00, 0xff, 0x0a, 0x49,0x00, 0x02, 0x50, 0xFF, 0xFF, 0x00      ,0xFF          ,0x15      ,0x93          ,0x00	        , 54000000,50},
    {  29 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10	        , 54000000,50},
    {  30 ,0x04, 0xff, 0x16, 0x96,0x00, 0x05, 0xA0, 0xFF, 0xFF, 0x00      ,0xFF          ,0x2D      ,0x2D          ,0x10	        , 54000000,50},
    {  31 ,0x06, 0xff, 0x0e, 0x3a,0x22, 0x04, 0x90, 0xFF, 0xFF, 0x00      ,0xFF          ,0xFF      ,0xFF          ,0xFF	        ,148500000,50},
    { 0xFF,0xFF, 0xff, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF      ,0xFF          ,0xFF      ,0xFF          ,0xFF            , 0       , 0}
#endif
} ;

                 // 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA0,0xA1,0xA2,0xA3
static _CODE BYTE DEGen480i[] = {0x89,0x35,0xee,0x8e,0x60,0xB3,0x7B,0x06,0x0C,0x02,0x11,0x01,0x10,0x18,0x08,0x21,0x0C,0x22,0x06,0x91} ;
static _CODE BYTE DEGen576i[] = {0xD9,0x35,0x06,0xA6,0x61,0xBD,0x7B,0x06,0x0C,0x02,0x16,0x36,0x10,0x4F,0x6F,0x21,0x00,0x30,0x39,0xC1} ;
static _CODE BYTE DEGen480p[] = {0x39,0xFF,0xEE,0x8E,0x60,0xAE,0x75,0x06,0x0C,0x02,0x24,0x04,0x20,0x23,0x36,0x00,0x00,0x60,0x35,0xff} ;
static _CODE BYTE DEGen576p[] = {0x39,0xFF,0x06,0xA6,0x61,0xBE,0x7E,0x06,0x0C,0x02,0x2C,0x6C,0x20,0x23,0x36,0x00,0x00,0x50,0x35,0xff} ;

BOOL 
ProgramSyncEmbeddedVideoMode(BYTE VIC, BYTE bInputSignalType)
{
    int i ;
    // if Embedded Video, need to generate timing with pattern register
    BOOL  rtn = TRUE;

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("ProgramSyncEmbeddedVideoMode(%d , %d)\n",VIC, bInputSignalType) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON

	if(bInputSignalType & T_MODE_SYNCEMB )
    {
      
        for( i = 0 ; SyncEmbTable[i].fmt != 0xFF ; i++ ) 
        {
            if( VIC == SyncEmbTable[i].fmt )
            {
                break ;
            }
        }
    
        if( SyncEmbTable[i].fmt == 0xFF )
        {
            return FALSE ;
        }
    
    //    HDMITX_SetReg_Byte(REG_HVPol,~0x6,(SyncEmbTable[i].RegHVPol)&6) ; // Reg90
        Switch_HDMITX_Bank(0) ;
    	
        if(bInputSignalType & T_MODE_CCIR656) {
#ifdef CAT6611_DRV_DEBUG_ON
    		ErrorF("Embedded,  CCIR8\n");
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_Byte(REG_HVPol,SyncEmbTable[i].REGHVPol656) ; // Reg90
            HDMITX_WriteI2C_Byte(REG_HfPixel,SyncEmbTable[i].REGHfPixel656) ; // Reg91
    		HDMITX_WriteI2C_Byte(REG_HSSL,SyncEmbTable[i].RegHSSL656) ; // Reg95
    		HDMITX_WriteI2C_Byte(REG_HSEL,SyncEmbTable[i].RegHSEL656) ; // Reg96
            HDMITX_WriteI2C_Byte(REG_HSH,SyncEmbTable[i].RegHSH656) ;   // Reg97
    	}
    	else 
    	{
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Embedded,  CCIR16\n"); 
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_Byte(REG_HVPol,SyncEmbTable[i].RegHVPol) ; // Reg90
            HDMITX_WriteI2C_Byte(REG_HfPixel,SyncEmbTable[i].RegHfPixel) ; // Reg91
    		HDMITX_WriteI2C_Byte(REG_HSSL,SyncEmbTable[i].RegHSSL) ; // Reg95
    		HDMITX_WriteI2C_Byte(REG_HSEL,SyncEmbTable[i].RegHSEL) ; // Reg96
    		HDMITX_WriteI2C_Byte(REG_HSH,SyncEmbTable[i].RegHSH) ; // Reg97
    	}
        HDMITX_WriteI2C_Byte(REG_VSS1,SyncEmbTable[i].RegVSS1) ; // RegA0
        HDMITX_WriteI2C_Byte(REG_VSE1,SyncEmbTable[i].RegVSE1) ; // RegA1 
        HDMITX_WriteI2C_Byte(REG_VSS2,SyncEmbTable[i].RegVSS2) ; // RegA2
        HDMITX_WriteI2C_Byte(REG_VSE2,SyncEmbTable[i].RegVSE2) ; // RegA3
    }
    else if( bInputSignalType & T_MODE_CCIR656 )
    {
        BYTE *pTimingGen ;
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Generate Sync and DE by Sync\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON

        switch(VIC)
        {
        case 2:
        case 3:
            // 480p
            pTimingGen = DEGen480p ;
            break ;
        case 6:
        case 7:
            // 480i
            pTimingGen = DEGen480i ;
            break ;
        case 17:
        case 18:
            // 576p
            pTimingGen = DEGen576p ;
            break ;
        case 21:
        case 22:
            // 576i
            pTimingGen = DEGen576i ;
            break ;
        default:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Not a supported timing for VIC %d to gen sync/DE from sync.\n",VIC) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            return FALSE ;                    
        }
        
        Switch_HDMITX_Bank(0) ;
        for( i = 0x90 ; i < 0xA4 ; i++ )
        {
            HDMITX_WriteI2C_Byte(i,pTimingGen[i-0x90]) ; // Reg90 ~ RegA3
        }
    }

    return TRUE ;
}
#endif

static void
SetHDMIMode(BOOL bHDMIMode)
{
    Switch_HDMITX_Bank(0) ;
    if(bHDMIMode)
    {
        HDMITX_WriteI2C_Byte(REG_HDMI_MODE,B_HDMI_MODE) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_HDMI_MODE,B_DVI_MODE) ;

        DISABLE_NULL_PKT();
        DISABLE_ACP_PKT();
        DISABLE_ISRC1_PKT();
        DISABLE_ISRC2_PKT();
        DISABLE_AVI_INFOFRM_PKT();
        DISABLE_AUD_INFOFRM_PKT();
        DISABLE_SPD_INFOFRM_PKT();
        DISABLE_MPG_INFOFRM_PKT();
        Switch_HDMITX_Bank(1) ;
        HDMITX_WriteI2C_Byte(REG_AVIINFO_DB1,0) ;
    }
    Switch_HDMITX_Bank(0) ;
}

BOOL
EnableVideoOutput(BOOL HiFreq, unsigned char inputColorMode, unsigned char inputSignalType, unsigned char outputColorMode, BOOL bHDMI)
{
    // bInputVideoMode, bOutputVideoMode, bVideoInputType, bAudioInputType, should be configured by upper F/W or loaded from EEPROM.
    // should be configured by initsys.c
    int i ;

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("EnableVideoOutput()\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON

    HDMITX_WriteI2C_Byte(REG_SW_RST, B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;
    
    TXDev_Info[I2CDEV].bHDMIMode = bHDMI ;
    
    if( bHDMI )
    {
        SetAVMute(TRUE) ;
    }

    SetInputMode(inputColorMode,inputSignalType) ;

    SetCSCScale(inputColorMode,outputColorMode) ;

    //if( inputVideoType & T_MODE_SYNCEMB )
    //{
    //    ProgramSyncEmbeddedVideoMode(VIC, inputVideoType) ;
    //}
    
//     if(bHDMI)
//     {
#ifdef CAT6611_DRV_DEBUG_ON
//         ErrorF("InitVideoMode(): SetHDMIMode\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
//         HDMITX_WriteI2C_Byte(REG_HDMI_MODE,B_HDMI_MODE) ;
//     }
//     else
//     {
#ifdef CAT6611_DRV_DEBUG_ON
//         ErrorF("InitVideoMode(): SetDVIMode\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
//         HDMITX_WriteI2C_Byte(REG_HDMI_MODE,B_DVI_MODE) ;
//     }
    SetHDMIMode(bHDMI) ;

    SetupAFE(HiFreq) ; // pass if High Freq request    
    
    for(i = 0 ; i < 100 ; i++)
    {
        if( HDMITX_ReadI2C_Byte(REG_SYS_STATUS) & B_TXVIDSTABLE )
        {
            break ;
        }
        DelayMS(1) ;
    }
    FireAFE() ;
	return TRUE ;
}

BOOL
EnableAudioOutput(unsigned long VideoPixelClock, unsigned long AudioSampleClock, int AudioChannel, BOOL bSPDIF)
{
    BYTE bAudioChannelEnable ;
    currVideoPixelClock = VideoPixelClock ;    
    switch(AudioChannel)
    {
    case 7:
    case 8:
        bAudioChannelEnable = 0xF ;
        break ;        
    case 6:
    case 5:
        bAudioChannelEnable = 0x7 ;
        break ;        
    case 4:
    case 3:
        bAudioChannelEnable = 0x3 ;
        break ;        
    case 2:
    case 1:
    default:
        bAudioChannelEnable = 0x1 ;
        break ;        
    }
    
    if(bSPDIF) bAudioChannelEnable |= B_AUD_SPDIF ;
    SetNCTS(VideoPixelClock, AudioSampleClock) ;
    SetAudioFormat(AudioChannel,bAudioChannelEnable,AudioSampleClock,24,0) ;
    // ConfigAudioInfoFrm(AudioChannel) ;
    // HDMITX_AndReg_Byte(REG_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;
#ifdef CAT6611_DRV_DEBUG_ON
    // DumpCat6611Reg();
#endif // #ifdef CAT6611_DRV_DEBUG_ON
    return TRUE ;    
}

void DisableVideoOutput(void)
{
    HDMITX_WriteI2C_Byte(REG_SW_RST, B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;
    HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL, B_AFE_DRV_PWD|B_AFE_DRV_ENBIST) ;
}

void DisableAudioOutput(void)
{
    BYTE uc ;
    
    Switch_HDMITX_Bank(0) ;
    
    uc = HDMITX_ReadI2C_Byte(REG_AUDIO_CTRL0) ;
    uc = 0xC0 ;
    HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL0, uc) ;
    uc = HDMITX_ReadI2C_Byte(REG_SW_RST) ;
    HDMITX_WriteI2C_Byte(REG_SW_RST, uc|B_AUD_RST|B_AREF_RST) ;
    bPendingAudio = FALSE ;
    bPendingAudioAdjust = FALSE ;

}

BOOL 
GetEDIDData(int EDIDBlockID, unsigned char *pEDIDData)
{
    if( ReadEDID(pEDIDData,EDIDBlockID/2,(EDIDBlockID%2)*128,128) == ER_FAIL )
    {
        return FALSE ;
    }
    
    return TRUE ;
}

BOOL
EnableHDCP(BOOL bEnable)
{
    if( bEnable )
    {
        if( HDCP_Authenticate() == ER_FAIL )
        {
            return FALSE ;
        }
    }
    else
    {
        HDMITX_OrReg_Byte(REG_SW_RST,B_HDCP_RST) ;
        HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE, 0) ;
    }
    
    return TRUE ;
}

BOOL
CheckHDMITX(BYTE *pHPD, BYTE *pHPDChange) 
{
    BYTE intdata1, intdata2, intdata3, sysstat;
    BYTE  intclr3 = 0 ;
    static BOOL PrevHPD = FALSE ;
    BOOL HPD ;
    
    sysstat = HDMITX_ReadI2C_Byte(REG_SYS_STATUS) ;
    //printf("sysstat=0x%x\n",sysstat);
    
    HPD = ((sysstat & (B_HPDETECT|B_RXSENDETECT)) == (B_HPDETECT|B_RXSENDETECT))?TRUE:FALSE ;
    
    // 2007/06/20 added by jj_tseng@chipadvanced.com
    if( pHPDChange )
    {
    	*pHPDChange = FALSE ;
    }
    //~jj_tseng@chipadvanced.com 2007/06/20
    
    if( !HPD )
    {
        bAuthenticated = FALSE ;
    }

    //printf("sysstat=0x%x 0x%x  0x%x\n",sysstat,B_INT_ACTIVE,sysstat & B_INT_ACTIVE);
    if(sysstat & B_INT_ACTIVE)
    {
		// printf("sysstat=0x%x 0x%x  0x%x\n",sysstat,B_INT_ACTIVE,sysstat & B_INT_ACTIVE);
        intdata1 = HDMITX_ReadI2C_Byte(REG_INT_STAT1) ;
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("INT_Handler: reg%02x = %02x\n",REG_INT_STAT1,intdata1) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
        
		if( intdata1 & B_INT_DDCFIFO_ERR )
		{
			// printf ("DDC FIFO Error.\n");
#ifdef CAT6611_DRV_DEBUG_ON
		    ErrorF("DDC FIFO Error.\n");
#endif // #ifdef CAT6611_DRV_DEBUG_ON
		    ClearDDCFIFO() ;
		}


		if( intdata1 & B_INT_DDC_BUS_HANG )
		{
			// printf ("DDC BUS HANG.\n");
#ifdef CAT6611_DRV_DEBUG_ON
		    ErrorF("DDC BUS HANG.\n") ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
	            AbortDDC() ;
	            
	            if( bAuthenticated )
	            {
	                // when DDC hang, and aborted DDC, the HDCP authentication need to restart.
	                HDCP_ResumeAuthentication() ;
	            }
		}


		if( intdata1 & (B_INT_HPD_PLUG|B_INT_RX_SENSE ))
		{
			// printf ("intdata1 : 0x%02X\n", intdata1 & (B_INT_HPD_PLUG|B_INT_RX_SENSE ));
	            if(pHPDChange) *pHPDChange = TRUE ;
	            
	            if(!HPD) 
	            {
	                HDMITX_WriteI2C_Byte(REG_SW_RST,B_AREF_RST|B_VID_RST|B_AUD_RST|B_HDCP_RST) ;
	                DelayMS(1) ;
	                HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;
#ifdef CAT6611_DRV_DEBUG_ON
	                ErrorF("Unplug, %x %x\n",HDMITX_ReadI2C_Byte(REG_SW_RST), HDMITX_ReadI2C_Byte(REG_AFE_DRV_CTRL)) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
	                // VState = VSTATE_Unplug ;
	            }
		}
		

        intdata2 = HDMITX_ReadI2C_Byte(REG_INT_STAT2) ;
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("INT_Handler: reg%02x = %02x\n",REG_INT_STAT2,intdata2) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON

		#ifdef SUPPORT_HDCP
		if( intdata2 & B_INT_AUTH_DONE )
		{
			// printf ("interrupt Authenticate Done.\n");
#ifdef CAT6611_DRV_DEBUG_ON
	            ErrorF("interrupt Authenticate Done.\n") ;                
#endif // #ifdef CAT6611_DRV_DEBUG_ON
	            HDMITX_OrReg_Byte(REG_INT_MASK2,B_AUTH_DONE_MASK) ;
	            bAuthenticated = TRUE ;
	            SetAVMute(FALSE) ;
		}


	if( intdata2 & B_INT_AUTH_FAIL )
		{
			// printf ("interrupt Authenticate Fail.\n");
#ifdef CAT6611_DRV_DEBUG_ON
			ErrorF("interrupt Authenticate Fail.\n") ;       
#endif // #ifdef CAT6611_DRV_DEBUG_ON
			AbortDDC();   // @emily add 
			HDCP_ResumeAuthentication() ;
        }
        #endif // SUPPORT_HDCP

        intdata3 = HDMITX_ReadI2C_Byte(REG_INT_STAT3) ;
        if( intdata3 & B_INT_VIDSTABLE )
        {
			// printf ("B_INT_VIDSTABLE.\n");
            if(sysstat & B_TXVIDSTABLE)
            {
                FireAFE() ;
            }
        }

        HDMITX_WriteI2C_Byte(REG_INT_CLR0, 0xFF) ;
        HDMITX_WriteI2C_Byte(REG_INT_CLR1, 0xFF) ;
        intclr3 = (HDMITX_ReadI2C_Byte(REG_SYS_STATUS))|B_CLR_AUD_CTS | B_INTACTDONE ;
        HDMITX_WriteI2C_Byte(REG_SYS_STATUS,intclr3) ; // clear interrupt.
        intclr3 &= ~(B_INTACTDONE|B_CLR_AUD_CTS) ;
        HDMITX_WriteI2C_Byte(REG_SYS_STATUS,intclr3) ; // INTACTDONE reset to zero.
    }
    else
    {
        if(pHPDChange) 
        {
            *pHPDChange = (HPD != PrevHPD)?TRUE:FALSE ;
            
            if( *pHPDChange &&(!HPD))
            {
				printf ("Unplug...\n");
                HDMITX_WriteI2C_Byte(REG_SW_RST,B_AREF_RST|B_VID_RST|B_AUD_RST|B_HDCP_RST) ;
                DelayMS(1) ;
                HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;
#ifdef CAT6611_DRV_DEBUG_ON
                ErrorF("Unplug, %x %x\n",HDMITX_ReadI2C_Byte(REG_SW_RST), HDMITX_ReadI2C_Byte(REG_AFE_DRV_CTRL)) ;
#endif // #ifdef CAT6611_DRV_DEBUG_ON
            }
        }
    }
    
    SetAudioChannel() ;
    
    if( bManualCheckRi )
    {
        ManualCheckRiCounter ++ ;
        if( ManualCheckRiCounter == 1000 )
        {
            ManualCheckRiCounter = 0 ;
            HDCP_ReadRi() ;
        }
    }
    
    if( pHPD ) 
    {
        *pHPD = HPD ? TRUE:FALSE ;
    }
    
    PrevHPD = HPD ;
    return HPD ;
}


BOOL
EnableAVIInfoFrame(BOOL bEnable, unsigned char *pAVIInfoFrame)
{
    if( !bEnable )
    {
        DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }
    
    if(SetAVIInfoFrame((AVI_InfoFrame *)pAVIInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }
    
    return FALSE ;
}

BOOL
EnableAudioInfoFrame(BOOL bEnable, unsigned char *pAudioInfoFrame)
{
    if( !bEnable )
    {
        DISABLE_AUD_INFOFRM_PKT();
        return TRUE ;
    }
    
    
    if(SetAudioInfoFrame((Audio_InfoFrame *)pAudioInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }
    
    return FALSE ;
}

void
SetAVMute(BOOL bEnable) 
{
    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_AV_MUTE,bEnable?B_SET_AVMUTE:B_CLR_AVMUTE) ;
    HDMITX_WriteI2C_Byte(REG_PKT_GENERAL_CTRL, B_ENABLE_PKT|B_REPEAT_PKT) ;
}
////////////////////////////////////////////////////////////////////////////////
// Initialization process                                                     //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Function: SetInputMode
// Parameter: InputMode, VideoInputType
//      InputMode - use [1:0] to identify the color space for reg70[7:6], 
//                  definition:
//                     #define F_MODE_RGB444  0
//                     #define F_MODE_YUV422 1
//                     #define F_MODE_YUV444 2
//                     #define F_MODE_CLRMOD_MASK 3
//      VideoInputType - defined the CCIR656 D[0], SYNC Embedded D[1], and 
//                     DDR input in D[2].
// Return: N/A
// Remark: program Reg70 with the input value.
// Side-Effect: Reg70.
////////////////////////////////////////////////////////////////////////////////

static void
SetInputMode(BYTE InputColorMode, BYTE InputSignalType)
{
    BYTE ucData ;
    
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("SetInputColorMode(%02X, %02X)\n",InputColorMode,InputSignalType) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

    ucData = HDMITX_ReadI2C_Byte(REG_INPUT_MODE) ;

    ucData &= ~(M_INCOLMOD|B_2X656CLK|B_SYNCEMB|B_INDDR) ;

    switch( InputColorMode & F_MODE_CLRMOD_MASK )
    {
    case F_MODE_YUV422:
        ucData |= B_IN_YUV422 ;
        break ;
    case F_MODE_YUV444:
        ucData |= B_IN_YUV444 ;
        break ;
    case F_MODE_RGB444:
    default:
        ucData |= B_IN_RGB ;
        break ;
    }

    if( InputSignalType & T_MODE_CCIR656 )
    {
        ucData |= B_2X656CLK ;
#ifdef CAT6611_DRV_DEBUG_ON
		ErrorF("CCIR656 mode\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }

    if( InputSignalType & T_MODE_SYNCEMB )
    {
        ucData |= B_SYNCEMB ;
#ifdef CAT6611_DRV_DEBUG_ON
		ErrorF("Sync Embedded mode\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }

    if( InputSignalType & T_MODE_INDDR )
    {
        ucData |= B_INDDR ;
#ifdef CAT6611_DRV_DEBUG_ON
		ErrorF("Input DDR mode\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }

    HDMITX_WriteI2C_Byte(REG_INPUT_MODE, ucData) ;
}

////////////////////////////////////////////////////////////////////////////////
// Function: SetCSCScale
// Parameter: bInputMode - 
//             D[1:0] - Color Mode
//             D[4] - Colorimetry 0: ITU_BT601 1: ITU_BT709
//             D[5] - Quantization 0: 0_255 1: 16_235
//             D[6] - Up/Dn Filter 'Required' 
//                    0: no up/down filter
//                    1: enable up/down filter when csc need.
//             D[7] - Dither Filter 'Required'
//                    0: no dither enabled.
//                    1: enable dither and dither free go "when required".
//            bOutputMode -
//             D[1:0] - Color mode.
// Return: N/A
// Remark: reg72~reg8D will be programmed depended the input with table.
// Side-Effect: 
////////////////////////////////////////////////////////////////////////////////

static void
SetCSCScale(BYTE bInputMode,BYTE bOutputMode)
{
    BYTE ucData; 
    BYTE csc = B_CSC_YUV2RGB;
    BYTE filter = 0 ; // filter is for Video CTRL DN_FREE_GO, EN_DITHER, and ENUDFILT


	// (1) YUV422 in, RGB/YUV444 output ( Output is 8-bit, input is 12-bit)
	// (2) YUV444/422  in, RGB output ( CSC enable, and output is not YUV422 )
	// (3) RGB in, YUV444 output   ( CSC enable, and output is not YUV422 )
    // 
    // YUV444/RGB444 <-> YUV422 need set up/down filter.

	//printf("SetCSCScale : bInputMode[0x%02X] , bOutputMode[0x%02X]\n", bInputMode, bOutputMode);
    
    switch(bInputMode&F_MODE_CLRMOD_MASK)
    {
    #ifdef SUPPORT_INPUTYUV444
    case F_MODE_YUV444:
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Input mode is YUV444 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_BYPASS ;
            break ;

        case F_MODE_YUV422:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV422\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            if( bInputMode & F_VIDMODE_EN_UDFILT) // YUV444 to YUV422 need up/down filter for processing.
            {
                filter |= B_EN_UDFILTER ;
            }
            csc = B_CSC_BYPASS ;
            break ;
        case F_MODE_RGB444:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is RGB444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_YUV2RGB ;
            if( bInputMode & F_VIDMODE_EN_DITHER) // YUV444 to RGB444 need dither
            {
                filter |= B_EN_DITHER | B_DNFREE_GO ;
            }

            break ;
        }
        break ;

    #endif   
    #ifdef SUPPORT_INPUTRGB
    case F_MODE_RGB444:
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Input mode is RGB444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_RGB2YUV ;
            
            if( bInputMode & F_VIDMODE_EN_DITHER) // RGB444 to YUV444 need dither
            {
                filter |= B_EN_DITHER | B_DNFREE_GO ;
            }
            break ;

        case F_MODE_YUV422:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV422\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            if( bInputMode & F_VIDMODE_EN_UDFILT) // RGB444 to YUV422 need down filter.
            {
                filter |= B_EN_UDFILTER ;
            }

            if( bInputMode & F_VIDMODE_EN_DITHER) // RGB444 to YUV422 need dither
            {
                filter |= B_EN_DITHER | B_DNFREE_GO ;
            }
            csc = B_CSC_RGB2YUV ;
            break ;

        case F_MODE_RGB444:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is RGB444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_BYPASS ;
            break ;
        }
        break ;
    #endif   
    #ifdef SUPPORT_INPUTYUV422
    case F_MODE_YUV422:
    default:                    
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Input mode is YUV422\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_BYPASS ;
            if( bInputMode & F_VIDMODE_EN_UDFILT) // YUV422 to YUV444 need up filter
            {
                filter |= B_EN_UDFILTER ;
            }

            if( bInputMode & F_VIDMODE_EN_DITHER) // YUV422 to YUV444 need dither
            {
                filter |= B_EN_DITHER | B_DNFREE_GO ;
            }

            break ;
        case F_MODE_YUV422:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is YUV422\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_BYPASS ;

            break ;

        case F_MODE_RGB444:
        default:       
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Output mode is RGB444\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            csc = B_CSC_YUV2RGB ;
            if( bInputMode & F_VIDMODE_EN_UDFILT) // YUV422 to RGB444 need up/dn filter.
            {
                filter |= B_EN_UDFILTER ;
            }
            
            if( bInputMode & F_VIDMODE_EN_DITHER) // YUV422 to RGB444 need dither
            {
                filter |= B_EN_DITHER | B_DNFREE_GO ;
            }
            break ;
        }
        break ;
    #endif        
    
    }
    #ifdef SUPPORT_INPUTRGB
    // set the CSC metrix registers by colorimetry and quantization 
    if( csc == B_CSC_RGB2YUV )
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("CSC = RGB2YUV %x ",csc) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        switch(bInputMode&(F_VIDMODE_ITU709|F_VIDMODE_16_235))
        {
        case F_VIDMODE_ITU709|F_VIDMODE_16_235:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU709 16-235 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_16_235, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU709|F_VIDMODE_0_255:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU709 0-255 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_0_255, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_0_255,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU601|F_VIDMODE_16_235:
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU601 16-235 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_16_235, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU601|F_VIDMODE_0_255:
        default:            
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU601 0-255 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_0_255, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_0_255,SIZEOF_CSCMTX) ;
            break ;
        }

    }
    #endif
    #ifdef SUPPORT_INPUTYUV
    if(csc == B_CSC_BYPASS)
    {

    }
    else// ( csc == B_CSC_YUV2RGB )
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("CSC = YUV2RGB %x ",csc) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

        switch(bInputMode&(F_VIDMODE_ITU709|F_VIDMODE_16_235))
        {
        case F_VIDMODE_ITU709|F_VIDMODE_16_235://11
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU709 16-235 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_16_235, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU709|F_VIDMODE_0_255://01
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU709 0-255 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_0_255, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_0_255,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU601|F_VIDMODE_16_235://10
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU601 16-235 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_16_235, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_VIDMODE_ITU601|F_VIDMODE_0_255://00
        default:            
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ITU601 0-255 ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            HDMITX_WriteI2C_ByteN(REG_CSC_YOFF, bCSCOffset_0_255, SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_0_255,SIZEOF_CSCMTX) ;
            break ;
        }
    }
    #endif
    ucData = HDMITX_ReadI2C_Byte(REG_CSC_CTRL) & ~(M_CSC_SEL|B_DNFREE_GO|B_EN_DITHER|B_EN_UDFILTER) ;
    ucData |= filter|csc ;

    HDMITX_WriteI2C_Byte(REG_CSC_CTRL,ucData) ;

    // set output Up/Down Filter, Dither control

}


////////////////////////////////////////////////////////////////////////////////
// Function: SetupAFE
// Parameter: BOOL HighFreqMode
//            FALSE - PCLK < 80Hz ( for mode less than 1080p) 
//            TRUE  - PCLK > 80Hz ( for 1080p mode or above)
// Return: N/A
// Remark: set reg62~reg65 depended on HighFreqMode
//         reg61 have to be programmed at last and after video stable input.
// Side-Effect:
////////////////////////////////////////////////////////////////////////////////

static void
SetupAFE(BOOL HighFreqMode)
{
    BYTE uc ;
    
	// @emily turn off reg61 before SetupAFE parameters.
    Switch_HDMITX_Bank(0) ;
    uc = (HDMITX_ReadI2C_Byte(REG_INT_CTRL)&B_IDENT_6612)?B_AFE_XP_BYPASS:0 ;
    // for identify the CAT6612 operation.
    
    HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,B_AFE_DRV_RST);/* 0x00 */
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("SetupAFE()\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    if( HighFreqMode )
    {
        HDMITX_WriteI2C_Byte(REG_AFE_XP_CTRL1,B_AFE_XP_GAINBIT|B_AFE_XP_RESETB/* 0x88 */|uc) ;
        HDMITX_WriteI2C_Byte(REG_AFE_XP_CTRL2,B_XP_CLKSEL_1_PCLKHV/*0x01*/) ;
        HDMITX_WriteI2C_Byte(REG_AFE_IP_CTRL, B_AFE_IP_GAINBIT|B_AFE_IP_SEDB|B_AFE_IP_RESETB|B_AFE_IP_PDIV1 /*0x56*/) ;
        HDMITX_WriteI2C_Byte(REG_AFE_RING,0x00) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_AFE_XP_CTRL1,B_AFE_XP_ER0|B_AFE_XP_RESETB/* 0x18 */|uc) ;
        HDMITX_WriteI2C_Byte(REG_AFE_XP_CTRL2,B_XP_CLKSEL_1_PCLKHV/*0x01*/) ;
        HDMITX_WriteI2C_Byte(REG_AFE_IP_CTRL, B_AFE_IP_SEDB|B_AFE_IP_ER0|B_AFE_IP_RESETB|B_AFE_IP_PDIV1 /*0x1E*/) ;
        HDMITX_WriteI2C_Byte(REG_AFE_RING,0x00) ;
    }
    HDMITX_AndReg_Byte(REG_SW_RST,~(B_REF_RST|B_VID_RST/*|B_AREF_RST*/|B_HDMI_RST)) ;

    // REG_AFE_DRV_CTRL have to be set at the last step of setup .
}


////////////////////////////////////////////////////////////////////////////////
// Function: FireAFE
// Parameter: N/A
// Return: N/A
// Remark: write reg61 with 0x04
//         When program reg61 with 0x04, then audio and video circuit work.
// Side-Effect: N/A
////////////////////////////////////////////////////////////////////////////////

static void
FireAFE(void)
{
    Switch_HDMITX_Bank(0) ;
    // 2008/09/19 modified by jj_tseng@chipadvanced.com
    // HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,0x01<<O_AFE_DRV_SR /* 0x04 */) ;
    HDMITX_WriteI2C_Byte(REG_AFE_DRV_CTRL,0x00) ; // disable pre-emphisis
    //~jj_tseng@chipadvanced.com
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("reg[%02X] = %02X",REG_AFE_DRV_CTRL,HDMITX_ReadI2C_Byte(REG_AFE_DRV_CTRL)) ;
    ErrorF("reg[%02X] = %02X ",REG_AFE_XP_CTRL1,HDMITX_ReadI2C_Byte(REG_AFE_XP_CTRL1)) ;
    ErrorF("reg[%02X] = %02X ",REG_AFE_XP_CTRL2,HDMITX_ReadI2C_Byte(REG_AFE_XP_CTRL2)) ;
    ErrorF("reg[%02X] = %02X ",REG_AFE_IP_CTRL,HDMITX_ReadI2C_Byte(REG_AFE_IP_CTRL)) ;
    ErrorF("reg[%02X] = %02X\n",REG_AFE_RING,HDMITX_ReadI2C_Byte(REG_AFE_RING)) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

}

////////////////////////////////////////////////////////////////////////////////
// Audio Output
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: SetAudioFormat
// Parameter: 
//    NumChannel - number of channel, from 1 to 8
//    AudioEnable - Audio source and type bit field, value of bit field are 
//        ENABLE_SPDIF    (1<<4)
//        ENABLE_I2S_SRC3  (1<<3)
//        ENABLE_I2S_SRC2  (1<<2)
//        ENABLE_I2S_SRC1  (1<<1)
//        ENABLE_I2S_SRC0  (1<<0)
//    SampleFreq - the audio sample frequence in Hz
//    AudSWL - Audio sample width, only support 16,18,20, or 24.
//    AudioCatCode - The audio channel catalogy code defined in IEC 60958-3
// Return: ER_SUCCESS if done, ER_FAIL for otherwise.
// Remark: program audio channel control register and audio channel registers
//         to enable audio by input.
// Side-Effect: register bank will keep in bank zero.
////////////////////////////////////////////////////////////////////////////////
void
SetNonPCMAudio(BOOL NLPCM)
{
	BYTE reg ;
	bNLPCM = NLPCM ;
	
    Switch_HDMITX_Bank(1) ;
    reg = HDMITX_ReadI2C_Byte(REG_AUDCHST_MODE) & ~B_AUD_NLPCM ;
    reg |= (bNLPCM)?B_AUD_NLPCM:0 ;
    HDMITX_WriteI2C_Byte(REG_AUDCHST_MODE,reg) ; // 2 audio channel without pre-emphasis, if NumChannel set it as 1.
    Switch_HDMITX_Bank(0) ;
	
}

static SYS_STATUS
SetAudioFormat(BYTE NumChannel, BYTE AudioEnable, ULONG SampleFreq, BYTE AudSWL, BYTE AudioCatCode)
{
    BYTE fs ;
    BYTE SWL ;
	BYTE bChannelSwap ;
    BYTE SourceValid ;
    BYTE SoruceNum ;


#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("SetAudioFormat(%d channel,%02X,SampleFreq %ldHz,AudSWL %d,%02X)\n", NumChannel,  AudioEnable,  SampleFreq,  AudSWL,  AudioCatCode) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        

    switch(SampleFreq)
    {
    case 32000: fs = FS_32K ; break ;
    case 44100: fs = FS_44K1 ; break ;
    case 48000: fs = FS_48K ; break ;
    case 88200: fs = FS_88K2 ; break ;
    case 96000: fs = FS_96K ; break ;
    case 176400: fs = FS_176K4 ; break ;
    case 192000: fs = FS_192K ; break ;
    default: fs = FS_OTHER ; break ;
    }

#ifdef CAT6611_DRV_DEBUG_ON
    // ErrorF("Sampling Freq = %ld fs = %x\n",SampleFreq,fs) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    bOutputAudioMode |= B_AUD_FULLPKT ;
    if( NumChannel > 6 )
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S3VALID | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 4 ;
    }
    else if ( NumChannel > 4 )
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 3 ;
    }
    else if ( NumChannel > 2 )
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S1VALID ;
        SoruceNum = 2 ;
    }
    else
    {
        SourceValid = B_AUD_ERR2FLAT ; // only two channel.
        SoruceNum = 1 ;
        bOutputAudioMode &= ~B_AUD_FULLPKT ;
    }



   if( AudioEnable & B_AUD_SPDIF )
    {
         AudioEnable &= ~(B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1) ;
         AudioEnable |= B_AUD_EN_I2S0 ;
    }

    AudioEnable &= ~ (M_AUD_SWL|B_SPDIFTC) ;

    switch(AudSWL)
    {
    case 16:
        SWL = AUD_SWL_16 ;
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        SWL = AUD_SWL_18 ;
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        SWL = AUD_SWL_20 ;
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
        SWL = AUD_SWL_24 ;
        AudioEnable |= M_AUD_24BIT ;
        break ;
    default:
        return ER_FAIL ;
    }


    Switch_HDMITX_Bank(0) ;

	// 2008/05/20 added by jj_tseng@chipadvanced.com
	if( HDMITX_ReadI2C_Byte(REG_INT_CTRL) & B_IDENT_6612 )
	{
		bChannelSwap = bAudioChannelSwap & 0xF ;
	}
	else
	{
		// if cat6611, regE3[3:0] bit '1' for no swap.
		bChannelSwap = (~bAudioChannelSwap) & 0xF ;
	}
	//~jj_tseng@chipadvanced.com
	
    HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL0, AudioEnable&0xF0) ; // disable audio channel before audio enabled.
    HDMITX_AndReg_Byte(REG_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;    // enable Audio

    HDMITX_WriteI2C_Byte(REG_AUDIO_FIFOMAP, 0xE4 ) ; // default mapping.
#ifdef CHSTAT_BY_SPDIF    
    HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL3,((AudioEnable&B_AUD_SPDIF)?B_CHSTSEL:0)|bChannelSwap) ;
#else
    HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL3,bChannelSwap) ;
#endif    
    HDMITX_WriteI2C_Byte(REG_AUD_SRCVALID_FLAT,SourceValid) ;
    HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL1, bOutputAudioMode) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    // suggested to be 0x41

    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REG_AUDCHST_MODE,((bNLPCM)?B_AUD_NLPCM:0) |((NumChannel == 1)?B_AUD_MONO:0)) ; // 2 audio channel without pre-emphasis, if NumChannel set it as 1.
    HDMITX_WriteI2C_Byte(REG_AUDCHST_CAT,AudioCatCode) ;
    HDMITX_WriteI2C_Byte(REG_AUDCHST_SRCNUM,SoruceNum) ;
    HDMITX_WriteI2C_Byte(REG_AUD0CHST_CHTNUM,0) ;
    HDMITX_WriteI2C_Byte(REG_AUDCHST_CA_FS,0x00|fs) ; // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    HDMITX_WriteI2C_Byte(REG_AUDCHST_OFS_WL,(fs<<4)|SWL) ;
    Switch_HDMITX_Bank(0) ;
    bAudioEnableSetting = AudioEnable ;
    bAudioPendingCounter = 10 ; // or for reference delay on customer part.
    bPendingAudio = TRUE ;
    bPendingAudioAdjust = FALSE ;
    // HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL0, AudioEnable) ; // 
    return ER_SUCCESS;
}

static void
AdjustAudioSampleClock(void)
{
    ULONG N, CTS ;
    ULONG SampleFreq ;
    byte fs,uc ;
    
    bPendingAudioAdjust = TRUE ;
    Switch_HDMITX_Bank(1) ;
    
    N = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN2)&0xF) << 16 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN1)) <<8 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN0)) ;
    
    CTS = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt2)&0xF) << 16 ;
    CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt1)) <<8 ;
    CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt0)) ;
    Switch_HDMITX_Bank(0) ;
    
    // CTS = TMDSCLK * N / ( 128 * SampleFreq ) 
    // SampleFreq = TMDSCLK * N / (128*CTS) 
    
    if( CTS == 0 ) 
    {
        return  ;
    }
    
    SampleFreq = currVideoPixelClock/CTS ;
    SampleFreq *= N ;
    SampleFreq /= 128 ;
            
    if( SampleFreq>31000 && SampleFreq<=38050 )
    {
        SampleFreq = 32000 ;
        fs = FS_32K ;
    }
    else if (SampleFreq < 46050 ) // 44.1KHz
    {
        SampleFreq = 44100 ;
        fs = FS_44K1 ;
    }
    else if (SampleFreq < 68100 ) // 48KHz
    {
        SampleFreq = 48000 ;
        fs = FS_48K ;
    }
    else if (SampleFreq < 92100 ) // 88.2 KHz
    {
        SampleFreq = 88200 ;
        fs = FS_88K2 ;
    }
    else if (SampleFreq < 136200 ) // 96KHz
    {
        SampleFreq = 96000 ;
        fs = FS_96K ;
    }
    else if (SampleFreq < 184200 ) // 176.4KHz
    {
        SampleFreq = 176400 ;
        fs = FS_176K4 ;
    }
    else if (SampleFreq < 240200 ) // 192KHz
    {
        SampleFreq = 192000 ;
        fs = FS_192K ;
    }
    else 
    {
        SampleFreq = 0 ;
        fs = FS_OTHER ;
    }

    bPendingAudioAdjust = FALSE ;
    
    SetNCTS(currVideoPixelClock, SampleFreq ) ; // set N, CTS by new generated clock.
    
    Switch_HDMITX_Bank(1) ; // adjust the new fs in channel status registers
    HDMITX_WriteI2C_Byte(REG_AUDCHST_CA_FS,0x00|fs) ; // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    uc = HDMITX_ReadI2C_Byte(REG_AUDCHST_OFS_WL) ;
    uc &= 0xF ;
    uc |= fs << 4 ;
    HDMITX_WriteI2C_Byte(REG_AUDCHST_OFS_WL,uc) ;
    
    Switch_HDMITX_Bank(0) ;    
}

static void
SetAudioChannel(void)
{
    if( bPendingAudio )
    {
        if((HDMITX_ReadI2C_Byte(REG_SW_RST) & (B_AUD_RST|B_AREF_RST)) == 0)
        {
            Switch_HDMITX_Bank(0) ;
            if(bAudioEnableSetting&B_AUD_SPDIF)
            {
                if(HDMITX_ReadI2C_Byte(REG_CLK_STATUS2) & B_OSF_LOCK)
                {
                    AdjustAudioSampleClock() ;
                }
                else
                {
                    return ;
                }
            }
            else
            {
                if( bAudioPendingCounter )
                {
                    bAudioPendingCounter -- ;
                    return ;
                }
                else
                {
                    AdjustAudioSampleClock() ;
                }
            }
            HDMITX_WriteI2C_Byte(REG_AUDIO_CTRL0, bAudioEnableSetting) ; 
            bPendingAudio = FALSE ;
        }
    }
    
    if(bPendingAudioAdjust)
    {
        AdjustAudioSampleClock() ;
    }
}


////////////////////////////////////////////////////////////////////////////////
// Function: SetNCTS
// Parameter: PCLK - video clock in Hz.
//            Fs - audio sample frequency in Hz
// Return: ER_SUCCESS if success
// Remark: set N value, the CTS will be auto generated by HW.
// Side-Effect: register bank will reset to bank 0.
////////////////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetNCTS(ULONG PCLK, ULONG Fs)
{
    ULONG n,MCLK ;

    MCLK = Fs * 256 ; // MCLK = fs * 256 ;

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("SetNCTS(%ld,%ld): MCLK = %ld\n",PCLK,Fs,MCLK) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

	switch (Fs) {
		case 32000:
			switch (PCLK) {
				case 74175000: n = 11648; break;
				case 14835000: n = 11648; break;
				default: n = 4096;
			}
			break;
		case 44100:
			switch (PCLK) {
				case 74175000: n = 17836; break;
				case 14835000: n = 8918; break;
				default: n = 6272;
			}
			break;
		case 48000:
			switch (PCLK) {
				case 74175000: n = 11648; break;
				case 14835000: n = 5824; break;
				default: n = 6144;
			}
			break;
		case 88200:
			switch (PCLK) {
				case 74175000: n = 35672; break;
				case 14835000: n = 17836; break;
				default: n = 12544;
			}
			break;
		case 96000:
			switch (PCLK) {
				case 74175000: n = 23296; break;
				case 14835000: n = 11648; break;
				default: n = 12288;
			}
			break;
		case 176400:
			switch (PCLK) {
				case 74175000: n = 71344; break;
				case 14835000: n = 35672; break;
				default: n = 25088;
			}
			break;
		case 192000:
			switch (PCLK) {
				case 74175000: n = 46592; break;
				case 14835000: n = 23296; break;
				default: n = 24576;
			}
			break;
		default: n = MCLK / 2000;
	}

#ifdef CAT6611_DRV_DEBUG_ON
    // ErrorF(" n = %ld\n",n) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF)) ;
    Switch_HDMITX_Bank(0) ;

    HDMITX_WriteI2C_Byte(REG_PKT_SINGLE_CTRL, 0) ; // D[1] = 0, HW auto count CTS

    HDMITX_SetReg_Byte(REG_CLK_CTRL0,~M_EXT_MCLK_SEL,B_EXT_256FS) ;
    return ER_SUCCESS ;
}

////////////////////////////////////////////////////////////////////////////////
// DDC Function.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function: ClearDDCFIFO
// Parameter: N/A
// Return: N/A
// Remark: clear the DDC FIFO.
// Side-Effect: DDC master will set to be HOST.
////////////////////////////////////////////////////////////////////////////////

static void
ClearDDCFIFO(void)
{
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL, B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD, CMD_FIFO_CLR ) ;
}

static void
GenerateDDCSCLK(void)
{
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_GEN_SCLCLK) ;
}

////////////////////////////////////////////////////////////////////////////////
// Function: AbortDDC
// Parameter: N/A
// Return: N/A
// Remark: Force abort DDC and reset DDC bus.
// Side-Effect: 
////////////////////////////////////////////////////////////////////////////////

static void
AbortDDC(void)
{
    BYTE CPDesire, SWReset, DDCMaster ;
    BYTE count, uc ;
    
    // save the SW reset, DDC master, and CP Desire setting.
    SWReset = HDMITX_ReadI2C_Byte(REG_SW_RST) ;
    CPDesire = HDMITX_ReadI2C_Byte(REG_HDCP_DESIRE) ;
    DDCMaster = HDMITX_ReadI2C_Byte(REG_DDC_MASTER_CTRL) ;
    
	
    HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE,CPDesire&(~B_CPDESIRE)) ; // @emily change order
    HDMITX_WriteI2C_Byte(REG_SW_RST, SWReset|B_HDCP_RST) ;		 // @emily change order
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL, B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD, CMD_DDC_ABORT ) ;
    
    for( count = 200 ; count > 0 ; count-- )
    {
        uc = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if( uc & B_DDC_DONE )
        {
            break ;
        }
        if( uc & B_DDC_ERROR )
        {
            // error when abort.
            break ;
        }
        DelayMS(1) ; // delay 1 ms to stable.
    }
    
    // restore the SW reset, DDC master, and CP Desire setting.
    HDMITX_WriteI2C_Byte(REG_SW_RST, SWReset) ;
    HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE,CPDesire) ;
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,DDCMaster) ;
}

////////////////////////////////////////////////////////////////////////////////
// Function: ReadEDID
// Parameter: pData - the pointer of buffer to receive EDID ucdata.
//            bSegment - the segment of EDID readback.
//            offset - the offset of EDID ucdata in the segment. in byte.
//            count - the read back bytes count, cannot exceed 32
// Return: ER_SUCCESS if successfully getting EDID. ER_FAIL otherwise.
// Remark: function for read EDID ucdata from reciever.
// Side-Effect: DDC master will set to be HOST. DDC FIFO will be used and dirty.
////////////////////////////////////////////////////////////////////////////////

static SYS_STATUS
ReadEDID(BYTE *pData, BYTE bSegment, BYTE offset, SHORT Count)
{
    SHORT RemainedCount, ReqCount ;
    BYTE bCurrOffset ;
    SHORT TimeOut ;
    BYTE *pBuff = pData ;
    BYTE ucdata ;

#ifdef CAT6611_DRV_DEBUG_ON
    // ErrorF("ReadEDID(%08lX, %d, %d, %d)\n",(ULONG)pData, bSegment, offset, Count) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    if( !pData )
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("ReadEDID(): Invallid pData pointer %08lX\n",(ULONG)pData) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        return ER_FAIL ;
    }

    if(HDMITX_ReadI2C_Byte(REG_INT_STAT1) & B_INT_DDC_BUS_HANG)
    {
#ifdef CAT6611_DRV_DEBUG_ON
    	ErrorF("Called AboutDDC()\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        AbortDDC() ;

    }

    ClearDDCFIFO() ;

    RemainedCount = Count ;
    bCurrOffset = offset ;

    Switch_HDMITX_Bank(0) ;

    while( RemainedCount > 0 )
    {

        ReqCount = (RemainedCount > DDC_FIFO_MAXREQ)?DDC_FIFO_MAXREQ:RemainedCount ;
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("ReadEDID(): ReqCount = %d, bCurrOffset = %d\n",ReqCount, bCurrOffset ) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

        ClearDDCFIFO() ;

        HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL, B_MASTERDDC|B_MASTERHOST) ;
        HDMITX_WriteI2C_Byte(REG_DDC_HEADER, DDC_EDID_ADDRESS ) ; // for EDID ucdata get
        HDMITX_WriteI2C_Byte(REG_DDC_REQOFF, bCurrOffset ) ;
        HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT, (BYTE)ReqCount ) ;
        HDMITX_WriteI2C_Byte(REG_DDC_EDIDSEG, bSegment) ;
        HDMITX_WriteI2C_Byte(REG_DDC_CMD, CMD_EDID_READ) ;

        bCurrOffset += ReqCount ;
        RemainedCount -= ReqCount ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut -- )
        {
            DelayMS(1) ;
            ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
            if( ucdata & B_DDC_DONE )
            {
                break ;
            }

            if( ucdata & B_DDC_ERROR)
            {
#ifdef CAT6611_DRV_DEBUG_ON
                ErrorF("ReadEDID(): DDC_STATUS = %02X, fail.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
                return ER_FAIL ;
            }
        }

        if( TimeOut == 0 )
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("ReadEDID(): DDC TimeOut. \n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            return ER_FAIL ;
        }

        do
        {
            *(pBuff++) = HDMITX_ReadI2C_Byte(REG_DDC_READFIFO) ;
            ReqCount -- ;
        }while(ReqCount > 0) ;

    }

    return ER_SUCCESS ;
}

////////////////////////////////////////////////////////////////////////////////
// Packet and InfoFrame
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Function: SetAVIInfoFrame()
// Parameter: pAVIInfoFrame - the pointer to HDMI AVI Infoframe ucData
// Return: N/A
// Remark: Fill the AVI InfoFrame ucData, and count checksum, then fill into
//         AVI InfoFrame registers.
// Side-Effect: N/A
////////////////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    int i ;
    byte ucData ;

    if(!pAVIInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1) ;
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB1,pAVIInfoFrame->pktbyte.AVI_DB[0]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB2,pAVIInfoFrame->pktbyte.AVI_DB[1]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB3,pAVIInfoFrame->pktbyte.AVI_DB[2]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB4,pAVIInfoFrame->pktbyte.AVI_DB[3]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB5,pAVIInfoFrame->pktbyte.AVI_DB[4]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB6,pAVIInfoFrame->pktbyte.AVI_DB[5]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB7,pAVIInfoFrame->pktbyte.AVI_DB[6]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB8,pAVIInfoFrame->pktbyte.AVI_DB[7]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB9,pAVIInfoFrame->pktbyte.AVI_DB[8]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB10,pAVIInfoFrame->pktbyte.AVI_DB[9]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB11,pAVIInfoFrame->pktbyte.AVI_DB[10]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB12,pAVIInfoFrame->pktbyte.AVI_DB[11]);
	HDMITX_WriteI2C_Byte(REG_AVIINFO_DB13,pAVIInfoFrame->pktbyte.AVI_DB[12]);
	ucData = 0;
	// printf ("SetAVIInfoFrame : ucData [%d]\n", ucData);
	
    for( i = 0, ucData = 0; i < 13 ; i++ )
    {
        ucData -= pAVIInfoFrame->pktbyte.AVI_DB[i] ;
    }
	// ErrorF("SetAVIInfo(): ") ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB1)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB2)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB3)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB4)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB5)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB6)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB7)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB8)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB9)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB10)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB11)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB12)) ;
    //ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_AVIINFO_DB13)) ;
	// ErrorF("\n") ;
    ucData -= 0x80+AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN ;
	HDMITX_WriteI2C_Byte(REG_AVIINFO_SUM, ucData);


    Switch_HDMITX_Bank(0) ;
    ENABLE_AVI_INFOFRM_PKT();
    return ER_SUCCESS ;
}

////////////////////////////////////////////////////////////////////////////////
// Function: SetAudioInfoFrame()
// Parameter: pAudioInfoFrame - the pointer to HDMI Audio Infoframe ucData
// Return: N/A
// Remark: Fill the Audio InfoFrame ucData, and count checksum, then fill into
//         Audio InfoFrame registers.
// Side-Effect: N/A
////////////////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
    int i ;
    BYTE ucData ;

    if( !pAudioInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REG_PKT_AUDINFO_CC,pAudioInfoFrame->pktbyte.AUD_DB[0]);
    HDMITX_WriteI2C_Byte(REG_PKT_AUDINFO_CA,pAudioInfoFrame->pktbyte.AUD_DB[1]);
    HDMITX_WriteI2C_Byte(REG_PKT_AUDINFO_DM_LSV,pAudioInfoFrame->pktbyte.AUD_DB[4] ) ;

	ucData = 0;

	// printf ("SetAudioInfoFrame : ucData [%d]\n", ucData);

    for( i = 0, ucData = 0 ; i< 5 ; i++ )
    {
        ucData -= pAudioInfoFrame->pktbyte.AUD_DB[i] ;
    }
    ucData -= 0x80+AUDIO_INFOFRAME_VER+AUDIO_INFOFRAME_TYPE+AUDIO_INFOFRAME_LEN ;

    HDMITX_WriteI2C_Byte(REG_PKT_AUDINFO_SUM,ucData) ;

#ifdef CAT6611_DRV_DEBUG_ON
    DumpCat6611Reg();
#endif // #ifdef CAT6611_DRV_DEBUG_ON

    Switch_HDMITX_Bank(0) ;
    ENABLE_AUD_INFOFRM_PKT();
    return ER_SUCCESS ;
}


///////////////////////////////////////////////////////////////////////////////
// Authentication
////////////////////////////////////////////////////////////////////////////////

#ifdef SUPPORT_HDCP
//////////////////////////////////////////////////////////////////////
// Authentication
//////////////////////////////////////////////////////////////////////
static void
HDCP_ClearAuthInterrupt()
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte(REG_INT_MASK2) & (~(B_KSVLISTCHK_MASK|B_AUTH_DONE_MASK|B_AUTH_FAIL_MASK));
    HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_AUTH_FAIL|B_CLR_AUTH_DONE|B_CLR_KSVLISTCHK) ;
    HDMITX_WriteI2C_Byte(REG_INT_CLR1,0) ;
    HDMITX_WriteI2C_Byte(REG_SYS_STATUS,B_INTACTDONE) ;
}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_EnableEncryption
// Parameter: N/A
// Return: ER_SUCCESS if done.
// Remark: Set regC1 as zero to enable continue authentication.
// Side-Effect: register bank will reset to zero.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_EnableEncryption(void)
{
    Switch_HDMITX_Bank(0) ;
	return HDMITX_WriteI2C_Byte(REG_ENCRYPTION,B_ENABLE_ENCRYPTION);
}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_Auth_Fire()
// Parameter: N/A
// Return: N/A
// Remark: write anything to reg21 to enable HDCP authentication by HW
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_Auth_Fire(void)
{
#ifdef CAT6611_DRV_DEBUG_ON
    // ErrorF("HDCP_Auth_Fire():\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHDCP) ; // MASTERHDCP,no need command but fire.
	HDMITX_WriteI2C_Byte(REG_AUTHFIRE,1);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StartAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Start the Cipher to free run for random number. When stop,An is
//         ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StartAnCipher()
{
    HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_START_CIPHER_GEN) ;
    DelayMS(1) ; // delay 1 ms
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StopAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Stop the Cipher,and An is ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StopAnCipher(void)
{
    HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_STOP_CIPHER_GEN) ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GenerateAn
// Parameter: N/A
// Return: N/A
// Remark: start An ciper random run at first,then stop it. Software can get
//         an in reg30~reg38,the write to reg28~2F
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
HDCP_GenerateAn(void)
{
    BYTE Data[8] ;

    HDCP_StartAnCipher() ;
    // HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_START_CIPHER_GEN) ;
    // DelayMS(1) ; // delay 1 ms
    // HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_STOP_CIPHER_GEN) ;

    HDCP_StopAnCipher() ;

    Switch_HDMITX_Bank(0) ;
    // new An is ready in reg30
    HDMITX_ReadI2C_ByteN(REG_AN_GEN,Data,8) ;
    HDMITX_WriteI2C_ByteN(REG_AN,Data,8) ;

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBCaps
// Parameter: pBCaps - pointer of byte to get BCaps.
//            pBStatus - pointer of two bytes to get BStatus
// Return: ER_SUCCESS if successfully got BCaps and BStatus.
// Remark: get B status and capability from HDCP reciever via DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,DDC_HDCP_ADDRESS) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x40) ; // BCaps offset
    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,3) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1) ;

        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            //ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetBCaps(): DDC fail by reg16=%02X.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN(REG_BSTAT,(PBYTE)pBStatus,2) ;
    *pBCaps = HDMITX_ReadI2C_Byte(REG_BCAP) ;
    return ER_SUCCESS ;

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBKSV
// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
// Return: ER_SUCCESS if successfuly got BKSV from Rx.
// Remark: Get BKSV from HDCP reciever.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBKSV(BYTE *pBKSV)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,DDC_HDCP_ADDRESS) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x00) ; // BKSV offset
    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,5) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1) ;

        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetBCaps(): DDC No ack or arbilose,%x,maybe cable did not connected. Fail.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN(REG_BKSV,(PBYTE)pBKSV,5) ;

    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function:HDCP_Authenticate
// Parameter: N/A
// Return: ER_SUCCESS if Authenticated without error.
// Remark: do Authentication with Rx
// Side-Effect:
//  1. bAuthenticated global variable will be TRUE when authenticated.
//  2. Auth_done interrupt and AUTH_FAIL interrupt will be enabled.
//////////////////////////////////////////////////////////////////////
static void
HDCP_ReadRi(void)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,DDC_HDCP_ADDRESS) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x08) ; // Ri offset
    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,2) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;

    for( TimeOut = 200 ; TimeOut > 0 ; TimeOut -- )
    {
        DelayMS(1) ;

        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if( ucdata & B_DDC_DONE )
        {
            break ;
        }

        if( ucdata & B_DDC_ERROR )
        {
            break ;
        }
    }

}
void
HDCP_Reset(void)
{
    BYTE uc ;
    HDMITX_WriteI2C_Byte(REG_LISTCTRL,0) ;
    HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE,0) ;
    uc = HDMITX_ReadI2C_Byte(REG_SW_RST) ;
    uc |= B_HDCP_RST ;
    HDMITX_WriteI2C_Byte(REG_SW_RST,uc) ;
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDCP_ClearAuthInterrupt() ;
    AbortDDC() ;
}

static SYS_STATUS
HDCP_Authenticate(void)
{
    BYTE ucdata ;
    BYTE BCaps ;
    USHORT BStatus ;
    USHORT TimeOut ;

    BYTE revoked = FALSE ;
    BYTE BKSV[5] ;

    bAuthenticated = FALSE ;

    // Authenticate should be called after AFE setup up.

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("HDCP_Authenticate():\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
	HDCP_Reset() ;
    // ClearDDCFIFO() ;
    // AbortDDC() ;

    if(HDCP_GetBCaps(&BCaps,&BStatus) != ER_SUCCESS)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("HDCP_GetBCaps fail.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        return ER_FAIL ;
    }


	// if(bHDMIMode)
	{
		if((BStatus & B_CAP_HDMI_MODE)==0)
		{
#ifdef CAT6611_DRV_DEBUG_ON
			ErrorF("Not a HDMI mode,do not authenticate and encryption. BCaps = %x BStatus = %x\n",BCaps,BStatus) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
			return ER_FAIL ;
		}
	}

#ifdef CAT6611_DRV_DEBUG_ON
	ErrorF("BCaps = %02X BStatus = %04X\n",BCaps,BStatus) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    /*
    if((BStatus & M_DOWNSTREAM_COUNT)> 6)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Down Stream Count %d is over maximum supported number 6,fail.\n",(BStatus & M_DOWNSTREAM_COUNT)) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        return ER_FAIL ;
    }
    */

    HDCP_GetBKSV(BKSV) ;
#ifdef CAT6611_DRV_DEBUG_ON
	ErrorF("BKSV %02X %02X %02X %02X %02X\n",BKSV[0],BKSV[1],BKSV[2],BKSV[3],BKSV[4]) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
	
	for(TimeOut = 0 ; TimeOut < 5 ; TimeOut ++)
	{
	    if(BKSV[TimeOut] == 0xFF)
	    {
			bAuthenticated = TRUE ;
	        return ER_SUCCESS ;
	    }
	}
    #ifdef SUPPORT_DSSSHA
    HDCP_VerifyRevocationList(SRM1,BKSV,&revoked) ;
    if(revoked)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("BKSV is revoked\n") ; return ER_FAIL ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("BKSV %02X %02X %02X %02X %02X is NOT %srevoked\n",BKSV[0],BKSV[1],BKSV[2],BKSV[3],BKSV[4],revoked?"not ":"") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    #endif // SUPPORT_DSSSHA

    Switch_HDMITX_Bank(0) ; // switch bank action should start on direct register writting of each function.

    // 2006/08/11 added by jjtseng
    // enable HDCP on CPDired enabled.
	ucdata = HDMITX_ReadI2C_Byte(REG_SW_RST) ;
	ucdata &= ~B_HDCP_RST ;
	HDMITX_WriteI2C_Byte(REG_SW_RST, ucdata) ;
    //~jjtseng 2006/08/11

    if(BCaps & B_CAP_HDCP_1p1)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("RX support HDCP 1.1\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE,B_ENABLE_HDPC11|B_CPDESIRE) ;
    }
    else
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("RX not support HDCP 1.1\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE,B_CPDESIRE) ;
    }


    // HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_AUTH_DONE|B_CLR_AUTH_FAIL|B_CLR_KSVLISTCHK) ;
    // HDMITX_WriteI2C_Byte(REG_INT_CLR1,0) ; // don't clear other settings.
    // ucdata = HDMITX_ReadI2C_Byte(REG_SYS_STATUS) ;
    // ucdata = (ucdata & M_CTSINTSTEP) | B_INTACTDONE ;
    // HDMITX_WriteI2C_Byte(REG_SYS_STATUS,ucdata) ; // clear action.

    // HDMITX_AndREG_Byte(REG_INT_MASK2,~(B_AUTH_FAIL_MASK|B_T_AUTH_DONE_MASK)) ;    // enable GetBCaps Interrupt
    HDCP_ClearAuthInterrupt() ;
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("int2 = %02X DDC_Status = %02X\n",HDMITX_ReadI2C_Byte(REG_INT_STAT2),HDMITX_ReadI2C_Byte(REG_DDC_STATUS)) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON


    HDCP_GenerateAn() ;
    HDMITX_WriteI2C_Byte(REG_LISTCTRL,0) ;
    bAuthenticated = FALSE ;

    if((BCaps & B_CAP_HDMI_REPEATER) == 0)
    {
        HDCP_Auth_Fire();
        // wait for status ;
        
        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            DelayMS(5) ; // delay 1ms
            ucdata = HDMITX_ReadI2C_Byte(REG_AUTH_STAT) ;
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("reg46 = %02x reg16 = %02x\n",ucdata,HDMITX_ReadI2C_Byte(0x16)) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON

            if(ucdata & B_AUTH_DONE)
            {
                bAuthenticated = TRUE ;
                break ;
            }
            
            ucdata = HDMITX_ReadI2C_Byte(REG_INT_STAT2) ;
            if(ucdata & B_INT_AUTH_FAIL)
            {
                /*
                HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_AUTH_FAIL) ;
                HDMITX_WriteI2C_Byte(REG_INT_CLR1,0) ;
                HDMITX_WriteI2C_Byte(REG_SYS_STATUS,B_INTACTDONE) ;
                HDMITX_WriteI2C_Byte(REG_SYS_STATUS,0) ;
                */
#ifdef CAT6611_DRV_DEBUG_ON
                ErrorF("HDCP_Authenticate(): Authenticate fail\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
                bAuthenticated = FALSE ;
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
#ifdef CAT6611_DRV_DEBUG_ON
             ErrorF("HDCP_Authenticate(): Time out. return fail\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
             bAuthenticated = FALSE ;
             return ER_FAIL ;
        }
        return ER_SUCCESS ;
    }

    return HDCP_Authenticate_Repeater() ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_VerifyIntegration
// Parameter: N/A
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark: no used now.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_VerifyIntegration()
{
//    BYTE ucdata ;
    // if any interrupt issued a Auth fail,returned the Verify Integration fail.

    if(HDMITX_ReadI2C_Byte(REG_INT_STAT1) & B_INT_AUTH_FAIL)
    {
        HDCP_ClearAuthInterrupt() ;
        bAuthenticated = FALSE ;
        return ER_FAIL ;
    }

    if(bAuthenticated == TRUE)
    {
        return ER_SUCCESS ;
    }

    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Authenticate_Repeater
// Parameter: BCaps and BStatus
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark:
// Side-Effect: as Authentication
//////////////////////////////////////////////////////////////////////
static _XDATA BYTE KSVList[32] ;
static _XDATA BYTE Vr[20] ;
static _XDATA BYTE M0[8] ;
static _XDATA ULONG w[80];
static _XDATA ULONG digest[5];

#define rol(x,y) (((x) << (y)) | (((ULONG)x) >> (32-y)))

void SHATransform(ULONG * h)
{
	LONG t;

      
	for (t = 16; t < 80; t++) {
		ULONG tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
		w[t] = rol(tmp,1);
	}

	h[0] = 0x67452301 ;
	h[1] = 0xefcdab89;
	h[2] = 0x98badcfe;
	h[3] = 0x10325476;
	h[4] = 0xc3d2e1f0;

	for (t = 0; t < 20; t++) {
		ULONG tmp =
			rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t] + 0x5a827999;
		printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;

		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
		
	}
	for (t = 20; t < 40; t++) {
		ULONG tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0x6ed9eba1;
		printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	for (t = 40; t < 60; t++) {
		ULONG tmp = rol(h[0],
						 5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t] +
			0x8f1bbcdc;
		printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	for (t = 60; t < 80; t++) {
		ULONG tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0xca62c1d6;
		printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;

	h[0] += 0x67452301 ;
	h[1] += 0xefcdab89;
	h[2] += 0x98badcfe;
	h[3] += 0x10325476;
	h[4] += 0xc3d2e1f0;
	printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
}

/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end,and pass those blocks to the core SHA algorithm.
 */


void SHA_Simple(void *p,LONG len,BYTE *output)
{
	// SHA_State s;
    int i, t ;
    ULONG c ;
    char *pBuff = p ;
    
    
    for( i = 0 ; i < len ; i++ )
    {
        t = i/4 ;
        if( i%4 == 0 )
        {
            w[t] = 0 ;
        }
        c = pBuff[i] ;
        c <<= (3-(i%4))*8 ;
        w[t] |= c ;
        printf("pBuff[%d] = %02x, c = %08lX, w[%d] = %08lX\n",i,pBuff[i],c,t,w[t]) ;
    }
    t = i/4 ;
    if( i%4 == 0 )
    {
        w[t] = 0 ;
    }
    c = 0x80 << ((3-i%4)*24) ;
    w[t]|= c ; t++ ;
    for( ; t < 15 ; t++ )
    {
        w[t] = 0 ;
    }
    w[15] = len*8  ;
    
    for( t = 0 ; t< 16 ; t++ )
    {
        printf("w[%2d] = %08lX\n",t,w[t]) ;
    }

    SHATransform(digest) ;
    
    for( i = 0 ; i < 20 ; i+=4 )
    {
        output[i]   = (BYTE)((digest[i/4]>>24)&0xFF) ;
        output[i+1] = (BYTE)((digest[i/4]>>16)&0xFF) ;
        output[i+2] = (BYTE)((digest[i/4]>>8) &0xFF) ;
        output[i+3] = (BYTE)(digest[i/4]      &0xFF) ;
    }
}

static void
HDCP_CancelRepeaterAuthenticate(void)
{
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("HDCP_CancelRepeaterAuthenticate") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    AbortDDC() ;
    HDMITX_WriteI2C_Byte(REG_LISTCTRL,B_LISTFAIL|B_LISTDONE) ;
    HDCP_ClearAuthInterrupt() ;
}

static void
HDCP_ResumeRepeaterAuthenticate(void)
{
    HDMITX_WriteI2C_Byte(REG_LISTCTRL,B_LISTDONE) ;
	HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERHDCP) ;
}


static SYS_STATUS
HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream)
{
    BYTE TimeOut = 100 ;
	BYTE ucdata ;
	
	if(cDownStream == 0 || pKSVList == NULL)
	{
	    return ER_FAIL ;
	}
	
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,0x74) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x43) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,cDownStream * 5) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {

        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetKSVList(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetKSVList(): DDC Fail by REG_DDC_STATUS = %x.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            return ER_FAIL ;
        }
        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("HDCP_GetKSVList(): KSV") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        pKSVList[TimeOut] = HDMITX_ReadI2C_Byte(REG_DDC_READFIFO) ;
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF(" %02X",pKSVList[TimeOut]) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }
#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
	return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetVr(BYTE *pVr)
{
    BYTE TimeOut  ;
	BYTE ucdata ;
	
	if(pVr == NULL)
	{
	    return ER_FAIL ;
	}
	
    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,0x74) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x20) ;
    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,20) ;
    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetVr(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_GetVr(): DDC fail by REG_DDC_STATUS = %x.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            return ER_FAIL ;
        }
        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
#ifdef CAT6611_DRV_DEBUG_ON
       // ErrorF("HDCP_GetVr(): DDC fail by timeout.\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(0) ;

    for(TimeOut = 0 ; TimeOut < 5 ; TimeOut++)
    {
        HDMITX_WriteI2C_Byte(REG_SHA_SEL ,TimeOut) ;
        pVr[TimeOut*4+3]  = (ULONG)HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE1) ;
        pVr[TimeOut*4+2] = (ULONG)HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE2) ;
        pVr[TimeOut*4+1] = (ULONG)HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE3) ;
        pVr[TimeOut*4] = (ULONG)HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE4) ;
#ifdef CAT6611_DRV_DEBUG_ON
		ErrorF("V' = %02X %02X %02X %02X\n",pVr[TimeOut*4],pVr[TimeOut*4+1],pVr[TimeOut*4+2],pVr[TimeOut*4+3]) ; 
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    }
    
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetM0(BYTE *pM0)
{
	int i ;

    if(!pM0)
    {
        return ER_FAIL ;
    }
    
    HDMITX_WriteI2C_Byte(REG_SHA_SEL,5) ; // read m0[31:0] from reg51~reg54
    pM0[0] = HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE1) ;
    pM0[1] = HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE2) ;
    pM0[2] = HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE3) ;
    pM0[3] = HDMITX_ReadI2C_Byte(REG_SHA_RD_BYTE4) ;
    HDMITX_WriteI2C_Byte(REG_SHA_SEL,0) ; // read m0[39:32] from reg55
    pM0[4] = HDMITX_ReadI2C_Byte(REG_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_SHA_SEL,1) ; // read m0[47:40] from reg55
    pM0[5] = HDMITX_ReadI2C_Byte(REG_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_SHA_SEL,2) ; // read m0[55:48] from reg55
    pM0[6] = HDMITX_ReadI2C_Byte(REG_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_SHA_SEL,3) ; // read m0[63:56] from reg55
    pM0[7] = HDMITX_ReadI2C_Byte(REG_AKSV_RD_BYTE5) ;

#ifdef CAT6611_DRV_DEBUG_ON
    ErrorF("M[] =") ;
	for(i = 0 ; i < 8 ; i++){
		ErrorF("0x%02x,",pM0[i]) ;
	}
	ErrorF("\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    return ER_SUCCESS ;
}

static _XDATA BYTE SHABuff[64] ;
static _XDATA BYTE V[20] ;

static SYS_STATUS
HDCP_CheckSHA(BYTE pM0[],USHORT BStatus,BYTE pKSVList[],int cDownStream,BYTE Vr[])
{
    int i,n ;
    
    for(i = 0 ; i < cDownStream*5 ; i++)
    {
        SHABuff[i] = pKSVList[i] ;
    }
    SHABuff[i++] = BStatus & 0xFF ;
    SHABuff[i++] = (BStatus>>8) & 0xFF ;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        SHABuff[i] = pM0[n] ;
    }
    n = i ;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        SHABuff[i] = 0 ;
    }
    // n = cDownStream * 5 + 2 /* for BStatus */ + 8 /* for M0 */ ;
    // n *= 8 ;
    // SHABuff[62] = (n>>8) & 0xff ;
    // SHABuff[63] = (n>>8) & 0xff ;
    for(i = 0 ; i < 64 ; i++)
	{
		if(i % 16 == 0) printf("SHA[]: ") ;
		printf(" %02X",SHABuff[i]) ;
		if((i%16)==15) printf("\n") ;
	}
    SHA_Simple(SHABuff,n,V) ;
    printf("V[] =") ;
    for(i = 0 ; i < 20 ; i++)
    {
        printf(" %02X",V[i]) ;
    }
    printf("\nVr[] =") ;
    for(i = 0 ; i < 20 ; i++)
    {
        printf(" %02X",Vr[i]) ;
    }
        
    for(i = 0 ; i < 20 ; i++)
    {
        if(V[i] != Vr[i])
        {
            return ER_FAIL ;
        }
    }
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_Authenticate_Repeater()
{
    BYTE uc ;
    #ifdef SUPPORT_DSSSHA    
    BYTE revoked ;
    int i ;
    #endif // _DSS_SHA_
	// BYTE test;
	// BYTE test06;
	// BYTE test07;
	// BYTE test08;
    BYTE cDownStream ;

    BYTE BCaps;
    USHORT BStatus ;
    USHORT TimeOut ;

#ifdef CAT6611_DRV_DEBUG_ON
	ErrorF("Authentication for repeater\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
    // emily add for test,abort HDCP
    // 2007/10/01 marked by jj_tseng@chipadvanced.com
    // HDMITX_WriteI2C_Byte(0x20,0x00) ;
    // HDMITX_WriteI2C_Byte(0x04,0x01) ;
	// HDMITX_WriteI2C_Byte(0x10,0x01) ;
	// HDMITX_WriteI2C_Byte(0x15,0x0F) ;
	// DelayMS(100);
    // HDMITX_WriteI2C_Byte(0x04,0x00) ;
	// HDMITX_WriteI2C_Byte(0x10,0x00) ;
	// HDMITX_WriteI2C_Byte(0x20,0x01) ;
	// DelayMS(100);
	// test07 = HDMITX_ReadI2C_Byte(0x7)  ;
	// test06 = HDMITX_ReadI2C_Byte(0x6);
	// test08 = HDMITX_ReadI2C_Byte(0x8);
	//~jj_tseng@chipadvanced.com
	// end emily add for test
    //////////////////////////////////////
    // Authenticate Fired
    //////////////////////////////////////
  
    HDCP_GetBCaps(&BCaps,&BStatus) ;
	DelayMS(2);
    HDCP_Auth_Fire();
	DelayMS(550); // emily add for test

    for(TimeOut = 250*6 ; TimeOut > 0 ; TimeOut --)
    {

        uc = HDMITX_ReadI2C_Byte(REG_INT_STAT1) ;
        if(uc & B_INT_DDC_BUS_HANG)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("DDC Bus hang\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            goto HDCP_Repeater_Fail ;
        }

        uc = HDMITX_ReadI2C_Byte(REG_INT_STAT2) ;

        if(uc & B_INT_AUTH_FAIL)
        {
			/*
            HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_AUTH_FAIL) ;
            HDMITX_WriteI2C_Byte(REG_INT_CLR1,0) ;
            HDMITX_WriteI2C_Byte(REG_SYS_STATUS,B_INTACTDONE) ;
            HDMITX_WriteI2C_Byte(REG_SYS_STATUS,0) ;
			*/
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("HDCP_Authenticate_Repeater(): B_INT_AUTH_FAIL.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            goto HDCP_Repeater_Fail ;
        }
        // emily add for test
		// test =(HDMITX_ReadI2C_Byte(0x7)&0x4)>>2 ;
        if(uc & B_INT_KSVLIST_CHK)
        {
            HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_KSVLISTCHK) ;
            HDMITX_WriteI2C_Byte(REG_INT_CLR1,0) ;
            HDMITX_WriteI2C_Byte(REG_SYS_STATUS,B_INTACTDONE) ;
            HDMITX_WriteI2C_Byte(REG_SYS_STATUS,0) ;
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("B_INT_KSVLIST_CHK\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            break ;
        }

        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Time out for wait KSV List checking interrupt\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        goto HDCP_Repeater_Fail ;
    }

    ///////////////////////////////////////
    // clear KSVList check interrupt.
    ///////////////////////////////////////

    for(TimeOut = 500 ; TimeOut > 0 ; TimeOut --)
    {
		if((TimeOut % 100) == 0)
		{
#ifdef CAT6611_DRV_DEBUG_ON
		    ErrorF("Wait KSV FIFO Ready %d\n",TimeOut) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
		}
		
        if(HDCP_GetBCaps(&BCaps,&BStatus) == ER_FAIL)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("Get BCaps fail\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
            goto HDCP_Repeater_Fail ;
        }

        if(BCaps & B_CAP_KSV_FIFO_RDY)
        {
#ifdef CAT6611_DRV_DEBUG_ON
			 ErrorF("FIFO Ready\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
			 break ;
        }
        DelayMS(5) ;

    }

    if(TimeOut == 0)
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Get KSV FIFO ready TimeOut\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        goto HDCP_Repeater_Fail ;
    }

#ifdef CAT6611_DRV_DEBUG_ON
	ErrorF("Wait timeout = %d\n",TimeOut) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
	
    ClearDDCFIFO() ;
    GenerateDDCSCLK() ;
    cDownStream =  (BStatus & M_DOWNSTREAM_COUNT) ;
    
    if(cDownStream == 0 || cDownStream > 6 || BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
#ifdef CAT6611_DRV_DEBUG_ON
        ErrorF("Invalid Down stream count,fail\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
        goto HDCP_Repeater_Fail ;
    }
    

    if(HDCP_GetKSVList(KSVList,cDownStream) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

    #ifdef SUPPORT_DSSSHA    
    for(i = 0 ; i < cDownStream ; i++)
    {
		revoked=FALSE ;
        HDCP_VerifyRevocationList(SRM1,&KSVList[i*5],&revoked) ;
        if(revoked)
        {
#ifdef CAT6611_DRV_DEBUG_ON
            ErrorF("KSVFIFO[%d] = %02X %02X %02X %02X %02X is revoked\n",i,KSVList[i*5],KSVList[i*5+1],KSVList[i*5+2],KSVList[i*5+3],KSVList[i*5+4]) ; 
#endif //#ifdef CAT6611_DRV_DEBUG_ON
			 goto HDCP_Repeater_Fail ;
        }
    }
    #endif

    
    if(HDCP_GetVr(Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ; 
    }

    if(HDCP_GetM0(M0) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }
    
    // do check SHA
    if(HDCP_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }
    
    
    HDCP_ResumeRepeaterAuthenticate() ;
	bAuthenticated = TRUE ;
    return ER_SUCCESS ;
    
HDCP_Repeater_Fail:
    HDCP_CancelRepeaterAuthenticate() ;
    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_ResumeAuthentication
// Parameter: N/A
// Return: N/A
// Remark: called by interrupt handler to restart Authentication and Encryption.
// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////

static void
HDCP_ResumeAuthentication(void)
{
    SetAVMute(TRUE) ;
    if(HDCP_Authenticate() == ER_SUCCESS)
	{
		HDCP_EnableEncryption() ;
	}
	SetAVMute(FALSE) ;
}



#endif // SUPPORT_HDCP
//
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_Auth_Fire()
//// Parameter: N/A
//// Return: N/A
//// Remark: write anything to reg21 to enable HDCP authentication by HW
//// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////////////////
//
//static void
//HDCP_Auth_Fire()
//{
#ifdef CAT6611_DRV_DEBUG_ON
//    // ErrorF("HDCP_Auth_Fire():\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHDCP) ; // MASTERHDCP, no need command but fire.
//	HDMITX_WriteI2C_Byte(REG_AUTHFIRE, 1);
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_StartAnCipher
//// Parameter: N/A
//// Return: N/A
//// Remark: Start the Cipher to free run for random number. When stop, An is
////         ready in Reg30.
//// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////////////////
//
//static void
//HDCP_StartAnCipher()
//{
//    HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_START_CIPHER_GEN) ;
//    DelayMS(1) ; // delay 1 ms
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_StopAnCipher
//// Parameter: N/A
//// Return: N/A
//// Remark: Stop the Cipher, and An is ready in Reg30.
//// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////////////////
//
//static void
//HDCP_StopAnCipher()
//{
//    HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_STOP_CIPHER_GEN) ;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_GenerateAn
//// Parameter: N/A
//// Return: N/A
//// Remark: start An ciper random run at first, then stop it. Software can get
////         an in reg30~reg38, the write to reg28~2F
//// Side-Effect:
//////////////////////////////////////////////////////////////////////////////////
//
//static void
//HDCP_GenerateAn()
//{
//    BYTE Data[8] ;
//
//    HDCP_StartAnCipher() ;
//    // HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_START_CIPHER_GEN) ;
//    // DelayMS(1) ; // delay 1 ms
//    // HDMITX_WriteI2C_Byte(REG_AN_GENERATE,B_STOP_CIPHER_GEN) ;
//
//    HDCP_StopAnCipher() ;
//
//    Switch_HDMITX_Bank(0) ;
//    // new An is ready in reg30
//    HDMITX_ReadI2C_ByteN(REG_AN_GEN,Data,8) ;
//    HDMITX_WriteI2C_ByteN(REG_AN,Data,8) ;
//
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_GetBCaps
//// Parameter: pBCaps - pointer of byte to get BCaps.
////            pBStatus - pointer of two bytes to get BStatus
//// Return: ER_SUCCESS if successfully got BCaps and BStatus.
//// Remark: get B status and capability from HDCP reciever via DDC bus.
//// Side-Effect:
//////////////////////////////////////////////////////////////////////////////////
//
//static SYS_STATUS
//HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus)
//{
//    BYTE ucdata ;
//    BYTE TimeOut ;
//
//    Switch_HDMITX_Bank(0) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,DDC_HDCP_ADDRESS) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x40) ; // BCaps offset
//    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,3) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;
//
//    for( TimeOut = 200 ; TimeOut > 0 ; TimeOut -- )
//    {
//        DelayMS(1) ;
//
//        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
//        if( ucdata & B_DDC_DONE )
//        {
#ifdef CAT6611_DRV_DEBUG_ON
//            ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//            break ;
//        }
//
//        if( ucdata & B_DDC_ERROR )
//        {
#ifdef CAT6611_DRV_DEBUG_ON
//            ErrorF("HDCP_GetBCaps(): DDC Error. Reg16 = %02X.\n", ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//            return ER_FAIL ;
//        }
//
//    }
//
//    if(TimeOut == 0)
//    {
//        return ER_FAIL ;
//    }
//
//    HDMITX_ReadI2C_ByteN(REG_BSTAT,(PBYTE)pBStatus,2) ;
//    *pBCaps = HDMITX_ReadI2C_Byte(REG_BCAP) ;
//    return ER_SUCCESS ;
//
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_GetBKSV
//// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
//// Return: ER_SUCCESS if successfuly got BKSV from Rx.
//// Remark: Get BKSV from HDCP reciever.
//// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////////////////
//
//static SYS_STATUS
//HDCP_GetBKSV(BYTE *pBKSV)
//{
//    BYTE ucdata ;
//    BYTE TimeOut ;
//
//    Switch_HDMITX_Bank(0) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_HEADER,DDC_HDCP_ADDRESS) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_REQOFF,0x00) ; // BKSV offset
//    HDMITX_WriteI2C_Byte(REG_DDC_REQCOUNT,5) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;
//
//    for( TimeOut = 200 ; TimeOut > 0 ; TimeOut -- )
//    {
//        DelayMS(1) ;
//
//        ucdata = HDMITX_ReadI2C_Byte(REG_DDC_STATUS) ;
//        if( ucdata & B_DDC_DONE )
//        {
#ifdef CAT6611_DRV_DEBUG_ON
//            ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//            break ;
//        }
//
//        if( ucdata & B_DDC_ERROR )
//        {
#ifdef CAT6611_DRV_DEBUG_ON
//            ErrorF("HDCP_GetBCaps(): DDC Error. reg16 = %02X.\n", ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//            return ER_FAIL ;
//        }
//    }
//
//    if(TimeOut == 0)
//    {
//        return ER_FAIL ;
//    }
//
//    HDMITX_ReadI2C_ByteN(REG_BKSV,(PBYTE)pBKSV,5) ;
//
//    return ER_SUCCESS ;
//}
//
//////////////////////////////////////////////////////////////////////////////////
//// Function:HDCP_Authenticate
//// Parameter: N/A
//// Return: ER_SUCCESS if Authenticated without error.
//// Remark: do Authentication with Rx
//// Side-Effect:
////  1. bAuthenticated global variable will be TRUE when authenticated.
////  2. Auth_done interrupt and AUTH_FAIL interrupt will be enabled.
//////////////////////////////////////////////////////////////////////////////////
//static void
//HDCP_ResetAuthenticate()
//{
//    BYTE ucdata ;
//    
//    HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE, 0) ;
//    HDMITX_WriteI2C_Byte(REG_LISTCTRL, REG_LISTCTRL) ;
//    HDMITX_WriteI2C_Byte(REG_DDC_MASTER_CTRL, B_MASTERHOST) ;
//    ucdata = HDMITX_ReadI2C_Byte(REG_SW_RST) | B_HDCP_RST ;
//    HDMITX_WriteI2C_Byte(REG_SW_RST, ucdata) ;
//    AbortDDC() ;
//    ClearDDCFIFO() ;
//    ManualCheckRiCounter = 0 ;
//
//    bAuthenticated = FALSE ;
//}
//

//
//
//static SYS_STATUS
//HDCP_Authenticate()
//{
//    BYTE ucdata ;
//    BYTE BCaps ;
//    USHORT BStatus ;
//    USHORT TimeOut ;
//    USHORT cDownStream ;
//
//    BYTE BKSV[5] ;
//
//    bAuthenticated = FALSE ;
//
//    HDCP_ResetAuthenticate() ; // reset HDCP authenticate
//
//    // Authenticate should be called after AFE setup up.
//
#ifdef CAT6611_DRV_DEBUG_ON
//    ErrorF("HDCP_Authenticate(): ") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//
//    if( HDCP_GetBCaps( &BCaps, &BStatus ) != ER_SUCCESS )
//    {
#ifdef CAT6611_DRV_DEBUG_ON
//        ErrorF("HDCP_GetBCaps fail.\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//        return ER_FAIL ;
//    }
#ifdef CAT6611_DRV_DEBUG_ON
//    // ErrorF("BCaps %02X BStatus %04X\n",BCaps, BStatus) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//
//    HDCP_GetBKSV(BKSV) ;
//
//    bManualCheckRi = FALSE ;
//    for( cDownStream = 0 ; cDownStream < 5 ; cDownStream++ )
//    {
//        if( BKSV[cDownStream] == 0xFF )
//        {
//            bManualCheckRi = TRUE ;
//            return TRUE ;
//        }
//    }
//
//    if( BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER) ) // if MAX_CASCADE_EXCEEDED or MAX_DEVICE_EXCEEDED
//    {
//        return ER_FAIL ; // down stream level is out of cascaded or too much device connected.
//    }
//
//    Switch_HDMITX_Bank(0) ; // switch bank action should start on direct register writting of each function.
//
//    // 2006/08/11 added by jjtseng
//    // enable HDCP on CPDired enabled.
//    HDMITX_AndReg_Byte(REG_SW_RST,~(B_HDCP_RST)) ;
//    //~jjtseng 2006/08/11
//
////    if( BCaps & B_CAP_HDCP_1p1 )
////    {
#ifdef CAT6611_DRV_DEBUG_ON
////        ErrorF("RX support HDCP 1.1\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
////        HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE, B_ENABLE_HDPC11|B_CPDESIRE) ;
////    }
////    else
////    {
#ifdef CAT6611_DRV_DEBUG_ON
////        ErrorF("RX not support HDCP 1.1\n") ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//    HDMITX_WriteI2C_Byte(REG_HDCP_DESIRE, B_CPDESIRE) ;
////    }
//    cDownStream =  (BStatus & M_DOWNSTREAM_COUNT)+1 ;
//
//    HDMITX_WriteI2C_Byte(REG_INT_CLR0,B_CLR_AUTH_DONE|B_CLR_AUTH_FAIL) ;
//    HDMITX_WriteI2C_Byte(REG_INT_CLR1, 0 ) ; // don't clear other settings.
//    ucdata = HDMITX_ReadI2C_Byte(REG_SYS_STATUS) ;
//    ucdata = (ucdata & M_CTSINTSTEP) | B_INTACTDONE ;
//    HDMITX_WriteI2C_Byte(REG_SYS_STATUS, ucdata ) ; // clear action.
//
//    HDMITX_AndReg_Byte(REG_INT_MASK2,~(B_AUTH_FAIL_MASK|B_AUTH_DONE_MASK)) ;    // enable GetBCaps Interrupt
#ifdef CAT6611_DRV_DEBUG_ON
//    // ErrorF("int2 = %02X DDC_Status = %02X\n",HDMITX_ReadI2C_Byte(REG_INT_STAT2), HDMITX_ReadI2C_Byte(REG_DDC_STATUS)) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//
//    HDCP_GenerateAn() ;
//
//    HDCP_Auth_Fire();
//
//    // wait for status ;
//    bAuthenticated = FALSE ;
//    for(TimeOut = cDownStream*250 ; TimeOut > 0 ; TimeOut -- )
//    {
//        DelayMS(1) ; // delay 1ms
//        ucdata = HDMITX_ReadI2C_Byte(REG_AUTH_STAT) ;
#ifdef CAT6611_DRV_DEBUG_ON
//        ErrorF("reg46 = %02x\n",ucdata) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//
//        if( ucdata & B_AUTH_DONE)
//        {
//            bAuthenticated = TRUE ;
//            break ;
//        }
//    }
//
//    if( TimeOut == 0 )
//    {
#ifdef CAT6611_DRV_DEBUG_ON
//         ErrorF("HDCP_Authenticate(): Time out. return fail\n" ) ;
#endif //#ifdef CAT6611_DRV_DEBUG_ON
//         bAuthenticated = FALSE ;
//         return ER_FAIL ;
//    }
//    return ER_SUCCESS ;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
//// Function: HDCP_ResumeAuthentication
//// Parameter: N/A
//// Return: N/A
//// Remark: called by interrupt handler to restart Authentication and Encryption.
//// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////////////////
//
//static void
//HDCP_ResumeAuthentication()
//{
//    SetAVMute(TRUE) ;
//    HDCP_Authenticate() ;
//    HDCP_EnableEncryption() ;
//    SetAVMute(FALSE) ;
//}
//
//SYS_STATUS
//HDCP_EnableEncryption()
//{
//    Switch_HDMITX_Bank(0) ;
//	return HDMITX_WriteI2C_Byte(REG_ENCRYPTION, B_ENABLE_ENCRYPTION);
//}
//

////////////////////////////////////////////////////////////////////////////////
// Function: DumpCat6611Reg()
// Parameter: N/A
// Return: N/A
// Remark: Debug function, dumps the registers of CAT6611.
// Side-Effect: N/A
////////////////////////////////////////////////////////////////////////////////

#ifdef CAT6611_DRV_DEBUG_ON
void
DumpCat6611Reg(void)
{
    int i,j ; 
    BYTE ucData ;
    
	 ErrorF("\n ========   This is testing log   =========\n");

    ErrorF("       ") ;
    for( j = 0 ; j < 16 ; j++ )
    {
        ErrorF(" %02X",j) ;
        if( (j == 3)||(j==7)||(j==11))
        {
            ErrorF("  ") ;
        }
    }
    ErrorF("\n        -----------------------------------------------------\n") ;

    Switch_HDMITX_Bank(0) ;

    for(i = 0 ; i < 0x100 ; i+=16 )
    {
        ErrorF("[%3X]  ",i) ;
        for( j = 0 ; j < 16 ; j++ )
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF)) ;
            ErrorF(" %02X",ucData) ;
            if( (j == 3)||(j==7)||(j==11))
            {
                ErrorF(" -") ;
            }
        }
        ErrorF("\n") ;
        if( (i % 0x40) == 0x30)
        {
            ErrorF("        -----------------------------------------------------\n") ;
        }
    }

    Switch_HDMITX_Bank(1) ;
    for(i = 0x130; i < 0x1B0 ; i+=16 )
    {
        ErrorF("[%3X]  ",i) ;
        for( j = 0 ; j < 16 ; j++ )
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF)) ;
            ErrorF(" %02X",ucData) ;
            if( (j == 3)||(j==7)||(j==11))
            {
                ErrorF(" -") ;
            }
        }
        ErrorF("\n") ;
        if( i == 0x160 )
        {
            ErrorF("        -----------------------------------------------------\n") ;
        }
        
    }
    Switch_HDMITX_Bank(0) ;
}
#endif //#ifdef CAT6611_DRV_DEBUG_ON

