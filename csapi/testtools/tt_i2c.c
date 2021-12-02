#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "csi2c.h"


static void I2C_CONF(int argc, char **argv)
{
	CSI2C_HANDLE hi2c;
	CSI2C_Attr i2c_attr;
	char *errstr = NULL;
	unsigned int dev = 0;

	if (argc < 3 || argc > 7) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	dev = strtol(argv[1], NULL, 16);

	if (NULL == (hi2c = CSI2C_Open(dev))) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Open succeed\n");

	if (!strcmp(argv[2], "set")) {
		i2c_attr.speed = strtol(argv[3], NULL, 10);
		i2c_attr.write_delayus = strtol(argv[4], NULL, 10);
		i2c_attr.subaddr_num = strtol(argv[5], NULL, 10);
		i2c_attr.loop = strtol(argv[6], NULL, 10);

		if (CSI2C_SetAttr(hi2c, &i2c_attr)) {
			errstr = CSI2C_GetErrString(hi2c);
			printf("%s  %s\n", __FUNCTION__, errstr);
			return;
		}

	} else if (!strcmp(argv[2], "get")) {
		if (CSI2C_GetAttr(hi2c, &i2c_attr)) {
			errstr = CSI2C_GetErrString(hi2c);
			printf("%s  %s\n", __FUNCTION__, errstr);
			return;
		}

		printf("i2c : speed %s, write delay us %d, subaddress number %d, loop flag %d\n",
		       (i2c_attr.speed == I2C_SPEED_100K) ? "100K" : "400K",
		       i2c_attr.write_delayus, i2c_attr.subaddr_num, i2c_attr.loop);
	} else {
		printf("%s Error: Unkown chioce!\n", __FUNCTION__);
	}
	printf("I2C_CONF succeed\n");

	if (CSI2C_Close(hi2c)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Close succeed\n");

}

static void I2C_READ(int argc, char *argv[])
{
	CSI2C_HANDLE hi2c;
	char *errstr = NULL;
	char *addr = NULL;
	int len = 0;
	int i = 0;
	unsigned int dev = 0;

	if (argc != 3) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	dev = strtol(argv[1], NULL, 16);
	if (NULL == (hi2c = CSI2C_Open(dev))) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Open succeed\n");

	len = strtol(argv[2], NULL, 16);

	if (NULL == (addr = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}
	memset(addr, 0, len);

	if (CSI2C_Read(hi2c, 0, addr, len)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	for (i = 0; i < len; i++) {
		printf("read data %d: 0x%x\n", i, addr[i]);
	}
	free(addr);
	printf("%s succeed\n", __FUNCTION__);

	if (CSI2C_Close(hi2c)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Close succeed\n");
}

static void I2C_READEXT(int argc, char *argv[])
{
	CSI2C_HANDLE hi2c;
	char *errstr = NULL;
	char *addr = NULL;
	int len = 0, subaddr = 0;
	int i = 0;
	unsigned int dev = 0;

	if (argc != 4) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	dev = strtol(argv[1], NULL, 16);
	if (NULL == (hi2c = CSI2C_Open(dev))) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Open succeed\n");

	subaddr = strtol(argv[2], NULL, 16);
	len = strtol(argv[3], NULL, 16);

	if (NULL == (addr = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}
	memset(addr, 0, len);

	if (CSI2C_Read(hi2c, subaddr, addr, len)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	for (i = 0; i < len; i++) {
		printf("read data %d: 0x%x\n", i, addr[i]);
	}
	free(addr);

	printf("%s succeed\n", __FUNCTION__);

	if (CSI2C_Close(hi2c)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Close succeed\n");
}

static void I2C_WRITE(int argc, char *argv[])
{
	CSI2C_HANDLE hi2c;
	char *errstr = NULL;
	char *addr = NULL;
	int len = 0;
	int i = 0;
	unsigned int dev = 0;
	unsigned int subaddr = 0;

	if (argc < 4) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	dev = strtol(argv[1], NULL, 16);
	if (NULL == (hi2c = CSI2C_Open(dev))) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Open succeed\n");

	subaddr = strtol(argv[2], NULL, 16);
	len = strtol(argv[3], NULL, 16);

	if (NULL == (addr = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}
	memset(addr, 0, len);

	for (i = 0; i < len; i++) {
		*(addr + i) = strtol(argv[i + 4], NULL, 16);
	}

	if (CSI2C_Write(hi2c, subaddr, addr, len)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}

	free(addr);

	printf("I2C_WRITE succeed\n");

	if (CSI2C_Close(hi2c)) {
		errstr = CSI2C_GetErrString(hi2c);
		printf("%s  %s\n", __FUNCTION__, errstr);
		return;
	}
	printf("CSI2C_Close succeed\n");
}

/* TODO sunhe: after csi2c updated, add devid parameter. */
static struct cmd_t csi2c_tt[] = {
	{
	 "i2c_conf",
	 NULL,
	 I2C_CONF,
	 "i2c_conf - get/set to set/get I2C configuration, including speed, write delay, subaddress, and loop flags. \
	\n    usage: I2C_info <id>(set/get) <speed (1:100k, 2:400k)><write delay us><subaddress (>0)><loop (0 or 1)> \
	\n       eg: i2c_conf 0 set 1 1000000 1 0 \
	\n       eg: i2c_conf 0 get \n"},

	{
	 "i2c_r",
	 NULL,
	 I2C_READ,
	 "I2C_read - read data  from an I2C device \
	\n    usage: I2C_read <id><read length> \
	\n       eg: i2c_r  0 0x3 \n"},

	{
	 "i2c_re",
	 NULL,
	 I2C_READEXT,
	 "I2C_readext - read data from an I2C device \
	\n    usage: I2C_readext <id><subaddress><read length> \
	\n       eg: i2c_re 0 0x5 0x2 \n"},

	{
	 "i2c_w",
	 NULL,
	 I2C_WRITE,
	 "I2C_write - write a I2C device \
	\n    usage: I2C_write <id><subaddr><write length><data>......<data> \
	\n       eg: i2c_w 0 0 0x1 0x5 \n"}
};
