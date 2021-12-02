#ifndef __DEF_H__
#define __DEF_H__

#include <wait.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <mqueue.h>
#include <time.h>
#include <linux/ioctl.h>
    
typedef struct 
{
	unsigned char Addr;	/* Address */
	unsigned char Reset;	/* Default value */
	unsigned char Value;	/* Current value */
} Tuner_Register_t;

typedef struct 
{
	int Reg; /* Register index  */
	unsigned char Pos; /* Bit position */
	unsigned char Bits; /* Bit width */
	unsigned char Type; /* Signed or unsigned */
	unsigned char Mask; /* Mask compute with width and position */
} FIELD;

#define I2C_COMMAND_LEN 	6
#define TUNER_DELAY(__ms__) 	usleep(1000*(__ms__))

#define TUNER_NOT_LOCKED 	0
#define TUNER_LOCKED 		1
#define TUNER_CARRIER_OK 	2

#define UNSIGNED 		0
#define SIGNED			1

/* 
 * i2c I/O interface.
 */
//int  tuner_i2c_init(void);	/* 02.25.2008 ehnus modify! */
int  i2c_init(void);
int  tuner_i2c_read(unsigned int address, int subaddr, unsigned char *buffer, unsigned int num);
int  tuner_i2c_write(unsigned int address, unsigned int subaddr, unsigned char *buffer, unsigned int num);

/* 
 * gpio I/O interface.
 */
int gpio_read(unsigned pin);
int gpio_write(unsigned int pin, unsigned int val);

/* 
 * tuner 0288 interface.
 */
void set_FR_OFF(long frequency_offset);
void calculate_pll_divider_byte(long freq, unsigned char *byte);
long demod0288_get_mclk_freq(void);
long binary_float_div(long n1, long n2, int precision);
void write_symbol_rate_registers(unsigned long baud);
unsigned char check_signal_strength_and_quality(unsigned int * strength, unsigned int * quality);
void demod0288_get_timing_loop(void);
int check_lock_status(void);
void calculate_pll_lpf_cutoff(long baud, unsigned char *byte_);
void calculate_pll_vco(long freq, unsigned char *byte_);
int tuner_write_0288(int size, unsigned char *buffer);
void pll_setdata(unsigned char *byte_);
void init_pll_ic(unsigned char *byte_);
void tun_setfreq(long freq, long baud, unsigned char *byte);
void STV0299SetIQ(unsigned char bIQStatus);
unsigned char search_false_lock(long freq, long baud);
short VZ0295SetFrequency(long _TunerFrequency, long _SymbolRate);
#endif

