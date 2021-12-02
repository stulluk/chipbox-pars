#include "linuxos.h"

#include "database.h"
#include "sys_setup.h"
#include "csttx.h"
#include "csttxdraw.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "csaudio.h"

static CSAPP_Applet_t		CSApp_Teletext_Applets;

static BOOL					OpenTTX=FALSE;
static UpdateEventPara*		QueueData;

static BOOL 				MainPaintFlag = TRUE;
static tMWStream			TeletextStream;
static U16					Current_Teletext_Lang = 0;
static U16					Total_Teletext_Lang = 0;


static int Teletext_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
static BOOL CSAPP_TeletextEH(WPARAM wParam);

void MV_Draw_TTXMenuBar(HDC hdc)
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

	if( Total_Teletext_Lang == 0 )
		CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NONE), -1, &TmpRect, DT_CENTER);
	else
	{
		snprintf(temp_str, 20, "%s", CS_MW_LoadLanguageStringByIso(TeletextStream.Stream[Current_Teletext_Lang].Language)); 
		CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
	}
}


void MV_TTX_SetWindow(HDC hdc)
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
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_TTX);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_CONT_Y),ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY));

	MV_Draw_TTXMenuBar(hdc);
}

static void CSApp_TTXDisplayNotify(U32 pBitmap,U32 lpara)
{
	 BroadcastMessage (MSG_TTX_DISPLAY, pBitmap, lpara);
}

CSAPP_Applet_t	CSApp_Teletext(void)
{
    int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
  	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Teletext_Applets = CSApp_Applet_Error;

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

	
	CreateInfo.dwStyle	 = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE;
	CreateInfo.spCaption = "csteletext window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Teletext_Msg_cb;
	CreateInfo.lx = BASE_X;
	CreateInfo.ty = BASE_Y;
	CreateInfo.rx = BASE_X+WIDTH;
	CreateInfo.by = BASE_Y+HEIGHT;
	CreateInfo.iBkColor = COLOR_black;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	return CSApp_Applet_Error;

	ShowWindow(hwndMain, SW_SHOWNORMAL);

	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	return CSApp_Teletext_Applets;
    
}


static int Teletext_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
    HDC 					hdc;

	switch(message)
    {
        case MSG_CREATE:
			CS_MW_GetTeletextStream(&TeletextStream);
			Current_Teletext_Lang=0;
			Total_Teletext_Lang=TeletextStream.Number;
			// printf("\nTeletext_Msg_cb :: Total_Teletext_Lang : %d\n", Total_Teletext_Lang);
			break;
		case MSG_PAINT:
			{				
				hdc=BeginPaint(hwnd);				
				if(MainPaintFlag)
				{         
					MV_TTX_SetWindow(hdc);
					MainPaintFlag = FALSE;
				}
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_TTX_DISPLAY:
			{
				TTXDisplayBitmap		*TtxDisplayBmp;
				
				hdc=BeginPaint(hwnd);	
				TtxDisplayBmp=(TTXDisplayBitmap*)wparam;
				FillBoxWithBitmap(hdc,TtxDisplayBmp->x,TtxDisplayBmp->y,TtxDisplayBmp->TtxBitmap.bmWidth,TtxDisplayBmp->TtxBitmap.bmHeight,&TtxDisplayBmp->TtxBitmap);
				EndPaint(hwnd,hdc);
				TtxDisplayBmp->Update=FALSE;
			}
			break;

		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_Teletext_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
			}
                                 
			if(CSAPP_TeletextEH(wparam)==FALSE)
			{
				break;
			}
			
			switch(wparam)
			{
				//case CSAPP_KEY_TEXT:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Teletext_Applets = CSApp_Applet_Normal;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				case CSAPP_KEY_LEFT:
					{
						if(Total_Teletext_Lang==0)
						{
							Current_Teletext_Lang=0;
						}
						else
						{
							if(Current_Teletext_Lang == 0)
								Current_Teletext_Lang = Total_Teletext_Lang-1;
							else
								Current_Teletext_Lang--;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_TTXMenuBar(hdc);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_ENTER:
					if(TeletextStream.Number==0)
						break;
					SetKeyInputControl(FALSE);
					// printf("=============>>  Enable TTX with Language (%s) - %d\n", TeletextStream.Stream[Current_Teletext_Lang].Language, OsTimeNowMilli());
					CreateTeletext(TeletextStream.Stream[Current_Teletext_Lang].Pid, 0x100, CS_TTX_DEFAULT,TeletextStream.Stream[Current_Teletext_Lang].Language, CSApp_TTXDisplayNotify);
					OpenTTX=TRUE;
					break;
					
				case CSAPP_KEY_RIGHT:
					{
						if(Total_Teletext_Lang==0)
						{
							Current_Teletext_Lang=0;
						}
						else
						{
							if(Current_Teletext_Lang == (Total_Teletext_Lang-1))
								Current_Teletext_Lang = 0;
							else
								Current_Teletext_Lang++;
						}
						hdc=BeginPaint(hwnd);
						MV_Draw_TTXMenuBar(hdc);
						EndPaint(hwnd,hdc);
					}
					break;
				default:
					break;
			}
			
			break;
		case MSG_CLOSE:
			SetKeyInputControl(TRUE);
    		CS_APP_SetFirstInDesktop(FALSE);
			MainPaintFlag = TRUE;
			if(OpenTTX==TRUE)
			{
				OpenTTX=FALSE;
				DestroyTeletext();					
			}
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
		default:
			break;		
    }
    return DefaultMainWinProc(hwnd,message,wparam,lparam);
}


static BOOL CSAPP_TeletextEH(WPARAM wParam)
{
	CSOS_MessageQueue_t*		pMsg;

	if(OpenTTX==FALSE)
		return TRUE;

	pMsg=GetTTXpMsgQ();

	QueueData=CSOS_AllocateMemory(NULL, sizeof(UpdateEventPara));

	// printf("=============== TTX Key : [0x%02X]\n", wParam);

	switch(wParam)
	{
		case CSAPP_KEY_0 :
			QueueData->uMsg=CS_TTX_KEY_MSG_0;
			break;
		case CSAPP_KEY_1 :
			QueueData->uMsg=CS_TTX_KEY_MSG_1;
			break;
		case CSAPP_KEY_2 :
			QueueData->uMsg=CS_TTX_KEY_MSG_2;
			break;
		case CSAPP_KEY_3 :
			QueueData->uMsg=CS_TTX_KEY_MSG_3;
			break;
		case CSAPP_KEY_4 :
			QueueData->uMsg=CS_TTX_KEY_MSG_4;
			break;
		case CSAPP_KEY_5 :
			QueueData->uMsg=CS_TTX_KEY_MSG_5;
			break;
		case CSAPP_KEY_6 :
			QueueData->uMsg=CS_TTX_KEY_MSG_6;
			break;
		case CSAPP_KEY_7 :
			QueueData->uMsg=CS_TTX_KEY_MSG_7;
			break;
		case CSAPP_KEY_8 :
			QueueData->uMsg=CS_TTX_KEY_MSG_8;
			break;
		case CSAPP_KEY_9 :
			QueueData->uMsg=CS_TTX_KEY_MSG_9;
			break;
		case CSAPP_KEY_ENTER :
			QueueData->uMsg=CS_TTX_KEY_MSG_OK;
			break;
		case CSAPP_KEY_UP :
			QueueData->uMsg=CS_TTX_KEY_MSG_PAGE_UP;
			break;
		case CSAPP_KEY_DOWN :
			QueueData->uMsg=CS_TTX_KEY_MSG_PAGE_DOWN;
			break;
		case CSAPP_KEY_PG_UP :
			QueueData->uMsg=CS_TTX_KEY_MSG_PAGE10_UP;
			break;
		case CSAPP_KEY_PG_DOWN :
			QueueData->uMsg=CS_TTX_KEY_MSG_PAGE10_DOWN;
			break;
		case CSAPP_KEY_LEFT :
			QueueData->uMsg=CS_TTX_KEY_MSG_SUBPAGE_DOWN;
			break;
		case CSAPP_KEY_RIGHT :
			QueueData->uMsg=CS_TTX_KEY_MSG_SUBPAGE_UP;
			break;
		case CSAPP_KEY_RED :
			QueueData->uMsg=CS_TTX_KEY_MSG_RED;
			break;
		case CSAPP_KEY_BLUE :
			QueueData->uMsg=CS_TTX_KEY_MSG_BLUE;
			break;
		case CSAPP_KEY_GREEN :
			QueueData->uMsg=CS_TTX_KEY_MSG_GREEN;
			break;
		case CSAPP_KEY_YELLOW :
			QueueData->uMsg=CS_TTX_KEY_MSG_GREEN;
			break;
		case CSAPP_KEY_MENU :
		case CSAPP_KEY_ESC :
			CSOS_DeallocateMemory(NULL, QueueData);
			return TRUE;
		default  :
			CSOS_DeallocateMemory(NULL, QueueData);
			return FALSE;
	}
	
	CSOS_SendMessage(pMsg, QueueData, sizeof(UpdateEventPara),0);
	CSOS_DeallocateMemory(NULL, QueueData);

	return FALSE;
}



