#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
 
#include "cshdmi.h"

static void HDMI_INIT(int argc, char *argv[])
{
	CSHDMI_Init();
}

static void HDMI_TERMINATE(int argc, char *argv[])
{
	CSHDMI_Terminate();
}

static struct cmd_t cshdmi_tt[] = {
 	{
 	 "hdmi_init",
 	 NULL,
 	 HDMI_INIT,
 	 "hdmi_init: to initialize HDMI. \
 \n    usage: hdmi_init \
 \n       eg: hdmi_init \n"},
 
 	{
 	 "hdmi_terminate",
 	 NULL,
 	 HDMI_TERMINATE,
 	 "hdmi_terminate: to terminate HDMI. \
 \n    usage: hdmi_terminate \
 \n       eg: hdmi_terminate \n"}
};
