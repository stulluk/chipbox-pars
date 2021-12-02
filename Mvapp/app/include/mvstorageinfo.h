#ifndef  _MV_APP_STOREGE_INFO_H_
#define  _MV_APP_STOREGE_INFO_H_

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#define 	STORAGE_USAGE_Y		( MV_INSTALL_MENU_Y + ( MV_INSTALL_MENU_HEIGHT * 16 ))

typedef enum
{	
	CSAPP_SINFO_TOTAL=0,
	CSAPP_SINFO_USED,
	CSAPP_SINFO_UNUSED,	
	CSAPP_SINFO_TYPE,
	CSAPP_SINFO_VENDER,
	CSAPP_SINFO_PRODUCT,
	CSAPP_SINFO_SERIAL,
	CSAPP_SINFO_FORMAT,
	CSAPP_SINFO_ITEM_MAX
} eMV_Storageinfo_Items;

typedef enum
{	
	CSAPP_USB_TYPE=0,
	CSAPP_USB_VENDER,
	CSAPP_USB_PRODUCT,
	CSAPP_USB_SERIAL,
	CSAPP_USB_PROTOCOL,
	CSAPP_USB_TRANSPORT,
	CSAPP_USB_QUIRKS,
	CSAPP_USBINFO_MAX
} eMV_USB_FILE_Items;

CSAPP_Applet_t MVApp_Storage_Info(void);
int Storageinfo_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
void MV_Draw_SInfoMenuBar(HDC hdc, int esItem);
void MV_Draw_SInfo_MenuBar(HDC hdc);

#endif
/*   E O F  */

