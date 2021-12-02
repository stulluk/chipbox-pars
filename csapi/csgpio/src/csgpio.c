/*
  Copyright 2007 Celestial Semiconductor
  xiaodong fan
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "csgpio.h"

#define  CSGPIO_OBJ_TYPE  	'G'
#define  CSGPIO2_OBJ_TYPE  	'g'

#define CSGPIO_MAGIC_NUMBER 'G'
#define MAX_GPIO_NUMBER 16
#define MAX_GPIO2_NUMBER 55

#define CMD_READ_GPIO     _IOR('G', 1, unsigned long)
#define CMD_WRITE_GPIO    _IOW('G', 2, unsigned long)
#define CMD_SET_DIRECTION _IOW('G', 3, unsigned long)
#define CMD_READ_GPIO2	 _IOR('g', 6, unsigned long)
#define CMD_WRITE_GPIO2   _IOW('g', 7, unsigned long)
#define CMD_SET_DIRECTION_GPIO2 _IOW('g', 8, unsigned long)

//#define GPIO_DEVICE_DIR "/dev/gpio/"
//#define GPIO2_DEVICE_DIR "/dev/gpio2/"

#define GPIO_DEVICE_0 "/dev/gpio/0"
#define GPIO_DEVICE_1 "/dev/gpio/1"
#define GPIO_DEVICE_2 "/dev/gpio/2"
#define GPIO_DEVICE_3 "/dev/gpio/3"
#define GPIO_DEVICE_4 "/dev/gpio/4"
#define GPIO_DEVICE_5 "/dev/gpio/5"
#define GPIO_DEVICE_6 "/dev/gpio/6"
#define GPIO_DEVICE_7 "/dev/gpio/7"
#define GPIO_DEVICE_8 "/dev/gpio/8"
#define GPIO_DEVICE_9 "/dev/gpio/9"
#define GPIO_DEVICE_10 "/dev/gpio/10"
#define GPIO_DEVICE_11 "/dev/gpio/11"
#define GPIO_DEVICE_12 "/dev/gpio/12"
#define GPIO_DEVICE_13 "/dev/gpio/13"
#define GPIO_DEVICE_14 "/dev/gpio/14"
#define GPIO_DEVICE_15 "/dev/gpio/15"


#define GPIO2_DEVICE_0 "/dev/gpio2/0"
#define GPIO2_DEVICE_1 "/dev/gpio2/1"

#define GPIO2_DEVICE_2 "/dev/gpio2/2"
#define GPIO2_DEVICE_3 "/dev/gpio2/3"
#define GPIO2_DEVICE_4 "/dev/gpio2/4"
#define GPIO2_DEVICE_5 "/dev/gpio2/5"
#define GPIO2_DEVICE_6 "/dev/gpio2/6"
#define GPIO2_DEVICE_7 "/dev/gpio2/7"
#define GPIO2_DEVICE_8 "/dev/gpio2/8"
#define GPIO2_DEVICE_9 "/dev/gpio2/9"
#define GPIO2_DEVICE_10 "/dev/gpio2/10"
#define GPIO2_DEVICE_11 "/dev/gpio2/11"
#define GPIO2_DEVICE_12 "/dev/gpio2/12"
#define GPIO2_DEVICE_13 "/dev/gpio2/13"
#define GPIO2_DEVICE_14 "/dev/gpio2/14"
#define GPIO2_DEVICE_15 "/dev/gpio2/15"
#define GPIO2_DEVICE_16 "/dev/gpio2/16"
#define GPIO2_DEVICE_17 "/dev/gpio2/17"
#define GPIO2_DEVICE_18 "/dev/gpio2/18"
#define GPIO2_DEVICE_19 "/dev/gpio2/19"

#define GPIO2_DEVICE_20 "/dev/gpio2/20"
#define GPIO2_DEVICE_21 "/dev/gpio2/21"
#define GPIO2_DEVICE_22 "/dev/gpio2/22"
#define GPIO2_DEVICE_23 "/dev/gpio2/23"
#define GPIO2_DEVICE_24 "/dev/gpio2/24"
#define GPIO2_DEVICE_25 "/dev/gpio2/25"
#define GPIO2_DEVICE_26 "/dev/gpio2/26"
#define GPIO2_DEVICE_28 "/dev/gpio2/28"
#define GPIO2_DEVICE_27 "/dev/gpio2/27"
#define GPIO2_DEVICE_29 "/dev/gpio2/29"
#define GPIO2_DEVICE_30 "/dev/gpio2/30"
#define GPIO2_DEVICE_31 "/dev/gpio2/31"
#define GPIO2_DEVICE_32 "/dev/gpio2/32"
#define GPIO2_DEVICE_33 "/dev/gpio2/33"
#define GPIO2_DEVICE_34 "/dev/gpio2/34"
#define GPIO2_DEVICE_35 "/dev/gpio2/35"
#define GPIO2_DEVICE_36 "/dev/gpio2/36"
#define GPIO2_DEVICE_37 "/dev/gpio2/37"
#define GPIO2_DEVICE_38 "/dev/gpio2/38"
#define GPIO2_DEVICE_39 "/dev/gpio2/39"
#define GPIO2_DEVICE_40 "/dev/gpio2/40"
#define GPIO2_DEVICE_41 "/dev/gpio2/41"
#define GPIO2_DEVICE_42 "/dev/gpio2/42"
#define GPIO2_DEVICE_43 "/dev/gpio2/43"
#define GPIO2_DEVICE_44 "/dev/gpio2/44"
#define GPIO2_DEVICE_45 "/dev/gpio2/45"
#define GPIO2_DEVICE_46 "/dev/gpio2/46"
#define GPIO2_DEVICE_47 "/dev/gpio2/47"
#define GPIO2_DEVICE_48 "/dev/gpio2/48"
#define GPIO2_DEVICE_49 "/dev/gpio2/49"
#define GPIO2_DEVICE_50 "/dev/gpio2/50"
#define GPIO2_DEVICE_51 "/dev/gpio2/51"
#define GPIO2_DEVICE_52 "/dev/gpio2/52"
#define GPIO2_DEVICE_53 "/dev/gpio2/53"
#define GPIO2_DEVICE_54 "/dev/gpio2/54"


#ifdef CSGPIO_DEBUG
#define debug_printf  printf
#else
#define debug_printf(fmt,args...)
#endif

typedef struct tagGPIO_OBJ {
	unsigned char obj_type;
	CSGPIO_DIRECTION current_direction;
	int gpio;
	int errcode;
	int dev_fd;
} CSGPIO_OBJ;


CSGPIO_HANDLE CSGPIO2_Open(unsigned int gpio)
{
	CSGPIO_OBJ *handle = NULL;
	char device_name[15];
	
	if (gpio > MAX_GPIO2_NUMBER) {
		printf("\n  Error: gpio2 number is lager than the MAX_GPIO2_NUMBER \n");
		return NULL;
	}
	handle = (CSGPIO_OBJ *) malloc(sizeof(struct tagGPIO_OBJ));
	if (handle == NULL) {
		debug_printf(" GPIO2 Open Error: There is no enough memory!\n");
		return handle;
	}
	switch (gpio) {
	case 0:
		strcpy(device_name, GPIO2_DEVICE_0);
		break;
	case 1:
		strcpy(device_name, GPIO2_DEVICE_1);
		break;
	case 2:
		strcpy(device_name, GPIO2_DEVICE_2);
		break;
	case 3:
		strcpy(device_name, GPIO2_DEVICE_3);
		break;
	case 4:
		strcpy(device_name, GPIO2_DEVICE_4);
		break;
	case 5:
		strcpy(device_name, GPIO2_DEVICE_5);
		break;
	case 6:
		strcpy(device_name, GPIO2_DEVICE_6);
		break;
	case 7:
		strcpy(device_name, GPIO2_DEVICE_7);
		break;
	case 8:
		strcpy(device_name, GPIO2_DEVICE_8);
		break;
	case 9:
		strcpy(device_name, GPIO2_DEVICE_9);
		break;
	case 10:
		strcpy(device_name, GPIO2_DEVICE_10);
		break;
	case 11:
		strcpy(device_name, GPIO2_DEVICE_11);
		break;
	case 12:
		strcpy(device_name, GPIO2_DEVICE_12);
		break;
	case 13:
		strcpy(device_name, GPIO2_DEVICE_13);
		break;
	case 14:
		strcpy(device_name, GPIO2_DEVICE_14);
		break;
	case 15:
		strcpy(device_name, GPIO2_DEVICE_15);
		break;
	case 16:
		strcpy(device_name, GPIO2_DEVICE_16);
		break;
	case 17:
		strcpy(device_name, GPIO2_DEVICE_17);
		break;
	case 18:
		strcpy(device_name, GPIO2_DEVICE_18);
		break;
	case 19:
		strcpy(device_name, GPIO2_DEVICE_19);
		break;
	case 20:
		strcpy(device_name, GPIO2_DEVICE_20);
		break;
	case 21:
		strcpy(device_name, GPIO2_DEVICE_21);
		break;
	case 22:
		strcpy(device_name, GPIO2_DEVICE_22);
		break;
	case 23:
		strcpy(device_name, GPIO2_DEVICE_23);
		break;
	case 24:
		strcpy(device_name, GPIO2_DEVICE_24);
		break;
	case 25:
		strcpy(device_name, GPIO2_DEVICE_25);
		break;
	case 26:
		strcpy(device_name, GPIO2_DEVICE_26);
		break;
	case 27:
		strcpy(device_name, GPIO2_DEVICE_27);
		break;
	case 28:
		strcpy(device_name, GPIO2_DEVICE_28);
		break;
	case 29:
		strcpy(device_name, GPIO2_DEVICE_29);
		break;
	case 30:
		strcpy(device_name, GPIO2_DEVICE_30);
		break;
	case 31:
		strcpy(device_name, GPIO2_DEVICE_31);
		break;
	case 32:
		strcpy(device_name, GPIO2_DEVICE_32);
		break;
	case 33:
		strcpy(device_name, GPIO2_DEVICE_33);
		break;
	case 34:
		strcpy(device_name, GPIO2_DEVICE_34);
		break;
	case 35:
		strcpy(device_name, GPIO2_DEVICE_35);
		break;
	case 36:
		strcpy(device_name, GPIO2_DEVICE_36);
		break;
	case 37:
		strcpy(device_name, GPIO2_DEVICE_37);
		break;
	case 38:
		strcpy(device_name, GPIO2_DEVICE_38);
		break;
	case 39:
		strcpy(device_name, GPIO2_DEVICE_39);
		break;
	case 40:
		strcpy(device_name, GPIO2_DEVICE_40);
		break;
	case 41:
		strcpy(device_name, GPIO2_DEVICE_41);
		break;
	case 42:
		strcpy(device_name, GPIO2_DEVICE_42);
		break;
	case 43:
		strcpy(device_name, GPIO2_DEVICE_43);
		break;
	case 44:
		strcpy(device_name, GPIO2_DEVICE_44);
		break;
	case 45:
		strcpy(device_name, GPIO2_DEVICE_45);
		break;
	case 46:
		strcpy(device_name, GPIO2_DEVICE_46);
		break;
	case 47:
		strcpy(device_name, GPIO2_DEVICE_47);
		break;
	case 48:
		strcpy(device_name, GPIO2_DEVICE_48);
		break;
	case 49:
		strcpy(device_name, GPIO2_DEVICE_49);
		break;
	case 50:
		strcpy(device_name, GPIO2_DEVICE_50);
		break;
	case 51:
		strcpy(device_name, GPIO2_DEVICE_51);
		break;
	case 52:
		strcpy(device_name, GPIO2_DEVICE_52);
		break;
	case 53:
		strcpy(device_name, GPIO2_DEVICE_53);
		break;
	case 54:
		strcpy(device_name, GPIO2_DEVICE_54);
		break;
	default:
		printf(" GPIO2 Open Error: Wrong device number!");
		free(handle);
		return NULL;
	}

	handle->dev_fd = open((const char *) device_name, O_RDWR);
	if (handle->dev_fd < 0) {
		debug_printf(" GPIO2 Open Error : %s\n", strerror(errno));
		if (handle->dev_fd == -EBUSY)
			printf("This GPIO2 pin is already held by other component\n");
		free(handle);
		return NULL;
	}

	handle->current_direction = GPIO_DIRECTION_READ;
	handle->obj_type = CSGPIO2_OBJ_TYPE;
	handle->gpio = gpio;

	return (CSGPIO_HANDLE) handle;
}

CSGPIO_HANDLE CSGPIO_Open(unsigned int gpio)
{
	CSGPIO_OBJ *handle = NULL;
	char device_name[15];
	if (gpio > MAX_GPIO_NUMBER) {
		printf("\n  Error: gpio number is lager than the MAX_GPIO_NUMBER \n");
		return NULL;
	}
	handle = (CSGPIO_OBJ *) malloc(sizeof(struct tagGPIO_OBJ));
	if (handle == NULL) {
		debug_printf(" GPIO Open Error: There is no enough memory!\n");
		return handle;
	}

	switch (gpio) {
	case 0:
		strcpy(device_name, GPIO_DEVICE_0);
		break;
	case 1:
		strcpy(device_name, GPIO_DEVICE_1);
		break;
	case 2:
		strcpy(device_name, GPIO_DEVICE_2);
		break;
	case 3:
		strcpy(device_name, GPIO_DEVICE_3);
		break;
	case 4:
		strcpy(device_name, GPIO_DEVICE_4);
		break;
	case 5:
		strcpy(device_name, GPIO_DEVICE_5);
		break;
	case 6:
		strcpy(device_name, GPIO_DEVICE_6);
		break;
	case 7:
		strcpy(device_name, GPIO_DEVICE_7);
		break;
	case 8:
		strcpy(device_name, GPIO_DEVICE_8);
		break;
	case 9:
		strcpy(device_name, GPIO_DEVICE_9);
		break;
	case 10:
		strcpy(device_name, GPIO_DEVICE_10);
		break;
	case 11:
		strcpy(device_name, GPIO_DEVICE_11);
		break;
	case 12:
		strcpy(device_name, GPIO_DEVICE_12);
		break;
	case 13:
		strcpy(device_name, GPIO_DEVICE_13);
		break;
	case 14:
		strcpy(device_name, GPIO_DEVICE_14);
		break;
	case 15:
		strcpy(device_name, GPIO_DEVICE_15);
		break;
	default:
		debug_printf(" GPIO Open Error: Wrong device number!");
		free(handle);
		return NULL;
	}

	handle->dev_fd = open((const char *) device_name, O_RDWR);
	if (handle->dev_fd < 0) {
		debug_printf(" GPIO Open Error : %s\n", strerror(errno));
		if (handle->dev_fd == -EBUSY)
			printf("This GPIO pin is already held by other component\n");
		free(handle);
		return NULL;
	}

	handle->current_direction = GPIO_DIRECTION_READ;
	handle->obj_type = CSGPIO_OBJ_TYPE;
	handle->gpio = gpio;

	return (CSGPIO_HANDLE) handle;
}

CSAPI_RESULT CSGPIO_Close(CSGPIO_HANDLE handle)
{
	int retval = 0;
	CSGPIO_OBJ *dev_obj = (CSGPIO_OBJ *) handle;

	//CHECK_HANDLE_VALID(dev_obj, CSGPIO_OBJ_TYPE);
	if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE || dev_obj->obj_type == CSGPIO_OBJ_TYPE)
	{
		retval = close(dev_obj->dev_fd);
		if (retval < 0) {
			dev_obj->errcode = GPIO_CLOSE_ERROR;
			if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
				debug_printf(" GPIO Close Error(close): %s\n", strerror(errno));
			else
				debug_printf(" GPIO2 Close Error(close): %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		free(dev_obj);
		return CSAPI_SUCCEED;
	}
	else
		return CSAPI_FAILED;
}

CSAPI_RESULT CSGPIO_Read(CSGPIO_HANDLE handle, unsigned char *bit)
{
	int retval = 0;
	char basevalue = '0';
	CSGPIO_OBJ *dev_obj = (CSGPIO_OBJ *) handle;

	//CHECK_HANDLE_VALID(dev_obj, CSGPIO_OBJ_TYPE);
	if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE || dev_obj->obj_type == CSGPIO_OBJ_TYPE)
	{
		if (bit == NULL) {
			dev_obj->errcode = GPIO_BAD_PARAMATER;
			return CSAPI_FAILED;
		}
		if (dev_obj->current_direction != GPIO_DIRECTION_READ) {
			dev_obj->errcode = GPIO_READ_ERROR;
			if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
				debug_printf(" GPIO Read Error(wrong direction)\n");
			else if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE)
				debug_printf(" GPIO2 Read Error(wrong direction)\n");
			return CSAPI_FAILED;
		}

		retval = read(dev_obj->dev_fd, bit, 1);
		if (retval < 0) {
			if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
				debug_printf(" GPIO Read Error(read): %s\n", strerror(errno));
			else if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE)
				debug_printf(" GPIO2 Read Error(read): %s\n", strerror(errno));
			dev_obj->errcode = GPIO_READ_ERROR;
			return CSAPI_FAILED;
		}
		*bit = *bit - basevalue;	/* return value is character '0' or '1', but we want to get a digital value */
		dev_obj->errcode = GPIO_SUCCESS;
		return CSAPI_SUCCEED;
	}
	else
		return CSAPI_FAILED;
}

CSAPI_RESULT CSGPIO_Write(CSGPIO_HANDLE handle, unsigned char bit)
{				/* only 0 and 1 are valid */
	int retval = 0;
	char value;
	CSGPIO_OBJ *dev_obj = (CSGPIO_OBJ *) handle;

	//CHECK_HANDLE_VALID(dev_obj, CSGPIO_OBJ_TYPE);
	if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE || dev_obj->obj_type == CSGPIO_OBJ_TYPE)
	{
		if (bit != 1 && bit != 0) {
			dev_obj->errcode = GPIO_BAD_PARAMATER;
			return CSAPI_FAILED;
		}
		if (dev_obj->current_direction != GPIO_DIRECTION_WRITE) {
			dev_obj->errcode = GPIO_WRITE_ERROR;
			if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
				debug_printf(" GPIO Write Error(wrong direction)\n");
			else if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE)
				debug_printf(" GPIO2 Write Error(wrong direction)\n");
			return CSAPI_FAILED;
		}

		if (bit == 1)
			value = '1';
		else
			value = '0';

		retval = write(dev_obj->dev_fd, &value, 1);
		if (retval < 0) {
			if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
				debug_printf(" GPIO Write Error(write): %s\n", strerror(errno));
			else if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE)
				debug_printf(" GPIO2 Write Error(write): %s\n", strerror(errno));
			dev_obj->errcode = GPIO_WRITE_ERROR;
			return CSAPI_FAILED;
		}

		dev_obj->errcode = GPIO_SUCCESS;
		return CSAPI_SUCCEED;
	}
	else
		return CSAPI_FAILED;
}

CSAPI_RESULT CSGPIO_SetDirection(CSGPIO_HANDLE handle, CSGPIO_DIRECTION gpio_direction)
{
	int retval = 0;
	CSGPIO_OBJ *dev_obj = (CSGPIO_OBJ *) handle;

	//CHECK_HANDLE_VALID(dev_obj, CSGPIO_OBJ_TYPE);
	if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE || dev_obj->obj_type == CSGPIO_OBJ_TYPE)
	{
		if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE)
			{
		
				retval = ioctl(dev_obj->dev_fd, CMD_SET_DIRECTION_GPIO2, gpio_direction);
				if (retval < 0) {
					dev_obj->errcode = GPIO_SETDIR_ERROR;
					debug_printf(" GPIO2 SetDirection Error(ioctl): %s\n", strerror(errno));
					return CSAPI_FAILED;
				}
			}
		else if(dev_obj->obj_type == CSGPIO_OBJ_TYPE)
			{
				retval = ioctl(dev_obj->dev_fd, CMD_SET_DIRECTION, gpio_direction);
				if (retval < 0) {
					dev_obj->errcode = GPIO_SETDIR_ERROR;
					debug_printf(" GPIO SetDirection Error(ioctl): %s\n", strerror(errno));
					return CSAPI_FAILED;
				}
			}
		dev_obj->current_direction = gpio_direction;

		dev_obj->errcode = GPIO_SUCCESS;
		return CSAPI_SUCCEED;
	}
	else
		return CSAPI_FAILED;
}

CSAPI_RESULT CSGPIO_GetDirection(CSGPIO_HANDLE handle, CSGPIO_DIRECTION * gpio_direction)
{
	CSGPIO_OBJ *dev_obj = (CSGPIO_OBJ *) handle;

	//CHECK_HANDLE_VALID(dev_obj, CSGPIO_OBJ_TYPE);
	if(dev_obj->obj_type == CSGPIO2_OBJ_TYPE || dev_obj->obj_type == CSGPIO_OBJ_TYPE)
	{
		*gpio_direction = dev_obj->current_direction;
		dev_obj->errcode = GPIO_SUCCESS;
		return CSAPI_SUCCEED;
	}
	else
		return CSAPI_FAILED;
}

CSGPIO_ErrCode CSGPIO_GetErrCode(CSGPIO_HANDLE handle)
{
	CSGPIO_OBJ *gpio_handle = (CSGPIO_OBJ *) handle;

	if (gpio_handle == NULL || (gpio_handle->obj_type != CSGPIO_OBJ_TYPE && gpio_handle->obj_type != CSGPIO2_OBJ_TYPE) || (gpio_handle->dev_fd < 0))
		return GPIO_BAD_PARAMATER;
	return (gpio_handle->errcode);
}

char *CSGPIO_GetErrString(CSGPIO_HANDLE handle)
{
	char *errorstr = NULL;
	CSGPIO_OBJ *gpio_handle = (CSGPIO_OBJ *) handle;

	if (gpio_handle == NULL || (gpio_handle->obj_type != CSGPIO_OBJ_TYPE && gpio_handle->obj_type != CSGPIO2_OBJ_TYPE) || gpio_handle->dev_fd < 0)
		{
			if(gpio_handle->obj_type != CSGPIO_OBJ_TYPE && gpio_handle->obj_type != CSGPIO2_OBJ_TYPE)
				errorstr =" \n gpio_handle->obj_type is wrong\n";
				else if(gpio_handle == NULL )
					{
						if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
							errorstr =" \n gpio_handle == NULL \n";
						else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
							errorstr =" \n gpio2_handle == NULL \n";
					}
					else if ( gpio_handle->dev_fd < 0)
						{
							if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
								errorstr =" \n gpio_handle->dev_fd < 0 \n";
							else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
								errorstr =" \n gpio2_handle->dev_fd < 0 \n";
						}
						else
							{
								if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
									errorstr = " GPIO: Input PARAMATER is invalid\n";
								else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
									errorstr = " GPIO2: Input PARAMATER is invalid\n";
							}
						
		}
	else {
		switch (gpio_handle->errcode) {
		case GPIO_SUCCESS:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Operation is success\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Operation is success\n";
			break;
		case GPIO_OPEN_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Open  operation is failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Open  operation is failed\n";
			break;
		case GPIO_CLOSE_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Close operation is failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Close operation is failed\n";
			break;
		case GPIO_READ_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Read  operation is failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
					errorstr = " GPIO2: Read  operation is failed\n";
			break;
		case GPIO_WRITE_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Write operation is failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Write operation is failed\n";
			break;
		case GPIO_BAD_PARAMATER:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Input paramater is invalid\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Input paramater is invalid\n";
			break;
		case GPIO_SETDIR_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Set attribute  is  failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
					errorstr = " GPIO2: Set attribute  is  failed\n";
			break;
		case GPIO_GETDIR_ERROR:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Set attribute  is  failed\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
					errorstr = " GPIO2: Set attribute  is  failed\n";
			break;
		default:
			if(gpio_handle->obj_type == CSGPIO_OBJ_TYPE)
				errorstr = " GPIO: Invaild Error Type\n";
			else if(gpio_handle->obj_type == CSGPIO2_OBJ_TYPE)
				errorstr = " GPIO2: Invaild Error Type\n";
			break;
		}
	}
	return errorstr;
}
