#include <stdio.h>
#include <stdlib.h>
 
#include "cstvout.h"

static void DISPOUT_SETMODE(int argc, char *argv[])
{
	int mod = 0;
	int tveid = 0;
	CSTVOUT_HANDLE tvout_handler;

	if (argc < 3) {
		printf("invalid parameters, see help\n");
		return;
	}

	tveid = atoi(argv[1]);
	mod = atoi(argv[2]);
#if 0
 	if (mod < 0 || mod > 7) {
 		printf("invalid parameters, see help \n");
 		return;
 	}
#endif

	tvout_handler = CSTVOUT_Open(tveid);
	CSTVOUT_SetMode(tvout_handler, mod);
	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_EN(int argc, char *argv[])
{
	CSTVOUT_HANDLE tvout_handler;
	int tveid, enable;

	if (argc < 3) {
		printf("invalid parameters, see help\n");
		return;
	}
	tveid = atoi(argv[1]);
	enable = atoi(argv[2]);
	tvout_handler = CSTVOUT_Open(tveid);
	if (enable)	CSTVOUT_Enable(tvout_handler);
	else { CSTVOUT_Disable(tvout_handler); printf("%%"); }
	CSTVOUT_Close(tvout_handler);
}

#if 0
static void DISPOUT_GETMODE(int argc, char *argv[])
{
	int mod = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);
	CSTVOUT_GetMode(tvout_handler, &mod);
	printf(" current TV mode = %d \n", mod);
	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_SETBRIGHTNESS(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);

	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}

	val = atoi(argv[1]);
	if (val < 1 || val > 254) {
		printf("invalid parameters, see help \n");
		return;
	}

	CSTVOUT_SetBrightness(tvout_handler, val);

	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_SETCONTRAST(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);

	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}

	val = atoi(argv[1]);
	if (val < 1 || val > 254) {
		printf("invalid parameters, see help \n");
		return;
	}

	CSTVOUT_SetContrast(tvout_handler, val);

	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_SETSATURATION(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);

	if (argc < 2) {
		printf("invalid parameters, see help\n");
		return;
	}

	val = atoi(argv[1]);
	if (val < 1 || val > 254) {
		printf("invalid parameters, see help \n");
		return;
	}

	CSTVOUT_SetSaturation(tvout_handler, val);

	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_GETBRIGHTNESS(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);
	CSTVOUT_GetBrightness(tvout_handler, &val);
	printf(" current brightness value = %d \n", val);

	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_GETCONTRAST(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);
	CSTVOUT_GetContrast(tvout_handler, &val);
	printf(" current ontrast value = %d \n", val);

	CSTVOUT_Close(tvout_handler);
}

static void DISPOUT_GETSATURATION(int argc, char *argv[])
{
	int val = -1;
	CSTVOUT_HANDLE tvout_handler;

	tvout_handler = CSTVOUT_Open(0);
	CSTVOUT_GetSaturation(tvout_handler, &val);
	printf(" current value = %d \n", val);

	CSTVOUT_Close(tvout_handler);
}
#endif

static struct cmd_t cstvout_tt[] = {
    {
	"tvout_setmode",
	NULL,
	DISPOUT_SETMODE,
	"tvout_setmode  - set dispout mode, the following options are valid\
	    \n    usage: tvout_setmode <tveid> <mode> \
	    \n \
	    \n      0 - 576I \
	    \n      1 - 480I, \
	    \n      2 - 576P, \
	    \n      3 - 480P, \
	    \n      4 - 720P50, \
	    \n      5 - 720P60, \
	    \n      6 - 1080I25, \
	    \n      7 - 1080I30 \
	    \n \
	    \n       eg: tvout_setmode 0 7 \n"},

	{
	    "tvout_enable",
	    NULL, 
	    DISPOUT_EN,
	    " tvout_enable - to enable / disable TVE. \
	\n    usage:  tvout_enable <tveid> <0/1>\
	\n       eg: tvout_enable 0 1 \n"},
#if 0
	{
	    "tvout_setbrightness",
	    NULL, 
	    DISPOUT_SETBRIGHTNESS,
	    ""
	},
	{
	    "tvout_getbrightness",
	    NULL, 
	    DISPOUT_GETBRIGHTNESS,
	    ""
	},
	{
	    "tvout_setcontrast",
	    NULL, 
	    DISPOUT_SETCONTRAST,
	    ""
	},
	{
	    "tvout_getcontrast",
	    NULL, 
	    DISPOUT_GETCONTRAST,
	    ""
	},
#endif
};
