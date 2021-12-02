/*
** $Id: worldtime.h,v 1.1 2003/08/19 10:41:36 weiym Exp $
**
** worldtime.h: worldtime for FHAS.
**
** Copyright (C) 2002, 2003 Feynman Software, all rights reserved.
**
** Use of this source package is subject to specific license terms
** from Beijing Feynman Software Technology Co., Ltd.
**
** URL: http://www.minigui.com
**
** Current maintainer: <Geng Yue> (<gengyue@minigui.org>).
**
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/mgext.h>

#define MAINWINDOW_LX               0
#define MAINWINDOW_TY               0

#define CAPTION_H                 14
#define STARTBAR_HEIGHT           0
#define MAINWINDOW_YY             160
#define MAINWINDOW_RX             160

#define MAINWINDOW_BY               (MAINWINDOW_YY-STARTBAR_H)

#define STARTBAR_H                0

#define IDC_TIMEZONE        100

#ifdef PDA_SIZE_240x320

#define ORG_X               5
#define ORG_Y               5

#define WIDTH_LABEL         230
#define HEIGHT_LABEL        20

#define WIDTH_ECHOTIME		230
#define HEIGHT_ECHOTIME		20

#define HEIGHT_TIME		    40

#else

#define ORG_X               5
#define ORG_Y               5

#define WIDTH_LABEL         150
#define HEIGHT_LABEL        14

#define WIDTH_ECHOTIME		150
#define HEIGHT_ECHOTIME		14

#define HEIGHT_TIME		    28

#endif

#define HEIGHT_LISTBOX      114

#define TIMER_CLOCK         100

const char *gettime(char *timezone);
const char *getday(char *timezone);
const char *getdate(char *timezone);
const char* getdatetime (char *timezone);
const char *getdatefull(char *timezone);

typedef struct tagTZINFO
{
	char timezone[20];
	char tzname[50];
}TZINFO;
