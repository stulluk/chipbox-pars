
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
#include "dvbtuner.h"
#include "mv_motor.h"

#define MAX_POSITION 64

U16 D_Usals_Str[USALS_ITEM_MAX+1] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TPSELECT,
	CSAPP_STR_SAT_LONGITUDE,
	CSAPP_STR_LOCAL_LONGITUDE,
	CSAPP_STR_LOCAL_LATITUDE,
	CSAPP_STR_LIMIT_SET,
	CSAPP_STR_WEST_LIMIT,
	CSAPP_STR_EAST_LIMIT,
	CSAPP_STR_GOTO_REF
};

U8	Usals_Arrow_Kind[USALS_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

U8	Usals_Enter_Kind[USALS_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_NUMERIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

static CSAPP_Applet_t		CSApp_Usals_Applets;
static eUsalsItemID			Usals_Focus_Item = USALS_ITEM_SATELLITE;
static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static U8					u8TpCount;
static U32					Temp_Limit_Use = FALSE;
static MV_stTPInfo 			MV_TPInfo;
static BOOL					Sat_List_Status;
static U8					sTemp_Sat_Long[5];
static U8					sTemp_Usals_Long[5];
static U8					sTemp_Usals_Lati[5];
static U8					Input_Status = 0;
static U8					u8Temp_Sat_index;

/* By KB Kim 2011.05.21 */
static U8                   u8DataChanged;
static U32                  u32LongitudeBackup;
static U32                  u32LatitudeBackup;

/* For Motor Control By KB Kim 2011.05.22 */
extern U32					*Tuner_HandleId;

int Usals_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

void MV_Usals_Draw_Value(HDC hdc, RECT *lrect, U8 u8Focuskind, U8 u8DrawItem)
{
	RECT		acRect;
	char		acTemp_Time[20];
	char		acTemp_Str[20];
	int			i;

	SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
	SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	SetBkMode(hdc,BM_TRANSPARENT);
	
	acRect.top = lrect->top;
	acRect.left = lrect->left + (( lrect->right - lrect->left )/2 - 50) - 10;
	acRect.bottom = lrect->bottom;
	acRect.right = acRect.left + 25;

	memset( acTemp_Time, 0x00, 20 );
	memset( acTemp_Str, 0x00, 20 );

	if ( u8DrawItem == USALS_ITEM_LONGITUDE )
		sprintf(acTemp_Time, "%s", sTemp_Usals_Long);
	else if ( u8DrawItem == USALS_ITEM_SAT_LONGITUDE )
		sprintf(acTemp_Time, "%s", sTemp_Sat_Long);
	else if ( u8DrawItem == USALS_ITEM_LATITUDE )
	{
		if (sTemp_Usals_Lati[0] == '0')
		{
			sprintf(acTemp_Time, "N%s", sTemp_Usals_Lati + 1);
		}
		else
		{
			sprintf(acTemp_Time, "S%s", sTemp_Usals_Lati + 1);
		}
	}
	
//	FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));
	if ( u8Focuskind == FOCUS )
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
	else
		SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(acRect.left + ( 3 * 25)),ScalerHeigthPixel(acRect.top), ".");

	acRect.left -= 25;	
	acRect.right = acRect.left + 25;

	for ( i = 0 ; i < 4 ; i++ )
	{
		if ( i == 3 )
			acRect.left += 35;
		else
			acRect.left += 25;
		
		acRect.right = acRect.left + 25;
		
		sprintf(acTemp_Str, "%c", acTemp_Time[i]);
		//printf("\n=== %d , %d , %d , %d -- %d : %s ==== \n", acRect.top, acRect.bottom, acRect.left, acRect.right, u8DrawItem, acTemp_Str);
		if ( i == Input_Status && u8Focuskind == FOCUS )
		{
			SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));	
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
		else
		{
			//SetBrushColor(hdc, MVAPP_BLACK_COLOR_ALPHA);
			if ( u8Focuskind == FOCUS )
				SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
			else
				SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			
			SetBkMode(hdc,BM_TRANSPARENT);
			//FillBox(hdc,ScalerWidthPixel(acRect.left), ScalerHeigthPixel(acRect.top), ScalerWidthPixel(acRect.right - acRect.left),ScalerHeigthPixel(acRect.bottom - acRect.top));	
			CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
		}
	}
//	CS_MW_DrawText(hdc, acTemp_Str, -1, &acRect, DT_CENTER | DT_VCENTER );
}

/* By KB Kim for value check */
BOOL MV_Usals_Update_Value(WPARAM wparam, eUsalsItemID Current_Index)
{
	char	Temp;
	char    backupChar;
	
	switch(wparam)
	{
		case CSAPP_KEY_0:
			Temp = '0';
			break;

		case CSAPP_KEY_1:
			Temp = '1';
			break;

		case CSAPP_KEY_2:
			Temp = '2';
			break;

		case CSAPP_KEY_3:
			Temp = '3';
			break;

		case CSAPP_KEY_4:
			Temp = '4';
			break;

		case CSAPP_KEY_5:
			Temp = '5';
			break;

		case CSAPP_KEY_6:
			Temp = '6';
			break;

		case CSAPP_KEY_7:
			Temp = '7';
			break;

		case CSAPP_KEY_8:
			Temp = '8';
			break;

		case CSAPP_KEY_9:
			Temp = '9';
			break;

		default:
			Temp = '0';
			break;
	}

	switch (Current_Index)
	{
		case USALS_ITEM_SAT_LONGITUDE:
			backupChar = sTemp_Sat_Long[Input_Status];
			sTemp_Sat_Long[Input_Status] = Temp;
			if (atoi(sTemp_Sat_Long) > 3600)
			{
				sTemp_Sat_Long[Input_Status] = backupChar;
				return TRUE;
			}
			break;
			
		case USALS_ITEM_LONGITUDE:
			backupChar = sTemp_Usals_Long[Input_Status];
			sTemp_Usals_Long[Input_Status] = Temp;
			if (atoi(sTemp_Usals_Long) > 3600)
			{
				sTemp_Usals_Long[Input_Status] = backupChar;
				return TRUE;
			}
			break;
			
		case USALS_ITEM_LATITUDE:
			backupChar = sTemp_Usals_Lati[Input_Status];
			sTemp_Usals_Lati[Input_Status] = Temp;
			if (atoi(sTemp_Usals_Lati + 1) > 900)
			{
				sTemp_Usals_Lati[Input_Status] = backupChar;
				return TRUE;
			}
			break;
			
		default:
			break;
	}

	return FALSE;
	
	//acTemp_duration[u8Timer_InCnt] = Temp;
}

void MV_Draw_UsalsSelectBar(HDC hdc, int y_gap, eScanItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Usals_Enter_Kind[esItem])
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
	
	if ( Usals_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_UsalsMenuBar(HDC hdc, U8 u8Focuskind, eUsalsItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	temp_str[30];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_UsalsSelectBar(hdc, y_gap, esItem);
	} else {
		if ( Temp_Limit_Use == FALSE && ( esItem == USALS_ITEM_WEST_LIMIT || esItem == USALS_ITEM_EAST_LIMIT ))
		{
			SetTextColor(hdc,MVAPP_GRAY_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );
		} else {
			SetTextColor(hdc,MV_BAR_UNFOCUS_CHAR_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_SetBrushColor( hdc, MV_BAR_UNFOCUS_COLOR );
			MV_FillBox( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(ScreenWidth - MV_INSTALL_MENU_X*2),ScalerHeigthPixel(MV_INSTALL_MENU_BAR_H) );					
		}
	}

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(D_Usals_Str[esItem]));

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	memset ( temp_str, 0, 30 );

	switch(esItem)
	{
		
		case USALS_ITEM_SATELLITE:
			CS_MW_DrawText(hdc, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, -1, &TmpRect, DT_CENTER);
			break;
		case USALS_ITEM_TP:
			if ( u8TpCount > 0 )
			{
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&MV_TPInfo, u8Glob_Sat_Focus, u8Glob_TP_Focus);
				
				if ( MV_TPInfo.u8Polar_H == 1 )
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount + 1, MV_TPInfo.u16TPFrequency, "H", MV_TPInfo.u16SymbolRate);
				else
					sprintf(temp_str, "%d/%d. %d/%s/%d", u8Glob_TP_Focus + 1, u8TpCount + 1, MV_TPInfo.u16TPFrequency, "V", MV_TPInfo.u16SymbolRate);
			} else {
				sprintf(temp_str, "0/0. 0/H/0");
			}

			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);	
			break;
		case USALS_ITEM_SAT_LONGITUDE:
			// MV_Usals_Draw_Value(hdc, &TmpRect, u8Focuskind, esItem);
		case USALS_ITEM_LONGITUDE:
			// MV_Usals_Draw_Value(hdc, &TmpRect, u8Focuskind, esItem);
			// break;
		case USALS_ITEM_LATITUDE:
			MV_Usals_Draw_Value(hdc, &TmpRect, u8Focuskind, esItem);
			break;
		case USALS_ITEM_LIMIT_SET:
			if ( Temp_Limit_Use == TRUE )
				sprintf(temp_str, "%s", "USE");
			else
				sprintf(temp_str, "%s", "NOT USE");
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case USALS_ITEM_WEST_LIMIT:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case USALS_ITEM_EAST_LIMIT:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case USALS_ITEM_REF:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case USALS_ITEM_MAX:
		default:
			break;
	}
}

void MV_Draw_UsalsMenuFull(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < USALS_ITEM_MAX ; i++ )
	{
		if( Usals_Focus_Item == i )
		{
			MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_Usals_Motor(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_Usals_Applets = CSApp_Applet_Error;

	Usals_Focus_Item = USALS_ITEM_SATELLITE; 
	
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
	CreateInfo.spCaption 	= "USALS Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Usals_Msg_cb;
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
	
	return CSApp_Usals_Applets;
}

int Usals_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	
	switch(message)
	{
		case MSG_CREATE:
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);

			Usals_Focus_Item = USALS_ITEM_SATELLITE;
			SearchData.ScanMode = CS_DBU_GetAntenna_Type();
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);
			memset (&sTemp_Sat_Long, 0x00, 5);
			memset (&sTemp_Usals_Long, 0x00, 5);
			memset (&sTemp_Usals_Lati, 0x00, 5);
			Input_Status = 0;

			if ( MV_GetSelected_SatData_Count() > 0 )
				MV_GetSatelliteData(MV_Sat_Data);
			else
			{
				CSApp_Usals_Applets=CSApp_Applet_Install;
				SendMessage (hwnd, MSG_CLOSE, 0, 0);
			}

			/* By KB Kim 2011.05.21 */
			u8DataChanged = 0;
			u32LongitudeBackup = CS_DBU_GetLocal_Longitude();
			u32LatitudeBackup  = CS_DBU_GetLocal_Latitude();
			
			sprintf(sTemp_Sat_Long, "%04d", MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude);
			sprintf(sTemp_Usals_Long, "%04d", u32LongitudeBackup);
			sprintf(sTemp_Usals_Lati, "%04d", u32LatitudeBackup);
			
			u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus) - 1;

			Temp_Limit_Use = CS_DBU_Get_Motor_Limit();

			/* For Motor Control By KB Kim 2011.05.22 */
			// DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
			MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
			break;
		case MSG_CLOSE:
			KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}

			/* By KB Kim 2011.05.28 */
			if (CSApp_Usals_Applets != CSApp_Applet_Install)
			{
				CS_AV_VideoBlank();
			}
			
			PostQuitMessage (hwnd);
			DestroyMainWindow (hwnd);
			break;
		case MSG_TIMER:
			hdc = BeginPaint(hwnd);
			Show_Signal(hdc);
			EndPaint(hwnd,hdc);
			break;
		case MSG_MOTOR_MOVING: /* For Motor Control By KB Kim 2011.05.22 */
			if (wparam)
			{
				Mv_MotorMovingDisplay();
			}
			else
			{
				Motor_Moving_Stop();
			}
			break;
		case MSG_PAINT:

			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, USALS_SETTING);

			hdc = BeginPaint(hwnd);
			
			MV_Draw_UsalsMenuFull(hdc);
			
			SetTextColor(hdc,CSAPP_WHITE_COLOR);
			SetBkMode(hdc,BM_TRANSPARENT);
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y ), CS_MW_LoadStringByIdx(CSAPP_STR_STRENGTH));
			MV_CS_MW_TextOut( hdc,ScalerWidthPixel(MV_INSTALL_SIGNAL_X - 150),ScalerHeigthPixel(MV_INSTALL_SIGNAL_Y + MV_INSTALL_SIGNAL_YGAP ), CS_MW_LoadStringByIdx(CSAPP_STR_QUALITY));
			Show_Signal(hdc);

			//MV_Install_draw_help_banner(hdc, SATELLITE_SETTING);
			EndPaint(hwnd,hdc);

			return 0;
		case MSG_KEYDOWN:
			if ( MV_Check_Confirm_Window() == TRUE )
			{
				MV_Confirm_Proc(hwnd, wparam);
				
				if ( wparam == CSAPP_KEY_ESC || wparam == CSAPP_KEY_MENU || wparam == CSAPP_KEY_ENTER )
				{
					if ( wparam == CSAPP_KEY_ENTER )
					{
						if ( MV_Check_YesNo() == TRUE )
						{
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);

							// CSApp_Usals_Applets=CSApp_Applet_Install;
							
							CS_DBU_Set_Motor_Limit(Temp_Limit_Use);
							
							if ( Temp_Limit_Use == FALSE )
							{
								/* For Motor Control By KB Kim 2011.05.22 */
								TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_OFF, 0);
								// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_OFF,0);
							}
							
							MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude = atoi(sTemp_Sat_Long);
							CS_DBU_SetLocal_Longitude((U32)atoi(sTemp_Usals_Long));
							CS_DBU_SetLocal_Latitude((U32)atoi(sTemp_Usals_Lati));
							
							CS_DBU_SaveUserSettingDataInHW();
							MV_SetSatelliteData(MV_Sat_Data);
						}
						else
						{
							CS_DBU_SetLocal_Longitude(u32LongitudeBackup);
							CS_DBU_SetLocal_Latitude(u32LatitudeBackup);
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							
							// CSApp_Usals_Applets=CSApp_Applet_Install;
						}
					}
					else
					{
						CS_DBU_SetLocal_Longitude(u32LongitudeBackup);
						CS_DBU_SetLocal_Latitude(u32LatitudeBackup);
						hdc = BeginPaint(hwnd);
						Restore_Confirm_Window(hdc);
						EndPaint(hwnd,hdc);
					}

					SendMessage (hwnd, MSG_CLOSE, 0, 0);
				}
				
				if (wparam == CSAPP_KEY_IDLE)
				{
					CSApp_Usals_Applets=CSApp_Applet_Sleep;
					CS_DBU_SetLocal_Longitude(u32LongitudeBackup);
					CS_DBU_SetLocal_Latitude(u32LatitudeBackup);
					hdc = BeginPaint(hwnd);
					Restore_Confirm_Window(hdc);
					EndPaint(hwnd,hdc);
					SendMessage (hwnd, MSG_CLOSE, 0, 0);
				}

				break;
			}
			
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
							u8Temp_Sat_index = u8Glob_Sat_Focus;
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

							/* By KB Kim 2011.05.21 */
							sprintf(sTemp_Sat_Long, "%04d", MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude);
							MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_SAT_LONGITUDE);
							EndPaint(hwnd,hdc);

							// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);

							/* For Motor Control By KB Kim 2011.05.22 */
							// DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
//							MV_Draw_Disecq_Waiting_Window(hwnd, 200);

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
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_WEST_LIMIT);
							MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_EAST_LIMIT);
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
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
				}

				break;
			}

			switch (wparam)
			{
				case CSAPP_KEY_IDLE:
					CSApp_Usals_Applets = CSApp_Applet_Sleep;
					if ( u8DataChanged || Temp_Limit_Use != CS_DBU_Get_Motor_Limit() || CS_DBU_GetLocal_Longitude() != (U32)atoi(sTemp_Usals_Long) || CS_DBU_GetLocal_Latitude() != (U32)atoi(sTemp_Usals_Lati) || MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude != (U16)atoi(sTemp_Sat_Long))
					{
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVEHELP);
					} else {
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
						
				case CSAPP_KEY_DOWN: 
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, Usals_Focus_Item);
						/* By KB Kim 2011.05.21 */
						Input_Status = 0;
						switch (Usals_Focus_Item)
						{
							case USALS_ITEM_SAT_LONGITUDE :
								if (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude != atoi(sTemp_Sat_Long))
								{
									MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude = atoi(sTemp_Sat_Long);
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							case USALS_ITEM_LONGITUDE :
								if (CS_DBU_GetLocal_Longitude() != (U32)atoi(sTemp_Usals_Long))
								{
									CS_DBU_SetLocal_Longitude((U32)atoi(sTemp_Usals_Long));
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							case USALS_ITEM_LATITUDE :
								if (CS_DBU_GetLocal_Latitude() != (U32)atoi(sTemp_Usals_Lati))
								{
									CS_DBU_SetLocal_Latitude((U32)atoi(sTemp_Usals_Lati));
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							default :
								break;
						}

						if(Usals_Focus_Item == USALS_ITEM_MAX - 1)
							Usals_Focus_Item = USALS_ITEM_SATELLITE;
						else
						{
							if ( Usals_Focus_Item == USALS_ITEM_LIMIT_SET && Temp_Limit_Use == FALSE )
								Usals_Focus_Item = USALS_ITEM_REF;
							else
								Usals_Focus_Item++;
						}

						MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
						
				case CSAPP_KEY_UP:
					{
						hdc = BeginPaint(hwnd);
						MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, Usals_Focus_Item);

						/* By KB Kim 2011.05.21 */
						Input_Status = 0;
						switch (Usals_Focus_Item)
						{
							case USALS_ITEM_SAT_LONGITUDE :
								if (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude != atoi(sTemp_Sat_Long))
								{
									MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude = atoi(sTemp_Sat_Long);
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							case USALS_ITEM_LONGITUDE :
								if (CS_DBU_GetLocal_Longitude() != (U32)atoi(sTemp_Usals_Long))
								{
									CS_DBU_SetLocal_Longitude((U32)atoi(sTemp_Usals_Long));
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							case USALS_ITEM_LATITUDE :
								if (CS_DBU_GetLocal_Latitude() != (U32)atoi(sTemp_Usals_Lati))
								{
									CS_DBU_SetLocal_Latitude((U32)atoi(sTemp_Usals_Lati));
									MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
									u8DataChanged = 1;
								}
								break;
							default :
								break;
						}
						
						if(Usals_Focus_Item == USALS_ITEM_SATELLITE)
							Usals_Focus_Item = USALS_ITEM_MAX - 1;
						else
						{
							if ( Usals_Focus_Item == USALS_ITEM_REF && Temp_Limit_Use == FALSE )
								Usals_Focus_Item = USALS_ITEM_LIMIT_SET;
							else
								Usals_Focus_Item--;
						}

						MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{

					} else {
						switch(Usals_Focus_Item)
						{
							case USALS_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);

								u8Temp_Sat_index = u8Glob_Sat_Focus;
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
								
								sprintf(sTemp_Sat_Long, "%04d", MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude);
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_SAT_LONGITUDE);
								EndPaint(hwnd,hdc);

								// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);

								/* For Motor Control By KB Kim 2011.05.22 */
								//DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
								//MV_Draw_Disecq_Waiting_Window(hwnd, 200);

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

							case USALS_ITEM_SAT_LONGITUDE:
							case USALS_ITEM_LONGITUDE:
							case USALS_ITEM_LATITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status > 0 )
									Input_Status--;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case USALS_ITEM_LIMIT_SET:
#if 0
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
#endif
								break;
								
							default:
								break;
						}
					}
        			break;			
				case CSAPP_KEY_RIGHT:
					if ( Sat_List_Status == TRUE )
					{

					} else {
						switch(Usals_Focus_Item)
						{
							case USALS_ITEM_SATELLITE:
								hdc = BeginPaint(hwnd);

								u8Temp_Sat_index = u8Glob_Sat_Focus;
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
									u8Glob_Sat_Focus/*Mv_SAT_SatFocus*/ = 0;

								{
									U8 Temp_Count = 0;
									Temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

									if ( Temp_Count > 0 )
										u8TpCount = Temp_Count - 1;
									else
										u8TpCount = 0;
								}
								
								u8Glob_TP_Focus/*Mv_SAT_TPFocus*/ = 0;

								sprintf(sTemp_Sat_Long, "%04d", MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude);
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);								
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_TP);
								MV_Draw_UsalsMenuBar(hdc, MV_UNFOCUS, USALS_ITEM_SAT_LONGITUDE);
								EndPaint(hwnd,hdc);

								// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);

								/* For Motor Control By KB Kim 2011.05.22 */
								// DVB_MotorGotoX (MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);

								//MV_Draw_Disecq_Waiting_Window(hwnd, 200);

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

							case USALS_ITEM_SAT_LONGITUDE:
							case USALS_ITEM_LONGITUDE:
							case USALS_ITEM_LATITUDE:
								hdc = BeginPaint(hwnd);

								if ( Input_Status < 3 )
									Input_Status++;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case USALS_ITEM_LIMIT_SET:
#if 0
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_UsalsMenuBar(hdc, MV_FOCUS, Usals_Focus_Item);
								EndPaint(hwnd,hdc);
#endif
								break;
								
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

							switch ( Usals_Focus_Item )
							{
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
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX ;
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
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = temp_Count;
										stContents.u8Focus_Position = u8Glob_TP_Focus;
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

								case USALS_ITEM_LATITUDE:
									if (Input_Status == 0)
									{
										if (sTemp_Usals_Lati[0] == '0')
										{
											sTemp_Usals_Lati[0] = '1';
										}
										else
										{
											sTemp_Usals_Lati[0] = '0';
										}

										Input_Status++;
										hdc = BeginPaint(hwnd);
										MV_Draw_UsalsMenuBar(hdc, FOCUS, Usals_Focus_Item);
										EndPaint(hwnd,hdc);
									}
									break;

								case USALS_ITEM_WEST_LIMIT:
									/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_WEST, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_WEST,0);
									break;

								case USALS_ITEM_EAST_LIMIT:
									/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_EAST, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_EAST,0);
									break;

								case USALS_ITEM_REF:
									/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_GOTO_REF, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_GOTO_REF,0);
									break;

								default:
									break;
							}
						} 
					}
					break;
				case CSAPP_KEY_INFO:
					
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
					if ( Usals_Focus_Item == USALS_ITEM_SAT_LONGITUDE || Usals_Focus_Item == USALS_ITEM_LONGITUDE || Usals_Focus_Item == USALS_ITEM_LATITUDE )
					{
						if ((Usals_Focus_Item == USALS_ITEM_LATITUDE) && (Input_Status == 0))
						{
							break;
						}
						
						if (MV_Usals_Update_Value(wparam, Usals_Focus_Item))
						{
							hdc=BeginPaint(hwnd);
							MV_Draw_Msg_Window(hdc, CSAPP_STR_INVALID_DATA);
							EndPaint(hwnd,hdc);

							usleep( 1000 * 1000 );

							hdc=BeginPaint(hwnd);
							Close_Msg_Window(hdc);
							EndPaint(hwnd,hdc);
							break;
						}
									
						if ( Input_Status < 3 )
						{
							Input_Status++;
						}
									
						hdc = BeginPaint(hwnd);
						MV_Draw_UsalsMenuBar(hdc, FOCUS, Usals_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;

				case CSAPP_KEY_ESC:
					CSApp_Usals_Applets=CSApp_Applet_Desktop;
					if ( u8DataChanged || Temp_Limit_Use != CS_DBU_Get_Motor_Limit() || CS_DBU_GetLocal_Longitude() != (U32)atoi(sTemp_Usals_Long) || CS_DBU_GetLocal_Latitude() != (U32)atoi(sTemp_Usals_Lati) || MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude != (U16)atoi(sTemp_Sat_Long))
					{
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVEHELP);
					} else {
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
					break;
				case CSAPP_KEY_MENU:
					CSApp_Usals_Applets=CSApp_Applet_Install;
					if ( u8DataChanged || Temp_Limit_Use != CS_DBU_Get_Motor_Limit() || CS_DBU_GetLocal_Longitude() != (U32)atoi(sTemp_Usals_Long) || CS_DBU_GetLocal_Latitude() != (U32)atoi(sTemp_Usals_Lati) || MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude != (U16)atoi(sTemp_Sat_Long))
					{
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVEHELP);
					} else {
						SendMessage (hwnd, MSG_CLOSE, 0, 0);
					}
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

