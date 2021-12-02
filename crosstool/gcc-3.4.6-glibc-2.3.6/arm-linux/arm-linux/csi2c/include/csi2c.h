#ifndef __CSAPI_I2C_H__
#define __CSAPI_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

#define I2C_SPEED_100K 1
#define I2C_SPEED_400K 2

#define I2C_ADDR_NOLOOP 0
#define I2C_ADDR_LOOP 1

typedef void *CSI2C_HANDLE;

typedef enum {
	I2C_SUCCESS = 0,
	I2C_OPEN_ERROR,
	I2C_CLOSE_ERROR,
	I2C_READ_ERROR,
	I2C_WRITE_ERROR,
	I2C_BAD_PARAMATER,
	I2C_SETATTR_ERROR,
	I2C_GETATTR_ERROR,
	I2C_ATTR_LOOPPARAMATER_ERROR,
	I2C_ATTR_SPEEDPARAMATER_ERROR,
	I2C_ATTR_SUBADDRPARAMATER_ERROR
} CSI2C_ErrCode;

typedef struct {
	int speed;		/* I2C_SPEED_100K 1; I2C_SPEED_400K 2 */
	unsigned char loop;	/* I2C_ADDR_NOLOOP 0, I2C_ADDR_LOOP 1 */
	unsigned int subaddr_num;
	unsigned int write_delayus;
} CSI2C_Attr;

CSI2C_HANDLE CSI2C_Open(unsigned int chipaddress);
CSAPI_RESULT CSI2C_Close(CSI2C_HANDLE handle);
CSAPI_RESULT CSI2C_Read(CSI2C_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num);
CSAPI_RESULT CSI2C_Write(CSI2C_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num);
CSAPI_RESULT CSI2C_GetAttr(CSI2C_HANDLE handle, CSI2C_Attr * i2c_attr);
CSAPI_RESULT CSI2C_SetAttr(CSI2C_HANDLE handle, CSI2C_Attr * i2c_attr);
CSI2C_ErrCode CSI2C_GetErrCode(CSI2C_HANDLE handle);
char *CSI2C_GetErrString(CSI2C_HANDLE handle);
void CSI2C_PrintErrorStr(CSI2C_ErrCode errcode);

#ifdef __cplusplus
}
#endif

#endif				/* __CSAPI_I2C_H */
