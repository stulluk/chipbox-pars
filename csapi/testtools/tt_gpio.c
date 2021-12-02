#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "csgpio.h"
  
static void GPIO_SET(int argc, char *argv[])
{
	int gpio = -1;
	CSGPIO_HANDLE hgpio = NULL;
	CSGPIO_DIRECTION dir = GPIO_DIRECTION_WRITE;
	char *errstr = NULL;

	if (argc > 1)
		gpio = atoi(argv[1]);

	if ((gpio > 15) || (gpio < 0)) {
		printf("no such device gpio %d\n", gpio);
		return;
	}

	if (NULL == (hgpio = CSGPIO_Open(gpio))) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_SetDirection(hgpio, dir)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_Write(hgpio, 1)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_Close(hgpio)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	printf("%s succeed\n", __FUNCTION__);
}

static void GPIO_CLEAR(int argc, char *argv[])
{
	int gpio = -1;
	CSGPIO_HANDLE hgpio = NULL;
	CSGPIO_DIRECTION dir = GPIO_DIRECTION_WRITE;
	char *errstr = NULL;

	if (argc > 1)
		gpio = atoi(argv[1]);

	if ((gpio > 15) || (gpio < 0)) {
		printf("no such device gpio %d\n", gpio);
		return;
	}

	if (NULL == (hgpio = CSGPIO_Open(gpio))) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_SetDirection(hgpio, dir)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_Write(hgpio, 0)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	if (CSAPI_FAILED == CSGPIO_Close(hgpio)) {
		errstr = CSGPIO_GetErrString(hgpio);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	printf("%s succeed\n", __FUNCTION__);
}

static void GPIO_INFO(int argc, char **argv)
{
	int status;

	if (fork() == 0)
		if (execl("/bin/cat", "cat", "/proc/gpio", (char *)NULL) < 0)
			perror("execl !\n");

	wait(&status);
	if (WIFEXITED(status))
		printf("%s succeed\n", __FUNCTION__);
	else
		printf("%s failed\n", __FUNCTION__);
}

static struct cmd_t csgpio_tt[] = {
	{
	 "gpio_set",
	 NULL,
	 GPIO_SET,
	 "gpio_set - set a gpio pin to high\
	\n    usage: gpio_set <pin_id> \
	\n       eg: gpio_set 3 \n"},
	{
	 "gpio_clear",
	 NULL,
	 GPIO_CLEAR,
	 "gpio_clear - clear a gpio pin to low\
	\n    usage: gpio_clear <pin_id> \
	\n       eg: gpio_clear 3 \n"},
	{
	 "gpio_info",
	 NULL,
	 GPIO_INFO,
	 "gpio_info - to print GPIO pin-mux info\
	\n    usage: gpio_info\
	\n       eg: gpio_info\n"},
};

