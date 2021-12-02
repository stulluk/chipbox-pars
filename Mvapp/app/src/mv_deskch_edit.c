#include "linuxos.h"

#include "database.h"
#include "av_zapping.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "ui_common.h"
#include "casdrv_def.h"
#include "caskey.h"
#include "casapi.h"

#define BISS_KEY

#define	CH_EDIT_DY	( SM_WINDOW_DY + (SM_WINDOW_ITEM_DY*6) )

enum
{
	CS_APP_DESKCH_EDIT_FAV = 0,
	CS_APP_DESKCH_EDIT_LOCK,
	CS_APP_DESKCH_EDIT_VOLUME,
	CS_APP_DESKCH_EDIT_VPID,
	CS_APP_DESKCH_EDIT_APID,
	CS_APP_DESKCH_EDIT_PPID,
	CS_APP_DESKCH_EDIT_SID,
	CS_APP_DESKCH_EDIT_SETTING_MAX
};

static CSAPP_Applet_t			CSApp_DeskCh_Edit_Applets;

static const U16 				DeskCh_Edit_item[CS_APP_DESKCH_EDIT_SETTING_MAX] = {
								CSAPP_STR_FAV_KEY,
								CSAPP_STR_LOCK_KEY,
								CSAPP_STR_VOLUME,
								CSAPP_STR_VPID,
								CSAPP_STR_APID,
								CSAPP_STR_PPID,
								CSAPP_STR_SID
							};

U8	DeskCh_Edit_Arrow_Kind[CS_APP_DESKCH_EDIT_SETTING_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_NUMERIC
};

static U16					Current_Item=0;
static MV_stServiceInfo 	pServiceData;

static U8					u8Favorite = 8;
static U16					u16Video_pid;
static U16					u16Audio_pid;
static U16					u16PCR_pid;
static U16					u16Service_id;
static U8					u8Sound_Offset;
static U8					u8Lock;
static BOOL					bCheck_Favorite[MV_MAX_FAV_KIND];

static U8					u8Favorite_back = 8;
static U16					u16Video_pid_back;
static U16					u16Audio_pid_back;
static U16					u16PCR_pid_back;
static U16					u16Service_id_back;
static U8					u8Sound_Offset_back;
static U8					u8Lock_back;
static BOOL					bCheck_Favorite_back[MV_MAX_FAV_KIND];

static char					sReturn_str[MAX_PID_LENGTH+1];


#ifdef BISS_KEY
static CasBissInfo_t 		bissInfo;
static U8					BissKey[16];
static U8					Send_BissKey[32];
static U8					BissLen = 0;
static MV_stTPInfo 			MV_TPInfo;

extern BOOL CasDrvGetBissKey (CasBissInfo_t bissInfo, U8 *key, U8 *keyLength);
#endif

extern void EditSList_Draw_Saving(HDC hdc);
extern void EditSList_Close_Confirm(HDC hdc);
static int DeskCh_Edit_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
static void MV_DeskCH_Edit_SetWindow(HDC hdc);
static void MV_Draw_DeskCH_Edit_SelectBar(HDC hdc, int y_gap, U8 esItem);
static void MV_Draw_DeskCh_Edit_MenuBar(HDC hdc, U8 u8Focuskind, U8 esItem);
static void MV_Draw_DeskCh_Edit_Bar(HDC hdc);

void MV_Set_KeyData_Reform(U8 *acSend_BissKey, U8 *acBissKey, U8 u8Len)
{
	int				i = 0;

	for ( i = 0 ; i < u8Len ; i++ )
		sprintf(&acSend_BissKey[i*2], "%02x", acBissKey[i]);
}

static void MV_DeskCh_Edit_SetVol_With_Offset(void)
{
	U8 u8DB_Volume = CS_AV_AudioGetVolume();
	
	if ( u8DB_Volume != 0 )
	{	
		if ( u8Sound_Offset > 32 )
		{
			if ( u8DB_Volume + ( u8Sound_Offset - 32 ) > 50 )
				CS_AV_AudioSetVolume(kCS_DBU_MAX_VOLUME);
			else
				CS_AV_AudioSetVolume(u8DB_Volume + ( u8Sound_Offset - 32 ) );
		}
		else if ( u8Sound_Offset < 32 )
		{
			if ( u8DB_Volume - ( 32 - u8Sound_Offset ) < 0 )
				CS_AV_AudioSetVolume(0);
			else
				CS_AV_AudioSetVolume(u8DB_Volume - ( 32 - u8Sound_Offset ));
		} 
		else 
			CS_AV_AudioSetVolume(u8DB_Volume);
	} else 
		CS_AV_AudioSetVolume(u8DB_Volume);
	
}

static void MV_DeskCH_Edit_SetWindow(HDC hdc)
{
	RECT	rc1;
	
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_TOP_RIGHT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + CH_EDIT_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_LEFT]);
	FillBoxWithBitmap (hdc, ScalerWidthPixel(SM_WINDOW_X + SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_RIGHT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y + CH_EDIT_DY - MV_BMP[MVBMP_BOARD_BOT_LEFT].bmHeight), ScalerWidthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_BOARD_BOT_RIGHT].bmHeight), &MV_BMP[MVBMP_BOARD_BOT_RIGHT]);

	SetBrushColor(hdc, MVAPP_BACKBLUE_COLOR);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth), ScalerHeigthPixel(SM_WINDOW_Y),ScalerWidthPixel(SM_WINDOW_DX - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmWidth * 2),ScalerHeigthPixel(CH_EDIT_DY));
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_X), ScalerHeigthPixel(SM_WINDOW_Y + MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight),ScalerWidthPixel(SM_WINDOW_DX),ScalerHeigthPixel(CH_EDIT_DY - MV_BMP[MVBMP_BOARD_TOP_LEFT].bmHeight * 2));	

	rc1.top = SM_WINDOW_TITLE_Y;
	rc1.left = SM_WINDOW_ITEM_X;
	rc1.bottom = SM_WINDOW_TITLE_Y + SM_WINDOW_ITEM_DY;
	rc1.right = rc1.left + SM_WINDOW_CONT_DX;
	
	MV_Draw_PopUp_Title_Bar_ByName(hdc, &rc1, CSAPP_STR_CH_CHANGE);
	
	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	FillBox(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_CONT_Y),ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY * 2));

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X), ScalerHeigthPixel(SM_WINDOW_Y + CH_EDIT_DY - 30), ScalerWidthPixel(MV_BMP[MVBMP_RED_BUTTON].bmWidth), ScalerHeigthPixel(MV_BMP[MVBMP_RED_BUTTON].bmHeight), &MV_BMP[MVBMP_RED_BUTTON]);
	CS_MW_TextOut(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + MV_BMP[MVBMP_RED_BUTTON].bmWidth + 10),	ScalerHeigthPixel(SM_WINDOW_Y + CH_EDIT_DY - 30),	CS_MW_LoadStringByIdx(CSAPP_STR_BISS));

	MV_Draw_DeskCh_Edit_Bar(hdc);
}

static void MV_Draw_DeskCH_Edit_SelectBar(HDC hdc, int y_gap, U8 esItem)
{
	int mid_width = SM_WINDOW_CONT_DX - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = SM_WINDOW_ITEM_X + MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	if ( DeskCh_Edit_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_CONT_DX - ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth) - ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth) ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

static void MV_Draw_DeskCh_Edit_MenuBar(HDC hdc, U8 u8Focuskind, U8 esItem)
{
	int 	y_gap = SM_WINDOW_CONT_Y + SM_WINDOW_ITEM_DY * esItem;
	RECT	TmpRect;

	if ( u8Focuskind == FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_Draw_DeskCH_Edit_SelectBar(hdc, y_gap, esItem);
	} else {
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);		
		MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
		MV_FillBox( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(SM_WINDOW_CONT_DX),ScalerHeigthPixel(SM_WINDOW_ITEM_DY) );
	}					

	CS_MW_TextOut( hdc, ScalerWidthPixel(SM_WINDOW_ITEM_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(DeskCh_Edit_item[esItem]));

	TmpRect.left	=ScalerWidthPixel(SM_WINDOW_ITEM_X + SM_WINDOW_ITEM_NAME_X);
	TmpRect.right	=TmpRect.left + SM_WINDOW_ITEM_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(SM_WINDOW_ITEM_DY);

	switch(esItem)
	{
		char	temp_str[20];
		
		case CS_APP_DESKCH_EDIT_FAV:
			MV_DB_Get_Favorite_Name(temp_str, u8Favorite);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
			
		case CS_APP_DESKCH_EDIT_LOCK:
			
			if ( u8Lock == 1 )
				sprintf(temp_str, CS_MW_LoadStringByIdx(CSAPP_STR_LOCKED));
			else
				sprintf(temp_str, CS_MW_LoadStringByIdx(CSAPP_STR_PROGRAM_UNLOCK));
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case CS_APP_DESKCH_EDIT_VOLUME:
			if ( u8Sound_Offset == 32 )
				sprintf(temp_str, "0");
			else if ( u8Sound_Offset < 32 )
				sprintf(temp_str, "-%d", 32 - u8Sound_Offset);
			else
				sprintf(temp_str, "+%d", u8Sound_Offset - 32);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case CS_APP_DESKCH_EDIT_VPID:
			sprintf(temp_str, "%d", u16Video_pid);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case CS_APP_DESKCH_EDIT_APID:
			sprintf(temp_str, "%d", u16Audio_pid);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case CS_APP_DESKCH_EDIT_PPID:
			sprintf(temp_str, "%d", u16PCR_pid);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;

		case CS_APP_DESKCH_EDIT_SID:
			sprintf(temp_str, "%d", u16Service_id);
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
			
		default:
			break;
	}
}

static void MV_Draw_DeskCh_Edit_Bar(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < CS_APP_DESKCH_EDIT_SETTING_MAX ; i++ )
	{
		if( Current_Item == i )
		{
			MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, i);
		} else {
			MV_Draw_DeskCh_Edit_MenuBar(hdc, UNFOCUS, i);
		}
	}
}

void MV_DeskCh_Set_Current_List(tCS_DB_ServiceListType chlist_type)
{
	tCS_DB_ServiceListTriplet	DeskCh_triplet;
	U16							DeskCh_Total_Service = 0;
	U16							u16Service_Index;
	
	DeskCh_triplet.sCS_DB_ServiceListType = chlist_type;
	DeskCh_triplet.sCS_DB_ServiceListTypeValue = 0;
	
	DeskCh_Total_Service = CS_DB_GetListServiceNumber(DeskCh_triplet);
	u16Service_Index = MV_DB_Get_ServiceAllList_Index(chlist_type, pServiceData.u16ChIndex);
	
	printf("=== TOTAL : %d , chlist_servicelist_type : %d \n", DeskCh_Total_Service, chlist_type);

	if( DeskCh_Total_Service > 0 )
	{
		CS_DB_SetCurrentList(DeskCh_triplet, FALSE);
		CS_DB_SetCurrentService_OrderIndex(u16Service_Index);
	}
}

CSAPP_Applet_t	CSApp_DeskCh_Edit(void)
{
	int   				BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   				msg;
	HWND  				hwndMain;
	MAINWINCREATE		CreateInfo;

	CSApp_DeskCh_Edit_Applets = CSApp_Applet_Error;

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
	CreateInfo.spCaption = "Desk Chedit window";
	CreateInfo.hMenu	 = 0;
	CreateInfo.hCursor	 = 0;
	CreateInfo.hIcon	 = 0;
	CreateInfo.MainWindowProc = DeskCh_Edit_Msg_cb;
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
	return CSApp_DeskCh_Edit_Applets;
    
}

static int DeskCh_Edit_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam)
{
	HDC 				hdc;
	int					i;
	U8					u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	char				acKey_Value[256];

	switch(message)
	{
		case MSG_CREATE:			
			MV_DB_GetServiceDataByIndex(&pServiceData, CS_DB_GetCurrentServiceIndex());

			/* By KB Kim 2011.01.20 */
			if ( pServiceData.u8TvRadio == eCS_DB_RADIO_SERVICE )
				u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
			else
				u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
			
			for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
			{
				if ( MV_DB_CheckFavoriteServiceBySrvIndex(u8TVRadio, pServiceData.u16ChIndex , i) == MV_MAX_FAV_COUNT )
					bCheck_Favorite_back[i] = bCheck_Favorite[i] = FALSE;
				else
					bCheck_Favorite_back[i] = bCheck_Favorite[i] = TRUE;
			}
			
			u8Favorite_back = u8Favorite = MV_DB_FindFavoriteServiceBySrvIndex( u8TVRadio, pServiceData.u16ChIndex);

			if ( u8Favorite == MV_MAX_FAV_KIND )
				u8Favorite_back = u8Favorite = 0;

			if ( pServiceData.u8AudioVolume == 0 )
			{
				
				u8Sound_Offset_back = pServiceData.u8AudioVolume;
				u8Sound_Offset = 32;
			}
			else
				u8Sound_Offset_back = u8Sound_Offset = pServiceData.u8AudioVolume;
				
			u16Video_pid_back = u16Video_pid = pServiceData.u16VideoPid;
			u16Audio_pid_back = u16Audio_pid = pServiceData.u16AudioPid;
			u16PCR_pid_back = u16PCR_pid = pServiceData.u16PCRPid;
			u16Service_id_back = u16Service_id = pServiceData.u16ServiceId;
			u8Lock_back = u8Lock = pServiceData.u8Lock;
			Current_Item = 0;
			break;
		case MSG_PAINT:
			{
				hdc=BeginPaint(hwnd);
				MV_DeskCH_Edit_SetWindow(hdc);
				EndPaint(hwnd,hdc);
			}
			return 0;

		case MSG_KEYDOWN:
			if ( MV_Get_HexaKeypad_Status() == TRUE )
			{
				MV_HexaKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ENTER )
				{
					char	Splite_Str[4];
					U32		Temp_U32;
					U8		Temp_U8;
					int		i = 0, k = 0, j = 0;
					U8		TempU8;
					U8		Send_Temp_Key[16];
					
					strcpy(acKey_Value, MV_Get_HexaEdited_String());

					k = strlen(acKey_Value);
					
					for ( i = 0 ; i < k ; i++ )
					{
						Splite_Str[i%2] = acKey_Value[i];
						if ( i%2 == 1 && i != 0 )
						{
							Str2Hex(Splite_Str, &TempU8);	
							//printf("i : %02d , j : %d ===== %s = %02x\n", i, j, Splite_Str, TempU8);
							Send_Temp_Key[j] = TempU8;
							j++;
						}					
					}

					CasDrvUpdateBissKey (bissInfo, &Temp_U32, &Temp_U8, Send_Temp_Key, j);
					CasDrvSaveKey2Flase();

					printf("== %02x%02x%02x%02x%02x%02x%02x%02x \n", Send_Temp_Key[0], Send_Temp_Key[1], Send_Temp_Key[2], Send_Temp_Key[3], Send_Temp_Key[4], Send_Temp_Key[5], Send_Temp_Key[6], Send_Temp_Key[7]);
				}
				
				break;
			}
			else if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8	u8Result_Value;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Current_Item)
					{
						case CS_APP_DESKCH_EDIT_FAV:
							u8Favorite = u8Result_Value;
							if ( bCheck_Favorite[u8Favorite] == TRUE )
								bCheck_Favorite[u8Favorite] = FALSE;
							else 
								bCheck_Favorite[u8Favorite] = TRUE;
							break;
							
						default:
							break;
					}
	
					hdc=BeginPaint(hwnd);
					MV_Draw_DeskCh_Edit_MenuBar(hdc, MV_FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
				}
				else if ( wparam == CSAPP_KEY_TV_AV )
				{
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}
				break;
			}

			if ( Get_NumKeypad_Status() == TRUE )
			{
				UI_NumKeypad_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_BLUE )
				{
					if ( wparam == CSAPP_KEY_BLUE )
					{
						if ( Get_Keypad_is_Save() == TRUE )
						{
							Get_Save_Str(sReturn_str);
							//printf("\n2 ========= %d ============\n", atoi(sReturn_str));

							switch(Current_Item)
							{
								case CS_APP_DESKCH_EDIT_VPID:
									u16Video_pid = atoi(sReturn_str);
									break;

								case CS_APP_DESKCH_EDIT_APID:
									u16Audio_pid = atoi(sReturn_str);
									break;

								case CS_APP_DESKCH_EDIT_PPID:
									u16PCR_pid = atoi(sReturn_str);
									break;

								case CS_APP_DESKCH_EDIT_SID:
									u16Service_id = atoi(sReturn_str);
									break;
									
								default:
									break;
							}
							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
						}
					} 
				}
				
				break;
			}

			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							pServiceData.u8AudioVolume = u8Sound_Offset;
							pServiceData.u16VideoPid = u16Video_pid;
							pServiceData.u16AudioPid = u16Audio_pid;
							pServiceData.u16PCRPid = u16PCR_pid;
							pServiceData.u16ServiceId = u16Service_id;
							pServiceData.u8Lock = u8Lock;

							for ( i = 0 ; i < MV_MAX_FAV_KIND + 1 ; i++ )
							{
								if ( bCheck_Favorite[i] != bCheck_Favorite_back[i] )
								{
									if ( bCheck_Favorite_back[i] == FALSE && bCheck_Favorite[i] == TRUE )
										MV_DB_AddFavoriteService(i, u8TVRadio, pServiceData.u16ChIndex);
									else if (  bCheck_Favorite_back[i] == TRUE && bCheck_Favorite[i] == FALSE )
										CS_DB_RemoveFavoriteService(pServiceData.u16ChIndex, u8TVRadio, i);
								}
							}

							MV_SetServiceData_By_Chindex(&pServiceData, pServiceData.u16ChIndex);
						}
						
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);

						CSApp_DeskCh_Edit_Applets = CSApp_Applet_Desktop;
						SendMessage(hwnd,MSG_CLOSE,0,0);
					} else {
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
					}
				}
				
				if (wparam != CSAPP_KEY_IDLE)
				{
					break;
				}
			}
			
			switch(wparam)
			{
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
							case CS_APP_DESKCH_EDIT_FAV:
								{
									char 	Temp_Str[100];
									
									for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
									{
										MV_DB_Get_Favorite_Name(Temp_Str, i);

										if ( bCheck_Favorite[i] == TRUE && bCheck_Favorite_back[i] == TRUE )
											sprintf(stContents.Contents[i], "* %s", Temp_Str);
										else if ( bCheck_Favorite[i] == TRUE && bCheck_Favorite_back[i] == FALSE )
											sprintf(stContents.Contents[i], "> %s", Temp_Str);
										else if ( bCheck_Favorite[i] == FALSE && bCheck_Favorite_back[i] == TRUE )
											sprintf(stContents.Contents[i], "< %s", Temp_Str);
										else
											sprintf(stContents.Contents[i], "  %s", Temp_Str);
									}
							
									smwRect.bottom = smwRect.top + SM_WINDOW_ITEM_DY * MV_MAX_FAV_KIND + 1;
									smwRect.right = smwRect.right + SM_WINDOW_ITEM_NAME_X;
									stContents.u8TotalCount = MV_MAX_FAV_KIND;
									stContents.u8Focus_Position = u8Favorite;
									MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
								}
								break;
								
							case CS_APP_DESKCH_EDIT_VOLUME:
								break;

							case CS_APP_DESKCH_EDIT_VPID:
								MV_Draw_NumKeypad(hwnd, u16Video_pid, 0, MAX_PID_LENGTH);
								break;

							case CS_APP_DESKCH_EDIT_APID:
								MV_Draw_NumKeypad(hwnd, u16Audio_pid, 0, MAX_PID_LENGTH);
								break;

							case CS_APP_DESKCH_EDIT_PPID:
								MV_Draw_NumKeypad(hwnd, u16PCR_pid, 0, MAX_PID_LENGTH);
								break;

							case CS_APP_DESKCH_EDIT_SID:
								MV_Draw_NumKeypad(hwnd, u16Service_id, 0, MAX_PID_LENGTH);
								break;
							
							default:
								break;
						}
					}
					break;
				case CSAPP_KEY_IDLE:
    				CSApp_DeskCh_Edit_Applets = CSApp_Applet_Sleep;
    				SendMessage(hwnd,MSG_CLOSE,0,0);
    				break;
					
				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_UP:
					hdc=BeginPaint(hwnd);
					MV_Draw_DeskCh_Edit_MenuBar(hdc, UNFOCUS, Current_Item);
					
					if(Current_Item == CS_APP_DESKCH_EDIT_FAV)
						Current_Item = CS_APP_DESKCH_EDIT_SETTING_MAX-1;
					else
						Current_Item--;
					
					MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_DOWN:					
					hdc=BeginPaint(hwnd);
					MV_Draw_DeskCh_Edit_MenuBar(hdc, UNFOCUS, Current_Item);

					if(Current_Item == CS_APP_DESKCH_EDIT_SETTING_MAX - 1)
						Current_Item = CS_APP_DESKCH_EDIT_FAV;
					else
						Current_Item++;
					
					MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
					EndPaint(hwnd,hdc);
					break;
					
				case CSAPP_KEY_LEFT:
					switch(Current_Item)
					{	
						case CS_APP_DESKCH_EDIT_FAV:
							if ( u8Favorite == 0 )
								u8Favorite = MV_MAX_FAV_KIND;
							else
								u8Favorite--;

							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
							
						case CS_APP_DESKCH_EDIT_LOCK:
							if ( u8Lock == 0 )
								u8Lock = 1;
							else
								u8Lock = 0;

							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;

						case CS_APP_DESKCH_EDIT_VOLUME:
							if ( u8Sound_Offset > 0 )
								u8Sound_Offset--;

							MV_DeskCh_Edit_SetVol_With_Offset();
							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
							
						default:
							break;
					}
					break;
					
				case CSAPP_KEY_RIGHT:
					switch(Current_Item)
					{	
						case CS_APP_DESKCH_EDIT_FAV:
							if ( u8Favorite == MV_MAX_FAV_KIND )
								u8Favorite = 0;
							else
								u8Favorite++;

							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
							
						case CS_APP_DESKCH_EDIT_LOCK:
							if ( u8Lock == 0 )
								u8Lock = 1;
							else
								u8Lock = 0;

							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;

						case CS_APP_DESKCH_EDIT_VOLUME:
							if ( u8Sound_Offset < 64 )
								u8Sound_Offset++;

							MV_DeskCh_Edit_SetVol_With_Offset();
							hdc=BeginPaint(hwnd);
							MV_Draw_DeskCh_Edit_MenuBar(hdc, FOCUS, Current_Item);
							EndPaint(hwnd,hdc);
							break;
							
						default:
							break;
					}
					break;

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
					{
						U8	Input_Key;

						switch(wparam)
						{
							case CSAPP_KEY_0:
								Input_Key = 0;
								break;
							case CSAPP_KEY_1:
								Input_Key = 1;
								break;
							case CSAPP_KEY_2:
								Input_Key = 2;
								break;
							case CSAPP_KEY_3:
								Input_Key = 3;
								break;
							case CSAPP_KEY_4:
								Input_Key = 4;
								break;
							case CSAPP_KEY_5:
								Input_Key = 5;
								break;
							case CSAPP_KEY_6:
								Input_Key = 6;
								break;
							case CSAPP_KEY_7:
								Input_Key = 7;
								break;
							case CSAPP_KEY_8:
								Input_Key = 8;
								break;
							case CSAPP_KEY_9:
								Input_Key = 9;
								break;
							default:
								Input_Key = 0;
								break;
						}
						
						MV_Draw_NumKeypad(hwnd, Input_Key, Input_Key, MAX_PID_LENGTH);
					}
					break;
#ifdef BISS_KEY
				case CSAPP_KEY_RED:
					MV_DB_Get_TPdata_By_ChNum(&MV_TPInfo, pServiceData.u16ChIndex);

					// printf("%d : %s , %d / %d / %d=======\n", pServiceData.u16ChIndex, pServiceData.acServiceName, MV_TPInfo.u16SymbolRate, MV_TPInfo.u8Polar_H, MV_TPInfo.u16TPFrequency);
					
					memset(&bissInfo, 0x00, sizeof(CasBissInfo_t));
					bissInfo.Symbol = MV_TPInfo.u16SymbolRate;
					bissInfo.TpFreq = MV_TPInfo.u16TPFrequency;
					bissInfo.Pol = MV_TPInfo.u8Polar_H;
					bissInfo.ServiceId = pServiceData.u16ServiceId;
					bissInfo.VideoPid = pServiceData.u16VideoPid;
					strncpy( bissInfo.ChannelName , pServiceData.acServiceName, MAX_SERVICE_NAME_LENGTH );
					memset(BissKey, 0x00, sizeof(BissKey));
					BissLen = 0;
					CasDrvGetBissKey (bissInfo, BissKey, &BissLen);
					memset(Send_BissKey, 0x00, sizeof(Send_BissKey));
					MV_Set_KeyData_Reform(Send_BissKey, BissKey, BissLen);
/*
					if ( BissLen != 0 )
						MV_Draw_HexaKeypad(hwnd, Send_BissKey, BissLen*2);
					else*/
						MV_Draw_HexaKeypad(hwnd, Send_BissKey, 32);
					break;
#endif
				case CSAPP_KEY_F2:
				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					{
						int 	i;
						
						if ( u16Video_pid_back != u16Video_pid || u16Audio_pid_back != u16Audio_pid || u16PCR_pid_back != u16PCR_pid || u16Service_id_back != u16Service_id )
						{
							MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SURE);
							break;
						} else {
							for ( i = 0 ; i < MV_MAX_FAV_KIND + 1 ; i++ )
							{
								if ( bCheck_Favorite[i] != bCheck_Favorite_back[i] )
								{
									//printf("============= %d FAVORITE SETTING %d ===============\n", i, pServiceData.u16ChIndex);
									if ( bCheck_Favorite_back[i] == FALSE && bCheck_Favorite[i] == TRUE )
										MV_DB_AddFavoriteService(i, u8TVRadio, pServiceData.u16ChIndex);
									else if (  bCheck_Favorite_back[i] == TRUE && bCheck_Favorite[i] == FALSE )
									{
										if ( MV_MAX_FAV_COUNT != MV_DB_CheckFavoriteServiceBySrvIndex( u8TVRadio, pServiceData.u16ChIndex, i) )
										{
											CS_DB_RemoveFavoriteService(pServiceData.u16ChIndex, u8TVRadio, i);

											if ( pServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE || pServiceData.u8TvRadio == eCS_DB_TV_SERVICE )
												MV_DeskCh_Set_Current_List(eCS_DB_TV_LIST);
											else
												MV_DeskCh_Set_Current_List(eCS_DB_RADIO_LIST);
										}
										else
											CS_DB_RemoveFavoriteService(pServiceData.u16ChIndex, u8TVRadio, i);
									}
								}
							}
							
							pServiceData.u8AudioVolume = u8Sound_Offset;
							pServiceData.u8Lock = u8Lock;
							MV_SetServiceData_By_Chindex(&pServiceData, pServiceData.u16ChIndex);
						}
					}
					CSApp_DeskCh_Edit_Applets = CSApp_Applet_Desktop;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
					
				default:
					break;
			}
			break;
			
		case MSG_CLOSE:
			CS_APP_SetFirstInDesktop(FALSE);
			if(CS_DB_CheckIfChanged() || memcmp(&bCheck_Favorite, &bCheck_Favorite_back, sizeof(BOOL)*MV_MAX_FAV_KIND) != 0 )
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

