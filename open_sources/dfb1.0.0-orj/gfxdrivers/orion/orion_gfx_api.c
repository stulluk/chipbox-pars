/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_api.c
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
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "orion_gfx_api.h"
#include "orion_gfx_hw_if.h"
#include "orion_gfx_loglib.h"
#include "orion_gfx_test_vect.h"
#include "orion_gfx_sys_cfg.h"

//#define ORION_GFX_WAIT_IDEL_AFTER_SEND_CMD
//#define ORION_GFX_WAIT_IDEL_BEFORE_SEND_CMD

//#define TRACE_GFX_CMD_DATA
//#define TRACE_GFX_RUN_TIME_INFO
#ifdef ORION_GFX_WAIT_IDEL_AFTER_SEND_CMD
#define CHECK_REG_AFTER_SEND_CMD
#endif
//#define RESET_GFX_ONLY_ONE_TIME

#ifdef RESET_GFX_ONLY_ONE_TIME
static I32 IsGfxHWReset = 0;
static I32 IsGfxSWReset = 0;
#endif
//Hardware Reset
void hGfxAPIHardWareReset()
{
#ifdef RESET_GFX_ONLY_ONE_TIME
	if(IsGfxHWReset)
	{
		return;
	}
	else
	{
		IsGfxHWReset++;
	}	
#endif
	GfxHWReset();
	hGfxAPICMDGroupCntSyncHW();
	
}
/*
//Return GfxCMDQueuSpace, 0 means Command Queue Full
*/

I32 hGfxAPISendCMD(U32 GfxCMD)
{
	//Check Available Space
	U32 GfxCMDQueuSpace = 0;	
	do
	{
		GfxCMDQueuSpace = (GfxRegRead(GFX_REG_CMDQUE_EMPTY_CNT) >> 
		                   GFX_REG_CMDQUE_EMPTY_CNT_LSB) &
		                  ((1 << GFX_REG_CMDQUE_EMPTY_CNT_WIDTH) - 1);
		if (GfxCMDQueuSpace <= 0)
		{
			U32 GfxStatusRegValue = 0;
			I32 AHBErrStatus    = 0;
			I32 GfxErrStatus    = 0;
			I32 InterruptStatus = 0; 
			I32 InterruptEnable = 0;
			//Some User Defined Sleep Function
			ERROR_L2("Gfx Command Queue Full, Waiting.....\n");
			//Clear Interrupt and ERR
			GfxStatusRegValue = GfxRegRead(GFX_REG_STATUS);
			AHBErrStatus      = (GfxStatusRegValue >> GFX_AHB_ERR_BIT) &(1);
			GfxErrStatus      = (GfxStatusRegValue >> GFX_ERR_BIT    ) &(1);
			InterruptStatus   = (GfxStatusRegValue >> GFX_INT_STATUS_BIT) &(1);
			if (AHBErrStatus) 
			{
				ERROR("##ERROR: Gfx AHBErrStatus Err, Reset Gfx\n");
				assert(!AHBErrStatus);
				hGfxAPISWReset();
				continue;
			}
			//assert(!GfxErrStatus);
			if (GfxErrStatus)
			{
				ERROR("##ERROR: Gfx Command Interpret Err\n");
			}
			if (InterruptEnable || GfxErrStatus) //if Err Must Be a Interrupt 
			{
				if (InterruptStatus == 1)
				{
					//Clear Interrupt
					I32 IsGfxGenINTToHost = 0;
					IsGfxGenINTToHost = IsGfxGenInterrupt();
					assert(IsGfxGenINTToHost);//Hardware Should Generate a Interrupt
					GfxRegWrite(GFX_REG_INT_CLEAR, 1);//Write Any Value
				}
			}
		}
	}while(GfxCMDQueuSpace <= 0);
	
	if (GfxCMDQueuSpace > 0)
	{
		GfxRegWrite(GFX_REG_CMD_QUE, GfxCMD);
		#ifdef TRACE_GFX_CMD_DATA
		PRINT_D1_L2("GfxCMD: %08X\n",GfxCMD);
		#endif
	}
	return GfxCMDQueuSpace;
}
/*
//Before Call Set Active Context, make sure the the Command Queue Have enough Space 
//and Gfx's Interrupt Has been Cleared, it's Better the Status is Idle
*/
static U32 sGfxCMDGroupCnt = 0;
static void hGfxAPICMDGroupCntInc()
{
	sGfxCMDGroupCnt++;
}
void hGfxAPICMDGroupCntSyncHW()
{
//Sync with Hardware
	sGfxCMDGroupCnt = (GfxRegRead( GFX_REG_CMD_INFO ) >> REG_INFO_CMD_CNT_LSB) & 
	                  ((1 << REG_INFO_CMD_CNT_WIDTH) -1 );
}
U32 hGfxAPIGetCMDGroupCnt()
{
	return sGfxCMDGroupCnt;
}

I32 hGfxAPIActiveDeviceContext( GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType)
{
	//Send S0 Related Command
	U32 GfxCMD   = 0;
	U32 GfxCMDQueSpace = 0;
	U32 CompositorOperation = 0;
	U32 S0Operation     = 0;
	U32 S1Operation     = 0;
	U32 ScanCtrl        = 0;
	U32 S0EndianCtrl    = 0;
	U32 S1EndianCtrl    = 0;
	U32 DEndianCtrl     = 0;
	U32 GlobalEndianCtrl  = 0;
	if (hGfxDeviceContext == NULL)
	{
		return 0;
	}
	else  //Check Command Queue
	{
			//Check Available Space
		U32 GfxCMDQueuSpace = 0;
		U32 GfxStatusRegValue = 0;
		I32 AHBErrStatus    = 0;
		I32 GfxErrStatus    = 0;
		I32 InterruptStatus = 0; 
		I32 InterruptEnable = 0;		
		do
		{
			//Clear Interrupt and ERR
			GfxStatusRegValue = GfxRegRead(GFX_REG_STATUS);
			AHBErrStatus      = (GfxStatusRegValue >> GFX_AHB_ERR_BIT) &(1);
			GfxErrStatus      = (GfxStatusRegValue >> GFX_ERR_BIT    ) &(1);
			InterruptStatus   = (GfxStatusRegValue >> GFX_INT_STATUS_BIT) &(1);
			
			if (AHBErrStatus) 
			{
				ERROR("##ERROR: Gfx AHBErrStatus Err, Reset Gfx\n");
				assert(!AHBErrStatus);
				hGfxAPISWReset();
				continue;
			}
			//assert(!GfxErrStatus);
			if (GfxErrStatus)
			{
				ERROR("##ERROR: Gfx Command Interpret Err\n");
			}
			if (InterruptEnable || GfxErrStatus) //if Err Must Be a Interrupt 
			{
				if (InterruptStatus == 1)
				{
					//Clear Interrupt
					I32 IsGfxGenINTToHost = 0;
					IsGfxGenINTToHost = IsGfxGenInterrupt();
					assert(IsGfxGenINTToHost);//Hardware Should Generate a Interrupt
					GfxRegWrite(GFX_REG_INT_CLEAR, 1);//Write Any Value
				}
			}
			GfxCMDQueuSpace = (GfxRegRead(GFX_REG_CMDQUE_EMPTY_CNT) >> 
			                   GFX_REG_CMDQUE_EMPTY_CNT_LSB) &
			                  ((1 << GFX_REG_CMDQUE_EMPTY_CNT_WIDTH) - 1);
			if (GfxCMDQueuSpace < 32)
			{				//Some User Defined Sleep Function
				ERROR_D1_L2("Gfx Command Queue Space Less than %d, Waiting.....\n", 32);
			}			
		}while(GfxCMDQueuSpace < 32);
	}
	#ifdef ORION_GFX_WAIT_IDEL_BEFORE_SEND_CMD
	if (!hGfxAPIWaitGfxToBeIdle(NULL)) return 0;
	#endif
	if (hGfxDeviceContext->S0Enable || hGfxDeviceContext->S0ScalorEnable)
	{
		if ((hGfxDeviceContext->S0FetchDram || hGfxDeviceContext->S0ScalorEnable )&& 
			(GfxDCType & GFX_DC_S0_DRAM_PARA) )
		{			
			//Dram Access Para
			#if (ORION_VERSION == ORION_130)
			{
				assert((hGfxDeviceContext->S0BaseAddr  & 0x3) == 0);
				assert((hGfxDeviceContext->S0LinePitch & 0x3) == 0);
				GfxCMD = ((CMD_S0_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					     ((hGfxDeviceContext->S0SkipPixelLine 
					                          & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
					     (((hGfxDeviceContext->S0LinePitch >> 2) //32Bit Word Address
					                          & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->S0BaseAddr >> 2);
			}
			#else
			{
				GfxCMD = ((CMD_S0_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					     ((hGfxDeviceContext->S0SkipPixelLine 
					                          & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
					     ((hGfxDeviceContext->S0LinePitch 
					                          & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->S0BaseAddr);
			}
			#endif
			#if (ORION_VERSION != ORION_130)
			if (hGfxDeviceContext->IsUsingSDSpecificEndian)
			{				
				S0EndianCtrl = ((hGfxDeviceContext->S0ByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
					           ((hGfxDeviceContext->S0NibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
					           ((hGfxDeviceContext->S016BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
					           ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
			}
			GfxCMD = ((CMD_SCALOR_S_PIXEL_NUM_PER_LINE & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
			         ((hGfxDeviceContext->S0PixelNumOneLine
		                           & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)) << CMD_LPIXEL_NUM_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);

			GfxCMD = ((CMD_SCALOR_S_TOTAL_LINE_NUM & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->S0TotalLineNum
						           & ((1 << CMD_LINE_NUM_WIDTH) -1)) << CMD_LINE_NUM_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			#endif
		}
		//Color Info

		if (GfxDCType & (GFX_DC_S0_COLOR_FORMAT))
		{
			if (GfxDCType & GFX_DC_S0_DEFAULT_COLOR)
			{
				U8 Alpha = 0;
				if (hGfxDeviceContext->S0ColorFormat != GFX_CF_ARGB1555)
				{
					GfxCMD = ((CMD_S0_FORMAT_L   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S0ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
				}
				else
				{
					GfxCMD = ((CMD_S0_FORMAT_L   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S0ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
							 ((hGfxDeviceContext->S0Alpha0
							                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
							 ((hGfxDeviceContext->S0Alpha1
							                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
				}
			
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				#if (ORION_VERSION == ORION_130)
				{
					if (hGfxDeviceContext->S0ColorFormat == GFX_CF_A0 ||
						hGfxDeviceContext->S0ColorFormat == GFX_CF_RGB565)
					{
						double a = 0;
						a = sqrt((double)hGfxDeviceContext->S0DefaultColor.Alpha/255);
						Alpha = (U8)(a * 255);
						//Alpha = 0xFF;
						hGfxDeviceContext->S0DefaultColor.Alpha = Alpha;
					}
				}
				#else
					Alpha = hGfxDeviceContext->S0DefaultColor.Alpha;
				#endif
				GfxCMD = (((Alpha >> (8-8))
					                     & ((1 << CMD_ALPHA_WIDTH) -1)) << CMD_ALPHA_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Red >> (8-8))
					                     & ((1 << CMD_RED_WIDTH) -1)) << CMD_RED_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Green >> (8-8))
					                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Blue >> (8-8))
					                     & ((1 << CMD_BLUE_WIDTH) -1)) << CMD_BLUE_LSB);
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			}
			else
			{
				if (hGfxDeviceContext->S0ColorFormat != GFX_CF_ARGB1555)
				{
					GfxCMD = ((CMD_S0_FORMAT     & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S0ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
				}
				else
				{
					GfxCMD = ((CMD_S0_FORMAT     & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S0ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
							 ((hGfxDeviceContext->S0Alpha0
							                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
							 ((hGfxDeviceContext->S0Alpha1
							                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
				}
			
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);				
			}
		}
		//ColorKey ARGB8888
		if (hGfxDeviceContext->S0ColorKeyEnable && (GfxDCType & GFX_DC_S0_COLORKEY_PARA))
		{
			GfxCMD = ((CMD_S0_COLOR_KEY_MIN & ((1 << CMD_IDX_WIDTH) -1))   << CMD_IDX_LSB)    |
				     ((hGfxDeviceContext->S0ColorKeyMin.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				     ((hGfxDeviceContext->S0ColorKeyMin.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				     ((hGfxDeviceContext->S0ColorKeyMin.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			GfxCMD = ((CMD_S0_COLOR_KEY_MAX & ((1 << CMD_IDX_WIDTH) -1))   << CMD_IDX_LSB)    |
				     ((hGfxDeviceContext->S0ColorKeyMax.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				     ((hGfxDeviceContext->S0ColorKeyMax.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				     ((hGfxDeviceContext->S0ColorKeyMax.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);		
		}		
	}
	//Send S1 Related Command
	if (hGfxDeviceContext->S1Enable)
	{
		if (hGfxDeviceContext->S1FetchDram && (GfxDCType & GFX_DC_S1_DRAM_PARA) )
		{
			
			//Dram Access Para
			#if (ORION_VERSION == ORION_130)
			{
				assert((hGfxDeviceContext->S1BaseAddr  & 0x3) == 0);
				assert((hGfxDeviceContext->S1LinePitch & 0x3) == 0);
				GfxCMD = ((CMD_S1_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					     ((hGfxDeviceContext->S1SkipPixelLine 
					                          & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
					     (((hGfxDeviceContext->S1LinePitch >> 2)
					                          & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->S1BaseAddr >> 2);
			}
			#else
			{
				GfxCMD = ((CMD_S1_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					     ((hGfxDeviceContext->S1SkipPixelLine 
					                          & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
					     ((hGfxDeviceContext->S1LinePitch 
					                          & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->S1BaseAddr);
			}
			#endif
			#if (ORION_VERSION != ORION_130)
			if (hGfxDeviceContext->IsUsingSDSpecificEndian)
			{				
				S1EndianCtrl = ((hGfxDeviceContext->S1ByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
					           ((hGfxDeviceContext->S1NibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
					           ((hGfxDeviceContext->S116BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
					           ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
			}
			GfxCMD = ((CMD_PIXEL_NUM_PER_LINE & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
			         ((hGfxDeviceContext->S1PixelNumOneLine
		                           & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)) << CMD_LPIXEL_NUM_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);

			GfxCMD = ((CMD_TOTAL_LINE_NUM & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->S1TotalLineNum
						           & ((1 << CMD_LINE_NUM_WIDTH) -1)) << CMD_LINE_NUM_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			#endif
			
		}
		//Color Info

		if (GfxDCType & GFX_DC_S1_COLOR_FORMAT)
		{
			if (GfxDCType & GFX_DC_S1_DEFAULT_COLOR)
			{
				U8 Alpha;
				if (hGfxDeviceContext->S1ColorFormat != GFX_CF_ARGB1555)
				{
					GfxCMD = ((CMD_S1_FORMAT_L   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S1ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
				}
				else
				{
					GfxCMD = ((CMD_S1_FORMAT_L   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S1ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
							 ((hGfxDeviceContext->S1Alpha0
							                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
							 ((hGfxDeviceContext->S1Alpha1
							                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
				}
				#if (ORION_VERSION == ORION_130)
				{
					if (hGfxDeviceContext->S1ColorFormat == GFX_CF_A0 ||
						hGfxDeviceContext->S1ColorFormat == GFX_CF_RGB565)
					{
						double a = 0;
						a = sqrt((double)hGfxDeviceContext->S1DefaultColor.Alpha/255);
						Alpha = (U8)(a * 255);
						//For Test Only
						//Alpha = 0xFF;
					}
					hGfxDeviceContext->S1DefaultColor.Alpha = Alpha;
				}
				#else
					Alpha = hGfxDeviceContext->S1DefaultColor.Alpha;
				#endif

				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
				GfxCMD = (((Alpha >> (8-8))  & ((1 << CMD_ALPHA_WIDTH) -1)) << CMD_ALPHA_LSB)|
						 (((hGfxDeviceContext->S1DefaultColor.Red >> (8-8))
						                     & ((1 << CMD_RED_WIDTH) -1)) << CMD_RED_LSB)|
						 (((hGfxDeviceContext->S1DefaultColor.Green >> (8-8))
						                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
						 (((hGfxDeviceContext->S1DefaultColor.Blue >> (8-8))
						                     & ((1 << CMD_BLUE_WIDTH) -1)) << CMD_BLUE_LSB);
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);	
			}
			else
			{
				if (hGfxDeviceContext->S1ColorFormat != GFX_CF_ARGB1555)
				{
					GfxCMD = ((CMD_S1_FORMAT     & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S1ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
				}
				else
				{
					GfxCMD = ((CMD_S1_FORMAT     & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
							 ((hGfxDeviceContext->S1ColorFormat
							                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
							 ((hGfxDeviceContext->S1Alpha0
							                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
							 ((hGfxDeviceContext->S1Alpha1
							                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
				}
			
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);				
			}
		}
		
		//ColorKey
		if (hGfxDeviceContext->S1ColorKeyEnable && (GfxDCType & GFX_DC_S1_COLORKEY_PARA))
		{
			GfxCMD = ((CMD_S1_COLOR_KEY_MIN & ((1 << CMD_IDX_WIDTH) -1))   << CMD_IDX_LSB)    |
				     ((hGfxDeviceContext->S1ColorKeyMin.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				     ((hGfxDeviceContext->S1ColorKeyMin.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				     ((hGfxDeviceContext->S1ColorKeyMin.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			GfxCMD = ((CMD_S1_COLOR_KEY_MAX & ((1 << CMD_IDX_WIDTH) -1))   << CMD_IDX_LSB)    |
				     ((hGfxDeviceContext->S1ColorKeyMax.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				     ((hGfxDeviceContext->S1ColorKeyMax.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				     ((hGfxDeviceContext->S1ColorKeyMax.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);		
		}		
	}

	//Clut Table Update
			//Clut4
	if (hGfxDeviceContext->UpdateClut4Table && (GfxDCType & GFX_DC_CLUT4_TABLE))
	{
		if (hGfxDeviceContext->UpdateClut4TableMode)//Update According Idx
		{
			U32 Clut4TableIdx = 0;
			Clut4TableIdx = (hGfxDeviceContext->Clut4TableIdx & ((1 << CMD_CLUT4_IDX_WIDTH) -1));
			assert(Clut4TableIdx < CLUT4_TABLE_LEN);
			GfxCMD =((CMD_CLUT4_IDX & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) | 
				    ( Clut4TableIdx << CMD_CLUT4_IDX_LSB);
				
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			//Send Clut4 Entry
			GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->Clut4Table[Clut4TableIdx]);
		}
		else
		{
			U32 i = 0;
			GfxCMD =((CMD_CLUT4_TAB & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			for (i = 0; i < CLUT4_TABLE_LEN; i++)
			{
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->Clut4Table[i]);
			}
		}
	#ifdef CHECK_REG_AFTER_SEND_CMD
		if (!hGfxAPIRegCheckClutTable(hGfxDeviceContext, GfxDCType, 1))
		{
			ERROR("##Failed to Check Clut4 Table Register\n");
			return 0;
		}
	#endif
	}
		//Clut8
	if (hGfxDeviceContext->UpdateClut8Table && (GfxDCType & GFX_DC_CLUT8_TABLE))
	{
		if (hGfxDeviceContext->UpdateClut8TableMode)//Update According Idx
		{
			U32 Clut8TableIdx = 0;
			Clut8TableIdx = (hGfxDeviceContext->Clut8TableIdx & ((1 << CMD_CLUT8_IDX_WIDTH) -1));
			assert(Clut8TableIdx < CLUT8_TABLE_LEN);
			GfxCMD =((CMD_CLUT8_IDX & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) | 
				    ( Clut8TableIdx << CMD_CLUT8_IDX_LSB);
			
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			//Send Clut8 Entry
			GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->Clut8Table[Clut8TableIdx]);
		}
		else
		{
			U32 i = 0;
			GfxCMD =((CMD_CLUT8_TAB & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			for (i = 0; i < CLUT8_TABLE_LEN; i++)
			{
				GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->Clut8Table[i]);
			}
		}
	#ifdef CHECK_REG_AFTER_SEND_CMD
		if (!hGfxAPIRegCheckClutTable(hGfxDeviceContext, GfxDCType, 0))
		{
			ERROR("##Failed to Check Clut8 Table Register\n");
			return 0;
		}
	#endif
	}
	//Send D Related Command
	
	//D Dram Para
	if ((GfxDCType & GFX_DC_D_DRAM_PARA))
	{
		#if (ORION_VERSION == ORION_130)
		{
			assert((hGfxDeviceContext->DBaseAddr  & 0x3) == 0);
			assert((hGfxDeviceContext->DLinePitch & 0x3) == 0);
			GfxCMD = ((CMD_D_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->DSkipPixelLine 
				                         & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
				     (((hGfxDeviceContext->DLinePitch >> 2)
						                 & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->DBaseAddr >> 2);		
		}
		#else
		{
			GfxCMD = ((CMD_D_ADDR_INFO_L & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->DSkipPixelLine 
				                         & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)) << CMD_SKIP_PIXEL_LSB) |
				     ((hGfxDeviceContext->DLinePitch 
						                 & ((1 << CMD_PITCH_WIDTH) -1)) << CMD_PITCH_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			GfxCMDQueSpace = hGfxAPISendCMD(hGfxDeviceContext->DBaseAddr);
		}
		#endif
		if (hGfxDeviceContext->IsUsingSDSpecificEndian)
		{				
			DEndianCtrl = ((hGfxDeviceContext->DByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
				          ((hGfxDeviceContext->DNibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
				          ((hGfxDeviceContext->D16BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
				          ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
		}
		//Set Img Width and Height
		#if (ORION_VERSION == ORION_130)
		{
			GfxCMD = ((CMD_PIXEL_NUM_PER_LINE & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->DPixelNumOneLine
			                           & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)) << CMD_LPIXEL_NUM_LSB) |
			         ((hGfxDeviceContext->DTotalLineNum
						               & ((1 << CMD_LINE_NUM_WIDTH) -1)) << CMD_LINE_NUM_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
		}
		#else
		{
			GfxCMD = ((CMD_PIXEL_NUM_PER_LINE & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->DPixelNumOneLine
			                           & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)) << CMD_LPIXEL_NUM_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);

			GfxCMD = ((CMD_TOTAL_LINE_NUM & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				     ((hGfxDeviceContext->DTotalLineNum
						                          & ((1 << CMD_LINE_NUM_WIDTH) -1)) << CMD_LINE_NUM_LSB) ;
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
		}
		#endif

	}
	//D Color Info should not be A0, no Default Color
	if ((GfxDCType & GFX_DC_D_COLOR_FORMAT))
	{
		if (hGfxDeviceContext->DColorFormat != GFX_CF_ARGB1555)
		{
			GfxCMD = ((CMD_D_FORMAT   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					 ((hGfxDeviceContext->DColorFormat
					                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
		}
		else
		{
			GfxCMD = ((CMD_D_FORMAT   & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
					 ((hGfxDeviceContext->DColorFormat
					                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
					 ((hGfxDeviceContext->DAlpha0Min
					                     & ((1 << CMD_FORMAT_ALPHA0_MIN_WIDTH) -1)) << CMD_FORMAT_ALPHA0_MIN_LSB)|
					 ((hGfxDeviceContext->DAlpha0Max
					                     & ((1 << CMD_FORMAT_ALPHA0_MAX_WIDTH) -1)) << CMD_FORMAT_ALPHA0_MAX_LSB);
		}	
		GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
	}
	//Set Gfx Interrupt Mode and Pipeline Enable
	//Send Scalor Command
	//Update VFIR Coeff
	#if (ORION_VERSION != ORION_130)
	if ( (hGfxDeviceContext->UpdateHFIRCoeff) && (GfxDCType & GFX_DC_SCALOR_HFIR_COFF))
	{
		U32 Phase = 0;
		U32 Tap   = 0;
		for (Phase = 0; Phase < SCALOR_PAHSE_NUM; Phase++)
		{
			for (Tap =0; Tap < SCALOR_H_FIR_TAP_NUM; Tap++)
			{
				GfxCMD = ((CMD_HFIR_COEFFICIETN_DATA & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				 		 ((Phase & ((1 << CMD_FIR_COEFF_PHASE_IDX_WIDTH) -1)) << CMD_FIR_COEFF_PHASE_IDX_LSB)|
				 		 ((Tap   & ((1 << CMD_FIR_COEFF_TAP_IDX_WIDTH) -1))   << CMD_FIR_COEFF_TAP_IDX_LSB)|
				 		 ((hGfxDeviceContext->HFIRCoeffTable[Phase][Tap]
				 		         & ((1 << CMD_FIR_COEFF_VALUE_WIDTH) -1))     << CMD_FIR_COEFF_VALUE_LSB);
				GfxCMDQueSpace = hGfxAPISendCMD (GfxCMD);
			}
		}
	}
	//Update HFIR Coeff
	if ((hGfxDeviceContext->UpdateVFIRCoeff) && (GfxDCType & GFX_DC_SCALOR_VFIR_COFF))
	{
		U32 Phase = 0;
		U32 Tap   = 0;
		for (Phase = 0; Phase < SCALOR_PAHSE_NUM; Phase++)
		{
			for (Tap =0; Tap < SCALOR_V_FIR_TAP_NUM; Tap++)
			{
				GfxCMD = ((CMD_VFIR_COEFFICIETN_DATA & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				 		 ((Phase & ((1 << CMD_FIR_COEFF_PHASE_IDX_WIDTH) -1)) << CMD_FIR_COEFF_PHASE_IDX_LSB)|
				 		 ((Tap   & ((1 << CMD_FIR_COEFF_TAP_IDX_WIDTH) -1))   << CMD_FIR_COEFF_TAP_IDX_LSB)|
				 		 ((hGfxDeviceContext->VFIRCoeffTable[Phase][Tap]
				 		         & ((1 << CMD_FIR_COEFF_VALUE_WIDTH) -1))     << CMD_FIR_COEFF_VALUE_LSB);
				GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
			}
		}
	}
	if (hGfxDeviceContext->S0ScalorEnable)
	{
		//Scalor Source Pixel Map
		//Send InitialPhase
		if ((GfxDCType & GFX_DC_SCALOR_INIT_PHASE))
		{
			GfxCMD = ((CMD_SCALOR_INITIAL_PHASE & ((1 << CMD_IDX_WIDTH) -1)) << CMD_IDX_LSB) |
				 		 ((hGfxDeviceContext->HInitialPhase 
				 		         & ((1 << CMD_SCALOR_H_INIT_PHASE_WIDTH) -1)) << CMD_SCALOR_H_INIT_PHASE_LSB)|
				 		 ((hGfxDeviceContext->VInitialPhase    
				 		         & ((1 << CMD_SCALOR_V_INIT_PHASE_WIDTH) -1)) << CMD_SCALOR_V_INIT_PHASE_LSB);
			GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
		}
	}
	#endif
	
	#ifdef CHECK_REG_AFTER_SEND_CMD
	if (!hGfxAPIRegCheckPreStartup(hGfxDeviceContext, GfxDCType))
	{
		ERROR("##Failed to Check Register Before Startup\n");
		return 0;
	}
	#endif

	//Send Startup Command
	if ((GfxDCType & GFX_DC_CTRL))
	{
		
		//Send Endian Mode		
		GlobalEndianCtrl = ((hGfxDeviceContext->DByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
			               ((hGfxDeviceContext->DNibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
			               ((hGfxDeviceContext->D16BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT);
		
		GfxCMD     = ((CMD_ENDIAN_CTRL  & ((1 << CMD_IDX_WIDTH)           -1)) << CMD_IDX_LSB)|
		             ((S0EndianCtrl     & ((1 << CMD_S0_ENDIAN_WIDTH)     -1)) << CMD_S0_ENDIAN_LSB)|
		             ((S1EndianCtrl     & ((1 << CMD_S1_ENDIAN_WIDTH)     -1)) << CMD_S1_ENDIAN_LSB)|
		             ((DEndianCtrl      & ((1 << CMD_D_ENDIAN_WIDTH)      -1)) << CMD_D_ENDIAN_LSB )|
		             ((GlobalEndianCtrl & ((1 << CMD_GLOBAL_ENDIAN_WIDTH) -1)) << CMD_GLOBAL_ENDIAN_LSB); 
		GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);	
		//Send Ctrl and Startup CMD
		ScanCtrl   =   ((hGfxDeviceContext->S0VReverseScan   & (1)) << S0_REVERSE_SCAN_BIT)  |
			           ((hGfxDeviceContext->S1VReverseScan   & (1)) << S1_REVERSE_SCAN_BIT)  |
			           ((hGfxDeviceContext->DVReverseScan    & (1)) << D_REVERSE_SCAN_BIT);
		CompositorOperation = 
			           ((hGfxDeviceContext->CompositorEnable & (1)) << COMPOSITOR_ENABLE_BIT)|
		               ((hGfxDeviceContext->S0OnTopS1        & (1)) << S0_ON_TOP_S1_BIT);
		S1Operation =  ((hGfxDeviceContext->S1Enable         & (1)) << S_ENABLE_BIT)|
		               ((hGfxDeviceContext->S1ColorKeyEnable & (1)) << S_COLORKEY_ENABLE_BIT)|
		               ((hGfxDeviceContext->S1ClutEnable     & (1)) << S_CLUT_ENABLE_BIT)    |
		               ((hGfxDeviceContext->S1FetchDram      & (1)) << S_FETCH_DATA_BIT);
		S0Operation =  ((hGfxDeviceContext->S0Enable         & (1)) << S_ENABLE_BIT)|
		               ((hGfxDeviceContext->S0ColorKeyEnable & (1)) << S_COLORKEY_ENABLE_BIT)|
		               ((hGfxDeviceContext->S0ClutEnable     & (1)) << S_CLUT_ENABLE_BIT)    |
		               ((hGfxDeviceContext->S0FetchDram      & (1)) << S_FETCH_DATA_BIT);

		GfxCMD = ((CMD_STARTUP  & ((1 << CMD_IDX_WIDTH)    -1)) << CMD_IDX_LSB)    |
				 ((ScanCtrl     & ((1 << SCAN_CTRL_WIDTH)  -1)) << SCAN_CTRL_LSB)  |
				 ((hGfxDeviceContext->S0ScalorEnable & (0x1))   << S0_SCAL_ENA_BIT)|
			     ((hGfxDeviceContext->InterruptEnable
				                & ((1 << INT_ENABLE_WIDTH) -1)) << INT_ENABLE_LSB) |
				 ((hGfxDeviceContext->ROPAlphaCtrl
				                & ((1 << ROP_ALPHA_CTRL_WIDTH)    -1)) << ROP_ALPHA_CTRL_LSB) |
				 ((hGfxDeviceContext->RopValue
				                & ((1 << ROP_VAL_WIDTH)    -1)) << ROP_VAL_LSB)    |
				 ((CompositorOperation
				                & ((1 << CMP_OPT_WIDTH)    -1)) << CMP_OPT_LSB)    |
				 ((S1Operation  & ((1 << S1_OPT_WIDTH)     -1)) << S1_OPT_LSB)     |
				 ((S0Operation  & ((1 << S0_OPT_WIDTH)     -1)) << S0_OPT_LSB);
		//Check Point
		GfxCMDQueSpace = hGfxAPISendCMD(GfxCMD);
		hGfxAPICMDGroupCntInc();
		#ifdef ORION_GFX_WAIT_IDEL_AFTER_SEND_CMD
		if (!hGfxAPIWaitGfxToBeIdle(hGfxDeviceContext)) return;
		#endif
	}
	hGfxDeviceContext->GfxDCType = GfxDCType;
	return 1;
}
I32 hGfxAPIWaitGfxToBeIdle(GFX_DEVICE_CONTEXT *hGfxDeviceContext) //Boolean Type Return
{
	//Run Time Info
	I32 hResult = 1;
	I32 GfxIdelStatus   = 0;
	I32 AHBErrStatus    = 0;
	I32 GfxErrStatus    = 0;
	I32 InterruptStatus = 0; 
	U32 GfxCMDQueuSpace = 0;

	I32 InterruptEnable = 0;

	U32 GfxStatusRegValue = 0;
	U32 GfxInfoRegValue   = 0;
	U32 TimeCnt = 0;
	U32 TimeOut = 0;
	if (hGfxDeviceContext == NULL)
	{
		TimeOut = 0; //No Time Out
	}
	else
	{
		U32 MaxWidth  = 0;
		U32 MaxHeight = 0;
		MaxWidth  = hGfxDeviceContext->S0PixelNumOneLine > hGfxDeviceContext->S1PixelNumOneLine ?
			        hGfxDeviceContext->S0PixelNumOneLine : hGfxDeviceContext->S1PixelNumOneLine ;
		MaxWidth  = MaxWidth > hGfxDeviceContext->DPixelNumOneLine ?
			        MaxWidth : hGfxDeviceContext->DPixelNumOneLine ;
		MaxHeight = hGfxDeviceContext->S0TotalLineNum > hGfxDeviceContext->S1TotalLineNum ?
			        hGfxDeviceContext->S0TotalLineNum : hGfxDeviceContext->S1TotalLineNum ;
		MaxHeight = MaxHeight > hGfxDeviceContext->DTotalLineNum ?
			        MaxHeight : hGfxDeviceContext->DTotalLineNum ;
		TimeOut = MaxWidth * MaxHeight *10; //One Circle One Pixel, for Scalor 3 Circle/pixel, so *10 is Enough
		if (TimeOut < 100) TimeOut = 100;
	}
	do 
	{
		GfxInfoRegValue   = GfxRegRead(GFX_REG_CMD_INFO);
		InterruptEnable   = (GfxInfoRegValue >> INT_ENABLE_LSB) &((1 << INT_ENABLE_WIDTH) -1); 
		//Check Gfx Status
		GfxStatusRegValue = GfxRegRead(GFX_REG_STATUS);
		GfxIdelStatus     = (GfxStatusRegValue >> GFX_STATUS_BIT ) &(1);
		AHBErrStatus      = (GfxStatusRegValue >> GFX_AHB_ERR_BIT) &(1);
		GfxErrStatus      = (GfxStatusRegValue >> GFX_ERR_BIT    ) &(1);
		InterruptStatus   = (GfxStatusRegValue >> GFX_INT_STATUS_BIT) &(1);
		GfxCMDQueuSpace   = (GfxRegRead(GFX_REG_CMDQUE_EMPTY_CNT) 
			                                   >> GFX_REG_CMDQUE_EMPTY_CNT_LSB) 
			                                   & ((1 << GFX_REG_CMDQUE_EMPTY_CNT_WIDTH) - 1);
		if (AHBErrStatus) 
		{
			ERROR("##ERROR: Gfx AHBErrStatus Err, Reset Gfx\n");
			assert(!AHBErrStatus);
			hGfxAPISWReset();
			continue;
		}
		//assert(!GfxErrStatus);
		if (GfxErrStatus)
		{
			ERROR("##ERROR: Gfx Command Interpret Err\n");
			hResult = 0;
		}
		if (InterruptEnable || GfxErrStatus) //if Err Must Be a Interrupt 
		{
			if (InterruptStatus == 1)
			{
				//Clear Interrupt
				I32 IsGfxGenINTToHost = 0;
				IsGfxGenINTToHost = IsGfxGenInterrupt();
				assert(IsGfxGenINTToHost);//Hardware Should Generate a Interrupt
				GfxRegWrite(GFX_REG_INT_CLEAR, 1);//Write Any Value
			}
		}
		TimeCnt++;
		//Some User Definde Sleep Function	
	}while(!GfxIdelStatus && (TimeCnt < TimeOut || TimeOut == 0));
	if(hGfxDeviceContext != NULL)
	{
		hGfxDeviceContext->GfxIdelStatus   = GfxIdelStatus;
		hGfxDeviceContext->AHBErrStatus    = AHBErrStatus;
		hGfxDeviceContext->GfxErrStatus    = GfxErrStatus;
		hGfxDeviceContext->InterruptStatus = InterruptStatus; 
		hGfxDeviceContext->CMDCnt          = GfxCMDQueuSpace;
	}
	if (TimeCnt >= TimeOut && TimeOut > 0)
	{
		int i = 0;
		ERROR_D1("##ERROR: TimeOut (TimeCnt = %d)\n", TimeCnt);
		PRINT_L1("Try To Reset Gfx Engine");
		GfxRegWrite(GFX_REG_SW_RESET, 1);
		for (i =0; i < 10; i++)
		{
			GfxStatusRegValue = GfxRegRead(GFX_REG_STATUS);
			GfxIdelStatus     = (GfxStatusRegValue >> GFX_STATUS_BIT ) &(1);		
			if (GfxIdelStatus)
			{
				break;
			}
		}		
		AHBErrStatus      = (GfxStatusRegValue >> GFX_AHB_ERR_BIT) &(1);
		GfxErrStatus      = (GfxStatusRegValue >> GFX_ERR_BIT    ) &(1);
		InterruptStatus   = (GfxStatusRegValue >> GFX_INT_STATUS_BIT) &(1);
		if (!GfxIdelStatus || AHBErrStatus || GfxErrStatus || InterruptStatus)
		{
			ERROR("##ERROR: Failed to Reset Gfx AfterTimeOut\n");
		}
		GfxRegWrite(GFX_REG_SW_RESET, 0);
		hGfxAPICMDGroupCntSyncHW();
		hResult = 0;
	}
	else
	{
		#ifdef TRACE_GFX_RUN_TIME_INFO
		PRINT_D1_L2("Gfx Idel Waitting Time = %d\n", TimeCnt);
		#endif
	}
	if(AHBErrStatus)
	{
		ERROR("##ERROR: Gfx AHBErr\n");
		hResult = 0;
	}
	if (GfxErrStatus)
	{
		ERROR("##ERROR: Gfx Command Interpret Err\n");
		hResult = 0;
	}
	//#ifdef CHECK_REG_AFTER_SEND_CMD	
	#if 0
	if(hResult && hGfxDeviceContext != NULL)
	{
		if (!hGfxAPIRegCheckPosStartup(hGfxDeviceContext, hGfxDeviceContext->GfxDCType))
		{
			ERROR("##Failed to Check Register After Startup\n");
			return 0;
		}
	}
	#endif
	return hResult;
}

static U32 sGfxCMDQueueDepth = 0x100; // 0x100 is a HW Reset Value means not initial
static void hGfxAPISetCMDQueueDepth(U32 GfxCMDQueueDepth)
{
	sGfxCMDQueueDepth = GfxCMDQueueDepth;
}
U32 hGfxAPIGetCMDQueueDepth()
{
	return sGfxCMDQueueDepth ;
}
void hGfxAPISWReset()
{
	U32 GfxCMDQueueDepth = 0;
#ifdef RESET_GFX_ONLY_ONE_TIME
	if(IsGfxSWReset && IsGfxHWReset)
	{
		return;
	}
	else
	{
		IsGfxSWReset++;
	}	
#endif
#if (ORION_VERSION != ORION_130)
	GfxRegWrite(GFX_REG_SW_RESET, 1);
	//Waitting Gfx to be Idel
	hGfxAPIWaitGfxToBeIdle(NULL); // To Ensure Reset is Stable, must Wait for the Idel
	GfxRegWrite(GFX_REG_SW_RESET, 0);
	GfxCMDQueueDepth = (GfxRegRead(GFX_REG_CMDQUE_EMPTY_CNT) 
			              >> GFX_REG_CMDQUE_EMPTY_CNT_LSB) 
			              & ((1 << GFX_REG_CMDQUE_EMPTY_CNT_WIDTH) - 1);
	assert(GfxCMDQueueDepth != 0);
	hGfxAPISetCMDQueueDepth(GfxCMDQueueDepth);
	hGfxAPICMDGroupCntSyncHW();
#endif
}

I32 hGfxAPIWaitCMDQueueEmpty()
{
	U32 GfxCMDQueueDepth = 0;
	U32 GfxCMDQueuSpace  = 0;
	U32 GfxStatusRegValue = 0;
	I32 AHBErrStatus    = 0;
	I32 GfxErrStatus    = 0;
	I32 InterruptStatus = 0; 
	I32 InterruptEnable = 0;
	//Clear Interrupt and ERR
	GfxCMDQueueDepth = hGfxAPIGetCMDQueueDepth();
	assert(GfxCMDQueueDepth != 0);
	if (GfxCMDQueueDepth == 0)
	{
		return 0;
	}
	do
	{
	
		GfxCMDQueuSpace = (GfxRegRead(GFX_REG_CMDQUE_EMPTY_CNT) 
			              >> GFX_REG_CMDQUE_EMPTY_CNT_LSB) 
			              & ((1 << GFX_REG_CMDQUE_EMPTY_CNT_WIDTH) - 1);
		if (GfxCMDQueuSpace < GfxCMDQueueDepth)
		{
			U32 GfxStatusRegValue = 0;
			I32 AHBErrStatus    = 0;
			I32 GfxErrStatus    = 0;
			I32 InterruptStatus = 0; 
			I32 InterruptEnable = 0;
			//Some User Defined Sleep Function
			ERROR_L2("Gfx Command Queue Full, Waiting.....\n");
			//Clear Interrupt and ERR
			GfxStatusRegValue = GfxRegRead(GFX_REG_STATUS);
			AHBErrStatus      = (GfxStatusRegValue >> GFX_AHB_ERR_BIT) &(1);
			GfxErrStatus      = (GfxStatusRegValue >> GFX_ERR_BIT    ) &(1);
			InterruptStatus   = (GfxStatusRegValue >> GFX_INT_STATUS_BIT) &(1);
			if (AHBErrStatus) 
			{
				ERROR("##ERROR: Gfx AHBErrStatus Err, Reset Gfx\n");
				assert(!AHBErrStatus);
				hGfxAPISWReset();
				continue;
			}
			//assert(!GfxErrStatus);
			if (GfxErrStatus)
			{
				ERROR("##ERROR: Gfx Command Interpret Err\n");
			}
			if (InterruptEnable || GfxErrStatus) //if Err Must Be a Interrupt 
			{
				if (InterruptStatus == 1)
				{
					//Clear Interrupt
					I32 IsGfxGenINTToHost = 0;
					IsGfxGenINTToHost = IsGfxGenInterrupt();
					assert(IsGfxGenINTToHost);//Hardware Should Generate a Interrupt
					GfxRegWrite(GFX_REG_INT_CLEAR, 1);//Write Any Value
				}
			}
		}
	}while(GfxCMDQueuSpace < GfxCMDQueueDepth );
	assert(GfxCMDQueuSpace == GfxCMDQueueDepth );
	if (GfxCMDQueuSpace > GfxCMDQueueDepth )
	{
		return 0;
	}
	return (GfxCMDQueuSpace == GfxCMDQueueDepth );	
}
static I32 GfxRegCheck ( U32 RegAddr, U32 ExpectedValue)  
{ 
	U32 GfxRegValue = 0;
	GfxRegValue = GfxRegRead((RegAddr));
	if (GfxRegValue != ExpectedValue)
	{
		PRINT_D3_L1("RegAddr(%08x),GfxRegValue(%08x)!= ExpectedValue(%08x)\n",
			         RegAddr,GfxRegValue, ExpectedValue);
	}
	assert(GfxRegValue == (ExpectedValue));
	return(GfxRegValue == (ExpectedValue));
}
I32 hGfxAPIRegCheckClutTable(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType, I32 IsClut4orClut8) //0: Clut4, 1: Clut8
{
	if (hGfxDeviceContext == NULL) return 0;
	if (!hGfxAPIWaitCMDQueueEmpty()) return 0;
	//Clut Table Update
	//Clut4
	if (hGfxDeviceContext->UpdateClut4Table && (GfxDCType & GFX_DC_CLUT4_TABLE) && (IsClut4orClut8 == 1))
	{
		U32 Clut4TableIdx  = 0;
		U32 Clut4TableData = 0;
		if (hGfxDeviceContext->UpdateClut4TableMode)//Update According Idx
		{
			Clut4TableIdx = (hGfxDeviceContext->Clut4TableIdx & ((1 << CMD_CLUT4_IDX_WIDTH) -1));
			assert(Clut4TableIdx < CLUT4_TABLE_LEN);
			GfxRegWrite(GFX_REG_CLUT_IDX, Clut4TableIdx);
			//Change to ARGB8565 Format
			Clut4TableData = (hGfxDeviceContext->Clut4Table[Clut4TableIdx]) & 
			                 ((0xFF000000)|(0x1F0000 << 3)|(0x3F00 << 2)|(0x1F<<3));
			if (!GfxRegCheck((GFX_REG_CLUT_ENTRY), (Clut4TableData))) return 0;			
		}
		else
		{
			for (Clut4TableIdx = 0; Clut4TableIdx < CLUT4_TABLE_LEN; Clut4TableIdx++)
			{
				GfxRegWrite(GFX_REG_CLUT_IDX, Clut4TableIdx);
				//Change to ARGB8565 Format
				Clut4TableData = (hGfxDeviceContext->Clut4Table[Clut4TableIdx]) & 
				                 ((0xFF000000)|(0x1F0000 << 3)|(0x3F00 << 2)|(0x1F<<3));
				if (!GfxRegCheck((GFX_REG_CLUT_ENTRY), (Clut4TableData))) return 0;			
			}
		}
	}
		//Clut8
	if (hGfxDeviceContext->UpdateClut8Table && (GfxDCType & GFX_DC_CLUT8_TABLE) && (IsClut4orClut8 == 0))
	{
		U32 Clut8TableIdx = 0;
		U32 Clut8TableData = 0;
		if (hGfxDeviceContext->UpdateClut8TableMode)//Update According Idx
		{
			Clut8TableIdx = (hGfxDeviceContext->Clut8TableIdx & ((1 << CMD_CLUT8_IDX_WIDTH) -1));
			assert(Clut8TableIdx < CLUT8_TABLE_LEN);
			GfxRegWrite(GFX_REG_CLUT_IDX, Clut8TableIdx);
			//Change to ARGB8565 Format
			Clut8TableData = (hGfxDeviceContext->Clut8Table[Clut8TableIdx]) & 
			                 ((0xFF000000)|(0x1F0000 << 3)|(0x3F00 << 2)|(0x1F<<3));
			if (!GfxRegCheck((GFX_REG_CLUT_ENTRY), (Clut8TableData))) return 0;			
		}
		else
		{
			for (Clut8TableIdx = 0; Clut8TableIdx < CLUT8_TABLE_LEN; Clut8TableIdx++)
			{
				GfxRegWrite(GFX_REG_CLUT_IDX, Clut8TableIdx);
				//Change to ARGB8565 Format
				Clut8TableData = (hGfxDeviceContext->Clut8Table[Clut8TableIdx]) & 
				                 ((0xFF000000)|(0x1F0000 << 3)|(0x3F00 << 2)|(0x1F<<3));
				if (!GfxRegCheck((GFX_REG_CLUT_ENTRY), (Clut8TableData))) return 0;			
			}
		}
	}	
	return 1;
}


I32 hGfxAPIRegCheckPosStartup(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType)
{
	U32 S0EndianCtrl     = 0;
	U32 S1EndianCtrl     = 0;
	U32 DEndianCtrl      = 0;
	U32 GlobalEndianCtrl = 0;
	U32 GfxRegValue      = 0;
	if (hGfxDeviceContext == NULL) return 0;
	if (!hGfxAPIWaitCMDQueueEmpty()) return 0;
	
	if (hGfxDeviceContext->S0Enable || hGfxDeviceContext->S0ScalorEnable)
	{
		if ((hGfxDeviceContext->S0FetchDram || hGfxDeviceContext->S0ScalorEnable) && 
			(GfxDCType & GFX_DC_S0_DRAM_PARA) )
		{
			if (hGfxDeviceContext->IsUsingSDSpecificEndian)
			{
				S0EndianCtrl = ((hGfxDeviceContext->S0ByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
					           ((hGfxDeviceContext->S0NibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
					           ((hGfxDeviceContext->S016BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
					           ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
			}
		}
	}
	if (hGfxDeviceContext->S1Enable)
	{
		if (hGfxDeviceContext->S1FetchDram && (GfxDCType & GFX_DC_S1_DRAM_PARA) )
		{
			if (hGfxDeviceContext->IsUsingSDSpecificEndian)
			{
				S1EndianCtrl = ((hGfxDeviceContext->S1ByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
					           ((hGfxDeviceContext->S1NibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
					           ((hGfxDeviceContext->S116BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
					           ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
			}
		}
	}
	if ((GfxDCType & GFX_DC_D_DRAM_PARA))
	{
		if (hGfxDeviceContext->IsUsingSDSpecificEndian)
		{
			DEndianCtrl = ((hGfxDeviceContext->DByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
					      ((hGfxDeviceContext->DNibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
					      ((hGfxDeviceContext->D16BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT)|
					      ((1 & 0x1)    << ENDIAN_ENABLE_BIT);
		}	
	}
	//Scalor S0, S1 and D Dram Access Para Check
	if (!hGfxDeviceContext->S0ScalorEnable)
	{
		if (!GfxRegCheck((GFX_REG_S0_PIXEL_NUM_PER_LINE), GfxRegRead(GFX_REG_PIXEL_NUM_PER_LINE))) return 0;

		if (!GfxRegCheck((GFX_REG_S0_TOTAL_LINE_NUM)    , GfxRegRead(GFX_REG_TOTAL_LINE_NUM))) return 0;
	}
	//Ctrl Status Check
	if ((GfxDCType & GFX_DC_CTRL))
	{
		U32 CompositorOperation = 0;
		U32 S0Operation     = 0;
		U32 S1Operation     = 0;
		U32 ScanCtrl        = 0;
		U32 GfxCMDGroupCnt  = 0;
		//Send Endian Mode
		//Endian Check
		GlobalEndianCtrl = ((hGfxDeviceContext->DByteEndian   & 0x1)    << BYTE_ENDIAN_BIT  )|
			               ((hGfxDeviceContext->DNibbleEndian & 0x1)    << NIBBLE_ENDIAN_BIT)|
			               ((hGfxDeviceContext->D16BitEndian  & 0x1)    << TWO_BYTE_ENDIAN_BIT);
		S0EndianCtrl = ((S0EndianCtrl >> ENDIAN_ENABLE_BIT)& 0x1) ?
			            (S0EndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) : 
					    (GlobalEndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) ;
	  	S1EndianCtrl = ((S1EndianCtrl >> ENDIAN_ENABLE_BIT)& 0x1) ?
						(S1EndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) : 
						(GlobalEndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) ;
		DEndianCtrl  = ((DEndianCtrl >> ENDIAN_ENABLE_BIT)& 0x1) ?
						(DEndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) : 
						(GlobalEndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) ;
		if (hGfxDeviceContext->S0ScalorEnable)
		{
			S1EndianCtrl = S0EndianCtrl;
		}
		GfxRegValue = ((S0EndianCtrl & ((1 << REG_ENDIAN_CTRL_S0_WIDTH) - 1)) << (REG_ENDIAN_CTRL_S0_LSB)) |
					  ((S1EndianCtrl & ((1 << REG_ENDIAN_CTRL_S1_WIDTH) - 1)) << (REG_ENDIAN_CTRL_S1_LSB)) |
					  ((DEndianCtrl  & ((1 << REG_ENDIAN_CTRL_D_WIDTH ) - 1)) << (REG_ENDIAN_CTRL_D_LSB)) ;

		if (!GfxRegCheck((GFX_REG_ENDIAN_CTRL), (GfxRegValue))) return 0;
		
		//Send Ctrl and Startup CMD
		if (hGfxDeviceContext->S0ScalorEnable)
		{
			ScanCtrl   =   ((hGfxDeviceContext->S0VReverseScan   & (1)) << S0_REVERSE_SCAN_BIT)  |
				           ((hGfxDeviceContext->S0VReverseScan   & (1)) << S1_REVERSE_SCAN_BIT)  | //Scalor S0 S1 Scan the Same
				           ((hGfxDeviceContext->DVReverseScan    & (1)) << D_REVERSE_SCAN_BIT);
		}
		else
		{
			ScanCtrl   =   ((hGfxDeviceContext->S0VReverseScan   & (1)) << S0_REVERSE_SCAN_BIT)  |
				           ((hGfxDeviceContext->S1VReverseScan   & (1)) << S1_REVERSE_SCAN_BIT)  |
				           ((hGfxDeviceContext->DVReverseScan    & (1)) << D_REVERSE_SCAN_BIT);
		}
		if (hGfxDeviceContext->S0ScalorEnable)
		{
			CompositorOperation = 0;		
		}
		else
		{
			CompositorOperation = 
				           ((hGfxDeviceContext->CompositorEnable & (1)) << COMPOSITOR_ENABLE_BIT)|
			               ((hGfxDeviceContext->S0OnTopS1        & (1)) << S0_ON_TOP_S1_BIT);
		}
		if (hGfxDeviceContext->S0ScalorEnable)
		{
			S1Operation = 0;
		}
		else
		{
			S1Operation =  ((hGfxDeviceContext->S1Enable         & (1)) << S_ENABLE_BIT)|
		                   ((hGfxDeviceContext->S1ColorKeyEnable & (1)) << S_COLORKEY_ENABLE_BIT)|
		                   ((hGfxDeviceContext->S1ClutEnable     & (1)) << S_CLUT_ENABLE_BIT)    |
		                   ((hGfxDeviceContext->S1FetchDram      & (1)) << S_FETCH_DATA_BIT);
			
		}

		S0Operation =  ((hGfxDeviceContext->S0Enable         & (1)) << S_ENABLE_BIT)|
		               ((hGfxDeviceContext->S0ColorKeyEnable & (1)) << S_COLORKEY_ENABLE_BIT)|
		               ((hGfxDeviceContext->S0ClutEnable     & (1)) << S_CLUT_ENABLE_BIT)    |
		               ((hGfxDeviceContext->S0FetchDram      & (1)) << S_FETCH_DATA_BIT);
		
		GfxCMDGroupCnt = hGfxAPIGetCMDGroupCnt();
		GfxRegValue  = ((GfxCMDGroupCnt & ((1 << REG_INFO_CMD_CNT_WIDTH)- 1)) << REG_INFO_CMD_CNT_LSB)|
				       ((ScanCtrl     & ((1 << SCAN_CTRL_WIDTH)  -1)) << SCAN_CTRL_LSB)  |
				       ((hGfxDeviceContext->S0ScalorEnable & (0x1))   << S0_SCAL_ENA_BIT)|
			           ((hGfxDeviceContext->InterruptEnable
				                & ((1 << INT_ENABLE_WIDTH) -1)) << INT_ENABLE_LSB) |
				       ((hGfxDeviceContext->ROPAlphaCtrl
				                & ((1 << ROP_ALPHA_CTRL_WIDTH)    -1)) << ROP_ALPHA_CTRL_LSB) |
				       ((hGfxDeviceContext->RopValue
				                & ((1 << ROP_VAL_WIDTH)    -1)) << ROP_VAL_LSB)    |
				       ((CompositorOperation
				                & ((1 << CMP_OPT_WIDTH)    -1)) << CMP_OPT_LSB)    |
				       ((S1Operation  & ((1 << S1_OPT_WIDTH)     -1)) << S1_OPT_LSB)     |
				       ((S0Operation  & ((1 << S0_OPT_WIDTH)     -1)) << S0_OPT_LSB);
		//Check Point
		if (!GfxRegCheck((GFX_REG_CMD_INFO), (GfxRegValue))) return 0;
	}
	return 1;
	
}
I32 hGfxAPIRegCheckPreStartup(GFX_DEVICE_CONTEXT *hGfxDeviceContext, GFX_DC_UPDATE_TYPE GfxDCType) //Boolean Type Return
{
	U32 GfxRegValue      = 0;
	if (hGfxDeviceContext == NULL) return 0;
	if (!hGfxAPIWaitCMDQueueEmpty()) return 0;
		
	if (hGfxDeviceContext->S0Enable || hGfxDeviceContext->S0ScalorEnable)
	{
		if ((hGfxDeviceContext->S0FetchDram || hGfxDeviceContext->S0ScalorEnable) && 
			(GfxDCType & GFX_DC_S0_DRAM_PARA) )
		{	
			if (!GfxRegCheck((GFX_REG_S0_SKIP_PIXEL), 
				(hGfxDeviceContext->S0SkipPixelLine & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)))) return 0;
			#if (ORION_VERSION == ORION_130)
			{
				if (!GfxRegCheck((GFX_REG_S0_LINE_PITCH), 
					((hGfxDeviceContext->S0LinePitch >> 2) & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S0_ADDR)      , 
					(hGfxDeviceContext->S0BaseAddr >> 2))) return 0;			
			}
			#else
			{
				if (!GfxRegCheck((GFX_REG_S0_LINE_PITCH), 
					(hGfxDeviceContext->S0LinePitch & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S0_ADDR)      , 
					(hGfxDeviceContext->S0BaseAddr))) return 0;			
				if (!GfxRegCheck((GFX_REG_S0_PIXEL_NUM_PER_LINE), 
					(hGfxDeviceContext->S0PixelNumOneLine & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)))) return 0;

				if (!GfxRegCheck((GFX_REG_S0_TOTAL_LINE_NUM), 
					(hGfxDeviceContext->S0TotalLineNum & ((1 << CMD_LINE_NUM_WIDTH) -1)))) return 0;
			}
			#endif
			
		}
		//Color Info
		if (GfxDCType & (GFX_DC_S0_COLOR_FORMAT))
		{
			if (hGfxDeviceContext->S0ColorFormat != GFX_CF_ARGB1555)
			{
				GfxRegValue = ((hGfxDeviceContext->S0ColorFormat
						                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
			}
			else
			{
				GfxRegValue = ((hGfxDeviceContext->S0ColorFormat
						                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
						      ((hGfxDeviceContext->S0Alpha0
						                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
						      ((hGfxDeviceContext->S0Alpha1
						                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
			}				
			if (!GfxRegCheck((GFX_REG_S0_COLOR_FORMAT), GfxRegValue)) return 0;
				
			if (GfxDCType & GFX_DC_S0_DEFAULT_COLOR)
			{	
				GfxRegValue = (((hGfxDeviceContext->S0DefaultColor.Alpha >> (8-8))
					                     & ((1 << CMD_ALPHA_WIDTH) -1)) << CMD_ALPHA_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Red >> (8-8))
					                     & ((1 << CMD_RED_WIDTH) -1)) << CMD_RED_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Green >> (8-8))
					                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
					     (((hGfxDeviceContext->S0DefaultColor.Blue >> (8-8))
					                     & ((1 << CMD_BLUE_WIDTH) -1)) << CMD_BLUE_LSB);
				
				if (!GfxRegCheck((GFX_REG_S0_DEFAULT_COLOR), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
			}
		}
		
		//ColorKey ARGB8888
		if (hGfxDeviceContext->S0ColorKeyEnable && (GfxDCType & GFX_DC_S0_COLORKEY_PARA))
		{
			GfxRegValue =  ((hGfxDeviceContext->S0ColorKeyMin.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				           ((hGfxDeviceContext->S0ColorKeyMin.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				           ((hGfxDeviceContext->S0ColorKeyMin.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			if (!GfxRegCheck((GFX_REG_S0_COLOR_KEY_MIN), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
			
			GfxRegValue = ((hGfxDeviceContext->S0ColorKeyMax.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				          ((hGfxDeviceContext->S0ColorKeyMax.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				          ((hGfxDeviceContext->S0ColorKeyMax.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			if (!GfxRegCheck((GFX_REG_S0_COLOR_KEY_MAX), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
		}		
	}	
	//S1 Related Register Check
	if (hGfxDeviceContext->S1Enable)
	{
		if (hGfxDeviceContext->S1FetchDram && (GfxDCType & GFX_DC_S1_DRAM_PARA) )
		{	
			#if (ORION_VERSION == ORION_130)
			{
				if (!GfxRegCheck((GFX_REG_S1_SKIP_PIXEL), 
					(hGfxDeviceContext->S1SkipPixelLine & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S1_LINE_PITCH), 
					((hGfxDeviceContext->S1LinePitch >> 2 ) & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S1_ADDR)      , 
					(hGfxDeviceContext->S1BaseAddr >> 2))) return 0;			
			}
			#else
			{
				if (!GfxRegCheck((GFX_REG_S1_SKIP_PIXEL), 
					(hGfxDeviceContext->S1SkipPixelLine & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S1_LINE_PITCH), 
					(hGfxDeviceContext->S1LinePitch & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
				if (!GfxRegCheck((GFX_REG_S1_ADDR)      , 
					(hGfxDeviceContext->S1BaseAddr))) return 0;
				
				if (!(GfxDCType & GFX_DC_D_DRAM_PARA))
				{
					if (!GfxRegCheck((GFX_REG_PIXEL_NUM_PER_LINE), 
						(hGfxDeviceContext->S1PixelNumOneLine & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)))) return 0;

					if (!GfxRegCheck((GFX_REG_TOTAL_LINE_NUM), 
						(hGfxDeviceContext->S1TotalLineNum & ((1 << CMD_LINE_NUM_WIDTH) -1)))) return 0;
				}
			}
			#endif
			
		}
		//Color Info
		if (GfxDCType & (GFX_DC_S1_COLOR_FORMAT))
		{
			if (hGfxDeviceContext->S1ColorFormat != GFX_CF_ARGB1555)
			{
				GfxRegValue = ((hGfxDeviceContext->S1ColorFormat
						                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
			}
			else
			{
				GfxRegValue = ((hGfxDeviceContext->S1ColorFormat
						                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
						      ((hGfxDeviceContext->S1Alpha0
						                     & ((1 << CMD_FORMAT_ALPHA0_WIDTH) -1)) << CMD_FORMAT_ALPHA0_LSB)|
						      ((hGfxDeviceContext->S1Alpha1
						                     & ((1 << CMD_FORMAT_ALPHA1_WIDTH) -1)) << CMD_FORMAT_ALPHA1_LSB);
			}				
			if (!GfxRegCheck((GFX_REG_S1_COLOR_FORMAT), GfxRegValue)) return 0;
			
			if (GfxDCType & GFX_DC_S1_DEFAULT_COLOR)
			{
				
				GfxRegValue = (((hGfxDeviceContext->S1DefaultColor.Alpha >> (8-8))
					                     & ((1 << CMD_ALPHA_WIDTH) -1)) << CMD_ALPHA_LSB)|
					     (((hGfxDeviceContext->S1DefaultColor.Red >> (8-8))
					                     & ((1 << CMD_RED_WIDTH) -1)) << CMD_RED_LSB)|
					     (((hGfxDeviceContext->S1DefaultColor.Green >> (8-8))
					                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
					     (((hGfxDeviceContext->S1DefaultColor.Blue >> (8-8))
					                     & ((1 << CMD_BLUE_WIDTH) -1)) << CMD_BLUE_LSB);
				
				if (!GfxRegCheck((GFX_REG_S1_DEFAULT_COLOR), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
			}
		}
		
		//ColorKey ARGB8888
		if (hGfxDeviceContext->S1ColorKeyEnable && (GfxDCType & GFX_DC_S1_COLORKEY_PARA))
		{
			GfxRegValue =  ((hGfxDeviceContext->S1ColorKeyMin.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				           ((hGfxDeviceContext->S1ColorKeyMin.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				           ((hGfxDeviceContext->S1ColorKeyMin.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			if (!GfxRegCheck((GFX_REG_S1_COLOR_KEY_MIN), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
			
			GfxRegValue = ((hGfxDeviceContext->S1ColorKeyMax.Red
				                     & ((1 << CMD_RED_WIDTH) -1))   << CMD_RED_LSB)    |
				          ((hGfxDeviceContext->S1ColorKeyMax.Green
				                     & ((1 << CMD_GREEN_WIDTH) -1)) << CMD_GREEN_LSB)|
				          ((hGfxDeviceContext->S1ColorKeyMax.Blue
				                     & ((1 << CMD_BLUE_WIDTH) -1))  << CMD_BLUE_LSB);
			if (!GfxRegCheck((GFX_REG_S1_COLOR_KEY_MAX), (GfxRegValue & GFX_COLOR_REG_MASK))) return 0;
		}		
	}
	//Send D Related Command
	
	//D Dram Para
	if ((GfxDCType & GFX_DC_D_DRAM_PARA))
	{
		#if (ORION_VERSION == ORION_130)
		{
			if (!GfxRegCheck((GFX_REG_D_SKIP_PIXEL), 
					(hGfxDeviceContext->DSkipPixelLine & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)))) return 0;
			if (!GfxRegCheck((GFX_REG_D_LINE_PITCH), 
					((hGfxDeviceContext->DLinePitch >> 2) & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
			if (!GfxRegCheck((GFX_REG_D_ADDR)      , 
					(hGfxDeviceContext->DBaseAddr >> 2))) return 0;

		}
		#else
		{
			if (!GfxRegCheck((GFX_REG_D_SKIP_PIXEL), 
					(hGfxDeviceContext->DSkipPixelLine & ((1 << CMD_SKIP_PIXEL_WIDTH) -1)))) return 0;
			if (!GfxRegCheck((GFX_REG_D_LINE_PITCH), 
					(hGfxDeviceContext->DLinePitch & ((1 << CMD_PITCH_WIDTH) -1)))) return 0;
			if (!GfxRegCheck((GFX_REG_D_ADDR)      , 
					(hGfxDeviceContext->DBaseAddr))) return 0;
		}
		#endif
		if (!GfxRegCheck((GFX_REG_PIXEL_NUM_PER_LINE), 
			(hGfxDeviceContext->DPixelNumOneLine & ((1 << CMD_LPIXEL_NUM_WIDTH) -1)))) return 0;

		if (!GfxRegCheck((GFX_REG_TOTAL_LINE_NUM), 
			(hGfxDeviceContext->DTotalLineNum & ((1 << CMD_LINE_NUM_WIDTH) -1)))) return 0;

		
	}
	//D Color Info should not be A0, no Default Color
	if ((GfxDCType & GFX_DC_D_COLOR_FORMAT))
	{
		if (hGfxDeviceContext->S0ColorFormat != GFX_CF_ARGB1555)
		{
			GfxRegValue = ((hGfxDeviceContext->DColorFormat
					                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB);
		}
		else
		{
			GfxRegValue = ((hGfxDeviceContext->DColorFormat
					                     & ((1 << CMD_FORMAT_WIDTH) -1)) << CMD_FORMAT_LSB)|
					      ((hGfxDeviceContext->DAlpha0Min
					                     & ((1 << CMD_FORMAT_ALPHA0_MIN_WIDTH) -1)) << CMD_FORMAT_ALPHA0_MIN_LSB)|
					      ((hGfxDeviceContext->DAlpha0Max
					                     & ((1 << CMD_FORMAT_ALPHA0_MAX_WIDTH) -1)) << CMD_FORMAT_ALPHA0_MAX_LSB);
		}				
		if (!GfxRegCheck((GFX_REG_D_COLOR_FORMAT), GfxRegValue)) return 0;
	}
	//Clut
	//Check it Separately
	//Scalor
	#if (ORION_VERSION != ORION_130)
	{
		if (hGfxDeviceContext->S0ScalorEnable)
		{
			//Scalor Source Pixel Map
			//Send InitialPhase
			if ((GfxDCType & GFX_DC_SCALOR_INIT_PHASE))
			{			
				GfxRegValue = (hGfxDeviceContext->HInitialPhase 
					 		         & ((1 << CMD_SCALOR_H_INIT_PHASE_WIDTH) -1));
				if (!GfxRegCheck((GFX_REG_SCALOR_HORIZONTAL_INITIAL_PHASE), (GfxRegValue))) return 0;
				
				GfxRegValue = ((hGfxDeviceContext->VInitialPhase    
					 		         & ((1 << CMD_SCALOR_V_INIT_PHASE_WIDTH) -1))) ;
				if (!GfxRegCheck((GFX_REG_SCALOR_VERTICAL_INITIAL_PHASE), (GfxRegValue))) return 0;
			}	
		}
	}
	#endif
	return 1;
}
