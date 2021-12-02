#ifndef  _CS_APP_PIN_SETTING_H_
#define  _CS_APP_PIN_SETTING_H_

#define     kPIN_key_buffer_size        6

typedef enum
{	
	CSAPP_PIN_ENABLE = 0,
	CSAPP_PIN_BOOT,
	CSAPP_PIN_MENU,
	CSAPP_PIN_EDIT,
	CSAPP_PIN_CHANNEL,
	CASPP_PIN_CHANGE,
	CSAPP_PIN_NOW,
	CSAPP_PIN_NEW,
	CSAPP_PIN_COMFIRM,
	CSAPP_PIN_SETTING_ITEM_MAX
} eMV_PinSetting_Items;

typedef struct
{	
    char		input_keys[kPIN_key_buffer_size];
    U8			input_count;    
}tPIN_keys_t;

enum
{
	CS_APP_DISABLE_PIN = 0,
	CS_APP_ENABLE_PIN,
    CS_APP_ENABLE_PIN_NUM,
};

CSAPP_Applet_t CSApp_PinSetting(void);

#endif

