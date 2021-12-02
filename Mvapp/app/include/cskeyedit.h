#ifndef  _CS_APP_KEYEDIT_H_
#define  _CS_APP_KEYEDIT_H_

#include "casapi.h"

#define	PROVIDER_ITEM_MAX		7
#define	KEY_ITEM_MAX			14
#define KEY_DATA_ARRAY_SIZE		32
#define	KEY_DATA_ARRAY_LENGTH	8
#define KEY_DATA_MAX_LENGTH		16

#define WINDOW_X_GAP			10

#define TOP_WINDOW_Y			130
#define TOP_WINDOW_DY			30

#define	CAS_WINDOW_X			130
#define	CAS_WINDOW_Y			170
#define	CAS_WINDOW_DX			200
#define	CAS_WINDOW_DY			420

#define PROVIDER_WINDOW_X		(CAS_WINDOW_X + CAS_WINDOW_DX + WINDOW_X_GAP)
#define PROVIDER_WINDOW_Y		CAS_WINDOW_Y
#define PROVIDER_WINDOW_DX		230
#define	PROVIDER_WINDOW_DY		CAS_WINDOW_DY
#define PROVIDER_ITEM_DY		60

#define KEY_WINDOW_X			(PROVIDER_WINDOW_X + PROVIDER_WINDOW_DX + WINDOW_X_GAP + 20)
#define KEY_WINDOW_Y			CAS_WINDOW_Y
#define KEY_WINDOW_DX			550
#define	KEY_WINDOW_DY			CAS_WINDOW_DY

U8			u8NumberOfCas;				/* Count of ALL CAS */
U8			u8NumberOfProvider;			/* Count of Provider in each CAS */
U8			u8NumberOfKey;				/* Count of Key Data in each Provider */

typedef enum
{	
	CSAPP_KEYEDIT_ITEM_CARD_STATUS=0,
	CSAPP_KEYEDIT_ITEM_CARD,
	CSAPP_KEYEDIT_ITEM_MAX
} eMV_KeyEdit_Items;

typedef enum
{
	KEYEDIT_CAS_WINDOW = 0,
	KEYEDIT_PROVIDER_WINDOW,
	KEYEDIT_KEY_WINDOW,
	KEYEDIT_MAX
} eMV_KeyFocus_Status;

typedef enum
{
	KEYEDIT_EDIT = 0,
	KEYEDIT_ADD,
	KEYEDIT_ADDOREDIT_MAX
} eMV_KeyEdit_Status;

typedef struct KeyTempData_s
{
	U8			KeyNumber;
	U8			KeyLength;
	U8			KeyData[KEY_DATA_ARRAY_SIZE];
} KeyTempData_t;

CSAPP_Applet_t CSApp_KeyEdit(void);
void MV_Draw_KeyEditMenuBar(HDC hdc, U8 u8Focuskind, eMV_KeyEdit_Items esItem);
void MV_Draw_KeyEdit_MenuBar(HDC hdc);

#endif
/*   E O F  */

