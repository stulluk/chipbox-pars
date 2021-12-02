#ifndef  _MV_APP_REC_FILE_H_
#define  _MV_APP_REC_FILE_H_

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cs_app_common.h"

typedef enum
{	
	CSAPP_REC_TOTAL=0,
	CSAPP_REC_USED,
	CSAPP_REC_UNUSED,	
	CSAPP_REC_TYPE,
	CSAPP_REC_VENDER,
	CSAPP_REC_PRODUCT,
	CSAPP_REC_SERIAL,
	CSAPP_REC_ITEM_MAX
} eMV_RecFile_Items;

#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
typedef enum
{	
	CSAPP_REC_TIMESHIFT=0,	
	CSAPP_REC_SHIFT_REC,
	CSAPP_REC_TS_TYPE,
	CSAPP_REC_JUMP,
	CSAPP_REC_CONF_ITEM_MAX
} eMV_RecConf_Items;
#endif

typedef enum
{	
	MVAPP_FILE_TS = 0,	
	MVAPP_FILE_MOVIE,
	MVAPP_FILE_MUSIC,
	MVAPP_FILE_PIC,
	MVAPP_FILE_TEXT,
	MVAPP_FILE_NORMAL,
	MVAPP_FILETYPE_MAX
} eMV_Filetype;

typedef struct
{
	U16		u16file_Count;
	char 	acFileName[2048][100];
	char	acFileExt[2048][10];
} stFile_db;

CSAPP_Applet_t CSApp_RecFile(void);
int RecFile_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
void MV_Draw_RecMenuBar(HDC hdc, int esItem, U8 u8Kind);
void MV_Draw_Rec_MenuBar(HDC hdc);

#ifdef RECORD_CONFG_SUPORT /* For record config remove by request : KB Kim 2012.02.06 */
CSAPP_Applet_t CSApp_RecConfig(void);
#endif

void MV_Draw_RecConfSelectBar(HDC hdc, int y_gap, eScanItemID esItem);
void MV_Draw_RecConfMenuBar(HDC hdc, U8 u8Focuskind, eScanItemID esItem);
void MV_Draw_RecConfMenuFull(HDC hdc);

CSAPP_Applet_t CSApp_FileTools(void);
int FileTool_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
void MV_Draw_FileToolMenuBar(HDC hdc, int esItem, U8 u8Kind);
void MV_Draw_FileTool_MenuBar(HDC hdc);
eMV_Filetype MV_Check_Filetype(char *Extention);
U8 Check_Picture_Kind(char *Extention);

CSAPP_Applet_t CSApp_MPlayer(void);
int MPlayer_Msg_cb(HWND hwnd,int message,WPARAM wparam,LPARAM lparam);
void MV_Draw_MPlayer_MenuBar(HDC hdc);
void MV_Draw_MPlayerMenuBar(HDC hdc, int esItem, U8 u8Kind);
#endif
/*   E O F  */

