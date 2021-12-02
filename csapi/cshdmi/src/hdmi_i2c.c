#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "hdmi_i2c.h"

#define 	TIMECOUNT	0x200
#define     I2C_SET     _IOW('I', 0, unsigned long)


#define I2C_SPEED_100K 1
#define I2C_SPEED_400K 2
#define i2c_set_speed_100k(x)  x = (((unsigned long)1<<20) + (unsigned long)((unsigned long)(x) & 0xff0fffff))
#define i2c_set_speed_400k(x)  x = (((unsigned long)1<<21) + (unsigned long)((unsigned long)(x) & 0xff0fffff))
#define i2c_get_speed(x)  ((unsigned long)x >> 20) & 0xf

static int hdmi_i2c_fd = 0;

void _delay_us(unsigned int count)
{
    unsigned int    i = 0;
    unsigned int    j = 0;

    for(i=0; i<count; i++)
    {
        for(j=0; j<0x8; j++)
        {
            ;
        }

    }
}  

int	hdmi_i2c_open(void)
{	
		
 	hdmi_i2c_fd = open("/dev/misc/orion_i2c", O_RDWR);
 	if(hdmi_i2c_fd < 0) 
 	{
 		printf("Error: /dev/misc/orion_i2c.\n");
 		return hdmi_i2c_fd;
 	}
	
    //  CSI2C_Open()
	return 0;
}


int hdmi_i2c_read(unsigned char address, int subaddr, char *buffer, unsigned int num)
{
	int subaddr_num = 0x01;
	int retval = 0;
	unsigned long private_data;
	if(subaddr < 0)
		subaddr_num = 0;
	
	private_data = ((subaddr_num << 16) | address);
	i2c_set_speed_100k(private_data);

	retval = ioctl(hdmi_i2c_fd, I2C_SET, private_data);	/* Chip: 0x39 or 0x3B*/
	if(retval < 0) 
	{
		printf("I2C_SET failed! ret=%d\n", retval);
		return -1;
	}

	if(subaddr_num > 0)
	{
		retval = lseek(hdmi_i2c_fd, subaddr, 0);	
		if(retval < 0)
		{
			printf("lseek ret=%d\n", retval);
			return -1;
		}
	}
	retval = read(hdmi_i2c_fd, buffer, num);
	if(retval < 0)
	{
		printf("read ret=%d\n", retval);
		return -1;
	}
	_delay_us(800);
	return 0;
}


int hdmi_i2c_read_byte(unsigned char address, int subaddr, char *buffer)
{
	int subaddr_num = 0x01;
	int retval = 0;
	unsigned long private_data;
	if(subaddr < 0)
		subaddr_num = 0;
	private_data = ((subaddr_num << 16) | address);
	i2c_set_speed_100k(private_data);
	
	retval = ioctl(hdmi_i2c_fd, I2C_SET, private_data);	/* Chip: 0x39 or 0x3B*/
	if(retval < 0) 
	{
		printf("22 I2C_SET failed! ret=%d\n", retval);
		return -1;
	}

	if(subaddr_num > 0)
	{
		retval = lseek(hdmi_i2c_fd, subaddr, 0);	
		if(retval < 0)
		{
			printf("lseek ret=%d\n", retval);
			return -1;
		}
	}

	retval = read(hdmi_i2c_fd, buffer, 1);
	if(retval < 0)
	{
		printf("read ret=%d\n", retval);
		return -1;
	}
	
   _delay_us(800);
	return 0;
}



int hdmi_i2c_write(unsigned char address, unsigned int subaddr, unsigned char *buffer, unsigned int num)
{
	
	int  retval = 0;
	unsigned long private_data;
	private_data = ((0x1<< 16) | address);
	i2c_set_speed_100k(private_data);

	retval = ioctl(hdmi_i2c_fd, I2C_SET, ((0x01 << 16) | address));	/* Chip address: 0x39*/
	if(retval < 0) 
	{
		printf("I2C_SET failed! ret=%d\n", retval);
		return retval;
	}
	retval = lseek(hdmi_i2c_fd, subaddr, 0);
	if(retval < 0)
	{
		printf("lseek ret=%d\n", retval);
		return -1;
	}
	retval = write(hdmi_i2c_fd, buffer, num);
	if(retval < 0)
	{
		printf("write failed ret=%d\n", retval);
		return -1;
	}
	_delay_us(2000);
	return 0;
}


int hdmi_i2c_write_byte(unsigned char address, unsigned int subaddr, unsigned char byte)
{
	int  retval = 0;
	unsigned long private_data;

	private_data = ((0x1 << 16) | address);
	i2c_set_speed_100k(private_data);

	retval = ioctl(hdmi_i2c_fd, I2C_SET, private_data);	/* Chip address: 0x39*/
	if(retval < 0) 
	{
		printf("I2C_SET failed! ret=%d\n", retval);
		return retval;
	}

	_delay_us(20);
	retval = lseek(hdmi_i2c_fd, subaddr, 0);
	if(retval < 0)
	{
		printf("lseek ret=%d\n", retval);
		return -1;
	}
	_delay_us(20);
	retval = write(hdmi_i2c_fd, &byte, 1);
	if(retval < 0)
	{
		printf("write failed ret=%d\n", retval);
		return -1;
	}
	
	_delay_us(2000);
	return 0;
}

