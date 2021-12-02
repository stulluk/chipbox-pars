#ifndef __CSAPI_SPI_H__
#define __CSAPI_SPI_H__

#ifdef __cplusplug
extern "C" {
#endif

#include "global.h"

#define CSSPI_OBJ_TYPE   'S'

#define SPI_MAGIC 'S'

#define SPI_SET_TMOD   		_IOW(SPI_MAGIC, 0, unsigned long)
#define SPI_BUS_STAT   		_IOW(SPI_MAGIC, 1, unsigned long)
#define SPI_SLAVE_STAT 		_IOW(SPI_MAGIC, 2, unsigned long)
#define SPI_SET_BAUD   		_IOW(SPI_MAGIC, 3, unsigned long)
#define SPI_GET_BAUD   		_IOW(SPI_MAGIC, 4, unsigned long)
#define SPI_SET_TIMING		_IOW(SPI_MAGIC, 5, unsigned long)
#define SPI_SET_POL  		_IOW(SPI_MAGIC, 6, unsigned long)
#define SPI_SET_PH  		_IOW(SPI_MAGIC, 7, unsigned long)

#define SPI_SET_MODE0  		_IOW(SPI_MAGIC, 8, unsigned long)
#define SPI_SET_MODE3  		_IOW(SPI_MAGIC, 9, unsigned long)

#define SPI_FLASH_R  		_IOW(SPI_MAGIC, 10, unsigned long)
#define SPI_FLASH_W  		_IOW(SPI_MAGIC, 11, unsigned long)
#define SPI_FLASH_E  		_IOW(SPI_MAGIC, 12, unsigned long)

#define SPI_READ_BYTE		_IOW(SPI_MAGIC, 13, unsigned long)
#define SPI_WRITE_BYTE		_IOW(SPI_MAGIC, 14, unsigned long)

#ifdef _DEBUG_
#define debug_printf  printf
#else
#define debug_printf(fmt,args...)
#endif

typedef void *CSSPI_HANDLE;

typedef enum {
	SPI_SUCCESS = 0,
	SPI_OPEN_ERROR,
	SPI_CLOSE_ERROR,
	SPI_READ_ERROR,
	SPI_WRITE_ERROR,
	SPI_BAD_PARAMATER } CSSPI_ErrCode;

typedef enum CSSPI_SCBAUD_ {
	CSSPI_500K = 0,
	CSSPI_1MH,
	CSSPI_2MH,
	CSSPI_3MH,
	CSSPI_4MH,
	CSSPI_5MH } CSSPI_SCBAUD;

typedef enum CSSPI_TMOD_ {
	CSSPI_TXRX = 0,
	CSSPI_TXO = 1,
	CSSPI_RXO = 2,
	CSSPI_EEPROM = 3 } CSSPI_TMOD;   

typedef enum CSSPI_SCPOL_ {
	CSSPI_LOWPOL = 0,
	CSSPI_HIGHPOL = 1} CSSPI_SCPOL;

typedef enum CSSPI_SCPH_ {
	CSSPI_MIDDLE = 0,
	CSSPI_START = 1 } CSSPI_SCPH;

typedef struct tagSPI_OBJ {
	unsigned char obj_type;
	int dev_fd;
	int errcode;
	CSSPI_SCBAUD speed;
} CSSPI_OBJ;


//CSSPI_HANDLE CSSPI_Open(CSSPI_SCBAUD slavespd);
CSSPI_HANDLE CSSPI_Open(void);
CSAPI_RESULT CSSPI_Close(CSSPI_HANDLE handle);
CSAPI_RESULT CSSPI_Read(CSSPI_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num);
CSAPI_RESULT CSSPI_Write(CSSPI_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num);
CSSPI_ErrCode CSSPI_GetErrCode(CSSPI_HANDLE handle);
char *CSSPI_GetErrString(CSSPI_HANDLE handle);
void CSSPI_PrintErrorStr(CSSPI_ErrCode errcode);
CSAPI_RESULT CSSPI_SetTMode(CSSPI_HANDLE handle, CSSPI_TMOD tmod);
CSAPI_RESULT CSSPI_SetClockPolarity(CSSPI_HANDLE handle, CSSPI_SCPOL pol);
CSAPI_RESULT CSSPI_SetClockPhase(CSSPI_HANDLE handle, CSSPI_SCPH ph);

#ifdef __cplusplug
}
#endif

#endif 		/* __CSAPI_SPI_H__ */
