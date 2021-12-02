#ifndef  _CS_APP_DESKTOP_H_
#define  _CS_APP_DESKTOP_H_

#define	RECALL_SINGLE	0
#define RECALL_MULTI	1

#define PROGRAM_MAX_NUM		4

typedef enum
{
	CS_BANNER_TV=0,
	CS_BANNER_RD,
	CS_BANNER_FAV
}eCSBNRServiceType;

typedef enum
{
	CS_SIGNAL_UNLOCK=0,
	CS_NO_SERVICE,
	CS_SERVICE_ENCRYPT,	
	CS_SERVICE_LOCK,	
}eCSDesktopClew;

typedef struct
{
	U16                 CH_Num;
	U16					u16Index;
    char				Name[20];
    BOOL				Update;	
}tCSBNRServiceName;

typedef struct
{
	BOOL					HD; // 1:is hd
	BOOL					Update;
}tCSBNRHD;

typedef struct
{
	BOOL					H264; // 1:is h264
	BOOL					Update;
}tCSBNRH264;

typedef struct
{
	BOOL					MPEG; // 1:is h264
	BOOL					Update;
}tCSBNRMPEG;

typedef struct
{
	BOOL					AC3; // 1: is ac3
	BOOL					Update;
}tCSBNRAC3;

typedef struct
{
	BOOL					TTX; // 1:have ttx
	BOOL					Update;
}tCSBNRTTX;

typedef struct
{
	BOOL					SUB; // 1:have subtitle
	BOOL					Update;
}tCSBNRSub;

typedef struct
{
	BOOL					FAV; // 1:is fav
	BOOL					Update;
}tCSBNRFav;

typedef struct
{
	BOOL					LOCK; // 1:is lock
	BOOL					Update;
}tCSBNRLock;

typedef struct
{
	BOOL					SCRAMBLE; // 1:is Scramble
	BOOL					Update;
}tCSBNRScramble;

typedef struct
{
	BOOL					EPG; // 1:is have epg
	BOOL					Update;
}tCSBNREPG;

typedef struct
{
	tCSBNRHD		HD;
	tCSBNRH264		H264;//for 1200
	tCSBNRAC3		AC3;
	tCSBNRMPEG		MEPG;
	tCSBNRTTX		TTX;
	tCSBNRSub		Sub;
	tCSBNRFav		Fav;
	tCSBNRLock		Lock;
	tCSBNREPG		EPG;
	tCSBNRScramble	SCRAMBLE;
}tCSBNRSerivceAttr;

typedef struct
{
	U8				Percent;
	U8				Text[8];
	BOOL			Update;
}tCSBNRProgress;

typedef struct
{
	U16				StarMjd;
	U16				StartTime;
	U16				Duration;
	U8				Name[64];
	U8				Text[64];
	U8				RateText[6];
	BOOL			Update;
}tCSBNREventInfo;


typedef struct
{
	tCSBNRServiceName		ServiceName;
	tCSBNRSerivceAttr		ServiceAttribute;
	tCSBNRProgress			Progress;//for 1200				
	tCSBNREventInfo			Present;
	tCSBNREventInfo			Follow;
	BOOL					DrawBMP;
}tCSBNRInfo;

typedef struct
{
	tCSBNRInfo				BannerInfo;
	BOOL					Draw; // 1:draw 0:clear
	BOOL					OnScreen;
}tCSDesktopBanner;

typedef struct
{
	char					Volume;
	U8						Text[12];
	BOOL					Draw; // 1:draw 0:clear
	BOOL					OnScreen;
	
}tCSDesktopVolume;

typedef struct
{
	BOOL					Mute; // 1:mute 0 unmute
	U8						Text[12];
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopMute;

typedef struct
{
	BOOL					VMode;
	U8						u8VMode; // 1:mute 0 unmute
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopVMode;

typedef struct
{
	BOOL					Pause; // 1:mute 0 unmute
	//U8						Text[12];
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopPause;

typedef struct
{
	U16						Input;
	U8						Cursor;
	U8						Text[8];
	BOOL					Active;
	BOOL					CreateTimer;
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopInput;

typedef struct
{
	eCSDesktopClew			Warning;
	U8						Text[32];
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopWarning;

typedef struct
{
	U16						Pid;
	BOOL					Open;
	BOOL					Sub;
	BOOL					Draw;
}tCSDesktopSubtitle;

typedef struct
{
	U8						Text[10];
	U8						Input[5];
	U8						Cursor;
	BOOL					Lock;
	BOOL					Draw; // 1:draw 0 clear
}tCSDesktopPin;

typedef struct
{
	MV_stServiceInfo	ServiceData;
	tCSDesktopBanner	Banner;
	tCSDesktopVolume	Volume;
	tCSDesktopMute		Mute;
	tCSDesktopVMode		VMode;
	//tCSDesktopPause	Pause;
	BOOL				Pause;
	tCSDesktopInput		Input;
	tCSDesktopWarning	Warning;	
	tCSDesktopPin		ParentPin;
	tCSDesktopSubtitle	Subtitle;
	U16					Current_Service;
	U16					Service_Index;
	U16					TPIndex;
	BOOL				LockStatus;
}tCSDesktopInfo;

CSAPP_Applet_t	CSApp_Desktop(void);

#endif
/*   E O F  */





