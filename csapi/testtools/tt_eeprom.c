#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
 
#include "csi2c.h"

static void EEPROM_READ(int argc, char *argv[])
{
	int dev_fd;
	char *addr = NULL;
	int len = 0;
	int offset = 0;
	int i = 0;

	if (argc != 3) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	offset = strtol(argv[1], NULL, 16);
	len = strtol(argv[2], NULL, 16);

	if (NULL == (addr = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}
	memset(addr, 0, len);

	dev_fd = open("/dev/misc/eeprom", O_RDWR);
	if (dev_fd < 0) {
		printf("open eeprom device error : %s\n", strerror(errno));
		return;
	}

	lseek(dev_fd, offset, SEEK_SET);
	len = read(dev_fd, addr, len);
	if (len >= 0) {
		printf("successfully read length is %d\n", len);
		printf("%s succeed\n", __FUNCTION__);
	}
	else {
		printf("read eeprom error %s\n", strerror(errno));
	}

	for (i = 0; i < len; i++) {
		printf("read data %d: 0x%x\n", i, addr[i]);
	}

	free(addr);

	close(dev_fd);
}

static void EEPROM_WRITE(int argc, char *argv[])
{
	int dev_fd;
	char *addr = NULL;
	int len = 0;
	int offset = 0;
	int i = 0;

	if (argc < 3) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	offset = strtol(argv[1], NULL, 16);
	len = strtol(argv[2], NULL, 16);

	if (NULL == (addr = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}
	memset(addr, 0, len);

	for (i = 0; i < len; i++) {
		*(addr + i) = strtol(argv[i + 3], NULL, 16);
	}

	dev_fd = open("/dev/misc/eeprom", O_RDWR);
	if (dev_fd < 0) {
		printf("open eeprom device error : %s\n", strerror(errno));
		return;
	}

	lseek(dev_fd, offset, SEEK_SET);
	len = write(dev_fd, addr, len);
	if (len >= 0) {
		printf("successfully write length is %d\n", len);
		printf("%s succeed\n", __FUNCTION__);
	}

	else {
		printf("write eeprom error %s\n", strerror(errno));
	}

	free(addr);

	close(dev_fd);
}

static void EEPROM_ERASE(int argc, char *argv[])
{
	int dev_fd;
	int len = 0;
	int offset = 0;
	char *erase_data = NULL;

	if (argc != 3) {
		printf("%s  paramater error!\n", __FUNCTION__);
		return;
	}

	offset = strtol(argv[1], NULL, 16);
	len = strtol(argv[2], NULL, 16);

	dev_fd = open("/dev/misc/eeprom", O_RDWR);
	if (dev_fd < 0) {
		printf("open eeprom device error : %s\n", strerror(errno));
		return;
	}

	lseek(dev_fd, offset, SEEK_SET);

	if (NULL == (erase_data = malloc(len))) {
		printf("memory error: %s\n", strerror(errno));
	}

	memset(erase_data, 0, len);

	len = write(dev_fd, erase_data, len);

	if (len >= 0) {
		printf("successfully erase length is %d\n", len);
		printf("%s succeed\n", __FUNCTION__);
	}

	else {
		printf("erase eeprom error %s\n", strerror(errno));
	}

	free(erase_data);
	close(dev_fd);
}

static struct cmd_t eeprom_tt[] = {
	{
	 "eeprom_read",
	 NULL,
	 EEPROM_READ,
	 "eeprom_read - read data from eeprom \
	\n    usage: eeprom_read <offset><len> \
	\n       eg: eeprom_read  0x16 0x4 \n"},

	{
	 "eeprom_write",
	 NULL,
	 EEPROM_WRITE,
	 "eeprom_write - write data to eeprom \
	\n    usage: eeprom_write <offset><len><data>...<data> \
	\n       eg: eeprom_write 0x16 0x4 0x1 0x2 0x3 0x4 \n"},

	{
	 "eeprom_erase",
	 NULL,
	 EEPROM_ERASE,
	 "eeprom_erase - erase data of eeprom \
	\n    usage: eeprom_erase <offset><len> \
	\n       eg: eeprom_erase 0x16 0x4\n"}
};
