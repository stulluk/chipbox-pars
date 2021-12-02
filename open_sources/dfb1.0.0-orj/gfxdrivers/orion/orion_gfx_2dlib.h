/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_2dlib.h
// MODULE NAME      : 
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-23
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-02   jiasheng Chen           Original
// 0.2      07-05-14   Jiasheng Chen           Modified to support DirectFB
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Host 2d Operation Lib icluding Fill, COPY, Scalor, ColorKey, Compositer and ROP
// And Some Para Initial, For example: Clut4/8 Table and Scalor FIR Coeff Update
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#ifndef _ORION_GFX_2DLIB_H_
#define _ORION_GFX_2DLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "orion_gfx_type_def.h"
#include "orion_gfx_api.h"

//Color Suf Buf Manage
typedef struct _COLOR_IMG_
{
	U32 StartAddress;
    U32 PixelWidth; 
    U32 PixelHeight;
	U32 LinePitch;
    GFX_COLOR_FORMAT ColorFormat;
	GFX_ARGB_COLOR DefaultColor;
	U32 InitialType; // 0 Stable, 1 Zero Increase, 2,Do not Initial
	GFX_MEM_BYTE_ENDIAN_MODE    ByteEndian;
	GFX_MEM_NIBBLE_ENDIAN_MODE  NibbleEndian;
	GFX_16BIT_ENDIAN_MODE       TwoBytesEndian;
	//FILE *fpImgIn;
	//FILE *fpImgOut;
	I32 IsMemAllocated;
}COLOR_IMG;

typedef struct tagRECT
{
    U32 left;
    U32 top;
    U32 right;
    U32 bottom;
} RECT;

typedef enum _HGFX_2D_RESULT_
{
	HG2D_SUCCESS      =  0,
	HG2D_UNKNOW       =  (1 << 0),
	HG2D_INVALID_PARA =  (1 << 1),
	HG2D_HW_ERROR     =  (1 << 2),
	HG2D_FAILED_TO_GET_DEFAULT_CLUT4_TABLE = (1 <<  3),
	HG2D_FAILED_TO_GET_DEFAULT_CLUT8_TABLE = (1 <<  4),
	HG2D_FAILED_TO_GET_DEFAULT_VFIR_COEFF  = (1 <<  5),
	HG2D_FAILED_TO_GET_DEFAULT_HFIR_COEFF  = (1 <<  6),
	HG2D_FAILED_ACTIVE_DEVICE_CONTEXT      = (1 <<  7),
	HG2D_INVALID_DES_REC_VALUE             = (1 <<  8), 
	HG2D_INVALID_SRC_REC_VALUE             = (1 <<  9),
	HG2D_DIFFERENT_SRC_DES_RECT            = (1 << 10),
	HG2D_UNSUPPORTED_COLOR_FORMAT          = (1 << 11),
	HG2D_GFX_HW_ERROR                      = (1 << 31),

}HG2D_RESULT;

I32 IsRectValid( COLOR_IMG *Img, RECT *Rect); //Boolean return
I32 CmpRect    ( RECT *Rect0, RECT *Rect1); //Equal == 1 not Equal return 0
//Clut4&8 Table
//Gfx Initial
HG2D_RESULT hGfx2DInit();
HG2D_RESULT hGfx2DInitClut4Table ( GFX_DEVICE_CONTEXT * hGfxDC, U32 *Clut4Table);
U32 *hGfx2DGetDefaultClut4Table();
HG2D_RESULT hGfx2DSetDefaultClut4Table(GFX_DEVICE_CONTEXT * hGfxDC);
HG2D_RESULT hGfx2DInitClut8Table ( GFX_DEVICE_CONTEXT * hGfxDC, U32 *Clut8Table);
U32 *hGfx2DGetDefaultClut8Table();
HG2D_RESULT hGfx2DSetDefaultClut8Table(GFX_DEVICE_CONTEXT * hGfxDC);

//Fill 
HG2D_RESULT hGfx2DFill
( 
    GFX_DEVICE_CONTEXT *hGfxDC, 
    I32                 GfxSrcId,
    COLOR_IMG          *DesImg, 
    RECT               *DesRect, 
    GFX_ARGB_COLOR     *FillColor,
    ROP_OPT             RopValue 
);
//Copy, Copy with no Scalor, 
HG2D_RESULT hGfx2DCopy
( 
    GFX_DEVICE_CONTEXT  *hGfxDC,
    I32                  GfxSrcId,
    COLOR_IMG           *SrcImg, 
    RECT                *SrcRect, 
    I32                  SrcVReverse,
    COLOR_IMG           *DesImg, 
    RECT                *DesRect, 
    I32                  DesVReverse,
    ROP_OPT              RopValue 
    
);

//Scalor and Anti-flicker
//Coeff
U32 SCALOR_COEFF( U32 SignBit, U32 Nmerator, U32 denominator) ;
HG2D_RESULT hGfx2DInitScalorVFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC, U32 *VFIRCoeff);
U32 *hGfx2DGetDefaultScalorVFIRCoeff();
HG2D_RESULT hGfx2DSetDefaultScalorVFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC);

HG2D_RESULT hGfx2DInitScalorHFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC, U32 *HFIRCoeff);
U32 *hGfx2DGetDefaultScalorHFIRCoeff();
HG2D_RESULT hGfx2DSetDefaultScalorHFIRCoeff(GFX_DEVICE_CONTEXT *hGfxDC);

HG2D_RESULT hGfx2DScalor
( 
    GFX_DEVICE_CONTEXT  *hGfxDC, 
    COLOR_IMG           *SrcImg, 
    RECT                *SrcRect, 
    I32                  SrcVReverse,
    COLOR_IMG           *DesImg, 
    RECT                *DesRect, 
    I32                  DesVReverse,
    ROP_OPT              RopValue,
    U32                  VInitialPhase, 
    U32                  HInitialPhase
);

//
//Colokey Compositor and ROP
typedef struct _HGFX_BLT_SRC_PARA_
{
  COLOR_IMG *Img;
  RECT      *Rect;
  I32        VReverse;
  I32        ColorKeyEnable;
  GFX_ARGB_COLOR MinColor;
  GFX_ARGB_COLOR MaxColor;	
  
}HGFX_BLT_SRC_PARA;

typedef struct _HGFX_BLT_DES_PARA_
{
	I32        BlendEnable;
	I32        IsS0OnTopS1;// 1 S0 On Top S1
	U32        ROPAlphaCtrl;
	ROP_OPT    RopValue;
    COLOR_IMG *Img;
	RECT      *Rect;
	I32        VReverse;  
}HGFX_BLT_DES_PARA;

void CalcuDramPara
(
    COLOR_IMG *DesImg, 
    RECT      *DesRect,	
    I32        IsVReverse,
	U32       *pBaseAddr,                        //Bytes Addr
	U32       *pLinePitch,                       //Bytes Addr
	U32       *pSkipPixelLine               //Valid Only When Clut4
);

HG2D_RESULT hGfx2DBLT
(
	 GFX_DEVICE_CONTEXT *hGfxDC, 
     HGFX_BLT_SRC_PARA  *Src0Para,
     HGFX_BLT_SRC_PARA  *Src1Para, 
     HGFX_BLT_DES_PARA  *DesPara
);

HG2D_RESULT hGfx2DBLTSrc0
(
    GFX_DEVICE_CONTEXT *hGfxDC, 
    HGFX_BLT_SRC_PARA  *Src0Para,
    I32 Src1Enable,
    GFX_ARGB_COLOR     *Src1DefaultColor, 
    HGFX_BLT_DES_PARA  *DesPara
);

HG2D_RESULT hGfx2DBLTSrc1
(
    GFX_DEVICE_CONTEXT *hGfxDC, 
    HGFX_BLT_SRC_PARA  *Src1Para,
	I32 Src0Enable,
    GFX_ARGB_COLOR     *Src0DefaultColor, 
    HGFX_BLT_DES_PARA  *DesPara
);

#ifdef __cplusplus
}
#endif

#endif

