#ifndef CS_MIDDWARE_SVC_H
#define CS_MIDDWARE_SVC_H

#include "linuxos.h"
#include "db_builder.h"

#define	MW_ES_STREAM_MAX	15


typedef enum
{
	MW_SVC_MP2 = 0,
	MW_SVC_AC3,	
	MW_SVC_AAC,         /* Add By River 06.12.2008 */
	MW_SVC_LATM,        /* Add By River 06.12.2008 */
	MW_SVC_TTX_SUBTITLE,
	MW_SVC_DVB_SUBTITLE,
	MW_SVC_TELETEXT,
	MW_SVC_UNKOWN,
	MW_SVC_MAX_STREAMS
} eMWStreamType;

typedef enum
{
	UPDATE_PMT = 0,
	UPDATE_FE,
	UPDATE_TTX,
	UPDATE_SUB,
	UPDATE_HD,
} eMWNotifyType;

typedef enum
{
	FE_LOCK = 0,
	FE_UNLOCK,
	FE_LOST,
} eMWFEStatus;

typedef struct
{
	U16				CompositionPageID;
	U16				AncillaryPageID;
} tMWSubtitle;

typedef struct
{
	U8				MagazineID;
	U8				PageID;
} tMWTeletext;

typedef struct
{
	eMWStreamType	Type;
	U8				Language[kCS_SI_TRIGRAM_MAX_LENGTH+1];
	U16				Pid;
	union
	{
		tMWSubtitle	Subtitle;
		tMWTeletext	Teletext;
	} uDescriptor;
} tMWStreamInfo;

typedef struct
{	
	U8				Number;
	tMWStreamInfo	Stream[MW_ES_STREAM_MAX];
} tMWStream;

typedef struct
{
	eMWNotifyType	type;
	union
	{
		U16			ServiceID;
		eMWFEStatus	FEStatus;	
	} uData;
} tMWNotifyData;

typedef struct
{
	BOOL			Teletext;
	BOOL			Subtitle;
	BOOL			AC3;
} tMWStreamStatus;

typedef struct
{
	U16				ServiceID;
	U16				PMT_PID;
} tMWPATProgram;

typedef struct
{
	U8				Number;
	tMWPATProgram	Programs[kCS_SI_MAX_NO_OF_SERVICE_PER_TS];
} tMWPATInfo;

typedef void (*MW_SVCNotify)(tMWNotifyData NotifyData);

#ifdef SUPPORT_CI


#define             MSG_PARAM_CI_Clear_Display		1
#define             MSG_PARAM_CI_Display_Enquiry	2
#define             MSG_PARAM_CI_Display_List		3
#define             MSG_PARAM_CI_Display_Menu		4
#define             MSG_PARAM_CI_Insert_Notify		5
#define             MSG_PARAM_CI_Cas_Info_Arrived	6
#define             MSG_PARAM_CI_App_Info_Changed	7
#define             MSG_PARAM_CI_MMI_Broken			8


#define             MAX_CAPMT_SIZE					4096

typedef struct 
{
	unsigned char	* text;
	unsigned char	length;
	unsigned char	blind_answer;
	unsigned char	expected_answer_length;
} tMW_CI_Enquiry_info;
#endif

U8 IsNewProgram(void);

BOOL CS_MW_SVC_Init(void);
BOOL CS_MW_SVC_Open(MW_SVCNotify SVCNotify);
BOOL CS_MW_SVC_Close(void);
BOOL CS_MW_SVC_Term(void);

void CS_MW_GetAudioStream(tMWStream *pStream);
void CS_MW_GetTeletextStream(tMWStream *pStream);
void CS_MW_GetSubtitleStream(tMWStream *pStream);

#ifdef SUPPORT_CI

BOOL MW_CI_MMI_Get_App_Info_Status(void);
#endif

U16 CS_MW_GetCurrentPlayProgram(void);
BOOL CS_MW_PlayServiceByIdx(U16 Index, U8 RetuneKind);
BOOL CS_MW_StopService(BOOL blank);
BOOL MV_MW_StopService(void);

BOOL CS_MW_SetSmallWindow(U16 X,U16 Y,U16 W,U16 H);
BOOL CS_MW_SetNormalWindow(void);
void CS_MW_SwitchVideoFreeze(void);
void CS_MW_SwitchVideoUnFreeze(void); /* By KB Kim 2011.06.02 */
void MvRePlayVideo(void); /* By KB Kim 2011.06.02 */

BOOL CS_MW_TTXSUBInit(void);
U16 CS_MW_GetSubtitlePid(void);
void CS_MW_SetSubtitlePid(U16 Pid);

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
void MvClearCurrentSubtitle(void);
void MvSetCurrentSubtitle(U8 current);
U8 MvGetCurrentSubtitle(void);
U8 CS_MW_Get_Subtitle_Status(void);
U8 MvGetTotalSubtitleNumber(void);
void MvSetSubtitleMode(U8 mode);
U8 MvGetSubtitleMode(void);
void CS_MW_OpenSubtitle(void);

CSOS_MessageQueue_t* GetTTXpMsgQ(void);
void CS_MW_CloseSubtitle(void);
BOOL MV_MW_StartService(U16 u16Chindex);
BOOL MV_Get_b8PlayService(void);
void CS_MW_SetSubtitleLang(U8 *Lang,U8 Length);
U8 MW_CI_Get_Cam_Information(U8 *infoData);
void MW_FE_EIT_Start(void);

#endif

