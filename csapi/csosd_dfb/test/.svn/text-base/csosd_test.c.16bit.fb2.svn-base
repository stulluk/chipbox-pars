#include <stdio.h>
#include <stdlib.h>

#include<sys/types.h>
#include<unistd.h>
#include <string.h>

//#include "csapi.h"
#include "csosd.h"
#include "../../cstvout/include/cstvout.h"
#include "directfb.h"



int
main(void)
{
	CSOSD_HANDLE 		g_osd_handle = 0;	

	int ret=0, i;
	PIC_ID	id0, id1, id2, id3, id4, id5, id6, id7;
	FILE *rgb_file = NULL;
	CSOSD_REC_Region	region;
	CSOSD_TRI_Region	triregion;
	CSOSD_LINE_Region	lineregion;
	CSOSD_Color	color;
	CSTVOUT_HANDLE		tvout_handle;
	//CSTVOUT_HANDLE		tvout2_handle;
	unsigned char read_buf[300*200*2+72];

	if(NULL == (tvout_handle = CSTVOUT_Open(0)))
	{
		printf("open tvout device Failed\n");
		return 0;
	}

	/*if(NULL == (tvout2_handle = CSTVOUT_Open(1)))
	{
		printf("open tvout2 device Failed\n");
		return 0;
	}

	CSTVOUT_SetOutput(tvout_handle, OUTPUT_MODE_RGB);
	CSTVOUT_BindVID(tvout_handle, TVOUT_VID_DEV_1);
	CSTVOUT_BindGFX(tvout_handle, TVOUT_OSD_LAYER_0);

	CSTVOUT_SetOutput(tvout2_handle, OUTPUT_MODE_YPBPR);
	CSTVOUT_BindVID(tvout2_handle, TVOUT_VID_DEV_0);
	CSTVOUT_BindGFX(tvout2_handle, TVOUT_OSD_LAYER_1);*/
	
	CSTVOUT_SetMode(tvout_handle, TVOUT_MODE_576I);
	
	g_osd_handle = CSOSD_Open(OSD_LAYER_0);
	if(g_osd_handle == NULL)
	{
		printf("CSOSD_Open error.....\n");
	}																											
	printf("\n");
	ret = CSOSD_Enable(g_osd_handle);
	ret |= CSOSD_SetAlpha(g_osd_handle, 0xff);
	if(ret == CSAPI_FAILED)
		printf( "error \n");
	ret = CSOSD_OpenDFB(OSD_LAYER_0);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	

	printf("ready to CSOSD_DrawLine\n");getchar();
	lineregion.x1 = 350;
	lineregion.y1 = 150;
	lineregion.x2 = 650;
	lineregion.y2 = 150;
	color.b = 0x7f;
	color.g = 0x9f;
	color.r = 0x8f;
	color.a = 0xff;
	ret = CSOSD_DrawLine(g_osd_handle, &lineregion, &color, &id5);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	
	printf("ready to CSOSD_DrawLine\n");getchar();
	lineregion.x1 = 320;
	lineregion.y1 = 50;
	lineregion.x2 = 320;
	lineregion.y2 = 200;
	color.b = 0x7f;
	color.g = 0x9f;
	color.r = 0x8f;
	color.a = 0xff;
	ret = CSOSD_DrawLine(g_osd_handle, &lineregion, &color, &id5);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	
	printf("ready to CSOSD_DrawLine\n");getchar();
	lineregion.x1 = 300;
	lineregion.y1 = 80;
	lineregion.x2 = 250;
	lineregion.y2 = 250;
	color.b = 0x7f;
	color.g = 0x9f;
	color.r = 0xcf;
	color.a = 0xff;
	ret = CSOSD_DrawLine(g_osd_handle, &lineregion, &color, &id5);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	


	/*printf("ready to CSOSD_DrawHorizonDottedLine\n");getchar();
	lineregion.x1 = 300;
	lineregion.y1 = 250;
	lineregion.x2 = 600;
	lineregion.y2 = 250;
	color.b = 0x7f;
	color.g = 0x9f;
	color.r = 0xcf;
	color.a = 0xff;
	ret = CSOSD_DrawHorizonDottedLine(g_osd_handle, &lineregion, 20, 20, &color, &id7);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	*/


	printf("ready to CSOSD_DrawRectangle\n");getchar();
	region.x = 300;
	region.y = 200;
	region.width = 200;
	region.height = 100;
	color.b = 0x1f;
	color.g = 0x8f;
	color.r = 0x5f;
	color.a = 0xff;
	ret = CSOSD_DrawRectangle(g_osd_handle, &region, &color, &id6);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	

	
	printf("ready to CSOSD_FillTriangle\n");getchar();
	triregion.x1 = 80;
	triregion.y1 = 50;
	triregion.x2 = 80;
	triregion.y2 = 150;
	triregion.x3 = 280;
	triregion.y3 = 100;
	color.b = 0x8f;
	color.g = 0x9f;
	color.r = 0xaf;
	color.a = 0xff;
	ret = CSOSD_FillTriangle(g_osd_handle, &triregion, &color, &id4);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));		
	

	printf("ready to CSOSD_FillRectangle\n");getchar();
	region.x = 100;
	region.y = 376-100;
	region.width = 300;
	region.height = 60;
	color.b = 0x3f;
	color.g = 0x4f;
	color.r = 0x5f;
	color.a = 0xff;
	ret = CSOSD_FillRectangle(g_osd_handle, &region, &color, &id2);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));						
	

	printf("ready to CSOSD_OpenImageBuffer\n");getchar();
	rgb_file = fopen("./300200565.bmp","rb");
	if (rgb_file == 0){
		printf( "----------Error open input device.\r\n" );
		return( -1 );
	}
	fseek(rgb_file, 0, SEEK_SET);
	fread(read_buf, sizeof(unsigned char), 300*200*2+72, rgb_file);
	fclose(rgb_file);
	region.x = 350;
	region.y = 200;
	region.width = 300;
	region.height = 200;
	ret = CSOSD_OpenImageBuffer(g_osd_handle, &region, OSD_COLOR_FORMAT_RGB565, read_buf+72, &id0);
	if(ret == CSAPI_FAILED)
		printf( "error \n");
	else
		printf("id0 = %d\n",id0);

	printf("ready to CSOSD_ShowImage\n");getchar();
	ret = CSOSD_ShowImage(g_osd_handle, id0);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));


	printf("ready to CSTVOUT_SetMode 480I\n");getchar();
	ret = CSTVOUT_SetMode(tvout_handle, TVOUT_MODE_480I);
	if(ret == CSAPI_FAILED)
		printf( "CSTVOUT_SetMode error \n");										printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));


	printf("ready to CSOSD_OpenImage\n");getchar();
	region.x = 100;
	region.y = 50;
	region.width = 400;
	region.height = 150;
	ret = CSOSD_OpenImage(g_osd_handle, &region, "./bg_h.png", &id7);
	if(ret == CSAPI_FAILED)
		printf( "error \n");
	else
		printf("id1 = %d\n",id7);

	printf("ready to CSOSD_ShowImageRoll\n");getchar();
	ret = CSOSD_ShowImageRoll(g_osd_handle, id7, OSD_DOWN, 40, (EXTRA_FUNC)CSOSD_Flip);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			//printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));


	printf("ready to CSOSD_OpenImage\n");getchar();
	region.x = 100;
	region.y = 570;
	region.width = 400;
	region.height = 200;
	ret = CSOSD_OpenImage(g_osd_handle, &region, "./smokey_light.jpg", &id1);
	if(ret == CSAPI_FAILED)
		printf( "error \n");
	else
		printf("id1 = %d\n",id1);
	printf("ready to CSOSD_ShowImage\n");getchar();
	ret = CSOSD_ShowImage(g_osd_handle, id1);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	printf("ready to CSOSD_ObjectMove\n");getchar();
	for( i = 0; i < 4; i++ ){
		ret = CSOSD_ObjectMove(g_osd_handle, 0, -40, OSD_DISTANCE, id1);
		if(ret != CSAPI_SUCCEED)
			printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	}
	CSOSD_ObjectMove(g_osd_handle, 0, -20, OSD_DISTANCE, id1);CSOSD_Flip(g_osd_handle);
	CSOSD_ObjectMove(g_osd_handle, 0, -20, OSD_DISTANCE, id1);CSOSD_Flip(g_osd_handle);
		

	printf("ready to CSOSD_FillRectangle\n");getchar();
	region.x = 100;
	region.y = 376-30;
	region.width = 300;
	region.height = 60;
	color.b = 0x3f;
	color.g = 0x4f;
	color.r = 0x5f;
	color.a = 0x9f;
	ret = CSOSD_FillRectangle(g_osd_handle, &region, &color, &id3);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));	


	printf("ready to CSOSD_ObjectFadeInOut\n");getchar();
	ret = CSOSD_ObjectFadeInOut(g_osd_handle, id0, (EXTRA_FUNC)CSOSD_Flip);
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");	


	printf("ready to CSOSD_SetObjectAlpha\n");getchar();
	ret = CSOSD_SetObjectAlpha(g_osd_handle, id1, 0xd0);			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));getchar();
	ret |= CSOSD_SetObjectAlpha(g_osd_handle, id1, 0x00);			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));getchar();
	ret |= CSOSD_SetObjectAlpha(g_osd_handle, id1, 0xff);			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");


	printf("ready to CSOSD_ObjectMove\n");getchar();
	ret = CSOSD_ObjectMove(g_osd_handle, -40, 0, OSD_DISTANCE, id1);
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));

	
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;
	printf("ready to CSOSD_ObjectKeyColor\n");getchar();
	ret = CSOSD_ObjectKeyColor(g_osd_handle, id1, &color);
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));		
	

	printf("ready to CSOSD_ObjectFadeInOut_Stop\n");getchar();
	ret = CSOSD_ObjectFadeInOut_Stop(g_osd_handle, id0);
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");	


	printf("ready to CSOSD_ObjectMove\n");getchar();
	ret = CSOSD_ObjectMove(g_osd_handle, 0, 40, OSD_DISTANCE, id3);
	if(ret != CSAPI_SUCCEED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));


	printf("ready to CSOSD_Object Clear\n");getchar();
	ret = CSOSD_ObjectClear(g_osd_handle, id0);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	ret = CSOSD_ObjectClear(g_osd_handle, id1);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));
	ret = CSOSD_ObjectClearAll(g_osd_handle);
	if(ret == CSAPI_FAILED)
		printf( "error \n");																			printf("CSOSD_Flip: %d\n",CSOSD_Flip(g_osd_handle));


	printf("ready to CSOSD_Close\n");getchar();
	ret = CSOSD_Close(g_osd_handle);
	if(ret == CSAPI_FAILED)
		printf( "error \n");


	return 0;
}
