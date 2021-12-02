/*
** $Id: freetype.c,v 1.26 2003/09/25 04:08:54 snig Exp $
** 
** freetype.c: TrueType font support based on FreeType 1.3.1.
** 
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: WEI Yongming.
**
** Create date: 2000/08/21
*/

/*
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Library General Public
**  License as published by the Free Software Foundation; either
**  version 2 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Library General Public License for more details.
**
**  You should have received a copy of the GNU Library General Public
**  License along with this library; if not, write to the Free
**  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
**  MA 02111-1307, USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "devfont.h"
#include "charset.h"
#include "fontname.h"
#include "misc.h"

#if defined(_TTF_SUPPORT) && !defined(_HAS_FREETYPE2)

#include <freetype1/freetype/freetype.h>
#include <freetype1/freetype/ftxkern.h>
#include <freetype1/freetype/ftnameid.h>
#include <freetype1/freetype/ftxcmap.h>
#include <freetype1/freetype/ftxwidth.h>

#include "freetype.h"

#if TT_FREETYPE_MAJOR != 1 | TT_FREETYPE_MINOR < 3
    #error "You must link with freetype lib version 1.3.x +, and not freetype 2. \
        You can download it at http://www.freetype.org"
#endif

/******************************* Global data ********************************/
static TT_Engine ttf_engine;        /* The ONLY freetype engine */
static BYTE virtual_palette [] = {0, 32, 64, 128, 255};

/************************ Create/Destroy FreeType font ***********************/
static BOOL CreateFreeTypeFont (const char *name, TTFGLYPHINFO* ttf_glyph_info)
{
    unsigned short      i, n;
    unsigned short      platform, encoding;
    TT_Face_Properties  properties;

    /* Load face */
    if (TT_Open_Face (ttf_engine, name, &ttf_glyph_info->face) != TT_Err_Ok)
        return FALSE;

    /* Load first kerning table */
    ttf_glyph_info->can_kern = TRUE;
    if (TT_Load_Kerning_Table (ttf_glyph_info->face, 0) != TT_Err_Ok)
        ttf_glyph_info->can_kern = FALSE;
    else {
        if (TT_Get_Kerning_Directory (ttf_glyph_info->face, &ttf_glyph_info->directory)
            != TT_Err_Ok)
            ttf_glyph_info->can_kern = FALSE;
        else {
            /* Support only version 0 kerning table ... */
            if ((ttf_glyph_info->directory.version != 0) ||
                (ttf_glyph_info->directory.nTables <= 0) ||
                (ttf_glyph_info->directory.tables->loaded != 1) ||
                (ttf_glyph_info->directory.tables->version != 0) ||
                (ttf_glyph_info->directory.tables->t.kern0.nPairs <= 0))
                    ttf_glyph_info->can_kern = FALSE;
        }
    }

#if 0
    fprintf (stderr, "Font %s: can kern? %c\n", name, ttf_glyph_info->can_kern ? 'y' : 'n');
#endif

    /* Get face properties and allocate preload arrays */
    TT_Get_Face_Properties (ttf_glyph_info->face, &properties);

    /* Create a glyph container */
    if (TT_New_Glyph (ttf_glyph_info->face, &ttf_glyph_info->glyph) != TT_Err_Ok)
        return FALSE;

    /* Look for a Unicode charmap: Windows flavor of Apple flavor only */
    n = properties.num_CharMaps;

    for (i = 0; i < n; i++) {
        TT_Get_CharMap_ID (ttf_glyph_info->face, i, &platform, &encoding);
        if (((platform == TT_PLATFORM_MICROSOFT) &&
            (encoding == TT_MS_ID_UNICODE_CS)) ||
                ((platform == TT_PLATFORM_APPLE_UNICODE) &&
                     (encoding == TT_APPLE_ID_DEFAULT)))
        {
            TT_Get_CharMap (ttf_glyph_info->face, i, &ttf_glyph_info->char_map);
            i = n + 1;
        }
    }

    if (i == n) {
        fprintf (stderr, "GDI (When Create TTF Font): no unicode map table\n");
        return FALSE;
    }
    
    ttf_glyph_info->first_char = TT_CharMap_First (ttf_glyph_info->char_map, NULL);
    ttf_glyph_info->last_char = TT_CharMap_Last (ttf_glyph_info->char_map, NULL);
    ttf_glyph_info->last_glyph_index
            = (properties.num_Glyphs > 255) ? 255 : properties.num_Glyphs - 1;

    return TRUE;
}

static void DestroyFreeTypeFont (TTFGLYPHINFO* ttf_glyph_info)
{
    if (ttf_glyph_info->valid) {
        TT_Done_Glyph (ttf_glyph_info->glyph);
        TT_Close_Face (ttf_glyph_info->face);
        ttf_glyph_info->valid = FALSE;
    }
}

/************************ Alloc/Free raster bitmap buffer ********************/
static BYTE* rb_buffer;
static size_t rb_buf_size;

static BYTE* get_raster_bitmap_buffer (size_t size)
{
    if (size <= rb_buf_size) return rb_buffer;

    rb_buf_size = ((size + 31) >> 5) << 5;
#if 0
    fprintf (stderr, "buf_size: %d.\n", buf_size);
#endif
    rb_buffer = realloc (rb_buffer, rb_buf_size);

    return rb_buffer;
}

static void free_raster_bitmap_buffer (void)
{
    free (rb_buffer);
    rb_buffer = NULL;
    rb_buf_size = 0;
}

/************************ Init/Term of FreeType fonts ************************/
static int nr_fonts;
static TTFGLYPHINFO* ttf_glyph_infos;
static DEVFONT* ttf_dev_fonts;

#define SECTION_NAME    "truetypefonts"

BOOL InitFreeTypeFonts (void)
{
    int i;
    char font_name [LEN_UNIDEVFONT_NAME + 1];

    /* Does load TrueType fonts? */
    if (GetMgEtcIntValue (SECTION_NAME, "font_number",
                &nr_fonts) < 0 )
        return FALSE;

    if ( nr_fonts < 1) return TRUE;

    /* Alloc space for devfont and ttfinfo. */
    ttf_glyph_infos = calloc (nr_fonts, sizeof (TTFGLYPHINFO));
    ttf_dev_fonts = calloc (nr_fonts, sizeof (DEVFONT));
    if (ttf_glyph_infos == NULL || ttf_dev_fonts == NULL) {
        goto error_alloc;
    }

    for (i = 0; i < nr_fonts; i++)
        ttf_glyph_infos [i].valid = FALSE;

    /* Init freetype library */
    if (TT_Init_FreeType (&ttf_engine) != TT_Err_Ok) {
        goto error_alloc;
    }

    TT_Set_Raster_Gray_Palette (ttf_engine, virtual_palette);

    /* Init kerning extension */
    if (TT_Init_Kerning_Extension (ttf_engine) != TT_Err_Ok) {
        TT_Done_FreeType (ttf_engine);
        goto error_alloc;
    }

    for (i = 0; i < nr_fonts; i++) {
        char key [11];
        char charset [LEN_FONT_NAME + 1];
        char file [MAX_PATH + 1];
        CHARSETOPS* charset_ops;

        sprintf (key, "name%d", i);
        if (GetMgEtcValue (SECTION_NAME, key,
                           font_name, LEN_UNIDEVFONT_NAME) < 0 )
            goto error_load;

        if (!fontGetCharsetFromName (font_name, charset)) {
            fprintf (stderr, "GDI: Invalid font name (charset): %s.\n",
                    font_name);
            goto error_load;
        }

        if ((charset_ops = GetCharsetOps (charset)) == NULL) {
            fprintf (stderr, "GDI: Not supported charset: %s.\n", charset);
            goto error_load;
        }

        sprintf (key, "fontfile%d", i);
        if (GetMgEtcValue (SECTION_NAME, key, file, MAX_PATH) < 0)
            goto error_load;

        if (!CreateFreeTypeFont (file, ttf_glyph_infos + i))
            goto error_load;

        strncpy (ttf_dev_fonts[i].name, font_name, LEN_UNIDEVFONT_NAME);
        ttf_dev_fonts[i].name [LEN_UNIDEVFONT_NAME] = '\0';
        ttf_dev_fonts[i].font_ops = &freetype_font_ops;
        ttf_dev_fonts[i].charset_ops = charset_ops;
        ttf_dev_fonts[i].data = ttf_glyph_infos + i;
#if 0
        fprintf (stderr, "GDI: TTFDevFont %i: %s.\n", i, ttf_dev_fonts[i].name);
#endif

        ttf_glyph_infos [i].valid = TRUE;
    }

    for (i = 0; i < nr_fonts; i++) {
        int nr_charsets;
        char charsets [LEN_UNIDEVFONT_NAME + 1];

        if (ttf_dev_fonts [i].charset_ops->bytes_maxlen_char > 1) {
            AddMBDevFont (ttf_dev_fonts + i);
        }
        else
            AddSBDevFont (ttf_dev_fonts + i);

        fontGetCharsetPartFromName (ttf_dev_fonts[i].name, charsets);
        if ((nr_charsets = charsetGetCharsetsNumber (charsets)) > 1) {

            int j;
            for (j = 1; j < nr_charsets; j++) {
                char charset [LEN_FONT_NAME + 1];
                CHARSETOPS* charset_ops;
                DEVFONT* new_devfont;

                charsetGetSpecificCharset (charsets, j, charset);
                if ((charset_ops = GetCharsetOps (charset)) == NULL)
                    continue;

                new_devfont = calloc (1, sizeof (DEVFONT));
                memcpy (new_devfont, ttf_dev_fonts + i, sizeof (DEVFONT));
                new_devfont->charset_ops = charset_ops;
                if (new_devfont->charset_ops->bytes_maxlen_char > 1)
                    AddMBDevFont (new_devfont);
                else
                    AddSBDevFont (new_devfont);
            }
        }
    }

    return TRUE;

error_load:
    fprintf (stderr, "GDI: Error in loading TrueType fonts\n");
    for (i = 0; i < nr_fonts; i++)
        DestroyFreeTypeFont (ttf_glyph_infos + i);

error_alloc:
    free (ttf_glyph_infos);
    free (ttf_dev_fonts);
    ttf_glyph_infos = NULL;
    ttf_dev_fonts = NULL;
    TT_Done_FreeType (ttf_engine);
    return FALSE;
}

void TermFreeTypeFonts (void)
{
    if (ttf_glyph_infos) {
        TT_Done_FreeType (ttf_engine);
        ttf_glyph_infos = NULL;
    }
    
    free_raster_bitmap_buffer ();
}

/*************** TrueType on FreeType font operations ************************/
static TT_UShort 
Get_Glyph_Width (TTFINSTANCEINFO* ttf_inst_info, TT_UShort glyph_index)
{
    TT_F26Dot6  xmin, xmax;
    TT_Outline  outline;
    TT_BBox     bbox;
    TTFGLYPHINFO* ttf_glyph_info = ttf_inst_info->ttf_glyph_info;

    if (TT_Load_Glyph (ttf_inst_info->instance, ttf_glyph_info->glyph, glyph_index,
                TTLOAD_DEFAULT) != TT_Err_Ok)
    {
        /* Try to load default glyph: index 0 */
        if (TT_Load_Glyph (ttf_inst_info->instance, ttf_glyph_info->glyph, 0,
                    TTLOAD_DEFAULT) != TT_Err_Ok)
        {
            return 0;
        }
    }

    TT_Get_Glyph_Outline (ttf_glyph_info->glyph, &outline);
    TT_Get_Outline_BBox (&outline, &bbox);

    xmin = (bbox.xMin & -64) >> 6;
    xmax = ((bbox.xMax + 63) & -64) >> 6;

    return (xmax - xmin);
}

static int get_char_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    TT_UShort       index;
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);

#if 0	//old version, changed by xm.chen
    index = TT_Char_Index (ttf_inst_info->ttf_glyph_info->char_map, 
            (*devfont->charset_ops->conv_to_uc16) (mchar, len));
#else
    index = TT_Char_Index (ttf_inst_info->ttf_glyph_info->char_map, 
            (*devfont->charset_ops->conv_to_uc32) (mchar));
#endif
    if (index < 256)
        return ttf_inst_info->widths [index];
    return ttf_inst_info->ave_width;
}

static int get_max_width (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return ttf_inst_info->max_width;
}

static int
compute_kernval (TTFINSTANCEINFO* ttf_inst_info)
{
    int i = 0;
    int kernval;
    TTFGLYPHINFO* ttf_glyph_info = ttf_inst_info->ttf_glyph_info;
    int nPairs = ttf_glyph_info->directory.tables->t.kern0.nPairs;
    TT_Kern_0_Pair *pair = ttf_glyph_info->directory.tables->t.kern0.pairs;

    if (ttf_inst_info->last_glyph_code != -1) {
        while ((pair->left != ttf_inst_info->last_glyph_code)
            && (pair->right != ttf_inst_info->cur_glyph_code))
        {
            pair++;
            i++;
            if (i == nPairs)
                break;
        }

        if (i == nPairs)
            kernval = 0;
        else
            /* We round the value (hence the +32) */
            kernval = (pair->value + 32) & -64;
    } else
        kernval = 0;

    return kernval;
}

static int get_str_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mstr, int n, int cExtra)
{
    unsigned short uni_char;
    TT_F26Dot6  x = 0;
    TT_UShort   index;
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    TTFGLYPHINFO* ttf_glyph_info = ttf_inst_info->ttf_glyph_info;
#if 1
    TT_Glyph_Metrics metrics;
#endif

    ttf_inst_info->last_glyph_code = -1;       /* reset kerning*/
    ttf_inst_info->last_pen_pos = -1;
    while (n > 0) {
        int char_len = (*devfont->charset_ops->len_first_char) (mstr, n);
#if 0	//old version, changed by xm.chen
        uni_char = (*devfont->charset_ops->conv_to_uc16) (mstr, char_len);
#else
        uni_char = (*devfont->charset_ops->conv_to_uc32) (mstr);
#endif
        n -= char_len;
        mstr += char_len;

        index = TT_Char_Index (ttf_glyph_info->char_map, uni_char);

        if (index < 256)
            x += ttf_inst_info->widths [index];
        else {
#if 1
            if (TT_Load_Glyph (ttf_inst_info->instance, ttf_glyph_info->glyph, index,
                    TTLOAD_DEFAULT) != TT_Err_Ok)
                continue;

            TT_Get_Glyph_Metrics (ttf_glyph_info->glyph, &metrics);

            x += metrics.advance / 64;
#else
            x += ttf_inst_info->ave_width;
#endif
        }

        if ((logfont->style & FS_SETWIDTH_CONDENSED) && ttf_glyph_info->can_kern) {
                x += compute_kernval (ttf_inst_info) / 64;
        }

        /* Kerning point syndrome avoidance */
        if (ttf_inst_info->last_pen_pos > x)
            x = ttf_inst_info->last_pen_pos;
        ttf_inst_info->last_pen_pos = x;

        ttf_inst_info->last_glyph_code = index;

        x += cExtra;
    }

    return x;
}

static int get_ave_width (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return ttf_inst_info->ave_width;
}

static int get_font_height (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return ttf_inst_info->height;
}

static int get_font_size (LOGFONT* logfont, DEVFONT* devfont, int expect)
{
    return expect;
}

static int get_font_ascent (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return ttf_inst_info->ascent;
}

static int get_font_descent (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return -ttf_inst_info->descent;
}

static size_t char_bitmap_size (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    int char_width;
    TT_UShort index;
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    TTFGLYPHINFO* ttf_glyph_info = ttf_inst_info->ttf_glyph_info;

#if 0	//old version, changed by xm.chen
    index = TT_Char_Index (ttf_glyph_info->char_map, 
            (*devfont->charset_ops->conv_to_uc16) (mchar, len));
#else
    index = TT_Char_Index (ttf_inst_info->ttf_glyph_info->char_map, 
            (*devfont->charset_ops->conv_to_uc32) (mchar));
#endif

    if (index < 256)
        char_width = ttf_inst_info->widths [index];
    else
        char_width = ttf_inst_info->ave_width;

    return ((char_width + 7) >> 3) * ttf_inst_info->height;
}

static size_t max_bitmap_size (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    return ((ttf_inst_info->max_width + 7) >> 3) * ttf_inst_info->height;
}

/* call this function before output a string */
static void start_str_output (LOGFONT* logfont, DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);

    ttf_inst_info->last_glyph_code = -1;       /* reset kerning*/
    ttf_inst_info->last_pen_pos = -1;
}

/* call this function before getting the bitmap/pixmap of the char
 * to get the bbox of the char */
static int get_char_bbox (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, 
                int* px, int* py, int* pwidth, int* pheight)
{
    TT_UShort uni_char;
    TT_F26Dot6 x = 0, y = 0;
    TT_F26Dot6  xmin, ymin, xmax, ymax;
    TT_Pos vec_x, vec_y;
    TT_Glyph_Metrics metrics;
    TT_BBox     bbox;

    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    TTFGLYPHINFO* ttf_glyph_info = ttf_inst_info->ttf_glyph_info;

    if (px) x = *px; if (py) y = *py;

#if 0	//old version, changed by xm.chen
    uni_char = (*devfont->charset_ops->conv_to_uc16) (mchar, len);
#else
    uni_char = (*devfont->charset_ops->conv_to_uc32) (mchar);
#endif
    ttf_inst_info->cur_glyph_code 
             = TT_Char_Index (ttf_glyph_info->char_map, uni_char);

    vec_x = 0;
#if 0
    vec_y = ttf_inst_info->ascent << 6;
#else
    vec_y = 0;
#endif
    TT_Transform_Vector (&vec_x, &vec_y, &ttf_inst_info->matrix);
    x -= (vec_x & -64) >> 6;
    y += ((vec_y + 63) & -64) >> 6;

    if (TT_Load_Glyph (ttf_inst_info->instance, ttf_glyph_info->glyph, 
            ttf_inst_info->cur_glyph_code, TTLOAD_DEFAULT) != TT_Err_Ok)
        return 0;

    TT_Get_Glyph_Metrics (ttf_glyph_info->glyph, &metrics);
    ttf_inst_info->cur_advance = metrics.advance;

    TT_Get_Glyph_Outline (ttf_glyph_info->glyph, &ttf_inst_info->cur_outline);
    if (ttf_inst_info->rotation) {
        TT_Transform_Outline (&ttf_inst_info->cur_outline, &ttf_inst_info->matrix);
    }

    if ((logfont->style & FS_SETWIDTH_CONDENSED) && ttf_glyph_info->can_kern) {
        if (ttf_inst_info->rotation) {
            vec_x = compute_kernval (ttf_inst_info);
            vec_y = 0;
            TT_Transform_Vector (&vec_x, &vec_y, &ttf_inst_info->matrix);

            x += vec_x / 64;
            y -= vec_y / 64;
        } else
            x += compute_kernval (ttf_inst_info) / 64;
    }

    /* we begin by grid-fitting the bounding box */
    TT_Get_Outline_BBox (&ttf_inst_info->cur_outline, &bbox);

    xmin = (bbox.xMin & -64) >> 6;
    ymin = (bbox.yMin & -64) >> 6;
    xmax = ((bbox.xMax + 63) & -64) >> 6;
    ymax = ((bbox.yMax + 63) & -64) >> 6;

#if 0
    /* hack for ... */
    if (ttf_inst_info->ascent < ymax) {
        int off = ymax - ttf_inst_info->ascent;
        ymax -= off;
        ymin -= off;
    }
#endif

    ttf_inst_info->cur_xmin = (bbox.xMin & -64);
    ttf_inst_info->cur_ymin = (bbox.yMin & -64);
    ttf_inst_info->cur_width = xmax - xmin;
    ttf_inst_info->cur_height = ymax - ymin;

    if (pwidth) *pwidth = (int)(xmax - xmin);
    if (pheight) *pheight = (int)(ymax - ymin);

    if (px) *px = (int)(x + xmin);
    if (py) *py = (int)(y - ymax);
    return (int)(xmax - xmin);
}

/* call this function to get the bitmap/pixmap of the char */ 
static const void* 
char_bitmap_pixmap (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len, int* pitch) 
{
    int size;
    TT_Raster_Map Raster;
    TT_Error error;
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);

    size = ttf_inst_info->cur_width * ttf_inst_info->cur_height;

    /* now allocate the raster bitmap */
    Raster.rows = ttf_inst_info->cur_height;
    Raster.width = ttf_inst_info->cur_width;
    if (!pitch)
        Raster.cols = (Raster.width + 7) >> 3;  /* pad to 8-bits */
    else
        *pitch = Raster.cols = (Raster.width + 3) & -4;  /* pad to 32-bits */
    Raster.flow = TT_Flow_Down;
    Raster.size = Raster.rows * Raster.cols;
    Raster.bitmap = get_raster_bitmap_buffer (Raster.size);
    memset (Raster.bitmap, 0, Raster.size);

    /* now render the glyph in the small bitmap/pixmap */

    /* IMPORTANT NOTE: the offset parameters passed to the function     */
    /* TT_Get_Glyph_Bitmap() must be integer pixel values, i.e.,        */
    /* multiples of 64.  HINTING WILL BE RUINED IF THIS ISN'T THE CASE! */
    /* This is why we _did_ grid-fit the bounding box, especially xmin  */
    /* and ymin.                                                        */
    if (!pitch) {
        TT_Translate_Outline (&ttf_inst_info->cur_outline, 
                -ttf_inst_info->cur_xmin, -ttf_inst_info->cur_ymin);
        if ((error = TT_Get_Outline_Bitmap (ttf_engine, &ttf_inst_info->cur_outline,
                &Raster)))
            return NULL;
    }
    else {
        TT_Translate_Outline (&ttf_inst_info->cur_outline, 
                -ttf_inst_info->cur_xmin, -ttf_inst_info->cur_ymin);

        if ((error = TT_Get_Outline_Pixmap (ttf_engine, &ttf_inst_info->cur_outline, 
                &Raster)))
            return NULL;
    }

#if 0
    {
        int i, j, k;
        for (i = 0; i < Raster.rows; i++) {
            for (j = 0; j < Raster.cols; j++) {
                BYTE byte = *((BYTE*)Raster.bitmap + i * Raster.cols + j);
                for (k = 0; k < 8; k++)
                    if (byte & (0x80>>k))
                        fprintf (stderr, "*");
                    else
                        fprintf (stderr, " ");
            }
            fprintf (stderr, "\n");
        }
    }
#endif
    
    return Raster.bitmap;
}

static const void* get_char_bitmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len)
{
    return char_bitmap_pixmap (logfont, devfont, mchar, len, NULL);
}

static const void* get_char_pixmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len, int* pitch)
{
    return char_bitmap_pixmap (logfont, devfont, mchar, len, pitch);
}

/* call this function after getting the bitmap/pixmap of the char
 * to get the advance of the char */
static void get_char_advance (LOGFONT* logfont, DEVFONT* devfont, 
            const unsigned char* mchar, int len, int* px, int* py)
{
    TT_Pos vec_x, vec_y;
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);
    TTFGLYPHINFO* ttf_glyph_info = TTF_GLYPH_INFO_P (devfont);

    if (ttf_inst_info->rotation) {
        vec_x = ttf_inst_info->cur_advance;
        vec_y = 0;
        TT_Transform_Vector (&vec_x, &vec_y, &ttf_inst_info->matrix);

        *px += vec_x / 64;
        *py -= vec_y / 64;
    } else {
        *px += ttf_inst_info->cur_advance / 64;

        if ((logfont->style & FS_SETWIDTH_CONDENSED) && ttf_glyph_info->can_kern) {
                *px += compute_kernval (ttf_inst_info) / 64;
        }

        /* Kerning point syndrome avoidance */
        if (ttf_inst_info->last_pen_pos > *px)
            *px = ttf_inst_info->last_pen_pos;
        ttf_inst_info->last_pen_pos = *px;
    }

    ttf_inst_info->last_glyph_code = ttf_inst_info->cur_glyph_code;
}

static DEVFONT* new_instance (LOGFONT* logfont, DEVFONT* devfont, BOOL need_sbc_font)
{
    TTFGLYPHINFO* ttf_glyph_info = TTF_GLYPH_INFO_P (devfont);
    TTFINSTANCEINFO* ttf_inst_info = NULL;
    DEVFONT* new_devfont = NULL;
    float angle;
    TT_Face_Properties  properties;
    TT_Instance_Metrics imetrics;
    int i, sum_width;
    unsigned short* widths = NULL;

    if ((new_devfont = calloc (1, sizeof (DEVFONT))) == NULL)
        goto out;

    if ((ttf_inst_info = calloc (1, sizeof (TTFINSTANCEINFO))) == NULL)
        goto out;

    if ((widths = calloc (256, sizeof (unsigned short))) == NULL)
        goto out;

    memcpy (new_devfont, devfont, sizeof (DEVFONT));

    if (need_sbc_font && devfont->charset_ops->bytes_maxlen_char > 1) {
        char charset [LEN_FONT_NAME + 1];

        fontGetCompatibleCharsetFromName (devfont->name, charset);
        new_devfont->charset_ops = GetCharsetOps (charset);
    }

    new_devfont->data = ttf_inst_info;
    ttf_inst_info->ttf_glyph_info = ttf_glyph_info;
    ttf_inst_info->widths = widths;

    /* Create instance */
    if (TT_New_Instance (ttf_glyph_info->face, &ttf_inst_info->instance) != TT_Err_Ok)
        goto out;

    /* Set the instance resolution */
    if (TT_Set_Instance_Resolutions (ttf_inst_info->instance, 96, 96) != TT_Err_Ok)
        goto out;

    /* We want real pixel sizes ... not points ...*/
    TT_Set_Instance_PixelSizes (ttf_inst_info->instance, logfont->size,
                                logfont->size, logfont->size * 64);

    /* reset kerning*/
    ttf_inst_info->last_glyph_code = -1;
    ttf_inst_info->last_pen_pos = -1;

    /* Font rotation */
    ttf_inst_info->rotation = logfont->rotation; /* in tenthdegrees */

    /* Build the rotation matrix with the given angle */
    TT_Set_Instance_Transform_Flags (ttf_inst_info->instance, TRUE, FALSE);

    angle = ttf_inst_info->rotation * M_PI / 1800;
    ttf_inst_info->matrix.yy = (TT_Fixed) (cos (angle) * (1 << 16));
    ttf_inst_info->matrix.yx = (TT_Fixed) (sin (angle) * (1 << 16));
    ttf_inst_info->matrix.xx = ttf_inst_info->matrix.yy;
    ttf_inst_info->matrix.xy = -ttf_inst_info->matrix.yx;

    /* Fill up the info fields */
    TT_Get_Face_Properties (ttf_glyph_info->face, &properties);
    TT_Get_Instance_Metrics(ttf_inst_info->instance, &imetrics);

    ttf_inst_info->max_width = ((properties.horizontal->xMax_Extent * \
                            imetrics.x_scale)/ 0x10000) >> 6;

    ttf_inst_info->ascent = (((properties.horizontal->Ascender * \
                            imetrics.y_scale)/0x10000) >> 6) + 1;
    ttf_inst_info->descent = (((properties.horizontal->Descender * \
                            imetrics.y_scale)/0x10000) >> 6) + 1;

    ttf_inst_info->height = ttf_inst_info->ascent - ttf_inst_info->descent;

    sum_width = 0;
    for (i = 0; i <= ttf_glyph_info->last_glyph_index; i++) {
        widths [i] = Get_Glyph_Width (ttf_inst_info, i);
        sum_width += widths [i];
    }

    if (properties.num_Glyphs > 255)
        ttf_inst_info->ave_width = Get_Glyph_Width (ttf_inst_info, 256);
    else
        ttf_inst_info->ave_width = sum_width / properties.num_Glyphs;

    return new_devfont;

out:
    free (widths);
    free (ttf_inst_info);
    free (new_devfont);
    return NULL;
}

static void delete_instance (DEVFONT* devfont)
{
    TTFINSTANCEINFO* ttf_inst_info = TTF_INST_INFO_P (devfont);

    TT_Done_Instance (ttf_inst_info->instance);
    free (ttf_inst_info->widths);
    free (ttf_inst_info);
    free (devfont);
}

/**************************** Global data ************************************/
static FONTOPS freetype_font_ops = {
    get_char_width,
    get_str_width,
    get_ave_width,
    get_max_width,  
    get_font_height,
    get_font_size,
    get_font_ascent,
    get_font_descent,
    char_bitmap_size,
    max_bitmap_size,
    get_char_bitmap,
    get_char_pixmap,
    start_str_output,
    get_char_bbox,
    get_char_advance,
    new_instance,
    delete_instance
};

#endif /* _TTF_SUPPORT */

