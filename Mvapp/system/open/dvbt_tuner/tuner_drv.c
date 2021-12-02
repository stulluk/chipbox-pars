#include "linuxos.h"

#include "tuner_reg.h"
#include "tuner_drv.h"

#if  TDM1316AL_1
tTUNER_PLL_CBTable 	PLL_Table[] = {PHILIPS_TD13111_TABLE};
tTUNER_PLL_Info 		PLL_IFInfo = {PHILIPS_TD13111_INFO};
#else
tTUNER_PLL_CBTable 	PLL_Table[] = {PHILIPS_TD13113_TABLE};
tTUNER_PLL_Info 		PLL_IFInfo = {PHILIPS_TD13113_INFO};
#endif

void zl10353_Delay(U16 i)
{
	  while(i>0)
	  	i--;  
}


BOOL zl10353_SetCoef(void)
{
	U8 reg_buffer;

	 reg_buffer = 0x3C;
	 Reg_SetOneDemodReg(ZL10353_BW_CTL,reg_buffer);

	 reg_buffer = 0xB4;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x04;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0xCD;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x19;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x87;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x22;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0xBD;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x3D;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x19;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x4A;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x09;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x53;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0xC1;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x6B;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x5C;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x7A;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x8B;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x83;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x37;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0x9B;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x6E;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0xAA;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0xDF;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0xB3;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x34;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0xC0;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);

	 reg_buffer = 0x51;
	 Reg_SetOneDemodReg(ZL10353_UK_REG2,reg_buffer);
	 reg_buffer = 0xD0;
	 Reg_SetOneDemodReg(ZL10353_UK_REG3,reg_buffer);
	 
	 reg_buffer = 0x30;
	 Reg_SetOneDemodReg(ZL10353_BW_CTL,reg_buffer);

	 return TRUE;

}

BOOL ZL10353_Set_CaptRange(U8 pCaptRange)
{

     U8 reg_buffer;
     reg_buffer = 0x10 |pCaptRange;
	 Reg_SetOneDemodReg(ZL10353_CAPT_RANGE,reg_buffer);

     return TRUE;

}

BOOL ZL10353_SetBandWidth(U8 pBW)
{
	U8 reg_buffer;
	U8 szBuffer[4];

	if(pBW == 6)
	 	zl10353_SetCoef();
   
	if(pBW == 6)
		reg_buffer = 0x30;
	else if(pBW == 7)
		reg_buffer = 0x35;
	else 
		reg_buffer = 0x36;
	
	Reg_SetOneDemodReg(ZL10353_BW_CTL,reg_buffer);

	if(pBW == 6)
	{
		 szBuffer[0] = 0x4D;  
		 szBuffer[1] = 0xEC;
	}
	else if(pBW == 7)
	{
		 szBuffer[0] = 0x5A;
		 szBuffer[1] = 0xE9; 
	}
	else
	{
		 szBuffer[0] = 0x67;
		 szBuffer[1] = 0xE5; 
	}
	Reg_SetOneDemodReg(ZL10353_TRL_NOMNIAL_RATE_1, szBuffer[0] );
	Reg_SetOneDemodReg(ZL10353_TRL_NOMNIAL_RATE_0, szBuffer[1] );


	if(pBW == 6)
		reg_buffer = 0x9C;
	else if(pBW == 7)
		reg_buffer = 0x86;
	else
		reg_buffer = 0x75;
	Reg_SetOneDemodReg(ZL10353_MCLK_CTL,reg_buffer);

	if(pBW == 6)
		reg_buffer = 0xDD;
	else 
		reg_buffer = 0x73;
	Reg_SetOneDemodReg(ZL10353_UK_REG4,reg_buffer);

	return TRUE;

}

BOOL zl10353_SetFsmGo(void)
{
 	U8  reg_buffer;   
    	int  i;
	
	Reg_SetOneDemodReg(ZL10353_TUNER_GO,ZL10353_TUNER_GOAQUIRE);

    reg_buffer = 0x01;
    i = 0;
    while((reg_buffer&0x01) && i < 20)
    {
	  Reg_GetOneDemodReg(ZL10353_TUNER_GO, &reg_buffer); 
	  usleep(100);
	  i++;
    }

    printf("zl10353_SetFsmGo loop = %d, time[%d]\n", i, CS_OS_time_now());

    Reg_SetOneDemodReg(ZL10353_FSM_GO,ZL10353_TUNER_GOAQUIRE);
    return TRUE;
}


BOOL zl10353_SetTunerTop(void)
{

#if DOUBLE_AGC
	return 1;
#else

	BOOL  ret;
	U8  reg_buffer;   
	U8  szBuffer[7];   

	szBuffer[0] = 0x02;
	szBuffer[1] = 0x41;

#if  TDM1316AL_1   
	szBuffer[2] = 0x80;//0x85; 
	szBuffer[3] = 0x69;
#else 
	szBuffer[2] = 0x9c;
	szBuffer[3] = 0x50;  /* 0x40; */
#endif
	szBuffer[4] = 0x00;

	Reg_SetOneDemodReg(ZL10353_CHAN_START_1, szBuffer[0]);
	Reg_SetOneDemodReg(ZL10353_CHAN_START_0, szBuffer[1]);
	Reg_SetOneDemodReg(ZL10353_CHAN_STOP_1, szBuffer[0]);
	Reg_SetOneDemodReg(ZL10353_CHAN_STOP_0,szBuffer[1]);
	Reg_SetOneDemodReg(ZL10353_TUNER_CONT_1, szBuffer[2]);
	Reg_SetOneDemodReg(ZL10353_TUNER_CONT_0, szBuffer[3]);

	ret = zl10353_SetFsmGo();
	Reg_SetOneDemodReg(ZL10353_TUNER_ADDR, 0xc0);
	zl10353_SetFsmGo();

	return ret;

#endif    
}



BOOL ZL10353_Init(void)
{

	U8 reg_buffer;
	int 	err = CS_NO_ERROR;
	tReg_OpenParams_t open_params;

	open_params.Dev_Address = ZL10353_I2C_ADDRESS;
	open_params.Reg_Count = ZL10353_NO_OF_REGS;
	                  
	Reg_Init(&open_params);

	Reg_AddDemodReg(ZL10353_OFDM_STAT,0x06,0x00 );

	Reg_AddDemodReg(ZL10353_FEC_STAT,0x07,0x00 );

	Reg_AddDemodReg(ZL10353_TUNER_STAT,0x08,0x00 );

	Reg_AddDemodReg(ZL10353_TPS_INFO,0x09,0x00 );	

	Reg_AddDemodReg(ZL10353_RF_LEVEL,0x0E,0x00 );
	        
	Reg_AddDemodReg(ZL10353_SNR,0x10,0x00 );
	        
	Reg_AddDemodReg(ZL10353_TPS_RECEIVED_1,0x1D,0x00 );
	        
	Reg_AddDemodReg(ZL10353_TPS_RECEIVED_0,0x1E,0x00 );
	        
	Reg_AddDemodReg(ZL10353_TPS_CURRENT_1,0x1F,0x00 );
	        
	Reg_AddDemodReg(ZL10353_TPS_CURRENT_0,0x20,0x00 );
	        
	Reg_AddDemodReg(ZL10353_AGC_GAIN_1,0xA,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_GAIN_0,0xB,0x00 );

	/*READ/WRITE*/
        Reg_AddDemodReg(ZL10353_ADC_CTL ,0xea,0x01);
    
	Reg_AddDemodReg(ZL10353_CONFIG,0x50,0x0C );

	Reg_AddDemodReg(ZL10353_CLOCK_CTL_0,0x51,0x44 );

	Reg_AddDemodReg(ZL10353_CLOCK_CTL_1,0x52,0x46 );

	Reg_AddDemodReg(ZL10353_PLL_0,0x53,0x15 );

	Reg_AddDemodReg(ZL10353_PLL_1,0x54,0x0F );

	Reg_AddDemodReg(ZL10353_RESET,0x55,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_TARGET,0x56,0x31 );

	Reg_AddDemodReg(ZL10353_AGC_CTL,0x89,0x43 );

	Reg_AddDemodReg(ZL10353_UK_REG1,0x9C,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_CTRL_5,0x8E,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_IF_LOLIM,0x90,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_RF_HILIM,0x91,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_IF_MAX,0x92,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_IF_MIN,0x93,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_RF_MAX,0x94,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_RF_MIN,0x58,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_KIF,0x95,0x00 );

	Reg_AddDemodReg(ZL10353_AGC_KRF,0x96,0x00 );

	Reg_AddDemodReg(ZL10353_AFC_CTL,0x7C,0x29 );

	Reg_AddDemodReg(ZL10353_ACQ_CTL,0x5E,0x43 );

	Reg_AddDemodReg(ZL10353_ACQ_DLY,0xE9,0x00 );

	Reg_AddDemodReg(ZL10353_BW_CTL,0x64,0x36 );

	Reg_AddDemodReg(ZL10353_TRL_NOMNIAL_RATE_1,0x65,0x67 );

	Reg_AddDemodReg(ZL10353_TRL_NOMNIAL_RATE_0,0x66,0xE5 );

	Reg_AddDemodReg(ZL10353_MCLK_CTL,0x5C,0x75 );

	Reg_AddDemodReg(ZL10353_INPUT_FREQ_1,0x6C,0xCD );

	Reg_AddDemodReg(ZL10353_INPUT_FREQ_0,0x6D,0x7E );

	Reg_AddDemodReg(ZL10353_TUNER_ADDR,0x67,Tuner_addr );

	Reg_AddDemodReg(ZL10353_CHAN_START_1,0x68,0x00 );

	Reg_AddDemodReg(ZL10353_CHAN_START_0,0x69,0x00 );

	Reg_AddDemodReg(ZL10353_TUNER_CONT_1,0x6A,0x80 );

	Reg_AddDemodReg(ZL10353_TUNER_CONT_0,0x6B,0x00 );

	Reg_AddDemodReg(ZL10353_CHAN_STOP_1,0xE2,0x00 );

	Reg_AddDemodReg(ZL10353_CHAN_STOP_0,0xE3,0x00 );

	Reg_AddDemodReg(ZL10353_CAPT_RANGE,0x5F,0x11 );

	Reg_AddDemodReg(ZL10353_TPS_GIVEN_1,0x6E,0x40 );

	Reg_AddDemodReg(ZL10353_TPS_GIVEN_0,0x6F,0x80 );

	Reg_AddDemodReg(ZL10353_TUNER_GO,0x70,0x00 );

	Reg_AddDemodReg(ZL10353_FSM_GO,0x71,0x00 );

	Reg_AddDemodReg(ZL10353_OP_CTRL_0,0x5A,0x48 );

	Reg_AddDemodReg(ZL10353_OP_CTRL_1,0x5B,0x00 );

	Reg_AddDemodReg(ZL10353_SCANCTL,0x62,0x0A);

	Reg_AddDemodReg(ZL10353_UK_REG2,0x9A,0x00);

	Reg_AddDemodReg(ZL10353_UK_REG3,0x9B,0x00);

	Reg_AddDemodReg(ZL10353_UK_REG4,0xCC,0x00 );

	Reg_ApplyDefault();

	reg_buffer = 0x0B;
#if MPEG_OFF 
	reg_buffer |= 0x04;
#endif
	err = Reg_SetOneDemodReg(ZL10353_CONFIG, reg_buffer);
	//printf("--------Reg_SetOneDemodReg = 0x%x\n", err);

	reg_buffer = 0x04;
#if MPEG_ClkInv 
	reg_buffer |= 0x40;
#endif   
#if MPEG_SERIAL 
	reg_buffer |= 0x20;
#endif
	Reg_SetOneDemodReg(ZL10353_CLOCK_CTL_0,reg_buffer);
	reg_buffer = 0x46;
	Reg_SetOneDemodReg(ZL10353_CLOCK_CTL_1,reg_buffer);

        usleep(200);

	reg_buffer = 0x80;
	Reg_SetOneDemodReg(ZL10353_RESET,reg_buffer);

	Reg_SetOneDemodReg(ZL10353_ADC_CTL,0x01);
	Reg_SetOneDemodReg(ZL10353_ADC_CTL,0x00);

	reg_buffer = 0x20; 
	Reg_SetOneDemodReg(ZL10353_AGC_TARGET,reg_buffer);

	reg_buffer = 0xA0 ; 
	Reg_SetOneDemodReg(ZL10353_UK_REG1,reg_buffer);

	reg_buffer = 0x03;
	Reg_SetOneDemodReg(ZL10353_AGC_CTL,reg_buffer);

        Reg_GetOneDemodReg(ZL10353_AGC_CTL, &reg_buffer);
        printf("ZL10353_AGC_CTL = 0x%x\n", reg_buffer);

	reg_buffer = 0x28;
	Reg_SetOneDemodReg(ZL10353_AFC_CTL,reg_buffer);


	reg_buffer = 0x40; 
	Reg_SetOneDemodReg(ZL10353_ACQ_CTL,reg_buffer);

	if(!ZL10353_SetBandWidth(8))  return FALSE;

	Reg_SetOneDemodReg(ZL10353_INPUT_FREQ_1, 0xCD);
	Reg_SetOneDemodReg(ZL10353_INPUT_FREQ_0, 0x7E);

	ZL10353_Set_CaptRange( 2);

	reg_buffer = 0x48;
#if DOUBLE_AGC
	reg_buffer = reg_buffer & 0xbf;
#endif

#if MPEG_clkinEn
	reg_buffer = reg_buffer|0x10;
#endif

#if MEPG_NoTEI		
	reg_buffer &= (~0x08);
#endif

#if MPEG_BKERRinvert
	reg_buffer |= 0x04;
#endif

#if MPEG_MDOswap
	reg_buffer = reg_buffer|0x02;
#endif

#if MPEG_MPEGgap12 
	    reg_buffer |= 0x01;
#endif
	Reg_SetOneDemodReg(ZL10353_OP_CTRL_0,reg_buffer);

	reg_buffer = 0x00;
#if MPEG_serialLSB1st
	reg_buffer = reg_buffer|0x10;
#endif
	Reg_SetOneDemodReg(ZL10353_OP_CTRL_1,reg_buffer);

	reg_buffer = PLL_IFInfo.Address;
	Reg_SetOneDemodReg(ZL10353_TUNER_ADDR,reg_buffer);

#if DOUBLE_AGC
	reg_buffer = 0x00;
	Reg_SetOneDemodReg(ZL10353_AGC_CTRL_5,reg_buffer);
	reg_buffer = 0x00;
	Reg_SetOneDemodReg(ZL10353_AGC_IF_LOLIM,reg_buffer);
	reg_buffer = 0xFF;
	Reg_SetOneDemodReg(ZL10353_AGC_RF_HILIM,reg_buffer);
	reg_buffer = 0xFF;
	Reg_SetOneDemodReg(ZL10353_AGC_IF_MAX,reg_buffer);
	reg_buffer = 0x00;
	Reg_SetOneDemodReg(ZL10353_AGC_IF_MIN,reg_buffer);
	reg_buffer = 0xFF;
	Reg_SetOneDemodReg(ZL10353_AGC_RF_MAX,reg_buffer);
	reg_buffer = 0x00;
	Reg_SetOneDemodReg(ZL10353_AGC_RF_MIN,reg_buffer);
	reg_buffer = 0x3F;
	Reg_SetOneDemodReg(ZL10353_AGC_KIF,reg_buffer);
	reg_buffer = 0x3F;
	Reg_SetOneDemodReg(ZL10353_AGC_KRF,reg_buffer);
#endif


	if (!zl10353_SetTunerTop()) return FALSE;

	usleep(200);
	return TRUE;
   
}

BOOL ZL10353_ProgramPLL(U32 pFrequency, U8 pBW)
{
	U8 Index;
	U32 TunerFreq;

	Reg_SetOneDemodReg(ZL10353_TUNER_ADDR, Tuner_addr);

	TunerFreq= pFrequency/1000;
	
	for (Index = PLL_IFInfo.TableCount; Index>1; Index--)
	{
		if (TunerFreq >= PLL_Table[Index-1].FreqMHz) 
			break;
	}

	TunerFreq = PLL_Table[Index-1].ControlByte; 

	switch (pBW)
	{
		case 6:TunerFreq |= PLL_IFInfo.Control6MHz; break;
		case 7:TunerFreq |= PLL_IFInfo.Control7MHz; break;
		case 8:TunerFreq |= PLL_IFInfo.Control8MHz; break;
		default: break;
	}

	Reg_SetOneDemodReg(ZL10353_TUNER_CONT_1, (U8)(TunerFreq>>8));
	Reg_SetOneDemodReg(ZL10353_TUNER_CONT_0, (U8)TunerFreq);

	TunerFreq = pFrequency;

	if ((PLL_IFInfo.Settings) & TUNER_LO_INJECTION)
		TunerFreq -= PLL_IFInfo.IF1kHz;
	else
		TunerFreq += PLL_IFInfo.IF1kHz;

	if (PLL_Table[Index-1].StepkHzx100==0) 
		return FALSE;
	
	TunerFreq = (TunerFreq * 1000) /PLL_Table[Index-1].StepkHzx100;
	TunerFreq +=5;
	TunerFreq /=10;

	Reg_SetOneDemodReg(ZL10353_CHAN_START_1,(U8)(TunerFreq>>8));
	Reg_SetOneDemodReg(ZL10353_CHAN_START_0,(U8)TunerFreq);

	Reg_SetOneDemodReg(ZL10353_CHAN_STOP_1,(U8)(TunerFreq>>8));
	Reg_SetOneDemodReg(ZL10353_CHAN_STOP_0,(U8)TunerFreq);

	zl10353_SetFsmGo();
	Reg_SetOneDemodReg(ZL10353_TUNER_ADDR, 0xc0);
	zl10353_SetFsmGo();

	return TRUE;

}


BOOL ZL10353_IsLocked(void)
{
	U8 	reg_buffer =0;
	int err;

	err = Reg_GetOneDemodReg( ZL10353_FEC_STAT, &reg_buffer);
	//printf("Reg_GetOneDemodReg = 0x%x\n", err);
	//printf("ZL10353_FEC_STAT = 0x%x\n", reg_buffer);
	return ((reg_buffer & ZL10353_FECSTAT_LOCKED)==ZL10353_FECSTAT_LOCKED);
	//return ((reg_buffer&0x10)==0x10);
}

U16 ZL10353_ReadAGC(void)
{
           U16 pAGC;
	  U8 reg_buffer[2];

	  Reg_GetOneDemodReg(ZL10353_AGC_GAIN_1, &(reg_buffer[1]));
	  Reg_GetOneDemodReg(ZL10353_AGC_GAIN_0, &(reg_buffer[0]));
           
           pAGC=( (U16)(reg_buffer[1]<<8)|reg_buffer[0] )&0x3FFF;
           return(pAGC);
}

U8 ZL10353_GetSNR(void)
{
    	U32 	uSNR;
	U8 	reg_buffer =0;
	 
	 if(ZL10353_IsLocked())
	 {
	 	 Reg_GetOneDemodReg( ZL10353_SNR, &reg_buffer);
	 }

	 //printf("ZL10353_SNR = 0x%x\n", reg_buffer);

    	uSNR = (reg_buffer*100/256);
    	if(uSNR > 100)
    		uSNR = 100;
	
    return ((U8)uSNR);
}

U16 ZL10353_GetReceivedTPS(void)
{
	U16 	pTPS;
	U8 	reg_buffer[2];

	Reg_GetOneDemodReg(ZL10353_TPS_RECEIVED_1, &(reg_buffer[1]));
	Reg_GetOneDemodReg(ZL10353_TPS_RECEIVED_0, &(reg_buffer[0]));

	pTPS = ((reg_buffer[1]<<8) |reg_buffer[0]);

	//printf("pTPS = 0x%x\n", pTPS);

	return(pTPS);
}

void ZL10353_PartReset(void)
{
	Reg_SetOneDemodReg(ZL10353_RESET,0x40);
}

void ZL10353_SetTSPriority(U8 pValue)
{
	Reg_SetOneDemodReg(ZL10353_TPS_GIVEN_1, pValue);
}

