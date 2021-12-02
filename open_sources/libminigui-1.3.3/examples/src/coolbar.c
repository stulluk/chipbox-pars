/* 
** $Id: coolbar.c,v 1.1 2003/06/13 10:19:00 snig Exp $
**
** Listing 17.1
**
** coolbar.c: Sample program for MiniGUI Programming Guide
**      Usage of COOLBAR control.
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
#include <minigui/mgext.h>

#define ITEM_NUM	10

static const char* caption[] =
{
   "0", "1", "2", "3", "4", "5","6", "7", "8", "9"
};

static const char* hint[] =
{
   "数字 0", "数字 1", "数字 2", "数字 3", "数字 4", 
   "数字 5", "数字 6", "数字 7", "数字 8", "数字 9"
};

static void create_coolbar (HWND hWnd)
{
    HWND cb;
    COOLBARITEMINFO item;
    int i;

    cb = CreateWindow (CTRL_COOLBAR,
                    "",
                    WS_CHILD | WS_VISIBLE | WS_BORDER, 
                    100,
                    10, 100, 100, 20,
                    hWnd,
                    0);

    item.ItemType = TYPE_TEXTITEM;
    item.Bmp = NULL;
    item.dwAddData = 0;
    for (i = 0; i < ITEM_NUM; i++) {
	item.insPos = i;
	item.id = i;
	item.Caption = caption[i];
	item.ItemHint = hint[i];
	SendMessage (cb, CBM_ADDITEM, 0, (LPARAM)&item);
    }
}

static int CoolbarWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static HWND ed;

    switch (message) {
    case MSG_CREATE:
    ed = CreateWindow (CTRL_EDIT,
                    "",
                    WS_CHILD | WS_VISIBLE | WS_BORDER, 
                    200,
                    10, 10, 100, 20,
                    hWnd,
                    0);

	create_coolbar (hWnd);
        break;

    case MSG_COMMAND:
	{
	    int id = LOWORD (wParam);
	    int code = HIWORD (wParam);

	    if (id == 100) {
		static char buffer[100];
		char buf[2];

		sprintf (buf, "%d", code);
		SendMessage (ed, MSG_GETTEXT, 90, (LPARAM)buffer);
		strcat (buffer, buf);
		SendMessage (ed, MSG_SETTEXT, 0, (LPARAM)buffer);
	    }
	}
	break;

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
    if (!InitMiniGUIExt()) {
        return 2;
    }

    CreateInfo.dwStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "酷工具栏";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = CoolbarWinProc;
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

    MiniGUIExtCleanUp ();
    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif
