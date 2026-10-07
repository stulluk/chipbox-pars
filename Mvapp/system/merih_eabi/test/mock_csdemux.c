/*
 * mock_csdemux.c - CSDEMUX section filter replacement that replays a transport stream file.
 *
 * Only the calls used by si_tabledrv.c are provided. A feeder thread reads 188-byte TS
 * packets, reassembles PSI sections per PID and, like the Orion section filter, delivers a
 * section to every enabled filter on that PID whose 12-byte match/mask (section bytes 0..11,
 * the CSDEMUX layout) fits. Delivery: queue the section, then call the section notify.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "csapi.h"

#define MOCK_FILTERS      64
#define MOCK_QUEUE        64
#define MOCK_SECTION_MAX  4096
#define TS_PACKET         188

typedef struct
{
	int             Id;
	int             Open;
	int             Enabled;
	unsigned short  Pid;
	unsigned char   Match[12];
	unsigned char   Mask[12];
	void          (*Notify)(CSDEMUX_HANDLE, CSDEMUX_SECEVENT *);
	unsigned char  *Queue[MOCK_QUEUE];
	unsigned int    QueueLength[MOCK_QUEUE];
	unsigned int    Head;
	unsigned int    Count;
} MockFilter_t;

typedef struct
{
	unsigned char Data[MOCK_SECTION_MAX + TS_PACKET];
	unsigned int  Length;
	int           Active;
} MockAssembly_t;

static MockFilter_t    Filters[MOCK_FILTERS];
static MockFilter_t    PidFilters[MOCK_FILTERS];
static pthread_mutex_t Lock = PTHREAD_MUTEX_INITIALIZER;
static MockAssembly_t  Assembly[8192];
static unsigned long   SectionsDelivered;

/* Statistics for the test report. */
unsigned long MockSectionsDelivered(void)
{
	return SectionsDelivered;
}

CSDEMUX_HANDLE CSDEMUX_Filter_Open(CSDEMUX_FILTER_ID id)
{
	MockFilter_t *filter = &Filters[id];

	pthread_mutex_lock(&Lock);
	memset(filter, 0, sizeof(*filter));
	filter->Id   = id;
	filter->Open = 1;
	pthread_mutex_unlock(&Lock);
	return filter;
}

CSAPI_RESULT CSDEMUX_Filter_Close(CSDEMUX_HANDLE handle)
{
	MockFilter_t *filter = handle;

	pthread_mutex_lock(&Lock);
	while (filter->Count)
	{
		free(filter->Queue[filter->Head]);
		filter->Head = (filter->Head + 1) % MOCK_QUEUE;
		filter->Count--;
	}
	filter->Open    = 0;
	filter->Enabled = 0;
	pthread_mutex_unlock(&Lock);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_SetFilter(CSDEMUX_HANDLE handle, const unsigned char *const match,
                                      const unsigned char *const mask)
{
	MockFilter_t *filter = handle;

	memcpy(filter->Match, match, 12);
	memcpy(filter->Mask, mask, 12);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_SetFilterType(CSDEMUX_HANDLE handle, CSDEMUX_FILTER_TYPE type)
{
	(void)handle;
	return (type == DEMUX_FILTER_TYPE_SEC) ? CSAPI_SUCCEED : CSAPI_FAILED;
}

CSAPI_RESULT CSDEMUX_Filter_AddPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	((MockFilter_t *)handle)->Pid = pid;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_FILTER_SetSectionNotify(CSDEMUX_HANDLE handle,
                                             void (*notify)(CSDEMUX_HANDLE, CSDEMUX_SECEVENT *),
                                             CSDEMUX_SECEVENT event, int enable)
{
	(void)event;
	((MockFilter_t *)handle)->Notify = enable ? notify : NULL;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_Enable(CSDEMUX_HANDLE handle)
{
	((MockFilter_t *)handle)->Enabled = 1;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_Disable(CSDEMUX_HANDLE handle)
{
	((MockFilter_t *)handle)->Enabled = 0;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_CheckDataSize(CSDEMUX_HANDLE handle, unsigned int *size)
{
	MockFilter_t *filter = handle;

	pthread_mutex_lock(&Lock);
	*size = filter->Count ? filter->QueueLength[filter->Head] : 0;
	pthread_mutex_unlock(&Lock);
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_Filter_ReadSectionData(CSDEMUX_HANDLE handle, unsigned char *buf,
                                            unsigned int *size)
{
	MockFilter_t *filter = handle;
	unsigned int  length;

	pthread_mutex_lock(&Lock);
	if (filter->Count == 0)
	{
		pthread_mutex_unlock(&Lock);
		return CSAPI_FAILED;
	}
	length = filter->QueueLength[filter->Head];
	if (length > *size)
		length = *size;
	memcpy(buf, filter->Queue[filter->Head], length);
	free(filter->Queue[filter->Head]);
	filter->Head = (filter->Head + 1) % MOCK_QUEUE;
	filter->Count--;
	*size = length;
	pthread_mutex_unlock(&Lock);
	return CSAPI_SUCCEED;
}

CSDEMUX_HANDLE CSDEMUX_PIDFT_Open(CSDEMUX_PIDFT_ID id)
{
	PidFilters[id].Id = id;
	return &PidFilters[id];
}

CSAPI_RESULT CSDEMUX_PIDFT_Close(CSDEMUX_HANDLE handle)
{
	(void)handle;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetChannel(CSDEMUX_HANDLE handle, CSDEMUX_CHL_ID channel)
{
	(void)handle;
	(void)channel;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_SetPID(CSDEMUX_HANDLE handle, unsigned short pid)
{
	((MockFilter_t *)handle)->Pid = pid;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_Enable(CSDEMUX_HANDLE handle)
{
	((MockFilter_t *)handle)->Enabled = 1;
	return CSAPI_SUCCEED;
}

CSAPI_RESULT CSDEMUX_PIDFT_Disable(CSDEMUX_HANDLE handle)
{
	((MockFilter_t *)handle)->Enabled = 0;
	return CSAPI_SUCCEED;
}

/* Hardware-style match: byte i of the section against Match[i] under Mask[i]. */
static int MockMatch(const MockFilter_t *filter, const unsigned char *section, unsigned int length)
{
	unsigned int byte;

	for (byte = 0; (byte < 12) && (byte < length); byte++)
	{
		if ((section[byte] & filter->Mask[byte]) != (filter->Match[byte] & filter->Mask[byte]))
			return 0;
	}
	return 1;
}

/* Is any enabled section filter waiting for pid? */
static int MockPidWanted(unsigned short pid)
{
	int index;

	for (index = 0; index < MOCK_FILTERS; index++)
	{
		if (Filters[index].Open && Filters[index].Enabled && (Filters[index].Pid == pid))
			return 1;
	}
	return 0;
}

/* Hand a complete section to every matching filter. */
static void MockDeliver(unsigned short pid, const unsigned char *section, unsigned int length)
{
	CSDEMUX_SECEVENT event = DEMUX_SECTION_AVAIL;
	MockFilter_t    *filter;
	unsigned int     slot;
	int              index;

	for (index = 0; index < MOCK_FILTERS; index++)
	{
		filter = &Filters[index];
		pthread_mutex_lock(&Lock);
		if (!filter->Open || !filter->Enabled || (filter->Pid != pid) ||
		    !PidFilters[index].Enabled || !MockMatch(filter, section, length) ||
		    (filter->Count >= MOCK_QUEUE))
		{
			pthread_mutex_unlock(&Lock);
			continue;
		}
		slot = (filter->Head + filter->Count) % MOCK_QUEUE;
		filter->Queue[slot] = malloc(length);
		memcpy(filter->Queue[slot], section, length);
		filter->QueueLength[slot] = length;
		filter->Count++;
		SectionsDelivered++;
		pthread_mutex_unlock(&Lock);
		if (filter->Notify != NULL)
			filter->Notify(filter, &event);
	}
}

/* Append TS payload to the PID's section assembly and emit finished sections. */
static void MockAssemble(unsigned short pid, const unsigned char *payload, unsigned int size,
                         int unitStart)
{
	MockAssembly_t *assembly = &Assembly[pid];
	unsigned int    pointer;
	unsigned int    total;

	if (unitStart)
	{
		pointer = payload[0];
		if (assembly->Active && (pointer > 0) && (assembly->Length + pointer <= sizeof(assembly->Data)))
		{
			memcpy(&assembly->Data[assembly->Length], &payload[1], pointer);
			assembly->Length += pointer;
		}
		if (assembly->Active && (assembly->Length >= 3))
		{
			total = (((assembly->Data[1] & 0x0f) << 8) | assembly->Data[2]) + 3;
			if (assembly->Length >= total)
				MockDeliver(pid, assembly->Data, total);
		}
		payload += 1 + pointer;
		size    -= (pointer + 1 <= size) ? pointer + 1 : size;
		assembly->Length = 0;
		assembly->Active = 1;
	}
	if (!assembly->Active)
		return;

	while (size > 0)
	{
		if (assembly->Length + size > sizeof(assembly->Data))
		{
			assembly->Active = 0;
			return;
		}
		memcpy(&assembly->Data[assembly->Length], payload, size);
		assembly->Length += size;
		size = 0;
		/* Several sections may follow each other in one packet. */
		while ((assembly->Length >= 3) && (assembly->Data[0] != 0xff))
		{
			total = (((assembly->Data[1] & 0x0f) << 8) | assembly->Data[2]) + 3;
			if (assembly->Length < total)
				break;
			MockDeliver(pid, assembly->Data, total);
			memmove(assembly->Data, &assembly->Data[total], assembly->Length - total);
			assembly->Length -= total;
		}
		if ((assembly->Length > 0) && (assembly->Data[0] == 0xff))
		{
			assembly->Length = 0;     /* stuffing */
			assembly->Active = 0;
		}
	}
}

typedef struct
{
	const char   *Path;
	unsigned long Bytes;
	unsigned int  PacketsPerMs;
} MockFeedArg_t;

/* Feeder thread: replay the file; only PIDs with an enabled section filter are parsed. */
static void *MockFeeder(void *param)
{
	MockFeedArg_t *arg = param;
	unsigned char  packet[TS_PACKET];
	unsigned long  done = 0;
	unsigned int   counter = 0;
	unsigned short pid;
	unsigned int   offset;
	FILE          *file;

	file = fopen(arg->Path, "rb");
	if (file == NULL)
	{
		perror(arg->Path);
		return NULL;
	}
	while ((done < arg->Bytes) && (fread(packet, 1, TS_PACKET, file) == TS_PACKET))
	{
		done += TS_PACKET;
		if (packet[0] != 0x47)
			continue;
		pid = ((packet[1] & 0x1f) << 8) | packet[2];
		if (MockPidWanted(pid))
		{
			offset = 4;
			if (packet[3] & 0x20)
				offset += 1 + packet[4];
			if ((packet[3] & 0x10) && (offset < TS_PACKET))
				MockAssemble(pid, &packet[offset], TS_PACKET - offset, packet[1] & 0x40);
		}
		if (++counter >= arg->PacketsPerMs)
		{
			counter = 0;
			usleep(1000);
		}
	}
	fclose(file);
	return NULL;
}

/* Start replaying path (first bytes only) at packetsPerMs TS packets per millisecond. */
int MockCsdemuxStart(const char *path, unsigned long bytes, unsigned int packetsPerMs)
{
	static MockFeedArg_t arg;
	pthread_t            thread;

	arg.Path         = path;
	arg.Bytes        = bytes;
	arg.PacketsPerMs = packetsPerMs ? packetsPerMs : 1;
	return pthread_create(&thread, NULL, MockFeeder, &arg);
}
