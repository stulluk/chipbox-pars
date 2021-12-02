#include "linuxos.h"
#include "ci_api.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cscimmi.h"
#include "userdefine.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "database.h"
#include "casapi.h"

#define	MAX_CI_LINE			10

static CSAPP_Applet_t		CSApp_CI_Applets;

static U8					u8CI_Focus_Item = 0;
static U8					u8Menu_List_cnt = 0;
static U8					u8Menu_Subtitle_cnt = 0;
static U8					u8Cur_Page = 0;
static char					acMenu_List[100][100];
static char					acMenu_Title[256];
static ci_text_t			acSubTitle[10];
static char					acBTMText[256];

static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;

typedef enum{
	APP_CI_MMI_STATUS_MENU   = 0x00,
	APP_CI_MMI_STATUS_LIST,
	APP_CI_MMI_STATUS_ENQUIRY,
	APP_CI_MMI_STATUS_NONE
} eCSAPP_CI_MMI_Status;


static int CI_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

void MV_Clear_CI_Menu(HDC hdc)
{
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	MV_FillBox( hdc, ScalerWidthPixel(MV_MENU_BACK_X), ScalerHeigthPixel(MV_MENU_BACK_Y), ScalerWidthPixel(MV_MENU_BACK_DX), ScalerHeigthPixel(MV_MENU_BACK_DY) );
}

void MV_Subtitle_Str_Parsing(char	*TempStr)
{
	unsigned int 	i, j = 0;
	char			Str_Temp[100];

	u8Menu_Subtitle_cnt = 0;

	memset(acSubTitle, 0x00, sizeof(ci_text_t)*10);

	for ( i = 0 ; i < strlen(TempStr) ; i++ )
	{
		if ( TempStr[i] > 0x7A || TempStr[i] < 0x20 )
		{
			strncpy(acSubTitle[u8Menu_Subtitle_cnt].text, Str_Temp, j);
			acSubTitle[u8Menu_Subtitle_cnt].length = j;
			memset(Str_Temp, 0x00, 100);
			j = 0;
			u8Menu_Subtitle_cnt++;
		} else {
			Str_Temp[j] = TempStr[i];
			j++;
		}
	}
}

void MV_Draw_CI_Status(HDC hdc, U8 u8Focuskind, char *TempStr)
{
	int 	y_gap = MV_CI_MENU_STATE_Y;
	RECT	TmpRect;
	
	if( u8Focuskind == MV_UNFOCUS )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_CI_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_CI_MENU_X*2),ScalerHeigthPixel(MV_CI_MENU_Y_GAP) );
	} else {
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_CI_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_CI_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel((MV_CI_MENU_DX - MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth) - MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel((MV_CI_MENU_X + MV_CI_MENU_DX) - MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	}

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X + 20);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	CS_MW_DrawText(hdc, TempStr, -1, &TmpRect, DT_LEFT);
}

void MV_Draw_CIMenuBar(HDC hdc, U8 u8Focuskind, eMV_CAS_Items esItem)
{
	int 	y_gap = MV_CI_MENU_Y + ( MV_CI_MENU_Y_GAP * esItem );
	RECT	TmpRect;
	
	if( u8Focuskind == MV_UNFOCUS )
	{
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_CI_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_CI_MENU_X*2),ScalerHeigthPixel(MV_CI_MENU_Y_GAP) );
	} else {
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_CI_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_CI_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel((MV_CI_MENU_DX - MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth) - MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel((MV_CI_MENU_X + MV_CI_MENU_DX) - MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
	}

	//printf("\n################ %d ###############\n",esItem);

	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X + 10);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	CS_MW_DrawText(hdc, acMenu_List[esItem + ( u8Cur_Page * MAX_CI_LINE)], -1, &TmpRect, DT_LEFT);
}

void MV_Draw_CI_Line(HDC hdc)
{
	RECT	TmpRect;
	int 	y_gap = MV_CI_MENU_Y - ( MV_CI_MENU_Y_GAP * 4 + 2 );

	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X - 150);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX + 300;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + 4;

	MV_Draw_Box(hdc, &TmpRect, MVAPP_GRAY_COLOR, DR_TOP );

	y_gap = MV_CI_MENU_Y - MV_CI_MENU_Y_GAP + 2;
	
	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X - 150);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX + 300;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + 4;
	MV_Draw_Box(hdc, &TmpRect, MVAPP_GRAY_COLOR, DR_TOP );

	if ( u8Menu_List_cnt > MAX_CI_LINE )
		y_gap = MV_CI_MENU_Y + ( MV_CI_MENU_Y_GAP * MAX_CI_LINE ) + 8;
	else
		y_gap = MV_CI_MENU_Y + ( MV_CI_MENU_Y_GAP * ( u8Menu_List_cnt )) + 8;
	
	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X - 150);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX + 300;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + 4;
	MV_Draw_Box(hdc, &TmpRect, MVAPP_GRAY_COLOR, DR_TOP );
}

void MV_Draw_CI_Title(HDC hdc)
{
	int 	y_gap = MV_CI_MENU_Y - ( MV_CI_MENU_Y_GAP * 5 );
	RECT	TmpRect;

	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );	
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	CS_MW_DrawText(hdc, acMenu_Title, -1, &TmpRect, DT_CENTER);

	MV_Draw_CI_Line(hdc);
}

void MV_Draw_CI_SubTitle(HDC hdc)
{
	int 	y_gap = MV_CI_MENU_SUBTITLE;
	RECT	TmpRect;
	int		i;

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	for ( i = 0 ; i < u8Menu_Subtitle_cnt ; i++ )
	{
		TmpRect.left	= 200;
		TmpRect.right	= 1280;
		TmpRect.top		= ScalerHeigthPixel(y_gap + (MV_CI_MENU_Y_GAP * i ));
		TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

		CS_MW_DrawText(hdc, acSubTitle[i].text, -1, &TmpRect, DT_LEFT);
	}
}

void MV_Draw_CI_BTMText(HDC hdc)
{
	int 	y_gap = MV_CI_MENU_Y + ( MV_CI_MENU_Y_GAP * ( u8Menu_List_cnt + 1 ));
	RECT	TmpRect;

	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
	
	TmpRect.left	= ScalerWidthPixel(MV_CI_MENU_X);
	TmpRect.right	= TmpRect.left + MV_CI_MENU_DX;
	TmpRect.top		= ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	= TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	CS_MW_DrawText(hdc, acBTMText, -1, &TmpRect, DT_CENTER);
}

void MV_Draw_CI_MenuBar(HDC hdc)
{
	U8		i = 0;
	U8		u8Start = u8Cur_Page * MAX_CI_LINE;

	if ( u8Menu_List_cnt >= MAX_CI_LINE )
	{
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(MV_CI_MENU_X),ScalerHeigthPixel( MV_CI_MENU_Y ), ScalerWidthPixel(ScreenWidth - MV_CI_MENU_X*2),ScalerHeigthPixel(MV_CI_MENU_Y_GAP * MAX_CI_LINE) );
	}
	
	if ( ( u8Menu_List_cnt - u8Start ) < MAX_CI_LINE )
	{
		for( i = 0 ; i < ( u8Menu_List_cnt - u8Start ) ; i++ )
		{
			if ( i == u8CI_Focus_Item )
				MV_Draw_CIMenuBar(hdc, MV_FOCUS, i);
			else
				MV_Draw_CIMenuBar(hdc, MV_UNFOCUS, i);
		}
	} else {
		for( i = 0 ; i < MAX_CI_LINE ; i++ )
		{
			if ( i == u8CI_Focus_Item )
				MV_Draw_CIMenuBar(hdc, MV_FOCUS, i);
			else
				MV_Draw_CIMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

void MV_Draw_CI_Menu(HDC hdc)
{
	MV_Clear_CI_Menu(hdc);
	MV_Draw_CI_Title(hdc);
	
	MV_Draw_CI_MenuBar(hdc);
	
	MV_Draw_CI_SubTitle(hdc);
	MV_Draw_CI_BTMText(hdc);
}

CSAPP_Applet_t CSApp_CI(void)
{
	   	int 				BASE_X, BASE_Y, WIDTH, HEIGHT;
		MSG 				msg;
		HWND				hwndMain;
		MAINWINCREATE		CreateInfo;
	
		CSApp_CI_Applets = CSApp_Applet_Error;
			
#ifdef  Screen_1080
        BASE_X = 0;
        BASE_Y = 0;
        WIDTH  = 1920;
        HEIGHT = 1080;
#else
        BASE_X = 0;
        BASE_Y = 0;
        WIDTH  = ScalerWidthPixel(1280);
        HEIGHT = ScalerHeigthPixel(720);
#endif

	
		CreateInfo.dwStyle	 = WS_VISIBLE;
		CreateInfo.dwExStyle = WS_EX_NONE;
		CreateInfo.spCaption = "ci_mmi";
		CreateInfo.hMenu	 = 0;
		CreateInfo.hCursor	 = 0;
		CreateInfo.hIcon	 = 0;
		CreateInfo.MainWindowProc = CI_Msg_cb;
		CreateInfo.lx = BASE_X;
		CreateInfo.ty = BASE_Y;
		CreateInfo.rx = BASE_X+WIDTH;
		CreateInfo.by = BASE_Y+HEIGHT;
		CreateInfo.iBkColor = CSAPP_BLACK_COLOR;
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
	
		return CSApp_CI_Applets;
   
}


static int CI_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{ 
   	HDC 								hdc;
	int									count=0;
	int 								rc;
	static BOOL							init_flag = TRUE;	
	unsigned char 						strBuf[80],strText[80];
	static eCSAPP_CI_MMI_Status 		mmi_status = APP_CI_MMI_STATUS_NONE;
   
	switch(message)
	{
		case MSG_CREATE:
			u8CI_Focus_Item = 0;
			u8Menu_List_cnt = 0;
			u8Cur_Page = 0;
			mmi_status = APP_CI_MMI_STATUS_NONE;
			memset(acMenu_List, 0x00, (100*100));
			memset(acSubTitle, 0x00, sizeof(ci_text_t)*10);
			break;
			
		case MSG_PAINT:
			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_TOOL, EN_ITEM_FOCUS_CI);
			
			hdc = BeginPaint(hwnd);

			MV_TOOLS_draw_help_banner(hdc, EN_ITEM_FOCUS_CI);
			
			//printf("CI_MMI_Msg_cb MSG_PAINT init_flag = %d\n", init_flag);

			if(init_flag)
			{
				MV_Clear_CI_Menu(hdc);
				mmi_status = APP_CI_MMI_STATUS_MENU;
				
				if(MW_CI_MMI_Get_App_Info_Status())
				{
					memset(strText, 0x00, 80);
					memset(strBuf, 0x00, 80);
					MW_CI_Get_Cam_Information(strText);

					SetBkMode(hdc,BM_TRANSPARENT);
					SetTextColor(hdc,CSAPP_WHITE_COLOR);

					sprintf(strBuf, "Slot 1 : %s", strText);
					MV_Draw_CI_Status(hdc, MV_FOCUS, strBuf);
					//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), strBuf);
				}
				else
				{
					SetBkMode(hdc,BM_TRANSPARENT);
					SetTextColor(hdc,CSAPP_WHITE_COLOR);
					sprintf(strBuf, "Slot 1 : %s", CS_MW_LoadStringByIdx(CSAPP_STR_MODULE_NO));
					MV_Draw_CI_Status(hdc, MV_UNFOCUS, strBuf);
					//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), "Slot 1 : No Modulel");
				}

				init_flag = FALSE;
			}
			//else


			EndPaint(hwnd,hdc);
			return 0;

		case MSG_CI_MMI_UPDATE:
			{	
				switch(wparam)
				{
					case MSG_PARAM_CI_Insert_Notify:				
						hdc = BeginPaint(hwnd);
						MV_Clear_CI_Menu(hdc);
						SetBkMode(hdc,BM_TRANSPARENT);
						SetTextColor(hdc,CSAPP_WHITE_COLOR);
						mmi_status = APP_CI_MMI_STATUS_MENU;
						switch(lparam)
						{
							case 0:
							case 1:
							case 2:
							case 3:
								sprintf(strBuf, "Slot 1 : %s", CS_MW_LoadStringByIdx(CSAPP_STR_MODULE_INIT));
								MV_Draw_CI_Status(hdc, MV_UNFOCUS, strBuf);
								//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), "Slot 1 : Module Initialing......");
								break;
							case 4:
								sprintf(strBuf, "Slot 1 : %s", CS_MW_LoadStringByIdx(CSAPP_STR_MODULE_OK));
								MV_Draw_CI_Status(hdc, MV_UNFOCUS, strBuf);
								//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), "Slot 1 : Module Ok");
								break;
							case 5:
							default:
								sprintf(strBuf, "Slot 1 : %s", CS_MW_LoadStringByIdx(CSAPP_STR_MODULE_NO));
								MV_Draw_CI_Status(hdc, MV_UNFOCUS, strBuf);
								//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), "Slot 1 : No Module");
								break;
						}
						EndPaint(hwnd,hdc);
						break;
					case MSG_PARAM_CI_App_Info_Changed:						
						hdc = BeginPaint(hwnd);
						if((count = MW_CI_Get_Cam_Information(strText)) > 0)
						{
							MV_Clear_CI_Menu(hdc);
							SetBkMode(hdc,BM_TRANSPARENT);
							SetTextColor(hdc,CSAPP_WHITE_COLOR);
							
							mmi_status = APP_CI_MMI_STATUS_MENU;
							
							memset(strText, 0x00, 80);
							memset(strBuf, 0x00, 80);

							//count = MW_CI_Get_Cam_Information(strText);

							usleep( 1000*1000 );
							MW_CI_Get_Cam_Information(strText);
							
							if( count > 0)
							{
								sprintf(strBuf, "Slot 1 : %s", strText);
								MV_Draw_CI_Status(hdc, MV_FOCUS, strBuf);
								//printf("=== strBuf : %s ==============\n",strBuf);
								//CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), strBuf);
							} else {
								//printf("MW_CI_Get_Cam_Information : %d \n", count);
							}
						}else {
							//printf("MSG_PARAM_CI_App_Info_Changed : %d \n", MW_CI_MMI_Get_App_Info_Status());
						}
						EndPaint(hwnd,hdc);
						break;
					case MSG_PARAM_CI_Display_Enquiry:
						{
							tMW_CI_Enquiry_info 	*enq_info;
							unsigned char 			buf[60];

							enq_info = (tMW_CI_Enquiry_info *)lparam;

							mmi_status = APP_CI_MMI_STATUS_ENQUIRY;

							if(enq_info->length > 58)
								enq_info->length = 58;
							
							memset(buf, 0x00, 60);

							if(enq_info->length)
							{
								memcpy(buf, enq_info->text, enq_info->length);
								/*
								CS_MW_TextOut(hdc,ScalerWidthPixel(MV_CI_MENU_X), ScalerHeigthPixel(MV_CI_MENU_Y), buf);
								*/
								MV_Draw_PIN_Window_For_CI(hwnd, buf);
							}

							free(enq_info->text);
						}
						break;
					case MSG_PARAM_CI_Display_List:
					case MSG_PARAM_CI_Display_Menu:
					//case MSG_PARAM_CI_Cas_Info_Arrived:
						{
							menu_t * 		p_menu;
							unsigned char 	i = 0;

							hdc = BeginPaint(hwnd);
							p_menu = mmi_get_menu();
							
							memset(acMenu_List, 0x00, (100*100));

							if(p_menu->is_valid)
							{
								u8CI_Focus_Item = 0;
								mmi_status = APP_CI_MMI_STATUS_LIST;
								
								u8Menu_List_cnt	 = p_menu->cnt;

								if(p_menu->title.length)
								{
									memset(acMenu_Title, 0x00, 256);
									strncpy(acMenu_Title, p_menu->title.text, strlen(p_menu->title.text));
								}

								for(i = 0 ; i < u8Menu_List_cnt ; i++ )
								{
									if(p_menu->item[i].length)
									{
										strncpy( acMenu_List[i], p_menu->item[i].text, strlen(p_menu->item[i].text));
									}
								}

								if(p_menu->subtitle.length)
								{
									MV_Subtitle_Str_Parsing(p_menu->subtitle.text);
								}

								if(p_menu->btmtext.length)
								{
									memset(acBTMText, 0x00, 256);
									strncpy(acBTMText, p_menu->btmtext.text, strlen(p_menu->btmtext.text));
								}

								MV_Draw_CI_Menu(hdc);
							}

							if ( p_menu != NULL )
								mmi_free_menu(p_menu);
							
							EndPaint(hwnd,hdc);
						}
						break;

					case MSG_PARAM_CI_Clear_Display:
						break;
					default:
						break;
				}
			}
			break;

		case MSG_KEYDOWN:
			if (MV_Get_Password_Flag() == TRUE)
			{
				MV_Password_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					char		Temp_Str[100];
					int			answer = 0;
					
					if( wparam == CSAPP_KEY_ENTER )
					{
						memset(Temp_Str, 0x00, 100);
						
						sprintf(Temp_Str, "%s", MV_Password_Return_Str());
						answer = mmi_enter_input(1, Temp_Str, strlen(Temp_Str));
						//printf("\n=========== %d , %s, %d ==================\n", answer, Temp_Str, strlen(Temp_Str));
					} else {
						answer = mmi_enter_input(0, NULL, 0);
					}
				}
				break;
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_ENTER:
					if(mmi_status == APP_CI_MMI_STATUS_MENU)
					{
						rc = ai_enter_menu(0); //return 0 sucess.

						u8Cur_Page = 0;
						
						if(rc !=0)
						{
							printf("enter menu call FALSE\n");
						} else {
							//printf("enter menu call SUCCESS\n");
						}
					}
					else if(mmi_status == APP_CI_MMI_STATUS_LIST)
					{
						rc = mmi_enter_choice(( u8CI_Focus_Item + ( u8Cur_Page * MAX_CI_LINE ) ) + 1);
						if(rc !=0)
						{
							printf("enter list call %d FALSE\n", u8CI_Focus_Item + 1);
						} else {
							//printf("enter list call %d SUCCESS\n", u8CI_Focus_Item + 1);
						}
					}
					break;
				case CSAPP_KEY_ESC:
					if(mmi_status == APP_CI_MMI_STATUS_LIST)
					{
						rc = mmi_enter_choice(0);
						if(rc !=0)
						{
							printf("exit list call FALSE\n");
						}
					}
					else// if(mmi_status == APP_CI_MMI_STATUS_MENU)
					{
						CSApp_CI_Applets=CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;
					
				case CSAPP_KEY_MENU:
					if(mmi_status == APP_CI_MMI_STATUS_LIST)
					{
						rc = mmi_enter_choice(0);
						if(rc !=0)
						{
							printf("exit list call FALSE\n");
						}
					}
					else// if(mmi_status == APP_CI_MMI_STATUS_MENU)
					{
						CSApp_CI_Applets=b8Last_App_Status;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;

				case CSAPP_KEY_UP:
					if (mmi_status == APP_CI_MMI_STATUS_LIST )
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_CIMenuBar(hdc, MV_UNFOCUS, u8CI_Focus_Item);
						
						if ( u8CI_Focus_Item == 0 )
						{
							if ( u8Menu_List_cnt < MAX_CI_LINE )
								u8CI_Focus_Item = u8Menu_List_cnt - 1;
							else
							{
								if ( u8Cur_Page > 0 )
								{
									u8Cur_Page--;
									u8CI_Focus_Item = MAX_CI_LINE - 1;
									MV_Draw_CI_MenuBar(hdc);
								}
								else
								{
									u8Cur_Page = u8Menu_List_cnt/MAX_CI_LINE;
									if ( ( u8Menu_List_cnt%MAX_CI_LINE ) == 0 )
										u8CI_Focus_Item = 9;
									else
										u8CI_Focus_Item = ( u8Menu_List_cnt%MAX_CI_LINE ) - 1;
									MV_Draw_CI_MenuBar(hdc);
								}
							}
						}
						else
						{
							u8CI_Focus_Item--;
							MV_Draw_CIMenuBar(hdc, MV_FOCUS, u8CI_Focus_Item);
						}
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_DOWN:
					if (mmi_status == APP_CI_MMI_STATUS_LIST )
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_CIMenuBar(hdc, MV_UNFOCUS, u8CI_Focus_Item);
						
						if ( ( u8CI_Focus_Item + ( u8Cur_Page * MAX_CI_LINE )) == u8Menu_List_cnt - 1 )
						{
							u8Cur_Page = 0;
							u8CI_Focus_Item = 0;
							MV_Draw_CI_MenuBar(hdc);
						}
						else
						{
							if ( u8Menu_List_cnt < MAX_CI_LINE )
							{
								u8CI_Focus_Item++;
								MV_Draw_CIMenuBar(hdc, MV_FOCUS, u8CI_Focus_Item);
							} else {
								if ( u8CI_Focus_Item == MAX_CI_LINE - 1 )
								{
									u8Cur_Page++;
									u8CI_Focus_Item = 0;
									MV_Draw_CI_MenuBar(hdc);
								} else {
									u8CI_Focus_Item++;
									MV_Draw_CIMenuBar(hdc, MV_FOCUS, u8CI_Focus_Item);
								}
							}
						}
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_IDLE:
					CSApp_CI_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
				default:
					break;
			}
			break;

		case MSG_CLOSE:
			init_flag = TRUE;
			//CS_CI_UnRegister_CI_Notify();
			mmi_status = APP_CI_MMI_STATUS_NONE;

			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			break;

		default:
			break;
	}

   
   return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

