/* 
** $Id: mycontrol.c,v 1.4 2003/06/13 06:50:39 weiym Exp $
**
** Listing 3.1
**
** mycontrol.c: Sample program for MiniGUI Programming Guide
**      Use my own control to print "Hello, world!".
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define MY_CTRL_NAME "mycontrol"

static int MyControlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    switch (message) {
    case MSG_PAINT:
        /* Output hello, world */
        hdc = BeginPaint (hwnd);
        TextOut (hdc, 0, 0, "Hello, world! - from my control.");
        EndPaint (hwnd, hdc);
        return 0;
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

static BOOL RegisterMyControl (void)
{
    WNDCLASS MyClass;

    MyClass.spClassName = MY_CTRL_NAME;
    MyClass.dwStyle     = 0;
    MyClass.hCursor     = GetSystemCursor (IDC_ARROW);
    MyClass.iBkColor    = COLOR_lightwhite;
    MyClass.WinProc     = MyControlProc;

    return RegisterWindowClass (&MyClass);
}

static void UnregisterMyControl (void)
{
    UnregisterWindowClass (MY_CTRL_NAME);
}

static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_CREATE:
            CreateWindow (MY_CTRL_NAME, "", WS_VISIBLE, IDC_STATIC, 100, 100, 200, 20, hWnd, 0);
            return 0;

        case MSG_DESTROY:
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

    RegisterMyControl ();

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "Hello, world";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
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
    UnregisterMyControl ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

