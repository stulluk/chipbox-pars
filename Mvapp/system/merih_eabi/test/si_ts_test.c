/*
 * si_ts_test.c - run the EABI SI port (si_table.c / si_tabledrv.c) against a recorded
 * transport stream through mock_csdemux.c and print what mvapp would receive.
 *
 * Usage: si_ts_test <file.ts> [megabytes] [packets_per_ms]
 *   1. channel scan with NIT (SiStartSearchChannel) -> services, PIDs, names, NIT TPs
 *   2. live search for the first TV service (SiStartLiveSearch) -> live PMT callback
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mvosapi.h"
#include "tableApi.h"

int           MockCsdemuxStart(const char *path, unsigned long bytes, unsigned int packetsPerMs);
unsigned long MockSectionsDelivered(void);
BOOL          CS_CRC_Init(void);

static volatile int SearchDone;
static volatile int LiveDone;
static volatile int NitDone;
static U16          FirstTvService = 0xffff;

static const char *EsTypeName(SiEsType type)
{
	static const char *names[] = { "video", "audio", "teletext", "subtitle", "unknown" };

	return (type < SI_ES_MAX) ? names[type] : "?";
}

/* Print one program as the scan / live callbacks deliver it. */
static void PrintProgram(const SiProgramData_t *program)
{
	U8 es;

	printf("  service %5u pmt 0x%04x pcr 0x%04x type %d scrambled %u name '%.30s' es %u\n",
	       program->ServideId, program->PmtPid, program->PcrPid, program->ServiceType,
	       program->ChannelScramble, program->ChannelName, program->NumberOfEs);
	for (es = 0; es < program->NumberOfEs; es++)
	{
		printf("      pid 0x%04x %-8s sub %u lang '%.3s'\n", program->EsData[es].EsPid,
		       EsTypeName(program->EsData[es].EsType), program->EsData[es].EsSubData.EsSubType,
		       program->EsData[es].EsLang);
	}
}

static void SearchResult(SiService_t *service)
{
	const SiProgramData_t *program;

	if (service == NULL)
	{
		printf("SEARCH: no PAT (empty result)\n");
		SearchDone = 1;
		return;
	}
	printf("SEARCH: ts %u onid %u programs %u name '%.30s'\n", service->TsId, service->ONId,
	       service->NumberOfProgram, service->ServiceName);
	for (program = service->ProgramData; program != NULL; program = program->Next_p)
	{
		PrintProgram(program);
		if ((FirstTvService == 0xffff) &&
		    ((program->ServiceType == TV_SERVICE) || (program->ServiceType == HDTV_SERVICE)) &&
		    !program->ChannelScramble)
			FirstTvService = program->ServideId;
	}
	SearchDone = 1;
}

static void NitResult(SiNetworkData_t *network)
{
	const SiNitTpData_t *tp;
	int                  count = 0;

	printf("NIT: network %u '%.30s' tps %u\n", network->NetworkId, network->NetworkName,
	       network->NumberOfTp);
	for (tp = network->NitTpData; (tp != NULL) && (count < 12); tp = tp->Next_p, count++)
	{
		printf("  ts %5u onid %u type %d freq %u sr %u pol %u orbit %u\n", tp->TsId, tp->OnId,
		       tp->TpType, tp->SatTpData.Frequency, tp->SatTpData.SymbolRate,
		       tp->SatTpData.Polarization, tp->SatTpData.Orbit);
	}
	NitDone = 1;
}

static void LiveResult(SiProgramData_t *program, U8 *pmt, U32 length)
{
	printf("LIVE: PMT %u bytes, table id 0x%02x\n", length, pmt[0]);
	PrintProgram(program);
	LiveDone = 1;
}

int main(int argc, char **argv)
{
	unsigned long megabytes = (argc > 2) ? strtoul(argv[2], NULL, 0) : 80;
	unsigned int  rate      = (argc > 3) ? strtoul(argv[3], NULL, 0) : 10;
	int           seconds;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <file.ts> [megabytes] [packets_per_ms]\n", argv[0]);
		return 2;
	}
	CS_CRC_Init();               /* mvapp does this in CS_DRV_Init() */
	if (SiInitTable())
	{
		fprintf(stderr, "SiInitTable failed\n");
		return 1;
	}
	SiRegisterSearchCallBack(SearchResult);
	SiRegisterNitCallBack(NitResult);
	SiRegisterLiveSearchCallBack(LiveResult);

	SiStartSearchChannel(0, 1, 1, 0);
	MockCsdemuxStart(argv[1], megabytes << 20, rate);
	for (seconds = 0; (seconds < 120) && !SearchDone; seconds++)
		sleep(1);
	printf("scan %s after %d s, %lu sections delivered, NIT %s\n",
	       SearchDone ? "finished" : "TIMEOUT", seconds, MockSectionsDelivered(),
	       NitDone ? "reported" : "not reported");
	SiStopSearchChannel();

	if (FirstTvService != 0xffff)
	{
		SiStartLiveSearch(0, 0, FirstTvService);
		MockCsdemuxStart(argv[1], megabytes << 20, rate);
		for (seconds = 0; (seconds < 60) && !LiveDone; seconds++)
			sleep(1);
		printf("live %s after %d s\n", LiveDone ? "PMT received" : "TIMEOUT", seconds);
		SiStopLiveSearch();
	}
	return (SearchDone && (FirstTvService == 0xffff || LiveDone)) ? 0 : 1;
}
