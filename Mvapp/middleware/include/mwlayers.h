/****************************************************************************
 * Copyright (c) 2007 Celestial Corporation  All Rights Reserved.
 *
 * Module:      mwlayers.h
 *
 * Authors:     River<jin.jiang@celestialsemi.com>
 *
 * Ver:         1.0(06.17.2007)
 *
 * Description: The Public Header of middleware porting layers.
 *
 * Notes:
 *
 ***************************************************************************/

#ifndef _CS_MW_LAYERS_H_
#define _CS_MW_LAYERS_H_


#ifdef __cplusplus
extern "C"
{
#endif/*__cplusplus*/

#include "date_time.h"

typedef enum
{
	EN_FONT_TYPE8_TTF_1 = 0,
	EN_FONT_TYPE8_TTF_2,
	EN_FONT_TYPE8_TTF_3,
	EN_FONT_TYPE8_TTF_4,
	EN_FONT_TYPE8_FIXED,
	EN_FONT_TYPE8_TTF_MAX
}EN_FONT_TYPE;

typedef enum
{
	EN_FONT_TYPE8_22 = 0,
	EN_FONT_TYPE8_24,
	EN_FONT_TYPE8_26,
	EN_FONT_TYPE8_28,
	EN_FONT_TYPE8_30,
	EN_FONT_TYPE8_32,
	EN_FONT_TYPE8_MAX
}EN_FONT_SIZE;

#ifdef USE_CHINESE_FONT
typedef enum
{
	EN_FONT_TYPE16_FIXED_12 = 0,
	EN_FONT_TYPE16_FIXED_16,
	EN_FONT_TYPE16_FIXED_24,
	EN_FONT_TYPE16_FIXED_30,
	EN_FONT_TYPE16_MAX
}EN_FONT_TYPE16;
#endif

/* For Front Communication By KB Kim 2011.02.01 */
typedef enum
{
	BOOT_NORMAL = 0,
	BOOT_AC_POWER_ON,
	BOOT_TIMER,
	BOOT_WATCHDOG,
	BOOT_MODE_UNKNOWN
}SystemBootMede;

extern void AdjustVideoWindows(void);
extern int IAL_GetKeyMap(int *keymap_tab, int *keymap_num);
extern int IAL_SetKeyMap(int *keymap_tab, int keymap_num);

/* For 576i Mode booting problem by KB Kim 20101225 */
void CS_MW_Init_VideoDefinition(U8 mode);

/* For AC Power ON Control By KB Kim 2011.03.11 */
U8 GetPowerStatus(void);
void SetPowerStatus(U8 val);

BOOL CS_MW_Init(void);
BOOL CS_MW_Open(void);
BOOL CS_MW_Close(void);
BOOL CS_MW_Term(void);

void CS_MW_TextOut( HDC hdc, U32 x, U32 y, char* text );
void CS_MW_TextOut_Static( HDC hdc, U32 x, U32 y, char* text );
void CS_MW_DrawText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawBigText( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawText_Fixed( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawText_Static( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawText_Fixed_Small( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
void MV_MW_DrawText_Title( HDC hdc, const char* text, int nCount, RECT* pRect, UINT nFormat );
BOOL CS_MW_Font_Creation(BOOL Start_Time);

// BOOL CS_MW_FrontPanelDisplay( char* str );
BOOL FbSendFndDisplay(char *data);
BOOL FbSendFndDisplayNum(U32 num);
BOOL FbRquestBootMode(void);
BOOL FbRequestStandBy(U8 displayOn);   // False Power Off
BOOL FbRequestShoutDowm(U8 displayOn); // Real Power Off

/* For Front Register Power Key By KB Kim 2011.01.30 */
BOOL FbSetRcuPowerKey(U8 num, U8 *keyData);
BOOL FbRegistPoweKey(void);

BOOL FbRequestNormalMode(void);
BOOL FbSetTime(void);
BOOL FbSetTimer(U16 date, U16 time);
BOOL FbSetTestTimer(void);
BOOL FbSetFrontDisplay(U8 level);
BOOL FbSetFrontClock(U8 on);
BOOL FbSendBootEnd(void);
BOOL FbStartWatchdog(U8 time);
BOOL FbStopWatchdog(void);
BOOL FbSendKick(void);

/* For Front Communication By KB Kim 2011.02.01 */
U8 FbGetBootMode(void);

BOOL CS_MW_SetKeyMap( U16 KeyIndex, U8 KeyValue );
BOOL CS_MW_USBDeviceConnect(void);

BOOL CS_MW_USBDeviceFWUpdate(void);


#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif/*_CS_MW_LAYERS_H_*/


