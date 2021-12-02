#ifndef CS_EPG_MENU_H
#define CS_EPG_MENU_H

#define		ITEM_TEXT_LENGTH				21

/*
typedef enum
{
	eCS_APP_EPG_FOCUS_CHANNEL,
	eCS_APP_EPG_FOCUS_EVENT
} eCS_APP_EPG_Focus_Mode_t;
*/

#ifdef DAILY_EPG
CSAPP_Applet_t CSApp_Daily_Epg(CSAPP_Applet_t   slist_type);
#endif
CSAPP_Applet_t CSApp_Epg(CSAPP_Applet_t   slist_type);
BOOL AddEpgChannel(U16 channelIndex, U16 serviceId);

#endif

