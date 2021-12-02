/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_api.h
// MODULE NAME      : Gfx Engine Basic Operation API
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-02
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-02   jiasheng Chen           Original
// 0.2      07-05-14   Jiasheng Chen           Modified to support DirectFB
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Gfx Engine Basic Operation API
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/
#ifndef _ORION_GFX_API_H_
#define _ORION_GFX_API_H_

#include "orion_gfx_type_def.h"
#include "orion_gfx_cmd_reg_def.h"
typedef enum _GFX_DC_UPDATE_TYPE_
{
	GFX_DC_S0_COLOR_FORMAT   = (1 << 0),
	GFX_DC_S0_DEFAULT_COLOR  = (1 << 1),
	GFX_DC_S0_COLORKEY_PARA  = (1 << 2),
	GFX_DC_S0_DRAM_PARA      = (1 << 3),

	GFX_DC_S1_COLOR_FORMAT   = (1 << 4),
	GFX_DC_S1_DEFAULT_COLOR  = (1 << 5),
	GFX_DC_S1_COLORKEY_PARA  = (1 << 6),
	GFX_DC_S1_DRAM_PARA      = (1 << 7),

	GFX_DC_D_COLOR_FORMAT    = (1 << 8),
	GFX_DC_D_DRAM_PARA       = (1 << 9),

	GFX_DC_CLUT4_TABLE       = (1 << 10),
	GFX_DC_CLUT8_TABLE       = (1 << 11),
	//Scalor
	GFX_DC_SCALOR_HFIR_COFF  = (1 << 12),
	GFX_DC_SCALOR_VFIR_COFF  = (1 << 13),
	GFX_DC_SCALOR_INIT_PHASE = (1 << 14),
	GFX_DC_CTRL              = (1 << 31),	
	GFX_DC_ALL               = (~0x0),
	
}GFX_DC_UPDATE_TYPE;
typedef struct _GFX_DEVICE_CONTEXT_
{
//Run Time Info
	I32 GfxIdelStatus;
	I32 AHBErrStatus;
	I32 GfxErrStatus;
	I32 InterruptStatus;                 //Interrupt Satatus
	U32 CMDCnt;
//Clut Table:
	I32 UpdateClut4Table;                //1 Update, 0 donot Update
	I32 UpdateClut4TableMode;            //0 Update all, 1 Update accoding Idx                               
	U32 Clut4TableIdx;
	U32 Clut4Table[CLUT4_TABLE_LEN];
	I32 UpdateClut8Table;                //1 Update, 0 donot Update
	I32 UpdateClut8TableMode;            //0 Update all, 1 Update accoding Idx
	U32 Clut8TableIdx;
	U32 Clut8Table[CLUT8_TABLE_LEN];
//RunTimeControl
	I32 InterruptEnable;
//S0 Control
	I32 S0Enable;
	I32 S0VReverseScan;
	I32 S0FetchDram;                     // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
	                                     // if S0 Using Default Color then S0ColorFormat should be ARGB565 or A0
	GFX_COLOR_FORMAT S0ColorFormat;
	U32 S0Alpha0;   //For ARGB1555 Alpha Value 0
	U32 S0Alpha1;	//For ARGB1555 Alpha Value 1
	GFX_ARGB_COLOR   S0DefaultColor;     //ARGB8888 Format
	U32 S0BaseAddr;                      //Bytes Addr
	U32 S0LinePitch;                     //Bytes Addr
	U32 S0SkipPixelLine;            //Valid Only When Clut4
	U32 S0PixelNumOneLine;
	U32 S0TotalLineNum;	
	GFX_MEM_BYTE_ENDIAN_MODE    S0ByteEndian;
	GFX_MEM_NIBBLE_ENDIAN_MODE  S0NibbleEndian;
	GFX_16BIT_ENDIAN_MODE       S016BitEndian;
	I32 S0ClutEnable;
	I32 S0ColorKeyEnable;
	GFX_ARGB_COLOR S0ColorKeyMin;
	GFX_ARGB_COLOR S0ColorKeyMax;
	
//S1 Control
	I32 S1Enable;
	I32 S1VReverseScan;
	I32 S1FetchDram;                     // 1 Fetch, 0 do not Fetch Using Default Color in ARGB
	                                     // if S1 Using Default Color then S1ColorFormat should be ARGB565 or A0
	GFX_COLOR_FORMAT S1ColorFormat;
	U32 S1Alpha0;   //For ARGB1555 Alpha Value 0
	U32 S1Alpha1;	//For ARGB1555 Alpha Value 1

	GFX_ARGB_COLOR   S1DefaultColor;
	U32 S1BaseAddr;                      //Bytes Addr
	U32 S1LinePitch;                     //Bytes Addr
	U32 S1SkipPixelLine;            //Valid Only When Clut4
	U32 S1PixelNumOneLine;
	U32 S1TotalLineNum;	
	GFX_MEM_BYTE_ENDIAN_MODE    S1ByteEndian;
	GFX_MEM_NIBBLE_ENDIAN_MODE  S1NibbleEndian;
	GFX_16BIT_ENDIAN_MODE       S116BitEndian;

	I32 S1ClutEnable;
	I32 S1ColorKeyEnable;
	GFX_ARGB_COLOR S1ColorKeyMin;
	GFX_ARGB_COLOR S1ColorKeyMax;

    //ROP and Compositor
	I32 CompositorEnable;
	U32 ROPAlphaCtrl;
	ROP_OPT RopValue;
	I32 S0OnTopS1;                        // 1 S0 On Top S1
	//D Dram Para
	I32 DVReverseScan;
	U32 DBaseAddr;                        //Bytes Addr
	U32 DLinePitch;                       //Bytes Addr
	U32 DSkipPixelLine;               //Valid Only When Clut4
	U32 DPixelNumOneLine;
	U32 DTotalLineNum;
	GFX_COLOR_FORMAT DColorFormat;
	U32 DAlpha0Min;
	U32 DAlpha0Max;
	GFX_MEM_BYTE_ENDIAN_MODE    DByteEndian;
	GFX_MEM_NIBBLE_ENDIAN_MODE  DNibbleEndian;	
	GFX_16BIT_ENDIAN_MODE       D16BitEndian;

	//Endian Ctrl
	I32 IsUsingSDSpecificEndian; //0, Global Endian, 1 Specifica Endian
	//Scalor Control
	U32 S0ScalorEnable;
	U32 HInitialPhase;
	U32 VInitialPhase;
	I32 UpdateHFIRCoeff;
	I32 UpdateVFIRCoeff;
	U32 HFIRCoeffTable[SCALOR_PAHSE_NUM][SCALOR_H_FIR_TAP_NUM];
	U32 VFIRCoeffTable[SCALOR_PAHSE_NUM][SCALOR_V_FIR_TAP_NUM];
	GFX_DC_UPDATE_TYPE GfxDCType;
}GFX_DEVICE_CONTEXT;




void hGfxAPIHardWareReset();
U32 hGfxAPIGetCMDQueueDepth();
void hGfxAPISWReset();
I32 hGfxAPIWaitCMDQueueEmpty();

U32 hGfxAPIGetCMDGroupCnt();
void hGfxAPICMDGroupCntSyncHW();
I32 hGfxAPISendCMD(U32 GfxCMD);
I32 hGfxAPIWaitGfxToBeIdle(GFX_DEVICE_CONTEXT *hGfxDeviceContext); //Boolean Type Return
I32 hGfxAPIActiveDeviceContext( GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType);

I32 hGfxAPIRegCheckClutTable(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType, I32 IsClut4orClut8); //0: Clut4, 1: Clut8;
I32 hGfxAPIRegCheckPosStartup(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType);
I32 hGfxAPIRegCheckPreStartup(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType); //Boolean Type Return
#endif
