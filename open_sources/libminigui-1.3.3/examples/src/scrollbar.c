/* 
** $Id: scrollbar.c,v 1.5 2003/06/13 07:15:49 weiym Exp $
**
** Listing 23.1
**
** scrollbar.c: Sample program for MiniGUI Programming Guide
**      Demo for scrollbar
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

static char* strLine[] = {
        "This is the 1st line.",
        "This is the 2nd line.",
        "This is the 3rd line.",
        "This is the 4th line.",
        "This is the 5th line.",
        "This is the 6th line.",
        "This is the 7th line.",
        "This is the 8th line.",
        "This is the 9th line.",
        "This is the 10th line.",
        "This is the 11th line.",
        "This is the 12th line.",
        "This is the 13th line.",
        "This is the 14th line.",
        "This is the 15th line.",
        "This is the 16th line.",
        "This is the 17th line."
};

static int ScrollWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static int iStart = 0;
    static int iStartPos = 0; 
    
    switch (message) {
        case MSG_CREATE:
            if (!CreateCaret (hWnd, NULL, 8, 14))
                fprintf (stderr, "Create caret error!\n");
        break;
        
        case MSG_SHOWWINDOW:
            EnableScrollBar (hWnd, SB_HORZ, FALSE);
            SetScrollRange (hWnd, SB_VERT, 0, 20);
            ShowCaret (hWnd);
        break;

        case MSG_SETFOCUS:
            ActiveCaret (hWnd);
            ShowCaret (hWnd);
        break;

        case MSG_KILLFOCUS:
            HideCaret (hWnd);
        break;
        
        case MSG_LBUTTONDOWN:
            SetCaretPos (hWnd, LOWORD (lParam), HIWORD (lParam));
        break;
        
        case MSG_LBUTTONDBLCLK:
            ShowScrollBar (hWnd, SB_HORZ, FALSE);
        break;

        case MSG_RBUTTONDBLCLK:
            ShowScrollBar (hWnd, SB_HORZ, TRUE);
        break;
       
        case MSG_HSCROLL:
            if (wParam == SB_LINERIGHT) {
                
                if (iStartPos < 5) {
                    iStartPos ++;
                    ScrollWindow (hWnd, -GetSysCharWidth (), 0, NULL, NULL);
                }
            }
            else if (wParam == SB_LINELEFT) {
                if (iStartPos > 0) {
                    iStartPos --;

                    ScrollWindow (hWnd, GetSysCharWidth (), 0, NULL, NULL);
                }
            }
        break;
        
        case MSG_VSCROLL:
            if (wParam == SB_LINEDOWN) {
                
                if (iStart < 12) {
                    iStart ++;
                    ScrollWindow (hWnd, 0, -20, NULL, NULL);
                }
            }
            else if (wParam == SB_LINEUP) {
                if (iStart > 0) {
                    iStart --;

                    ScrollWindow (hWnd, 0, 20, NULL, NULL);
                }
            }
            SetScrollPos (hWnd, SB_VERT, iStart);
        break;
        
        case MSG_PAINT:
        {
            HDC hdc;
            int i;
            RECT rcClient;
            
            GetClientRect (hWnd, &rcClient);
            
            hdc = BeginPaint (hWnd);
            for (i = 0; i < 17 - iStart; i++) {
                rcClient.left = 0;
                rcClient.right = (strlen (strLine [i + iStart]) - iStartPos)
                                 * GetSysCharWidth ();
                rcClient.top = i*20;
                rcClient.bottom = rcClient.top + 20;

                TextOut (hdc, 0, i*20, strLine [i + iStart] + iStartPos);
            }

            EndPaint (hWnd, hdc);
        }
        return 0;

        case MSG_CLOSE:
            DestroyCaret (hWnd);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static void InitCreateInfo(PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_BORDER | WS_CAPTION | WS_HSCROLL | WS_VSCROLL;
    pCreateInfo->dwExStyle = WS_EX_NONE | WS_EX_IMECOMPOSE;
    pCreateInfo->spCaption = "The scrollable main window" ;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = ScrollWinProc;
    pCreateInfo->lx = 0; 
    pCreateInfo->ty = 0;
    pCreateInfo->rx = 400;
    pCreateInfo->by = 280;
    pCreateInfo->iBkColor = COLOR_lightwhite; 
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

int MiniGUIMain(int args, const char* arg[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif

    InitCreateInfo(&CreateInfo);

    hMainWnd = CreateMainWindow(&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return 1;

    ShowWindow(hMainWnd, SW_SHOW);

    while (GetMessage(&Msg, hMainWnd) ) {
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup(hMainWnd);
    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

