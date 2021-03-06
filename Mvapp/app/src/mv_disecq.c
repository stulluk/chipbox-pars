
#include "linuxos.h"

#include "database.h"
#include "ch_install.h"
#include "mwsetting.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "csinstall.h"
#include "mv_motor.h"
#include "ui_common.h"
#include "fe_mngr.h"
#include "dvbtuner.h"

#define MV_MOTOR_DBG_TRACE		printf( "[%s]\n", __FUNCTION__ )
#define	MAX_POSITION			64

U16 D_Mortor_Str[MOTOR_ITEM_MAX+1] = {
	CSAPP_STR_SATELLITE,
	CSAPP_STR_TPSELECT,
	CSAPP_STR_POSITION,
	CSAPP_STR_GOTOX,
	CSAPP_STR_STEPMOVE,
	CSAPP_STR_AUTOMOVE,
	CSAPP_STR_LIMIT_SET,
	CSAPP_STR_WEST_LIMIT,
	CSAPP_STR_EAST_LIMIT,
	CSAPP_STR_GOTO_REF,
	CSAPP_STR_RECALC,
	CSAPP_STR_SAVE
};

U8	Motor_Arrow_Kind[MOTOR_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC,
	MV_STATIC
};

U8	Motor_Enter_Kind[MOTOR_ITEM_MAX] = {
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_STATIC,
	MV_STATIC,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT,
	MV_SELECT
};

static CSAPP_Applet_t		CSApp_Motor_Applets;
static eMotorItemID			Motor_Focus_Item = MOTOR_ITEM_SATELLITE;
static MV_stSatInfo			MV_Sat_Data[MV_MAX_SATELLITE_COUNT];
static U32					ScreenWidth = CSAPP_OSD_MAX_WIDTH;
static U8					u8TpCount;
static U8					Temp_Motor_Position = 0;
static U8					Prev_Motor_Position = 0;
static U8					Temp_Motor_GotoX = 0;
static U32					Temp_Limit_Use = FALSE;
static MV_stTPInfo 			MV_TPInfo;
static BOOL					Sat_List_Status;
static EN_MOTOR_ACTION_T	u8Temp_Step_Move;
static U8					u8Temp_Sat_index;
static U8					u8TempSavePosition;

static pthread_t  			hMotor_TaskHandle;

/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
static U8                   hMotorOn = 0;

/* For Motor Control By KB Kim 2011.05.22 */
extern U32					*Tuner_HandleId;

int Motor_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam);

void *MV_Motor_Task( void *param )
{
	/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
	while( hMotorOn )
	{
		if ( u8Temp_Step_Move == EN_MOTOR_EAST )
		{
			/* For Motor Control By KB Kim 2011.05.22 */
			TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_DRIVE_EAST, 120);
       		// DVB_MotorControl(EN_MOTOR_CMD_DRIVE_EAST, 120);
		}
		else if ( u8Temp_Step_Move == EN_MOTOR_WEST )
		{
			/* For Motor Control By KB Kim 2011.05.22 */
			TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_DRIVE_WEST, 120);
			// DVB_MotorControl(EN_MOTOR_CMD_DRIVE_WEST, 120);
		}
		usleep( 110000 * 1000 );
	}
	return ( param );
}

int MV_Motor_Init(void)
{
	/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
	printf("MV_Motor_Init\n");
	hMotorOn = 1;
	pthread_create( &hMotor_TaskHandle, NULL, MV_Motor_Task, NULL );
	return( 0 );
}

void MV_Motor_Stop(void)
{
	printf("MV_Motor_Stop\n");
	pthread_cancel( hMotor_TaskHandle );
	/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
	hMotorOn = 0;
}

void MV_Draw_MotorSelectBar(HDC hdc, int y_gap, eScanItemID esItem)
{
	int mid_width = (ScreenWidth - MV_INSTALL_MENU_X*2) - ( MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth );
	int right_x = MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth + mid_width;

	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_LEFT]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X+MV_BMP[MVBMP_YELLOW_BAR_LEFT].bmWidth),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(mid_width),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_MIDDLE].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_MIDDLE]);
	FillBoxWithBitmap(hdc,ScalerWidthPixel(right_x),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_YELLOW_BAR_RIGHT].bmHeight),&MV_BMP[MVBMP_YELLOW_BAR_RIGHT]);

	switch(Motor_Enter_Kind[esItem])
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
	
	if ( Motor_Arrow_Kind[esItem] == MV_SELECT )
	{
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_LEFT_ARROW].bmHeight),&MV_BMP[MVBMP_LEFT_ARROW]);
		FillBoxWithBitmap(hdc,ScalerWidthPixel(MV_INSTALL_MENU_X + mid_width - 12 ),ScalerHeigthPixel( y_gap ), ScalerWidthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmWidth),ScalerHeigthPixel(MV_BMP[MVBMP_RIGHT_ARROW].bmHeight),&MV_BMP[MVBMP_RIGHT_ARROW]);
	}
}

void MV_Draw_MotorMenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem)
{
	int 	y_gap = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * esItem;
	RECT	TmpRect;
	char	temp_str[30];

	if ( u8Focuskind == MV_FOCUS )
	{
		SetTextColor(hdc,MV_BAR_FOCUS_CHAR_COLOR);
		SetBkMode(hdc,BM_TRANSPARENT);
		MV_SetBrushColor( hdc, MVAPP_YELLOW_COLOR );
		MV_Draw_MotorSelectBar(hdc, y_gap, esItem);
	} else {
		if ( Temp_Limit_Use == FALSE && ( esItem == MOTOR_ITEM_WEST_LIMIT || esItem == MOTOR_ITEM_EAST_LIMIT ))
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

	MV_CS_MW_TextOut( hdc, ScalerWidthPixel(MV_INSTALL_MENU_X + 10),ScalerHeigthPixel(y_gap+4), CS_MW_LoadStringByIdx(D_Mortor_Str[esItem]));

	TmpRect.left	=ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
	TmpRect.right	=TmpRect.left + MV_MENU_TITLE_DX;
	TmpRect.top		=ScalerHeigthPixel(y_gap+4);
	TmpRect.bottom	=TmpRect.top + ScalerHeigthPixel(MV_INSTALL_MENU_HEIGHT2);

	memset ( temp_str, 0, 30 );

	switch(esItem)
	{
		
		case MOTOR_ITEM_SATELLITE:
			CS_MW_DrawText(hdc, MV_Sat_Data[u8Glob_Sat_Focus].acSatelliteName, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_TP:
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
		case MOTOR_ITEM_POSITION:
			
			if ( Temp_Motor_Position == 0 )
				sprintf(temp_str, "%s", "REF");
			else
				sprintf(temp_str, "%d", Temp_Motor_Position);
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_GOTOX:
			if ( Temp_Motor_GotoX == 0 )
				sprintf(temp_str, "%s", "REF");
			else
				sprintf(temp_str, "%d", Temp_Motor_GotoX);
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_STEPMOVE:
			if ( u8Temp_Step_Move == EN_MOTOR_STOP )
				sprintf(temp_str, "%s", "STEP");
			else if ( u8Temp_Step_Move == EN_MOTOR_WEST )
				sprintf(temp_str, "%s", "Move West");
			else
				sprintf(temp_str, "%s", "Move East");
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_AUTOMOVE:
			if ( u8Temp_Step_Move == EN_MOTOR_STOP )
				sprintf(temp_str, "%s", "Auto");
			else if ( u8Temp_Step_Move == EN_MOTOR_WEST )
				sprintf(temp_str, "%s", "Moving West - Stop : OK");
			else
				sprintf(temp_str, "%s", "Moving East - Stop : OK");
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_LIMIT_SET:
			if ( Temp_Limit_Use == TRUE )
				sprintf(temp_str, "%s", "USE");
			else
				sprintf(temp_str, "%s", "NOT USE");
			
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_WEST_LIMIT:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_EAST_LIMIT:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_REF:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_RECALC:
			sprintf(temp_str, "%s", "ENTER");
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_SAVE:
			if (u8TempSavePosition)
			{
				sprintf(temp_str, "%s-%d", CS_MW_LoadStringByIdx(CSAPP_STR_SAVE),Temp_Motor_Position);
			}
			else
			{
				sprintf(temp_str, "%s", "ENTER");
			}
			CS_MW_DrawText(hdc, temp_str, -1, &TmpRect, DT_CENTER);
			break;
		case MOTOR_ITEM_MAX:
		default:
			break;
	}
}

void MV_Draw_MotorMenuFull(HDC hdc)
{
	U16 	i = 0;
	
	for( i = 0 ; i < MOTOR_ITEM_MAX ; i++ )
	{
		if( Motor_Focus_Item == i )
		{
			MV_Draw_MotorMenuBar(hdc, MV_FOCUS, i);
		} else {
			MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, i);
		}
	}
}

CSAPP_Applet_t CSApp_Disecq_Motor(void)
{
	int   					BASE_X, BASE_Y, WIDTH, HEIGHT;
	MSG   					msg;
	HWND  					hwndMain;
	MAINWINCREATE			CreateInfo;
		
	CSApp_Motor_Applets = CSApp_Applet_Error;

	Motor_Focus_Item = MOTOR_ITEM_SATELLITE; 
	
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
	CreateInfo.spCaption 	= "Motor Setting";
	CreateInfo.hMenu	 	= 0;
	CreateInfo.hCursor	 	= 0;
	CreateInfo.hIcon	 	= 0;
	CreateInfo.MainWindowProc = Motor_Msg_cb;
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
	
	return CSApp_Motor_Applets;
}

int Motor_Msg_cb(HWND hwnd, int message, WPARAM wparam, LPARAM lparam)
{
	HDC 			hdc=0;
	
	switch(message)
	{
		case MSG_CREATE:
			if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
				KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);

			Motor_Focus_Item = MOTOR_ITEM_SATELLITE;
			SearchData.ScanMode = CS_DBU_GetAntenna_Type();
			
			memset (&Capture_bmp, 0, sizeof (BITMAP));
			memset (&WarningCapture_bmp, 0, sizeof (BITMAP));
			memset (&Search_Condition_Sat_Index, 0xFF, MAX_MULTI_SAT);

			if ( MV_GetSelected_SatData_Count() > 0 )
				MV_GetSatelliteData(MV_Sat_Data);
			else
			{
				CSApp_Motor_Applets=CSApp_Applet_Install;
				SendMessage (hwnd, MSG_CLOSE, 0, 0);
			}
			Temp_Limit_Use = CS_DBU_Get_Motor_Limit();
			u8TpCount = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus) - 1;
			Prev_Motor_Position = Temp_Motor_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;
			Temp_Motor_GotoX = 0 ;
			/* For Motor Control By KB Kim 2011.05.22 */
			// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
			// DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_Position);
			MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
			break;
		case MSG_CLOSE:
			/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
			if (hMotorOn)
			{
				u8Temp_Step_Move = EN_MOTOR_STOP;
				MV_Motor_Stop();								
				/* For Motor Control By KB Kim 2011.05.22 */
				TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
				// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);									
			}
			if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
				KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
			
			
			/* For Motor Control By KB Kim 2011.05.22 */
			if(Motor_Moving_State())
			{
				Motor_Moving_Stop();
			}
			
			/* By KB Kim 2011.05.28 */
			if (CSApp_Motor_Applets != CSApp_Applet_Install)
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

			MV_DRAWING_MENUBACK(hwnd, CSAPP_MAINMENU_INSTALL, MOTOR_SETTING);

			hdc = BeginPaint(hwnd);
			
			MV_Draw_MotorMenuFull(hdc);
			
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

							CSApp_Motor_Applets=CSApp_Applet_Install;
							
							CS_DBU_Set_Motor_Limit(Temp_Limit_Use);
							
							if ( Temp_Limit_Use == FALSE )
							{
								/* For Motor Control By KB Kim 2011.05.22 */
								TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_OFF, 0);
								// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_OFF,0);
							}
									
							MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition = Temp_Motor_Position;
							/* For Motor Control By KB Kim 2011.05.22 */
							TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_STORE_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
							// DVB_MotorControl(EN_MOTOR_CMD_STORE_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
							MV_SetSatelliteData(MV_Sat_Data);
							CS_DBU_SaveUserSettingDataInHW();
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}else {
							hdc = BeginPaint(hwnd);
							Restore_Confirm_Window(hdc);
							EndPaint(hwnd,hdc);
							
							CSApp_Motor_Applets=CSApp_Applet_Install;
							SendMessage (hwnd, MSG_CLOSE, 0, 0);
						}
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

			if ( MV_Get_PopUp_Window_Status() == TRUE )
			{
				MV_PopUp_Proc(hwnd, wparam);

				if ( wparam == CSAPP_KEY_ENTER )
				{
					U8						u8Result_Value;
					//MV_stSatInfo			Temp_SatData;

					u8Result_Value = MV_Get_PopUp_Window_Result();

					switch(Motor_Focus_Item)
					{
						case MOTOR_ITEM_SATELLITE:
							hdc = BeginPaint(hwnd);
							//u8Glob_Sat_Focus = u8Result_Value;
							u8Temp_Sat_index = u8Glob_Sat_Focus;
							u8TempSavePosition = 0;
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
							Prev_Motor_Position = Temp_Motor_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;

							// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);
							/* For Motor Control By KB Kim 2011.05.22 */
							// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
							// DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_Position);
								
							MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
							MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_TP);
							MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_POSITION);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case MOTOR_ITEM_TP:
							hdc = BeginPaint(hwnd);
							u8Glob_TP_Focus = u8Result_Value;							
							MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case MOTOR_ITEM_POSITION:
							hdc = BeginPaint(hwnd);
							Temp_Motor_Position = u8Result_Value;
							MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
							EndPaint(hwnd,hdc);
							
							// MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case MOTOR_ITEM_GOTOX:
							hdc = BeginPaint(hwnd);
							Temp_Motor_GotoX = u8Result_Value;
							MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
							EndPaint(hwnd,hdc);

							/* For Motor Control By KB Kim 2011.05.22 */
							TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
							// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
							// Motor_Moving_Start(370, 0);
							/* For Motor Control By KB Kim 2011.05.22 */
							Mv_MotorMovingDisplay();
							TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_GOTO_POSITION, Temp_Motor_GotoX);
							// DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_GotoX);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						case MOTOR_ITEM_LIMIT_SET:
							hdc = BeginPaint(hwnd);
							
							if ( u8Result_Value == 0 )
								Temp_Limit_Use = FALSE;
							else
								Temp_Limit_Use = TRUE;
							
							MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
							MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_WEST_LIMIT);
							MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_EAST_LIMIT);
							EndPaint(hwnd,hdc);
							
							MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
							break;
						default:
							break;
					}

					MV_SetSatelliteData(MV_Sat_Data);
						
					if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
						KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
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
					CSApp_Motor_Applets = CSApp_Applet_Sleep;
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;

				case CSAPP_KEY_TV_AV:
					ScartSbOnOff(); /* By KB Kim : 2010_08_31 for Scart Control */
					break;
					
				case CSAPP_KEY_DOWN: 
					{
						/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
						if ((Motor_Focus_Item == MOTOR_ITEM_AUTOMOVE) && (hMotorOn))
						{
							u8Temp_Step_Move = EN_MOTOR_STOP;
							MV_Motor_Stop();
							/* For Motor Control By KB Kim 2011.05.22 */
							TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
							// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
						}
						
						if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
						{
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
						}
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						
						hdc = BeginPaint(hwnd);
						MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, Motor_Focus_Item);

						if ( Temp_Limit_Use == FALSE && Motor_Focus_Item == MOTOR_ITEM_LIMIT_SET )
						{
							Motor_Focus_Item = MOTOR_ITEM_REF;
						} else {
							if(Motor_Focus_Item == MOTOR_ITEM_MAX - 1)
								Motor_Focus_Item = MOTOR_ITEM_SATELLITE;
							else
								Motor_Focus_Item++;
						}

						MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
						
				case CSAPP_KEY_UP:
					{
						/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
						if ((Motor_Focus_Item == MOTOR_ITEM_AUTOMOVE) && (hMotorOn))
						{
							u8Temp_Step_Move = EN_MOTOR_STOP;
							MV_Motor_Stop();
							/* For Motor Control By KB Kim 2011.05.22 */
							TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
							// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
						}
						
						if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
						{
							KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
						}
						SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
						
						hdc = BeginPaint(hwnd);
						MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, Motor_Focus_Item);

						if ( Temp_Limit_Use == FALSE && Motor_Focus_Item == MOTOR_ITEM_REF )
						{
							Motor_Focus_Item = MOTOR_ITEM_LIMIT_SET;
						} else {
							if(Motor_Focus_Item == MOTOR_ITEM_SATELLITE)
								Motor_Focus_Item = MOTOR_ITEM_MAX - 1;
							else
								Motor_Focus_Item--;
						}

						MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
						EndPaint(hwnd,hdc);
					}
					break;
					
				case CSAPP_KEY_LEFT:
					if ( Sat_List_Status == TRUE )
					{

					} else {
						switch(Motor_Focus_Item)
						{
							case MOTOR_ITEM_SATELLITE:
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
								
								Temp_Motor_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;

								// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);
								/* For Motor Control By KB Kim 2011.05.22 */
								// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
								// DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_Position);
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_TP);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_POSITION);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case MOTOR_ITEM_TP:
								hdc = BeginPaint(hwnd);

								if ( u8Glob_TP_Focus == 0 )
									u8Glob_TP_Focus = u8TpCount;
								else
									u8Glob_TP_Focus--;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case MOTOR_ITEM_POSITION:
								hdc = BeginPaint(hwnd);

								if ( Temp_Motor_Position == 0 )
									Temp_Motor_Position = MAX_POSITION;
								else
									Temp_Motor_Position--;

								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								// MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case MOTOR_ITEM_GOTOX:
#if 0
								hdc = BeginPaint(hwnd);

								if ( Temp_Motor_GotoX == 0 )
									Temp_Motor_GotoX = MAX_POSITION;
								else
									Temp_Motor_GotoX--;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION,Temp_Motor_GotoX);
								usleep(1500);
#endif
								break;
								
							case MOTOR_ITEM_STEPMOVE:
								hdc = BeginPaint(hwnd);
								u8Temp_Step_Move = EN_MOTOR_WEST;
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								
								printf("==== LEFT STEP MOVE\n");
								/* For Motor Control By KB Kim 2011.05.22 */
								TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_DRIVE_WEST, 0);
       							// DVB_MotorControl(EN_MOTOR_CMD_DRIVE_WEST, 0);

								hdc = BeginPaint(hwnd);
								u8Temp_Step_Move = EN_MOTOR_STOP;
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case MOTOR_ITEM_AUTOMOVE:
								/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
								if (hMotorOn)
								{
									if (u8Temp_Step_Move == EN_MOTOR_WEST)
									{
										break;
									}
									else if (u8Temp_Step_Move == EN_MOTOR_EAST)
									{
										u8Temp_Step_Move = EN_MOTOR_STOP;
										MV_Motor_Stop();
										/* For Motor Control By KB Kim 2011.05.22 */
										TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
										// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
										hdc = BeginPaint(hwnd);
										u8Temp_Step_Move = EN_MOTOR_WEST;
										MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
										EndPaint(hwnd,hdc);
										usleep( 500 * 1000 );
									}
								}
								else
								{
									hdc = BeginPaint(hwnd);
									u8Temp_Step_Move = EN_MOTOR_WEST;
									MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
									EndPaint(hwnd,hdc);
								}

								printf("==== LEFT AUTO MOVE\n");
								MV_Motor_Init();
#if 0 /* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
								hdc = BeginPaint(hwnd);
								//MV_Draw_Disecq_Waiting_Window(hwnd, 15);
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
#endif
								break;
								
							case MOTOR_ITEM_LIMIT_SET:
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_WEST_LIMIT);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_EAST_LIMIT);
								EndPaint(hwnd,hdc);
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
						switch(Motor_Focus_Item)
						{
							case MOTOR_ITEM_SATELLITE:
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

								Temp_Motor_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;

								// Motor_Moving_Start((U16)MV_Sat_Data[u8Glob_Sat_Focus].s16Longitude, (U16)MV_Sat_Data[u8Temp_Sat_index].s16Longitude);
								/* For Motor Control By KB Kim 2011.05.22 */
								// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
								// DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_Position);
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);								
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_TP);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_POSITION);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;

							case MOTOR_ITEM_TP:
								hdc = BeginPaint(hwnd);

								if ( u8Glob_TP_Focus == u8TpCount )
									u8Glob_TP_Focus = 0;
								else
									u8Glob_TP_Focus++;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
								
							case MOTOR_ITEM_POSITION:
								hdc = BeginPaint(hwnd);

								if ( Temp_Motor_Position == MAX_POSITION )
									Temp_Motor_Position = 0;
								else
									Temp_Motor_Position++;

								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								// MV_FE_SetTuning_TP(&MV_Sat_Data[u8Glob_Sat_Focus], u8Glob_Sat_Focus, u8Glob_TP_Focus);
								break;
								
							case MOTOR_ITEM_GOTOX:
#if 0
								hdc = BeginPaint(hwnd);

								if ( Temp_Motor_GotoX == MAX_POSITION )
									Temp_Motor_GotoX = 0;
								else
									Temp_Motor_GotoX++;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_Motor_GotoX);
								usleep(1500);
#endif
								break;
								
							case MOTOR_ITEM_STEPMOVE:
								hdc = BeginPaint(hwnd);
								u8Temp_Step_Move = EN_MOTOR_EAST;
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								
								printf("==== RIGHT STEP MOVE\n");
 								/* For Motor Control By KB Kim 2011.05.22 */
								TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_DRIVE_EAST, 0);
      							// DVB_MotorControl(EN_MOTOR_CMD_DRIVE_EAST, 0);

								hdc = BeginPaint(hwnd);							
								u8Temp_Step_Move = EN_MOTOR_STOP;
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
								break;
								
							case MOTOR_ITEM_AUTOMOVE:
								/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
								if (hMotorOn)
								{
									if (u8Temp_Step_Move == EN_MOTOR_EAST)
									{
										break;
									}
									else if (u8Temp_Step_Move == EN_MOTOR_WEST)
									{
										u8Temp_Step_Move = EN_MOTOR_STOP;
										MV_Motor_Stop();
		 								/* For Motor Control By KB Kim 2011.05.22 */
										TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
										// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
										hdc = BeginPaint(hwnd);
										u8Temp_Step_Move = EN_MOTOR_EAST;
										MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
										EndPaint(hwnd,hdc);
										usleep( 1000 * 1000 );
									}
								}
								else
								{
									hdc = BeginPaint(hwnd);
									u8Temp_Step_Move = EN_MOTOR_EAST;
									MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
									EndPaint(hwnd,hdc);
								}
								
								printf("==== RIGHT AUTO MOVE\n");
								MV_Motor_Init();
#if 0 /* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
								hdc = BeginPaint(hwnd);
								//MV_Draw_Disecq_Waiting_Window(hwnd, 15);
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								EndPaint(hwnd,hdc);
#endif
								break;
								
							case MOTOR_ITEM_LIMIT_SET:
								hdc = BeginPaint(hwnd);

								if ( Temp_Limit_Use == FALSE )
									Temp_Limit_Use = TRUE;
								else
									Temp_Limit_Use = FALSE;
								
								MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_WEST_LIMIT);
								MV_Draw_MotorMenuBar(hdc, MV_UNFOCUS, MOTOR_ITEM_EAST_LIMIT);
								EndPaint(hwnd,hdc);
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
							if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
								KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);

							switch ( Motor_Focus_Item )
							{
								case MOTOR_ITEM_SATELLITE:
									{
										int						i = 0;
										int						SelSat_Count = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;
										MV_stSatInfo			Temp_SatData;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
#if 1
										SelSat_Count = MV_GetSelected_SatData_Count();
										
										for ( i = 0 ; i < SelSat_Count ; i++ )
										{
											MV_GetSelected_SatData_By_Count(&Temp_SatData, i );
											sprintf(stContents.Contents[i], "%s", Temp_SatData.acSatelliteName);
										}
#else
										for ( i = 0 ; i < MV_SAT_MAX ; i++ )
										{
											if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
											{
												MV_GetSatelliteData_ByIndex(&Temp_SatData, i);
												sprintf(stContents.Contents[i], "%s", Temp_SatData.acSatelliteName);
												SelSat_Count++;
											}
										}
#endif
										if ( SelSat_Count > 1 )
										{
											smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Motor_Focus_Item + 1 );
											smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
											smwRect.right = smwRect.left + MV_MENU_TITLE_DX ;
											// smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
											// smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
											smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
											stContents.u8TotalCount = SelSat_Count;
											stContents.u8Focus_Position = MV_GetSelected_Index_By_Satindex(u8Glob_Sat_Focus);
											MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
										}
									} 
									break;

								case MOTOR_ITEM_TP:
									{
										int						i = 0;
										int						temp_Count = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;
										MV_stTPInfo				Temp_TP;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));
										temp_Count = MV_DB_Get_TPCount_By_Satindex(u8Glob_Sat_Focus);

										// printf("Motor TP count : %d\n", temp_Count);
										
										for ( i = 0 ; i < temp_Count ; i++ )
										{
											MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TP, u8Glob_Sat_Focus, i);
											if ( MV_TPInfo.u8Polar_H == 1 )
												sprintf(stContents.Contents[i], "%d/%d. %d/%s/%d", i + 1, u8TpCount + 1, Temp_TP.u16TPFrequency, "H", Temp_TP.u16SymbolRate);
											else
												sprintf(stContents.Contents[i], "%d/%d. %d/%s/%d", i + 1, u8TpCount + 1, Temp_TP.u16TPFrequency, "V", Temp_TP.u16SymbolRate);
										}
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Motor_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX;
										// smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4) - 40;
										// smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 + 80;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = temp_Count;
										stContents.u8Focus_Position = u8Glob_TP_Focus;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									} 
									break;

								case MOTOR_ITEM_POSITION:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "REF");
										
										for ( i = 1 ; i < 64 ; i++ )
											sprintf(stContents.Contents[i], "%d", i);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Motor_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 64;
										stContents.u8Focus_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								case MOTOR_ITEM_GOTOX:
									{
										int						i = 0;
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "REF");
										
										for ( i = 1 ; i < 64 ; i++ )
											sprintf(stContents.Contents[i], "%d", i);
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Motor_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 64;
										stContents.u8Focus_Position = 0;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;

								case MOTOR_ITEM_AUTOMOVE:
									/* For Diseqc Motor Step Problem By KB Kim 2011.04.19 */
									if (hMotorOn)
									{
										u8Temp_Step_Move = EN_MOTOR_STOP;
										MV_Motor_Stop();								
		 								/* For Motor Control By KB Kim 2011.05.22 */
										TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_HALT, 0);
										// DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
										
										hdc = BeginPaint(hwnd);
										MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
										EndPaint(hwnd,hdc);
									}

									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									break;

								case MOTOR_ITEM_LIMIT_SET:
									{
										RECT					smwRect;
										stPopUp_Window_Contents stContents;

										memset( &stContents, 0x00, sizeof(stPopUp_Window_Contents));

										sprintf(stContents.Contents[0], "%s", "NOT USE");
										sprintf(stContents.Contents[1], "%s", "USE");
										
										smwRect.top = MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT + MV_INSTALL_MENU_YGAP ) * ( Motor_Focus_Item + 1 );
										smwRect.left = ScalerWidthPixel(MV_INSTALL_MENU_X + MV_MENU_TITLE_DX + MV_MENU_TITLE_DX/4);
										smwRect.right = smwRect.left + MV_MENU_TITLE_DX/2 ;
										smwRect.bottom = smwRect.top + MV_INSTALL_MENU_BAR_H * 10;
										stContents.u8TotalCount = 2;
										MV_Draw_NoName_Window(hwnd, &smwRect, &stContents);
									}
									break;
								case MOTOR_ITEM_WEST_LIMIT:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									
	 								/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_WEST, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_WEST,0);
									break;
								case MOTOR_ITEM_EAST_LIMIT:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									
	 								/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_EAST, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_EAST,0);
									break;
								case MOTOR_ITEM_REF:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									
	 								/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_GOTO_REF, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_GOTO_REF,0);
									break;
								case MOTOR_ITEM_RECALC:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									
	 								/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_RECALCULATION, 0);
									// DVB_MotorControl(EN_MOTOR_CMD_RECALCULATION,0);
									break;
								case MOTOR_ITEM_SAVE:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									hdc = BeginPaint(hwnd);
									u8TempSavePosition = 1;
									MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
									EndPaint(hwnd,hdc);
									
									printf("==== Save Position\n");

									
									CS_DBU_Set_Motor_Limit(Temp_Limit_Use);
									
									if ( Temp_Limit_Use == FALSE )
									{
		 								/* For Motor Control By KB Kim 2011.05.22 */
										TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_LIMIT_OFF, 0);
										// DVB_MotorControl(EN_MOTOR_CMD_LIMIT_OFF,0);
									}
									
									MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition = Temp_Motor_Position;
	 								/* For Motor Control By KB Kim 2011.05.22 */
									TunerControlMotor(Tuner_HandleId[0], MOTOR_CMD_STORE_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
									// DVB_MotorControl(EN_MOTOR_CMD_STORE_POSITION, MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition);
									Prev_Motor_Position = Temp_Motor_Position = MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition;
									MV_SetSatelliteData(MV_Sat_Data);
									CS_DBU_SaveUserSettingDataInHW();
									
									hdc = BeginPaint(hwnd);							
									u8TempSavePosition = 0;
									MV_Draw_MotorMenuBar(hdc, MV_FOCUS, Motor_Focus_Item);
									EndPaint(hwnd,hdc);
									break;

								default:
									if(IsTimerInstalled(hwnd, CHECK_SIGNAL_TIMER_ID))
										KillTimer(hwnd, CHECK_SIGNAL_TIMER_ID);
									SetTimer(hwnd, CHECK_SIGNAL_TIMER_ID, SIGNAL_TIMER);
									
									break;
							}
						} 
					}
					break;

				case CSAPP_KEY_INFO:
					
					break;

				case CSAPP_KEY_ESC:
				case CSAPP_KEY_MENU:
					if ( Temp_Limit_Use != CS_DBU_Get_Motor_Limit() || MV_Sat_Data[u8Glob_Sat_Focus].u8MotorPosition != Prev_Motor_Position )
					{
						MV_Draw_Confirm_Window(hwnd, CSAPP_STR_SAVEHELP);
					} else {
						CSApp_Motor_Applets=CSApp_Applet_Install;
						//MV_SetSatelliteData(MV_Sat_Data);
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

