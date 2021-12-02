/*
    Copyright @ Celestial 2007
	by xiaodong.fan@celestialsmei.cn
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>
#include "hdmi_gpio.h"

static inline int _gpio_open(char *devname);

 int _gpio_write_onebit(char * devname, int value);
 int _gpio_read(char *gpionum,char *buf);
static int _gpio_write(char *gpionum, char* buf, int len );

int hdmi_gpio_reset(void)
{
	int retval = 0;
	char value = '0';
	retval = _gpio_write("/dev/gpio/15",&value,1);
	
	usleep(20000);
	value = '1';
	retval = _gpio_write("/dev/gpio/15", &value,1);
	usleep(3000);
	puts("write gpio 15 to 1");
 
	return 0;	
}

static inline int _gpio_open(char *devname)
{	
	int gpio_fd;
	gpio_fd = open(devname, O_RDWR);
	if(gpio_fd < 0) 
	{
		printf("Open Error: %s.\n",devname);
		
	}

	return gpio_fd;
}

 int _gpio_read(char *devname, char *buf)
{
	int gpio_fd;
	int retval;

	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Read Error: Open %s.\n",devname);
		return gpio_fd;
	}
	retval = read(gpio_fd, buf, 1);
	if (retval != 1)
	{
		printf("Read Error: Read %s. \n",devname);
	}
	close(gpio_fd);
	return retval;
	
	
}

static int _gpio_write(char *devname, char* buf, int len )
{
	int gpio_fd;
	int retval;
	char cmd='O';
    
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Write Error: Open %s.\n",devname);
		return -1;
	}
	
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Write Error: write cmd %s. \n",devname);
		return -1;
	}

	retval = write(gpio_fd, buf , len);
	if (retval != len)
	{
		printf("Write Error: write data %s. \n",devname);
		return -1;
	}
    
    retval = close(gpio_fd);

	return len;
}

 int _gpio_write_onebit(char * devname, int value)
{
	int gpio_fd;
	int retval;
	char cmd ='o';
	struct gpio_cmd{
		int cmd;
		int value;
		} gpio_arg;
	
	gpio_fd = open(devname,O_RDWR);
	if (gpio_fd <= 0)
	{
		printf("Write Error: Open %s.\n",devname);
		return -1;
	}
	gpio_arg.cmd=0x02;
	gpio_arg.value = value;
	ioctl(gpio_fd,0x02,&gpio_arg);

	
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Write Error: write %s. \n",devname);
		return -1;
	}
    retval = close(gpio_fd);
	return 0;
}

