#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "i2c_if.h"
#include "stv0288_regs.h"

Tuner_Register_t RegMap[NBREG];

#define CN_LOOKUP_TAB_SIZE	30
#define MAX_TUNER_DATA_BYTES	(7)
#define BYTES2WORD(a, b) 	((a << 8) + (b))
#define ABS0194(x) 		((((int)x)>=0) ? ((int)x): (-((int)x)))
#define IN_RANGE(x, y, z) 	(((x <= y && y <= z) || (z <= y && y <= x)) ? 1 : 0)

#define bus_open() 		do { stv0288_write(R288_I2CRPT, 0x95); } while(0)
#define bus_close() 		do { stv0288_write(R288_I2CRPT, 0x15); } while(0)

struct {
	int realval;
	int regval;
} CN_LOOKUP_TAB[CN_LOOKUP_TAB_SIZE] = 
{
	{ 25, 8680 },  
	{ 30, 8420 }, 
	{ 35, 8217 }, 
	{ 40, 7897 }, 
	{ 50, 7333 }, 
	{ 60, 6747 }, 
	{ 70, 6162 }, 
	{ 80, 5580 }, 
	{ 90, 5029 }, 
	{ 100, 4529 },
	{ 110, 4080 },
	{ 120, 3685 },
	{ 130, 3316 },
	{ 140, 2982 },
	{ 150, 2688 },
	{ 160, 2418 },
	{ 170, 2188 },
	{ 180, 1982 },
	{ 190, 1802 },
	{ 200, 1663 },
	{ 210, 1520 },
	{ 220, 1400 },
	{ 230, 1295 },
	{ 240, 1201 },
	{ 250, 1123 },
	{ 260, 1058 },
	{ 270, 1004 },
	{ 280, 957 }, 
	{ 290, 920 }, 
	{ 300, 890 } 
};

extern sem_t sem_pll;

void set_FR_OFF(long frequency_offset)
{
	unsigned char pbDerot[2];
	unsigned int dwDerotFrequency;
	signed int nDerotFrequency;

	if (frequency_offset < 0) {
		nDerotFrequency = (-1) * frequency_offset;
		dwDerotFrequency = (unsigned int) nDerotFrequency *0x400 / 1375;
		dwDerotFrequency = 0xFFFF - dwDerotFrequency;
	}
	else {
		dwDerotFrequency = (unsigned int) frequency_offset *0x400 / 1375;
	}

	pbDerot[0] = (unsigned char) (dwDerotFrequency >> 8);
	pbDerot[1] = (unsigned char) (dwDerotFrequency & 0xFF);

	stv0288_write(R288_CFRM, pbDerot[0]);
	stv0288_write(R288_CFRL, pbDerot[1]);
}

void calculate_pll_divider_byte(long freq, unsigned char *byte)
{
	long data, data1;

	data = (long) ((double) (freq) / 500);
	data1 = (data / 32);

	*(byte + 0) = (unsigned char) ((data >> 8) & 0x7f);
	*(byte + 1) = (unsigned char) (data & 0xff);
}

long demod0288_get_mclk_freq(void)
{
	long mclk_Hz;		/* master clock frequency (Hz) */
	long pll_divider;	/* pll divider */
	long pll_selratio;	/* 4 or 6 ratio */
	long pll_bypass;	/* pll bypass */
	long ExtClk_Hz;

	ExtClk_Hz = 4000000;

	pll_divider = stv0288_read(R288_PLLCTRL) + 1;
	pll_selratio = (((stv0288_read(R288_SYNTCTRL) >> 2) & 0x01) ? 4 : 6);
	pll_bypass = stv0288_read(R288_SYNTCTRL) & 0x01;
	if (pll_bypass)
		mclk_Hz = ExtClk_Hz;
	else
		mclk_Hz = (ExtClk_Hz * pll_divider) / pll_selratio;

	return mclk_Hz;
}

long binary_float_div(long n1, long n2, int precision)
{
	int i = 0;
	long result = 0;

	while (i <= precision) { /* n1>0 */
		if (n1 < n2) {
			result <<= 1;
			n1 <<= 1;
		}
		else {
			result = (result << 1) + 1;
			n1 = (n1 - n2) << 1;
		}

		i++;
	}

	return result;
}

void write_symbol_rate_registers(unsigned long baud)
{
	long MasterClock_Hz;
	unsigned long dwCalcData;
	unsigned char pbValue[4];

	pbValue[0] = 0x80;
	pbValue[1] = 0;
	pbValue[2] = 0;

	stv0288_write(R288_SFRH, pbValue[0]);
	stv0288_write(R288_SFRM, pbValue[1]);
	stv0288_write(R288_SFRL, pbValue[2]);

	MasterClock_Hz = demod0288_get_mclk_freq();
	dwCalcData = binary_float_div(baud, MasterClock_Hz, 20);

	pbValue[0] = (unsigned char) ((dwCalcData >> 12) & 0xFF);
	pbValue[1] = (unsigned char) ((dwCalcData >> 4) & 0xff);
	pbValue[2] = (unsigned char) ((dwCalcData) & 0xff);

	stv0288_write(R288_SFRH, pbValue[0]);
	stv0288_write(R288_SFRM, pbValue[1]);
	stv0288_write(R288_SFRL, pbValue[2]);
	stv0288_write(R288_RTFM, 0);
	stv0288_write(R288_RTFL, 0);
} 

unsigned char check_signal_strength_and_quality(unsigned int * strength, unsigned int * quality)
{
	unsigned short pbSNR[2];
	int regval, Imin, Imax, i;

	if (check_lock_status() == TUNER_LOCKED) 
	{
		Imin = 0;
		Imax = CN_LOOKUP_TAB_SIZE - 1;
		pbSNR[0] = stv0288_read(R288_NIRM);
		pbSNR[1] = stv0288_read(R288_NIRL);
		regval = pbSNR[0] * 255 + pbSNR[1];

		if (IN_RANGE(CN_LOOKUP_TAB[Imin].regval, regval, CN_LOOKUP_TAB[Imax].regval)) 
		{
			while ((Imax - Imin) > 1) 
			{
				i = (Imax + Imin) / 2;

				if (IN_RANGE(CN_LOOKUP_TAB[Imin].regval, regval, CN_LOOKUP_TAB[i].regval))
					Imax = i;
				else
					Imin = i;
			}

			*quality = ((regval - CN_LOOKUP_TAB[Imin].regval)
				    * (CN_LOOKUP_TAB[Imax].realval - CN_LOOKUP_TAB[Imin].realval)
				    / (CN_LOOKUP_TAB[Imax].regval - CN_LOOKUP_TAB[Imin].regval))
				    + CN_LOOKUP_TAB[Imin].realval;
		}
		else
			*quality = 99;

		if (*quality >= 100)
			*quality = 99;

		*strength = *quality - sqrtf((*quality) * 2);
	}
	else {
		*strength = 5;
		*quality = 0;
	}

	return check_lock_status();
}

void demod0288_get_timing_loop(void)
{
	int FREQ_MSB;
	int FREQ_LSB;
	long timing;

	FREQ_MSB = stv0288_read(R288_RTFM);
	FREQ_LSB = stv0288_read(R288_RTFL);

	timing = BYTES2WORD(FREQ_MSB, FREQ_LSB);
}
 
int check_lock_status(void)
{
	int ret;
	ret = stv0288_read(R288_VSTATUS);

	if ((stv0288_read(R288_VSTATUS) & 0x98) == 0x98) 
	{
		demod0288_get_timing_loop();
		return TUNER_LOCKED;
	}
	else if ((stv0288_read(R288_VSTATUS) & 0x80) == 0x80) 
	{
		return TUNER_CARRIER_OK;
	}

	return TUNER_NOT_LOCKED;
}

void calculate_pll_lpf_cutoff(long baud, unsigned char *byte_)
{
	long local_baud = baud;

	*(byte_ + 2) &= 0xE7;
	*(byte_ + 3) &= 0xF3;
	local_baud = (baud * 6) / 4;

	if (local_baud >= 30000000) /* LPF cut-off Frequency = 30MHz */
	{
		*(byte_ + 2) |= (0x02 << 3);
		*(byte_ + 3) |= (0x03 << 2);
	}
	if ((local_baud < 30000000) && (local_baud >= 28000000)) // LPF cut-off Frequency = 28MHz 
	{
		*(byte_ + 3) |= (0x03 << 2);
	}
	if ((local_baud < 28000000) && (local_baud >= 26000000)) // LPF cut-off Frequency = 26MHz
	{
		*(byte_ + 2) |= (0x03 << 3);
		*(byte_ + 3) |= (0x02 << 2);
	}
	if ((local_baud < 26000000) && (local_baud >= 24000000)) // LPF cut-off Frequency = 24MHz
	{
		*(byte_ + 2) |= (0x01 << 3);
		*(byte_ + 3) |= (0x01 << 2);
	}
	if ((local_baud < 24000000) && (local_baud >= 22000000)) // LPF cut-off Frequency = 22MHz
	{
		*(byte_ + 2) |= (0x02 << 3);
		*(byte_ + 3) |= (0x01 << 2);
	}
	if ((local_baud < 22000000) && (local_baud >= 20000000)) // LPF cut-off Frequency = 20MHz
	{
		*(byte_ + 2) |= (0x00 << 3);
		*(byte_ + 3) |= (0x01 << 2);
	}
	if ((local_baud < 20000000) && (local_baud >= 18000000)) // LPF cut-off Frequency = 18MHz
	{
		*(byte_ + 2) |= (0x03 << 3);
		*(byte_ + 3) |= (0x02 << 2);
	}
	if ((local_baud < 18000000) && (local_baud >= 16000000)) // LPF cut-off Frequency = 16MHz
	{
		*(byte_ + 2) |= (0x01 << 3);
		*(byte_ + 3) |= (0x02 << 2);
	}
	if ((local_baud < 16000000) && (local_baud >= 14000000)) // LPF cut-off Frequency = 14MHz
	{
		*(byte_ + 2) |= (0x02 << 3);
		*(byte_ + 3) |= (0x02 << 2);
	}
	if ((local_baud < 14000000) && (local_baud >= 12000000)) // LPF cut-off Frequency = 12MHz
	{
		*(byte_ + 2) |= (0x00 << 3);
		*(byte_ + 3) |= (0x02 << 2);
	}
	if ((local_baud < 12000000) && (local_baud >= 10000000)) // LPF cut-off Frequency = 10MHz
	{
		*(byte_ + 2) |= (0x03 << 3);
		*(byte_ + 3) |= (0x00 << 2);
	}
}

void calculate_pll_vco(long freq, unsigned char *byte_)
{
	unsigned char bDIVValue = 1;
	unsigned char bBAValue = 5;

#if 1
	if (950000 <= freq && freq < 970000.) {
		bDIVValue = 1;
		bBAValue = 6;
	}

	else
#endif
	if (970000 <= freq && freq < 1065000.) {
		bDIVValue = 1;
		bBAValue = 6;
	}

	else if (1065000 <= freq && freq < 1170000.) {
		bDIVValue = 1;
		bBAValue = 7;
	}

	else if (1170000 <= freq && freq < 1300000.) {
		bDIVValue = 0;
		bBAValue = 1;
	}

	else if (1300000 <= freq && freq < 1445000.) {
		bDIVValue = 0;
		bBAValue = 2;
	}

	else if (1445000 <= freq && freq < 1607000.) {
		bDIVValue = 0;
		bBAValue = 3;
	}

	else if (1607000 <= freq && freq < 1778000.) {
		bDIVValue = 0;
		bBAValue = 4;
	}

	else if (1778000 <= freq && freq < 1942000.) {
		bDIVValue = 0;
		bBAValue = 5;
	}

	else if (1942000 <= freq && freq < 2131000.) {
		bDIVValue = 0;
		bBAValue = 6;
	}

#if 1
	else if (2131000 <= freq) {
		bDIVValue = 0;
		bBAValue = 6;
	}

#endif
	*(byte_ + 3) &= 0x1D;
	*(byte_ + 3) |= (bBAValue << 5) | (bDIVValue << 1);
}

int tuner_write_0288(int size, unsigned char *buffer)
{
	int nRet = 0;
	unsigned char i2c_buffer[I2C_COMMAND_LEN];

	sem_wait(&sem_pll);

	memcpy(i2c_buffer, buffer, size);
	nRet = tuner_i2c_write(0xc0, 0, i2c_buffer, 4);

	sem_post(&sem_pll);

	return (nRet);
}

void pll_setdata(unsigned char *byte_)
{
	unsigned char byte4, byte5;

	byte4 = *(byte_ + 2);
	byte5 = *(byte_ + 3);
	*(byte_ + 2) &= 0xE3;
	*(byte_ + 3) &= 0xF3;

	bus_open();

	tuner_write_0288(4, byte_);
	*(byte_ + 2) |= 0x04;

	tuner_write_0288(4, byte_);
	*(byte_ + 2) = byte4;
	*(byte_ + 3) = byte5;

	tuner_write_0288(4, byte_);

	bus_close();
}

void init_pll_ic(unsigned char *byte_)
{
	*(byte_ + 2) = 0xE5;
	*(byte_ + 3) = 0x00;
}

void tun_setfreq(long freq, long baud, unsigned char *byte)
{
	init_pll_ic(byte);

	calculate_pll_divider_byte(freq, byte);
	calculate_pll_lpf_cutoff(baud, byte);
	calculate_pll_vco(freq, byte);

	pll_setdata(byte);
} 

void STV0299SetIQ(unsigned char bIQStatus)
{
	unsigned char bValue = 0;

	if (bIQStatus)
		bValue |= 0x08;
	else
		bValue &= 0xf7;

	stv0288_write(R288_AGC1IN, bValue);

	return;
}

unsigned char search_false_lock(long freq, long baud)
{
	int jj = 0;
	long df = 0;
	long step;

	unsigned char flag;
	unsigned int nDerotMax = 0;
	unsigned int nDerotMin = 0;
	unsigned int nFirstDerotFrequency = 0;
	unsigned char bUpperFlag = 0;

	freq *= 1000;
	baud *= 1000;

	if (baud > 10000000)
		step = baud / 4000;
	else
		step = baud / 8000;

	if (step > 1000)
		step = 1000;

	if (step < 125)
		step = 125;

	nDerotMin = -3000;
	nDerotMax = 3000;

	jj = 0;
	flag = check_lock_status();

	do {
		if (flag == TUNER_LOCKED) {
			return flag;
		}
		else if (flag == TUNER_CARRIER_OK) {
			STV0299SetIQ(1);
			TUNER_DELAY(baud > 10000000 ? 20 : 30);

			flag = check_lock_status();
			if (flag == TUNER_LOCKED)
				return flag;
			else
				STV0299SetIQ(0);
		}

		df = jj * step / 2;
		if (jj % 2)
			df *= (-1);
		df += nFirstDerotFrequency;
		
		if (ABS0194(df) > ABS0194(nDerotMax))
			bUpperFlag = 1;

		set_FR_OFF(df);
		TUNER_DELAY(baud > 10000000 ? 40 : 50);
		
		flag = check_lock_status();
		jj++;
	} while (!bUpperFlag);

	flag = check_lock_status();

	return flag;
}

short VZ0295SetFrequency(long _TunerFrequency, long _SymbolRate)
{
	long band;
	long temp_frequency;
	unsigned char data[MAX_TUNER_DATA_BYTES];

	band = _SymbolRate * 1000;
	temp_frequency = _TunerFrequency * 1000;

	tun_setfreq(temp_frequency, band, &data[0]);
	usleep(10000); /* Modify By River 09.29.2007 usleep(1000000) */

	/* Setup of non-modified parameters */
	stv0288_set_field(F288_ALPHA, 7);
	stv0288_set_field(F288_BETA, 28);
	stv0288_set_field(F288_IND1_ACC, 0);
	stv0288_set_field(F288_IND2_ACC, 0xff);

	write_symbol_rate_registers(1000000);
	write_symbol_rate_registers(band);

	if (band < 5000000) 
	{
		stv0288_set_field(F288_ALPHA, 8);
		stv0288_set_field(F288_BETA, 17);
	}
	else if (band > 35000000) 
	{
		stv0288_set_field(F288_ALPHA, 8);
		stv0288_set_field(F288_BETA, 36);
	}

	return 0;
}

