#include "linuxos.h"
#include "isdb_i2capi.h"


static U8 TC90507_ucAddress=TC90507_DEVICE_ADDR;
static int i2c_fd=0;


int	ALPS_i2c_init(void)
{	

	i2c_fd = open("/dev/misc/orion_i2c", O_RDWR);
	if(i2c_fd < 0) 
	{
		printf("Error: /dev/misc/orion_i2c.\n");
		return i2c_fd;
	}

	return 0;
}

static void i2c_delay_us(unsigned int count)
{
	usleep(count*1000);
}

static int ALPS_I2C_RegisterWriteArray(unsigned int address, int subaddr, unsigned int num, unsigned char *buffer)
{
    int icount = 0;
    int  retval = 0;
    unsigned char buf_val[245];

     retval = ioctl(i2c_fd, I2C_SET, (address>>1));  /* Chip: 0x50, 2 Byte Address */
    if(retval < 0) 
    {
         printf("I2C_SET failed! ret=%d\n", retval);
         return ALPS_I2C_ERR_INDEX;
    }

	    buf_val[0]=subaddr;
	    for(icount = 0; icount < num; icount++)
	     buf_val[icount+1] = buffer[icount];

    
     
    num+=1;
    retval = write(i2c_fd, buf_val, num);
    if(retval < 0)
    {
         printf("write failed1 ret=%d,buf0=%x\n", buf_val[1],subaddr);
         return ALPS_I2C_ERR_INDEX;
    }
    fflush(stdout);
    i2c_delay_us(10); 


    return ALPS_I2C_ERR_NON;

}



/*This read time mode*/
/* Start | ACK | SubAddr | Ack | Data[0] |Ack .... Data[N] |Ack | Stop*/
static int ALPS_I2C_RegisterReadArray(unsigned int address, int subaddr, unsigned int num, unsigned char *buffer)
{
    int subaddr_num = 0x01;
    int icount = 0;
    int retval = 0;
    unsigned char buf_val[245];

    memset(&buf_val, 0, 245);

    retval = ioctl(i2c_fd, I2C_SET, (address>>1));	 /* Chip: 0x50, 2 Byte Address */
    if(retval < 0) 
    {
         printf("22 I2C_SET failed! ret=%d\n", retval);
         return ALPS_I2C_ERR_INDEX;
    }

    if(subaddr >=0)
 	{
        retval = write( i2c_fd , &subaddr , 1 );
        if(retval < 0)
        {
             printf("read  write reg ret=%d\n", retval);
             return ALPS_I2C_ERR_INDEX;
        }
 	}

    retval = read(i2c_fd, buf_val, num);
    if(retval < 0)
    {
         printf("read ret=%d\n", retval);
         return ALPS_I2C_ERR_INDEX;
    }
    i2c_delay_us(10);

    for(icount = 0; icount < num; icount++)
    buffer[icount] = buf_val[icount];
	
    return ALPS_I2C_ERR_NON;	 
}



int TC90507_I2C_OpenPort(void)
{
	int iRetVal=ALPS_I2C_ERR_NON;	

	return iRetVal;
}

void TC90507_I2C_ClosePort(void)
{

}

void TC90507_Set_SlaveAddress(U8 ucAddr)
{
	TC90507_ucAddress=ucAddr;
}

void TC90507_Get_SlaveAddress(U8 *pucAddr)
{
	*pucAddr=TC90507_ucAddress;
}

int TC90507_RegWrite8(U8 ucRegAddr8, U8 ucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8 ucBuffer;

	ucBuffer=ucData8;
	iRetVal=ALPS_I2C_RegisterWriteArray(TC90507_ucAddress, ucRegAddr8, (int)1, &ucBuffer);

	return iRetVal;
}

int TC90507_RegWrite8Array(U8 ucRegAddr8, U8 ucDataLen, U8 *pucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;


	iRetVal=ALPS_I2C_RegisterWriteArray(TC90507_ucAddress, ucRegAddr8, ucDataLen, pucData8);

	return iRetVal;
}

int TC90507_RegRead8(U8 ucRegAddr8, U8 *pucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	

	iRetVal=ALPS_I2C_RegisterReadArray(TC90507_ucAddress, ucRegAddr8, 1, pucData8);
	
	return iRetVal;
}

int  TC90507_RegRead8Array(U8 ucRegAddr8, U8 ucDataLen, U8 *pucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;


	iRetVal=ALPS_I2C_RegisterReadArray(TC90507_ucAddress, ucRegAddr8, ucDataLen, pucData8);

	return iRetVal;
}

int TC90507_RegWriteMaskBit8(U8 ucRegAddr8, U8 ucMask8, U8 ucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	U8	ucBuffer;

	iRetVal=TC90507_RegRead8(ucRegAddr8,&ucBuffer);
	if(iRetVal!=ALPS_I2C_ERR_NON)
		{
			printf("MaskBit8 read error\n");
			return iRetVal;
		}
	ucBuffer=ucBuffer&(~ucMask8);
	ucBuffer=ucBuffer|(ucMask8&ucData8);
	iRetVal=ALPS_I2C_RegisterWriteArray(TC90507_ucAddress, ucRegAddr8, 1, &ucBuffer);
	
	return iRetVal;
}

// Return Value : ALPS_I2C_ERR_NON, ALPS_I2C_ERR_DEMOD_UNACK, ALPS_I2C_ERR_MOP_WRITE_UNACK
int TC90507_Bypass_WriteArray(U8 ucDataLen, U8 *pucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;
	
	iRetVal=ALPS_I2C_RegisterWriteArray(TC90507_ucAddress, 0xFE, ucDataLen, pucData8);

	return iRetVal;
}

// Return Value : ALPS_I2C_ERR_DEMOD_UNACK, ALPS_I2C_ERR_MOP_READ_UNACK
 int TC90507_Bypass_ReadArray(U8 ucSubAddr, U8 ucDataLen, U8 *pucData8)
{
	int iRetVal=ALPS_I2C_ERR_NON;

	iRetVal=ALPS_I2C_RegisterReadArray(TC90507_ucAddress, 0xFE, 1, pucData8);

	return iRetVal;
}


static int isdb_gpio_write(char *devname, char* buf, int len )
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
/*
	cmd= 'o';
	retval = write(gpio_fd, &cmd , 1);
	if (retval != 1)
	{
		printf("Error: Read %s. \n",devname);
		return -1;
	}
*/

    retval = close(gpio_fd);

	return len;
    
}


int TC90507_gpio_reset(void)
{
	int  retval = 0;
	char value = '0';
	// retval = isdb_gpio_write("/dev/gpio/6",&value,1);
	printf("write gpio 6 to 0\n");
	usleep(400000);
	value = '1';
	// retval = isdb_gpio_write("/dev/gpio/6", &value,1);
	usleep(400000);
	printf("write gpio 6 to 1\n");
	return 0;	
}

