#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "csfrontpanel.h"
#include <linux/input.h>
#include <pthread.h>
#include "../csevt/include/csevt.h"

#ifdef CSFRONTPANEL_DEBUG
#define debug_printf printf
#else
#define debug_printf(fmt,args...)
#endif
#define CSFP_OBJ_TYPE 'F'
#define CSFP_FPC_FILE "/dev/orion_fpc"
#define CSFP_INPUT_FILE "/dev/input/event0"
#define CSFP_SERIAL1_FILE "/dev/serialport1"

#define FPC_MAGIC        'z'
#define FPC_LED_EN	 	        _IOW(FPC_MAGIC, 0x06, int)
#define FPC_LED_DISP	    	_IOW(FPC_MAGIC, 0x08, int)
#define FPC_RC_SET_SYSTEMCODE	_IOW(FPC_MAGIC, 0x09, int)
#define FPC_LED_GET		        _IOR(FPC_MAGIC, 0x0a, int)
#define FPC_KEYSCAN_EN 			_IOW(FPC_MAGIC, 0x0b, int)
#define FPC_KSCAN_GET			_IOR(FPC_MAGIC, 0x0c, int)
/* Adjust bit time count outside for nonstandard rc protocal */ 
#define FPC_SET_BitTimeCnt      _IOR(FPC_MAGIC, 0x0d, int)
#define FPC_GET_BitTimeCnt      _IOR(FPC_MAGIC, 0x0e, int)

/*
#define FPC_LED_EN_DIM1         _IOW(FPC_MAGIC, 0x0d, int)
#define FPC_LED_EN_DIM2         _IOW(FPC_MAGIC, 0x0e, int)
#define FPC_LED_EN_DIM3         _IOW(FPC_MAGIC, 0x0f, int)
#define FPC_LED_EN_DIM4         _IOW(FPC_MAGIC, 0x10, int)
#define FPC_LED_EN_DIM5         _IOW(FPC_MAGIC, 0x11, int)
#define FPC_LED_EN_DIM6         _IOW(FPC_MAGIC, 0x12, int)
#define FPC_LED_EN_DIM7         _IOW(FPC_MAGIC, 0x13, int)*/
#define FPC_RC_EN               _IOW(FPC_MAGIC, 0x14, int)



#define SP_MAGIC	'j'
#define SP_SET_UART_CONFIG	    _IOW(SP_MAGIC, 1, int)
#define SP_SET_UART_LOOPBACK	_IOW(SP_MAGIC, 2, int)
#define SP_SET_RST_LOOPBACK		_IOW(SP_MAGIC, 3, int)
#define SP_SET_IR_MODE			_IOW(SP_MAGIC, 4, int)
#define SP_SET_IR_LOOPBACK		_IOW(SP_MAGIC, 5, int)
#define SP_WR_TX_DATA_UART		_IOW(SP_MAGIC, 6, int)
#define SP_SET_ENABLE_SERIALIN	_IOW(SP_MAGIC, 7, int)
#define SP_SET_DISABLE_SERIALIN	_IOW(SP_MAGIC, 8, int)
#define SP_SET_RESET_SERIALIN	_IOW(SP_MAGIC, 9, int)
#define SP_SET_SERIALIN_ATTR	_IOW(SP_MAGIC, 10, int)
#define SP_SET_LCR				_IOW(SP_MAGIC, 11, int)
#define SP_GET_RBR_DATA			_IOR(SP_MAGIC, 12, int)


#define RC_HEAD 0x0000
#define KEYSCAN_HEAD 0x1000



typedef struct 
{
	CSFP_HANDLE fp_handle;
	void (* callfun)(CSFP_HANDLE ,CSFP_NOTIFYEVENT *, unsigned char *);
	CSFP_NOTIFYEVENT fp_event;

}CSFP_CALLBAK_PARAM;

CSFP_CALLBAK_PARAM fp_callback_input;
CSFP_CALLBAK_PARAM fp_callback_serialin;
CSEVT_HANDLE frontpanel_evt = NULL;

typedef struct {
  unsigned char obj_type;		
  int fpc_fd;
  int input_fd;
  int serialport_fd;
  unsigned char row;
  unsigned char column;
  unsigned char raw_display;
  CSFP_LEDDISPMODE mode; /* Initial mode OFF */
  CSFP_LEDDISPATTR attr; /* Initial attribute ON_BRIGHT */
  int errcode;
}CSFP_HANDLE_OBJ;

static CSFP_HANDLE_OBJ csfp_obj={0,-1,-1,-1,0,0,0,0,0,0};

static char *fpc_errstr[] = {
	"CSFP:no error",
	"CSFP:open fpc device failed",
	"CSFP:open input device failed",
	"CSFP:open serial port device failed",
	"CSFP:ioctl set systemcode failed",
	"CSFP:ioctl enable/disable keyscan failed",
	"CSFP:ioctl enable/disable rc failed",
	"CSFP:ioctl enable serial port  failed",
	"CSFP:ioctl disable serial port failed",
	"CSFP:ioctl reset serial port failed",
	"CSFP:ioctl set attribute of serial port failed",
	"CSFP:serial port is not opened",
	"CSFP:invalid arguments",
	"CSFP:unknown device name",
	"CSFP:device busy",
	"CSFP:invalid handle",
	"CSFP:Device already initialized",
	"CSFP:close front panel device failed",
	"CSFP:close serial device failed",
	"CSFP:close input device failed",
	"CSFP:register call back failed",
};


static unsigned char num_tab[] = { 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xff */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x10 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x20 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 
	0x03, 0x9f, 0x25, 0x0d, 0x99, 0x49, 0x41, 0x1f, /* 0x30 */
	0x01, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0x11, 0xff, 0x63, 0xff, 0x61, 0x71, 0xff, /* 0x40 */
	0x91, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xff, 
	0x31, 0xff, 0xff, 0xff, 0x73, 0x83, 0xff, 0xff, /* 0x50 */
	0xff, 0x89, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x60 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x70 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x80 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0x90 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xa0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xb0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xc0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xd0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xe0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* 0xf0 */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff 
};

static unsigned char Kscan_Map[16] = {
				0x00,0x01,0x02,0x03,
				0x04,0x05,0x06,0x07,
				0x08,0x09,0x0a,0x0b,
				0x0c,0x0d,0x0e,0x0f
				     };

CSFP_HANDLE  CSFP_Open(void)
{
	if (csfp_obj.obj_type != CSFP_OBJ_TYPE) {
		csfp_obj.fpc_fd = open(CSFP_FPC_FILE, O_RDWR);
		if (csfp_obj.fpc_fd < 0) {
			csfp_obj.errcode = CSFP_ERROR_OPEN_FPC_FAILED;
			return NULL;
		}
		csfp_obj.input_fd = open(CSFP_INPUT_FILE, O_RDWR);
		if (csfp_obj.input_fd < 0) {
			csfp_obj.errcode = CSFP_ERROR_OPEN_INPUT_FAILED;
			return NULL;
		}
		csfp_obj.attr = CSFP_LEDDISP_ON_BRIGHT;
		csfp_obj.mode = CSFP_LEDDISP_OFF;
		csfp_obj.obj_type = CSFP_OBJ_TYPE;
	}
	frontpanel_evt = CSEVT_Init();
	return (CSFP_HANDLE) &csfp_obj; 
}

CSAPI_RESULT CSFP_Close(CSFP_HANDLE handle)
{
	int retval;

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 


	if (handle_obj->fpc_fd != -1){
		retval = close(handle_obj->fpc_fd);
		if (retval < 0) {
			handle_obj->errcode = CSFP_ERROR_CLOSE_FPC;
			return CSAPI_FAILED;
		}

	}
	if (handle_obj->input_fd != -1){
		retval = close(handle_obj->input_fd);
		if (retval < 0) {
			handle_obj->errcode = CSFP_ERROR_CLOSE_INPUT;
			return CSAPI_FAILED;
		}

	}
	if (handle_obj->serialport_fd != -1){
		retval = close(handle_obj->serialport_fd);
		if (retval < 0) {
			handle_obj->errcode = CSFP_ERROR_CLOSE_SERIAL;
			return CSAPI_FAILED;
		}			
	}
	
	csfp_obj.obj_type=0;
	csfp_obj.fpc_fd=-1;
	csfp_obj.input_fd= -1;
	csfp_obj.serialport_fd = -1;
	csfp_obj.row =0;
	csfp_obj.column =0;
	csfp_obj.raw_display =0;
    csfp_obj.mode =0;
    csfp_obj.attr =0;
    csfp_obj.errcode =0;
	CSEVT_Term(frontpanel_evt );
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSFP_SetLEDDisplay(CSFP_HANDLE handle, const char const* num_str)
{
	int   val,i,fd,enb;
	char  str[5] = { 0 };


	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	fd = handle_obj->fpc_fd;

	if (NULL == num_str) 
		sprintf(str,"%-4s","");
	else
		sprintf(str,"%-4s",num_str);

	for (val = 0, i = 0; i<4 ; i++)
		val |= num_tab[(int)str[i]] << (8 * i);
	debug_printf("val in Set API = %d:\n",val);
	enb = 1;

	ioctl(fd, FPC_LED_DISP,val);
	ioctl(fd, FPC_LED_EN,enb);	
/*	while(1)
{

	sleep(2);
//		ioctl(fd, FPC_LED_DISP,&val);
		ioctl(fd, FPC_LED_EN_DIM1,&enb);
		sleep(2);	
//		ioctl(fd, FPC_LED_DISP,&val);
		ioctl(fd, FPC_LED_EN_DIM2,&enb);
		sleep(2);
//		ioctl(fd, FPC_LED_DISP,&val);
		ioctl(fd, FPC_LED_EN_DIM3,&enb);
		sleep(2);	
		ioctl(fd, FPC_LED_EN_DIM4,&enb);
		sleep(2);	
		ioctl(fd, FPC_LED_EN_DIM5,&enb);
		sleep(2);	
		ioctl(fd, FPC_LED_EN_DIM6,&enb);
		sleep(2);
		ioctl(fd, FPC_LED_EN_DIM7,&enb);
		sleep(2);
	}*/

	

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSFP_EnableSystemCode(CSFP_HANDLE handle)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSFP_DisableSystemCode(CSFP_HANDLE handle)
{
	return CSFP_SetSystemCode(handle, 0xffffffff);
}

CSAPI_RESULT CSFP_SetSystemCode(CSFP_HANDLE handle, int system_code)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	if (-1 == ioctl(handle_obj->fpc_fd, FPC_RC_SET_SYSTEMCODE, system_code)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETSYSTEMCODE_FAILED;
		return CSAPI_FAILED;

	}

	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSFP_SetLEDDisplayPos(CSFP_HANDLE handle, unsigned char row, unsigned char column)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	if (row < 4 && column <= 4 ){
		handle_obj->column = column;
		handle_obj->row = row;
	}
	return CSAPI_SUCCEED;
	
}



CSAPI_RESULT CSFP_GetLEDDisplay(CSFP_HANDLE handle, unsigned char *str_display )
{
	unsigned int val =0,temp;
	int i,fd,x;
	unsigned char  str[5] = { 0 };

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	if (str_display == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	fd = handle_obj->fpc_fd;
	
	ioctl(fd,FPC_LED_GET,&val);

	temp = val;
	for(i=0;i<4;i++) 
		str [i] = ((unsigned char ) (temp >>( 8 * i)) & 0XFFFFFFFF);
	str[4] = '\0' ;

	for(x=0;x<4;x++)
	{
		for(i=0;i<256;i++)
		{
			if(num_tab[i] == str[x])
			{
				str[x] = (char)i;
				break;
			}
		}
	}
	str[4] = '\0' ;
	strcpy(str_display,str);
	debug_printf("Get LED Char = %s \n",str);
	return CSAPI_SUCCEED;
}




CSAPI_RESULT CSFP_SetLEDDisplayMode (CSFP_HANDLE handle, CSFP_LEDDISPMODE mode )
{
		
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	handle_obj->mode = mode;

	if (CSFP_LEDDISP_ON == mode) 
		ioctl( handle_obj->fpc_fd, FPC_LED_EN, 1);
	else if (CSFP_LEDDISP_OFF == mode)
		ioctl( handle_obj->fpc_fd, FPC_LED_EN, 0);
		
	return CSAPI_SUCCEED;	
}


CSAPI_RESULT CSFP_GetLEDDisplayMode(CSFP_HANDLE handle, CSFP_LEDDISPMODE *mode )
{

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	if (mode == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	
	*mode = handle_obj->mode;
	
	return CSAPI_SUCCEED;

}


CSAPI_RESULT CSFP_SetLEDDisplayChar(CSFP_HANDLE handle, unsigned char *char_value)
{
	unsigned char disp_char[2];
	int val=0,count = 60,enb,temp;

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	if (char_value == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	strcpy(disp_char,char_value);

	disp_char[1] = '\0';

	val = num_tab[(int)disp_char[0]]<<(handle_obj->column * 8);

	if (handle_obj->column == 0)
	val |= 0xffffff00;
	if (handle_obj->column == 1)
	val |= 0xffff00ff;
	if (handle_obj->column == 2)
	val |= 0xff00ffff;
	if (handle_obj->column == 3)
	val |=0x00ffffff;
	enb =1;
	temp = 0xffffffff;
	switch(handle_obj->mode)
		{
			case CSFP_LEDDISP_ON :

				ioctl( handle_obj->fpc_fd, FPC_LED_DISP,val);
				ioctl( handle_obj->fpc_fd, FPC_LED_EN, enb);
				break;
			case CSFP_LEDDISP_BLINK:
				
				while(count--)
					{
						ioctl( handle_obj->fpc_fd, FPC_LED_DISP,val);
						ioctl( handle_obj->fpc_fd, FPC_LED_EN, enb);
						usleep(200000);
						ioctl( handle_obj->fpc_fd, FPC_LED_DISP,temp);
						ioctl( handle_obj->fpc_fd, FPC_LED_EN, enb);
						usleep(200000);
						if(CSFP_LEDDISP_BLINK != handle_obj->mode)
							break;
						
					}
				break;				
			case CSFP_LEDDISP_OFF :
				ioctl( handle_obj->fpc_fd, FPC_LED_DISP,temp);
				ioctl( handle_obj->fpc_fd, FPC_LED_EN, enb);				
				break;
			default :
				break;		
		}
	return CSAPI_SUCCEED;
}


CSAPI_RESULT CSFP_GetLEDDisplayChar(CSFP_HANDLE handle, unsigned char *char_value)
{	
	unsigned long val;
	int i= 0;
	unsigned char str[2];
	
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (char_value == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	ioctl(handle_obj->fpc_fd,FPC_LED_GET,&val);

	val = (val >> (handle_obj->column * 8));
	str[0] = (unsigned char)val;
	for(i=0;i<256;i++)
	{
		if(num_tab[i] == str[0] )
		{
			str[0] = i;
			break;
		}
	}
	str[1] = '\0';
	strcpy(char_value,str);
	debug_printf("Char in Get Led Display Char = %s \n",char_value);
	return CSAPI_SUCCEED;

}

CSAPI_RESULT CSFP_SetLEDDisplayAttr (CSFP_HANDLE handle, CSFP_LEDDISPATTR attr )
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	handle_obj->attr = attr;
	return CSAPI_SUCCEED;

}

CSAPI_RESULT CSFP_GetLEDDisplayAttr(CSFP_LEDDISP_HANDLE handle, CSFP_LEDDISPATTR *attr)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	if (attr == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	*attr = handle_obj->attr;
	return CSAPI_SUCCEED;
}	

CSAPI_RESULT CSFP_SetLEDDisplayRaw(CSFP_HANDLE handle, unsigned char *raw_value, int raw_len)
{
	unsigned char disp_char[4];
	int i=0,val=0,count = 60,enb,temp, column, disnum = 0;

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (raw_value == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
	
	if (raw_len > 4) raw_len = 4;

	memset(disp_char, 0, sizeof(disp_char));
	memcpy(disp_char, raw_value, raw_len);
	
	disnum = raw_len;

	column = handle_obj->column;
	
	if( (disnum == 0) || ((column + disnum) > 4) ){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
		
	if (column == 1)
		val |= 0x000000ff;
	if (column == 2)
		val |= 0x0000ffff;
	if (column == 3)
		val |=0x00ffffff;
	
	for( i = 0; i < disnum; i++,column++ )
	{
		temp = disp_char[i] << (column * 8);
		val |= temp;	
	}

	if (column == 1)
		val |= 0xffffff00;
	if (column == 2)
		val |= 0xffff0000;
	if (column == 3)
		val |=0xff000000;
	
	enb =1;
	temp = 0xffffffff;

	switch(handle_obj->mode)
	{
		case CSFP_LEDDISP_ON :

			ioctl(handle_obj->fpc_fd, FPC_LED_DISP,val);
			ioctl(handle_obj->fpc_fd, FPC_LED_EN, enb);
			break;
		case CSFP_LEDDISP_BLINK:

			while(count--)
			{
				ioctl(handle_obj->fpc_fd, FPC_LED_DISP,val);
				ioctl(handle_obj->fpc_fd, FPC_LED_EN, enb);
				usleep(200000);
				ioctl(handle_obj->fpc_fd, FPC_LED_DISP,temp);
				ioctl(handle_obj->fpc_fd, FPC_LED_EN, enb);
				usleep(200000);
				if(CSFP_LEDDISP_BLINK != handle_obj->mode)
					break;						
			}
			break;				
		case CSFP_LEDDISP_OFF :
			ioctl(handle_obj->fpc_fd, FPC_LED_DISP,temp);
			ioctl(handle_obj->fpc_fd, FPC_LED_EN, enb);				
			break;	
		default :
			break;			
	}

	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSFP_GetLEDDisplayRaw(CSFP_HANDLE handle, unsigned char *raw_value)
{
	unsigned long val,temp;
	unsigned char i, str[5];

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (raw_value == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}
		
	ioctl(handle_obj->fpc_fd,FPC_LED_GET,&val);

	for( i = 0; i < 4; i++ )
	{
		temp = (val >> (i * 8));
		str[i] = (unsigned char)temp;
	}
	str[i] = '\0';

	strcpy(raw_value,str);
	return CSAPI_SUCCEED;
	
}

CSAPI_RESULT CSFP_EnableKeyScan (CSFP_HANDLE handle)
{	
	int enable;
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	enable = 1;
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_KEYSCAN_EN, enable)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETKEYSCAN_FAILED;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSFP_DisableKeyScan(CSFP_HANDLE handle)
{
	int disable;
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	disable = 0;
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_KEYSCAN_EN, disable)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETKEYSCAN_FAILED;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
	
}


CSAPI_RESULT CSFP_EnableRemoteController (CSFP_HANDLE handle)
{	
	int enable;
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	enable = 1;
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_RC_EN, enable)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETRC_FAILED;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;	
}

CSAPI_RESULT CSFP_DisableRemoteController(CSFP_HANDLE handle)
{
	int disable;
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	disable = 0;
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_RC_EN, disable)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETRC_FAILED;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
	
}

/**********************************************************************************/
/* 
 *	Add for adjust remote controller bit time count outside.
 */

CSAPI_RESULT CSFP_SetBitTimeCnt(CSFP_HANDLE handle, unsigned short bittime)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_SET_BitTimeCnt, bittime)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_SETBitTimeCnt_FAILED;
		return CSAPI_FAILED;
	}
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSFP_GetBitTimeCnt(CSFP_HANDLE handle, unsigned short *bittime)
{
	unsigned long val;
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (-1 == ioctl(handle_obj->fpc_fd, FPC_SET_BitTimeCnt, &val)){ 
		handle_obj->errcode = CSFP_ERROR_IOCTL_GETBitTimeCnt_FAILED;
		return CSAPI_FAILED;
	}

	*bittime = val & 0xffff;

	return CSAPI_SUCCEED;
}

/**********************************************************************************/

CSAPI_RESULT CSFP_SetKeyScanMap(CSFP_HANDLE handle, unsigned char raw_input, unsigned char sys_code)
{
	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;
	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 

	Kscan_Map[raw_input] = sys_code;	
	return CSAPI_SUCCEED;
}

void CSFPNotify(void *puser, int fd, int events)
{
        struct input_event ev;
        unsigned long data;
        CSFP_NOTIFYEVENT csfp_event = ((CSFP_CALLBAK_PARAM* )puser)->fp_event;
        unsigned long code;
        events =events;
        if (puser == NULL)
                return;
        CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) ((CSFP_CALLBAK_PARAM* )puser)->fp_handle;


        if(fd == handle_obj->serialport_fd && csfp_event == CSFP_KEYSCAN_IN){
                ioctl(handle_obj->serialport_fd, SP_GET_RBR_DATA, &data);

                ((CSFP_CALLBAK_PARAM* )puser)->callfun((CSFP_HANDLE)handle_obj, &csfp_event, (unsigned char *) &data);
        }

        else if (fd == handle_obj->input_fd &&(csfp_event == CSFP_KEYSCAN_IN || csfp_event == CSFP_RCBYTE_IN)){
                read(handle_obj->input_fd, &ev,sizeof(struct input_event));
                if(ev.type == EV_KEY && ev.value != 0){
                        code = ev.code;
                ((CSFP_CALLBAK_PARAM* )puser)->callfun((CSFP_HANDLE)handle_obj, &csfp_event, (unsigned char *) &code);
                }
        }

        return;
}

CSAPI_RESULT CSFP_SetNotify(CSFP_HANDLE handle, void (*call_back_function)(CSFP_HANDLE, CSFP_NOTIFYEVENT *, unsigned char *), 
										CSFP_NOTIFYEVENT fp_event, unsigned char event_enable)
{

	CSFP_HANDLE_OBJ * handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE)) { 
		return CSAPI_FAILED; 
	} 
	
	if (call_back_function == NULL){
		handle_obj->errcode = CSFP_ERROR_INVALID_PARAMETERS;
		return CSAPI_FAILED;
	}

	if (fp_event == CSFP_KEYSCAN_IN || fp_event == CSFP_RCBYTE_IN){
			if (handle_obj->input_fd != -1 || frontpanel_evt != NULL){
				fp_callback_input.fp_handle = handle;
				fp_callback_input.callfun = call_back_function;
				fp_callback_input.fp_event = fp_event;
				if (CSAPI_FAILED == CSEVT_Register(frontpanel_evt,  
										handle_obj->input_fd, 
										CSFPNotify, 
										&fp_callback_input, 
										event_enable ? EVT_READ : EVT_INVALID)){
					handle_obj->errcode = CSFP_ERROR_REGISTER_CALLBACK;	
					//CSEVT_Term(frontpanel_evt);
					return CSAPI_FAILED;	
				}
			}
			else{ 			
				handle_obj->errcode=CSFP_ERROR_OPEN_INPUT_FAILED;
				return CSAPI_FAILED;
			}
		
		}

	return CSAPI_SUCCEED;
}

CSFP_ErrCode CSFP_GetErrCode(CSFP_HANDLE handle)
{
	CSFP_HANDLE_OBJ *handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE))
		return CSFP_ERROR_INVALID_PARAMETERS;

	return handle_obj->errcode;
}

char *CSFP_GetErrString(CSFP_HANDLE handle)
{
	CSFP_HANDLE_OBJ *handle_obj = (CSFP_HANDLE_OBJ *) handle;

	if ((NULL == handle_obj) || (handle_obj->obj_type != CSFP_OBJ_TYPE))
		return fpc_errstr[CSFP_ERROR_INVALID_HANDLE];

	return fpc_errstr[handle_obj->errcode];
}

