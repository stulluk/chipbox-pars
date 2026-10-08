/*
 * si_int.h - internal definitions of the EABI re-implementation of the Merih SI library
 * (libsystem_merih.a: table.o and tabledrv.o), written from their disassembly (2026-10-07).
 *
 * tabledrv: section filters on top of CSDEMUX (32 slots, chained for multi-filter tables),
 *           a read task copying sections into a linear buffer, CRC / not-mask checks.
 * table:    PAT / PMT / SDT / NIT parsing for the channel scan and the live service PMT.
 */
#ifndef MERIH_SI_INT_H
#define MERIH_SI_INT_H

#include <stdio.h>

#include "mvosapi.h"
#include "csapi.h"
#include "table_def.h"

int tunerDebugEnabled(void);

/* Diagnostics on stdout when the environment has MERIH_TUNER_DEBUG=1. */
#define SI_DEBUG(...) \
	do { if (tunerDebugEnabled()) { printf("[si] " __VA_ARGS__); fflush(stdout); } } while (0)

#define DEMUX_INFO_NUMBER        TABLE_INFO_NUMBER   /* 32 filter slots */
#define DEMUX_INFO_NONE          DEMUX_INFO_NUMBER   /* "no slot" / end of a chain */
#define DEMUX_FILTER_BYTES       8
#define DEMUX_LINEAR_SIZE        0x32000             /* section buffer, 200 KiB */
#define DEMUX_SECTION_MAX        4096
#define DEMUX_SLOT_RESERVED      2                   /* slot used directly by mvapp */

/* DemuxStartTable() request. */
typedef struct
{
	U16             Pid;
	U16             ChannelId;        /* table id extension (service / TS id), 0xffff = any */
	U8              TunerId;
	U8              TableId;
	U8              NumberOfFilter;   /* chained slots, one 8-byte filter each */
	U8              ChannelMode;
	U8              CrcEnable;
	U8             *Match;
	U8             *Mask;
	U8             *NotMask;          /* bytes that must differ from Match (e.g. version) */
	DemuxCallback_f CallBack;
} DemuxStartParam_t;

/* Message from a section callback to the SI parse task. */
typedef struct
{
	U8  InfoId;
	U32 Length;
	U8 *Data;
} SiProcessMessage_t;

/* Results posted to the notify task. */
#define SI_NOTIFY_SEARCH         0
#define SI_NOTIFY_LIVE_PMT       1
#define SI_NOTIFY_NIT            2

/* tabledrv */
void *DemuxMemoryAllocate(U32 size);
BOOL  DemuxMemoryFree(void *memory);
U16   DemuxGetInfoLength(U8 *data, U8 tableId);
U16   DemuxGetPid(U8 *data);
U16   DemuxGetStreamId(U8 *data);
BOOL  DemuxSendTableNotifyMessage(U32 type);
BOOL  DemuxReceiveTableNotifyMessage(U32 *type);
BOOL  DemuxSendTableProcessMessage(SiProcessMessage_t message);
U8   *DemuxLinearBufferBase(void);
BOOL  DemuxReceiveTableProcessMessage(SiProcessMessage_t *message, U32 timeout);
void  DemuxTableProcessLock(void);
void  DemuxTableProcessRelease(void);
void  DemuxUpdateLinearPoint(U32 length, BOOL write);
void  DemuxInitInfoBuffer(BOOL reserveAll);
BOOL  DemuxStartTable(DemuxStartParam_t *param, U8 *infoId);
BOOL  DemuxStopTable(U8 infoId);
BOOL  DemuxPauseSection(U8 infoId);
BOOL  DemuxReStartSection(U8 infoId);
void  DemuxStopAllTable(void);
BOOL  DemuxCheckSameTable(U8 infoId, U8 otherId);
BOOL  DemuxGetTableData(U8 infoId, U8 *tableId, U8 *tunerId, U8 *channelMode, U16 *channelId);
BOOL  DemuxDrvInit(void);
void  DemuxDrvTerm(void);

/* table: task entries started by DemuxDrvInit() */
void  TableSiParseProcess(void *param);
void  SendResultProcess(void *param);

#endif /* MERIH_SI_INT_H */
