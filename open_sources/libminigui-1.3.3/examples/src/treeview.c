/*
** $Id: treeview.c,v 1.1 2003/06/16 02:58:20 snig Exp $
**
** treeview.c: Sample program for MiniGUI Programming Guide
**      Usage of TREEVIEW control.
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


#define IDC_TREEVIEW  100


#define CHAPTER_NUM	5

static const char *chapter[] = 
{
    "第十六章 树型控件",
    "第十七章 列表型控件",
    "第十八章 月历控件",
    "第十九章 旋钮控件",
    "第二十章 酷工具栏控件",
};

static const char *section[] =
{
    "控件风格",
    "控件消息",
    "控件通知码"
};

static int BookProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{

    switch (message) {
        case MSG_INITDIALOG:
        {
            TVITEMINFO tvItemInfo;
	    int item;
	    int i, j;

	    for (i = 0; i < CHAPTER_NUM; i++) {
		tvItemInfo.text = (char*)chapter[i];
	        item = SendMessage (GetDlgItem(hDlg, IDC_TREEVIEW), TVM_ADDITEM, 
			    0, (LPARAM)&tvItemInfo);
		for (j = 0; j < 3; j++) {
		    tvItemInfo.text = (char*)section[j];
	            SendMessage (GetDlgItem(hDlg, IDC_TREEVIEW), TVM_ADDITEM, 
			    item, (LPARAM)&tvItemInfo);
		}
	    }
        }
        break;

        case MSG_COMMAND:
        {
            //int id   = LOWORD(wParam);
            //int code = HIWORD(wParam);
        }
        break;
        
        case MSG_CLOSE:
	    EndDialog (hDlg, 0);
            return 0;
    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static TVITEMINFO bookInfo =
{
    "MiniGUI编程指南"
};

static DLGTEMPLATE DlgBook =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 320, 240,
    "Book Content",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA CtrlBook[] =
{
    {
        "treeview",
        WS_BORDER | WS_CHILD | WS_VISIBLE | 
		WS_VSCROLL | WS_HSCROLL,
         10, 10, 280, 180,
        IDC_TREEVIEW,
        "treeview control",
        (LPARAM)&bookInfo
    }
};

int MiniGUIMain (int argc, const char* argv[])
{
#ifdef _LITE_VERSION
    SetDesktopRect(0, 0, 1024, 768);
#endif
    
    if (!InitMiniGUIExt()) {
        return 2;
    }

    DlgBook.controls = CtrlBook;
    
    DialogBoxIndirectParam (&DlgBook, HWND_DESKTOP, BookProc, 0L);

    MiniGUIExtCleanUp ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

