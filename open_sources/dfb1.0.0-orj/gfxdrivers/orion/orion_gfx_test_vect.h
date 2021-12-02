/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_test_vect.h
// MODULE NAME      : test vect generation
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-13
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-13   jiasheng Chen           Original
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Test Vector
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#ifndef _ORION_GFX_TEST_VECT_H_
#define _ORION_GFX_TEST_VECT_H_
#include "orion_gfx_type_def.h"
#include "orion_gfx_2dlib.h"

#define GEN_GFX_TEST_VECT    (0)

#if GEN_GFX_TEST_VECT

	void _GfxTVCMdQue     (I32 IsCntInc, char *fmt, ...);
	void _GfxTVS0Req      (char *fmt, ...);
	void _GfxTVS0DataIn   (char *fmt, ...);
	void _GfxTVS1Req      (char *fmt, ...);
	void _GfxTVS1DataIn   (char *fmt, ...);
	void _GfxTVDDataOut   (char *fmt, ...);

	void _GfxS0FifoIn     (char *fmt, ...);
	void _GfxS0FifoOut    (char *fmt, ...);

	void _GfxScalorData   (char *fmt, ...);


	I32 _GfxTVS0ImgDramData(COLOR_IMG *S0Img);
	I32 _GfxTVS1ImgDramData(COLOR_IMG *S1Img);
	I32 _GfxTVDImgInitDramData(COLOR_IMG *DImg);
	I32 _GfxTVDImgInitDramDataRaw(COLOR_IMG *DImg);
	I32 _GfxTVDImgDramData(COLOR_IMG *DImg);
	I32 _GfxTVARMSimStart();
	I32 _GfxTVARMSimEnd();
	void _GfxTVNewCMDGroup(); 
	void _InitGfxTestVect();
	void _EndGfxTestVect ();
	#if (!ENABLE_TRACE_S_DATA_AT_ARBITOR)
	void TraceSrcImgData (U32 StartAddr, 
	                      U32 LinePitch, 
	                      U32 LineWidthInBytes, 
	                      U32 LineNum, 
	                      I32 SrcId, 
	                      I32 SrcVRevers);
	#endif
	#define GfxTVCMDQue      _GfxTVCMdQue
	#define GfxTVS0Req       _GfxTVS0Req
	#define GfxTVS0DataIn    _GfxTVS0DataIn
	#define GfxTVS1Req       _GfxTVS1Req
	#define GfxTVS1DataIn    _GfxTVS1DataIn
	#define GfxTVDDataOut    _GfxTVDDataOut
	

	#define GfxTVNewCMDGroup() _GfxTVNewCMDGroup()

	#if ENABLE_TRACE_GFX_S_FIFO
	#define GfxS0FifoIn      _GfxS0FifoIn
	#define GfxS0FifoOut     _GfxS0FifoOut
	#else
	#define GfxS0FifoIn       
	#define GfxS0FifoOut     
	#endif
	#define InitGfxTestVect()  _InitGfxTestVect()
	#define EndGfxTestVect()   _EndGfxTestVect()

	#define GfxTVS0ImgDramData _GfxTVS0ImgDramData
	#define GfxTVS1ImgDramData _GfxTVS1ImgDramData
	#define GfxTVDImgInitDramData _GfxTVDImgInitDramData
	#define GfxTVDImgDramData _GfxTVDImgDramData
	

#else	
	#define GfxTVCMDQue
	#define GfxTVS0Req
	#define GfxTVS0DataIn
	#define GfxTVS1Req
	#define GfxTVS1DataIn
	#define GfxTVDDataOut
	#define GfxScalorData
	
	#define GfxTVNewCMDGroup()
	
	#define GfxS0FifoIn 
	#define GfxS0FifoOut  
	

	#define InitGfxTestVect() 
	#define EndGfxTestVect()  

	#define GfxTVS0ImgDramData 
	#define GfxTVS1ImgDramData 
	#define GfxTVDImgInitDramData
	#define GfxTVDImgDramData 
#endif

#if ENABLE_TRACE_GFX_SCALOR_FIR
	#define GfxScalorData    _GfxScalorData
#else
	#define GfxScalorData
#endif

#endif
