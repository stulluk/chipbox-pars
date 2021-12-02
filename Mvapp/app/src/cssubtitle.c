#include "linuxos.h"

#include "database.h"
#include "sys_setup.h"
#include "mwsvc.h"
#include "mwsetting.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csaudio.h"

static CSAPP_Applet_t	CSApp_Subtitle_Applets;

static BOOL 			MainPaintFlag = TRUE;
static tMWStream		SubtitleStream;

/* By KB Kim 2011.04.11 */
static U8				Current_Subtitle_Lang = 0;
static U8				Total_Subtitle_Lang = 0;

static int Subtitle_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Draw_SubtitleMenuBar(HDC hdc)
{
	char	temp_str[20];
	int 	y_gap = SM_WINDOW_CONT_Y;
	int 	mid_width = SM_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int 	right_x = SM_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;
	RECT	TmpRect;

	SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_CONT_DX - ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth) - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);

	CS_MW_TextOut( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(CSAPP_STR_LANGUAGE));

	TmpRect.left	=ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X);
	TmpRect.right	=TmpRect.left + SM_WINDOW_ITEM_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(SM_WINDOW_ITEM_DY);

	if( Total_Subtitle_Lang == 0 )
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NONE), -1, &TmpRect, DT_CENTER);
	else
	{
		snprintf(temp_str, 20, "%s", CS_MW_LoadLanguageStringByIso(SubtitleStream.Stream[Current_Subtitle_Lang].Language)); 
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
	}
}


void MV_Subtitle_SetWindow(HDC hdc)
{
	RECT	rc1;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + ( SM_WINDOW_DY - 30) - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y + ( SM_WINDOW_DY - 30) - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y),ScalerWidthPixel(SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(( SM_WINDOW_DY - 30)));
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(SM_WINDOW_DX),ScalerHeigthPixel(( SM_WINDOW_DY - 30) - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = SM_WINDOW_TITLE_Y;
	rc1.left = SM_WINDOW_ITEM_X;
	rc1.bottom = SM_WINDOW_TITLE_Y + SM_WINDOW_ITEM_DY;
	rc1.right = rc1.left + SM_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_SUB);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_CONT_Y),ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY));

	MV_Draw_SubtitleMenuBar(hdc);
}

CSAPP_Applet_t	CSApp_Subtitle(void)
{
    int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Subtitle_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption 		= "cssub window";
	CreateInfo.hMenu	 		= 0;
	CreateInfo.hCursor	 		= 0;
	CreateInfo.hIcon	 		= 0;
	CreateInfo.MainWindowProc 	= Subtitle_Msg_cb;
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
	return CSApp_Subtitle_Applets;
    
}


static int Subtitle_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
    HDC 	hdc;

	switch(message)
	    {
	        case MSG_CREATE:
				CS_MW_GetSubtitleStream(&SubtitleStream);
				
				/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
				Current_Subtitle_Lang = MvGetCurrentSubtitle();
				Total_Subtitle_Lang = SubtitleStream.Number;	
				if (Current_Subtitle_Lang >= Total_Subtitle_Lang)
				{
					Current_Subtitle_Lang = 0;
				}
				printf("\nSubtitle_Msg_cb :: Total_Subtitle_Lang : %d\n", Total_Subtitle_Lang);
				break;
			case MSG_PAINT:
				{
					hdc=BeginPaint(hwnd);				
					if(MainPaintFlag)
					{         
						MV_Subtitle_SetWindow(hdc);
						MainPaintFlag = FALSE;
					}
					EndPaint(hwnd,hdc);
				}
				return 0;

			case MSG_TTX_DISPLAY:
				printf("++++++++++MSG_TTX_DISPLAY++++++++\n");
				CSOS_DelayTaskMs(100);
				break;

			case MSG_KEYDOWN:
				switch(wparam)
				{
					case CSAPP_KEY_SUBTITLE:
					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						CSApp_Subtitle_Applets = CSApp_Applet_Desktop;
						/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
						if(Total_Subtitle_Lang==0)
						{
							MvClearCurrentSubtitle();
						}
						else
						{
							Current_Subtitle_Lang = Total_Subtitle_Lang;
							MvSetCurrentSubtitle(Total_Subtitle_Lang);
						}

						MvSetSubtitleMode(0);
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
						
					case CSAPP_KEY_LEFT:
						{
							if(Total_Subtitle_Lang==0)
							{
								Current_Subtitle_Lang=0;
								/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
								MvClearCurrentSubtitle();
							}
							else
							{
								if(Current_Subtitle_Lang == 0)
									Current_Subtitle_Lang = Total_Subtitle_Lang-1;
								else
									Current_Subtitle_Lang--;
							}
							hdc=BeginPaint(hwnd);
							MV_Draw_SubtitleMenuBar(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
					case CSAPP_KEY_RIGHT:
						{
							if(Total_Subtitle_Lang==0)
							{
								Current_Subtitle_Lang=0;
								/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
								MvClearCurrentSubtitle();
							}
							else
							{
								if(Current_Subtitle_Lang == (Total_Subtitle_Lang-1))
									Current_Subtitle_Lang = 0;
								else
									Current_Subtitle_Lang++;
							}
							hdc=BeginPaint(hwnd);
							MV_Draw_SubtitleMenuBar(hdc);
							EndPaint(hwnd,hdc);
						}
						break;
					case CSAPP_KEY_ENTER:
						if(SubtitleStream.Number==0)
						{
							MvSetSubtitleMode(0);
							break;
						}
						
						/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
						// CS_MW_SetSubtitlePid(SubtitleStream.Stream[Current_Subtitle_Lang].Pid);
						// CS_MW_SetSubtitleLang(SubtitleStream.Stream[Current_Subtitle_Lang].Language,kCS_SI_TRIGRAM_MAX_LENGTH+1);
						MvSetCurrentSubtitle(Current_Subtitle_Lang);
						if (Current_Subtitle_Lang < Total_Subtitle_Lang)
						{
							MvSetSubtitleMode(1);
						}
						else
						{
							MvSetSubtitleMode(0);
						}
						CSApp_Subtitle_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
						break;
					default:
						break;
				}
				break;
				
			case MSG_CLOSE:
				CS_APP_SetFirstInDesktop(FALSE);
				MainPaintFlag = TRUE;
				PostQuitMessage(hwnd);
				DestroyMainWindow(hwnd);
				break;
				
			default:
				break;		
	    }
    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


