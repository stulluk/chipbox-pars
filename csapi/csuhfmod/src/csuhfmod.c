/*
  UHF Modulator driver based on I2C API
  Copyright@Celestial Semiconductor
  xiaodong.fan@celestialsemi.com
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "csapi.h"
#include "csuhfmod.h"

#define CSUHFMOD_OBJ_TYPE   'U'

#ifdef CSUHFMOD_DEBUG
#define debug_printf  printf
#else
#define debug_printf(fmt,args...)
#endif

#define MIN_CHANNEL 21
#define MAX_CHANNEL 69
#define CHANNEL_BW  8000 /* KHz */
#define MIN_FQ      471250 /* KHz */


#define UHFMOD_I2C_ADDRESS (0xca >> 1)
/*TENA TNF0170U623R C1 / SAMSUNG_RMUP74055AG C1*/
#define SO (1<<5)
#define PS (1<<3)
#define TENA_TNF0170U623R_C1_X3 (1<<2)
#define X2 (1<<1)

/*TENA TNF0170U623R C2 / SAMSUNG SAMSUNG_RMUP74055AG C0*/
#define TENA_TNF0170U623R_C2_PWC  (1<<7)
#define OSC  (1<<6)
#define ATT  (1<<5)
#define SFD1 (1<<4)
#define SFD0 (1<<3)
#define TENA_TNF0170U623R_C2_X5   (1<<1)
#define TENA_TNF0170U623R_C2_X4   (1<<0)

/*TENA TNF0170U623R PB1 / SAMSUNG_RMUP74055AG FM*/
#define TENA_TNF0170U623R_PB1_TPEN (1<<6)
#define N11  (1<<5)
#define N10  (1<<4)
#define N9   (1<<3)
#define N8   (1<<2)
#define N7   (1<<1)
#define N6   (1<<0)

/*TENA TNF0170U623R PB2/SAMSUNG_RMUP74055AG FL*/
#define N5   (1<<7)
#define N4   (1<<6)
#define N3   (1<<5)
#define N2   (1<<4)
#define N1   (1<<3)
#define N0   (1<<2)
#define X1   (1<<1)
#define X0   (1<<0)

/*TENA TNF0170U623R status/SAMSUNG_RMUP74055AG status*/
#define OOR (1<<0)
#define Y1 (1<<1)
#define Y2 (1<<2)


struct _modulator_datastruct {
    unsigned char C1; /*SAMSUNG_RMUP74055AG: C1 */
    unsigned char C2; /*SAMSUNG_RMUP74055AG: C0 */
    unsigned char PB1; /*SAMSUNG_RMUP74055AG: FM */
    unsigned char PB2; /*SAMSUNG_RMUP74055AG: FL */
  
} modulator_datastruct;

static char *uhfmodout_errstr[] = {
	"UHFMOD: no error",
	"UHFMOD: invalid handle",
	"UHFMOD: invalid i2c handle",
    "UHFMOD: invalid arguments",
    "UHFMOD: set mode error",
    "UHFMOD: set params error",
    "UHFMOD: set channel error",
    "UHFMOD: get status error"
}; 

typedef struct CSUHFMOD_OBJECT_{
    CSUHFMOD_MODELNUM             model;
    CSUHFMOD_PARAMETERS           params;
    CSUHFMOD_MODE                 mode;
    CSUHFMOD_CHANNEL              channel;
    CSUHFMOD_ErrCode              errcode;
    CSI2C_HANDLE                  i2c_handle;
	unsigned char obj_type;
} CSUHFMOD_OBJ;


CSUHFMOD_OBJ obj_uhfmod_handle={0,{0,1},0,0,0,NULL,0};

static int _init_device(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model)
{
    unsigned char controldata[4];
    
    switch (model) {
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
        modulator_datastruct.C1 = (1<<7) | SO | PS;
        modulator_datastruct.C2 =  ATT | OSC;
        modulator_datastruct.PB1 = 0;
        modulator_datastruct.PB2 = 0;
        controldata[3] = modulator_datastruct.PB2; 
        controldata[2] = modulator_datastruct.PB1; 
        controldata[1] = modulator_datastruct.C2; 
        controldata[0] = modulator_datastruct.C1;
        if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 4) != CSAPI_SUCCEED) {
            return 1;
        }
        return 0;
    case CSUHFMOD_TENA_TNF0170U623R:
        modulator_datastruct.C1 = (1<<7) | SO | PS;
        modulator_datastruct.C2 =  ATT;
        modulator_datastruct.PB1 = 0;
        modulator_datastruct.PB2 = 0;
        //        controldata = modulator_datastruct.PB2 << 24 | modulator_datastruct.PB1 << 16 | modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
        controldata[3] = modulator_datastruct.PB2; 
        controldata[2] = modulator_datastruct.PB1; 
        controldata[1] = modulator_datastruct.C2; 
        controldata[0] = modulator_datastruct.C1;

        if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 4) != CSAPI_SUCCEED) {
            printf("init device: write error!\n");
            return 1;
        }
        return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }

}

static int _config_mode(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model, CSUHFMOD_MODE mode)
{
    unsigned char controldata[4];
    switch (model) {
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
        switch (mode) {
        case CSUHFMOD_STANDBY:
            modulator_datastruct.C1 = modulator_datastruct.C1 | SO;
            modulator_datastruct.C2 = (modulator_datastruct.C2 | ATT | OSC);
            //            controldata =  modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
            controldata[1] = modulator_datastruct.C2; 
            controldata[0] = modulator_datastruct.C1;
        
            if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
                return 1;
            }
            break;
        case CSUHFMOD_ACTIVE:
            modulator_datastruct.C1 = modulator_datastruct.C1 & (~SO);
            modulator_datastruct.C2 = (modulator_datastruct.C2 & (~ (ATT | OSC)));
            //            controldata =  modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
            controldata[1] = modulator_datastruct.C2; 
            controldata[0] = modulator_datastruct.C1;

            if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
                return 1;
            }
            break;

        default:
            printf("Don't support this mode!\n");
        }

        return 0;

    case CSUHFMOD_TENA_TNF0170U623R:
        switch (mode) {
        case CSUHFMOD_STANDBY:
            modulator_datastruct.C1 = modulator_datastruct.C1 | SO;
            modulator_datastruct.C2 = (modulator_datastruct.C2 | ATT) & (~ OSC);
            //            controldata =  modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
            controldata[1] = modulator_datastruct.C2; 
            controldata[0] = modulator_datastruct.C1;

            if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
                return 1;
            }
            break;
        case CSUHFMOD_ACTIVE:
            //            printf("111111111111C1=0x%x, ~SO=0x%x\n",modulator_datastruct.C1,(~SO));
            modulator_datastruct.C1 = modulator_datastruct.C1 & (~SO);
            modulator_datastruct.C2 = (modulator_datastruct.C2 & (~ATT)) | OSC;
        
            controldata[1] = modulator_datastruct.C2; 
            controldata[0] = modulator_datastruct.C1;
            
            //  printf("write data c2=0x%x, c1=0x%x\n",controldata[1],controldata[0]);
            if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
                printf("Set mode: write i2c error!\n");
                return 1;
            }
            break;

        default:
            printf("Don't support this mode!\n");
        }

        return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }
}


static int _config_soundsubcarrier(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model, CSUHFMOD_SOUNDSUBCARRIER ssubcarrier)
{
    unsigned char controldata[2];
    unsigned char sfd_data = 0;
    switch (model){
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
    case CSUHFMOD_TENA_TNF0170U623R:
        switch (ssubcarrier){
        case CSUHFMOD_4_5_MHZ:
            sfd_data = 0;
            break;
        case CSUHFMOD_5_5_MHZ:
            sfd_data = SFD0;
            break;
        case CSUHFMOD_6_0_MHZ:
            sfd_data = SFD1;
            break;
        case CSUHFMOD_6_5_MHZ:
            sfd_data = SFD0 | SFD1;
            break;
        default:
            printf("Invalid SOUNDSUBCARRIER paramater!\n");
            return 1;
        }
        modulator_datastruct.C2 = (modulator_datastruct.C2 | sfd_data);
        //   controldata =  modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
        controldata[1] = modulator_datastruct.C2; 
        controldata[0] = modulator_datastruct.C1;

        if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
            return 1;
        }
  
       return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }

}


static int _config_picsoundratio(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model, CSUHFMOD_PICSOUNDRATIO picsoundratio)
{
    unsigned char controldata[2];
    switch (model) {
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
    case CSUHFMOD_TENA_TNF0170U623R:
        switch (picsoundratio){
        case CSUHFMOD_12DB:
            modulator_datastruct.C1 = (modulator_datastruct.C1 & (~PS));
            break;
        case CSUHFMOD_16DB:
            modulator_datastruct.C1 = (modulator_datastruct.C1 | PS );
            break;
        default:
            return 1;
        }
        
        //controldata =  modulator_datastruct.C2 << 8 | modulator_datastruct.C1;
        controldata[1] = modulator_datastruct.C2; 
        controldata[0] = modulator_datastruct.C1;

        if (CSI2C_Write(i2c_handle, 0, (char *)controldata, 2) != CSAPI_SUCCEED) {
            return 1;
        }

        return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }

}

/* Setting output channel frequecy (KHz)*/
static int _config_frequency(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model, unsigned int frequency)
{
    
    unsigned char freq_data[2];
    switch (model) {
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
    case CSUHFMOD_TENA_TNF0170U623R:
        modulator_datastruct.PB2 = ((frequency / 250) << 2 ) & 0xff;
        modulator_datastruct.PB1 = ((frequency / 250) >> 6) & 0x3f ;
        freq_data[0] = modulator_datastruct.PB1;
        freq_data[1] = modulator_datastruct.PB2;
        printf("Set frequency=%d, freq_data[0]=0x%x, freq_data[1]=0x%x\n",frequency, freq_data[0], freq_data[1]);
        if (CSI2C_Write(i2c_handle, 0, (char *)freq_data, 2) != CSAPI_SUCCEED) {
            return 1;
        }

        return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }
    
}

static int _get_status(CSI2C_HANDLE i2c_handle, CSUHFMOD_MODELNUM model, CSUHFMOD_STATUS * status)
{
    unsigned char status_data;
    switch (model) {
    case CSUHFMOD_SAMSUNG_RMUP74055AG:
    case CSUHFMOD_TENA_TNF0170U623R:
        if (CSI2C_Read(i2c_handle, 0, (char *)&status_data, 1) != CSAPI_SUCCEED) {
            printf("read status i2c error!\n");
            printf("%s",CSI2C_GetErrString(i2c_handle));
            return 1;
        }
        if (status_data & OOR){ 
            status->VCOOutOfRange = 1;
            if (status_data & Y1){
                status->VCOFreqTooHigh = 1;
                printf("VCO is out of range, freq too high!\n");
            }
            else{ 
                status->VCOFreqTooHigh = 0;
                printf("VCO is out of range, freq too low!\n");
            }
        }
        else {
            status->VCOOutOfRange = 0;
        }
        if (status_data & Y2){
            status->LowVCOAvtive = 1;
            printf("High VCO is active!\n");
        }
        else{
            status->LowVCOAvtive = 0;
            printf("Low VCO is active!\n");
        }
        return 0;
    default:
        printf("Don't support this device!\n");
        return 1;
        
    }
}


CSUHFMOD_HANDLE CSUHFMOD_Open(CSUHFMOD_MODELNUM model)
{
    CSAPI_RESULT retval;
    int error;
    CSUHFMOD_OBJ * handle= NULL;
	CSI2C_Attr i2c_attr;

    handle = &obj_uhfmod_handle;
    if (handle->obj_type == CSUHFMOD_OBJ_TYPE && handle->i2c_handle != NULL ) {
        printf("The device is already Opened");
        return (void *) handle;
    }

    if (handle->i2c_handle != NULL ){
        retval=CSI2C_Close(handle->i2c_handle);
        if (retval != CSAPI_SUCCEED){
            handle->obj_type = 0;
            return NULL;
        }
    }

    handle->i2c_handle = CSI2C_Open(UHFMOD_I2C_ADDRESS);
    if (handle->i2c_handle == NULL) {
        printf(" UHFMOD Open Error: Can't open i2c device!\n");
        return NULL;
    }

	retval = CSI2C_GetAttr(handle->i2c_handle, &i2c_attr);
	if (retval != CSAPI_SUCCEED) {
		printf("Get I2C Attr error!\n");
        retval=CSI2C_Close(handle->i2c_handle);
        handle->i2c_handle = NULL;
		return NULL;
	}

	i2c_attr.subaddr_num = 0;
    i2c_attr.speed = I2C_SPEED_100K;
    i2c_attr.write_delayus = 500;
	retval = CSI2C_SetAttr(handle->i2c_handle, &i2c_attr);
	if (retval != CSAPI_SUCCEED) {
		printf("Get I2C Attr error!\n");
        retval=CSI2C_Close(handle->i2c_handle);
        handle->i2c_handle = NULL;
		return NULL;
	}

    error = _init_device(handle->i2c_handle, model);
	if (error != 0) {
        printf("Can't init device!\n");
        retval=CSI2C_Close(handle->i2c_handle);
        handle->i2c_handle = NULL;
		return NULL;
	}


    handle->model = model;
    handle->obj_type = CSUHFMOD_OBJ_TYPE;
    handle->channel =0;
    (handle->params).ssubcarrier = CSUHFMOD_4_5_MHZ;
    (handle->params).picsoundratio = CSUHFMOD_16DB;
    handle->errcode = CSUHFMOD_SUCCESS;

    return (CSUHFMOD_HANDLE) handle;
}

CSAPI_RESULT CSUHFMOD_Close(CSUHFMOD_HANDLE handle)
{
   CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }
       
    if (CSI2C_Close(obj_handle->i2c_handle) != CSAPI_SUCCEED){
        printf("Can't Close I2C device!\n");
        
        return CSAPI_FAILED;
    }
    
    obj_handle->model = 0;
    obj_handle->obj_type = 0;
    obj_handle->channel = 0;
    obj_handle->mode = 0;
    (obj_handle->params).ssubcarrier = CSUHFMOD_4_5_MHZ;
    (obj_handle->params).picsoundratio = CSUHFMOD_16DB;
    obj_handle->errcode = CSUHFMOD_SUCCESS;
    obj_handle->i2c_handle = NULL;
    
    return CSAPI_SUCCEED;
    
}


CSAPI_RESULT CSUHFMOD_SetParameters(CSUHFMOD_HANDLE handle, CSUHFMOD_PARAMETERS params)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;
    int retval;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (params.ssubcarrier != (obj_handle->params).ssubcarrier) {
        retval = _config_soundsubcarrier(obj_handle->i2c_handle, obj_handle->model, params.ssubcarrier);
        if (retval == 1){
            obj_handle->errcode = CSUHFMOD_SET_PARAMS_ERROR;
            return CSAPI_FAILED;
        }
            
    }

    (obj_handle->params).ssubcarrier = params.ssubcarrier;
    if (params.picsoundratio != (obj_handle->params).picsoundratio) {
       retval =  _config_picsoundratio(obj_handle->i2c_handle, obj_handle->model, params.picsoundratio);
        if (retval != 0){
            obj_handle->errcode = CSUHFMOD_SET_PARAMS_ERROR;
            return CSAPI_FAILED;
        }
    }
    
    (obj_handle->params).picsoundratio = params.picsoundratio;
    
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSUHFMOD_GetParameters(CSUHFMOD_HANDLE handle, CSUHFMOD_PARAMETERS *params)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }
    if (params == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
        printf("Bad paramaters of the function!\n");
        return CSAPI_FAILED;
    }
    
    params->picsoundratio = (obj_handle->params).picsoundratio;
    params->ssubcarrier = (obj_handle->params).ssubcarrier;
    return CSAPI_SUCCEED;
    
}

CSAPI_RESULT CSUHFMOD_GetMode(CSUHFMOD_HANDLE handle, CSUHFMOD_MODE *mode)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  

    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (mode == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
        printf("Bad paramaters of the function!\n");
        return CSAPI_FAILED;
    }

    *mode = obj_handle->mode;
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSUHFMOD_SetMode(CSUHFMOD_HANDLE handle, CSUHFMOD_MODE mode)
{
    int retval;
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  

    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (mode != obj_handle->mode) {
        retval = _config_mode(obj_handle->i2c_handle, obj_handle->model, mode);
        if (retval != 0){
            obj_handle->errcode = CSUHFMOD_SET_MODE_ERROR;
            return CSAPI_FAILED;
        }
    }
    obj_handle->mode = mode;
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSUHFMOD_GetStatus(CSUHFMOD_HANDLE handle, CSUHFMOD_STATUS * status)
{
    int retval;
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (status == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
        printf("Bad paramaters of the function!\n");
        return CSAPI_FAILED;
    }

    retval = _get_status(obj_handle->i2c_handle, obj_handle->model, status);
    if (retval != 0){
        obj_handle->errcode = CSUHFMOD_GET_STATUS_ERROR;
        return CSAPI_FAILED;
    }

    return CSAPI_SUCCEED;

}


CSAPI_RESULT CSUHFMOD_GetChannel(CSUHFMOD_HANDLE handle, CSUHFMOD_CHANNEL *channel)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (channel == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
        printf("Bad paramaters of the function!\n");
        return CSAPI_FAILED;
    }
    
    *channel = obj_handle->channel;
    return CSAPI_SUCCEED;
}

CSAPI_RESULT CSUHFMOD_SetChannel(CSUHFMOD_HANDLE handle, CSUHFMOD_CHANNEL channel)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    unsigned int fq;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        printf("Bad CSUHFMOD handle!\n");
        return CSAPI_FAILED;
    }  
    if (obj_handle->i2c_handle == NULL){
        obj_handle->errcode = CSUHFMOD_BAD_I2C_HANDLE;
        printf("Bad CSUHFMOD I2C handle!\n");
        return CSAPI_FAILED;
    }

    if (channel < MIN_CHANNEL || channel > MAX_CHANNEL ){
        obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
        printf("Bad Channel number!\n");
        return CSAPI_FAILED;

    }

    if (obj_handle->channel != channel){
        fq = MIN_FQ + (channel - MIN_CHANNEL) * CHANNEL_BW;
        if(_config_frequency(obj_handle->i2c_handle, obj_handle->model, fq) ==1) {
            obj_handle->errcode = CSUHFMOD_BAD_PARAMATERS;
            return CSAPI_FAILED;
        }
    
        obj_handle->channel = channel;
    }
    return CSAPI_SUCCEED;
}


CSUHFMOD_ErrCode CSUHFMOD_GetErrCode(CSUHFMOD_HANDLE handle)
{
    CSUHFMOD_OBJ * obj_handle= (CSUHFMOD_OBJ *)handle;

    if (obj_handle == NULL || obj_handle->obj_type != CSUHFMOD_OBJ_TYPE ){
        return CSUHFMOD_BAD_HANDLE;
    }  
    return obj_handle->errcode;
}

char *CSUHFMOD_GetErrString(CSUHFMOD_HANDLE handle)
{
	CSUHFMOD_OBJ *obj_handle = (CSUHFMOD_OBJ *) handle;

	if ((NULL == obj_handle) || (obj_handle->obj_type != CSUHFMOD_OBJ_TYPE))
		return uhfmodout_errstr[CSUHFMOD_BAD_PARAMATERS];

	return uhfmodout_errstr[obj_handle->errcode];

}
