/*****************************************************************************

File name   : panel.h

Description : Front Panel Driver API


History:
-----------------------------------------------------------------------------
14/09/07	Created 			by 	Simon
24/12/07	Updated 			by 	Simon
			Add support DSD418 Front Panel
			Because IC CH450 is not a full I2C Compatible device. Sti2c is not suit for it,
			So we use two other GPIOs as Serial bus
			
18/01/08	Updated 			by 	Simon
			Add support Front Panel of DSD418 CAD DV
*****************************************************************************/

#ifndef _PANEL_H_
#define _PANEL_H_


#ifdef __cplusplus
extern "C" {
#endif
/*
#ifndef BOOL
#define BOOL unsigned char
#endif*/

#define _IN_

typedef enum _PanelT_LedColor
{
	PANELT_LED_DARK, 
	PANELT_LED_RED,
	PANELT_LED_GREEN,
	PANELT_LED_YELLOW
}PanelT_LedColor;

typedef enum _PanelT_KeyEvent
{
	PANELT_KEY_DOWN,
	PANELT_KEY_UP,
	PANELT_KEY_REPEAT
}PanelT_KeyEvent;

typedef enum _ERR_CODE
{
	OK=0,
	FAIL
}ERR_CODE;


typedef struct _PanelT_InitParams
{
	unsigned char bTaskPriority;
	void (*PanelKeyProc)(PanelT_KeyEvent iKeyEvent, unsigned char bKeyCode);
}PanelT_InitParams;

ERR_CODE PanelF_Initialize(_IN_ PanelT_InitParams *pInitParams);

ERR_CODE  PanelF_DisplayString(_IN_ const char *pString);

ERR_CODE  PanelF_DisplayTime(_IN_ const unsigned char bHour, _IN_ const unsigned char bMin);

ERR_CODE  PanelF_SetBiColorLED(_IN_ PanelT_LedColor iLedColor);

const char*   PanelF_GetRevision( void );


#ifdef __cplusplus
}
#endif


#endif



