/* 
** $Id: mginit.c,v 1.8 2003/06/19 04:51:16 weiym Exp $
**
** Listing 31.1
**
** mginit.c: Sample program for MiniGUI Programming Guide
**      A simple mginit program.
**
** Copyright (C) 2003 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

static BOOL quit = FALSE;

static void on_new_del_client (int op, int cli)
{       
    static int nr_clients = 0;

    if (op == LCO_NEW_CLIENT) {
        nr_clients ++;
    }   
    else if (op == LCO_DEL_CLIENT) {
        nr_clients --;
        if (nr_clients == 0) {
            printf ("There is no any client, I will quit.\n");
            quit = TRUE;
        }               
        else if (nr_clients < 0) {
            printf ("Serious error: nr_clients less than zero.\n");
        }
    }
    else
        printf ("Serious error: incorrect operations.\n");
}

static pid_t exec_app (const char* file_name, const char* app_name)
{
    pid_t pid = 0;

    if ((pid = vfork ()) > 0) {
        fprintf (stderr, "new child, pid: %d.\n", pid);
    }
    else if (pid == 0) {
        execl (file_name, app_name, NULL);
        perror ("execl");
        _exit (1);
    }
    else {
        perror ("vfork");
    }

    return pid;
}

static unsigned int old_tick_count;

static pid_t pid_scrnsaver = 0;

static int my_event_hook (PMSG msg)
{
    old_tick_count = GetTickCount ();

    if (pid_scrnsaver) {
        kill (pid_scrnsaver, SIGINT);
        ShowCursor (TRUE);
        pid_scrnsaver = 0;
    }

    if (msg->message == MSG_KEYDOWN) {
        switch (msg->wParam) {
            case SCANCODE_F1:
               exec_app ("./edit", "edit");
               break;
            case SCANCODE_F2:
               exec_app ("./timeeditor", "timeeditor");
               break;
            case SCANCODE_F3:
               exec_app ("./propsheet", "propsheet");
               break;
            case SCANCODE_F4:
               exec_app ("./bmpbkgnd", "bmpbkgnd");
               break;
	}
    }

    return HOOK_GOON;
}

int MiniGUIMain (int args, const char* arg[])
{
    MSG msg;

    OnNewDelClient = on_new_del_client;

    if (!ServerStartup ()) {
        fprintf (stderr, "Can not start mginit.\n");
        return 1;
    }
    
    if (SetDesktopRect (0, 1024, 0, 1024) == 0) {
        fprintf (stderr, "Empty desktop rect.\n");
        return 2;
    }

    SetServerEventHook (my_event_hook);

    if (exec_app ("./helloworld", "helloworld") == 0)
        return 3;

    old_tick_count = GetTickCount ();

    while (!quit && GetMessage (&msg, HWND_DESKTOP)) {
        if (pid_scrnsaver == 0 && GetTickCount () > old_tick_count + 1000) {
            ShowCursor (FALSE);
            pid_scrnsaver = exec_app ("./scrnsaver", "scrnsaver");
        }
        DispatchMessage (&msg);
    }

    return 0;
}

