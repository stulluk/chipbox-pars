#include "linuxos.h"

#include "database.h"
#include "av_zapping.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"

#define	RECALL_DY	SM_WINDOW_DY + (SM_WINDOW_ITEM_DY*8)

enum
{
	CS_APP_RECALL_1 = 0,
	CS_APP_RECALL_2,
	CS_APP_RECALL_3,
	CS_APP_RECALL_4,
	CS_APP_RECALL_5,
	CS_APP_RECALL_6,
	CS_APP_RECALL_7,
	CS_APP_RECALL_8,
	CS_APP_RECALL_9,
	CS_APP_RECALL_10,
	CS_APP_RECALL_MAX
};

static CSAPP_Applet_t			CSApp_Recall_Applets;
static U16						Current_Item=0;
static U16						Current_Play_Index = 0;
static tCS_DBU_Service     		servicetriplet[MV_RECALL_MAX_ITEM];
static MV_stServiceInfo 		pServiceData;

static int Recall_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
static void MV_Recall_SetWindow(HDC hdc);
static void MV_Draw_Recall_SelectBar(HDC hdc, int y_gap);
static void MV_Draw_Recall_MenuBar(HDC hdc, U8 u8Focuskind, U8 esItem);
static void MV_Draw_Recall_Bar(HDC hdc);
static U8 MV_Count_Recall_Size(void);

void MV_Set_Recall_Current_List(U32 List_type, U8 u8List_Index)
{
	tCS_DB_ServiceListTriplet	Recall_triplet;
	
	switch ( List_type )
	{
		case eCS_DB_RADIO_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_RADIO_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case eCS_DB_TV_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = 0;
			break;
		case eCS_DB_FAV_TV_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_TV_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = u8List_Index;
			break;
		case eCS_DB_FAV_RADIO_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_FAV_RADIO_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = u8List_Index;
			break;
		case eCS_DB_SAT_TV_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_TV_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = u8List_Index;
			break;
		case eCS_DB_SAT_RADIO_LIST:
			Recall_triplet.sCS_DB_ServiceListType = eCS_DB_SAT_RADIO_LIST;
			Recall_triplet.sCS_DB_ServiceListTypeValue = u8List_Index;
			break;
		default:
			break;
	}
	
	CS_DB_SetCurrentList(Recall_triplet, FALSE);
}

static U8 MV_Count_Recall_Size(void)
{
	int		i;

	for ( i = 0 ; i < CS_APP_RECALL_MAX ; i++ )
	{
		if ( servicetriplet[i].sCS_DBU_ServiceIndex == 0xFFFFFFFF )
		{
			return i;
		}
	}
	return CS_APP_RECALL_MAX;
}

static void MV_Recall_SetWindow(HDC hdc)
{
	RECT	rc1;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + RECALL_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y + RECALL_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y),ScalerWidthPixel(SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(RECALL_DY));
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(SM_WINDOW_DX),ScalerHeigthPixel(RECALL_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = SM_WINDOW_TITLE_Y;
	rc1.left = SM_WINDOW_ITEM_X;
	rc1.bottom = SM_WINDOW_TITLE_Y + SM_WINDOW_ITEM_DY;
	rc1.right = rc1.left + SM_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_RECALL);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_CONT_Y),ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY * 2));

	MV_Draw_Recall_Bar(hdc);
}

static void MV_Draw_Recall_SelectBar(HDC hdc, int y_gap)
{
	int mid_width = SM_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = SM_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);
}

static void MV_Draw_Recall_MenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 				y_gap = SM_WINDOW_CONT_Y + SM_WINDOW_ITEM_DY * esItem;
	RECT				TmpRect;
	U16					u16OrginalIndex = 0;

	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_Recall_SelectBar(hdc, y_gap);
	} else {	
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		
		if ( esItem%2 == 0 )
		{
			MV_SetBrushColor( hdc, MVAPP_DARKBLUE_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY) );
		} else {
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY) );
		}
	}

	TmpRect.left	=ScalerWidthPixel(SM_WINDOW_ITEM_X) + 16;
	TmpRect.right	=TmpRect.left + SM_WINDOW_CONT_DX - 32;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(SM_WINDOW_ITEM_DY);

	if ( servicetriplet[esItem].sCS_DBU_ServiceIndex == 0xFFFFFFFF )
	{
		CS_MW_DrawText(hdc, "-", -1, &TmpRect, DT_LEFT);
	} else {
		u16OrginalIndex = MV_DB_GetServiceindex_ByOrderIndex(servicetriplet[esItem].sCS_DBU_ServiceList.sCS_DB_ServiceListType, servicetriplet[esItem].sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, servicetriplet[esItem].sCS_DBU_ServiceIndex);
		MV_DB_GetServiceDataByIndex(&pServiceData, u16OrginalIndex);
		
		CS_MW_DrawText(hdc, pServiceData.acServiceName, -1, &TmpRect, DT_LEFT);
	}
}

static void MV_Draw_Recall_Bar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CS_APP_RECALL_MAX ; i++ )
	{		
		if( Current_Item == i )
		{
			MV_Draw_Recall_MenuBar(hdc, FOCUS, i);
		} else {
			MV_Draw_Recall_MenuBar(hdc, UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t	CSApp_Recall(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_Recall_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption = "Recall window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = Recall_Msg_cb;
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
	return CSApp_Recall_Applets;
    
}

static int Recall_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	MV_stServiceInfo 	pTemp_ServiceData;
	U16					u16OrginalIndex = 0;

	switch(message)
	{
		case MSG_CREATE:
			Current_Item = 0;
			Current_Play_Index = wparam;
			MV_Get_ReCall_List(servicetriplet);
			break;
			
		case MSG_PAINT:
			{
				hdc=BeginPaint(hwnd);
				MV_Recall_SetWindow(hdc);
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:

			if (MV_Get_Password_Flag() == TRUE)
			{			
				MV_Password_Proc(hwnd, wparam);
				switch(wparam)
				{
					case CSAPP_KEY_0:
					case CSAPP_KEY_1:
					case CSAPP_KEY_2:
					case CSAPP_KEY_3:
					case CSAPP_KEY_4:
					case CSAPP_KEY_5:
					case CSAPP_KEY_6:
					case CSAPP_KEY_7:
					case CSAPP_KEY_8:
					case CSAPP_KEY_9:
						if(MV_Password_Retrun_Value() == TRUE)
						{
							
							MV_Password_Set_Flag(FALSE);
							hdc = BeginPaint(hwnd);
							MV_Restore_PopUp_Window( hdc );
							EndPaint(hwnd,hdc);
							
							CS_DB_SetLastServiceTriplet();
							MV_Set_Recall_Current_List(servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListType, servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue);
							CS_DB_SetCurrentService_OrderIndex(servicetriplet[Current_Item].sCS_DBU_ServiceIndex);							
							CS_DBU_SaveCurrentService(servicetriplet[Current_Item]);
							FbSendFndDisplayNum((unsigned)servicetriplet[Current_Item].sCS_DBU_ServiceIndex+1);		
							CS_APP_SetLastUnlockServiceIndex((unsigned)servicetriplet[Current_Item].sCS_DBU_ServiceIndex);
							printf("===== %d =====>>>>>>\n", servicetriplet[Current_Item].sCS_DBU_ServiceIndex);
							CSApp_Recall_Applets = CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;
						
					case CSAPP_KEY_ENTER:
						if(MV_Password_Retrun_Value() == TRUE)
						{
							
							MV_Password_Set_Flag(FALSE);
							hdc = BeginPaint(hwnd);
							MV_Restore_PopUp_Window( hdc );
							EndPaint(hwnd,hdc);
							
							CS_DB_SetLastServiceTriplet();
							MV_Set_Recall_Current_List(servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListType, servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue);
							CS_DB_SetCurrentService_OrderIndex(servicetriplet[Current_Item].sCS_DBU_ServiceIndex);							
							CS_DBU_SaveCurrentService(servicetriplet[Current_Item]);
							FbSendFndDisplayNum((unsigned)servicetriplet[Current_Item].sCS_DBU_ServiceIndex+1);
							CS_APP_SetLastUnlockServiceIndex((unsigned)servicetriplet[Current_Item].sCS_DBU_ServiceIndex);
							CSApp_Recall_Applets = CSApp_Applet_Desktop;
							SendMessage(hwnd,MSG_CLOSE,0,0);
						}
						break;

					case CSAPP_KEY_ESC:
					case CSAPP_KEY_MENU:
						MV_Password_Set_Flag(FALSE);
						hdc = BeginPaint(hwnd);
						MV_Restore_PopUp_Window( hdc );
						EndPaint(hwnd,hdc);
						break;
				}
				break;
			}
			
			switch(wparam)
			{
				case CSAPP_KEY_ENTER:
					u16OrginalIndex = MV_DB_GetServiceindex_ByOrderIndex(servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListType, servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, servicetriplet[Current_Item].sCS_DBU_ServiceIndex);
					MV_DB_GetServiceDataByIndex(&pTemp_ServiceData, u16OrginalIndex);

					//printf("==== %d : %d : %s : %d ====\n", servicetriplet[Current_Item].sCS_DBU_ServiceIndex, pTemp_ServiceData.u16ChIndex, pTemp_ServiceData.acServiceName, pTemp_ServiceData.u8Lock);
#if 0
					if ((CS_MW_GetServicesLockStatus()) && (pTemp_ServiceData.u8Lock== eCS_DB_LOCKED))
					{
						MV_Draw_Password_Window(hwnd);
					}
					else
#endif
					{	
						CS_DB_SetLastServiceTriplet();

						MV_Set_Recall_Current_List(servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListType, servicetriplet[Current_Item].sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue);

						CS_DB_SetCurrentService_OrderIndex(servicetriplet[Current_Item].sCS_DBU_ServiceIndex);
						
						CS_DBU_SaveCurrentService(servicetriplet[Current_Item]);

						FbSendFndDisplayNum((unsigned)servicetriplet[Current_Item].sCS_DBU_ServiceIndex+1);
						CS_APP_SetLastUnlockServiceIndex(0xffff);				
						CSApp_Recall_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					}
					break;
					
				case CSAPP_KEY_IDLE:
    				CSApp_Recall_Applets = CSApp_Applet_Sleep;
    				SendMessage(hwnd,MSG_CLOSE,0,0);
    				break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_Recall_MenuBar(hdc, UNFOCUS, Current_Item);
					
					if(Current_Item == CS_APP_RECALL_1)
						if ( MV_Count_Recall_Size() < CS_APP_RECALL_MAX-1 )
							Current_Item = MV_Count_Recall_Size() - 1;
						else
							Current_Item = CS_APP_RECALL_MAX-1;
					else
						Current_Item--;
					
					MV_Draw_Recall_MenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_DOWN:					
					hdc=BeginPaint(hwnd);
					MV_Draw_Recall_MenuBar(hdc, UNFOCUS, Current_Item);

					if ( MV_Count_Recall_Size() < CS_APP_RECALL_MAX - 1)
					{
						if(Current_Item == MV_Count_Recall_Size() - 1)
							Current_Item = CS_APP_RECALL_1;
						else
							Current_Item++;
					} else {
						if(Current_Item == CS_APP_RECALL_MAX - 1)
							Current_Item = CS_APP_RECALL_1;
						else
							Current_Item++;
					}
					
					MV_Draw_Recall_MenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_LEFT:
					break;
					
				case CSAPP_KEY_RIGHT:
					break;
				
				case CSAPP_KEY_SWAP:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					CSApp_Recall_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				default:
					break;
			}
			break;
			
		case MSG_CLOSE:
			CS_APP_SetFirstInDesktop(FALSE);
			PostQuitMessage(hwnd);
			DestroyMainWindow(hwnd);
			break;
			
		default:
			break;		
	}
	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

