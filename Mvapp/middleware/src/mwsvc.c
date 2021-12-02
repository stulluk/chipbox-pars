#include "linuxos.h"


#include "eit_engine.h"
#include "database.h"
#include "db_builder.h"

/* 2010.09.04 By KB KIm For New SI */
// #include "demux.h"
/* Added for CI TS Control by KB Kim 20110107 */
#include "mvosapi.h"
#include "mvmiscapi.h"

#include "tableApi.h"

#include "subtitle.h"
#include "av_zapping.h"
#include "csmpr_recorder.h"
#include "csmpr_player.h"

#ifdef SUPPORT_CI
#include "ci_api.h"
#include "userdefine.h"
#endif

#include "mwpublic.h"
#include "fe_mngr.h"
#include "sys_setup.h"
#include "csttx.h"
#include "csttxdraw.h"
#include "mwsvc.h"
#include "mwlayers.h"
#include "mvosapi.h"
#include "mv_cfg.h"

// #define MW_SVC_printf					printf
#define MW_SVC_printf(fmt, ... )  {;}

static U16						LastTPIdx = 0xFFFF;  /* by kb : 201004060 */
static U16                      LastServixeIdx = 0xFFFF;
static U8						FEClientId;
static BOOL						OpenSVC = FALSE;
static BOOL						b8PlayService = FALSE;
static MV_stServiceInfo			mv_service_data;
static MW_SVCNotify				SvcCallBack = NULL;
static tMWStream				AudioStream;
static tMWStream				TeletextStream;
static tMWStream				SubtitleStream;
static U16						CurrentSubPid = 0xFFFF;
static U8						CurrentSubLang[kCS_SI_TRIGRAM_MAX_LENGTH+1];
BOOL							CS_MW_IsVideoFreezed = FALSE;
static	U8						EITClientID;
static  U8                      VideoReplayOn = 0;  /* by kb : 201004060 For No signal -> Signal On */

/* For channel change problem */
#define							MSG_NOW_NEXT_UPDATE				MSG_USER+9
#define							MSG_TTX_DISPLAY					MSG_USER+20

#define                         SUBTITLE_OPEN_NONE 				0
#define                         SUBTITLE_OPEN_EBU				1
#define                         SUBTITLE_OPEN_DVB				2
#define                         PMT_CA_DESCRIPTOR				0x09

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
#define                        INVALID_SUBTITLE_NUMBER         0xFF
U8 CurrentSubtitleNumber = INVALID_SUBTITLE_NUMBER;
U8 SubtitleModeOn        = 0;
BOOL MV_CompareLanguageCodeByType(U8 *code);

eMWStreamType SiDbAudioType[AUDIO_TYPE_MAX] =
{
	MW_SVC_MP2,
	MW_SVC_AC3,
	MW_SVC_AAC,
	MW_SVC_LATM,
	MW_SVC_MP2
};

/* For Cas By Jacob 10Feb.2011 */
extern BOOL CasDrvStopChannel (U8 sourceId);
extern BOOL CasDrvStartNewChannel  (U8 sourceId, U16 dmxId, U16 channelId, U16 serviceId, U16 vPid, U16 aPid);
extern BOOL CasDrvNotifyPMT(U8 sourceId, U8 *PmtData);

#if 1
static void EitSectionNotify(U16  service_id, tCS_EIT_Type eit_type)
{
	tCS_EIT_Type Temp_warning;

	Temp_warning = eit_type;
	/*
	if(service_id>=0x1FFF)
		return;
		*/

	if ((eit_type == eCS_EIT_PRESENT) || (eit_type == eCS_EIT_FOLLOWING))
	{
		BroadcastMessage (MSG_NOW_NEXT_UPDATE, service_id, 0);
	}

	return;
}
#endif

#ifdef SUPPORT_CI

//static  BOOL			Cas_Info_isvalid = FALSE;
static  BOOL			App_Info_isvalid = FALSE;
static  BOOL			FirstCaPmt       = TRUE;
static  U8              CiCamStatus = 0xFF;
/* Added for CI TS Control by KB Kim 20110107 */
U32                 CiTsControlGpioId;

U8  					CiCamInformation[80];
U8  					CiCamInformationLength = 0;

/* For CI TS Stream Control By KB Kim 2011.02.01 */
BOOL MW_CiTsControl(U8 tsCiOn)
{
	U8 gpioVal = 1;

	if (CiTsControlGpioId == 0)
	{
		/* Ci TS control GPIO is not initialized */
		return TRUE;
	}

	if (tsCiOn)
	{
		gpioVal = 0;
	}

	return GpioPortWrite(CiTsControlGpioId, gpioVal);
}

U32 AddCaDescriptor(U8 *descData, U8 *infoData, U32 infoSize)
{
	U32 descPointer;
	U32 infoPointer;
	U32 descSize;
	U16 casId;

	descPointer = 0;
	infoPointer = 0;

	while (infoPointer < infoSize)
	{
		descSize = infoData[infoPointer + 1] + 2;
		/* Copy CA Descriptor only */
		// printf("Descriptor Tag : 0x%02X\n", infoData[infoPointer]);
		if (infoData[infoPointer] == PMT_CA_DESCRIPTOR)
		{
			casId = (U16)Array2Word(infoData + infoPointer + 2, 2);
			// printf("Ca PMT found CasID : 0x%04X\n", casId);
			if(Check_ca_system_id_valid_status(casId))
			{
				/* Found Valid Cas ID */
				// printf("Ca PMT found Valid CasID : 0x%04X\n", casId);
				memcpy(descData + descPointer, infoData + infoPointer, descSize);
				descPointer += descSize;
			}
		}

		infoPointer += descSize;
	}

	return descPointer;
}

U32 AddCaPmtLengthField(U8 *lengthField, U32 length)
{
	U32 fieldPaointer;
	U8 fieldCounter;
	U8 counter;
	U8 tmpLen[4];

	fieldPaointer = 0;

	if (length < 0x80)
	{
		lengthField [fieldPaointer++] = length;
	}
	else
	{
		fieldCounter = 0;
		while(length > 0)
		{
			tmpLen[fieldCounter] = (U8)(length & 0xFF);
			fieldCounter++;
			length >>= 8;
		}

		lengthField [fieldPaointer++] = 0x80 | fieldCounter;
		for (counter = 0; counter < fieldCounter; counter++)
		{
			lengthField [fieldPaointer++]  = tmpLen[fieldCounter - counter - 1];
		}
	}

	return fieldPaointer;
}

U32 MW_Make_Ca_Pmt(eCS_CI_ca_pmt_list_management ca_pmt_list_mgmt,
		                   eCS_CI_ca_pmt_cmd_id ca_pmt_cmd,
		                   U8 *pmtData,
		                   U32 pmtSize,
		                   U8 *caPmtData)
{
	U8  tmpCaPmt[MAX_PMT_LENGTH];
	U32 caPmtIndex;
	U32 pmtIndex;
	U32 infoSize;
	U32 descSize;
	U32 caPmtSize;

	if ((pmtSize == 0) || (pmtSize > MAX_PMT_LENGTH))
	{
		/* No PMT Data */

		return 0;
	}

	if (!App_Info_isvalid)
	{
		/* CAM is not installed */

		return 0;
	}

	caPmtIndex = 0;

	tmpCaPmt[caPmtIndex] = ca_pmt_list_mgmt;
	caPmtIndex++;
	pmtIndex   = 3;
	/* Copy Program number (Service Id) */
	tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++];
	tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++];
	/* Version number  & Current next indicator */
	tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++] & 0x3F; /* Set reserved as 0 */
	pmtIndex += 4; /* Skip Section number, Last Section number and PCR Pid */

	infoSize = Array2Word(pmtData + pmtIndex, 2) & 0x0FFF;
	// printf("System Info Size : 0x%04X\n", infoSize);
	pmtIndex += 2;
	if ((pmtIndex + infoSize) > pmtSize)
	{
		/* Error in the Data*/
		return 0;
	}
	/* Process System CA Descriptor */
	descSize = AddCaDescriptor(tmpCaPmt + caPmtIndex + 3, pmtData + pmtIndex, infoSize);
	pmtIndex += infoSize;
	if (descSize > 0)
	{
		descSize++;
	Word2Array(tmpCaPmt + caPmtIndex, descSize, 2); /* Set Descriptor Lenght */
	caPmtIndex += 2;
		tmpCaPmt[caPmtIndex] = ca_pmt_cmd;
		caPmtIndex = caPmtIndex + descSize;
	}
	else
	{
		/* Info Length set to 0 */
		tmpCaPmt[caPmtIndex++] = 0;
		tmpCaPmt[caPmtIndex++] = 0;
	}

	while (pmtIndex < pmtSize)
	{
		/* Stream Type */
		tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++];
		/* ES Pid */
		tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++] & 0x1F; /* Set reserved as 0 */
		tmpCaPmt[caPmtIndex++] = pmtData[pmtIndex++];

		/* Process ES CA Descriptor */
		infoSize = Array2Word(pmtData + pmtIndex, 2) & 0x0FFF;
		// printf("ES Info Size : 0x%04X\n", infoSize);
		pmtIndex += 2;
		descSize = AddCaDescriptor(tmpCaPmt + caPmtIndex + 3, pmtData + pmtIndex, infoSize);
		pmtIndex += infoSize;
		if (descSize > 0)
		{
			descSize++;
			Word2Array(tmpCaPmt + caPmtIndex, descSize, 2); /* Set Descriptor Lenght */
			caPmtIndex += 2;
			tmpCaPmt[caPmtIndex] = ca_pmt_cmd;
			caPmtIndex = caPmtIndex + descSize;
		}
		else
		{
			/* Info Length set to 0 */
			tmpCaPmt[caPmtIndex++] = 0;
			tmpCaPmt[caPmtIndex++] = 0;
		}
	}

	caPmtSize = 0;

	/* CA PMT has Data */
	caPmtData[caPmtSize++] = 0x9F;
	caPmtData[caPmtSize++] = 0x80;
	caPmtData[caPmtSize++] = 0x32;
	descSize = AddCaPmtLengthField(caPmtData + caPmtSize, caPmtIndex);
	caPmtSize += descSize;
	memcpy(caPmtData + caPmtSize, tmpCaPmt, caPmtIndex);
	caPmtSize += caPmtIndex;

	// OsDumpData("PMT", pmtData, pmtSize);
	// OsDumpData("CA PMT", caPmtData, caPmtSize);
	return caPmtSize;
}

static void	MW_CI_MMI_Clear_Display(void)
{
        printf("MW_CI_MMI_Clear_Display\n");
        BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_Clear_Display, 0);
}

static void	MW_CI_MMI_Broken(void)
{
        printf("MW_CI_MMI_Broken\n");
        BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_MMI_Broken, 0);
}


static int	MW_CI_MMI_Display_Enquiry(	unsigned char * str,
												unsigned char length,
												unsigned char blind_answer,
												unsigned char expected_answer_length)
{
	static tMW_CI_Enquiry_info msg_buff;
	memset(&msg_buff, 0 ,sizeof(tMW_CI_Enquiry_info));

	printf("MW_CI_MMI_Display_Enquiry\n");

	if(length > 0)
	{
		msg_buff.text = malloc(length);

		memcpy(msg_buff.text, str, length);

		msg_buff.length = length;
		msg_buff.blind_answer = blind_answer;
		msg_buff.expected_answer_length = expected_answer_length;

		BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_Display_Enquiry, (LPARAM)(&msg_buff));
	}

	return 1;
}

static int	MW_CI_MMI_Display_List(void)
{
    printf("MW_CI_MMI_Display_List\n");
    BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_Display_List, 0);
	return 1;
}

static  int	MW_CI_MMI_Display_Menu(void)
{
    printf("MW_CI_MMI_Display_Menu\n");
    BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_Display_Menu, 0);
	return 1;
}

static void	MW_CI_MMI_Insert_Notify (unsigned char messageNo)
{
	if (CiCamStatus != messageNo)
	{
		if (messageNo == 4)
		{
			MW_CiTsControl(1);
		}
		CiCamStatus = messageNo;
		printf("MW_CI_MMI_Insert_Notify %d\n", messageNo);
		BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_Insert_Notify, messageNo);
		if(messageNo == 5)
		{
			/* For CI TS Stream Control By KB Kim 2011.02.01 */
			MW_CiTsControl(0);
			App_Info_isvalid = FALSE;
			memset(CiCamInformation, 0x00, 80);
			CiCamInformationLength = 0;
		}
	}
}

static void	MW_CI_MMI_Cas_Info_Arrived(void)
{
	U8   pmtBuffer[MAX_PMT_LENGTH];
	U8   caPmtBuffer[MAX_PMT_LENGTH];
	U32  pmtLength;
	U32  caPmtLength;
	eCS_CI_ca_pmt_list_management caPmtListMgmt;

	printf("MW_CI_MMI_Cas_Info_Arrived\n");
	App_Info_isvalid = TRUE;

	memset(pmtBuffer, 0x00, MAX_PMT_LENGTH);
	memset(caPmtBuffer, 0x00, MAX_PMT_LENGTH);
	pmtLength = SiGetLivePmtData(pmtBuffer);
	if ( pmtLength > 4)
	{
		/* Need to Process CA_PMT */
		if (FirstCaPmt)
		{
			caPmtListMgmt = CI_CA_PMT_LIST_ONLY;
		}
		else
		{
			caPmtListMgmt = CI_CA_PMT_LIST_UPDATE;
		}

		pmtLength -= 4; /* CRC */
 		caPmtLength = MW_Make_Ca_Pmt(caPmtListMgmt,
			                         CI_CA_PMT_CMD_OK_DESCRAMBLING,
			                         pmtBuffer,
			                         pmtLength,
			                         caPmtBuffer);
		if (caPmtLength > 0)
		{
			FirstCaPmt = FALSE;
			cas_send_ca_pmt_list_to_all((unsigned char *)caPmtBuffer, (unsigned int)caPmtLength);
		}
	}
	else
	{
		caPmtLength = 0;
	}
}

U8 MW_CI_Get_Cam_Information(U8 *infoData)
{
	if (CiCamInformationLength > 0)
	{
		memcpy(infoData, CiCamInformation, CiCamInformationLength);
	}

	return CiCamInformationLength;
}

static void	MW_CI_MMI_App_Info_Changed(int flag)
{
	int				count=0;
	struct 			application_information_t *info = NULL;

    printf("MW_CI_MMI_App_Info_Changed(%d)\n", flag);

	count = ai_get_application_information(&info);

	if (info == NULL)
	{
		printf("MW_CI_MMI_App_Info_Changed No Info\n");
		CiCamInformationLength = 0;
		memset(CiCamInformation, 0x00, 80);
		return;
	}

	memset(CiCamInformation, 0x00, 80);
	memcpy((void*)CiCamInformation, (void*)((info)->manufacture_info), (info)->string_length);
	CiCamInformationLength = (info)->string_length;

	printf("manufacture_info : %s [0x%02X - 0x%04X - 0x%04X]\n", CiCamInformation, info->application_type, info->application_manufacture, info->manufacture_code);
	// For D-Smart CAM : Videoguard CA [0x01 - 0xCAFE - 0xBABE]

	if(count)
		free(info);

    BroadcastMessage (MSG_CI_MMI_UPDATE, MSG_PARAM_CI_App_Info_Changed, flag);
}

BOOL MW_CI_MMI_Get_App_Info_Status(void)
{
	return(App_Info_isvalid);
}

#endif  // #ifdef SUPPORT_CI

static void ResetStreamTable(void)
{
	U8			i;

	AudioStream.Number=0;
	for(i=0;i<MW_ES_STREAM_MAX;i++)
	{
		AudioStream.Stream[i].Pid=kDB_DEMUX_INVAILD_PID;
		AudioStream.Stream[i].Language[0]=0;
		AudioStream.Stream[i].Type=MW_SVC_MP2;
	}

	TeletextStream.Number=0;
	for(i=0;i<MW_ES_STREAM_MAX;i++)
	{
		TeletextStream.Stream[i].Pid=kDB_DEMUX_INVAILD_PID;
		TeletextStream.Stream[i].Language[0]=0;
		TeletextStream.Stream[i].Type=MW_SVC_MP2;
	}

	SubtitleStream.Number=0;
	for(i=0;i<MW_ES_STREAM_MAX;i++)
	{
		SubtitleStream.Stream[i].Pid=kDB_DEMUX_INVAILD_PID;
		SubtitleStream.Stream[i].Language[0]=0;
		SubtitleStream.Stream[i].Type=MW_SVC_MP2;
	}
}

static void MW_PlayProgram(MV_stServiceInfo   service_data)
{
	tCS_AV_PlayParams   play_params;

	OpenSVC = TRUE;
	play_params.Video_PID = mv_service_data.u16VideoPid;
	play_params.Audio_PID = mv_service_data.u16AudioPid;
	play_params.PCR_PID = mv_service_data.u16PCRPid;

	if ( CS_AV_AudioGetVolume() != 0 )
	{
		if ( service_data.u8AudioVolume > 32 )
		{
			if ( CS_AV_AudioGetVolume() + ( service_data.u8AudioVolume - 32 ) > kCS_DBU_MAX_VOLUME )
				play_params.Audio_Volume = kCS_DBU_MAX_VOLUME;
			else
				play_params.Audio_Volume = CS_AV_AudioGetVolume() + ( service_data.u8AudioVolume - 32 );
		} else if ( service_data.u8AudioVolume < 32 ) {
			if ( CS_AV_AudioGetVolume() - ( 32 - service_data.u8AudioVolume ) < 0 )
				play_params.Audio_Volume = 0;
			else
				play_params.Audio_Volume = CS_AV_AudioGetVolume() - ( 32 - service_data.u8AudioVolume );
		} else
			play_params.Audio_Volume = CS_AV_AudioGetVolume();
	} else
		play_params.Audio_Volume = CS_AV_AudioGetVolume();

	switch(service_data.u8VideoType)
	{
		case eCS_DB_VIDEO_MPEG2:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_MPEG2;
			break;

		case eCS_DB_VIDEO_H264:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_H264;
			break;

		default:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_MPEG2;
			break;
	}

	switch(service_data.u8Audio_Type)
	{
		case eCS_DB_AUDIO_MPEG2:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_MPEG2;
			break;

		case eCS_DB_AUDIO_AC3:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_AC3;
			break;

		/* Add By River 06.12.2008 */
		case eCS_DB_AUDIO_AAC:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_AAC;
			break;

		/* Add By River 06.12.2008 */
		case eCS_DB_AUDIO_LATM:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_LATM;
			break;

		default:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_MPEG2;
			break;
	}

	switch(service_data.u8Audio_Mode)
	{
		case MV_DB_STEREO:
			play_params.AudioMode = MV_DB_STEREO;
			break;

		case MV_DB_LEFT:
			play_params.AudioMode = MV_DB_LEFT;
			break;

		case MV_DB_RIGHT:
			play_params.AudioMode = MV_DB_RIGHT;
			break;

		default:
			play_params.AudioMode = MV_DB_MONO;
			break;
	}

	CS_AV_ProgramPlay(play_params);

	CS_MW_IsVideoFreezed = FALSE;
}

/* By KB Kim 2011.06.02 */
void MvRePlayVideo(void)
{
	tCS_AV_PlayParams   play_params;

	play_params.Video_PID = mv_service_data.u16VideoPid;
	play_params.Audio_PID = mv_service_data.u16AudioPid;
	play_params.PCR_PID   = mv_service_data.u16PCRPid;

	if ( CS_AV_AudioGetVolume() != 0 )
	{
		if ( mv_service_data.u8AudioVolume > 32 )
		{
			if ( CS_AV_AudioGetVolume() + ( mv_service_data.u8AudioVolume - 32 ) > kCS_DBU_MAX_VOLUME )
				play_params.Audio_Volume = kCS_DBU_MAX_VOLUME;
			else
				play_params.Audio_Volume = CS_AV_AudioGetVolume() + ( mv_service_data.u8AudioVolume - 32 );
		} else if ( mv_service_data.u8AudioVolume < 32 ) {
			if ( CS_AV_AudioGetVolume() - ( 32 - mv_service_data.u8AudioVolume ) < 0 )
				play_params.Audio_Volume = 0;
			else
				play_params.Audio_Volume = CS_AV_AudioGetVolume() - ( 32 - mv_service_data.u8AudioVolume );
		} else
			play_params.Audio_Volume = CS_AV_AudioGetVolume();
	} else
		play_params.Audio_Volume = CS_AV_AudioGetVolume();

	switch(mv_service_data.u8VideoType)
	{
		case eCS_DB_VIDEO_MPEG2:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_MPEG2;
			break;

		case eCS_DB_VIDEO_H264:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_H264;
			break;

		default:
			play_params.VideoType = eCS_AV_VIDEO_STREAM_MPEG2;
			break;
	}

	switch(mv_service_data.u8Audio_Type)
	{
		case eCS_DB_AUDIO_MPEG2:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_MPEG2;
			break;

		case eCS_DB_AUDIO_AC3:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_AC3;
			break;

		/* Add By River 06.12.2008 */
		case eCS_DB_AUDIO_AAC:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_AAC;
			break;

		/* Add By River 06.12.2008 */
		case eCS_DB_AUDIO_LATM:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_LATM;
			break;

		default:
			play_params.AudioType = eCS_AV_AUDIO_STREAM_MPEG2;
			break;
	}

	switch(mv_service_data.u8Audio_Mode)
	{
		case MV_DB_STEREO:
			play_params.AudioMode = MV_DB_STEREO;
			break;

		case MV_DB_LEFT:
			play_params.AudioMode = MV_DB_LEFT;
			break;

		case MV_DB_RIGHT:
			play_params.AudioMode = MV_DB_RIGHT;
			break;

		default:
			play_params.AudioMode = MV_DB_MONO;
			break;
	}

	Mv_VideoRestart(play_params);
}

static void MW_UpdateAVPlay(tMWStream Audiostream, U16 Vpid, tCS_DB_VideoType Vtype)
{
	//tCS_DB_ServiceData		NewService;
	U8						i;
	BOOL					Update=FALSE;

	if(mv_service_data.u16AudioPid == kDB_DEMUX_INVAILD_PID)
	{
		for(i=0;i<MW_ES_STREAM_MAX;i++)
		{
			if(Audiostream.Stream[i].Pid!=kDB_DEMUX_INVAILD_PID)
			{
				Update=TRUE;
				break;
			}
		}
	}
	else
	{
		for(i=0;i<MW_ES_STREAM_MAX;i++)
		{
			if((Audiostream.Stream[i].Pid==mv_service_data.u16AudioPid)
					&&(Audiostream.Stream[i].Type == mv_service_data.u8Audio_Type))
			{
				Update=FALSE;
				break;
			}
			Update=TRUE;
		}
	}

	//

	if(Update==TRUE)
	{
		mv_service_data.u8Audio_Type	=	Audiostream.Stream[0].Type&0xff;
		mv_service_data.u16AudioPid		=	Audiostream.Stream[0].Pid&0x1fff;
	}

	if((mv_service_data.u16VideoPid!=Vpid)||(mv_service_data.u8VideoType != Vtype))
	{
		Update=TRUE;
		mv_service_data.u16VideoPid = Vpid&0x1fff;
		mv_service_data.u8VideoType  = Vtype&0xff;
	}

	if( Update == FALSE ) return;

	//printf("\n==== MW_UpdateAVPlay : MW_PlayProgram ===== \n");
	MW_PlayProgram( mv_service_data );

	//update database in here

}

void MW_FE_EIT_Start(void)
{
	CS_Eit_Start(LastTPIdx, eCS_EIT_ACTUAL_ALL, 0xFF);
	// CS_Eit_Start(LastTPIdx, eCS_EIT_ACTUAL_PF, 0xFF);
}

static void MW_FENotify(tCS_FE_Notification value)
{
	tMWNotifyData		NotifyData;


	if( OpenSVC == FALSE )
	{
		//MW_SVC_printf("MW_FENotify: Don't open MW SVC\n");
		return;
	}

	switch(value)
	{
		case eCS_FE_LOCKED:
			//printf("FE LOCKED , time = %d, LastTPIndex[%d]\n", CS_OS_time_now(), LastTPIdx);
			NotifyData.uData.FEStatus=FE_LOCK;
			MW_SVC_printf("eCS_FE_LOCKED\n");

			/* 2010.09.04 By KB KIm For New SI */
			SiStartLiveSearch(0, 0, mv_service_data.u16ServiceId);

			/* */
			CS_Eit_Start(LastTPIdx, eCS_EIT_ACTUAL_ALL, 0xFF);
			// CS_Eit_Start(LastTPIdx, eCS_EIT_ACTUAL_PF, 0xFF);
			CS_DT_Start();

			/* by kb : 20100406 */
			if (VideoReplayOn)
			{
				VideoReplayOn = 0;
				//printf("\n==== MW_FENotify : MW_PlayProgram ===== \n");
				MW_PlayProgram( mv_service_data );
			}
			break;

		case eCS_FE_SIGNAL_LOST:
			NotifyData.uData.FEStatus=FE_LOST;
			MW_SVC_printf("eCS_FE_SIGNAL_LOST\n");
			LastServixeIdx=0xFFFF;

			/* 2010.09.04 By KB KIm For New SI */
			SiStopLiveSearch();

			if( CSMPR_Player_GetStatus() != CSMPR_PLAY_RUN )
			{
				CS_AV_ProgramStop();

				if ( mv_service_data.u8TvRadio != eCS_DB_RADIO_SERVICE )
				CS_AV_VideoBlank();
				else
				{
					if ( VideoReplayOn == 0 )
						CS_AV_Play_IFrame(RADIO_BACK);
				}

				VideoReplayOn = 1;  // by kb : 20100406
			}

			ResetStreamTable();

			break;

		case eCS_FE_UNLOCKED:
			NotifyData.uData.FEStatus=FE_UNLOCK;
			MW_SVC_printf("eCS_FE_UNLOCKED\n");
			LastServixeIdx=0xFFFF;

			/* 2010.09.04 By KB KIm For New SI */
			SiStopLiveSearch();

			if( CSMPR_Player_GetStatus() != CSMPR_PLAY_RUN )
			{
				CS_AV_ProgramStop();

				if ( mv_service_data.u8TvRadio != eCS_DB_RADIO_SERVICE )
				CS_AV_VideoBlank();
				else
				{
					if ( VideoReplayOn == 0 )
						CS_AV_Play_IFrame(RADIO_BACK);
				}

				VideoReplayOn = 1;  // by kb : 20100406
			}

			ResetStreamTable();
			break;

		default:
			break;
	}

	if(SvcCallBack!=NULL)
	{
		NotifyData.type=UPDATE_FE;
		SvcCallBack(NotifyData);
	}
}

void LivePmtCallBack(SiProgramData_t *progData, U8 *pmtData, U32 length)
{
	U8					esCount;
	U16					videoPid  = kDB_DEMUX_INVAILD_PID;
	tCS_DB_VideoType	videotype = eCS_DB_VIDEO_MPEG2;
	SiEsData_t			*esData;
	tMWNotifyData		notifyData;
#ifdef SUPPORT_CI
	U8   				caPmtBuffer[MAX_PMT_LENGTH];
	U32  				caPmtLength;
	eCS_CI_ca_pmt_list_management caPmtListMgmt;
#endif
	// printf("LivePmtCallBack : GET PMT!\n");
	if(OpenSVC==FALSE)
	{
		printf("LivePmtCallBack : Sercice Stopped\n");
		return;
	}

	if (mv_service_data.u16ServiceId != progData->ServideId)
	{
		printf("LivePmtCallBack : Service is not matched [0x%04X / 0x%04X]\n", mv_service_data.u16ServiceId, progData->ServideId);
		/* Stop and re-start service */
		SiStopLiveSearch();
		SiStartLiveSearch(0, 0, mv_service_data.u16ServiceId);
		return;
	}

	/* For Cas By Jacob 10Feb.2011 */
	CasDrvNotifyPMT(0, pmtData);
	// OsDumpData("PMT Data", pmtData, length);

#ifdef SUPPORT_CI
	/* Make CA PMT from PMT Data */

	memset(caPmtBuffer, 0x00, MAX_PMT_LENGTH);

	if ( length > 4)
	{

		/* Need to Process CA_PMT */
		if (FirstCaPmt)
		{
			caPmtListMgmt = CI_CA_PMT_LIST_ONLY;
		}
		else
		{
			caPmtListMgmt = CI_CA_PMT_LIST_UPDATE;
		}

 		caPmtLength = MW_Make_Ca_Pmt(caPmtListMgmt,
			                         CI_CA_PMT_CMD_OK_DESCRAMBLING,
			                         pmtData,
			                         (length - 4),
			                         caPmtBuffer);
		if (caPmtLength > 0)
		{
			FirstCaPmt = FALSE;
			cas_send_ca_pmt_list_to_all((unsigned char *)caPmtBuffer, (unsigned int)caPmtLength);
		}
	}
	else
	{
		caPmtLength = 0;
	}
#endif //#ifdef SUPPORT_CI

	ResetStreamTable();
	for (esCount = 0; esCount < progData->NumberOfEs ; esCount++)
	{
		esData = &progData->EsData[esCount];
		switch(esData->EsType)
		{
			case SI_ES_VIDEO        :
				videoPid = esData->EsPid;
				if (esData->EsSubData.EsSubType == (U8)VIDEO_TYPE_H264)
				{
					videotype = eCS_DB_VIDEO_H264;
				}
				else
				{
					videotype = eCS_DB_VIDEO_MPEG2;
				}
				break;
			case SI_ES_AUDIO        :
				if(AudioStream.Number >= MW_ES_STREAM_MAX)
				{
					break;
				}
				AudioStream.Stream[AudioStream.Number].Pid = esData->EsPid;
				memset(AudioStream.Stream[AudioStream.Number].Language, 0x00, kCS_SI_TRIGRAM_MAX_LENGTH+1);
				memcpy(AudioStream.Stream[AudioStream.Number].Language, esData->EsLang ,kCS_SI_TRIGRAM_MAX_LENGTH);
				AudioStream.Stream[AudioStream.Number].Type = SiDbAudioType[esData->EsSubData.EsSubType];
				AudioStream.Number++;
				break;
			case SI_ES_TELETEXT     :
				if (esData->EsSubData.EsSubType != (U8)TELETEXT_SUB_TITLE)
				{
					if(TeletextStream.Number >= MW_ES_STREAM_MAX)
					{
						break;
					}
					TeletextStream.Stream[TeletextStream.Number].Pid = esData->EsPid;
					memset(TeletextStream.Stream[TeletextStream.Number].Language, 0x00, kCS_SI_TRIGRAM_MAX_LENGTH+1);
					memcpy(TeletextStream.Stream[TeletextStream.Number].Language, esData->EsLang ,kCS_SI_TRIGRAM_MAX_LENGTH);
					TeletextStream.Stream[TeletextStream.Number].Type = MW_SVC_TELETEXT;
					TeletextStream.Stream[TeletextStream.Number].uDescriptor.Teletext.MagazineID = esData->EsSubData.TeletextData.MagazineNumber;
					TeletextStream.Stream[TeletextStream.Number].uDescriptor.Teletext.PageID     = esData->EsSubData.TeletextData.PageNumber;
					TeletextStream.Number++;
					//printf("Teletext pid=%d  Lang=%s\n",TeletextStream.Stream[TeletextStream.Number-1].Pid,TeletextStream.Stream[TeletextStream.Number-1].Language);
					if(SvcCallBack!=NULL)
					{
						notifyData.type=UPDATE_TTX;
						SvcCallBack(notifyData);
					}
					break;
				}
			case SI_ES_SUBTITLE :
				if(SubtitleStream.Number >= MW_ES_STREAM_MAX)
				{
					break;
				}

				SubtitleStream.Stream[SubtitleStream.Number].Pid = esData->EsPid;
				memset(SubtitleStream.Stream[SubtitleStream.Number].Language, 0x00, kCS_SI_TRIGRAM_MAX_LENGTH+1);
				memcpy(SubtitleStream.Stream[SubtitleStream.Number].Language, esData->EsLang ,kCS_SI_TRIGRAM_MAX_LENGTH);
				if (esData->EsSubData.EsSubType == DVB_SUBTITLE)
				{
					SubtitleStream.Stream[SubtitleStream.Number].Type=MW_SVC_DVB_SUBTITLE;
					SubtitleStream.Stream[SubtitleStream.Number].uDescriptor.Subtitle.CompositionPageID = esData->EsSubData.SubtitleData.CompositionPage;
					SubtitleStream.Stream[SubtitleStream.Number].uDescriptor.Subtitle.AncillaryPageID   = esData->EsSubData.SubtitleData.AncillaryPage;
				}
				else
				{
					SubtitleStream.Stream[SubtitleStream.Number].Type=MW_SVC_TTX_SUBTITLE;
					SubtitleStream.Stream[SubtitleStream.Number].uDescriptor.Teletext.MagazineID = esData->EsSubData.TeletextData.MagazineNumber;
					SubtitleStream.Stream[SubtitleStream.Number].uDescriptor.Teletext.PageID     = esData->EsSubData.TeletextData.PageNumber;
				}

				/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
				if (CurrentSubtitleNumber == INVALID_SUBTITLE_NUMBER)
				{
					if (MV_CompareLanguageCodeByType((U8 *)esData->EsLang))
					{
						/* Found matched subtitle language */
						CurrentSubtitleNumber = SubtitleStream.Number;
					}
				}

				SubtitleStream.Number++;

				/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
				/*
				if(SvcCallBack! = NULL)
				{
					notifyData.type = UPDATE_SUB;
					SvcCallBack(notifyData);
				}
				*/

				break;
			default :
				break;
		}
	}

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
	if (SubtitleStream.Number > 0)
	{
		if (CurrentSubtitleNumber == INVALID_SUBTITLE_NUMBER)
		{
			/* Didn't find matched Subtitle language */
			CurrentSubtitleNumber = 0;
		}
		else if (CurrentSubtitleNumber > SubtitleStream.Number)
		{
			/* Disable Subtitle */
			CurrentSubtitleNumber = SubtitleStream.Number;
		}

		if(SvcCallBack != NULL)
		{
			notifyData.type = UPDATE_SUB;
			SvcCallBack(notifyData);
		}
	}

	MW_UpdateAVPlay(AudioStream, videoPid, videotype);
	if(SvcCallBack != NULL)
	{
		notifyData.type = UPDATE_PMT;
		notifyData.uData.ServiceID = progData->ServideId;
		SvcCallBack(notifyData);
	}

}

BOOL CS_MW_SVC_Init(void)
{
#ifdef SUPPORT_CI
	tCS_CI_NotifyFunctions  mmi_funcs;
	tCS_CI_InitParam        ci_param;

	/* Added for CI TS Control by KB Kim 20110107 */
	GpioOpenParam_t         tsControlGpioOpenParam;
	GpioOpenParam_t         tsResetGpioOpenParam;
#endif

	SiRegisterLiveSearchCallBack(LivePmtCallBack);

	if(CS_FE_Register_Tuner_Notify(&FEClientId,MW_FENotify)!=eCS_FE_NO_ERROR)
	{
		MW_SVC_printf("CS_FE_Register_Tuner_Notify   error\n");
		return FALSE;
	}

	CS_EIT_RegisterNotify(&EITClientID, EitSectionNotify);

#ifdef SUPPORT_CI
	/* Added for CI TS Control by KB Kim 20110107 */
	tsControlGpioOpenParam.PortNumber = 1;
	tsControlGpioOpenParam.BitNumber  = 52;
	tsControlGpioOpenParam.Mode       = 1;
	/* For CI TS Stream Control By KB Kim 2011.02.01 */
	tsControlGpioOpenParam.Value      = 1;
	GpioPortOpen(&CiTsControlGpioId, &tsControlGpioOpenParam);
	/* For CI TS Stream Control By KB Kim 2011.02.01 */
	MW_CiTsControl(0);
	/*
	if (CiTsControlGpioId == 0)
	{
		printf("CS_CI_init : Error to Init TS control GPIO\n");
	}
	else
	{
		printf("CS_CI_init : GPIO[%d] inited to %d\n", tsControlGpioOpenParam.BitNumber, tsControlGpioOpenParam.Value);
	}
	*/

	mmi_funcs.clear_display = &MW_CI_MMI_Clear_Display;
	mmi_funcs.display_enquiry = &MW_CI_MMI_Display_Enquiry;
	mmi_funcs.display_list = &MW_CI_MMI_Display_List;
	mmi_funcs.display_menu = &MW_CI_MMI_Display_Menu;
	mmi_funcs.insert_notify = &MW_CI_MMI_Insert_Notify;
	mmi_funcs.cas_info_arrived = &MW_CI_MMI_Cas_Info_Arrived;
	mmi_funcs.app_info_changed = &MW_CI_MMI_App_Info_Changed;
	mmi_funcs.mmi_broken = &MW_CI_MMI_Broken;

    ci_param.gpio_ready = 14;
    ci_param.gpio_reset = 12;

	CS_CI_init(ci_param);
	printf("CS_CI_init\n");

	CS_CI_Register_CI_Notify(mmi_funcs);

#endif

	ResetStreamTable();

	LastTPIdx=0xFFFF;
	LastServixeIdx=0xFFFF;
	SvcCallBack=NULL;

	return TRUE;
}

BOOL CS_MW_SVC_Open(MW_SVCNotify SVCNotify)
{
	if(SVCNotify != NULL)
		SvcCallBack=SVCNotify;

	return TRUE;
}

BOOL CS_MW_SVC_Close(void)
{
	SvcCallBack=NULL;

	return TRUE;
}

BOOL CS_MW_SVC_Term(void)
{
	CS_FE_Unregister_Tuner_Notify(FEClientId);
#ifdef SUPPORT_CI
	if (CiTsControlGpioId != 0)
	{
		GpioPortClose(CiTsControlGpioId);
	}
#endif //#ifdef SUPPORT_CI
	return TRUE;
}

void CS_MW_GetAudioStream(tMWStream *pStream)
{
	memcpy(pStream,&AudioStream,sizeof(tMWStream));
}

void CS_MW_GetTeletextStream(tMWStream *pStream)
{
	memcpy(pStream,&TeletextStream,sizeof(tMWStream));
}

void CS_MW_GetSubtitleStream(tMWStream *pStream)
{
	memcpy(pStream,&SubtitleStream,sizeof(tMWStream));
}

static U8           SubOpen = SUBTITLE_OPEN_NONE;
static BOOL			SubInit=FALSE;
static TTXInit_Params 	InitPara;
BOOL CS_MW_TTXSUBInit(void)
{
	SUBInit_Params SubInitPara;

	InitPara.pMessage=CSOS_CreateMessageQueue("/csttxQ",sizeof(UpdateEventPara),64);
	InitPara.Colormode=CS_TTX_COLOR_16BITS;
//	InitPara.Displaymode=CS_TTX_DEFAULT_MODE;
	InitPara.Displaymode=CS_TTX_720_MODE;
	InitialTeletext(InitPara);
	SubInitPara.Colormode=CS_SUB_COLOR_16BITS;
//	SubInitPara.Displaymode=CS_SUB_DEFAULT_MODE;
	SubInitPara.Displaymode=CS_SUB_720_MODE;
	InitialSubtitle(SubInitPara);
	SubInit=TRUE;
	SubOpen = SUBTITLE_OPEN_NONE;

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
	CurrentSubPid  = 0xFFFF;
	SubtitleModeOn = 0;
	CurrentSubtitleNumber = INVALID_SUBTITLE_NUMBER;

	return(TRUE);
}


U16 CS_MW_GetSubtitlePid(void)
{
	return CurrentSubPid;
}

void CS_MW_SetSubtitlePid(U16 Pid)
{
	CurrentSubPid = Pid;
}

void CS_MW_SetSubtitleLang(U8 *Lang,U8 Length)
{
	memcpy(CurrentSubLang, Lang, Length);
}

#if 1
CSOS_MessageQueue_t* GetTTXpMsgQ(void)
{
	return InitPara.pMessage;
}
#endif

static void CS_MW_TTXNotify(U32 pBitmap,U32 lpara)
{
	BroadcastMessage (MSG_TTX_DISPLAY, pBitmap, lpara);
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
void MvClearCurrentSubtitle(void)
{
	CurrentSubtitleNumber = INVALID_SUBTITLE_NUMBER;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
void MvSetCurrentSubtitle(U8 current)
{
	CurrentSubtitleNumber = current;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U8 MvGetCurrentSubtitle(void)
{
	return CurrentSubtitleNumber;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U8 MvGetTotalSubtitleNumber(void)
{
	return SubtitleStream.Number;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
void MvSetSubtitleMode(U8 mode)
{
	SubtitleModeOn = mode;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
U8 MvGetSubtitleMode(void)
{
	return SubtitleModeOn;
}

void CS_MW_CloseSubtitle(void)
{
	MvSetSubtitleMode(0); /* Turn Off temp subtitle mode */

	if(SubInit == FALSE || SubOpen == SUBTITLE_OPEN_NONE)
	{
		return;
	}

	CurrentSubPid=0xFFFF;

	if (SubOpen == SUBTITLE_OPEN_EBU)
	{
		DestroyTeletext();
	}
	else if (SubOpen == SUBTITLE_OPEN_DVB)
	{
		CloseSubtitle();
	}

	SubOpen = SUBTITLE_OPEN_NONE;
}

/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
void CS_MW_OpenSubtitle(void)
{
	tMWStreamInfo		*pStreamInfo=NULL;
	U16					PageHex;
	U16                 pid;
	// U8					i=0;

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
	// if(SubOpen==TRUE||SubInit==FALSE)

	/* For Subtitle Auto Start By KB Kim 2011.09.06 */
	// if((SubInit==FALSE) || (SubtitleModeOn == 0))
	if(SubInit==FALSE)
		return;

	if ((CS_DBU_Get_Use_SubTitle() == 0) && (MvGetSubtitleMode() == 0))
	{
		return;
	}

	/* Close previous Subtitle */
	if (SubOpen != SUBTITLE_OPEN_NONE)
	{
		CS_MW_CloseSubtitle();
	}

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
#if 1
	if (CurrentSubtitleNumber < SubtitleStream.Number)
	{
		pStreamInfo = &SubtitleStream.Stream[CurrentSubtitleNumber];
	}
#else
	for(i=0;i<SubtitleStream.Number;i++)
	{
		if((SubtitleStream.Stream[i].Pid==Pid)&&(!memcmp(SubtitleStream.Stream[i].Language,CurrentSubLang,kCS_SI_TRIGRAM_MAX_LENGTH+1)))
		{
			pStreamInfo = &SubtitleStream.Stream[i];
			break;
		}
	}
#endif

	if(pStreamInfo==NULL)
	{
		// printf("Do not match stream\n");
		return;
	}

	pid = pStreamInfo->Pid;
	CS_MW_SetSubtitlePid (pid);

	if(pStreamInfo->Type == MW_SVC_TTX_SUBTITLE)
	{
		if((pStreamInfo->uDescriptor.Teletext.MagazineID==0x00))
		{
			PageHex=(0x0800|pStreamInfo->uDescriptor.Teletext.PageID);
		}
		else
		{
			PageHex=((pStreamInfo->uDescriptor.Teletext.MagazineID<<8)|pStreamInfo->uDescriptor.Teletext.PageID);
		}
		// printf("=============>>  EBU Subtitle PageHex=%x (%d)\n", PageHex, PageHex);
		CreateTeletext( pid, PageHex, CS_TTX_SUBTITLE, pStreamInfo->Language, CS_MW_TTXNotify);
		SubOpen = SUBTITLE_OPEN_EBU;
	}
	else if(pStreamInfo->Type==MW_SVC_DVB_SUBTITLE)
	{
	    // printf("Open DVB Subtitle\n");
		OpenSubtitle( pid, CS_MW_TTXNotify );
		SubOpen = SUBTITLE_OPEN_DVB;
	}

}

U8 CS_MW_Get_Subtitle_Status(void)
{
	return SubOpen;
}

U16 CS_MW_GetCurrentPlayProgram(void)
{
	return LastServixeIdx;
}

BOOL CS_MW_PlayServiceByIdx(U16 Index, U8 RetuneKind)
{
	MV_stServiceInfo			prevServiceData;
	MV_stTPInfo					channel_data;
	MV_ScanParams				scan_data;

	//dprintf(("******************  CS_MW_PlayServiceByIdx  : %d -- %d *************************\n", Index, RetuneKind));
	//CS_AV_Play_IFrame(BOOT_LOGO);
	/* By KB Kim for Radio Back : 2011.06.20 */
	if(MV_DB_GetServiceDataByIndex(&prevServiceData, LastServixeIdx)!=eCS_DB_OK)
	{
		prevServiceData.u8TvRadio = (U8)DATA_SERVICE;
	}

	if(MV_DB_GetServiceDataByIndex(&mv_service_data, Index)!=eCS_DB_OK)
	{
		MW_SVC_printf("MV_DB_GetServiceDataByIndex   error\n");
		return FALSE;
	}

	if ((LastServixeIdx == Index ) && (RetuneKind == NOT_TUNNING))
	{
		/* by kb : 20100403
		if ( RetuneKind == RE_TUNNING )
		 	MW_PlayProgram( mv_service_data );
		*/
		return TRUE;
	}

	/* For Auto Select Language for Audio, Subtitle and Teletext By KB Kim 2011.04.06 */
	if (LastServixeIdx != Index)
	{
		MvClearCurrentSubtitle();
	}

#ifdef SUPPORT_CI
	FirstCaPmt       = TRUE;
#endif // #ifdef SUPPORT_CI

	/* 2010.09.04 By KB KIm For New SI */
	SiStopLiveSearch();

	LastServixeIdx = Index;

	if ((CS_DBU_GetCH_Change_Type() == 0) &&
		((prevServiceData.u8TvRadio != (U8)RADIO_SERVICE) ||
		 (mv_service_data.u8TvRadio != (U8)RADIO_SERVICE)))
	{
		CS_MW_StopService(TRUE);
	}
	else
	{
		CS_MW_StopService(FALSE);
	}

	//FE
	// printf("CS_MW_PlayServiceByIdx index = %d  %d\n", Index, RetuneKind);

	if ((LastTPIdx != mv_service_data.u16TransponderIndex) || (RetuneKind != NOT_TUNNING))
	{
		// printf("CS_MW_PlayServiceByIdx New Tune\n");
		b8PlayService = TRUE;
		//CS_FE_StopScan();
		CS_DT_Stop();
		CS_Eit_Stop();
		DB_DemuxResetChannel();

		LastTPIdx = mv_service_data.u16TransponderIndex;  // By kb : 20100403

		ResetStreamTable();

		if(MV_DB_GetTPDataByIndex( &channel_data, mv_service_data.u16TransponderIndex )!=eCS_DB_OK)
		{
			MW_SVC_printf("CS_DB_GetTerTPDataByIndex   error %d %s\n", mv_service_data.u16TransponderIndex, mv_service_data.acServiceName);
			return FALSE;
		}

		OpenSVC = TRUE;

		/*
		printf("CS_MW_PlayServiceByIdx Info : CurrentTP Index[%d], u16TransponderIndex[%d] / %d\n",
			channel_data.u16TPIndex, mv_service_data.u16TransponderIndex, LastTPIdx);
			*/
		scan_data.u16Tpnumber		=	channel_data.u16TPIndex;
		scan_data.u16TPFrequency	=	channel_data.u16TPFrequency;
		scan_data.u16SymbolRate		=	channel_data.u16SymbolRate;
		scan_data.u8Polar_H			=	channel_data.u8Polar_H;
		/* By KB Kim 2011.08.08 */
		// printf("CS_MW_PlayServiceByIdx : Start Scan + CasDrvStartNewChannel!!\n");
		CasDrvStartNewChannel  (0, 0, Index, mv_service_data.u16ServiceId, mv_service_data.u16VideoPid, mv_service_data.u16AudioPid);
		CS_FE_StartScan(scan_data, 1);
		MW_PlayProgram( mv_service_data );
	}
	else
	{
		/* 2010.09.04 By KB KIm For New SI */
		// CS_SI_Enable_PAT_Acquisition();
		MW_PlayProgram( mv_service_data );
		/* By KB Kim 2011.08.08 */
		// printf("******************  CS_MW_PlayServiceByIdx  : CasDrvStartNewChannel!!\n");
		CasDrvStartNewChannel  (0, 0, Index, mv_service_data.u16ServiceId, mv_service_data.u16VideoPid, mv_service_data.u16AudioPid);
		// printf("******************  CS_MW_PlayServiceByIdx  : SiStartLiveSearch!!\n");
		SiStartLiveSearch(0, 0, mv_service_data.u16ServiceId);
	}
	// MW_PlayProgram( mv_service_data );

	LastServixeIdx=Index;
	// printf(" TUNER SETTING END\n");
	return TRUE;
}

BOOL MV_MW_StopService(void)
{
	VideoReplayOn = 0;  // by kb : 20100406
	OpenSVC = FALSE;
	b8PlayService = FALSE;
	//LastServixeIdx=0xFFFF;

	/* 2010.09.04 By KB KIm For New SI */
	SiStopLiveSearch();

    //CS_MW_EPG_Close();
    CS_Eit_Stop();
	CS_DT_Stop();
    CS_AV_ProgramStop();
    CS_AV_VideoBlank();

	if ( mv_service_data.u8TvRadio == RADIO_SERVICE )
	    CS_AV_Play_IFrame(RADIO_BACK);
	else
		CS_AV_Play_IFrame(BOOT_LOGO);

    ResetStreamTable();
    CS_FE_StopScan();
    return TRUE;
}

BOOL CS_MW_StopService(BOOL blank)
{
	VideoReplayOn = 0;  // by kb : 20100406
	OpenSVC = FALSE;
	b8PlayService = FALSE;
	//LastServixeIdx=0xFFFF;

	/* For Cas By Jacob 10Feb.2011 */
	CasDrvStopChannel (0);

	/* 2010.09.04 By KB KIm For New SI */
	SiStopLiveSearch();

	//CS_MW_EPG_Close();
	/*  By kb : 20100403
	CS_Eit_Stop();
	*/
	CS_AV_ProgramStop();
	/*  By kb : 20100403
	CS_AV_VideoBlank();
	*/
	ResetStreamTable();
	CS_FE_StopScan();
	if (blank)
	{
		CS_AV_VideoBlank();
	}
	return TRUE;
}

BOOL MV_MW_StartService(U16 u16Chindex)
{
    if ( b8PlayService == FALSE )
		CS_MW_PlayServiceByIdx(u16Chindex, RE_TUNNING);

    return TRUE;
}

BOOL MV_Get_b8PlayService(void)
{
	return b8PlayService;
}

/* By KB Kim 2011.06.02 */
void CS_MW_SwitchVideoFreeze(void)
{
	CS_MW_IsVideoFreezed = TRUE;
	CS_AV_VideoFreeze();

	return;
}

/* By KB Kim 2011.06.02 */
void CS_MW_SwitchVideoUnFreeze(void)
{
	CS_MW_IsVideoFreezed = FALSE;

	if (mv_service_data.u8VideoType == eCS_AV_VIDEO_STREAM_H264)
	{
		MvRePlayVideo();
	}
	else
	{
		CS_AV_VideoUnfreeze();
	}

	return;
}

BOOL CS_MW_SetSmallWindow(U16 X,U16 Y,U16 W,U16 H)
{
	tCS_AV_VideoRect vid_rect;
	vid_rect.x = X;
	vid_rect.y = Y;
	vid_rect.w = W;
	vid_rect.h = H;
	CS_AV_VideoScalor(&vid_rect);
	//  printf("CS_MW_SetSmallWindow----\n");
	return(TRUE);

}

BOOL CS_MW_SetNormalWindow(void)
{
//	CSVID_Rect 	src, dst;

/*
	printf("@@@@@@@@@@@@@CS_MW_SetNormalWindow----\n");

	memset( &src, 0, sizeof(CSVID_Rect));
	dst.top=0;
	dst.left=0;
	dst.right=1280;
	dst.bottom=720;
	AV_SetOutputPostion(&src,&dst);
*/
	CS_AV_VideoScalor(NULL);
	return(TRUE);
}

