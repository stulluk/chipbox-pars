/* 
** $Id: window.c,v 1.146 2003/09/22 01:21:19 snig Exp $
**
** window.c: The Window module.
**
** Copyright (C) 2003 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999.04.19
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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "control.h"
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "menu.h"
#include "ctrlclass.h"

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
#include "client.h"
#endif

/* added by xm.chen */
#include <fcntl.h> 
#include <linux/ioctl.h>
typedef struct tagRECT
{
	unsigned int left;
	unsigned int top;
	unsigned int right;
	unsigned int bottom;
} CSRECT;

typedef struct _2d_scalor_params{
	CSRECT src_rect;
	unsigned int src_width;
	unsigned int src_height;
	int src_color_format;
	unsigned int src_pitch_line;
	unsigned int src_bits_pixel;
	unsigned int src_phy_addr;

	CSRECT dst_rect;
	unsigned int dst_width;
	unsigned int dst_height;
	int dst_color_format;
	unsigned int dst_pitch_line;
	unsigned int dst_bits_pixel;
	unsigned int dst_phy_addr;

}gfx2d_scalor_params;

#define FBIO_GFX_FLIP _IOW('F', 0x51, gfx2d_scalor_params)
/* added by xm.chen over */

// function defined in menu module.
void DrawMenuBarHelper (const MAINWIN *pWin, HDC hdc, const RECT* pClipRect);

// functions defined in caret module.
BOOL BlinkCaret (HWND hWnd);
void GetCaretBitmaps (PCARETINFO pCaretInfo);

// this message will auto-repeat when MSG_IDLE received
static MSG sg_repeat_msg = {HWND_DESKTOP, 0, 0, 0};

void GUIAPI SetAutoRepeatMessage (HWND hwnd, int msg, WPARAM wParam, LPARAM lParam)
{
    sg_repeat_msg.hwnd = hwnd;
    sg_repeat_msg.message  = msg;
    sg_repeat_msg.wParam = wParam;
    sg_repeat_msg.lParam = lParam;
}

static void RecalcClientArea (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;
    RECT rcWin, rcClient;
        
    memcpy (&rcWin, &pWin->left, sizeof (RECT));
    memcpy (&rcClient, &pWin->cl, sizeof (RECT));

    if (SendAsyncMessage (hWnd, MSG_SIZECHANGED, 
            (WPARAM)&rcWin, (LPARAM)&rcClient))
        memcpy (&pWin->cl, &rcClient, sizeof(RECT));
}

static PCONTROL wndMouseInWhichControl (PMAINWIN pWin, int x, int y, 
                        int* UndHitCode)
{
    PCONTROL pCtrl;
    int hitcode;
   
    pCtrl = (PCONTROL)pWin->hPrimitive;
    if (pCtrl) {
        if (pCtrl->primitive) {
            if (UndHitCode)
                *UndHitCode = HT_CLIENT;
            return pCtrl;
        }
        else {
            hitcode = SendAsyncMessage ((HWND)pCtrl, MSG_HITTEST, 
                                        (WPARAM)x, (LPARAM)y);
            if (hitcode != HT_OUT && hitcode != HT_TRANSPARENT) {
                if (UndHitCode)
                    *UndHitCode = hitcode;
                return pCtrl;
            }
        }
    }

    pCtrl = (PCONTROL)(pWin->hFirstChild);
    while (pCtrl) {
        if ((pCtrl->dwStyle & WS_VISIBLE) 
                && PtInRect ((PRECT)(&pCtrl->left), x, y)) {
            hitcode = SendAsyncMessage((HWND)pCtrl, MSG_HITTEST, 
                                        (WPARAM)x, (LPARAM)y);
            if (hitcode != HT_OUT && hitcode != HT_TRANSPARENT) {
                if (UndHitCode)
                    *UndHitCode = hitcode;
                return pCtrl;
            }
        }

        pCtrl = pCtrl->next;
    }

    return NULL;
}

// NOTE:
// this function is CONTROL mouse messages handler,
// can automaticly capture mouse depend on HITTEST code.
//
static int DefaultMouseMsgHandler (PMAINWIN pWin, int message, 
                           WPARAM flags, int x, int y)
{
    static PMAINWIN pCapturedWin = NULL;
    static PCONTROL pCaptured = NULL;
    PCONTROL pUnderPointer;
    int CapHitCode = HT_UNKNOWN;
    int UndHitCode = HT_UNKNOWN;
    int cx = 0, cy = 0;

    if (message == MSG_WINDOWCHANGED) {
        POINT mousePos;
        
        pCaptured = NULL;
        CapHitCode = HT_UNKNOWN;

        GetCursorPos (&mousePos);
        PostMessage (HWND_DESKTOP, MSG_MOUSEMOVE, 0, MAKELONG (mousePos.x, mousePos.y));
        return 0;
    }

    if (pCaptured) {
        // convert to parent window's client coordinates.
        ScreenToClient ((HWND)pCapturedWin, &x, &y);

        CapHitCode = SendAsyncMessage((HWND)pCaptured, MSG_HITTEST,
                                        (WPARAM)x, (LPARAM)y);
        
        pUnderPointer = NULL;
    }
    else {
        pUnderPointer = wndMouseInWhichControl (pWin, x, y, &UndHitCode);
        if (pUnderPointer && (pUnderPointer->dwStyle & WS_DISABLED))
            pUnderPointer = NULL;

        if (pUnderPointer) {
            cx = x - pUnderPointer->cl;
            cy = y - pUnderPointer->ct;
        }
    }

    switch (message) {
        case MSG_MOUSEMOVE:
            if (pCaptured)
                PostMessage((HWND)pCaptured, MSG_NCMOUSEMOVE, 
                                CapHitCode, MAKELONG (x, y));
            else {
                if (pWin->hOldUnderPointer != (HWND)pUnderPointer) {
                    if (pWin->hOldUnderPointer) {
                        PostMessage ((HWND)pWin->hOldUnderPointer,
                            MSG_MOUSEMOVEIN, FALSE, (LPARAM)pUnderPointer);
                        PostMessage ((HWND)pWin->hOldUnderPointer,
                            MSG_NCMOUSEMOVE, HT_OUT, MAKELONG (x, y));
                    }

                    if (pUnderPointer)
                        PostMessage ((HWND)pUnderPointer,
                            MSG_MOUSEMOVEIN, TRUE, (LPARAM)pWin->hOldUnderPointer);

                    pWin->hOldUnderPointer = (HWND)pUnderPointer;
                }

                if (pUnderPointer == NULL) {
                    pWin->hOldUnderPointer = 0;
                    break;
                }

                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    SetCursor (GetSystemCursor (IDC_ARROW));
                    break;
                }

                if (UndHitCode == HT_CLIENT) {
                    PostMessage ((HWND)pUnderPointer,
                                    MSG_SETCURSOR, 0, MAKELONG (cx, cy));

                    PostMessage((HWND)pUnderPointer, MSG_NCMOUSEMOVE, 
                            UndHitCode, MAKELONG (x, y));
                    PostMessage((HWND)pUnderPointer, MSG_MOUSEMOVE, 
                            flags, MAKELONG (cx, cy));
                }
                else
                {
                    PostMessage((HWND)pUnderPointer, MSG_NCSETCURSOR, 
                            UndHitCode, MAKELONG (x, y));
                    PostMessage((HWND)pUnderPointer, MSG_NCMOUSEMOVE, 
                            UndHitCode, MAKELONG (x, y));
                }
            }
        break;

        case MSG_LBUTTONDOWN:
        case MSG_RBUTTONDOWN:
            if (pUnderPointer) {
                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    Ping ();
                    break;
                }

                PostMessage ((HWND) pUnderPointer,
                        MSG_MOUSEACTIVE, UndHitCode, 0);
                
                if (UndHitCode != HT_CLIENT) {
                    if (UndHitCode & HT_NEEDCAPTURE)
                    {
                        SetCapture ((HWND)pUnderPointer);
                        pCapturedWin = pWin;
                        pCaptured = pUnderPointer;
                    }
                    else
                        pCaptured = NULL;

                    PostMessage ((HWND)pUnderPointer, message + MSG_NCMOUSEOFF,
                            UndHitCode, MAKELONG (x, y));
                }
                else {
                    PostMessage((HWND)pUnderPointer, message, 
                        flags, MAKELONG(cx, cy));
                    pCaptured = NULL;
                }
            }
            else {
                if (pWin->hActiveChild) {
                    PostMessage(pWin->hActiveChild, message + MSG_NCMOUSEOFF, 
                        HT_OUT, MAKELONG(x, y));
                }
            }
        break;

        case MSG_LBUTTONUP:
        case MSG_RBUTTONUP:
            if (pCaptured) {
                PostMessage ((HWND)pCaptured, message + MSG_NCMOUSEOFF,
                        CapHitCode, MAKELONG (x, y));
                ReleaseCapture ();
                pCapturedWin = NULL;
                pCaptured = NULL;
            }
            else if (pUnderPointer) {
                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    break;
                }

                if (UndHitCode == HT_CLIENT)
                    PostMessage((HWND)pUnderPointer, message, 
                            flags, MAKELONG (cx, cy));
                else
                    PostMessage((HWND)pUnderPointer, message + MSG_NCMOUSEOFF, 
                            UndHitCode, MAKELONG (x, y));
            }
            else {
                if (pWin->hActiveChild) {
                    PostMessage(pWin->hActiveChild, message + MSG_NCMOUSEOFF, 
                        HT_OUT, MAKELONG(x, y));
                }
            }
        break;
        
        case MSG_LBUTTONDBLCLK:
        case MSG_RBUTTONDBLCLK:
            if (pUnderPointer) {
                if (pUnderPointer->dwStyle & WS_DISABLED) {
                    Ping ();
                    break;
                }

                if (UndHitCode == HT_CLIENT)
                    PostMessage((HWND)pUnderPointer, message, 
                        flags, MAKELONG(cx, cy));
                else
                    PostMessage((HWND)pUnderPointer, message + MSG_NCMOUSEOFF, 
                        UndHitCode, MAKELONG (x, y));
            }
            else {
                if (pWin->hActiveChild) {
                    PostMessage(pWin->hActiveChild, message + MSG_NCMOUSEOFF, 
                        HT_OUT, MAKELONG(x, y));
                }
            }
        break;

    }

    return 0;
}

static inline int wndGetBorder (const MAINWIN* pWin)
{
    if (pWin->dwStyle & WS_BORDER) 
        return GetMainWinMetrics(MWM_BORDER);
    else if (pWin->dwStyle & WS_THICKFRAME)
        return GetMainWinMetrics(MWM_THICKFRAME);
    else if (pWin->dwStyle & WS_THINFRAME)
        return GetMainWinMetrics (MWM_THINFRAME);

    return 0;
}


static BOOL wndGetVScrollBarRect (const MAINWIN* pWin, 
                RECT* rcVBar)
{
    if (pWin->dwStyle & WS_VSCROLL) {
        int iBorder = wndGetBorder (pWin);

        rcVBar->left = pWin->right - GetMainWinMetrics (MWM_CXVSCROLL) 
                        - iBorder;
        rcVBar->right = pWin->right - iBorder;

#ifdef _FLAT_WINDOW_STYLE
        rcVBar->top  = pWin->ct - 1;
#else
        rcVBar->top  = pWin->ct;
#endif
        rcVBar->bottom = pWin->bottom - iBorder;

        if (pWin->dwStyle & WS_HSCROLL && !(pWin->hscroll.status & SBS_HIDE))
            rcVBar->bottom -= GetMainWinMetrics (MWM_CYHSCROLL);
        
#ifdef _FLAT_WINDOW_STYLE
        if (iBorder > 0)
            OffsetRect (rcVBar, 1, 0);
#endif

        return TRUE;
    }
    
    return FALSE;
}

static BOOL wndGetHScrollBarRect (const MAINWIN* pWin, 
                RECT* rcHBar)
{
    if (pWin->dwStyle & WS_HSCROLL) {
        int iBorder = wndGetBorder (pWin);

        rcHBar->top = pWin->bottom - GetMainWinMetrics (MWM_CYHSCROLL)
                        - iBorder;
        rcHBar->bottom = pWin->bottom - iBorder;

#ifdef _FLAT_WINDOW_STYLE
        rcHBar->left  = pWin->cl - 1;
#else
        rcHBar->left  = pWin->cl;
#endif
        rcHBar->right = pWin->right - iBorder;

        if (pWin->dwStyle & WS_VSCROLL && !(pWin->vscroll.status & SBS_HIDE))
            rcHBar->right -= GetMainWinMetrics (MWM_CXVSCROLL);

#ifdef _FLAT_WINDOW_STYLE
        if (iBorder > 0)
            OffsetRect (rcHBar, 0, 1);
#endif

        return TRUE;
    }
    
    return FALSE;
    
}

static int wndGetHScrollBarPos (PMAINWIN pWin, int x, int y)
{
    RECT rcBar;
    RECT rcArea;

    if (pWin->hscroll.status & SBS_DISABLED)
        return SBPOS_UNKNOWN;

    wndGetHScrollBarRect (pWin, &rcBar);

    if (!PtInRect (&rcBar, x, y))
        return SBPOS_UNKNOWN;

    rcArea.top  = rcBar.top;
    rcArea.bottom = rcBar.bottom;

    // Left arrow area
    rcArea.left = rcBar.left;
    rcArea.right = rcArea.left + GetMainWinMetrics (MWM_CXHSCROLL);

    if (PtInRect (&rcArea, x, y))
        return SBPOS_LEFTARROW;

    // Right arrow area
    rcArea.left = rcBar.right - GetMainWinMetrics (MWM_CXHSCROLL);
    rcArea.right = rcBar.right;

    if (PtInRect (&rcArea, x, y))
        return SBPOS_RIGHTARROW;


    if (x < (rcBar.left + pWin->hscroll.barStart 
            + GetMainWinMetrics (MWM_CXHSCROLL)))
        return SBPOS_LEFTSPACE;

    if (x > (rcBar.left + pWin->hscroll.barStart + pWin->hscroll.barLen
            + GetMainWinMetrics (MWM_CXHSCROLL)))
        return SBPOS_RIGHTSPACE;

    return SBPOS_THUMB;
}

static int wndGetVScrollBarPos (PMAINWIN pWin, int x, int y)
{
    RECT rcBar;
    RECT rcArea;

    if (pWin->vscroll.status & SBS_DISABLED)
        return SBPOS_UNKNOWN;

    wndGetVScrollBarRect (pWin, &rcBar);

    if (!PtInRect (&rcBar, x, y))
        return SBPOS_UNKNOWN;

    rcArea.left  = rcBar.left;
    rcArea.right = rcBar.right;

    // Left arrow area
    rcArea.top = rcBar.top;
    rcArea.bottom = rcArea.top + GetMainWinMetrics (MWM_CYVSCROLL);

    if (PtInRect (&rcArea, x, y))
        return SBPOS_UPARROW;

    // Right arrow area
    rcArea.top = rcBar.bottom - GetMainWinMetrics (MWM_CYVSCROLL);
    rcArea.bottom = rcBar.bottom;

    if (PtInRect (&rcArea, x, y))
        return SBPOS_DOWNARROW;

    if (y < (rcBar.top + pWin->vscroll.barStart
            + GetMainWinMetrics (MWM_CYVSCROLL)))
        return SBPOS_UPSPACE;

    if (y > (rcBar.top + pWin->vscroll.barStart + pWin->vscroll.barLen
            + GetMainWinMetrics (MWM_CYVSCROLL)))
        return SBPOS_DOWNSPACE;

    return SBPOS_THUMB;
}

#ifndef _FLAT_WINDOW_STYLE
static BOOL sbGetSBarArrowPos (PMAINWIN pWin, int location, 
                            int* x, int* y, int* w, int* h)
{
    RECT rcBar;

    if (location < SBPOS_UPARROW) 
        wndGetHScrollBarRect (pWin, &rcBar);
    else
        wndGetVScrollBarRect (pWin, &rcBar);

    *w = GetMainWinMetrics (MWM_CXHSCROLL);
    *h = GetMainWinMetrics (MWM_CYHSCROLL);
    switch (location) {
        case SBPOS_LEFTARROW:
            *x = rcBar.left;
            *y = rcBar.top;
        break;
        
        case SBPOS_RIGHTARROW:
            *x = rcBar.right - GetMainWinMetrics (MWM_CXHSCROLL) ;
            *y = rcBar.top;
        break;

        case SBPOS_UPARROW:
            *x = rcBar.left;
            *y = rcBar.top;
        break;

        case SBPOS_DOWNARROW:
            *x = rcBar.left;
            *y = rcBar.bottom - GetMainWinMetrics (MWM_CYVSCROLL);
        break;

        default:
        return FALSE;
    }

    *x -= pWin->left;
    *y -= pWin->top;

    return TRUE;
}

static BOOL sbGetButtonPos (PMAINWIN pWin, int location, 
                            int* x, int* y, int* w, int* h)
{
    RECT rc;
    int iBorder;
    int iCaption = 0, bCaption;
    int iIconX = 0;
    int iIconY = 0;

    // scroll bar position
    if (location & SBPOS_MASK)
        return sbGetSBarArrowPos (pWin, location, x, y, w, h);

    iBorder = wndGetBorder (pWin);

    if (!(pWin->dwStyle & WS_CAPTION))
         return FALSE;

    if (pWin->hIcon)
    {
        iIconX = GetMainWinMetrics(MWM_ICONX);
        iIconY = GetMainWinMetrics(MWM_ICONY);
    }

    iCaption = GetMainWinMetrics(MWM_CAPTIONY);
    bCaption = iBorder + iCaption - 1;

    rc.left = iBorder;
    rc.top = iBorder;
    rc.right = pWin->right - pWin->left - iBorder;
    rc.bottom = iBorder + iCaption;
                    
    // close button
    *x = rc.right - GetMainWinMetrics (MWM_SB_WIDTH);
    *y = GetMainWinMetrics (MWM_SB_HEIGHT);
    *w = GetMainWinMetrics (MWM_SB_WIDTH);
    if (*y < bCaption) {
        *y = iBorder + ((bCaption - *y)>>1);
        *h = GetMainWinMetrics (MWM_SB_HEIGHT);
    }
    else {
        *y = iBorder;
        *h = iIconY;
    }

    if (!(pWin->dwExStyle & WS_EX_NOCLOSEBOX)) {
        if (location == HT_CLOSEBUTTON)
            return TRUE;

        *x -= GetMainWinMetrics (MWM_SB_WIDTH);
        *x -= GetMainWinMetrics (MWM_SB_INTERX)<<1;
    }

    if (pWin->dwStyle & WS_MAXIMIZEBOX) {
        // restore/maximize button
        if (location == HT_MAXBUTTON)
            return TRUE;

        *x -= GetMainWinMetrics (MWM_SB_WIDTH);
        *x -= GetMainWinMetrics (MWM_SB_INTERX);
    }

    if (pWin->dwStyle & WS_MINIMIZEBOX) {
        // minimize button.
        if (location == HT_MINBUTTON)
            return TRUE;
    }

    return FALSE;
}

static BOOL sbDownButton (PMAINWIN pWin, int downCode)
{
    HDC hdc;
    int x, y, w, h;
    
    if (!sbGetButtonPos (pWin, downCode, &x, &y, &w, &h))
        return FALSE;

    hdc = GetDC ((HWND)pWin);

    w += x - 1;
    h += y - 1;
#if 0
    Draw3DDownFrame (hdc, x, y, w, h, 0);
#else
    SetPenColor (hdc, GetWindowElementColor (WEC_FLAT_BORDER));
    Rectangle (hdc, x, y, w, h);
    SetPenColor (hdc, GetWindowElementColor (BKC_CONTROL_DEF));
    Rectangle (hdc, x + 1, y + 1, w - 1, h - 1);
#endif
    ReleaseDC (hdc);

    return TRUE;
}

static BOOL sbUpButton (PMAINWIN pWin, int downCode)
{
    HDC hdc;
    int x, y, w, h;
    PBITMAP bmp = NULL;
    int xo = 0, yo = 0, bw = 0, bh = 0;
    
    if (!sbGetButtonPos (pWin, downCode, &x, &y, &w, &h))
        return FALSE;

    hdc = GetDC ((HWND)pWin);
    switch (downCode) {
        case HT_MAXBUTTON:
            bmp = GetSystemBitmap (SYSBMP_CAPBTNS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight;
            xo = yo = 0;
            break;
        case HT_MINBUTTON:
            bmp = GetSystemBitmap (SYSBMP_CAPBTNS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight;
            xo = bw; yo = 0;
            break;
        case HT_CLOSEBUTTON:
            bmp = GetSystemBitmap (SYSBMP_CAPBTNS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight;
            xo = (bw << 1) + bw; yo = 0;
            break;

        case SBPOS_UPARROW:
            bmp = GetSystemBitmap (SYSBMP_ARROWS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight >> 1;
            xo = yo = 0;
            break;
        case SBPOS_DOWNARROW:
            bmp = GetSystemBitmap (SYSBMP_ARROWS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight >> 1;
            xo = bw; yo = 0;
            break;
        case SBPOS_LEFTARROW:
            bmp = GetSystemBitmap (SYSBMP_ARROWS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight >> 1;
            xo = bw << 1; yo = 0;
            break;
        case SBPOS_RIGHTARROW:
            bmp = GetSystemBitmap (SYSBMP_ARROWS);
            bw = bmp->bmWidth >> 2;
            bh = bmp->bmHeight >> 1;
            xo = (bw << 1) + bw; yo = 0;
            break;
    }

    if (bmp)
        FillBoxWithBitmapPart (hdc, x, y, bw, bh, 0, 0, bmp, xo, yo);

    ReleaseDC (hdc);

    return TRUE;
}

#else

#define sbDownButton(pWin, downCode)
#define sbUpButton(pWin, downCode)

#endif

int MenuBarHitTest (HWND hwnd, int x, int y);

static void wndTrackMenuBarOnMouseMove(PMAINWIN pWin, int message, 
                                    int location, int x, int y)
{
    PMENUBAR pMenuBar;
    int oldBarItem;
    int barItem;

    pMenuBar = (PMENUBAR)(pWin->hMenu);
    if (!pMenuBar) return;

    oldBarItem = pMenuBar->hilitedItem;
    
    if (location == HT_OUT) {
        if (oldBarItem >= 0)
            HiliteMenuBarItem ((HWND)pWin, oldBarItem, HMF_DEFAULT);
        return;
    }

    barItem = MenuBarHitTest ((HWND)pWin, x, y);

    if (barItem != oldBarItem) {

        if (oldBarItem >= 0)
            HiliteMenuBarItem ((HWND)pWin, oldBarItem, HMF_DEFAULT);

        if (barItem >= 0) {
            HiliteMenuBarItem ((HWND)pWin, barItem, HMF_UPITEM);
            pMenuBar->hilitedItem = barItem;
            pMenuBar->hiliteFlag = HMF_UPITEM;
        }
        else
            pMenuBar->hilitedItem = -1;
    }
}

static BOOL wndHandleHScrollBar (PMAINWIN pWin, int message, int x, int y)
{
    static int downPos = SBPOS_UNKNOWN;
    static int movePos = SBPOS_UNKNOWN;
    static int sbCode;
    static int oldBarStart;
    static int oldThumbPos;
    static int oldx;

    int curPos;
    RECT rcBar;

    wndGetHScrollBarRect (pWin, &rcBar);
    rcBar.left += GetMainWinMetrics (MWM_CXHSCROLL);
    rcBar.right -= GetMainWinMetrics (MWM_CXHSCROLL);

    curPos = wndGetHScrollBarPos (pWin, x, y);
    if (curPos == SBPOS_UNKNOWN && downPos == SBPOS_UNKNOWN)
        return FALSE;
    
    switch( message )
    {
        case MSG_NCLBUTTONDOWN:
            oldBarStart = pWin->hscroll.barStart;
            oldThumbPos = pWin->hscroll.curPos;
            oldx = x;

            downPos = curPos;
            movePos = curPos;
            if (curPos == SBPOS_LEFTARROW) {
                sbDownButton (pWin, curPos);
                if (pWin->hscroll.curPos == pWin->hscroll.minPos)
                    break;

                sbCode = SB_LINELEFT;
            }
            else if (curPos == SBPOS_RIGHTARROW) {
                sbDownButton (pWin, curPos);
                if (pWin->hscroll.curPos == pWin->hscroll.maxPos)
                    break;
                
                sbCode = SB_LINERIGHT;
            }
            else if (curPos == SBPOS_LEFTSPACE) {
                if (pWin->hscroll.curPos == pWin->hscroll.minPos)
                    break;

                sbCode = SB_PAGELEFT;
            }
            else if (curPos == SBPOS_RIGHTSPACE) {
                if (pWin->hscroll.curPos == pWin->hscroll.maxPos)
                    break;
                
                sbCode = SB_PAGERIGHT;
            }
            else if (curPos == SBPOS_THUMB) {
                sbCode = SB_THUMBTRACK;
                break;
            }
            SendNotifyMessage ((HWND)pWin, MSG_HSCROLL, sbCode, 0);
            SetAutoRepeatMessage ((HWND)pWin, MSG_HSCROLL, sbCode, 0);
        break;

        case MSG_NCLBUTTONUP:
            if (sbCode == SB_THUMBTRACK && downPos == SBPOS_THUMB) {
                int newBarStart = oldBarStart + x - oldx;
                int newThumbPos = newBarStart
                    * (pWin->hscroll.maxPos - pWin->hscroll.minPos + 1) 
                    / (rcBar.right - rcBar.left) + pWin->hscroll.minPos;
                    
                if (newThumbPos != oldThumbPos
                    && newThumbPos >= pWin->hscroll.minPos
                    && newThumbPos <= pWin->hscroll.maxPos)
                    SendNotifyMessage ((HWND)pWin,
                        MSG_HSCROLL, SB_THUMBPOSITION, newThumbPos);

                downPos = SBPOS_UNKNOWN;
                movePos = SBPOS_UNKNOWN;
                break;
            }
            
            if (downPos != SBPOS_UNKNOWN) {
                sbUpButton (pWin, curPos);
                SendNotifyMessage ((HWND)pWin, MSG_HSCROLL, SB_ENDSCROLL, 0);
                SetAutoRepeatMessage (HWND_DESKTOP, 0, 0, 0);
            }

            downPos = SBPOS_UNKNOWN;
            movePos = SBPOS_UNKNOWN;
        break;
    
        case MSG_NCMOUSEMOVE:
            if (sbCode == SB_THUMBTRACK && downPos == SBPOS_THUMB) {
                int newBarStart = oldBarStart + x - oldx;
                int newThumbPos = newBarStart
                    * (pWin->hscroll.maxPos - pWin->hscroll.minPos + 1) 
                    / (rcBar.right - rcBar.left) + pWin->hscroll.minPos;
                    
                if (newThumbPos != oldThumbPos
                        && newThumbPos >= pWin->hscroll.minPos
                        && newThumbPos <= pWin->hscroll.maxPos) {
                    SendNotifyMessage ((HWND)pWin,
                        MSG_HSCROLL, SB_THUMBTRACK, newThumbPos);
                    oldThumbPos = newThumbPos;
                }
                movePos = curPos;
                break;
            }

            if (movePos == downPos && curPos != downPos)
                sbUpButton (pWin, downPos);
            else if (movePos != downPos && curPos == downPos)
                sbDownButton (pWin, downPos);
            movePos = curPos;
        break;
    }

    return TRUE;
}

static BOOL wndHandleVScrollBar (PMAINWIN pWin, int message, int x, int y)
{
    static int downPos = SBPOS_UNKNOWN;
    static int movePos = SBPOS_UNKNOWN;
    static int sbCode;
    static int oldBarStart;
    static int oldThumbPos;
    static int oldy;
    int curPos;
    RECT rcBar;
    int newBarStart;
    int newThumbPos;

    wndGetVScrollBarRect (pWin, &rcBar);
    rcBar.top += GetMainWinMetrics (MWM_CYVSCROLL);
    rcBar.bottom -= GetMainWinMetrics (MWM_CYVSCROLL);

    curPos = wndGetVScrollBarPos (pWin, x, y);

    if (curPos == SBPOS_UNKNOWN && downPos == SBPOS_UNKNOWN)
        return FALSE;
    
    switch (message)
    {
        case MSG_NCLBUTTONDOWN:
            oldBarStart = pWin->vscroll.barStart;
            oldThumbPos = pWin->vscroll.curPos;
            oldy = y;
            
            downPos = curPos;
            movePos = curPos;
            if (curPos == SBPOS_UPARROW) {
                sbDownButton (pWin, curPos);
                if (pWin->vscroll.curPos == pWin->vscroll.minPos)
                    break;

                sbCode = SB_LINEUP;
            }
            else if (curPos == SBPOS_DOWNARROW) {
                sbDownButton (pWin, curPos);
                if (pWin->vscroll.curPos == pWin->vscroll.maxPos)
                    break;

                sbCode = SB_LINEDOWN;
            }
            else if (curPos == SBPOS_UPSPACE) {
                if (pWin->vscroll.curPos == pWin->vscroll.minPos)
                    break;

                sbCode = SB_PAGEUP;
            }
            else if (curPos == SBPOS_DOWNSPACE) {
                if (pWin->vscroll.curPos == pWin->vscroll.maxPos)
                    break;

                sbCode = SB_PAGEDOWN;
            }
            else if (curPos == SBPOS_THUMB) {
                sbCode = SB_THUMBTRACK;
                break;
            }
            SendNotifyMessage ((HWND)pWin, MSG_VSCROLL, sbCode, 0);
            SetAutoRepeatMessage ((HWND)pWin, MSG_VSCROLL, sbCode, 0);
        break;

        case MSG_NCLBUTTONUP:
            if (sbCode == SB_THUMBTRACK && downPos == SBPOS_THUMB) {
                newBarStart = oldBarStart + y - oldy;
                newThumbPos = newBarStart
                    * (pWin->vscroll.maxPos - pWin->vscroll.minPos + 1) 
                    / (rcBar.bottom - rcBar.top) + pWin->vscroll.minPos;
                    
                if (newThumbPos != oldThumbPos
                        && newThumbPos >= pWin->vscroll.minPos
                        && newThumbPos <= pWin->vscroll.maxPos)
                    SendNotifyMessage ((HWND)pWin,
                        MSG_VSCROLL, SB_THUMBPOSITION, newThumbPos);
                        
                downPos = SBPOS_UNKNOWN;
                movePos = SBPOS_UNKNOWN;
                break;
            }

            if (downPos != SBPOS_UNKNOWN) {
                sbUpButton (pWin, curPos);
                SendNotifyMessage ((HWND)pWin, MSG_VSCROLL, SB_ENDSCROLL, 0);
                SetAutoRepeatMessage (HWND_DESKTOP, 0, 0, 0);
            }

            downPos = SBPOS_UNKNOWN;
            movePos = SBPOS_UNKNOWN;
        break;
    
        case MSG_NCMOUSEMOVE:
            if (sbCode == SB_THUMBTRACK && downPos == SBPOS_THUMB) {
                newBarStart = oldBarStart + y - oldy;
                newThumbPos = newBarStart
                        * (pWin->vscroll.maxPos - pWin->vscroll.minPos + 1) 
                        / (rcBar.bottom - rcBar.top) + pWin->vscroll.minPos;
                    
                if (newThumbPos != oldThumbPos
                    && newThumbPos >= pWin->vscroll.minPos
                    && newThumbPos <= pWin->vscroll.maxPos) {
                    SendNotifyMessage ((HWND)pWin,
                        MSG_VSCROLL, SB_THUMBTRACK, newThumbPos);
                    oldThumbPos = newThumbPos;
                }
                movePos = curPos;
                break;
            }
            
            if (movePos == downPos && curPos != downPos)
                sbUpButton (pWin, downPos);
            else if (movePos != downPos && curPos == downPos)
                sbDownButton (pWin, downPos);
            movePos = curPos;
        break;
    }

    return TRUE;
}

// this function is CONTROL safe.
static int DefaultNCMouseMsgHandler(PMAINWIN pWin, int message, 
                           int location, int x, int y)
{
    static PMAINWIN downWin  = NULL;
    static int downCode = HT_UNKNOWN;
    static int moveCode = HT_UNKNOWN;
#ifdef _MOVE_WINDOW_BY_MOUSE
    static int oldx, oldy;
    static RECT rcWindow;
#endif

    int barItem;

    if (pWin->WinType == TYPE_MAINWIN && message == MSG_NCMOUSEMOVE)
        wndTrackMenuBarOnMouseMove(pWin, message, location, x, y);

    if ((pWin->dwStyle & WS_HSCROLL) 
            && wndHandleHScrollBar (pWin, message, x, y))
        return 0;
    
    if ((pWin->dwStyle & WS_VSCROLL)
            && wndHandleVScrollBar (pWin, message, x, y))
        return 0;

    switch( message )
    {
        case MSG_NCLBUTTONDOWN:
            if (location == HT_MENUBAR) {
                barItem = MenuBarHitTest ((HWND)pWin, x, y);
                if (barItem >= 0)
                    TrackMenuBar ((HWND)pWin, barItem);

                return 0;
            }
#ifdef _MOVE_WINDOW_BY_MOUSE
            else if (location == HT_CAPTION) {
                GetWindowRect ((HWND)pWin, &rcWindow);
                SetPenColor (HDC_SCREEN, PIXEL_lightwhite);
                FocusRect (HDC_SCREEN, rcWindow.left, rcWindow.top,
                              rcWindow.right, rcWindow.bottom);
                oldx = x;
                oldy = y;
            }
#endif
            downCode = location;
            moveCode = location;
            downWin  = pWin;
            sbDownButton (pWin, downCode);
            break;

        case MSG_NCMOUSEMOVE:
            if (pWin->hOldUnderPointer && location == HT_OUT) {
                PostMessage (pWin->hOldUnderPointer,
                            MSG_MOUSEMOVEIN, FALSE, 0);
                PostMessage (pWin->hOldUnderPointer,
                            MSG_NCMOUSEMOVE, HT_OUT, MAKELONG (x, y));
                pWin->hOldUnderPointer = 0;
            }

            if (downCode != HT_UNKNOWN) { 
                if (downCode == HT_CAPTION && downWin == pWin) {
#ifdef _MOVE_WINDOW_BY_MOUSE
                    SetPenColor (HDC_SCREEN, PIXEL_lightwhite);
                    FocusRect (HDC_SCREEN, rcWindow.left, rcWindow.top,
                              rcWindow.right, rcWindow.bottom);
                    OffsetRect (&rcWindow, x - oldx, y - oldy);
                    FocusRect (HDC_SCREEN, rcWindow.left, rcWindow.top,
                              rcWindow.right, rcWindow.bottom);
                    
                    oldx = x;
                    oldy = y;
#endif
                }
                else if (moveCode == downCode && location != downCode) {
                    sbUpButton (pWin, downCode);
                    moveCode = location;
                }
                else if (moveCode != downCode && location == downCode) {
                    sbDownButton (pWin, downCode);
                    moveCode = location;
                }
            }
            break;

        case MSG_NCLBUTTONUP:
            if (downCode == HT_CAPTION) {
#ifdef _MOVE_WINDOW_BY_MOUSE
                SetPenColor (HDC_SCREEN, PIXEL_lightwhite);
                FocusRect (HDC_SCREEN, rcWindow.left, rcWindow.top,
                              rcWindow.right, rcWindow.bottom);
                    
                MoveWindow ((HWND)pWin, rcWindow.left,
                                        rcWindow.top,
                                        RECTW (rcWindow),
                                        RECTH (rcWindow),
                                        FALSE);
#endif
            }
            else if (location == downCode) {
                sbUpButton (pWin, downCode);
                switch (location) {
                    case HT_CLOSEBUTTON:
                        SendNotifyMessage ((HWND)pWin, MSG_CLOSE, 0, 0);
                    break;

                    case HT_MAXBUTTON:
                        SendNotifyMessage ((HWND)pWin, MSG_MAXIMIZE, 0, 0);
                    break;

                    case HT_MINBUTTON:
                        SendNotifyMessage ((HWND)pWin, MSG_MINIMIZE, 0, 0);
                    break;

                    case HT_ICON:
                        if (pWin->hSysMenu)
                            TrackPopupMenu (pWin->hSysMenu, 
                                TPM_SYSCMD, x, y, (HWND)pWin);
                    break;

                    case HT_CAPTION:
                    break;

                }
            }
            downCode = HT_UNKNOWN;
            moveCode = HT_UNKNOWN;
            downWin  = NULL;
            break;
            
        case MSG_NCRBUTTONUP:
            if (location == HT_CAPTION && pWin->hSysMenu)
                TrackPopupMenu (pWin->hSysMenu, TPM_SYSCMD, x, y, (HWND)pWin);
            break;
            
        case MSG_NCLBUTTONDBLCLK:
            if (location == HT_ICON)
                SendNotifyMessage ((HWND)pWin, MSG_CLOSE, 0, 0);
//            else if (location == HT_CAPTION)
//                SendNotifyMessage ((HWND)pWin, MSG_MAXIMIZE, 0, 0);
            break;

//        case MSG_NCRBUTTONDOWN:
//        case MSG_NCRBUTTONDBLCLK:
//            break;

    }

    return 0;
}

static int DefaultKeyMsgHandler (PMAINWIN pWin, int message, 
                           WPARAM wParam, LPARAM lParam)
{
// NOTE:
// Currently only handle one message.
// Should handle fowllowing messages:
//
// MSG_KEYDOWN,
// MSG_KEYUP,
// MSG_CHAR,
// MSG_SYSKEYDOWN,
// MSG_SYSCHAR.
    if (message == MSG_KEYDOWN || message == MSG_KEYUP 
        || message == MSG_CHAR) {
        // printf("DefaultKeyMsgHandler : message[0x%X], wParam[0x%X], lParam[0x%x]\n", message, wParam, lParam);
        if (pWin->hActiveChild
                && !(pWin->dwStyle & WS_DISABLED)) {
            SendMessage (pWin->hActiveChild, message, wParam, lParam);
        }
    }
    else if (message == MSG_SYSKEYUP) {
       if (pWin->WinType == TYPE_MAINWIN
                && !(pWin->dwStyle & WS_DISABLED))
           TrackMenuBar ((HWND)pWin, 0);
    }

    return 0;
}

static int DefaultCreateMsgHandler(PMAINWIN pWin, int message, 
                           WPARAM wParam, LPARAM lParam)
{

// NOTE:
// Currently does nothing.
// Should handle fowllowing messages:
//
// MSG_NCCREATE,
// MSG_CREATE,
// MSG_INITPANES,
// MSG_DESTROYPANES,
// MSG_DESTROY,
// MSG_NCDESTROY.

    return 0;
}

static void wndScrollBarPos (PMAINWIN pWin, BOOL bIsHBar, RECT* rcBar)
{
    UINT moveRange;
    PSCROLLBARINFO pSBar;

    if (bIsHBar)
        pSBar = &pWin->hscroll;
    else
        pSBar = &pWin->vscroll;

    if (pSBar->minPos == pSBar->maxPos) {
        pSBar->status |= SBS_HIDE;
        return;
    }

    if (bIsHBar)
        moveRange = RECTWP (rcBar) - (GetMainWinMetrics (MWM_CXHSCROLL)<<1);
    else
        moveRange = RECTHP (rcBar) - (GetMainWinMetrics (MWM_CYVSCROLL)<<1);

    if (pSBar->pageStep == 0) {
        pSBar->barLen = GetMainWinMetrics (MWM_DEFBARLEN);

        if (pSBar->barLen > moveRange)
            pSBar->barLen = GetMainWinMetrics (MWM_MINBARLEN);
    }
    else {
        pSBar->barLen = (int) (moveRange*pSBar->pageStep * 1.0f/
                               (pSBar->maxPos - pSBar->minPos + 1) + 0.5);
        if (pSBar->barLen < GetMainWinMetrics (MWM_MINBARLEN))
            pSBar->barLen = GetMainWinMetrics (MWM_MINBARLEN);
    }

    pSBar->barStart = (int) (moveRange*(pSBar->curPos - pSBar->minPos) * 1.0f/
                               (pSBar->maxPos - pSBar->minPos + 1) + 0.5);

    if (pSBar->barStart + pSBar->barLen > moveRange)
        pSBar->barStart = moveRange - pSBar->barLen;
    if (pSBar->barStart < 0)
        pSBar->barStart = 0;
}

// This function is CONTROL safe.
static void OnChangeSize(PMAINWIN pWin, PRECT pDestRect, PRECT pResultRect)
{
    int iBorder = 0;
    int iCaptionY = 0;
    int iIconX = 0;
    int iIconY = 0;
    int iMenuY = 0;

    iBorder = wndGetBorder (pWin);

    if( pWin->dwStyle & WS_CAPTION )
    {
        iCaptionY = GetMainWinMetrics(MWM_CAPTIONY);

        if (pWin->WinType == TYPE_MAINWIN && pWin->hIcon) {
            iIconX = GetMainWinMetrics(MWM_ICONX);
            iIconY = GetMainWinMetrics(MWM_ICONY);
        }
    }

    if (pWin->WinType == TYPE_MAINWIN && pWin->hMenu) {
        iMenuY = GetMainWinMetrics (MWM_MENUBARY);
        iMenuY += GetMainWinMetrics (MWM_MENUBAROFFY)<<1;
    }

    if (pDestRect) {
        int minWidth = 0, minHeight = 0;

        memcpy(&pWin->left, pDestRect, sizeof(RECT));

        minHeight = iMenuY + (iCaptionY<<1);
        if (pWin->dwStyle & WS_VSCROLL) {
            minWidth += GetMainWinMetrics (MWM_CXVSCROLL);
            minHeight += (GetMainWinMetrics (MWM_CYVSCROLL)<<1) +
                         (GetMainWinMetrics (MWM_MINBARLEN)<<1);
        }
        
        if (pWin->WinType == TYPE_MAINWIN)
            minWidth += GetMainWinMetrics (MWM_MINWIDTH);

        if (pWin->dwStyle & WS_HSCROLL) {
            minHeight += GetMainWinMetrics (MWM_CYHSCROLL);
            minWidth += (GetMainWinMetrics (MWM_CXHSCROLL)<<1) +
                        (GetMainWinMetrics (MWM_MINBARLEN)<<1);
        }

        if(minHeight > (pWin->bottom - pWin->top))
            pWin->bottom = pWin->top + minHeight;

        if(pWin->right < (pWin->left + minWidth))
            pWin->right = pWin->left + minWidth;

        if( pResultRect )
             memcpy(pResultRect, &pWin->left, sizeof(RECT));
    }

    memcpy(&pWin->cl, &pWin->left, sizeof(RECT));

    pWin->cl += iBorder;
    pWin->ct += iBorder;
    pWin->cr -= iBorder;
    pWin->cb -= iBorder;
    pWin->ct += iCaptionY;
    pWin->ct += iMenuY;
    
    if (pWin->dwStyle & WS_HSCROLL && !(pWin->hscroll.status & SBS_HIDE)) {
    
        RECT rcBar;
        wndGetHScrollBarRect (pWin, &rcBar);
        wndScrollBarPos (pWin, TRUE, &rcBar);

        pWin->cb -= GetMainWinMetrics (MWM_CYHSCROLL);
#ifdef _FLAT_WINDOW_STYLE
        if (iBorder > 0) pWin->cb ++;
#endif
    }
        
    if (pWin->dwStyle & WS_VSCROLL && !(pWin->vscroll.status & SBS_HIDE)) {
    
        RECT rcBar;
        wndGetVScrollBarRect (pWin, &rcBar);
        wndScrollBarPos (pWin, FALSE, &rcBar);

        pWin->cr -= GetMainWinMetrics (MWM_CXVSCROLL);
#ifdef _FLAT_WINDOW_STYLE
        if (iBorder > 0) pWin->cr ++;
#endif
    }
}

static void OnQueryNCRect(PMAINWIN pWin, PRECT pRC)
{
    memcpy(pRC, &pWin->left, sizeof(RECT));
}

static void OnQueryClientArea(PMAINWIN pWin, PRECT pRC)
{
    memcpy(pRC, &pWin->cl, sizeof(RECT));
}

int ClientWidthToWindowWidth (DWORD dwStyle, int cw)
{
    int iBorder = 0;
    int iScroll = 0;

    if (dwStyle & WS_BORDER) 
        iBorder = GetMainWinMetrics(MWM_BORDER);
    else if (dwStyle & WS_THICKFRAME)
        iBorder = GetMainWinMetrics(MWM_THICKFRAME);
    else if (dwStyle & WS_THINFRAME)
        iBorder = GetMainWinMetrics (MWM_THINFRAME);

    if (dwStyle & WS_VSCROLL)
        iScroll = GetMainWinMetrics (MWM_CXVSCROLL);
        
    return cw + (iBorder<<1) + iScroll;
}

int ClientHeightToWindowHeight (DWORD dwStyle, int ch, BOOL hasMenu)
{
    int iBorder  = 0;
    int iCaption = 0;
    int iScroll  = 0;
    int iMenu    = 0;

    if (dwStyle & WS_BORDER) 
        iBorder = GetMainWinMetrics(MWM_BORDER);
    else if (dwStyle & WS_THICKFRAME)
        iBorder = GetMainWinMetrics(MWM_THICKFRAME);
    else if (dwStyle & WS_THINFRAME)
        iBorder = GetMainWinMetrics (MWM_THINFRAME);

    if (dwStyle & WS_HSCROLL)
        iScroll = GetMainWinMetrics (MWM_CYHSCROLL);
        
    if (dwStyle & WS_CAPTION)
        iCaption = GetMainWinMetrics(MWM_CAPTIONY);

    if (hasMenu) {
        iMenu = GetMainWinMetrics (MWM_MENUBARY);
        iMenu += GetMainWinMetrics (MWM_MENUBAROFFY)<<1;
    }
    
    return ch + (iBorder<<1) + iCaption + iScroll + iMenu;
}

// this function is CONTROL safe.
static int HittestOnNClient (PMAINWIN pWin, int x, int y)
{
    RECT rcCaption, rcIcon, rcButton, rcMenuBar;
    int iBorder = wndGetBorder (pWin);
    int iCaption = 0;
    int iIconX = 0;
    int iIconY = 0;

    if (pWin->dwStyle & WS_HSCROLL && !(pWin->hscroll.status & SBS_HIDE)) {
    
        RECT rcBar;
        wndGetHScrollBarRect (pWin, &rcBar);

        if (PtInRect (&rcBar, x, y))
            return HT_HSCROLL;
    }
        
    if (pWin->dwStyle & WS_VSCROLL && !(pWin->vscroll.status & SBS_HIDE)) {
    
        RECT rcBar;
        wndGetVScrollBarRect (pWin, &rcBar);

        if (PtInRect (&rcBar, x, y)) {
            return HT_VSCROLL;
        }
    }
    
    if (pWin->dwStyle & WS_CAPTION) {
        iCaption = GetMainWinMetrics(MWM_CAPTIONY);

        if (pWin->WinType == TYPE_MAINWIN && pWin->hIcon) {
            iIconX = GetMainWinMetrics(MWM_ICONX);
            iIconY = GetMainWinMetrics(MWM_ICONY);
        }

        // Caption rect;
        rcCaption.left = pWin->left + iBorder;
        rcCaption.top = pWin->top + iBorder;
        rcCaption.right = pWin->right - iBorder;
        rcCaption.bottom = rcCaption.top + iCaption;
                        
        if (pWin->WinType == TYPE_MAINWIN && pWin->hIcon)
        { 
            rcIcon.left = rcCaption.left;
            rcIcon.top = rcCaption.top;
            rcIcon.right = rcIcon.left + iIconX;
            rcIcon.bottom = rcIcon.top + iIconY;
    
            if (PtInRect (&rcIcon, x, y))
                return HT_ICON;
        }
    
        rcButton.left = rcCaption.right - GetMainWinMetrics (MWM_SB_WIDTH);
        rcButton.top = rcCaption.top;
        rcButton.right = rcCaption.right;
        rcButton.bottom = rcCaption.top + GetMainWinMetrics (MWM_SB_HEIGHT);

        if (!(pWin->dwExStyle & WS_EX_NOCLOSEBOX)) {
            if (PtInRect (&rcButton, x, y))
                return HT_CLOSEBUTTON;

            rcButton.left -= GetMainWinMetrics (MWM_SB_WIDTH);
            rcButton.left -= GetMainWinMetrics (MWM_SB_INTERX)<<1;
        }
    
        if (pWin->dwStyle & WS_MAXIMIZEBOX) {
            rcButton.right = rcButton.left + GetMainWinMetrics (MWM_SB_WIDTH);
            if (PtInRect (&rcButton, x, y))
                return HT_MAXBUTTON;
    
            rcButton.left -= GetMainWinMetrics (MWM_SB_WIDTH);
            rcButton.left -= GetMainWinMetrics (MWM_SB_INTERX);
        }
    
        if (pWin->dwStyle & WS_MINIMIZEBOX) {
            rcButton.right = rcButton.left + GetMainWinMetrics (MWM_SB_WIDTH);
            if (PtInRect (&rcButton, x, y))
                return HT_MINBUTTON;
        }
    
        if (pWin->WinType == TYPE_MAINWIN && pWin->hMenu) {
            rcMenuBar.left = rcCaption.left;
            rcMenuBar.top = rcCaption.bottom + 1;
            rcMenuBar.right = rcCaption.right;
            rcMenuBar.bottom = rcMenuBar.top + GetMainWinMetrics (MWM_MENUBARY);
            rcMenuBar.bottom += GetMainWinMetrics (MWM_MENUBAROFFY)<<1;
                    
            if (PtInRect (&rcMenuBar, x, y))
                return HT_MENUBAR;
        }

        if (PtInRect (&rcCaption, x, y))
            return HT_CAPTION;
    }
    else {
        // Caption rect;
        rcCaption.left = pWin->left + iBorder;
        rcCaption.top = pWin->top + iBorder;
        rcCaption.right = pWin->right - iBorder;
        rcCaption.bottom = rcCaption.top;

        if (pWin->WinType == TYPE_MAINWIN && pWin->hMenu) {
            rcMenuBar.left = rcCaption.left;
            rcMenuBar.top = rcCaption.bottom + 1;
            rcMenuBar.right = rcCaption.right;
            rcMenuBar.bottom = rcMenuBar.top + GetMainWinMetrics (MWM_MENUBARY);
            rcMenuBar.bottom += GetMainWinMetrics (MWM_MENUBAROFFY)<<1;
                    
            if (PtInRect (&rcMenuBar, x, y))
                return HT_MENUBAR;
        }
    }

    return HT_BORDER;
}

extern HWND __mg_ime_wnd;
static void open_ime_window (PMAINWIN pWin, int message, HWND rev_hwnd)
{
#ifndef _LITE_VERSION
    if (__mg_ime_wnd && pWin) {
        BOOL is_edit = SendAsyncMessage ((HWND)pWin, MSG_DOESNEEDIME, 0, 0L);

        if (pWin->pMainWin && ((pWin->pMainWin->dwExStyle & WS_EX_TOOLWINDOW)
                           || ((HWND)(pWin->pMainWin) == __mg_ime_wnd)))
            return;

        if (message == MSG_SETFOCUS) {
            if (is_edit)
                SendNotifyMessage (__mg_ime_wnd, MSG_IME_OPEN, 0, 0);
            else
                SendNotifyMessage (__mg_ime_wnd, MSG_IME_CLOSE, 0, 0);
        }
        else if (message == MSG_KILLFOCUS && is_edit) {
            BOOL other_is_edit 
                = rev_hwnd && SendAsyncMessage (rev_hwnd, MSG_DOESNEEDIME, 0, 0L);
            if (!other_is_edit)
                SendNotifyMessage (__mg_ime_wnd, MSG_IME_CLOSE, 0, 0);
        }
    }
#elif !defined(_STAND_ALONE)
    if (!mgIsServer && pWin) {
        BOOL is_edit = SendAsyncMessage ((HWND)pWin, MSG_DOESNEEDIME, 0, 0L);
        REQUEST req;
        BOOL open = FALSE;

        if (pWin->pMainWin && (pWin->pMainWin->dwExStyle & WS_EX_TOOLWINDOW))
            return;

        req.id = REQID_OPENIMEWND;
        req.data = &open;
        req.len_data = sizeof (BOOL);
        if (message == MSG_SETFOCUS) {
            if (is_edit)
                open = TRUE;
            cli_request (&req, NULL, 0);
        }
        else if (message == MSG_KILLFOCUS && is_edit) {
            BOOL other_is_edit 
                = rev_hwnd && SendAsyncMessage (rev_hwnd, MSG_DOESNEEDIME, 0, 0L);
            if (!other_is_edit) {
                cli_request (&req, NULL, 0);
            }
        }
    }
#endif
}

static int DefaultPostMsgHandler(PMAINWIN pWin, int message,
                           WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case MSG_SETCURSOR:
//
// NOTE:
// this message is only implemented for main window.
// for CONTROL, must handle this message and should NOT 
// call default window procedure
// when handle MSG_SETCURSOR.
//
            if (wndMouseInWhichControl (pWin, LOSWORD(lParam), HISWORD(lParam), 
                    NULL))
                break;

            if (pWin->hCursor)
                SetCursor(pWin->hCursor);
        break;

        case MSG_NCSETCURSOR:
//
// NOTE:
// this message is only implemented for main window.
// for CONTROL, must handle this message and should NOT 
// call default window procedure
// when handle MSG_SETCURSOR.
//
            SetCursor (GetSystemCursor (IDC_ARROW));
        break;
        
        case MSG_HITTEST:
            if (PtInRect((PRECT)(&(pWin->cl)), (int)wParam, (int)lParam) )
                return HT_CLIENT;
            else
                return HittestOnNClient (pWin, 
                            (int)wParam, (int)lParam);
        break;

        case MSG_CHANGESIZE:
            OnChangeSize (pWin, (PRECT)wParam, (PRECT)lParam);
        break;

        case MSG_SIZECHANGING:
            memcpy ((PRECT)lParam, (PRECT)wParam, sizeof (RECT));
        break;
        
        case MSG_QUERYNCRECT:
            OnQueryNCRect(pWin, (PRECT)lParam);
        break;

        case MSG_QUERYCLIENTAREA:
            OnQueryClientArea(pWin, (PRECT)lParam);
        break;

        case MSG_SETFOCUS:
        case MSG_KILLFOCUS:
            if (pWin->hActiveChild)
                SendNotifyMessage (pWin->hActiveChild, message, 0, 0);
            open_ime_window (pWin, message, (HWND)wParam);
        break;
        
        case MSG_MOUSEACTIVE:
            if (pWin->WinType == TYPE_CONTROL 
                && !(pWin->dwStyle & WS_DISABLED)) {

                PCONTROL pCtrl = (PCONTROL)pWin;
                PCONTROL old_active = pCtrl->pParent->active;
               
                if (old_active != (PCONTROL)pWin) {
                    if (old_active) {
                        SendNotifyMessage ((HWND)old_active, MSG_ACTIVE, FALSE, 0);
                        SendNotifyMessage ((HWND)old_active, MSG_KILLFOCUS, (WPARAM)pWin, 0);
                    }

                    pCtrl->pParent->active = (PCONTROL)pWin;

                    SendNotifyMessage ((HWND)pWin, MSG_ACTIVE, TRUE, 0);
                    SendNotifyMessage ((HWND)pWin, MSG_SETFOCUS, (WPARAM)old_active, 0);
                }
            }
        break;
    }

    return 0;
}

// this function is CONTROL safe.
static void wndDrawNCArea (const MAINWIN* pWin, HDC hdc)
{
#ifdef _FLAT_WINDOW_STYLE
    int i, iBorder;
#endif

    // Draw window frame
    if (pWin->dwStyle & WS_BORDER) {
#ifdef _FLAT_WINDOW_STYLE
        iBorder = GetMainWinMetrics (MWM_BORDER);
        SetPenColor (hdc, GetWindowElementColor (WEC_FLAT_BORDER));
        for (i = 0; i < iBorder; i++)
            Rectangle (hdc, i, i, 
                      pWin->right - pWin->left - i - 1, 
                      pWin->bottom - pWin->top - i - 1);
#else

        if (pWin->dwStyle & WS_CHILD)
            Draw3DDownFrame(hdc, 
                   0, 0, 
                   pWin->right - pWin->left - 1, 
                   pWin->bottom - pWin->top - 1, 
                   PIXEL_invalid);
        else
            Draw3DUpFrame(hdc, 
                   0, 0, 
                   pWin->right - pWin->left, 
                   pWin->bottom - pWin->top, 
                   PIXEL_invalid);
#endif
    }
    else if ((pWin->dwStyle & WS_THICKFRAME) ||
            (pWin->dwStyle & WS_THINFRAME))
    {
       SetPenColor(hdc, GetWindowElementColor (WEC_FRAME_NORMAL));
       Rectangle(hdc, 0, 0, 
                      pWin->right - pWin->left - 1, 
                      pWin->bottom - pWin->top - 1);
    }

}

// this function is CONTROL safe.
static void wndDrawScrollBar (MAINWIN* pWin, HDC hdc)
{
    int iBorder, start = 0;
    RECT rcHBar, rcVBar;
    PBITMAP bmp;
    int xo, yo, bw, bh;
    
    iBorder = wndGetBorder (pWin);
   
    wndGetVScrollBarRect (pWin, &rcVBar);
    rcVBar.left -= pWin->left;
    rcVBar.top  -= pWin->top;
    rcVBar.right -= pWin->left;
    rcVBar.bottom -= pWin->top;
    wndGetHScrollBarRect (pWin, &rcHBar);
    rcHBar.left -= pWin->left;
    rcHBar.top  -= pWin->top;
    rcHBar.right -= pWin->left;
    rcHBar.bottom -= pWin->top;

    bmp = GetSystemBitmap (SYSBMP_ARROWS);
    bw = bmp->bmWidth >> 2;
    bh = bmp->bmHeight >> 1;

    if (pWin->dwStyle & WS_HSCROLL && !(pWin->hscroll.status & SBS_HIDE)) {
        int w = RECTW (rcHBar);
        if (pWin->dwStyle & WS_VSCROLL && !(pWin->vscroll.status & SBS_HIDE))
            w += GetMainWinMetrics (MWM_CXVSCROLL);

        SetBrushColor (hdc, GetWindowElementColor (BKC_CONTROL_DEF));

#ifdef _FLAT_WINDOW_STYLE
        FillBox (hdc, rcHBar.left, rcHBar.top, w, RECTH (rcHBar) - 1);
#else
        FillBox (hdc, rcHBar.left, rcHBar.top, w, RECTH (rcHBar));
#endif

        // draw left and right buttons.
        if (pWin->hscroll.status & SBS_DISABLED) {
            xo = bw << 1; yo = bh;
        }
        else {
            xo = bw << 1; yo = 0;
        }
        
        FillBoxWithBitmapPart (hdc, rcHBar.left, rcHBar.top, 
                                bw, bh, 0, 0, bmp, xo, yo);

        if (pWin->hscroll.status & SBS_DISABLED) {
            xo = (bw << 1) + bw; yo = bh;
        }
        else {
            xo = (bw << 1) + bw; yo = 0;
        }
        
        FillBoxWithBitmapPart (hdc, 
                        rcHBar.right - GetMainWinMetrics (MWM_CXHSCROLL), rcHBar.top,
                        bw, bh, 0, 0, bmp, xo, yo);

#ifdef _FLAT_WINDOW_STYLE
        SetPenColor (hdc, GetWindowElementColor (FGC_CONTROL_DEF));
        Rectangle (hdc, rcHBar.left, rcHBar.top, rcHBar.right, rcHBar.bottom - 1);
#endif
        // draw moving bar.
        start = rcHBar.left +
                    GetMainWinMetrics (MWM_CXHSCROLL) +
                    pWin->hscroll.barStart;

        if (start + pWin->hscroll.barLen > rcHBar.right)
            start = rcHBar.right - pWin->hscroll.barLen;

#ifdef _FLAT_WINDOW_STYLE
#ifdef _GRAY_SCREEN
        SetBrushColor (hdc, GetWindowElementColor (FGC_CONTROL_DEF));
        FillBox (hdc, start, rcHBar.top + 2,
                        pWin->hscroll.barLen,
                        RECTH (rcHBar) - 4);
#else
        SetBrushColor (hdc, PIXEL_lightgray);
        FillBox (hdc, start, rcHBar.top + 1,
                            pWin->hscroll.barLen + 1,
                            RECTH (rcHBar) - 2);
        SetPenColor (hdc, PIXEL_black);
        MoveTo (hdc, start, rcHBar.top + 1);
        LineTo (hdc, start, rcHBar.top + 1 + RECTH (rcHBar) - 2);
        MoveTo (hdc, start+pWin->hscroll.barLen + 1, rcHBar.top + 1);
        LineTo (hdc, start+pWin->hscroll.barLen + 1, rcHBar.top + 1 + RECTH (rcHBar) - 2);
#endif
#else
        Draw3DUpFrame (hdc, start,
                            rcHBar.top,
                            start + pWin->hscroll.barLen,
                            rcHBar.bottom,
                            PIXEL_invalid);
#endif
    }

    if (pWin->dwStyle & WS_VSCROLL && !(pWin->vscroll.status & SBS_HIDE)) {

        SetBrushColor (hdc, GetWindowElementColor (BKC_CONTROL_DEF));
        FillBox (hdc, rcVBar.left, rcVBar.top, 
                        RECTW (rcVBar), RECTH (rcVBar));

        // draw top and bottom buttons.
        if (pWin->vscroll.status & SBS_DISABLED) {
            xo = 0; yo = bh;
        }
        else {
            xo = 0; yo = 0;
        }

        FillBoxWithBitmapPart (hdc, rcVBar.left, rcVBar.top, 
                        bw, bh, 0, 0, bmp, xo, yo);

        if (pWin->vscroll.status & SBS_DISABLED) {
            xo = bw; yo = bh;
        }
        else {
            xo = bw; yo = 0;
        }
        
        FillBoxWithBitmapPart (hdc, 
                        rcVBar.left, rcVBar.bottom - GetMainWinMetrics (MWM_CYVSCROLL),
                        bw, bh, 0, 0, bmp, xo, yo);

#ifdef _FLAT_WINDOW_STYLE
        SetPenColor (hdc, GetWindowElementColor (FGC_CONTROL_DEF));
        Rectangle (hdc, rcVBar.left, rcVBar.top, rcVBar.right - 1, rcVBar.bottom);
#endif

        // draw moving bar
        start = rcVBar.top +
                    GetMainWinMetrics (MWM_CYVSCROLL) +
                    pWin->vscroll.barStart;
                    
        if (start + pWin->vscroll.barLen > rcVBar.bottom)
            start = rcVBar.bottom - pWin->vscroll.barLen;

#ifdef _FLAT_WINDOW_STYLE
#ifdef _GRAY_SCREEN
        SetBrushColor (hdc, GetWindowElementColor (FGC_CONTROL_DEF));
        FillBox (hdc, rcVBar.left + 2, start,
                    RECTW (rcVBar) - 4, pWin->vscroll.barLen);
#else
        SetBrushColor (hdc, PIXEL_lightgray);
        FillBox (hdc, rcVBar.left + 1, start,
                            RECTW (rcVBar) - 2,
                            pWin->vscroll.barLen);
        SetPenColor (hdc, PIXEL_black);
        MoveTo (hdc, rcVBar.left + 1, start);
        LineTo (hdc, rcVBar.left + 1 + RECTW (rcVBar) - 2, start);
        MoveTo (hdc, rcVBar.left + 1, start + pWin->vscroll.barLen - 1);
        LineTo (hdc, rcVBar.left + 1 + RECTW (rcVBar) - 2, start + pWin->vscroll.barLen - 1);
#endif
#else
        Draw3DUpFrame (hdc, rcVBar.left,
                            start,
                            rcVBar.right,
                            start + pWin->vscroll.barLen,
                            PIXEL_invalid);
#endif
    }
}

// this function is CONTROL safe.
static void wndDrawCaption(const MAINWIN* pWin, HDC hdc, BOOL bFocus)
{
    int i;
    RECT rc;
    int iBorder = 0;
    int iCaption = 0, bCaption;
    int iIconX = 0;
    int iIconY = 0;
    int x, y, w, h;
#if defined(_FLAT_WINDOW_STYLE) && defined(_GRAY_SCREEN)
    SIZE text_ext;
#endif
    PBITMAP bmp;
    int bw, bh;

    bmp = GetSystemBitmap (SYSBMP_CAPBTNS);
    bw = bmp->bmWidth >> 2;
    bh = bmp->bmHeight;

    if (pWin->dwStyle & WS_BORDER) 
        iBorder = GetMainWinMetrics(MWM_BORDER);
    else if( pWin->dwStyle & WS_THICKFRAME ) {
        iBorder = GetMainWinMetrics(MWM_THICKFRAME);
        
        SetPenColor (hdc, bFocus
                            ? GetWindowElementColor (WEC_FRAME_ACTIVED)
                            : GetWindowElementColor (WEC_FRAME_NORMAL));
        for (i=1; i<iBorder; i++)
            Rectangle(hdc, i, i, 
                      pWin->right - pWin->left - i - 1, 
                      pWin->bottom - pWin->top - i - 1);

    }
    else if (pWin->dwStyle & WS_THINFRAME)
        iBorder = GetMainWinMetrics (MWM_THINFRAME);

    if (!(pWin->dwStyle & WS_CAPTION))
        return;

    if (pWin->hIcon ) {
        iIconX = GetMainWinMetrics(MWM_ICONX);
        iIconY = GetMainWinMetrics(MWM_ICONY);
    }


    iCaption = GetMainWinMetrics(MWM_CAPTIONY);
    bCaption = iBorder + iCaption - 1;

    // draw Caption
    rc.left = iBorder;
    rc.top = iBorder;
    rc.right = pWin->right - pWin->left - iBorder;
    rc.bottom = iBorder + iCaption;
    ClipRectIntersect (hdc, &rc);

    SelectFont (hdc, GetSystemFont (SYSLOGFONT_CAPTION));

#ifdef _FLAT_WINDOW_STYLE
#ifdef _GRAY_SCREEN
    GetTextExtent (hdc, pWin->spCaption, -1, &text_ext);
    if (pWin->hIcon) text_ext.cx += iIconX + 2;

    SetBrushColor (hdc, bFocus
                        ? GetWindowElementColor (BKC_CAPTION_ACTIVED)
                        : GetWindowElementColor (BKC_CAPTION_NORMAL));
    FillBox (hdc, iBorder, iBorder, text_ext.cx + 4, bCaption);

    SetBrushColor(hdc, GetWindowElementColor (FGC_CAPTION_ACTIVED));
    FillBox (hdc, iBorder + text_ext.cx + 4, iBorder,
               pWin->right - pWin->left, bCaption);

    SetPenColor(hdc, bFocus
                        ? GetWindowElementColor (BKC_CAPTION_ACTIVED)
                        : GetWindowElementColor (BKC_CAPTION_NORMAL));

    MoveTo (hdc, iBorder, bCaption);
    LineTo (hdc, pWin->right - pWin->left, bCaption);

    SetPenColor(hdc, GetWindowElementColor (FGC_CAPTION_ACTIVED));

    MoveTo (hdc, iBorder + text_ext.cx + 4, bCaption - 3);
    LineTo (hdc, iBorder + text_ext.cx + 2, bCaption - 1);
    MoveTo (hdc, iBorder + text_ext.cx + 4, bCaption - 2);
    LineTo (hdc, iBorder + text_ext.cx + 3, bCaption - 1);

    MoveTo (hdc, iBorder + text_ext.cx + 2, iBorder);
    LineTo (hdc, iBorder + text_ext.cx + 4, iBorder + 2);
    MoveTo (hdc, iBorder + text_ext.cx + 3, iBorder);
    LineTo (hdc, iBorder + text_ext.cx + 4, iBorder + 1);
#else
    {
        unsigned char bits [256];
        BITMAP bmp;

        InitBitmap (hdc, 64, 1, 128, bits, &bmp);

        if (bFocus) {
            int i;
            Uint8 r, g, b;
            gal_pixel pixel;

            Pixel2RGB (hdc, GetWindowElementColor (BKC_CAPTION_ACTIVED), &r, &g, &b);
            if (r > 191) r = 255; else r += 64;
            if (g > 191) g = 255; else g += 64;
            if (b > 191) b = 255; else b += 64;
            for (i = 0; i < 64; i++) {
                pixel = RGB2Pixel (hdc, r - i, g - i, b - i);
                SetPixelInBitmap (&bmp, 63 - i, 0, pixel);
            }
        }
        else {
            int i;
            Uint8 r, g, b;
            gal_pixel pixel;

            Pixel2RGB (hdc, GetWindowElementColor (BKC_CAPTION_NORMAL), &r, &g, &b);
            if (r > 191) r = 255; else r += 64;
            if (g > 191) g = 255; else g += 64;
            if (b > 191) b = 255; else b += 64;
            for (i = 0; i < 64; i++) {
                pixel = RGB2Pixel (hdc, r - i, g - i, b - i);
                SetPixelInBitmap (&bmp, 63 - i, 0, pixel);
            }
        }

        FillBoxWithBitmap (hdc, iBorder, iBorder,
                                pWin->right - pWin->left - (iBorder << 1), 
                                bCaption + 1, &bmp);
    }
#endif
#else
    SetBrushColor (hdc, bFocus
                        ? GetWindowElementColor (BKC_CAPTION_ACTIVED)
                        : GetWindowElementColor (BKC_CAPTION_NORMAL));
    FillBox(hdc, iBorder, iBorder,
                       pWin->right - pWin->left - iBorder,
                       bCaption + 1);
#endif

    if (pWin->hIcon)
        DrawIcon (hdc, iBorder, iBorder + (bCaption - iIconY) / 2, 
                        iIconX, iIconY, pWin->hIcon);

    SetTextColor(hdc, bFocus
                        ? GetWindowElementColor (FGC_CAPTION_ACTIVED)
                        : GetWindowElementColor (FGC_CAPTION_NORMAL));
    SetBkColor(hdc, bFocus
                        ? GetWindowElementColor (BKC_CAPTION_ACTIVED)
                        : GetWindowElementColor (BKC_CAPTION_NORMAL));
    SetBkMode(hdc, BM_TRANSPARENT);
    TextOut(hdc, iBorder + iIconX + 2, iBorder + 3,
                pWin->spCaption);

    // draw system button
    w = GetMainWinMetrics (MWM_SB_WIDTH);
    x = rc.right - w;
    y = GetMainWinMetrics (MWM_SB_HEIGHT);
    if (y < bCaption) {
        y = iBorder + ((bCaption - y)>>1);
        h = GetMainWinMetrics (MWM_SB_HEIGHT);
    }
    else {
        y = iBorder;
        h = iIconY;
    }

    if (!(pWin->dwExStyle & WS_EX_NOCLOSEBOX)) {
        // close box
#if defined(_FLAT_WINDOW_STYLE) && !defined(_GRAY_SCREEN)
        int xx = x - 3;
        int yy = y + 1;
        int hh = h - 2;
        SetPenColor (hdc, PIXEL_lightwhite);
        Rectangle (hdc, xx, yy, xx + w, yy + hh);
        MoveTo (hdc, xx + 5, yy + hh / 2);
        LineTo (hdc, xx + w - 5, yy + hh / 2);
#else
        FillBoxWithBitmapPart (hdc, x, y, bw, bh, 0, 0, bmp, (bw << 1) + bw, 0);
#endif
        x -= GetMainWinMetrics (MWM_SB_WIDTH);
        x -= GetMainWinMetrics (MWM_SB_INTERX) << 1;
    }

    if (pWin->dwStyle & WS_MAXIMIZEBOX) {
        // restore/maximize/question box
#if defined(_FLAT_WINDOW_STYLE) && !defined(_GRAY_SCREEN)
        int xx = x - 3;
        int yy = y + 1;
        int hh = h - 2;
        SetPenColor (hdc, PIXEL_lightwhite);
        Rectangle (hdc, xx, yy, xx + w, yy + hh);
#if 1
        MoveTo (hdc, xx + 2, yy + 3);
        LineTo (hdc, xx + w - 5, yy + 3);
        MoveTo (hdc, xx + 2, yy + 5);
        LineTo (hdc, xx + w - 3, yy + 5);
        MoveTo (hdc, xx + 2, yy + 7);
        LineTo (hdc, xx + w - 2, yy + 7);
        MoveTo (hdc, xx + 2, yy + 9);
        LineTo (hdc, xx + w - 4, yy + 9);
#else
        MoveTo (hdc, xx + 5, yy);
        LineTo (hdc, xx + 5, yy + hh);
        MoveTo (hdc, xx + w - 5, yy);
        LineTo (hdc, xx + w - 5, yy + hh);
        MoveTo (hdc, xx + 5, yy + hh / 2);
        LineTo (hdc, xx + w - 5, yy + hh / 2);
#endif
#else
        if (pWin->dwStyle & WS_MAXIMIZE)
            FillBoxWithBitmapPart (hdc, x, y, bw, bh, 0, 0, bmp, bw << 1, 0);
        else
            FillBoxWithBitmapPart (hdc, x, y, bw, bh, 0, 0, bmp, 0, 0);
#endif
        x -= GetMainWinMetrics (MWM_SB_WIDTH);
        x -= GetMainWinMetrics (MWM_SB_INTERX);
    }

    if (pWin->dwStyle & WS_MINIMIZEBOX) {
        // minimize/ok box
#if defined(_FLAT_WINDOW_STYLE) && !defined(_GRAY_SCREEN)
        int xx = x - 3;
        int yy = y + 1;
        int hh = h - 2;
        SetPenColor (hdc, PIXEL_lightwhite);
        Rectangle (hdc, xx, yy, xx + w, yy + hh);
        Circle (hdc, xx + w / 2, yy + hh / 2, hh / 2 - 2);
#else
        FillBoxWithBitmapPart (hdc, x, y, bw, bh, 0, 0, bmp, bw, 0);
#endif
    }
}

static void wndEraseBackground(const MAINWIN* pWin, 
            HDC hdc, const RECT* pClipRect)
{
    RECT rcTemp;
    BOOL fGetDC = FALSE;

    if (hdc == 0) {
        hdc = GetClientDC ((HWND)pWin);
        fGetDC = TRUE;
    }

    if (pClipRect) {
        rcTemp = *pClipRect;
        if (pWin->WinType == TYPE_MAINWIN) {
            ScreenToClient ((HWND)pWin, &rcTemp.left, &rcTemp.top);
            ScreenToClient ((HWND)pWin, &rcTemp.right, &rcTemp.bottom);
        }
    }
    else {
        rcTemp.left = rcTemp.top = 0;
        rcTemp.right = pWin->cr - pWin->cl;
        rcTemp.bottom = pWin->cb - pWin->ct;
    }
    
    SetBrushColor(hdc, pWin->iBkColor);

    FillBox(hdc, rcTemp.left, rcTemp.top, 
                 RECTW (rcTemp), RECTH (rcTemp));

    if (fGetDC)
        ReleaseDC (hdc);
}

// this function is CONTROL safe.
static void wndDrawNCFrame(MAINWIN* pWin, HDC hdc, const RECT* prcInvalid)
{
    BOOL fGetDC = FALSE;
    
    if (hdc == 0) {
        hdc = GetDC ((HWND)pWin);
        fGetDC = TRUE;
    }
        
    if (prcInvalid)
        ClipRectIntersect (hdc, prcInvalid);

    wndDrawNCArea (pWin, hdc);

    wndDrawScrollBar (pWin, hdc);

    if (pWin->WinType == TYPE_MAINWIN) {
        wndDrawCaption (pWin, hdc, !(pWin->dwStyle & WS_DISABLED) 
            && (GetActiveWindow() == (HWND)pWin));
        DrawMenuBarHelper (pWin, hdc, prcInvalid);
    }
    else {
        wndDrawCaption (pWin, hdc, !(pWin->dwStyle & WS_DISABLED) && 
                ((PCONTROL)pWin)->pParent->active == (PCONTROL)pWin);
    }

    if (fGetDC)
        ReleaseDC (hdc);
}

// this function is CONTROL safe.
static void wndActiveMainWindow (PMAINWIN pWin, BOOL fActive)
{
    HDC hdc;

    hdc = GetDC ((HWND)pWin);

    wndDrawCaption (pWin, hdc, fActive);
        
    ReleaseDC (hdc);
}

#if 0
static void OnShowWindow (PMAINWIN pWin, int iShowCmd)
{
    PCONTROL pCtrl;
    
    if (iShowCmd != SW_HIDE) {
        
        pCtrl = (PCONTROL)(pWin->hFirstChild);

        while (pCtrl) {
            ShowWindow ((HWND)pCtrl, iShowCmd);

            pCtrl = pCtrl->next;
        }
    }
}
#endif

static int DefaultPaintMsgHandler(PMAINWIN pWin, int message,
                           WPARAM wParam, LPARAM lParam)
{
    switch( message )
    {
#if 0
        case MSG_SHOWWINDOW:
            OnShowWindow (pWin, (int)wParam);
        break;
#endif
        
        case MSG_NCPAINT:
            wndDrawNCFrame (pWin, (HDC)wParam, (const RECT*)lParam);
        break;

        case MSG_ERASEBKGND:
            wndEraseBackground (pWin, (HDC)wParam, (const RECT*)lParam);
        break;

        case MSG_NCACTIVATE:
            wndActiveMainWindow (pWin, (BOOL)wParam);
        break;

        case MSG_SYNCPAINT:
            wndActiveMainWindow (pWin, (BOOL)wParam);
            PostMessage ((HWND)pWin, MSG_NCPAINT, 0, 0);
            PostMessage ((HWND)pWin, MSG_ERASEBKGND, 0, 0);
        break;

        case MSG_PAINT:
        {
            PINVRGN pInvRgn;

            pInvRgn = &pWin->InvRgn;

#ifndef _LITE_VERSION
            pthread_mutex_lock (&pInvRgn->lock);
#endif
            EmptyClipRgn (&pInvRgn->rgn);
#ifndef _LITE_VERSION
            pthread_mutex_unlock (&pInvRgn->lock);
#endif
        }

        break;

    }

    return 0;
}

static int DefaultControlMsgHandler(PMAINWIN pWin, int message,
                           WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    int len;

    switch( message )
    {
        case MSG_ENABLE:
            if ( (!(pWin->dwStyle & WS_DISABLED) && !wParam)
                    || ((pWin->dwStyle & WS_DISABLED) && wParam) ) {
                if (wParam)
                    pWin->dwStyle &= ~WS_DISABLED;
                else
                    pWin->dwStyle |=  WS_DISABLED;
            }
        break;
        
        case MSG_SYSCOMMAND:
            if (wParam == SC_CLOSE)
                SendNotifyMessage ((HWND)pWin, MSG_CLOSE, 0, 0);
        break;

        case MSG_GETTEXTLENGTH:
            if (pWin->spCaption)
                return strlen (pWin->spCaption);
            else
                return 0;

        case MSG_GETTEXT:
            if (pWin->spCaption) {
                char* buffer = (char*)lParam;

                len = MIN (strlen (pWin->spCaption), wParam);
                memcpy (buffer, pWin->spCaption, len);
                buffer [len] = '\0';
                return len;
            }
            else
                return 0;
        break;

        case MSG_FONTCHANGED:
            UpdateWindow ((HWND)pWin, TRUE);
            break;

        case MSG_SETTEXT:
//
// NOTE:
// this message is only implemented for main window.
// for CONTROL, must handle this message and should NOT 
// call default window procedure
// when handle MSG_SETTEXT.
//
            if (pWin->WinType == TYPE_CONTROL)
                return 0;

            FreeFixStr (pWin->spCaption);
            len = strlen ((char*)lParam);
            pWin->spCaption = FixStrAlloc (len);
            if (len > 0)
                strcpy (pWin->spCaption, (char*)lParam);

            hdc = GetDC ((HWND)pWin);
            wndDrawCaption(pWin, hdc, GetForegroundWindow () == (HWND)pWin);
            ReleaseDC (hdc);
        break;
    }

    return 0;
}

static int DefaultSessionMsgHandler(PMAINWIN pWin, int message,
                           WPARAM wParam, LPARAM lParam)
{

// NOTE:
// Currently does nothing, should handle fowllowing messages:
//
// MSG_QUERYENDSESSION:
// MSG_ENDSESSION:
    
    return 0;
}

static int DefaultSystemMsgHandler(PMAINWIN pWin, int message, 
                           WPARAM wParam, LPARAM lParam)
{

// NOTE:
// Currently does nothing, should handle following messages:
//
// MSG_IDLE, MSG_CARETBLINK:

    if (message == MSG_IDLE) {
        if (pWin == GetMainWindowPtrOfControl (sg_repeat_msg.hwnd))
            SendNotifyMessage (sg_repeat_msg.hwnd, 
                    sg_repeat_msg.message, sg_repeat_msg.wParam, sg_repeat_msg.lParam);
    }
    else if (message == MSG_CARETBLINK && pWin->dwStyle & WS_VISIBLE) {
        BlinkCaret ((HWND)pWin);
    }
    
    return 0;
}

// NOTE:
// This default main window call-back procedure
// also implemented for control.
int DefaultMainWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (message >= MSG_FIRSTMOUSEMSG && message <= MSG_NCMOUSEOFF)
        return DefaultMouseMsgHandler(pWin, message, 
            wParam, LOSWORD (lParam), HISWORD (lParam));
    else if (message > MSG_NCMOUSEOFF && message <= MSG_LASTMOUSEMSG)
        return DefaultNCMouseMsgHandler(pWin, message, 
            (int)wParam, LOSWORD (lParam), HISWORD (lParam));
    else if (message >= MSG_FIRSTKEYMSG && message <= MSG_LASTKEYMSG)
        return DefaultKeyMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTPOSTMSG && message <= MSG_LASTPOSTMSG)
        return DefaultPostMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTCREATEMSG && message <= MSG_LASTCREATEMSG) 
        return DefaultCreateMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTPAINTMSG && message <= MSG_LASTPAINTMSG) 
        return DefaultPaintMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTSESSIONMSG && message <= MSG_LASTSESSIONMSG) 
        return DefaultSessionMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTCONTROLMSG && message <= MSG_LASTCONTROLMSG) 
        return DefaultControlMsgHandler(pWin, message, wParam, lParam);
    else if (message >= MSG_FIRSTSYSTEMMSG && message <= MSG_LASTSYSTEMMSG) 
        return DefaultSystemMsgHandler(pWin, message, wParam, lParam);

    return 0;
}

int DefaultControlProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    if (message == MSG_SETTEXT)
        return 0;
    else if (message == MSG_SETCURSOR) {
        if (GetWindowExStyle (hWnd) & WS_EX_USEPARENTCURSOR)
            return 0;

        SetCursor (GetWindowCursor (hWnd));
        return 0;
    }
    else if (message == MSG_NCSETCURSOR) {
        SetCursor (GetSystemCursor (IDC_ARROW));
        return 0;
    }

    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

/************************* GUI calls support ********************************/

static HWND set_focus_helper (HWND hWnd)
{
    PMAINWIN pWin;
    PMAINWIN old_active;
    
    if (IsMainWindow (hWnd))
        return HWND_INVALID;

    pWin = (PMAINWIN) GetParent (hWnd);
    old_active = (PMAINWIN)pWin->hActiveChild;
    if (old_active != (PMAINWIN)hWnd) {

        if (old_active)
            SendNotifyMessage ((HWND)old_active, MSG_KILLFOCUS, (WPARAM)hWnd, 0);
                
        pWin->hActiveChild = hWnd;
        SendNotifyMessage (hWnd, MSG_SETFOCUS, (WPARAM)old_active, 0);
    }
    
    return pWin->hActiveChild;
}

HWND GUIAPI SetFocusChild (HWND hWnd)
{
    HWND hOldActive;

    if ((hOldActive = set_focus_helper (hWnd)) != HWND_INVALID) {
        do {
            hWnd = GetParent (hWnd);
        } while (set_focus_helper (hWnd) != HWND_INVALID);
    }

    return hOldActive;
}

HWND GUIAPI GetFocusChild (HWND hWnd)
{
    PMAINWIN pWin;
    
    pWin = (PMAINWIN) hWnd;
    if (pWin)
        return pWin->hActiveChild;
    
    return HWND_INVALID;
}

// NOTE: this function support ONLY main window.
HWND GUIAPI SetActiveWindow(HWND hMainWnd)
{
    HWND hOld;
    
    if (hMainWnd == HWND_DESKTOP || !IsMainWindow (hMainWnd))
        return HWND_INVALID;

    hOld = (HWND) SendMessage (HWND_DESKTOP, MSG_SETACTIVEMAIN,
                (WPARAM)hMainWnd, 0);

    SendMessage (hMainWnd, MSG_NCACTIVATE, TRUE, 0);

    if (hOld && hOld != hMainWnd)
        SendMessage (hOld, MSG_NCACTIVATE, FALSE, 0);

    return hOld;
}

// NOTE: this function support ONLY main window.
HWND GUIAPI GetActiveWindow (void)
{
    return (HWND)SendMessage (HWND_DESKTOP, MSG_GETACTIVEMAIN, 0, 0);
}

HWND GUIAPI SetCapture(HWND hWnd)
{
    return (HWND) SendMessage (HWND_DESKTOP, MSG_SETCAPTURE, (WPARAM)hWnd, 0);
}


HWND GUIAPI GetCapture(void)
{
    return (HWND) SendMessage (HWND_DESKTOP, MSG_GETCAPTURE, 0, 0);
}

void GUIAPI ReleaseCapture(void)
{
    SendMessage (HWND_DESKTOP, MSG_SETCAPTURE, 0, 0);
}

/*************************** Main window and thread **************************/

BOOL GUIAPI IsWindow (HWND hWnd)
{
    PCONTROL pWin;

    pWin = (PCONTROL) hWnd;
    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return FALSE;

    return TRUE;
}

BOOL GUIAPI IsMainWindow(HWND hWnd)
{
    PCONTROL pChildWin;

    pChildWin = (PCONTROL)hWnd;
    if(pChildWin->DataType != TYPE_HWND) return FALSE;
    if(pChildWin->WinType == TYPE_MAINWIN) return TRUE;

    return FALSE;
}

BOOL GUIAPI IsControl(HWND hWnd)
{
    PCONTROL pChildWin;

    pChildWin = (PCONTROL)hWnd;
    if(pChildWin->DataType != TYPE_HWND) return FALSE;
    if(pChildWin->WinType == TYPE_CONTROL) return TRUE;

    return FALSE;
}

HWND GUIAPI GetMainWindowHandle (HWND hWnd)
{
    return (HWND)GetMainWindowPtrOfControl (hWnd);
}

HWND GUIAPI GetParent (HWND hWnd)
{
    PCONTROL pChildWin = (PCONTROL)hWnd;

    if (hWnd == HWND_DESKTOP)
        return HWND_DESKTOP;

    if (pChildWin->DataType != TYPE_HWND)
        return HWND_INVALID;
    
    return (HWND)pChildWin->pParent;
}

HWND GUIAPI GetHosting (HWND hWnd)
{
    PMAINWIN pWin;

    if (!(pWin = CheckAndGetMainWindowPtr (hWnd)))
        return HWND_INVALID;

    return (HWND)(pWin->pHosting);
}

HWND GUIAPI GetFirstHosted (HWND hWnd)
{
    PMAINWIN pWin;

    if (!(pWin = CheckAndGetMainWindowPtr (hWnd)))
        return HWND_INVALID;
        
    return (HWND)(pWin->pFirstHosted);
}

HWND GUIAPI GetNextHosted (HWND hHosting, HWND hHosted)
{
    PMAINWIN pWin;
    PMAINWIN pHosted;
    
    if (!(pWin = CheckAndGetMainWindowPtr (hHosting)))
        return HWND_INVALID;
        
    pHosted = (PMAINWIN)hHosted;
    if (pHosted->pHosting != pWin)
        return HWND_INVALID;
        
    return (HWND)(pHosted->pNextHosted);
}

HWND GUIAPI GetNextChild (HWND hWnd, HWND hChild)
{
    PCONTROL pControl, pChild;

    pControl = (PCONTROL)hWnd;
    pChild = (PCONTROL)hChild;

    if (pChild == NULL) {
        return (HWND)pControl->children;
    }
    else if (pControl != pChild->pParent) {
        return HWND_INVALID;
    }

    return (HWND)pChild->next;
}

HWND GUIAPI GetNextMainWindow (HWND hMainWnd)
{
    PMAINWIN pMainWin;
    if (hMainWnd == HWND_DESKTOP)
        pMainWin = NULL;
    else if (!(pMainWin = CheckAndGetMainWindowPtr (hMainWnd)))
        return HWND_INVALID;

    return (HWND) SendMessage (HWND_DESKTOP, 
                    MSG_GETNEXTMAINWIN, (WPARAM)pMainWin, 0L);
}

// NOTE: this function is CONTROL safe
void GUIAPI ScrollWindow (HWND hWnd, int iOffx, int iOffy, 
                    const RECT* rc1, const RECT* rc2)
{
    SCROLLWINDOWINFO    swi;

    swi.iOffx = iOffx;
    swi.iOffy = iOffy;
    swi.rc1   = rc1;
    swi.rc2   = rc2;
    
    // hide caret
    HideCaret (hWnd);

    SendMessage (HWND_DESKTOP, MSG_SCROLLMAINWIN, (WPARAM)hWnd, (LPARAM)(&swi));

    // show caret
    ShowCaret (hWnd);
}

static PSCROLLBARINFO wndGetScrollBar (MAINWIN* pWin, int iSBar)
{
    if (iSBar == SB_HORZ) {
        if (pWin->dwStyle & WS_HSCROLL)
            return &pWin->hscroll;
    }
    else if (iSBar == SB_VERT) {
        if (pWin->dwStyle & WS_VSCROLL)
            return &pWin->vscroll;
    }

    return NULL;
}

BOOL GUIAPI EnableScrollBar (HWND hWnd, int iSBar, BOOL bEnable)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    BOOL bPrevState;
    RECT rcBar;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    bPrevState = !(pSBar->status & SBS_DISABLED);

    if (bEnable && !bPrevState)
        pSBar->status &= ~SBS_DISABLED;
    else if (!bEnable && bPrevState)
        pSBar->status |= SBS_DISABLED;
    else
        return FALSE;

    if (iSBar == SB_VERT)
        wndGetVScrollBarRect (pWin, &rcBar);
    else
        wndGetHScrollBarRect (pWin, &rcBar);
        
    rcBar.left -= pWin->left;
    rcBar.top  -= pWin->top;
    rcBar.right -= pWin->left;
    rcBar.bottom -= pWin->top;

    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, (LPARAM)(&rcBar));

    return TRUE;
}

BOOL GUIAPI GetScrollPos (HWND hWnd, int iSBar, int* pPos)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    *pPos = pSBar->curPos;
    return TRUE;
}

BOOL GUIAPI GetScrollRange (HWND hWnd, int iSBar, int* pMinPos, int* pMaxPos)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    *pMinPos = pSBar->minPos;
    *pMaxPos = pSBar->maxPos;
    return TRUE;
}

BOOL GUIAPI SetScrollPos (HWND hWnd, int iSBar, int iNewPos)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    RECT rcBar;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    if (iNewPos < pSBar->minPos)
        pSBar->curPos = pSBar->minPos;
    else
        pSBar->curPos = iNewPos;

    {
        int max = pSBar->maxPos;
        max -= ((pSBar->pageStep - 1) > 0)?(pSBar->pageStep - 1):0;

        if (pSBar->curPos > max)
            pSBar->curPos = max;
    }
    
    if (iSBar == SB_VERT)
        wndGetVScrollBarRect (pWin, &rcBar);
    else
        wndGetHScrollBarRect (pWin, &rcBar);

    rcBar.left -= pWin->left;
    rcBar.top  -= pWin->top;
    rcBar.right -= pWin->left;
    rcBar.bottom -= pWin->top;

    wndScrollBarPos (pWin, iSBar == SB_HORZ, &rcBar);

    if (iSBar == SB_VERT) {
        rcBar.top += GetMainWinMetrics (MWM_CYVSCROLL);
        rcBar.bottom -= GetMainWinMetrics (MWM_CYVSCROLL);
    }
    else {
        rcBar.left += GetMainWinMetrics (MWM_CXHSCROLL);
        rcBar.right -= GetMainWinMetrics (MWM_CXHSCROLL);
    }

    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, (LPARAM)(&rcBar));

    return TRUE;
}

BOOL GUIAPI SetScrollRange (HWND hWnd, int iSBar, int iMinPos, int iMaxPos)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    RECT rcBar;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    pSBar->minPos = (iMinPos < iMaxPos)?iMinPos:iMaxPos;
    pSBar->maxPos = (iMinPos > iMaxPos)?iMinPos:iMaxPos;
    
    // validate parameters.
    if (pSBar->curPos < pSBar->minPos)
        pSBar->curPos = pSBar->minPos;

    if (pSBar->pageStep <= 0)
        pSBar->pageStep = 0;
    else if (pSBar->pageStep > (pSBar->maxPos - pSBar->minPos + 1))
        pSBar->pageStep = pSBar->maxPos - pSBar->minPos + 1;
    
    {
        int max = pSBar->maxPos;
        max -= ((pSBar->pageStep - 1) > 0)?(pSBar->pageStep - 1):0;

        if (pSBar->curPos > max)
            pSBar->curPos = max;
    }

    if (iSBar == SB_VERT)
        wndGetVScrollBarRect (pWin, &rcBar);
    else
        wndGetHScrollBarRect (pWin, &rcBar);

    rcBar.left -= pWin->left;
    rcBar.top  -= pWin->top;
    rcBar.right -= pWin->left;
    rcBar.bottom -= pWin->top;

    wndScrollBarPos (pWin, iSBar == SB_HORZ, &rcBar);

    if (iSBar == SB_VERT) {
        rcBar.top += GetMainWinMetrics (MWM_CYVSCROLL);
        rcBar.bottom -= GetMainWinMetrics (MWM_CYVSCROLL);
    }
    else {
        rcBar.left += GetMainWinMetrics (MWM_CXHSCROLL);
        rcBar.right -= GetMainWinMetrics (MWM_CXHSCROLL);
    }

    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, (LPARAM)(&rcBar));

    return TRUE;
}

BOOL GUIAPI SetScrollInfo (HWND hWnd, int iSBar, 
                const SCROLLINFO* lpsi, BOOL fRedraw)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    RECT rcBar;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;
        
    if( lpsi->fMask & SIF_RANGE )
    {
        pSBar->minPos = (lpsi->nMin < lpsi->nMax)?lpsi->nMin:lpsi->nMax;
        pSBar->maxPos = (lpsi->nMin < lpsi->nMax)?lpsi->nMax:lpsi->nMin;
    }
    
    if( lpsi->fMask & SIF_POS )
        pSBar->curPos = lpsi->nPos;
    
    if( lpsi->fMask & SIF_PAGE )
        pSBar->pageStep = lpsi->nPage;

    // validate parameters.
    if (pSBar->curPos < pSBar->minPos)
        pSBar->curPos = pSBar->minPos;

    if (pSBar->pageStep <= 0)
        pSBar->pageStep = 0;
    else if (pSBar->pageStep > (pSBar->maxPos - pSBar->minPos + 1))
        pSBar->pageStep = pSBar->maxPos - pSBar->minPos + 1;
    
    {
        int max = pSBar->maxPos;
        max -= ((pSBar->pageStep - 1) > 0)?(pSBar->pageStep - 1):0;

        if (pSBar->curPos > max)
            pSBar->curPos = max;
    }

    if(fRedraw)
    {
        if (iSBar == SB_VERT)
            wndGetVScrollBarRect (pWin, &rcBar);
        else
            wndGetHScrollBarRect (pWin, &rcBar);
        
        rcBar.left -= pWin->left;
        rcBar.top  -= pWin->top;
        rcBar.right -= pWin->left;
        rcBar.bottom -= pWin->top;
    
        wndScrollBarPos (pWin, iSBar == SB_HORZ, &rcBar);

        if (iSBar == SB_VERT) {
            rcBar.top += GetMainWinMetrics (MWM_CYVSCROLL);
            rcBar.bottom -= GetMainWinMetrics (MWM_CYVSCROLL);
        }
        else {
            rcBar.left += GetMainWinMetrics (MWM_CXHSCROLL);
            rcBar.right -= GetMainWinMetrics (MWM_CXHSCROLL);
        }

        SendAsyncMessage (hWnd, MSG_NCPAINT, 0, (LPARAM)(&rcBar));
    }
    
    return TRUE;
}

BOOL GUIAPI GetScrollInfo(HWND hWnd, int iSBar, PSCROLLINFO lpsi)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    
    pWin = (PMAINWIN)hWnd;
    
    if (!(pSBar = wndGetScrollBar (pWin, iSBar)))
        return FALSE;
        
    if (lpsi->fMask & SIF_RANGE) {
        lpsi->nMin = pSBar->minPos;
        lpsi->nMax = pSBar->maxPos;
    }
    
    if (lpsi->fMask & SIF_POS) {
        lpsi->nPos = pSBar->curPos;
    }
    
    if (lpsi->fMask & SIF_PAGE)
        lpsi->nPage = pSBar->pageStep;
    
    return TRUE;
}

BOOL GUIAPI ShowScrollBar (HWND hWnd, int iSBar, BOOL bShow)
{
    PSCROLLBARINFO pSBar;
    PMAINWIN pWin;
    BOOL bPrevState;
    RECT rcBar;
    
    pWin = (PMAINWIN)hWnd;
    
    if ( !(pSBar = wndGetScrollBar (pWin, iSBar)) )
        return FALSE;

    bPrevState = !(pSBar->status & SBS_HIDE);

    if (bShow && !bPrevState)
        pSBar->status &= ~SBS_HIDE;
    else if (!bShow && bPrevState)
        pSBar->status |= SBS_HIDE;
    else
        return FALSE;

    SendAsyncMessage (hWnd, MSG_CHANGESIZE, 0, 0);

    if (iSBar == SB_VERT)
        wndGetVScrollBarRect (pWin, &rcBar);
    else
        wndGetHScrollBarRect (pWin, &rcBar);

    InflateRect (&rcBar, 1, 1);

    RecalcClientArea (hWnd);
    
    if (bShow) {
        SendAsyncMessage (hWnd, MSG_NCPAINT, 0, 0);
    }
    else {
        rcBar.left -= pWin->cl;
        rcBar.top  -= pWin->ct;
        rcBar.right -= pWin->cl;
        rcBar.bottom -= pWin->ct;
        SendAsyncMessage (hWnd, MSG_NCPAINT, 0, 0);
        InvalidateRect (hWnd, &rcBar, TRUE);
    }

    return TRUE;
}

/*************************** Main window creation ****************************/
#ifndef _LITE_VERSION
int GUIAPI CreateThreadForMainWindow (pthread_t* thread,
                                     pthread_attr_t* attr,
                                     void * (*start_routine)(void *),
                                     void * arg)
{
    pthread_attr_t new_attr;
    int ret;

    pthread_attr_init (&new_attr);
    if (attr == NULL) {
        attr = &new_attr;
        pthread_attr_setdetachstate (attr, PTHREAD_CREATE_DETACHED);
    }

    ret = pthread_create(thread, attr, start_routine, arg);

    pthread_attr_destroy (&new_attr);

    return ret;
}

int GUIAPI WaitMainWindowClose(HWND hWnd, void** returnval)
{
    pthread_t th;
   
    th = GetMainWinThread(hWnd);

    return pthread_join(th, returnval);
}
#endif

void GUIAPI MainWindowThreadCleanup (HWND hMainWnd)
{
    PMAINWIN pWin = (PMAINWIN)hMainWnd;

#ifndef _LITE_VERSION
    if (pWin->pHosting == NULL) {
        DestroyMsgQueue (pWin->pMessages);

        free(pWin->pMessages);
    }
#endif

    free (pWin);
}

HWND GUIAPI CreateMainWindow(PMAINWINCREATE pCreateInfo)
{
    PMAINWIN pWin;

    if( !(pWin = calloc(1, sizeof(MAINWIN))) ) return HWND_INVALID;

#ifndef _LITE_VERSION
    if (pCreateInfo->hHosting == HWND_DESKTOP) {
        // Create message queue for this new main window.
        if( !(pWin->pMessages = malloc(sizeof(MSGQUEUE))) ) {
            free(pWin);
            return HWND_INVALID;
        }
        
        // Init message queue.
        if (!InitMsgQueue(pWin->pMessages, 0))
            goto err;
    }
    else
    {
        pWin->pMessages = GetMsgQueue (pCreateInfo->hHosting);
    }
#else
    pWin->pMessages = &__mg_dsk_msgs;
#endif

    pWin->pMainWin      = NULL;
    pWin->hParent       = HWND_DESKTOP;
    pWin->pFirstHosted  = NULL;
    pWin->pHosting      = GetMainWindowPtrOfControl (pCreateInfo->hHosting);
    pWin->pNextHosted   = NULL;
        
    pWin->DataType      = TYPE_HWND;
    pWin->WinType       = TYPE_MAINWIN;

#ifndef _LITE_VERSION
    pWin->th            = pthread_self();
#endif

    pWin->hFirstChild   = 0;
    pWin->hActiveChild  = 0;
    pWin->hOldUnderPointer = 0;
    pWin->hPrimitive    = 0;

    pWin->NotifProc     = NULL;

    pWin->dwStyle       = pCreateInfo->dwStyle;
    pWin->dwExStyle     = pCreateInfo->dwExStyle;

    pWin->hMenu         = pCreateInfo->hMenu;
    pWin->hCursor       = pCreateInfo->hCursor;
    pWin->hIcon         = pCreateInfo->hIcon;
    if ((pWin->dwStyle & WS_CAPTION) && (pWin->dwStyle & WS_SYSMENU))
        pWin->hSysMenu= CreateSystemMenu ((HWND)pWin, pWin->dwStyle);
    else
        pWin->hSysMenu = 0;

    pWin->pLogFont     = GetSystemFont (SYSLOGFONT_WCHAR_DEF);

    pWin->spCaption    = FixStrAlloc (strlen (pCreateInfo->spCaption));
    if (pCreateInfo->spCaption [0])
        strcpy (pWin->spCaption, pCreateInfo->spCaption);

    pWin->MainWindowProc = pCreateInfo->MainWindowProc;
    pWin->iBkColor    = pCreateInfo->iBkColor;

    pWin->pCaretInfo = NULL;

    pWin->dwAddData = pCreateInfo->dwAddData;
    pWin->dwAddData2 = 0;

    if ( !( pWin->pZOrderNode = malloc (sizeof(ZORDERNODE))) )
        goto err;

    // Scroll bar
    if (pWin->dwStyle && WS_VSCROLL) {
        pWin->vscroll.minPos = 0;
        pWin->vscroll.maxPos = 100;
        pWin->vscroll.curPos = 0;
        pWin->vscroll.pageStep = 0;
        pWin->vscroll.barStart = 0;
        pWin->vscroll.barLen = 10;
        pWin->vscroll.status = SBS_NORMAL;
    }
    else
        pWin->vscroll.status = SBS_HIDE | SBS_DISABLED;
        
    if (pWin->dwStyle && WS_HSCROLL) {
        pWin->hscroll.minPos = 0;
        pWin->hscroll.maxPos = 100;
        pWin->hscroll.curPos = 0;
        pWin->hscroll.pageStep = 0;
        pWin->hscroll.barStart = 0;
        pWin->hscroll.barLen = 10;
        pWin->hscroll.status = SBS_NORMAL;
    }
    else
        pWin->hscroll.status = SBS_HIDE | SBS_DISABLED;

    if (SendMessage ((HWND)pWin, MSG_NCCREATE, 0, (LPARAM)pCreateInfo))
        goto err;

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (!(pWin->dwStyle & WS_ABSSCRPOS)) {
        pCreateInfo->lx += g_rcDesktop.left;
        pCreateInfo->ty += g_rcDesktop.top;
        pCreateInfo->rx += g_rcDesktop.left;
        pCreateInfo->by += g_rcDesktop.top;
    }
#endif

    SendMessage ((HWND)pWin, MSG_SIZECHANGING, (WPARAM)&pCreateInfo->lx, (LPARAM)&pWin->left);
    SendMessage ((HWND)pWin, MSG_CHANGESIZE, (WPARAM)&pWin->left, 0);

    RecalcClientArea ((HWND)pWin);
   
    SendMessage (HWND_DESKTOP, MSG_ADDNEWMAINWIN,
                                (WPARAM) pWin, (LPARAM) pWin->pZOrderNode);

    /* There was a very large bug. 
     * We should add the new main window in system and then
     * SendMessage MSG_CREATE for application to create
     * child windows.
     */
    if (SendMessage ((HWND)pWin, MSG_CREATE, 0, (LPARAM)pCreateInfo)) {
        SendMessage(HWND_DESKTOP, MSG_REMOVEMAINWIN, (WPARAM)pWin, 0);
        goto err;
    }

    // Create private client dc.
    if (pWin->dwExStyle & WS_EX_USEPRIVATECDC)
        pWin->privCDC = CreatePrivateClientDC ((HWND)pWin);
    else
        pWin->privCDC = 0;

    return (HWND)pWin;

err:
#ifndef _LITE_VERSION
    if (pWin->pMessages) {
        DestroyMsgQueue (pWin->pMessages);
        free (pWin->pMessages);
    }
#endif

    if (pWin->pZOrderNode) free (pWin->pZOrderNode);
    if (pWin->privCDC) DeletePrivateDC (pWin->privCDC);
    free (pWin);

    return HWND_INVALID;
}

BOOL GUIAPI DestroyMainWindow (HWND hWnd)
{
    PMAINWIN pWin;
    PMAINWIN head, next;    // hosted window list.
    
    if( !(pWin = CheckAndGetMainWindowPtr (hWnd)) ) return FALSE;

    if (SendMessage (hWnd, MSG_DESTROY, 0, 0))
        return FALSE;

    // destroy all hosted main window here.
    head = pWin->pFirstHosted;
    while (head) {
        next = head->pNextHosted;

        if (!DestroyMainWindow ((HWND)head))
            return FALSE;
	MainWindowCleanup ((HWND)head);

        head = next;
    }

    SendMessage(HWND_DESKTOP, MSG_REMOVEMAINWIN, (WPARAM)hWnd, 0);

    ThrowAwayMessages (hWnd);

    free (pWin->pZOrderNode);

    if (pWin->privCDC)
        DeletePrivateDC (pWin->privCDC);

    FreeFixStr (pWin->spCaption);

    if (pWin->hMenu)
        DestroyMenu (pWin->hMenu);
    if (pWin->hSysMenu)
        DestroyMenu (pWin->hSysMenu);

    EmptyClipRgn (&pWin->pGCRInfo->crgn);
    EmptyClipRgn (&pWin->InvRgn.rgn);

#ifndef _LITE_VERSION
    pthread_mutex_destroy (&pWin->pGCRInfo->lock);
    pthread_mutex_destroy (&pWin->InvRgn.lock);
#endif

    return TRUE;
}

/*************************** Main window creation ****************************/
void GUIAPI UpdateWindow (HWND hWnd, BOOL fErase)
{
    if (fErase) // Recalculate the client area.
        SendAsyncMessage (hWnd, MSG_CHANGESIZE, 0, 0);

    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, 0);
    if (fErase)
        InvalidateRect (hWnd, NULL, TRUE);
    else
        InvalidateRect (hWnd, NULL, FALSE);

    if (hWnd != HWND_DESKTOP) {
        PMAINWIN pWin;
    
        pWin = (PMAINWIN) hWnd;
        SendMessage (hWnd, MSG_PAINT, 0, (LPARAM)(&pWin->InvRgn.rgn));
    }
    else
        SendMessage (hWnd, MSG_PAINT, 0, 0);
}

// this function show window in behavious by specified iCmdShow.
// if the window was previously visible, the return value is nonzero.
// if the window was previously hiddedn, the return value is zero.
//
BOOL GUIAPI ShowWindow(HWND hWnd, int iCmdShow)
{
    if (IsMainWindow (hWnd)) {
        switch (iCmdShow)
        {
            case SW_SHOWNORMAL:
                SendMessage (HWND_DESKTOP, 
                    MSG_MOVETOTOPMOST, (WPARAM)hWnd, 0);
            break;
            
            case SW_SHOW:
                SendMessage (HWND_DESKTOP, 
                    MSG_SHOWMAINWIN, (WPARAM)hWnd, 0);
            break;

            case SW_HIDE:
                SendMessage (HWND_DESKTOP, 
                    MSG_HIDEMAINWIN, (WPARAM)hWnd, 0);
            break;
        }
    }
    else {
        PCONTROL pControl;

        pControl = (PCONTROL)hWnd;
        
        if (pControl->dwExStyle & WS_EX_CTRLASMAINWIN) {
            if (iCmdShow == SW_SHOW)
                SendMessage (HWND_DESKTOP, MSG_SHOWGLOBALCTRL, (WPARAM)hWnd, iCmdShow);
            else if (iCmdShow == SW_HIDE)
                SendMessage (HWND_DESKTOP, MSG_HIDEGLOBALCTRL, (WPARAM)hWnd, iCmdShow);
            else
                return FALSE;
        }
        else {
            switch (iCmdShow) {
            case SW_SHOWNORMAL:
            case SW_SHOW:
                if (!(pControl->dwStyle & WS_VISIBLE)) {
                    pControl->dwStyle |= WS_VISIBLE;

                    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, 0);
                    InvalidateRect (hWnd, NULL, TRUE);
                }
            break;
            
            case SW_HIDE:
                if (pControl->dwStyle & WS_VISIBLE) {
                
                    pControl->dwStyle &= ~WS_VISIBLE;
                    InvalidateRect ((HWND)(pControl->pParent), 
                        (RECT*)(&pControl->left), TRUE);
                }
            break;
            }
        }

        if (iCmdShow == SW_HIDE && pControl->pParent->active == pControl) {
            SendNotifyMessage (hWnd, MSG_KILLFOCUS, 0, 0);
            pControl->pParent->active = NULL;
        }
    }
    
    SendNotifyMessage (hWnd, MSG_SHOWWINDOW, (WPARAM)iCmdShow, 0);
    return TRUE;
}

BOOL GUIAPI EnableWindow (HWND hWnd, BOOL fEnable)
{
    BOOL fOldStatus;
    
    if (IsMainWindow (hWnd)) {
        fOldStatus = SendMessage (HWND_DESKTOP, MSG_ENABLEMAINWIN,
                        (WPARAM)hWnd, (LPARAM)fEnable);
    }
    else {
        PCONTROL pControl;

        pControl = (PCONTROL)hWnd;
        
        fOldStatus = !(pControl->dwStyle & WS_DISABLED);
    }
    
    SendNotifyMessage (hWnd, MSG_ENABLE, fEnable, 0);

    return fOldStatus;
}

BOOL GUIAPI IsWindowEnabled (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    return !(pWin->dwStyle & WS_DISABLED);
}

void GUIAPI ScreenToClient (HWND hWnd, int* x, int* y)
{
    PCONTROL pParent;
    PCONTROL pCtrl;

    pParent = pCtrl = (PCONTROL) hWnd;
    if (hWnd == HWND_DESKTOP || pCtrl->DataType != TYPE_HWND)
        return;

    *x -= pCtrl->cl;
    *y -= pCtrl->ct;
    while ((pParent = pParent->pParent)) {
        *x -= pParent->cl;
        *y -= pParent->ct;
    }
}

void GUIAPI ClientToScreen(HWND hWnd, int* x, int* y)
{
    PCONTROL pParent;
    PCONTROL pCtrl;

    pParent = pCtrl = (PCONTROL) hWnd;
    if (hWnd == HWND_DESKTOP || pCtrl->DataType != TYPE_HWND)
        return;

    *x += pCtrl->cl;
    *y += pCtrl->ct;
    while ((pParent = pParent->pParent)) {
        *x += pParent->cl;
        *y += pParent->ct;
    }
}

void GUIAPI ScreenToWindow(HWND hWnd, int* x, int* y)
{
    PCONTROL pParent;
    PCONTROL pCtrl;

    pParent = pCtrl = (PCONTROL) hWnd;
    if (hWnd == HWND_DESKTOP || pCtrl->DataType != TYPE_HWND)
        return;

    *x -= pCtrl->left;
    *y -= pCtrl->top;
    while ((pParent = pParent->pParent)) {
        *x -= pParent->left;
        *y -= pParent->top;
    }
}

void GUIAPI WindowToScreen(HWND hWnd, int* x, int* y)
{
    PCONTROL pParent;
    PCONTROL pCtrl;

    pParent = pCtrl = (PCONTROL) hWnd;
    if (hWnd == HWND_DESKTOP || pCtrl->DataType != TYPE_HWND)
        return;

    *x += pCtrl->left;
    *y += pCtrl->top;
    while ((pParent = pParent->pParent)) {
        *x += pParent->left;
        *y += pParent->top;
    }
}

BOOL GUIAPI GetClientRect (HWND hWnd, PRECT prc)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP) {
        *prc = g_rcScr;
        return TRUE;
    }
    else if (pWin->DataType != TYPE_HWND)
        return FALSE;

    prc->left = prc->top = 0;
    prc->right = pWin->cr - pWin->cl;
    prc->bottom = pWin->cb - pWin->ct;
    return TRUE;
}

/******************** main window and control styles support *****************/
int GUIAPI GetWindowBkColor (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return PIXEL_invalid;

    return pWin->iBkColor;
}

int GUIAPI SetWindowBkColor (HWND hWnd, int new_bkcolor)
{
    int old_bkcolor;

    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return PIXEL_invalid;

    old_bkcolor = pWin->iBkColor;
    pWin->iBkColor = new_bkcolor;
    return old_bkcolor;
}

PLOGFONT GUIAPI GetWindowFont (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return NULL;

    return pWin->pLogFont;
}

PLOGFONT GUIAPI SetWindowFont (HWND hWnd, PLOGFONT pLogFont)
{
    PLOGFONT old_logfont = NULL;

    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        goto ret;

    if (pLogFont == NULL)
        pLogFont = GetSystemFont (SYSLOGFONT_WCHAR_DEF);

    if (SendMessage (hWnd, MSG_FONTCHANGING, 0, (LPARAM)pLogFont))
        goto ret;

    old_logfont = pWin->pLogFont;
    pWin->pLogFont = pLogFont;
    SendNotifyMessage (hWnd, MSG_FONTCHANGED, 0, 0);

ret:
    return old_logfont;
}

HCURSOR GUIAPI GetWindowCursor (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    return pWin->hCursor;
}

HCURSOR GUIAPI SetWindowCursor (HWND hWnd, HCURSOR hNewCursor)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    if (pWin->WinType == TYPE_MAINWIN)
        return SendMessage (HWND_DESKTOP, 
            MSG_SETWINCURSOR, (WPARAM)hWnd, (LPARAM)hNewCursor);
    else if (pWin->WinType == TYPE_CONTROL) {
        HCURSOR old = pWin->hCursor;
        pWin->hCursor = hNewCursor;
        return old;
    }
        
    return 0;
}

DWORD GetWindowStyle (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    return pWin->dwStyle;
}

BOOL GUIAPI ExcludeWindowStyle (HWND hWnd, DWORD dwStyle)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return FALSE;

    pWin->dwStyle &= ~dwStyle;
    return TRUE;
}

BOOL GUIAPI IncludeWindowStyle (HWND hWnd, DWORD dwStyle)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return FALSE;

    pWin->dwStyle |= dwStyle;
    return TRUE;
}

DWORD GetWindowExStyle (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    return pWin->dwExStyle;
}

DWORD GUIAPI GetWindowAdditionalData (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    return pWin->dwAddData;
}

DWORD GUIAPI SetWindowAdditionalData (HWND hWnd, DWORD newData)
{
    DWORD    oldOne = 0L;
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return oldOne;

    oldOne = pWin->dwAddData;
    pWin->dwAddData = newData;
    return oldOne;
}

DWORD GUIAPI GetWindowAdditionalData2 (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;

    return pWin->dwAddData2;
}

DWORD GUIAPI SetWindowAdditionalData2 (HWND hWnd, DWORD newData)
{
    DWORD    oldOne = 0L;
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return oldOne;

    oldOne = pWin->dwAddData2;
    pWin->dwAddData2 = newData;
    return oldOne;
}

DWORD GUIAPI GetWindowClassAdditionalData (HWND hWnd)
{
    PMAINWIN pWin;
    PCONTROL pCtrl;

    pWin = (PMAINWIN)hWnd;
    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;
    else if (pWin->WinType == TYPE_CONTROL) {
        pCtrl = (PCONTROL)hWnd;
        return pCtrl->pcci->dwAddData;
    }

    return 0;
}

DWORD GUIAPI SetWindowClassAdditionalData (HWND hWnd, DWORD newData)
{
    DWORD    oldOne = 0L;
    PMAINWIN pWin;
    PCONTROL pCtrl;

    pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return 0;
    else if (pWin->WinType == TYPE_CONTROL) {
        pCtrl = (PCONTROL)hWnd;
        oldOne = pCtrl->pcci->dwAddData;
        pCtrl->pcci->dwAddData = newData;
    }
    
    return oldOne;
}

const char* GUIAPI GetClassName (HWND hWnd)
{
    PMAINWIN pWin;
    PCONTROL pCtrl;

    pWin = (PMAINWIN)hWnd;
    if (hWnd == HWND_DESKTOP || pWin->WinType == TYPE_MAINWIN)
        return MAINWINCLASSNAME;
    else if (pWin->WinType == TYPE_CONTROL) {
        pCtrl = (PCONTROL)hWnd;
        return pCtrl->pcci->name;
    }

    return NULL;
}

BOOL GUIAPI IsWindowVisible (HWND hWnd)
{
    PMAINWIN pMainWin;
    PCONTROL pCtrl;
    
    if ((pMainWin = CheckAndGetMainWindowPtr (hWnd))) {
        return pMainWin->dwStyle & WS_VISIBLE;
    }
    else if (IsControl (hWnd)) {
        if (!IsWindowVisible (GetParent (hWnd)))
            return FALSE;

        pCtrl = (PCONTROL)hWnd;
        return pCtrl->dwStyle & WS_VISIBLE;
    }
        
    return FALSE;
}

BOOL GUIAPI GetWindowRect (HWND hWnd, PRECT prc)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return FALSE;

    prc->left = pWin->left;
    prc->top = pWin->top;
    prc->right = pWin->right;
    prc->bottom = pWin->bottom;
    return TRUE;
}

WNDPROC GUIAPI GetWindowCallbackProc (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN) hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return NULL;

    return pWin->MainWindowProc;
}

WNDPROC GUIAPI SetWindowCallbackProc (HWND hWnd, WNDPROC newProc)
{
    PMAINWIN pWin = (PMAINWIN) hWnd;
    WNDPROC old_proc;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return NULL;

    old_proc = pWin->MainWindowProc;
    if (newProc)
        pWin->MainWindowProc = newProc;
    return old_proc;
}

const char* GUIAPI GetWindowCaption (HWND hWnd)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP || pWin->DataType != TYPE_HWND)
        return NULL;

    return pWin->spCaption;
}

BOOL GUIAPI SetWindowCaption (HWND hWnd, const char* spCaption)
{
    PMAINWIN pWin = (PMAINWIN)hWnd;

    if (hWnd == HWND_DESKTOP)
        return FALSE;
    else if (pWin->WinType == TYPE_MAINWIN)
        return SetWindowText (hWnd, spCaption);
    else if (pWin->WinType == TYPE_CONTROL) {
        PCONTROL pCtrl;
        pCtrl = (PCONTROL)hWnd;
        if (pCtrl->spCaption) {
            FreeFixStr (pCtrl->spCaption);
            pCtrl->spCaption = NULL;
        }

        if (spCaption) {
            pCtrl->spCaption = FixStrAlloc (strlen (spCaption));
            if (spCaption [0])
                strcpy (pCtrl->spCaption, spCaption);
        }

        return TRUE;
    }

    return FALSE;
}

int GUIAPI GetWindowTextLength (HWND hWnd)
{
    return SendMessage (hWnd, MSG_GETTEXTLENGTH, 0, 0);
}

int GUIAPI GetWindowText (HWND hWnd, char* spString, int nMaxLen)
{
    return SendMessage (hWnd, MSG_GETTEXT, (WPARAM)nMaxLen, (LPARAM)spString);
}

BOOL GUIAPI SetWindowText (HWND hWnd, const char* spString)
{
    return (SendMessage (hWnd, MSG_SETTEXT, 0, (LPARAM)spString) == 0);
}

// NOTE: This function is control safe
BOOL GUIAPI MoveWindow (HWND hWnd, int x, int y, int w, int h, BOOL fPaint)
{
    RECT rcWindow;
    RECT rcExpect, rcResult;
    
    SetRect (&rcExpect, x, y, x + w, y + h);
    
    GetWindowRect (hWnd, &rcWindow);
    SendMessage (hWnd, MSG_SIZECHANGING, 
            (WPARAM)(&rcExpect), (LPARAM)(&rcResult));

    if (EqualRect (&rcWindow, &rcResult))
        return FALSE;

    if (IsControl (hWnd)) {
        PCONTROL pParent;
        PCONTROL pCtrl;
        HDC hdc;

        pCtrl = (PCONTROL)hWnd;
        pParent = pCtrl->pParent;

        if ((pCtrl->dwStyle & WS_VISIBLE)
                && (pParent->dwStyle & WS_VISIBLE)
                && (pParent->InvRgn.frozen == 0)) {
            hdc = GetClientDC ((HWND)pParent);
            BitBlt (hdc, pCtrl->left, pCtrl->top, 
                     pCtrl->right - pCtrl->left,
                     pCtrl->bottom - pCtrl->top,
                     hdc, rcResult.left, rcResult.top, 0);
            ReleaseDC (hdc);

            // set to invisible temporarily.
            pCtrl->dwStyle &= ~WS_VISIBLE;
            InvalidateRect ((HWND)pParent, (PRECT)(&pCtrl->left), TRUE);
            pCtrl->dwStyle |= WS_VISIBLE;
        }
    }
    else
        SendMessage (HWND_DESKTOP, MSG_MOVEMAINWIN, 
            (WPARAM)hWnd, (LPARAM)(&rcResult));

    SendMessage (hWnd, MSG_CHANGESIZE, (WPARAM)(&rcResult), 0);

    RecalcClientArea (hWnd);

    SendAsyncMessage (hWnd, MSG_NCPAINT, 0, 0);
    if (fPaint) {
        InvalidateRect (hWnd, NULL, TRUE);
//        SendAsyncMessage (hWnd, MSG_PAINT, 0, 0);
    }

    return TRUE;
}

/*************************** Paint support ***********************************/
void wndUpdateInvalidRegion (HWND hWnd, 
        const RECT* prc, const RECT* prcClient)
{
    PMAINWIN pWin;
    PCONTROL pCtrl;
    PINVRGN pInvRgn;
    RECT rcInter;
    RECT rcTemp;

    pWin = (PMAINWIN)hWnd;

    if (pWin->WinType == TYPE_MAINWIN)
        pInvRgn = &pWin->InvRgn;
    else if (pWin->WinType == TYPE_CONTROL) {
        pCtrl = (PCONTROL)hWnd;
        pInvRgn = &pCtrl->InvRgn;
    }
    else
        return;

    if (pInvRgn->frozen)
        return;

#ifndef _LITE_VERSION
    pthread_mutex_lock (&pInvRgn->lock);
#endif
    if (prc) {
        rcTemp = *prc;
        NormalizeRect (&rcTemp);
        if (IsCovered (prcClient, &rcTemp)) {
            SetClipRgn (&pInvRgn->rgn, prcClient);
        }
        else {
            if (IntersectRect (&rcInter, &rcTemp, prcClient))
                AddClipRect (&pInvRgn->rgn, &rcInter);
        }
    }
    else {
        SetClipRgn (&pInvRgn->rgn, prcClient);
    }
#ifndef _LITE_VERSION
    pthread_mutex_unlock (&pInvRgn->lock);
#endif
}

static void wndCascadeInvalidateChildren (PCONTROL pCtrl, const RECT* prc, BOOL bEraseBkgnd)
{
    RECT rcInter;
    RECT rcTemp;
    RECT rcCtrlClient;

    while (pCtrl) {

        if (!(pCtrl->dwStyle & WS_VISIBLE)) {
             pCtrl = pCtrl->next;
             continue;
        }
            
        rcCtrlClient.left = rcCtrlClient.top = 0;
        rcCtrlClient.right = pCtrl->cr - pCtrl->cl;
        rcCtrlClient.bottom = pCtrl->cb - pCtrl->ct;
        if (prc) {
            
            RECT rcCtrl;
                
            rcCtrl.left = rcCtrl.top = 0;
            rcCtrl.right = pCtrl->right - pCtrl->left;
            rcCtrl.bottom = pCtrl->bottom - pCtrl->top;
            rcTemp.left = prc->left - pCtrl->left;
            rcTemp.top  = prc->top  - pCtrl->top;
            rcTemp.right = prc->right - pCtrl->left;
            rcTemp.bottom = prc->bottom - pCtrl->top;

            if (IntersectRect (&rcInter, &rcTemp, &rcCtrl))
                SendAsyncMessage ((HWND)pCtrl,
                            MSG_NCPAINT, 0, (LPARAM)(&rcInter));

            rcTemp.left = prc->left - pCtrl->cl;
            rcTemp.top  = prc->top  - pCtrl->ct;
            rcTemp.right = prc->right - pCtrl->cl;
            rcTemp.bottom = prc->bottom - pCtrl->ct;

            if (IntersectRect (&rcInter, &rcTemp, &rcCtrlClient)) {
                wndUpdateInvalidRegion ((HWND)pCtrl, &rcInter, &rcCtrlClient);
                SendAsyncMessage ((HWND)pCtrl, 
                        MSG_ERASEBKGND, 0, (LPARAM)(&rcInter));
            }

            if (pCtrl->children)
                wndCascadeInvalidateChildren (pCtrl->children, &rcTemp, bEraseBkgnd);
        }
        else {
            SendAsyncMessage ((HWND)pCtrl, MSG_NCPAINT, 0, 0);
            wndUpdateInvalidRegion ((HWND)pCtrl, NULL, &rcCtrlClient);
            SendAsyncMessage ((HWND)pCtrl, MSG_ERASEBKGND, 0, 0);
            if (pCtrl->children)
                wndCascadeInvalidateChildren (pCtrl->children, NULL, bEraseBkgnd);
        }

        pCtrl = pCtrl->next;
    }
}

BOOL wndInvalidateRect (HWND hWnd, const RECT* prc, BOOL bEraseBkgnd)
{
    PMAINWIN pWin;
    PCONTROL pCtrl = NULL;
    RECT rcInter;
    RECT rcInvalid;
    RECT rcClient;
    PINVRGN pInvRgn;

    pWin = (PMAINWIN)hWnd;
    pCtrl = (PCONTROL)hWnd;

    if (pWin->WinType == TYPE_MAINWIN) {

        rcClient.left = rcClient.top = 0;
        rcClient.right = pWin->cr - pWin->cl;
        rcClient.bottom = pWin->cb - pWin->ct;

        pInvRgn = &pWin->InvRgn;

        if (pInvRgn->frozen)
            return FALSE;

#ifndef _LITE_VERSION
        pthread_mutex_lock (&pInvRgn->lock);
#endif
        if (prc) {
            rcInvalid = *prc;
            NormalizeRect (&rcInvalid);
            if (IsCovered (&rcClient, &rcInvalid)) {
                SetClipRgn (&pInvRgn->rgn, &rcClient);
            }
            else {
                if (IntersectRect (&rcInter, &rcInvalid, &rcClient)) {
                    AddClipRect (&pInvRgn->rgn, &rcInter);
                }
            }
        }
        else {
            SetClipRgn (&pInvRgn->rgn, &rcClient);
            rcInvalid = rcClient;
        }
#ifndef _LITE_VERSION
        pthread_mutex_unlock (&pInvRgn->lock);
#endif
        
        // erase background here.
        if (bEraseBkgnd) {
#if 1
            ClientToScreen (hWnd, &rcInvalid.left, &rcInvalid.top);
            ClientToScreen (hWnd, &rcInvalid.right, &rcInvalid.bottom);
            SendAsyncMessage (hWnd, MSG_ERASEBKGND, 0, (LPARAM)&rcInvalid);
#else
            HDC hdc;

            hdc = GetClientDC (hWnd);
            SetBrushColor(hdc, pWin->iBkColor);
            FillBox(hdc, rcInvalid.left, rcInvalid.top, 
                         RECTW (rcInvalid), RECTH (rcInvalid));
            ReleaseDC (hdc);
#endif
        }
    }
    else if (pWin->WinType == TYPE_CONTROL) {
        
        rcClient.left = rcClient.top = 0;
        rcClient.right = pCtrl->cr - pCtrl->cl;
        rcClient.bottom = pCtrl->cb - pCtrl->ct;

        if (bEraseBkgnd) {
            if (prc && IntersectRect (&rcInter, prc, &rcClient)) {
                SendAsyncMessage ((HWND)pCtrl, 
                        MSG_ERASEBKGND, 0, (LPARAM)(&rcInter));
            }
            else if (!prc)
                SendAsyncMessage ((HWND)pCtrl, MSG_ERASEBKGND, 0, 0);
        }

        wndUpdateInvalidRegion (hWnd, prc, &rcClient);
    }
    else
        return FALSE;

    if (pCtrl->children)
        wndCascadeInvalidateChildren (pCtrl->children, prc, bEraseBkgnd);    
        
    return TRUE;
}

BOOL GUIAPI InvalidateRect (HWND hWnd, const RECT* prc, BOOL bEraseBkgnd)
{
    BOOL retval;

    retval = wndInvalidateRect (hWnd, prc, bEraseBkgnd);
    PostMessage (hWnd, MSG_PAINT, 0, 0);
    return retval;
}

BOOL GUIAPI GetUpdateRect (HWND hWnd, RECT* update_rect)
{
    PMAINWIN pWin;

    pWin = (PMAINWIN)hWnd;
    if (pWin->DataType != TYPE_HWND)
        return FALSE;

    *update_rect = pWin->InvRgn.rgn.rcBound;
    return TRUE;
}

HDC GUIAPI BeginPaint (HWND hWnd)
{
    PMAINWIN pWin;
    PINVRGN pInvRgn;
    HDC hdc;

    /* added by Zhongkai Du, */
    if (HWND_DESKTOP == hWnd) return CreatePrivateDC (hWnd);

    pWin = (PMAINWIN)hWnd;
    if (pWin->DataType != TYPE_HWND)
        return HDC_INVALID;

    hdc = GetClientDC (hWnd);

    pInvRgn = &pWin->InvRgn;

#ifndef _LITE_VERSION
    pthread_mutex_lock (&pInvRgn->lock);
#endif
    pInvRgn->frozen ++;

    // hide caret
    if (pWin->pCaretInfo && pWin->pCaretInfo->fBlink) {
        if (pWin->pCaretInfo->fShow)
#ifdef _USE_NEWGAL
            pWin->pCaretInfo->caret_bmp.bmBits = pWin->pCaretInfo->pNormal;
            FillBoxWithBitmap (hdc,
                        pWin->pCaretInfo->x, pWin->pCaretInfo->y, 0, 0, 
                        &pWin->pCaretInfo->caret_bmp);
#else
            PutSavedBoxOnDC (hdc, 
                        pWin->pCaretInfo->x, pWin->pCaretInfo->y,
                        pWin->pCaretInfo->nEffWidth,
                        pWin->pCaretInfo->nEffHeight,
                        pWin->pCaretInfo->pNormal);
#endif /* _USE_NEWGAL */
    }

    SelectClipRegion (hdc, &pInvRgn->rgn);
    EmptyClipRgn (&pInvRgn->rgn);
    
    pInvRgn->frozen --;

#ifndef _LITE_VERSION
    pthread_mutex_unlock (&pInvRgn->lock);
#endif

    return hdc;
}

void GUIAPI EndPaint (HWND hWnd, HDC hdc)
{
    int fbcon;	/* added by xm.chen */
    PMAINWIN pWin;

    /* added by Zhongkai Du, */
    if (HWND_DESKTOP == hWnd) { 
DeletePrivateDC (hdc); 

    /* added by xm.chen */
    fbcon = open("/dev/fb/2", O_RDWR, 0);
    if(fbcon < 0)
	printf("fb0 open failed!\n");
    if(-1 == ioctl(fbcon, FBIO_GFX_FLIP, NULL))
	printf("fb0 flip failed!\n");
    if(!(fbcon < 0)) 
	close(fbcon);

return; 
}

    pWin = (PMAINWIN)hWnd;
    if (pWin->DataType != TYPE_HWND)
        return ;

    // show caret
    if (pWin->pCaretInfo && pWin->pCaretInfo->fBlink) {
        if (pWin->pCaretInfo->fShow) {
            SetMapMode (hdc, MM_TEXT);
//            SelectClipRect (hdc, NULL);
            GetCaretBitmaps (pWin->pCaretInfo);
#ifdef _USE_NEWGAL
            pWin->pCaretInfo->caret_bmp.bmBits = pWin->pCaretInfo->pXored;
            FillBoxWithBitmap (hdc,
                        pWin->pCaretInfo->x, pWin->pCaretInfo->y, 0, 0, 
                        &pWin->pCaretInfo->caret_bmp);
#else
            PutSavedBoxOnDC (hdc, 
                        pWin->pCaretInfo->x, pWin->pCaretInfo->y,
                        pWin->pCaretInfo->nEffWidth,
                        pWin->pCaretInfo->nEffHeight,
                        pWin->pCaretInfo->pXored);
#endif /* _USE_NEWGAL */

        }
    }
    
    ReleaseDC (hdc);

    /* added by xm.chen */
    fbcon = open("/dev/fb/2", O_RDWR, 0);
    if(fbcon < 0)
	printf("fb0 open failed!\n");
    if(-1 == ioctl(fbcon, FBIO_GFX_FLIP, NULL))
	printf("fb0 flip failed!\n");
    if(!(fbcon < 0)) 
	close(fbcon);
}

BOOL RegisterWindowClass (PWNDCLASS pWndClass)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_REGISTERWNDCLASS, 0, (LPARAM)pWndClass) == ERR_OK;
}

BOOL UnregisterWindowClass (const char* szClassName)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_UNREGISTERWNDCLASS, 0, (LPARAM)szClassName) == ERR_OK;
}

BOOL GUIAPI GetWindowClassInfo (PWNDCLASS pWndClass)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_CTRLCLASSDATAOP, CCDOP_GETCCI, (LPARAM)pWndClass) == ERR_OK;
}

BOOL GUIAPI SetWindowClassInfo (const WNDCLASS* pWndClass)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_CTRLCLASSDATAOP, CCDOP_SETCCI, (LPARAM)pWndClass) == ERR_OK;
}

HWND GUIAPI CreateWindowEx (const char* spClassName, const char* spCaption,
                  DWORD dwStyle, DWORD dwExStyle, int id, 
                  int x, int y, int w, int h,
                  HWND hParentWnd, DWORD dwAddData)
{
    PMAINWIN pMainWin;
    PCTRLCLASSINFO cci;
    PCONTROL pNewCtrl;
    RECT rcExpect;

    if (!(pMainWin = GetMainWindowPtrOfControl (hParentWnd))) return HWND_INVALID;

    cci = (PCTRLCLASSINFO)SendMessage (HWND_DESKTOP, 
                MSG_GETCTRLCLASSINFO, 0, (LPARAM)spClassName);
                
    if (!cci) return HWND_INVALID;

    pNewCtrl = calloc (1, sizeof (CONTROL));

    if (!pNewCtrl) return HWND_INVALID;

    pNewCtrl->DataType = TYPE_HWND;
    pNewCtrl->WinType  = TYPE_CONTROL;

    pNewCtrl->left     = x;
    pNewCtrl->top      = y;
    pNewCtrl->right    = x + w;
    pNewCtrl->bottom   = y + h;

    memcpy (&pNewCtrl->cl, &pNewCtrl->left, sizeof (RECT));
    memcpy (&rcExpect, &pNewCtrl->left, sizeof (RECT));

    if (spCaption) {
        int len = strlen (spCaption);
        
        pNewCtrl->spCaption = FixStrAlloc (len);
        if (len > 0)
            strcpy (pNewCtrl->spCaption, spCaption);
    }
    else
        pNewCtrl->spCaption = "";
        
    pNewCtrl->dwStyle   = dwStyle | WS_CHILD | cci->dwStyle;
    pNewCtrl->dwExStyle = dwExStyle | cci->dwExStyle;

    pNewCtrl->iBkColor  = cci->iBkColor;
    pNewCtrl->hCursor   = cci->hCursor;
    pNewCtrl->hMenu     = 0;
    pNewCtrl->hAccel    = 0;
    pNewCtrl->hIcon     = 0;
    pNewCtrl->hSysMenu  = 0;
    if (pNewCtrl->dwExStyle & WS_EX_USEPARENTFONT)
        pNewCtrl->pLogFont = pMainWin->pLogFont;
    else
        pNewCtrl->pLogFont = GetSystemFont (SYSLOGFONT_CONTROL);

    pNewCtrl->id        = id;

    pNewCtrl->pCaretInfo = NULL;
    
    pNewCtrl->dwAddData = dwAddData;
    pNewCtrl->dwAddData2 = 0;

    pNewCtrl->ControlProc = cci->ControlProc;

    // Scroll bar
    if (pNewCtrl->dwStyle && WS_VSCROLL) {
        pNewCtrl->vscroll.minPos = 0;
        pNewCtrl->vscroll.maxPos = 100;
        pNewCtrl->vscroll.curPos = 0;
        pNewCtrl->vscroll.pageStep = 0;
        pNewCtrl->vscroll.barStart = 0;
        pNewCtrl->vscroll.barLen = 10;
        pNewCtrl->vscroll.status = SBS_NORMAL;
    }
    else
        pNewCtrl->vscroll.status = SBS_HIDE | SBS_DISABLED;

    if (pNewCtrl->dwStyle && WS_HSCROLL) {
        pNewCtrl->hscroll.minPos = 0;
        pNewCtrl->hscroll.maxPos = 100;
        pNewCtrl->hscroll.curPos = 0;
        pNewCtrl->hscroll.pageStep = 0;
        pNewCtrl->hscroll.barStart = 0;
        pNewCtrl->hscroll.barLen = 10;
        pNewCtrl->hscroll.status = SBS_NORMAL;
    }
    else
        pNewCtrl->hscroll.status = SBS_HIDE | SBS_DISABLED;

    pNewCtrl->children = NULL;
    pNewCtrl->active   = NULL;
    pNewCtrl->old_under_pointer = NULL;
    pNewCtrl->primitive = NULL;

    pNewCtrl->notif_proc = NULL;

    pNewCtrl->pMainWin = (PMAINWIN)pMainWin;
    pNewCtrl->pParent  = (PCONTROL)hParentWnd;
    pNewCtrl->next     = NULL;

    pNewCtrl->pcci     = cci;

    if (dwExStyle & WS_EX_CTRLASMAINWIN) {
        if ( !(pNewCtrl->pGCRInfo = malloc (sizeof (GCRINFO))) ) {
            goto error;
        }
        if ( !(pNewCtrl->pZOrderNode = malloc (sizeof(ZORDERNODE))) ) {
            goto error;
        }
    }
    else
        pNewCtrl->pGCRInfo = pMainWin->pGCRInfo;

    if (cci->dwStyle & CS_OWNDC) {
        pNewCtrl->privCDC = CreatePrivateClientDC ((HWND)pNewCtrl);
        pNewCtrl->dwExStyle |= WS_EX_USEPRIVATECDC;
    }
    else
        pNewCtrl->privCDC = 0;

    if (SendMessage ((HWND)pNewCtrl, MSG_NCCREATE, 0, (LPARAM)pNewCtrl)) {
        goto error;
    }

    SendMessage (HWND_DESKTOP, 
        MSG_NEWCTRLINSTANCE, (WPARAM)hParentWnd, (LPARAM)pNewCtrl);

    if (SendMessage ((HWND)pNewCtrl, MSG_CREATE, (WPARAM)hParentWnd, (LPARAM)dwAddData)) {
        SendMessage (HWND_DESKTOP, 
                MSG_REMOVECTRLINSTANCE, (WPARAM)hParentWnd, (LPARAM)pNewCtrl);
        goto error;
    }

    SendMessage ((HWND)pNewCtrl, MSG_SIZECHANGING, (WPARAM)&rcExpect, (LPARAM)&pNewCtrl->left);
    SendMessage ((HWND)pNewCtrl, MSG_CHANGESIZE, (WPARAM)(&pNewCtrl->left), 0);

    RecalcClientArea ((HWND)pNewCtrl);

    if (pNewCtrl->pParent->dwStyle & WS_VISIBLE && pNewCtrl->dwStyle & WS_VISIBLE)
        UpdateWindow ((HWND)pNewCtrl, TRUE);

    // reset static variables
    DefaultMouseMsgHandler (HWND_DESKTOP, MSG_WINDOWCHANGED, 0, 0, 0);

    return (HWND)pNewCtrl;

error:
    if (pNewCtrl->pGCRInfo) free (pNewCtrl->pGCRInfo);
    if (pNewCtrl->pZOrderNode) free (pNewCtrl->pZOrderNode);
    if (pNewCtrl->privCDC) DeletePrivateDC (pNewCtrl->privCDC);
    free (pNewCtrl);

    return HWND_INVALID;
}

BOOL GUIAPI DestroyWindow (HWND hWnd)
{
    PCONTROL pCtrl;
    PCONTROL pParent;
    
    if (!IsControl (hWnd)) return FALSE;

    if (SendMessage (hWnd, MSG_DESTROY, 0, 0))
        return FALSE;

    pCtrl = (PCONTROL)hWnd;
    pParent = pCtrl->pParent;
    if (pParent->active == (PCONTROL) hWnd)
        pParent->active = NULL;
    if (pParent->old_under_pointer == (PCONTROL) hWnd)
        pParent->old_under_pointer = NULL;
    if (pParent->primitive == (PCONTROL) hWnd)
        pParent->primitive = NULL;

    if (SendMessage (HWND_DESKTOP, 
        MSG_REMOVECTRLINSTANCE, (WPARAM)pParent, (LPARAM)pCtrl))
        return FALSE;

    DefaultMouseMsgHandler (HWND_DESKTOP, MSG_WINDOWCHANGED, 0, 0, 0);
    
    pCtrl->dwStyle &= ~WS_VISIBLE;
    if (IsWindowVisible ((HWND) pParent))
        InvalidateRect ((HWND) pParent, (PRECT)(&pCtrl->left), TRUE);

    if (pCtrl->privCDC)
        DeletePrivateDC (pCtrl->privCDC);

    ThrowAwayMessages (hWnd);

    if (pCtrl->dwExStyle & WS_EX_CTRLASMAINWIN) {
        EmptyClipRgn (&pCtrl->pGCRInfo->crgn);
        free (pCtrl->pGCRInfo);
        free (pCtrl->pZOrderNode);
    }
    EmptyClipRgn (&pCtrl->InvRgn.rgn);

    if (pCtrl->spCaption)
        FreeFixStr (pCtrl->spCaption);

    free (pCtrl);
    return TRUE;
}

NOTIFPROC GUIAPI SetNotificationCallback (HWND hwnd, NOTIFPROC notif_proc)
{
    NOTIFPROC old_proc;
    PCONTROL control = (PCONTROL)hwnd;

    if (hwnd == HWND_DESKTOP || control->DataType != TYPE_HWND)
        return NULL;

    old_proc = control->notif_proc;
    control->notif_proc = notif_proc;
    return old_proc;
}

NOTIFPROC GUIAPI GetNotificationCallback (HWND hwnd)
{
    PCONTROL control = (PCONTROL)hwnd;

    if (hwnd == HWND_DESKTOP || control->DataType != TYPE_HWND)
        return NULL;

    return control->notif_proc;
}

/****************************** Hooks support ********************************/
MSGHOOK GUIAPI RegisterKeyMsgHook (void* context, MSGHOOK hook)
{
    return (MSGHOOK)SendMessage (HWND_DESKTOP, 
            MSG_REGISTERKEYHOOK, (WPARAM)context, (LPARAM)hook);
}

MSGHOOK GUIAPI RegisterMouseMsgHook (void* context, MSGHOOK hook)
{
    return (MSGHOOK)SendMessage (HWND_DESKTOP, 
            MSG_REGISTERMOUSEHOOK, (WPARAM)context, (LPARAM)hook);
}

/**************************** IME support ************************************/
int GUIAPI RegisterIMEWindow (HWND hWnd)
{
    return SendMessage (HWND_DESKTOP, MSG_IME_REGISTER, (WPARAM)hWnd, 0);
}

int GUIAPI UnregisterIMEWindow (HWND hWnd)
{
    return SendMessage (HWND_DESKTOP, MSG_IME_UNREGISTER, (WPARAM)hWnd, 0);
}

int GUIAPI GetIMEStatus (int StatusCode)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_IME_GETSTATUS, (WPARAM)StatusCode, 0);
}

int GUIAPI SetIMEStatus (int StatusCode, int Value)
{
    return SendMessage (HWND_DESKTOP, 
            MSG_IME_SETSTATUS, (WPARAM)StatusCode, Value);
}

