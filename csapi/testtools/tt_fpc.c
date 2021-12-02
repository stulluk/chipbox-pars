#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
 
#include "csfrontpanel.h"

#ifdef _DEBUG_
#define debug_printf printf
#else
#define debug_printf(fmt,args...)
#endif

#define INPUT_DEVICE "/dev/input/event0"

/* Call back function to translate code */
typedef int (*translate_cb) (unsigned short code);
static translate_cb rc_cb = NULL;

static void RC_REMOTEKEY(int argc, char *argv[])
{
	static int fd_rc = -1;
	struct input_event ev;
	int code;

	if (fd_rc < 0) {
		fd_rc = open(INPUT_DEVICE, O_RDONLY);
		if (fd_rc < 0)
			return;
	}

	while (1) {
		printf("press any key ... \n");

		read((int) fd_rc, &ev, sizeof(struct input_event));
		if (ev.type == EV_KEY && ev.value != 0) {
			if (rc_cb != NULL)
				code = rc_cb(ev.code);
			else
				code = ev.code;

			printf("%02x \n", code);

			if (code != -1)
				return;
		}
	}
}

static void LED_DISPLAY(int argc, char *argv[])
{
	unsigned char Get_char[2];
	char ch;
	CSFP_HANDLE led_handle;
	int column =0;
	printf("Came inside Test case\n");

	if (argc < 4)
	{
		printf("Argument Error\n");
		return;
	}

	led_handle = CSFP_Open();

	if (NULL == led_handle)
	{
		printf("Open FP Error\n");
		return;
	}

	column = atoi(argv[3]);

	CSFP_SetLEDDisplayPos(led_handle,1,column);
	
	if(0 == strcmp("ON",argv[2]))	   
		CSFP_SetLEDDisplayMode(led_handle,CSFP_LEDDISP_ON);

	else if (0 == strcmp("OFF",argv[2]))
		CSFP_SetLEDDisplayMode(led_handle,CSFP_LEDDISP_OFF);

	else  
		CSFP_SetLEDDisplayMode(led_handle,CSFP_LEDDISP_BLINK);

	if( CSAPI_SUCCEED == CSFP_SetLEDDisplayChar(led_handle, argv[1])){
		
		printf("Set Led Charecters Success\n");
	}
	else
		printf("Set Led Charecters Error= %s\n",CSFP_GetErrString(led_handle));

	CSFP_GetLEDDisplayChar(led_handle,Get_char);
	printf("Char in test program = %s\n",Get_char);

	printf("Press Y to check Set DisplayRaw else Press N\n");
	ch = getchar();

	if(ch == 'Y')
	{
		if( CSAPI_SUCCEED == CSFP_SetLEDDisplayRaw(led_handle, argv[1], 4))
			printf("Set Led Charecters Success\n");
		else
			printf("Set Led Charecters Error\n");

		CSFP_GetLEDDisplayRaw(led_handle,Get_char);
		printf("Raw Char in test program = %s\n",Get_char);

	}

	//CSFP_Close(led_handle);
	return;
}

static struct cmd_t csfpc_tt[] = {
	{
	 "fpc_getkey",
	 NULL,
	 RC_REMOTEKEY,
	 "fpc_getkey - get a key from front panel \
\n    usage: fpc_getkey \
\n       eg: fpc_getkey \n"},
	{
	 "fpc_leddisp",
	 NULL,
	 LED_DISPLAY,
	 "fpc_leddisp - display a LED string \
\n    usage: fpc_leddisp  <Charecter> <mode> <Column_No>\
\n       eg: fpc_leddisp  '1'OFF/BLINK/ON 1-4\n"},
};
