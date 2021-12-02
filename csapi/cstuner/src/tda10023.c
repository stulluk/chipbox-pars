
#include <stdio.h>
//#include <linux/delay.h>
#include <time.h>
#include <string.h>

#include "cs_tuner.h"

#include "tuner_i2c.h"
#include "tuner_error.h"

#include "tda10023.h"

#define CU1216_IF 36125000
#define TUNER_MUL 62500

#define XTAL   28920000UL
#define PLL_M  8UL
#define PLL_P  4UL
#define PLL_N  1UL
#define SYSCLK (XTAL * PLL_M / (PLL_N * PLL_P))  // -> 57840000


static unsigned char reg0_state = 0;

static unsigned char tda10023_inittab[]=
{
	// reg mask val
	0x2a,0xff,0x02,  // PLL3, Bypass, Power Down
	0xff,0x64,0x00,  // Sleep 100ms
	0x2a,0xff,0x03,  // PLL3, Bypass, Power Down
	0xff,0x64,0x00,  // Sleep 100ms
	0x28,0xff, (PLL_M - 1),  // PLL1 M = 8
	0x29,0xff,(((PLL_P - 1)<<6) | (PLL_N - 1)),  // PLL2
	0x00,0xff,0x23,  // GPR FSAMPLING=1
	0x2a,0xff,0x08,  // PLL3 PSACLK=1
	0xff,0x64,0x00,  // Sleep 100ms
	0x1f,0xff,0x00,  // RESET
	0xff,0x64,0x00,  // Sleep 100ms
	0xe6,0x0c,0x04,  // RSCFG_IND
	0x10,0xc0,0x80,  // DECDVBCFG1 PBER=1

	0x0e,0xff,0x82,  // GAIN1
	0x03,0x08,0x08,  // CLKCONF DYN=1
	0x2e,0xbf,0x30,  // AGCCONF2 TRIAGC=0,POSAGC=ENAGCIF=1 PPWMTUN=0 PPWMIF=0
	0x01,0xff,0x30,  // AGCREF
	0x1e,0x84,0x84,  // CONTROL SACLK_ON=1
	0x1b,0xff,0xc8,  // ADC TWOS=1
	0x3b,0xff,0xff,  // IFMAX
	0x3c,0xff,0x00,  // IFMIN
	0x34,0xff,0x00,  // PWMREF
	0x35,0xff,0xff,  // TUNMAX
	0x36,0xff,0x00,  // TUNMIN
	0x06,0xff,0x7f,  // EQCONF1 POSI=7 ENADAPT=ENEQUAL=DFE=1    // 0x77
	0x1c,0x30,0x30,  // EQCONF2 STEPALGO=SGNALGO=1
	0x37,0xff,0xf6,  // DELTAF_LSB
	0x38,0xff,0xff,  // DELTAF_MSB
	0x02,0xff,0x93,  // AGCCONF1  IFS=1 KAGCIF=2 KAGCTUN=3
	0x2d,0xff,0xf6,  // SWEEP SWPOS=1 SWDYN=7 SWSTEP=1 SWLEN=2
	0x04,0x10,0x00,   // SWRAMP=1
	0x12,0xff,0xa1,  // INTP1 POCLKP=1 FEL=1 MFS=0
	0x2b,0x01,0xa1,  // INTS1
	0x20,0xff,0x04,  // INTP2 SWAPP=? MSBFIRSTP=? INTPSEL=?
	0x2c,0xff,0x0d,  // INTP/S TRIP=0 TRIS=0
	0xc4,0xff,0x00,
	0xc3,0x30,0x00,
	0xb5,0xff,0x19,  // ERAGC_THD
	0x00,0x03,0x01,  // GPR, CLBS soft reset
	0x00,0x03,0x03,  // GPR, CLBS soft reset
	0xff,0x64,0x00,  // Sleep 100ms
	0xff,0xff,0xff
};

static void tda10023_writetab(unsigned char *tab)
{
	unsigned char subaddr = 0;
	unsigned char mask = 0;
	unsigned char val = 0;
	
	
	while (1) 
	{
		subaddr = (*tab++);
		mask = (*tab++);
		val = (*tab++);
		
		if (subaddr == 0xff) 
		{
			if (mask == 0xff)
				break;
			else
				tuner_delay_us(1000 * mask);
		}
		
		else
			tuner_write_mask(P10023_DEMOD_I2C_ADDR, subaddr, mask, val);
	}
	
	return;
}
int p10023_demodu_init(void)
{
	int val = 0;
	
	printf("DVB: p10023_demodu_init.\n");
	val = 0x33;
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x00, &val, 1);
	
	tda10023_writetab(tda10023_inittab);
	
	reg0_state = 0x23;


	return 0;
}

static void tda10023_mul64(unsigned int u_up1, unsigned int u_up2, unsigned int* u_height_init, unsigned int* u_low_init)
{
    unsigned int  u_low_initTmp, u_height_initTmp, u_up1_16, u_up1_0, u_up2_16, u_up2_0;
    unsigned char   bCounter;

    // init value
    u_up1_0  = u_up1 & 0x0000FFFF;
    u_up1_16 = u_up1 >> 16;
    u_up2_0  = u_up2 & 0x0000FFFF;
    u_up2_16 = u_up2 >> 16;

    // calculate X1Y1
    *u_height_init = u_up1_16*u_up2_16;

    // calculate X0Y1+X1Y0
    u_low_initTmp = u_up1_16*u_up2_0+u_up1_0*u_up2_16;
    u_height_initTmp = 0;
    for (bCounter = 0; bCounter < 16; bCounter++)
    {
        if ((u_low_initTmp & 0x80000000) || (u_height_initTmp != 0))
        {
            u_height_initTmp <<= 1; 
            u_height_initTmp |= (u_low_initTmp>>31);
        }
        u_low_initTmp <<= 1;
    }

    // test overflow for low byte
    *u_low_init  = (u_up1_0*u_up2_0)>>1;
    *u_low_init += u_low_initTmp>>1;
    if (*u_low_init & 0x80000000)
        u_height_initTmp++;

    // calculate X0Y0 Low Byte
    *u_low_init = u_up1_0*u_up2_0+u_low_initTmp; 

    // calculate High Byte
    *u_height_init += u_height_initTmp;
}


static unsigned int tda10023_div64(unsigned int u_height_init, unsigned int u_low_init, unsigned int u_down)
{
    unsigned int volatile result;     // no optimisation
    unsigned int volatile remain1;    // no optimisation
    unsigned int volatile remain2;    // no optimisation
    unsigned int u_height_init1, u_low_init1;
    unsigned int u_height_init2, u_low_init2, remain3;

    // divide u_height_init by u_down
    result = 0xFFFFFFFF / u_down;
    result *= u_height_init;

    // calculate the remainder
    tda10023_mul64((0xFFFFFFFF%u_down), u_height_init, &u_height_init1, &u_low_init1);

    // uRemain = Div64(u_height_initTmp, u_low_initTmp, u_down);
    // divide the remainder
    remain1 =  0xFFFFFFFF/u_down;
    remain1 *= u_height_init1;

    // calculate the remainder of the remainder
    tda10023_mul64((0xFFFFFFFF%u_down), u_height_init1, &u_height_init2, &u_low_init2);

    // uRemain = Div64(u_height_initTmp, u_low_initTmp, u_down);
    // divide the remainder
    remain2 =  0xFFFFFFFF/u_down;
    remain2 *= u_height_init2;

    // calculate the remainder of the remainder
    remain3 = 0xFFFFFFFF%u_down;
    remain3 *= u_height_init2;

    //update the remainder
    remain2 += (u_low_init2+u_height_init2+remain3+(u_down/2))/u_down;

    //update the remainder
    remain1 += (u_low_init1+u_height_init1+(u_down/2))/u_down + remain2;

    // divide u_low_init by u_down and add remain 
    result += (u_low_init+u_height_init+(u_down/2))/u_down + remain1;

    return result;
}

static unsigned int tda10023_mul_div32(unsigned int u_up1, unsigned int u_up2, unsigned int u_down)
{
    unsigned int u_height_init, u_low_init;

    // multiplied 32bits*32Bits->64bits
    tda10023_mul64(u_up1, u_up2, &u_height_init, &u_low_init);

    // divide 64bits/32bits->32bits
    return tda10023_div64(u_height_init, u_low_init, u_down);
}



static int tda10023_set_symbolrate (unsigned int SymRate)
{
	int sym_rate = SymRate * 1000;
	int buf_val = 0;
	int BDR = 0;
	int BDRI = 0;
	short SFIL=0;
	unsigned short NDEC = 0;

	if (sym_rate > (SYSCLK / (2*4)))
		sym_rate = SYSCLK /(2*4);

	if (sym_rate < 870000)
		sym_rate = 870000;

	if (sym_rate < (unsigned int)(SYSCLK / (984 / 10))) 
	{
		NDEC = 3;
		SFIL = 1;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (640 / 10))) 
	{
		NDEC = 3;
		SFIL = 0;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (492 / 10))) 
	{
		NDEC = 2;
		SFIL = 1;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (320 / 10))) 
	{
		NDEC = 2;
		SFIL = 0;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (246 / 10))) 
	{
		NDEC = 1;
		SFIL = 1;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (160 / 10))) 
	{
		NDEC = 1;
		SFIL = 0;
	} 
	
	else if (sym_rate < (unsigned int)(SYSCLK / (123 / 10))) 
	{
		NDEC = 0;
		SFIL = 1;
	}

	BDRI = SYSCLK * 16;
	BDRI >>= NDEC;
	BDRI += (sym_rate / 2);
	BDRI /= sym_rate;

	if (BDRI > 255)
		BDRI = 255;


        BDR = tda10023_mul_div32(1 << (24 + NDEC), sym_rate, SYSCLK);


	//printf("Symbolrate %i, BDR %i BDRI %i, NDEC %i\n",sym_rate,BDR,BDRI,NDEC);
	
	tuner_write_mask(P10023_DEMOD_I2C_ADDR, 0x03, 0xc0, NDEC << 6);
	
	buf_val = BDR & 255;
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x0a, &buf_val, 1);
	
	buf_val = (BDR >> 8) & 255;
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x0b, &buf_val, 1);
	
	buf_val = (BDR >> 16) & 31;
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x0c, &buf_val, 1);
	
	buf_val = BDRI;
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x0d, &buf_val, 1);
	
	buf_val = (SFIL << 7);
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x3d, &buf_val, 1);
	
	return 0;
}


int p10023_demodu(unsigned int	frequency, unsigned int	symbol_rate, TUNER_QAM_MODULATION_E modulation, TUNER_SPECTRAL_INVERSION_E inversion)
{	
	TUNER_QAM_MODULATION_E qam = 0;
	//int tmp_reg0 = 0;
	int buf_val = 0;
	
	//printf("into p10023_demodu \n");
	static int qamvals[6][6] = 
	{
		//  QAM   LOCKTHR  MSETH   AREF AGCREFNYQ  ERAGCNYQ_THD
		{ (5<<2),  0x78,    0x8c,   0x96,   0x78,   0x4c  },  // 4 QAM     //0
		{ (0<<2),  0x87,    0xa2,   0x91,   0x8c,   0x57  },  // 16 QAM    //1
		{ (1<<2),  0x64,    0x74,   0x96,   0x8c,   0x57  },  // 32 QAM    //2
		{ (2<<2),  0x46,    0x43,   0x6a,   0x6a,   0x44  },  // 64 QAM    //3
		{ (3<<2),  0x36,    0x34,   0x7e,   0x78,   0x4c  },  // 128 QAM   //4
		{ (4<<2),  0x26,    0x23,   0x6c,   0x5c,   0x3c  },  // 256 QAM   //5
	};
	
	

	
	qam = modulation;
	if (qam < 0 || qam > 5)
		return -1;

	/*lock tuner pll*/
	p10023_tuner_set_pll(frequency);

	/*set symbol rate*/
	tda10023_set_symbolrate (symbol_rate);
	
	/*set qam mode*/
	buf_val = qamvals[qam][1];
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x05, &buf_val, 1);
	
	buf_val = qamvals[qam][2];
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x08, &buf_val, 1);
	
	buf_val = qamvals[qam][3];
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0x09, &buf_val, 1);
	
	buf_val = qamvals[qam][4];
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0xb4, &buf_val, 1);
	
	buf_val = qamvals[qam][5];
	c_tuner_i2c_write(P10023_DEMOD_I2C_ADDR, 0xb6, &buf_val, 1);
	
	//DVB mode
        //GPR: program the modulation in QAM bits
	tuner_write_mask(P10023_DEMOD_I2C_ADDR, 0x00, 0x1c, qamvals[qam][0]);
	
	/*Spectrum Auto*/
	tuner_write_mask(P10023_DEMOD_I2C_ADDR, 0x04, 0x40, 0x40);
	
	
	return 0;
}

static int tda10023_enable_c_tuner_i2c(int demod_addr)
{
	int result;
	//int val = 0;

	result = tuner_write_mask(demod_addr, 0x0f, 0x80, 0x80);

	tuner_delay_us(1000);
	
	return result;
}

static int tda10023_disable_c_tuner_i2c(int demod_addr)
{
	//int val = 0;
	int result = 0;
	
	result = tuner_write_mask(demod_addr, 0x0f, 0x80, 0);
	
	return result;
}

int p10023_tuner_set_pll(unsigned int channelfre)
{
	//int sudaddr = 0;
	int freq = channelfre * 1000; /*Hz*/
	unsigned int tuner_buf[6];

	unsigned int  div = (freq + CU1216_IF + TUNER_MUL / 2) / TUNER_MUL;

	memset(&tuner_buf, 0, sizeof(unsigned int) * 6);
	
	tuner_buf[0] = (div >> 8) & 0x7f;
	tuner_buf[1] = div & 0xff;
	tuner_buf[2] = 0xce;
	tuner_buf[3] = (freq < 150000000 ? 0x01 : freq < 445000000 ? 0x02 : 0x04);	
	tuner_buf[4] = 0xde;
	tuner_buf[5] = 0x20;


	tda10023_enable_c_tuner_i2c(P10023_DEMOD_I2C_ADDR);
	
	//for(i = 0; i < 6; i++)
	//	printf("tuner_buf[%d] = 0x%x \n", i, tuner_buf[i]);

	c_tuner_i2c_write(P10023_TUNER_I2C_ADDR, tuner_buf[0], (unsigned int *)&tuner_buf[1], 5);
		
	//c_tuner_i2c_write(P10023_TUNER_I2C_ADDR, tuner_buf[4], (unsigned int *)&tuner_buf[5], 1);
	//c_tuner_i2c_write(P10023_TUNER_I2C_ADDR, tuner_buf[0], (unsigned int *)&tuner_buf[1], 4);
	tuner_delay_us(500);
#if 0
{
	unsigned int read_buf = 0;
	int i = 0;
	/* wait for the pll lock */	
	for (i = 0; i < 20; i++) 
	{
		
		c_tuner_i2c_read(P10023_TUNER_I2C_ADDR, -1, (unsigned int *)&read_buf, 1);
		printf("p10023_tuner_set_pll is read_buf = 0x%x.\n", read_buf);
		if(read_buf & 0x40)
		{
			printf("p10023_tuner_set_pll is lock.\n");
			break;
		}
			
		tuner_delay_us(500);
	}

	/* switch the charge pump to the lower current */
	//tuner_buf[2] &= (~0x40);
	//c_tuner_i2c_write(P10023_TUNER_I2C_ADDR, tuner_buf[2], (unsigned int *)&tuner_buf[3], 1);
}	
#endif		

	tda10023_disable_c_tuner_i2c(P10023_DEMOD_I2C_ADDR);

	tuner_delay_us(500);


	return 0;
}


int p10023_tuner_lock_check(void)
{
	unsigned int read_buffer = 0;
	unsigned int j = 0;
	
	tuner_delay_us(1000);

	for (j = 0 ;j < 2;j++)
	{
		c_tuner_i2c_read(P10023_DEMOD_I2C_ADDR, 0x11, &read_buffer, 1);
		printf("read_buffer is %x!!!!!!!!!\n", read_buffer);
	}
	// decode

	if((read_buffer & 0x02) || (read_buffer & 0x04) || (read_buffer & 0x08))
	{
		return 1;
	}

	return 0;

}





	

   
