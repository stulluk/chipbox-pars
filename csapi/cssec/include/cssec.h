
/* 
 * Author: John A. Thodiyil 05/31/2008 
 */

#ifndef __CSAPI_SEC_H__
#define __CSAPI_SEC_H__

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *CSSEC_HANDLE;

typedef enum {
	SEC_TDES = 0,
	SEC_AES  = 1
} CSSEC_CRYPTO_METHOD;

typedef enum {
	SEC_KEY_128 = 128,
	SEC_KEY_192 = 192,
	SEC_KEY_256 = 256
} CSSEC_KEY_LENGTH;

typedef struct SEC_IRDETOSETTING  	/* See Irdeto Doc: 730882 V1.6 */
{
	unsigned  int	wCSSN;  	/* Irdeto assigned secure chip serial number */	
	unsigned  char 	bCrypto;  	/* Supported key ladder algorithms for secure PVR and secure CW */
	unsigned  int	wRsaBoot;	/* Indicate enablement of boot security feature */
	unsigned  int   wJtag;		/* Security setting related to JTAG */
	unsigned  int   wCwMode;	/* Secure CW & OVR mode supported */
} CSSEC_IRDETO_SETTING;

typedef union SEC_SETTING
{
	CSSEC_IRDETO_SETTING irdeto_scss;
} CSSEC_SETTING;

CSSEC_HANDLE CSSEC_Open(void);
CSAPI_RESULT CSSEC_Close(CSSEC_HANDLE handle);

CSAPI_RESULT CSSEC_GetUniqueKey(CSSEC_HANDLE handle, unsigned char *uni_key);

CSAPI_RESULT CSSEC_CheckSecureBootEnable(CSSEC_HANDLE handle, unsigned char *en_flag);
CSAPI_RESULT CSSEC_CheckSecureJTAGEnable(CSSEC_HANDLE handle, unsigned char *en_flag);
CSAPI_RESULT CSSEC_CheckSecureCWEnable(CSSEC_HANDLE handle, unsigned char *en_flag);

CSAPI_RESULT CSSEC_SetAuthenticationKey(CSSEC_HANDLE handle, CSSEC_CRYPTO_METHOD method, unsigned char *key, int key_len);
CSAPI_RESULT CSSEC_SetSessionKey(CSSEC_HANDLE handle, CSSEC_CRYPTO_METHOD method, unsigned char *key, int key_len);
CSAPI_RESULT CSSEC_SetCWLadderModeL2(CSSEC_HANDLE handle, CSSEC_CRYPTO_METHOD method, unsigned char *key, int key_len);
CSAPI_RESULT CSSEC_SetCWLadderModeL3(CSSEC_HANDLE handle, CSSEC_CRYPTO_METHOD method, unsigned char *key, int key_len);

/* 
 * this function only support Irdeto CA now.
 * suggest that other customers utilize the above functions.
 */
CSAPI_RESULT CSSEC_GetSettings(CSSEC_HANDLE handle, CSSEC_SETTING *ssettings);

#ifdef __cplusplus
}
#endif

#endif /*  __CSAPI_SEC_H__ */

