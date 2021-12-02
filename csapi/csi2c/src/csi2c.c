#define __USE_UNIX98

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <string.h>

#include "csi2c.h"

#define CSI2C_OBJ_TYPE   'I'

#define I2C_SET	            _IOW('I', 0, unsigned long)
#define I2C_GET             _IOR('I', 1, unsigned long)
#define I2C_SETSPEED        _IOW('I', 2, unsigned long)
#define I2C_GETSPEED        _IOR('I', 3, unsigned long)
#define I2C_SETLOOPADDR     _IOW('I', 4, unsigned long)
#define I2C_CLRLOOPADDR     _IOW('I', 5, unsigned long)
#define I2C_GETLOOPINFO     _IOR('I', 6, unsigned long)
#define I2C_SETCHIPADDRESS  _IOW('I', 7, unsigned long)
#define I2C_GETCHIPADDRESS  _IOR('I', 8, unsigned long)
#define I2C_SETADDRALEN     _IOW('I', 9, unsigned long)

#define i2c_set_speed_100k(x)  x = (void *)(1<<20 + ((unsigned long)x) & 0xff0fffff)
#define i2c_set_speed_400k(x)  x = (void *)(1<<21 + ((unsigned long)x) & 0xff0fffff)
#define i2c_get_speed(x)  ((unsigned long)x >> 20) & 0xf

#define i2c_set_loop_address(x) x = (void *)(1<< 24 + ((unsigned long)x) & 0xf0ffffff)
#define i2c_clr_loop_address(x) x = (void *)((unsigned long)x & 0xf0ffffff)
#define i2c_get_loop_info(x) ((unsigned long)x>>24) & 0xf

#define i2c_set_chip_address(x,chipaddress) x = (void *)(((unsigned long)x & 0xffffff00) | (chipaddress & 0xff))
#define i2c_get_chip_address(x) (unsigned char) ((unsigned long)x & 0xff)

#define i2c_get_sleeptime(x) (((unsigned long)x >> 8) & 0xff
#define i2c_get_chipaddress(x) ((unsigned long)x) & 0xff
#define i2c_get_address_alen(x) (((unsigned long)x) >> 16) & 0xff

#ifdef CSI2C_DEBUG
#define debug_printf  printf
#else
#define debug_printf(fmt,args...)
#endif

typedef struct tagI2C_OBJ {
	unsigned char obj_type;
	int dev_fd;
	unsigned int chipaddress;
	int errcode;
	CSI2C_Attr i2c_attr;
} CSI2C_OBJ;

CSI2C_HANDLE CSI2C_Open(unsigned int chipaddress)
{
	int retval = 0;
	CSI2C_OBJ *handle = NULL;

	handle = malloc(sizeof(struct tagI2C_OBJ));
	if (handle == NULL) {
		debug_printf(" I2C Open Error: There is no enough memory!\n");
		return handle;
	}

	handle->dev_fd = open("/dev/misc/orion_i2c", O_RDWR);
	if (handle->dev_fd < 0) {
		debug_printf(" I2C Open Error : %s\n", strerror(errno));
		free(handle);
		return NULL;
	}

	retval = ioctl(handle->dev_fd, I2C_SETCHIPADDRESS, chipaddress);
	if (retval < 0) {
		debug_printf(" I2C Open Error(I2C_SETCHIPADDRESS failed!): %s\n", strerror(errno));
		close(handle->dev_fd);
		free(handle);
		return NULL;
	}
	handle->chipaddress = chipaddress;

	handle->i2c_attr.speed = ioctl(handle->dev_fd, I2C_GETSPEED);
	if ((handle->i2c_attr.speed) < 0) {
		debug_printf(" I2C Open Error(I2C_GETSPEED failed!): %s\n", strerror(errno));
		close(handle->dev_fd);
		free(handle);
		return NULL;
	}

	handle->i2c_attr.loop = I2C_ADDR_NOLOOP;
	handle->i2c_attr.subaddr_num = 1;
	handle->i2c_attr.write_delayus = 0;
	handle->obj_type = 'I';
	return handle;
}

CSAPI_RESULT CSI2C_Close(CSI2C_HANDLE handle)
{
	int retval = 0;
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSI2C_OBJ_TYPE);

	retval = close(dev_obj->dev_fd);
	if (retval < 0) {
		dev_obj->errcode = I2C_CLOSE_ERROR;
		debug_printf(" I2C Close Error(close): %s\n", strerror(errno));
		return CSAPI_FAILED;
	}
	free(dev_obj);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSI2C_Read(CSI2C_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num)
{
	int retval = 0;
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSI2C_OBJ_TYPE);

	if (buffer == NULL) {
		dev_obj->errcode = I2C_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	retval = lseek(dev_obj->dev_fd, subaddr, SEEK_SET);
	if (retval < 0) {
		debug_printf(" I2C Read Error(lseek): %s\n", strerror(errno));
		dev_obj->errcode = I2C_READ_ERROR;
		return CSAPI_FAILED;
	}

	if (num > 0) {
		retval = read(dev_obj->dev_fd, buffer, num);
		if (retval < 0) {
			debug_printf(" I2C Read Error(read): %s\n", strerror(errno));
			dev_obj->errcode = I2C_READ_ERROR;
			return CSAPI_FAILED;
		}
	}
	dev_obj->errcode = I2C_SUCCESS;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSI2C_Write(CSI2C_HANDLE handle, unsigned int subaddr, char *buffer, unsigned int num)
{
	int retval = 0;
	unsigned int burst_num;
	unsigned int j = 0;
	unsigned int i = 0;
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSI2C_OBJ_TYPE);

	if (buffer == NULL) {
		dev_obj->errcode = I2C_BAD_PARAMATER;
		return CSAPI_FAILED;
	}
	retval = lseek(dev_obj->dev_fd, subaddr, SEEK_SET);
	if (retval < 0) {
		dev_obj->errcode = I2C_WRITE_ERROR;
		debug_printf(" I2C Write Error(lseek): %s\n", strerror(errno));
		return CSAPI_FAILED;
	}
	if (dev_obj->i2c_attr.loop == I2C_ADDR_LOOP) {
		burst_num = 1;
	}
	else {
		burst_num = num;
	}
	while (burst_num <= num && num != 0) {
		retval = write(dev_obj->dev_fd, buffer, burst_num);
		if (retval < 0) {
			dev_obj->errcode = I2C_WRITE_ERROR;
			debug_printf(" I2C Write Error(write): %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		num--;
		if (dev_obj->i2c_attr.loop == I2C_ADDR_LOOP) {
			buffer += 1;
		}
		for (i = 0; i < dev_obj->i2c_attr.write_delayus; i++) {
			for (j = 0; j < 0x8; j++) {
				;
			}
		}
	}
	dev_obj->errcode = I2C_SUCCESS;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSI2C_GetAttr(CSI2C_HANDLE handle, CSI2C_Attr * attr)
{
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSI2C_OBJ_TYPE);

	if (attr == NULL) {
		dev_obj->errcode = I2C_BAD_PARAMATER;
		return CSAPI_FAILED;
	}

	attr->write_delayus = dev_obj->i2c_attr.write_delayus;
	attr->speed = dev_obj->i2c_attr.speed;
	attr->loop = dev_obj->i2c_attr.loop;
	attr->subaddr_num = dev_obj->i2c_attr.subaddr_num;
	dev_obj->errcode = I2C_SUCCESS;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSI2C_SetAttr(CSI2C_HANDLE handle, CSI2C_Attr * attr)
{
	int retval = 0;
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	CHECK_HANDLE_VALID(dev_obj, CSI2C_OBJ_TYPE);

	if (attr == NULL) {
		dev_obj->errcode = I2C_BAD_PARAMATER;
		return CSAPI_FAILED;
	}

	if (attr->write_delayus != dev_obj->i2c_attr.write_delayus) {
		dev_obj->i2c_attr.write_delayus = attr->write_delayus;
	}

	if (attr->speed != dev_obj->i2c_attr.speed) {
		if ((attr->speed != I2C_SPEED_100K) && (attr->speed != I2C_SPEED_400K)) {
			dev_obj->errcode = I2C_ATTR_SPEEDPARAMATER_ERROR;
			return CSAPI_FAILED;
		}
		retval = ioctl(dev_obj->dev_fd, I2C_SETSPEED, attr->speed);
		if (retval < 0) {
			dev_obj->errcode = I2C_SETATTR_ERROR;
			debug_printf(" I2C SetAttr Error(ioctl): %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		dev_obj->i2c_attr.speed = attr->speed;
	}

	if (attr->loop != dev_obj->i2c_attr.loop) {

		switch (attr->loop) {
		case I2C_ADDR_LOOP:
			retval = ioctl(dev_obj->dev_fd, I2C_SETLOOPADDR, attr->loop);
			if (retval < 0) {
				dev_obj->errcode = I2C_SETATTR_ERROR;
				debug_printf(" I2C SetAttr Error(ioctl): %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			break;
		case I2C_ADDR_NOLOOP:
			retval = ioctl(dev_obj->dev_fd, I2C_CLRLOOPADDR, attr->loop);
			if (retval < 0) {
				dev_obj->errcode = I2C_SETATTR_ERROR;
				debug_printf(" I2C SetAttr Error(ioctl): %s\n", strerror(errno));
				return CSAPI_FAILED;
			}
			break;
		default:
			dev_obj->errcode = I2C_ATTR_LOOPPARAMATER_ERROR;
			return CSAPI_FAILED;
		}

		dev_obj->i2c_attr.loop = attr->loop;
	}
	if (attr->subaddr_num != dev_obj->i2c_attr.subaddr_num) {
//		if (attr->subaddr_num < 0) {
//			dev_obj->errcode = I2C_ATTR_SUBADDRPARAMATER_ERROR;
//			return CSAPI_FAILED;
//		}

		retval = ioctl(dev_obj->dev_fd, I2C_SETADDRALEN, attr->subaddr_num);
		if (retval < 0) {
			dev_obj->errcode = I2C_SETATTR_ERROR;
			debug_printf(" I2C SetAttr Error(ioctl): %s\n", strerror(errno));
			return CSAPI_FAILED;
		}
		dev_obj->i2c_attr.subaddr_num = attr->subaddr_num;
	}
	dev_obj->errcode = I2C_SUCCESS;
	return CSAPI_SUCCEED;
}

CSI2C_ErrCode CSI2C_GetErrCode(CSI2C_HANDLE handle)
{
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	if (dev_obj == NULL || (dev_obj->dev_fd < 0) || dev_obj->obj_type != 'I')
		return I2C_BAD_PARAMATER;
	return (dev_obj->errcode);
}

char *CSI2C_GetErrString(CSI2C_HANDLE handle)
{
	char *errorstr;
	CSI2C_OBJ *dev_obj = (CSI2C_OBJ *) handle;

	if (dev_obj == NULL || dev_obj->dev_fd < 0 || dev_obj->obj_type != 'I')
		errorstr = " I2C: Input PARAMATER is invalid\n";
	else {
		switch (dev_obj->errcode) {
		case I2C_SUCCESS:
			errorstr = " I2C: Operation is success\n";
			break;
		case I2C_OPEN_ERROR:
			errorstr = " I2C: Open  operation is failed\n";
			break;
		case I2C_CLOSE_ERROR:
			errorstr = " I2C: Close operation is failed\n";
			break;
		case I2C_READ_ERROR:
			errorstr = " I2C: Read  operation is failed\n";
			break;
		case I2C_WRITE_ERROR:
			errorstr = " I2C: Write operation is failed\n";
			break;
		case I2C_BAD_PARAMATER:
			errorstr = " I2C: Input paramater is invalid\n";
			break;
		case I2C_SETATTR_ERROR:
			errorstr = " I2C: Set attribute  is  failed\n";
			break;
		case I2C_ATTR_LOOPPARAMATER_ERROR:
			errorstr = " I2C: LOOP paramater of attribute is invalid\n";
			break;
		case I2C_ATTR_SPEEDPARAMATER_ERROR:
			errorstr = " I2C: Speed paramater of attribute is invalid\n";
			break;
		case I2C_ATTR_SUBADDRPARAMATER_ERROR:
			errorstr = " I2C: Subaddr_num paramater of attribute  is  invalid\n";
			break;
		default:
			errorstr = " I2C: Invaild Error Type\n";
			break;
		}
	}
	return errorstr;
}

void CSI2C_PrintErrorStr(CSI2C_ErrCode errcode)
{

	switch (errcode) {
	case I2C_SUCCESS:
		printf(" I2C: Operation is success\n");
		break;
	case I2C_OPEN_ERROR:
		printf(" I2C: Open  Operation is failed\n");
		break;
	case I2C_CLOSE_ERROR:
		printf(" I2C: Close Operation is failed\n");
		break;
	case I2C_READ_ERROR:
		printf(" I2C: Read  Operation is failed\n");
		break;
	case I2C_WRITE_ERROR:
		printf(" I2C: Write Operation is failed\n");
		break;
	case I2C_BAD_PARAMATER:
		printf(" I2C: Input PARAMATER is invalid\n");
		break;
	case I2C_SETATTR_ERROR:
		printf(" I2C: Set Attribute  is  failed\n");
		break;
	case I2C_ATTR_LOOPPARAMATER_ERROR:
		printf(" I2C: LOOP paramater of attribute is invalid\n");
		break;
	case I2C_ATTR_SPEEDPARAMATER_ERROR:
		printf(" I2C: Speed paramater of attribute is invalid\n");
		break;
	case I2C_ATTR_SUBADDRPARAMATER_ERROR:
		printf(" I2C: Subaddr_num paramater of attribute  is  invalid\n");
		break;
	default:
		printf(" I2C: Invaild Error Type\n");
		break;
	}
	return;
}
