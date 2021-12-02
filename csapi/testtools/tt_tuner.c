
#include "global.h"
#include "dvbs_tuner.h"
#include "cs_tuner.h"
 
static void TUNER_Notify(CSTUNER_EVENT_TYPE event, CSTUNER_DEV_TYPE dev_type) 
{
	switch (event)
	{
		case TUNER_EVT_NO_OPERATION:
			printf(" Tuner: NO OPERATION\n");
			break;

		case TUNER_EVT_LOCKED:
			printf(" Tuner: LOCKED\n");
			break;

		case TUNER_EVT_UNLOCKED:
			printf(" Tuner: UNLOCKED\n");
			break;

		case TUNER_EVT_WAITLOCKED:
			printf(" Tuner: WAITLOCKED\n");
			break;

		case TUNER_EVT_SCAN_FAILED:
			printf(" Tuner: SCAN FAILED\n");
			break;

		case TUNER_EVT_TIMEOUT:
			printf(" Tuner: TIMEOUT\n");
			break;

		case TUNER_EVT_SIGNAL_CHANGE:
			printf(" Tuner: SIGNAL CHANGED\n");
			break;

		default:
			break;
	}

	return;
}

static int _gpio_write(char *devname, char* buf, int len )
{
	int gpio_fd;
	int retval;
	char cmd='O';
	puts(devname);
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Error: Open %s.\n",devname);
		return -1;
	}

	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}

	retval = write(gpio_fd, buf , len);
	if (retval != len)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}

	retval = close(gpio_fd);

	return 0;
}

static int _tuner_gpio_reset(void)
{
	int retval;
	char value = '0';
	retval = _gpio_write("/dev/gpio/6",&value,1);
	usleep(400000);
	value = '1';
	retval = _gpio_write("/dev/gpio/6", &value,1);
	usleep(400000);

	return 0;       
}

/* FIXME: 11/17/08 added for the whole set */

CSTUNER_ScanParams ScanParams;
TUNER_PARAMS_S tuner_param;
int tuner_initialed = 0;

static void tuner_setconf(int argc, char **argv)
{
	CSTUNER_HANDLE tuner_handle;
	CSAPI_RESULT result;
	CSTUNER_InitParams initparam;
	int err_code = 0;
	int tuner_fd = 0;


	if (argc < 6) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	if (!strcmp(argv[1], "dvbs")) {
		tuner_handle = CSTUNER_Open(TUNER_SAT);
		if (tuner_handle == (void *) CSAPI_FAILED) {
			printf(" open tuner error\n");	
			return;
		}

		initparam.notify_proc = TUNER_Notify;
		result = CSTUNER_Init(tuner_handle, initparam);
		if (result == CSAPI_SUCCEED) 
			printf(" tuner_init is ok\n");

		ScanParams.Frequency = atoi(argv[2]);
		ScanParams.SymbolRate = atoi(argv[3]);
		ScanParams.LnbFrequency = atoi(argv[4]);
		ScanParams.Polarization = atoi(argv[5]);
		result = CSTUNER_SetFrequencyWithTimeout(tuner_handle, ScanParams, 1000);
		if (result == CSAPI_SUCCEED)
			printf(" tuner_setconf is ok\n");
		else
			printf(" tuner_setconf is failed\n");

		CSTUNER_Close(tuner_handle);

	} else if (!strcmp(argv[1], "dvbc")) {
		_tuner_gpio_reset();
		err_code = cs_tuner_init();
		if(err_code < 0) {
			printf("Error: cs_tuner_init.\n");
			return;
		}

		tuner_fd = cs_tuner_open(TUNER_ID_0);
		if(tuner_fd < 0) {
			printf("Error: cs_tuner_init.\n");
			return;
		}       

		tuner_param.frequency = atoi(argv[2]) * 1000; 
		tuner_param.qam_params.symbol_rate =  atoi(argv[3]);
		tuner_param.qam_params.modulation = atoi(argv[4]);
		tuner_param.inversion = atoi(argv[5]);

		err_code = cs_tuner_set_params(tuner_fd, &tuner_param);
		if(err_code < 0) {
			printf("Error: tuner_setconf.\n");
			return;
		}       

		cs_tuner_close(TUNER_ID_0);
	}

	tuner_initialed = 1;
}

static void tuner_getconf(int argc, char **argv)
{
	CSAPI_RESULT result;
	CSTUNER_Info tuner_info;
	CSTUNER_HANDLE tuner_handle;
	CSTUNER_InitParams initparam;

	if (! tuner_initialed) {
		printf("Error: tuner_getconf, not setconfiguration.\n");
		return;
	}

	if (argc < 2) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	if (!strcmp(argv[1], "dvbs")) {
		tuner_handle = CSTUNER_Open(TUNER_SAT);
		if (tuner_handle == (void *) CSAPI_FAILED) {
			printf(" open tuner error\n");	
			return;
		}

		initparam.notify_proc = TUNER_Notify;
		result = CSTUNER_Init(tuner_handle, initparam);
		if (result == CSAPI_SUCCEED) 
			printf(" tuner_init is ok\n");
#if 0
		printf("ScanParams.Frequency: %d\n", ScanParams.Frequency);
		printf("ScanParams.SymbolRate: %d\n", ScanParams.SymbolRate);
		printf("ScanParams.LnbFrequency: %d\n", ScanParams.LnbFrequency);
		printf("ScanParams.Polarization: %d\n", ScanParams.Polarization);
#endif
		result = CSTUNER_GetTunerInfo(tuner_handle, &tuner_info);
		printf(" LnbFrequency:  %d KHz\n", tuner_info.LnbFrequency);
		printf(" Frequency:     %d MHz\n", tuner_info.Frequency);
		printf(" SymbolRate:    %d Ms/s\n", tuner_info.SymbolRate);
		printf(" Polarization:  %d (0=V &1=H)\n", tuner_info.Polarization);
		printf(" SignalQuality: %d \n", tuner_info.SignalQuality);
		printf(" SignalLevel:   %d \n", tuner_info.SignalLevel);

		CSTUNER_Close(tuner_handle);

	} else if (!strcmp(argv[1], "dvbc")) {
		printf("tuner_param.frequency: %d\n", tuner_param.frequency);
		printf("tuner_param.qam_params.symbol_rate: %d\n", tuner_param.qam_params.symbol_rate);
		printf("tuner_param.qam_params.modulation: %d\n", tuner_param.qam_params.modulation);
		printf("tuner_param.inversion: %d\n", tuner_param.inversion);
	}
}

static void tuner_getlockstatus(int argc, char **argv)
{
	CSAPI_RESULT result;
	TUNER_STATUS_E tuner_status = 0;
	int err_code = 0;
	CSTUNER_Info tuner_info;
	CSTUNER_HANDLE tuner_handle;
	CSTUNER_InitParams initparam;
	int tuner_fd = 0;

	if (! tuner_initialed) {
		printf("Error: tuner_getlockstatus, not set configuration.\n");
		return;
	}

	if (argc < 2) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	if (!strcmp(argv[1], "dvbs")) {
		tuner_handle = CSTUNER_Open(TUNER_SAT);
		if (tuner_handle == (void *) CSAPI_FAILED) {
			printf(" open tuner error\n");	
			return;
		}

		initparam.notify_proc = TUNER_Notify;
		result = CSTUNER_Init(tuner_handle, initparam);
		if (result == CSAPI_SUCCEED) 
			printf(" tuner_init is ok\n");

		result = CSTUNER_GetTunerInfo(tuner_handle, &tuner_info);
		switch (tuner_info.Status) {
			case S_TUNER_STATUS_UNLOCKED:
				printf(" UNLOCKED\n");
				break;
			case S_TUNER_STATUS_SCANNING:
				printf(" SCANNING\n");
				break;
			case S_TUNER_STATUS_LOCKED:
				printf(" LOCKED\n");
				break;
			case S_TUNER_STATUS_WAITLOCKED:
				printf(" WAITLOCKED\n");
				break;
			case S_TUNER_STATUS_NOT_FOUND:
				printf(" NOT_FOUND\n");
				break;
			default:
				break;
		}

		CSTUNER_Close(tuner_handle);

	} else if (!strcmp(argv[1], "dvbc")) {
		_tuner_gpio_reset();
		err_code = cs_tuner_init();
		if(err_code < 0) {
			printf("Error: cs_tuner_init.\n");
			return;
		}

		tuner_fd = cs_tuner_open(TUNER_ID_0);
		if(tuner_fd < 0) {
			printf("Error: cs_tuner_init.\n");
			return;
		}       

		err_code = cs_tuner_read_status(0, &tuner_status);
		if(err_code < 0) {
			printf("Error: cs_tuner_read_status.\n");
			return;
		}

		switch (tuner_status) {
			case TUNER_STATUS_PLL_LOCKED:
				printf("TUNER_STATUS_PLL_LOCKED.\n");
				break;
			case TUNER_STATUS_FE_LOCKED:
				printf("TUNER_STATUS_FE_LOCKED.\n");
				break;
			case TUNER_STATUS_UNLOCKED:
				printf("TUNER_STATUS_UNLOCKED.\n");
				break;
			case TUNER_STATUS_SCANNING:
				printf("TUNER_STATUS_SCANNING.\n");
				break;
			case TUNER_STATUS_NOT_FOUND:
				printf("TUNER_STATUS_NOT_FOUND.\n");
				break;
			default:
				printf("Unkown state!\n");
				break;
		}

		cs_tuner_close(TUNER_ID_0);
	}

}

static void tuner_getinfo(int argc, char **argv)
{
	CSAPI_RESULT result;

	UNUSED_VARIABLE(result);

	if (! tuner_initialed) {
		printf("Error: tuner_getconf, not setconfiguration.\n");
		return;
	}

	if (argc < 2) {
		printf(" invalid parameters, see help! \n");
		return;
	}

	if (!strcmp(argv[1], "dvbs")) {
		printf(" frequency_min The minimum value of frequency \n");
		printf(" frequency_max The maximum value of frequency \n");
		printf(" frequency_stepsize The step size of frequency \n");
		printf(" frequency_tolerance The tolerance value of frequency \n");
		printf(" symbol_rate_max The maximum value of symbol rate \n");
		printf(" symbol_rate_min The minimum value of symbol rate \n");
		printf(" symbol_rate_tolerance The tolerance value of symbol rate \n");
	} else if (!strcmp(argv[1], "dvbc")) {
		printf(" frequency_min The minimum value of frequency \n");
		printf(" frequency_max The maximum value of frequency \n");
		printf(" frequency_stepsize The step size of frequency \n");
		printf(" frequency_tolerance The tolerance value of frequency \n");
		printf(" symbol_rate_max The maximum value of symbol rate \n");
		printf(" symbol_rate_min The minimum value of symbol rate \n");
		printf(" symbol_rate_tolerance The tolerance value of symbol rate \n");
	}
}


static struct cmd_t cstuner_tt[] = {
 	{	
 		"tuner_setconf",
 		NULL,
 		tuner_setconf,
 		 "Set Tuner Configuration and scan the provided frequency.. \
 		\n usage: tuner_setconf <TunerType> <Frequency><QAMModulation><SymbolRate><Inversion> \
 		\n eg:tuner_setconf  0 ...\n"},
 	{
 		"tuner_getconf",
 		NULL,
 		tuner_getconf,
 		 "Get the Current Tuner Configurations.. \
 		\n usage: tuner_getconf <TunerType>\
 		\n eg:tuner_getconf 0\n"},
 	{
 		"tuner_getlockstatus",
 		NULL,
 		tuner_getlockstatus,
 		 " Get the Current Lock status of the Tuner to the console.  Output. \
 		\n usage: tuner_getlockstatus <TunerType>\
 		\n eg:tuner_getlockstatus 0\n"},
 	{
 		"tuner_getinfo",
 		NULL,
 		tuner_getinfo,
 		 "Get the Current Tuner Configurations.. \
 		\n usage: tuner_getinfo <TunerType>\
 		\n eg:tuner_getinfo 0\n"},
};
