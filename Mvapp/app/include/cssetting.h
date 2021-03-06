#ifndef  _CS_APP_SYSTEM_SETTING_H_
#define  _CS_APP_SYSTEM_SETTING_H_

typedef enum
{
#ifdef	USE_LNB
	CSAPP_SETTING_LNB = 0,
	CSAPP_SETTING_INFO,
#else
	CSAPP_SETTING_INFO = 0,
#endif
	CSAPP_SETTING_TRANS,
	CSAPP_SETTING_CH_CHANGE,
	CSAPP_SETTING_CH_LIST_TYPE,
	CSAPP_SETTING_RECALL,
	CSAPP_SETTING_POWER,
	CSAPP_SETTING_LED,
	CSAPP_SETTING_ANI,
	CSAPP_SETTING_HEARTBIT, /* For Heart bit control By KB Kim 2011.03.11 */
	CSAPP_SETTING_FONT,
	CSAPP_SETTING_FONT_SIZE,
	CSAPP_SETTING_SKIN,
	CSAPP_SETTING_AUTO_SUBT,
	CSAPP_SETTING_ITEM_MAX
} eMV_SYSSetting_Items;

#ifdef	USE_LNB
enum
{
	CS_APP_LNB_P_ON = 0,
	CS_APP_LNB_P_OFF,
	CS_APP_LNB_P_NUM
};
#endif

enum
{
	CS_APP_CH_CHANGE_BALCK = 0,
	CS_APP_CH_CHANGE_PAUSE,
	CS_APP_CH_CHANGE_NUM
};

enum
{
	CS_APP_LIST_NORMAL = 0,
	CS_APP_LIST_EXTEND,
	CS_APP_LIST_NUM
};

enum
{
	CS_APP_RECALL_ONE = 0,
	CS_APP_RECALL_MULTI,
	CS_APP_RECALL_NUM
};

enum
{
	CS_APP_POWER_SLEEP = 0,
	CS_APP_POWER_REAL,
	CS_APP_POWER_NUM
};

enum
{
	CS_APP_LED_OFF = 0,
	CS_APP_LED_ON,
	CS_APP_LED_NUM
};

#define	CS_APP_BANNER_TIME_NUM		10
#define CA_APP_BANNET_TIME_NIM		0

CSAPP_Applet_t CSApp_Setting(void);
CSAPP_Applet_t CSApp_Fav_Edit(void);

#endif
/*   E O F  */

