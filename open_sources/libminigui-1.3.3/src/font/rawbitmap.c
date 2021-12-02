/*
** $Id: rawbitmap.c,v 1.29 2003/09/25 04:08:55 snig Exp $
** 
** rawbitmap.c: The Raw Bitmap Font operation set.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 2000 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/13
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
** TODO:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "misc.h"

#ifdef HAVE_MMAP
    #include <sys/mman.h>
#endif

#include "charset.h"
#include "devfont.h"
#include "rawbitmap.h"
#include "fontname.h"

#ifdef _RBF_SUPPORT

#ifdef _INCORE_RES

static INCORE_RBFINFO* incore_rbfonts [] = {
    &fixed12x24_iso8859_1,
#ifdef _INCORERBF_GB12
    &hei24_gb2312, /* FIXME@Zhongkai's code */
#endif
};

#define NR_RBFONTS  (sizeof (incore_rbfonts) /sizeof (INCORE_RBFINFO*))

static DEVFONT* incore_rbf_dev_fonts;
static RBFINFO* incore_rbf_infos;

BOOL InitIncoreRBFonts (void)
{
    int i;

    incore_rbf_dev_fonts = calloc (NR_RBFONTS, sizeof (DEVFONT));
    incore_rbf_infos = calloc (NR_RBFONTS, sizeof (RBFINFO));

    if (!incore_rbf_dev_fonts || !incore_rbf_infos)
        return TRUE;

    for (i = 0; i < NR_RBFONTS; i++) {
        char charset [LEN_FONT_NAME + 1];

        if (!fontGetCharsetFromName (incore_rbfonts [i]->name, charset)) {
            fprintf (stderr, "GDI: Invalid font name (charset): %s.\n", 
                    incore_rbfonts [i]->name);
            goto error_load;
        }

        if ((incore_rbf_infos [i].charset_ops 
               = GetCharsetOps (charset)) == NULL) {
            fprintf (stderr, "GDI: Not supported charset: %s.\n", charset);
            goto error_load;
        }

        if ((incore_rbf_infos [i].width = fontGetWidthFromName (incore_rbfonts [i]->name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (width): %s.\n", 
                    incore_rbfonts [i]->name);
            goto error_load;
        }
        
        if ((incore_rbf_infos [i].height = fontGetHeightFromName (incore_rbfonts [i]->name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (height): %s.\n",
                    incore_rbfonts [i]->name);
            goto error_load;
        }
        
        incore_rbf_infos [i].nr_chars = incore_rbf_infos [i].charset_ops->nr_chars;

        incore_rbf_infos [i].font_size = ((incore_rbf_infos [i].width + 7) >> 3) * 
                incore_rbf_infos [i].height * incore_rbf_infos [i].nr_chars;
        incore_rbf_infos [i].font = (unsigned char*) incore_rbfonts [i]->data;

        strncpy (incore_rbf_dev_fonts[i].name, incore_rbfonts [i]->name, LEN_DEVFONT_NAME);
        incore_rbf_dev_fonts[i].name [LEN_DEVFONT_NAME] = '\0';
        incore_rbf_dev_fonts[i].font_ops = &raw_bitmap_font_ops;
        incore_rbf_dev_fonts[i].charset_ops = incore_rbf_infos [i].charset_ops;
        incore_rbf_dev_fonts[i].data = incore_rbf_infos + i;
#if 0
        fprintf (stderr, "GDI: RBFDevFont %i: %s.\n", i, incore_rbf_dev_fonts[i].name);
#endif
    }

    for (i = 0; i < NR_RBFONTS; i++) {
        if (incore_rbf_infos [i].charset_ops->bytes_maxlen_char > 1)
            AddMBDevFont (incore_rbf_dev_fonts + i);
        else
            AddSBDevFont (incore_rbf_dev_fonts + i);
    }

    return TRUE;

error_load:
    fprintf (stderr, "GDI: Error in initializing incore raw bitmap fonts!\n");

    TermIncoreRBFonts ();
    return FALSE;
}

void TermIncoreRBFonts (void)
{
    free (incore_rbf_dev_fonts);
    incore_rbf_dev_fonts = NULL;

    free (incore_rbf_infos);
    incore_rbf_infos = NULL;
}

#else

/********************** Load/Unload of raw bitmap font ***********************/
BOOL LoadRawBitmapFont (const char* file, RBFINFO* RBFInfo)
{
#ifdef HAVE_MMAP
    int fd;
#else
    FILE* fp;
#endif
    size_t size;

    size = ((RBFInfo->width + 7) >> 3) * RBFInfo->height * RBFInfo->nr_chars;
    RBFInfo->font_size = size;

#ifdef HAVE_MMAP
    if ((fd = open (file, O_RDONLY)) < 0)
        return FALSE;

    if ((RBFInfo->font = mmap (NULL, size, PROT_READ, MAP_SHARED, 
            fd, 0)) == MAP_FAILED) {
        close (fd);
        return FALSE;
    }

    close (fd);
#else
    // Allocate memory for font data.
 //printf("ready to malloc font buf(size:%d)!\n",size);
    if (!(RBFInfo->font = (char *)malloc (size)) )
        return FALSE;

 //printf("ready to open ZK file:%s!\n",file);
    // Open font file and read information to the system memory.
    if (!(fp = fopen (file, "rb"))) {
        return FALSE;
    }
 //printf("ready to fread ZK!\n");
    if (fread (RBFInfo->font, sizeof(char), size, fp) < size) {
        fclose(fp);
        return FALSE;
    }
 //printf("fread ZK OK!\n");

    fclose(fp);
#endif  /* HAVE_MMAP */
    return TRUE;
}

void UnloadRawBitmapFont (RBFINFO* RBFInfo)
{
#ifdef HAVE_MMAP
    if (RBFInfo->font)
        munmap (RBFInfo->font, RBFInfo->font_size);
#else
    free (RBFInfo->font);
#endif
}

/********************** Init/Term of raw bitmap font ***********************/
static int nr_fonts;
static RBFINFO* rbf_infos;
static DEVFONT* rbf_dev_fonts;

#define SECTION_NAME "rawbitmapfonts"

BOOL InitRawBitmapFonts (void)
{
    int i;
    char font_name [LEN_DEVFONT_NAME + 1];

    if (GetMgEtcIntValue (SECTION_NAME, "font_number", 
                           &nr_fonts) < 0 )
        return FALSE;
    if ( nr_fonts < 1) return TRUE;

//#ifdef _UNICODE_SUPPORT
#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
    rbf_infos = calloc (nr_fonts + 3, sizeof (RBFINFO));		// +3 means adding UTF-8, UTF-16LE and UTF-16BE
    rbf_dev_fonts = calloc (nr_fonts + 3, sizeof (DEVFONT));
#else
    rbf_infos = calloc (nr_fonts, sizeof (RBFINFO));		
    rbf_dev_fonts = calloc (nr_fonts, sizeof (DEVFONT));
#endif

    if (rbf_infos == NULL || rbf_dev_fonts == NULL) {
        free (rbf_infos);
        free (rbf_dev_fonts);
        return FALSE;
    }

    for (i = 0; i < nr_fonts; i++)
        rbf_infos [i].font = NULL;

    for (i = 0; i < nr_fonts; i++) {
        char key [11];
        char charset [LEN_FONT_NAME + 1];
        char file [MAX_PATH + 1];

        sprintf (key, "name%d", i);
        if (GetMgEtcValue (SECTION_NAME, key, 
                           font_name, LEN_DEVFONT_NAME) < 0 )
            goto error_load;

        if (!fontGetCharsetFromName (font_name, charset)) {
            fprintf (stderr, "GDI: Invalid font name (charset): %s.\n", 
                    font_name);
            goto error_load;
        }

        if ((rbf_infos [i].charset_ops 
               = GetCharsetOps (charset)) == NULL) {
            fprintf (stderr, "GDI: Not supported charset: %s.\n", charset);
            goto error_load;
        }

        if ((rbf_infos [i].width = fontGetWidthFromName (font_name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (width): %s.\n", 
                    font_name);
            goto error_load;
        }
        
        if ((rbf_infos [i].height = fontGetHeightFromName (font_name)) == -1) {
            fprintf (stderr, "GDI: Invalid font name (height): %s.\n",
                    font_name);
            goto error_load;
        }
        
        rbf_infos [i].nr_chars = rbf_infos [i].charset_ops->nr_chars;

        sprintf (key, "fontfile%d", i);
        if (GetMgEtcValue (SECTION_NAME, key, 
                           file, MAX_PATH) < 0)
            goto error_load;

        if (!LoadRawBitmapFont (file, rbf_infos + i))
            goto error_load;

        strncpy (rbf_dev_fonts[i].name, font_name, LEN_DEVFONT_NAME);
        rbf_dev_fonts[i].name [LEN_DEVFONT_NAME] = '\0';
        rbf_dev_fonts[i].font_ops = &raw_bitmap_font_ops;
        rbf_dev_fonts[i].charset_ops = rbf_infos [i].charset_ops;
        rbf_dev_fonts[i].data = rbf_infos + i;
#if 0
        fprintf (stderr, "GDI: RBFDevFont %i: %s.\n", i, rbf_dev_fonts[i].name);
#endif
    }

    for (i = 0; i < nr_fonts; i++) {
        if (rbf_infos [i].charset_ops->bytes_maxlen_char > 1)
            AddMBDevFont (rbf_dev_fonts + i);
        else
            AddSBDevFont (rbf_dev_fonts + i);
    }

//#ifdef _UNICODE_SUPPORT
#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
    /* added by xm.chen --- in order to support UNICODE */
    if (((rbf_infos [nr_fonts].charset_ops = GetCharsetOpsEx ("utf-8")) == NULL) ||
        ((rbf_infos [nr_fonts + 1].charset_ops = GetCharsetOpsEx ("utf-16le")) == NULL) ||
        ((rbf_infos [nr_fonts + 2].charset_ops = GetCharsetOpsEx ("utf-16be")) == NULL)) 
    {
	fprintf (stderr, "GDI: Not supported charset: %s.\n", "utf-8 or utf-16le or utf-16be");
	goto error_load;
    }

    /* UTF-8 related */
    rbf_infos [nr_fonts].width 	= rbf_infos [nr_fonts - 1].width;
    rbf_infos [nr_fonts].height = rbf_infos [nr_fonts - 1].height;
    rbf_infos [nr_fonts].nr_chars = rbf_infos [nr_fonts].charset_ops->nr_chars;

    strncpy (rbf_dev_fonts[nr_fonts].name, "rbf-fixed-rrncnn-*-*-UTF-8", LEN_DEVFONT_NAME);
    rbf_dev_fonts[nr_fonts].name [LEN_DEVFONT_NAME] = '\0';
    rbf_dev_fonts[nr_fonts].font_ops = &raw_bitmap_font_ops;
    rbf_dev_fonts[nr_fonts].charset_ops = rbf_infos [nr_fonts].charset_ops;
    rbf_dev_fonts[nr_fonts].data = rbf_infos + i;

    AddMBDevFont (rbf_dev_fonts + nr_fonts);

    /* UTF-16LE related */
    rbf_infos [nr_fonts + 1].width 	= rbf_infos [nr_fonts - 1].width;
    rbf_infos [nr_fonts + 1].height 	= rbf_infos [nr_fonts - 1].height;
    rbf_infos [nr_fonts + 1].nr_chars 	= rbf_infos [nr_fonts + 1].charset_ops->nr_chars;

    strncpy (rbf_dev_fonts[nr_fonts + 1].name, "rbf-fixed-rrncnn-*-*-UTF-16LE", LEN_DEVFONT_NAME);
    rbf_dev_fonts[nr_fonts + 1].name [LEN_DEVFONT_NAME] = '\0';
    rbf_dev_fonts[nr_fonts + 1].font_ops = &raw_bitmap_font_ops;
    rbf_dev_fonts[nr_fonts + 1].charset_ops = rbf_infos [nr_fonts + 1].charset_ops;
    rbf_dev_fonts[nr_fonts + 1].data = rbf_infos + i + 1;

    AddMBDevFont (rbf_dev_fonts + nr_fonts + 1);

    /* UTF-16BE related */
    rbf_infos [nr_fonts + 2].width 	= rbf_infos [nr_fonts - 1].width;
    rbf_infos [nr_fonts + 2].height 	= rbf_infos [nr_fonts - 1].height;
    rbf_infos [nr_fonts + 2].nr_chars 	= rbf_infos [nr_fonts + 2].charset_ops->nr_chars;

    strncpy (rbf_dev_fonts[nr_fonts + 2].name, "rbf-fixed-rrncnn-*-*-UTF-16BE", LEN_DEVFONT_NAME);
    rbf_dev_fonts[nr_fonts + 2].name [LEN_DEVFONT_NAME] = '\0';
    rbf_dev_fonts[nr_fonts + 2].font_ops = &raw_bitmap_font_ops;
    rbf_dev_fonts[nr_fonts + 2].charset_ops = rbf_infos [nr_fonts + 2].charset_ops;
    rbf_dev_fonts[nr_fonts + 2].data = rbf_infos + i + 2;

    AddMBDevFont (rbf_dev_fonts + nr_fonts + 2);
    /* added by xm.chen --- in order to support UNICODE over */
#endif

    return TRUE;

error_load:
    fprintf (stderr, "GDI: Error in loading raw bitmap fonts!\n");
    for (i = 0; i < nr_fonts; i++)
        UnloadRawBitmapFont (rbf_infos + i);
    
    free (rbf_infos);
    free (rbf_dev_fonts);
    rbf_infos = NULL;
    rbf_dev_fonts = NULL;
    return FALSE;
}

void TermRawBitmapFonts (void)
{
    int i;

    for (i = 0; i < nr_fonts; i++)
        UnloadRawBitmapFont (rbf_infos + i);
    
    free (rbf_infos);
    free (rbf_dev_fonts);

    rbf_infos = NULL;
    rbf_dev_fonts = NULL;
}

#endif /* _INCORE_RES */

/*************** Raw bitmap font operations *********************************/
static int get_char_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    return RBFONT_INFO_P (devfont)->width;
}

static int get_str_width (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mstr, int len, int cExtra)
{
    int number;
    
    number = (*devfont->charset_ops->nr_chars_in_str) (mstr, len);
    return (RBFONT_INFO_P (devfont)->width + cExtra )* number;
}

static int get_ave_width (LOGFONT* logfont, DEVFONT* devfont)
{
    return RBFONT_INFO_P (devfont)->width;
}

static int get_font_height (LOGFONT* logfont, DEVFONT* devfont)
{
    return RBFONT_INFO_P (devfont)->height;
}

static int get_font_size (LOGFONT* logfont, DEVFONT* devfont, int expect)
{
    return RBFONT_INFO_P (devfont)->height;
}

static int get_font_ascent (LOGFONT* logfont, DEVFONT* devfont)
{
    int height = RBFONT_INFO_P (devfont)->height;
    
    if (height >= 20)
        return height - 3;
    else if (height >= 15)
        return height - 2;
    else if (height >= 10)
        return height - 1;

    return height;
}

static int get_font_descent (LOGFONT* logfont, DEVFONT* devfont)
{
    int height = RBFONT_INFO_P (devfont)->height;
    
    if (height >= 20)
        return 3;
    else if (height >= 15)
        return 2;
    else if (height >= 10)
        return 1;

    return 0;
}

static size_t char_bitmap_size (LOGFONT* logfont, DEVFONT* devfont, 
                const unsigned char* mchar, int len)
{
    return ((RBFONT_INFO_P (devfont)->width + 7) >> 3) 
                * RBFONT_INFO_P (devfont)->height;
}

static size_t max_bitmap_size (LOGFONT* logfont, DEVFONT* devfont)
{
    return ((RBFONT_INFO_P (devfont)->width + 7) >> 3) 
                * RBFONT_INFO_P (devfont)->height;
}

static const void* get_char_bitmap (LOGFONT* logfont, DEVFONT* devfont,
            const unsigned char* mchar, int len)
{
    int bitmap_size;
    int offset;

/*--------------------------------------------------
* printf("get_char_bitmap...%s   %s\n", devfont->charset_ops->name, mchar);
*--------------------------------------------------*/
//#ifdef _UNICODE_SUPPORT
#if defined(_UNICODE_SUPPORT) && !defined(_TTF_SUPPORT)
    if(strcmp(devfont->charset_ops->name, "UTF-8") == 0)
    {
	DEVFONT* pfont_utf8 = devfont;
	unsigned int code;
	devfont--;		//pointer to the previous RBF dev font (maybe gb2312 or big5)
	if(devfont == NULL)
	{
	    fprintf(stderr, "error UTF-8 decoding!\n");
	    return NULL;
	}
//printf("change charset to---%s\n",devfont->charset_ops->name);
	code = (*pfont_utf8->charset_ops->conv_to_uc32)(mchar);
	offset = (*devfont->charset_ops->conv_from_uc32) (code, NULL);
    }
    else if(strcmp(devfont->charset_ops->name, "UTF-16LE") == 0)
    {
	DEVFONT* pfont_utf16le = devfont;
	UChar32 code;
	devfont -= 2;		//pointer to the previous RBF dev font (maybe gb2312 or big5)
	if(devfont == NULL)
	{
	    fprintf(stderr, "error UTF-16LE decoding!\n");
	    return NULL;
	}
//printf("change charset to---%s\n",devfont->charset_ops->name);
	code = (*pfont_utf16le->charset_ops->conv_to_uc32)(mchar);
	offset = (*devfont->charset_ops->conv_from_uc32) (code, NULL);
    }
    else if(strcmp(devfont->charset_ops->name, "UTF-16BE") == 0)
    {
	DEVFONT* pfont_utf16be = devfont;
	UChar32 code;
	devfont -= 3;		//pointer to the previous RBF dev font (maybe gb2312 or big5)
	if(devfont == NULL)
	{
	    fprintf(stderr, "error UTF-16LE decoding!\n");
	    return NULL;
	}
//printf("change charset to---%s\n",devfont->charset_ops->name);
	code = (*pfont_utf16be->charset_ops->conv_to_uc32)(mchar);
	offset = (*devfont->charset_ops->conv_from_uc32) (code, NULL);
    }
    else
#endif
    {
	offset = (*devfont->charset_ops->char_offset) (mchar);
    }

    if (offset >= RBFONT_INFO_P (devfont)->font_size)
        offset = (*devfont->charset_ops->char_offset) 
                    (devfont->charset_ops->def_char);

    bitmap_size = ((RBFONT_INFO_P (devfont)->width + 7) >> 3) 
                * RBFONT_INFO_P (devfont)->height; 

    return RBFONT_INFO_P (devfont)->font + bitmap_size * offset;
}

/**************************** Global data ************************************/
FONTOPS raw_bitmap_font_ops = {
    get_char_width,
    get_str_width,
    get_ave_width,
    get_ave_width,  // max_width same as ave_width
    get_font_height,
    get_font_size,
    get_font_ascent,
    get_font_descent,

    char_bitmap_size,
    max_bitmap_size,
    get_char_bitmap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

#endif  /* _RBF_SUPPORT */

