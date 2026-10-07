/*
 * si_table.c - SI table processing of the Merih library (port of table.o).
 *
 * Channel scan (SiStartSearchChannel): PAT -> PMT of every program, SDT (names, types) and
 * NIT (other transponders), with section reassembly and timeouts; the result is reported
 * through the search / NIT callbacks. Live mode (SiStartLiveSearch): PAT -> PMT of the
 * current service, then the PMT is watched for version changes and reported through the
 * live callback. SiStartLiveSection lets mvapp run its own section filters (EIT, TDT, ...).
 */
#include <stdio.h>
#include <string.h>

#include "si_int.h"
#include "tableApi.h"
#include "mvutil.h"

#define PAT_TABLE_ID            0x00
#define PMT_TABLE_ID            0x02
#define NIT_ACTUAL_TABLE_ID     0x40
#define SDT_ACTUAL_TABLE_ID     0x42
#define TDT_TABLE_ID            0x70
#define PAT_PID                 0x0000
#define NIT_PID                 0x0010
#define SDT_PID                 0x0011
#define TDT_PID                 0x0014
#define INVALID_PID             0x1fff
#define INVALID_ID              0xffff
#define MAX_PAT_PROGRAM         128
#define MAX_NVOD                20
#define LIVE_PMT_VERSION_NONE   0xff

/* Timeouts in parse-loop ticks (100 ms without a section). */
#define PAT_TIMEOUT             20
#define PMT_TIMEOUT             20
#define NIT_TIMEOUT             20
#define NIT_TP_TIMEOUT          90
#define SDT_TIMEOUT             120

/* AddData2List() results. */
#define LIST_ADDED              0
#define LIST_FULL               2
#define LIST_NO_MEMORY          3
#define LIST_DUPLICATE          4

/* State of one running table. */
typedef struct
{
	U8  InfoId;
	U8  Active;
	U32 Count;          /* timeout ticks */
} TableProcessData_t;

/* Received section kept until its table is complete. */
typedef struct TableBuffer_s
{
	struct TableBuffer_s *Next;
	U16                   Id;          /* table id extension */
	U32                   Length;
	U8                    SectionNo;
	U8                    LastSectionNo;
	U8                   *Data;
} TableBuffer_t;

/* PMT PID of the scan, shared by Count programs. */
typedef struct
{
	U16                Pid;
	U8                 Count;          /* programs still to be parsed */
	U8                 Received;       /* sections seen again for parsed programs */
	TableProcessData_t Process;
} PmtList_t;

/* Parsed PAT. */
typedef struct
{
	U16 TsId;
	U8  NumberOfProgram;
	U16 ServiceId[MAX_PAT_PROGRAM];
	U16 PmtPid[MAX_PAT_PROGRAM];
} PatData_t;

/* Live section request from mvapp. */
typedef struct LiveProcessData_s
{
	struct LiveProcessData_s *Next;
	U16                       Pid;
	U8                        InfoId;
	U8                        Active;
	DemuxCallback_f           CallBack;
} LiveProcessData_t;

/* NVOD names collected from the SDT. */
typedef struct
{
	U16  ServiceId[MAX_NVOD];
	char Name[MAX_NVOD][MAX_NAME_LEN];
} NvodNameList_t;

/* ATSC AC-3 language codes. */
typedef struct
{
	U8          Code;
	const char *Lang;
} AtscLang_t;

static const AtscLang_t AtscAc3AudioLang[11] =
{
	{ 0x06, "tur" }, { 0x08, "deu" }, { 0x09, "eng" }, { 0x0a, "spa" }, { 0x0f, "fra" },
	{ 0x15, "ita" }, { 0x29, "tur" }, { 0x65, "kor" }, { 0x70, "gre" }, { 0x75, "ara" },
	{ 0xff, "eng" },
};

static SearchResult_f      SearchCallBack;
static LiveSearchResult_f  LiveSearchCallBack;
static NitResult_f         NitSarchCallBack;

static SiService_t         SiServiceData;
static SiNetworkData_t     SiNetworkData;
static SiProgramData_t     SiLiveProgramData;
static U16                 SiLiveProgramServiceId = INVALID_ID;
static U8                  PmtData[MAX_PMT_LENGTH];
static U32                 PmtSize;
static PatData_t           PatData;
static PmtList_t           PmtList[MAX_PAT_PROGRAM];
static U8                  PmtNumberOfPid;
static LiveProcessData_t  *LiveProcessData_P;
static TableBuffer_t      *CurrentSdtData_p;
static TableBuffer_t      *NitData_p;
static NvodNameList_t      NvodSdtNameList;
static U32                 NvodCount;
static U32                 NvodRefFound;
static U32                 NvodShiftServiceFound;
static U16                 NvodShiftRefProgramNo;

static TableProcessData_t  PATProcessData;
static TableProcessData_t  NITProcessData;
static TableProcessData_t  SDTProcessData;
static TableProcessData_t  TDTProcessData;
static TableProcessData_t  LivePMTProcessData;

static U8                  PATProcessed;
static U8                  PMTProcessed;
static U8                  NITProcessed;
static U8                  SDTProcessed;
static U8                  TDTProcessed;
static U8                  SDTReceived;
static U8                  SearchNotLive;     /* 1 while a channel scan runs */
static U8                  NitSearchOn;
static U8                  TpSearchOn;
static U8                  SatSearchMode;
static U8                  SatelliteType;
static U16                 CurrentSatNetWorkId;

/*
 * Convert a DVB text field (Annex A) into dest: drop the character table selector, keep
 * the table code in *code and skip the 0x80-0x9f control codes. Returns the length.
 */
U32 SiCodeConverter(U8 *dest, U8 *src, U32 len, U16 *code)
{
	U32 in;
	U32 out = 0;
	U8  first = src[0];

	if (first == 0)
	{
		dest[0] = 0;
		return 0;
	}
	if (first < 0x10)
	{
		*code = first;
		in    = 1;
	}
	else if (first == 0x10)
	{
		*code = (U16)((src[1] << 8) | src[2]);
		in    = 3;
	}
	else if (first == 0x1f)
	{
		*code = 0;
		in    = 2;
	}
	else
	{
		*code = 0;
		in    = (first <= 0x1e) ? 1 : 0;
	}

	while ((src[in] != 0) && (in < len) && (out < len))
	{
		if ((U8)(src[in] - 0x80) > 0x1f)
			dest[out++] = src[in];
		in++;
	}
	if (out < len)
		dest[out] = 0;
	return out;
}

/* ---- live section list ----------------------------------------------------------- */

static void InitLiveProcessData(LiveProcessData_t *data)
{
	if (data == NULL)
		return;
	data->Next     = NULL;
	data->InfoId   = DEMUX_INFO_NONE;
	data->Active   = 0;
	data->CallBack = NULL;
}

static void ClearLiveProcessData(LiveProcessData_t *data)
{
	if (data == NULL)
		return;
	InitLiveProcessData(data);
	DemuxMemoryFree(data);
}

static LiveProcessData_t *AllocateLiveProcessData(void)
{
	LiveProcessData_t *data = (LiveProcessData_t *)DemuxMemoryAllocate(sizeof(*data));

	InitLiveProcessData(data);
	return data;
}

/* Live request whose filter chain contains infoId. */
static LiveProcessData_t *FindLiveProcessDataByInfoId(U8 infoId)
{
	LiveProcessData_t *data;

	for (data = LiveProcessData_P; data != NULL; data = data->Next)
	{
		if (DemuxCheckSameTable(data->InfoId, infoId))
			return data;
	}
	return NULL;
}

static void StopLiveProcess(LiveProcessData_t *data)
{
	if (data == NULL)
		return;
	if (data->InfoId < DEMUX_INFO_NUMBER)
		DemuxStopTable(data->InfoId);
	ClearLiveProcessData(data);
}

static void StopAllLiveProcess(void)
{
	LiveProcessData_t *data;

	while (LiveProcessData_P != NULL)
	{
		data              = LiveProcessData_P;
		LiveProcessData_P = data->Next;
		StopLiveProcess(data);
	}
}

/* Unlink and stop a live request. FALSE = ok. */
static BOOL TableStopLiveSection(U8 infoId)
{
	LiveProcessData_t **link = &LiveProcessData_P;
	LiveProcessData_t  *data;

	for (data = *link; data != NULL; link = &data->Next, data = *link)
	{
		if (data->InfoId == infoId)
		{
			*link = data->Next;
			StopLiveProcess(data);
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL TablePauseLiveSection(U8 infoId)
{
	LiveProcessData_t *data;

	for (data = LiveProcessData_P; data != NULL; data = data->Next)
	{
		if (data->InfoId == infoId)
		{
			data->Active = 0;
			return FALSE;
		}
	}
	return TRUE;
}

/* Append a new live request at the end of the list. */
static LiveProcessData_t *TableAddLiveSection(void)
{
	LiveProcessData_t **link = &LiveProcessData_P;

	while (*link != NULL)
		link = &(*link)->Next;
	*link = AllocateLiveProcessData();
	return *link;
}

/* Section callback of mvapp's live filters: hand a private copy to mvapp. */
static void LiveTableCallBack(U8 infoId, U8 *data, U32 size)
{
	LiveProcessData_t *live = FindLiveProcessDataByInfoId(infoId);
	U8                *copy;

	if (live == NULL)
	{
		printf("LiveTableCallBack : Can't find Live Process data for InfoID[%d]\n", infoId);
	}
	else if (live->CallBack != NULL)
	{
		copy = (U8 *)DemuxMemoryAllocate(size);
		if (copy != NULL)
		{
			memcpy(copy, data, size);
			live->CallBack(live->InfoId, copy, size);
			DemuxMemoryFree(copy);
		}
	}
	DemuxUpdateLinearPoint(size, FALSE);
}

/* ---- section buffers (SDT / NIT reassembly) ---------------------------------------- */

static void ClearTableBuffer(TableBuffer_t *buffer)
{
	if (buffer == NULL)
		return;
	if (buffer->Data != NULL)
		DemuxMemoryFree(buffer->Data);
	DemuxMemoryFree(buffer);
}

static void ClearAllTableBuffer(TableBuffer_t **list)
{
	TableBuffer_t *buffer = *list;
	TableBuffer_t *next;

	while (buffer != NULL)
	{
		next = buffer->Next;
		ClearTableBuffer(buffer);
		buffer = next;
	}
	*list = NULL;
}

/* Drop all sections that do not belong to table id extension id. */
static void ClearUnusedTableBuffer(TableBuffer_t **list, U16 id)
{
	TableBuffer_t **link = list;
	TableBuffer_t  *buffer;

	while ((buffer = *link) != NULL)
	{
		if (buffer->Id == id)
		{
			link = &buffer->Next;
		}
		else
		{
			*link = buffer->Next;
			ClearTableBuffer(buffer);
		}
	}
}

static TableBuffer_t *FindBuffer(TableBuffer_t *list, U16 id, U8 sectionNo)
{
	for (; list != NULL; list = list->Next)
	{
		if ((list->Id == id) && (list->SectionNo == sectionNo))
			return list;
	}
	return NULL;
}

/* All sections 0..last of table id present (the list is sorted)? */
static BOOL CheckTableBufferFinished(TableBuffer_t *list, U16 id)
{
	U8 expected = 0;

	for (; list != NULL; list = list->Next)
	{
		if (list->Id != id)
			continue;
		if (list->SectionNo != expected)
			return FALSE;
		if (list->LastSectionNo == expected)
			return TRUE;
		expected++;
	}
	return FALSE;
}

/* Create a single buffer node holding a copy of the section. */
static U8 AddTable2Buffer(TableBuffer_t **list, U16 id, U32 length, U8 sectionNo,
                          U8 lastSectionNo, U8 *data)
{
	TableBuffer_t *buffer;

	if (length == 0)
		return 1;
	if ((*list != NULL) && ((*list)->Id <= 254))
		return LIST_FULL;
	buffer = (TableBuffer_t *)DemuxMemoryAllocate(sizeof(*buffer));
	if (buffer == NULL)
	{
		*list = NULL;
		return LIST_NO_MEMORY;
	}
	buffer->Data = (U8 *)DemuxMemoryAllocate(length);
	if (buffer->Data == NULL)
	{
		DemuxMemoryFree(buffer);
		return LIST_NO_MEMORY;
	}
	buffer->SectionNo     = sectionNo;
	buffer->LastSectionNo = lastSectionNo;
	buffer->Id            = id;
	buffer->Length        = length;
	memcpy(buffer->Data, data, length);
	buffer->Next = NULL;
	*list        = buffer;
	return LIST_ADDED;
}

/* Insert a section into the list sorted by (id, section number). */
static U8 AddData2List(TableBuffer_t **list, U16 id, U32 length, U8 sectionNo,
                       U8 lastSectionNo, U8 *data)
{
	TableBuffer_t *node = NULL;
	TableBuffer_t *current;
	TableBuffer_t *next;
	U8             result;

	current = *list;
	if ((current == NULL) || (current->Id > id) ||
	    ((current->Id == id) && (current->SectionNo > sectionNo)))
	{
		result = AddTable2Buffer(&node, id, length, sectionNo, lastSectionNo, data);
		if (result == LIST_ADDED)
		{
			node->Next = current;
			*list      = node;
		}
		return result;
	}

	for (;;)
	{
		if ((current->Id == id) && (current->SectionNo == sectionNo))
			return LIST_DUPLICATE;
		next = current->Next;
		if ((next == NULL) || (next->Id > id) ||
		    ((next->Id == id) && (next->SectionNo > sectionNo)))
		{
			result = AddTable2Buffer(&node, id, length, sectionNo, lastSectionNo, data);
			if (result == LIST_ADDED)
			{
				node->Next    = next;
				current->Next = node;
			}
			return result;
		}
		current = next;
	}
}

/* ---- table control ---------------------------------------------------------------- */

static void TableStopSection(TableProcessData_t *process)
{
	if (process->InfoId < DEMUX_INFO_NUMBER)
		DemuxStopTable(process->InfoId);
	process->Count  = 0;
	process->InfoId = DEMUX_INFO_NONE;
	process->Active = 0;
}

static void ClearPmtList(U8 index)
{
	if (index >= MAX_PAT_PROGRAM)
		return;
	PmtList[index].Pid            = INVALID_PID;
	PmtList[index].Process.Count  = 0;
	PmtList[index].Process.InfoId = DEMUX_INFO_NONE;
	PmtList[index].Process.Active = 0;
	PmtList[index].Count          = 0;
	PmtList[index].Received       = 0;
}

static void ClearAllPmtList(void)
{
	U8 index;

	for (index = 0; index < MAX_PAT_PROGRAM; index++)
		ClearPmtList(index);
}

static void TableStopPmt(U8 index)
{
	TableStopSection(&PmtList[index].Process);
	ClearPmtList(index);
}

static void StopAllPmt(void)
{
	U8 index;

	for (index = 0; index < PmtNumberOfPid; index++)
		TableStopPmt(index);
}

/* ---- network (NIT) data ------------------------------------------------------------- */

static void DeleteAllNitTp(void)
{
	SiNitTpData_t *tp = SiNetworkData.NitTpData;
	SiNitTpData_t *next;

	while (tp != NULL)
	{
		next = tp->Next_p;
		memset(tp, 0, sizeof(*tp));
		DemuxMemoryFree(tp);
		tp = next;
	}
}

static void InitNetworkData(void)
{
	memset(&SiNetworkData, 0, sizeof(SiNetworkData));
	SiNetworkData.NetworkId = INVALID_ID;
}

static void ClearNetworkData(void)
{
	DeleteAllNitTp();
	InitNetworkData();
}

/* Append a transponder to the network data. */
static SiNitTpData_t *AddNitTp(SiNitTpType type, U16 tsId, U16 onId)
{
	SiNitTpData_t **link = &SiNetworkData.NitTpData;
	SiNitTpData_t  *tp;

	while (*link != NULL)
		link = &(*link)->Next_p;
	tp = (SiNitTpData_t *)DemuxMemoryAllocate(sizeof(*tp));
	if (tp == NULL)
		return NULL;
	memset(tp, 0, sizeof(*tp));
	tp->TsId   = tsId;
	tp->OnId   = onId;
	tp->Next_p = NULL;
	tp->TpType = type;
	SiNetworkData.NumberOfTp++;
	*link = tp;
	return tp;
}

/* ---- service (PAT / SDT) data ------------------------------------------------------- */

static void DeleteAllProgramData(void)
{
	SiProgramData_t *program;

	while (SiServiceData.ProgramData != NULL)
	{
		program                   = SiServiceData.ProgramData;
		SiServiceData.ProgramData = program->Next_p;
		memset(program, 0, sizeof(*program));
		DemuxMemoryFree(program);
	}
}

static void InitServiceData(void)
{
	SiServiceData.ONId            = INVALID_ID;
	SiServiceData.TsId            = INVALID_ID;
	SiServiceData.NumberOfProgram = 0;
	SiServiceData.ProgramData     = NULL;
	memset(SiServiceData.ServiceName, 0, MAX_NAME_LEN);
}

static void ClearServiceData(void)
{
	DeleteAllProgramData();
	InitServiceData();
	memset(&NvodSdtNameList, 0, sizeof(NvodSdtNameList));
	NvodShiftServiceFound = 0;
	NvodCount             = 0;
	NvodRefFound          = 0;
	NvodShiftRefProgramNo = INVALID_ID;
}

static void ClearProgramData(SiProgramData_t *program)
{
	U8 index;

	if (program == NULL)
		return;
	program->ServideId       = INVALID_ID;
	program->PmtPid          = INVALID_PID;
	program->PcrPid          = INVALID_PID;
	program->ServiceType     = DATA_SERVICE;
	program->NumberOfEs      = 0;
	program->ChannelScramble = 0;
	memset(program->ChannelName, 0, MAX_NAME_LEN);
	for (index = 0; index < MAX_NUMBER_OF_ES; index++)
	{
		program->EsData[index].EsPid      = INVALID_PID;
		program->EsData[index].EsType     = SI_ES_UNKNOWN;
		memset(program->EsData[index].EsLang, 0, sizeof(program->EsData[index].EsLang));
		program->EsData[index].EsSubData.EsSubType                   = 0xff;
		program->EsData[index].EsSubData.SubtitleData.CompositionPage = 0;
		program->EsData[index].EsSubData.SubtitleData.AncillaryPage   = 0;
		program->EsData[index].EsSubData.TeletextData.MagazineNumber  = 0;
		program->EsData[index].EsSubData.TeletextData.PageNumber      = 0;
	}
	program->Next_p = NULL;
}

static SiProgramData_t *AllocateNewProgram(U16 serviceId, U16 pmtPid)
{
	SiProgramData_t *program = (SiProgramData_t *)DemuxMemoryAllocate(sizeof(*program));

	if (program == NULL)
		return NULL;
	ClearProgramData(program);
	program->ServideId = serviceId;
	program->PmtPid    = pmtPid;
	return program;
}

/* Insert a program sorted by service id. */
static SiProgramData_t *AddServiceProgram(U16 serviceId, U16 pmtPid)
{
	SiProgramData_t **link = &SiServiceData.ProgramData;
	SiProgramData_t  *program;

	while ((*link != NULL) && ((*link)->ServideId <= serviceId))
		link = &(*link)->Next_p;
	program = AllocateNewProgram(serviceId, pmtPid);
	if (program != NULL)
	{
		program->Next_p = *link;
		*link           = program;
	}
	return program;
}

/* Stop the whole scan / live processing. */
static void TableStopAllSection(void)
{
	TDTProcessed = 0;
	PATProcessed = 0;
	PMTProcessed = 0;
	NITProcessed = 0;
	SDTProcessed = 0;
	ClearServiceData();
	TableStopSection(&PATProcessData);
	TableStopSection(&NITProcessData);
	TableStopSection(&SDTProcessData);
	TableStopSection(&TDTProcessData);
	StopAllPmt();
	StopAllLiveProcess();
	TableStopSection(&LivePMTProcessData);
	DemuxStopAllTable();
}

/* Abort the scan (no PAT) and report an empty result. */
static void TableStopSearchByEvent(void)
{
	if (!SearchNotLive)
		return;
	TableStopAllSection();
	SearchNotLive = 0;
	if (SearchCallBack != NULL)
		SearchCallBack(NULL);
}

/* Section callback of the SI tables: queue the section for the parse task. */
static void TableCallBack(U8 infoId, U8 *data, U32 size)
{
	SiProcessMessage_t message;

	message.InfoId = infoId;
	message.Length = size;
	message.Data   = data;
	DemuxSendTableProcessMessage(message);
}

/* Start one table and record it in process. FALSE = ok. */
static BOOL TableStartSearch(DemuxStartParam_t *param, TableProcessData_t *process)
{
	U8 infoId = DEMUX_INFO_NONE;

	if ((DemuxStartTable(param, &infoId) == FALSE) && (infoId < DEMUX_INFO_NUMBER))
	{
		process->InfoId = infoId;
		process->Active = 1;
		process->Count  = 0;
		return FALSE;
	}
	process->Count  = 0;
	process->InfoId = DEMUX_INFO_NONE;
	process->Active = 0;
	return TRUE;
}

static BOOL TableStartPatSearch(U8 tunerId, U8 channelMode)
{
	U8                match[8] = { PAT_TABLE_ID };
	U8                mask[8]  = { 0xff };
	DemuxStartParam_t param;

	memset(&param, 0, sizeof(param));
	param.Pid            = PAT_PID;
	param.ChannelId      = INVALID_ID;
	param.TunerId        = tunerId;
	param.TableId        = PAT_TABLE_ID;
	param.NumberOfFilter = 1;
	param.ChannelMode    = channelMode;
	param.CrcEnable      = 1;
	param.Match          = match;
	param.Mask           = mask;
	param.CallBack       = TableCallBack;
	return TableStartSearch(&param, &PATProcessData);
}

/* PMT of serviceId (0xffff = any); version <= 31 waits for a different version. */
static BOOL TableStartPmtSearch(U16 pid, U16 serviceId, U8 version, U8 tunerId, U8 channelMode,
                                TableProcessData_t *process)
{
	U8                match[8]   = { PMT_TABLE_ID };
	U8                mask[8]    = { 0xff };
	U8                notMask[8] = { 0 };
	DemuxStartParam_t param;

	memset(&param, 0, sizeof(param));
	if (serviceId != INVALID_ID)
	{
		match[1] = (U8)(serviceId >> 8);
		match[2] = (U8)serviceId;
		mask[1]  = 0xff;
		mask[2]  = 0xff;
	}
	if (version <= 31)
	{
		notMask[3]    = 0x3e;
		match[3]      = (U8)((version << 1) & 0x3e);
		mask[3]       = 0;
		param.NotMask = notMask;
	}
	param.Pid            = pid;
	param.ChannelId      = serviceId;
	param.TunerId        = tunerId;
	param.TableId        = PMT_TABLE_ID;
	param.NumberOfFilter = 1;
	param.ChannelMode    = channelMode;
	param.CrcEnable      = 1;
	param.Match          = match;
	param.Mask           = mask;
	param.CallBack       = TableCallBack;
	return TableStartSearch(&param, process);
}

static BOOL TableStartNitSearch(U8 tunerId, U8 nitMode, U8 channelMode)
{
	U8                match[8] = { NIT_ACTUAL_TABLE_ID };
	U8                mask[8]  = { 0xff };
	DemuxStartParam_t param;

	(void)nitMode;
	memset(&param, 0, sizeof(param));
	param.Pid            = NIT_PID;
	param.ChannelId      = INVALID_ID;
	param.TunerId        = tunerId;
	param.TableId        = NIT_ACTUAL_TABLE_ID;
	param.NumberOfFilter = 1;
	param.ChannelMode    = channelMode;
	param.CrcEnable      = 1;
	param.Match          = match;
	param.Mask           = mask;
	param.CallBack       = TableCallBack;
	return TableStartSearch(&param, &NITProcessData);
}

/* SDT actual; tsId 0xff (as called by the scan) means any transport stream. */
static BOOL TableStartSdtSearch(U8 tunerId, U16 tsId, U8 channelMode)
{
	U8                match[8] = { SDT_ACTUAL_TABLE_ID };
	U8                mask[8]  = { 0xff };
	DemuxStartParam_t param;

	memset(&param, 0, sizeof(param));
	if (tsId != 0xff)
	{
		match[1] = (U8)(tsId >> 8);
		match[2] = (U8)tsId;
		mask[1]  = 0xff;
		mask[2]  = 0xff;
	}
	param.Pid            = SDT_PID;
	param.ChannelId      = tsId;
	param.TunerId        = tunerId;
	param.TableId        = SDT_ACTUAL_TABLE_ID;
	param.NumberOfFilter = 1;
	param.ChannelMode    = channelMode;
	param.CrcEnable      = 1;
	param.Match          = match;
	param.Mask           = mask;
	param.CallBack       = TableCallBack;
	return TableStartSearch(&param, &SDTProcessData);
}

/* ---- PAT ---------------------------------------------------------------------------- */

/* Parse a PAT into PatData. FALSE = ok. */
static BOOL PatProcess(U8 *data, U32 length)
{
	U32 total;
	U32 end;
	U32 index;
	U16 program;

	memset(&PatData, 0, sizeof(PatData));
	if (data[0] != PAT_TABLE_ID)
	{
		PatData.NumberOfProgram = 0;
		PatData.TsId            = INVALID_ID;
		return TRUE;
	}
	total = DemuxGetInfoLength(&data[1], data[0]) + 3;
	if ((length < total) || (total <= 11))
	{
		PatData.NumberOfProgram = 0;
		PatData.TsId            = INVALID_ID;
		return TRUE;
	}
	PatData.TsId = DemuxGetStreamId(&data[3]);
	end = total - 4;
	for (index = 8; index < end; index += 4)
	{
		program = DemuxGetStreamId(&data[index]);
		if (program == 0)
			continue;                     /* network PID */
		PatData.ServiceId[PatData.NumberOfProgram] = program;
		PatData.PmtPid[PatData.NumberOfProgram]    = DemuxGetPid(&data[index + 2]);
		PatData.NumberOfProgram++;
		if (PatData.NumberOfProgram >= MAX_PAT_PROGRAM)
			break;
	}
	return FALSE;
}

static U16 GetPmtPidByServiceId(U16 serviceId)
{
	U8 index;

	for (index = 0; index < PatData.NumberOfProgram; index++)
	{
		if (PatData.ServiceId[index] == serviceId)
			return PatData.PmtPid[index];
	}
	return INVALID_PID;
}

/* Programs from the PAT; one PmtList entry per distinct PMT PID. Returns the PID count. */
static U8 GetPmtdataFromPat(void)
{
	U8 program;
	U8 index;

	ClearAllPmtList();
	SiServiceData.NumberOfProgram = PatData.NumberOfProgram;
	SiServiceData.TsId            = PatData.TsId;
	PmtNumberOfPid                = 0;
	for (program = 0; program < PatData.NumberOfProgram; program++)
	{
		AddServiceProgram(PatData.ServiceId[program], PatData.PmtPid[program]);
		for (index = 0; index < PmtNumberOfPid; index++)
		{
			if (PmtList[index].Pid == PatData.PmtPid[program])
				break;
		}
		if (index < PmtNumberOfPid)
		{
			PmtList[index].Count++;
		}
		else
		{
			PmtList[index].Pid      = PatData.PmtPid[program];
			PmtList[index].Received = 0;
			PmtList[index].Count    = 1;
			PmtNumberOfPid++;
		}
	}
	return PmtNumberOfPid;
}

static U8 FindPmtLisByPid(U16 pid)
{
	U8 index;

	for (index = 0; index < PmtNumberOfPid; index++)
	{
		if (PmtList[index].Pid == pid)
			return index;
	}
	return PmtNumberOfPid;
}

static SiProgramData_t *FindProgramDatabyServiceId(U16 serviceId)
{
	SiProgramData_t *program = SiServiceData.ProgramData;
	U8               count   = 0;

	while ((program != NULL) && (count < SiServiceData.NumberOfProgram))
	{
		count++;
		if (program->ServideId == serviceId)
			return program;
		program = program->Next_p;
	}
	return NULL;
}

/* ---- PMT ---------------------------------------------------------------------------- */

static U8 SelectAc3Langage(U8 code)
{
	U8 index;

	for (index = 0; index < 11; index++)
	{
		if ((AtscAc3AudioLang[index].Code == code) || (AtscAc3AudioLang[index].Code == 0xff))
			return index;
	}
	return 10;
}

/* Elementary stream type from the PMT stream_type. */
static void PmtGetEsType(U8 streamType, SiEsData_t *es)
{
	switch (streamType)
	{
		case 0x01:
		case 0x02:
			es->EsSubData.EsSubType = VIDEO_TYPE_MPEG2;
			es->EsType              = SI_ES_VIDEO;
			break;
		case 0x10:
		case 0x1b:
			es->EsSubData.EsSubType = VIDEO_TYPE_H264;
			es->EsType              = SI_ES_VIDEO;
			break;
		case 0x03:
		case 0x04:
			es->EsSubData.EsSubType = AUDIO_TYPE_MPEG;
			es->EsType              = SI_ES_AUDIO;
			break;
		case 0x0f:
			es->EsSubData.EsSubType = AUDIO_TYPE_AAC;
			es->EsType              = SI_ES_AUDIO;
			break;
		case 0x11:
			es->EsSubData.EsSubType = AUDIO_TYPE_LATM;
			es->EsType              = SI_ES_AUDIO;
			break;
		case 0x6a:
		case 0x6b:
		case 0x81:
			es->EsSubData.EsSubType = AUDIO_TYPE_AC3;
			es->EsType              = SI_ES_AUDIO;
			break;
		case 0x06:
			/* Private PES: teletext / subtitles / AC-3, decided by the descriptors. */
			es->EsType = SI_ES_TELETEXT;
			break;
		default:
			es->EsType = SI_ES_UNKNOWN;
			break;
	}
}

/* ES descriptors: CA, language, teletext, subtitling, AC-3. desc points at the tag. */
static void PmtProcessDescriptor(SiProgramData_t *program, SiEsData_t *es, U8 *desc, U8 tag)
{
	U8 type;

	switch (tag)
	{
		case 0x09:                                    /* CA */
			program->ChannelScramble = 1;
			break;
		case 0x0a:                                    /* ISO 639 language */
			memcpy(es->EsLang, &desc[2], 3);
			break;
		case 0x56:                                    /* teletext */
			if (es->EsType != SI_ES_TELETEXT)
				break;
			memcpy(es->EsLang, &desc[2], 3);
			type = desc[5] >> 3;
			if (type == TELETEXT_SUB_TITLE)
			{
				type       = TELETEXT_SUBTITLE;
				es->EsType = SI_ES_SUBTITLE;
			}
			es->EsSubData.EsSubType                  = type;
			es->EsSubData.TeletextData.MagazineNumber = desc[5] & 7;
			es->EsSubData.TeletextData.PageNumber     = desc[6];
			break;
		case 0x59:                                    /* DVB subtitling */
			if ((es->EsType != SI_ES_TELETEXT) && (es->EsType != SI_ES_SUBTITLE))
				break;
			memcpy(es->EsLang, &desc[2], 3);
			es->EsSubData.EsSubType                     = DVB_SUBTITLE;
			es->EsType                                  = SI_ES_SUBTITLE;
			es->EsSubData.SubtitleData.CompositionPage = DemuxGetStreamId(&desc[6]);
			es->EsSubData.SubtitleData.AncillaryPage   = DemuxGetStreamId(&desc[8]);
			break;
		case 0x6a:                                    /* AC-3 */
		case 0x6b:                                    /* AC-3 (ANSI) */
			if (es->EsType != SI_ES_AUDIO)
			{
				es->EsSubData.EsSubType = AUDIO_TYPE_AC3;
				es->EsType              = SI_ES_AUDIO;
			}
			break;
		case 0x81:                                    /* ATSC AC-3 */
			es->EsSubData.EsSubType = AUDIO_TYPE_AC3;
			es->EsType              = SI_ES_AUDIO;
			memcpy(es->EsLang, AtscAc3AudioLang[SelectAc3Langage(desc[5])].Lang, 3);
			break;
		default:
			break;
	}
}

/* Fill program from a PMT section. FALSE = ok. */
static BOOL PmtParse(SiProgramData_t *program, U8 *data)
{
	SiEsData_t *es;
	U32         end;
	U32         index;
	U32         infoLength;
	U32         used;
	U8          number = 0;

	if (program == NULL)
		return TRUE;
	end = (U16)(DemuxGetInfoLength(&data[1], PMT_TABLE_ID) + 3);
	if (DemuxGetStreamId(&data[3]) != program->ServideId)
		return TRUE;
	program->PcrPid = DemuxGetPid(&data[8]);
	infoLength      = DemuxGetInfoLength(&data[10], 0xff);
	end            -= 4;

	/* Program descriptors: only the CA descriptor matters. */
	for (index = 12, used = 0; (index < end) && (used < infoLength); )
	{
		if (data[index] == 0x09)
			program->ChannelScramble = 1;
		used  += data[index + 1] + 2;
		index += data[index + 1] + 2;
	}

	while (index < end)
	{
		es = &program->EsData[number];
		PmtGetEsType(data[index], es);
		index++;
		es->EsPid  = DemuxGetPid(&data[index]);
		index     += 2;
		infoLength = DemuxGetInfoLength(&data[index], 0xff);
		index     += 2;
		for (used = 0; (index < end) && (used < infoLength); )
		{
			PmtProcessDescriptor(program, es, &data[index], data[index]);
			used  += data[index + 1] + 2;
			index += data[index + 1] + 2;
		}
		number++;
		if (number >= MAX_NUMBER_OF_ES - 1)
			number = MAX_NUMBER_OF_ES - 1;
	}
	program->NumberOfEs = number;
	return FALSE;
}

/* Scan PMT: parse it once per program; *pmtIndex gets the PmtList entry. FALSE = ok. */
static BOOL PmtProcess(U8 *data, U32 length, U8 *pmtIndex)
{
	SiProgramData_t *program;
	U32              total;
	U8               index;

	if (data[0] != PMT_TABLE_ID)
		return TRUE;
	total = DemuxGetInfoLength(&data[1], data[0]) + 3;
	if ((length < total) || (total <= 11))
		return TRUE;
	program = FindProgramDatabyServiceId(DemuxGetStreamId(&data[3]));
	if (program == NULL)
		return TRUE;
	index = FindPmtLisByPid(program->PmtPid);
	if (index >= PmtNumberOfPid)
		return TRUE;

	if (program->PcrPid != INVALID_PID)
	{
		/* Already parsed: after seeing it twice give up waiting for the others. */
		PmtList[index].Received++;
		if (PmtList[index].Received > 1)
		{
			PmtList[index].Received = 0;
			PmtList[index].Count    = 0;
			*pmtIndex = index;
			return FALSE;
		}
		return TRUE;
	}
	PmtParse(program, data);
	PmtList[index].Count--;
	*pmtIndex = index;
	return FALSE;
}

/* ---- SDT ---------------------------------------------------------------------------- */

/* NVOD name slot of serviceId (the next free slot when unknown). */
static U32 GetNvodListFromServiceId(NvodNameList_t *list, U32 count, U16 serviceId)
{
	U32 index = count;

	while (index > 0)
	{
		index--;
		if (list->ServiceId[index] == serviceId)
			return index;
	}
	return count;
}

static SiServiceType SdtGetServiceType(U8 type)
{
	if (type > 27)
		return DATA_SERVICE;
	if ((1u << type) & 0x01c00032u)     /* 1, 4, 5, 22, 23, 24 */
		return TV_SERVICE;
	if ((1u << type) & 0x00000084u)     /* 2, 7 */
		return RADIO_SERVICE;
	if ((1u << type) & 0x0e020000u)     /* 17, 25, 26, 27 */
		return HDTV_SERVICE;
	return DATA_SERVICE;
}

/* Copy a converted NVOD name. */
static void SdtSaveNvodName(U16 serviceId, const char *name, U32 length)
{
	U32 index = GetNvodListFromServiceId(&NvodSdtNameList, NvodCount, serviceId);

	if ((index >= MAX_NVOD) || (length >= MAX_NAME_LEN))
		return;
	memcpy(NvodSdtNameList.Name[index], name, length);
	NvodSdtNameList.Name[index][length] = 0;
}

/* Service descriptors: name / type (0x48), NVOD reference (0x4b), time shift (0x4c). */
static void SdtProcessDescriptor(SiProgramData_t *program, U8 *desc, U8 tag, U16 serviceId)
{
	U16 code;
	U8  nameLength;
	U8  provider;
	U8 *name;
	U32 length;
	U32 index;

	switch (tag)
	{
		case 0x48:
			if (program != NULL)
				program->ServiceType = SdtGetServiceType(desc[0]);
			provider   = desc[1];
			nameLength = desc[2 + provider];
			name       = &desc[3 + provider];
			if (nameLength == 0)
				break;
			if (program != NULL)
			{
				memset(program->ChannelName, 0, MAX_NAME_LEN);
				length = SiCodeConverter((U8 *)program->ChannelName, name,
				                         (nameLength < 29) ? nameLength : 29, &code);
				if (NvodRefFound)
					SdtSaveNvodName(serviceId, program->ChannelName, length);
			}
			else if (NvodRefFound)
			{
				index = GetNvodListFromServiceId(&NvodSdtNameList, NvodCount, serviceId);
				if (index >= MAX_NVOD)
					break;
				memset(NvodSdtNameList.Name[index], 0, MAX_NAME_LEN);
				length = SiCodeConverter((U8 *)NvodSdtNameList.Name[index], name,
				                         (nameLength < 29) ? nameLength : 29, &code);
				NvodSdtNameList.Name[index][length] = 0;
			}
			break;
		case 0x4b:
			if (NvodCount < MAX_NVOD)
			{
				NvodSdtNameList.ServiceId[NvodCount] = serviceId;
				NvodRefFound = 1;
				NvodCount++;
			}
			break;
		case 0x4c:
			NvodShiftServiceFound = 1;
			NvodShiftRefProgramNo = DemuxGetStreamId(desc);
			break;
		default:
			break;
	}
}

/* Walk the buffered SDT sections of the scanned TS. FALSE = ok. */
static BOOL SdtProcess(void)
{
	TableBuffer_t   *buffer;
	TableBuffer_t   *next;
	SiProgramData_t *program;
	U8              *data;
	U32              total;
	U32              end;
	U32              index;
	U32              loopLength;
	U32              used;
	U16              serviceId;
	U8               tag;
	U8               length;

	buffer = FindBuffer(CurrentSdtData_p, SiServiceData.TsId, 0);
	if (buffer == NULL)
		return TRUE;

	for (;;)
	{
		data  = buffer->Data;
		total = DemuxGetInfoLength(&data[1], SDT_ACTUAL_TABLE_ID) + 3;
		if (((data[0] & 0xfb) == SDT_ACTUAL_TABLE_ID) && (buffer->Length >= total) && (total > 11))
		{
			if (data[6] != buffer->SectionNo)
				return TRUE;
			SiServiceData.ONId = DemuxGetStreamId(&data[8]);
			end = total - 4;
			for (index = 11; index < end; )
			{
				serviceId  = DemuxGetStreamId(&data[index]);
				index     += 3;
				loopLength = DemuxGetInfoLength(&data[index], 0xff);
				index     += 2;
				program    = FindProgramDatabyServiceId(serviceId);
				if (program == NULL)
				{
					index += loopLength;
					continue;
				}
				for (used = 0; (index < end) && (used < loopLength); )
				{
					tag     = data[index++];
					length  = data[index++];
					SdtProcessDescriptor(program, &data[index], tag, serviceId);
					index  += length;
					used   += length + 2;
				}
			}
		}

		next = buffer->Next;
		if ((next == NULL) || (buffer->SectionNo >= buffer->LastSectionNo) ||
		    (buffer->Id != next->Id) || (buffer->SectionNo == next->SectionNo))
			return FALSE;
		buffer = next;
	}
}

/* ---- NIT ---------------------------------------------------------------------------- */

/* Delivery system descriptors: satellite (0x43), cable (0x44), terrestrial (0x5a). */
static void NitProcessDescriptor(U8 *desc, U8 tag, U16 tsId, U16 onId)
{
	SiNitTpData_t *tp;

	switch (tag)
	{
		case 0x43:
			tp = AddNitTp(SI_TP_SATELLITE, tsId, onId);
			if (tp == NULL)
				break;
			tp->SatTpData.Frequency    = BcdArray2Word(desc, 3);
			tp->SatTpData.Orbit        = (U16)BcdArray2Word(&desc[4], 2);
			tp->SatTpData.EastNotWest  = desc[6] >> 7;
			tp->SatTpData.Polarization = (desc[6] >> 5) & 3;
			tp->SatTpData.SymbolRate   = BcdArray2Word(&desc[7], 3);
			break;
		case 0x44:
			tp = AddNitTp(SI_TP_CABLE, tsId, onId);
			if (tp == NULL)
				break;
			tp->CableTpData.Frequency  = BcdArray2Word(desc, 2);
			tp->CableTpData.Modulation = desc[6];
			tp->CableTpData.SymbolRate = BcdArray2Word(&desc[7], 3);
			break;
		case 0x5a:
			tp = AddNitTp(SI_TP_TERRESTIAL, tsId, onId);
			if (tp == NULL)
				break;
			tp->TerrTpData.Frequency     = (Array2Word(desc, 4) >> 5) / 3125;
			tp->TerrTpData.BandWidth     = desc[4] >> 5;
			tp->TerrTpData.Constellation = desc[5] >> 6;
			tp->TerrTpData.TxMode        = (desc[6] >> 1) & 3;
			break;
		default:
			break;
	}
}

/* Parse the buffered NIT sections: network name and, if requested, the transponders. */
static void NitProcess(U8 withTp)
{
	TableBuffer_t *buffer;
	U8            *data;
	U32            total;
	U32            index;
	U32            loopLength;
	U32            used;
	U32            tsLoopLength;
	U32            tsUsed;
	U16            tsId;
	U16            onId;
	U16            code;
	U8             tag;
	U8             length;

	for (buffer = NitData_p; buffer != NULL; buffer = buffer->Next)
	{
		data  = buffer->Data;
		total = DemuxGetInfoLength(&data[1], SDT_ACTUAL_TABLE_ID) + 3;
		if (((data[0] & 0xfe) != NIT_ACTUAL_TABLE_ID) || (buffer->Length < total) || (total <= 10))
			continue;

		SiNetworkData.NetworkId = DemuxGetStreamId(&data[3]);
		loopLength = DemuxGetInfoLength(&data[8], 0xff);
		for (index = 10, used = 0; (index < total) && (used < loopLength); )
		{
			tag    = data[index++];
			length = data[index++];
			if (tag == 0x40)                          /* network name */
			{
				memset(SiNetworkData.NetworkName, 0, MAX_NAME_LEN);
				SiCodeConverter((U8 *)SiNetworkData.NetworkName, &data[index],
				                (length < 29) ? length : 29, &code);
			}
			index += length;
			used  += length + 2;
		}
		if (!withTp)
			return;

		tsLoopLength = DemuxGetInfoLength(&data[index], 0xff);
		index += 2;
		for (tsUsed = 0; (index < total) && (tsUsed < tsLoopLength); )
		{
			tsId        = DemuxGetStreamId(&data[index]);
			index      += 2;
			onId        = DemuxGetStreamId(&data[index]);
			index      += 2;
			loopLength  = DemuxGetInfoLength(&data[index], 0xff);
			index      += 2;
			for (used = 0; (index < total) && (used < loopLength); )
			{
				tag    = data[index++];
				length = data[index++];
				NitProcessDescriptor(&data[index], tag, tsId, onId);
				index += length;
				used  += length + 2;
			}
			tsUsed = 0;   /* the original only bounds the loop by the section length */
		}
	}
}

/* Section header: table id extension, section number, last section number. FALSE = ok. */
static BOOL TablePreProcess(U8 *data, U32 length, U16 *id, U8 *sectionNo, U8 *lastSectionNo,
                            U16 *extension)
{
	if (data == NULL)
		return TRUE;
	if (length < (U32)(DemuxGetInfoLength(&data[1], data[0]) + 3))
		return TRUE;
	*id            = DemuxGetStreamId(&data[3]);
	*extension     = DemuxGetStreamId(&data[8]);
	*sectionNo     = data[6];
	*lastSectionNo = data[7];
	return FALSE;
}

/* A PMT PID finished; TRUE when it was the last one. */
static BOOL TableControlPmt(U8 index, U8 *remaining)
{
	U8 left = *remaining;

	TableStopPmt(index);
	left--;
	if (left != 0)
	{
		*remaining = left;
		return FALSE;
	}
	StopAllPmt();
	PMTProcessed = 1;
	*remaining   = 0;
	return TRUE;
}

/* Count one timeout tick; TRUE (and the table stopped) once max ticks passed. */
static BOOL TableCheckTableTimeOut(TableProcessData_t *process, U32 max)
{
	if (!process->Active)
		return FALSE;
	if (process->Count <= max)
	{
		process->Count++;
		return FALSE;
	}
	TableStopSection(process);
	process->Count = 0;
	return TRUE;
}

/* PMT timeouts; TRUE when any PMT timed out (slots were freed). */
static BOOL TableCheckPmtTimeOut(U8 *remaining)
{
	BOOL timedOut = FALSE;
	U8   index;

	for (index = 0; index < PmtNumberOfPid; index++)
	{
		if (PmtList[index].Count == 0)
			continue;
		if (!TableCheckTableTimeOut(&PmtList[index].Process, PMT_TIMEOUT))
			continue;
		PmtList[index].Count = 0;
		timedOut = TRUE;
		if (TableControlPmt(index, remaining))
			return TRUE;
	}
	return timedOut;
}

/* Copy the network name as service name when the SDT has none. */
static void TableCopyNetworkName(void)
{
	memcpy(SiServiceData.ServiceName, SiNetworkData.NetworkName, MAX_NAME_LEN);
}

/*
 * TableSiProcessTask: handles queued sections and timeouts for the channel scan
 * (SearchNotLive) and for the live PAT / PMT.
 */
void TableSiParseProcess(void *param)
{
	SiProcessMessage_t message;
	U32                timeout = 1000;
	U16                currentTsId = INVALID_ID;
	U16                livePmtPid = INVALID_PID;
	U16                channelId;
	U16                id;
	U16                extension;
	U8                 liveVersion = LIVE_PMT_VERSION_NONE;
	U8                 pmtCount = 0;         /* PMT PIDs of the scan */
	U8                 pmtStarted = 0;       /* PMT searches started so far */
	U8                 pmtRemaining = 0;
	U8                 startPmt = 0;         /* start more PMT searches */
	U8                 tableId;
	U8                 tunerId = 0;
	U8                 channelMode = 0;
	U8                 sectionNo;
	U8                 lastSectionNo;
	U8                 pmtIndex;
	U8                 version;
	U8                 result;
	BOOL               none;

	(void)param;
	SatSearchMode = 0;
	SatelliteType = 1;
	SearchNotLive = 0;
	TableStopAllSection();

	for (;;)
	{
		none = DemuxReceiveTableProcessMessage(&message, timeout);
		DemuxTableProcessLock();

		if (none)
		{
			/* ---- timeouts ---- */
			if (!SearchNotLive)
			{
				currentTsId = INVALID_ID;
			}
			else
			{
				if (!PATProcessed && TableCheckTableTimeOut(&PATProcessData, PAT_TIMEOUT))
				{
					/* No PAT on this TP: finish the scan. */
					currentTsId = INVALID_ID;
					pmtCount    = 1;
					TableStopAllSection();
					startPmt     = 1;
					PATProcessed = 1;
					PMTProcessed = 1;
					NITProcessed = 1;
					SDTProcessed = 1;
					pmtStarted   = 1;
				}
				if (!PMTProcessed && TableCheckPmtTimeOut(&pmtRemaining))
				{
					if (pmtRemaining == 0)
					{
						pmtCount   = 0;
						pmtStarted = 0;
					}
					else
					{
						startPmt = 1;
					}
				}
				if (!NITProcessed &&
				    TableCheckTableTimeOut(&NITProcessData,
				                           (NitSearchOn && TpSearchOn) ? NIT_TP_TIMEOUT : NIT_TIMEOUT))
				{
					NITProcessed = 1;
					NitProcess(NitSearchOn);
					startPmt = 1;
					if (NitSearchOn)
						DemuxSendTableNotifyMessage(SI_NOTIFY_NIT);
					if (PATProcessed)
						TableCopyNetworkName();
				}
				if (!SDTProcessed && TableCheckTableTimeOut(&SDTProcessData, SDT_TIMEOUT))
				{
					SDTProcessed = 1;
					startPmt     = 1;
				}
			}
		}
		else if (DemuxGetTableData(message.InfoId, &tableId, &tunerId, &channelMode, &channelId)
		         == FALSE)
		{
			if (SearchNotLive)
			{
				/* ---- channel scan ---- */
				switch (tableId)
				{
					case PAT_TABLE_ID:
						TableStopSection(&PATProcessData);
						PatProcess(message.Data, message.Length);
						if (PatData.NumberOfProgram == 0)
						{
							TableStopSearchByEvent();
							break;
						}
						currentTsId = PatData.TsId;
						pmtCount    = GetPmtdataFromPat();
						SI_DEBUG("scan PAT: ts %u, %u programs, %u PMT PIDs\n", currentTsId,
						         PatData.NumberOfProgram, pmtCount);
						ClearUnusedTableBuffer(&CurrentSdtData_p, currentTsId);
						if (CheckTableBufferFinished(CurrentSdtData_p, currentTsId))
						{
							TableStopSection(&SDTProcessData);
							SDTReceived = 1;
						}
						if (SDTReceived)
						{
							SDTProcessed = 1;
							SDTReceived  = 0;
							SdtProcess();
						}
						if (NITProcessed)
							TableCopyNetworkName();
						startPmt     = 1;
						pmtRemaining = pmtCount;
						pmtStarted   = 0;
						PMTProcessed = 0;
						PATProcessed = 1;
						break;

					case PMT_TABLE_ID:
						if (!PATProcessed)
							break;
						if (PmtProcess(message.Data, message.Length, &pmtIndex))
							break;
						PmtList[pmtIndex].Process.Count = 0;
						if (PmtList[pmtIndex].Count != 0)
							break;
						if (TableControlPmt(pmtIndex, &pmtRemaining))
						{
							pmtCount   = 0;
							pmtStarted = 0;
						}
						else
						{
							startPmt = 1;
						}
						break;

					case NIT_ACTUAL_TABLE_ID:
						if (NITProcessed)
						{
							TableStopSection(&NITProcessData);
							startPmt = 1;
							break;
						}
						if (TablePreProcess(message.Data, message.Length, &id, &sectionNo,
						                    &lastSectionNo, &extension))
						{
							if (!NitSearchOn)
								NITProcessed = 1;
							SatelliteType = 1;
						}
						else
						{
							NITProcessData.Count = 0;
							result = AddData2List(&NitData_p, id, message.Length, sectionNo,
							                      lastSectionNo, message.Data);
							if ((result == LIST_DUPLICATE) || (result == LIST_DUPLICATE + 1) ||
							    !NitSearchOn || CheckTableBufferFinished(NitData_p, id))
								NITProcessed = 1;
						}
						if (NITProcessed)
						{
							TableStopSection(&NITProcessData);
							startPmt = 1;
							NitProcess(NitSearchOn);
							if (PATProcessed)
								TableCopyNetworkName();
							if (NitSearchOn)
								DemuxSendTableNotifyMessage(SI_NOTIFY_NIT);
						}
						break;

					case SDT_ACTUAL_TABLE_ID:
						if (SDTProcessed)
						{
							TableStopSection(&SDTProcessData);
							startPmt = 1;
							break;
						}
						TablePreProcess(message.Data, message.Length, &id, &sectionNo,
						                &lastSectionNo, &extension);
						if (PATProcessed && (id != currentTsId))
							break;
						SDTProcessData.Count = 0;
						result = AddData2List(&CurrentSdtData_p, id, message.Length, sectionNo,
						                      lastSectionNo, message.Data);
						if (result == LIST_DUPLICATE)
						{
							TableStopSection(&SDTProcessData);
							startPmt = 1;
							if (!PATProcessed)
							{
								SDTProcessed = 0;
								SDTReceived  = 1;
							}
							else
							{
								SdtProcess();
								SDTProcessed = 1;
							}
						}
						else if (PATProcessed && CheckTableBufferFinished(CurrentSdtData_p, currentTsId))
						{
							TableStopSection(&SDTProcessData);
							SDTProcessed = 1;
							startPmt     = 1;
							SdtProcess();
						}
						break;

					case TDT_TABLE_ID:
						TableStopSection(&TDTProcessData);
						break;

					default:
						break;
				}
			}
			else
			{
				/* ---- live service ---- */
				if (tableId == PAT_TABLE_ID)
				{
					TableStopSection(&PATProcessData);
					PatProcess(message.Data, message.Length);
					liveVersion = LIVE_PMT_VERSION_NONE;
					SI_DEBUG("live PAT: ts %u, %u programs\n", PatData.TsId, PatData.NumberOfProgram);
					if (PatData.NumberOfProgram != 0)
					{
						livePmtPid = GetPmtPidByServiceId(SiLiveProgramServiceId);
						if (livePmtPid <= 0x1ffe)
						{
							SiLiveProgramData.ServideId = SiLiveProgramServiceId;
							TableStartPmtSearch(livePmtPid, SiLiveProgramServiceId, liveVersion,
							                    tunerId, channelMode, &LivePMTProcessData);
						}
					}
				}
				else if (tableId == PMT_TABLE_ID)
				{
					TableStopSection(&LivePMTProcessData);
					version = (message.Data[5] & 0x3e) >> 1;
					if (version != liveVersion)
					{
						if (PmtParse(&SiLiveProgramData, message.Data))
						{
							liveVersion = LIVE_PMT_VERSION_NONE;
						}
						else
						{
							PmtSize = message.Length;
							if (message.Length >= MAX_PMT_LENGTH)
							{
								liveVersion = LIVE_PMT_VERSION_NONE;
							}
							else
							{
								memcpy(PmtData, message.Data, message.Length);
								DemuxSendTableNotifyMessage(SI_NOTIFY_LIVE_PMT);
								liveVersion = version;
							}
						}
					}
					/* Keep watching the PMT for the next version. */
					TableStartPmtSearch(livePmtPid, SiLiveProgramServiceId, liveVersion, tunerId,
					                    channelMode, &LivePMTProcessData);
				}
			}
			/* The section has been consumed. */
			DemuxUpdateLinearPoint(message.Length, FALSE);
		}

		/* ---- scan completion and PMT scheduling ---- */
		if (SearchNotLive && PATProcessed && PMTProcessed && SDTProcessed)
		{
			if (!NitSearchOn && !NITProcessed)
			{
				TableStopSection(&NITProcessData);
				NITProcessed = 1;
			}
			if (NITProcessed)
			{
				DemuxSendTableNotifyMessage(SI_NOTIFY_SEARCH);
				currentTsId   = INVALID_ID;
				SearchNotLive = 0;
			}
		}
		if (startPmt)
		{
			startPmt = 0;
			while (pmtStarted < pmtCount)
			{
				if (TableStartPmtSearch(PmtList[pmtStarted].Pid, INVALID_ID,
				                        LIVE_PMT_VERSION_NONE, tunerId, channelMode,
				                        &PmtList[pmtStarted].Process))
					break;                    /* no free slot: retry later */
				pmtStarted++;
			}
		}
		timeout = 100;
		DemuxTableProcessRelease();
	}
}

/* TableSiNotifyTask: deliver results to mvapp outside the parse task. */
void SendResultProcess(void *param)
{
	U32 type;

	(void)param;
	for (;;)
	{
		if (DemuxReceiveTableNotifyMessage(&type))
			continue;
		switch (type)
		{
			case SI_NOTIFY_SEARCH:
				SI_DEBUG("scan done: ts %u onid %u, %u programs, name '%.30s'\n",
				         SiServiceData.TsId, SiServiceData.ONId, SiServiceData.NumberOfProgram,
				         SiServiceData.ServiceName);
				if (SearchCallBack != NULL)
					SearchCallBack(&SiServiceData);
				break;
			case SI_NOTIFY_LIVE_PMT:
				SI_DEBUG("live PMT: service %u pcr %u, %u ES, scrambled %u, %u bytes\n",
				         SiLiveProgramData.ServideId, SiLiveProgramData.PcrPid,
				         SiLiveProgramData.NumberOfEs, SiLiveProgramData.ChannelScramble,
				         (unsigned)PmtSize);
				if (LiveSearchCallBack != NULL)
					LiveSearchCallBack(&SiLiveProgramData, PmtData, PmtSize);
				break;
			case SI_NOTIFY_NIT:
				SI_DEBUG("NIT: network %u '%.30s', %u TPs\n", SiNetworkData.NetworkId,
				         SiNetworkData.NetworkName, SiNetworkData.NumberOfTp);
				if ((NitSarchCallBack != NULL) && (SiNetworkData.NumberOfTp != 0))
					NitSarchCallBack(&SiNetworkData);
				break;
			default:
				break;
		}
	}
}

/* ---- public API (tableApi.h) -------------------------------------------------------- */

BOOL SiGetTableInfo(U8 tableInfoId, U8 *tableId, U8 *tunerId, U8 *channelMode, U16 *channelId)
{
	BOOL result;

	if (tableInfoId >= DEMUX_INFO_NUMBER)
		return TRUE;
	DemuxTableProcessLock();
	result = DemuxGetTableData(tableInfoId, tableId, tunerId, channelMode, channelId);
	DemuxTableProcessRelease();
	return result;
}

void SiUnRegisterNitCallBack(void)
{
	DemuxTableProcessLock();
	NitSarchCallBack = NULL;
	DemuxTableProcessRelease();
}

void SiUnRegisterLiveSearchCallBack(void)
{
	DemuxTableProcessLock();
	LiveSearchCallBack = NULL;
	DemuxTableProcessRelease();
}

void SiUnRegisterSearchCallBack(void)
{
	DemuxTableProcessLock();
	SearchCallBack = NULL;
	DemuxTableProcessRelease();
}

void SiRegisterNitCallBack(NitResult_f callback)
{
	DemuxTableProcessLock();
	NitSarchCallBack = callback;
	DemuxTableProcessRelease();
}

void SiRegisterLiveSearchCallBack(LiveSearchResult_f callback)
{
	DemuxTableProcessLock();
	LiveSearchCallBack = callback;
	DemuxTableProcessRelease();
}

void SiRegisterSearchCallBack(SearchResult_f callback)
{
	DemuxTableProcessLock();
	SearchCallBack = callback;
	DemuxTableProcessRelease();
}

BOOL SiStopLiveSection(U8 tableInfoId)
{
	if (tableInfoId >= DEMUX_INFO_NUMBER)
		return FALSE;
	DemuxTableProcessLock();
	TableStopLiveSection(tableInfoId);
	DemuxTableProcessRelease();
	return FALSE;
}

BOOL SiPauseLiveSection(U8 tableInfoId)
{
	if (tableInfoId >= DEMUX_INFO_NUMBER)
		return FALSE;
	DemuxTableProcessLock();
	TablePauseLiveSection(tableInfoId);
	DemuxTableProcessRelease();
	return FALSE;
}

/* Start an mvapp section filter (not during a channel scan). FALSE = ok. */
BOOL SiStartLiveSection(U8 *infoId, U16 pid, U16 tableId, U8 tunerId, U8 channelMode,
                        U8 numberOfFilter, U8 crcEnable, U8 *matchData, U8 *matchMask,
                        U8 *notMask, DemuxCallback_f callBack)
{
	DemuxStartParam_t  param;
	LiveProcessData_t *live;
	U8                 id = DEMUX_INFO_NONE;

	DemuxTableProcessLock();
	if (SearchNotLive)
	{
		DemuxTableProcessRelease();
		*infoId = DEMUX_INFO_NONE;
		return TRUE;
	}

	memset(&param, 0, sizeof(param));
	param.Pid            = pid;
	param.ChannelId      = INVALID_ID;
	param.TunerId        = tunerId;
	param.TableId        = (U8)tableId;
	param.NumberOfFilter = numberOfFilter;
	param.ChannelMode    = channelMode;
	param.CrcEnable      = crcEnable;
	param.Match          = matchData;
	param.Mask           = matchMask;
	param.NotMask        = notMask;
	param.CallBack       = LiveTableCallBack;
	if ((DemuxStartTable(&param, &id) != FALSE) || (id >= DEMUX_INFO_NUMBER))
	{
		*infoId = id;
		DemuxTableProcessRelease();
		return TRUE;
	}
	*infoId = id;

	live = TableAddLiveSection();
	if (live == NULL)
	{
		DemuxStopTable(id);
		*infoId = DEMUX_INFO_NONE;
		DemuxTableProcessRelease();
		return TRUE;
	}
	live->InfoId   = id;
	live->CallBack = callBack;
	live->Pid      = pid;
	live->Active   = 1;
	DemuxTableProcessRelease();
	return FALSE;
}

/* Copy the current live PMT; returns its length. */
U32 SiGetLivePmtData(U8 *buffer)
{
	U32 size;

	DemuxTableProcessLock();
	size = PmtSize;
	if (size != 0)
		memcpy(buffer, PmtData, size);
	DemuxTableProcessRelease();
	return size;
}

void SiStopLiveSearch(void)
{
	DemuxTableProcessLock();
	TableStopSection(&PATProcessData);
	TableStopSection(&LivePMTProcessData);
	SiLiveProgramServiceId = INVALID_ID;
	ClearProgramData(&SiLiveProgramData);
	DemuxTableProcessRelease();
}

/* Follow the PMT of serviceId on the current TP. */
void SiStartLiveSearch(U8 tunerId, U8 channelMode, U16 serviceId)
{
	SI_DEBUG("SiStartLiveSearch: service %u\n", serviceId);
	DemuxTableProcessLock();
	TableStopSection(&PATProcessData);
	TableStopSection(&LivePMTProcessData);
	ClearProgramData(&SiLiveProgramData);
	memset(PmtData, 0, sizeof(PmtData));
	PmtSize                = 0;
	SiLiveProgramServiceId = serviceId;
	TableStartPatSearch(tunerId, channelMode);
	DemuxTableProcessRelease();
}

void SiStopSearchChannel(void)
{
	DemuxTableProcessLock();
	TableStopAllSection();
	DemuxInitInfoBuffer(TRUE);
	NitSearchOn   = 0;
	SearchNotLive = 0;
	DemuxTableProcessRelease();
}

/* Scan the current TP: PAT, PMTs, SDT and NIT (nitMode: collect NIT transponders). */
void SiStartSearchChannel(U8 tunerId, U8 nitMode, U8 tpModeOn, U8 channelMode)
{
	SI_DEBUG("SiStartSearchChannel: nit %u tp %u mode %u\n", nitMode, tpModeOn, channelMode);
	DemuxTableProcessLock();
	TableStopAllSection();
	DemuxInitInfoBuffer(FALSE);
	ClearAllTableBuffer(&NitData_p);
	ClearAllTableBuffer(&CurrentSdtData_p);
	SearchNotLive = 1;
	TpSearchOn    = tpModeOn;
	PATProcessed  = 0;
	PMTProcessed  = 0;
	NitSearchOn   = nitMode;
	SDTReceived   = 0;
	ClearServiceData();
	ClearNetworkData();
	TableStartPatSearch(tunerId, channelMode);
	SDTProcessed        = 0;
	NITProcessed        = 0;
	CurrentSatNetWorkId = INVALID_ID;
	SDTReceived         = 0;
	TableStartSdtSearch(tunerId, 0xff, channelMode);
	TableStartNitSearch(tunerId, nitMode, channelMode);
	DemuxTableProcessRelease();
}

BOOL SiInitTable(void)
{
	NitSarchCallBack   = NULL;
	SearchCallBack     = NULL;
	LiveSearchCallBack = NULL;
	InitServiceData();
	InitNetworkData();
	ClearProgramData(&SiLiveProgramData);
	return DemuxDrvInit();
}

void SiTermTable(void)
{
	TableStopAllSection();
	ClearAllTableBuffer(&NitData_p);
	ClearAllTableBuffer(&CurrentSdtData_p);
	NitSarchCallBack   = NULL;
	SearchCallBack     = NULL;
	LiveSearchCallBack = NULL;
	DemuxDrvTerm();
}
