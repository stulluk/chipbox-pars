/*
 * arm-linux-gcc spi-test-mod.c ../lib/libcsspi.a -I ../include/ -I ../../include/ 
 * 
 * verify example  on board 
 * 1. set
 * 	./a.out <param>
 * 2. verify
 * 	echo "rl 0" > /proc/spi_io
 * 
 * */
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "csspi.h"

#define DELAY_SOME_TIME	2
CSSPI_OBJ *handle = NULL;

int tread(int argc, char **argv)
{
	int i;
	int ret;
	int addr;
	int len;
	char *buffer = NULL;

	if (argc < 3) {
		printf("Paramter not valid!\n"
		       "Usage: cmd read <size> data...\n");
		return -1;
	}

	addr = strtoul(argv[2], NULL, 16);
	len = strtoul(argv[3], NULL, 16);
	buffer = (char *)malloc(len * sizeof(char));

	ret = CSSPI_Read(handle, addr, buffer, len);	
	if (ret < 0) {
		printf(" CSAPI_Read Error!\n");
		return 1;
	}

	printf(" CSSPI_Read: Read Data From Flash:\n");
	for (i = 0; i < len; i++)
		printf("CSSPI_Read: Flash Data[%x] : <%x>\n", i, buffer[i]);
	
	return 0;
}

int twrite(int argc, char **argv)
{
	int i;
	int ret;
	int addr;
	int len;
	char *buffer = NULL;

	if (argc < 3) {
		printf("Paramter not valid!\n"
		       "Usage: cmd read <size> data...\n");
		return -1;
	}
	
	addr = strtoul(argv[2], NULL, 16);
	len = strtoul(argv[3], NULL, 16);
	buffer = (char *)malloc(len * sizeof(char));
	for (i = 0; i < len; i++)
		buffer[i] = strtoul(argv[i+4], NULL, 16);

	ret = CSSPI_Write(handle, addr, buffer, len);	
	if (ret < 0) {
		printf(" CSSPI_Write Error!\n");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	char buffer[32];
	memset(buffer, 0, sizeof(buffer));

	if (argc < 2) {
		printf("bad paramter!!!\n");
		return 0;
	}
		
	handle = CSSPI_Open();

	if (! strcmp(argv[1], "read"))
		tread(argc, argv);

	if (! strcmp(argv[1], "write"))
		twrite(argc, argv);

	if (! strcmp(argv[1], "rxtx"))
			CSSPI_SetTMode(handle, CSSPI_TXRX);

	if (! strcmp(argv[1], "rxo"))

			CSSPI_SetTMode(handle, CSSPI_RXO);

	if (! strcmp(argv[1], "txo"))
			CSSPI_SetTMode(handle, CSSPI_TXO);

	if (! strcmp(argv[1], "eeprom"))
			CSSPI_SetTMode(handle, CSSPI_EEPROM);



	if (! strcmp(argv[1], "hi"))
			CSSPI_SetClockPolarity(handle, CSSPI_HIGHPOL);

	if (! strcmp(argv[1], "lo"))
			CSSPI_SetClockPolarity(handle, CSSPI_LOWPOL);


	if (! strcmp(argv[1], "middle"))
			CSSPI_SetClockPhase(handle, CSSPI_MIDDLE);

	if (! strcmp(argv[1], "start"))
			CSSPI_SetClockPhase(handle, CSSPI_START);

	CSSPI_Close(handle);

	return 0;
}

