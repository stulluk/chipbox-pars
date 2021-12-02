
#include "linuxos.h"

#include "database.h"
#include "ch_install.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csinstall.h"
#include "ui_common.h"
#include "fe_mngr.h"

#define MAX_POSITION 64

U16 UniCable_Str[UNICABLE_ITEM_MAX+1] = {
	CSAPP_STR_UNICABLE_LNB,
	CSAPP_STR_UNICABLE_POS,
	CSAPP_STR_UNICABLE_CHAN,
	CSAPP_STR_UNICABLE_FREQ
};

U8	UniCable_Arrow_Kind[UNICABLE_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

U8	UniCable_Enter_Kind[UNICABLE_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

static CSAPP_Applet_t		CSApp_UniCable_Applets;
static eUniCableItemID		UniCable_Focus_Item = UNICABLE_ITEM_SEL;
static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static U8					u8TpCount;
static U8					UniCable_Position = 0;
static U8					UniCable_Channel = 0;
static U8					UniCable_Frequency = 0;
static BOOL					UniCable_Select = FALSE;
static BOOL					Sat_List_Status;

int UniCable_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

void MV_Draw_UniCableSelectBar(HDC hdc, int y_gap, eScanItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(UniCable_Enter_Kind[esItem])
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
	
	if ( UniCable_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_UniCableMenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	temp_str[30];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_UniCableSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
	}

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(UniCable_Str[esItem]));

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	memset ( temp_str, 0, 30 );

	switch(esItem)
	{
		
		case UNICABLE_ITEM_SEL:
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case UNICABLE_ITEM_POS:
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case UNICABLE_ITEM_CHAN:
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case UNICABLE_ITEM_FREQ:
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_UniCableMenuFull(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < UNICABLE_ITEM_MAX ; i++ )
	{
		if( UniCable_Focus_Item == i )
		{
			MV_Draw_UniCableMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_UniCableMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_UniCable(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_UniCable_Applets = CSApp_Applet_Error;

	UniCable_Focus_Item = UNICABLE_ITEM_SEL; 
	
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
	CreateInfo.spCaption 	= "UniCable Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = UniCable_Msg_cb;
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
	
	return CSApp_UniCable_Applets;
}

int UniCable_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	
	switch(message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);

			UniCable_Focus_Item = UNICABLE_ITEM_SEL;
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);

			if ( MV_GetSelected_SatData_Count() > 0 )
				MV_GetSatelliteData(MV_Sat_Data);
			else
			{
				CSApp_UniCable_Applets=CSApp_Applet_Install;
				SendMessage (hwnd, MSG_CLOSE, 0, 0);
			}
			
			u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus) - 1;

			UniCable_Position = 0;
			UniCable_Channel = 0;
			UniCable_Frequency = 0;
			UniCable_Select = FALSE;
			
			MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
			break;
		case MSG_CLOSE:
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
		case MSG_TIMER:
			hdc = BeginPaint(hwnd);
			Show_Signal(hdc);
			EndPaint(hwnd,hdc);
			break;
		case MSG_PAINT:

			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, UNICABLE_SETTING);

			hdc = BeginPaint(hwnd);
			
			MV_Draw_UniCableMenuFull(hdc);
			
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y ), CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH));
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y + MV_INSTALL_SIGNAL_YGAP ), CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY));
			Show_Signal(hdc);

			//MV_Install_draw_help_banner(hdc, SATELLITE_SETTING);
			EndPaint(hwnd,hdc);

			return 0;
		case MSG_KEYDOWN:
/*
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8						u8Result_Value;
					//MV_stSatInfo			Temp_SatData;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Usals_Focus_Item)
					{
						case USALS_ITEM_SATELLITE:
							hdc = BeginPaint(hwnd);
							//u8Glob_Sat_Focus = u8Result_Value;
							u8Glob_Sat_Focus = MV_GetSelected_SatIndex_By_Count(u8Result_Value );

							{
								U8 Temp_Count = 0;
								Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

								if ( Temp_Count > 0 )
									u8TpCount = Temp_Count - 1;
								else
									u8TpCount = 0;
							}
							
							u8Glob_TP_Focus = 0;
								
							MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_LONGITUDE);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case USALS_ITEM_TP:
							hdc = BeginPaint(hwnd);
							u8Glob_TP_Focus = u8Result_Value;							
							MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case USALS_ITEM_LIMIT_SET:
							hdc = BeginPaint(hwnd);
							
							if ( u8Result_Value == 0 )
								Temp_Limit_Use = TRUE;
							else
								Temp_Limit_Use = FALSE;
							
							MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						default:
							break;
					}

					MV_SetSatelliteData(MV_Sat_Data);
					SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); // By KB Kim : 2010_08_31 for Scart Control
				}

				break;
			}
*/
			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_UniCable_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_DOWN: 
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_UniCableMenuBar(hdc, MV_UNFOCUS, UniCable_Focus_Item);

						if(UniCable_Focus_Item == UNICABLE_ITEM_MAX - 1)
							UniCable_Focus_Item = UNICABLE_ITEM_SEL;
						else
							UniCable_Focus_Item++;

						MV_Draw_UniCableMenuBar(hdc, MV_FOCUS, UniCable_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
						
				case CSAPP_KEY_UP:
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_UniCableMenuBar(hdc, MV_UNFOCUS, UniCable_Focus_Item);
						
						if(UniCable_Focus_Item == UNICABLE_ITEM_SEL)
							UniCable_Focus_Item = UNICABLE_ITEM_MAX - 1;
						else
							UniCable_Focus_Item--;

						MV_Draw_UniCableMenuBar(hdc, MV_FOCUS, UniCable_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{

					} else {
						switch(UniCable_Focus_Item)
						{
/*
							case USALS_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);
								
								if ( MV_GetSelected_SatData_Count() > 0 )
								{
									if( u8Glob_Sat_Focus == 0 )
										u8Glob_Sat_Focus = MV_SAT_MAX -1;
									else
										u8Glob_Sat_Focus--;

									while( MV_Sat_Data[u8Glob_Sat_Focus].u16Select != SAT_SELECT )
									{
										if( u8Glob_Sat_Focus == 0 )
											u8Glob_Sat_Focus = MV_SAT_MAX -1;
										else
											u8Glob_Sat_Focus--;
									}
								}
								else
									u8Glob_Sat_Focus = 0;
								
								{
									U8 Temp_Count = 0;
									Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

									if ( Temp_Count > 0 )
										u8TpCount = Temp_Count - 1;
									else
										u8TpCount = 0;
								}
								
								u8Glob_TP_Focus = 0;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_LONGITUDE);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case USALS_ITEM_TP:
								hdc = BeginPaint(hwnd);

								if ( u8Glob_TP_Focus == 0 )
									u8Glob_TP_Focus = u8TpCount;
								else
									u8Glob_TP_Focus--;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case USALS_ITEM_LONGITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status > 0 )
									Input_Status--;

								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case USALS_ITEM_LATITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status > 0 )
									Input_Status--;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case USALS_ITEM_LIMIT_SET:
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
*/
							default:
								break;

						}
					}
        			break;			
				case CSAPP_KEY_RIGHT:
					if ( Sat_List_Status == TRUE )
					{

					} else {
						switch(UniCable_Focus_Item)
						{
/*
							case USALS_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);

								if ( MV_GetSelected_SatData_Count() > 0 )
								{
									if( u8Glob_Sat_Focus == MV_SAT_MAX -1 )
										u8Glob_Sat_Focus = 0;
									else
										u8Glob_Sat_Focus++;

									while( MV_Sat_Data[u8Glob_Sat_Focus].u16Select != SAT_SELECT )
									{
										if( u8Glob_Sat_Focus == MV_SAT_MAX -1 )
											u8Glob_Sat_Focus = 0;
										else
											u8Glob_Sat_Focus++;
									}
								}
								else
									u8Glob_Sat_Focus = 0;

								{
									U8 Temp_Count = 0;
									Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

									if ( Temp_Count > 0 )
										u8TpCount = Temp_Count - 1;
									else
										u8TpCount = 0;
								}
								
								u8Glob_TP_Focus = 0;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);								
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_LONGITUDE);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case USALS_ITEM_TP:
								hdc = BeginPaint(hwnd);

								if ( u8Glob_TP_Focus == u8TpCount )
									u8Glob_TP_Focus = 0;
								else
									u8Glob_TP_Focus++;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
								
							case USALS_ITEM_LONGITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status < 4 )
									Input_Status++;

								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
								
							case USALS_ITEM_LATITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status < 4 )
									Input_Status++;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case USALS_ITEM_LIMIT_SET:
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
*/
							default:
								break;

						}
					}

					break;

				case CSAPP_KEY_ENTER:     
					{
						if ( Sat_List_Status == TRUE )
						{

						}
						else
						{
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);

							switch ( UniCable_Focus_Item )
							{
/*
								case USALS_ITEM_SATELLITE:
									{
										int						i = 0;
										int						SelSat_Count = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;
										MV_stSatInfo			Temp_SatData;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										SelSat_Count = MV_GetSelected_SatData_Count();
										
										for ( i = 0 ; i < SelSat_Count ; i++ )
										{
											MV_GetSelected_SatData_By_Count(&Temp_SatData, i );
											sprintf(stContents.Contents[i], "%s", Temp_SatData.acSatelliteName);
										}
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Usals_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = SelSat_Count;
										stContents.u8Focus_Position = MV_GetSelected_Index_By_Satindex(u8Glob_Sat_Focus);
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									} 
									break;

								case USALS_ITEM_TP:
									{
										int						i = 0;
										int						temp_Count = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;
										MV_stTPInfo				Temp_TP;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);
										
										for ( i = 0 ; i < temp_Count ; i++ )
										{
											MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TP, u8Glob_Sat_Focus, i);
											if ( MV_TPInfo.u8Polar_H == 1 )
												sprintf(stContents.Contents[i], "%d/%d. %d/%s/%d", i + 1, u8TpCount + 1, Temp_TP.u16TPFrequency, "H", Temp_TP.u16SymbolRate);
											else
												sprintf(stContents.Contents[i], "%d/%d. %d/%s/%d", i + 1, u8TpCount + 1, Temp_TP.u16TPFrequency, "V", Temp_TP.u16SymbolRate);
										}
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Usals_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4) - 40;
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 + 80;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = temp_Count;
										stContents.u8Focus_Position = u8Glob_TP_Focus;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									} 
									break;

								case USALS_ITEM_LONGITUDE:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "REF");
										
										for ( i = 1 ; i < 64 ; i++ )
											sprintf(stContents.Contents[i], "%d", i);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Usals_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 64;
										stContents.u8Focus_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								case USALS_ITEM_LATITUDE:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "REF");
										
										for ( i = 1 ; i < 64 ; i++ )
											sprintf(stContents.Contents[i], "%d", i);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Usals_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 64;
										stContents.u8Focus_Position = 0;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								case USALS_ITEM_LIMIT_SET:
									{
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "USE");
										sprintf(stContents.Contents[1], "%s", "NOT USE");
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Usals_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 2;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;
*/
								default:
									break;

							}
						} 
					}
					break;

				case CSAPP_KEY_INFO:
					break;

				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:					
					CSApp_UniCable_Applets=CSApp_Applet_Install;
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

