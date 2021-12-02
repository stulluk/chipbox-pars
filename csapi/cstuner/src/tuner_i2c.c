 
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "tuner_i2c.h"

#define 	TIMECOUNT	0x200


static int i2c_fd = 0;

#ifdef DEBUG_PRINT
static int mask_flag_print = 1;
#endif

int	c_tuner_i2c_init(void)
{	
	//printf("into c_tuner_i2c_init function called. \n");
		
	i2c_fd = open("/dev/misc/orion_i2c", O_RDWR);
	if(i2c_fd < 0) 
	{
		printf("Error: /dev/misc/orion_i2c.\n");
		return i2c_fd;
	}
	//printf("out c_tuner_i2c_init function called. \n");
	
	return 0;
}


/*This read time mode*/
/* Start | ACK | SubAddr | Ack | Data[0] |Ack .... Data[N] |Ack | Stop*/
int c_tuner_i2c_read(unsigned int address, int subaddr, unsigned int *buffer, unsigned int num)
{
	int subaddr_num = 0x01;
	int icount = 0;
	int retval = 0;
	unsigned char buf_val[24576];

	memset(&buf_val, 0, 24576);

	if(subaddr < 0)
		subaddr_num = 0;
		
	retval = ioctl(i2c_fd, I2C_SET, ((subaddr_num << 16) | address));	/* Chip: 0x50, 2 Byte Address */
	if(retval < 0) 
	{
		printf("22 I2C_SET failed! ret=%d\n", retval);
		return -1;
	}

#if 0
	buf_val[0] = subaddr;
	for(icount = 0; icount < num; icount++)
		buf_val[num - icount] = buffer[icount];
	
	num++;
#endif

	if(subaddr_num > 0)
	{
		retval = lseek(i2c_fd, (unsigned int)(subaddr&0xff), 0);	
		if(retval < 0)
		{
			printf("lseek ret=%d\n", retval);
			return -1;
		}
	}
	
	retval = read(i2c_fd, (unsigned int *)buf_val, num);
	if(retval < 0)
	{
		printf("read ret=%d\n", retval);
		return -1;
	}
	
	tuner_delay_us(400);

	for(icount = 0; icount < num; icount++)
		buffer[icount] = (unsigned int)buf_val[icount];

#if 0
	printf("read address = 0x%x subaddr = 0x%x sum = %d  ", address, subaddr, num);
	for(icount = 0; icount < num; icount++)
		printf("c_tuner_i2c_read buf_val[%d] = 0x%x   ", icount, buf_val[icount]);		
	printf("\n");	
#endif

	return 0;
}


int tuner_write_mask(int addr, int subaddr, int mask, int data)
{
	int tmp_val = 0;
	int ret = 0;
	int val = 0;
	
	if((mask & 0xff) == 0xff)
	{
		val = data;
		//printf("tuner_write_mask subaddr=0x%x, mask=0x%x, data=0x%x val=0x%x\n", subaddr, mask, data, val);
		
		ret = c_tuner_i2c_write(addr, (unsigned int)(subaddr&0xff), &val, 1);
		if(ret < 0)
			printf("Error: mask.c_tuner_i2c_write\n");

		return ret;
	}

	// read a byte and check
	c_tuner_i2c_read(addr, (unsigned int)(subaddr&0xff), (int *)&val, 1);	
	tmp_val = val;
	if (val < 0)
	{
		printf("Error: c_tuner_i2c_read\n");
		return val;
	}

	//mask if off
	val = (val & (~mask));
	val = (val | (data & mask));

#ifdef DEBUG_PRINT	
//if( subaddr == 0x70)
{
	printf("tuner_write_mask addr = 0x%x subaddr = 0x%x mask = 0x%x data = 0x%x tmp_val = 0x%x val = 0x%x\n", addr, subaddr, mask, data, tmp_val, val);
	mask_flag_print = 0;
}	
#endif

	// write it out again
	ret = c_tuner_i2c_write(addr, (unsigned int)(subaddr&0xff), &val, 1);
	if(ret < 0)
		printf("Error: c_tuner_i2c_write.\n");
	
	return ret;
	
}


int c_tuner_i2c_write(unsigned int address, int subaddr, unsigned int *buffer, unsigned int num)
{
	int icount = 0;
	int  retval = 0;
	int subaddr_num = 1;
	unsigned char buf_val[24576];


	if(subaddr < 0)
		subaddr_num = 0;

	retval = ioctl(i2c_fd, I2C_SET, ((subaddr_num << 16) | address));	 
	if(retval < 0) 
	{
		printf("I2C_SET failed! ret=%d\n", retval);
		return retval;
	}

	for(icount = 0; icount < num; icount++)
		buf_val[icount] = (unsigned char)buffer[icount];

	if(subaddr_num > 0)
	{
		retval = lseek(i2c_fd, (unsigned int)(subaddr&0xff), 0);	
		if(retval < 0)
		{
			printf("lseek ret=%d\n", retval);
			return -1;
		}
	}	

#ifdef DEBUG_PRINT
//	if(num > 100)
//		mask_flag_print = 0;

	//if(mask_flag_print)
	if(num < 100)	
	{
		printf("writ address = 0x%x subaddr = 0x%x sum = %d  ", address, subaddr, num);
		for(icount = 0; icount < num; icount++)
			printf("c_tuner_i2c_writ buf_val[%d] = 0x%x   ", icount, buf_val[icount]);		
		printf("\n");
	}
//	mask_flag_print = 1;
#endif

	retval = write(i2c_fd, &buf_val, num);
	if(retval < 0)
	{
		printf("write failed ret=%d\n", retval);
		return -1;
	}
	
	tuner_delay_us(800);

#if 0
if(num < 100)
{

	if(address != 0x60)
	{
		memset(&buf_val, 0, 1000);
		if(num > 100)
		{
			printf("c_tuner_i2c_write num = %d\n", num);
			num = 100;
		}
		
		c_tuner_i2c_read(address, (unsigned int)(subaddr&0xff), (unsigned int *)&buf_val, num);
		printf("read address = 0x%x subaddr = 0x%x sum = %d  ", address, subaddr, num);

		for(icount = 0; icount < num; icount++)
			printf("c_tuner_i2c_read buf_val[%d] = 0x%x   ", icount, buf_val[icount]);		
		printf("\n");	
	}
	else if(address == 0x60)
	{
		tuner_delay_us(3000);
		subaddr = -1;
		num = 1;
		
		memset(&buf_val, 0, 1000);
		if(num > 100)
		{
			printf("c_tuner_i2c_write num = %d\n", num);
			num = 100;
		}
		
		c_tuner_i2c_read(address, (unsigned int)(subaddr&0xff), (unsigned int *)&buf_val, num);
		printf("read address = 0x%x subaddr = 0x%x sum = %d  ", address, subaddr, num);

		for(icount = 0; icount < num; icount++)
			printf("c_tuner_i2c_read buf_val[%d] = 0x%x   ", icount, buf_val[icount]);		
		printf("\n");	

	}

}	
#endif


	return 0;
}


void tuner_delay_us(unsigned int count)
{
	unsigned int	i = 0;
	unsigned int	j = 0;

	for(i=0; i<count; i++)
	{
		for(j=0; j<0x8; j++)
		{
			;
		}
		
	}
}
