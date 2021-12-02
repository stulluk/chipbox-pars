/*
   (c) Copyright 2001-2007  The DirectFB Organization (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __SURFACES_H__
#define __SURFACES_H__

#include <directfb.h>

#include <direct/list.h>
#include <direct/serial.h>

#include <fusion/object.h>
#include <fusion/lock.h>
#include <fusion/reactor.h>

#include <core/coretypes.h>
#include <core/coredefs.h>

#include <core/gfxcard.h>


struct _Chunk;

typedef enum {
     CSNF_SIZEFORMAT     = 0x00000001,  /* width, height, format */
     CSNF_SYSTEM         = 0x00000002,  /* system instance information */
     CSNF_VIDEO          = 0x00000004,  /* video instance information */
     CSNF_DESTROY        = 0x00000008,  /* surface is about to be destroyed */
     CSNF_FLIP           = 0x00000010,  /* surface buffer pointer swapped */
     CSNF_FIELD          = 0x00000020,  /* active (displayed) field switched */
     CSNF_PALETTE_CHANGE = 0x00000040,  /* another palette has been set */
     CSNF_PALETTE_UPDATE = 0x00000080,  /* current palette has been altered */
     CSNF_ALPHA_RAMP     = 0x00000100   /* alpha ramp was modified */
} CoreSurfaceNotificationFlags;

typedef struct {
     CoreSurfaceNotificationFlags  flags;
     CoreSurface                  *surface;
} CoreSurfaceNotification;

typedef enum {
     CSH_INVALID         = 0x00000000,  /* surface isn't stored */
     CSH_STORED          = 0x00000001,  /* surface is stored,
                                           well and kicking */
     CSH_RESTORE         = 0x00000002   /* surface needs to be
                                           reloaded into area */
} CoreSurfaceHealth;

typedef enum {
     CSP_SYSTEMONLY      = 0x00000000,  /* never try to swap
                                           into video memory */
     CSP_VIDEOLOW        = 0x00000001,  /* try to store in video memory,
                                           low priority */
     CSP_VIDEOHIGH       = 0x00000002,  /* try to store in video memory,
                                           high priority */
     CSP_VIDEOONLY       = 0x00000003   /* always and only
                                           store in video memory */
} CoreSurfacePolicy;

typedef enum {
     CSS_NONE            = 0x00000000,  /* neither video nor auxiliary */
     CSS_VIDEO           = 0x00000001,  /* actually stored 
                                           into video memory */
     CSS_AUXILIARY       = 0x00000002,  /* actually stored 
                                           into PCI/AGP/PCIE memory */
} CoreSurfaceStorage;

typedef enum {
     SBF_NONE            = 0x00000000,
     SBF_FOREIGN_SYSTEM  = 0x00000001,  /* system memory is preallocated by
                                           application, won't be freed */
     SBF_WRITTEN         = 0x00000002,  /* buffer has been locked for writing
                                           since its birth */
     SBF_SUSPENDED       = 0x00000004   /* buffer is suspended, i.e. temp. unallocated */
} SurfaceBufferFlags;

typedef enum {
     VAF_NONE            = 0x00000000,  /* no write access happened since last
                                           clearing of all bits */
     VAF_SOFTWARE_WRITE  = 0x00000001,  /* software wrote to buffer */
     VAF_HARDWARE_WRITE  = 0x00000002,  /* hardware wrote to buffer */
     VAF_SOFTWARE_READ   = 0x00000004,  /* software read from buffer */
     VAF_HARDWARE_READ   = 0x00000008,  /* hardware read from buffer */
     VAF_SOFTWARE_LOCK   = 0x00000010   /* software access to buffer */
} VideoAccessFlags;


struct _SurfaceBuffer
{
     SurfaceBufferFlags      flags;     /* additional information */
     CoreSurfacePolicy       policy;    /* swapping policy for surfacemanager */
     CoreSurfaceStorage      storage;   /* system, video or bus memory? */

     DFBSurfacePixelFormat   format;    /* pixel format of buffer, usually that of its surface */

     struct {
          CoreSurfaceHealth  health;    /* currently stored in system memory? */
          int                locked;    /* system instance is locked,
                                           stick to it */

          int                pitch;     /* number of bytes til next line */
          void              *addr;      /* address pointing to surface data */

          int                size;      /* buffer size in bytes */
     } system;

     struct {
          CoreSurfaceHealth  health;    /* currently stored in video memory? */
          int                locked;    /* video instance is locked, don't
                                           try to kick out, could deadlock */
          
          VideoAccessFlags   access;    /* information about recent read/write
                                           accesses to video buffer memory */
          CoreGraphicsSerial serial;    /* serial of last write operation */

          int                pitch;     /* number of bytes til next line */
          int                offset;    /* byte offset from the beginning
                                           of the framebuffer */
          struct _Chunk     *chunk;     /* points to the allocated chunk */
     } video;

     CoreSurface            *surface;   /* always pointing to the surface this
                                           buffer belongs to, surfacemanger
                                           always depends on this! */
};

struct _CoreSurface
{
     FusionObject           object;

     DirectSerial           serial;        /* notification serial */

     /* size/format and instances */
     int                    width;         /* pixel width of the surface */
     int                    height;        /* pixel height of the surface */
     DFBSurfacePixelFormat  format;        /* pixel format of the surface */
     DFBSurfaceCapabilities caps;

     int                    min_width;     /* minimum allocation width */
     int                    min_height;    /* minimum allocation height */

     CorePalette           *palette;
     GlobalReaction         palette_reaction;

     int                    field;

     u8                     alpha_ramp[4];

     SurfaceBuffer         *front_buffer;  /* buffer for reading
                                              (blit from or display buffer) */

     SurfaceBuffer         *back_buffer;   /* buffer for (reading&)writing
                                              (drawing/blitting destination) */

     SurfaceBuffer         *idle_buffer;   /* triple buffering */

     SurfaceBuffer         *depth_buffer;  /* Z buffer for 3D rendering */

     SurfaceManager        *manager;

     FusionSHMPoolShared   *shmpool;       /* for structures, e.g. CoreSurface or SurfaceBuffer */
     FusionSHMPoolShared   *shmpool_data;  /* for raw data in the surface buffers */

     FusionID               owner;
};

static inline void *
dfb_surface_data_offset( const CoreSurface *surface,
                         void              *data,
                         int                pitch,
                         int                x,
                         int                y )
{
     D_ASSERT( surface != NULL );
     D_ASSERT( data != NULL );
     D_ASSERT( pitch > 0 );
     D_ASSERT( x >= 0 );
     D_ASSERT( x < surface->width );
     D_ASSERT( y >= 0 );
     D_ASSERT( y < surface->height );

     if (surface->caps & DSCAPS_SEPARATED) {
          if (y & 1)
               y += surface->height;

          y >>= 1;
     }

     return (u8*) data + pitch * y + DFB_BYTES_PER_LINE( surface->format, x );
}

static inline void
dfb_surface_caps_apply_policy( CoreSurfacePolicy       policy,
                               DFBSurfaceCapabilities *caps )
{
     switch (policy) {
          case CSP_SYSTEMONLY:
               *caps = (DFBSurfaceCapabilities)((*caps & ~DSCAPS_VIDEOONLY) | DSCAPS_SYSTEMONLY);
               break;

          case CSP_VIDEOONLY:
               *caps = (DFBSurfaceCapabilities)((*caps & ~DSCAPS_SYSTEMONLY) | DSCAPS_VIDEOONLY);
               break;

          default:
               *caps = (DFBSurfaceCapabilities)(*caps & ~(DSCAPS_SYSTEMONLY | DSCAPS_VIDEOONLY));
               break;
     }
}


/*
 * Creates a pool of surface objects.
 */
FusionObjectPool *dfb_surface_pool_create( const FusionWorld *world );

/*
 * Generates dfb_surface_ref(), dfb_surface_attach() etc.
 */
FUSION_OBJECT_METHODS( CoreSurface, dfb_surface )


/*
 * creates a surface with specified width and height in the specified
 * pixelformat using the specified swapping policy
 */
DFBResult dfb_surface_create             ( CoreDFB                 *core,
                                           int                      width,
                                           int                      height,
                                           DFBSurfacePixelFormat    format,
                                           CoreSurfacePolicy        policy,
                                           DFBSurfaceCapabilities   caps,
                                           CorePalette             *palette,
                                           CoreSurface            **surface );

/*
 * like surface_create, but with preallocated system memory that won't be
 * freed on surface destruction
 */
DFBResult dfb_surface_create_preallocated( CoreDFB                 *core,
                                           int                      width,
                                           int                      height,
                                           DFBSurfacePixelFormat    format,
                                           CoreSurfacePolicy        policy,
                                           DFBSurfaceCapabilities   caps,
                                           CorePalette             *palette,
                                           void                    *front_data,
                                           void                    *back_data,
                                           int                      front_pitch,
                                           int                      back_pitch,
                                           CoreSurface            **surface );

/*
 * initialize surface structure, not required for surface_create_*
 */
DFBResult dfb_surface_init               ( CoreDFB                *core,
                                           CoreSurface            *surface,
                                           int                     width,
                                           int                     height,
                                           DFBSurfacePixelFormat   format,
                                           DFBSurfaceCapabilities  caps,
                                           CorePalette            *palette );

/*
 * reallocates data for the specified surface
 */
DFBResult dfb_surface_reformat           ( CoreDFB               *core,
                                           CoreSurface           *surface,
                                           int                    width,
                                           int                    height,
                                           DFBSurfacePixelFormat  format );

/*
 * Change policies of buffers.
 */
DFBResult dfb_surface_reconfig           ( CoreSurface       *surface,
                                           CoreSurfacePolicy  front_policy,
                                           CoreSurfacePolicy  back_policy );

/*
 * Add/remove the depth buffer capability.
 */
DFBResult dfb_surface_allocate_depth     ( CoreSurface *surface );
void      dfb_surface_deallocate_depth   ( CoreSurface *surface );

/*
 * Change the palette of the surface.
 */
DFBResult dfb_surface_set_palette        ( CoreSurface *surface,
                                           CorePalette *palette );

/*
 * helper function
 */
DirectResult dfb_surface_notify_listeners( CoreSurface                  *surface,
                                           CoreSurfaceNotificationFlags  flags);

/*
 * really swaps front_buffer and back_buffer if they have the same policy,
 * otherwise it does a back_to_front_copy, notifies listeners
 */
void dfb_surface_flip_buffers            ( CoreSurface *surface,
                                           bool         write_front );

void dfb_surface_set_field               ( CoreSurface *surface,
                                           int          field );

void dfb_surface_set_alpha_ramp          ( CoreSurface *surface,
                                           u8           a0,
                                           u8           a1,
                                           u8           a2,
                                           u8           a3 );

/*
 * This is a utility function for easier usage.
 * It locks the surface maneger, does a surface_software_lock, and unlocks
 * the surface manager.
 */
DFBResult dfb_surface_soft_lock          ( CoreDFB              *core,
                                           CoreSurface          *surface,
                                           DFBSurfaceLockFlags   flags,
                                           void                **data,
                                           int                  *pitch,
                                           bool                  front );

/*
 * unlocks a previously locked surface
 * note that the other instance's health is CSH_RESTORE now, if it has
 * been CSH_STORED before
 */
void dfb_surface_unlock                  ( CoreSurface *surface,
                                           int          front );

/*
 * suspend/resume a surface buffer, i.e. deallocate its data meanwhile
 */
DFBResult dfb_surface_buffer_suspend( SurfaceBuffer *buffer );
DFBResult dfb_surface_buffer_resume ( SurfaceBuffer *buffer );

/*
 * Creates one or two files with the surface content (front buffer).
 *
 * The complete path will be <directory>/<prefix>_####.ppm for RGB and
 * <directory>/<prefix>_####.pgm for the alpha channel if present.
 */
DFBResult dfb_surface_dump               ( CoreDFB     *core,
                                           CoreSurface *surface,
                                           const char  *directory,
                                           const char  *prefix );

/* global reactions */
ReactionResult _dfb_surface_palette_listener( const void *msg_data,
                                              void       *ctx );

typedef enum {
     DFB_LAYER_REGION_SURFACE_LISTENER,
     DFB_WINDOWSTACK_BACKGROUND_IMAGE_LISTENER
} DFB_SURFACE_GLOBALS;

#endif

