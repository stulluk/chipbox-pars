#ifndef __MV_CFG_H
#define __MV_CFG_H

#include "linuxos.h"

#define	CFG_FILE		"/usr/work0/app/mv_app.cfg"
#define	TEMP_FILE		"/tmp/temp.cfg"
#define COLOR_FILE		"color.cfg"
#define CFG_MAX_COL		1024

typedef enum
{
	MVCFG_RESOUCE=0,					// Resource File Directory .. Directory full name
	MVCFG_SET_TYPE,						// DVB-C, T, S, Combo
	MVCFG_PNG_TEST,						// PNG File Test .. on/off 
	MVCFG_FOR_INHEO_TEST,				// Yinhe TEST --> Auto Display Menu rootine .. on/off
	MVCFG_LOAD_DEFAILT_VALUE,			// Auto Setting default value on Booting Time .. on/off
	MVCFG_NO_BACK_IMAGE,				// Background Image Use or Unuse .. on/off
	MVCFG_FAC_TEST,						// Factory Test mode .. on/off
	MVCFG_SKIN,
	MVCFG_MAX
}	MV_CFG;

typedef enum
{
	MV_COLOR_MAIN_BACK = 0,
	MV_COLOR_MENU_BACK_COLOR,
	MV_COLOR_INFO_TOP_BACK,
	MV_COLOR_INFO_BOT_BACK,
	MV_COLOR_CFG_MAX
} MV_COLOR_CFG;

typedef enum
{
	CFG_OK=0,
	CFG_NOFILE,
	CFG_READ_FAIL,
	CFG_NO_NAME
} MV_CFG_RETURN;

typedef struct
{
	U8	MV_R;
	U8 	MV_G;
	U8	MV_B;
	U8	MV_A;
} stMV_Color_RGB;

char			CFG_Resource[100];
U8				CFG_Set_Type;
BOOL			CFG_PNG_Test;
BOOL			CFG_Yinhe_Test;
BOOL			CFG_Default_Value;
BOOL			CFG_Noback_Image;
BOOL			CFG_Factory_Mode;
U8				CFG_Skin_Kind;
stMV_Color_RGB	CFG_Back_Color;
stMV_Color_RGB	CFG_Menu_Back_Color;
stMV_Color_RGB	CFG_Info_top_Color;
stMV_Color_RGB	CFG_Info_bot_Color;

char 	BOOT_LOGO[100];
char 	RADIO_BACK[100];

MV_CFG_RETURN MV_LoadCFGFile(void);
void MV_Parser_CFG_Value( char *Temp, char *tempSection );
MV_CFG MV_CFG_Namecheck(char *tempSection);
void Parser_Color_CFG(MV_COLOR_CFG Color_cfg, char *temp_str);
MV_CFG_RETURN MV_Resource_Change_Cfg_File(U8 u8Skin_kind);

#endif

