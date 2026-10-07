/*
 * si_tabledrv.c - section filter driver of the Merih SI library (port of tabledrv.o).
 *
 * 32 filter slots map 1:1 to CSDEMUX PID / section filter indexes. A table can use several
 * chained slots (one 8-byte match/mask per slot). Slots 0, 1 (and 8, 9, 15 outside a channel
 * scan) are reserved for mvapp's own demux users. Sections are read by DemuxReadTask into a
 * linear buffer, checked (not-mask, CRC) and handed to the table's callback.
 */
#include <stdio.h>
#include <string.h>

#include "si_int.h"
#include "mvutil.h"
#include "crc.h"

/* One filter slot. */
typedef struct
{
	CSDEMUX_HANDLE  PidFilter;
	CSDEMUX_HANDLE  SecFilter;
	U32             Reserved;                     /* DEMUX_SLOT_RESERVED: not for SI */
	DemuxCallback_f CallBack;
	U16             Pid;
	U16             ChannelId;
	U8              Mask[DEMUX_FILTER_BYTES];
	U8              Match[DEMUX_FILTER_BYTES];
	U8              NotMask[DEMUX_FILTER_BYTES];
	U8              TableId;
	U8              InUse;
	U8              Running;
	U8              CrcEnable;
	U8              ChannelMode;
	U8              Next;                         /* next chained slot, DEMUX_INFO_NONE = end */
} DemuxInfo_t;

/* Message from the CSDEMUX notify callback to the read task. */
typedef struct
{
	U8             InfoId;
	CSDEMUX_HANDLE Filter;
	U32            Spare;
} DemuxReadMessage_t;

static DemuxInfo_t DemuxInfo[DEMUX_INFO_NUMBER];
static U32         DemuxAccessSem;
static U32         SemTableProcess;
static U32         DemuxQueueId;
static U32         SiProcessMessageQueueId;
static U32         SiNotifyMessageQueueId;
static U32         TableNotifyTaskId;
static U32         TableSiTaskId;
static U32         DemuxTaskId;
static U8         *SectionLinearBuff;
static U32         LinearBufferSize;
static U32         LinearWritePointer;
static U32         LinearReadPointer;
static U8          SectionFlushBuffer[DEMUX_SECTION_MAX];

/* Memory helpers used by the table code. */
void *DemuxMemoryAllocate(U32 size)
{
	return OsMemoryAllocate((int)size);
}

BOOL DemuxMemoryFree(void *memory)
{
	return (OsMemoryFree(memory) != OS_NO_ERROR) ? TRUE : FALSE;
}

static void DemuxEnterCriticalSection(void)
{
	OsSemaphoreWait(DemuxAccessSem, TIMEOUT_FOREVER);
}

static void DemuxLeaveCriticalSection(void)
{
	OsSemaphoreSignal(DemuxAccessSem);
}

/* Lock shared by the parse task and the Si* API. */
void DemuxTableProcessLock(void)
{
	OsSemaphoreWait(SemTableProcess, TIMEOUT_FOREVER);
}

void DemuxTableProcessRelease(void)
{
	OsSemaphoreSignal(SemTableProcess);
}

static void ClearLinearPoint(void)
{
	LinearWritePointer = 0;
	LinearReadPointer  = 0;
	if (SectionLinearBuff != NULL)
		memset(SectionLinearBuff, 0, LinearBufferSize);
}

/* Advance the write (reader task) or read (consumer) position; wrap near the end. */
void DemuxUpdateLinearPoint(U32 length, BOOL write)
{
	U32 *pointer = write ? &LinearWritePointer : &LinearReadPointer;
	U32  value   = *pointer + length;

	if (value + DEMUX_SECTION_MAX > LinearBufferSize)
		value = 0;
	*pointer = value;
}

/* section_length (12 bits for EIT 0x4e..0x6f, 10 bits otherwise). */
U16 DemuxGetInfoLength(U8 *data, U8 tableId)
{
	U16 value = (U16)Array2Word(data, 2);

	if ((U8)(tableId - 0x4e) <= 33)
		return value & 0x0fff;
	return value & 0x03ff;
}

U16 DemuxGetPid(U8 *data)
{
	return (U16)(Array2Word(data, 2) & 0x1fff);
}

U16 DemuxGetStreamId(U8 *data)
{
	return (U16)Array2Word(data, 2);
}

BOOL DemuxSendTableNotifyMessage(U32 type)
{
	U32 *message = (U32 *)OsClaimMessage(SiNotifyMessageQueueId);

	if (message == NULL)
		return TRUE;
	*message = type;
	return (OsSendMessage(SiNotifyMessageQueueId, message) != OS_NO_ERROR) ? TRUE : FALSE;
}

BOOL DemuxReceiveTableNotifyMessage(U32 *type)
{
	U32 *message;

	if (type == NULL)
		return TRUE;
	message = (U32 *)OsReceiveMessage(SiNotifyMessageQueueId, TIMEOUT_FOREVER);
	if (message == NULL)
		return TRUE;
	*type = *message;
	OsReleaseMessage(SiNotifyMessageQueueId, message);
	return FALSE;
}

BOOL DemuxSendTableProcessMessage(SiProcessMessage_t message)
{
	SiProcessMessage_t *slot = (SiProcessMessage_t *)OsClaimMessage(SiProcessMessageQueueId);

	if (slot == NULL)
		return TRUE;
	*slot = message;
	return (OsSendMessage(SiProcessMessageQueueId, slot) != OS_NO_ERROR) ? TRUE : FALSE;
}

/* Receive a parse request; FALSE = got one, TRUE = timeout. */
BOOL DemuxReceiveTableProcessMessage(SiProcessMessage_t *message, U32 timeout)
{
	SiProcessMessage_t *slot;

	if (message == NULL)
		return TRUE;
	slot = (SiProcessMessage_t *)OsReceiveMessage(SiProcessMessageQueueId, timeout);
	if (slot == NULL)
		return TRUE;
	*message = *slot;
	OsReleaseMessage(SiProcessMessageQueueId, slot);
	return FALSE;
}

static void DemuxInfoBufferClear(U8 index)
{
	DemuxInfo_t *info = &DemuxInfo[index];

	memset(info, 0, sizeof(*info));
	info->Pid       = 0x1fff;
	info->ChannelId = 0xffff;
	info->TableId   = 0xff;
	info->Next      = DEMUX_INFO_NONE;
}

/* Reset all slots; reserveAll also keeps the slots mvapp uses outside a scan. */
void DemuxInitInfoBuffer(BOOL reserveAll)
{
	U8 index;

	for (index = 0; index < DEMUX_INFO_NUMBER; index++)
		DemuxInfoBufferClear(index);
	DemuxInfo[0].InUse    = 1;
	DemuxInfo[1].InUse    = 1;
	DemuxInfo[0].Reserved = DEMUX_SLOT_RESERVED;
	DemuxInfo[1].Reserved = DEMUX_SLOT_RESERVED;
	if (reserveAll)
	{
		DemuxInfo[8].Reserved  = DEMUX_SLOT_RESERVED;
		DemuxInfo[9].Reserved  = DEMUX_SLOT_RESERVED;
		DemuxInfo[15].InUse    = 1;
		DemuxInfo[15].Reserved = DEMUX_SLOT_RESERVED;
	}
}

/* CSDEMUX notification: queue the slot for the read task. */
static void SectionReceived(CSDEMUX_HANDLE filter, CSDEMUX_SECEVENT *event)
{
	DemuxReadMessage_t *message;
	U8                  index;
	U8                  found = DEMUX_INFO_NONE;

	DemuxEnterCriticalSection();
	for (index = 0; index < DEMUX_INFO_NUMBER; index++)
	{
		if (DemuxInfo[index].SecFilter == filter)
		{
			if (DemuxInfo[index].Running)
				found = index;
			break;
		}
	}
	DemuxLeaveCriticalSection();

	{
		static U32 traced[DEMUX_INFO_NUMBER + 1];

		if (traced[found] < 3)
		{
			traced[found]++;
			SI_DEBUG("notify filter %p slot %u event %d\n", filter, found, (int)*event);
		}
	}
	if ((found >= DEMUX_INFO_NUMBER) || (*event != DEMUX_SECTION_AVAIL) ||
	    !DemuxInfo[found].Running)
		return;
	message = (DemuxReadMessage_t *)OsClaimMessage(DemuxQueueId);
	if (message == NULL)
		return;
	message->InfoId = found;
	message->Filter = filter;
	OsSendMessage(DemuxQueueId, message);
}

/* Pause a table (all chained slots). FALSE = ok. */
BOOL DemuxPauseSection(U8 infoId)
{
	DemuxInfo_t *info;
	U8           index = infoId;

	if (index >= DEMUX_INFO_NUMBER)
		return TRUE;
	DemuxEnterCriticalSection();
	do
	{
		info = &DemuxInfo[index];
		if (!info->InUse || !info->Running || info->Reserved)
		{
			DemuxLeaveCriticalSection();
			return TRUE;
		}
		info->Running = 0;
		CSDEMUX_Filter_Disable(info->SecFilter);
		CSDEMUX_PIDFT_Disable(info->PidFilter);
		index = info->Next;
	} while (index < DEMUX_INFO_NUMBER);
	DemuxLeaveCriticalSection();
	return FALSE;
}

/* Restart a paused table. FALSE = ok. */
BOOL DemuxReStartSection(U8 infoId)
{
	DemuxInfo_t *info;
	U8           index = infoId;

	if (index >= DEMUX_INFO_NUMBER)
		return TRUE;
	DemuxEnterCriticalSection();
	do
	{
		info = &DemuxInfo[index];
		if (!info->InUse || info->Reserved)
		{
			DemuxLeaveCriticalSection();
			return TRUE;
		}
		if (info->Running != 1)
		{
			CSDEMUX_PIDFT_Enable(info->PidFilter);
			CSDEMUX_Filter_Enable(info->SecFilter);
		}
		info->Running = 1;
		index = info->Next;
	} while (index < DEMUX_INFO_NUMBER);
	DemuxLeaveCriticalSection();
	return FALSE;
}

/* Is otherId part of the chain starting at infoId? */
BOOL DemuxCheckSameTable(U8 infoId, U8 otherId)
{
	U8 index = infoId;

	if ((infoId >= DEMUX_INFO_NUMBER) || (otherId >= DEMUX_INFO_NUMBER))
		return FALSE;
	DemuxEnterCriticalSection();
	do
	{
		if (index == otherId)
		{
			DemuxLeaveCriticalSection();
			return TRUE;
		}
		index = DemuxInfo[index].Next;
	} while (index < DEMUX_INFO_NUMBER);
	DemuxLeaveCriticalSection();
	return FALSE;
}

/* Table id, channel mode and table id extension of a running table. FALSE = ok. */
BOOL DemuxGetTableData(U8 infoId, U8 *tableId, U8 *tunerId, U8 *channelMode, U16 *channelId)
{
	DemuxInfo_t *info;

	if (infoId >= DEMUX_INFO_NUMBER)
		return TRUE;
	info = &DemuxInfo[infoId];
	if (!info->Running)
	{
		*tableId     = 0xff;
		*tunerId     = 0xff;
		*channelMode = 0xff;
		*channelId   = 0xffff;
		return TRUE;
	}
	*tableId     = info->TableId;
	*tunerId     = 0;
	*channelMode = info->ChannelMode;
	*channelId   = info->ChannelId;
	return FALSE;
}

/* Stop a table and free all chained slots. FALSE = ok. */
BOOL DemuxStopTable(U8 infoId)
{
	DemuxInfo_t *info;
	U8           index = infoId;
	U8           next;

	if (index >= DEMUX_INFO_NUMBER)
		return TRUE;
	DemuxPauseSection(index);
	DemuxEnterCriticalSection();
	do
	{
		info = &DemuxInfo[index];
		if (!info->InUse || info->Reserved)
		{
			DemuxLeaveCriticalSection();
			return TRUE;
		}
		next = info->Next;
		CSDEMUX_Filter_Close(info->SecFilter);
		CSDEMUX_PIDFT_Close(info->PidFilter);
		DemuxInfoBufferClear(index);
		index = next;
	} while (index < DEMUX_INFO_NUMBER);
	DemuxLeaveCriticalSection();
	return FALSE;
}

/* Stop every SI table and reset the section buffer. */
void DemuxStopAllTable(void)
{
	U8 index;

	for (index = 0; index < DEMUX_INFO_NUMBER; index++)
	{
		if ((DemuxInfo[index].InUse == 1) && (DemuxInfo[index].Reserved == 0))
			DemuxStopTable(index);
	}
	DemuxEnterCriticalSection();
	ClearLinearPoint();
	DemuxLeaveCriticalSection();
}

/*
 * Start a table on param->NumberOfFilter chained free slots. The 8 filter bytes cover the
 * section header bytes 0 and 3..9 (the length bytes 1-2 are skipped in the hardware
 * filter). *infoId gets the first slot (DEMUX_INFO_NONE on error). FALSE = ok.
 */
BOOL DemuxStartTable(DemuxStartParam_t *param, U8 *infoId)
{
	DemuxInfo_t   *info;
	CSDEMUX_HANDLE secFilter;
	CSDEMUX_HANDLE pidFilter;
	U8             hwMatch[12];
	U8             hwMask[12];
	U8            *match   = param->Match;
	U8            *mask    = param->Mask;
	U8            *notMask = param->NotMask;
	U8            *link    = infoId;
	U8             count;
	U8             slot;
	U8             byte;
	U8             position;

	*infoId = DEMUX_INFO_NONE;
	if (param->NumberOfFilter == 0)
		return TRUE;

	DemuxEnterCriticalSection();
	for (count = 0; count < param->NumberOfFilter; count++)
	{
		for (slot = 0; slot < DEMUX_INFO_NUMBER; slot++)
		{
			if ((DemuxInfo[slot].InUse == 0) && (DemuxInfo[slot].Reserved == 0))
				break;
		}
		if (slot >= DEMUX_INFO_NUMBER)
		{
			/* Out of slots: undo the part of the chain already started. */
			slot = *infoId;
			DemuxLeaveCriticalSection();
			if (slot < DEMUX_INFO_NUMBER)
				DemuxStopTable(slot);
			*infoId = DEMUX_INFO_NONE;
			return TRUE;
		}

		info = &DemuxInfo[slot];
		DemuxInfoBufferClear(slot);
		if (match != NULL)
			memcpy(info->Match, match, DEMUX_FILTER_BYTES);
		if (mask != NULL)
			memcpy(info->Mask, mask, DEMUX_FILTER_BYTES);
		if (notMask != NULL)
			memcpy(info->NotMask, notMask, DEMUX_FILTER_BYTES);

		memset(hwMatch, 0, sizeof(hwMatch));
		memset(hwMask, 0, sizeof(hwMask));
		position = 0;
		for (byte = 0; byte < DEMUX_FILTER_BYTES; byte++)
		{
			/* "Not equal" bytes are checked in software, not in the hardware filter. */
			if (info->NotMask[byte])
				info->Mask[byte] &= ~info->NotMask[byte];
			if (byte == 1)
				position = 3;
			hwMask[position]  = info->Mask[byte];
			hwMatch[position] = info->Match[byte];
			position++;
		}

		secFilter = CSDEMUX_Filter_Open((CSDEMUX_FILTER_ID)slot);
		pidFilter = CSDEMUX_PIDFT_Open((CSDEMUX_PIDFT_ID)slot);
		CSDEMUX_PIDFT_SetChannel(pidFilter, DEMUX_CHL_ID0);
		CSDEMUX_PIDFT_SetPID(pidFilter, param->Pid);
		CSDEMUX_Filter_SetFilter(secFilter, hwMatch, hwMask);
		CSDEMUX_Filter_SetFilterType(secFilter, DEMUX_FILTER_TYPE_SEC);
		CSDEMUX_Filter_AddPID(secFilter, param->Pid);
		CSDEMUX_FILTER_SetSectionNotify(secFilter, SectionReceived, DEMUX_SECTION_AVAIL, 1);
		CSDEMUX_PIDFT_Enable(pidFilter);
		CSDEMUX_Filter_Enable(secFilter);

		info->Pid         = param->Pid;
		info->PidFilter   = pidFilter;
		info->SecFilter   = secFilter;
		info->TableId     = param->TableId;
		info->ChannelMode = param->ChannelMode;
		info->CallBack    = param->CallBack;
		info->ChannelId   = param->ChannelId;
		info->CrcEnable   = param->CrcEnable;
		info->Running     = 1;
		info->InUse       = 1;
		SI_DEBUG("start slot %u pid 0x%x tid 0x%02x sec %p pid %p\n", slot, param->Pid,
		         param->TableId, secFilter, pidFilter);

		*link = slot;
		link  = &info->Next;
		if (match != NULL)
			match += DEMUX_FILTER_BYTES;
		if (mask != NULL)
			mask += DEMUX_FILTER_BYTES;
		if (notMask != NULL)
			notMask += DEMUX_FILTER_BYTES;
	}
	DemuxLeaveCriticalSection();
	return FALSE;
}

/* Drain a filter whose slot is no longer running. */
static void SectionFlushFilterData(CSDEMUX_HANDLE filter)
{
	unsigned int size = 0;

	if (filter == NULL)
		return;
	if ((CSDEMUX_Filter_CheckDataSize(filter, &size) != CSAPI_SUCCEED) || (size == 0))
		return;
	size = sizeof(SectionFlushBuffer);
	CSDEMUX_Filter_ReadSectionData(filter, SectionFlushBuffer, &size);
}

/* DemuxReadTask: read notified sections, filter them and call the table callback. */
static void SectionReceiveTask(void *param)
{
	DemuxReadMessage_t *message;
	DemuxInfo_t        *info;
	CSDEMUX_HANDLE      filter;
	unsigned int        available;
	unsigned int        size;
	U8                 *section;
	U32                 length;
	U32                 crc;
	U8                  index;
	U8                  byte;
	U8                  position;
	BOOL                reject;

	(void)param;
	for (;;)
	{
		message = (DemuxReadMessage_t *)OsReceiveMessage(DemuxQueueId, TIMEOUT_FOREVER);
		if (message == NULL)
			continue;
		index  = message->InfoId;
		filter = message->Filter;
		OsReleaseMessage(DemuxQueueId, message);

		DemuxEnterCriticalSection();
		if ((index >= DEMUX_INFO_NUMBER) || !DemuxInfo[index].Running)
		{
			SectionFlushFilterData(filter);
			DemuxLeaveCriticalSection();
			continue;
		}
		info = &DemuxInfo[index];

		size = DEMUX_SECTION_MAX;
		available = 0;
		{
			static U32 traced[DEMUX_INFO_NUMBER];
			CSAPI_RESULT check = CSDEMUX_Filter_CheckDataSize(filter, &available);

			if (traced[index] < 3)
			{
				traced[index]++;
				SI_DEBUG("read slot %u pid 0x%x check %d available %u\n", index, info->Pid,
				         (int)check, available);
			}
		}
		if (available == 0)
		{
			DemuxLeaveCriticalSection();
			continue;
		}
		section = SectionLinearBuff + LinearWritePointer;
		if (CSDEMUX_Filter_ReadSectionData(filter, section, &size) != CSAPI_SUCCEED)
		{
			SI_DEBUG("read slot %u failed (size %u)\n", index, size);
			DemuxLeaveCriticalSection();
			continue;
		}
		length = DemuxGetInfoLength(&section[1], section[0]) + 3;
		{
			static U32 traced[DEMUX_INFO_NUMBER];

			if (traced[index] < 3)
			{
				traced[index]++;
				SI_DEBUG("section slot %u tid 0x%02x len %u (read %u)\n", index, section[0],
				         (unsigned)length, size);
			}
		}
		if (length <= 7)
		{
			DemuxLeaveCriticalSection();
			continue;
		}

		/* Not-mask bytes must differ from the match value (e.g. a new version). */
		reject   = FALSE;
		position = 0;
		for (byte = 0; byte < DEMUX_FILTER_BYTES; byte++)
		{
			if (byte == 1)
				position = 3;
			if (info->NotMask[byte] &&
			    ((info->Match[byte] & info->NotMask[byte]) ==
			     (section[position] & info->NotMask[byte])))
			{
				reject = TRUE;
				break;
			}
			position++;
		}
		if (reject)
		{
			DemuxLeaveCriticalSection();
			continue;
		}

		if (info->CrcEnable)
		{
			crc = 0;
			CS_CRC_32bCalculate(section, length - 4, &crc);
			if (crc != Array2Word(&section[length - 4], 4))
			{
				DemuxLeaveCriticalSection();
				continue;
			}
		}

		if (info->CallBack != NULL)
		{
			DemuxCallback_f callBack = info->CallBack;

			DemuxLeaveCriticalSection();
			callBack(index, section, length);
			DemuxEnterCriticalSection();
		}
		DemuxUpdateLinearPoint(length, TRUE);
		DemuxLeaveCriticalSection();
	}
}

/* Allocate the section buffer, queues, semaphores and the three SI tasks. FALSE = ok. */
BOOL DemuxDrvInit(void)
{
	DemuxInitInfoBuffer(TRUE);
	LinearBufferSize  = DEMUX_LINEAR_SIZE;
	SectionLinearBuff = (U8 *)OsMemoryAllocate((int)LinearBufferSize);
	if (SectionLinearBuff == NULL)
	{
		LinearBufferSize = 0;
		return TRUE;
	}
	ClearLinearPoint();

	if ((OsCreateMessageQueue(&DemuxQueueId, "DemuxQueue", sizeof(DemuxReadMessage_t), 50) !=
	     OS_NO_ERROR) || (DemuxQueueId == 0))
		return TRUE;
	if ((OsCreateMessageQueue(&SiProcessMessageQueueId, "SiProcessQueue",
	                          sizeof(SiProcessMessage_t), 50) != OS_NO_ERROR) ||
	    (SiProcessMessageQueueId == 0))
		return TRUE;
	if ((OsCreateMessageQueue(&SiNotifyMessageQueueId, "SiNotifyQueue", sizeof(U32), 50) !=
	     OS_NO_ERROR) || (SiNotifyMessageQueueId == 0))
		return TRUE;
	if ((OsCreateSemaphore(&DemuxAccessSem, 1) != OS_NO_ERROR) || (DemuxAccessSem == 0))
		return TRUE;
	if ((OsCreateSemaphore(&SemTableProcess, 1) != OS_NO_ERROR) || (SemTableProcess == 0))
		return TRUE;
	if ((OsCreateTask(&TableNotifyTaskId, "TableSiNotifyTask", SendResultProcess, NULL, 10,
	                  2048) != OS_NO_ERROR) || (TableNotifyTaskId == 0))
		return TRUE;
	if ((OsCreateTask(&TableSiTaskId, "TableSiProcessTask", TableSiParseProcess, NULL, 10,
	                  2048) != OS_NO_ERROR) || (TableSiTaskId == 0))
		return TRUE;
	if ((OsCreateTask(&DemuxTaskId, "DemuxReadTask", SectionReceiveTask, NULL, 10, 2048) !=
	     OS_NO_ERROR) || (DemuxTaskId == 0))
		return TRUE;
	return FALSE;
}

/* Stop everything and free the driver resources. */
void DemuxDrvTerm(void)
{
	if (DemuxAccessSem)
		DemuxStopAllTable();
	if (DemuxTaskId)
		OsDeleteTask(DemuxTaskId);
	if (TableSiTaskId)
		OsDeleteTask(TableSiTaskId);
	if (TableNotifyTaskId)
		OsDeleteTask(TableNotifyTaskId);
	if (SiNotifyMessageQueueId)
		OsDeleteMessageQueue(SiNotifyMessageQueueId);
	if (SiProcessMessageQueueId)
		OsDeleteMessageQueue(SiProcessMessageQueueId);
	if (DemuxQueueId)
		OsDeleteMessageQueue(DemuxQueueId);
	if (DemuxAccessSem)
		OsDeleteSemaphore(DemuxAccessSem);
	if (SemTableProcess)
		OsDeleteSemaphore(SemTableProcess);
	if (SectionLinearBuff)
		OsMemoryFree(SectionLinearBuff);
	DemuxTaskId = TableSiTaskId = TableNotifyTaskId = 0;
	SiNotifyMessageQueueId = SiProcessMessageQueueId = DemuxQueueId = 0;
	DemuxAccessSem = SemTableProcess = 0;
	SectionLinearBuff = NULL;
}
