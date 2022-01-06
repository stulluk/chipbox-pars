/****************************************************************
*
* FILENAME
*	mvmisc.c
*
* PURPOSE 
*	Middle ware for Miscellaneous Functions
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <termios.h>
#include "mvosapi.h"
#include "mvmisc.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
//#define MVMISC_DEBUG_ON

#define I2C_SET	       _IOW('I', 0, unsigned long)
#define I2C_GET        _IOR('I', 1, unsigned long)
#define I2C_SETSPEED   _IOW('I', 2, unsigned long)
#define I2C_GETSPEED   _IOR('I', 3, unsigned long)
#define I2C_SETLOOPADDR   _IOW('I', 4, unsigned long)
#define I2C_CLRLOOPADDR   _IOW('I', 5, unsigned long)
#define I2C_GETLOOPINFO     _IOR('I', 6, unsigned long)
#define I2C_SETCHIPADDRESS  _IOW('I', 7, unsigned long)
#define I2C_GETCHIPADDRESS  _IOR('I', 8, unsigned long)
#define I2C_SETADDRALEN     _IOW('I', 9, unsigned long)

#define GPIO_NAME              "/dev/gpio/"
#define GPIO2_NAME             "/dev/gpio2/"

#define CMD_READ_GPIO	 _IOR('G', 1, unsigned long)
#define CMD_WRITE_GPIO   _IOW('G', 2, unsigned long)
#define CMD_SET_DIRECTION _IOW('G', 3, unsigned long)
#define CMD_READ_GPIO2	 _IOR('g', 6, unsigned long)
#define CMD_WRITE_GPIO2   _IOW('g', 7, unsigned long)
#define CMD_SET_DIRECTION_GPIO2 _IOW('g', 8, unsigned long)

#define GPIO_FILE_NAME_LENGTH   20

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
typedef struct
{
	int I2cFd;
	U8  I2cNumber;
	U8  SubAddrLen;
}I2cHandle_t;

typedef struct
{
	int GpioFd;
	U8  PortNumber;
	U8  Mode;       // 0 : read, 1 : Write,  2 : I/O
	U8  Direction;  // 0 : read, 1 : Write
	U8  Value;      // Initial Value
}GpioHandle_t;

typedef struct
{
	int          FrontFd;
	unsigned int BaudRate;
}FrontHandle_t;

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/
 U32 I2cDeviceAccessSem[MAX_NUMBER_OF_I2C_DEVICE];
 U8 MaxBitsPerGpio[MAX_NUMBER_OF_GPIO] = {16, 55};

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

/****************************************************************
 *                         Functions                            *
 ****************************************************************/
/*
 *  Name         : I2cOpen
 *  Description
 *     Open I2C device with given Parameter.
 *  INPUT Arg
 *     I2cOpenParam_t *openParam  : I2C Device open parameter 
 *  OUTPUT Arg
 *     U32 *deviceId              : Opened device ID
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL I2cOpen(U32 *deviceId, I2cOpenParam_t *openParam)
{
	I2cHandle_t *i2cHandle;
	int          i2cFd;

	if ((openParam == NULL) || (openParam->DevideNo >= MAX_NUMBER_OF_I2C_DEVICE))
	{
		/* Error in the open Parameter */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : Wrong Parameter\n");
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		return OS_RETURN_ERROR;
	}

	if (I2cDeviceAccessSem[openParam->DevideNo] == 0)
	{
		/* First call for I2C Device */
		OsCreateSemaphore(&I2cDeviceAccessSem[openParam->DevideNo], 1);
	}

	i2cFd = open("/dev/misc/orion_i2c", O_RDWR);
	if (i2cFd < 0)
	{
		/* Error to open I2C Device */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : open error (%d)\n", i2cFd);
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		return OS_RETURN_ERROR;
	}
	if(ioctl(i2cFd, I2C_SETCHIPADDRESS, (unsigned long)(openParam->Address) >> 1) < 0)
	{
		/* Error to Set Chip Address */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : Error to set Chip address\n");
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		close(i2cFd);
		return OS_RETURN_ERROR;
	}
	if(ioctl(i2cFd, I2C_SETADDRALEN, (unsigned long)(openParam->SubAddrLen)) < 0)
	{
		/* Error to Set Sub Address Length */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : Error to set Sub address Length\n");
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		close(i2cFd);
		return OS_RETURN_ERROR;
	}

	if(ioctl(i2cFd, I2C_SETSPEED, (unsigned long)(openParam->Speed)) < 0)
	{
		/* Error to Set Sub Address Length */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : Error to set Speed\n");
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		close(i2cFd);
		return OS_RETURN_ERROR;
	}
	
	i2cHandle = (I2cHandle_t *)OsMemoryAllocate(sizeof(I2cHandle_t));
	if (i2cHandle == NULL)
	{
		/* Error to Allocate memory for Handle */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cOpen Error : Error to Allocate Handle\n");
#endif // #ifdef MVMISC_DEBUG_ON
		*deviceId = 0;
		close(i2cFd);
		return OS_RETURN_ERROR;
	}

	i2cHandle->I2cFd      = i2cFd;
	i2cHandle->I2cNumber  = openParam->DevideNo;
	i2cHandle->SubAddrLen = openParam->SubAddrLen;

#ifdef MVMISC_DEBUG_ON
	// OsDebugPrtf("I2cOpen Info : I2C opened for Device[0x%02X] on [0x%08X]:[%d]\n", openParam->Address, (U32)i2cHandle, i2cFd);
#endif // #ifdef MVMISC_DEBUG_ON

	*deviceId = (U32)i2cHandle;

	return OS_NO_ERROR;
}

/*
 *  Name         : I2cClose
 *  Description
 *     Close I2C device.
 *  INPUT Arg
 *     U32 deviceId               : Device Id which want to Close
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL I2cClose(U32 deviceId)
{
	int          i2cFd;
	I2cHandle_t *i2cHandle;
	
	if (deviceId == 0)
	{
		/* Device ID is NULL : not defined device */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cClose Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	i2cHandle = (I2cHandle_t *)deviceId;

	i2cFd = i2cHandle->I2cFd;
	close(i2cFd);
	OsMemoryFree((void *)i2cHandle);

	return OS_NO_ERROR;
}

/*
 *  Name         : I2cAccessLock
 *  Description
 *     Lock the I2C Device Access.
 *  INPUT Arg
 *     U8 i2cNumber               : I2C Port Number which want to Lock
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL I2cAccessLock(U8 i2cNumber)
{
	U32          semId;
	
	if (i2cNumber >= MAX_NUMBER_OF_I2C_DEVICE)
	{
		/* I2C Number error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cAccessLock Error : I2C Number Error [%d]\n", i2cNumber);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	semId = I2cDeviceAccessSem[i2cNumber];

	if (semId == 0)
	{
		/* Semaphore not defined */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cAccessLock Error : Semaphore not defined\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	OsSemaphoreWait(semId, TIMEOUT_FOREVER);

	return OS_NO_ERROR;
}

/*
 *  Name         : I2cAccessRelease
 *  Description
 *     Release the I2C Device Access.
 *  INPUT Arg
 *     U8 i2cNumber               : Device Id which want to Release
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL I2cAccessRelease(U8 i2cNumber)
{
	U32          semId;
	
	if (i2cNumber >= MAX_NUMBER_OF_I2C_DEVICE)
	{
		/* I2C Number error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cAccessRelease Error : I2C Number Error [%d]\n", i2cNumber);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	semId = I2cDeviceAccessSem[i2cNumber];

	if (semId == 0)
	{
		/* Semaphore not defined */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cAccessRelease Error : Semaphore not defined\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	OsSemaphoreSignal(semId);

	return OS_NO_ERROR;
}

/*
 *  Name         : I2cRead
 *  Description
 *     Read Data from given sub address of I2C device.
 *  INPUT Arg
 *     U32 deviceId               : I2C Device Id to read
 *     U32 addr                   : Sub address in the device to read
 *     U32 len                    : Read Data length
 *  OUTPUT Arg
 *     U8 *readBuffer             : Read data buffer
 *  RETURN : U32 (Length of read data)
 */
U32 I2cRead(U32 deviceId, U32 addr, U8 *readBuffer, U32 len)
{
	int          i2cFd;
	int          ret;
	U8          *buffer;
	I2cHandle_t *i2cHandle;
	
	if (deviceId == 0)
	{
		/* Device ID is NULL : not defined device */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cClose Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	i2cHandle = (I2cHandle_t *)deviceId;

	i2cFd = i2cHandle->I2cFd;
	ret = lseek(i2cFd, (unsigned int)(addr), SEEK_SET);
	if( ret < 0)
	{
		/* Device ID is NULL : not defined device */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cRead Error : Set sub address error\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return 0;
	}

	/* First allocate temp buffer */
	buffer = (U8 *)OsMemoryAllocate((int)len);
	if (buffer == NULL)
	{
		/* Error to allocate temp buffer */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cRead Error : Memory allocation error for temp buffer\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return 0;
	}
	memset(buffer, 0, (int)len);
	ret = read(i2cFd, (char *)buffer, len);
	if ((ret < 0) || (ret != (int)len))
	{
		/* I2c device read error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cRead Error : read error(%d/%d)\n", ret, len);
#endif // #ifdef MVMISC_DEBUG_ON
		OsMemoryFree(buffer);
		return 0;
	}

	memcpy(readBuffer, buffer, len);
	OsMemoryFree(buffer);

	return len;
}

/*
 *  Name         : I2cWrite
 *  Description
 *     Write Data to given sub address of I2C device.
 *  INPUT Arg
 *     U32 deviceId               : I2C Device Id
 *     U32 addr                   : Sub address in the device
 *     U8 *writeBuffer            : Write data buffer
 *     U32 len                    : Write Data length
 *  OUTPUT Arg
 *  RETURN : U32 (Length of written data)
 */
U32 I2cWrite(U32 deviceId, U32 addr, U8 *writeBuffer, U32 len)
{
	int          i2cFd;
	int          ret;
	I2cHandle_t *i2cHandle;
	
	if (deviceId == 0)
	{
		/* Device ID is NULL : not defined device */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cClose Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	i2cHandle = (I2cHandle_t *)deviceId;

	i2cFd = i2cHandle->I2cFd;
	ret = lseek(i2cFd, (unsigned int)(addr), SEEK_SET);
	if(ret < 0)
	{
		/* Sub addres set error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cWrite Error : Set sub address error\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return 0;
	}

	ret = write(i2cFd, (char *)writeBuffer, len);
	if ((ret < 0) || (ret != (int)len))
	{
		/* I2c device read error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("I2cWrite Error : write error(%d/%d)\n", ret, len);
#endif // #ifdef MVMISC_DEBUG_ON
		return 0;
	}

	return len;
}

/*
 *  Name         : GpioPortOpen
 *  Description
 *     Open GPIO Port device with given Parameter.
 *  INPUT Arg
 *     GpioOpenParam_t *openParam : GPIO Open parameters
 *  OUTPUT Arg
 *     U32 *deviceId              : Opened device Id
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioPortOpen(U32 *deviceId, GpioOpenParam_t *openParam)
{
	GpioHandle_t *gpioHandle;
	char          writeChar;
	char          deviceName[GPIO_FILE_NAME_LENGTH];
	U32           command;

	*deviceId = 0;
	if ((openParam->PortNumber >= MAX_NUMBER_OF_GPIO) ||
		(openParam->BitNumber >= MaxBitsPerGpio[openParam->PortNumber]))
	{
		/* Wrong Parameter error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortOpen Error : Wrong Parameter error(%d/%d)\n", openParam->PortNumber, openParam->BitNumber);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}
	
	gpioHandle = (GpioHandle_t *)OsMemoryAllocate(sizeof(GpioHandle_t));
	if (gpioHandle == (GpioHandle_t *)NULL)
	{
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortOpen Error : Cannot Allocate GPIO Handle\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	memset(deviceName, 0x00, GPIO_FILE_NAME_LENGTH);
	/* Set GPOI node name */
	if (openParam->PortNumber == 1)
	{
		/* Set GPIO node to GPIO 2 */
		sprintf(deviceName, "/dev/gpio2/%d", openParam->BitNumber);
		command = CMD_SET_DIRECTION_GPIO2;
	}
	else
	{
		/* Set GPIO node to GPIO */
		sprintf(deviceName, "/dev/gpio/%d", openParam->BitNumber);
		command = CMD_SET_DIRECTION;
	}

#ifdef MVMISC_DEBUG_ON
	// OsDebugPrtf("GpioPortOpen Info: GPIO Device Name : %s\n", deviceName);
#endif // #ifdef MVMISC_DEBUG_ON
	/* Open GPIO Port by node */
	gpioHandle->GpioFd = open((const char *)deviceName, O_RDWR);
	if (gpioHandle->GpioFd < 0)
	{
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortOpen Error : Cannot open node point\n");
#endif // #ifdef MVMISC_DEBUG_ON
		OsMemoryFree((void *)gpioHandle);
		return OS_RETURN_ERROR;
	}
	gpioHandle->PortNumber = openParam->PortNumber;
	gpioHandle->Mode       = openParam->Mode;

	switch(openParam->Mode)
	{
		case GPIO_READ_MODE : // Read Mode
		case GPIO_IO_MODE   : // IO Mode (Start with Read Mode)
			if (ioctl(gpioHandle->GpioFd, command, GPIO_READ_MODE) < 0)
			{
#ifdef MVMISC_DEBUG_ON
				OsDebugPrtf("GpioPortOpen Error : Cannot set Port Direction to Read\n");
#endif // #ifdef MVMISC_DEBUG_ON
				OsMemoryFree((void *)gpioHandle);
				return OS_RETURN_ERROR;
			}
			gpioHandle->Direction = GPIO_READ_MODE;
			break;
		case GPIO_WRITE_MODE : // Write Mode
			if (ioctl(gpioHandle->GpioFd, command, GPIO_WRITE_MODE) < 0)
			{
#ifdef MVMISC_DEBUG_ON
				OsDebugPrtf("GpioPortOpen Error : Cannot set Port Direction to Write\n");
#endif // #ifdef MVMISC_DEBUG_ON
				OsMemoryFree((void *)gpioHandle);
				return OS_RETURN_ERROR;
			}
			/* Set initial value */
			gpioHandle->Direction = GPIO_WRITE_MODE;
			if(openParam->Value > 0)
			{
				writeChar = '1';
			}
			else
			{
				writeChar = '0';
			}
			
			if (write(gpioHandle->GpioFd, &writeChar, 1) < 0)
			{
#ifdef MVMISC_DEBUG_ON
				OsDebugPrtf("GpioPortOpen Error : Cannot set Port Direction to Write\n");
#endif // #ifdef MVMISC_DEBUG_ON
				OsMemoryFree((void *)gpioHandle);
				return OS_RETURN_ERROR;
			}

			gpioHandle->Value = openParam->Value;
			break;
		default :
#ifdef MVMISC_DEBUG_ON
			OsDebugPrtf("GpioPortOpen Error : Wrong Operation Mode(%d)\n", openParam->Mode);
#endif // #ifdef MVMISC_DEBUG_ON
			OsMemoryFree((void *)gpioHandle);
			return OS_RETURN_ERROR;
	}

	*deviceId = (U32)gpioHandle;
	return OS_NO_ERROR;
}

/*
 *  Name         : GpioPortClose
 *  Description
 *     Close GPIO Port.
 *  INPUT Arg
 *     U32 deviceId               : Device Id which want to Close
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioPortClose(U32 deviceId)
{
	GpioHandle_t *gpioHandle;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortClose Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	gpioHandle = (GpioHandle_t *)deviceId;
	close(gpioHandle->GpioFd);
	OsMemoryFree((void *) gpioHandle);
	
	return OS_NO_ERROR;
}

/*
 *  Name         : GpioSetPortetDirection
 *  Description
 *     Set GPIO Port Drection(Read or Write).
 *  INPUT Arg
 *     U32  deviceId              : GPIO Port Id
 *     U8   direction             : Direction (0 : Read, 1 : Write)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioSetPortetDirection(U32 deviceId, U8 direction)
{
	GpioHandle_t *gpioHandle;
	U32           command;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetDirection Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	if (direction > GPIO_WRITE_MODE)
	{
		/* Not supported Direction */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetDirection Error : Not Supported Direction(%d)\n", direction);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}
	
	gpioHandle = (GpioHandle_t *)deviceId;

	if (gpioHandle->Direction == direction)
	{
		/* Wrong Direction */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetDirection Worning : Requested Direction is same with current(%d)\n", direction);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_NO_ERROR;
	}

	if ((gpioHandle->Mode != GPIO_IO_MODE) && (gpioHandle->Mode != direction))
	{
		/* Wrong Mode */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetDirection Error : Cannot set direction(%d) under this mode(%d)\n",
		              direction, gpioHandle->Mode);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	if (gpioHandle->PortNumber == 1)
	{
		/* Set GPIO node to GPIO 2 */
		command = CMD_SET_DIRECTION_GPIO2;
	}
	else
	{
		/* Set GPIO node to GPIO */
		command = CMD_SET_DIRECTION;
	}

	if (ioctl(gpioHandle->GpioFd, command, direction) < 0)
	{
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetDirection Error : Cannot set Port Direction\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	gpioHandle->Direction = direction;

	return OS_NO_ERROR;
	
}

/*
 *  Name         : GpioSetPortetMode
 *  Description
 *     Set GPIO Port Mode(Read, Write or I/O Mode).
 *  INPUT Arg
 *     U32  deviceId              : GPIO Port Id
 *     U8   mode                  : Mode(0 : Read Mode, 1 : Write Mode, 2 : I/O Mode)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioSetPortetMode(U32 deviceId, U8 mode)
{
	GpioHandle_t *gpioHandle;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetMode Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	if (mode > GPIO_IO_MODE)
	{
		/* Wrong Direction */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetMode Error : Not Supported Mode(%d)\n", mode);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}
	
	gpioHandle = (GpioHandle_t *)deviceId;
	
	if (gpioHandle->Mode == mode)
	{
		/* Wrong Mode */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioSetPortetMode Worning : Requested mode is same with current mode(%d)\n", mode);
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_NO_ERROR;
	}

	/* Sync GPIO Mode and Direction */
	if ((mode != GPIO_IO_MODE) && (gpioHandle->Direction != GPIO_IO_MODE))
	{
		if (GpioSetPortetDirection(deviceId, mode) == OS_RETURN_ERROR)
		{
			/* Error to set Direction for Sync */
#ifdef MVMISC_DEBUG_ON
			OsDebugPrtf("GpioSetPortetMode Worning : Erro to set Direction\n");
#endif // #ifdef MVMISC_DEBUG_ON
			return OS_RETURN_ERROR;
		}
		gpioHandle->Direction = mode;
	}
	
	gpioHandle->Mode = mode;

	return OS_NO_ERROR;
}

/*
 *  Name         : GpioPortRead
 *  Description
 *     Read Data from Gpio Port.
 *  INPUT Arg
 *     U32 deviceId               : GPIO Port Id to read
 *  OUTPUT Arg
 *     U8 *readData               : Read data buffer
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioPortRead(U32 deviceId, U8 *readData)
{
	GpioHandle_t *gpioHandle;
	char          gpioVal;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortRead Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	gpioHandle = (GpioHandle_t *)deviceId;
	if (gpioHandle->Mode == GPIO_WRITE_MODE)
	{
		/* Wrong Mode */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortRead Error : Current Mode can not support GPIO Port read\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	if (gpioHandle->Direction != GPIO_READ_MODE)
	{
		if (GpioSetPortetDirection(deviceId, GPIO_READ_MODE) == OS_RETURN_ERROR)
		{
#ifdef MVMISC_DEBUG_ON
			OsDebugPrtf("GpioPortRead Error : Cannot set Port Direction to Read\n");
#endif // #ifdef MVMISC_DEBUG_ON
			return OS_RETURN_ERROR;
		}

		gpioHandle->Direction = GPIO_READ_MODE;
	}

	if(read(gpioHandle->GpioFd, (char *)&gpioVal, 1) < 0)
	{
		/* Read Error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortRead Error : Read Error\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	*readData = gpioVal - '0';

	gpioHandle->Value = *readData;
	
	return OS_NO_ERROR;
}

/*
 *  Name         : GpioPortWrite
 *  Description
 *     Write Data to Gpio Port.
 *  INPUT Arg
 *     U32  deviceId              : GPIO Port Id to Write
 *     U8   writeData             : Write Data
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL GpioPortWrite(U32 deviceId, U8 writeData)
{
	GpioHandle_t *gpioHandle;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortWrite Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	gpioHandle = (GpioHandle_t *)deviceId;
	if (gpioHandle->Mode == GPIO_READ_MODE)
	{
		/* Wrong Mode */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortWrite Error : Current Mode can not support GPIO Port Write\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	if (gpioHandle->Direction != GPIO_WRITE_MODE)
	{
		if (GpioSetPortetDirection(deviceId, GPIO_WRITE_MODE) == OS_RETURN_ERROR)
		{
#ifdef MVMISC_DEBUG_ON
			OsDebugPrtf("GpioPortWrite Error : Cannot set Port Direction to Write\n");
#endif // #ifdef MVMISC_DEBUG_ON
			return OS_RETURN_ERROR;
		}

		gpioHandle->Direction = GPIO_WRITE_MODE;
	}

	if (writeData >= 1)
	{
		writeData = '1';
	}
	else
	{
		writeData = '0';
	}

	if(write(gpioHandle->GpioFd, (char *)&writeData, 1) < 0)
	{
		/* Write Error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("GpioPortWrite Error : Write Error\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	gpioHandle->Value = writeData;
	
	return OS_NO_ERROR;
}

/*
 *  Name         : FrontIfOpen
 *  Description
 *     Open Front If.
 *  INPUT Arg
 *  OUTPUT Arg
 *     U32 *deviceId              : Opened device Id
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL FrontIfOpen(U32 *deviceId)
{
	FrontHandle_t *frontHandle;
	struct termios option;

	*deviceId = 0;

	frontHandle = (FrontHandle_t *)OsMemoryAllocate(sizeof(FrontHandle_t));
	if (frontHandle == (FrontHandle_t *)NULL)
	{
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("FrontIfOpen Error : Cannot Allocate Front I/F Handle\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}
	
	// frontHandle->FrontFd = open("/dev/ttyS1",O_RDWR|O_NOCTTY);
	frontHandle->FrontFd = open("/dev/ttyS1", O_WRONLY|O_NOCTTY);
	if(frontHandle->FrontFd == -1)
	{
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("FrontIfOpen Error : Can not open Serian1(ttyS1) for Front\n");
#endif // #ifdef MVMISC_DEBUG_ON
		OsMemoryFree((void *)frontHandle);
		return OS_RETURN_ERROR;
	}

	tcgetattr(frontHandle->FrontFd,&option);
	cfmakeraw(&option);
	cfsetispeed(&option,B19200);
	cfsetospeed(&option,B19200);

	tcsetattr(frontHandle->FrontFd,TCSANOW,&option);
	frontHandle->BaudRate = B19200;

	*deviceId = (U32)frontHandle;
#ifdef MVMISC_DEBUG_ON
	OsDebugPrtf("FrontIfOpen Info : Open Serian1(ttyS1) for Front\n");
#endif // #ifdef MVMISC_DEBUG_ON
	return OS_NO_ERROR;
}

/*
 *  Name         : FrontIfClose
 *  Description
 *     Close Front I/F.
 *  INPUT Arg
 *     U32 deviceId               : Device Id which want to Close
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL FrontIfClose(U32 deviceId)
{
	FrontHandle_t *frontHandle;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("FrontIfClose Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	frontHandle = (FrontHandle_t *)deviceId;
	close(frontHandle->FrontFd);
	OsMemoryFree((void *) frontHandle);
	
	return OS_NO_ERROR;
}

/*
 *  Name         : FrontIfWrite
 *  Description
 *     Write Data to Front.
 *  INPUT Arg
 *     U32  deviceId              : Front Id to Write
 *     U8   writeData             : Write Data
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL (OS_NO_ERROR : No error, OS_RETURN_ERROR : Error)
 */
BOOL FrontIfWrite(U32 deviceId, U8 *data, U32 len)
{
	FrontHandle_t *frontHandle;

	if (deviceId == 0)
	{
		/* Wrong Device Id */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("FrontIfWrite Error : Device ID is NULL -> not defined device\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}
	
	frontHandle = (FrontHandle_t *)deviceId;
	if(write(frontHandle->FrontFd, (char *)data, (int)len) < 0)
	{
		/* Write Error */
#ifdef MVMISC_DEBUG_ON
		OsDebugPrtf("FrontIfWrite Error : Write Error\n");
#endif // #ifdef MVMISC_DEBUG_ON
		return OS_RETURN_ERROR;
	}

	return OS_NO_ERROR;
}
