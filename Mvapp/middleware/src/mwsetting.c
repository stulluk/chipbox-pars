#include "linuxos.h"

#include "date_time.h"
#include "database.h"
#include "av_zapping.h"
#include "eit_engine.h"

#include "mwpublic.h"
#include "english.h"
#include "turkish.h"
#include "german.h"
#include "french.h"
#include "mwsetting.h"
#include "mwlayers.h"

typedef struct 
{
	unsigned char *name;
	unsigned char* biblio;
	unsigned char* term;
}ISO639_2;

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
typedef struct 
{
	U16            NameCode;
	unsigned char* biblio;
	unsigned char* term;
}ISO639_2_Name_t;

static ISO639_2 Languages[CSAPP_LANG_MAX_NUM]=
{
	{"eng", "eng", "ENG"},
	{"tur", "tur", "TUR"},
	{"ger", "ger", "GER"},
	{"fre", "fre", "FRE"}
};

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
static ISO639_2_Name_t LanguageString[LANG_MAX] =
{
	{CSAPP_STR_ENGLISH	, "eng", "eng"},
	{CSAPP_STR_TURKISH	, "tur", "tur"},
	{CSAPP_STR_GERMAN	, "ger", "deu"},
	{CSAPP_STR_FRANCE	, "fre", "fra"},
	{CSAPP_STR_GREEK	, "gre", "ell"},
	{CSAPP_STR_ARABIC	, "ara", "ara"},
	{CSAPP_STR_PERCIAN	, "per", "fas"}
};
	
static CSAPP_Lang_Type 		CurrentMenuLangId = CSAPP_LANG_ENGLISH;
static CSAPP_Lang_Type 		CurrentSubLangId  = CSAPP_LANG_ENGLISH;
static CSAPP_Lang_Type 		CurrentAudioLangId = CSAPP_LANG_ENGLISH;

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U16 MV_GetLanguageStringByCode(U8 *code)
{
	U32			i;

	for(i = 0; i < CSAPP_LANG_MAX_NUM; i++)
	{
		if((!memcmp(LanguageString[i].biblio, code, 3))||(!memcmp(LanguageString[i].term, code, 3)))
		{
			return LanguageString[i].NameCode;
		}
	}

	return CSAPP_STR_UNKNOWN;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U32 MV_GetLanguageTypeByCode(U8 *code)
{
	U32			i;

	for(i = 0; i < CSAPP_LANG_MAX_NUM; i++)
	{
		if((!memcmp(LanguageString[i].biblio, code, 3))|| (!memcmp(LanguageString[i].term, code, 3)))
		{
			return i;
		}
	}

	return CSAPP_LANG_MAX_NUM;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U16 MV_GetLanguageStringByType(U32 languageType)
{
	if (languageType < CSAPP_LANG_MAX_NUM)
	{
		return LanguageString[languageType].NameCode;
	}

	return CSAPP_STR_UNKNOWN;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
BOOL MV_CompareLanguageCodeByType(U8 *code)
{
	U32 languageType;

	languageType = CS_DBU_GetSubtitleLanguage();
	
	if (languageType < CSAPP_LANG_MAX_NUM)
	{
		if((!memcmp(LanguageString[languageType].biblio, code, 3))|| (!memcmp(LanguageString[languageType].term, code, 3)))
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CS_MW_ValidCurrentSetting(void)
{
	CS_MW_SetCurrentMenuLanguage(CS_MW_GetCurrentMenuLanguage());
	CS_MW_SetTimeZone(CS_MW_GetTimeZone());
	CS_MW_SetTxprc(CS_MW_GetTxprc());
	CS_MW_SetLcnMode(CS_MW_GetLcnMode());
	CS_MW_SetParentsRate(CS_MW_GetParentsRate());
	CS_MW_SetTimeRegion(CS_MW_GetTimeRegion());

	CS_MW_SetTimeMode(CS_MW_GetTimeMode());

	CS_MW_SetAspectRatio(CS_MW_GetAspectRatio());
	CS_MW_SetAspectMode(CS_MW_GetAspectMode());
	// CS_MW_SetVideoDefinition(CS_MW_GetVideoDefinition()); /* By KB Kim 2011.06.07 */

	CS_MW_SetDefaultSpdifAudioType(CS_MW_GetDefaultSpdifAudioType());

	CS_DBU_Set_Sleep(kCS_DBU_DEFAULT_Sleep);
	// CS_DBU_SaveUserSettingDataInHW(); /* By KB Kim 2011.06.07 */

	//CS_MW_SetVideoOutput(CS_MW_GetVideoOutput());

	//CS_MW_AudioSetVolume(CS_MW_AudioGetVolume());
	//CS_MW_Audio_SetMuteStatus(CS_MW_Audio_GetMuteStatus());

	return(TRUE);
}

void CS_MW_SetCurrentMenuLanguage(CSAPP_Lang_Type u32lang)
{
	if(CurrentMenuLangId != u32lang)
	{
		CurrentMenuLangId = u32lang;

		CS_DBU_SetMenuLanguage((U32)u32lang);

		CS_EIT_FreeDatabase();
	}
}

U32 CS_MW_GetCurrentMenuLanguage(void)
{
	U32 	u32lang;
        
	u32lang = CS_DBU_GetMenuLanguage();

	// printf("CS_MW_GetCurrentMenuLanguage[%d]\n", u32lang);
        
	return(u32lang);
}

void CS_MW_SetCurrentSubtitleLanguage(CSAPP_Lang_Type u32lang)
{
	if(CurrentSubLangId != u32lang)
	{
		CurrentSubLangId = u32lang;
		CS_DBU_SetSubtitleLanguage((U32)u32lang);
	}
}

U32 CS_MW_GetCurrentSubLanguage(void)
{
	U32 	u32lang;
        
	u32lang = CS_DBU_GetSubtitleLanguage();

	// printf("CS_DBU_GetSubtitleLanguage[%d]\n", u32lang);
        
	return(u32lang);
}

void CS_MW_SetCurrentAudioLanguage(CSAPP_Lang_Type u32lang)
{
	if(CurrentAudioLangId != u32lang)
	{
		CurrentAudioLangId = u32lang;
		CS_DBU_SetAudioLanguage((U32)u32lang);
	}
}

U32 CS_MW_GetCurrentAudioLanguage(void)
{
	U32 	u32lang;
        
	u32lang = CS_DBU_GetAudioLanguage();

	// printf("CS_DBU_GetSubtitleLanguage[%d]\n", u32lang);
        
	return(u32lang);
}

char* CS_MW_GetMenuLanguageName(CSAPP_Lang_Type   lang)
{
	return(Languages[lang].name);
}

U8* CS_MW_LoadStringByIdx(U32 StringIdx)
{
	U8*		pString=NULL;

	switch(CurrentMenuLangId)
	{
		case CSAPP_LANG_ENGLISH:
			pString=EngnishString[StringIdx];
			break;

		case CSAPP_LANG_TURKISH:
			pString=TurkishString[StringIdx];
			break;
			
		case CSAPP_LANG_GERMAN:
			pString=GermanString[StringIdx];
			break;

		case CSAPP_LANG_FRENCH:
			pString=FrenchString[StringIdx];
			break;
			
		default:		
			pString=EngnishString[StringIdx];
			break;
			
	}

	return pString;
}


char *CS_MW_LoadLanguageStringByIso(char *str)
{
	U16			i;

	for(i=0;i<CSAPP_LANG_MAX_NUM;i++)
	{
		if((!memcmp(Languages[i].biblio,str,3))||(!memcmp(Languages[i].term,str,3)))
		{
			return Languages[i].name;
		}
	}

	return str;
}

BOOL CS_MW_SetTimeZone(U16  offset_index)
{
	U32     				save_value = 0;
	tCS_DT_Time 			offset_time;
	tCS_DT_OffsetPolarity   offsetype;

	if(offset_index < 24)
	{
		offsetype = eCS_DT_OFFSET_NEGATIVE;
		offset_time.hour = 12-(offset_index+1)/2;
		offset_time.minute = (offset_index%2)*30;
	}
	else if(offset_index < CSAPP_TIME_ZONE_MAX_NUM)
	{
		offsetype = eCS_DT_OFFSET_POSITIVE;
		offset_time.hour = offset_index/2-12;
		offset_time.minute = (offset_index%2)*30;
	}
	else
		return(FALSE);
        
    save_value = CS_DT_HMtoUTC(offset_time);
	CS_DT_ManualSetUTCOffset(save_value, offsetype);
        
	if(offsetype == eCS_DT_OFFSET_NEGATIVE)
		save_value |= 1<<31;

	CS_DBU_SetTimeOffset(save_value);

	return(TRUE);
        
}

U16 CS_MW_GetTimeZone(void)
{
	U32     	get_value;
	tCS_DT_Time offset_time;
	U16     	temp;
        
	get_value = CS_DBU_GetTimeOffset();

	temp = (U16)(get_value&0xffff);
	offset_time = CS_DT_UTCtoHM(temp);

	if((get_value&0x80000000)!=0)/*eCS_DT_OFFSET_NEGATIVE*/
		temp = (12 - offset_time.hour)*2 - (offset_time.minute!=0?1:0);
	else/*eCS_DT_OFFSET_POSITIVE*/
		temp = (12 + offset_time.hour)*2 + (offset_time.minute!=0?1:0);

	return(temp);
}

BOOL CS_MW_SetTimeMode(U16  timemode_index)
{
	BOOL enbale = TRUE;
    
	switch(timemode_index)
	{
		case eCS_DBU_TIME_AUTOMATIC:
			enbale = TRUE;
			break;
		case eCS_DBU_TIME_MANUAL:
			enbale = FALSE;
			break;
		case eCS_DBU_TIME_INTERNET:
			enbale = FALSE;
			break;
		default:
			return(FALSE);
	}
	CS_DBU_SetTimeMode((tCS_DBU_TimeMode)timemode_index);
	//printf("\n SAVE OK : %d ============================ \n", timemode_index);
	CS_DT_EnableAutomaticTime(enbale);

	return(TRUE);
}

U16 CS_MW_GetTimeMode(void)
{
	U16 dbu;

	dbu = CS_DBU_GetTimeMode();

	return(dbu);
}


BOOL CS_MW_SetTxprc(U16  trans_index)
{
	CS_AV_SetOSDAlpha(trans_index);
	CS_DBU_SetScreenTransparent(trans_index);

	return(TRUE);
}

U16 CS_MW_GetTxprc(void)
{
	U16 dbu;

	dbu = CS_DBU_GetScreenTransparent();

	//return(dbu/20);
	return(dbu);
}

BOOL CS_MW_SetLNB_Power(U16  u16Lnb_Power)
{
	//CS_MW_SetVideoAspect( AspectRatio_index );
	CS_DBU_SetLNB_Power(u16Lnb_Power);

	return(TRUE);
}

BOOL CS_MW_SetAspectRatio(U16  AspectRatio_index)
{
	//CS_MW_SetVideoAspect( AspectRatio_index );
	CS_DBU_SetVideoAspectRatio((tCS_DBU_VideoAspectRatio)AspectRatio_index);

	return(TRUE);
}

U16 CS_MW_GetAspectRatio(void)
{
	U16 dbu;

	dbu = CS_DBU_GetVideoAspectRatio();

	return(dbu);
}

BOOL CS_MW_SetAspectMode(U16  AspectMode_index)
{
	CS_DBU_SetAspectRatioMode((tCS_DBU_AspectRatioMode)AspectMode_index);

	return(TRUE);
}

U16 CS_MW_GetAspectMode(void)
{
	U16 dbu;

	dbu = CS_DBU_GetAspectRatioMode();

	return(dbu);
}

BOOL CS_MW_SetVideoDefinition(U16  definition_index)
{
	/* 2010.09.02 by KB Kim for Setting bug from Auto to other */
	// printf("CS_MW_SetVideoDefinition : CS_DBU_SetVideoDefinition : definition_index = %d\n", definition_index);
	CS_DBU_SetVideoDefinition((tCS_DBU_VideoDefinition)definition_index);
	
	switch((tCS_DBU_VideoDefinition)definition_index)
	{
		case eCS_DBU_DEFINITION_480I:
                        // by KB Kim 2010.08.31 for AV Setting
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_NTSC);
		  	// CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_PAL);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_480I\n", definition_index);
			break;
		case eCS_DBU_DEFINITION_576I:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_PAL);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_576I\n", definition_index);
			break;
		case eCS_DBU_DEFINITION_576P:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_576P50);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_576P\n", definition_index);
			break;
#if 0
		case eCS_DBU_DEFINITION_720P:
			CS_AV_SetTVOutDefinition(tCS_AV_VideoDefinition definition)(eCS_AV_VIDEO_FORMAT_720P60);
		break;
		case eCS_DBU_DEFINITION_1080I:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_1080I30);
		break;
#else
		case eCS_DBU_DEFINITION_720P:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_720P50);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_720P\n", definition_index);
			break;
		case eCS_DBU_DEFINITION_1080I:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_1080I25);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_1080I\n", definition_index);
			break;
#endif
		case eCS_DBU_DEFINITION_AUTOMATIC:
                        // by KB Kim 2010.08.31 for AV Setting
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_AUTO);
			// printf("definition_index = %d : eCS_DBU_DEFINITION_AUTOMATIC\n", definition_index);
			break;

		default:
			CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_PAL);
			// printf("definition_index = %d : Unkown\n", definition_index);
			//CS_AV_SetTVOutDefinition(eCS_AV_VIDEO_FORMAT_1080I25);
			break;
	}

	return(TRUE);
}

U16 CS_MW_GetVideoDefinition(void)
{
	U16 dbu;

	dbu = CS_DBU_GetVideoDefinition();

	return(dbu);
}

BOOL CS_MW_SetVideoOutput(U16  output_index)
{
	CS_DBU_SetVideoOutput((tCS_DBU_VideoOutput)output_index);
		/* By KB Kim 2010.08.31 for RGB Control */
	CS_AV_SetTVOutput();
    
	return(TRUE);
}

U16 CS_MW_GetVideoOutput(void)
{
	U16 dbu;

	dbu = CS_DBU_GetVideoOutput();
	return(dbu);
}


BOOL CS_MW_SetLcnMode(U16  lcnmode_index)
{
	CS_DBU_SetLCNMode((tCS_DB_LCNMode)lcnmode_index);

	return(TRUE);
}

U16 CS_MW_GetLcnMode(void)
{
	U16 dbu;

	dbu = CS_DBU_GetLCNMode();

	return(dbu);
}

BOOL CS_MW_SetParentsRate(U16  age_rate)
{
	CS_DBU_SetParentalRate(age_rate);

	return(TRUE);
}

U16 CS_MW_GetParentsRate(void)
{
	U16 dbu;

	dbu = CS_DBU_GetParentalRate();

	return(dbu);
}

BOOL CS_MW_SetServicesLockStatus(BOOL  Status)
{
    
	if(Status)
		CS_DBU_SetServicesLockStatus(eCS_DBU_ON);
	else
		CS_DBU_SetServicesLockStatus(eCS_DBU_OFF);

	return(TRUE);
}

U16 CS_MW_GetServicesLockStatus(void)
{
	U16 dbu;

	dbu = CS_DBU_GetServicesLockStatus();

	return(dbu);
}


BOOL CS_MW_SetTimeRegion(U16  reg_index)
{
	CS_DBU_SetTimeRegion(reg_index);

	return(TRUE);
}

U16 CS_MW_GetTimeRegion(void)
{
	U16 dbu;

	dbu = CS_DBU_GetTimeRegion();

	return(dbu);
}

BOOL CS_MW_GetServiceIdxByLcn(U16 lcn,U16 *Svcidx,U16 *Itemidx)
{
	tCS_DB_ServiceManageData		ItemData;
	tCS_DB_Error					error;
	U16							svcmax,i;
	BOOL						Status;

	Status=FALSE;
	svcmax=CS_DB_GetCurrentList_ServiceNum();
	for(i=0;i<svcmax;i++)
	{
		error=CS_DB_GetCurrentList_ServiceData(&ItemData,i);
		//printf("=>>i=%d   item lcn=%d  lcn=%d\n",i,ItemData.LCN,lcn);
		if((error==eCS_DB_OK)&&(ItemData.LCN==lcn)) 
		{	
			*Svcidx=ItemData.Service_Index;
			*Itemidx=i;
			Status=TRUE;
			break;
		}
	}

	return Status;
}

BOOL CS_MW_GetServiceIdxByItemIdx(U16 idx,U16 *Svcidx)
{
	tCS_DB_ServiceManageData		ItemData;
	tCS_DB_Error					error;
	BOOL						Status;

	Status=FALSE;
	error=CS_DB_GetCurrentList_ServiceData(&ItemData,idx);
	if(error==eCS_DB_OK)
	{
		Status=TRUE;
		*Svcidx=ItemData.Service_Index;
	}

	return Status;
}

BOOL CS_MW_SetDefaultSpdifAudioType(U16  AudioType)
{
    
	CS_DBU_SetDefaultAudioType((tCS_DBU_AudioType)AudioType);

	return(TRUE);
}

U16 CS_MW_GetDefaultSpdifAudioType(void)
{
	U16 dbu;

	dbu = CS_DBU_GetDefaultAudioType();

	return(dbu);
}



