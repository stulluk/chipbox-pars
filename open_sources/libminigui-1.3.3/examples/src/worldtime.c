/*
** $Id: worldtime.c,v 1.1 2003/08/19 10:41:36 weiym Exp $
**
** worldtime.c: worldtime for FHAS.
**
** Copyright (C) 2002, 2003 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: Wei Yongming (ymwei@minigui.org).
**
*/

#include "worldtime.h"

static const char* timezones [] =
{
    "GMT-12 夸贾林岛",
    "GMT-11 中途岛，东萨摩亚",
    "GMT-10 夏威夷",
    "GMT-9 阿拉斯加",
    "GMT-8 洛杉矶，旧金山",
    "GMT-8 温哥华",
    "GMT-7 亚利桑那",
    "GMT-6 圣路易斯，芝加哥",
    "GMT-6 墨西哥城，蒙特雷",
    "GMT-6 温尼伯",
    "GMT-6 中美洲",
    "GMT-5 华盛顿，纽约，波士顿",
    "GMT-5 多伦多",
    "GMT-5 波哥大，利马，哈瓦那",
    "GMT-4 加拉加斯，拉巴斯",
    "GMT-4 圣地亚哥",
    "GMT-3:30 纽芬兰",
    "GMT-3 圣保罗，里约热内卢",
    "GMT-3 布宜诺斯艾利斯",
    "GMT-3 格陵兰",
    "GMT-2 大西洋中部",
    "GMT-1 亚速尔群岛",
    "GMT+0 伦敦，都柏林",
    "GMT+0 卡萨布兰卡",
    "GMT+1 阿姆斯特丹，巴塞罗那",
	"GMT+1 贝尔格莱德，柏林",
	"GMT+1 布鲁塞尔，布达佩斯",
	"GMT+1 哥本哈根，日内瓦",
    "GMT+1 里斯本，马德里，巴黎",
    "GMT+1 罗马，斯德哥尔摩",
    "GMT+1 中非西部",
    "GMT+2 雅典，布加勒斯特",
    "GMT+2 开罗，赫尔辛基",
    "GMT+2 伊斯坦布尔，耶路撒冷",
    "GMT+3 巴格达，科威特",
    "GMT+3 莫斯科，慕尼黑",
    "GMT+3 利雅得，内罗毕",
    "GMT+3:30 德黑兰",
    "GMT+4 阿布扎比，迪拜",
    "GMT+4:30 喀布尔",
    "GMT+5 伊斯兰堡",
	"GMT+5:30 新德里，孟买",
    "GMT+5:30 斯里兰卡",
    "GMT+5:45 加德满都",
    "GMT+6 阿斯坦纳，达卡",
    "GMT+6:30 仰光",
    "GMT+7 曼谷，河内，雅加达",
    "GMT+8 北京，香港特别行政区",
    "GMT+8 台北，澳门",
    "GMT+8 吉隆坡，新加坡",
    "GMT+8 马尼拉",
    "GMT+8 乌兰巴托",
    "GMT+9 汉城，平壤",
    "GMT+9 东京，大阪",
    "GMT+9:30 达尔文",
    "GMT+10 悉尼，墨尔本",
    "GMT+10 堪培拉，布里斯班",
    "GMT+10 符拉迪沃斯托克",
    "GMT+10 关岛",
    "GMT+11 马加丹",
    "GMT+12 新西兰",
    "GMT+13 汤加",
};

static void on_create (HWND hWnd)
{
    int i;
    HWND wnd_tzs;

    CreateWindow (CTRL_STATIC,
        "系统时间：",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, ORG_Y,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd, 0);

    CreateWindow (CTRL_STATIC,
        "北京时间：",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, ORG_Y + HEIGHT_TIME,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd,0);

    CreateWindow (CTRL_STATIC,
        "格林威治时间：",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 2,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd, 0);

    CreateWindow (CTRL_STATIC,
        "请选择其他时区：",
        WS_VISIBLE | SS_SIMPLE,
        IDC_STATIC,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 3,
        WIDTH_LABEL, HEIGHT_LABEL,
        hWnd,0);

    wnd_tzs = CreateWindow (CTRL_COMBOBOX,
        "未选定时区",
        WS_VISIBLE | CBS_DROPDOWNLIST | CBS_NOTIFY | CBS_READONLY,
        IDC_TIMEZONE,
        ORG_X, 
        ORG_Y + HEIGHT_TIME * 3 + HEIGHT_LABEL,
        MAINWINDOW_RX - ORG_X*2, HEIGHT_LABEL,
        hWnd, HEIGHT_LISTBOX);

    for (i = 0; i < TABLESIZE (timezones); i++) {
        SendMessage (wnd_tzs, CB_ADDSTRING, 0, (LPARAM)timezones [i]);
    }
}

/* Main WindProc function */
static int WorldTimeWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static char timezone [20];
    static const char *pdate;
    switch (message) {
        case MSG_CREATE:
            on_create (hWnd);
            SetTimer (hWnd, TIMER_CLOCK, 10);
        break;

        break;

        case MSG_COMMAND:
        {
            int id   = LOWORD (wParam);
            int code = HIWORD (wParam);
            if (id == IDC_TIMEZONE && code == LBN_SELCHANGE) {
                int selected = SendDlgItemMessage (hWnd, id, CB_GETCURSEL, 0, 0);
                if (selected >= 0) {

                    int space;
                    HDC hdc;

                    strcpy (timezone, "TZ=");
                    space = strchr (timezones [selected], ' ') - timezones [selected];
                    strncat (timezone, timezones [selected], space);
                    if (timezone [6] == '+')
                        timezone [6] = '-';
                    else
                        timezone [6] = '+';

                    hdc = GetClientDC (hWnd);
                    pdate = getdate (timezone);
                    TextOut (hdc, ORG_X, ORG_Y + HEIGHT_TIME * 4 + 10, pdate);
                    ReleaseDC (hdc);
                }
            }

            break;
        }

        case MSG_TIMER:
                if (wParam == TIMER_CLOCK) {
                    HDC hdc;
                    char *London;
                    char *BeiJing;
    
                    hdc = GetClientDC(hWnd);
                    SetBkColor (hdc, COLOR_lightwhite);

                    TextOut (hdc, ORG_X,
                                  ORG_Y + HEIGHT_LABEL,
                                  getdatetime ("TZ=GMT-8"));

                    BeiJing = "TZ=GMT-8";
                    TextOut (hdc, ORG_X , 
                                  ORG_Y + HEIGHT_TIME + HEIGHT_LABEL,
                                  gettime(BeiJing));

                    London = "TZ=GMT+0";
                    TextOut (hdc, ORG_X,
                                  ORG_Y + HEIGHT_TIME * 2 + HEIGHT_LABEL,
                                  gettime(London));

                    if (pdate != NULL) {
                        pdate = getdate (timezone);
                        TextOut (hdc, ORG_X, ORG_Y + HEIGHT_TIME * 4 + 10, pdate);
                    }

                    ReleaseDC (hdc);
                }
            break;
    
       case MSG_MAXIMIZE:
            break;

       case MSG_KEYDOWN:
            break;

       case MSG_CLOSE:
            pdate = NULL;
            KillTimer (hWnd, TIMER_CLOCK);
            DestroyAllControls (hWnd);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

static void InitCreateInfo(PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_CAPTION | WS_MAXIMIZEBOX | WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_NONE;
    pCreateInfo->spCaption = "世界时" ;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = WorldTimeWinProc;
    pCreateInfo->lx = MAINWINDOW_LX; 
    pCreateInfo->ty = MAINWINDOW_TY;
    pCreateInfo->rx = MAINWINDOW_RX;
    pCreateInfo->by = MAINWINDOW_BY;
    pCreateInfo->iBkColor = COLOR_lightwhite; 
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = HWND_DESKTOP;
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG Msg;
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

#ifdef _LITE_VERSION
    SetDesktopRect(DESKTOP_LX,DESKTOP_TY,DESKTOP_RX,DESKTOP_BY);
#endif

    InitCreateInfo(&CreateInfo);
    hMainWnd = CreateMainWindow(&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
#ifdef _LITE_VERSION
        return 3;
#else
        return NULL;
#endif

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while(GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }    
    
    MainWindowThreadCleanup(hMainWnd);

    return 0;
}

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

