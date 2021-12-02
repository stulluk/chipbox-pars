
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>  /* By KB Kim : 2010_06_17 */

#include "common.h"

#ifdef _ORION14_IAL
#define ORION14_IAL_FIP //FIXME

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "misc.h"
#include "ial.h"
#include "keycodes.h"

static int fpc_fd = -1;

/* By KB Kim : 2010_06_17 */
#define KBD_FPC 1

#ifdef ORION14_IAL_FIP
#define KBD_FIP 2
static int fip_fd = -1;
#endif
static int keyboard_type = 0;

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
static int FrontFd = -1;
static int FrontKeyIn = 0;
int SubFunctionEnable;
unsigned int   FrontKeyCount;
long  FrontKeyInterval; /* 1sec */
static unsigned int CurrentKeyCode = MGUI_NR_KEYS;
struct timespec KeyTime;

/* For Front Communication by kb : 20110201 */
unsigned char FrontDataBuffer[MAX_FB_COMM_LENGTH];
unsigned char FrontDataUpdate = 0;
#endif

static unsigned char state[MGUI_NR_KEYS];

extern BOOL RcuKeyMessageDisable;  // by kb: 20100408

/* by kb : 20101229 For Key buffer */
extern unsigned char MaxKeyInputBufferSize;
unsigned char RcuKeyBufferCount = 0;

/* 
 * IAL_SetKeyMap: set keyboard map table
 */
int IAL_SetKeyMap(int *keymap_tab, int keymap_num)
{
	if (NULL == keymap_tab) return FALSE;
	if (keymap_num > MGUI_NR_KEYS) return FALSE;

	/* By KB Kim : 2010_06_17 */
	// memcpy(basic_keycodes, keymap_tab, keymap_num * sizeof(int));

	return TRUE;
}

/* 
 * IAL_GetKeyMap: get keyboard map table
 */
int IAL_GetKeyMap(int *keymap_tab, int *keymap_num)
{
	if (NULL == keymap_tab) return FALSE;
	if (NULL == keymap_num) return FALSE;

	if (*keymap_num > MGUI_NR_KEYS) *keymap_num = MGUI_NR_KEYS;
	/* By KB Kim : 2010_06_17 */
	// memcpy(keymap_tab, basic_keycodes, (*keymap_num) * sizeof(int));

	return TRUE;
}

static void init_keymap(void)
{
	return;
}

static int mouse_update(void)
{
	return 1;
}

static void mouse_getxy (int* x, int* y)
{
}

static int mouse_getbutton(void)
{
	return 0;
}

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
unsigned short GetCurrentPowerScanCode(KeyCode_t *currentKeyCodeP)
{
	unsigned short count;

	if (currentKeyCodeP == NULL)
	{
		return SCANCODE_USER;
	}

	for(count = 0; count < 0x100; count++)
	{
		if (currentKeyCodeP->KeyCode[count] == CSAPP_KEY_POWER)
		{
			return count;
		}
	}

	return SCANCODE_USER;
}

unsigned char GetRcuPowerKeyCode(unsigned char *keyData)
{
	unsigned char  numberOfRcu;
	unsigned short powerScanCode;
	unsigned       dataPointer;
	KeyCode_t     *currentKeyCodeP;

	if (keyData == NULL)
	{
		return 0;
	}
	
	numberOfRcu = 0;
	dataPointer = 0;
	currentKeyCodeP = FirstKeyCode;
	while((currentKeyCodeP != NULL) && (numberOfRcu < MAX_FRONT_SUPPORT_RCU))
	{
		powerScanCode = GetCurrentPowerScanCode(currentKeyCodeP);
		if (powerScanCode < 0x100)
		{
			keyData[dataPointer++] = currentKeyCodeP->CompanyCode[0];
			keyData[dataPointer++] = currentKeyCodeP->CompanyCode[1];
			keyData[dataPointer++] = (unsigned char)(powerScanCode & 0xFF);
			numberOfRcu++;
		}

		currentKeyCodeP = currentKeyCodeP->Next;
	}

	return numberOfRcu;
}

void AddRcuKeyCode(KeyMaps_t *keyMapData)
{
	KeyCode_t    *currentKeyCodeP;
	KeyCode_t    **currentPointer;

	unsigned      keyNumber;
	unsigned      count;
	unsigned      upperScanCode;
	unsigned char scanCode;
	unsigned char keyCode;

	currentPointer  = &FirstKeyCode;
	currentKeyCodeP = FirstKeyCode;

	while (currentKeyCodeP != NULL)
	{
		currentPointer  = &currentKeyCodeP->Next;
		currentKeyCodeP = currentKeyCodeP->Next;
	}

	currentKeyCodeP = malloc(sizeof(KeyCode_t));
	if (currentKeyCodeP == NULL)
	{
		printf("AddRcuKeyCode Error : Can not Allocate Memory!\n");
		return;
	}

	currentKeyCodeP->Next = NULL;
	keyNumber             = keyMapData->KeyNumber;
	currentKeyCodeP->CompanyCode[0] = keyMapData->CompanyCode[0];
	currentKeyCodeP->CompanyCode[1] = keyMapData->CompanyCode[1];
	// printf("AddRcuKeyCode Info : New KeyCode [0x%02X] [0x%02X] : %d\n", currentKeyCodeP->CompanyCode[0], currentKeyCodeP->CompanyCode[1], keyNumber);

	memset(currentKeyCodeP->KeyCode, RCKEY_DEF_VAL, SCANCODE_USER);
	for (count = 0; count < keyNumber; count++)
	{
		scanCode = keyMapData->KeyScanCode[count].ScanCode;
		keyCode  = keyMapData->KeyScanCode[count].KeyValue;
		if (currentKeyCodeP->KeyCode[scanCode] == RCKEY_DEF_VAL)
		{
			/* First key function assign */
			currentKeyCodeP->KeyCode[scanCode] = keyCode;
		}
		else
		{
			/* This Scancode share with other function */
			upperScanCode = (unsigned)scanCode;
			upperScanCode |= 0x100;
			currentKeyCodeP->KeyCode[upperScanCode] = keyCode;
		}
	}

	*currentPointer = currentKeyCodeP;
}

int SetRcuKeyCode(char *data)
{
	KeyCode_t    *currentKeyCodeP;
	unsigned      scanCode;
	unsigned      keyCode;

	currentKeyCodeP = FirstKeyCode;
	while(currentKeyCodeP != NULL)
	{
		if ((data[2] == currentKeyCodeP->CompanyCode[0]) && (data[3] == currentKeyCodeP->CompanyCode[1]))
		{
			scanCode = data[4];
			keyCode = currentKeyCodeP->KeyCode[scanCode];
			// printf("SetRcuKeyCode info : Found Matched RCU Code for [0x%02X] [0x%02X] - [0x%02X]\n", data[2], data[3], data[4]);
			if (keyCode != RCKEY_DEF_VAL)
			{
				// printf("SetRcuKeyCode info : Main Function KeyCode [0x%02X]\n", keyCode);
				state[keyCode] = 1;
				/* Check sub function */
				scanCode |= 0x100;
				keyCode = currentKeyCodeP->KeyCode[scanCode];
				if (keyCode != RCKEY_DEF_VAL)
				{
					// printf("SetRcuKeyCode info : Sub Function KeyCode [0x%02X]\n", keyCode);
					state[keyCode] = 1;
					SubFunctionEnable = 1;
				}
			}
			return NR_KEYS;
		}
		else
		{
			/*
			printf("SetRcuKeyCode info : Not Matched [0x%02X]/[0x%02X] [0x%02X]/[0x%02X]\n",
				data[2], currentKeyCodeP->CompanyCode[0], data[3], currentKeyCodeP->CompanyCode[1]);
				*/

			currentKeyCodeP = currentKeyCodeP->Next;
		}
	}

	return 0;
}

int FbUartReceiveFrontData(int fd, char *buffer, unsigned int timeout)
{
	unsigned int  timeCount;
	int           count;
	int           dataLen;
	char          header[MAX_FB_COMM_LENGTH];
	char          waitHeader;

	if (buffer == NULL)
	{
		return -1;
	}
	
	timeCount  = 0;
	waitHeader = 1;
	count      = 0;
	while (waitHeader && (timeCount < timeout))
	{
		count = read(fd, header, MAX_FB_COMM_LENGTH);
		if (count != (-1))
		{
			// printf("FbReceiveFrontData info : Get data %d Bytes Header[0x%02X] \n", count, header[0]);
			if (header[0] == FRONT_START_CODE)
			{
				waitHeader = 0;
			}
		}
		else
		{
			usleep (100);
			timeCount++;
		}
	}

	if ((timeCount >= timeout) && waitHeader && (count < 3))
	{
		/* Receive Timeout */
		printf("FbReceiveFrontData Error : read Timeout\n");
		return (-1);
	}

	dataLen = 2;
	memcpy(buffer, header + 1, dataLen);

	count = (int)header[2];
	if (count > 0)
	{
		memcpy(buffer + dataLen, header + dataLen +1, count);
		dataLen += count;
	}

	if (header[dataLen + 1] == FRONT_END_CODE)
	{
		return dataLen;
	}

	return (-1);
}

/* For Front Communication by kb : 20110201 */
static void FbSetFrontData(unsigned char *data)
{
	int count;

	count = (int)data[1] + 2; /* header + length + data */
	memset(FrontDataBuffer, 0x00, MAX_FB_COMM_LENGTH);
	memcpy(FrontDataBuffer, data, count);
	FrontDataUpdate = count;

	// printf("FbSetFrontData : 0x%02X 0x%02X ..\n", FrontDataBuffer[0], FrontDataBuffer[1]);
}

int FbGetFrontData(unsigned char *data)
{
	int count;

	count = FrontDataUpdate;
	if (count > 0)
	{
		memcpy(data, FrontDataBuffer, count);
		FrontDataUpdate = 0;
	}

	return count;
}

#endif // #ifdef _MERIH_UART1_IAL

static int keyboard_update(void)
{
	static int zero_key = 0; /* By KB Kim : 2010_06_17 */
	struct input_event fpc_code;
#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	char            uipData[MAX_FB_COMM_LENGTH];
	unsigned int    keyCode;
	long            timeDiff;
	struct timespec currentTime;
#endif

#ifdef ORION14_IAL_FIP
	unsigned char fip_code;
	static unsigned char fip_realcode;
#endif

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	if (SubFunctionEnable)
	{
		return NR_KEYS;
	}
	
	if (FrontKeyIn)
	{
		FbUartReceiveFrontData(FrontFd, uipData, 1000);

		/* For Front Communication by kb : 20110201 */
		if ((uipData[0] & FRONT_KEY_COMMAND_HIGH_MASK) != FRONT_RCU_KEY)
		{
			/* Not a Key Data */
			FbSetFrontData(uipData);
			FrontKeyIn = 0;

			return 0;
		}

		/* by kb : 20101229 For Key buffer */
		if (RcuKeyMessageDisable == TRUE)
		{
			if (RcuKeyBufferCount >= MaxKeyInputBufferSize)
			{
				FrontKeyIn = 0;

				return 0;
			}
			RcuKeyBufferCount++;
		}
		else
		{
			RcuKeyBufferCount = 0;
		}
		
		// read (FrontFd, &fpc_code, sizeof(fpc_code));
		switch(uipData[0])
		{
			case FRONT_FRONT_KEY :
				CurrentKeyCode = (unsigned int)uipData[2];
				clock_gettime(CLOCK_REALTIME, &KeyTime);
				FrontKeyCount = 0;
				FrontKeyInterval = 500000;
				break;
			case FRONT_FRONT_REPEAT :
				keyCode = (unsigned int)uipData[2];
				clock_gettime(CLOCK_REALTIME, &currentTime);
				timeDiff = ((currentTime.tv_sec - KeyTime.tv_sec) * 1000000 + (currentTime.tv_nsec / 1000)) - (KeyTime.tv_nsec / 1000);
				if (CurrentKeyCode != keyCode)
				{
					CurrentKeyCode = keyCode;
					KeyTime = currentTime;
					FrontKeyCount = 0;
					break;
				}
				
				if (timeDiff < FrontKeyInterval)
				{
					FrontKeyIn = 0;
				}
				else
				{
					FrontKeyCount++;
					KeyTime = currentTime;
					if (FrontKeyCount == 2)
					{
						FrontKeyInterval = 400000; /* 750 msec */
					}
					else if (FrontKeyCount == 3)
					{
						FrontKeyInterval = 250000; /* 500 msec */
					}
					else if (FrontKeyCount > 3)
					{
						FrontKeyInterval = 100000; /* 250 msec */
					}
				}
				break;
			case FRONT_RCU_KEY :
			case FRONT_RCU_REPEAT :
				FrontKeyCount = 0;
				FrontKeyIn = 0;
				// FrontKeyInterval = 500000;
				clock_gettime(CLOCK_REALTIME, &currentTime);
				timeDiff = currentTime.tv_sec*1000 + currentTime.tv_nsec/1000000;

				// printf("keyboard_update info : RCU Scan code[0x%02X, 0x%02X, 0x%02X]\n", uipData[2], uipData[3], uipData[4]);
				return SetRcuKeyCode(uipData);
		}
		
		if (FrontKeyIn)
		{
			// printf("keyboard_update info : CurrentKeyCode [0x%02X]\n", CurrentKeyCode);
			if (FrontKeyCode[CurrentKeyCode] != RCKEY_DEF_VAL)
			{
				/* Valid Key */
				// printf("keyboard_update info : CurrentKeyCode [0x%02X] FrontKeyCode [0x%02X]  FrontKeyCount[%d]\n", CurrentKeyCode, FrontKeyCode[CurrentKeyCode], FrontKeyCount);
				state[FrontKeyCode[CurrentKeyCode]] = 1;
			}
			FrontKeyIn = 0;
		}
		else
		{
			return 0; 
		}

		return NR_KEYS;
	}

	return 0;
#else // #ifdef _MERIH_UART1_IAL

#if 0
	if(keyboard_type & KBD_FPC) {
	    keyboard_type &= ~KBD_FPC;
	    read (fpc_fd, &fpc_code, sizeof(fpc_code));
		// printf("KeyIn : Code[0x%02X], basic_keycodes[0x%02X], value[0x%02X]\n", fpc_code.code, basic_keycodes[fpc_code.code], fpc_code.value);

	    if (fpc_code.code && (fpc_code.code >= 0 || fpc_code.code < MGUI_NR_KEYS)) {
			state[basic_keycodes[fpc_code.code]] = fpc_code.value;
	    }
	    else if ((fpc_code.code == 0) && (fpc_code.value)) {
			zero_key = 1;
			state[basic_keycodes[fpc_code.code]] = fpc_code.value;
	    }
	    else if (zero_key)
	    {
			zero_key = 0;
			state[basic_keycodes[0]] = 0;
	    }
	    else 
			return 0; 
	}
#endif

#ifdef ORION14_IAL_FIP
	if(keyboard_type & KBD_FIP) {
	    keyboard_type &= ~KBD_FIP;
	    if(fip_fd >= 0)
		read (fip_fd, &fip_code, sizeof(fip_code));
	    else
		return 0;

	    if(fip_code == 0) {  /* release a fip key accorrding to reading a zero value */
		if(fip_realcode != 0) {
		    state[basic_keycodes[fip_realcode]] = 0;
		    fip_realcode = 0;
		}
		else 
		    return 0;
	    }
	    else {	/* a real fip key pressed */
		fip_realcode = fip_code;
		state[basic_keycodes[fip_realcode]] = 1;
	    }
	}
#endif
	return NR_KEYS;
#endif // #ifdef _MERIH_UART1_IAL
}

static const char * keyboard_get_fpcstate(void)
{
	return (char *)state;
}

#ifdef  _LITE_VERSION
static int wait_event (int which, int maxfd, fd_set *in, fd_set *out, fd_set *except,
		struct timeval *timeout)
#else
static int wait_event (int which, fd_set *in, fd_set *out, fd_set *except,
		struct timeval *timeout)
#endif
{
	int    e;
	fd_set rfds;

	if (!in) {
		in = &rfds;
		FD_ZERO (in);
	}

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	if ((which & IAL_MOUSEEVENT || which & IAL_KEYEVENT) && FrontFd >= 0)
	{
	    FD_SET (FrontFd, in);
	}
#else  //#ifdef _MERIH_UART1_IAL

#if 0
	if ((which & IAL_MOUSEEVENT || which & IAL_KEYEVENT) && fpc_fd >= 0) {
	    FD_SET (fpc_fd, in);
#ifdef ORION14_IAL_FIP
	    if(fip_fd >= 0)
		FD_SET (fip_fd, in);
#endif

#ifdef _LITE_VERSION
		if (fpc_fd > maxfd) maxfd = fpc_fd;
#endif
	}
#endif
	
#endif // #ifdef _MERIH_UART1_IAL

#ifdef _LITE_VERSION
	e = select (maxfd + 1, in, out, except, timeout) ;
#else
	e = select (FD_SETSIZE, in, out, except, timeout) ;
#endif
	if (e < 0) 
		return -1;

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	if ((FrontFd >= 0)  && FD_ISSET (FrontFd, in))
	{
		unsigned   currentTime = 0;
		struct timespec  systemTime;

		clock_gettime(CLOCK_REALTIME, &systemTime);
		currentTime = systemTime.tv_sec*1000 + systemTime.tv_nsec/1000000;
		
		// printf("wait_event Info : FD_ISSET\n");
	    FD_CLR (FrontFd, in);
		FrontKeyIn = 1;
		// printf("Get Key Input From Front : 1st Action - %d\n", currentTime);
		return IAL_KEYEVENT;
	}

	return 0;
#endif

#if 0
	if (fpc_fd >= 0 && FD_ISSET (fpc_fd, in)) {
		FD_CLR (fpc_fd, in);
		keyboard_type |= KBD_FPC;
		return IAL_KEYEVENT;
	}
#endif
#ifdef ORION14_IAL_FIP
	if (fip_fd >= 0 && FD_ISSET (fip_fd, in)) {
		FD_CLR (fip_fd, in);
		keyboard_type |= KBD_FIP;
		return IAL_KEYEVENT;
	}
#endif

	return 0;
}

BOOL InitOrionInput (INPUT* input, const char* mdev, const char* mtype)
{
	int i ;
#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	struct termios option;
#endif

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	FirstKeyCode = NULL;
	AddRcuKeyCode(&ChipBoxRcu);
	AddRcuKeyCode(&X3TurboKey);
	AddRcuKeyCode(&CelestialKey);
	SubFunctionEnable = 0;
#endif // #ifdef _MERIH_UART1_IAL

#if 0
	printf("InitOrionInput Info : InitOrionInput\n");
	fpc_fd = open("/dev/input/event0",O_RDWR);
	if (fpc_fd < 0)
		return FALSE;
#endif

#ifdef ORION14_IAL_FIP
	fip_fd = open("/dev/fip", O_RDONLY);
	if (fip_fd < 0) {
	    printf("warnning: You have no fip frontpanel!\n");
	}
#endif

#ifdef _MERIH_UART1_IAL /* By KB Kim : 2010_06_17 */
	FrontFd = open("/dev/ttyS1", O_RDONLY|O_NOCTTY);
	if (FrontFd < 0)
	{
	    printf("warnning: You have no UART frontpanel!\n");
	}

	tcgetattr(FrontFd, &option);
	cfmakeraw(&option);
	cfsetispeed(&option, B19200);
	cfsetospeed(&option, B19200);
	tcsetattr(FrontFd, TCSANOW, &option);

	CurrentKeyCode = MGUI_NR_KEYS;
	FrontKeyCount = 0;
	FrontKeyInterval = 500000;
	clock_gettime(CLOCK_REALTIME, &KeyTime);
#endif
	for (i = 0 ; i < MGUI_NR_KEYS; i++)
		state[i] = 0;

	init_keymap();

	input->update_mouse = mouse_update;
	input->get_mouse_xy = mouse_getxy;
	input->set_mouse_xy = NULL;
	input->get_mouse_button = mouse_getbutton;
	input->set_mouse_range = NULL;

	input->update_keyboard = keyboard_update;
	input->get_keyboard_state = keyboard_get_fpcstate;
	input->set_leds = NULL;
	input->wait_event = wait_event;

	return TRUE;
}

void TermOrionInput (void)
{
	if (fpc_fd >= 0) close (fpc_fd);
}

#endif /* _TFSTB_IAL */

