#include "linuxos.h"

#include "lgs_types.h"
//#include "tuner_i2c.h"
#include "lgs8913.h"

//#define GI_AUTODETECT

#define LGS_Debug(x) 

#define FILE_INFO
UINT8 g_lgsBaseAddress = 0;


LGS_HANDLE 	g_lgsHandleI2c = 0;
LGS_OPEN_I2C 	LGS_OpenI2C = NULL;
LGS_READ_I2C 	LGS_ReadI2C = NULL;
LGS_WRITE_I2C 	LGS_WriteI2C = NULL;
LGS_CLOSE_I2C 	LGS_CloseI2C = NULL;

/*********cs i2c private access**********/
#if defined(USE_CH_TUNER)
#define             TUNER_I2C_ADDR          0xc0
#elif defined(USE_SHARP_TUNER)
#define             TUNER_I2C_ADDR          0xc2
#endif
#define             DEMOD_SEC1_I2C_ADDR_OFFSET          0x2
#define             DEMOD_SEC2_I2C_ADDR_OFFSET          0x6

static CSI2C_HANDLE pll_i2c_handle;
static CSI2C_HANDLE demod_sec1_i2c_handle;
static CSI2C_HANDLE demod_sec2_i2c_handle;

static CSOS_Semaphore_t*  sem_Demod_ic_access = NULL ;

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




int tuner_i2c_init(void)
{
	CSI2C_Attr tuner_handle_attr, demod_handle_attr;

        sem_Demod_ic_access = CSOS_CreateSemaphoreFifo (NULL, 1);
	
	pll_i2c_handle = CSI2C_Open(TUNER_I2C_ADDR >> 1);
	if (pll_i2c_handle == NULL) 
		return -1;

	demod_sec1_i2c_handle = CSI2C_Open((g_lgsBaseAddress + DEMOD_SEC1_I2C_ADDR_OFFSET) >> 1);
	if (demod_sec1_i2c_handle == NULL) 
		return -1;

        demod_sec2_i2c_handle = CSI2C_Open((g_lgsBaseAddress + DEMOD_SEC2_I2C_ADDR_OFFSET) >> 1);
	if (demod_sec2_i2c_handle == NULL) 
		return -1;
		
	tuner_handle_attr.speed = 1;
	tuner_handle_attr.loop = 0;
	tuner_handle_attr.subaddr_num = 0;
	tuner_handle_attr.write_delayus = 0;
	CSI2C_SetAttr(pll_i2c_handle, &tuner_handle_attr);

	demod_handle_attr.speed = 1;
	demod_handle_attr.loop = 0;
	demod_handle_attr.subaddr_num = 0;
	demod_handle_attr.write_delayus = 0;
	CSI2C_SetAttr(demod_sec1_i2c_handle, &demod_handle_attr);
	CSI2C_SetAttr(demod_sec2_i2c_handle, &demod_handle_attr);

	return 0;
}

static int  tuner_i2c_read(UINT8 address, int subaddr, unsigned char *buffer, unsigned int num)
{
        CSI2C_HANDLE    handle;
        CSAPI_RESULT        err;

        if((buffer == NULL)||(num == 0))
            return -1;

        CSOS_WaitSemaphore(sem_Demod_ic_access);
        
        if(address == TUNER_I2C_ADDR)
            {
                handle = pll_i2c_handle;

                err = CSI2C_Read(handle, subaddr, buffer, num);

                if(err != CSAPI_SUCCEED)
                    {
                        printf("CSI2C_Read err = %d\n", err);
                    }
            }
        else 
            {
                char     sub_addr;
                
                if(address == (g_lgsBaseAddress + DEMOD_SEC1_I2C_ADDR_OFFSET))
            {
                handle = demod_sec1_i2c_handle;
            }
        else
            {
                handle = demod_sec2_i2c_handle;
            }

                sub_addr = subaddr;

                err = CSI2C_Write(handle, 0, &sub_addr, 1);
                if(err != CSAPI_SUCCEED)
                    {
                        printf("CSI2C_Write err = %d\n", err);
                    }

                err = CSI2C_Read(handle, 0, buffer, num);
                if(err != CSAPI_SUCCEED)
                    {
                        printf("CSI2C_Read err = %d\n", err);
                    }

            }

        CSOS_SignalSemaphore(sem_Demod_ic_access);

	i2c_delay_us(6000);
        
        

	return 0;
}

static int tuner_i2c_write(UINT8 address, unsigned int subaddr, unsigned char *buffer, unsigned int num)
{
        CSI2C_HANDLE    handle;
        CSAPI_RESULT        err;

        if((buffer == NULL)||(num == 0))
            return -1;

        CSOS_WaitSemaphore(sem_Demod_ic_access);
        
        if(address == TUNER_I2C_ADDR)
            {
                handle = pll_i2c_handle;

                err = CSI2C_Write(handle, subaddr, buffer, num);
                if(err != CSAPI_SUCCEED)
                    {
                        printf("CSI2C_Write err = %d\n", err);
                    }
            }
        else 
            {
                char *  tempbuffer;
                
                if(address == (g_lgsBaseAddress + DEMOD_SEC1_I2C_ADDR_OFFSET))
            {
                handle = demod_sec1_i2c_handle;
            }
        else
            {
                handle = demod_sec2_i2c_handle;
            }

                tempbuffer = malloc(num +1);

                if(tempbuffer == NULL)
                    {
        CSOS_SignalSemaphore(sem_Demod_ic_access);
                        return -1;
                    }

                tempbuffer[0] = subaddr;
                memcpy(&(tempbuffer[1]), buffer, num );
         
                err = CSI2C_Write(handle, 0, tempbuffer, (num+1));
                if(err != CSAPI_SUCCEED)
                    {
                        printf("CSI2C_Write err = %d\n", err);
                    }

                free(tempbuffer);
            }
        
        CSOS_SignalSemaphore(sem_Demod_ic_access);
	i2c_delay_us(6000);    

	return 0;
}


/************************************/


//LGS_WAIT LGS_Wait = NULL;
void LGS_Wait(UINT16 ms)
{
	usleep(1000*ms);
}

/**
 *compute demodulator I2C address
 *
 *@param	registerAddr	register address
 *@param	psectionAddr	section address
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_ComputeSectionAddress(UINT8 registerAddr, 
			       UINT8 *psectionAddr); 

/**
 *check whether demodulator is auto mode
 *
 *@param 	presult		0	auto mode
 *				1	manual mode
 *@return	macros defined in lgs_8913.h
 */
INT8 LGS_CheckAutoMode(UINT8 *presult);

/**
 *check whether demodulator is locked
 *
 *@param 	presult		0	unlocked
 *				1	locked
 *@return	macros defined in lgs_8913.h
 */

/**
 *set demodulator to auto mode
 *
 *@param 	void
 *@return 	macros defined in lgs_8913.h
 */

/**
 *check whether demodulator has finished auto detect
 *
 *@param	presult		0	undone
 * 				1	done
 *@return	macros defined in lgs_8913.h
 */

/**
 *get demodulator parameters in auto mode
 *
 *@param 	pcontrolFrame		control frame
 *@param	pmode			demodulation mode
 *@param	prate			FEC rate
 *@param 	pinterleaveLength	interleave length
 *@return	macros defined in lgs_8913.h
 */


void LGS_RegisterI2C(LGS_OPEN_I2C open, 
		     LGS_READ_I2C read, 
		     LGS_WRITE_I2C write, 
		     LGS_CLOSE_I2C close)
{
	LGS_OpenI2C = open;
	LGS_ReadI2C = read;
	LGS_WriteI2C = write;
	LGS_CloseI2C = close;
}

void LGS_RegisterWait(LGS_WAIT wait)
{
	;//LGS_Wait = wait;
}

void LGS_DemodulatorBaseAddress(UINT8 CE_A2,
				UINT8 CE_A1,
				UINT8 CE_A0)
{
	g_lgsBaseAddress = (CE_A2 << 5) | (CE_A1 << 4) | (CE_A0 << 3);
	g_lgsBaseAddress = g_lgsBaseAddress & 0x38;
}

INT8 LGS_ComputeSectionAddress(UINT8 registerAddr, 
			       UINT8 *psectionAddr)
{
	/* Sect 1: 0x00 ~ 0xBF; Sect 2: 0xC0 ~ 0xFF , Jay	*/
	if (registerAddr <= 0xBF) {
		*psectionAddr = g_lgsBaseAddress + DEMOD_SEC1_I2C_ADDR_OFFSET;
	}	
	else if (registerAddr >= 0xC0) {
		*psectionAddr = g_lgsBaseAddress + DEMOD_SEC2_I2C_ADDR_OFFSET;
	}
	else 
		return LGS_REGISTER_ERROR; /* illegal register address */

	return LGS_NO_ERROR; 
}

INT8 LGS_WriteRegister(UINT8 registerAddress, UINT8 registerData)
{
	UINT8 sectionAddr;
  	UINT8 localBuffer[4];
  	INT8 err = LGS_NO_ERROR;

	LGS_Debug((FILE_INFO, "\t Write register(%02x) with (%02x)\n", registerAddress, registerData));

	err = LGS_ComputeSectionAddress(registerAddress, &sectionAddr);
	if ( err != LGS_NO_ERROR )
	{
		LGS_Debug((FILE_INFO, "\t LGS_ComputeSectionAddress is failed!\n"));
		goto failed;
	}

   	localBuffer[0] = registerAddress;
    	localBuffer[1] = registerData;
	   
        err = tuner_i2c_write(sectionAddr,localBuffer[0],&localBuffer[1],1);
        if(err != 0)
        {
        	LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed, register(%02x) value(%02x)!\n",
        	sectionAddr, localBuffer[0], localBuffer[1]));
		goto failed;
        }

failed:
    	return err;
}

INT8 LGS_ReadRegister(UINT8 registerAddress, UINT8 *pregisterData)
{
	UINT8 sectionAddr;
  	UINT8 localBuffer[4];
  	INT8 err = LGS_NO_ERROR;

	LGS_Debug((FILE_INFO, "\t Read register(%02x)\n", registerAddress));

	err = LGS_ComputeSectionAddress(registerAddress, &sectionAddr);
	if ( err != LGS_NO_ERROR )
	{
		LGS_Debug((FILE_INFO, "\t LGS_ComputeSectionAddress is failed!\n"));
		goto failed;
	}
	
	localBuffer[0] = registerAddress;
	   
    	
        err =tuner_i2c_read(sectionAddr, registerAddress, pregisterData, 1);
        if(err != 0)
        {
        	LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
        }

failed:
    	return err;
}

INT8 LGS_SoftReset(void)
{
	INT8 err = LGS_NO_ERROR;

	err = LGS_WriteRegister (0x02, 0x00);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x02 to 0x00!\n"));
		goto failed;
	}
		
	i2c_delay_us(1);
	
	err = LGS_WriteRegister (0x02, 0x01);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x02 to 0x01!\n"));
		goto failed;
	}
		
	//LGS_Wait(200); /* wait 500 ms, allows SNR, AGC, AFC and feedback loops to stabilize */
failed:
	return err;
}

INT8 LGS_CheckAutoMode(UINT8 *presult)
{
	INT8 err = LGS_NO_ERROR;

	err = LGS_ReadRegister (0x7E, presult);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x7E!\n"));
		goto failed;
	}

failed:
	return err;
}

INT8 LGS_CheckLocked(UINT8 *presult)
{
	/* returns true if demod CA locked and AFC Locked, false otherwise */
	INT8 err = LGS_NO_ERROR;

	err = LGS_ReadRegister (0x4B, presult);
    //printf("reg 0x4b = 0x%x\n", *presult);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x7E!\n"));
		goto failed;
	}

	*presult = *presult & 0xC0;		/* Check Flag CA_Locked and AFC_Locked, Jay	*/
	if (*presult == 0xC0)
		*presult = 1;
	else
		*presult = 0;

failed:
	return err;
}

INT8 LGS_GetStatus(UINT8 *pcontrolFrame, UINT8 *pmode, UINT8 *prate, UINT8 *pinterleaverLength, UINT8 *pguardInterval) 
{
	INT8 	err = LGS_NO_ERROR;
	UINT8  	autoDetect; 
	UINT8  	registerData;

	err = LGS_CheckAutoMode (&autoDetect);
	if (autoDetect == 1)
	{
		err = LGS_ReadRegister (0xA2, &registerData);
		if (err != LGS_NO_ERROR)
		{
			LGS_Debug((FILE_INFO, "\t error in reading register 0xA2!\n"));
			goto failed;
		}
		
		registerData = (~registerData & 0x60) | (registerData & 0x9F);		
	}
	else
	{
		err = LGS_ReadRegister (0x7D, &registerData);
		if (err != LGS_NO_ERROR)
		{
			LGS_Debug((FILE_INFO, "\t error in reading register 0x7D!\n"));
			goto failed;
		}
	}
	
	/* sub-carrier */
	*pmode = registerData & RX_SC_MASK;
	
	/* FEC */
	*prate = registerData & RX_FEC_MASK;
		
	/* Time de-interleaver */
	*pinterleaverLength = registerData & 0x60;
		
	/* Control frames */
	*pcontrolFrame = registerData & RX_CF_MASK;
		
	/* Guard interval */
	err = LGS_ReadRegister (0x04, &registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x04!\n"));
		goto failed;
	}
	
	*pguardInterval = registerData & RX_GI_MASK;
		
failed:
	return err;
}

INT8 LGS_SetAutoMode(void)
{

	/* sets the demod to auto-detect mode 
	   using hard reset is a better way to do this
	   returns true if OK, false otherwise */
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;				

	err = LGS_WriteRegister (0xC6, 0x01);
	if (err != LGS_NO_ERROR)
	{	
		LGS_Debug((FILE_INFO, "\t error in writing register 0x03 to 0x00!\n"));
		goto failed;
	}
	err = LGS_ReadRegister (0xC5, &registerData);	/* iteration loop */
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xC5!\n"));
		goto failed;
	}
	registerData = registerData &0xE0;
	err = LGS_WriteRegister (0xC5, registerData);
	if (err != LGS_NO_ERROR)
	{	
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC5[BIT4..BIT0] to 0x00!\n"));
		goto failed;
	}		
	/* master auto-detect reg */
	registerData = 0x01;	
	err = LGS_WriteRegister (0x7E, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x04 to %02x!\n", registerData));
		goto failed;
	}
		
	/* FEC auto-detect */
	registerData = 0x03;	
	err = LGS_WriteRegister (0xC1, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC1 to %02x!\n", registerData));
		goto failed;
	}
	
	err = LGS_ReadRegister (0x7C, &registerData);	/* iteration loop */
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x7C!\n"));
		goto failed;
	}	
	registerData = (registerData & 0x8C) | 0x03;	/* Set as New TPS 2 Group, Jay */
	err =LGS_WriteRegister (0x7C, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x7C to %02x!\n", registerData));
		goto failed;
	}
	
	/* Set BER Test Mode		 */
	err = LGS_ReadRegister (0xC3, &registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xC3!\n"));
		goto failed;
	}
	registerData = (registerData & 0xEF) |  0x10; 
	err = LGS_WriteRegister (0xC3, registerData);	
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC3 to %02x!\n", registerData));
		goto failed;
	}
#if 0	
	err = LGS_SoftReset();
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t LGS_SoftReset is failed!\n"));
		goto failed;
	}
#endif	
failed:
	return err;
}

INT8 LGS_SetManualMode(UINT8 mode, UINT8 rate, UINT8 interleaverLength, UINT8 guardInterval) 
{

	/* reads the current auto-detect parameters and then sets the demod manual mode, 
	   thus turning off auto-detect and making the demod more resilient to noise 
	   returns true if OK, false otherwise */
	
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;
	
	
#ifdef GI_AUTODETECT
	/* Write to manual config to register */
	err = LGS_WriteRegister(0x04, guardInterval);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x04 to %02x!\n", guardInterval));
		goto failed;
	}
	
	/*关闭guard interval自动侦测*/
	err = LGS_WriteRegister (0x03, 00);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x03 to 0x00!\n"));
		goto failed;
	}
#endif	
	if(interleaverLength == RX_TIM_LONG)
	{
		interleaverLength = 0x60;
	}
	else
	{
		interleaverLength = 0x40;
	}

	registerData = interleaverLength | mode | rate;/* set mode, rate, tim reg */

	LGS_Debug((FILE_INFO, "LGS_SetManualMode register(0x7D) is 0x%x\n", registerData));
		
	err = LGS_WriteRegister (0x7D, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x7D to %02x!\n", registerData));
		goto failed;
	}
	
	err = LGS_WriteRegister (0xC0, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC0 to %02x!\n", registerData));
		goto failed;
	}
		
	/* now turn off auto-detect, manual settings take effect */	
	registerData = 0x00;
	err = LGS_WriteRegister (0x7E, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x7E to %02x!\n", registerData));
		goto failed;
	}

	err = LGS_WriteRegister (0xC1, registerData);	/* reg C1 must follow reg 7E */
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC1 to %02x!\n", registerData));
		goto failed;
	}
	
	err = LGS_ReadRegister (0xC5, &registerData);	
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xC5!\n"));
		goto failed;
	}	
	registerData = (registerData & 0xE0) | 0x06;	
	err =LGS_WriteRegister (0xC5, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC5 to %02x!\n", registerData));
		goto failed;
	}
#if 0
	err = LGS_SoftReset();
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t LGS_SoftReset is failed!\n"));
		goto failed;
	}
#endif	
failed:		
	return err;
}

INT8 LGS_AutoDetectDone(UINT8 *presult)
{
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;
	
	err = LGS_ReadRegister (0xA4, &registerData);	/* Address changed */
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xA4!\n"));
		goto failed;
	}
	
	if((registerData & 0x03) == 0x01)
	{
		*presult = 1;
	}
	else
	{
		*presult = 0;
	}
	
failed:
	return err; 
}

INT8 LGS_GetAutoParameters(UINT8 *pcontrolFrame, UINT8 *pmode, UINT8 *prate, UINT8 *pinterleaverLength) 
{	 
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;
	
	err = LGS_ReadRegister (0xA2, &registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xA2!\n"));
		goto failed;
	}
	
	LGS_Debug((FILE_INFO, "parameters is 0x%x\n", registerData));
	
	*pcontrolFrame 		= registerData & RX_CF_MASK;
	*pmode 				= registerData & RX_SC_MASK;
	*prate 				= registerData & RX_FEC_MASK;	
	*pinterleaverLength = registerData & RX_TIM_MASK;	
	
failed:
	return err;
}

INT8 LGS_LoopAutoDetect(UINT8 *pcontrolFrame, UINT8 *pmode, UINT8 *prate, UINT8 *pinterleaverLength, UINT8 *pguardInterval)
{
	UINT8 k;
	INT8 err = LGS_NO_ERROR;
	UINT8 locked, registerData;
#ifdef	GI_AUTODETECT
	err = LGS_ReadRegister (0x44, &registerData);
	if (err != LGS_NO_ERROR)
	{	
		LGS_Debug((FILE_INFO, "get wrong GI\n"));
		/*return;*/
	}
	
	LGS_Debug((FILE_INFO, "GI registerData = 0x%x\n", registerData));
	
	if(registerData == 0)
		*pguardInterval = RX_GI_420;
	else
		*pguardInterval = RX_GI_945;
	
	for (k=0; k<9; k++)
	{
		LGS_Debug((FILE_INFO,"++++++++++++++++++++++++++++++%d\n", k));
		printf("++++++++++++++++++++++++++++++%d\n", k);
		LGS_SetAutoMode();
		LGS_Wait(200);
		err = LGS_AutoDetectDone(&locked);
		if ((err == LGS_NO_ERROR) && (locked == 1))
		{	
			LGS_Debug((FILE_INFO, "LGS_AutoDetectDone() done\n"));
			printf("LGS_AutoDetectDone() done\n");
			err = LGS_CheckLocked(&locked);
			if ((err == LGS_NO_ERROR) && (locked == 1))
			{	
				LGS_Debug((FILE_INFO, "LGS_CheckLocked() locked\n"));
				printf("LGS_CheckLocked() locked\n");
				break;
			}
		}			
	}
	
#else	
	/*关闭guard interval自动侦测*/
	err = LGS_WriteRegister (0x03, 00);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x03 to 0x00!\n"));
		return err;
	}
	
	for (k=0; k<5; k++)
	{
		LGS_Debug((FILE_INFO, "------------------------------%d\n", k));
				
		LGS_WriteRegister(0x04, RX_GI_420);
		LGS_SetAutoMode();
                LGS_SoftReset();
		LGS_Wait(300);
		err = LGS_AutoDetectDone(&locked);
		if ((err == LGS_NO_ERROR) && (locked == 1))
		{	
			LGS_Debug((FILE_INFO, "LGS_AutoDetectDone() 420 done\n"));
			
			err = LGS_CheckLocked(&locked);
			if ((err == LGS_NO_ERROR) && (locked == 1))
			{	
				LGS_Debug((FILE_INFO, "LGS_CheckLocked() 420 locked\n"));
				printf("LGS_CheckLocked()hhhaa  420 locked\n");
				*pguardInterval = RX_GI_420;
				break;
			}
		}	
		
		LGS_WriteRegister(0x04, RX_GI_945);
		LGS_SetAutoMode();
                LGS_SoftReset();
		LGS_Wait(300);	
		err = LGS_AutoDetectDone(&locked);
		if ((err == LGS_NO_ERROR) && (locked == 1))
		{	
			LGS_Debug((FILE_INFO, "LGS_AutoDetectDone() 945 done\n"));
			
			err = LGS_CheckLocked(&locked);
			if ((err == LGS_NO_ERROR) && (locked == 1))
			{	
				LGS_Debug((FILE_INFO, "LGS_CheckLocked() 945 locked\n"));
				printf("LGS_CheckLocked()hhhaa  945 locked\n");
				*pguardInterval = RX_GI_945;
				break;
			}
			
		}			
	}

#endif
	
	if ((err == LGS_NO_ERROR) && (locked == 1))
	{
		LGS_GetAutoParameters(pcontrolFrame, pmode, prate, pinterleaverLength);
	}
        else
            {
                printf("GI NOT LOCKED!!!\n");
            }
	return err;
}

INT8 LGS_ComputePayload(UINT32 *ppayload, UINT8 mode, UINT8 rate, UINT8 guardInterval)
{

	/* compute and return the data payload in bps
	   sc = 4, 16, or 64
	   fec = 49, 23, or 89
	   gi = 420 595, or 945 
	   returns payload if OK, zero otherwise */

	LGS_Debug((FILE_INFO, "mode = %d rate = %d guardInterval = %d\n", mode, rate, guardInterval));
	if (mode == RX_SC_QAM4) /* QPSK */
	{	
		if (rate == RX_FEC_0_4) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 5414400;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 4812800;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_6) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 8121600;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 7219200;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_8) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 10828800;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 9625600;
			}
			else 
			{
				goto failed;
			}
		}
		else
			goto failed;
	}
	else if (mode == RX_SC_QAM16)  /* 16 QAM */
	{
		if (rate == RX_FEC_0_4) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 10828800;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 9625600;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_6) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 16243200;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 14438400;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_8) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 21657600;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 19251200;
			}
			else 
			{
				goto failed;
			}
		}
		else
			goto failed;
	}
	else if (mode == RX_SC_QAM64) /* 64 QAM */
	{ 
		if (rate == RX_FEC_0_4) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 16243200;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 14438400;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_6) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 24364800;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 21657600;
			}
			else 
			{
				goto failed;
			}
		}
		else if (rate == RX_FEC_0_8) 
		{
			if (guardInterval == RX_GI_420) 
			{
				*ppayload = 32486400;
			}
			else if (guardInterval == RX_GI_945) 
			{
				*ppayload = 28876800;
			}
			else 
			{
				goto failed;
			}
		}
		else
			goto failed;
	}
	else
		goto failed;
	
	return 0;
	
failed:
	return LGS_PAYLOAD_ERROR;
}


INT8 LGS_GetMpegMode(UINT8 *pserial, UINT8 *pclkPolarity, UINT8 *pclkGated)
{	
	INT8 err = LGS_NO_ERROR;
	UINT8 result;

	err = LGS_ReadRegister(0xC2, &result);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xC2!\n"));
		goto failed;
	}

	*pserial = result & 0x01;
	*pclkPolarity = result & 0x02;
	*pclkGated = result & 0x04;
	
failed:
	return err;
}

INT8 LGS_SetMpegMode(UINT8 serial, UINT8 clkPolarity, UINT8 clkGated) 
{
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;

	err = LGS_ReadRegister (0xC2, &registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0xC2!\n"));
		goto failed;
	}
	
	registerData = registerData & 0xF8;	
	registerData = registerData | (serial & 0x01) | (clkPolarity & 0x02) | (clkGated & 0x04);

	err = LGS_WriteRegister (0xC2, registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0xC2 to %02x!\n", registerData));
		goto failed;
	}

failed:
	return err;
}

INT8 LGS_ReadEcho(UINT16 framePos, UINT8 *pecho)
{

	/* reads the echo power at the position in the frame indicated
	   by frame pos. 
	   Max frame_pos for guard interval 420 is 0 to 419.
	   Max frame_pos for guard interval 945 is 0 to 944.

	   example: for guard interval 420 call this routine 420 times
	   with frame pos from 0 to 419, and then plot the results with
	   time 0 to 55 us on the x-axis to create an echo graph

	   returns echo if OK, -1 otherwise */

	UINT8 addrMsb = 0;
	UINT8 addrLsb = 0;
	INT8 err = LGS_NO_ERROR;

	
	addrMsb = (framePos >> 8) & 0xFF;	
	addrLsb = framePos & 0xFF;

	err = LGS_WriteRegister(0x83, addrLsb);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x83 to %02x!\n", addrLsb));
		goto failed;
	}
	err = LGS_WriteRegister(0x84, addrMsb);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x84 to %02x!\n", addrMsb));
		goto failed;
	}
	
	err = LGS_ReadRegister(0x94, pecho);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x94!\n"));
		goto failed;
	}

failed:
	return err;
}


UINT16 LGS_ReadBER(void)
{
	UINT16 berval = 0;
	UINT32 ECounter = 0;
	UINT32 TCounter = 0;
	//UINT8  result   = 0;
	UINT8  result0  = 0;
	UINT8  result1  = 0;
	UINT8  result2  = 0;
	UINT8  result3  = 0;

	LGS_WriteRegister( 0xc6, 0x01 );
	LGS_WriteRegister( 0xc6, 0x41 );
    LGS_WriteRegister( 0xc6, 0x01 );
	LGS_Wait(200); //adjust this time according to requisition
	LGS_WriteRegister( 0xc6, 0x81 );

	LGS_ReadRegister( 0xd0, &result0 );
	LGS_ReadRegister( 0xd1, &result1 );
	LGS_ReadRegister( 0xd2, &result2 );
	LGS_ReadRegister( 0xd3, &result3 );

	TCounter = TCounter | result3;
	TCounter = ( TCounter << 8 ) | result2;
	TCounter = ( TCounter << 8 ) | result1;
	TCounter = ( TCounter << 8 ) | result0;

	LGS_ReadRegister( 0xd4, &result0 );
	LGS_ReadRegister( 0xd5, &result1 );
	LGS_ReadRegister( 0xd6, &result2 );
	LGS_ReadRegister( 0xd7, &result3 );

	ECounter = ECounter | result3;
	ECounter = ( ECounter << 8 ) | result2;
	ECounter = ( ECounter << 8 ) | result1;
	ECounter = ( ECounter << 8 ) | result0;

	if( TCounter != 0 )
	{
		berval = ECounter / TCounter;
	}

	if( berval > 100 ) berval = 100;
	return berval;


}


INT8 LGS_OpenTunerI2C(UINT8 tunerAddress)
{
	INT8 err = LGS_NO_ERROR;
	
	err = LGS_WriteRegister(0x01, 0x80 | tunerAddress);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x01 to %02x!\n", 0x80 | tunerAddress));
		goto failed;
	}
	
failed:
	return 0;
}

INT8 LGS_CloseTunerI2C(void)
{
	INT8 err = LGS_NO_ERROR;
	UINT8 registerData;
	
	err = LGS_ReadRegister (0x01, &registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in reading register 0x01!\n"));
		goto failed;
	}
	
	err = LGS_WriteRegister(0x01, 0x7F & registerData);
	if (err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "\t error in writing register 0x01 to %02x!\n", 0x7F & registerData));
		goto failed;
	}
	
failed:
	return 0;
}

#if defined(USE_CH_TUNER)
INT8 LGS_ComputeTunerRegister(UINT16 frequency, 
			      UINT8 *byte1, 
			      UINT8 *byte2, 
			      UINT8 *byte3, 
			      UINT8 *byte4)
{
	UINT16 tunerDivi;
	
	if (frequency >= 213 && frequency <= 264) {
		*byte3 = 0xb4;
		*byte4 = 0x12;
	}
	else if (frequency >= 474/*510*/ && frequency < 735) {
		*byte3 = 0xbc;
		*byte4 = 0x08;
	}
	else if (frequency >= 735 && frequency < 835) {
		*byte3 = 0xf4;
		*byte4 = 0x08;
	}
	else if (frequency >= 835 && frequency <= 896) {
		*byte3 = 0xfc;
		*byte4 = 0x08;
	}
	else
		return LGS_FREQUENCY_ERROR; /* illegal tuner frequency */
	
	
	/*Npro = (freq + 36.1667)*24/4 */
	tunerDivi = (frequency*6) + 217;

	printf("tunerDivi=%x\n",tunerDivi);
	/* now convert the tuner divider into the two tuner divider registers */
	*byte1 = (tunerDivi >> 8) & 0xFF;  /* High byte	*/
	*byte2 = tunerDivi & 0xFF;         /* Low  byte	*/
	
	return LGS_NO_ERROR;
}



INT8 LGS_StartTuner(UINT16 frequency)
{
	INT8 err = LGS_NO_ERROR;
	UINT8 tunerRegister[4];
  	UINT8 byte1, byte2, byte3, byte4;
	unsigned char locked;
		
	err = LGS_ComputeTunerRegister(frequency, &byte1, &byte2, &byte3, &byte4);
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_ComputeTunerRegister is failed!\n"));
		goto failed;
	}	
	
     		  
	err = LGS_OpenTunerI2C(0x60);
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_OpenTunerI2C is failed!\n"));
		goto failed;
	}
#if 0	
	err = LGS_SoftReset();
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_SoftReset is failed!\n"));
		goto failed;
	}
#endif
	tunerRegister[0] = byte1;
	tunerRegister[1] = byte2;
	tunerRegister[2] = byte3;
	tunerRegister[3] = byte4;
	
	err = tuner_i2c_write(TUNER_I2C_ADDR,tunerRegister[0],&tunerRegister[0],3);
    if (err != 0)
    {
        LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
    }
	LGS_Wait(10);

	byte3&=0xdf;
	if((frequency >= 213 && frequency <= 264)||(frequency >= 735 && frequency < 835))
		byte3|=0x08;
	byte4=0xa0;
	tunerRegister[0] = byte1;
	tunerRegister[1] = byte2;
	tunerRegister[2] = byte3;
	tunerRegister[3] = byte4;
	
	err = tuner_i2c_write(TUNER_I2C_ADDR,tunerRegister[0],&tunerRegister[0],3);
	if (err != 0)
    {
        LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
    }

	LGS_Wait(10);

	byte4=0x20;
	tunerRegister[0] = byte1;
	tunerRegister[1] = byte2;
	tunerRegister[2] = byte3;
	tunerRegister[3] = byte4;
	
	err = tuner_i2c_write(TUNER_I2C_ADDR,tunerRegister[0],&tunerRegister[0],3);
	if (err != LGS_NO_ERROR)
    {
        LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
    }

	LGS_Wait(10);		/* wait 100 ms */
    
    err = LGS_CloseTunerI2C();
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_CloseTunerI2C is failed!\n"));
		goto failed;
	}
	
failed:	
  	return err;
}

#elif defined(USE_SHARP_TUNER)
INT8 LGS_ComputeTunerRegister(UINT16 frequency, 
			      UINT8 *ppd1, 
			      UINT8 *ppd2, 
			      UINT8 *pcb1, 
			      UINT8 *pcb2, 
			      UINT8 *pcb3a, 
			      UINT8 *pcb3b) 
{
	UINT16 tunerDivi;
	
	if (frequency >= 434 && frequency < 578) {
		*pcb2 = 0x94;
	}
	else if (frequency >= 578 && frequency < 650) {
		*pcb2 = 0xB4;
	}
	else if (frequency >= 650 && frequency < 746) {
		*pcb2 = 0xD4;
	}
	else if (frequency >= 746 && frequency <= 858) {
		*pcb2 = 0xF4;
	}
	else
		return LGS_FREQUENCY_ERROR; /* illegal tuner frequency */
	
	*pcb1  = 0xCA;
	*pcb3a = 0x8C;		/* Search Mode */
	*pcb3b = 0x84;		/* Normal Mode */
	
	/*Npro = (freq + 36.1667)*24/4 */
	tunerDivi = (frequency*6) + 217;

	/* now convert the tuner divider into the two tuner divider registers */
	*ppd1 = (tunerDivi >> 8) & 0xFF;  /* High byte	*/
	*ppd2 = tunerDivi & 0xFF;         /* Low  byte	*/
	
	return LGS_NO_ERROR;
}



INT8 LGS_StartTuner(UINT16 frequency)
{
	INT8 err = LGS_NO_ERROR;
	UINT8 tunerRegister[5];
  	UINT8 pd1, pd2, cb1, cb2, cb3a, cb3b, locked;
		
	err = LGS_ComputeTunerRegister(frequency, &pd1, &pd2, &cb1, &cb2, &cb3a, &cb3b);
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_ComputeTunerRegister is failed!\n"));
		goto failed;
	}	
	
     		  
    	err = LGS_OpenTunerI2C(0x61);
    	if(err != LGS_NO_ERROR)
    	{
    		LGS_Debug((FILE_INFO, "LGS_OpenTunerI2C is failed!\n"));
    		goto failed;
    	}

        i2c_delay_us(100);

        /*err = LGS_SoftReset();
	if(err != LGS_NO_ERROR)
	{
		LGS_Debug((FILE_INFO, "LGS_SoftReset is failed!\n"));
		goto failed;
	}*/
	
	tunerRegister[0] = pd1;
	tunerRegister[1] = pd2;
	tunerRegister[2] = cb1;
	tunerRegister[3] = cb2;
	tunerRegister[4] = cb3a;
	err = tuner_i2c_write(TUNER_I2C_ADDR,tunerRegister[0],&tunerRegister[0],5);
    	//err = LGS_WriteI2C(g_sharpHandleI2c, tunerRegister, 5, 100);
    	if (err != 0)
        {
            	LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
        }

	tunerRegister[0] = pd1;
	tunerRegister[1] = pd2;
	tunerRegister[2] = cb1;
	tunerRegister[3] = cb2;
	tunerRegister[4] = cb3b;
	err = tuner_i2c_write(TUNER_I2C_ADDR,tunerRegister[0],&tunerRegister[0],5);
	//err = LGS_WriteI2C(g_sharpHandleI2c, tunerRegister, 5, 100);
    	if (err != LGS_NO_ERROR)
        {
            	LGS_Debug((FILE_INFO, "\t LGS_WriteI2C is failed!\n"));
		goto failed;
        }
    
    	i2c_delay_us(100);		/* wait 100 ms */
        
        err = LGS_CloseTunerI2C();
    	if(err != LGS_NO_ERROR)
    	{
    		LGS_Debug((FILE_INFO, "LGS_CloseTunerI2C is failed!\n"));
    		goto failed;
    	}
	
failed:	
  	return err;
}

#endif

