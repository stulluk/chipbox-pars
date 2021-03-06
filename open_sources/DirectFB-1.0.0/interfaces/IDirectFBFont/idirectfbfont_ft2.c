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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include <ft2build.h>
#include FT_GLYPH_H

#include <directfb.h>

#include <core/fonts.h>
#include <core/gfxcard.h>
#include <core/surfaces.h>
#include <core/surfacemanager.h>

#include <gfx/convert.h>

#include <media/idirectfbfont.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/utf8.h>
#include <direct/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#ifndef FT_LOAD_TARGET_MONO
    /* FT_LOAD_TARGET_MONO was added in FreeType-2.1.3, we have to use (less good)
       FT_LOAD_MONOCHROME with older versions. Make it an alias for code simplicity. */
    #define FT_LOAD_TARGET_MONO FT_LOAD_MONOCHROME
#endif


static DFBResult
Probe( IDirectFBFont_ProbeContext *ctx );

static DFBResult
Construct( IDirectFBFont      *thiz,
	   ... );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBFont, FT2 )

static FT_Library      library           = NULL;
static int             library_ref_count = 0;
static pthread_mutex_t library_mutex     = PTHREAD_MUTEX_INITIALIZER;

#define KERNING_CACHE_MIN    0
#define KERNING_CACHE_MAX  127
#define KERNING_CACHE_SIZE (KERNING_CACHE_MAX - KERNING_CACHE_MIN + 1)

#define KERNING_DO_CACHE(a,b)      ((a) >= KERNING_CACHE_MIN && \
                                    (a) <= KERNING_CACHE_MAX && \
                                    (b) >= KERNING_CACHE_MIN && \
                                    (b) <= KERNING_CACHE_MAX)

#define KERNING_CACHE_ENTRY(a,b)   \
     (data->kerning[(a)-KERNING_CACHE_MIN][(b)-KERNING_CACHE_MIN])

#define CHAR_INDEX(c)    (((c) < 256) ? data->indices[c] : FT_Get_Char_Index( data->face, c ))

typedef struct {
     FT_Face      face;
     int          disable_charmap;
     int          fixed_advance;
     unsigned int indices[256];
} FT2ImplData;

typedef struct {
     signed char x;
     signed char y;
} KerningCacheEntry;

typedef struct {
     FT2ImplData base;

     KerningCacheEntry kerning[KERNING_CACHE_SIZE][KERNING_CACHE_SIZE];
} FT2ImplKerningData;

/**********************************************************************************************************************/

static DFBResult
ft2UTF8GetCharacterIndex( CoreFont     *thiz,
                          unsigned int  character,
                          unsigned int *ret_index )
{
     FT2ImplData *data = thiz->impl_data;

     D_MAGIC_ASSERT( thiz, CoreFont );

     if (data->disable_charmap)
          *ret_index = character;
     else {
          pthread_mutex_lock ( &library_mutex );

          *ret_index = CHAR_INDEX( character );

          pthread_mutex_unlock ( &library_mutex );
     }

     return DFB_OK;
}

static DFBResult
ft2UTF8DecodeText( CoreFont       *thiz,
                   const void     *text,
                   int             length,
                   unsigned int   *ret_indices,
                   int            *ret_num )
{
     int pos = 0, num = 0;
     const u8 *bytes   = text;
     FT2ImplData *data = thiz->impl_data;

     D_MAGIC_ASSERT( thiz, CoreFont );
     D_ASSERT( text != NULL );
     D_ASSERT( length >= 0 );
     D_ASSERT( ret_indices != NULL );
     D_ASSERT( ret_num != NULL );

     pthread_mutex_lock ( &library_mutex );

     while (pos < length) {
          unsigned int c;

          if (bytes[pos] < 128)
               c = bytes[pos++];
          else {
               c = DIRECT_UTF8_GET_CHAR( &bytes[pos] );
               pos += DIRECT_UTF8_SKIP(bytes[pos]);
          }

          if (data->disable_charmap)
               ret_indices[num++] = c;
          else
               ret_indices[num++] = CHAR_INDEX( c );
     }

     pthread_mutex_unlock ( &library_mutex );

     *ret_num = num;

     return DFB_OK;
}

static const CoreFontEncodingFuncs ft2UTF8Funcs = {
     GetCharacterIndex:  ft2UTF8GetCharacterIndex,
     DecodeText:         ft2UTF8DecodeText
};

/**********************************************************************************************************************/

static DFBResult
ft2Latin1GetCharacterIndex( CoreFont     *thiz,
                            unsigned int  character,
                            unsigned int *ret_index )
{
     FT2ImplData *data = thiz->impl_data;

     D_MAGIC_ASSERT( thiz, CoreFont );

     if (data->disable_charmap)
          *ret_index = character;
     else
          *ret_index = data->indices[character];

     return DFB_OK;
}

static DFBResult
ft2Latin1DecodeText( CoreFont       *thiz,
                     const void     *text,
                     int             length,
                     unsigned int   *ret_indices,
                     int            *ret_num )
{
     int i;
     const u8 *bytes   = text;
     FT2ImplData *data = thiz->impl_data;

     D_MAGIC_ASSERT( thiz, CoreFont );
     D_ASSERT( text != NULL );
     D_ASSERT( length >= 0 );
     D_ASSERT( ret_indices != NULL );
     D_ASSERT( ret_num != NULL );

     if (data->disable_charmap) {
          for (i=0; i<length; i++)
               ret_indices[i] = bytes[i];
     }
     else {
          for (i=0; i<length; i++)
               ret_indices[i] = data->indices[bytes[i]];
     }

     *ret_num = length;

     return DFB_OK;
}

static const CoreFontEncodingFuncs ft2Latin1Funcs = {
     GetCharacterIndex:  ft2Latin1GetCharacterIndex,
     DecodeText:         ft2Latin1DecodeText
};

/**********************************************************************************************************************/

static DFBResult
render_glyph( CoreFont      *thiz,
              unsigned int   index,
              CoreGlyphData *info )
{
     FT_Error     err;
     FT_Face      face;
     FT_Int       load_flags;
     u8          *src;
     void        *dst;
     int          y;
     int          pitch;
     FT2ImplData *data    = thiz->impl_data;
     CoreSurface *surface = info->surface;

     pthread_mutex_lock ( &library_mutex );

     face = data->face;

     load_flags = (FT_Int) face->generic.data;
     load_flags |= FT_LOAD_RENDER;

     if ((err = FT_Load_Glyph( face, index, load_flags ))) {
          D_HEAVYDEBUG( "DirectFB/FontFT2: "
                         "Could not render glyph for character index #%d!\n", index );
          pthread_mutex_unlock ( &library_mutex );
          return DFB_FAILURE;
     }

     pthread_mutex_unlock ( &library_mutex );

     err = dfb_surface_soft_lock( thiz->core, surface, DSLF_WRITE, &dst, &pitch, 0 );
     if (err) {
          D_ERROR( "DirectB/FontFT2: Unable to lock surface!\n" );
          return err;
     }

     info->width = face->glyph->bitmap.width;
     if (info->width + info->start > surface->width)
          info->width = surface->width - info->start;

     info->height = face->glyph->bitmap.rows;
     if (info->height > surface->height)
          info->height = surface->height;

     info->left = face->glyph->bitmap_left;
     info->top  = thiz->ascender - face->glyph->bitmap_top;

     src = face->glyph->bitmap.buffer;
     dst += DFB_BYTES_PER_LINE(surface->format, info->start);

     for (y=0; y < info->height; y++) {
          int    i, j, n;
          u8    *dst8  = dst;
          u16   *dst16 = dst;
          u32   *dst32 = dst;

          switch (face->glyph->bitmap.pixel_mode) {
               case ft_pixel_mode_grays:
                    switch (surface->format) {
                         case DSPF_ARGB:
                              if (thiz->surface_caps & DSCAPS_PREMULTIPLIED) {
                                   for (i=0; i<info->width; i++)
                                        dst32[i] = ((src[i] << 24) |
                                                    (src[i] << 16) |
                                                    (src[i] <<  8) | src[i]);
                              }
                              else
                                   for (i=0; i<info->width; i++)
                                        dst32[i] = (src[i] << 24) | 0xFFFFFF;
                              break;
                         case DSPF_AiRGB:
                              for (i=0; i<info->width; i++)
                                   dst32[i] = ((src[i] ^ 0xFF) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_ARGB4444:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (src[i] << 8) | 0xFFF;
                              break;
                         case DSPF_ARGB2554:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (src[i] << 8) | 0x3FFF;
                              break;
                         case DSPF_ARGB1555:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (src[i] << 8) | 0x7FFF;
                              break;
                         case DSPF_A8:
                              direct_memcpy( dst, src, info->width );
                              break;
                         case DSPF_A4:
                              for (i=0, j=0; i<info->width; i+=2, j++)
                                   dst8[j] = (src[i] & 0xF0) | (src[i+1] >> 4);
                              break;
                         case DSPF_A1:
                              for (i=0, j=0; i < info->width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<info->width; ++i, ++n)
                                        p |= (src[i] & 0x80) >> n;

                                   dst8[j] = p;
                              }
                              break;
                         case DSPF_LUT2:
                              for (i=0, j=0; i < info->width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<info->width; ++i, n+=2)
                                        p |= (src[i] & 0xC0) >> n;

                                   dst8[j] = p;
                              }
                              break;
                         default:
                              D_UNIMPLEMENTED();
                              break;
                    }
                    break;

               case ft_pixel_mode_mono:
                    switch (surface->format) {
                         case DSPF_ARGB:
                              for (i=0; i<info->width; i++)
                                   dst32[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0xFF : 0x00) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_AiRGB:
                              for (i=0; i<info->width; i++)
                                   dst32[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x00 : 0xFF) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_ARGB4444:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0xF : 0x0) << 12) | 0xFFF;
                              break;
                         case DSPF_ARGB2554:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x3 : 0x0) << 14) | 0x3FFF;
                              break;
                         case DSPF_ARGB1555:
                              for (i=0; i<info->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x1 : 0x0) << 15) | 0x7FFF;
                              break;
                         case DSPF_A8:
                              for (i=0; i<info->width; i++)
                                   dst8[i] = (src[i>>3] &
                                              (1<<(7-(i%8)))) ? 0xFF : 0x00;
                              break;
                         case DSPF_A4:
                              for (i=0, j=0; i<info->width; i+=2, j++)
                                   dst8[j] = ((src[i>>3] &
                                              (1<<(7-(i%8)))) ? 0xF0 : 0x00) |
                                             ((src[(i+1)>>3] &
                                              (1<<(7-((i+1)%8)))) ? 0x0F : 0x00);
                              break;
                         case DSPF_A1:
                              direct_memcpy( dst, src, DFB_BYTES_PER_LINE(DSPF_A1, info->width) );
                              break;
                         default:
                              D_UNIMPLEMENTED();
                              break;
                    }
                    break;

               default:
                    break;

          }

          src += face->glyph->bitmap.pitch;
          dst += pitch;
     }

     dfb_surface_unlock( surface, 0 );

     return DFB_OK;
}


static DFBResult
get_glyph_info( CoreFont      *thiz,
                unsigned int   index,
                CoreGlyphData *info )
{
     FT_Error err;
     FT_Face  face;
     FT_Int   load_flags;
     FT2ImplData *data = (FT2ImplData*) thiz->impl_data;

     pthread_mutex_lock ( &library_mutex );

     face = data->face;

     load_flags = (FT_Int) face->generic.data;

     if ((err = FT_Load_Glyph( face, index, load_flags ))) {
          D_HEAVYDEBUG( "DirectB/FontFT2: "
                         "Could not load glyph for character index #%d!\n", index );

          pthread_mutex_unlock ( &library_mutex );

          return DFB_FAILURE;
     }

     if (face->glyph->format != ft_glyph_format_bitmap) {
          err = FT_Render_Glyph( face->glyph,
                                 (load_flags & FT_LOAD_TARGET_MONO) ? ft_render_mode_mono : ft_render_mode_normal );
          if (err) {
               D_ERROR( "DirectFB/FontFT2: Could not "
                         "render glyph for character index #%d!\n", index );

               pthread_mutex_unlock ( &library_mutex );

               return DFB_FAILURE;
          }
     }

     pthread_mutex_unlock ( &library_mutex );

     info->width   = face->glyph->bitmap.width;
     info->height  = face->glyph->bitmap.rows;
     info->advance = data->fixed_advance ?
                     data->fixed_advance : (face->glyph->advance.x >> 6);

     return DFB_OK;
}


static DFBResult
get_kerning( CoreFont     *thiz,
             unsigned int  prev,
             unsigned int  current,
             int          *kern_x,
             int          *kern_y)
{
     FT_Vector vector;

     FT2ImplKerningData *data = thiz->impl_data;
     KerningCacheEntry *cache = NULL;

     D_ASSUME( (kern_x != NULL) || (kern_y != NULL) );

     /*
      * Use cached values if characters are in the
      * cachable range and the cache entry is already filled.
      */
     if (KERNING_DO_CACHE (prev, current)) {
          cache = &KERNING_CACHE_ENTRY (prev, current);

          if (kern_x)
               *kern_x = (int) cache->x;

          if (kern_y)
               *kern_y = (int) cache->y;

          return DFB_OK;
     }

     pthread_mutex_lock ( &library_mutex );

     /* Lookup kerning values for the character pair. */
     FT_Get_Kerning( data->base.face,
                     prev, current, ft_kerning_default, &vector );

     pthread_mutex_unlock ( &library_mutex );

     /* Convert to integer. */
     if (kern_x)
          *kern_x = vector.x >> 6;

     if (kern_y)
          *kern_y = vector.y >> 6;

     return DFB_OK;
}

static void
init_kerning_cache( FT2ImplKerningData *data )
{
     int a, b;

     pthread_mutex_lock ( &library_mutex );

     for (a=KERNING_CACHE_MIN; a<=KERNING_CACHE_MAX; a++) {
          for (b=KERNING_CACHE_MIN; b<=KERNING_CACHE_MAX; b++) {
               FT_Vector          vector;
               KerningCacheEntry *cache = &KERNING_CACHE_ENTRY( a, b );

               /* Lookup kerning values for the character pair. */
               FT_Get_Kerning( data->base.face,
                               a, b, ft_kerning_default, &vector );

               cache->x = (signed char) (vector.x >> 6);
               cache->y = (signed char) (vector.y >> 6);
          }
     }

     pthread_mutex_unlock ( &library_mutex );
}

static DFBResult
init_freetype( void )
{
     FT_Error err;

     pthread_mutex_lock ( &library_mutex );

     if (!library) {
          D_HEAVYDEBUG( "DirectFB/FontFT2: "
                         "Initializing the FreeType2 library.\n" );
          err = FT_Init_FreeType( &library );
          if (err) {
               D_ERROR( "DirectFB/FontFT2: "
                         "Initialization of the FreeType2 library failed!\n" );
               library = NULL;
               pthread_mutex_unlock( &library_mutex );
               return DFB_FAILURE;
          }
     }

     library_ref_count++;
     pthread_mutex_unlock( &library_mutex );

     return DFB_OK;
}


static void
release_freetype( void )
{
     pthread_mutex_lock( &library_mutex );

     if (library && --library_ref_count == 0) {
          D_HEAVYDEBUG( "DirectFB/FontFT2: "
                         "Releasing the FreeType2 library.\n" );
          FT_Done_FreeType( library );
          library = NULL;
     }

     pthread_mutex_unlock( &library_mutex );
}


static void
IDirectFBFont_FT2_Destruct( IDirectFBFont *thiz )
{
     IDirectFBFont_data *data = (IDirectFBFont_data*)thiz->priv;

     if (data->font->impl_data) {
          FT2ImplData *impl_data = (FT2ImplData*) data->font->impl_data;

          pthread_mutex_lock ( &library_mutex );
          FT_Done_Face( impl_data->face );
          pthread_mutex_unlock ( &library_mutex );

          D_FREE( impl_data );

          data->font->impl_data = NULL;
     }

     IDirectFBFont_Destruct( thiz );

     release_freetype();
}


static DFBResult
IDirectFBFont_FT2_Release( IDirectFBFont *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBFont)

     if (--data->ref == 0) {
          IDirectFBFont_FT2_Destruct( thiz );
     }

     return DFB_OK;
}


static DFBResult
Probe( IDirectFBFont_ProbeContext *ctx )
{
     FT_Error err;
     FT_Face  face;

     D_HEAVYDEBUG( "DirectFB/FontFT2: Probe font `%s'.\n", ctx->filename );

     if (!ctx->filename)
          return DFB_UNSUPPORTED;

     if (init_freetype() != DFB_OK) {
          return DFB_FAILURE;
     }

     pthread_mutex_lock ( &library_mutex );
     /*
      * This should be
      * err = FT_New_Face( library, ctx->filename, -1, NULL );
      * but due to freetype bugs it doesn't work.
      */
     err = FT_New_Face( library, ctx->filename, 0, &face );
     if (!err)
          FT_Done_Face( face );
     pthread_mutex_unlock ( &library_mutex );

     release_freetype();

     return err ? DFB_UNSUPPORTED : DFB_OK;
}


static DFBResult
Construct( IDirectFBFont      *thiz,
	   ... )
{
     int                    i;
     DFBResult              ret;
     CoreFont              *font;
     FT_Face                face;
     FT_Error               err;
     FT_Int                 load_flags = FT_LOAD_DEFAULT;
     FT2ImplData           *data;
     bool                   disable_charmap = false;
     bool                   disable_kerning = false;
     bool                   load_mono = false;
     u32                    mask = 0;

     CoreDFB *core;
     char *filename;
     DFBFontDescription *desc;

     va_list tag;
     va_start(tag, thiz);
     core = va_arg(tag, CoreDFB *);
     filename = va_arg(tag, char *);
     desc = va_arg(tag, DFBFontDescription *);
     va_end( tag );

     D_HEAVYDEBUG( "DirectFB/FontFT2: "
                    "Construct font from file `%s' (index %d) at pixel size %d x %d.\n",
                    filename,
                    (desc->flags & DFDESC_INDEX)  ? desc->index  : 0,
                    (desc->flags & DFDESC_WIDTH)  ? desc->width  : 0,
                    (desc->flags & DFDESC_HEIGHT) ? desc->height : 0 );

     if (init_freetype() != DFB_OK) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return DFB_FAILURE;
     }

     pthread_mutex_lock ( &library_mutex );
     err = FT_New_Face( library, filename,
                        (desc->flags & DFDESC_INDEX) ? desc->index : 0,
                        &face );
     pthread_mutex_unlock ( &library_mutex );
     if (err) {
          switch (err) {
               case FT_Err_Unknown_File_Format:
                    D_ERROR( "DirectFB/FontFT2: "
                              "Unsupported font format in file `%s'!\n", filename );
                    break;
               default:
                    D_ERROR( "DirectFB/FontFT2: "
                              "Failed loading face %d from font file `%s'!\n",
                              (desc->flags & DFDESC_INDEX) ? desc->index : 0,
                              filename );
                    break;
          }
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return DFB_FAILURE;
     }

     if (dfb_config->font_format == DSPF_A1 || dfb_config->font_format == DSPF_ARGB1555)
          load_mono = true;

     if (desc->flags & DFDESC_ATTRIBUTES) {
          if (desc->attributes & DFFA_NOHINTING)
               load_flags |= FT_LOAD_NO_HINTING;
          if (desc->attributes & DFFA_NOCHARMAP)
               disable_charmap = true;
          if (desc->attributes & DFFA_NOKERNING)
               disable_kerning = true;
          if (desc->attributes & DFFA_MONOCHROME)
               load_mono = true;
     }

     if (load_mono)
          load_flags |= FT_LOAD_TARGET_MONO;

     if (!disable_charmap) {
          pthread_mutex_lock ( &library_mutex );
          err = FT_Select_Charmap( face, ft_encoding_unicode );
          pthread_mutex_unlock ( &library_mutex );

#if FREETYPE_MINOR > 0

          /* ft_encoding_latin_1 has been introduced in freetype-2.1 */
          if (err) {
               D_HEAVYDEBUG( "DirectFB/FontFT2: "
                              "Couldn't select Unicode encoding, "
                              "falling back to Latin1.\n");
               pthread_mutex_lock ( &library_mutex );
               err = FT_Select_Charmap( face, ft_encoding_latin_1 );
               pthread_mutex_unlock ( &library_mutex );
          }
#endif
          if (err) {
               D_HEAVYDEBUG( "DirectFB/FontFT2: "
                              "Couldn't select Unicode/Latin1 encoding, "
                              "trying Symbol.\n");
               pthread_mutex_lock ( &library_mutex );
               err = FT_Select_Charmap( face, ft_encoding_symbol );
               pthread_mutex_unlock ( &library_mutex );

               if (!err)
                    mask = 0xf000;
          }
     }

#if 0
     if (err) {
          D_ERROR( "DirectFB/FontFT2: "
                    "Couldn't select a suitable encoding for face %d from font file `%s'!\n", (desc->flags & DFDESC_INDEX) ? desc->index : 0, filename );
          pthread_mutex_lock ( &library_mutex );
          FT_Done_Face( face );
          pthread_mutex_unlock ( &library_mutex );
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return DFB_FAILURE;
     }
#endif

     if (desc->flags & (DFDESC_HEIGHT       | DFDESC_WIDTH |
                        DFDESC_FRACT_HEIGHT | DFDESC_FRACT_WIDTH))
     {
          int fw = 0, fh = 0;

          if (desc->flags & DFDESC_FRACT_HEIGHT)
               fh = desc->fract_height;
          else if (desc->flags & DFDESC_HEIGHT)
               fh = desc->height << 6;

          if (desc->flags & DFDESC_FRACT_WIDTH)
               fw = desc->fract_width;
          else if (desc->flags & DFDESC_WIDTH)
               fw = desc->width << 6;

          pthread_mutex_lock ( &library_mutex );
          err = FT_Set_Char_Size( face, fw, fh, 0, 0 );
          pthread_mutex_unlock ( &library_mutex );
          if (err) {
               D_ERROR( "DirectB/FontFT2: "
                         "Could not set pixel size to %d x %d!\n",
                         (desc->flags & DFDESC_WIDTH)  ? desc->width  : 0,
                         (desc->flags & DFDESC_HEIGHT) ? desc->height : 0 );
               pthread_mutex_lock ( &library_mutex );
               FT_Done_Face( face );
               pthread_mutex_unlock ( &library_mutex );
               DIRECT_DEALLOCATE_INTERFACE( thiz );
               return DFB_FAILURE;
          }
     }

     face->generic.data = (void *) load_flags;
     face->generic.finalizer = NULL;

     ret = dfb_font_create( core, &font );
     if (ret) {
          pthread_mutex_lock ( &library_mutex );
          FT_Done_Face( face );
          pthread_mutex_unlock ( &library_mutex );
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     D_ASSERT( font->pixel_format == DSPF_ARGB ||
               font->pixel_format == DSPF_AiRGB ||
               font->pixel_format == DSPF_ARGB4444 ||
               font->pixel_format == DSPF_ARGB2554 ||
               font->pixel_format == DSPF_ARGB1555 ||
               font->pixel_format == DSPF_A8 ||
               font->pixel_format == DSPF_A4 ||
               font->pixel_format == DSPF_A1 );

     font->ascender   = face->size->metrics.ascender >> 6;
     font->descender  = face->size->metrics.descender >> 6;
     font->height     = font->ascender + ABS(font->descender) + 1;
     font->maxadvance = face->size->metrics.max_advance >> 6;

     D_HEAVYDEBUG( "DirectFB/FontFT2: font->height = %d\n", font->height );
     D_HEAVYDEBUG( "DirectFB/FontFT2: font->ascender = %d\n", font->ascender );
     D_HEAVYDEBUG( "DirectFB/FontFT2: font->descender = %d\n",font->descender );

     font->GetGlyphData = get_glyph_info;
     font->RenderGlyph  = render_glyph;

     if (FT_HAS_KERNING(face) && !disable_kerning) {
          font->GetKerning = get_kerning;
          data = D_CALLOC( 1, sizeof(FT2ImplKerningData) );
     }
     else
          data = D_CALLOC( 1, sizeof(FT2ImplData) );

     data->face            = face;
     data->disable_charmap = disable_charmap;

     if (FT_HAS_KERNING(face) && !disable_kerning)
          init_kerning_cache( (FT2ImplKerningData*) data );

     if (desc->flags & DFDESC_FIXEDADVANCE) {
          data->fixed_advance = desc->fixed_advance;
          font->maxadvance    = desc->fixed_advance;
     }

     for (i=0; i<256; i++)
          data->indices[i] = FT_Get_Char_Index( face, i | mask );

     font->impl_data = data;

     dfb_font_register_encoding( font, "UTF8",   &ft2UTF8Funcs,   DTEID_UTF8 );
     dfb_font_register_encoding( font, "Latin1", &ft2Latin1Funcs, DTEID_OTHER );

     IDirectFBFont_Construct( thiz, font );

     thiz->Release = IDirectFBFont_FT2_Release;

     return DFB_OK;
}
