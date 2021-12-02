#ifndef  _CS_APP_PVR_REC_H_
#define  _CS_APP_PVR_REC_H_

typedef struct
{
	BOOL					Draw; // 1:draw 0:clear
	BOOL					OnScreen;
} stPvrBanner;

CSAPP_Applet_t CSApp_PVR_Record(void);
CSAPP_Applet_t CSApp_PVR_Streaming(void);
#endif
/*   E O F  */



