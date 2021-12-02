#include "linuxos.h"

#include "database.h"
#include "av_zapping.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define							FIELDS_PER_LINE				2

static CSAPP_Applet_t			CSApp_Audio_Applets;

static const U16 				audio_track[] = {
								CSAPP_STR_AUDIO_STEREO,
								CSAPP_STR_AUDIO_LEFT,
								CSAPP_STR_AUDIO_RIGHT,
								CSAPP_STR_AUDIO_MONO
							};

static const U16 				audio_item[CS_APP_AUDIO_SETTING_MAX] = {
								CSAPP_STR_AUDIO_LANG,
								CSAPP_STR_AUDIO_TRACK,
							};

U8	Audio_Arrow_Kind[CS_APP_AUDIO_SETTING_MAX] = {
	MV_SELECT,
	MV_SELECT
};

static tMWStream			AudioStream;
static U16					Current_Audio_Lang = 0;
static U16					Total_Audiot_Lang = 0;
static U16					Current_Audio_Track = 0;
static U16					Total_Audio_Track = 0;
static U16					Current_Item=0;

extern void EditSList_Draw_Saving(HDC hdc);
extern void EditSList_Close_Confirm(HDC hdc);
static int Audio_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);

void MV_Audio_SetWindow(HDC hdc)
{
	RECT	rc1;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + SM_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y + SM_WINDOW_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y),ScalerWidthPixel(SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(SM_WINDOW_DY));
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(SM_WINDOW_DX),ScalerHeigthPixel(SM_WINDOW_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = SM_WINDOW_TITLE_Y;
	rc1.left = SM_WINDOW_ITEM_X;
	rc1.bottom = SM_WINDOW_TITLE_Y + SM_WINDOW_ITEM_DY;
	rc1.right = rc1.left + SM_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_AUDIO_SETTING);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_CONT_Y),ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY * 2));

	MV_Draw_Audio_Bar(hdc);
}

void MV_Draw_AudioSelectBar(HDC hdc, int y_gap, U8 esItem)
{
	int mid_width = SM_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = SM_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	if ( Audio_Arrow_Kind[esItem] == MV_SELECT && Total_Audiot_Lang > 1)
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_CONT_DX - ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth) - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_AudioMenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 	y_gap = SM_WINDOW_CONT_Y + SM_WINDOW_ITEM_DY * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_AudioSelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY) );
	}					

	CS_MW_TextOut( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(audio_item[esItem]));

	TmpRect.left	=ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X);
	TmpRect.right	=TmpRect.left + SM_WINDOW_ITEM_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(SM_WINDOW_ITEM_DY);

	switch(esItem)
	{
		char	temp_str[20];
		
		case CS_APP_AUDIO_LANG:
			if(Total_Audiot_Lang==0)
				CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(CSAPP_STR_NONE), -1, &TmpRect, DT_CENTER);
			else
			{
				if(AudioStream.Stream[Current_Audio_Lang].Type == MW_SVC_AC3)
					snprintf(temp_str,20,"%s%s",CS_MW_LoadLanguageStringByIso(AudioStream.Stream[Current_Audio_Lang].Language),"(AC3)"); 
				else 
					snprintf(temp_str,20,"%s%s",CS_MW_LoadLanguageStringByIso(AudioStream.Stream[Current_Audio_Lang].Language),"(MP2)");
					
				CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			}
			break;
		case CS_APP_AUDIO_TRACK:
			CS_MW_DrawText(hdc, CS_MW_LoadStringByIdx(audio_track[Current_Audio_Track]), -1, &TmpRect, DT_CENTER);
			break;
		default:
			break;
	}
}

void MV_Draw_Audio_Bar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CS_APP_AUDIO_SETTING_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_AudioMenuBar(hdc, FOCUS, i);
		} else {
			MV_Draw_AudioMenuBar(hdc, UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t	CSApp_Audio(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Audio_Applets = CSApp_Applet_Audio;

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
	CreateInfo.spCaption = "csaudio window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Audio_Msg_cb;
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
	return CSApp_Audio_Applets;
    
}

static int Audio_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	tCS_DB_Error		err;

	switch(message)
	{
		case MSG_CREATE:
			CS_MW_GetAudioStream(&AudioStream);
			Current_Audio_Lang=0;
			Total_Audiot_Lang = AudioStream.Number;

			dprintf(("Total_Audiot_Lang=%d\n",Total_Audiot_Lang));

			Total_Audio_Track = CS_APP_AUDIO_MODE_MAX;
			Current_Audio_Track = 1;

			{
				tCS_DB_ServiceManageData 	item_data;
				MV_stServiceInfo				service_data;
				U16 							CurrentService,ii;

				CurrentService=CS_DB_GetCurrentService_OrderIndex();
				err = CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);
				if(err == eCS_DB_OK)
				{
					err = MV_DB_GetServiceDataByIndex(&service_data, item_data.Service_Index);
					if(err == eCS_DB_OK)
					{
						for(ii=0;ii<Total_Audiot_Lang;ii++)
						{
							if(service_data.u16AudioPid==AudioStream.Stream[ii].Pid)
							{
								Current_Audio_Lang=ii;
								Current_Audio_Track=service_data.u8Audio_Mode;
							}
						}
					}
				}
			}
			Current_Item=0;
			break;
		case MSG_PAINT:
			{
				hdc=BeginPaint(hwnd);
				
				MV_Audio_SetWindow(hdc);
/*					
				if(Total_Audiot_Lang==0)
					CS_MW_TextOut(hdc,ScalerWidthPixel(Audio_ComboList_First.element_fields[1].x), ScalerHeigthPixel(Audio_ComboList_First.element.y+5), "    None");
				else
				{
					char TextBuffer[20];

					if(AudioStream.Stream[Current_Audio_Lang].Type==MW_SVC_AC3)
					{
						snprintf(TextBuffer,20,"%s%s",CS_MW_LoadLanguageStringByIso(AudioStream.Stream[Current_Audio_Lang].Language),"(AC3)"); 
					}
					else 
					{
						snprintf(TextBuffer,20,"%s%s",CS_MW_LoadLanguageStringByIso(AudioStream.Stream[Current_Audio_Lang].Language),"(MP2)");
					}
					CS_MW_TextOut(hdc,ScalerWidthPixel(Audio_ComboList_First.element_fields[1].x), ScalerHeigthPixel(Audio_ComboList_First.element.y+8), TextBuffer);	    							
				}
				CS_MW_TextOut(hdc,ScalerWidthPixel(Audio_ComboList_First.element_fields[1].x+50), ScalerHeigthPixel(Audio_ComboList_First.element.y+ Audio_ComboList_First.element.dy+8), CS_MW_LoadStringByIdx(audio_track[Current_Audio_Track]));
*/
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Current_Item)
					{
						case CS_APP_AUDIO_LANG:
							Current_Audio_Lang = u8Result_Value;
							break;
						case CS_APP_AUDIO_TRACK:
							Current_Audio_Track = u8Result_Value;
							break;
						default:
							break;
					}

					if(Current_Item==CS_APP_AUDIO_TRACK)
					{
						tCS_DB_ServiceManageData item_data;
						U16 						CurrentService;
							
						CurrentService=CS_DB_GetCurrentService_OrderIndex();
						CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);
						CS_AV_AudioSetStereoMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX);
						MV_DB_ModifyAudioMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX,item_data.Service_Index);
					}
					else if((Current_Item==CS_APP_AUDIO_LANG)&&(Total_Audiot_Lang!=1))
					{
						tCS_DB_ServiceManageData item_data;
						U16 						CurrentService;
							
						CurrentService=CS_DB_GetCurrentService_OrderIndex();
						CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);

                            /* Modify By River 06.12.2008 */
						switch( AudioStream.Stream[Current_Audio_Lang].Type )
						{
							case MW_SVC_MP2:
								CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
								MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
								break;
                            
							case MW_SVC_AC3:
								CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AC3 );
								MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid,eCS_DB_AUDIO_AC3,item_data.Service_Index);
								break;

							case MW_SVC_AAC:
								CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AAC);
								MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_AAC, item_data.Service_Index );
								break;
                                
							case MW_SVC_LATM:
								CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_LATM );
								MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_LATM, item_data.Service_Index );
								break;
                                
							default:
								CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
								MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
								break;
						}
					}	
					hdc=BeginPaint(hwnd);
					MV_Draw_AudioMenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}
				break;
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_AUDIO:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Audio_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
				case CSAPP_KEY_ENTER:
					{
						int						i = 0;
						RECT					smwRect;
						stPopUp_Window_Contents stContents;

						memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
						smwRect.top = SM_WINDOW_CONT_Y + ( SM_WINDOW_ITEM_DY * ( Current_Item + 1 ));
						smwRect.left = ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X);
						smwRect.right = smwRect.left + SM_WINDOW_ITEM_NAME_X;
						
						switch(Current_Item)
						{
							case CS_APP_AUDIO_LANG:
								if ( Total_Audiot_Lang > 1 )
								{
									for ( i = 0 ; i < Total_Audiot_Lang ; i++ )
										sprintf(stContents.Contents[i], "%s", AudioStream.Stream[i].Language);
							
									smwRect.bottom = smwRect.top + SM_WINDOW_ITEM_DY * Total_Audiot_Lang;
									stContents.u8TotalCount = Total_Audiot_Lang;
									stContents.u8Focus_Position = 0;
									MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								}
								break;
							case CS_APP_AUDIO_TRACK:
								for ( i = 0 ; i < CS_APP_AUDIO_MODE_MAX ; i++ )
									sprintf(stContents.Contents[i], "%s", CS_MW_LoadStringByIdx(audio_track[i]));
						
								smwRect.bottom = smwRect.top + SM_WINDOW_ITEM_DY * CS_APP_AUDIO_MODE_MAX;
								stContents.u8TotalCount = CS_APP_AUDIO_MODE_MAX;
								stContents.u8Focus_Position = 0;
								MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								break;
							default:
								break;
						}
					}
					break;
				case CSAPP_KEY_IDLE:
    				CSApp_Audio_Applets = CSApp_Applet_Sleep;
    				SendMessage(hwnd,MSG_CLOSE,0,0);
    				break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_AudioMenuBar(hdc, UNFOCUS, Current_Item);
					
					if(Current_Item == CS_APP_AUDIO_LANG)
					{
						if(Total_Audiot_Lang==0) 
							break;
						Current_Item = CS_APP_AUDIO_TRACK;
					}
					else
						Current_Item--;
					
					MV_Draw_AudioMenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_DOWN:					
					hdc=BeginPaint(hwnd);
					MV_Draw_AudioMenuBar(hdc, UNFOCUS, Current_Item);

					if(Current_Item == CS_APP_AUDIO_TRACK)
						Current_Item = CS_APP_AUDIO_LANG;
					else
					{
						if(Total_Audiot_Lang==0) 
							break;
						Current_Item++;
					}
					
					MV_Draw_AudioMenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
				case CSAPP_KEY_LEFT:
					{
						U16	*total_element = &Total_Audiot_Lang;
						U16	*current = &Current_Audio_Lang;

						if(Total_Audiot_Lang==0) break;
                                                            
						switch(Current_Item)
						{
							case CS_APP_AUDIO_LANG:
								total_element = &Total_Audiot_Lang;
								current = &Current_Audio_Lang;
								break;
							case CS_APP_AUDIO_TRACK:
								total_element = &Total_Audio_Track;
								current = &Current_Audio_Track;
								break;
							default:
								total_element = &Total_Audiot_Lang;
								current = &Current_Audio_Lang;
								break;
						}

						if(*current == 0)
							*current = *total_element-1;
						else
							(*current)--;

						if(Current_Item==CS_APP_AUDIO_TRACK)
						{
							tCS_DB_ServiceManageData item_data;
							U16 						CurrentService;
								
							CurrentService=CS_DB_GetCurrentService_OrderIndex();
							CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);
							CS_AV_AudioSetStereoMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX);
							MV_DB_ModifyAudioMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX,item_data.Service_Index);
						}
						else if((Current_Item==CS_APP_AUDIO_LANG)&&(Total_Audiot_Lang!=1))
						{
							tCS_DB_ServiceManageData item_data;
							U16 						CurrentService;
								
							CurrentService=CS_DB_GetCurrentService_OrderIndex();
							CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);

                                /* Modify By River 06.12.2008 */
							switch( AudioStream.Stream[Current_Audio_Lang].Type )
							{
								case MW_SVC_MP2:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
									MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
									break;
                                
								case MW_SVC_AC3:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AC3 );
									MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid,eCS_DB_AUDIO_AC3,item_data.Service_Index);
									break;

								case MW_SVC_AAC:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AAC);
									MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_AAC, item_data.Service_Index );
									break;
                                    
								case MW_SVC_LATM:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_LATM );
									MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_LATM, item_data.Service_Index );
									break;
                                    
								default:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
									MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
									break;
							}
						}

							//ComboList_Update_Element(hwnd, Current_Item);
						hdc=BeginPaint(hwnd);
						MV_Draw_AudioMenuBar(hdc, FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
								
					}
					break;
				case CSAPP_KEY_RIGHT:
					{
						U16	*total_element = &Total_Audiot_Lang;
						U16	*current = &Current_Audio_Lang;

						if(Total_Audiot_Lang==0) break;

						switch(Current_Item)
						{
							case CS_APP_AUDIO_LANG:
								total_element = &Total_Audiot_Lang;
								current = &Current_Audio_Lang;
								break;
							case CS_APP_AUDIO_TRACK:
								total_element = &Total_Audio_Track;
								current = &Current_Audio_Track;
								break;
							default:
								total_element = &Total_Audiot_Lang;
								current = &Current_Audio_Lang;
								break;
						}

						if(*current == (*total_element-1))
							*current = 0;
						else
							(*current)++;

						if(Current_Item==CS_APP_AUDIO_TRACK)
						{
							tCS_DB_ServiceManageData item_data;
							U16 						CurrentService;

							CurrentService=CS_DB_GetCurrentService_OrderIndex();
							CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);
							CS_AV_AudioSetStereoMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX);
							MV_DB_ModifyAudioMode(Current_Audio_Track%CS_APP_AUDIO_MODE_MAX,item_data.Service_Index);
						}
						else if((Current_Item==CS_APP_AUDIO_LANG)&&(Total_Audiot_Lang!=1))
						{
							tCS_DB_ServiceManageData item_data;
							U16 						CurrentService;

							CurrentService=CS_DB_GetCurrentService_OrderIndex();
							CS_DB_GetCurrentList_ServiceData(&item_data, CurrentService);

	                                /* Modify By River 06.12.2008 */
							switch( AudioStream.Stream[Current_Audio_Lang].Type )
							{
								case MW_SVC_MP2:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
	    							MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
									break;                                    

								case MW_SVC_AC3:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AC3 );
									MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid,eCS_DB_AUDIO_AC3,item_data.Service_Index);
									break;

								case MW_SVC_AAC:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_AAC);
									MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_AAC, item_data.Service_Index );
									break;

								case MW_SVC_LATM:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_LATM );
									MV_DB_ModifyServiceAudio( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_LATM, item_data.Service_Index );
									break;

								default:
									CS_AV_ChangeAudioPid( AudioStream.Stream[Current_Audio_Lang].Pid, eCS_AV_AUDIO_STREAM_MPEG2 );
	    							MV_DB_ModifyServiceAudio(AudioStream.Stream[Current_Audio_Lang].Pid, eCS_DB_AUDIO_MPEG2, item_data.Service_Index);
									break; 
							}
						}

						//ComboList_Update_Element(hwnd, Current_Item);
						hdc=BeginPaint(hwnd);
						MV_Draw_AudioMenuBar(hdc, FOCUS, Current_Item);
						EndPaint(hwnd,hdc);
									
					}
					break;
				default:
					break;
			}
			break;
		case MSG_CLOSE:
			CS_APP_SetFirstInDesktop(FALSE);
			if(CS_DB_CheckIfChanged())
			{
				hdc = BeginPaint(hwnd);
				EditSList_Draw_Saving(hdc);
				EndPaint(hwnd,hdc);
				
				dprintf(("save\n"));
				CS_DB_SaveDatabase();

				hdc = BeginPaint(hwnd);
				EditSList_Close_Confirm(hdc);
				EndPaint(hwnd,hdc);
			}
			
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
		default:
			break;		
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

