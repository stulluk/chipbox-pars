
#include "linuxos.h"

#include "av_zapping.h"

#include "database.h"
#include "mwsvc.h"

#include "userdefine.h"
#include "cs_app_common.h"
#include "cs_app_main.h"
#include "mvosapi.h"
#include "cat6611api.h"
#include "dvbtuner.h" /* For Loop throuth contron in Sleep By KB Kim 2011.10.11 */
#include "scart.h"    /* For Scart OFF in Sleep mode by KB Kim 2012.04.07 */

extern U32	*Tuner_HandleId;

#define   CS_SLEEP_TIMER_ID		120
#define	CS_SLEEP_TIMER_MAX	1000*30

static CSAPP_Applet_t		CSApp_Sleep_Applets;

static void CSApp_Sleep(void);
static int Sleep_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam);
// static void CSApp_Wakeup(void);
extern void MV_TIMER_Get_Last_Schadule(tCS_TIMER_JobInfo *stJob_Info);

CSAPP_Applet_t CSApp_SysSleep(void)

{
		int 				BASE_X, BASE_Y, WIDTH, HEIGHT;
		MSG 				msg;
		HWND				hwndMain;
		MAINWINCREATE		CreateInfo;
		
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
		CreateInfo.spCaption = "Sleep";
		CreateInfo.hMenu	 = 0;
		CreateInfo.hCursor	 = 0;
		CreateInfo.hIcon	 = 0;
		CreateInfo.MainWindowProc = Sleep_Msg_cb;
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
	
		return CSApp_Applet_Desktop;
			
	}

static int Sleep_Msg_cb(HWND hwnd , int message, WPARAM wparam, LPARAM lparam)
{

	//	 tCS_DT_Time			CurrentTime;
	tCS_TIMER_JobInfo 	stJob_Info;
	tCS_DT_Date			tTemp_YMD;
	tCS_DT_Time			tTemp_HM;
	struct timespec 	time_value;
	struct tm			Temp_Time;
	static U16			Temp_MJD = 0;
	static U16			Temp_UTC = 0;

	switch(message)
	{
		case MSG_CREATE:
			stJob_Info.CS_Timer_Status = eCS_TIMER_Disable;
			stJob_Info.CS_Timer_Cycle = eCS_TIMER_Onetime;
			stJob_Info.CS_Timer_Type = eCS_TIMER_Wakeup; 
			stJob_Info.CS_Begin_Weekday = 0xFFFF;
			stJob_Info.CS_Begin_MDJ = 0xFFFF;
			stJob_Info.CS_Begin_UTC = 0xFFFF;
			stJob_Info.CS_Duration_UTC= 0xFFFF;	    	
			stJob_Info.CS_Wakeup_Service.SList_Type = eCS_TIMER_SERVICE_INVALID;
			stJob_Info.CS_Wakeup_Service.SList_Value= 0xFFFF;
			stJob_Info.CS_Wakeup_Service.Service_Index = 0xFFFF;

			CS_DBU_Set_Sleep(kCS_DBU_DEFAULT_Sleep);
			CS_DBU_SaveUserSettingDataInHW();

			CS_DBU_SetPower_Off_Mode(MV_POWER_OFF);

			/* For Loop throuth contron in Sleep By KB Kim 2011.10.11 */
			TunerOff(Tuner_HandleId[0]);
			/* For AC Power ON Control By KB Kim 2011.03.11 */
			SetPowerStatus(0);
			FbSendFndDisplay("----");
			CS_APP_SetLastUnlockServiceIndex(0xffff);
			
#if 0			
			SetTimer(hwnd, CS_SLEEP_TIMER_ID, CS_SLEEP_TIMER_MAX);
#endif
			break;

		case MSG_PAINT:
			/*
			{
				HDC				hdc;
				hdc = BeginPaint(hwnd);
				printf("_________CSApp_Sleep____MSG_PAINT_________\n");
				SetBrushColor(hdc,RGBA2Pixel(hdc,0x00, 0x00, 0x00, 0xff));
				FillBox(hdc,0,0,720,576);
				ClearOSD();
				EndPaint (hwnd, hdc);
			}*/
			ClearOSD();
			CSApp_Sleep();
			FbSetTime();
			/* For Scart OFF in Sleep mode by KB Kim 2012.04.07 */
			ScartSbControl(0);

			if ( CS_DBU_GetPower_Type() == 1 )
			{
				MV_TIMER_Get_Last_Schadule(&stJob_Info);

				if ( stJob_Info.CS_Timer_Status == eCS_TIMER_Enable )
				{
					tTemp_HM = CS_DT_UTCtoHM(stJob_Info.CS_Begin_UTC);
					tTemp_YMD = CS_DT_MJDtoYMD(stJob_Info.CS_Begin_MDJ);

					Temp_Time.tm_sec = 0;
					Temp_Time.tm_min = tTemp_HM.minute;
					Temp_Time.tm_hour = tTemp_HM.hour;
					Temp_Time.tm_mday = tTemp_YMD.day;
					Temp_Time.tm_mon = tTemp_YMD.month - 1;
					Temp_Time.tm_year = tTemp_YMD.year - 1900;

					time_value.tv_sec = (time_t)mktime(&Temp_Time);

					time_value.tv_sec -= 120;

					memcpy(&Temp_Time, localtime(&time_value.tv_sec), sizeof(struct tm));

					tTemp_HM.minute = Temp_Time.tm_min;
					tTemp_HM.hour = Temp_Time.tm_hour;
					tTemp_YMD.day = Temp_Time.tm_mday;
					tTemp_YMD.month = Temp_Time.tm_mon + 1;
					tTemp_YMD.year = Temp_Time.tm_year + 1900;

					Temp_MJD = CS_DT_YMDtoMJD(tTemp_YMD);
					Temp_UTC = CS_DT_HMtoUTC(tTemp_HM);
					
					// printf("\n ------ Timer Type : %d -----\n\n", stJob_Info.CS_Timer_Type);
					// printf("\n ------ %02d:%02d  %02d/%02d/%04d -----\n\n", tTemp_HM.hour, tTemp_HM.minute, tTemp_YMD.day, tTemp_YMD.month, tTemp_YMD.year);
					FbSetTimer(Temp_MJD, Temp_UTC);
				} else {
					// printf("\n ------  Timer Disable  -----\n\n");
				}
				
				if ( CS_DBU_GetLED_Type() == 1 )
					FbRequestShoutDowm(1);
				else
					FbRequestShoutDowm(0);
			}
			else
			{
				if ( CS_DBU_GetLED_Type() == 1 )
					FbRequestStandBy(1);
				else
					FbRequestStandBy(0);
			}

			break;
#if 0
		case MSG_TIMER:
			if(wparam == CS_SLEEP_TIMER_ID)
			{
				//CurrentTime=CS_DT_UTCtoHM(CS_DT_GetLocalUTC());
				FbSendFndDisplay("----");
				//CS_MW_SetFPTime(CurrentTime.hour,CurrentTime.minute);
				KillTimer(hwnd, CS_SLEEP_TIMER_ID);
				SetTimer(hwnd, CS_SLEEP_TIMER_ID, CS_SLEEP_TIMER_MAX);
			}
			break;
#endif
		case MSG_KEYDOWN:
			switch(wparam)
			{
				case CSAPP_KEY_IDLE:
					// printf("Sleep Mode Power On!!!\n");
					CS_DBU_SetPower_Off_Mode(MV_POWER_ON);
					// printf("Sleep Mode Power On : Set Mode\n");

					/* For AC Power ON Control By KB Kim 2011.03.11 */
					SetPowerStatus(1);
					// printf("Sleep Mode Power On : Set Status\n");

					/* For Scart OFF in Sleep mode by KB Kim 2012.04.07 */
					ScartSbControl(1);
					if (CS_MW_GetTimeMode() == eCS_DBU_TIME_INTERNET)
					{
						MV_OS_Get_Time_From_NTP();
						// printf("Sleep Mode Power On : Get Time from Internet\n");
					}
					SendMessage(hwnd,MSG_CLOSE,0,0);
					break;
				default:
					break;
			}
			break;
			
		case MSG_CLOSE:
			FbRequestNormalMode();
			//CSApp_Wakeup();
			CS_AV_EnableTVOut();
			KillTimer(hwnd,CS_SLEEP_TIMER_ID);
			CSApp_Sleep_Applets = CSApp_Applet_Desktop;
			DestroyMainWindow(hwnd);
			PostQuitMessage(hwnd);
			Cat6611ClearVideoState();
			break;
			
		default:
			break;
	}

	return DefaultMainWinProc(hwnd,message,wparam,lparam);
}

static void CSApp_Sleep(void)
{	
	CS_MW_StopService(TRUE);
	CSOS_DelayTaskMs(200);
	CS_AV_DisableTVOut();
}

/*
static void CSApp_Wakeup(void)
{	
	CS_AV_EnableTVOut();
	MV_MW_StartService(CS_DB_GetCurrentServiceIndex());
}
*/

