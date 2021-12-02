#include "linuxos.h"

#include "database.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "fe_mngr.h"
#include "mvrecfile.h"

#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
U16 RecConf_Str[CSAPP_REC_CONF_ITEM_MAX+1] = {
	CSAPP_STR_TIME_SHIFT,
	CSAPP_STR_TIME_SHIFT_REC,
	CSAPP_STR_TIME_TS_TYPE,
	CSAPP_STR_TIME_JUMP
};

U8	RecConf_Arrow_Kind[CSAPP_REC_CONF_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	RecConf_Enter_Kind[CSAPP_REC_CONF_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

char TS_TYPE[3][3] = {
	"TS",
	"PS"
};

static CSAPP_Applet_t		CSApp_RecConf_Applets;
static eMV_RecConf_Items	RecConf_Focus_Item = CSAPP_REC_TIMESHIFT;
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static BOOL					Time_Shift = FALSE;
static BOOL					Time_Shift_Rec = FALSE;
static U8					TS_Format = 0;
static U8					Jump_Time = 0;

int RecConf_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

void MV_Draw_RecConfSelectBar(HDC hdc, int y_gap, eScanItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(RecConf_Enter_Kind[esItem])
	{
		case MV_NUMERIC:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_NUMBER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_NUMBER].bmHeight),&MV_BMP[MVBMP_Y_NUMBER]);
			break;
		case MV_SELECT:
			FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 - ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_Y_ENTER].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_Y_ENTER].bmHeight),&MV_BMP[MVBMP_Y_ENTER]);
			break;
		default:
			break;
	}
	
	if ( RecConf_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_RecConfMenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	temp_str[30];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_RecConfSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(RecConf_Str[esItem]));

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	memset ( temp_str, 0, 30 );

	switch(esItem)
	{
		case CSAPP_REC_TIMESHIFT:
			if( Time_Shift == TRUE )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_ON), -1, &TmpRect, DT_CENTER);	
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OFF), -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_REC_SHIFT_REC:
			if( Time_Shift_Rec == TRUE )
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_ON), -1, &TmpRect, DT_CENTER);	
			else
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_OFF), -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_REC_TS_TYPE:
			CS_MW_DrawText(hdc, TS_TYPE[TS_Format], -1, &TmpRect, DT_CENTER);	
			break;
		case CSAPP_REC_JUMP:
			sprintf(temp_str, "%d", Jump_Time);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_RecConfMenuFull(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CSAPP_REC_CONF_ITEM_MAX ; i++ )
	{
		if( RecConf_Focus_Item == i )
		{
			MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_RecConfMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_RecConfig(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_RecConf_Applets = CSApp_Applet_Error;

	RecConf_Focus_Item = CSAPP_REC_TIMESHIFT; 
	
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
	
	
	CreateInfo.dwStyle	 	= WS_VISIBLE;
	CreateInfo.dwExStyle 	= WS_EX_NONE;
	CreateInfo.spCaption 	= "RecConf Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = RecConf_Msg_cb;
	CreateInfo.lx 			= BASE_X;
	CreateInfo.ty 			= BASE_Y;
	CreateInfo.rx 			= BASE_X+WIDTH;
	CreateInfo.by 			= BASE_Y+HEIGHT;
	CreateInfo.iBkColor 	= COLOR_transparent;
	CreateInfo.dwAddData 	= 0;
	CreateInfo.hHosting 	= HWND_DESKTOP;
	
	hwndMain = CreateMainWindow (&CreateInfo);

	if (hwndMain == HWND_INVALID)	
		return CSApp_Applet_Error;
	
	ShowWindow(hwndMain, SW_SHOWNORMAL);
	
	while (GetMessage(&msg, hwndMain)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	MainWindowThreadCleanup (hwndMain);
	
	//printf("Sat setting Cleanup\n");
	
	return CSApp_RecConf_Applets;
}

int RecConf_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	
	switch(message)
	{
		case MSG_CREATE:
			RecConf_Focus_Item = CSAPP_REC_TIMESHIFT;
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);

			Time_Shift = FALSE;
			Time_Shift_Rec = FALSE;
			TS_Format = 0;
			Jump_Time = 5;
			
			break;
			
		case MSG_CLOSE:
			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
			
		case MSG_TIMER:
			hdc = BeginPaint(hwnd);
			Show_Signal(hdc);
			EndPaint(hwnd,hdc);
			break;
			
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_MEDIA, EN_ITEM_FOCUS_RECORD_CONF);
			
			hdc = BeginPaint(hwnd);
			MV_Draw_RecConfMenuFull(hdc);			
			//MV_Media_draw_help_banner(hdc, EN_ITEM_FOCUS_RECORD_CONF);
			EndPaint(hwnd,hdc);
			return 0;
			
		case MSG_KEYDOWN:
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8						u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(RecConf_Focus_Item)
					{
						case CSAPP_REC_TIMESHIFT:
							hdc = BeginPaint(hwnd);

							if ( u8Result_Value == 0 )
								Time_Shift = TRUE;
							else
								Time_Shift = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_SHIFT_REC:
							hdc = BeginPaint(hwnd);
							
							if ( u8Result_Value == 0 )
								Time_Shift_Rec = TRUE;
							else
								Time_Shift_Rec = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_TS_TYPE:
							hdc = BeginPaint(hwnd);		
							
							TS_Format = u8Result_Value;							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_JUMP:
							hdc = BeginPaint(hwnd);		
							
							Jump_Time = u8Result_Value + 5;							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						default:
							break;
					}
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}

				break;
			}

			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_RecConf_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_DOWN: 
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_RecConfMenuBar(hdc, MV_UNFOCUS, RecConf_Focus_Item);

						if(RecConf_Focus_Item == CSAPP_REC_CONF_ITEM_MAX - 1)
							RecConf_Focus_Item = CSAPP_REC_TIMESHIFT;
						else
							RecConf_Focus_Item++;

						MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
						
				case CSAPP_KEY_UP:
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_RecConfMenuBar(hdc, MV_UNFOCUS, RecConf_Focus_Item);
						
						if(RecConf_Focus_Item == CSAPP_REC_TIMESHIFT)
							RecConf_Focus_Item = CSAPP_REC_CONF_ITEM_MAX - 1;
						else
							RecConf_Focus_Item--;

						MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_LEFT:
					switch(RecConf_Focus_Item)
					{
						case CSAPP_REC_TIMESHIFT:
							hdc = BeginPaint(hwnd);

							if ( Time_Shift == FALSE )
								Time_Shift = TRUE;
							else
								Time_Shift = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_SHIFT_REC:
							hdc = BeginPaint(hwnd);
							
							if ( Time_Shift_Rec == FALSE )
								Time_Shift_Rec = TRUE;
							else
								Time_Shift_Rec = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_TS_TYPE:
							hdc = BeginPaint(hwnd);		

							if ( TS_Format == 0 )
								TS_Format = 1;
							else
								TS_Format = 0;
								
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_JUMP:
							hdc = BeginPaint(hwnd);		

							if ( Jump_Time == 5 )
								Jump_Time = 10;
							else
								Jump_Time--;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						default:
							break;
					}
        			break;
					
				case CSAPP_KEY_RIGHT:
					switch(RecConf_Focus_Item)
					{
						case CSAPP_REC_TIMESHIFT:
							hdc = BeginPaint(hwnd);

							if ( Time_Shift == FALSE )
								Time_Shift = TRUE;
							else
								Time_Shift = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_SHIFT_REC:
							hdc = BeginPaint(hwnd);
							
							if ( Time_Shift_Rec == FALSE )
								Time_Shift_Rec = TRUE;
							else
								Time_Shift_Rec = FALSE;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_TS_TYPE:
							hdc = BeginPaint(hwnd);		

							if ( TS_Format == 0 )
								TS_Format = 1;
							else
								TS_Format = 0;
								
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						case CSAPP_REC_JUMP:
							hdc = BeginPaint(hwnd);		

							if ( Jump_Time == 10 )
								Jump_Time = 5;
							else
								Jump_Time++;
							
							MV_Draw_RecConfMenuBar(hdc, MV_FOCUS, RecConf_Focus_Item);
							
							EndPaint(hwnd,hdc);
							break;
						default:
							break;
					}
					break;

				case CSAPP_KEY_ENTER:    
					switch(RecConf_Focus_Item)
					{
						case CSAPP_REC_TIMESHIFT:
							{
								RECT					smwRect;
								stPopUp_Window_Contents stContents;

								memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ON));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( RecConf_Focus_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;

								if ( Time_Shift == TRUE )
									stContents.u8Focus_Position = 0 ;
								else
									stContents.u8Focus_Position = 1 ;
								
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
							}
							break;
						case CSAPP_REC_SHIFT_REC:
							{
								RECT					smwRect;
								stPopUp_Window_Contents stContents;

								memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

								sprintf(stContents.Contents[0], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_ON));
								sprintf(stContents.Contents[1], "%s", CS_MW_LoadStringByIdx(CSAPP_STR_OFF));
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( RecConf_Focus_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;
								
								if ( Time_Shift_Rec == TRUE )
									stContents.u8Focus_Position = 0 ;
								else
									stContents.u8Focus_Position = 1 ;
								
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
							}
							break;
						case CSAPP_REC_TS_TYPE:
							{
								RECT					smwRect;
								stPopUp_Window_Contents stContents;

								memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

								sprintf(stContents.Contents[0], "%s", TS_TYPE[0]);
								sprintf(stContents.Contents[1], "%s", TS_TYPE[1]);
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( RecConf_Focus_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 2;
								stContents.u8Focus_Position = TS_Format;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
							}
							break;
						case CSAPP_REC_JUMP:
							{
								int						i = 0;
								RECT					smwRect;
								stPopUp_Window_Contents stContents;

								memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
								for ( i = 5 ; i < 11 ; i++ )
									sprintf(stContents.Contents[i - 5], "%d", i);
								
								smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( RecConf_Focus_Item + 1 );
								smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
								smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
								smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
								stContents.u8TotalCount = 6;
								stContents.u8Focus_Position = Jump_Time - 5;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
							}
							break;
						default:
							break;
					}
					break;

				case CSAPP_KEY_ESC:					
					CSApp_RecConf_Applets=CSApp_Applet_Desktop;
					SendMessage (hwnd, MSG_CLOSE, 0, 0);
					break;
					
				case CSAPP_KEY_MENU:					
					CSApp_RecConf_Applets=b8Last_App_Status;
					SendMessage (hwnd, MSG_CLOSE, 0, 0);
					break;
				default:					
					break;
			}
			break;
			
		default:
			break;			
	}
	
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}
#endif

