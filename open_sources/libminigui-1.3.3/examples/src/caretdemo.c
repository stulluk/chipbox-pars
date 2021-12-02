/* 
** $Id: caretdemo.c,v 1.3 2003/06/13 06:50:39 weiym Exp $
**
** Listing 25.4
**
** caretdemo.c: Sample program for MiniGUI Programming Guide
**      Demo of using caret
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

#define IDC_MYEDIT    100

/* a simple edit window */
static int MyeditWindowProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static char *pBuffer = NULL;
    static int pos = 0, len = 0;
    HDC hdc;

    switch (message) {
    case MSG_CREATE:
        SetWindowFont(hWnd, GetSystemFont(SYSLOGFONT_WCHAR_DEF));
        if (!CreateCaret (hWnd, NULL, 1, GetSysCharHeight())) {
            return -1;
        }
        pBuffer = (char *) malloc(10);
        *pBuffer = 0;
    break;

    case MSG_SETFOCUS:
        SetCaretPos(hWnd, pos*GetSysCharWidth(), 0);
        ShowCaret(hWnd);
    break;

    case MSG_KILLFOCUS:
        HideCaret(hWnd);
    break;

    case MSG_CHAR:
        switch (wParam) {
            case '\t':
            case '\b':
            case '\n':
                SetCaretBlinkTime(hWnd, GetCaretBlinkTime(hWnd)-100);
                break;

            default:
            {
                char ch, buf[10];
                char *tmp;
                ch = wParam;
                if (len == 10)
                    break;
                tmp = pBuffer+pos;
                if (*tmp != 0) {
                    strcpy(buf, tmp);
                    strcpy(tmp+1, buf);
                }
                *tmp = ch;
                pos++;
                len++;
              }
              break;
           }
        break;

    case MSG_KEYDOWN:
        switch (wParam) {
            case SCANCODE_CURSORBLOCKLEFT:
                pos = MAX(pos-1, 0);
                break;
            case SCANCODE_CURSORBLOCKRIGHT:
                pos = MIN(pos+1, len);
                break;
            case SCANCODE_BACKSPACE:
            {
                char buf[10];
                char *tmp;
                if (len == 0 || pos == 0)
                    break;
                tmp = pBuffer+pos;
                strcpy(buf, tmp);
                strcpy(tmp-1, buf);
                pos--;
                len--;
                break;
            }
        }
        SetCaretPos(hWnd, pos*GetSysCharWidth(), 0);
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case MSG_PAINT:
        hdc = BeginPaint(hWnd);
        TextOut(hdc, 0, 0, pBuffer);
        EndPaint(hWnd, hdc);
        return 0;

    case MSG_DESTROY:
        DestroyCaret (hWnd);
        if (pBuffer)
            free(pBuffer);
        return 0;
    }

    return DefaultControlProc(hWnd, message, wParam, lParam);
}

BOOL RegisterMyedit(void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = "myedit";
    WndClass.dwStyle     = 0;
    WndClass.dwExStyle   = 0;
    WndClass.hCursor     = GetSystemCursor(IDC_IBEAM);
    WndClass.iBkColor    = PIXEL_lightwhite;
    WndClass.WinProc     = MyeditWindowProc;

    return RegisterWindowClass (&WndClass);
}

/* main windoww proc */
static int CaretdemoWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hMyedit;

    switch (message) {
    case MSG_CREATE:
        RegisterMyedit();
        hMyedit = CreateWindow("myedit", "", WS_VISIBLE | WS_CHILD, IDC_MYEDIT, 
                30, 50, 100, 20, hWnd, 0);
        SetFocus(hMyedit);
        break;

    case MSG_CLOSE:
        DestroyAllControls (hWnd);
        DestroyMainWindow (hWnd);
        PostQuitMessage (hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "using caret demo";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = CaretdemoWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 320;
    CreateInfo.by = 240;
    CreateInfo.iBkColor = COLOR_lightgray;
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

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

