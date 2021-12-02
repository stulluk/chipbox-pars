#ifndef _ORION_GFX_H_
#define _ORION_GFX_H_

#include <dfb_types.h>
#include "orion_gfx_2dlib.h"
#include "orion_gfx_sys_cfg.h"

typedef enum _ORION_GFX_BLT_TYPE_
{
	ORION_GFX_NO_OPT     = -1,
	ORION_GFX_FILL_ONLY  = 0,
	ORION_GFX_COPY_ONLY  = 1,
	ORION_GFX_BLEND_DES_WITH_CONST = 2, //Const Color Store in SrcImg
	ORION_GFX_BLEND_SRC_WITH_CONST = 3, //Const Color Store in DesImg
	ORION_GFX_BLEND_TWO_SOURCE = 4,
	ORION_GFX_SCALOR = 5,
} OrionGfxBLTType;

typedef struct _ORION_GFX_DRIVER_DATA_
{
	//Register Bank Base
	volatile u8   *mmio_base;
    //Source Para 
	COLOR_IMG SrcImg; 
	I32       SrcVReverse;
	I32       SrcColorKeyEnable;
	GFX_ARGB_COLOR SrcMinColor;
	GFX_ARGB_COLOR SrcMaxColor;	
	//Des Para
    COLOR_IMG  DesImg;
	I32        DesColorKeyEnable;
	GFX_ARGB_COLOR DesMinColor;
	GFX_ARGB_COLOR DesMaxColor;	
	
	I32        DesVReverse;  
	//BLT Control
	OrionGfxBLTType BLTType;
	I32        IsSrcOnTopDes;//IsS0OnTopS1;// 1 S0 On Top S1
	U32        ROPAlphaCtrl;
	ROP_OPT    RopValue;

}OrionGfxDriverData;


typedef struct _ORION_GFX_DEVICE_DATA_
{
	GFX_DEVICE_CONTEXT GfxDC;
	
}OrionGfxDeviceData;

/*
      * Called after driver->InitDevice() and during dfb_gfxcard_unlock( true ).
      * The driver should do the one time initialization of the engine,
      * e.g. writing some registers that are supposed to have a fixed value.
      *
      * This happens after mode switching or after returning from
      * OpenGL state (e.g. DRI driver).
      */
	void OrionGfxEngineReset( void *driver_data, void *device_data );

/*
      * Makes sure that graphics hardware has finished all operations.
      *
      * This method is called before the CPU accesses a surface' buffer
      * that had been written to by the hardware after this method has been
      * called the last time.
      *
      * It's also called before entering the OpenGL state (e.g. DRI driver).
      */
     DFBResult OrionGfxEngineSync( void *driver_data, void *device_data );

     /*
      * Called during dfb_gfxcard_lock() to notify the driver that
      * the current rendering state is no longer valid.
      */
     void OrionGfxInvalidateState( void *driver_data, void *device_data );

	/*
      * after the video memory has been written to by the CPU (e.g. modification
      * of a texture) make sure the accelerator won't use cached texture data
      */
     void OrionGfxFlushTextureCache( void *driver_data, void *device_data );

     /*
      * After the video memory has been written to by the accelerator
      * make sure the CPU won't read back cached data.
      */
     void OrionGfxFlushReadCache( void *driver_data, void *device_data );

     /*
      * Called before a software access to a video surface buffer.
      */
     void OrionGfxSurfaceEnter( void *driver_data, void *device_data,
                           SurfaceBuffer *buffer, DFBSurfaceLockFlags flags );

     /*
      * Called after a software access to a video surface buffer.
      */
     void OrionGfxSurfaceLeave( void *driver_data, void *device_data, SurfaceBuffer *buffer );


	/*
      * Check if the function 'accel' can be accelerated with the 'state'.
      * If that's true, the function sets the 'accel' bit in 'state->accel'.
      * Otherwise the function just returns, no need to clear the bit.
      */
     void OrionGfxCheckState( void *driver_data, void *device_data,
                         CardState *state, DFBAccelerationMask accel );

     /*
      * Program card for execution of the function 'accel' with the 'state'.
      * 'state->modified' contains information about changed entries.
      * This function has to set at least 'accel' in 'state->set'.
      * The driver should remember 'state->modified' and clear it.
      * The driver may modify 'funcs' depending on 'state' settings.
      */
     void OrionGfxSetState  ( void *driver_data, void *device_data,
                         struct _GraphicsDeviceFuncs *funcs,
                         CardState *state, DFBAccelerationMask accel );

     /*
      * drawing functions
      */
     bool OrionGfxFillRectangle ( void *driver_data, void *device_data,
                             DFBRectangle *rect );

     bool OrionGfxDrawRectangle ( void *driver_data, void *device_data,
                             DFBRectangle *rect );

/*
     bool (*DrawLine)      ( void *driver_data, void *device_data,
                             DFBRegion *line );

     bool (*FillTriangle)  ( void *driver_data, void *device_data,
                             DFBTriangle *tri );
*/
     /*
      * blitting functions
      */
     bool OrionGfxBlit            ( void *driver_data, void *device_data,
                               DFBRectangle *rect, int dx, int dy );

     bool OrionGfxStretchBlit ( void *driver_data, void *device_data,
                               DFBRectangle *srect, DFBRectangle *drect );

/*     bool (*TextureTriangles)( void *driver_data, void *device_data,
                               DFBVertex *vertices, int num,
                               DFBTriangleFormation formation );
*/
#endif

