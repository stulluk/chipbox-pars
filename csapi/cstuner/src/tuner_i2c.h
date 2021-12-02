/** -----------------------------------------------------------------------
 *  \file    tuner_i2c.h
 *  \brief  Tuner I2C control head file
 *  \author  XuJiwei
 *  \date    2006-10-17 01:26ÏÂÎç
 *  \note    Following items should be observed:
 *           -#  notice point A  
 --------------------------------------------------------------------------*/
 
#ifndef		_TUNER_I2C_H_
#define		_TUNER_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif


//	when read and write the tuner I2C chip, there is no use the index parameters
#define	I2C_NO_INDEX		0xFFFF

//I2C register
#define TUNER_I2C_BASE			0x10170000
#define TUNER_I2C_CON			(TUNER_I2C_BASE + 0x00)
#define TUNER_I2C_TAR			(TUNER_I2C_BASE + 0x04)
#define TUNER_I2C_DATA_CMD		(TUNER_I2C_BASE + 0x10)
#define TUNER_I2C_SS_HCNT		(TUNER_I2C_BASE + 0x14)
#define TUNER_I2C_SS_LCNT		(TUNER_I2C_BASE + 0x18)
#define TUNER_I2C_RX_TL			(TUNER_I2C_BASE + 0x38)
#define TUNER_I2C_TX_TL			(TUNER_I2C_BASE + 0x3c)
#define TUNER_I2C_ENABLE			(TUNER_I2C_BASE + 0x6c)
#define TUNER_I2C_STATUS		(TUNER_I2C_BASE + 0x70)

typedef struct {
	unsigned int con;
	unsigned int ss_hcnt;
	unsigned int ss_lcnt;
	unsigned int tar;
	unsigned int rx_tl;
	unsigned int tx_tl;
} TUNER_I2C_CONFIG, *PTUNER_I2C_CONFIG;

#define I2C_SET _IOW('I', 0, sizeof(unsigned long))

int	c_tuner_i2c_init(void);
int c_tuner_i2c_read(unsigned int address, int subaddr, unsigned int *buffer, unsigned int num);
int c_tuner_i2c_write(unsigned int address, int subaddr, unsigned int *buffer, unsigned int num);
void tuner_delay_us(unsigned int	count);
int tuner_write_mask(int addr, int subaddr, int mask, int data);

 #ifdef __cplusplus
}
#endif

#endif
