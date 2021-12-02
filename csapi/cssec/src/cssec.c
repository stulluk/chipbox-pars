
/* 
 * Author: John A. Thodiyil 05/31/2008 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <pthread.h>

#include "cssec.h"

#ifdef CSSEC_DEBUG
#define debug_printf printf
#else
#define debug_printf(fmt,args...)
#endif

#define CSSEC_OBJ_TYPE 'F'
#define CSSEC_SEC_FILE "/dev/orion_sec"

#define SEC_MAGIC 'z'

#define SEC_WR_MAILBOX0         _IOW(SEC_MAGIC, 0x00, int)
#define SEC_WR_MAILBOX1         _IOW(SEC_MAGIC, 0x01, int)
#define SEC_WR_MAILBOX2         _IOW(SEC_MAGIC, 0x02, int)
#define SEC_WR_MAILBOX3         _IOW(SEC_MAGIC, 0x03, int)
#define SEC_WR_MAILBOX4         _IOW(SEC_MAGIC, 0x04, int)
#define SEC_WR_MAILBOX5         _IOW(SEC_MAGIC, 0x05, int)
#define SEC_WR_MAILBOX6         _IOW(SEC_MAGIC, 0x06, int)

#define SEC_RD_MAILBOX0         _IOR(SEC_MAGIC, 0x07, int)
#define SEC_RD_MAILBOX1         _IOR(SEC_MAGIC, 0x08, int)
#define SEC_RD_MAILBOX2         _IOR(SEC_MAGIC, 0x09, int)
#define SEC_RD_MAILBOX3         _IOR(SEC_MAGIC, 0x0a, int)
#define SEC_RD_MAILBOX4         _IOR(SEC_MAGIC, 0x0b, int)
#define SEC_RD_MAILBOX5         _IOR(SEC_MAGIC, 0x0c, int)
#define SEC_RD_MAILBOX6         _IOR(SEC_MAGIC, 0x0d, int)
#define SEC_RD_MAILBOX7         _IOR(SEC_MAGIC, 0x0e, int)

#define SEC_SET_OTPOFST         _IOW(SEC_MAGIC, 0x80, int)
#define SEC_RD_OTP              _IOW(SEC_MAGIC, 0x81, int)

typedef struct tagCSSEC_OBJ {
	char obj_type;
	int dev_fd;

	int errno;
} CSSEC_OBJ;

// FIXME static char *sec_errstr[] = {
// FIXME 	"CSSEC: no error",
// FIXME 	"CSSEC: open sec device failed",
// FIXME 	"CSSEC: invalid handle passed",
// FIXME 	"CSSEC: set otp offset for read failed",
// FIXME 	"CSSEC: read otp failed",
// FIXME 	"CSSEC: close sec device failed",
// FIXME };

typedef enum 
{ 
	SEC_NO_ERROR = 0, 
	SEC_ERROR_OPEN_FAILED,		/* open filed                 */
	SEC_ERROR_IOCTL_FAILED,		/* ioctl filed                */
	SEC_ERROR_INVALID_PARAMETERS,	/* Bad parameter passed       */
} CSSEC_ErrCode;

CSSEC_HANDLE CSSEC_Open(void)
{
	CSSEC_OBJ *sec_obj;

	/* Need to check chip version to see if secure engine enabled
           EG. CSM1200.1 or CSM1201 if not do not allow open */

	sec_obj = malloc(sizeof(CSSEC_OBJ));
	if (NULL == sec_obj)
		return NULL;

	sec_obj->dev_fd = open(CSSEC_SEC_FILE, O_RDWR);
	if (sec_obj->dev_fd < 0)
		return NULL;

	sec_obj->obj_type = CSSEC_OBJ_TYPE;

	return sec_obj;
}

CSAPI_RESULT CSSEC_Close(CSSEC_HANDLE handle)
{
	CSSEC_OBJ *obj = (CSSEC_OBJ *)handle;

	if (NULL == obj)
		return CSAPI_FAILED;

	if (obj->dev_fd >= 0)
		close(obj->dev_fd);

	free(obj);
	obj = NULL;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_GetUniqueKey(CSSEC_HANDLE handle, unsigned char *uni_key)
{
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if (NULL == uni_key) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x000000FC, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &uni_key, SEC);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_CheckSecureBootEnable(CSSEC_HANDLE handle, unsigned char *en_flag)
{
	unsigned int sec_conf = 0;

	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if (NULL == en_flag) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x00000000, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &sec_conf, SEC);

	*en_flag = (sec_conf & 0x00000008) >> 3;

	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSSEC_CheckSecureJTAGEnable(CSSEC_HANDLE handle, unsigned char *en_flag)
{
	unsigned int sec_conf = 0;

	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if (NULL == en_flag) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x00000000, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &sec_conf, SEC);

	*en_flag = (sec_conf & 0x00000040) >> 5;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_CheckSecureCWEnable(CSSEC_HANDLE handle, unsigned char *en_flag)
{
	unsigned int sec_conf = 0;

	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if (NULL == en_flag) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x00000000, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &sec_conf, SEC);

	*en_flag = (sec_conf & 0x00000010) >> 4;

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_SetAuthenticationKey(CSSEC_HANDLE handle, 
					CSSEC_CRYPTO_METHOD method, 
					unsigned char *key, int key_len)
{
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if ((NULL == key) || ((key_len/8) < 16)) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_WR_MAILBOX2, key[0], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX3, key[4], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX4, key[8], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX5, key[12], SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX1, method, SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX0, 0, SEC);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_SetSessionKey(CSSEC_HANDLE handle, 
				CSSEC_CRYPTO_METHOD method, 
				unsigned char *key, int key_len)
{
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if ((NULL == key) || ((key_len/8) < 16))
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_WR_MAILBOX2, key[0], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX3, key[4], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX4, key[8], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX5, key[12], SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX1, method, SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX0, 1, SEC);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_SetCWLadderModeL2(CSSEC_HANDLE handle, 
				CSSEC_CRYPTO_METHOD method, 
				unsigned char *key, int key_len)
{
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if ((NULL == key) || ((key_len/8) < 16)) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_WR_MAILBOX2, key[0], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX3, key[4], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX4, key[8], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX5, key[12], SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX1, method, SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX0, 2, SEC);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_SetCWLadderModeL3(CSSEC_HANDLE handle, 
				CSSEC_CRYPTO_METHOD method, 
				unsigned char *key, int key_len)
{
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);
	
	if ((NULL == key) || ((key_len/8) < 16)) 
		return CSAPI_FAILED;

	IOCTL(dev_obj, SEC_WR_MAILBOX2, key[0], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX3, key[4], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX4, key[8], SEC);
	IOCTL(dev_obj, SEC_WR_MAILBOX5, key[12], SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX1, method, SEC);

	IOCTL(dev_obj, SEC_WR_MAILBOX0, 3, SEC);

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSSEC_GetSettings(CSSEC_HANDLE handle, CSSEC_SETTING *ssettings)
{
	unsigned int secconfig;
	unsigned int uniqueid;
	CSSEC_OBJ *dev_obj = (CSSEC_OBJ *)handle;

	CHECK_HANDLE_VALID(dev_obj, CSSEC_OBJ_TYPE);

	/* otp_offset 0: security configuration , 0x0FC: Unique Id */
	/* Use ioctl to */
	/* Get secconfig & set corresponding bits in ssettings */
	/* Get unique id & set corresponding bits in ssettings */ 

	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x00000000, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &secconfig, SEC);
	IOCTL(dev_obj, SEC_SET_OTPOFST, 0x000000fc, SEC);
	IOCTL(dev_obj, SEC_RD_OTP, &uniqueid, SEC);

	ssettings->irdeto_scss.wCSSN = uniqueid;
	ssettings->irdeto_scss.bCrypto = 0x01;
	ssettings->irdeto_scss.wRsaBoot = (secconfig & 0x8 ) >> 3 ;
	ssettings->irdeto_scss.wJtag = (secconfig & 0x40 ) >> 5;
	ssettings->irdeto_scss.wCwMode = 0x11;
	ssettings->irdeto_scss.wCwMode |= (secconfig & 0x10 ) >> 3;

	return CSAPI_SUCCEED;
}
