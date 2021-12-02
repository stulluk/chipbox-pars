#ifndef  _CS_APP_OSCAM_H_
#define  _CS_APP_OSCAM_H_

#include "casapi.h"

#define		MAX_URL_LANGTH		110
#define		OSCAM_NUM			MAX_SERVER_DATA
#define 	ID_LENGTH			SERVER_USER_ID_LENGTH
#define		PORT_LENGTH			6
#define		DES_KEY_LENGTH		28

typedef enum
{	
	CSAPP_OSCAM_ITEM_NUMBER=0,
	CSAPP_OSCAM_ITEM_MODE,
	CSAPP_OSCAM_ITEM_URL,
	CSAPP_OSCAM_ITEM_PORT1,
	CSAPP_OSCAM_ITEM_PORT2,
	CSAPP_OSCAM_ITEM_ID,
	CSAPP_OSCAM_ITEM_PW,
	CSAPP_OSCAM_ITEM_KEY1,
	CSAPP_OSCAM_ITEM_ENABLE,
#ifdef STATUS_MODE_ON  /* By KB Kim 2011.05.04 */
	CSAPP_OSCAM_ITEM_STATUS,
#endif
	CSAPP_OSCAM_ITEM_MAX
} eMV_OSCAM_Items;

typedef struct
{
	U8			u8Num;
	U8			u8Protocal;
	char		acURL[MAX_URL_LANGTH];
	U16			u16Port1;
	U16			u16Port2;
	char		acID[ID_LENGTH];
	char		acPW[ID_LENGTH];
	char		btKeyData[30];
	U8          enable; /* By KB Kim  : 2011.05.07 */
	char        statusIdx;  /* By KBKim 2011.04.23 */
	U8          u8delete;   /* By KBKim 2011.05.26 */
} stOSCAM_St;

stOSCAM_St	stOSCAM_Data[OSCAM_NUM];

CSAPP_Applet_t CSApp_OSCAM(void);
void MV_Draw_OSCAMMenuBar(HDC hdc, U8 u8Focuskind, eMV_CAS_Items esItem);
void MV_Draw_OSCAM_MenuBar(HDC hdc);

#endif
/*   E O F  */

