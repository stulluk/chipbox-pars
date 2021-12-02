/****************************************************************
*
* FILENAME
*	mvmiscapi.h
*
* PURPOSE 
*	Header for Miscellaneous Function APIs
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/
#ifndef MV_MISC_API_H
#define MV_MISC_API_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include "mvmisc.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#define NULL_I2C_PORT     0xFF

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/
 extern U8 MaxBitsPerGpio[MAX_NUMBER_OF_GPIO];

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern BOOL I2cOpen(U32 *deviceId, I2cOpenParam_t *openParam);
extern BOOL I2cClose(U32 deviceId);
extern BOOL I2cAccessLock(U8 i2cNumber);
extern BOOL I2cAccessRelease(U8 i2cNumber);
extern U32  I2cRead(U32 deviceId, U32 addr, U8 *readBuffer, U32 len);
extern U32  I2cWrite(U32 deviceId, U32 addr, U8 *writeBuffer, U32 len);
extern BOOL GpioPortOpen(U32 *deviceId, GpioOpenParam_t *openParam);
extern BOOL GpioPortClose(U32 deviceId);
extern BOOL GpioSetPortetDirection(U32 deviceId, U8 direction);
extern BOOL GpioSetPortetMode(U32 deviceId, U8 mode);
extern BOOL GpioPortRead(U32 deviceId, U8 *readData);
extern BOOL GpioPortWrite(U32 deviceId, U8 writeData);
extern BOOL FrontIfOpen(U32 *deviceId);
extern BOOL FrontIfClose(U32 deviceId);
extern BOOL FrontIfWrite(U32 deviceId, U8 *data, U32 len);

#endif // #ifndef MV_MISC_API_H

