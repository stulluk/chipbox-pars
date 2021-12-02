#ifndef  _CS_LANG_SETTING_H_
#define  _CS_LANG_SETTING_H_

typedef enum
{
	LANG_OSD = 0,
	LANG_SUBTITLE,
	LANG_AUDIO,
	LANG_SETTING_MAX
} eMV_LangSetting_Items;

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
#if 0
typedef enum
{
	LANG_ENGLISH = 0,
	LANG_TURKISH,
	LANG_GERMAN,
	LANG_FRANCE,
	LANG_GREEK,
	LANG_ARABIC,
	LANG_PERCIAN,
	LANG_MAX
} eMV_Lang_Items;
#endif

enum
{
	CS_APP_ENGLISH = 0,
	CS_APP_TURCK,
	CS_APP_GERMAN,
	CS_APP_FRENCH,
	CS_APP_LANG_NUM_MAX
};

CSAPP_Applet_t	CSApp_LangSetting(void);

#endif
