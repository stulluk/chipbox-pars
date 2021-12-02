#include "linuxos.h"

#include "db_builder.h"
#include "sys_setup.h"
#include "av_zapping.h"

//#include "mwtypes.h"
//#include "mwOS.h"
#include "mwpublic.h"
#include "mwsetting.h"
#include "mwsvc.h"
#include "mwlayers.h"
#include "fpanel.h"
#include "mv_cfg.h"
#include "userdefine.h"
#include "e2p.h"

#define USE_MV_FRONT
#define	CSAPP_DHCP_ON 	0
#define	CSAPP_DHCP_OFF 	1

#define FONT_TEST  // 한개의 폰트 creation 테스트

#ifdef USE_MV_FRONT
#include "mvfront.h"
#include "mvosapi.h"
#include "mvmiscapi.h"

/* For Front Register Power Key By KB Kim 2011.01.30 */
#include <minigui/merihkey.h>

U32 		FrontIfHandle;

/* For 576i Mode booting problem by KB Kim 20101225 */
U16         DafaultBootVideoDefinition = eCS_DBU_DEFINITION_720P;

/* For Internet Time Problem By KB Kim : 2011.09.15 */
BOOL MV_Get_Network_Status(void);

#ifndef	FONT_TEST
PLOGFONT 	pfutf8[EN_FONT_TYPE8_TTF_MAX][EN_FONT_TYPE8_MAX];
#else
PLOGFONT 	pfutf8;
PLOGFONT 	pfutf8_s;
PLOGFONT 	pfutf8_f;
PLOGFONT 	pfutf8_b;
PLOGFONT 	pfutf8_t;
PLOGFONT	pfutf8_fs;
#endif

U8			acFont_Size[EN_FONT_TYPE8_MAX] = { 16, 18, 20, 22 };
#ifdef USE_CHINESE_FONT
PLOGFONT 	pfutf16be[EN_FONT_TYPE16_MAX];
#endif

typedef enum
{
	FILE_OK=0,
	FILE_NOFILE,
	FILE_READ_FAIL,
	FILE_NO_NAME
} MV_File_Return;

extern MV_File_Return MV_LoadDHCPFile(void);
extern MV_File_Return MV_Load_IPData_File(void);
extern void MV_Setting_Manual_Network(void);
#ifdef CHECK_CH_WATCH
extern void MV_ReSet_Current_Channel_Time(void);
#endif // #ifdef CHECK_CH_WATCH

#define MULTI_FONT

BOOL FbOpenFrontIf(void)
{
	BOOL reslt;

	reslt = FrontIfOpen(&FrontIfHandle);
	if ((reslt != OS_NO_ERROR) || (FrontIfHandle == (U32)NULL))
	{
		FrontIfHandle = (U32)NULL;
		return TRUE;
	}

	return FALSE;
}

BOOL FbSendCommand(U8 command, char *data, U8 len)
{
	char  sendData[FB_COMM_SEND_LENGTH];
	U32   totalLength;
	BOOL  ret;

	if (FrontIfHandle == (U32)NULL)
	{
		printf("FbSendCommand Error : No Front Handle\n");

		return TRUE;
	}

	if ((len + 4) > FB_COMM_SEND_LENGTH)
	{
		printf("FbSendCommand Error : Length[%d] Error\n", len);
		return TRUE;
	}

	sendData[0] = FB_START_CODE;
	sendData[1] = command;
	sendData[2] = len;
	totalLength = 3;
	if (len > 0)
	{
		memcpy(sendData + totalLength, data, len);
		totalLength += (U32)len;
	}
	sendData[totalLength] = FB_END_CODE;
	totalLength++;

	ret = FrontIfWrite(FrontIfHandle, sendData, totalLength);
	// printf("FbSendCommand Info : Send [%d]/[%d]\n", ret, totalLength);

	return ret;
}

BOOL FbSendFndDisplay(char *data)
{
	return FbSendCommand(FB_DISPLAY_FND_ALL, data, 4);
}

BOOL FbSendFndDisplayNum(U32 num)
{
	U32 value;
	U8  data[4];
	if (num > 9999)
	{
		value = num % 10000;
	}
	else
	{
		value = num;
	}

	// printf("FbSendFndDisplayNum Info : Send Data [%d]/[%d]\n", value, num);
#if 0
	data[0] = (U8)((value /1000) + 0x30);  /* Convert First digit to Ascii Code */
	value = value % 1000;
	data[1] = (U8)((value /100) + 0x30);  /* Convert Second digit to Ascii Code */
	value = value % 100;
	data[2] = (U8)((value /10) + 0x30);  /* Convert Third digit to Ascii Code */
	value = value % 10;
	data[3] = (U8)(value  + 0x30);  /* Convert Fourth digit to Ascii Code */
#else
	sprintf(data, "%04d", value);
#endif

	return FbSendCommand(FB_DISPLAY_FND_ALL, data, 4);
}

BOOL FbRquestBootMode(void)
{
	return FbSendCommand(FB_REQUEST_BOOT_MODE, NULL, 0);
}

BOOL FbRequestNormalMode(void)
{
	U8 data;

	data = FB_RUN_NORMAL;
	return FbSendCommand(FB_BOOT_MODE, &data, 1);
}

/* For Front Register Power Key By KB Kim 2011.01.30 */
BOOL FbSetRcuPowerKey(U8 num, U8 *keyData)
{
	U8 data[4];

	data[0] = num;
	data[1] = keyData[0];
	data[2] = keyData[1];
	data[3] = keyData[2];
	return FbSendCommand(FB_SET_RCU, (char *)data, 4);
}

/* For Front Register Power Key By KB Kim 2011.01.30 */
BOOL FbRegistPoweKey(void)
{
	U8 numberOfRcu;
	U8 count;
	U8 pointer;
	U8 rcuPowerScancode[MAX_FRONT_SUPPORT_RCU * 3];
	BOOL result;
	// int dataSize = MAX_FRONT_SUPPORT_RCU * 3;

	pointer = 0;

	// rcuPowerScancode = OsMemoryAllocate(dataSize);
	if (rcuPowerScancode != NULL)
	{
		numberOfRcu = GetRcuPowerKeyCode(rcuPowerScancode);
		// printf("FbRegistPoweKey : numberOfRcu[%d]\n", numberOfRcu);
		if ((numberOfRcu > 0) && (numberOfRcu <= MAX_FRONT_SUPPORT_RCU))
		{
			for(count = 0 ; count < numberOfRcu; count++)
			{
				result = FbSetRcuPowerKey(count, rcuPowerScancode + pointer);
				// printf("Company Code [%d]: 0x%02X%02X, PowerKey : 0x%02X\n", count, rcuPowerScancode[pointer], rcuPowerScancode[pointer + 1], rcuPowerScancode[pointer + 2]);
				pointer += 3;
				if (result == TRUE)
				{
					return result;
				}
			}
		}
		// OsMemoryFree(rcuPowerScancode);
		// rcuPowerScancode == NULL;
	}

	return FALSE;
}

BOOL FbRequestStandBy(U8 displayOn)
{
	U8 data;

	data = FB_STAND_BY | (displayOn << 1);
	return FbSendCommand(FB_BOOT_MODE, &data, 1);
}

/* Real Power Off */
BOOL FbRequestShoutDowm(U8 displayOn)
{
	U8 data;

	/* For Front Register Power Key By KB Kim 2011.01.30 */
	FbRegistPoweKey();

	data = FB_SHUT_DOWN | (displayOn << 1);
	return FbSendCommand(FB_BOOT_MODE, &data, 1);
}

BOOL FbSetTime(void)
{
	U8  data[7];
	struct timespec time_value;
	struct tm		tm_time;

	clock_gettime(CLOCK_REALTIME, &time_value);
	memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));
	tm_time.tm_year += 1900;
	tm_time.tm_mon++;

	data[0] = (U8)((tm_time.tm_year >> 8) & 0xFF);
	data[1] = (U8)(tm_time.tm_year & 0xFF);
	data[2] = (U8)tm_time.tm_mon;
	data[3] = (U8)tm_time.tm_mday;
	data[4] = (U8)tm_time.tm_hour;
	data[5] = (U8)tm_time.tm_min;
	data[6] = (U8)tm_time.tm_sec;


	printf("Front Set Time : %04d/%02d/%02d %02d:%02d:%02d \n", tm_time.tm_year, data[2], data[3], data[4], data[5], data[6]);
	return FbSendCommand(FB_SET_CURRENT_TIME, (char *)data, 7);
}

BOOL FbSetTimer(U16 date, U16 time)
{
	tCS_DT_Date currentDate;
	tCS_DT_Time currentTime;
	U8  data[7];

	currentDate = CS_DT_MJDtoYMD(date);
	currentTime = CS_DT_UTCtoHM(time);
	data[0] = (U8)((currentDate.year >> 8) & 0xFF);
	data[1] = (U8)(currentDate.year & 0xFF);
	data[2] = currentDate.month;
	data[3] = currentDate.day;
	data[4] = currentTime.hour;
	data[5] = currentTime.minute;
	data[6] = 0; /* Set sec to 0 */

	printf("Front Set Timer : %04d/%02d/%02d %02d:%02d:%02d \n", currentDate.year, data[2], data[3], data[4], data[5], data[6]);
	return FbSendCommand(FB_SET_TIMER, (char *)data, 7);
}

BOOL FbSetTestTimer(void)
{
	tCS_DT_Time currentTime;
	U16         currentMjd;
	U16         currentUtc;

	currentMjd  = CS_DT_GetLocalMJD();
	currentTime = CS_DT_UTCtoHM(CS_DT_GetLocalUTC());

	currentTime.minute += 2; /* Add 2 Minute */
	if (currentTime.minute >= 60)
	{
		currentTime.hour ++;
		currentTime.minute -= 60;
		if (currentTime.hour >= 24)
		{
			currentMjd++;
			currentTime.hour -= 24;
		}
	}
	currentUtc = CS_DT_HMtoUTC(currentTime);

	return FbSetTimer(currentMjd, currentUtc);
}

BOOL FbSetFrontDisplay(U8 level)
{
	return FbSendCommand(FB_SET_FRONT_LEVEL, &level, 1);
}

BOOL FbSetFrontClock(U8 on)
{
	return FbSendCommand(FB_SET_FRONT_CLOCK, &on, 1);
}

BOOL FbSendBootEnd(void)
{
	return FbSendCommand(FB_CLEAR_BOOT_END, NULL, 0);
}

BOOL FbStartWatchdog(U8 time)
{
	U8 data;
	data = time;
	return FbSendCommand(FB_WATCHDOG_START, &data, 1);
}

BOOL FbStopWatchdog(void)
{
	return FbSendCommand(FB_WATCHDOG_STOP, NULL, 0);
}

BOOL FbSendKick(void)
{
	return FbSendCommand(FB_WATCHDOG_KICK, NULL, 0);
}

/* For Front Communication By KB Kim 2011.02.01 */
int FbReceiveFrontData(U8 *data)
{
	int length;
	int count;

	count = 0;
	do
	{
		length = FbGetFrontData(data);
		if (length > 0)
		{
			return length;
		}
		OsWaitMillisecond(100);
		count++;
	}
	while(count < FB_COMM_MAX_WAIT);

	return 0;
}

/* For Front Communication By KB Kim 2011.02.01 */
U8 FbGetBootMode(void)
{
	U8             data[FB_COMM_SEND_LENGTH];
	int            length;
	SystemBootMede bootMode;

	FbRquestBootMode();

	length = FbReceiveFrontData(data);
	// printf("FbGetBootMode : %d - 0x%02X, 0x%02X, 0x%02X\n", length, data[0], data[1], data[2]);
	if ((length < 3) || (data[0] != FB_BOOT_MODE))
	{
		bootMode = BOOT_MODE_UNKNOWN;
	}
	else
	{
		switch(data[2])
		{
			case FB_BOOT_MODE_NORMAL :
				bootMode = BOOT_NORMAL;
				break;

			case FB_BOOT_MODE_IR_NORMAL :
				bootMode = BOOT_NORMAL;
				break;

			case FB_BOOT_MODE_AC_ON :
				bootMode = BOOT_AC_POWER_ON;
				break;

			case FB_BOOT_MODE_TIMER :
				bootMode = BOOT_TIMER;
				break;

			case FB_BOOT_MODE_WATCH :
				bootMode = BOOT_WATCHDOG;
				break;

			default :
				bootMode = BOOT_MODE_UNKNOWN;
				break;
		}
	}

	return (U8)bootMode;
}

#else  // #ifdef USE_MV_FRONT
#if 1
static unsigned char fp_letter_code[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xff */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x10 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x20 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff,
	0x03, 0x9f, 0x25, 0x0d, 0x99, 0x49, 0x41, 0x1f, /* 0x30 */
	0x01, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x11, 0xff, 0x63, 0xff, 0x61, 0x71, 0xff, /* 0x40 */
	0x91, 0xdf, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff,
	0x31, 0xff, 0xff, 0xff, 0x73, 0x83, 0xff, 0xff, /* 0x50 */
	0xff, 0x89, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x60 */
	0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x31, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x70 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x80 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x90 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xa0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xb0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xc0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xd0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xe0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xf0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
#endif

#ifndef USE_MV_FRONT // by KB Kim 2010.08.31 for GPIO2
static CSFP_HANDLE fp_handle = NULL;

void key_notify(CSFP_HANDLE handle, CSFP_NOTIFYEVENT * event , unsigned char * flag)
{
    U16 def_index = 0;

 //  printf("event = %d, flag = 0x%x\n", * event, * flag);

    if((* event == CSFP_RCBYTE_IN)&&(* flag == 0x11))   //sat
        {
            def_index = CS_MW_GetVideoDefinition();

            def_index++;

            if(def_index > eCS_DBU_DEFINITION_1080I)
                def_index = 0;

            CS_MW_SetVideoDefinition(def_index);

            //BroadcastMessage (MSG_PAINT, 11, 0);

            CS_DBU_SaveUserSettingDataInHW();
        }
}
#endif // #ifndef USE_MV_FRONT

#if	0
static void Fpanel_Notify(PanelT_KeyEvent iKeyEvent, unsigned char bKeyCode)
{
	printf("iKeyEvent=%d  bKeyCode=0x%x\n",iKeyEvent,bKeyCode);
}
#endif

#endif // #ifdef USE_MV_FRONT

/* For 576i Mode booting problem by KB Kim 20101225 */
void CS_MW_Init_VideoDefinition(U8 mode)
{
	U16 currentVideoDefinition;

	if (mode)
	{
		currentVideoDefinition = CS_MW_GetVideoDefinition();
		// printf("CS_MW_Init_VideoDefinition : current = %d, boot Def = %d\n", currentVideoDefinition, DafaultBootVideoDefinition);
		if (currentVideoDefinition != DafaultBootVideoDefinition)
		{
			// printf("Current Def [%d] is not same with Default Def [%d]\n", currentVideoDefinition, DafaultBootVideoDefinition);
			CS_MW_SetVideoDefinition(DafaultBootVideoDefinition);
		}
	}
	else
	{
		DafaultBootVideoDefinition = CS_MW_GetVideoDefinition();
		// printf("CS_MW_Init_VideoDefinition : boot Def = %d\n", DafaultBootVideoDefinition);
		CS_MW_SetVideoDefinition(eCS_DBU_DEFINITION_720P);
		/*
		if (DafaultBootVideoDefinition == eCS_DBU_DEFINITION_576I)
		{
			CS_MW_SetVideoDefinition(eCS_DBU_DEFINITION_720P);
		}
		else
		{
			CS_MW_SetVideoDefinition(DafaultBootVideoDefinition);
		}
		*/
	}
}

void Check_Mac_Address(void)
{
	char		command_shell[100];
	char		MacAddress[MAC_SIZE];
	char		MAC[MAC_SIZE] = {0x02,0x03,0x04,0x00,0x00,0x00};
	char 		tempSection [CFG_MAX_COL];
	char		Splite_Str[3];
	int			Shell_Result = 0;
	FILE*		obj_file;
	U8			TempU8;
	int			i = 0, j = 0;

	memset(MacAddress, 0x00, MAC_SIZE);
	memset(tempSection, 0x00, CFG_MAX_COL);

	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0x114, MacAddress, 6);

//	printf("\n\nMAC 1 : %02x:%02x:%02x:%02x:%02x:%02x\n", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);
//	printf("MAC 2 : %02x:%02x:%02x:%02x:%02x:%02x\n\n\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);

	if ( memcmp(&MacAddress, &MAC, MAC_SIZE) == 0 )
	{
		/* wget -O /tmp/mac.txt "http://www.merihvideo.com.tr/chipbox/mac.php?IDT=MRH&stb_id=1" */
		memset(command_shell, 0x00, 100);
		system("udhcpc -n -t 5 -T 2");
		sprintf(command_shell, "wget -O /tmp/mac.txt \"http://www.merihvideo.com.tr/chipbox/mac.php?IDT=MRH&stb_id=1\"");
		Shell_Result = system(command_shell);

		if ( Shell_Result == 0 )
		{
			if (!(obj_file = fopen("/tmp/mac.txt", "r")))
			{
		         // printf("there is no /tmp/mac.txt =====\n\n\n");
				 return;
			}

			if ( !fgets(tempSection, CFG_MAX_COL, obj_file) )
			{
				// printf("/tmp/mac.txt has no data =====\n\n\n");
				fclose (obj_file);
				return;
			}
//			printf("DownLoad MAC ADD ===>>> %s\n\n\n", tempSection);
			fclose (obj_file);

			memset(Splite_Str, 0x00, 3);
			for ( i = 0 ; i < (int)strlen(tempSection) ; i++ )
			{
				Splite_Str[i%2] = tempSection[i];
				if ( i%2 == 1 && i != 0 )
				{
					Str2Hex(Splite_Str, &TempU8);
//					printf("i : %02d , j : %d ===== %s = %02x\n", i, j, Splite_Str, TempU8);
					MacAddress[j] = TempU8;
					j++;
				}
			}
			// printf("\n\nMAC 3 : %02x:%02x:%02x:%02x:%02x:%02x\n", MacAddress[0], MacAddress[1], MacAddress[2], MacAddress[3], MacAddress[4], MacAddress[5]);

			if ( memcmp(&MacAddress, &MAC, MAC_SIZE) != 0 )
			{
				E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0x114, MacAddress, 6);
				system("reboot");
			}
		} else {
			// printf("Wget Error =====\n\n\n");
				return;
		}
	}
	else
	{
		// printf("MAC is Correct =====\n\n\n");
		return;
	}
}

static pthread_t  	hAni_TaskHandle;

void Disc_Rdate_Stop(void)
{
	pthread_cancel( hAni_TaskHandle );
}

void *Disk_Rdate_Task( void *param )
{
	int 			Result = 0;
	struct tm		tm_time;
	tCS_DT_Time		time_HM;
	tCS_DT_Date 	date_ymd;
	struct timespec time_value;

	/* For Internet Time Problem By KB Kim : 2011.09.15 */
	if (MV_Get_Network_Status() != TRUE)
	{
		// printf("Disk_Rdate_Task : Get NTP Time Network Error ======\n");
		Disc_Rdate_Stop();

		return ( param );
	}

	printf("ntpclient ile deneniyor...\n");
	Result = system( "ntpclient -c 1 -s -h tr.pool.ntp.org" );
	printf("Tarih:");
	system("date");

	/*if ( Result != 0 )
		Result = system( "rdate -s time-b.nist.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s time-a.timefreq.bldrdoc.gov" );

	if ( Result != 0 )
		Result = system( "rdate -s tick.ucla.edu" );*/

	if ( Result == 0 )
	{
		time_value = MV_OS_Time_Trans();

		memcpy(&tm_time, localtime(&time_value.tv_sec), sizeof(tm_time));

		date_ymd.year = tm_time.tm_year + 1900;
		date_ymd.month = tm_time.tm_mon + 1;
		date_ymd.day = tm_time.tm_mday;
		time_HM.hour = tm_time.tm_hour;
		time_HM.minute = tm_time.tm_min;

		if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
			// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
			printf("clock_settime :: Error\n");
		}

		CS_DT_ManualSetDateAndTime(CS_DT_YMDtoMJD(date_ymd), CS_DT_HMtoUTC(time_HM));
		// CS_DBU_SaveUserSettingDataInHW(); /* By KB Kim 20110712 */
		// printf("===Task %04d / %02d / %02d ==> %02d : %02d   :  %ld\n", date_ymd.year, date_ymd.month, date_ymd.day, time_HM.hour, time_HM.minute, time_value.tv_sec);
	}
	/*
	else
	{
		printf("Get NTP Time Error ======\n");
	}
	*/

	Disc_Rdate_Stop();

	return ( param );
}

int Disc_Rdate_Init(void)
{
	pthread_create( &hAni_TaskHandle, NULL, Disk_Rdate_Task, NULL );
	return( 0 );
}

/* For AC Power ON Control By KB Kim 2011.03.11 */
U8 GetPowerStatus(void)
{
	U8		powerStatus[4];

	memset(powerStatus, 0x00, 4);

	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0, powerStatus, 1);

	// printf("\n\n\n GetPowerStatus = %d ============ \n\n\n", powerStatus[0]);

	return powerStatus[0];
}


/* For AC Power ON Control By KB Kim 2011.03.11 */
void SetPowerStatus(U8 val)
{
	U8		powerStatus[4];

	memset(powerStatus, 0x00, 4);
	powerStatus[0] = val;

	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), 0, powerStatus, 1);
}

BOOL CS_MW_Init(void)
{
	MV_CFG_RETURN 	ret;
	U16				Current_TimeZone;

	/* For EEPROM access problem by KB Kim 2011.09.13 */
	E2P_Init();

	Check_Mac_Address();

	ret = MV_LoadCFGFile();

	//printf("\n\n\n %d ============ \n\n\n", ret);
	// printf("\n\n\n %s = %d =========== \n\n\n", CFG_Resource, CFG_PNG_Test);

#ifdef USE_MV_FRONT
	FbOpenFrontIf();
	FbSendFndDisplay("Load");

#else //#ifdef USE_MV_FRONT

#if	0
{
     PanelT_InitParams	PanelParas;

    fp_handle = CSFP_Open();
    CSFP_DisableKeyScan(fp_handle);
    CSFP_SetLEDDisplayMode(fp_handle,CSFP_LEDDISP_OFF);
    CSFP_Close(fp_handle);


    PanelParas.bTaskPriority=0;
    PanelParas.PanelKeyProc=Fpanel_Notify;
    PanelF_Initialize(&PanelParas);
}
#else
	fp_handle = CSFP_Open();

	//system("cat /dev/input/event0 |hexdump");
	CSFP_EnableKeyScan(fp_handle);

	//CSFP_EnableSystemCode(fp_handle);

	CSFP_SetSystemCode(fp_handle, 0x00);

	CS_MW_FrontPanelDisplay("----");

	CSFP_SetNotify(fp_handle, key_notify,CSFP_RCBYTE_IN, 1);

	//CS_MW_SetKeyMap( 0x0, 0x48 );
#endif

#endif // #ifdef USE_MV_FRONT

	if ( !CFG_Factory_Mode )
	{
		//printf("\n\n ============= Normal Booting ================ \n\n");
	    CS_DRV_Init();
		CS_MW_Font_Creation(1);
	    CS_MW_SetAspectRatio(CS_MW_GetAspectRatio());
	    CS_MW_SetAspectMode(CS_MW_GetAspectMode());
		/* For By KB Kim 20110604 */
	    /* For 576i Mode booting problem by KB Kim 20101225 */
		CS_MW_Init_VideoDefinition(0);
		AdjustVideoWindows();
	    CS_AV_Play_IFrame(BOOT_LOGO);
		Current_TimeZone = CS_MW_GetTimeZone();
	    CS_SYS_Init();
	    CS_MW_ValidCurrentSetting();

#ifdef CHECK_CH_WATCH
		MV_ReSet_Current_Channel_Time();
#endif // #ifdef CHECK_CH_WATCH
		if (CS_MW_GetTimeMode() == eCS_DBU_TIME_INTERNET)
		{
			//MV_OS_Get_Time_From_NTP();
			Disc_Rdate_Init();
		}

		CS_MW_TTXSUBInit();
		CS_MW_SVC_Init();
#if 0
		if (CS_MW_GetTimeMode() == eCS_DBU_TIME_INTERNET)
		{
			struct timespec		time_value;

			clock_gettime(CLOCK_REALTIME, &time_value);
			if ( time_value.tv_sec < 1296577417 ) /* Time small than 01/02/2011 16:23 */
				MV_OS_Get_Time_From_NTP();
		}
#endif
	}
	else
	{
		CS_DRV_Init();

	    CS_MW_SetAspectRatio(CS_MW_GetAspectRatio());
	    CS_MW_SetAspectMode(CS_MW_GetAspectMode());
		CS_MW_Init_VideoDefinition(0);
		AdjustVideoWindows();
	    // by KB Kim 2010.08.31 for AV Setting
	    /*
	    CS_MW_SetVideoDefinition(CS_MW_GetVideoDefinition());
		AdjustVideoWindows();
		*/

	    //CS_AV_Play_IFrame("/home/resource/bootlogo.mpg");

	    CS_SYS_Init();
	    CS_MW_ValidCurrentSetting();
		CS_MW_SVC_Init();
	}

    return TRUE;
}


void CS_MW_TextOut( HDC hdc, U32 x, U32 y, char* text )
{
#if 0
	if (*(text) == 0x11)
	{
		pfutf16be = CreateLogFontByName("*-fixed-rrncnn-*-24-UTF-16BE");
		if(!pfutf16be)
			CS_MW_DbgPrintf("\ncreate font *-fixed-rrncnn-*-24-UTF-16BE error!\n");
		tmp = SelectFont((hdc), pfutf16be);
		if(!tmp)
			CS_MW_DbgPrintf("\nselect font error!\n");
		TextOut((hdc), (x), (y), ((text)+1));
		SelectFont((hdc), tmp);
		DestroyLogFont(pfutf16be);
	}
	else
	{
		pfutf8 = CreateLogFontByName("*-fixed-rrncnn-*-24-UTF-8");
		if(!pfutf8)
			CS_MW_DbgPrintf("\ncreate font *-fixed-rrncnn-*-24-UTF-8 error!\n");
		tmp = SelectFont((hdc), pfutf8);
		if(!tmp)
			CS_MW_DbgPrintf("\nselect font error!\n");

    	if (*(text) == 0x15)
    	{
    			TextOut((hdc), (x), (y), ((text)+1));
    	}
    	else
    	{
    			TextOut((hdc), (x), (y), (text));
		}
		SelectFont((hdc), tmp);
		DestroyLogFont(pfutf8);
   }
#else
	PLOGFONT tmp;

	//printf("\n\n\n CS_MW_TextOut FONT : TYPE : %d , SIZE : %d \n\n\n", CS_DBU_GetFont_Type(), CS_DBU_GetFont_Size());

#ifndef FONT_TEST
	tmp = SelectFont((hdc), pfutf8[CS_DBU_GetFont_Type()][CS_DBU_GetFont_Size()]);
#else
	tmp = SelectFont((hdc), pfutf8);
#endif

	if(!tmp)
		printf("\nselect font error!\n");

	//printf("CS_MW_TextOut : ======= DREAM_FONT : %x, %s\n", *(text), text);

    if (*(text) <0x20)
	{
		/* No Text Test */
		TextOut((hdc), (x), (y), ((text)+1));
	}
	else
	{

		/* No Text Test */
		TextOut((hdc), (x), (y), (text));
	}

	SelectFont((hdc), tmp);
#endif
}

void CS_MW_TextOut_Static( HDC hdc, U32 x, U32 y, char* text )
{
	PLOGFONT tmp;

	//printf("\n\n\n CS_MW_TextOut FONT : TYPE : %d , SIZE : %d \n\n\n", CS_DBU_GetFont_Type(), CS_DBU_GetFont_Size());

	tmp = SelectFont((hdc), pfutf8_s);

	if(!tmp)
		printf("\nselect font error!\n");

	//printf("CS_MW_TextOut : ======= DREAM_FONT : %x, %s\n", *(text), text);

    if (*(text) <0x20)
	{
		/* No Text Test */
		TextOut((hdc), (x), (y), ((text)+1));
	}
	else
	{

		/* No Text Test */
		TextOut((hdc), (x), (y), (text));
	}

	SelectFont((hdc), tmp);
}

#define DREAM_FONT

void minigui_check_print_font(void)
{
	PLOGFONT 	pfutf8;
	int			i;

	for ( i = 0 ; i < NR_SYSLOGFONTS ; i++ )
	{
		pfutf8 = GetSystemFont (i);
		// printf("\n%s ## %s ##############!\n", pfutf8->mbc_devfont->name, pfutf8->sbc_devfont->name);
	}
}

void CS_MW_DrawText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
#ifndef DREAM_FONT
    PLOGFONT pfutf8;
    PLOGFONT pfutf16be;
#endif // #ifndef DREAM_FONT
    PLOGFONT tmp;

#if 1 /// For TEST
	int		i, j = 0, k = 0;
	/* For factory fedault problem under Turkish language */
	char 	Temp[256];

	k = strlen(text);
	memset(Temp, 0x00, 256);

	//printf("\n1 ===== %s =======\n", text);
	for ( i = 0 ; i < k ; i++ )
	{
		if ( text[i] < 0x80 || text[i] > 0x90 )
		{
			Temp[j] = text[i];
			j++;
		}
	}
	//printf("\n2 ===== %s =======\n", text);
#endif

#ifdef USE_CHINESE_FONT
	if (*(Temp) == 0x11)
	{
#ifdef DREAM_FONT
		tmp = SelectFont((hdc), pfutf16be[EN_FONT_TYPE16_FIXED_16]);
		if(!tmp)
			CS_MW_DbgPrintf("\nselect font error!\n");
		/* No Text Test */
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
		SelectFont((hdc), tmp);
#else  //#ifdef DREAM_FONT
		pfutf16be = CreateLogFontByName("*-fixed-rrncnn-*-24-UTF-16BE");

		if(!pfutf16be)
			printf("\ncreate font *-fixed-rrncnn-*-24-UTF-16BE error!\n");
			//CS_MW_DbgPrintf("\ncreate font *-fixed-rrncnn-*-24-UTF-16BE error!\n");

		tmp = SelectFont((hdc), pfutf16be);
		if(!tmp)
			CS_MW_DbgPrintf("\nselect font error!\n");
		/* No Text Test */
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
		SelectFont((hdc), tmp);
		DestroyLogFont(pfutf16be);
#endif  //#ifdef DREAM_FONT
	}
	else
#endif  //#ifdef USE_CHINESE_FONT
	{
#ifdef DREAM_FONT
		if( *(Temp) == 0x11 )
			return;
		//printf("\n\n\n CS_MW_DrawText FONT : TYPE : %d , SIZE : %d \n\n\n", CS_DBU_GetFont_Type(), CS_DBU_GetFont_Size());
#ifndef FONT_TEST
		tmp = SelectFont((hdc), pfutf8[CS_DBU_GetFont_Type()][CS_DBU_GetFont_Size()]);
#else
		tmp = SelectFont((hdc), pfutf8);
#endif
		if(!tmp)
			printf("\nselect font error!\n");

		if( *(Temp) == 0x15 )
		{
			/* No Text Test */
			DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
		}
		else
		{
			/* No Text Test */
			DrawText((hdc), (Temp), nCount, pRect, nFormat);
		}
		SelectFont((hdc), tmp);
#else //#ifdef DREAM_FONT
		pfutf8 = CreateLogFontByName("*-fixed-rrncnn-*-16-UTF-8");
		//pfutf8 = INV_LOGFONT;

		if(!pfutf8)
			printf("\ncreate font *-fixed-rrncnn-*-24-UTF-8 error!\n");
			//CS_MW_DbgPrintf("\ncreate font *-fixed-rrncnn-*-24-UTF-8 error!\n");

		//printf("\n%s ## %s ##############!\n", pfutf8->mbc_devfont->name, pfutf8->sbc_devfont->name);

		tmp = SelectFont((hdc), pfutf8);
		if(!tmp)
			printf("\nselect font error!\n");
			//CS_MW_DbgPrintf("\nselect font error!\n");
		//printf("\n2. %s ## %s ##############!\n", tmp->mbc_devfont->name, tmp->sbc_devfont->name);


		if( *(text) == 0x15 )
		{
			/* No Text Test */
			DrawText((hdc), ((text)+1), nCount, pRect, nFormat);
		}
		else
		{
			/* No Text Test */
			DrawText((hdc), (text), nCount, pRect, nFormat);
		}
#if 1
		SelectFont((hdc), tmp);
#else
		SelectFont((hdc), pfutf8);
		DestroyLogFont(pfutf8);
#endif
#endif //#ifdef DREAM_FONT
	}
}

void MV_MW_DrawText_Static( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
    PLOGFONT tmp;

	int		i, j = 0, k = 0;
	/* For factory fedault problem under Turkish language */
	char 	Temp[256];

	k = strlen(text);
	memset(Temp, 0x00, 256);

	for ( i = 0 ; i < k ; i++ )
	{
		if ( text[i] < 0x80 || text[i] > 0x90 )
		{
			Temp[j] = text[i];
			j++;
		}
	}

	if( *(Temp) == 0x11 )
		return;

	tmp = SelectFont((hdc), pfutf8_s);

	if(!tmp)
		printf("\nselect font error!\n");

	if( *(Temp) == 0x15 )
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
	else
		DrawText((hdc), (Temp), nCount, pRect, nFormat);

	SelectFont((hdc), tmp);
}

void MV_MW_DrawText_Fixed( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
    PLOGFONT tmp;

	int		i, j = 0, k = 0;
	/* For factory fedault problem under Turkish language */
	char 	Temp[256];

	k = strlen(text);
	memset(Temp, 0x00, 256);

	for ( i = 0 ; i < k ; i++ )
	{
		if ( text[i] < 0x80 || text[i] > 0x90 )
		{
			Temp[j] = text[i];
			j++;
		}
	}

	if( *(Temp) == 0x11 )
		return;

	tmp = SelectFont((hdc), pfutf8_f);

	if(!tmp)
		printf("\nselect font error!\n");

	if( *(Temp) == 0x15 )
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
	else
		DrawText((hdc), (Temp), nCount, pRect, nFormat);

	SelectFont((hdc), tmp);
}

void MV_MW_DrawText_Fixed_Small( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
    PLOGFONT tmp;

	int		i, j = 0, k = 0;
	/* For factory fedault problem under Turkish language */
	char 	Temp[256];

	k = strlen(text);
	memset(Temp, 0x00, 256);

	for ( i = 0 ; i < k ; i++ )
	{
		if ( text[i] < 0x80 || text[i] > 0x90 )
		{
			Temp[j] = text[i];
			j++;
		}
	}

	if( *(Temp) == 0x11 )
		return;

	tmp = SelectFont((hdc), pfutf8_fs);

	if(!tmp)
		printf("\nselect font error!\n");

	if( *(Temp) == 0x15 )
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
	else
		DrawText((hdc), (Temp), nCount, pRect, nFormat);

	SelectFont((hdc), tmp);
}

void MV_MW_DrawText_Title( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
    PLOGFONT tmp;

	int		i, j = 0, k = 0;
	/* For factory fedault problem under Turkish language */
	char 	Temp[256];

	k = strlen(text);
	memset(Temp, 0x00, 256);

	for ( i = 0 ; i < k ; i++ )
	{
		if ( text[i] < 0x80 || text[i] > 0x90 )
		{
			Temp[j] = text[i];
			j++;
		}
	}

	if( *(Temp) == 0x11 )
		return;

	tmp = SelectFont((hdc), pfutf8_t);

	if(!tmp)
		printf("\nselect font error!\n");

	if( *(Temp) == 0x15 )
		DrawText((hdc), ((Temp)+1), nCount, pRect, nFormat);
	else
		DrawText((hdc), (Temp), nCount, pRect, nFormat);

	SelectFont((hdc), tmp);
}


void MV_MW_DrawText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
//    PLOGFONT pfutf16be;
    PLOGFONT tmp;

#if 0
	pfutf16be = CreateLogFontByName("ttf-arial-rrncnn-*-24-ISO8859-1");
	if(!pfutf16be)
		CS_MW_DbgPrintf("\ncreate ttf-arial-rrncnn-*-24-ISO8859-1 error!\n");
	tmp = SelectFont((hdc), pfutf16be);
	if(!tmp)
		printf("\nselect font error!\n");
#else

#ifndef FONT_TEST
	tmp = SelectFont((hdc), pfutf8[CS_DBU_GetFont_Type()][EN_FONT_TYPE8_12]);
#else
	tmp = SelectFont((hdc), pfutf8_s);
#endif

	if(!tmp)
		printf("\nselect font error!\n");
#endif

	if( *(text) == 0x15 )
	{
		/* No Text Test */
		DrawText((hdc), ((text)+1), nCount, pRect, nFormat);
	}
	else
	{
		/* No Text Test */
		DrawText((hdc), (text), nCount, pRect, nFormat);
	}
	SelectFont((hdc), tmp);
}

void MV_MW_DrawBigText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat )
{
//    PLOGFONT pfutf16be;
    PLOGFONT tmp;

	tmp = SelectFont((hdc), pfutf8_b);

	if(!tmp)
		printf("\nselect font error!\n");

	if( *(text) == 0x15 )
		DrawText((hdc), ((text)+1), nCount, pRect, nFormat);
	else
		DrawText((hdc), (text), nCount, pRect, nFormat);

	SelectFont((hdc), tmp);
}

extern U8		Font_Size[EN_FONT_TYPE8_MAX];

BOOL CS_MW_Font_Creation(BOOL Start_Time)
{
	FILE* 	fp;
    char 	tempSection [1024 + 2];
	char	Tmp_Str[256];
	char	Tmp_Str2[256];
	U8		i = 0, j = 0;

	//printf("\n\n=======================================================\n");
	//printf("                   FONT CREATION\n");
	//printf("=======================================================\n\n\n");

	if (!(fp = fopen(FONT_FILE, "r")))
	{
		printf("\nCAN NOT OPEN FONT CONFIG FILE\n");
         return 0;
	}

	while (!feof(fp)) {
		if(!fgets(tempSection, 1024, fp))
			break;

		memset(Tmp_Str, 0x00, 100);

		while (tempSection[j] != '\n')
		{
			Tmp_Str[j] = tempSection[j];
			j++;
		}

		strcpy(acFont_Name[i], Tmp_Str);
		i++;
		j = 0;
    }

	if ( CS_DBU_GetFont_Type() > i )
		CS_DBU_SetFont_Type(0);

	u8Font_Kind = i;
	fclose (fp);

	sprintf(Tmp_Str, "*-fixed-rrncnn-*-24-ISO8859-1");
	pfutf8_fs = CreateLogFontByName(Tmp_Str);

	if ( CS_DBU_GetFont_Type() == 0 )
	{
		sprintf(Tmp_Str, "*-fixed-rrncnn-*-%d-ISO8859-1", Font_Size[CS_DBU_Get_Fixed_Font_Size()]);
		sprintf(Tmp_Str2, "*-fixed-rrncnn-*-32-ISO8859-1");
	}
	else
	{
		sprintf(Tmp_Str, "ttf-%s-rrncnn-*-%d--ISO8859-1", acFont_Name[CS_DBU_GetFont_Type()], CS_DBU_GetFont_Size());
		sprintf(Tmp_Str2, "ttf-%s-rrncnn-*-27--ISO8859-1", acFont_Name[CS_DBU_GetFont_Type()]);
	}

	pfutf8 = CreateLogFontByName(Tmp_Str);
	pfutf8_t = CreateLogFontByName(Tmp_Str2);

	if ( Start_Time )
	{
		if ( CS_DBU_GetFont_Type() == 0 )
			sprintf(Tmp_Str, "*-fixed-rrncnn-*-24-ISO8859-1");
		else
			sprintf(Tmp_Str, "ttf-%s-rrncnn-*-22--ISO8859-1", acFont_Name[CS_DBU_GetFont_Type()]);
		pfutf8_s = CreateLogFontByName(Tmp_Str);
	}
	else
	{
		if ( CS_DBU_GetFont_Type() == 0 )
			sprintf(Tmp_Str, "*-fixed-rrncnn-*-24-ISO8859-1");
		else
			sprintf(Tmp_Str, "ttf-%s-rrncnn-*-22--ISO8859-1", acFont_Name[CS_DBU_GetFont_Type()]);

		pfutf8_s = CreateLogFontByName(Tmp_Str);
	}

	sprintf(Tmp_Str, "*-fixed-rrncnn-*-32-ISO8859-1");
	pfutf8_f = CreateLogFontByName(Tmp_Str);

	sprintf(Tmp_Str, "ttf-%s-rrncnn-*-40--ISO8859-1", acFont_Name[CS_DBU_GetFont_Type()]);
	pfutf8_b = CreateLogFontByName(Tmp_Str);

#ifdef USE_CHINESE_FONT
	pfutf16be[EN_FONT_TYPE16_FIXED_12] = CreateLogFontByName("*-fixed-rrncnn-*-12-UTF-16BE");
	if(!pfutf16be[EN_FONT_TYPE16_FIXED_12])
		printf("\ncreate font *-fixed-rrncnn-*-12-UTF-16BE error!\n");

	pfutf16be[EN_FONT_TYPE16_FIXED_16] = CreateLogFontByName("*-fixed-rrncnn-*-16-UTF-16BE");
	if(!pfutf16be[EN_FONT_TYPE16_FIXED_12])
		printf("\ncreate font *-fixed-rrncnn-*-12-UTF-16BE error!\n");

	pfutf16be[EN_FONT_TYPE16_FIXED_24] = CreateLogFontByName("*-fixed-rrncnn-*-24-UTF-16BE");
	if(!pfutf16be[EN_FONT_TYPE16_FIXED_12])
		printf("\ncreate font *-fixed-rrncnn-*-12-UTF-16BE error!\n");

	pfutf16be[EN_FONT_TYPE16_FIXED_30] = CreateLogFontByName("*-fixed-rrncnn-*-30-UTF-16BE");
	if(!pfutf16be[EN_FONT_TYPE16_FIXED_12])
		printf("\ncreate font *-fixed-rrncnn-*-12-UTF-16BE error!\n");
#endif
	return FALSE;
}

#ifndef USE_MV_FRONT

BOOL CS_MW_FrontPanelDisplay( char* str )
{
    char LEDDisplayBuff[5];

    U8  i;

    if( str == NULL ) return FALSE;

    CSFP_SetLEDDisplayMode(fp_handle, CSFP_LEDDISP_ON);
    CSFP_SetLEDDisplayPos(fp_handle, 6, 0);

    CS_MW_MemSet( LEDDisplayBuff, 0, 5 );

    for(i=0;i<4;i++)
    {
        U8  temp;

        temp = str[i];

        LEDDisplayBuff[i] = fp_letter_code[temp];
    }

    LEDDisplayBuff[4] = 0;

    if( CSAPI_SUCCEED == CSFP_SetLEDDisplayRaw(fp_handle, LEDDisplayBuff, 4))
		printf("Set Led Charecters Success\n");
    else
		printf("Set Led Charecters Error\n");

	return TRUE;
}
#endif // #ifndef USE_MV_FRONT

#if 0
BOOL CS_MW_SetKeyMap( U16 KeyIndex, U8 KeyValue )
{
	int key_map[0x1ff], key_num = 0x1ff;

	IAL_GetKeyMap( key_map, &key_num );

        //printf("default key_value = 0x%x\n", key_map[ KeyIndex ]);

	if( KeyIndex > key_num ) return FALSE;

	key_map[ KeyIndex ] = KeyValue;
	IAL_SetKeyMap( key_map, key_num );

	return TRUE;
}
#endif


BOOL CS_MW_USBDeviceConnect(void)
{
#if 0
    int     fp_usbdevice;
    int     retval;
    char    usb_status;

    fp_usbdevice = fopen( "/proc/usb_conn_status", "r" );

    if( fp_usbdevice <= 0 )
    {
    	return(FALSE);
    }

    retval = fread( &usb_status, 1, 1, fp_usbdevice );
    if( retval != 1 )
    {
        fclose(fp_usbdevice);
        return(FALSE);
    }


    CS_MW_DbgPrintf( "USB Device Connected %c\n", usb_status );
    fclose(fp_usbdevice);

    if( usb_status == '1' )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
#else
    return(TRUE);
#endif
}


BOOL CS_MW_USBDeviceFWUpdate(void)
{
#if 0
    if ( CS_MW_USBDeviceConnect() != TRUE ) return FALSE;

    CS_MW_Delayms(3000);
    system("mount -t vfat -o iocharset=cp936 /dev/ub/a/part1 /tmp");

    if( access( "/tmp/csapp", 0 ) != -1 )
    {
        CS_MW_DbgPrintf("USB Update csapp...\n");
        system("cp /tmp/csapp /home -rf");
    }

    if( access( "/tmp/resource", 0 ) != -1 )
    {
        CS_MW_DbgPrintf("USB Update resource...\n");
        system("cp /tmp/resource /home -rf");
    }

    system("umount /tmp");
#endif
    return(TRUE);

}

#if 0
void LED_DISPLAY(int argc, char *argv[])
{


column = atoi(argv[3]);

CSFP_SetLEDDisplayPos(fp_handle,1,column);

if(0 == strcmp("ON",argv[2]))
CSFP_SetLEDDisplayMode(fp_handle,CSFP_LEDDISP_ON);
else if (0 == strcmp("OFF",argv[2]))
CSFP_SetLEDDisplayMode(fp_handle,CSFP_LEDDISP_OFF);
else
CSFP_SetLEDDisplayMode(fp_handle,CSFP_LEDDISP_BLINK);

if( CSAPI_SUCCEED == CSFP_SetLEDDisplayRaw(fp_handle, argv[1]))
printf("Set Led Charecters Success\n");
else
printf("Set Led Charecters Error\n");

CSFP_GetLEDDisplayRaw(fp_handle,Get_char);
printf("Raw Char in test program = %s\n",Get_char);

CSFP_Close(fp_handle);
return;
}
#endif

