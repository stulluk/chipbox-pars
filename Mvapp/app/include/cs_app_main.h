#ifndef  _CS_APP_MAIN_H_
#define  _CS_APP_MAIN_H_

#include "userdefine.h"
#include "csaudio.h"
#include "csavsetting.h"
#include "csdesktop.h"
#include "mv_pvr_record.h"
#include "mv_pvr_play.h"
#include "cseditslist.h"
#include "csepg.h"
#include "csinstall.h"
#include "cspinsetting.h"
#include "csreset.h"
#include "csbackup.h"
#include "cssetting.h"
#include "csslist.h"
#include "cs_ext_slist.h"
#include "cssubtitle.h"
#include "cssysteminfo.h"
#include "csteletext.h"
#include "mainmenu.h"
#include "cstimesetting.h"
#include "cssleep.h"
#include "cslangsetting.h"
#include "csnetsetting.h"
#include "mvstorageinfo.h"
#include "mvrecfile.h"
#include "cscas.h"
#include "cscimmi.h"
#include "csextinfo.h"
#include "mvtimer.h"
#include "push.h"

#ifdef SMART_PHONE
#include "mv_new_menu.h"
#endif

#ifdef Screen_1080
#define  CSAPP_OSD_MAX_WIDTH		1920
#define  CSAPP_OSD_MAX_HEIGHT		1080
#else
#define  CSAPP_OSD_MAX_WIDTH		1280
#define  CSAPP_OSD_MAX_HEIGHT		720
#endif

enum
{
	CSAPP_SUCCESS=0,
	CSAPP_ERROR=1,
};

void MV_Set_CurrentGuiApplet(BOOL SetType);
void MV_Set_BootMode(SystemBootMede Bootmode);
U8 MV_Get_BootMode(void);

#endif
/*   E O F  */
