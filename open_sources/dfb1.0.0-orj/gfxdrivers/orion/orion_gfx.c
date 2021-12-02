/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2007 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx.c
// MODULE NAME      : Orion 2D Gfx Direct FB Driver
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 07-05-14
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      07-05-14   jiasheng Chen           Original
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Orion 2D Gfx Direct FB Driver
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#include <config.h>

#include <dfb_types.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <fbdev/fb.h>

#include <directfb.h>

#include <direct/messages.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/state.h>
#include <core/gfxcard.h>
#include <core/surfaces.h>

#include <gfx/convert.h>
#include <gfx/util.h>
#include <misc/conf.h>

#include <core/graphics_driver.h>
DFB_GRAPHICS_DRIVER( orion_gfx )

#include "orion_gfx.h"
#include "orion_gfx_2dlib.h"
#include "orion_gfx_api.h"
#include "orion_gfx_hw_if.h"
#include "orion_gfx_loglib.h"
#include <assert.h>
#include <memory.h>



#define ORIONGFX_SUPPORTED_DRAWINGFLAGS \
               (DSDRAW_NOFX |DSDRAW_BLEND)

#define ORIONGFX_SUPPORTED_DRAWINGFUNCTIONS \
               (DFXL_FILLRECTANGLE )

#define ORIONGFX_SUPPORTED_BLITTINGFLAGS \
               (DSBLIT_SRC_COLORKEY | DSBLIT_DST_COLORKEY |\
                DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA)

#if (ORION_VERSION != ORION_130)
#define ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS \
               (DFXL_BLIT | DFXL_STRETCHBLIT)
#else
#define ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS \
               (DFXL_BLIT )
#endif

/*
      * Called after driver->InitDevice() and during dfb_gfxcard_unlock( true ).
      * The driver should do the one time initialization of the engine,
      * e.g. writing some registers that are supposed to have a fixed value.
      *
      * This happens after mode switching or after returning from
      * OpenGL state (e.g. DRI driver).
      */
void OrionGfxEngineReset( void *driver_data, void *device_data )
{
	PRINT_D1_L1("ORION_GFX: %s\n", __FUNCTION__);
	//Do Software Reset
	//Initial the Device Data to Reset State
	hGfx2DInit();
}


/*
      * Makes sure that graphics hardware has finished all operations.
      *
      * This method is called before the CPU accesses a surface' buffer
      * that had been written to by the hardware after this method has been
      * called the last time.
      *
      * It's also called before entering the OpenGL state (e.g. DRI driver).
      */
DFBResult OrionGfxEngineSync( void *driver_data, void *device_data )
{
	PRINT_D1_L1("ORION_GFX: %s\n", __FUNCTION__);
	OrionGfxDriverData *drv = (OrionGfxDriverData *)driver_data;
	OrionGfxDeviceData *dev = (OrionGfxDeviceData *)device_data;
	//Wait Gfx Engine to be Idel and Command Queue is Empty
	if (!hGfxAPIWaitCMDQueueEmpty()) return DFB_FAILURE;
	if (!hGfxAPIWaitGfxToBeIdle(&dev->GfxDC)) return DFB_FAILURE;
	return DFB_OK;
}

     /*
      * Called during dfb_gfxcard_lock() to notify the driver that
      * the current rendering state is no longer valid.
      */
void OrionGfxInvalidateState( void *driver_data, void *device_data )
{
	
}
	 /*
      * after the video memory has been written to by the CPU (e.g. modification
      * of a texture) make sure the accelerator won't use cached texture data
      */
void OrionGfxFlushTextureCache( void *driver_data, void *device_data )
{
	//Flush Cache	
}

/*
 * After the video memory has been written to by the accelerator
 * make sure the CPU won't read back cached data.
 */
void OrionGfxFlushReadCache( void *driver_data, void *device_data )
{
	//Flush Cache;
}

     /*
      * Called before a software access to a video surface buffer.
      */
void OrionGfxSurfaceEnter( void *driver_data, void *device_data,
                           SurfaceBuffer *buffer, DFBSurfaceLockFlags flags )
{
	
}

     /*
      * Called after a software access to a video surface buffer.
      */
void OrionGfxSurfaceLeave( void *driver_data, void *device_data, SurfaceBuffer *buffer )
{
	
}


	/*
      * Check if the function 'accel' can be accelerated with the 'state'.
      * If that's true, the function sets the 'accel' bit in 'state->accel'.
      * Otherwise the function just returns, no need to clear the bit.
      */
void OrionGfxCheckState( void *driver_data, void *device_data,
                         CardState *state, DFBAccelerationMask accel )
{
	PRINT_L1("ORION_GFX: OrionGfxCheckState Start:\n");
	switch (state->destination->format) {
	case DSPF_LUT8:
#if (ORION_VERSION == ORION_131)
	case DSPF_ARGB1555: 
#endif
	case DSPF_RGB16:
	case DSPF_ARGB4444:
		break;
	default:
		ERROR_D1_L1("##ORION_GFX: un-surpported Destination Format %d\n", state->destination->format);
		return;
	}

	if (DFB_DRAWING_FUNCTION(accel)) {
		if (state->drawingflags & ~ORIONGFX_SUPPORTED_DRAWINGFLAGS)
			return;
		state->accel |= ORIONGFX_SUPPORTED_DRAWINGFUNCTIONS;
	}
	else {
		if (state->blittingflags & ~ORIONGFX_SUPPORTED_BLITTINGFLAGS)
			return;
		
		switch (state->source->format) {
		case DSPF_LUT8:
	#if (ORION_VERSION == ORION_131)
		case DSPF_ARGB1555: 
	#endif
		case DSPF_RGB16:
		case DSPF_ARGB4444:
			break;
		default:
			ERROR_D1_L1("##ORION_GFX: un-surpported Source Format %d\n", state->source->format);
			return;
		}
		if (accel & DFXL_STRETCHBLIT)
		{
			if (state->blittingflags ||
				state->source->format == DSPF_LUT8)
			{
				ERROR_L1("##ORION_GFX: un-surpported Scalor Operation %d\n");
				state->accel &= (~DFXL_STRETCHBLIT);
				return;
			}
		}
		state->accel |= ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS;
	}
	PRINT_L1("ORION_GFX: OrionGfxCheckState END\n");
}

     /*
      * Program card for execution of the function 'accel' with the 'state'.
      * 'state->modified' contains information about changed entries.
      * This function has to set at least 'accel' in 'state->set'.
      * The driver should remember 'state->modified' and clear it.
      * The driver may modify 'funcs' depending on 'state' settings.
      */
int MapOrionGfxColorFmt( GFX_COLOR_FORMAT *OrionColorFmt, DFBSurfacePixelFormat  DFBColorFmt)
{
	if (OrionColorFmt == NULL) return 0;
	
	switch (DFBColorFmt) {
	case DSPF_LUT8:
		*OrionColorFmt = GFX_CF_CLUT8;
		break;
#if (ORION_VERSION == ORION_131)
	case DSPF_ARGB1555: 
		*OrionColorFmt = GFX_CF_ARGB1555;
		break;
#endif
	case DSPF_RGB16:
		*OrionColorFmt = GFX_CF_RGB565;
		break;
	case DSPF_ARGB4444:
		*OrionColorFmt = GFX_CF_ARGB4444;
		break;
	default:
		return 0;
	}
	return 1;
}

int MapOrionGfxSrcImgPara(OrionGfxDriverData *drv, CardState *state)
{
	COLOR_IMG *SrcImg  = NULL;
	CoreSurface   *src = NULL;
	assert(state != NULL);

	src = state->source;
	
	if (drv == NULL || state == NULL) return 0;
	
	SrcImg = &drv->SrcImg;

	if (!MapOrionGfxColorFmt(&SrcImg->ColorFormat, src->format)) return 0;
	SrcImg->PixelHeight  = src->height;
	SrcImg->PixelWidth   = src->width;
	SrcImg->StartAddress = dfb_gfxcard_memory_physical( NULL, src->front_buffer->video.offset);
	SrcImg->LinePitch    = src->front_buffer->video.pitch;
	//Default Color 
	SrcImg->DefaultColor.Alpha = state->color.a;
	SrcImg->DefaultColor.Red   = state->color.r;
	SrcImg->DefaultColor.Green = state->color.g;
	SrcImg->DefaultColor.Blue  = state->color.b;
	return 1;
}

int MapOrionGfxDesImgPara(OrionGfxDriverData *drv, CardState *state)
{
	COLOR_IMG *DesImg  = NULL;
	CoreSurface   *dst = NULL;
	assert(state != NULL);

	dst = state->destination;
	
	if (drv == NULL || state == NULL) return 0;
	
	DesImg = &drv->DesImg;

	if (!MapOrionGfxColorFmt(&DesImg->ColorFormat, dst->format)) return 0;
	DesImg->PixelHeight  = dst->height;
	DesImg->PixelWidth   = dst->width;
	DesImg->StartAddress = dfb_gfxcard_memory_physical( NULL, dst->back_buffer->video.offset);
	DesImg->LinePitch    = dst->back_buffer->video.pitch;
	//Default Color for Fill or Draw
	DesImg->DefaultColor.Alpha = state->color.a;
	DesImg->DefaultColor.Red   = state->color.r;
	DesImg->DefaultColor.Green = state->color.g;
	DesImg->DefaultColor.Blue  = state->color.b;
	return 1;
}

void OrionGfxSetState  ( void *driver_data, void *device_data,
                         struct _GraphicsDeviceFuncs *funcs,
                         CardState *state, DFBAccelerationMask accel )
{
	OrionGfxDriverData *drv = (OrionGfxDriverData *)driver_data;
	OrionGfxDeviceData *dev = (OrionGfxDeviceData *)device_data;
	COLOR_IMG *DesImg  = NULL, *SrcImg = NULL;
	DesImg = &drv->DesImg;
	SrcImg = &drv->SrcImg;
/*
// For Orion Gfx Driver update all the field infomation according the operation function only
// in first step.
*/
	PRINT_L1("ORION_GFX: OrionGfxSetState Start:\n");
	if (state->modified) {
		if (state->modified & SMF_DRAWING_FLAGS)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_DRAWING_FLAGS");
		}
		if (state->modified & SMF_BLITTING_FLAGS)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_BLITTING_FLAGS");
		}
		if (state->modified & SMF_CLIP)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_CLIP");
		}
		if (state->modified & SMF_COLOR)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_COLOR");
		}
		if (state->modified & SMF_SRC_BLEND)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_SRC_BLEND");
		}
		if (state->modified & SMF_DST_BLEND)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_DST_BLEND");
		}
		if (state->modified & SMF_SRC_COLORKEY)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_SRC_COLORKEY");
		}
		if (state->modified & SMF_DST_COLORKEY)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_DST_COLORKEY");
		}
		if (state->modified & SMF_DESTINATION)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_DESTINATION");
		}
		if (state->modified & SMF_SOURCE)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_SOURCE");
		}
		if (state->modified & SMF_INDEX_TRANSLATION)
		{
			PRINT_D2_L2("%s: State Modified: %s \n", __FUNCTION__,"SMF_INDEX_TRANSLATION");
		}
		PRINT_D1_L1("##ORION_GFX: State Modified: %08x \n", state->modified);
	}
	state->modified = 0;//??
	switch (accel) {
	case DFXL_FILLRECTANGLE:
		PRINT_D2_L2("%s: accel type: %s \n", __FUNCTION__,"DFXL_FILLRECTANGLE");
		if (state->drawingflags)
		{
			funcs->FillRectangle = NULL;
			state->set = DFXL_NONE;
			if (state->drawingflags & DSDRAW_BLEND)
			{	
				PRINT_D2_L2("%s: drawingflags: %s \n", __FUNCTION__,"DSDRAW_BLEND");
				//  Zero       SrcAlpha     Des         0xFF     COPY_PEN
				drv->BLTType = ORION_GFX_BLEND_DES_WITH_CONST;
			 	if(!MapOrionGfxDesImgPara(drv, state))
			 	{
			 		PRINT_L1("##ORION_GFX: Failed to Set the Destination Para for FILL_With Alpha\n");
					return;
			 	}
				drv->IsSrcOnTopDes = 1;
				drv->RopValue      = ROP_R2_COPYPEN;
				SrcImg->DefaultColor.Alpha = state->color.a;
				SrcImg->DefaultColor.Red   = state->color.r;
				SrcImg->DefaultColor.Green = state->color.g;
				SrcImg->DefaultColor.Blue  = state->color.b;
				
				DesImg->DefaultColor.Alpha = 0xFF;
				funcs->FillRectangle     = OrionGfxFillRectangle;
				state->set = DSDRAW_BLEND ;
			}		
			else
			{
				return;
			}
		}
		else 
		if (MapOrionGfxDesImgPara(drv, state))
		{
			state->set = ORIONGFX_SUPPORTED_DRAWINGFUNCTIONS;
			drv->BLTType = ORION_GFX_FILL_ONLY;
			funcs->FillRectangle     = OrionGfxFillRectangle;
		}
		else
		{
			PRINT_L1("##ORION_GFX: Failed to Set the Destination Para for Fill and Draw\n");
		}
		break;
	case DFXL_BLIT:
		PRINT_D2_L2("%s: accel type: %s \n", __FUNCTION__,"DFXL_BLIT");
		funcs->Blit = NULL;
		state->set  = DFXL_NONE;
		if (!state->blittingflags)
	    {
	    	//Jschen: Direct Copy Source 0 to Destination
	    	PRINT_D2_L2("%s: state->blittingflags: %s \n", __FUNCTION__,"NONE");
	    	drv->BLTType = ORION_GFX_COPY_ONLY;
			if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
			{
				PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for COPY_ONLY\n");
				return;
			}
			funcs->Blit = OrionGfxBlit;
			state->set = ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS;
	    }
		else
		{
			if (state->blittingflags & DSBLIT_DST_COLORKEY) 
			{
				U32 ColorKey = 0;
				/* Jschen: 
				// 1. Set Des ColorKey Min and Max Equals state->dst_colorkey
				*/
				PRINT_D2_L2("%s: state->blittingflags: %s \n", __FUNCTION__,"DSBLIT_DST_COLORKEY");
				drv->DesColorKeyEnable = 1;
				PRINT_D1_L2("state->dst_colorkey = 0x%08x\n", state->dst_colorkey);
				#if (ORION_VERSION == ORION_130) //For 16Bit Only
				if (DesImg->ByteEndian == BYTE_LITTLE_ENDIAN)
				{
					ColorKey = ((state->dst_colorkey & 0xFF) << 8) | ((state->dst_colorkey & 0xFF00) >> 8);
				}
				else
				{
					ColorKey = state->dst_colorkey;
				}
				#else
				ColorKey = state->dst_colorkey;
				#endif

				if (state->destination->format == DSPF_RGB16)
				{
					drv->DesMinColor.Red   = drv->DesMaxColor.Red   = ((ColorKey >> 11) & 0x1F) << 3;
					drv->DesMinColor.Green = drv->DesMaxColor.Green = ((ColorKey >>  5) & 0x3F) << 2;
					drv->DesMinColor.Blue  = drv->DesMaxColor.Blue  = ((ColorKey >>  0) & 0x1F) << 3;
				}
				else if (state->destination->format == DSPF_ARGB4444)
				{
					drv->DesMinColor.Red   = drv->DesMaxColor.Red   = ((ColorKey >>  8) & 0xF) << 4;
					drv->DesMinColor.Green = drv->DesMaxColor.Green = ((ColorKey >>  4) & 0xF) << 4;
					drv->DesMinColor.Blue  = drv->DesMaxColor.Blue  = ((ColorKey >>  0) & 0xF) << 4;
				}
				else if (state->destination->format == DSPF_ARGB1555)
				{
					drv->DesMinColor.Red   = drv->DesMaxColor.Red   = ((ColorKey >>  8) & 0x1F) << 3;
					drv->DesMinColor.Green = drv->DesMaxColor.Green = ((ColorKey >>  4) & 0x1F) << 3;
					drv->DesMinColor.Blue  = drv->DesMaxColor.Blue  = ((ColorKey >>  0) & 0x1F) << 3;
				}
				else 
				{
					ERROR_D2_L2("%UnSpported Format: %s \n", __FUNCTION__,"DSBLIT_DST_COLORKEY");
					return;
				}

		    }
			if (state->blittingflags & DSBLIT_SRC_COLORKEY) {
		    	/* Jschen: 
				// 1. Set Source's Color Key Min and Max equals state->src_colorkey
				*/
				U32 ColorKey = 0;
				PRINT_D2_L2("%s: state->blittingflags: %s \n", __FUNCTION__,"DSBLIT_SRC_COLORKEY");				
				drv->SrcColorKeyEnable = 1;
				PRINT_D1_L2("state->src_colorkey = 0x%08x\n", state->src_colorkey);
				#if (ORION_VERSION == ORION_130) //For 16Bit Only
				if (SrcImg->ByteEndian == BYTE_LITTLE_ENDIAN)
				{
					ColorKey = ((state->src_colorkey & 0xFF) << 8) | ((state->src_colorkey & 0xFF00) >> 8);
				}
				else
				{
					ColorKey = state->src_colorkey;
				}
				#else
				ColorKey = state->src_colorkey;
				#endif
				if (state->source->format == DSPF_RGB16)
				{
					drv->SrcMinColor.Red   = drv->SrcMaxColor.Red   = ((ColorKey >> 11) & 0x1F) << 3;
					drv->SrcMinColor.Green = drv->SrcMaxColor.Green = ((ColorKey >>  5) & 0x3F) << 2;
					drv->SrcMinColor.Blue  = drv->SrcMaxColor.Blue  = ((ColorKey >>  0) & 0x1F) << 3;
				}
				else if (state->source->format == DSPF_ARGB4444)
				{
					drv->SrcMinColor.Red   = drv->SrcMaxColor.Red   = ((ColorKey >>  8) & 0xF) << 4;
					drv->SrcMinColor.Green = drv->SrcMaxColor.Green = ((ColorKey >>  4) & 0xF) << 4;
					drv->SrcMinColor.Blue  = drv->SrcMaxColor.Blue  = ((ColorKey >>  0) & 0xF) << 4;
				}
				else if (state->source->format == DSPF_ARGB1555)
				{
					drv->SrcMinColor.Red   = drv->SrcMaxColor.Red   = ((ColorKey >>  8) & 0x1F) << 3;
					drv->SrcMinColor.Green = drv->SrcMaxColor.Green = ((ColorKey >>  4) & 0x1F) << 3;
					drv->SrcMinColor.Blue  = drv->SrcMaxColor.Blue  = ((ColorKey >>  0) & 0x1F) << 3;
				}
				else 
				{
					ERROR_D2_L2("%UnSpported Format: %s \n", __FUNCTION__,"DSBLIT_SRC_COLORKEY");
					return;
				}

		    }
		    if ((state->blittingflags & DSBLIT_SRC_PREMULTIPLY) ||
				(state->blittingflags & DSBLIT_SRC_PREMULTCOLOR) ||
				(state->blittingflags & DSBLIT_COLORIZE) ||
				(state->blittingflags & DSBLIT_DST_PREMULTIPLY)||
				(state->blittingflags & DSBLIT_XOR) )	//Jschen: Set the Final Stage ROP Operation, Disabled when 1.3(Having Bugs)
		    {
		    	ERROR_D1_L1("##ORION_GFX: Unspported blittingflags %08x\n",state->blittingflags);
				return;
		    }
			//Map Different Type of BLT Operation
			//state->color.a
			if (state->blittingflags & (DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA)) 
			{
				PRINT_D2_L2("%s: state->blittingflags: %s \n", __FUNCTION__,
					             "DSBLIT_BLEND_ALPHACHANNEL | DSBLIT_BLEND_COLORALPHA");
				
				PRINT_D3_L2("%s: SrcBlend Func(%d) and DesBlend Func(%d)\n",__FUNCTION__,
						    state->src_blend, state->dst_blend);
			 		//TopSuface   TopAlpha  BotSurface   BotAlpha   ROP
			 	if      (state->src_blend == DSBF_SRCALPHA && state->dst_blend == DSBF_INVSRCALPHA)
			 	{
			 		//  Src       SrcAlpha     Des         0xFF     COPY_PEN
			 		drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					else if (SrcImg->ColorFormat == GFX_CF_CLUT8 && DesImg->ColorFormat == GFX_CF_CLUT8)
					{
						PRINT_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = state->color.a;
					DesImg->DefaultColor.Alpha = 0xFF;
					
			 	}
			 	else if (state->src_blend == DSBF_ZERO && state->dst_blend == DSBF_ZERO)
			 	{
			 		//   Fill with Fixed Color 0
			 		drv->BLTType = ORION_GFX_FILL_ONLY;
			 		if(!MapOrionGfxDesImgPara(drv, state))
			 		{
			 			PRINT_L1("##ORION_GFX: Failed to Set the Destination Para for FILL_ONLY\n");
						return;
			 		}
					DesImg->DefaultColor.Alpha = 0xFF;
					DesImg->DefaultColor.Red   = 0x0;
					DesImg->DefaultColor.Green = 0x0;
					DesImg->DefaultColor.Blue  = 0x0;
			 	}
				else if (state->src_blend == DSBF_ONE && state->dst_blend == DSBF_ZERO)
				{
					//   Copy Src directly to Des
					drv->BLTType = ORION_GFX_COPY_ONLY;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for COPY_ONLY\n");
						return;
					}
				}
				else if (state->src_blend == DSBF_ONE && state->dst_blend == DSBF_INVSRCALPHA)
				{
					//   Src       0xFF         Des         0xFF     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					else if (SrcImg->ColorFormat == GFX_CF_CLUT8 && DesImg->ColorFormat == GFX_CF_CLUT8)
					{
						PRINT_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = 0xFF;
					DesImg->DefaultColor.Alpha = 0xFF;
				}
				else if (state->src_blend == DSBF_INVDESTALPHA && state->dst_blend == DSBF_ONE)
				{
					//   Des       0xFF         Src         0xFF     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 0;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = 0xFF;
					DesImg->DefaultColor.Alpha = 0xFF;
				}
				else if (state->src_blend == DSBF_DESTALPHA && state->dst_blend == DSBF_ZERO)
				{
					//   Copy Src directly to Des
					drv->BLTType = ORION_GFX_COPY_ONLY;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						D_INFO("##ORION_GFX: Failed to Set the Des and Src Para for COPY_ONLY\n");
						return;
					}
					
				}
				else if (state->src_blend == DSBF_ZERO && state->dst_blend == DSBF_INVSRCALPHA)
				{
					//  Zero       SrcAlpha     Des         0xFF     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_DES_WITH_CONST;
			 		if(!MapOrionGfxDesImgPara(drv, state))
			 		{
			 			PRINT_L1("##ORION_GFX: Failed to Set the Destination Para for FILL_ONLY\n");
						return;
			 		}
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					SrcImg->DefaultColor.Alpha = state->color.a;
					SrcImg->DefaultColor.Red   = 0x0;
					SrcImg->DefaultColor.Green = 0x0;
					SrcImg->DefaultColor.Blue  = 0x0;
					
					DesImg->DefaultColor.Alpha = 0xFF;
					
				}
				else if (state->src_blend == DSBF_DESTALPHA && state->dst_blend == DSBF_INVSRCALPHA)
				{
					//  Src        SrcAlpha     Des         0xFF     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					else if (SrcImg->ColorFormat == GFX_CF_CLUT8 && DesImg->ColorFormat == GFX_CF_CLUT8)
					{
						PRINT_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = state->color.a;
					DesImg->DefaultColor.Alpha = 0xFF;			
					
				}
				else if (state->src_blend == DSBF_INVDESTALPHA && state->dst_blend == DSBF_SRCALPHA)
				{
					//  Des        0xFF         Src         SrcAlpha COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 0;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = state->color.a;
					DesImg->DefaultColor.Alpha = 0xFF;
					
				}
				else if (state->src_blend == DSBF_ONE && state->dst_blend == DSBF_ONE)
				{
					//  Src        0x80         Des         0x80     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					else if (SrcImg->ColorFormat == GFX_CF_CLUT8 && DesImg->ColorFormat == GFX_CF_CLUT8)
					{
						PRINT_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = 0x80;
					DesImg->DefaultColor.Alpha = 0xFF;
					
				}
				else
				{
					ERROR_D2_L1("##ORION_GFX: Unspported SrcBlend(%d) and DesBlend Func(%d)\n",
						    state->src_blend, state->dst_blend);
					return;
				}				 
			 }
			else
			{
				//   Src       0xFF         Des         0xFF     COPY_PEN
					drv->BLTType = ORION_GFX_BLEND_TWO_SOURCE;
					drv->IsSrcOnTopDes = 1;
					drv->RopValue      = ROP_R2_COPYPEN;
					if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
					{
						PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for BLT\n");
						return;
					}
					else if (SrcImg->ColorFormat == GFX_CF_CLUT8 && DesImg->ColorFormat == GFX_CF_CLUT8)
					{
						PRINT_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
						return;
					}
					SrcImg->DefaultColor.Alpha = 0xFF;
					DesImg->DefaultColor.Alpha = 0xFF;
			}
			funcs->Blit = OrionGfxBlit;
			state->set = ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS;
		}		
		break;
	#if (ORION_VERSION != ORION_130)//Not Support in Orion1.3
	case DFXL_STRETCHBLIT:
		PRINT_D2_L2("%s: accel type: %s \n", __FUNCTION__,"DFXL_STRETCHBLIT");
		//Copy Operation Only
		funcs->StretchBlit = NULL;
		state->set = DFXL_NONE;
		if (!state->blittingflags)
	    {
	    	//Jschen: Call Scalor Operation
	    	drv->BLTType = ORION_GFX_SCALOR;
			if (!MapOrionGfxSrcImgPara(drv, state) || !MapOrionGfxDesImgPara(drv, state))
			{
				PRINT_L1("##ORION_GFX: Failed to Set the Des and Src Para for ORION_GFX_SCALOR\n");
				return;
			}
			else if ((SrcImg->ColorFormat != GFX_CF_ARGB1555) &&
			         (SrcImg->ColorFormat != GFX_CF_RGB565) &&
			         (SrcImg->ColorFormat != GFX_CF_ARGB4444))
			{
				PRINT_D1_L1("##ORION_GFX: Unsported Scalor Source Format(%d)\n", SrcImg->ColorFormat);
				return;
			}
			else if ((DesImg->ColorFormat != GFX_CF_ARGB1555) &&
			         (DesImg->ColorFormat != GFX_CF_RGB565) &&
					 (DesImg->ColorFormat != GFX_CF_ARGB4444))
			{
				PRINT_D1_L1("##ORION_GFX: Unsported Scalor Source Format(%d)\n", DesImg->ColorFormat);
				return;
			}
			else
			{
				PRINT_L1("##ORION_GFX: Set BLTType: ORION_GFX_SCALOR\n");
			}
			funcs->StretchBlit = OrionGfxStretchBlit;
			state->set = ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS;
	    }
		break;
	#endif
	default:
		ERROR_L1("#ORION_GFX: unexpected drawing or blitting function\n");
		break;
	}
	state->modified = 0;
	PRINT_L1("ORION_GFX: OrionGfxSetState END:\n");
}

/*
 * drawing functions
 */
bool OrionGfxFillRectangle ( void *driver_data, void *device_data, DFBRectangle *rect )
{
	OrionGfxDriverData *drv = (OrionGfxDriverData *)driver_data;
	OrionGfxDeviceData *dev = (OrionGfxDeviceData *)device_data;
	
	HG2D_RESULT hResult = HG2D_UNKNOW;
    GFX_DEVICE_CONTEXT *hGfxDC    = &dev->GfxDC; 
    I32                 GfxSrcId  = 1; //Source 1
    COLOR_IMG          *DesImg    = &drv->DesImg; 
    RECT                DesRect; 
    GFX_ARGB_COLOR     *FillColor = &drv->DesImg.DefaultColor;
    ROP_OPT             RopValue  = ROP_R2_NOP; //Direct Ouput Soruce 1;
	
	DesRect.left   = rect->x;
	DesRect.right  = rect->x + rect->w;
	DesRect.top    = rect->y;
	DesRect.bottom = rect->y + rect->h;

	PRINT_L1("ORION_GFX: OrionGfxFillRectangle Start:\n");
	PRINT_D3_L2("DesImg->ColorFormat(%d), DesImg->PixelWidth(%d), DesImg->PixelHeight(%d) \n", 
		      DesImg->ColorFormat, DesImg->PixelWidth, DesImg->PixelHeight);
	PRINT_D2_L2("DesImg->StartAddress(%08x), DesImg->LinePitch(%d)\n",
		DesImg->StartAddress, DesImg->LinePitch);
	PRINT_D2_L2("DesRect.left(%d), DesRect.right(%d) \n", DesRect.left, DesRect.right);
	PRINT_D2_L2("DesRect.top(%d), DesRect.bottom(%d) \n", DesRect.top,DesRect.bottom);
	
	if (drv->BLTType == ORION_GFX_FILL_ONLY)
	{
		PRINT_D2_L2("Fill Color: %02x%02x",FillColor->Alpha,FillColor->Red);
		PRINT_D2_L2("%02x%02x\n",FillColor->Green,FillColor->Blue);
		hResult = hGfx2DFill
		( 
		    hGfxDC, 
		    GfxSrcId,
		    DesImg, 
		    &DesRect, 
		    FillColor,
		    RopValue 
		);
	}
	else if (drv->BLTType == ORION_GFX_BLEND_DES_WITH_CONST)
	{
		HGFX_BLT_SRC_PARA  _Src1Para;
		HGFX_BLT_DES_PARA  _DesPara;
		GFX_ARGB_COLOR     *Src0DefaultColor = NULL;

		_Src1Para.Img = DesImg;
		_Src1Para.ColorKeyEnable = 0;//drv->DesColorKeyEnable;
		/*
		if (_Src1Para.ColorKeyEnable)
		{
			_Src1Para.MinColor.Red   = drv->DesMinColor.Red;
			_Src1Para.MinColor.Green = drv->DesMinColor.Green;
			_Src1Para.MinColor.Blue  = drv->DesMinColor.Blue;

			_Src1Para.MaxColor.Red   = drv->DesMaxColor.Red;
			_Src1Para.MaxColor.Green = drv->DesMaxColor.Green;
			_Src1Para.MaxColor.Blue  = drv->DesMaxColor.Blue;
		}
		*/
		_Src1Para.VReverse = 0;
		_Src1Para.Rect = &DesRect;
		
		Src0DefaultColor = &drv->SrcImg.DefaultColor;
		PRINT_D2_L2("Fill Color: %02x%02x",Src0DefaultColor->Alpha,Src0DefaultColor->Red);
		PRINT_D2_L2("%02x%02x\n",Src0DefaultColor->Green,Src0DefaultColor->Blue);
		_DesPara.BlendEnable  = 1;
		_DesPara.Img          = DesImg;
		_DesPara.IsS0OnTopS1  = drv->IsSrcOnTopDes;
		_DesPara.ROPAlphaCtrl = drv->ROPAlphaCtrl;
		_DesPara.RopValue     = drv->RopValue;
		_DesPara.VReverse     = 0;

		_DesPara.Rect = &DesRect;
		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_BLEND_DES_WITH_CONST");
		
		hResult = hGfx2DBLTSrc1
		(
		    hGfxDC,
		    &_Src1Para,
			1,
		    Src0DefaultColor, 
		    &_DesPara
		);
		return (hResult == HG2D_SUCCESS);

	}
	
	PRINT_L1("ORION_GFX: OrionGfxFillRectangle END\n");
	return (hResult == HG2D_SUCCESS);
}

bool OrionGfxDrawRectangle ( void *driver_data, void *device_data,
                             DFBRectangle *rect )
{
	//Not Implement
	PRINT_L1("ORION_GFX: OrionGfxDrawRectangle Start:\n");
	return 1;
}

/*
     bool (*DrawLine)      ( void *driver_data, void *device_data,
                             DFBRegion *line );

     bool (*FillTriangle)  ( void *driver_data, void *device_data,
                             DFBTriangle *tri );
*/
     /*
      * blitting functions
      */
bool OrionGfxBlit( void *driver_data, void *device_data,
                               DFBRectangle *rect, int dx, int dy )
{
	OrionGfxDriverData *drv = (OrionGfxDriverData *)driver_data;
	OrionGfxDeviceData *dev = (OrionGfxDeviceData *)device_data;
	
	HG2D_RESULT hResult = HG2D_UNKNOW;
    GFX_DEVICE_CONTEXT *hGfxDC    = &dev->GfxDC; 
	COLOR_IMG          *SrcImg    = &drv->SrcImg;
	COLOR_IMG          *DesImg    = &drv->DesImg; 
	RECT                 SrcRect;
	RECT                 DesRect;

	SrcRect.left   = rect->x;
	SrcRect.right  = rect->x + rect->w;
	SrcRect.top    = rect->y;
	SrcRect.bottom = rect->y + rect->h;

	DesRect.left   = dx;
	DesRect.right  = dx + rect->w;
	DesRect.top    = dy;
	DesRect.bottom = dy + rect->h;

	PRINT_D3_L2("SrcImg->ColorFormat(%d), SrcImg->PixelWidth(%d), SrcImg->PixelHeight(%d) \n", 
		      SrcImg->ColorFormat, SrcImg->PixelWidth, SrcImg->PixelHeight);
	PRINT_D2_L2("SrcImg->StartAddress(%08x), SrcImg->LinePitch(%d)\n",
		SrcImg->StartAddress, SrcImg->LinePitch);
	PRINT_D2_L2("SrcRect.left(%d), SrcRect.right(%d) \n", SrcRect.left, SrcRect.right);
	PRINT_D2_L2("SrcRect.top(%d), SrcRect.bottom(%d) \n", SrcRect.top,SrcRect.bottom);


	PRINT_D3_L2("DesImg->ColorFormat(%d), DesImg->PixelWidth(%d), DesImg->PixelHeight(%d) \n", 
		      DesImg->ColorFormat, DesImg->PixelWidth, DesImg->PixelHeight);
	PRINT_D2_L2("DesImg->StartAddress(%08x), DesImg->LinePitch(%d)\n",
		DesImg->StartAddress, DesImg->LinePitch);
	PRINT_D2_L2("DesRect.left(%d), DesRect.right(%d) \n", DesRect.left, DesRect.right);
	PRINT_D2_L2("DesRect.top(%d), DesRect.bottom(%d) \n", DesRect.top,DesRect.bottom);

	if (drv->BLTType == ORION_GFX_FILL_ONLY)
	{
	    I32                 GfxSrcId  = 1; //Source 1	    
	    GFX_ARGB_COLOR     *FillColor = &drv->DesImg.DefaultColor;
	    ROP_OPT             RopValue  = ROP_R2_NOP; //Direct Ouput Soruce 0;

		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_FILL_ONLY");
		hResult = hGfx2DFill
		( 
		    hGfxDC, 
		    GfxSrcId,
		    DesImg, 
		    &DesRect, 
		    FillColor,
		    RopValue 
		);
		return (hResult == HG2D_SUCCESS);

	}
	else if (drv->BLTType == ORION_GFX_COPY_ONLY)
	{
		I32                  GfxSrcId = 1;
		ROP_OPT              RopValue = ROP_R2_NOP;
		I32                  SrcVReverse = 0;
		I32                  DesVReverse = 0;

		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_COPY_ONLY");
		hResult = hGfx2DCopy
		( 
		    hGfxDC,
		    GfxSrcId,
		    SrcImg, 
		    &SrcRect, 
		    SrcVReverse,
		    DesImg, 
		    &DesRect, 
		    DesVReverse,
		    RopValue 		    
		);
		return (hResult == HG2D_SUCCESS);
	}
	else if (drv->BLTType == ORION_GFX_BLEND_DES_WITH_CONST)
	{
		HGFX_BLT_SRC_PARA  _Src1Para;
		HGFX_BLT_DES_PARA  _DesPara;
		GFX_ARGB_COLOR     *Src0DefaultColor = NULL;

		_Src1Para.Img = DesImg;
		_Src1Para.ColorKeyEnable = drv->DesColorKeyEnable;
		if (_Src1Para.ColorKeyEnable)
		{
			_Src1Para.MinColor.Red   = drv->DesMinColor.Red;
			_Src1Para.MinColor.Green = drv->DesMinColor.Green;
			_Src1Para.MinColor.Blue  = drv->DesMinColor.Blue;

			_Src1Para.MaxColor.Red   = drv->DesMaxColor.Red;
			_Src1Para.MaxColor.Green = drv->DesMaxColor.Green;
			_Src1Para.MaxColor.Blue  = drv->DesMaxColor.Blue;
		}
		_Src1Para.VReverse = 0;
		_Src1Para.Rect = &DesRect;
		
		Src0DefaultColor = &SrcImg->DefaultColor;

		_DesPara.BlendEnable  = 1;
		_DesPara.Img          = DesImg;
		_DesPara.IsS0OnTopS1  = drv->IsSrcOnTopDes;
		_DesPara.ROPAlphaCtrl = drv->ROPAlphaCtrl;
		_DesPara.RopValue     = drv->RopValue;
		_DesPara.VReverse     = 0;

		_DesPara.Rect = &DesRect;
		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_BLEND_DES_WITH_CONST");
		hResult = hGfx2DBLTSrc1
		(
		    hGfxDC,
		    &_Src1Para,
			1,
		    Src0DefaultColor, 
		    &_DesPara
		);
		return (hResult == HG2D_SUCCESS);

	}
	else if (drv->BLTType == ORION_GFX_BLEND_SRC_WITH_CONST)
	{
		HGFX_BLT_SRC_PARA  _Src1Para;
		HGFX_BLT_DES_PARA  _DesPara;
		GFX_ARGB_COLOR     *Src0DefaultColor = NULL;

		_Src1Para.Img = SrcImg;
		_Src1Para.ColorKeyEnable = drv->SrcColorKeyEnable;
		if (_Src1Para.ColorKeyEnable)
		{
			_Src1Para.MinColor.Red   = drv->SrcMinColor.Red;
			_Src1Para.MinColor.Green = drv->SrcMinColor.Green;
			_Src1Para.MinColor.Blue  = drv->SrcMinColor.Blue;

			_Src1Para.MaxColor.Red   = drv->SrcMaxColor.Red;
			_Src1Para.MaxColor.Green = drv->SrcMaxColor.Green;
			_Src1Para.MaxColor.Blue  = drv->SrcMaxColor.Blue;
		}
		_Src1Para.VReverse    = 0;
		_Src1Para.Rect = &SrcRect;
		
		Src0DefaultColor = &DesImg->DefaultColor;

		_DesPara.BlendEnable  = 1;
		_DesPara.Img          = DesImg;
		_DesPara.IsS0OnTopS1  = drv->IsSrcOnTopDes ? 0 : 1;
		_DesPara.ROPAlphaCtrl = drv->ROPAlphaCtrl;
		_DesPara.RopValue     = drv->RopValue;
		_DesPara.VReverse     = 0;

		_DesPara.Rect = &DesRect;
		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_BLEND_SRC_WITH_CONST");
		hResult = hGfx2DBLTSrc1
		(
		    hGfxDC,
		    &_Src1Para,
			1,
		    Src0DefaultColor, 
		    &_DesPara
		);
		return (hResult == HG2D_SUCCESS);
	}
	else if (drv->BLTType == ORION_GFX_BLEND_TWO_SOURCE)
	{
		HGFX_BLT_SRC_PARA  _Src0Para;
		HGFX_BLT_SRC_PARA  _Src1Para; 
		HGFX_BLT_DES_PARA  _DesPara;

		//Map: Src0Para -> SrcImg
		_Src0Para.Img = SrcImg;
		_Src0Para.ColorKeyEnable = drv->SrcColorKeyEnable;
		if (_Src0Para.ColorKeyEnable)
		{
			_Src0Para.MinColor.Red   = drv->SrcMinColor.Red;
			_Src0Para.MinColor.Green = drv->SrcMinColor.Green;
			_Src0Para.MinColor.Blue  = drv->SrcMinColor.Blue;

			_Src0Para.MaxColor.Red   = drv->SrcMaxColor.Red;
			_Src0Para.MaxColor.Green = drv->SrcMaxColor.Green;
			_Src0Para.MaxColor.Blue  = drv->SrcMaxColor.Blue;
		}
		_Src0Para.VReverse = 0;
 		_Src0Para.Rect = &SrcRect;
		//Map: Src1Para -> DesImg
		_Src1Para.Img = DesImg;
		_Src1Para.ColorKeyEnable = drv->DesColorKeyEnable;
		if (_Src1Para.ColorKeyEnable)
		{
			_Src1Para.MinColor.Red   = drv->DesMinColor.Red;
			_Src1Para.MinColor.Green = drv->DesMinColor.Green;
			_Src1Para.MinColor.Blue  = drv->DesMinColor.Blue;

			_Src1Para.MaxColor.Red   = drv->DesMaxColor.Red;
			_Src1Para.MaxColor.Green = drv->DesMaxColor.Green;
			_Src1Para.MaxColor.Blue  = drv->DesMaxColor.Blue;
		}
		_Src1Para.VReverse = 0;
		_Src1Para.Rect = &DesRect;
		
		//Map: DesPara  -> DesImg

		_DesPara.BlendEnable  = 1;
		_DesPara.Img          = DesImg;
		_DesPara.ROPAlphaCtrl = drv->ROPAlphaCtrl;
		_DesPara.RopValue     = drv->RopValue;
		_DesPara.VReverse     = 0;

 		_DesPara.Rect = &DesRect;
		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_BLEND_TWO_SOURCE");

		if (SrcImg->ColorFormat != GFX_CF_CLUT8 &&
			SrcImg->ColorFormat != GFX_CF_CLUT4 )
		{
			_DesPara.IsS0OnTopS1  = drv->IsSrcOnTopDes;
			hResult = hGfx2DBLT
			(
				 hGfxDC, 
			     &_Src0Para,
			     &_Src1Para, 
			     &_DesPara
			);
		}
		else if (DesImg->ColorFormat != GFX_CF_CLUT8 &&
			DesImg->ColorFormat != GFX_CF_CLUT4 ) 
		{
			_DesPara.IsS0OnTopS1  = drv->IsSrcOnTopDes ? 0 : 1;
			hResult = hGfx2DBLT
			(
				 hGfxDC, 
			     &_Src1Para,
			     &_Src0Para, 
			     &_DesPara
			);			
		}
		else 
		{
			ERROR_L1("##ORION_GFX: Unsupported Src and Des CLUT8 Both\n");
			return 0;
		}
		return (hResult == HG2D_SUCCESS);
	}
	else 
	{
		ERROR_D1_L1("##ORION_GFX: Error BLTType: %d\n", drv->BLTType);		
	}
	return 0;

}

bool OrionGfxStretchBlit ( void *driver_data, void *device_data,
                               DFBRectangle *srect, DFBRectangle *drect )
{
	OrionGfxDriverData *drv = (OrionGfxDriverData *)driver_data;
	OrionGfxDeviceData *dev = (OrionGfxDeviceData *)device_data;
	
	HG2D_RESULT hResult = HG2D_UNKNOW;
    GFX_DEVICE_CONTEXT *hGfxDC    = &dev->GfxDC; 
	COLOR_IMG          *SrcImg    = &drv->SrcImg;
	COLOR_IMG          *DesImg    = &drv->DesImg; 
	RECT               _SrcRect;
	I32                SrcVReverse = 0;
	RECT               _DesRect;
	I32                DesVReverse = 0;
	ROP_OPT            RopValue = ROP_R2_COPYPEN;
	U32                VInitialPhase = 0;
	U32                HInitialPhase = 0;
	
	if (drv->BLTType == ORION_GFX_SCALOR)
	{
		_SrcRect.left   = srect->x;
		_SrcRect.right  = srect->x + srect->w;
		_SrcRect.top    = srect->y;
		_SrcRect.bottom = srect->y + srect->h;
		
		_DesRect.left   = drect->x;
		_DesRect.right  = drect->x + drect->w;
		_DesRect.top    = drect->y;
		_DesRect.bottom = drect->y + drect->h;
		
		PRINT_D1_L1("##ORION_GFX: BLTType: %s\n", "ORION_GFX_SCALOR");
		hResult = hGfx2DScalor
		( 
		    hGfxDC, 
		    SrcImg, 
		    &_SrcRect, 
		    SrcVReverse,
		    DesImg, 
		    &_DesRect, 
		    DesVReverse,
		    RopValue,
		    VInitialPhase, 
		    HInitialPhase
		);

		return (hResult == HG2D_SUCCESS);
	}
	else 
	{
		ERROR_D1_L1("##ORION_GFX: Error BLTType: %d\n", drv->BLTType);		
	}
	return 0;
}


/*     bool (*TextureTriangles)( void *driver_data, void *device_data,
                               DFBVertex *vertices, int num,
                               DFBTriangleFormation formation );
*/


/* exported symbols */

static int
driver_probe( GraphicsDevice *device )
{
     switch (dfb_gfxcard_get_accelerator( device )) {
	 	  case FB_ACCEL_ORION_2DGFX:    //fbdev.h #define FB_ACCEL_ORION_2DGFX    0xFF    /* Celestialsemi Orion 2D Gfx*/        
               return 1;
     }
	 PRINT_L1("ORION_GFX: driver_probe\n");
     //return 0;
     return 1;
}

static void
driver_get_info( GraphicsDevice     *device,
                 GraphicsDriverInfo *info )
{
     /* fill driver info structure */
     snprintf( info->name,
               DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
               "Celestialsemi Orion1.3/1.3.1/1.4 2D Gfx Driver" );

     snprintf( info->vendor,
               DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
               "Celestial Semiconductor Inc." );

     info->version.major = 0;
     info->version.minor = 1;

     info->driver_data_size = sizeof (OrionGfxDriverData);
     info->device_data_size = sizeof (OrionGfxDeviceData);
	 PRINT_L1("ORION_GFX: driver_get_info\n");
}

static DFBResult
driver_init_driver( GraphicsDevice      *device,
                    GraphicsDeviceFuncs *funcs,
                    void                *driver_data,
                    void                *device_data,
                    CoreDFB             *core )
{
	PRINT_L1("ORION_GFX: driver_init_driver Start:\n");
     OrionGfxDriverData *drv = (OrionGfxDriverData*) driver_data;

     drv->mmio_base = (volatile u8*) dfb_gfxcard_map_mmio( device, 0, -1 );
     if (!drv->mmio_base)
     {
          return DFB_IO;
     }
	 else
	 {
	 	SetOrionGfxRegBaseAddr((U32)drv->mmio_base);
	 }
/*
//1. Initial the Gfx Driver Data,
*/
	D_ASSERT( driver_data != NULL);
	D_ASSERT( device_data != NULL);
	memset(driver_data, 0, sizeof(OrionGfxDriverData));
	memset(device_data, 0, sizeof(OrionGfxDeviceData));

	drv->SrcImg.ByteEndian     = BYTE_LITTLE_ENDIAN;//BYTE_LITTLE_ENDIAN;
	drv->SrcImg.NibbleEndian   = NIBBLE_BIG_ENDIAN;
	drv->SrcImg.TwoBytesEndian = TWO_BYTES_LITTLE_ENDIAN;

	drv->DesImg.ByteEndian     = drv->SrcImg.ByteEndian;
	drv->DesImg.NibbleEndian   = drv->SrcImg.NibbleEndian;
	drv->DesImg.TwoBytesEndian = drv->SrcImg.TwoBytesEndian;
	
/*
//2. Initial the Gfx Drive devide Function,
*/
     funcs->AfterSetVar       = NULL;
     funcs->EngineReset       = OrionGfxEngineReset;
     funcs->EngineSync        = OrionGfxEngineSync;
     funcs->InvalidateState   = NULL;//OrionGfxInvalidateState;
     funcs->FlushTextureCache = NULL;//OrionGfxFlushTextureCache;
     funcs->FlushReadCache    = NULL;//OrionGfxFlushReadCache;
     funcs->SurfaceEnter      = NULL;//OrionGfxSurfaceEnter;
     funcs->SurfaceLeave      = NULL;//OrionGfxSurfaceLeave;
     funcs->GetSerial         = NULL;
     funcs->WaitSerial        = NULL;
     funcs->EmitCommands      = NULL;
     funcs->CheckState        = OrionGfxCheckState;
     funcs->SetState          = OrionGfxSetState;
     funcs->FillRectangle     = OrionGfxFillRectangle;
     funcs->DrawRectangle     = NULL;//OrionGfxDrawRectangle;
     funcs->DrawLine          = NULL;
	 funcs->FillTriangle      = NULL;
     funcs->Blit              = OrionGfxBlit;
	 #if (ORION_VERSION != ORION_130)
     funcs->StretchBlit       = OrionGfxStretchBlit;
	 #else
	 funcs->StretchBlit       = NULL;
	 #endif
     funcs->TextureTriangles  = NULL;
	PRINT_L1("ORION_GFX: driver_init_driver End:\n");
     return DFB_OK;
}

static DFBResult
driver_init_device( GraphicsDevice     *device,
                    GraphicsDeviceInfo *device_info,
                    void               *driver_data,
                    void               *device_data )
{
     OrionGfxDriverData *drv     = (OrionGfxDriverData*) driver_data;
     OrionGfxDeviceData *dev     = (OrionGfxDeviceData*) device_data;

	PRINT_L1("ORION_GFX: driver_init_device Start:\n");
	 D_ASSERT( device_data != NULL);
	 memset(device_data, 0, sizeof(OrionGfxDeviceData));
     /* fill device info */
     snprintf( device_info->name,
               DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH, "Celestialsemi Orion1.3/1.3.1/1.4 2D Gfx");

     snprintf( device_info->vendor,
               DFB_GRAPHICS_DEVICE_INFO_VENDOR_LENGTH, "Celestial Semiconductor Inc." );


	/*
	//	caps.flags usage should be explpored?
	//  1. For Driver Infomation Tracing Only?
	*/
     device_info->caps.flags    = 0;//CCF_CLIPPING;
     device_info->caps.accel    = ORIONGFX_SUPPORTED_DRAWINGFUNCTIONS |
                                  ORIONGFX_SUPPORTED_BLITTINGFUNCTIONS;
     device_info->caps.drawing  = ORIONGFX_SUPPORTED_DRAWINGFLAGS;
     device_info->caps.blitting = ORIONGFX_SUPPORTED_BLITTINGFLAGS;

    /*
    // Memory Allocate Constrains
	*/
     device_info->limits.surface_byteoffset_alignment = 8;
     device_info->limits.surface_pixelpitch_alignment = 0;
	 device_info->limits.surface_bytepitch_alignment  = 8; // For Orion1.3

	//Reset Gfx Engine
	hGfx2DInit();
	 //Initial Clut8 and Scalor Coefficient
    hGfx2DSetDefaultClut8Table(&dev->GfxDC);
	hGfx2DSetDefaultScalorHFIRCoeff(&dev->GfxDC);
	hGfx2DSetDefaultScalorVFIRCoeff(&dev->GfxDC);
	PRINT_L1("ORION_GFX: driver_init_device End:\n");
     return DFB_OK;
}

static void
driver_close_device( GraphicsDevice *device,
                     void           *driver_data,
                     void           *device_data )
{
     OrionGfxDeviceData *tdev = (OrionGfxDeviceData*) device_data;
     OrionGfxDriverData *tdrv = (OrionGfxDriverData*) driver_data;

	return;
}

static void
driver_close_driver( GraphicsDevice *device,
                     void           *driver_data )
{
     OrionGfxDriverData *tdrv = (OrionGfxDriverData*) driver_data;

     dfb_gfxcard_unmap_mmio( device, tdrv->mmio_base, -1 );
}


