/*
 * Copyright 2007 celestial Semiconductor 
 * Reference to fxd's I2C
 * Author: Sun He
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "csspi.h"


CSSPI_HANDLE CSSPI_Open(void)
{
	CSSPI_OBJ *handle = NULL;
	
	handle = malloc(sizeof(struct tagSPI_OBJ));
	if (handle == NULL) {
		debug_printf(" SPI Open Error: There is no enough memory!\n");
		return handle;
	}

	handle->dev_fd = open("/dev/misc/orion_spi", O_RDWR);
	if (handle->dev_fd < 0) {
		debug_printf(" SPI Open Error: %s\n", strerror(errno));
		free(handle);
		return NULL;
	}

#if 0
	ioctl(handle->dev_fd, SPI_SET_BAUD, slavespd);
	if ((handle->speed) < 0) {
		debug_printf(" SPI Open Error(SPI_GETSPEED failed!): %s\n", strerror(errno));
		close(handle->dev_fd);
		free(handle);
		return NULL;
	}

	handle->speed = slavespd;
#endif

	handle->obj_type = CSSPI_OBJ_TYPE;
	return handle;
}

CSAPI_RESULT CSSPI_Close(CSSPI_HANDLE handle)
{
	int retval;
	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	retval = close(dev_obj->dev_fd);
	if (retval < 0) {
		dev_obj->errcode = SPI_CLOSE_ERROR;
		debug_printf(" SPI Close Error(close): %s\n", strerror(errno));
		return CSAPI_FAILED;		
	}
	free(dev_obj);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSPI_Read(CSSPI_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num)
{
	int retval; 

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *)handle;
	
	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	if (num > 32) {
		debug_printf(" SPI Read Error: NUM is to large (num<=32)!\n");
		return CSAPI_FAILED;
	}

	if (buffer == NULL) {
		dev_obj->errcode = SPI_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	retval = lseek(dev_obj->dev_fd, subaddr, SEEK_SET);
	if (retval < 0) {
		debug_printf(" SPI Read Error(lseek): %s\n", strerror(errno));
		dev_obj->errcode = SPI_READ_ERROR;
		return CSAPI_FAILED;
	}

	if (num > 0) {
		retval = read(dev_obj->dev_fd, buffer, num);
		if (retval < 0) {
			debug_printf(" SPI Read Error(read): %s\n", strerror(errno));
			dev_obj->errcode = SPI_READ_ERROR;
			return CSAPI_FAILED;
		}
	}
	dev_obj->errcode = SPI_SUCCESS;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSPI_Write(CSSPI_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num)
{
	int retval;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	if (num > 32) {
		debug_printf(" SPI Write Error: NUM is to large (num<=32)!\n");
		return CSAPI_FAILED;
	}

	if (buffer == NULL) {
		dev_obj->errcode = SPI_BAD_PARAMATER;
		return CSAPI_FAILED;
	}

	retval = lseek(dev_obj->dev_fd, subaddr, SEEK_SET);
	if (retval < 0) {
		dev_obj->errcode = SPI_WRITE_ERROR;
		debug_printf(" SPI Write Error(lseek)!: %s\n", strerror(errno));
		return CSAPI_FAILED;
	}

	retval = write(dev_obj->dev_fd, buffer, num);
	if (retval < 0) {
		dev_obj->errcode = SPI_WRITE_ERROR;
		debug_printf(" SPI Write Error(write): %s\n", strerror(errno));
		return CSAPI_FAILED;
	}

	dev_obj->errcode = SPI_SUCCESS;
	return CSAPI_SUCCEED;	
}

CSSPI_ErrCode CSSPI_GetErrCode(CSSPI_HANDLE handle)
{
	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *)handle;

	//CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);
	if (dev_obj == NULL || dev_obj->dev_fd < 0 || dev_obj->obj_type != 'S')
		return SPI_BAD_PARAMATER;
	return (dev_obj->errcode);
}

char *CSSPI_GetErrString(CSSPI_HANDLE handle)
{
	char *errorstr;
	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *)handle;

	if (dev_obj == NULL || dev_obj->dev_fd < 0 || dev_obj->obj_type != 'S') {
		errorstr = "SPI: Input PARAMETER is invalid\n";
	} else {
		switch (dev_obj->errcode) {
			case SPI_SUCCESS:
				errorstr = " SPI: Operation is success\n";
				break;
			case SPI_OPEN_ERROR:
				errorstr = " SPI: Open  operation is failed\n";
				break;
			case SPI_CLOSE_ERROR:
				errorstr = " SPI: Close operation is failed\n";
				break;
			case SPI_READ_ERROR:
				errorstr = " SPI: Read  operation is failed\n";
				break;
			case SPI_WRITE_ERROR:
				errorstr = " SPI: Write operation is failed\n";
				break;
			case SPI_BAD_PARAMATER:
				errorstr = " SPI: Input paramater is invalid\n";
				break;
			default:
				errorstr = " SPI: Invaild Error Type\n";
				break;	
		}
				
	}

	return errorstr;
}

void CSSPI_PrintErrorStr(CSSPI_ErrCode errcode)
{	
	switch (errcode) {
		case SPI_SUCCESS:
			printf(" SPI: Operation is success\n");
			break;
		case SPI_OPEN_ERROR:
			printf(" SPI: Open  Operation is failed\n");
			break;
		case SPI_CLOSE_ERROR:
			printf(" SPI: Close Operation is failed\n");
			break;
		case SPI_READ_ERROR:
			printf(" SPI: Read  Operation is failed\n");
			break;
		case SPI_WRITE_ERROR:
			printf(" SPI: Write Operation is failed\n");
			break;
		case SPI_BAD_PARAMATER:
			printf(" SPI: Input PARAMATER is invalid\n");
			break;
		default:
			printf(" SPI: Invaild Error Type\n");
			break;
	}

	return;
}

CSAPI_RESULT CSSPI_SetTMode(CSSPI_HANDLE handle, CSSPI_TMOD tmod)
{
	int ret = 0;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	ret = ioctl(dev_obj->dev_fd, SPI_SET_TMOD, tmod);
	if (ret < 0) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                close(dev_obj->dev_fd);
                free(dev_obj);
		dev_obj = NULL;
                return CSAPI_FAILED;
	}

	dev_obj->errcode = SPI_SUCCESS;

	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSSPI_SetClockPolarity(CSSPI_HANDLE handle, CSSPI_SCPOL pol)
{
	int ret = 0;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	ret = ioctl(dev_obj->dev_fd, SPI_SET_POL, pol);
	if (ret < 0) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                close(dev_obj->dev_fd);
                free(dev_obj);
		dev_obj = NULL;
                return CSAPI_FAILED;
	}

	dev_obj->errcode = SPI_SUCCESS;

	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSSPI_SetClockPhase(CSSPI_HANDLE handle, CSSPI_SCPH ph)
{
	int ret = 0;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	ret = ioctl(dev_obj->dev_fd, SPI_SET_PH, ph);
	if (ret < 0) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                close(dev_obj->dev_fd);
                free(dev_obj);
		dev_obj = NULL;
                return CSAPI_FAILED;
	}

	dev_obj->errcode = SPI_SUCCESS;

	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSSPI_ReadByte(CSSPI_HANDLE handle, char *byte)
{
	int ret;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	if (NULL == byte) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                free(dev_obj);
		dev_obj = NULL;
		return CSAPI_FAILED;
	}

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	ret = ioctl(dev_obj->dev_fd, SPI_READ_BYTE);
	if (ret < 0) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                close(dev_obj->dev_fd);
                free(dev_obj);
		dev_obj = NULL;
                return CSAPI_FAILED;
	}

	*byte = (char)ret;

	dev_obj->errcode = SPI_SUCCESS;

	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSSPI_WriteByte(CSSPI_HANDLE handle, char *byte)
{
	int ret = 0;

	CSSPI_OBJ *dev_obj = (CSSPI_OBJ *) handle;

	if (NULL == byte) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                free(dev_obj);
		dev_obj = NULL;
		return CSAPI_FAILED;
	}

	CHECK_HANDLE_VALID(dev_obj, CSSPI_OBJ_TYPE);

	ret = ioctl(dev_obj->dev_fd, SPI_WRITE_BYTE, *byte);
	if (ret < 0) {
		debug_printf("%s Error!: %s", __FUNCTION__, strerror(errno));
                close(dev_obj->dev_fd);
                free(dev_obj);
		dev_obj = NULL;
                return CSAPI_FAILED;
	}

	dev_obj->errcode = SPI_SUCCESS;

	return CSAPI_SUCCEED;	
}





