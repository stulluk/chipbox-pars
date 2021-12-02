
#include "readbmp.h"

// This function expand monochorate bitmap.
void GUIAPI ExpandMonoBitmap (HDC hdc, int w, int h, const BYTE* bits, int bits_flow, int pitch, 
                             BYTE* bitmap, int bg, int fg)
{
    int x, y;
    const BYTE* buf;
    int b = 0;
    int bpp;

    bpp = GAL_BytesPerPixel (dc_HDC2PDC(hdc)->gc);

    if (bits_flow == MYBMP_FLOW_UP)
        buf = bits + pitch * h;
    else
        buf = bits;

    // expand bits here.
    for (y = 0; y < h; y++) {
        if (bits_flow == MYBMP_FLOW_UP)
            buf -= pitch;
        bits = buf;
        for (x = 0; x < w; x++) {
            if (x % 8 == 0)
                b = *bits++;
            if ((b & (128 >> (x % 8))))   /* pixel */
                switch (bpp) {
                case 1:
                    *bitmap = fg;
                    bitmap++;
                    break;
                case 2:
                    *(Uint16 *) bitmap = fg;
                    bitmap += 2;
                    break;
                case 3:
                    *(Uint16 *) bitmap = fg;
                    *(bitmap + 2) = fg >> 16;
                    bitmap += 3;
                    break;
                case 4:
                    *(Uint32 *) bitmap = fg;
                    bitmap += 4;
                }
             else              /* background pixel */
                switch (bpp) {
                case 1:
                    *bitmap = bg;
                    bitmap++;
                    break;
                case 2:
                    *(Uint16 *) bitmap = bg;
                    bitmap += 2;
                    break;
                case 3:
                    *(Uint16 *) bitmap = bg;
                    *(bitmap + 2) = bg;
                    bitmap += 3;
                    break;
                case 4:
                    *(Uint32 *) bitmap = bg;
                    bitmap += 4;
                }
        }
        
        if (bits_flow != MYBMP_FLOW_UP)
            buf += pitch;
    }
}

static const RGB WindowsStdColor [] = {
    {0x00, 0x00, 0x00},     // black         --0
    {0x80, 0x00, 0x00},     // dark red      --1
    {0x00, 0x80, 0x00},     // dark green    --2
    {0x80, 0x80, 0x00},     // dark yellow   --3
    {0x00, 0x00, 0x80},     // dark blue     --4
    {0x80, 0x00, 0x80},     // dark magenta  --5
    {0x00, 0x80, 0x80},     // dark cyan     --6
    {0xC0, 0xC0, 0xC0},     // light gray    --7
    {0x80, 0x80, 0x80},     // dark gray     --8
    {0xFF, 0x00, 0x00},     // red           --9
    {0x00, 0xFF, 0x00},     // green         --10
    {0xFF, 0xFF, 0x00},     // yellow        --11
    {0x00, 0x00, 0xFF},     // blue          --12
    {0xFF, 0x00, 0xFF},     // magenta       --13
    {0x00, 0xFF, 0xFF},     // cyan          --14
    {0xFF, 0xFF, 0xFF},     // light white   --15
};

// This function expand 16-color bitmap.
void GUIAPI Expand16CBitmap (HDC hdc, int w, int h, const BYTE* bits, int bits_flow, int pitch,
                            BYTE* bitmap, const RGB* pal)
{
    PDC pdc;
    int x, y;
    const BYTE* buf;
    int b = 0;
    int c;
    int bpp;

    pdc = dc_HDC2PDC(hdc);
    bpp = GAL_BytesPerPixel (pdc->gc);

    if (bits_flow == MYBMP_FLOW_UP)
        buf = bits + pitch * h;
    else
        buf = bits;
    // expand bits here.
    for (y = 0; y < h; y++)
    {
        if (bits_flow == MYBMP_FLOW_UP)
            buf -= pitch;
        bits = buf;
        for (x = 0; x < w; x++) {
            if (x % 2 == 0)
                b = *bits++;

            if (x % 2 == 0)
                c = (b >> 4) & 0x0f;
            else
                c = b & 0x0f;

            if (pal)
                c = GAL_MapColor (pdc->gc, (GAL_Color*)(pal + c));
            else
                c = GAL_MapColor (pdc->gc, (GAL_Color*)(WindowsStdColor + c));

            switch (bpp) {
                case 1:
                    *bitmap = c;
                    bitmap++;
                    break;
                case 2:
                    *(Uint16 *) bitmap = c;
                    bitmap += 2;
                    break;
                case 3:
                    *(Uint16 *) bitmap = c;
                    *(bitmap + 2) = c >> 16;
                    bitmap += 3;
                    break;
                case 4:
                    *(Uint32 *) bitmap = c;
                    bitmap += 4;
            }
        }

        if (bits_flow != MYBMP_FLOW_UP)
            buf += pitch;
    }
}

// This function expands 256-color bitmap.
void GUIAPI Expand256CBitmap (HDC hdc, int w, int h, const BYTE* bits, int bits_flow, int pitch,
                             BYTE* bitmap, const RGB* pal)
{
    PDC pdc;
    int x, y;
    const BYTE* buf;
    int c;
    int bpp;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->gc);

    if (bits_flow == MYBMP_FLOW_UP)
        buf = bits + pitch * h;
    else
        buf = bits;

    // expand bits here.
    for (y = 0; y < h; y++)
    {
        if (bits_flow == MYBMP_FLOW_UP)
            buf -= pitch;
        bits = buf;
        for (x = 0; x < w; x++) {
            c = *bits++;
            c = GAL_MapColor (pdc->gc, (GAL_Color*)(pal + c));
            
            switch (bpp) {
                case 1:
                    *bitmap = c;
                    bitmap++;
                    break;
                case 2:
                    *(Uint16 *) bitmap = c;
                    bitmap += 2;
                    break;
                case 3:
                    *(Uint16 *) bitmap = c;
                    *(bitmap + 2) = c >> 16;
                    bitmap += 3;
                    break;
                case 4:
                    *(Uint32 *) bitmap = c;
                    bitmap += 4;
            }
        }
        if (bits_flow != MYBMP_FLOW_UP)
            buf += pitch;
    }
}

// This function compile a RGB bitmap
void GUIAPI CompileRGBBitmap (HDC hdc, int w, int h, const BYTE* bits, int bits_flow, int pitch,
                             BYTE* bitmap, int rgb_order)
{
    PDC pdc;
    int x, y;
    const BYTE* buf;
    int c;
    GAL_Color rgb;
    int bpp;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->gc);

    if (bits_flow == MYBMP_FLOW_UP)
        buf = bits + pitch * h;
    else
        buf = bits;

    // expand bits here.
    for (y = 0; y < h; y++)
    {
        if (bits_flow == MYBMP_FLOW_UP)
            buf -= pitch;
        bits = buf;
        for (x = 0; x < w; x++) {
            if (rgb_order == MYBMP_TYPE_BGR) {
                rgb.b = *bits++;
                rgb.g = *bits++;
                rgb.r = *bits++;
            }
            else {
                rgb.r = *bits++;
                rgb.g = *bits++;
                rgb.b = *bits++;
            }
            c = GAL_MapColor (pdc->gc, &rgb);
            
            switch (bpp) {
                case 1:
                    *bitmap = c;
                    bitmap++;
                    break;
                case 2:
                    *(Uint16 *) bitmap = c;
                    bitmap += 2;
                    break;
                case 3:
                    *(Uint16 *) bitmap = c;
                    *(bitmap + 2) = c >> 16;
                    bitmap += 3;
                    break;
                case 4:
                    *(Uint32 *) bitmap = c;
                    bitmap += 4;
            }
        }
        if (bits_flow != MYBMP_FLOW_UP)
            buf += pitch;
    }
}

// This function replaces one color with specified color.
void GUIAPI ReplaceBitmapColor (HDC hdc, PBITMAP pBitmap, gal_pixel iOColor, gal_pixel iNColor)
{
    PDC pdc;
    int i, size;
    BYTE* bitmap;   
    int bpp;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->gc);

    size = pBitmap->bmPitch * pBitmap->bmHeight;
    bitmap = pBitmap->bmBits;

    switch (bpp) {
        case 1:
            for(i=0; i<size; i++) {
                if( *bitmap == iOColor)
                    *bitmap = iNColor;
                bitmap++;
            }
            break;
        case 2:
            for(i=0; i<size; i+=2) {
                if( *(Uint16 *) bitmap == iOColor)
                    *(Uint16 *) bitmap = iNColor;
                bitmap += 2;
            }
            break;
        case 3: 
            for(i=0; i<size; i+=3) {
                if( (*(Uint16 *) bitmap == iOColor) 
                   && (*(bitmap + 2) == (iOColor >> 16)) )
                {
                    *(Uint16 *) bitmap = iNColor;
                    *(bitmap + 2) = iNColor >> 16;
                }
                bitmap += 3;
            }
            break;
        case 4:    
            for(i=0; i<size; i+=4) {
                if( *(Uint32 *) bitmap == iOColor )
                    *(Uint32 *) bitmap = iNColor;
                bitmap += 4;
            }
            break;
    }
}

void GUIAPI DrawHVDotLine (HDC hdc, int x, int y, int w_h, BOOL H_V)
{
    PDC pdc;
    BYTE* bitmap, *vbuff;
    int i, bpp, size;

    if (w_h < 1)
        return;

    pdc = dc_HDC2PDC (hdc);
    bpp = GAL_BytesPerPixel (pdc->gc);

    size = w_h * bpp;
#ifdef HAVE_ALLOCA
    if (!(bitmap = alloca (size)))
#else
    if (!(bitmap = malloc (size)))
#endif
        return;
    vbuff = bitmap;

    switch (bpp) {
        case 1:
            for (i = 0; i < size; i += 2) {
                *vbuff = (BYTE)(pdc->pencolor);
                vbuff++;
                *vbuff = (BYTE)(pdc->brushcolor);
                vbuff++;
            }
            break;
        case 2:
            for (i = 0; i < size; i += 4) {
                *(Uint16 *) vbuff = (Uint16)(pdc->pencolor);
                vbuff += 2;
                *(Uint16 *) vbuff = (Uint16)(pdc->brushcolor);
                vbuff += 2;
            }
            break;
        case 3:
            for (i = 0; i < size; i += 6) {
                *(Uint16 *) vbuff = (Uint16)(pdc->pencolor);
                *(vbuff + 2) = (BYTE)(pdc->pencolor >> 16);
                vbuff += 3;
                *(Uint16 *) vbuff = (Uint16)(pdc->brushcolor);
                *(vbuff + 2) = (BYTE)(pdc->brushcolor >> 16);
                vbuff += 3;
            }
            break;
        case 4:
            for (i = 0; i < size; i += 8) {
                *(Uint32 *) vbuff = (Uint32)(pdc->pencolor);
                vbuff += 4;
                *(Uint32 *) vbuff = (Uint32)(pdc->brushcolor);
                vbuff += 4;
            }
            break;
    }

    PutSavedBoxOnDC (hdc, x, y, H_V ? w_h : 1, H_V ? 1 : w_h, bitmap);
#ifndef HAVE_ALLOCA
    free (bitmap);
#endif
}

/* 
 * This function performs a fast box scaling.
 * This is a DDA-based algorithm; Iteration over target bitmap.
 *
 * This function comes from SVGALib, Copyright 1993 Harm Hanemaayer
 */

/* We use the 32-bit to 64-bit multiply and 64-bit to 32-bit divide of the
 * 386 (which gcc doesn't know well enough) to efficiently perform integer
 * scaling without having to worry about overflows.
 */

#if defined(__GNUC__) && defined(i386)

static inline int muldiv64(int m1, int m2, int d)
{
    /* int32 * int32 -> int64 / int32 -> int32 */
    int result;
    int dummy;
    __asm__ __volatile__ (
               "imull %%edx\n\t"
               "idivl %4\n\t"
  :               "=a"(result), "=d"(dummy)        /* out */
  :               "0"(m1), "1"(m2), "g"(d)                /* in */
               /***rjr***:               "ax", "dx"***/        /* mod */
        );
    return result;
}

#else

static inline int muldiv64 (int m1, int m2, int d)
{
    long long int mul = (long long int) m1 * m2;

    return (int) (mul / d);
}

#endif

/* Init a bitmap as a normal bitmap  */
BOOL GUIAPI InitBitmap (HDC hdc, Uint32 w, Uint32 h, Uint32 pitch, BYTE* bits, PBITMAP bmp)
{
    int depth = GetGDCapability (hdc, GDCAP_DEPTH);
    int bpp = GetGDCapability (hdc, GDCAP_BPP);

    if (w == 0 || h == 0)
        return FALSE;

    if (bits && pitch) {
        if (bpp * w > pitch)
            return FALSE;

        bmp->bmBits = bits;
        bmp->bmPitch = pitch;
    }
    else if (!bits) {
        bmpComputePitch (depth, w, &bmp->bmPitch, TRUE);

        if (!(bmp->bmBits = malloc (bmp->bmPitch * h)))
            return FALSE;
    }
    else /* bits is not zero, but pitch is zero */
        return FALSE;

    bmp->bmType = BMP_TYPE_NORMAL;
    bmp->bmWidth = w;
    bmp->bmHeight = h;
    bmp->bmBitsPerPixel = depth;
    bmp->bmBytesPerPixel = bpp;

    return TRUE;
}

BOOL ScaleBitmap (BITMAP *dst, const BITMAP *src)
{
    BYTE *dp1 = src->bmBits;
    BYTE *dp2 = dst->bmBits;
    int xfactor;
    int yfactor;

    if (dst->bmWidth == 0 || dst->bmHeight == 0)
        return TRUE;

    if (dst->bmBytesPerPixel != src->bmBytesPerPixel)
        return FALSE;

    xfactor = muldiv64 (src->bmWidth, 65536, dst->bmWidth);         /* scaled by 65536 */
    yfactor = muldiv64 (src->bmHeight, 65536, dst->bmHeight);       /* scaled by 65536 */

    switch (dst->bmBytesPerPixel) {
    case 1:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
#if 0 // defined(__GNUC__) && defined(i386)
                while (x < dst->bmWidth - 8) {
                    /* This saves just a couple of cycles per */
                    /* pixel on a 486, but I couldn't resist. */
                    __asm__ __volatile__("movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 1 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 2 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 3 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 4 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 5 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 6 (%1, %2)\n\t"
                                         "movl %4, %%eax\n\t"
                                         "shrl $16, %%eax\n\t"
                                         "addl %5, %4\n\t"
                                         "movb (%3, %%eax), %%al\n\t"
                                         "movb %%al, 7 (%1, %2)\n\t"
                                         :        /* output */
                                         :        /* input */
                                         "ax"(0), "r"(dp2), "r"(x), "r"(dp1),
                                         "r"(sx), "r"(xfactor)
                                         :"ax", "4"
                    );
                    *(dp2 + x) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 1) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 2) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 3) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 4) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 5) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 6) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    *(dp2 + x + 7) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    x += 8;
                }
#endif
                while (x < dst->bmWidth) {
                    *(dp2 + x) = *(dp1 + (sx >> 16));
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 2:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned short *) (dp2 + x * 2) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 3:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned short *) (dp2 + x * 3) =
                        *(unsigned short *) (dp1 + (sx >> 16) * 3);
                    *(unsigned char *) (dp2 + x * 3 + 2) =
                        *(unsigned char *) (dp1 + (sx >> 16) * 3 + 2);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    case 4:
        {
            int y, sy;
            sy = 0;
            for (y = 0; y < dst->bmHeight;) {
                int sx = 0;
                BYTE *dp2old = dp2;
                int x;
                x = 0;
                /* This can be greatly optimized with loop */
                /* unrolling; omitted to save space. */
                while (x < dst->bmWidth) {
                    *(unsigned *) (dp2 + x * 4) =
                        *(unsigned *) (dp1 + (sx >> 16) * 4);
                    sx += xfactor;
                    x++;
                }
                dp2 += dst->bmPitch;
                y++;
                while (y < dst->bmHeight) {
                    int l;
                    int syint = sy >> 16;
                    sy += yfactor;
                    if ((sy >> 16) != syint)
                        break;
                    /* Copy identical lines. */
                    l = dp2 - dp2old;
                    memcpy(dp2, dp2old, l);
                    dp2old = dp2;
                    dp2 += l;
                    y++;
                }
                dp1 = src->bmBits + (sy >> 16) * src->bmPitch;
            }
        }
    break;
    }

    return TRUE;
}

gal_pixel GUIAPI GetPixelInBitmap (const BITMAP* bmp, int x, int y)
{
    BYTE* dst;

    if (x < 0 || y < 0 || x >= bmp->bmWidth || y >= bmp->bmHeight)
        return 0;

    dst = bmp->bmBits + y * bmp->bmPitch + x * bmp->bmBytesPerPixel;
    return _mem_get_pixel (dst, bmp->bmBytesPerPixel);
}

BOOL GUIAPI SetPixelInBitmap (const BITMAP* bmp, int x, int y, gal_pixel pixel)
{
    BYTE* dst;

    if (x < 0 || y < 0 || x >= bmp->bmWidth || y >= bmp->bmHeight)
        return FALSE;

    dst = bmp->bmBits + y * bmp->bmPitch + x * bmp->bmBytesPerPixel;
    _mem_set_pixel (dst, bmp->bmBytesPerPixel, pixel);
    return TRUE;
}

