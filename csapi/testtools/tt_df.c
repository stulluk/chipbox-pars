#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "csdf.h"

int DF_ENABLE(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_Enable(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_DISABLE(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_Disable(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_SETBG(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    CSDF_RGB rgb;

    if (argc < 4) { printf(" invalid parameters, see help! \n"); return -1; }
    rgb.r = strtol(argv[1], NULL, 10);
    rgb.g = strtol(argv[2], NULL, 10);
    rgb.b = strtol(argv[3], NULL, 10);

    handle = CSDF_Open();
    CSDF_SetBackground(handle, &rgb);
    CSDF_Close(handle);

    return 0;
}

int DF_SETZORDER(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    CSDF_Z_ORDER order;
    int i;
    
    if (argc < 2) { printf(" invalid parameters, see help! \n"); return -1; }
    i = atoi(argv[1]);
    if(i == 0)
	order = Z_ORDER_V0_V1_G;
    else if(i == 1)
	order = Z_ORDER_V0_G_V1;
    else if(i == 2)
	order = Z_ORDER_V1_V0_G;
    else if(i == 3)
	order = Z_ORDER_V1_G_V0;
    else if(i == 4)
	order = Z_ORDER_G_V0_V1;
    else if(i == 5)
	order = Z_ORDER_G_V1_V0;
    else{
	printf(" invalid parameters, see help! \n"); 
	return -1; 
    }

    handle = CSDF_Open();
    CSDF_SetZorder(handle, order);
    CSDF_Close(handle);
    return 0;
}

int DF_GETZORDER(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    CSDF_Z_ORDER order;

    handle = CSDF_Open();

    CSDF_GetZorder(handle, &order);

    if(order == Z_ORDER_V0_V1_G)
	printf("Z_ORDER_V0_V1_G\n");
    else if(order == Z_ORDER_V0_G_V1)
	printf("Z_ORDER_V0_G_V1\n");
    else if(order == Z_ORDER_V1_V0_G)
	printf("Z_ORDER_V1_V0_G\n");
    else if(order == Z_ORDER_V1_G_V0)
	printf("Z_ORDER_V1_G_V0\n");
    else if(order == Z_ORDER_G_V0_V1)
	printf("Z_ORDER_G_V0_V1\n");
    else if(order == Z_ORDER_G_V1_V0)
	printf("Z_ORDER_G_V1_V0\n");
    else {
	printf(" bad Z-Order got!\n"); 
	return -1; 
    }

    CSDF_Close(handle);

    return 0;
}

int DF_ENABLEVID(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_VID_EnableOutput(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_DISABLEVID(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_VID_DisableOutput(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_SETALPHA(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    if (argc < 2) { printf(" invalid parameters, see help! \n"); return 1; }

    handle = CSDF_Open();
    CSDF_VID_SetOutputAlpha(handle, atoi(argv[1]));
    CSDF_Close(handle);

    return 0;
}

int DF_GETALPHA(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    int alpha;

    handle = CSDF_Open();
    CSDF_VID_GetOutputAlpha(handle, &alpha);
    printf("current alpha val: %d\n",alpha);
    CSDF_Close(handle);

    return 0;
}

int DF_SETPOSITION(int argc, char *argv[])
{
    CSDF_Rect src, dst;
    CSDF_HANDLE handle;

    if (argc < 9) { printf(" invalid parameters, see help! \n"); return 1; }

    src.left = atoi(argv[1]);
    src.right = atoi(argv[2]);
    src.top = atoi(argv[3]);
    src.bottom = atoi(argv[4]);
    dst.left = atoi(argv[5]);
    dst.right = atoi(argv[6]);
    dst.top = atoi(argv[7]);
    dst.bottom = atoi(argv[8]);

    handle = CSDF_Open();

    CSDF_VID_SetOutputPosition(handle, &src, &dst);
    CSDF_Close(handle);

    return 0;
}

int DF_GETPOSITION(int argc, char *argv[])
{
    CSDF_Rect src, dst;
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_VID_GetOutputPosition(handle, &src, &dst);
    printf("%d  %d  %d  %d  %d  %d  %d  %d\n", \
	    src.left, src.right, src.top, src.bottom, dst.left, dst.right, dst.top, dst.bottom);

    CSDF_Close(handle);

    return 0;
}

int DF_BLANKVIDEO(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_BlankVideo(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_UNBLANKVIDEO(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_UnBlankVideo(handle);
    CSDF_Close(handle);

    return 0;
}

int DF_ENABLEGFX(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_GFX_EnableOutput(handle, 0);
    CSDF_Close(handle);

    return 0;
}

int DF_DISABLEGFX(int argc, char *argv[])
{
    CSDF_HANDLE handle;

    handle = CSDF_Open();
    CSDF_GFX_DisableOutput(handle, 0);
    CSDF_Close(handle);

    return 0;
}

int DF_SETMODE(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    CSDF_OUTPUTMODE mode;
    int i;
    
    if (argc < 2) { printf(" invalid parameters, see help! \n"); return -1; }
    i = atoi(argv[1]);
    if(i == 0)
	mode = DF_MODE_PAL;
    else if(i == 1)
	mode = DF_MODE_NTSC;
    else if(i == 2)
	mode = DF_MODE_576P;
    else if(i == 3)
	mode = DF_MODE_480P;
    else if(i == 4)
	mode = DF_MODE_720P50;
    else if(i == 5)
	mode = DF_MODE_720P60;
    else if(i == 6)
	mode = DF_MODE_1080I25;
    else if(i == 7)
	mode = DF_MODE_1080I30;
    else{
	printf(" invalid parameters, see help! \n"); 
	return -1; 
    }

    handle = CSDF_Open();
    CSDF_SetOutputMode(handle, mode);
    CSDF_Close(handle);
    return 0;
}

int DF_GETMODE(int argc, char *argv[])
{
    CSDF_HANDLE handle;
    CSDF_OUTPUTMODE mode;

    handle = CSDF_Open();

    CSDF_GetOutputMode(handle, &mode);

    if(mode == DF_MODE_PAL)
	printf("DF_MODE_PAL\n");
    else if(mode == DF_MODE_NTSC)
	printf("DF_MODE_NTSC\n");
    else if(mode == DF_MODE_576P)
	printf("DF_MODE_576P\n");
    else if(mode == DF_MODE_480P)
	printf("DF_MODE_480P\n");
    else if(mode == DF_MODE_720P50)
	printf("DF_MODE_720P50\n");
    else if(mode == DF_MODE_720P60)
	printf("DF_MODE_720P60\n");
    else if(mode == DF_MODE_1080I25)
	printf("DF_MODE_1080I25\n");
    else if(mode == DF_MODE_1080I30)
	printf("DF_MODE_1080I30\n");
    else {
	printf(" bad output mode got!\n"); 
	return -1; 
    }

    CSDF_Close(handle);

    return 0;
}

static struct cmd_t csdf_tt[] = {
 	{
 	 "df_enable",
 	 NULL,
 	 DF_ENABLE,
 	 "df_enable - enable display feeder \
 	\n    usage: df_enable \
 	\n       eg: df_enable \n"},
 
 	{
 	 "df_disable",
 	 NULL,
 	 DF_DISABLE,
 	 "df_disable - disable display feeder \
 	\n    usage: df_disable \
 	\n       eg: df_disable \n"},
 
 	{
 	 "df_setbg",
 	 NULL,
 	 DF_SETBG,
 	 "df_setbg - set background color \
 	\n    usage: df_setbg <red val><green val><blue val> \
 	\n       eg: df_setbg 255 0 0 \n"},
 
 	{
 	 "df_setzorder",
 	 NULL,
 	 DF_SETZORDER,
 	 "df_setzorder -set df components Z-order \
 	\n    usage: df_setzorder <zorder val> \
 	\n\n             0 - Z_ORDER_V0_V1_G (top layer is graphics, middle layer is video1, bottom layer is video0)\
 	\n             1 - Z_ORDER_V0_G_V1 \
 	\n             2 - Z_ORDER_V1_V0_G \
 	\n             3 - Z_ORDER_V1_G_V0 \
 	\n             4 - Z_ORDER_G_V0_V1 \
 	\n             5 - Z_ORDER_G_V1_V0 \
 	\n\n     eg: df_setzorder 3 \n"},
 
 	{
 	 "df_getzorder",
 	 NULL,
 	 DF_GETZORDER,
 	 "df_getzorder -get df components Z-order \
 	\n    usage: df_getzorder \
 	\n     eg: df_getzorder \n"},
 
 	{
 	 "df_enablevid",
 	 NULL,
 	 DF_ENABLEVID,
 	 "df_enablevid - enable video layer output \
 	\n    usage: df_enablevid \
 	\n       eg: df_enablevid \n"},
 
 	{
 	 "df_disablevid",
 	 NULL,
 	 DF_DISABLEVID,
 	 "df_disablevid - disable video layer output \
 	\n    usage: df_disablevid \
 	\n       eg: df_disablevid \n"},
 
 	{
 	 "df_setalpha",
 	 NULL,
 	 DF_SETALPHA,
 	 "df_setalpha -set video layer alpha value \
 	\n    usage: df_setalpha <alpha val> \
 	\n     valid alpha val:0 ~ 255                              \
 	\n     eg: df_setalpha 80 \n"},
 
 	{
 	 "df_getalpha",
 	 NULL,
 	 DF_GETALPHA,
 	 "df_getalpha -get video layer alpha value \
 	\n    usage: df_getalpha \
 	\n     valid alpha val:0 ~ 255                              \
 	\n     eg: df_getalpha \n"},
 
 	{
 	 "df_setoutputpos",
 	 NULL,
 	 DF_SETPOSITION,
 	 "df_setoutputpos - set video layer output position \
 	\n    usage: df_setoutputpos <src_left><src_right><src_top><src_bottom><dst_left><dst_right><dst_top><dst_bottom> \
 	\n     eg: df_setoutputpos 0 720 0 576 0 1920 0 1080 \n"},
 
 	{
 	 "df_getoutputpos",
 	 NULL,
 	 DF_GETPOSITION,
 	 "df_getoutputpos - get video layer output position \
 	\n    usage: df_getoutputpos \
 	\n     the return value is in this format: \
 	\n 	 <src_left><src_right><src_top><src_bottom><dst_left><dst_right><dst_top><dst_bottom> \
 	\n     eg: df_getoutputpos \n"},
 
 	{
 	 "df_blankvideo",
 	 NULL,
 	 DF_BLANKVIDEO,
 	 "df_blankvideo - set display feeder output to background color \
 	\n    usage: df_blankvideo \
 	\n     eg: df_blankvideo \n"},
 
 	{
 	 "df_unblankvideo",
 	 NULL,
 	 DF_UNBLANKVIDEO,
 	 "df_unblankvideo - reverts to normal video and gfx output \
 	\n    usage: df_unblankvideo \
 	\n     eg: df_unblankvideo \n"},
 
 	{
 	 "df_enablegfx",
 	 NULL,
 	 DF_ENABLEGFX,
 	 "df_enablegfx - enable graphic layer output \
 	\n    usage: df_enablegfx \
 	\n     eg: df_enablegfx \n"},
 
 	{
 	 "df_disablegfx",
 	 NULL,
 	 DF_DISABLEGFX,
 	 "df_disablegfx - disable graphic layer output \
 	\n    usage: df_disablegfx \
 	\n     eg: df_disablegfx \n"},
 
 	{
 	 "df_setmode",
 	 NULL,
 	 DF_SETMODE,
 	 "df_setmode -set df output timing mode \
 	\n    usage: df_setmode <mode val> \
 	\n\n             0 - DF_MODE_PAL \
 	\n             1 - DF_MODE_NTSC \
 	\n             2 - DF_MODE_576P \
 	\n             3 - DF_MODE_480P \
 	\n             4 - DF_MODE_720P50 \
 	\n             5 - DF_MODE_720P60 \
 	\n             6 - DF_MODE_1080I25 \
 	\n             7 - DF_MODE_1080I30 \
 	\n\n     eg: df_setmode 2 \n"},
 
 	{
 	 "df_getmode",
 	 NULL,
 	 DF_GETMODE,
 	 "df_getmode -get df output timing mode \
 	\n    usage: df_getmode \
 	\n\n     eg: df_setmode \n"},
};
