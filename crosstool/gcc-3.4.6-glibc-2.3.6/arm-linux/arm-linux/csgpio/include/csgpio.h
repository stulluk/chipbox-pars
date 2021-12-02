#ifndef __CSAPI_GPIO_H__
#define __CSAPI_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

typedef void *CSGPIO_HANDLE;

typedef enum {
	GPIO_SUCCESS = 0,
	GPIO_OPEN_ERROR,
	GPIO_CLOSE_ERROR,
	GPIO_READ_ERROR,
	GPIO_WRITE_ERROR,
	GPIO_BAD_PARAMATER,
	GPIO_SETDIR_ERROR,
	GPIO_GETDIR_ERROR,
} CSGPIO_ErrCode;

typedef enum {
	GPIO_DIRECTION_READ = 0,
	GPIO_DIRECTION_WRITE
} CSGPIO_DIRECTION;

CSGPIO_HANDLE CSGPIO_Open(unsigned int gpio);
CSGPIO_HANDLE CSGPIO2_Open(unsigned int gpio);
CSAPI_RESULT CSGPIO_Close(CSGPIO_HANDLE handle);
CSAPI_RESULT CSGPIO_Read(CSGPIO_HANDLE handle, unsigned char *bit);
CSAPI_RESULT CSGPIO_Write(CSGPIO_HANDLE handle, unsigned char bit);	/* only 0 and 1 are valid */
CSAPI_RESULT CSGPIO_SetDirection(CSGPIO_HANDLE handle, CSGPIO_DIRECTION gpio_direction);
CSAPI_RESULT CSGPIO_GetDirection(CSGPIO_HANDLE handle, CSGPIO_DIRECTION * gpio_direction);
CSGPIO_ErrCode CSGPIO_GetErrCode(CSGPIO_HANDLE handle);
char *CSGPIO_GetErrString(CSGPIO_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif				/* __CSAPI_GPIO_H */
