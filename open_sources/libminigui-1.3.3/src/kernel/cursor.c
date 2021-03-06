/*
** $Id: cursor.c,v 1.51 2003/11/23 04:09:08 weiym Exp $
**
** cursor.c: the Cursor Support module of MiniGUI.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999.01.06
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


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"
#include "gal.h"
#include "dc.h"
#include "memops.h"
#include "inline.h"
#include "gal.h"
#include "cursor.h"
#include "ial.h"
#include "readbmp.h"
#include "misc.h"
#include "cursor.h"
#include "sysres.h"

// mutex ensuring exclusive access to mouse.
pthread_mutex_t __mg_mouselock;

static int curx, cury;
static int oldx = -1, oldy;

static RECT cliprc = { 0, 0, 0, 0};

#ifdef _CURSOR_SUPPORT
 
static PCURSOR SysCursor [MAX_SYSCURSORINDEX + 1];
static HCURSOR def_cursor;
static BYTE* savedbits = NULL;
static BYTE* cursorbits = NULL;
static int csrimgsize;
 
static int oldboxleft = -100, oldboxtop = -100;
static int nShowCount = 0;
static PCURSOR pCurCsr = NULL;

#ifdef _USE_NEWGAL
static int csrimgpitch;
Uint8* GetPixelUnderCursor (int x, int y, gal_pixel* pixel)
{
    Uint8* dst = NULL;
    
    pthread_mutex_lock (&__mg_mouselock);
    if (nShowCount >= 0 && pCurCsr
            && x >= oldboxleft && y >= oldboxtop
            && (x < oldboxleft + CURSORWIDTH)
            && (y < oldboxtop + CURSORHEIGHT)) {
        int _x = x - oldboxleft;
        int _y = y - oldboxtop;

        dst = savedbits + _y * csrimgpitch
                + _x * __gal_screen->format->BytesPerPixel;
        *pixel = _mem_get_pixel (dst, __gal_screen->format->BytesPerPixel);
    }
    pthread_mutex_unlock (&__mg_mouselock);

    return dst;
}
#endif

// Cursor creating and destroying.
// Only called from InitCursor and client code.
HCURSOR GUIAPI LoadCursorFromFile(const char* filename)
{
    FILE* fp;
    WORD wTemp;
    int  w, h, xhot, yhot, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;
    BYTE* image;
    HCURSOR csr = 0;
    
    if( !(fp = fopen(filename, "rb")) ) return 0;

    fseek(fp, sizeof(WORD), SEEK_SET);

    // the cbType of struct CURSORDIR.
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 2) goto error;

    // skip the cdCount of struct CURSORDIR, we always use the first cursor.
    fseek(fp, sizeof(WORD), SEEK_CUR);
    
    // cursor info, read the members of struct CURSORDIRENTRY.
    w = fgetc (fp);  // the width of first cursor.
    h = fgetc (fp);  // the height of first cursor.
    if(w != CURSORWIDTH || h != CURSORHEIGHT) goto error;
    fseek(fp, sizeof(BYTE)*2, SEEK_CUR); // skip the bColorCount and bReserved.
    wTemp = MGUI_ReadLE16FP (fp);
    xhot = wTemp;
    wTemp = MGUI_ReadLE16FP (fp);
    yhot = wTemp;
    size = MGUI_ReadLE32FP (fp);
    offset = MGUI_ReadLE32FP (fp);

    // read the cursor image info.
    fseek(fp, offset, SEEK_SET);
    fseek(fp, sizeof(DWORD), SEEK_CUR); // skip the biSize member.
    imagew = MGUI_ReadLE32FP (fp);
    imageh = MGUI_ReadLE32FP (fp);
    // check the biPlanes member;
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 1) goto error;
    // check the biBitCount member;
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp > 4) goto error;
    colornum = (int)wTemp;
    fseek(fp, sizeof(DWORD), SEEK_CUR); // skip the biCompression members.
    imagesize = MGUI_ReadLE32FP (fp);

    // skip the rest members and the color table.
    fseek(fp, sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum), SEEK_CUR);
    
    // allocate memory for image.
#ifdef HAVE_ALLOCA
    if ((image = (BYTE*)alloca (imagesize)) == NULL)
#else
    if ((image = (BYTE*)malloc (imagesize)) == NULL)
#endif
        goto error;

    // read image
    fread (image, imagesize, 1, fp);
    
    csr = CreateCursor(xhot, yhot, w, h, 
                        image + (imagesize - MONOSIZE), image, colornum);

#ifndef HAVE_ALLOCA
    free (image);
#endif

error:
    fclose (fp);
    return csr;
}

HCURSOR GUIAPI LoadCursorFromMem (const void* area)
{
    const Uint8* p = (Uint8*)area;
    WORD wTemp;

    int  w, h, xhot, yhot, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;
    
    p += sizeof (WORD);
    wTemp = MGUI_ReadLE16Mem (&p);
    if(wTemp != 2) goto error;

    // skip the cdCount of struct CURSORDIR, we always use the first cursor.
    p += sizeof (WORD);
    
    // cursor info, read the members of struct CURSORDIRENTRY.
    w = *p++;  // the width of first cursor.
    h = *p++;  // the height of first cursor.
    if (w != CURSORWIDTH || h != CURSORHEIGHT)
        goto error;

    // skip the bColorCount and bReserved.
    p += sizeof(BYTE)*2;
    xhot = MGUI_ReadLE16Mem (&p);
    yhot = MGUI_ReadLE16Mem (&p);
    size = MGUI_ReadLE32Mem (&p);
    offset = MGUI_ReadLE32Mem (&p);

    // read the cursor image info.
    p = (Uint8*)area + offset;

    // skip the biSize member.
    p += sizeof (DWORD);    
    imagew = MGUI_ReadLE32Mem (&p);
    imageh = MGUI_ReadLE32Mem (&p);

    // check the biPlanes member;
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp != 1) goto error;

    // check the biBitCount member;
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp > 4) goto error;
    colornum = wTemp;

    // skip the biCompression members.
    p += sizeof (DWORD);    
    imagesize = MGUI_ReadLE32Mem (&p);

    // skip the rest members and the color table.
    p += sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum);
    
    return CreateCursor (xhot, yhot, w, h, 
                        p + (imagesize - MONOSIZE), p, colornum);

error:
    return 0;
}

#ifdef _USE_NEWGAL
static BITMAP csr_bmp = {BMP_TYPE_NORMAL, 0, 0, 0, 0, CURSORWIDTH, CURSORHEIGHT};
#endif

// Only called from InitCursor and client code.
HCURSOR GUIAPI CreateCursor(int xhotspot, int yhotspot, int w, int h,
                     const BYTE* pANDBits, const BYTE* pXORBits, int colornum)
{
    PCURSOR pcsr;
    
    if( w != CURSORWIDTH || h != CURSORHEIGHT ) return 0;

    // allocate memory.
    if( !(pcsr = (PCURSOR)malloc(sizeof(CURSOR))) ) return 0;
    if( !(pcsr->AndBits = malloc(csrimgsize)) ) {
        free(pcsr);
        return 0;
    }
    if( !(pcsr->XorBits = malloc(csrimgsize)) ) {
        free(pcsr->AndBits);
        free(pcsr);
        return 0;
    }

    pcsr->xhotspot = xhotspot;
    pcsr->yhotspot = yhotspot;
    pcsr->width = w;
    pcsr->height = h;

#ifdef _USE_NEWGAL
    if (colornum == 1) {
        ExpandMonoBitmap (HDC_SCREEN, pcsr->AndBits, csrimgpitch, pANDBits, MONOPITCH,
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
        ExpandMonoBitmap (HDC_SCREEN, pcsr->XorBits, csrimgpitch, pXORBits, MONOPITCH,
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
    }
    else if (colornum == 4) {
        ExpandMonoBitmap (HDC_SCREEN, pcsr->AndBits, csrimgpitch, pANDBits, MONOPITCH,
                        w, h, MYBMP_FLOW_UP, 0, 0xFFFFFFFF);
        Expand16CBitmap (HDC_SCREEN, pcsr->XorBits, csrimgpitch, pXORBits, MONOPITCH*4,
                        w, h, MYBMP_FLOW_UP, NULL);
    }
#else
    if(colornum == 1) {
        ExpandMonoBitmap (HDC_SCREEN, w, h, pANDBits, MYBMP_FLOW_UP, MONOPITCH, 
                         pcsr->AndBits, 0, 0xFFFFFFFF);
        ExpandMonoBitmap (HDC_SCREEN, w, h, pXORBits, MYBMP_FLOW_UP, MONOPITCH,
                         pcsr->XorBits, 0, 0xFFFFFFFF);
    }
    else if(colornum == 4) {
        ExpandMonoBitmap (HDC_SCREEN, w, h, pANDBits, MYBMP_FLOW_UP, MONOPITCH, 
                         pcsr->AndBits, 0, 0xFFFFFFFF);
        Expand16CBitmap (HDC_SCREEN, w, h, pXORBits, MYBMP_FLOW_UP, MONOPITCH*4,
                        pcsr->XorBits, NULL);
    }
#endif

    return (HCURSOR)pcsr;
}

// Only called from client code.
BOOL GUIAPI DestroyCursor(HCURSOR hcsr)
{
    int i;
    PCURSOR pcsr = (PCURSOR)hcsr;

    if (pcsr == NULL)
        return TRUE;

    for(i = 0; i <= MAX_SYSCURSORINDEX; i++)
    {
        if(pcsr == SysCursor[i])
            return FALSE;
    }

    free(pcsr->AndBits);
    free(pcsr->XorBits);
    free(pcsr);
    return TRUE;
}

// Only called from client code, and accessed items are not changable ones.
HCURSOR GUIAPI GetSystemCursor(int csrid)
{
    if(csrid > MAX_SYSCURSORINDEX || csrid < 0)
        return 0; 

    return (HCURSOR)(SysCursor[csrid]);
}

HCURSOR GUIAPI GetDefaultCursor (void)
{
    return def_cursor;
}

#define CURSORSECTION   "cursorinfo"

BOOL InitCursor(void)
{
    char szValue[MAX_NAME + 1];
    int number;
    int i;

#ifdef _USE_NEWGAL
    csrimgsize = GAL_GetBoxSize (__gal_screen, CURSORWIDTH, CURSORHEIGHT, &csrimgpitch);

    csr_bmp.bmBitsPerPixel = __gal_screen->format->BitsPerPixel;
    csr_bmp.bmBytesPerPixel = __gal_screen->format->BytesPerPixel;
    csr_bmp.bmPitch = csrimgpitch;
#else
    csrimgsize = CURSORWIDTH * CURSORHEIGHT * BYTESPERPHYPIXEL;
#endif

    if( !(savedbits = malloc(csrimgsize)) )
        return FALSE;

    if( !(cursorbits = malloc(csrimgsize)) )
    {
        free(savedbits);
        savedbits = NULL;
        return FALSE;
    }

    if( GetMgEtcValue (CURSORSECTION, "cursornumber", szValue, 10) < 0 )
        goto error;
    
    number = atoi(szValue);

    if(number <= 0)
        return TRUE;

    number = number < (MAX_SYSCURSORINDEX + 1) ? 
             number : (MAX_SYSCURSORINDEX + 1);

    for(i = 0; i < number; i++) {
        if ( !(SysCursor[i] = LoadSystemCursor(i)) )
             goto error;
    }
	
    pthread_mutex_init(&__mg_mouselock, NULL);

    return TRUE;
error:
    TerminateCursor();
    return FALSE;
}

// The following function must be called at last. 
void TerminateCursor( void )
{
    int i;

    if (!savedbits ) return;

    pthread_mutex_destroy (&__mg_mouselock);

    free(savedbits);
    free(cursorbits);
    savedbits = NULL;
    pCurCsr = NULL;
    nShowCount = 0;
 
    for(i = 0; i<= MAX_SYSCURSORINDEX; i++)
    {
        if( SysCursor[i] ) {
            free(SysCursor[i]->AndBits);
            free(SysCursor[i]->XorBits);
            free(SysCursor[i]);
            SysCursor[i] = NULL;
       }
    }
}

HCURSOR GUIAPI GetCurrentCursor(void)
{
    HCURSOR hcsr;

    pthread_mutex_lock (&__mg_mouselock);
    
    hcsr = (HCURSOR)pCurCsr;
    pthread_mutex_unlock(&__mg_mouselock);

    return hcsr;
}

// Cursor pointer shape and hiding and showing.
static inline int boxleft(void)
{
    if(!pCurCsr) return -100;
    return curx - pCurCsr->xhotspot;
}
static inline int boxtop(void)
{
    if(!pCurCsr) return -100;
    return cury - pCurCsr->yhotspot;
}

#ifdef _USE_NEWGAL

static GAL_Rect csr_rect = {0, 0, CURSORWIDTH, CURSORHEIGHT};

static void hidecursor (void)
{
    csr_rect.x = oldboxleft;
    csr_rect.y = oldboxtop;

    csr_bmp.bmBits = savedbits;

    GAL_SetClipRect (__gal_screen, NULL);
    GAL_PutBox (__gal_screen, &csr_rect, &csr_bmp); 
    GAL_UpdateRects (__gal_screen, 1, &csr_rect);
}

static void showcursor (void)
{
    int x, y;

    x = boxleft ();
    y = boxtop ();

    csr_rect.x = x;
    csr_rect.y = y;
    csr_bmp.bmBits = savedbits;

    GAL_SetClipRect (__gal_screen, NULL);
    GAL_GetBox (__gal_screen, &csr_rect, &csr_bmp);

    oldboxleft = x;
    oldboxtop = y;

    GAL_memcpy4 (cursorbits, savedbits, csrimgsize >> 2);

#ifdef ASM_memandcpy4
    ASM_memandcpy4 (cursorbits, pCurCsr->AndBits, csrimgsize >> 2);
    ASM_memxorcpy4 (cursorbits, pCurCsr->XorBits, csrimgsize >> 2);
#else
    {
        int i;
        Uint32* andbits = (Uint32*) pCurCsr->AndBits;
        Uint32* xorbits = (Uint32*) pCurCsr->XorBits;
        Uint32* dst = (Uint32*) cursorbits;

        for (i = 0; i < csrimgsize >> 2; i++) {
            dst [i] &= andbits [i];
            dst [i] ^= xorbits[i];
        }
    }
#endif
    csr_bmp.bmBits = cursorbits;
    GAL_PutBox (__gal_screen, &csr_rect, &csr_bmp);
    GAL_UpdateRects (__gal_screen, 1, &csr_rect);
}

#else

static inline void hidecursor(void)
{
    GAL_SetGC(PHYSICALGC);

    GAL_EnableClipping(PHYSICALGC);
    GAL_PutBox(PHYSICALGC, oldboxleft, oldboxtop, CURSORWIDTH, CURSORHEIGHT, savedbits);
}

static inline void showcursor(void)
{
    int x, y;

    GAL_SetGC(PHYSICALGC);
    x = boxleft();
    y = boxtop();

    GAL_DisableClipping(PHYSICALGC);
    GAL_GetBox(PHYSICALGC, x, y, CURSORWIDTH, CURSORHEIGHT, savedbits);
    oldboxleft = x;
    oldboxtop = y;

    memcpy(cursorbits, savedbits, csrimgsize);
    {
        int i;
        Uint32* andbits = (Uint32*) pCurCsr->AndBits;
        Uint32* xorbits = (Uint32*) pCurCsr->XorBits;
        Uint32* dst = (Uint32*) cursorbits;

        for (i = 0; i < csrimgsize >> 2; i++) {
            dst [i] &= andbits [i];
            dst [i] ^= xorbits[i];
        }
    }
    GAL_EnableClipping(PHYSICALGC);
    GAL_PutBox(PHYSICALGC, x, y, CURSORWIDTH, CURSORHEIGHT, cursorbits);

}
#endif

HCURSOR GUIAPI SetCursorEx (HCURSOR hcsr, BOOL setdef)
{
    PCURSOR old, pcsr;

    pthread_mutex_lock (&__mg_mouselock);

    if (setdef) {
        old = (PCURSOR) def_cursor;
        def_cursor = hcsr;
    }
    else
        old = pCurCsr;

    if ((PCURSOR)hcsr == pCurCsr) {
        pthread_mutex_unlock(&__mg_mouselock);
        return (HCURSOR) old;
    }
    pthread_mutex_unlock(&__mg_mouselock);

    pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    pcsr = (PCURSOR)hcsr;

    if (pCurCsr)
        hidecursor();

    pCurCsr = pcsr;

    if (nShowCount >= 0 && pCurCsr)
        showcursor();

    pthread_mutex_unlock(&__mg_mouselock);
    pthread_mutex_unlock(&__mg_gdilock);
    return (HCURSOR) old;
}

void ShowCursorForGDI(BOOL fShow, const RECT* prc)
{
    int csrleft, csrright, csrtop, csrbottom;
    int intleft, intright, inttop, intbottom;

    if (!fShow)
        pthread_mutex_lock (&__mg_mouselock);

    csrleft = boxleft();
    csrright = csrleft + CURSORWIDTH;
    csrtop = boxtop();
    csrbottom = csrtop + CURSORHEIGHT;

    intleft = (csrleft > prc->left) ? csrleft : prc->left;
    inttop  = (csrtop > prc->top) ? csrtop : prc->top;
    intright = (csrright < prc->right) ? csrright : prc->right;
    intbottom = (csrbottom < prc->bottom) ? csrbottom : prc->bottom;

    if (intleft >= intright || inttop >= intbottom) {
        if (fShow) {
#ifdef _USE_NEWGAL
            GAL_UpdateRect (__gal_screen, prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
            pthread_mutex_unlock(&__mg_mouselock);
        }
        return;
    }

    if (fShow && nShowCount >= 0 && pCurCsr) {
        showcursor();
    }
    if (!fShow && nShowCount >= 0 && pCurCsr) {
        hidecursor();
    }

    if (fShow) {
#ifdef _USE_NEWGAL
        GAL_UpdateRect (__gal_screen, prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
        pthread_mutex_unlock(&__mg_mouselock);
    }
}

int GUIAPI ShowCursor(BOOL fShow)
{
    int count;

    pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    if(fShow) {
        nShowCount++;
        if(nShowCount == 0 && pCurCsr)
           showcursor();
    }
    else {
        nShowCount--;
        if(nShowCount == -1 && pCurCsr);
           hidecursor();
    }

    count = nShowCount;

    pthread_mutex_unlock(&__mg_mouselock);
    pthread_mutex_unlock(&__mg_gdilock);
    return count;
}

#else

void ShowCursorForGDI(BOOL fShow, const RECT* prc)
{
#ifdef _USE_NEWGAL
    if (fShow)
        GAL_UpdateRect (__gal_screen, prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
}

#endif /* _CURSOR_SUPPORT */

BOOL RefreshCursor(int* x, int* y, int* button)
{
    pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    IAL_GetMouseXY (x, y);
    curx = *x;
    cury = *y;
    *button = IAL_GetMouseButton ();
    if(oldx != curx || oldy != cury)
    {
#ifdef _CURSOR_SUPPORT
        if(nShowCount >= 0 && pCurCsr) {
            hidecursor();
            showcursor();
        }
#endif
        oldx = curx;
        oldy = cury;

        pthread_mutex_unlock(&__mg_mouselock);
        pthread_mutex_unlock(&__mg_gdilock);
        return TRUE;
    }

    pthread_mutex_unlock(&__mg_mouselock);
    pthread_mutex_unlock(&__mg_gdilock);
    return FALSE;
}

// Cursor position.

void GUIAPI GetCursorPos(POINT* ppt)
{
    pthread_mutex_lock (&__mg_mouselock);
    ppt->x = curx;
    ppt->y = cury;
    pthread_mutex_unlock(&__mg_mouselock);
}

void GUIAPI SetCursorPos(int x, int y)
{
    pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    IAL_SetMouseXY (x, y);
    IAL_GetMouseXY (&curx, &cury);

    if (oldx != curx || oldy != cury) {
#ifdef _CURSOR_SUPPORT
        if(nShowCount >= 0 && pCurCsr) {
            hidecursor();
            showcursor();
        }
#endif
        oldx = curx;
        oldy = cury;
    }

    pthread_mutex_unlock(&__mg_mouselock);
    pthread_mutex_unlock(&__mg_gdilock);
}

// Cursor clipping support.
void GUIAPI ClipCursor(const RECT* prc)
{
    RECT rc;

    pthread_mutex_lock (&__mg_mouselock);

    if( IsRectEmpty(&cliprc) )
        SetRect(&cliprc, 0, 0, WIDTHOFPHYGC - 1, HEIGHTOFPHYGC - 1);

    if(prc == NULL)
    {
        IAL_SetMouseRange (0,0,WIDTHOFPHYGC - 1,HEIGHTOFPHYGC - 1);
        SetRect(&cliprc, 0, 0, WIDTHOFPHYGC - 1, HEIGHTOFPHYGC - 1);
        pthread_mutex_unlock(&__mg_mouselock);
        return;
    }
        
    memcpy(&rc, prc, sizeof(RECT));
    NormalizeRect(&rc);
    IntersectRect(&cliprc, &rc, &cliprc);
    NormalizeRect(&cliprc);

    IAL_SetMouseRange (cliprc.left,cliprc.top, cliprc.right,cliprc.bottom);

    pthread_mutex_unlock(&__mg_mouselock);
}

void GUIAPI GetClipCursor(RECT* prc)
{
    pthread_mutex_lock (&__mg_mouselock);
    memcpy(prc, &cliprc, sizeof(RECT));
    pthread_mutex_unlock(&__mg_mouselock);
}

