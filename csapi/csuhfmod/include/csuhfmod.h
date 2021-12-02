#ifndef __CSAPI_UHFMOD_H__
#define __CSAPI_UHFMOD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

    
typedef enum CSUHFMOD_MODELNUM_{
    CSUHFMOD_SAMSUNG_RMUP74055AG = 0,
    CSUHFMOD_TENA_TNF0170U623R = 1,
    CSUHFMOD_OTHERS = 2
} CSUHFMOD_MODELNUM;
    
typedef enum CSUHFMOD_SOUNDSUBCARRIER_{
    CSUHFMOD_4_5_MHZ = 0,
    CSUHFMOD_5_5_MHZ = 1,
    CSUHFMOD_6_0_MHZ = 2,
    CSUHFMOD_6_5_MHZ = 3
} CSUHFMOD_SOUNDSUBCARRIER;
    
typedef enum CSUHFMOD_PICSOUNDRATIO_{
    CSUHFMOD_12DB = 0,
    CSUHFMOD_16DB = 1
} CSUHFMOD_PICSOUNDRATIO;

typedef unsigned int CSUHFMOD_CHANNEL;     /* 21 through 69   */

typedef struct CSUHFMOD_PARAMETERS_{
    CSUHFMOD_SOUNDSUBCARRIER           ssubcarrier;
    CSUHFMOD_PICSOUNDRATIO              picsoundratio;
    //    CSUHFMOD_CHANNEL                   channel;
} CSUHFMOD_PARAMETERS;

typedef enum CSUHFMOD_MODE_{
    CSUHFMOD_STANDBY = 0, /* SAMSUNG_RMUP74055AG: OSC=1; ATT=1; SO=1 */
                          /* TENA_TNF0170U623R: OSC=0; ATT=1; SO=1 */
    CSUHFMOD_ACTIVE = 1    /* ~ STANDBY */
} CSUHFMOD_MODE;
    
typedef struct CSUHFMOD_STATUS_{
    unsigned  char    VCOOutOfRange;
    unsigned  char    VCOFreqTooHigh;    /* Too High=1, Too Low = 0 , only valid of VCOOutOfRange is set */
    unsigned  char    LowVCOAvtive;       /* Low VCO active=1, High VCO active=0 */
} CSUHFMOD_STATUS;
    

typedef enum {
	CSUHFMOD_SUCCESS = 0,
	CSUHFMOD_BAD_HANDLE,
	CSUHFMOD_BAD_I2C_HANDLE,
	CSUHFMOD_BAD_PARAMATERS,
	CSUHFMOD_SET_MODE_ERROR,
	CSUHFMOD_SET_PARAMS_ERROR,
    CSUHFMOD_SET_CHANNEL_ERROR,
    CSUHFMOD_GET_STATUS_ERROR
} CSUHFMOD_ErrCode;

typedef void *CSUHFMOD_HANDLE;

CSUHFMOD_HANDLE CSUHFMOD_Open(CSUHFMOD_MODELNUM model);
CSAPI_RESULT CSUHFMOD_Close(CSUHFMOD_HANDLE handle);
CSAPI_RESULT CSUHFMOD_SetParameters(CSUHFMOD_HANDLE handle, CSUHFMOD_PARAMETERS params);
CSAPI_RESULT CSUHFMOD_GetParameters(CSUHFMOD_HANDLE handle, CSUHFMOD_PARAMETERS *params);
CSAPI_RESULT CSUHFMOD_GetMode(CSUHFMOD_HANDLE handle, CSUHFMOD_MODE *mode);
CSAPI_RESULT CSUHFMOD_SetMode(CSUHFMOD_HANDLE handle, CSUHFMOD_MODE mode);
CSAPI_RESULT CSUHFMOD_GetStatus(CSUHFMOD_HANDLE handle, CSUHFMOD_STATUS * status);
CSAPI_RESULT CSUHFMOD_GetChannel(CSUHFMOD_HANDLE handle, CSUHFMOD_CHANNEL *channel);
CSAPI_RESULT CSUHFMOD_SetChannel(CSUHFMOD_HANDLE handle, CSUHFMOD_CHANNEL channel);


CSUHFMOD_ErrCode CSUHFMOD_GetErrCode(CSUHFMOD_HANDLE handle);
char *CSUHFMOD_GetErrString(CSUHFMOD_HANDLE handle);

#ifdef __cplusplus
 }
#endif

#endif				/* __CSAPI_I2C_H */
