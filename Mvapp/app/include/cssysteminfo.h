#ifndef  _CS_APP_SYSTEM_INFO_H_
#define  _CS_APP_SYSTEM_INFO_H_

typedef enum
{	
	CSAPP_INFO_TYPE=0,
	CSAPP_INFO_HW,
	CSAPP_INFO_BOOT,
	CSAPP_INFO_KERNEL,
	CSAPP_INFO_ROOTFS,
	CSAPP_INFO_MAIN,
	CSAPP_INFO_DEF_DATA,
	CSAPP_INFO_DEF_DB,
	CSAPP_INFO_VBG,
	CSAPP_INFO_ABG,
	CSAPP_INFO_ITEM_MAX
} eMV_Sysinfo_Items;

CSAPP_Applet_t CSApp_Sysinfo(void);
void MV_Draw_InfoMenuBar(HDC hdc, U8 u8Focuskind, eMV_Sysinfo_Items esItem);
void MV_Draw_Info_MenuBar(HDC hdc);

#endif
/*   E O F  */

