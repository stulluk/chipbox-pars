#include "cs_app_common.h"
#include "mv_bitmap_link.h"
#include "ui_common.h"
#include "mv_cfg.h"

char	CHECK_COLOR_STRING[MV_COLOR_CFG_MAX][30] = {
						"main_back",
						"menu_back",
						"info_top",
						"info_bottom"
					};

MV_CFG MV_CFG_Colorcheck(char *tempSection)
{
	int 	i;
	char	Temp[20];

	for ( i = 0 ; i < MV_COLOR_CFG_MAX ; i++ )
	{
		memset (Temp, 0, sizeof(char) * 20);
		strncpy(Temp, tempSection, strlen(CHECK_COLOR_STRING[i]));

		if ( strcmp ( CHECK_COLOR_STRING[i], Temp ) == 0 )
			break;
	}

	return i;
}

MV_CFG_RETURN MV_LoadColorFile(void)
{
	FILE* 			fp;
    char 			tempSection [CFG_MAX_COL + 2];
	char			Temp[100];
	MV_CFG_RETURN	ret = CFG_OK;

	sprintf(Temp, "%s%s", CFG_Resource, COLOR_FILE);
	if (!(fp = fopen(Temp, "r")))
	{
         ret = CFG_NOFILE;
		 return ret;
	}

	memset(Temp, 0x00, 100);

	while (!feof(fp)) {
		memset (Temp, 0, sizeof(char) * 100);

        if (!fgets(tempSection, CFG_MAX_COL, fp)) {
			fclose (fp);
			ret = CFG_READ_FAIL;
			return ret;
        }

		MV_Parser_CFG_Value(Temp, tempSection);

		switch( MV_CFG_Colorcheck(tempSection) )
		{
			case MV_COLOR_MAIN_BACK:
				Parser_Color_CFG(MV_COLOR_MAIN_BACK, Temp);
				break;

			case MV_COLOR_MENU_BACK_COLOR:
				Parser_Color_CFG(MV_COLOR_MENU_BACK_COLOR, Temp);
				break;

			case MV_COLOR_INFO_TOP_BACK:
				Parser_Color_CFG(MV_COLOR_INFO_TOP_BACK, Temp);
				break;

			case MV_COLOR_INFO_BOT_BACK:
				Parser_Color_CFG(MV_COLOR_INFO_BOT_BACK, Temp);
				break;

			default:
				ret = CFG_NO_NAME;
				break;
		}
    }

	fclose (fp);
	return ret;
}


void MV_Draw_Loading_Image_Progress(MV_Bitmap Bit_Map, char *char_str)
{
	HDC		hdc;

	hdc =BeginPaint(HWND_DESKTOP);

	if ( Bit_Map > 7 )
		Draw_Loading_Progress_Bar(hdc, Bit_Map*100/MVBMP_INFO_BANNER_CIRCLE, char_str);

	EndPaint(HWND_DESKTOP,hdc);
}

int MV_LoadBmp(BOOL b8Load_Kind)
{

    printf("Image load started..\n");
	fflush(stdout);
	int		mv_error=0;
	char 	temp[512];
	char 	temp2[512];
	U16		i;
	HDC		hdc;

	printf("[mvapp] LoadBmp: color file + paint box (resource=%s)\n", CFG_Resource);
	fflush(stdout);
	MV_LoadColorFile();
#ifdef GOLDBOX
	hdc =BeginPaint(HWND_DESKTOP);
	MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(196),ScalerHeigthPixel(420), ScalerWidthPixel(888),ScalerHeigthPixel(148) );
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(200),ScalerHeigthPixel(424), ScalerWidthPixel(880),ScalerHeigthPixel(140) );
	EndPaint(HWND_DESKTOP,hdc);
#else
	hdc =BeginPaint(HWND_DESKTOP);
	MV_SetBrushColor( hdc, MVAPP_SCROLL_GRAY_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(100),ScalerHeigthPixel(100), ScalerWidthPixel(310),ScalerHeigthPixel(228) );
	MV_SetBrushColor( hdc, MVAPP_BLACK_COLOR_ALPHA );
	MV_FillBox( hdc, ScalerWidthPixel(104),ScalerHeigthPixel(104), ScalerWidthPixel(302),ScalerHeigthPixel(220) );
	EndPaint(HWND_DESKTOP,hdc);
#endif
	printf("[mvapp] LoadBmp: paint box done\n");
	fflush(stdout);

	memset(&MV_BMP, 0x00, sizeof(BITMAP)*MVBMP_MAX);

	memset(&temp, 0x00, 512);

	sprintf(temp, "%s%s", CFG_Resource , MV_Bitmap_Link[0]);
	printf("[mvapp] LoadBmp: file[0]=%s\n", temp);
	fflush(stdout);

	if (MV_BitmapFromFile(HDC_SCREEN, &MV_BMP[0], temp) != 0)
	{
		mv_error++;
		printf("=============>>>> %s no Image file\n", temp);
	}
	else
		printf("[mvapp] LoadBmp: file[0] ok w=%d h=%d\n",
		       (int)MV_BMP[0].bmWidth, (int)MV_BMP[0].bmHeight);
	fflush(stdout);
	/*else
        printf("%s loaded succesfully!!\n", temp);*/

	printf("[mvapp] LoadBmp: FillBoxWithBitmap boot loading...\n");
	fflush(stdout);
	hdc =BeginPaint(HWND_DESKTOP);
	FillBoxWithBitmap(hdc, ScalerWidthPixel((1280 - MV_BMP[MVBMP_BOOT_LOADING].bmWidth)/2), ScalerHeigthPixel(424 - MV_BMP[MVBMP_BOOT_LOADING].bmHeight - 6), ScalerWidthPixel(MV_BMP[MVBMP_BOOT_LOADING].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOOT_LOADING].bmHeight), &MV_BMP[MVBMP_BOOT_LOADING]);
	EndPaint(HWND_DESKTOP,hdc);
	printf("[mvapp] LoadBmp: FillBoxWithBitmap done; loop to %d\n",
	       (int)(MVBMP_INFO_BANNER_CIRCLE + 1));
	fflush(stdout);

	for ( i = 1 ; i < MVBMP_INFO_BANNER_CIRCLE + 1 ; i++ )
	{
		memset(&temp, 0x00, 512);
		memset(&temp2, 0x00, 512);

		sprintf(temp, "%s%s", CFG_Resource , MV_Bitmap_Link[i]);
		//sprintf(temp2, "%s", MV_Bitmap_Link[i]);
		sprintf(temp2, "%s", Temp_String_Str[CS_DBU_GetMenuLanguage()][i]);

		if ((i % 10) == 1 || i < 5) {
			printf("[mvapp] LoadBmp: file[%u]=%s\n", (unsigned)i, temp);
			fflush(stdout);
		}

		if (MV_BitmapFromFile(HDC_SCREEN, &MV_BMP[i], temp) != 0)
		{
			mv_error++;
			printf("=============>>>> %s no Image file\n", temp);
			fflush(stdout);
		}

		//printf("=============>>>> %s Loading \n", temp);

		if ( b8Load_Kind == TRUE )
		{
			MV_Draw_Loading_Image_Progress(i, temp2);
		}
	}

	printf("[mvapp] LoadBmp: CS_MW_Font_Creation...\n");
	fflush(stdout);
	CS_MW_Font_Creation(0);
	printf("Image load finished..\n");
	fflush(stdout);
	return mv_error;
}

int MV_LoadBmp_NoProgress(HWND hwnd)
{
	int		mv_error=0;
	char 	temp[512];
	U16		i;
	U8		Percent = 0;
	HDC		hdc;
	RECT	TmpRect;

	MV_LoadColorFile();

	memset(&MV_BMP, 0x00, sizeof(BITMAP)*MVBMP_MAX);
	memset(&temp, 0x00, 512);

	TmpRect.top = WARNING_WINDOW_CONTENT_Y + 80;
	TmpRect.bottom = TmpRect.top + 14;
	TmpRect.left = WARNING_WINDOW_CONTENT_X + 30;
	TmpRect.right = TmpRect.left + WARNING_WINDOW_CONTENT_DX - 60;

	sprintf(temp, "%s%s", CFG_Resource , MV_Bitmap_Link[0]);

	if (MV_BitmapFromFile(HDC_SCREEN, &MV_BMP[0], temp) != 0)
	{
		mv_error++;
		printf("=============>>>> %s no Image file\n", temp);
	}

	for ( i = 1 ; i < MVBMP_INFO_BANNER_CIRCLE + 1 ; i++ )
	{
		memset(&temp, 0x00, 512);
		sprintf(temp, "%s%s", CFG_Resource , MV_Bitmap_Link[i]);

		if (MV_BitmapFromFile(HDC_SCREEN, &MV_BMP[i], temp) != 0)
		{
			mv_error++;
			printf("=============>>>> %s no Image file\n", temp);
		}

		Percent = (i*100)/MVBMP_INFO_BANNER_CIRCLE;

		hdc = BeginPaint(hwnd);
		MV_Draw_LevelBar(hdc, &TmpRect, Percent, EN_TTEM_PROGRESS_NO_IMG_BMP);
		EndPaint(hwnd,hdc);
	}
	return mv_error;
}

void MV_InitBmp(void)
{
	int 		i;

	for ( i = 1 ; i < MVBMP_MAX ; i++ )
		UnloadBitmap (&MV_BMP[i]);
}

