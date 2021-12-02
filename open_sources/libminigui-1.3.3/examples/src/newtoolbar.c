/* 
** $Id: newtoolbar.c,v 1.4 2003/07/10 04:22:37 weiym Exp $
**
** Listing 13.1
**
** newtooltar.c: Sample program for MiniGUI Programming Guide
**      Usage of NEWTOOLBAR control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define IDC_NTB_LEFT  100
#define IDC_NTB_RIGHT 110
#define IDC_NTB_UP    120

static int offset = 0;
static RECT rcCircle = {0, 40, 300, 300};

static void my_notif_proc (HWND hwnd, int id, int nc, DWORD add_data)
{
    if (nc == IDC_NTB_LEFT) {
        offset -= 10;
        InvalidateRect (GetParent (hwnd), &rcCircle, TRUE);
    }
    else if (nc == IDC_NTB_RIGHT) {
        offset += 10;
        InvalidateRect (GetParent (hwnd), &rcCircle, TRUE);
    }
}

static BITMAP ntb_bmp;

static void create_new_toolbar (HWND hWnd)
{
    HWND ntb;
    NTBINFO ntb_info;
    NTBITEMINFO ntbii;
    gal_pixel pixel;

    ntb_info.nr_cells = 4;
    ntb_info.w_cell  = 0;
    ntb_info.h_cell  = 0;
    ntb_info.nr_cols = 0;
    ntb_info.image = &ntb_bmp;

    ntb = CreateWindow (CTRL_NEWTOOLBAR,
                    "",
                    WS_CHILD | WS_VISIBLE, 
                    100,
                    0, 0, 1024, 0,
                    hWnd,
                    (DWORD) &ntb_info);

    SetNotificationCallback (ntb, my_notif_proc);

    pixel = GetPixelInBitmap (&ntb_bmp, 0, 0);
    SetWindowBkColor (ntb, pixel);
    InvalidateRect (ntb, NULL, TRUE);

    memset (&ntbii, 0, sizeof (ntbii));
    ntbii.flags = NTBIF_PUSHBUTTON;
    ntbii.id = IDC_NTB_LEFT;
    ntbii.bmp_cell = 1;
    SendMessage(ntb, TBM_ADDITEM, 0, (LPARAM)&ntbii);

    ntbii.flags = NTBIF_PUSHBUTTON;
    ntbii.id = IDC_NTB_RIGHT;
    ntbii.bmp_cell = 2;
    SendMessage (ntb, TBM_ADDITEM, 0, (LPARAM)&ntbii);

    ntbii.flags = NTBIF_SEPARATOR;
    ntbii.id = 0;
    ntbii.bmp_cell = 0;
    ntbii.text = NULL;
    SendMessage (ntb, TBM_ADDITEM, 0, (LPARAM)&ntbii);

    ntbii.flags = NTBIF_PUSHBUTTON | NTBIF_DISABLED;
    ntbii.id = IDC_NTB_UP;
    ntbii.bmp_cell = 0;
    SendMessage (ntb, TBM_ADDITEM, 0, (LPARAM)&ntbii);
}

static int ToolBarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_CREATE:
        if (LoadBitmap (HDC_SCREEN, &ntb_bmp, "new2.jpg"))
            return -1;

        create_new_toolbar (hWnd);
        break;

    case MSG_PAINT:
    {
        HDC hdc = BeginPaint (hWnd);

        ClipRectIntersect (hdc, &rcCircle);

        SetBrushColor (hdc, PIXEL_red);
#if _USE_NEWGAL
        FillCircle (hdc, 140 + offset, 120, 50);
#else
        Circle (hdc, 140 + offset, 120, 50);
#endif

        EndPaint (hWnd, hdc);
        return 0;
    }

    case MSG_DESTROY:
        UnloadBitmap (&ntb_bmp);
        DestroyAllControls (hWnd);
        return 0;

    case MSG_CLOSE:
        DestroyMainWindow (hWnd);
        PostQuitMessage (hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif

    CreateInfo.dwStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "π§æﬂ¿∏∫Õ‘≤";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ToolBarWinProc;
    CreateInfo.lx = 0; 
    CreateInfo.ty = 0;
    CreateInfo.rx = 300;
    CreateInfo.by = 220;
    CreateInfo.iBkColor = GetWindowElementColor (BKC_CONTROL_DEF);
    CreateInfo.dwAddData = 0;
    CreateInfo.dwReserved = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
