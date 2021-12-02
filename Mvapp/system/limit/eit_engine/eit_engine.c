#include "linuxos.h"

#include "db_builder.h"
#include "eit_engine.h"
#include "eit_engine_priv.h"
#include "mvosapi.h"
/* 2010.09.04 By KB KIm For New SI */
// #include "demux.h"
#include "tableApi.h"
#include "mvutil.h"

// #define EIT_ENGINE_DEBUG_ON
// #define EPG_DATA_CAPTURE

#ifdef EIT_ENGINE_DEBUG_ON
#define EIT_printf(fmt, args...)   printf("Info:*** Function: %s : " fmt,  __FUNCTION__, ##args)
#else
#define EIT_printf(fmt, args...)
#endif

/* 2010.09.04 By KB KIm For New SI */
static U8 EitTableIfoId;

static CSOS_Semaphore_t 		     *sem_EIT_Manage_Access = NULL;
static tCS_EIT_NotificationCallBack   EIT_CallBack = NULL;
static tCS_EIT_ServiceList            EIT_ServiceList;

#if 0 //by kb : 20100404
#define 		EIT_free(p)			\
{			\
			if(p!=NULL)\
				{\
				CSOS_DeallocateMemory(NULL,p);\
				p=NULL;\
				}\
}
#else
#define  EIT_malloc(p) OsMemoryAllocate((int)p)
#define  EIT_free(p) OsMemoryFree((void *)p)
#endif

extern void MV_OS_Get_Time_to_MJD_UTC_Date_Time(U16 *u16MJD, U16 *u16UTC, tCS_DT_Date *stDate, tCS_DT_Time *stTime);

/* 2010.09.04 By KB KIm For New SI */
// static void EIT_DMXCallBack (void* userparam, DB_FilterHandle  handle, BOOL timedout, U8 const* data, U32 size);
void EIT_DMXCallBack (U8 tableInfoId, U8 *data, U32 size);
U32 EIT_CopyText(char *dst, char *src, U32 length);

U32 EIT_CopyText(char *dst, char *src, U32 length)
{
	U32 textLength;
	U16 code;
	
	/* By kb : 20100404 */
#if 0
	while (length--) 
	{
		*dst++ = *src++;
	}
	*dst++ = '\0';
	*dst   = '\0';
#else
	textLength = SiCodeConverter(dst, src, length, &code);
	// printf("EIT_CopyText : Stream Length [%d  0x%02X] , Test Length[%d 0x%02X]\n", length, src[0], textLength, dst[0]);

	dst += textLength;
	*dst++ = 0x0d;
	*dst   = 0x0a;
	textLength += 2;

	return textLength;
#endif

}

U32 EIT_CopyString(char *dst, char *src, U32 length)
{
	U32 textLength;
	U16 code;
	
	textLength = SiCodeConverter(dst, src, length, &code);
	// printf("EIT_CopyText : Stream Length [%d  0x%02X] , Test Length[%d 0x%02X]\n", length, src[0], textLength, dst[0]);
	dst += textLength;
	*dst++ = '\0';
	textLength++;

	return textLength;
}

BOOL CS_EIT_Init (void)
{
    sem_EIT_Manage_Access=CSOS_CreateSemaphoreFifo(NULL,1);
	//CS_SI_EitQueue=CSOS_CreateMessageQueue("Cssieit", sizeof(EitQueueData),20);
	if(sem_EIT_Manage_Access==NULL)
	{
		EIT_printf("Eit:Create Semaphore fail\n");
		return FALSE;
	}

	EIT_ServiceList.TP_Index         = 0xFFFF;
    EIT_ServiceList.eit_service_list = NULL;

	/* 2010.09.04 By KB KIm For New SI */
	EitTableIfoId = TABLE_INFO_NUMBER;

    return TRUE;
}

/* 2010.09.04 By KB KIm For New SI */
tCS_EIT_Error CS_Eit_Stop(void)
{
	/* 2010.09.04 By KB KIm For New SI */
	if (EitTableIfoId < TABLE_INFO_NUMBER)
	{
		SiStopLiveSection(EitTableIfoId);
		EitTableIfoId = TABLE_INFO_NUMBER;
	}

	return eCS_EIT_NO_ERROR;
}

/* 2010.09.04 By KB KIm For New SI */

tCS_EIT_Error CS_Eit_Start(U16  tpindex, tCS_EIT_Table_Type_t type, U8 version)
{
    BOOL error;
    U8   match[16];
    U8   mask[16];
    U8   notMatchMask[16];

	CS_Eit_Stop();

    memset(match, 0, 16);
    memset(mask , 0, 16);
    memset(notMatchMask , 0, 16);

    if(EIT_ServiceList.TP_Index != tpindex)
	{
		CS_EIT_FreeDatabase();
		EIT_ServiceList.TP_Index = tpindex;
	}

    switch(type)
    {
		case eCS_EIT_ACTUAL_PF:
			match[0] = SI_EIT_PF_ACTUAL_TABLE_ID;
			mask[0]  = 0xFF;
			if (version < INVALID_VERSION_NUMBER)
			{
				match[3]        = (version << 1) & VERSION_NUMBER_MASK;
				mask[3]         = 0;
				notMatchMask[3] = VERSION_NUMBER_MASK;
				error = SiStartLiveSection(&EitTableIfoId,
					                        TABLE_EIT_PID,
					                        SI_EIT_PF_ACTUAL_TABLE_ID,
					                        0,
					                        0,
					                        1,
					                        1,
					                        match,
					                        mask,
					                        notMatchMask,
					                        EIT_DMXCallBack);
			}
			else
			{
				error = SiStartLiveSection(&EitTableIfoId,
					                        TABLE_EIT_PID,
					                        SI_EIT_PF_ACTUAL_TABLE_ID,
					                        0,
					                        0,
					                        1,
					                        1,
					                        match,
					                        mask,
					                        NULL,
					                        EIT_DMXCallBack);
			}

			if ((error) || (EitTableIfoId >= TABLE_INFO_NUMBER))
			{
				return eCS_EIT_ERROR;
			}
			break;
		case eCS_EIT_ACTUAL_ALL :
			#if 1
			match[0] = SI_EIT_PF_ACTUAL_TABLE_ID;
			mask[0]  = 0xFF;
			match[8] = SI_EIT_SC_ACTUAL_FIRST_TABLE_ID;
			mask[8]  = 0xF0;
			error = SiStartLiveSection(&EitTableIfoId,
				                        TABLE_EIT_PID,
				                        SI_EIT_PF_ACTUAL_TABLE_ID,
				                        0,
				                        0,
				                        2,
				                        1,
				                        match,
				                        mask,
				                        NULL,
				                        EIT_DMXCallBack);
			#else
			match[0] = SI_EIT_SC_ACTUAL_FIRST_TABLE_ID;
			mask[0]  = 0xF0;
			error = SiStartLiveSection(&EitTableIfoId,
				                        TABLE_EIT_PID,
				                        SI_EIT_PF_ACTUAL_TABLE_ID,
				                        0,
				                        0,
				                        1,
				                        1,
				                        match,
				                        mask,
				                        NULL,
				                        EIT_DMXCallBack);
			#endif

			if ((error) || (EitTableIfoId >= TABLE_INFO_NUMBER))
			{
				return eCS_EIT_ERROR;
			}

			break;
		case eCS_EIT_ACTUAL_SCHEDULE:
		default:
			match[0] = SI_EIT_SC_ACTUAL_FIRST_TABLE_ID;
			mask[0]  = 0xF0;
			error = SiStartLiveSection(&EitTableIfoId,
				                        TABLE_EIT_PID,
				                        SI_EIT_SC_ACTUAL_FIRST_TABLE_ID,
				                        0,
				                        0,
				                        1,
				                        1,
				                        match,
				                        mask,
				                        NULL,
				                        EIT_DMXCallBack);

			if ((error) || (EitTableIfoId >= TABLE_INFO_NUMBER))
			{
				return eCS_EIT_ERROR;
			}

			break;
        }

    return eCS_EIT_NO_ERROR;
}

tCS_EIT_Error EIT_GetService(U16  service_id, eit_service_t **return_program)
{
	eit_service_t *service;

	if (return_program == NULL) 
	{
		return eCS_EIT_BAD_PARAMETER;
	}
	
	for (service = EIT_ServiceList.eit_service_list ; service != NULL ; service = service->next) 
	{
		if (service->service_id == service_id) 
		{
			*return_program = service;
			return eCS_EIT_NO_ERROR;
		}
	}

	return eCS_EIT_NOT_DONE;
}


tCS_EIT_Error  EIT_AddService(U16 service_id, eit_service_t **new_program)
{
	eit_service_t *service;
	if (new_program == NULL) 
	{
		return eCS_EIT_BAD_PARAMETER;
	}

	service = (eit_service_t *)EIT_malloc(sizeof(eit_service_t));
	if (service== NULL) 
	{
		*new_program = NULL;
		return eCS_EIT_NOT_DONE;
	}

	service->service_id = service_id;
    service->eit_pf_mask = 0;
	service->eit_pf_version = kCS_EIT_NO_VERSION;
	memset(service->eit_sc_complete, 0x00, MV_EIT_NUMBER_OF_TID);
	memset(service->eit_sc_version, kCS_EIT_NO_VERSION, MV_EIT_NUMBER_OF_TID);
	memset(service->eit_sc_section, kCS_EIT_NO_VERSION, MV_EIT_NUMBER_OF_TID);

	service->present_event   = NULL;
	service->following_event = NULL;
	service->sc_event_list   = NULL;
	
	service->next = EIT_ServiceList.eit_service_list;
	EIT_ServiceList.eit_service_list =  service;
	*new_program = service;

	return eCS_EIT_NO_ERROR;
}

void EIT_FreeEvent(eit_event_t *event)
{
	if (event != NULL) 
	{
		event->event_name_length     = 0;
		event->short_desc_length     = 0;
		event->extended_desc_length  = 0;

		if(event->event_name != NULL)
		{
			EIT_free(event->event_name);
		}
		if (event->short_description != NULL) 
		{
			EIT_free(event->short_description);
		}

		if (event->extended_description != NULL) 
		{
			EIT_free(event->extended_description);
		}
	}
}

tCS_EIT_Error  EIT_FreePresentEvent(eit_service_t *service)
{
	if (service == NULL) 
	{
		return eCS_EIT_BAD_PARAMETER;
	}

	if (service->present_event != NULL)
	{
		EIT_FreeEvent(service->present_event);
		EIT_free(service->present_event);

		service->present_event = NULL;
	}
    
	return eCS_EIT_NO_ERROR;
}

tCS_EIT_Error EIT_FreeFollowingEvent(eit_service_t *service)
{
	if (service == NULL) 
	{
		return eCS_EIT_BAD_PARAMETER;
	}


	if (service->following_event != NULL)
	{
		EIT_FreeEvent(service->following_event);
		EIT_free(service->following_event);

		service->following_event = NULL;
	}
    
	return eCS_EIT_NO_ERROR;
}

tCS_EIT_Error EIT_FreeScheduleEvents(eit_service_t *service)
{
	eit_event_t *current;
	eit_event_t *listhead;

	if (service == NULL) 
	{
		return eCS_EIT_BAD_PARAMETER;
	}

	listhead = service->sc_event_list;
	while (listhead != NULL) 
	{
		current = listhead;
		listhead = listhead->next;
		EIT_FreeEvent(current);
		EIT_free(current);
		current = NULL;
	}
	service->sc_event_list = NULL;

	return eCS_EIT_NO_ERROR;
}

void EIT_SectionEitDescriptors(U8 const *buffer, U16 totalLength, eit_event_t *info)
{
	U8 *data;
	U8  tag;
	U8  length;
	U8  point;
	U8  loopLength;
	U8  subLoopLength;
	U8  extDescNumber;
	U8  extDescLastNumber;
	U16 code;               /* For EPG Display Problem By KB Kim : 2011.08.17 */
	U32 textLength;

	data = (U8 *)buffer;

	if (buffer == NULL)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		OsDebugPrtf("EIT_SectionEitDescriptors Error : Input Data Error!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		return;
	}

	if (totalLength < 2)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		OsDebugPrtf("EIT_SectionEitDescriptors Error : Data Length[%d] Error!\n", totalLength);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		return;
	}
	
	if (info == NULL)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		OsDebugPrtf("EIT_SectionEitDescriptors Error : Info buffer Error!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		return;
	}

	while (totalLength > 0)
	{
		/* Start to check Descriptor loop */
		tag    = *data++;
		length = *data++;

		if (totalLength < (U16)(length+ 2))
		{
#ifdef EIT_ENGINE_DEBUG_ON
			OsDebugPrtf("EIT_SectionEitDescriptors Error : Remaind Length[%d] is too small [%d]!\n", totalLength, (length+ 2));
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			return;
		}

		totalLength -= (U16)(length+ 2);
		switch(tag)
		{
			case SHORT_EVENT_DESCRIPTOR     :
				if (length < 6)
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : SHORT_EVENT_DESCRIPTOR Length[%] is too small !\n", length);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					return;
				}

				point = 0;
				memcpy(info->short_desc_language, data, 3);
				point += 3;
				loopLength = data[point++];  /* Event Name Length */
				if (length < (point + loopLength))
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : SHORT_EVENT_DESCRIPTOR Length[%] is smaller than looplength[%d] 1!\n", length, (point + loopLength));
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					break;
				}
				
				if (loopLength > 0)
				{
					if (info->event_name != NULL)
					{
						EIT_free(info->event_name);
					}
					info->event_name = EIT_malloc((loopLength + 2));
					if (info->event_name == NULL)
					{
#ifdef EIT_ENGINE_DEBUG_ON
						OsDebugPrtf("EIT_SectionEitDescriptors Error : SHORT_EVENT_DESCRIPTOR Can not allocate event name!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
						return;
					}

					textLength = EIT_CopyString(info->event_name, (data + point), loopLength);
					info->event_name_length = textLength;
					point += loopLength;
				}
				loopLength = data[point++];  /* Event Short Text Length */
				if (length < (point + loopLength))
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : SHORT_EVENT_DESCRIPTOR Length[%] is smaller than looplength[%d] 2!\n", length, (point + loopLength));
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					break;
				}
				if (loopLength > 0)
				{
					if (info->short_description != NULL)
					{
						EIT_free(info->short_description);
					}
					info->short_description = EIT_malloc((loopLength + 2));
					if (info->short_description == NULL)
					{
#ifdef EIT_ENGINE_DEBUG_ON
						OsDebugPrtf("EIT_SectionEitDescriptors Error : SHORT_EVENT_DESCRIPTOR Can not allocate short Description!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
						return;
					}

					textLength = EIT_CopyString(info->short_description, (data + point), loopLength);
					info->short_desc_length = textLength;
					point += loopLength;
				}
				break;
			case EXTENDED_EVENT_DESCRIPTOR  :
				point = 0;
				extDescNumber     = (data[0] >> 4) & 0x0F;
				extDescLastNumber = data[0] & 0x0F;
				if (extDescNumber == 0)
				{
					/* First Discriptor in this event */
					/* Allocate Extend Data Buffer    */
					if (info->extended_description != NULL)
					{
						EIT_free(info->extended_description);
					}

					/* For EPG Dead problem By KB Kim 2011.08.17 */
					info->extended_description = EIT_malloc((extDescLastNumber + 2) * 256);  // reserve max Descriptor Size + 1 for some error stream
					info->extended_desc_length = 0;
				}
				if (info->extended_description == NULL)
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : EXTENDED_EVENT_DESCRIPTOR Can not allocate extended Description[%d]!\n", extDescNumber);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					return;
				}

				/* For EPG Dead problem By KB Kim 2011.08.17 */
				if (extDescNumber > (extDescLastNumber + 1))
				{
					break;
				}
				
				point++;
				memcpy(info->extended_desc_language, data + point, 3);
				point += 3;
				loopLength = data[point++];  /* Length of Item */
				if (length < (point + loopLength))
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : EXTENDED_EVENT_DESCRIPTOR0 Length[%] is smaller than looplength[%d]!\n", length, (point + loopLength));
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					break;
				}
				while (loopLength > 0)
				{
					subLoopLength = data[point++]; /* Length of Item */
					loopLength--;
					if (loopLength < subLoopLength)
					{
#ifdef EIT_ENGINE_DEBUG_ON
						OsDebugPrtf("EIT_SectionEitDescriptors Error : EXTENDED_EVENT_DESCRIPTOR subLoopLength[%d] bigger than remaind data[%d]!\n", subLoopLength, loopLength);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
						point += loopLength;
						break;
					}
					if (subLoopLength > 0)
					{
						textLength = (U16)EIT_CopyText((info->extended_description + info->extended_desc_length), (data + point), subLoopLength);
						info->extended_desc_length += textLength;  /* Extra mark "0x0d", "0x0a" : CR, LF*/
						point += subLoopLength;
						loopLength -= subLoopLength;
					}
				}
				loopLength = data[point++];
				if (length < (point + loopLength))
				{
#ifdef EIT_ENGINE_DEBUG_ON
					OsDebugPrtf("EIT_SectionEitDescriptors Error : EXTENDED_EVENT_DESCRIPTOR1 Length[%] is smaller than looplength[%d]!\n", length, (point + loopLength));
#endif // #ifdef EIT_ENGINE_DEBUG_ON
					break;
				}
				if (loopLength > 0)
				{
					/* For EPG Display Problem By KB Kim : 2011.08.17 */
					textLength = SiCodeConverter((info->extended_description + info->extended_desc_length), (data + point), loopLength, &code);
					// textLength = EIT_CopyText((info->extended_description + info->extended_desc_length), (data + point), loopLength);
					info->extended_desc_length += textLength;
					point += loopLength;
				}
				break;
			case CONTENT_DESCRIPTOR         :
				info->contentData = data[0];
				break;
			case PARENTAL_RATING_DESCRIPTOR :
#ifdef EIT_ENGINE_DEBUG_ON
				OsDebugPrtf("EIT_SectionEitDescriptors Info : PARENTAL_RATING_DESCRIPTOR [%c%c%c]!\n",
					data[0], data[1], data[2]);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
				info->parental_rating = data[3] + 3;
				break;
			default :
				break;
		}

		data += length;
	}
	
}

tCS_EIT_Error  Eit_AddSCEvent(eit_service_t *service, eit_event_t *new_event)
{
	eit_event_t *current_event;

	if (new_event == NULL)
	{
		return eCS_EIT_BAD_PARAMETER;
	}

	current_event   = service->sc_event_list;
	new_event->prev = NULL;

	while (current_event != NULL)
	{
		if ((new_event->date_mjd < current_event->date_mjd) ||
			((new_event->date_mjd == current_event->date_mjd) &&
			 (new_event->start_time_utc < current_event->start_time_utc)))
		{
			new_event->prev = current_event->prev;
			new_event->next = current_event;
			current_event->prev = new_event;
			current_event = new_event->prev;

			if (current_event != NULL)
			{
				current_event->next = new_event;
			}
			else
			{
				/* New list will be First of the list */
				service->sc_event_list = new_event;
			}

			return eCS_EIT_NO_ERROR;
		}
		else if ((new_event->date_mjd == current_event->date_mjd) &&
			     (new_event->start_time_utc == current_event->start_time_utc))
		{
			/* Same Data do nothing */
#ifdef EIT_ENGINE_DEBUG_ON
			printf("Eit_AddSCEvent Error : Same Event!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			return eCS_EIT_BAD_PARAMETER;
		}
		
		new_event->prev = current_event;
		current_event = current_event->next;
	}

	/* End of List */
	if (new_event->prev != NULL)
	{
		current_event = new_event->prev;
		current_event->next = new_event;
	}
	else
	{
		/* First of the list */
		service->sc_event_list = new_event;
	}
	new_event->next = NULL;

	return eCS_EIT_NO_ERROR;
}

void CS_EIT_FreeDatabase(void)
{
	eit_service_t *current;
	eit_service_t *listhead;
	
	CSOS_WaitSemaphore(sem_EIT_Manage_Access); 
	listhead = EIT_ServiceList.eit_service_list;
	
	while (listhead != NULL) 
	{
		current = listhead;
		listhead = listhead->next;

		EIT_FreePresentEvent(current);
		EIT_FreeFollowingEvent(current);
		EIT_FreeScheduleEvents(current);

		EIT_free(current);
		current = NULL;
	}
	
	EIT_ServiceList.eit_service_list = NULL;
	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
}

static BOOL check_sc_complete(eit_service_t *current)
{
	return FALSE;
}

static BOOL check_pf_complete(U8 pf_mask)
{
    if((pf_mask&(kCS_EIT_PRESENT_MASK|kCS_EIT_FOLLOWING_MASK))
                    ==(kCS_EIT_PRESENT_MASK|kCS_EIT_FOLLOWING_MASK))
        return TRUE;
    else
        return FALSE;
}

#if 0
static tCS_EIT_Error pre_check_and_match_eit_lang(U8 const* buffer, S32 total_length, char * short_lang, char * ext_lang)
{
    const U8 *data;
    U8  tag;
    U8  length;
    U8  descriptor_number;
    U8  last_descriptor_number;

    char lang_trigram[4];
    char first_short_desc_language[4];
    char first_extended_desc_language[4];

    BOOL Isfound_short_desc_language = FALSE;
    BOOL Isfound_extended_desc_language = FALSE;

    if (buffer == NULL) 
    {
        return eCS_EIT_ERROR;
    }

    memset(lang_trigram, 0, 4);
    memset(first_short_desc_language, 0, 4);
    memset(first_extended_desc_language, 0, 4);

    while (total_length > 0) 
    {

        tag    = *buffer++;
        length = *buffer++;

       if (length == 0) 
       {
            continue;
        }

        data          = buffer;
        buffer       += length;
        total_length -= (length + 2);

        switch (tag) 
        {
                case SHORT_EVENT_DESCRIPTOR:
                {         
                        if(!Isfound_short_desc_language)
                        {
                            lang_trigram[0] = data[0];
                            lang_trigram[1] = data[1];
                            lang_trigram[2] = data[2];
                            lang_trigram[3] = 0;

                            if(first_short_desc_language[0] == 0)
                            {
                                memcpy(first_short_desc_language, lang_trigram, 4);
                            }
							
                            if(!memcmp(CS_DBU_GetMenuLanguage(), lang_trigram,3))
                            {
                                memcpy(short_lang, lang_trigram, 4);

                                Isfound_short_desc_language = TRUE;
                         
                            }
                        }
                        break;
                }

                case EXTENDED_EVENT_DESCRIPTOR:
                {
                    if(!Isfound_extended_desc_language)
                    {
                            descriptor_number     = (U8)((data[0]&0xf0)>>4);
                            last_descriptor_number = (U8)(data[0]&0x0f);
                            data++;

                            lang_trigram[0] = data[0];
                            lang_trigram[1] = data[1];
                            lang_trigram[2] = data[2];
                            lang_trigram[3] = 0;

                            if(first_extended_desc_language[0] == 0)
                                {
                                    memcpy(first_extended_desc_language, lang_trigram, 4);
                                }

                            if(!memcmp(CS_DBU_GetMenuLanguage(), lang_trigram,3))
                            {
                                memcpy(ext_lang, lang_trigram, 4);

                                Isfound_extended_desc_language = TRUE;
                         
                            }
                                
                    }
            }
                break;

                default:
                    break;
          }
    }

    if(!Isfound_short_desc_language)
        {
             memcpy(short_lang, first_short_desc_language, 4);
        }

    if(!Isfound_extended_desc_language)
        {
             memcpy(ext_lang, first_extended_desc_language, 4);
        }

     return eCS_EIT_NO_ERROR;
    
}
#endif

/* 2010.09.04 By KB KIm For New SI */
#ifdef EIT_ENGINE_DEBUG_ON
U8 testEPG[] = {
0x51, 0xF8, 0xB0, 0x2E, 0xFE, 0xE5, 0xB8, 0xB8, 0x04, 0x41, 0x00, 0x01, 0xB8, 0x51, 0x8C, 0x24,
0xD9, 0xD3, 0x21, 0x25, 0x00, 0x00, 0x25, 0x00, 0x02, 0x16, 0x4D, 0x0E, 0x67, 0x65, 0x72, 0x09,
0x05, 0x56, 0x6F, 0x78, 0x74, 0x6F, 0x75, 0x72, 0x73, 0x00, 0x4E, 0xFF, 0x01, 0x67, 0x65, 0x72,
0x00, 0xF9, 0x05, 0x44, 0x6F, 0x77, 0x6E, 0x20, 0x55, 0x6E, 0x64, 0x65, 0x72, 0x20, 0x52, 0x65,
0x70, 0x6F, 0x72, 0x74, 0x65, 0x72, 0x20, 0x49, 0x3A, 0x20, 0x57, 0x65, 0x73, 0x74, 0x20, 0x41,
0x75, 0x73, 0x74, 0x72, 0x61, 0x6C, 0x69, 0x61, 0x8A, 0x4D, 0x6F, 0x64, 0x65, 0x72, 0x61, 0x74,
0x69, 0x6F, 0x6E, 0x3A, 0x20, 0x4B, 0x65, 0x72, 0x73, 0x74, 0x69, 0x6E, 0x20, 0x4B, 0x69, 0x72,
0x63, 0x68, 0x68, 0x6F, 0x66, 0x65, 0x72, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x44, 0x69, 0x72, 0x6B,
0x20, 0x4C, 0x6F, 0x74, 0x68, 0x6D, 0x61, 0x6E, 0x6E, 0x2E, 0x20, 0x44, 0x61, 0x73, 0x20, 0x65,
0x72, 0x73, 0x74, 0x65, 0x20, 0x4E, 0x61, 0x63, 0x68, 0x77, 0x75, 0x63, 0x68, 0x73, 0x72, 0x65,
0x70, 0x6F, 0x72, 0x74, 0x65, 0x72, 0x74, 0x65, 0x61, 0x6D, 0x20, 0x4B, 0x65, 0x72, 0x73, 0x74,
0x69, 0x6E, 0x20, 0x4B, 0x69, 0x72, 0x63, 0x68, 0x68, 0x6F, 0x66, 0x65, 0x72, 0x20, 0x75, 0x6E,
0x64, 0x20, 0x44, 0x69, 0x72, 0x6B, 0x20, 0x4C, 0x6F, 0x74, 0x68, 0x6D, 0x61, 0x6E, 0x6E, 0x20,
0x62, 0x65, 0x72, 0x69, 0x63, 0x68, 0x74, 0x65, 0x74, 0x20, 0x61, 0x75, 0x73, 0x20, 0x57, 0x65,
0x73, 0x74, 0x61, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6C, 0x69, 0x65, 0x6E, 0x21, 0x20, 0x44, 0x69,
0x65, 0x20, 0x54, 0x56, 0x2D, 0x48, 0x65, 0x6C, 0x64, 0x65, 0x6E, 0x20, 0x73, 0x63, 0x68, 0x6C,
0x61, 0x67, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20, 0x77, 0x61, 0x63, 0x6B, 0x65, 0x72,
0x20, 0x64, 0x75, 0x72, 0x63, 0x68, 0x73, 0x20, 0x74, 0x69, 0x65, 0x66, 0x73, 0x74, 0x65, 0x20,
0x4F, 0x75, 0x74, 0x62, 0x61, 0x63, 0x6B, 0x2C, 0x20, 0x72, 0x75, 0x4E, 0xFF, 0x11, 0x67, 0x65,
0x72, 0x00, 0xF9, 0x05, 0x6E, 0x64, 0x20, 0x75, 0x6D, 0x20, 0x64, 0x69, 0x65, 0x20, 0x47, 0x6F,
0x6C, 0x64, 0x6D, 0x69, 0x6E, 0x65, 0x6E, 0x2D, 0x4D, 0x65, 0x74, 0x72, 0x6F, 0x70, 0x6F, 0x6C,
0x65, 0x20, 0x4B, 0x61, 0x6C, 0x67, 0x6F, 0x6F, 0x72, 0x6C, 0x69, 0x65, 0x20, 0x75, 0x6E, 0x64,
0x20, 0x64, 0x69, 0x65, 0x20, 0x54, 0x6F, 0x70, 0x73, 0x70, 0x6F, 0x74, 0x73, 0x20, 0x64, 0x65,
0x73, 0x20, 0x49, 0x6E, 0x64, 0x69, 0x73, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x4F, 0x7A, 0x65, 0x61,
0x6E, 0x73, 0x20, 0x72, 0x75, 0x6E, 0x64, 0x20, 0x75, 0x6D, 0x20, 0x50, 0x65, 0x72, 0x74, 0x68,
0x20, 0x2D, 0x20, 0x6F, 0x68, 0x6E, 0x65, 0x20, 0x52, 0x75, 0x68, 0x65, 0x70, 0x61, 0x75, 0x73,
0x65, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x69, 0x6D, 0x6D, 0x65, 0x72, 0x20, 0x64, 0x69, 0x63, 0x68,
0x74, 0x20, 0x67, 0x65, 0x66, 0x6F, 0x6C, 0x67, 0x74, 0x20, 0x76, 0x6F, 0x6D, 0x20, 0x56, 0x6F,
0x78, 0x74, 0x6F, 0x75, 0x72, 0x73, 0x2D, 0x4B, 0x61, 0x6D, 0x65, 0x72, 0x61, 0x74, 0x65, 0x61,
0x6D, 0x2E, 0x20, 0x41, 0x75, 0x73, 0x67, 0x65, 0x77, 0xE4, 0x68, 0x6C, 0x74, 0x20, 0x77, 0x6F,
0x72, 0x64, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x6E, 0x64, 0x20, 0x64, 0x69, 0x65, 0x20, 0x62, 0x65,
0x69, 0x64, 0x65, 0x6E, 0x20, 0x62, 0x65, 0x69, 0x20, 0x65, 0x69, 0x6E, 0x65, 0x6D, 0x20, 0x57,
0x65, 0x74, 0x74, 0x62, 0x65, 0x77, 0x65, 0x72, 0x62, 0x20, 0x69, 0x6D, 0x20, 0x49, 0x6E, 0x74,
0x65, 0x72, 0x6E, 0x65, 0x74, 0x20, 0x6D, 0x69, 0x74, 0x20, 0x61, 0x6E, 0x73, 0x63, 0x68, 0x6C,
0x69, 0x65, 0xDF, 0x65, 0x6E, 0x64, 0x65, 0x6D, 0x20, 0x2E, 0x2E, 0x2E, 0x54, 0x02, 0x81, 0x00,
0x8C, 0x4C, 0xD9, 0xD3, 0x21, 0x50, 0x00, 0x00, 0x25, 0x00, 0x02, 0x0F, 0x4D, 0x0E, 0x67, 0x65,
0x72, 0x09, 0x05, 0x56, 0x6F, 0x78, 0x74, 0x6F, 0x75, 0x72, 0x73, 0x00, 0x4E, 0xFF, 0x00, 0x67,
0x65, 0x72, 0x00, 0xF9, 0x05, 0x44, 0x6F, 0x77, 0x6E, 0x20, 0x55, 0x6E, 0x64, 0x65, 0x72, 0x20,
0x52, 0x65, 0x70, 0x6F, 0x72, 0x74, 0x65, 0x72, 0x20, 0x49, 0x49, 0x3A, 0x20, 0x57, 0x65, 0x73,
0x74, 0x61, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6C, 0x69, 0x65, 0x6E, 0x20, 0x26, 0x20, 0x51, 0x75,
0x65, 0x65, 0x6E, 0x73, 0x6C, 0x61, 0x6E, 0x64, 0x8A, 0x4D, 0x6F, 0x64, 0x65, 0x72, 0x61, 0x74,
0x69, 0x6F, 0x6E, 0x3A, 0x20, 0x4B, 0x65, 0x72, 0x73, 0x74, 0x69, 0x6E, 0x20, 0x4B, 0x69, 0x72,
0x63, 0x68, 0x68, 0x6F, 0x66, 0x65, 0x72, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x44, 0x69, 0x72, 0x6B,
0x20, 0x4C, 0x6F, 0x74, 0x68, 0x6D, 0x61, 0x6E, 0x6E, 0x2E, 0x20, 0x44, 0x69, 0x65, 0x20, 0x52,
0x65, 0x69, 0x73, 0x65, 0x20, 0x64, 0x65, 0x73, 0x20, 0x65, 0x72, 0x73, 0x74, 0x65, 0x6E, 0x20,
0x4E, 0x61, 0x63, 0x68, 0x77, 0x75, 0x63, 0x68, 0x73, 0x72, 0x65, 0x70, 0x6F, 0x72, 0x74, 0x65,
0x72, 0x2D, 0x54, 0x65, 0x61, 0x6D, 0x73, 0x20, 0x4B, 0x65, 0x72, 0x73, 0x74, 0x69, 0x6E, 0x20,
0x4B, 0x69, 0x72, 0x63, 0x68, 0x68, 0x6F, 0x66, 0x65, 0x72, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x44,
0x69, 0x72, 0x6B, 0x20, 0x4C, 0x6F, 0x74, 0x68, 0x6D, 0x61, 0x6E, 0x6E, 0x20, 0x67, 0x65, 0x68,
0x74, 0x20, 0x77, 0x65, 0x69, 0x74, 0x65, 0x72, 0x20, 0x64, 0x75, 0x72, 0x63, 0x68, 0x20, 0x57,
0x65, 0x73, 0x74, 0x61, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6C, 0x69, 0x65, 0x6E, 0x21, 0x20, 0x4E,
0x61, 0x63, 0x68, 0x64, 0x65, 0x6D, 0x20, 0x64, 0x69, 0x65, 0x20, 0x54, 0x56, 0x2D, 0x48, 0x65,
0x6C, 0x64, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20, 0x73, 0x63, 0x68, 0x4E, 0xF8, 0x10,
0x67, 0x65, 0x72, 0x00, 0xF2, 0x05, 0x6F, 0x6E, 0x20, 0x65, 0x69, 0x6E, 0x69, 0x67, 0x65, 0x6E,
0x20, 0x77, 0x65, 0x73, 0x74, 0x61, 0x75, 0x73, 0x74, 0x72, 0x61, 0x6C, 0x69, 0x73, 0x63, 0x68,
0x65, 0x6E, 0x20, 0x48, 0x65, 0x72, 0x61, 0x75, 0x73, 0x66, 0x6F, 0x72, 0x64, 0x65, 0x72, 0x75,
0x6E, 0x67, 0x65, 0x6E, 0x20, 0x67, 0x65, 0x73, 0x74, 0x65, 0x6C, 0x6C, 0x74, 0x20, 0x68, 0x61,
0x62, 0x65, 0x6E, 0x2C, 0x20, 0x77, 0x69, 0x64, 0x6D, 0x65, 0x6E, 0x20, 0x73, 0x69, 0x65, 0x20,
0x73, 0x69, 0x63, 0x68, 0x20, 0x6E, 0x75, 0x6E, 0x20, 0x64, 0x65, 0x72, 0x20, 0x66, 0x61, 0x73,
0x7A, 0x69, 0x6E, 0x69, 0x65, 0x72, 0x65, 0x6E, 0x64, 0x65, 0x6E, 0x20, 0x61, 0x75, 0x73, 0x74,
0x72, 0x61, 0x6C, 0x69, 0x73, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x54, 0x69, 0x65, 0x72, 0x77, 0x65,
0x6C, 0x74, 0x2E, 0x20, 0x44, 0x6F, 0x63, 0x68, 0x20, 0x64, 0x61, 0x73, 0x20, 0x69, 0x73, 0x74,
0x20, 0x6E, 0x6F, 0x63, 0x68, 0x20, 0x6C, 0x61, 0x6E, 0x67, 0x65, 0x20, 0x6E, 0x69, 0x63, 0x68,
0x74, 0x20, 0x61, 0x6C, 0x6C, 0x65, 0x73, 0x2C, 0x20, 0x77, 0x65, 0x69, 0x74, 0x65, 0x72, 0x20,
0x67, 0x65, 0x68, 0x74, 0x27, 0x73, 0x20, 0x7A, 0x75, 0x6D, 0x20, 0x72, 0x69, 0x65, 0x73, 0x69,
0x67, 0x65, 0x6E, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x64, 0x65, 0x73, 0x20, 0x27, 0x53,
0x75, 0x72, 0x66, 0x20, 0x4C, 0x69, 0x76, 0x65, 0x20, 0x53, 0x61, 0x76, 0x69, 0x6E, 0x67, 0x20,
0x43, 0x68, 0x61, 0x6D, 0x70, 0x69, 0x6F, 0x6E, 0x73, 0x68, 0x69, 0x70, 0x27, 0x20, 0x2D, 0x20,
0x64, 0x69, 0x65, 0x20, 0x2E, 0x2E, 0x2E, 0x54, 0x02, 0x81, 0x00, 0x8C, 0x49, 0xD9, 0xD3, 0x22,
0x15, 0x00, 0x00, 0x55, 0x00, 0x02, 0x31, 0x4D, 0x2B, 0x67, 0x65, 0x72, 0x26, 0x05, 0x41, 0x75,
0x66, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x64, 0x61, 0x76, 0x6F, 0x6E, 0x20, 0x2D, 0x20, 0x4D, 0x65,
0x69, 0x6E, 0x20, 0x41, 0x75, 0x73, 0x6C, 0x61, 0x6E, 0x64, 0x73, 0x74, 0x61, 0x67, 0x65, 0x62,
0x75, 0x63, 0x68, 0x00, 0x4E, 0xFF, 0x01, 0x67, 0x65, 0x72, 0x00, 0xF9, 0x05, 0x46, 0x6F, 0x6C,
0x67, 0x65, 0x20, 0x34, 0x32, 0x2E, 0x20, 0x52, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6E, 0x62, 0x61,
0x63, 0x68, 0x20, 0x53, 0x74, 0x65, 0x65, 0x67, 0x65, 0x6E, 0x20, 0x62, 0x65, 0x69, 0x20, 0x4B,
0x61, 0x69, 0x73, 0x65, 0x72, 0x73, 0x6C, 0x61, 0x75, 0x74, 0x65, 0x72, 0x6E, 0x3A, 0x20, 0x49,
0x6E, 0x20, 0x64, 0x65, 0x6D, 0x20, 0x44, 0x6F, 0x72, 0x66, 0x20, 0x6D, 0x69, 0x74, 0x20, 0x31,
0x2E, 0x35, 0x30, 0x30, 0x20, 0x45, 0x69, 0x6E, 0x77, 0x6F, 0x68, 0x6E, 0x65, 0x72, 0x6E, 0x20,
0x66, 0xFC, 0x68, 0x6C, 0x74, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20, 0x4D, 0x61, 0x67, 0x67, 0x69,
0x65, 0x20, 0x61, 0x75, 0x73, 0x20, 0x48, 0x6F, 0x6E, 0x67, 0x6B, 0x6F, 0x6E, 0x67, 0x20, 0x6D,
0x69, 0x74, 0x74, 0x6C, 0x65, 0x72, 0x77, 0x65, 0x69, 0x6C, 0x65, 0x20, 0x72, 0x69, 0x63, 0x68,
0x74, 0x69, 0x67, 0x20, 0x77, 0x6F, 0x68, 0x6C, 0x2E, 0x20, 0x53, 0x69, 0x65, 0x20, 0x69, 0x73,
0x74, 0x20, 0x73, 0x65, 0x69, 0x74, 0x20, 0x72, 0x75, 0x6E, 0x64, 0x20, 0x66, 0xFC, 0x6E, 0x66,
0x20, 0x57, 0x6F, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x65, 0x75, 0x74, 0x73,
0x63, 0x68, 0x6C, 0x61, 0x6E, 0x64, 0x2C, 0x20, 0x62, 0x65, 0x73, 0x75, 0x63, 0x68, 0x74, 0x20,
0x65, 0x69, 0x6E, 0x65, 0x6E, 0x20, 0x53, 0x70, 0x72, 0x61, 0x63, 0x68, 0x6B, 0x75, 0x72, 0x73,
0x20, 0x75, 0x6E, 0x64, 0x20, 0x76, 0x65, 0x72, 0x62, 0x72, 0x69, 0x6E, 0x67, 0x74, 0x20, 0x67,
0x65, 0x72, 0x6E, 0x65, 0x20, 0x64, 0x69, 0x65, 0x20, 0x5A, 0x65, 0x69, 0x74, 0x20, 0x6D, 0x69,
0x74, 0x20, 0x69, 0x68, 0x72, 0x4E, 0xFD, 0x11, 0x67, 0x65, 0x72, 0x00, 0xF7, 0x05, 0x65, 0x72,
0x20, 0x47, 0x61, 0x73, 0x74, 0x66, 0x61, 0x6D, 0x69, 0x6C, 0x69, 0x65, 0x2E, 0x20, 0x41, 0x6C,
0x73, 0x20, 0x4D, 0x61, 0x67, 0x67, 0x69, 0x65, 0x20, 0x7A, 0x75, 0x6D, 0x20, 0x44, 0x61, 0x6E,
0x6B, 0x20, 0x61, 0x6E, 0x20, 0x69, 0x68, 0x72, 0x65, 0x20, 0x47, 0x61, 0x73, 0x74, 0x66, 0x61,
0x6D, 0x69, 0x6C, 0x69, 0x65, 0x20, 0x65, 0x69, 0x6E, 0x20, 0x63, 0x68, 0x69, 0x6E, 0x65, 0x73,
0x69, 0x73, 0x63, 0x68, 0x65, 0x73, 0x20, 0x45, 0x73, 0x73, 0x65, 0x6E, 0x20, 0x6B, 0x6F, 0x63,
0x68, 0x65, 0x6E, 0x20, 0x77, 0x69, 0x6C, 0x6C, 0x2C, 0x20, 0x6B, 0x6C, 0x61, 0x70, 0x70, 0x74,
0x20, 0x6C, 0x65, 0x69, 0x64, 0x65, 0x72, 0x20, 0x6E, 0x69, 0x63, 0x68, 0x74, 0x20, 0x61, 0x6C,
0x6C, 0x65, 0x73, 0x20, 0x77, 0x69, 0x65, 0x20, 0x67, 0x65, 0x70, 0x6C, 0x61, 0x6E, 0x74, 0x2E,
0x20, 0x44, 0x65, 0x6E, 0x6E, 0x20, 0x69, 0x6D, 0x20, 0x48, 0x61, 0x75, 0x73, 0x20, 0x75, 0x6E,
0x64, 0x20, 0x64, 0x65, 0x72, 0x20, 0x4B, 0xFC, 0x63, 0x68, 0x65, 0x20, 0x6B, 0x65, 0x6E, 0x6E,
0x74, 0x20, 0x73, 0x69, 0x65, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20, 0x6E, 0x6F, 0x63, 0x68, 0x20,
0x6E, 0x69, 0x63, 0x68, 0x74, 0x20, 0x72, 0x69, 0x63, 0x68, 0x74, 0x69, 0x67, 0x20, 0x61, 0x75,
0x73, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x61, 0x75, 0x63, 0x68, 0x20, 0x6D, 0x69, 0x74, 0x20, 0x64,
0x65, 0x6E, 0x20, 0x4D, 0x65, 0x6E, 0x67, 0x65, 0x6E, 0x61, 0x6E, 0x67, 0x61, 0x62, 0x65, 0x6E,
0x20, 0x68, 0x61, 0x74, 0x20, 0x73, 0x69, 0x65, 0x20, 0x73, 0x6F, 0x20, 0x69, 0x68, 0x72, 0x65,
0x20, 0x2E, 0x2E, 0x2E, 0x54, 0x02, 0x81, 0x00, 0x8C, 0x2A, 0xD9, 0xD3, 0x23, 0x10, 0x00, 0x00,
0x45, 0x00, 0x02, 0x1B, 0x4D, 0x13, 0x67, 0x65, 0x72, 0x0E, 0x05, 0x57, 0x69, 0x73, 0x73, 0x65,
0x6E, 0x73, 0x68, 0x75, 0x6E, 0x67, 0x65, 0x72, 0x00, 0x4E, 0xFF, 0x01, 0x67, 0x65, 0x72, 0x00,
0xF9, 0x05, 0x57, 0x69, 0x65, 0x20, 0x65, 0x6E, 0x74, 0x73, 0x74, 0x65, 0x68, 0x74, 0x20, 0x64,
0x69, 0x65, 0x20, 0x43, 0x72, 0x65, 0x6D, 0x65, 0x20, 0x61, 0x75, 0x66, 0x20, 0x64, 0x65, 0x6D,
0x20, 0x46, 0x65, 0x72, 0x74, 0x69, 0x67, 0x2D, 0x43, 0x61, 0x70, 0x70, 0x75, 0x63, 0x63, 0x69,
0x6E, 0x6F, 0x3F, 0x20, 0x48, 0x65, 0x6C, 0x66, 0x65, 0x6E, 0x20, 0x43, 0x6F, 0x6C, 0x61, 0x20,
0x75, 0x6E, 0x64, 0x20, 0x53, 0x61, 0x6C, 0x7A, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x65, 0x6E, 0x20,
0x77, 0x69, 0x72, 0x6B, 0x6C, 0x69, 0x63, 0x68, 0x20, 0x62, 0x65, 0x69, 0x20, 0x44, 0x75, 0x72,
0x63, 0x68, 0x66, 0x61, 0x6C, 0x6C, 0x3F, 0x20, 0x57, 0x61, 0x73, 0x20, 0x75, 0x6E, 0x74, 0x65,
0x72, 0x73, 0x63, 0x68, 0x65, 0x69, 0x64, 0x65, 0x74, 0x20, 0x54, 0x69, 0x65, 0x66, 0x6B, 0xFC,
0x68, 0x6C, 0x2D, 0x53, 0x70, 0x69, 0x6E, 0x61, 0x74, 0x20, 0x76, 0x6F, 0x6D, 0x20, 0x6D, 0x61,
0x72, 0x6B, 0x74, 0x66, 0x72, 0x69, 0x73, 0x63, 0x68, 0x65, 0x6E, 0x20, 0x47, 0x65, 0x6D, 0xFC,
0x73, 0x65, 0x3F, 0x20, 0x57, 0x69, 0x65, 0x20, 0x66, 0x75, 0x6E, 0x6B, 0x74, 0x69, 0x6F, 0x6E,
0x69, 0x65, 0x72, 0x74, 0x20, 0x65, 0x69, 0x6E, 0x65, 0x20, 0x45, 0x73, 0x70, 0x72, 0x65, 0x73,
0x73, 0x6F, 0x6D, 0x61, 0x73, 0x63, 0x68, 0x69, 0x6E, 0x65, 0x2C, 0x20, 0x75, 0x6E, 0x64, 0x20,
0x77, 0x61, 0x72, 0x75, 0x6D, 0x20, 0x73, 0x63, 0x68, 0x6D, 0x65, 0x63, 0x6B, 0x74, 0x20, 0x64,
0x65, 0x72, 0x20, 0x27, 0x6B, 0x75, 0x72, 0x7A, 0x65, 0x20, 0x53, 0x63, 0x68, 0x77, 0x61, 0x72,
0x7A, 0x65, 0x27, 0x20, 0x69, 0x6D, 0x20, 0x43, 0x61, 0x66, 0x4E, 0xFF, 0x11, 0x67, 0x65, 0x72,
0x00, 0xF9, 0x05, 0xE9, 0x20, 0x69, 0x6D, 0x6D, 0x65, 0x72, 0x20, 0x62, 0x65, 0x73, 0x73, 0x65,
0x72, 0x3F, 0x20, 0x41, 0x6C, 0x6C, 0x65, 0x73, 0x20, 0x46, 0x72, 0x61, 0x67, 0x65, 0x6E, 0x2C,
0x20, 0x64, 0x69, 0x65, 0x20, 0x73, 0x69, 0x63, 0x68, 0x20, 0x75, 0x6D, 0x20, 0x64, 0x61, 0x73,
0x20, 0x54, 0x68, 0x65, 0x6D, 0x61, 0x20, 0x45, 0x72, 0x6E, 0xE4, 0x68, 0x72, 0x75, 0x6E, 0x67,
0x20, 0x72, 0x61, 0x6E, 0x6B, 0x65, 0x6E, 0x20, 0x2D, 0x20, 0x64, 0x65, 0x6D, 0x20, 0x54, 0x68,
0x65, 0x6D, 0x61, 0x20, 0x75, 0x6E, 0x73, 0x65, 0x72, 0x65, 0x72, 0x20, 0x5A, 0x65, 0x69, 0x74,
0x2E, 0x20, 0x27, 0x57, 0x69, 0x73, 0x73, 0x65, 0x6E, 0x73, 0x68, 0x75, 0x6E, 0x67, 0x65, 0x72,
0x27, 0x20, 0x62, 0x6C, 0x69, 0x63, 0x6B, 0x74, 0x20, 0x69, 0x6E, 0x20, 0x64, 0x69, 0x65, 0x20,
0x46, 0x61, 0x62, 0x72, 0x69, 0x6B, 0x65, 0x6E, 0x20, 0x75, 0x6E, 0x64, 0x20, 0x42, 0x65, 0x74,
0x72, 0x69, 0x65, 0x62, 0x65, 0x2C, 0x20, 0x61, 0x75, 0x73, 0x20, 0x64, 0x65, 0x6E, 0x65, 0x6E,
0x20, 0x75, 0x6E, 0x73, 0x65, 0x72, 0x20, 0x45, 0x73, 0x73, 0x65, 0x6E, 0x20, 0x6B, 0x6F, 0x6D,
0x6D, 0x74, 0x2E, 0x20, 0x44, 0x61, 0x73, 0x20, 0x4D, 0x61, 0x67, 0x61, 0x7A, 0x69, 0x6E, 0x20,
0x62, 0x65, 0x67, 0x6C, 0x65, 0x69, 0x74, 0x65, 0x74, 0x20, 0x4D, 0x65, 0x6E, 0x73, 0x63, 0x68,
0x65, 0x6E, 0x2C, 0x20, 0x64, 0x69, 0x65, 0x20, 0x64, 0x61, 0x66, 0xFC, 0x72, 0x20, 0x76, 0x65,
0x72, 0x61, 0x6E, 0x74, 0x77, 0x6F, 0x72, 0x74, 0x6C, 0x69, 0x63, 0x68, 0x20, 0x73, 0x69, 0x6E,
0x64, 0x2C, 0x20, 0x64, 0x61, 0x73, 0x73, 0x20, 0x2E, 0x2E, 0x2E, 0x54, 0x02, 0x81, 0x00, 0x87,
0xC9, 0xA7, 0x98
};
#endif // #ifdef EIT_ENGINE_DEBUG_ON

// static void EIT_DMXCallBack (void* userparam, DB_FilterHandle  handle, BOOL timedout, U8 const* data, U32 size)
void EIT_DMXCallBack (U8 tableInfoId, U8 *data, U32 size)
{
    U8             table_id;
    U16            section_length;
    U16            service_id;
    U8             version_number;
    U8             section_number;
    U8             last_section_number;
    U8             segment_last_section_number;
    U8             last_table_id;
	U8             tagIdNumber;

    eit_service_t     *service = NULL;
    eit_event_t       *info = NULL;
    S32                descriptor_loop_length;
    eit_table_type_t   eit_table_type;
    tCS_EIT_Error      eit_error;
#ifdef EPG_DATA_CAPTURE
	FILE*		      epgResult;
#endif // #ifdef EPG_DATA_CAPTURE

#ifdef EIT_ENGINE_DEBUG_ON
	printf("EitTableIfoId = %d, tableInfoId = %d, TableId : 0x%02X\n", EitTableIfoId, tableInfoId, data[0]);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

	if ((EitTableIfoId < TABLE_INFO_NUMBER) && (tableInfoId != EitTableIfoId))
	{
		return;
	}


	if((size > 4096) || (size < 18))
	{
		EIT_printf("EIT:Section length overflow or underflow : %d\n", size);
		return;
	}

    table_id    = data[0];
    if((table_id < 0x4e)||(table_id>0x5F))
    {
		return;
	}

	if ((data[5] & 0x01)== 0)
	{
		EIT_printf("EIT:Section is Not applicable!\n");
		return;
	}

#ifdef EPG_DATA_CAPTURE
	epgResult = fopen("/tmp/epgData.txt", "at");
	OsDumpToFile(epgResult, data, size);
	fclose(epgResult);
#endif // #ifdef EPG_DATA_CAPTURE
    section_length       = (U16)(Array2Word(data + 1, 2) & 0x0FFF);
    service_id           = (U16)(Array2Word(data + 3, 2));
    version_number       = (U8) ((data[5] >> 1) & 0x1F);
    section_number       = data[6];
    last_section_number  = data[7];
    segment_last_section_number = data[12];
    last_table_id = data[13];
#ifdef EIT_ENGINE_DEBUG_ON

    printf("eit table_id = 0x%x, \nservice_id = %d, section_number = %d/%d, version_number = %d, section_length = %d, last_table_id = 0x%02X - %d\n",
		table_id, service_id, section_number, last_section_number, version_number, section_length,  last_table_id, segment_last_section_number);

	if ((table_id == 0x51) && (service_id  == 12030) && (section_number == 184))
	{
		OsDumpData("EIT for RTL", data, size);

		return;
	}
	
#endif // #ifdef EIT_ENGINE_DEBUG_ON
 	if (section_length <= 15)
	{
        return;
	}
	section_length -= 15;  /* 14 - 3 (header+Length byte) + 4(CRC)*/
	data    += 14;
	
    CSOS_WaitSemaphore(sem_EIT_Manage_Access); 

    eit_error = EIT_GetService(service_id, &service);
    if ((eit_error == eCS_EIT_NOT_DONE) || (service == NULL))
    {
		EIT_printf("EIT_AddService !!!\n");
        eit_error = EIT_AddService(service_id, &service);
        if (eit_error != eCS_EIT_NO_ERROR) 
        {
            EIT_printf("EIT_AddService error\n");
            CSOS_SignalSemaphore(sem_EIT_Manage_Access);
            return;
        }
    } 
    else if (eit_error != eCS_EIT_NO_ERROR) 
    {
        EIT_printf("EIT_GetService error\n");
        CSOS_SignalSemaphore(sem_EIT_Manage_Access);
        return;
    }

    switch (table_id) 
    {
        case SI_EIT_PF_ACTUAL_TABLE_ID:
        {
            if (section_number == 0) 
            {
                eit_table_type = eEIT_PRESENT_TABLE;
            } 
            else if (section_number == 1) 
            {
                eit_table_type = eEIT_FOLLOWING_TABLE;
            } 
            else 
            {
                CSOS_SignalSemaphore(sem_EIT_Manage_Access);
                return;
            }

#ifdef EIT_ENGINE_DEBUG_ON
           //  EIT_printf("epg_version = %d\n", service->eit_pf_version);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

            if (service->eit_pf_version != version_number)
            {
				EIT_FreePresentEvent(service);
				EIT_FreeFollowingEvent(service);
				service->eit_pf_version = version_number;
				service->eit_pf_mask = 0;
            }
#ifdef EIT_ENGINE_DEBUG_ON
			// printf ("epg_version = %d, eit_pf_mask = 0x%02X\n", service->eit_pf_version, service->eit_pf_mask);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

			if(check_pf_complete(service->eit_pf_mask))
			{
				CSOS_SignalSemaphore(sem_EIT_Manage_Access);
				return;
			}
#ifdef EIT_ENGINE_DEBUG_ON
			// printf ("service_id = 0x%04X , epg_version = %d, eit_pf_mask = 0x%02X\n", service_id, service->eit_pf_version, service->eit_pf_mask);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			
            break;
        }
        default :
       	/* By kb : 2010 0404 */
			if ((table_id & 0xF0) == SI_EIT_SC_ACTUAL_FIRST_TABLE_ID)
			{
				eit_table_type = eEIT_SCHEDULE_TABLE;
				tagIdNumber = table_id - SI_EIT_SC_ACTUAL_FIRST_TABLE_ID;
				if ((service->eit_sc_section[tagIdNumber] == section_number) && (service->eit_sc_complete[tagIdNumber] == 0))
				{
					service->eit_sc_complete[tagIdNumber] = 1;
					if(EIT_CallBack != NULL)
					{
						EIT_CallBack(service->service_id, eCS_EIT_SCHEDULE);
					}
				}
				
				if (service->eit_sc_version[tagIdNumber] != version_number)
                {
					/* We got net version for this Tag */
					if (service->eit_sc_version[tagIdNumber] != kCS_EIT_VERSION_INIT)
					{
						/* First Version change for this program  */
						/* We need to initial program information */
						EIT_printf("EIT_FreeScheduleEvents FIRST\n");
						EIT_FreeScheduleEvents(service);
						memset(service->eit_sc_version, kCS_EIT_VERSION_INIT, MV_EIT_NUMBER_OF_TID);
						memset(service->eit_sc_section, kCS_EIT_NO_VERSION, MV_EIT_NUMBER_OF_TID);
						memset(service->eit_sc_complete, 0x00, MV_EIT_NUMBER_OF_TID);
					}
                    service->eit_sc_version[tagIdNumber]  = version_number;
					service->eit_sc_section[tagIdNumber]  = section_number;
					service->eit_sc_complete[tagIdNumber] = 0;
                }

				if (service->eit_sc_complete[tagIdNumber])
				{
					CSOS_SignalSemaphore(sem_EIT_Manage_Access);
					return;
				}
			}
			else
			{
				CSOS_SignalSemaphore(sem_EIT_Manage_Access);

				return;
			}
			break;
    }

	/* Start Event loop */
    while (section_length > EIT_EVENT_HEADER_LENGTH) 
    {
		/* Length must bigger than Event Header length */
        switch (eit_table_type) 
        {
            case eEIT_PRESENT_TABLE:
            case eEIT_FOLLOWING_TABLE:
            case eEIT_SCHEDULE_TABLE:
#ifdef EIT_ENGINE_DEBUG_ON
				printf("Malloc : eit_event_t Start\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
                info = (eit_event_t*)EIT_malloc(sizeof(eit_event_t));
 #ifdef EIT_ENGINE_DEBUG_ON
				printf("Malloc : eit_event_t 0x%X\n", info);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
           	break;
            default:
            {
                info = NULL;
            }break;
        }
            
        if (info != NULL) 
        {
			info->prev = NULL;
            info->next = NULL;
            info->short_description  = NULL;
            info->extended_description = NULL;
			/* by kb : 20100404 */
            info->event_name  = NULL;
			memset ((char *)info->extended_desc_language, 0x00, 0x04);
			memset ((char *)info->short_desc_language, 0x00, 0x04);
			/*
            info->extended_desc_language = kCS_EIT_INVALID_LANGUAGE;
            info->short_desc_language = kCS_EIT_INVALID_LANGUAGE;
            */
        
            info->event_id          = ((U16)data[0] << 8) | (U16)data[1];
#ifdef EIT_ENGINE_DEBUG_ON
			printf("Eit event Id : 0x%04X\n", info->event_id);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
            info->date_mjd          = ((U16)data[2] << 8) | (U16)data[3];
            info->start_time_utc    = ((U16)data[4] << 8) | (U16)data[5];
            info->duration_utc      = ((U16)data[7] << 8) | (U16)data[8];
            info->running_status    = (U8)((data[10] & 0xE0) >> 5);
            info->free_ca           = (U8)((data[10] & 0x10) >> 4);
			/* by kb : 20100404
            memset(info->event_name, 0, kCS_EIT_MAX_EVENT_NAME_LENGTH);
            */
            info->event_name_length       = 0;
            info->short_desc_length       = 0;  
            info->extended_desc_length    = 0; 
			info->parental_rating         = 0;

            descriptor_loop_length = (((U16)data[10] & 0x0F) << 8) | data[11];
            data += 12;
            section_length -= 12;

			/* Check Length */
			if (section_length < section_length)
			{
#ifdef EIT_ENGINE_DEBUG_ON
				// OsDebugPrtf("EIT_DMXCallBack Error :  section_length[%] is smaller than looplength[%d]!\n", section_length, descriptor_loop_length);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
				CSOS_SignalSemaphore(sem_EIT_Manage_Access);
				return;
			}
            // pre_check_and_match_eit_lang(data, descriptor_loop_length, match_short_lang, match_ext_lang);

            // EIT_SectionEitDescriptors(data, descriptor_loop_length, info, match_short_lang, match_ext_lang);
            EIT_SectionEitDescriptors(data, descriptor_loop_length, info);
            switch (eit_table_type) 
            {
                case eEIT_PRESENT_TABLE:
                {
					#ifdef EIT_ENGINE_DEBUG_ON
					/*
					printf("\neEIT_PRESENT_TABLE : ServiceId[0x%04X], Running[%d] ", service->service_id, info->running_status);
					if (info->event_name_length > 0)
					{
						printf(" Event Name[%s]", info->event_name);
					}

					printf("\n");
					EIT_printf("receive eEIT_PRESENT_TABLE service id = 0x%x\n", service->service_id);
					*/
					#endif

					info->running_status = 4;
					service->present_event = info;
					if ((service->eit_pf_mask & kCS_EIT_PRESENT_MASK) == kCS_EIT_PRESENT_MASK)
					{
						/* We get Present event again without Following event */
						service->eit_pf_mask |= kCS_EIT_FOLLOWING_MASK;  /* Finish to get PF for this ServicId */
					}
					service->eit_pf_mask |= kCS_EIT_PRESENT_MASK;
					// EIT_printf("add eEIT_PRESENT_TABLE success !!\n");
                    if(check_pf_complete(service->eit_pf_mask))
                    {
						// CS_Eit_Stop();
						// CS_Eit_Start(EIT_ServiceList.TP_Index, eCS_EIT_ACTUAL_PF, service->eit_pf_version);
						if(EIT_CallBack != NULL)
						{
							EIT_CallBack(service->service_id, eCS_EIT_PRESENT);
						}
                    }
                    break;
                }
                case eEIT_FOLLOWING_TABLE:
                {
					#ifdef EIT_ENGINE_DEBUG_ON
					/*
					printf("eEIT_FOLLOWING_TABLE : ServiceId[0x%04X], Running[%d] ", service->service_id, info->running_status);
					if (info->event_name_length > 0)
					{
						printf(" Event Name[%s]", info->event_name);
					}

					printf("\n");
					*/
					#endif
					
					info->running_status = 1;
					service->following_event = info;
					service->eit_pf_mask |= kCS_EIT_FOLLOWING_MASK;
					// EIT_printf("add eEIT_FOLLOWING_TABLE success !!\n");
                    if(check_pf_complete(service->eit_pf_mask))
                    {
						// CS_Eit_Stop();
						// CS_Eit_Start(EIT_ServiceList.TP_Index, eCS_EIT_ACTUAL_PF, service->eit_pf_version);
						if(EIT_CallBack != NULL)
						{
							EIT_CallBack(service->service_id, eCS_EIT_PRESENT);
						}
                    }
                    break;
                }
                case eEIT_SCHEDULE_TABLE:
                {
#ifdef EIT_ENGINE_DEBUG_ON
					
					if (service->present_event != NULL)
					{
						if (service->present_event->start_time_utc == info->start_time_utc)
						{
							EIT_printf("Current Event!! : 0x%04X\n", info->start_time_utc);
						}
					}
					
					if (service->following_event != NULL)
					{
						if (service->following_event->start_time_utc == info->start_time_utc)
						{
							EIT_printf("Following Event!! : 0x%04X\n", info->start_time_utc);
						}
					}
					
					if (info->running_status)
					{
						EIT_printf("Running Event!! : 0x%04X \n", info->start_time_utc);
					}
					
#endif
					info->running_status = 1;
					EIT_printf("Eit_AddSCEvent : Start\n");
					if (Eit_AddSCEvent(service, info) != eCS_EIT_NO_ERROR)
					{
						EIT_printf("add eEIT_SCHEDULE_TABLE error!!\n");
						EIT_FreeEvent(info);
						EIT_free(info);
						info = NULL;
					}
					
					else
					{
						EIT_printf("add eEIT_SCHEDULE_TABLE success!!\n");
					}
					
                }
				break;
            }
        } 
        else 
        {
            descriptor_loop_length = (S32)(((data[10] & 0x0F) << 8) | data[11]);
            data += 12;
            section_length -= 12;
        }

        if (eit_table_type != eEIT_SCHEDULE_TABLE) 
        {
            break;
        }

        section_length -= descriptor_loop_length;
        data   += descriptor_loop_length;
    }

    
    CSOS_SignalSemaphore(sem_EIT_Manage_Access);
}


tCS_EIT_Error CS_EIT_RegisterNotify(U8 *pClientID, tCS_EIT_NotificationCallBack NotifyFunction)
{
	tCS_EIT_Error	ReturnedError = eCS_EIT_NO_ERROR;
    
	if( NotifyFunction != NULL )
	{
		EIT_CallBack = NotifyFunction;
	}
	return(ReturnedError);
}


tCS_EIT_Error CS_EIT_Get_Current_Parental_Rate(U16  tp_index, U16 service_id)
{
	eit_service_t *service;	
	eit_event_t * current;
	
	CSOS_WaitSemaphore(sem_EIT_Manage_Access);	

	if(EIT_ServiceList.TP_Index != tp_index)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}
	
	if(EIT_GetService(service_id, &service) != eCS_EIT_NO_ERROR)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}

	if(check_pf_complete(service->eit_pf_mask))
	{

		current = service->present_event;

		if(current->parental_rating == 0)
		{
			CSOS_SignalSemaphore(sem_EIT_Manage_Access);
			return eCS_EIT_NOT_DONE;
		}

		//EIT_CallBack(service_id, SI_EIT_PF_ACTUAL_TABLE_ID);
	}

	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
	
	return eCS_EIT_NO_ERROR;
}

tCS_EIT_Error EitCopyEvent(eit_event_t *src, tCS_EIT_Event_t *dst)
{
	U32 tmpLength;
	U32 point;
	U32 result         = 0 ;
	U16 start_date_mjd = 0;
	U16	start_time_utc = 0;
	U16	end_date_mjd   = 0;
    U16	end_time_utc   = 0;
    U16	dur_utc        = 0;

	memset(dst, 0x00, sizeof(tCS_EIT_Event_t));
	
	if(src != NULL)
	{
		dst->event_id        = src->event_id;
		dst->free_ca         = src->free_ca;
		dst->running_status  = src->running_status;
		dst->parental_rating = src->parental_rating;
		dst->contentData     = src->contentData;

		end_date_mjd         = src->date_mjd;
		start_date_mjd       = src->date_mjd;
		start_time_utc       = src->start_time_utc;
		dur_utc              = src->duration_utc;

		result               = CS_DT_UTC_Add(start_time_utc,dur_utc);
		end_time_utc         = (result & 0x0000FFFF);
		result               = result >> 24;
		if ( result > 0)
		{
			end_date_mjd += result;
		}

		CS_DT_Caculate_EPG_Localtime(&start_date_mjd, &start_time_utc);
		CS_DT_Caculate_EPG_Localtime(&end_date_mjd, &end_time_utc);
		dst->start_date_mjd = start_date_mjd;
		dst->start_time_utc = start_time_utc;
		dst->end_date_mjd   = end_date_mjd;
		dst->end_time_utc   = end_time_utc;
		dst->duration_utc   = dur_utc;
		memcpy(dst->short_desc_language, src->short_desc_language, 3);
		memcpy(dst->extended_desc_language, src->extended_desc_language, 3);
#ifdef EIT_ENGINE_DEBUG_ON
		// printf("EitCopyEvent Info : event start Time [%04X] / [%04X]\n", src->start_time_utc, start_time_utc);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

		if (src->event_name_length > 0)
		{
			if (src->event_name_length >= kCS_EIT_MAX_EVENT_NAME_LENGTH)
			{
#ifdef EIT_ENGINE_DEBUG_ON
				printf("EitCopyEvent Info : event_name_length[%d] / [%d] Over!\n", src->event_name_length, kCS_EIT_MAX_EVENT_NAME_LENGTH);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
				dst->nameLength = kCS_EIT_MAX_EVENT_NAME_LENGTH - 1;
			}
			else
			{
				dst->nameLength = src->event_name_length;
			}

			memcpy(dst->event_name, src->event_name, dst->nameLength);
		}

		dst->descriptionLength = 0;
		point = 0;
		if(src->short_desc_length > 0)
		{
			memcpy(dst->description_data, src->short_description, src->short_desc_length);
			point += src->short_desc_length;
		}
		
		if(src->extended_desc_length > 0)
		{
			if ((point + src->extended_desc_length) >= kCS_EIT_MAX_TOTAL_DESC_LENGTH)
			{
				tmpLength = kCS_EIT_MAX_TOTAL_DESC_LENGTH - point - 1;
#ifdef EIT_ENGINE_DEBUG_ON
				printf("EitCopyEvent Info :ext Desc [%d] / [%d] Over!\n", src->extended_desc_length, tmpLength);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			}
			else
			{
				tmpLength = src->extended_desc_length;
			}
			memcpy((dst->description_data + point), src->extended_description, tmpLength);
			point += tmpLength;
		}
		dst->descriptionLength = point;

		return eCS_EIT_NO_ERROR;
	}
	else
	{
		dst->event_id          = 0xffff;
		dst->nameLength        = 0;
		dst->descriptionLength = 0;
		dst->start_date_mjd    = kCS_EIT_INVALID_MJD;
	}

	return eCS_EIT_NOT_DONE;
}

tCS_EIT_Error CS_EIT_Get_PF_Event(U16 tp_index, U16  service_id, tCS_EIT_Event_t *present, tCS_EIT_Event_t *follow)
{
	eit_service_t *service;	

    if ((present == NULL) || (follow == NULL))
	{
		return eCS_EIT_BAD_PARAMETER;	
    }

	CSOS_WaitSemaphore(sem_EIT_Manage_Access);	

	if(EIT_ServiceList.TP_Index != tp_index)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}	   
	
	if(EIT_GetService( service_id, &service) != eCS_EIT_NO_ERROR)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}

    // EIT_printf("CS_EIT_Get_PF_Event service id = 0x%x\n", service->service_id);

	if(check_pf_complete(service->eit_pf_mask))
	{
		if (EitCopyEvent(service->present_event, present) != eCS_EIT_NO_ERROR)
		{
			CSOS_SignalSemaphore(sem_EIT_Manage_Access);
			return eCS_EIT_NOT_DONE;
		}

		EitCopyEvent(service->following_event, follow);

		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NO_ERROR;
	}

	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
	return eCS_EIT_NOT_DONE;
}

#if 0
tCS_EIT_Error CS_EIT_Get_SC_Events(U16 tp_index, U16 service_id, U16 requested_date_mjd, tCS_EIT_Event_t *sc_events)
{
	eit_service_t *service;
	eit_event_t      *cur_schedule;
	U16      num=0;
	U16           start_date_mjd = 0;
	U16	         start_time_utc = 0;
	U16	         end_date_mjd =0;
         U16	         end_time_utc =0;
         U16	         dur_utc =0;
 	U32 	         result =0 ;

        if(requested_date_mjd == kCS_EIT_INVALID_MJD)
            return eCS_EIT_BAD_PARAMETER;

	CSOS_WaitSemaphore(sem_EIT_Manage_Access);	

	if(EIT_ServiceList.TP_Index != tp_index)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}
	
	
	if(EIT_GetService( service_id, &service) != eCS_EIT_NO_ERROR)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}


	
	cur_schedule = service->sc_event_list;	

	while(cur_schedule)
	{		
		start_date_mjd = cur_schedule->date_mjd;
		end_date_mjd = cur_schedule->date_mjd;
		start_time_utc  = cur_schedule->start_time_utc;
		dur_utc  = cur_schedule->duration_utc;

		result = CS_DT_UTC_Add(start_time_utc,dur_utc);
		end_time_utc = (result & 0x0000FFFF);
		result = result >> 24;
		if ( result > 0) 	
		{
			end_date_mjd+=result;
		}
	
		CS_DT_Caculate_EPG_Localtime(&start_date_mjd,&start_time_utc);
		CS_DT_Caculate_EPG_Localtime(&end_date_mjd,&end_time_utc);

                //printf("start_date_mjd = 0x%x, requested_date_mjd = 0x%x\n", start_date_mjd, requested_date_mjd);

		
		if(start_date_mjd == requested_date_mjd)
		{
		
			sc_events[num].event_id = cur_schedule->event_id;
			sc_events[num].free_ca  = cur_schedule->free_ca;		

			sc_events[num].start_date_mjd = start_date_mjd;
			sc_events[num].start_time_utc= start_time_utc;	
			sc_events[num].end_date_mjd = end_date_mjd;
			sc_events[num].end_time_utc = end_time_utc;	
			sc_events[num].duration_utc = dur_utc;

			/* by kb : 20100404
			memcpy(sc_events[num].event_name,cur_schedule->event_name,kCS_EIT_MAX_EVENT_NAME_LENGTH);
			sc_events[num].event_name[kCS_EIT_MAX_EVENT_NAME_LENGTH-1] = '\0';
                           sc_events[num].event_name[kCS_EIT_MAX_EVENT_NAME_LENGTH-2] = '\0';
			sc_events[num].short_desc_language = cur_schedule->short_desc_language;

			sc_events[num].extended_desc_language = cur_schedule->extended_desc_language;
			*/
				
			if(cur_schedule->short_desc_length > 0)
			{
				if(cur_schedule->short_desc_length < kCS_EIT_MAX_SHORT_DESC_LENGTH-1)
				{
					memcpy(sc_events[num].short_description,cur_schedule->short_description,cur_schedule->short_desc_length);
					sc_events[num].short_description[cur_schedule->short_desc_length] = '\0';
                                            sc_events[num].short_description[cur_schedule->short_desc_length+1] = '\0';
				}
				else
				{
					memcpy(sc_events[num].short_description,cur_schedule->short_description,kCS_EIT_MAX_SHORT_DESC_LENGTH);
					sc_events[num].short_description[kCS_EIT_MAX_SHORT_DESC_LENGTH-1 ]= '\0';
                                             sc_events[num].short_description[kCS_EIT_MAX_SHORT_DESC_LENGTH-2 ]= '\0';
				}
			}
			else
			{
				sc_events[num].short_description[0] = '\0';
                                    sc_events[num].short_description[1] = '\0';
			}
				
			if(cur_schedule->extended_desc_length > 0)
			{
				if(cur_schedule->extended_desc_length < kCS_EIT_MAX_EXTENDED_DESC_LENGTH-1)
				{
					memcpy(sc_events[num].extended_description,cur_schedule->extended_description,cur_schedule->extended_desc_length);
					sc_events[num].extended_description[cur_schedule->extended_desc_length] = '\0';
                                            sc_events[num].extended_description[cur_schedule->extended_desc_length+1] = '\0';
				}
				else
				{
					memcpy(sc_events[num].extended_description,cur_schedule->extended_description,kCS_EIT_MAX_EXTENDED_DESC_LENGTH);
					sc_events[num].extended_description[kCS_EIT_MAX_EXTENDED_DESC_LENGTH-1] = '\0';
                                             sc_events[num].extended_description[kCS_EIT_MAX_EXTENDED_DESC_LENGTH-2] = '\0';
				}
			}
			else
			{
				sc_events[num].extended_description[0] = '\0';
                                    sc_events[num].extended_description[1] = '\0';
			}
			num++;
			
		}
		
		cur_schedule = cur_schedule->next;
	}

	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
	return eCS_EIT_NO_ERROR;
}
#endif

tCS_EIT_Error Mv_GetScEventList (U16 tp_index, U16 service_id, U16 dateOffset, tCS_EIT_Event_List_t *scEventList)
{
	eit_service_t	*service;
	eit_event_t		*curSchedule;
	eit_event_t		*startSchedule;
	U32              result;
	U16				 num = 0;
	U16				 cnt = 0;
	U16              eventPoint   = 0;
	U16				 startDateMjd = 0;
	U16				 startTimeUtc = 0;
	U16				 endDateMjd   = 0;
	U16				 endTimeUtc   = 0;
	U16				 durationUtc  = 0;
	U16              currentMjd;
	U16              currentUtc;
	U16              presentMjd;
	U16              presentUtc;
	U16              pDuration;
	U16              pEndMjd;
	U16              pEndUtc;
	U16              sMjd;
	U16              sUtc;
	U8               dayFlag;
	tCS_DT_Date  	 currentDate;
	tCS_DT_Time		 currentTime;
	tCS_EIT_Event_t	*eventData;


#ifdef EIT_ENGINE_DEBUG_ON
	printf("Mv_GetScEventList Day %d >>>>>>>>>>>>>>>>>>>>  %d\n", dateOffset, (U32)scEventList);
#endif // #ifdef EIT_ENGINE_DEBUG_ON
	
	CSOS_WaitSemaphore(sem_EIT_Manage_Access);
	if (scEventList == NULL)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		printf("Mv_GetScEventList scEventList == NULL \n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_BAD_PARAMETER;
	}

	if (scEventList->EventArray != NULL)
	{
		EIT_free(scEventList->EventArray);
		scEventList->EventArray = NULL;
	}
	scEventList->NumberOfEvent = 0;
	scEventList->StartPoint    = 0;
	
	if(EIT_ServiceList.TP_Index != tp_index)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		printf("Mv_GetScEventList TP not match \n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}	   
	
	if(EIT_GetService(service_id, &service) != eCS_EIT_NO_ERROR)
	{
#ifdef EIT_ENGINE_DEBUG_ON
		printf("Mv_GetScEventList Service not found \n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}

	MV_OS_Get_Time_to_MJD_UTC_Date_Time(&currentMjd, &currentUtc, &currentDate, &currentTime);

	curSchedule	 = service->present_event;
	currentMjd  += dateOffset;
	sMjd = currentMjd;
	sUtc = 0;

	if (curSchedule != NULL)
	{
		presentMjd		= curSchedule->date_mjd;
		presentUtc		= curSchedule->start_time_utc;
		pDuration		= curSchedule->duration_utc;
		pEndMjd			= curSchedule->date_mjd;
		
		result			= CS_DT_UTC_Add(presentUtc, pDuration);
		pEndUtc			= (U16)(result & 0x0000FFFF);
		result			= result >> 24;

		if ( result > 0)
		{
			pEndMjd += (U16)result;
		}

		CS_DT_Caculate_EPG_Localtime(&presentMjd, &presentUtc);
		CS_DT_Caculate_EPG_Localtime(&pEndMjd, &pEndUtc);

		if ((dateOffset == 0) && (presentMjd < sMjd))
		{
			sMjd = presentMjd;
			sUtc = presentUtc;
		}
	}
	else
	{
		presentMjd = 0;
		presentUtc = 0;
		pDuration  = 0;
		pEndMjd    = 0;
	}
	
#ifdef EIT_ENGINE_DEBUG_ON
	// printf("Eit Utc : 0x%04X , %02d:%02d\n", currentUtc, currentTime.hour, currentTime.minute);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

	curSchedule = service->sc_event_list;
	startSchedule = NULL;
	num = 0;
	
	while(curSchedule != NULL)
	{
		dayFlag = 0;
		
		startDateMjd       = curSchedule->date_mjd;
		startTimeUtc       = curSchedule->start_time_utc;
		endDateMjd         = curSchedule->date_mjd;
		durationUtc        = curSchedule->duration_utc;

		result			= CS_DT_UTC_Add(startTimeUtc, durationUtc);
		endTimeUtc		= (result & 0x0000FFFF);
		result			= result >> 24;
		if ( result > 0)
		{
			endDateMjd += (U16)result;
		}

		CS_DT_Caculate_EPG_Localtime(&startDateMjd, &startTimeUtc);
		CS_DT_Caculate_EPG_Localtime(&endDateMjd, &endTimeUtc);

		if (currentMjd < startDateMjd)
		{
#ifdef EIT_ENGINE_DEBUG_ON
			printf("Mv_GetScEventList End of Day\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			break;
		}
			
		if (sMjd < currentMjd)
		{
			/* Present event and it starts before today */
			if (((sMjd == startDateMjd) && (sUtc <= startTimeUtc)) ||
				(sMjd < startDateMjd)       ||
				(currentMjd == startDateMjd) ||
				(currentMjd == endDateMjd))
			{
				dayFlag = 1;
			}
		}
		else
		{
			if ((currentMjd == startDateMjd) ||
				(currentMjd == endDateMjd))
			{
				dayFlag = 1;
			}
		}

		if (dayFlag)
		{
			dayFlag = 0;
			if (startSchedule == NULL)
			{
#ifdef EIT_ENGINE_DEBUG_ON
				printf("Mv_GetScEventList First Service!!\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
				startSchedule = curSchedule;
				num = 0;
			}
#ifdef EIT_ENGINE_DEBUG_ON
			// printf("Mv_GetScEventList Add Service %d\n", num);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

			num++;
		}

		curSchedule = curSchedule->next;
	}

	eventPoint = 0;
	if ((num > 0) && (startSchedule != NULL))
	{
		eventData = EIT_malloc(sizeof(tCS_EIT_Event_t) * num);
		if (eventData == NULL)
		{
			scEventList->NumberOfEvent = 0;
			scEventList->StartPoint    = 0;
			scEventList->EventArray    = NULL;

#ifdef EIT_ENGINE_DEBUG_ON
			printf("Mv_GetScEventList Malloc error\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			CSOS_SignalSemaphore(sem_EIT_Manage_Access);
			return eCS_EIT_NOT_DONE;
		}

		cnt = 0;
		while(cnt < num)
		{
			EitCopyEvent(startSchedule, &eventData[cnt]);
			if ((eventData[cnt].start_date_mjd == presentMjd) && (eventData[cnt].start_time_utc == presentUtc))
			{
				eventData[cnt].running_status = 4;
				eventPoint = cnt;
			}
			startSchedule = startSchedule->next;
			cnt++;
		}
	}
	else
	{
		num = 0;
		/* No SC Data : Need to check PF Data */
		if (service->present_event != NULL)
		{
			startSchedule = NULL;
			
			if (dateOffset == 0)
			{
				startSchedule = service->present_event;
				dayFlag = 1;
				num++;
			}
			else
			{
				if ((currentMjd == presentMjd) ||
					(currentMjd == pEndMjd))
				{
					startSchedule = service->present_event;
					dayFlag = 1;
					num++;
				}
			}
			
			if (service->following_event != NULL)
			{
				curSchedule		= service->following_event;
				startDateMjd	= curSchedule->date_mjd;
				startTimeUtc	= curSchedule->start_time_utc;
				endDateMjd		= curSchedule->date_mjd;
				durationUtc		= curSchedule->duration_utc;

				result			= CS_DT_UTC_Add(startTimeUtc, durationUtc);
				endTimeUtc		= (result & 0x0000FFFF);
				result			= result >> 24;
				if ( result > 0)
				{
					endDateMjd += (U16)result;
				}

				CS_DT_Caculate_EPG_Localtime(&startDateMjd, &startTimeUtc);
				CS_DT_Caculate_EPG_Localtime(&endDateMjd, &endTimeUtc);

				if ((currentMjd == startDateMjd) ||
					(currentMjd == endDateMjd))
				{
					if (startSchedule == NULL)
					{
						startSchedule = service->following_event;
					}
					num++;
				}
			}
		}

		if (num > 0)
		{
			eventData = EIT_malloc(sizeof(tCS_EIT_Event_t) * num);
			if (eventData == NULL)
			{
				scEventList->NumberOfEvent = 0;
				scEventList->StartPoint    = 0;
				scEventList->EventArray    = NULL;

#ifdef EIT_ENGINE_DEBUG_ON
				printf("Mv_GetScEventList Malloc error1\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
				CSOS_SignalSemaphore(sem_EIT_Manage_Access);
				return eCS_EIT_NOT_DONE;
			}
			
			EitCopyEvent(startSchedule, &eventData[0]);
			if (dayFlag)
			{
				dayFlag = 0;
				eventData[cnt].running_status = 4;
			}
			if (num == 2)
			{
				EitCopyEvent(service->following_event, &eventData[1]);
			}
		}
		else
		{
			scEventList->NumberOfEvent = 0;
			scEventList->StartPoint    = 0;
			scEventList->EventArray    = NULL;

#ifdef EIT_ENGINE_DEBUG_ON
			printf("Mv_GetScEventList No Present Service\n");
#endif // #ifdef EIT_ENGINE_DEBUG_ON
			CSOS_SignalSemaphore(sem_EIT_Manage_Access);
			return eCS_EIT_NOT_DONE;
		}
	}

	scEventList->NumberOfEvent = num;
	scEventList->StartPoint    = eventPoint;
	scEventList->EventArray    = eventData;
#ifdef EIT_ENGINE_DEBUG_ON
	for (cnt = 0; cnt < num; cnt++)
	{
		printf("Date : %d Time : 0x%04X Name[%s] \n", scEventList->EventArray[cnt].start_date_mjd, scEventList->EventArray[cnt].start_time_utc, scEventList->EventArray[cnt].event_name);
	}

	printf("Start Point : %d [%s]\n", scEventList->StartPoint, scEventList->EventArray[scEventList->StartPoint].event_name);
#endif // #ifdef EIT_ENGINE_DEBUG_ON

	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
	return eCS_EIT_NO_ERROR;
}

tCS_EIT_Error CS_EIT_Get_SC_Events_Num(U16 tp_index, U16 service_id, U16 requested_date_mjd,U16 *sc_event_num)
{
	eit_service_t	*service;
	eit_event_t		*cur_schedule;
	U16				 num=0;
	U16				 start_date_mjd = 0;
	U16				 start_time_utc = 0;

	*sc_event_num = 0;
	if(requested_date_mjd == kCS_EIT_INVALID_MJD)
		return eCS_EIT_BAD_PARAMETER;

	CSOS_WaitSemaphore(sem_EIT_Manage_Access);	

	if(EIT_ServiceList.TP_Index != tp_index)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);
		return eCS_EIT_NOT_DONE;
	}
	
	
	if(EIT_GetService( service_id, &service) != eCS_EIT_NO_ERROR)
	{
		CSOS_SignalSemaphore(sem_EIT_Manage_Access);	
		return eCS_EIT_NOT_DONE;
	}

	cur_schedule = service->sc_event_list;
	while(cur_schedule)
	{
		start_date_mjd = cur_schedule->date_mjd ;
		start_time_utc  = cur_schedule->start_time_utc;		
		CS_DT_Caculate_EPG_Localtime(&start_date_mjd,&start_time_utc);		
		(start_date_mjd == requested_date_mjd) ? num++ : 0;
		cur_schedule = cur_schedule->next;		
	}
	// printf("=== EVENT NUM : %d ========\n", num);
	*sc_event_num = num;
	CSOS_SignalSemaphore(sem_EIT_Manage_Access);
	return eCS_EIT_NO_ERROR;
	
}

tCS_EIT_Error CS_EIT_Get_SC_Events_FirstMJD(U16  tp_index, U16 service_id, U16 *min_date_mjd)
{
    eit_service_t *service;
    eit_event_t   *cur_schedule;
    // U16      num=0;
    U16     start_time_utc = 0;
    
    *min_date_mjd = 0;

    CSOS_WaitSemaphore(sem_EIT_Manage_Access);  

    if(EIT_ServiceList.TP_Index != tp_index)
    {
        CSOS_SignalSemaphore(sem_EIT_Manage_Access);
        return eCS_EIT_NOT_DONE;
    }
    
    
    if(EIT_GetService( service_id, &service) != eCS_EIT_NO_ERROR)
    {
        CSOS_SignalSemaphore(sem_EIT_Manage_Access);    
        return eCS_EIT_NOT_DONE;
    }

    cur_schedule = service->sc_event_list;

    if(cur_schedule != NULL)
        {
            *min_date_mjd = cur_schedule->date_mjd ;
            start_time_utc  = cur_schedule->start_time_utc;   
            CS_DT_Caculate_EPG_Localtime(min_date_mjd, &start_time_utc);      
        }
    else
        {
            CSOS_SignalSemaphore(sem_EIT_Manage_Access);
            return eCS_EIT_NOT_DONE;
        }
    
    CSOS_SignalSemaphore(sem_EIT_Manage_Access);
    
    return(eCS_EIT_NO_ERROR);
    
}



