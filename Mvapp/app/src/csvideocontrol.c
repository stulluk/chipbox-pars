#include "linuxos.h"

#include "database.h"
#include "sys_setup.h"
#include "mwsvc.h"
#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csvideocontrol.h"

static CSAPP_Applet_t	CSApp_VideoSet_Applets;
static U16				Current_video_set = 0;
static unsigned int		u8Bright_level = 5;
static unsigned int		u8Contrast_level = 5;
static unsigned int		u8Color_level = 5;

static const U16 		video_item[CS_APP_VIDEO_SETTING_MAX] = {
							    CSAPP_STR_VIDEO_BRIGHT,
								CSAPP_STR_VIDEO_CONTRAST,
								CSAPP_STR_VIDEO_COLOR
							};

static int Video_Set_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_Video_SetMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 	y_gap = ( SM_VIDEO_WINDOW_CONT_Y + ( SM_VIDEO_WINDOW_ITEM_DY * esItem ));
	int 	mid_width = SM_VIDEO_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int 	right_x = SM_VIDEO_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;
	RECT	TmpRect;

	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
		FillBox(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X), ScalerHeigthPixel(y_gap),ScalerWidthPixel(SM_VIDEO_WINDOW_CONT_DX),ScalerHeigthPixel(SM_VIDEO_WINDOW_ITEM_DY));
	}

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X + SM_VIDEO_WINDOW_ITEM_NAME_X ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X + SM_VIDEO_WINDOW_CONT_DX - ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth) - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);

	CS_MW_TextOut( hdc, ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(video_item[esItem]));

	TmpRect.left	=ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X + SM_VIDEO_WINDOW_ITEM_NAME_X);
	TmpRect.right	=TmpRect.left + ( SM_VIDEO_WINDOW_CONT_DX - SM_VIDEO_WINDOW_ITEM_NAME_X ) - 10;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(SM_VIDEO_WINDOW_ITEM_DY);

	switch(esItem)
	{
		case CS_APP_VIDEO_BRIGHT:
			MV_Draw_LevelBar(hdc, &TmpRect, u8Bright_level, EN_ITEM_10_BAR_LEVEL_NONAME);
			break;

		case CS_APP_VIDEO_CONTRAST:
			MV_Draw_LevelBar(hdc, &TmpRect, u8Contrast_level, EN_ITEM_10_BAR_LEVEL_NONAME);
			break;

		case CS_APP_VIDEO_COLOR:
			MV_Draw_LevelBar(hdc, &TmpRect, u8Color_level, EN_ITEM_10_BAR_LEVEL_NONAME);
			break;

		default:
			break;
	}
}


void MV_Video_Set_SetWindow(HDC hdc)
{
	RECT	rc1;
	int		i;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_VIDEO_WINDOW_X), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_VIDEO_WINDOW_X + SM_VIDEO_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_VIDEO_WINDOW_X), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y + SM_VIDEO_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_VIDEO_WINDOW_X + SM_VIDEO_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y + SM_VIDEO_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y),ScalerWidthPixel(SM_VIDEO_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(SM_VIDEO_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_X), ScalerHeigthPixel(SM_VIDEO_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(SM_VIDEO_WINDOW_DX),ScalerHeigthPixel(SM_VIDEO_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = SM_VIDEO_WINDOW_TITLE_Y;
	rc1.left = SM_VIDEO_WINDOW_ITEM_X;
	rc1.bottom = SM_VIDEO_WINDOW_TITLE_Y + SM_VIDEO_WINDOW_ITEM_DY;
	rc1.right = rc1.left + SM_VIDEO_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_VIDEO_SETTING);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_VIDEO_WINDOW_ITEM_X), ScalerHeigthPixel(SM_VIDEO_WINDOW_CONT_Y),ScalerWidthPixel(SM_VIDEO_WINDOW_CONT_DX),ScalerHeigthPixel(SM_VIDEO_WINDOW_ITEM_DY));

	for ( i = 0 ; i < CS_APP_VIDEO_SETTING_MAX ; i++ )
	{
		if ( Current_video_set == i )
			MV_Draw_Video_SetMenuBar(hdc, FOCUS, i);
		else
			MV_Draw_Video_SetMenuBar(hdc, UNFOCUS, i);
	}
}

CSAPP_Applet_t	CSApp_Video_Set(void)
{
    int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_VideoSet_Applets = CSApp_Applet_Error;
	
#ifdef  Screen_1080
    BASE_X = 0;
    BASE_Y = 0;
    WIDTH  = 1920;
    HEIGHT = 1080;
#else
    BASE_X = 0;
    BASE_Y = 0;
    WIDTH  = ScalerWidthPixel(CSAPP_OSD_MAX_WIDTH);
    HEIGHT = ScalerHeigthPixel(CSAPP_OSD_MAX_HEIGHT);
#endif

	
	CreateInfo.dwStyle	 		= WS_VISIBLE;
	CreateInfo.dwExStyle 		= WS_EX_NONE;
	CreateInfo.spCaption 		= "videoset window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Video_Set_Msg_cb;
	CreateInfo.lx 				= BASE_X;
	CreateInfo.ty 				= BASE_Y;
	CreateInfo.rx 				= BASE_X+WIDTH;
	CreateInfo.by 				= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 		= COLOR_black;
	CreateInfo.dwAddData 		= 0;
	CreateInfo.hHosting 		= HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	return CSApp_VideoSet_Applets;
    
}


static int Video_Set_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
    HDC 	hdc;

	switch(message)
    {
        case MSG_CREATE:
			Current_video_set=0;

			CS_AV_VID_GetBrightness(&u8Bright_level);
			CS_AV_VID_GetContrast(&u8Contrast_level);
			CS_AV_VID_GetSaturation(&u8Color_level);

			break;
		case MSG_PAINT:
			hdc=BeginPaint(hwnd);				
			MV_Video_Set_SetWindow(hdc);
			EndPaint(hwnd,hdc);
			return 0;

		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_SD_HD:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
				case CSAPP_KEY_ENTER:
					CSApp_VideoSet_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_LEFT:
					{
						switch(Current_video_set)
						{
							case CS_APP_VIDEO_BRIGHT:
								if (u8Bright_level <= 0)
									u8Bright_level = 0;
								else
									u8Bright_level--;

								CS_AV_VID_SetBrightness(u8Bright_level);
								break;

							case CS_APP_VIDEO_CONTRAST:
								if (u8Contrast_level <= 0)
									u8Contrast_level = 0;
								else
									u8Contrast_level--;

								CS_AV_VID_SetContrast(u8Contrast_level);
								break;

							case CS_APP_VIDEO_COLOR:
								if (u8Color_level <= 0)
									u8Color_level = 0;
								else
									u8Color_level--;

								CS_AV_VID_SetSaturation(u8Color_level);
								break;

							default:
								if (u8Bright_level <= 0)
									u8Bright_level = 0;
								else
									u8Bright_level--;

								CS_AV_VID_SetBrightness(u8Bright_level);
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_Video_SetMenuBar(hdc, FOCUS, Current_video_set);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_RIGHT:
					{
						switch(Current_video_set)
						{
							case CS_APP_VIDEO_BRIGHT:
								if (u8Bright_level >= 10)
									u8Bright_level = 10;
								else
									u8Bright_level++;

								CS_AV_VID_SetBrightness(u8Bright_level);
								break;

							case CS_APP_VIDEO_CONTRAST:
								if (u8Contrast_level >= 10)
									u8Contrast_level = 10;
								else
									u8Contrast_level++;

								CS_AV_VID_SetContrast(u8Contrast_level);
								break;

							case CS_APP_VIDEO_COLOR:
								if (u8Color_level >= 10)
									u8Color_level = 10;
								else
									u8Color_level++;

								CS_AV_VID_SetSaturation(u8Color_level);
								break;

							default:
								if (u8Bright_level >= 10)
									u8Bright_level = 10;
								else
									u8Bright_level++;

								CS_AV_VID_SetBrightness(u8Bright_level);
								break;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_Video_SetMenuBar(hdc, FOCUS, Current_video_set);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_UP:
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Video_SetMenuBar(hdc, UNFOCUS, Current_video_set);
						
						if ( Current_video_set <= 0 )
							Current_video_set = CS_APP_VIDEO_SETTING_MAX - 1;
						else
							Current_video_set--;

						MV_Draw_Video_SetMenuBar(hdc, FOCUS, Current_video_set);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_DOWN:
					{
						hdc=BeginPaint(hwnd);
						MV_Draw_Video_SetMenuBar(hdc, UNFOCUS, Current_video_set);
						
						if ( Current_video_set >= CS_APP_VIDEO_SETTING_MAX - 1 )
							Current_video_set = 0;
						else
							Current_video_set++;

						MV_Draw_Video_SetMenuBar(hdc, FOCUS, Current_video_set);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_VideoSet_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				default:
					break;
			}
			break;

		case MSG_CLOSE:
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
			
		default:
			break;		
    }
    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


