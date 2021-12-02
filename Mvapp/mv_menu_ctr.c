#include "mv_menu_ctr.h"
#include "csmpr_usb.h"

//static U32		ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static U8		u8SubFocus_Item = 0;

BOOL			Checking_Copy = 0;

extern			U16	MAIN_MENU_STR[MAIN_ITEM_MAX][MAIN_SUB_ITEM_MAX];

U8 MV_Get_Submenu_Focus(void)
{
	return u8SubFocus_Item;
}

void MV_Set_Submenu_Focus(U8 Idx, U8 u8Focus)
{
	u8SubFocus_Item = u8Focus;
	stFocus[Idx].u8Sub_Focus = u8SubFocus_Item;
}

void MV_Loading_Main_Image( void )
{
	mv_main_bmp_focus[0][0] = MV_BMP[MVBMP_SD_INST1];
	mv_main_bmp_focus[0][1] = MV_BMP[MVBMP_SD_INST2];
	mv_main_bmp_focus[0][2] = MV_BMP[MVBMP_SD_INST3];
	mv_main_bmp_focus[1][0] = MV_BMP[MVBMP_SD_SYSTEM1];
	mv_main_bmp_focus[1][1] = MV_BMP[MVBMP_SD_SYSTEM2];
	mv_main_bmp_focus[1][2] = MV_BMP[MVBMP_SD_SYSTEM3];
	mv_main_bmp_focus[2][0] = MV_BMP[MVBMP_SD_MEDIA1];
	mv_main_bmp_focus[2][1] = MV_BMP[MVBMP_SD_MEDIA2];
	mv_main_bmp_focus[2][2] = MV_BMP[MVBMP_SD_MEDIA3];
	mv_main_bmp_focus[3][0] = MV_BMP[MVBMP_SD_TOOL1];
	mv_main_bmp_focus[3][1] = MV_BMP[MVBMP_SD_TOOL2];
	mv_main_bmp_focus[3][2] = MV_BMP[MVBMP_SD_TOOL3];
}

void MV_Draw_MainBack(HDC hdc)
{
	int 		i;
	int			wGap = mv_main_bmp_focus[0][0].bmWidth + BMP_MENU_GAP;
	int			width_dx = BMP_MENU_X;

	if ( MV_BMP[MVBMP_MAIN_BACK_CAPTURE].bmHeight == 0 )
	{
		MV_FillBoxWithBitmap (hdc, 0, ScalerHeigthPixel(BMP_MENU_Y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &MV_BMP[MVBMP_MAIN_BACK]);
		
		for ( i = 0; i < MAIN_ITEM_MAX ; i++ )
		{
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[i][0].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[i][0].bmHeight), &mv_main_bmp_focus[i][0]);
			width_dx += wGap;
		}
		MV_GetBitmapFromDC (hdc, 0, ScalerHeigthPixel(BMP_MENU_Y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &MV_BMP[MVBMP_MAIN_BACK_CAPTURE]);
	} else {
		MV_FillBoxWithBitmap (hdc, 0, ScalerHeigthPixel(BMP_MENU_Y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &MV_BMP[MVBMP_MAIN_BACK_CAPTURE]);
	}

	if ( CS_DBU_GetANI_Type() )
	{
	SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
	FillBox(hdc,ScalerWidthPixel(0), ScalerHeigthPixel(BMP_MENU_Y),ScalerWidthPixel(MAX_DX),ScalerHeigthPixel(BMP_MENU_DY));
	}
}

void MV_Mainmenu_Draw( HWND hwnd, U8 u8MainFocus_Item )
{
	HDC 		hdc;
	int 		x = 0;
	U16			Start_y = 0;

	if ( CS_DBU_GetANI_Type() )
	{
	hdc = MV_BeginPaint(hwnd); 
	MV_Draw_MainBack(hdc);
	EndPaint (hwnd, hdc);

    for ( x = 0 ; x < 5 ; x++ ) 
	{
		Start_y = BMP_MENU_Y + ((BMP_MENU_DY/5) * (4 - x));
		hdc = BeginPaint(hwnd);
		SetBrushColor(hdc, MVAPP_TRANSPARENTS_COLOR);
		FillBox(hdc,ScalerWidthPixel(0), ScalerHeigthPixel(BMP_MENU_Y),ScalerWidthPixel(MAX_DX),ScalerHeigthPixel(BMP_MENU_DY));
		MV_FillBoxWithBitmap (hdc, 0, ScalerHeigthPixel(Start_y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &MV_BMP[MVBMP_MAIN_BACK_CAPTURE]);
		EndPaint(hwnd,hdc);
		usleep(10);
    }

	MV_Mainmenu_Focus( hwnd, u8MainFocus_Item );
	
	hdc = MV_BeginPaint(hwnd); 
	MV_Submenu_Focus( hdc, u8MainFocus_Item );
	EndPaint (hwnd, hdc);
	}
	else
	{
	hdc = MV_BeginPaint(hwnd); 

	MV_Draw_MainBack(hdc);
	MV_Submenu_Focus( hdc, u8MainFocus_Item );

	EndPaint (hwnd, hdc);

	MV_Mainmenu_Focus( hwnd, u8MainFocus_Item );
	}
}

void MV_Mainmenu_Focus( HWND hwnd , U8 u8FocusIndex)
{
	HDC		hdc;
	int		width_dx = BMP_MENU_X;
	int		i = 0;

	for ( i = 0 ; i < u8FocusIndex ; i++ )
		width_dx += mv_main_bmp_focus[0][0].bmWidth + BMP_MENU_GAP;

	if ( MV_BMP[MVBMP_MAIN_BACK_CAPTURE].bmHeight != 0 )
	{
		hdc = MV_BeginPaint(hwnd);
		MV_FillBoxWithBitmap (hdc, 0, ScalerHeigthPixel(BMP_MENU_Y), ScalerWidthPixel(MAX_DX), ScalerHeigthPixel(BMP_MENU_DY), &MV_BMP[MVBMP_MAIN_BACK_CAPTURE]);
		EndPaint (hwnd, hdc);
	}
	
	MainMenu_Ani_Init(u8FocusIndex);
#if 0
	for ( j = 0 ; j < 2 /*MAIN_ANI_COUNT*/ ; j++ )
	{
		for ( i = 0 ; i < MAIN_ANI_MAX ; i++ )
		{		
			hdc = MV_BeginPaint(hwnd);
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[u8FocusIndex][i].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[u8FocusIndex][i].bmHeight), &mv_main_bmp_focus[u8FocusIndex][i]);
			MV_EndPaint (hwnd, hdc);
			usleep( 100 * 1000 );
		}

		for ( i = MAIN_ANI_MAX - 1 ; i <= 0 ; i-- )
		{		
			hdc = MV_BeginPaint(hwnd);	
			MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[u8FocusIndex][i].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[u8FocusIndex][i].bmHeight), &mv_main_bmp_focus[u8FocusIndex][i]);
			MV_EndPaint (hwnd, hdc);
			usleep( 100 * 1000 );
		}
	}
#endif
}


void MV_Mainmenu_UnFocus( HDC hdc, U8 u8UnFocusIndex )
{
//	int	width_dx = BMP_MENU_X;
//	int	i = 0;

	MainMenu_Ani_Stop();
#if 0	
	for ( i = 0 ; i < u8UnFocusIndex ; i++ )
		width_dx += mv_main_bmp_focus[0][0].bmWidth + BMP_MENU_GAP;
#endif
	MV_Submenu_UnFocus( hdc, u8UnFocusIndex);
#if 0
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(width_dx), ScalerHeigthPixel(BMP_MENU_ICON_Y), ScalerWidthPixel(mv_main_bmp_focus[u8UnFocusIndex][0].bmWidth), ScalerHeigthPixel(mv_main_bmp_focus[u8UnFocusIndex][0].bmHeight), &mv_main_bmp_focus[u8UnFocusIndex][0]);
#endif
}

void MV_Submenu_UnFocus( HDC hdc, U8 u8FocusIndex)
{
	if ( stFocus[u8FocusIndex].u8Focus_MAX > 0 )
	{
		MV_SetBrushColor( hdc, FILL_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(SUB_MENU_X - 10), ScalerHeigthPixel(BAR_MENU_Y - ( stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 ) - MV_BMP[MVBMP_TOP_MENU_BACK_TOP].bmHeight), ScalerWidthPixel(SUB_MENU_BAR_DX + 20 ), ScalerHeigthPixel((stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 ) + MV_BMP[MVBMP_TOP_MENU_BACK_TOP].bmHeight) );
	}
}

void MV_SubmenuBar_Focus( HDC hdc, U8 u8FocusIndex, U8 u8SubIndex, U8 u8Kind )
{
	RECT		Temp_Rect;

	Temp_Rect.top = ((BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 )) + ( BAR_MENU_GAP) * u8SubIndex + 10 + 4);
	Temp_Rect.bottom = Temp_Rect.top + MV_BMP[MVBMP_MAIN_SELECT_BAR].bmHeight;
	Temp_Rect.left = ( SUB_MENU_X + 10 );
	Temp_Rect.right = Temp_Rect.left + SUB_MENU_BAR_DX - 20;
		
	if ( u8Kind == FOCUS )
	{	
		SetTextColor(hdc,CSAPP_WHITE_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(SUB_MENU_X), ScalerHeigthPixel((BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 )) + ( BAR_MENU_GAP) * u8SubIndex + 10), ScalerWidthPixel(SUB_MENU_BAR_DX), ScalerHeigthPixel(MV_BMP[MVBMP_MAIN_SELECT_BAR].bmHeight), &MV_BMP[MVBMP_MAIN_SELECT_BAR]);
		//CS_MW_TextOut( hdc,ScalerWidthPixel(SUB_MENU_X + 10), ScalerHeigthPixel((BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 )) + ( BAR_MENU_GAP) * u8SubIndex + 10 + 4), CS_MW_LoadStringByIdx(MAIN_MENU_STR[u8FocusIndex][u8SubIndex]));
		CS_MW_DrawText( hdc, CS_MW_LoadStringByIdx(MAIN_MENU_STR[u8FocusIndex][u8SubIndex]), -1, &Temp_Rect, DT_CENTER );
	}
	else
	{
		if ( ( u8FocusIndex == CSAPP_MAINMENU_MEDIA && UsbCon_GetStatus() != USB_STATUS_MOUNTED ) || ( u8FocusIndex == CSAPP_MAINMENU_INSTALL && MV_GetSelected_SatData_Count() == 0 ) )
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		} else {
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
		}
		MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(SUB_MENU_X), ScalerHeigthPixel((BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 )) + ( BAR_MENU_GAP) * u8SubIndex + 10), ScalerWidthPixel(SUB_MENU_BAR_DX), ScalerHeigthPixel(MV_BMP[MVBMP_MAIN_UNSELECT_BAR].bmHeight), &MV_BMP[MVBMP_MAIN_UNSELECT_BAR]);
		//CS_MW_TextOut( hdc,ScalerWidthPixel(SUB_MENU_X + 10), ScalerHeigthPixel((BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 )) + ( BAR_MENU_GAP) * u8SubIndex + 10 + 4), CS_MW_LoadStringByIdx(MAIN_MENU_STR[u8FocusIndex][u8SubIndex]));
		CS_MW_DrawText( hdc, CS_MW_LoadStringByIdx(MAIN_MENU_STR[u8FocusIndex][u8SubIndex]), -1, &Temp_Rect, DT_CENTER );
	}
}

void MV_Draw_Submenu_Box(HDC hdc, RECT *boxRect)
{		
/* 126 MVBMP_SUBMENU_TOP_LEFT,		mainmenu - Submenu box left-top icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->left), ScalerHeigthPixel(boxRect->top), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LEFT]);

/* 127 MVBMP_SUBMENU_TOP_RIGHT,		mainmenu - Submenu box right-top icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->right - MV_BMP[MVBMP_SUBMENU_TOP_RIGHT].bmWidth), ScalerHeigthPixel(boxRect->top), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_RIGHT]);

/* 128 MVBMP_SUBMENU_BOT_LEFT,		mainmenu - Submenu box left-bottom icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->left), ScalerHeigthPixel(boxRect->bottom - MV_BMP[MVBMP_SUBMENU_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_LEFT]);

/* 129 MVBMP_SUBMENU_BOT_RIGHT,		mainmenu - Submenu box right-bottom icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->right - MV_BMP[MVBMP_SUBMENU_BOT_RIGHT].bmWidth), ScalerHeigthPixel(boxRect->bottom - MV_BMP[MVBMP_SUBMENU_BOT_RIGHT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOT_RIGHT]);

/* 130 MVBMP_SUBMENU_LEFT_LINE,		mainmenu - Submenu box left-line icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->left), ScalerHeigthPixel(boxRect->top + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_LEFT_LINE].bmWidth), ScalerHeigthPixel((boxRect->bottom - boxRect->top) - (MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight*2)), &MV_BMP[MVBMP_SUBMENU_LEFT_LINE]);

/* 131 MVBMP_SUBMENU_RIGHT_LINE,	mainmenu - Submenu box right-line icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->right - MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth), ScalerHeigthPixel(boxRect->top + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_SUBMENU_RIGHT_LINE].bmWidth), ScalerHeigthPixel((boxRect->bottom - boxRect->top) - (MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmHeight*2)), &MV_BMP[MVBMP_SUBMENU_RIGHT_LINE]);

/* 132 MVBMP_SUBMENU_TOP_LINE,		mainmenu - Submenu box top-line icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->left + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth), ScalerHeigthPixel(boxRect->top), ScalerWidthPixel((boxRect->right - boxRect->left) - (MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth*2)), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_TOP_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_TOP_LINE]);

/* 133 MVBMP_SUBMENU_BOTTOM_LINE,	mainmenu - Submenu box bottom-line icon */
	MV_FillBoxWithBitmap (hdc, ScalerWidthPixel(boxRect->left + MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth), ScalerHeigthPixel(boxRect->bottom - MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE].bmHeight), ScalerWidthPixel((boxRect->right - boxRect->left) - (MV_BMP[MVBMP_SUBMENU_TOP_LEFT].bmWidth*2)), ScalerHeigthPixel(MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE].bmHeight), &MV_BMP[MVBMP_SUBMENU_BOTTOM_LINE]);

	MV_FillBox( hdc, ScalerWidthPixel(boxRect->left + 7), ScalerHeigthPixel(boxRect->top + 7), ScalerWidthPixel(boxRect->right - boxRect->left - 14), ScalerHeigthPixel(boxRect->bottom - boxRect->top - 14));
}

void MV_Submenu_Focus( HDC hdc, U8 u8FocusIndex/*, U8 u8Key*/ )
{
	int		i = 0;
	U8		u8OldFocusIndex;
	RECT	boxRect;
	
	u8OldFocusIndex = u8FocusIndex;

	if ( stFocus[u8FocusIndex].u8Focus_MAX > 0 )
	{
		if ( CFG_Back_Color.MV_R == 0 && CFG_Back_Color.MV_G == 0 && CFG_Back_Color.MV_B == 0 )
			MV_SetBrushColor( hdc, MVAPP_GRAY_COLOR);
		else
			MV_SetBrushColor( hdc, RGBA2Pixel(hdc,CFG_Back_Color.MV_R, CFG_Back_Color.MV_G, CFG_Back_Color.MV_B, 0xFF) );

		boxRect.top = (BAR_MENU_Y - (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 ) - MV_BMP[MVBMP_TOP_MENU_BACK_TOP].bmHeight);
		boxRect.left = SUB_MENU_X - 10;
		boxRect.bottom = boxRect.top + (stFocus[u8FocusIndex].u8Focus_MAX * BAR_MENU_GAP + 20 + MV_BMP[MVBMP_TOP_MENU_BACK_TOP].bmHeight);
		boxRect.right = boxRect.left + (SUB_MENU_BAR_DX + 20);

		MV_Draw_Submenu_Box(hdc, &boxRect);

		if ( u8FocusIndex == CSAPP_MAINMENU_INSTALL && MV_GetSelected_SatData_Count() > 0 )
			MV_Set_Submenu_Focus(u8FocusIndex, 0);

		for ( i = 0 ; i < stFocus[u8FocusIndex].u8Focus_MAX ; i++ )
		{
			if ( i == MV_Get_Submenu_Focus() )
			{
				MV_SubmenuBar_Focus( hdc, u8FocusIndex, i, FOCUS );
			}
			else
			{
				MV_SubmenuBar_Focus( hdc, u8FocusIndex, i, UNFOCUS );
			}
		}
//		usleep( 30 * 1000 );
	}
}

void MV_Submenu_Clear(void)
{
	int 		i;
	
	for ( i = 0 ; i < 4 ; i++ )
	{
		//printf("%d st UnloadBitmap Start =============\n", i);
		UnloadBitmap(&stFocus[i].SubBmp);
		memset(&stFocus[i].SubBmp, 0x00, sizeof(BITMAP));
		//printf("%d st Success UnloadBitmap , %d =============\n", i, stFocus[i].SubBmp.bmHeight);
	}
}

