/*
** $Id: spinbox.c,v 1.2 2003/06/13 08:12:08 snig Exp $
**
** spinbox.c: Sample program for MiniGUI Programming Guide
**      Usage of SPINBOX control.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>


#define IDC_SPIN	10
#define IDC_CTRL1       20
#define IDC_CTRL2       30
#define IDC_CTRL3       40
#define IDC_CTRL4       50


static int
SpinProc (HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hSpin;
    SPININFO spinfo;

    hSpin = GetDlgItem (hDlg, IDC_SPIN);

    switch (message)
    {

    case MSG_INITDIALOG:
    {
	spinfo.min = 1;
	spinfo.max = 10;
	spinfo.cur = 1;
	SendMessage (hSpin, SPM_SETTARGET, 0, (LPARAM)hDlg);
	SendMessage (hSpin, SPM_SETINFO, 0, (LPARAM)&spinfo);
    }
    break;

    case MSG_KEYDOWN:
    {
	if (wParam == SCANCODE_CURSORBLOCKUP || 
			wParam == SCANCODE_CURSORBLOCKDOWN) {
	    if (!(lParam & KS_SPINPOST)) {
		int cur;
	        cur = SendMessage (hSpin, SPM_GETCUR, 0, 0);
		if (wParam == SCANCODE_CURSORBLOCKUP)
		    cur --;
		else
		    cur ++;
	        SendMessage (hSpin, SPM_SETCUR, cur, 0);
	    }
	    InvalidateRect (hDlg, NULL, TRUE);
	}
    }
    break;

    case MSG_PAINT:
    {
	HDC hdc;
	int x, y, w, h;
	int cur;

	cur = SendMessage (hSpin, SPM_GETCUR, 0, (LPARAM)&spinfo);
	x = 10;
	y = cur*10;
	w = 60;
	h = 10;

	if (y < 10)
	    y = 10;
	else if (y > 100)
	    y = 100;

	hdc = BeginPaint (hDlg);
	MoveTo (hdc, 2, 10);
	LineTo (hdc, 100, 10);
	Rectangle (hdc, x, y, x+w, y+h);
	SetBrushColor (hdc, PIXEL_black);
	FillBox (hdc, x, y, w, h);
	MoveTo (hdc, 2, 110);
	LineTo (hdc, 100, 110);
	EndPaint (hDlg, hdc);
    }
    break;

    case MSG_CLOSE:
    {
	EndDialog (hDlg, 0);
    }
    break;

    }

    return DefaultDialogProc (hDlg, message, wParam, lParam);
}

static DLGTEMPLATE DlgSpin =
{
    WS_BORDER | WS_CAPTION,
    WS_EX_NONE,
    100, 100, 320, 240,
    "spinbox and black block",
    0, 0,
    1, NULL,
    0
};

static CTRLDATA CtrlSpin[] =
{
    {
        CTRL_SPINBOX,
        SPS_AUTOSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE,
         200, 120, 0, 0,
        IDC_SPIN,
        "",
        0
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

    DlgSpin.controls = CtrlSpin;
    
    DialogBoxIndirectParam (&DlgSpin, HWND_DESKTOP, SpinProc, 0L);

    MiniGUIExtCleanUp ();

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

