
#include "i2c_if.h"
#include "csapi.h"

#define TUNER_I2C_ADDR		0xc0
#define STV0288_I2C_ADDR	0xd0

CSI2C_HANDLE tuner_i2c_handle;
CSI2C_HANDLE stv0288_i2c_handle;

int i2c_init(void)
{
	CSI2C_Attr tuner_handle_attr, stv0288_handle_attr;
	
	tuner_i2c_handle = CSI2C_Open(TUNER_I2C_ADDR >> 1);
	if (tuner_i2c_handle == NULL) 
		return -1;

	stv0288_i2c_handle = CSI2C_Open(STV0288_I2C_ADDR >> 1);
	if (stv0288_i2c_handle == NULL) 
		return -1;
		
	tuner_handle_attr.speed = 2;
	tuner_handle_attr.loop = 0;
	tuner_handle_attr.subaddr_num = 0;
	tuner_handle_attr.write_delayus = 0;
	CSI2C_SetAttr(tuner_i2c_handle, &tuner_handle_attr);

	stv0288_handle_attr.speed = 2;
	stv0288_handle_attr.loop = 0;
	stv0288_handle_attr.subaddr_num = 1;
	stv0288_handle_attr.write_delayus = 0;
	CSI2C_SetAttr(stv0288_i2c_handle, &stv0288_handle_attr);

	return 0;
}

int tuner_i2c_read(unsigned int address, int subaddr, unsigned char *buffer, unsigned int num)
{
	switch(address) {
		case TUNER_I2C_ADDR:
			CSI2C_Read(tuner_i2c_handle, subaddr, buffer, num);
			break;

		case STV0288_I2C_ADDR:
			CSI2C_Read(stv0288_i2c_handle, subaddr, buffer, num);
			break;
	}

	usleep(400);

	return 0;
}

int tuner_i2c_write(unsigned int address, unsigned int subaddr, unsigned char *buffer, unsigned int num)
{
	switch(address) {
		case TUNER_I2C_ADDR:
			CSI2C_Write(tuner_i2c_handle, subaddr, buffer, num);
			break;

		case STV0288_I2C_ADDR:
			subaddr = (unsigned int) buffer[0];
			num -= 1;;
			CSI2C_Write(stv0288_i2c_handle, subaddr, &buffer[1], num);
			break;
	}

	usleep(300);

	return 0;
}

int gpio_write(unsigned int pin, unsigned int val)
{
	CSGPIO_HANDLE pgpio = NULL;
	CSGPIO_DIRECTION dir = GPIO_DIRECTION_WRITE;

	if (pin > 15) return -1;

	if (NULL == (pgpio = CSGPIO_Open(pin))) return -1;

	CSGPIO_SetDirection(pgpio, dir);
	CSGPIO_Write(pgpio, val);
	
	CSGPIO_Close(pgpio);

	return 0;
}
