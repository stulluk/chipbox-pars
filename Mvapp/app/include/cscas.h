#ifndef  _CS_APP_CAS_H_
#define  _CS_APP_CAS_H_

typedef enum
{	
	CSAPP_CAS_ITEM_CARD_STATUS=0,
	CSAPP_CAS_ITEM_CARD,
	CSAPP_CAS_ITEM_CARD_NAME,
	CSAPP_CAS_ITEM_MAX
} eMV_CAS_Items;

typedef enum
{	
	CSAPP_CAS_EMPTY_SLOT=0,
	CSAPP_CAS_INSERT_CARD,
	CSAPP_CAS_CARD_STATUS_MAX
} eMV_Slot_Status;

CSAPP_Applet_t CSApp_CAS(void);
void MV_Draw_CASMenuBar(HDC hdc, U8 u8Focuskind, eMV_CAS_Items esItem);
void MV_Draw_CAS_MenuBar(HDC hdc);

#endif
/*   E O F  */

