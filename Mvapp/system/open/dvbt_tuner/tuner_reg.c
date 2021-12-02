#include "linuxos.h"

//#include "csi2c.h"
#include "tuner_reg.h"

#define I2C_TIMEOUT                (2)

static  CSOS_Semaphore_t 		*sem_RegAccess = NULL;
CSI2C_HANDLE				Demod_IOHandle = NULL;

tReg_Info_t 					*Reg_List = NULL;
U16							Reg_Count = 0;

static void i2c_delay_us(unsigned int count)
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


int Reg_Init(tReg_OpenParams_t * params)
{
	int	error = CS_NO_ERROR;
            CSI2C_Attr      param;

	if(params == NULL)
		return(CS_ERROR_BAD_PARAMETER);

        tuner_gpio_reset();

	Reg_List = (tReg_Info_t *)CSOS_AllocateMemory(NULL, sizeof(tReg_Info_t)*params->Reg_Count);
	if(Reg_List == NULL)
		return(CS_ERROR_NO_MEMORY);

	Reg_Count = params->Reg_Count;

	sem_RegAccess  = CSOS_CreateSemaphoreFifo (NULL, 1 );

        Demod_IOHandle = CSI2C_Open(params->Dev_Address);

        if(Demod_IOHandle == NULL)
            {
                return CS_ERROR_INVALID_HANDLE;
            }

         param.speed = 1;
	param.loop = 0;
	param.subaddr_num = 1;
	param.write_delayus = 0;

        CSI2C_SetAttr(Demod_IOHandle, &param);

	//printf("demod i2c open error = %d\n", error);

	return(error);

}

int Reg_AddDemodReg(U32 Id, U8 address, U8 def_value)
{
	tReg_Info_t *pReg;

	if(Id >= Reg_Count)
		return(CS_ERROR_BAD_PARAMETER);
	
	pReg=&Reg_List[Id];
	
	pReg->Address = address;
	pReg->Default_Value = def_value;

	return(CS_NO_ERROR);
	
}

int Reg_ApplyDefault(void)
{
	U32 reg;

	for ( reg = 0 ; reg < Reg_Count; reg++ )
		Reg_SetOneDemodReg(reg, Reg_List[reg].Default_Value);

	return(CS_NO_ERROR);
}


int Reg_SetOneDemodReg(U32 reg_id, U8 Data)   
{ 
	int 	error = CS_NO_ERROR;
	U8		data_buffer;
	U32       	actlen;

	if(reg_id >= Reg_Count)
		return(CS_ERROR_BAD_PARAMETER);

    CSOS_WaitSemaphore (sem_RegAccess);
	
	//data_buffer[0] = Reg_List[reg_id].Address;
	data_buffer = Data; 	
	
	error = CSI2C_Write(Demod_IOHandle, Reg_List[reg_id].Address, &data_buffer, 1);

	if ( error == CSAPI_SUCCEED)
	{
		Reg_List[reg_id].Current_Value = Data;
	}
        
	CSOS_SignalSemaphore (sem_RegAccess);
        i2c_delay_us(6000);  
            

	if(error!=CSAPI_SUCCEED)  printf("=>>Reg_SetOneDemodReg =>>> %x\n",error);

	return(error);
		
}

int Reg_GetOneDemodReg(U32 reg_id, U8 * Data)
{
	U32       actlen;	
	int error = CS_NO_ERROR;
	U8	data_buffer;

	if(reg_id >= Reg_Count)
		return(CS_ERROR_BAD_PARAMETER);
	
	CSOS_WaitSemaphore (sem_RegAccess);
#if 0		
	error = I2C_WriteNoStop(Demod_IOHandle, &reg_addr, 1, &actlen, I2C_TIMEOUT);
	if ( error == CS_NO_ERROR)
	{
		error = I2C_Read(Demod_IOHandle, &(Reg_List[reg_id].Current_Value), 1, &actlen, I2C_TIMEOUT);
		* Data = Reg_List[reg_id].Current_Value;
	}
#else
	data_buffer = Reg_List[reg_id].Address;
	error = CSI2C_Read(Demod_IOHandle, Reg_List[reg_id].Address, &data_buffer, 1 );

	* Data = data_buffer;
	Reg_List[reg_id].Current_Value = data_buffer;
#endif
	CSOS_SignalSemaphore (sem_RegAccess);

        i2c_delay_us(6000);  

	if(error!=CSAPI_SUCCEED)  printf("=>>Reg_GetOneDemodReg  ===> %x\n",error);
	
	return(error);
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


int tuner_gpio_reset(void)
{
	int  retval = 0;
	char value = '0';
	// retval = _gpio_write("/dev/gpio/6",&value,1);
	printf("write gpio 6 to 0\n");
	usleep(1000*1000);
	value = '1';
	// retval = _gpio_write("/dev/gpio/6", &value,1);
	usleep(1000*1000);
	printf("write gpio 6 to 1\n");
	return 0;	
}



