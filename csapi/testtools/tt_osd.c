#include <stdio.h>
#include <stdlib.h>
 
#include "csosd.h"

static void OSD_ENABLE(int argc, char *argv[])
{
	int layer;
	int enable;

	if (argc < 3) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);
	enable = atoi(argv[2]);
	
	CSOSD_HANDLE osd_handler = CSOSD_Open(layer);
	if (enable) {
		CSOSD_Enable(osd_handler);
		printf("CSOSD_Enabled!\n");
	} else {
		CSOSD_Disable(osd_handler);
		printf("CSOSD_Disabled!\n");
	}
	CSOSD_Close(osd_handler);
}

static void OSD_SETCONFIG(int argc, char *argv[])
{
	int mode;
	int layer;
	int format;
	CSOSD_HANDLE osd_handler;
	CSOSD_Config conf;

	if (argc < 4) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);
	mode = atoi(argv[2]);
	format = atoi(argv[3]);

	conf.mode = mode;
	conf.color_format = format; //OSD_COLOR_FORMAT_RGB565;

	osd_handler = CSOSD_Open(layer);
	CSOSD_SetConfig(osd_handler, &conf);
	CSOSD_Close(osd_handler);
}

static void OSD_SETALPHA(int argc, char *argv[])
{
	int alpha;
	int layer;
	CSOSD_HANDLE osd_handler;

	if (argc < 3) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);
	alpha = atoi(argv[2]);
	if (alpha < 0 || alpha > 255) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	osd_handler = CSOSD_Open(layer);
	CSOSD_SetAlpha(osd_handler, alpha);
	CSOSD_Close(osd_handler);
}

static void OSD_ENABLEKEYCOLOR(int argc, char *argv[])
{
	int layer;
	int enable;

	if (argc < 3) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);
	enable = atoi(argv[2]);

	CSOSD_HANDLE osd_handler = CSOSD_Open(layer);
	if (enable) {
		CSOSD_EnableKeyColor(osd_handler);
		printf("CSOSD_EnableKeyColor!\n");
	} else {
		CSOSD_DisableKeyColor(osd_handler);
		printf("CSOSD_DisableKeyColor!\n");
	}
	CSOSD_Close(osd_handler);
}

static void OSD_SETKEYCOLOR(int argc, char *argv[])
{
	int layer;
	int v1, v2, v3, v4, v5, v6;
	CSOSD_HANDLE osd_handler;
	CSOSD_KeyColor key_color;

	if (argc < 8) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);

	v1 = atoi(argv[2]);
	v2 = atoi(argv[3]);
	v3 = atoi(argv[4]);
	v4 = atoi(argv[5]);
	v5 = atoi(argv[6]);
	v6 = atoi(argv[7]);

	key_color.r_min = v1;
	key_color.r_max = v2;
	key_color.g_min = v3;
	key_color.g_max = v4;
	key_color.b_min = v5;
	key_color.b_max = v6;

	osd_handler = CSOSD_Open(layer);
	CSOSD_SetKeyColor(osd_handler, &key_color);
	CSOSD_Close(osd_handler);
}

static void OSD_FLIP(int argc, char *argv[])
{
	int layer;

	if (argc < 2) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	layer = atoi(argv[1]);

	CSOSD_HANDLE osd_handler = CSOSD_Open(layer);
	if (argc > 9) {
		CSOSD_Rect src_rect, dst_rect;
		src_rect.left = atoi(argv[2]);
		src_rect.top = atoi(argv[3]);
		src_rect.right = atoi(argv[4]);
		src_rect.bottom = atoi(argv[5]);
		dst_rect.left = atoi(argv[6]);
		dst_rect.top = atoi(argv[7]);
		dst_rect.right = atoi(argv[8]);
		dst_rect.bottom = atoi(argv[9]);
		CSOSD_FlipRect(osd_handler, &src_rect, &dst_rect);
	}
	else 
		CSOSD_Flip(osd_handler);
	CSOSD_Close(osd_handler);
}

static struct cmd_t csosd_tt[] = {
 	{
 	 "osd_enable",
 	 NULL,
 	 OSD_ENABLE,
 	 "osd_enable - to open / close a specified Gfx layer\
 \n    usage: osd_enable <layerid><flag>\
 \n       eg: osd_enable 0 0\n"},
 	{
 	 "osd_conf",
 	 NULL,
 	 OSD_SETCONFIG,
 	 "osd_conf \
 \n    usage: osd_conf <layer><mode><format>, the follow values are supported - \
 \n    mode:  0 - 720x576 \
 \n           1 - 720X480 \
 \n           2 - 1280X720 \
 \n           3 - 1920X1080 \
 \n\
 \n  format:  0 - RGB565 \
 \n           1 - ARGB4444 \
 \n           2 - ARGB1555 \
 \n\
 \n       eg: osd_conf 0 0 0 \n"},
 
 	{
 	 "osd_setalpha",
 	 NULL,
 	 OSD_SETALPHA,
 	 "osd_setalpha - to set alpha value of OSD layer.\
 \n    usage: osd_setalpha <layer> <alpha val, 1 ~ 254> \
 \n       eg: osd_setalpha 0 80 \n"},
 
 	{
 	 "osd_keycolor_enable",
 	 NULL,
 	 OSD_ENABLEKEYCOLOR,
 	 "osd_keycolor_enable - to enable/disable key color\
 \n    usage: osd_keycolor_enable <layerid><flag>\
 \n       eg: osd_keycolor_enable 0 0 \n"},

 	{
 	 "osd_keycolor_val",
 	 NULL,
 	 OSD_SETKEYCOLOR,
 	 "osd_keycolor_val - to set the values of key color\
 \n    usage: osd_keycolor_val <layerid><r_min r_max g_min g_max b_min b_max> \
 \n       eg: osd_keycolor_val ... \n"},
 	{
 	 "osd_flip",
 	 NULL,
 	 OSD_FLIP,
 	 "osd_flip - to flip OSD buffer from source to destination buffer \
 \n    usage: osd_flip <layer id> [l t r b l2 t2 r2 b2], if this parameter \
 \n      is not indicated, all of data move to destination buffer. otherwise, \
 \n	 to indicate a source and destination rectangle \
 \n       eg: osd_flip 0 0 0 720 576 0 0 1200 800\n"},
};
