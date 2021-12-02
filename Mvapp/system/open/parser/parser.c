#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "linuxos.h"
#include "linuxdefs.h"
#include "parser.h"
#include "ts_parser.h"
#include "demux.h"
#include "db_builder.h"
#include "database.h"
#include "av_zapping.h"
#include "eit_engine.h"

#define Parser_printf(fmt, args...)   //printf("Info:*** File: %s : Function: %s : " fmt, __FILE__,  __FUNCTION__, ##args) //printf

#define PAT_SECTION_NUM_MAX 5
#define PMT_SECTION_NUM_MAX 5
#define SDT_SECTION_NUM_MAX 10
#define NIT_SECTION_NUM_MAX 10
#define EIT_SECTION_NUM_MAX 100


const char* stream_type_info[] =
{
    "ITU-T | ISO/IEC Reserved",
    "ISO/IEC 11172 Video",
    "ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream",
    "ISO/IEC 11172 Audio",
    "ISO/IEC 13818-3 Audio",
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections",
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data",
    "ISO/IEC 13522 MHEG",
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A DSM-CC",
    "ITU-T Rec. H.222.1",
    "ISO/IEC 13818-6 type A",
    "ISO/IEC 13818-6 type",
    "ISO/IEC 13818-6 type",
    "ISO/IEC 13818-6 type D",
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary",
    "ISO/IEC 13818-7 Audio with ADTS transport syntax",
    "ISO/IEC 14496-2 Visual",
    "ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3 / AMD 1",
    "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets",
    "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC14496_sections.",
    "ISO/IEC 13818-6 Synchronized Download Protocol",
    "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved",
    "User Private"
};

U8   mpBuff[MAX_SECTION_SIZE];
U32  mPID;
U8   mTABID;
U32  mPos;
U8   mNextCounter;

U8 saved_section_num;
U8 save_section[MAX_SECTION_NUM];

ProgramInfoInPAT pmt_pid[MAX_PROGRAM_NUM];
U32 total_program_num = 0;
U32	current_program_num = 0;

stSDT_INFO sdt_info[MAX_PROGRAM_NUM];
U32 total_sdt_info_num = 0;
U32 current_sdt_info_num = 0;
stDT_Date g_local_date;
stDT_Time g_local_time;


typedef struct
{
    U32	toFilter;
    U32	filtered;
}tCS_Parser_SectionNumberMask;	/*暂时考虑最大SECTION数是32*/

tCS_SI_PMT_Info g_Current_PMT;
tCS_Parser_SectionNumberMask g_SectionNumberMask_PMT;
CSOS_Semaphore_t *g_sem_PMT_Access = NULL;
CSOS_Semaphore_t *g_Parser_Access = NULL;

typedef struct _Section_Data
{
    U16 section_length;
    U8* section_data;
}stSection_Data;

typedef struct _Save_Section
{
    U16 pid;
    U16 table_id;
    U8 section_count;
    stSection_Data section_data;
}stSave_Section;

stSave_Section g_save_section[1000];
U16 g_saved_section_count = 0;

/*^^***********************************************************************
 *    This two functions are used to get section data
 ***********************************************************************^^*/
void  section_parser_init(U32 pid, U8 tabid)
{
    mPID    =  pid;
    mTABID  =  tabid;
    mPos    =  0;

    return;
}

U32 section_parser(const U8 *_bf,const unsigned long _size)
{
    U32 pid;
    int len = TS_PACKET_SIZE;

    if ( mPos >= 3 && (mPos-3) >= (U32)(((mpBuff[1]&0xf)<<8) | mpBuff[2]) )		// section_length
    {
        mPos=(((mpBuff[1]&0xf)<<8)|mpBuff[2]);
        mPos+=3;

        return 1;
    }

    pid = ((U32)(_bf[1]&0x1f) << 8) | (U32)_bf[2];
    if ( pid != mPID || _size != TS_PACKET_SIZE || 0x47 != _bf[0] )
        return 0;

    if ((_bf[1]&0xc0)==0x40) // transport_error_indicator is 0 and payload_unit_start_indicator is 1
        // payload_unit_start_indicator is 1 means that current packet include the first byte of PSI
    {
        mPos=0;
        mNextCounter=(_bf[3]&0xf);	// continuity_counter
    }
    else if (mPos==0 || mNextCounter!=(_bf[3]&0xf) || (_bf[1]&0x80)) // not the first packet of PSI section
    {
        mPos=0;
        return 0;
    }

    mNextCounter++;
    mNextCounter&=0xf;
    len-=4;

    if (_bf[3]&0x20) // adaptation_field_control is 0x10 means only adaptation no payload in the packet
    {
        if (_bf[4]>=183)
            return 0;
        len-=_bf[4]+1;
    }

    _bf+=TS_PACKET_SIZE-len;

    if (mPos==0)
    {
        len-=_bf[0]+1;
        _bf+=_bf[0]+1;
        if (len<=0||_bf[0]!=mTABID)
            return 0;
    }

    if (mPos+len>MAX_SECTION_SIZE)
    {
        mPos=0;
        return 0;
    }

    memcpy(mpBuff+mPos,_bf,len);
    mPos+=len;

    return 0;
}

/*^^***********************************************************************
 *    This two functions are used to save sections parserd
 ***********************************************************************^^*/
void section_num_save_init(void)
{
    int i;

    for (i=0; i<MAX_SECTION_NUM; i++)
    {
        save_section[i] = 0xff;
    }
    saved_section_num = 0;
}

int check_and_save_section(unsigned char section_num, unsigned char last_section_num)
{
    int i;

    if (mTABID >= EIT_SCHEDUAL_TABLE_ID_ACTUAL_FIRST && mTABID <= EIT_SCHEDUAL_TABLE_ID_OTHER_LAST) // EIT schedual section num plus 8 every time
    {
        if (last_section_num/8+1 == saved_section_num)
            return 2;
    }
    else
    {
        if (last_section_num+1 == saved_section_num)
            return 2;	// all section saved
    }

    for (i=0; i<saved_section_num; i++)
    {
        if (save_section[i] == section_num)
            return 1;	// saved, but not all
    }

    // save section data
    g_save_section[g_saved_section_count].pid = mPID;
    g_save_section[g_saved_section_count].table_id = mTABID;

    if (mTABID >= EIT_SCHEDUAL_TABLE_ID_ACTUAL_FIRST && mTABID <= EIT_SCHEDUAL_TABLE_ID_OTHER_LAST) // EIT schedual section num plus 8 every time
    {
        g_save_section[g_saved_section_count].section_count = last_section_num/8+1;
    }
    else if (mTABID == TDT_TABLE_ID || mTABID == TOT_TABLE_ID)  // tdt & tot have only one section
    {
        g_save_section[g_saved_section_count].section_count = 1;
    }
    else
    {
        g_save_section[g_saved_section_count].section_count = last_section_num + 1;
    }
    g_save_section[g_saved_section_count].section_data.section_length = mPos;
    g_save_section[g_saved_section_count].section_data.section_data = (U8 *)CSOS_AllocateMemory(NULL, sizeof(U8)*mPos);
    memcpy(g_save_section[g_saved_section_count].section_data.section_data, mpBuff, mPos);
    g_saved_section_count++;

    return 0;	// have not saved
}


void ParserInit(void)
{
    g_sem_PMT_Access = CSOS_CreateSemaphoreFifo(NULL, 1);
    g_Parser_Access = CSOS_CreateSemaphoreFifo(NULL, 1);
}

/*^^**********************************************************************
 *    Section Parse Funchtions
 ***********************************************************************^^*/
void ParseNITSection(stTS_NIT *NIT_section, const U8 *buffer)
{
    int len = 0;
    int i =0;
    int pos = 12;

    NIT_section->table_id = buffer[0];
    NIT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    NIT_section->reserved_future_use_1 = buffer[1] >> 6 & 0x01;
    NIT_section->reserved_1 = buffer[1] >> 4 & 0x03;
    NIT_section->section_length = ((U16)buffer[1] & 0x0f) | buffer[2];
    NIT_section->network_id = (U16)buffer[3] << 8 | buffer[4];
    NIT_section->reserved_2 = buffer[5] >> 6 & 0x03;
    NIT_section->version_number = buffer[5] >> 1 & 0x1f;
    NIT_section->current_next_indicator = buffer[5] & 0x01;
    NIT_section->section_number = buffer[6];
    NIT_section->last_section_number = buffer[7];
    NIT_section->reserved_future_use_2 = buffer[8] >> 4 & 0x0f;
    NIT_section->network_descriptors_length = ((U16)buffer[8] & 0x0f) | buffer[9];
    NIT_section->reserved_future_use_3 = buffer[NIT_section->network_descriptors_length + 10] >> 4 & 0x0f;
    NIT_section->transport_stream_loop_length = ((U16)buffer[NIT_section->network_descriptors_length + 10] & 0x0f) |
            buffer[NIT_section->network_descriptors_length + 11];

    len = NIT_section->section_length + 3;
    NIT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16
                          | (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    if (NIT_section->table_id == NIT_TABLE_ID_ACTUAL)
        {
            Parser_printf("NIT ACTUAL:\n");
        }
    else
        {
            Parser_printf("NIT OTHER:\n");
        }

    Parser_printf("  table_id: 0x%02x\n", NIT_section->table_id);
    Parser_printf("  section_syntax_indicator: 0x%02x\n", NIT_section->section_syntax_indicator);
    Parser_printf("  section_length: 0x%02x(%d)\n", NIT_section->section_length, NIT_section->section_length);
    Parser_printf("  network_id: 0x%04x\n", NIT_section->network_id);
    Parser_printf("  version_number: 0x%02x\n", NIT_section->version_number);
    Parser_printf("  current_next_indicator: 0x%x\n", NIT_section->current_next_indicator);
    Parser_printf("  section_number: 0x%02x(%d)\n", NIT_section->section_number, NIT_section->section_number);
    Parser_printf("  last_section_number: 0x%02x(%d)\n", NIT_section->last_section_number, NIT_section->last_section_number);
    Parser_printf("  network_descriptors_length: 0x%02x\n", NIT_section->network_descriptors_length);
    Parser_printf("  transport_stream_loop_length: 0x%02x\n", NIT_section->transport_stream_loop_length);
    Parser_printf("  transport_streams:\n");

    stTRANSPORT_STREAM_INFO ts_info;
    for (pos += NIT_section->network_descriptors_length; pos < NIT_section->section_length + 3 - 4; )
    {
        ts_info.transport_stream_id = (U16)buffer[pos] << 8 | buffer[pos + 1];
        ts_info.original_network_id = (U16)buffer[pos + 2] << 8 | buffer[pos + 3];
        ts_info.reserved_future_use = buffer[pos + 4] >> 4 & 0x0f;
        ts_info.transport_descriptors_length = ((U16)buffer[pos + 4] & 0x0f) | buffer[pos + 5];

        Parser_printf("    transport_stream_id: 0x%04x\n", ts_info.transport_stream_id);
        Parser_printf("      original_network_id: 0x%04x\n", ts_info.original_network_id);
        Parser_printf("      transport_descriptors_length: 0x%02x\n", ts_info.transport_descriptors_length);

        pos = pos + 6 + ts_info.transport_descriptors_length;
        i++;
    }

    Parser_printf("  CRC_32: 0x%08x\n", NIT_section->CRC_32);
    Parser_printf("\n");

    return ;
}

void ParsePATSection (stTS_PAT *PAT_section, const U8 *buffer)
{
    U32 i = 0;
    int len = 0;
    U32	program_num;

    PAT_section->table_id = buffer[0];
    PAT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    //PAT_section->zero = buffer[1] >> 6 & 0x01;
    //PAT_section->reserved_1 = buffer[1] >> 4 & 0x3;
    PAT_section->section_length = (buffer[1] & 0x0F) << 8 | buffer[2];
    PAT_section->transport_stream_id = buffer[3] << 8 | buffer[4];
    //PAT_section->reserved_2 = buffer[5] >> 6;
    PAT_section->version_number = buffer[5] >> 1 &  0x1F;
    PAT_section->current_next_indicator = (buffer[5] << 7) >> 7;
    PAT_section->section_number = buffer[6];
    PAT_section->last_section_number = buffer[7];
    // Get CRC_32
    len = 3 + PAT_section->section_length;
    PAT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16
                          | (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    // Get PMT PID in PAT
    program_num = (PAT_section->section_length - 9) / 4;

    total_program_num = 0;
    for (i=total_program_num; i<total_program_num + program_num; i++)
    {
        pmt_pid[i].program_number = buffer[8+i*4] << 8 | buffer[9+i*4];
        //pmt_pid[i].reserved = (buffer[10+i*4] >> 5) & 0x1f;
        pmt_pid[i].PMT_PID = (U16)((buffer[10+i*4] & 0x07) << 8) | buffer[11+i*4];
    }

    total_program_num += program_num;
    if (total_program_num > MAX_PROGRAM_NUM)
        total_program_num = MAX_PROGRAM_NUM;

    Parser_printf("PAT:\n");
    Parser_printf("  table_id : 0x%02x\n", PAT_section->table_id);
    Parser_printf("  section_syntax_indicator : 0x%02x\n", PAT_section->section_syntax_indicator);
    Parser_printf("  section_length : 0x%02x(%d)\n", PAT_section->section_length, PAT_section->section_length);
    Parser_printf("  transport_stream_id : 0x%04x\n", PAT_section->transport_stream_id);
    Parser_printf("  version_number : 0x%02x\n", PAT_section->version_number);
    Parser_printf("  current_next_indicator : 0x%02x\n", PAT_section->current_next_indicator);
    Parser_printf("  section_number : 0x%02x(%d)\n", PAT_section->section_number, PAT_section->section_number);
    Parser_printf("  last_section_number : 0x%02x(%d)\n", PAT_section->last_section_number, PAT_section->last_section_number);
    Parser_printf("  programs:\n");
    for (i=0; i<program_num; i++)
    {
        Parser_printf("      program number: 0x%04x(%d) => pid: 0x%04x(%d)\n", pmt_pid[i].program_number,
                      pmt_pid[i].program_number,
                      pmt_pid[i].PMT_PID,
                      pmt_pid[i].PMT_PID);
    }
    Parser_printf("  CRC_32 :0x%2x\n", PAT_section->CRC_32);
    Parser_printf("\n");

    return ;
}

// Adjust PMT table
void ParsePMTSection (stTS_PMT *PMT_section, const U8 *buffer)
{
    U32 pos = 12, len = 0;
    int i = 0;

    PMT_section->table_id = buffer[0];
    PMT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    //PMT_section->zero = buffer[1] >> 6 & 0x01;
    //PMT_section->reserved_1 = buffer[1] >> 4;
    PMT_section->section_length = (buffer[1] & 0x0F) << 8 | buffer[2];
    PMT_section->program_number = buffer[3] << 8 | buffer[4];
    //PMT_section->reserved_2 = buffer[5] >> 6;
    PMT_section->version_number = buffer[5] >> 1 & 0x1F;
    PMT_section->current_next_indicator = (buffer[5] << 7) >> 7;
    PMT_section->section_number = buffer[6];
    PMT_section->last_section_number = buffer[7];
    //PMT_section->reserved_3 = buffer[8] >> 5;
    PMT_section->PCR_PID = ((buffer[8] << 8) | buffer[9]) & 0x1FFF;
    //PMT_section->reserved_4 = buffer[10] >> 4;
    PMT_section->program_info_length = (buffer[10] & 0x0F) << 8 | buffer[11];
    // Get CRC_32
    len = PMT_section->section_length + 3;
    PMT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16 |
                          (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    Parser_printf("PMT: program 0x%04x(%d) => pmt pid: 0x%03x(%d)\n", pmt_pid[current_program_num].program_number,
                  pmt_pid[current_program_num].program_number,
                  pmt_pid[current_program_num].PMT_PID,
                  pmt_pid[current_program_num].PMT_PID);
    Parser_printf("  table_id : 0x%02x\n", PMT_section->table_id);
    Parser_printf("  section_syntax_indicator : 0x%02x\n", PMT_section->section_syntax_indicator);
    Parser_printf("  section_length : 0x%02x(%d)\n", PMT_section->section_length, PMT_section->section_length);
    Parser_printf("  program_number : 0x%04x\n", PMT_section->program_number);
    Parser_printf("  version_number : 0x%02x\n", PMT_section->version_number);
    Parser_printf("  current_next_indicator : 0x%x\n", PMT_section->current_next_indicator);
    Parser_printf("  section_number : 0x%02x(%d)\n", PMT_section->section_number, PMT_section->section_number);
    Parser_printf("  last_section_number : 0x%02x(%d)\n", PMT_section->last_section_number, PMT_section->last_section_number);
    Parser_printf("  PCR_PID : 0x%03x(%d)\n", PMT_section->PCR_PID, PMT_section->PCR_PID);
    Parser_printf("  program_info_length : 0x%04x(%d)\n", PMT_section->program_info_length, PMT_section->program_info_length);

    // program info descriptor
    if ( PMT_section->program_info_length != 0 )
        pos += PMT_section->program_info_length;

    Parser_printf("  componet:\n");
    // Get stream type and PID
    stES_INFO es_info;
    for ( ; pos < (U32)(PMT_section->section_length + 3 ) -  4; )
    {
        // Store in es
        es_info.stream_type = buffer[pos];
        es_info.reserved1 = buffer[pos+1] >> 5;
        es_info.elementary_PID = ((buffer[pos+1] << 8) | buffer[pos+2]) & 0x1FFF;
        es_info.reserved2 = buffer[pos+3] >> 4;
        es_info.ES_info_length = (buffer[pos+3] & 0x0F) << 8 | buffer[pos+4];

        if (es_info.stream_type <= 0x14)
            {
            Parser_printf("      stream type: 0x%02x => %s\n", es_info.stream_type, stream_type_info[es_info.stream_type]);
            }
        else if (es_info.stream_type > 0x14 && es_info.stream_type <= 0x7e)
            {
            Parser_printf("      stream type: 0x%02x => %s\n", es_info.stream_type, stream_type_info[0x15]);
            }
        else if (es_info.stream_type >= 0x80 && es_info.stream_type <= 0xff)
            {
            Parser_printf("      stream type: 0x%02x => %s\n", es_info.stream_type, stream_type_info[0x16]);
            }
        else
            {
            Parser_printf("      stream type: 0x%02x\n", es_info.stream_type);
            }

        Parser_printf("          elementary_pid: 0x%03x(%d)\n", es_info.elementary_PID, es_info.elementary_PID);
        Parser_printf("          es_info_length: 0x%02x\n", es_info.ES_info_length);

        pos = pos + 5 + es_info.ES_info_length;
        i++;
    }

    Parser_printf("  CRC_32 : 0x%02x\n", PMT_section->CRC_32);
    Parser_printf("\n");

    return ;
}

stDT_Date MJD2YMD(U16 mjd)
{
    S32 year_x, month_x;
    S32 m_day, m_month,m_year;
    U8 k;
    stDT_Date date;

    year_x 	 = (int)( (mjd * 100 - 1507820) / 36525 );
    month_x  = (int)( (mjd * 10000 - 149561000 - (int)((year_x * 36525) / 100) * 10000) / 306001 );
    m_day = mjd - 14956 - (int) ((year_x * 36525) / 100) - (int) ((month_x * 306001) / 10000);

    if (month_x == 14 || month_x == 15)
        k = 1;
    else
        k = 0;

    m_year = (int) (year_x + k);
    m_month  = (int) (month_x - 1 - (k * 12));

    if ( m_month > 12 )
    {
        m_year++;
        m_month -= 12;
    }

    m_year += 1900;

    date.year = m_year;
    date.month = m_month;
    date.day = m_day;

    return date;
}

stDT_Time UTC2HM(U32 utc)
{
    stDT_Time time;

    time.hour = (U8) ( ( ((utc >> 20) & 0x0F) * 10 ) + ( (utc >> 16) & 0x0F ) );
    time.minute	= (U8) ( ( ((utc >> 12) & 0x0F) * 10 ) + ( (utc >> 8) & 0x0F ) );
    time.second	= (U8) ( ( ((utc >> 4) & 0x0F) * 10 ) + ( utc & 0x0F ) );

    return time;
}

void ParseTDTSection(stTS_TDT *TDT_section, const U8 *buffer)
{
    stDT_Date date;
    stDT_Time time;

    TDT_section->table_id = buffer[0];
    TDT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    TDT_section->reserved_future_use = buffer[1] >> 6 & 0x01;
    TDT_section->reserved = buffer[1] >> 4 & 0x03;
    TDT_section->section_length = ((U16)buffer[1] & 0x0f) | buffer[2];
    TDT_section->UTC_time_date = (U16)buffer[3] << 8 | buffer[4];
    TDT_section->UTC_time_time1 = ((U16)buffer[5] << 8) | buffer[6];
    TDT_section->UTC_time_time2 = buffer[7];

    date = MJD2YMD(TDT_section->UTC_time_date);
    time = UTC2HM((U32)TDT_section->UTC_time_time1 << 8 | TDT_section->UTC_time_time2);

    Parser_printf("TDT:\n");
    Parser_printf("  table id: 0x%02x\n", TDT_section->table_id);
    Parser_printf("  section_syntax_indicator: 0x%02x\n", TDT_section->section_syntax_indicator);
    Parser_printf("  section_length: 0x%02x(%d)\n", TDT_section->section_length, TDT_section->section_length);
    Parser_printf("  utc_time: 0x%04x%04x%02x => %04d/%02d/%02d %02d:%02d:%02d\n", TDT_section->UTC_time_date,
                  TDT_section->UTC_time_time1, TDT_section->UTC_time_time2, date.year, date.month, date.day,
                  time.hour, time.minute, time.second);
    Parser_printf("\n");

    return;
}

void ParseTOTSection(stTS_TOT *TOT_section, const U8 *buffer)
{
    stDT_Date date;
    stDT_Time time;
    int len = 0;
    int pos = 10;

    memset(TOT_section, 0, sizeof(stTS_TOT));
    TOT_section->table_id = buffer[0];
    TOT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    TOT_section->reserved_future_use = buffer[1] >> 6 & 0x01;
    TOT_section->reserved_1 = buffer[1] >> 4 & 0x03;
    TOT_section->section_length = ((U16)buffer[1] & 0x0f) | buffer[2];
    TOT_section->UTC_time_date = (U16)buffer[3] << 8 | buffer[4];
    TOT_section->UTC_time_time1 = (U16)buffer[5] << 8 | buffer[6];
    TOT_section->UTC_time_time2 = buffer[7];
    TOT_section->reserved_2 = buffer[8] >> 4 & 0x0f;
    TOT_section->descriptors_loop_length = ((U16)buffer[8] & 0x0f) | buffer[9];
    len = TOT_section->section_length + 3;
    TOT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16 |
                          (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    date = MJD2YMD(TOT_section->UTC_time_date);
    time = UTC2HM((U32)TOT_section->UTC_time_time1 << 8 | TOT_section->UTC_time_time2);

    Parser_printf("TOT:\n");
    Parser_printf("  table id: 0x%02x\n", TOT_section->table_id);
    Parser_printf("  section_syntax_indicator: 0x%02x\n", TOT_section->section_syntax_indicator);
    Parser_printf("  section_length: 0x%02x(%d)\n", TOT_section->section_length, TOT_section->section_length);
    Parser_printf("  utc_time: 0x%04x%04x%02x => %04d/%02d/%02d %02d:%02d:%02d\n", TOT_section->UTC_time_date,
                  TOT_section->UTC_time_time1, TOT_section->UTC_time_time2,
                  date.year, date.month, date.day, time.hour, time.minute, time.second);
    Parser_printf("  descriptors_loop_length: 0x%02x(%d)\n", TOT_section->descriptors_loop_length,
                  TOT_section->descriptors_loop_length);
    Parser_printf("  CRC_32: 0x%08x\n", TOT_section->CRC_32);
    Parser_printf("\n");

    if (TOT_section->descriptors_loop_length != 0)
    {
        int loop = 0;
        for ( ; loop < TOT_section->descriptors_loop_length; )
        {
            switch (buffer[pos + loop])
            {
            case 0x58:
                TOT_section->local_time_offset_descriptor[TOT_section->local_time_offset_descriptor_num] =
                    (st_local_time_offset_descriptor *)malloc(sizeof(st_local_time_offset_descriptor));
                loop += parse_local_time_offset_descriptor(TOT_section->local_time_offset_descriptor[TOT_section->local_time_offset_descriptor_num], buffer+pos+loop);
                TOT_section->local_time_offset_descriptor_num++;
                break;
            default:
                loop += *(buffer+pos+loop+1);
                break;
            }
        }
    }

    return ;
}

void ParseSDTSection(stTS_SDT *SDT_section, const U8 *buffer)
{
    int len = 0;
    U32 pos = 11;

    SDT_section->table_id = buffer[0];
    SDT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    //SDT_section->reserved_future_use_1 = buffer[1] >> 6 & 0x01;
    //SDT_section->reserved_1 = buffer[1] >> 4 & 0x03;
    SDT_section->section_length = ((U16)buffer[1] & 0x0f) | buffer[2];
    SDT_section->transport_stream_id = ((U16)buffer[3] & 0x0f) | buffer[4];
    //SDT_section->reserved_2 = buffer[5] >> 6;
    SDT_section->version_number = buffer[5] >> 1 & 0x1f;
    SDT_section->current_next_indicator = buffer[5] & 0x01;
    SDT_section->section_number = buffer[6];
    SDT_section->last_section_number = buffer[7];
    SDT_section->original_network_id = (U16)buffer[8] << 8 | buffer[9];
    //SDT_section->reserved_future_use_2 = buffer[10];
    len = SDT_section->section_length + 3;
    SDT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16 |
                          (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    Parser_printf("SDT:\n");
    Parser_printf("  table_id: 0x%02x\n", SDT_section->table_id);
    Parser_printf("  section_syntax_indicator: 0x%02x\n", SDT_section->section_syntax_indicator);
    Parser_printf("  section_length: 0x%02x(%d)\n", SDT_section->section_length, SDT_section->section_length);
    Parser_printf("  transport_stream_id: 0x%04x\n", SDT_section->transport_stream_id);
    Parser_printf("  version_number: 0x%02x\n", SDT_section->version_number);
    Parser_printf("  current_next_indicator: 0x%x\n", SDT_section->current_next_indicator);
    Parser_printf("  section_number: 0x%02x(%d)\n", SDT_section->section_number, SDT_section->section_number);
    Parser_printf("  last_section_number: 0x%02x(%d)\n", SDT_section->last_section_number, SDT_section->last_section_number);
    Parser_printf("  original_network_id: 0x%04x\n", SDT_section->original_network_id);
    Parser_printf("  services:\n");

    for (; pos < (U32)SDT_section->section_length + 3 - 4; )
    {
        memset(&sdt_info[current_sdt_info_num], 0, sizeof(stSDT_INFO));
        sdt_info[current_sdt_info_num].service_id = (U16)buffer[pos] << 8 | buffer[pos + 1];
        sdt_info[current_sdt_info_num].reserved_future_use = buffer[pos + 2] >> 2 & 0x3f;
        sdt_info[current_sdt_info_num].EIT_schedule_flag = buffer[pos + 2] >> 1 & 0x01;
        sdt_info[current_sdt_info_num].EIT_present_following_flag = buffer[pos + 2] & 0x01;
        sdt_info[current_sdt_info_num].running_status = buffer[pos + 3] >> 5 & 0x07;
        sdt_info[current_sdt_info_num].free_CA_mode = buffer[pos + 3] >> 4 & 0x01;
        sdt_info[current_sdt_info_num].descriptors_loop_length = ((U16)buffer[pos + 3] & 0x0f) | buffer[pos + 4];

        Parser_printf("    service_id: 0x%04x(%d)\n", sdt_info[current_sdt_info_num].service_id, sdt_info[current_sdt_info_num].service_id);
        Parser_printf("      EIT_schedule_flag: 0x%x\n", sdt_info[current_sdt_info_num].EIT_schedule_flag);
        Parser_printf("      EIT_present_following_flag: 0x%x\n", sdt_info[current_sdt_info_num].EIT_present_following_flag);
        Parser_printf("      running_status: 0x%x\n", sdt_info[current_sdt_info_num].running_status);
        Parser_printf("      free_CA_mode: 0x%x\n", sdt_info[current_sdt_info_num].free_CA_mode);
        Parser_printf("      descriptors_loop_length: 0x%02x\n", sdt_info[current_sdt_info_num].descriptors_loop_length);

        if (sdt_info[current_sdt_info_num].descriptors_loop_length != 0)
        {
            int loop = 0;
            for ( ; loop < sdt_info[current_sdt_info_num].descriptors_loop_length; )
            {
                switch (buffer[pos + 5 + loop])
                {
                case 0x48:
                {
                    sdt_info[current_sdt_info_num].service_descriptor[sdt_info[current_sdt_info_num].service_des_num]  =
                        (st_service_descriptor *)malloc(sizeof(st_service_descriptor));
                    loop += parse_service_descriptor(sdt_info[current_sdt_info_num].service_descriptor[sdt_info[current_sdt_info_num].service_des_num], buffer+pos+5+loop);
                    sdt_info[current_sdt_info_num].service_des_num++;
                    break;
                }
                case 0x5d:
                {
                    sdt_info[current_sdt_info_num].multi_service_name_descriptor[sdt_info[current_sdt_info_num].multi_service_name_des_num]  =
                        (st_multilingual_service_name_descriptor*)malloc(sizeof(st_multilingual_service_name_descriptor));
                    loop += parse_multilingual_service_name_descriptor(sdt_info[current_sdt_info_num].multi_service_name_descriptor[sdt_info[current_sdt_info_num].multi_service_name_des_num], buffer+pos+5+loop);
                    sdt_info[current_sdt_info_num].multi_service_name_des_num++;
                    break;
                }
                default:
                    loop += *(buffer+pos+5+loop+1);
                    break;
                }
            }
        }

        pos = pos + 5 + sdt_info[current_sdt_info_num].descriptors_loop_length;
        current_sdt_info_num++;
        total_sdt_info_num++;
    }


    Parser_printf("  CRC_32: 0x%08x\n", SDT_section->CRC_32);
    Parser_printf("\n");

    return ;
}

void ParseEITSection(stTS_EIT *EIT_section, const U8 *buffer)
{
    int len = 0;
    int pos = 14;
    stDT_Date date;
    stDT_Time time;
    stDT_Time duration;

    memset(EIT_section, 0, sizeof(stTS_EIT));
    EIT_section->table_id = buffer[0];
    EIT_section->section_syntax_indicator = buffer[1] >> 7 & 0x01;
    //EIT_section->reserved_future_use = buffer[1] >> 6 & 0x01;
    //EIT_section->reserved_1 = buffer[1] >> 4 & 0x03;
    EIT_section->section_length = ((U16)buffer[1] & 0x0f) | buffer[2];
    EIT_section->service_id = (U16)buffer[3] << 8 | buffer[4];
    //EIT_section->reserved_2 = buffer[5] >> 6 & 0x03;
    EIT_section->version_number = buffer[5] >> 1 & 0x1f;
    EIT_section->current_next_indicator = buffer[5] & 0x01;
    EIT_section->section_number = buffer[6];
    EIT_section->last_section_number = buffer[7];
    EIT_section->transport_stream_id = (U16)buffer[8] << 8 | buffer[9];
    EIT_section->original_network_id= (U16)buffer[10] << 8 | buffer[11];
    EIT_section->segment_last_section_number = buffer[12];
    EIT_section->last_table_id = buffer[13];

    len = EIT_section->section_length + 3;
    EIT_section->CRC_32 = (buffer[len-4] & 0x000000FF) << 24	| (buffer[len-3] & 0x000000FF) << 16 |
                          (buffer[len-2] & 0x000000FF) << 8 | (buffer[len-1] & 0x000000FF);

    if (EIT_section->table_id == EIT_TABLE_ID_ACTUAL)
    {
        Parser_printf("EIT ACTUAL, Section %d:\n", buffer[6]);
    }
    else if (EIT_section->table_id == EIT_TABLE_ID_OTHER)
    {
        Parser_printf("EIT OTHER:\n");
    }
    else if (EIT_section->table_id >= EIT_SCHEDUAL_TABLE_ID_ACTUAL_FIRST &&
             EIT_section->table_id <= EIT_SCHEDUAL_TABLE_ID_ACTUAL_LAST)
    {
        Parser_printf("EIT ACTUAL SCHEDUAL, Section %d:\n", buffer[6]);
    }
    else if (EIT_section->table_id >= EIT_SCHEDUAL_TABLE_ID_OTHER_FIRST &&
             EIT_section->table_id <= EIT_SCHEDUAL_TABLE_ID_OTHER_LAST)
    {
        Parser_printf("EIT OTHER SCHEDUAL:\n");
    }

    Parser_printf("  table_id: 0x%02x\n", EIT_section->table_id);
    Parser_printf("  section_syntax_indicator: 0x%02x\n", EIT_section->section_syntax_indicator);
    Parser_printf("  section_length: 0x%02x(%d)\n", EIT_section->section_length, EIT_section->section_length);
    Parser_printf("  service_id: 0x%04x\n", EIT_section->service_id);
    Parser_printf("  version_number: 0x%02x\n", EIT_section->version_number);
    Parser_printf("  current_next_indicator: 0x%x\n", EIT_section->current_next_indicator);
    Parser_printf("  section_number: 0x%02x(%d)\n", EIT_section->section_number, EIT_section->section_number);
    Parser_printf("  last_section_number: 0x%02x(%d)\n", EIT_section->last_section_number, EIT_section->last_section_number);
    Parser_printf("  transport_stream_id: 0x%04x\n", EIT_section->transport_stream_id);
    Parser_printf("  original_network_id: 0x%04x\n", EIT_section->original_network_id);
    Parser_printf("  segment_last_section_number: 0x%02x(%d)\n", EIT_section->segment_last_section_number,
                  EIT_section->segment_last_section_number);
    Parser_printf("  last_table_id: 0x%02x\n", EIT_section->last_table_id);
    Parser_printf("  events:\n");

    for ( ; pos < EIT_section->section_length + 3 - 4; )
    {
        EIT_section->eit_info[EIT_section->eit_info_num] = (stEIT_INFO *)malloc(sizeof(stEIT_INFO));
        memset(EIT_section->eit_info[EIT_section->eit_info_num], 0, sizeof(stEIT_INFO));

        EIT_section->eit_info[EIT_section->eit_info_num]->event_id = (U16)buffer[pos] << 8 | buffer[pos + 1];
        EIT_section->eit_info[EIT_section->eit_info_num]->start_time_date = (U16)buffer[pos + 2] << 8 | buffer[pos + 3];
        EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time1 = (U16)buffer[pos + 4] << 8 | buffer[pos + 5];
        EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time2 = buffer[pos + 6];
        EIT_section->eit_info[EIT_section->eit_info_num]->duration1 = (U16)buffer[pos + 7] << 8 | (U32)buffer[pos + 8];
        EIT_section->eit_info[EIT_section->eit_info_num]->duration2 = buffer[pos + 9];
        EIT_section->eit_info[EIT_section->eit_info_num]->running_status = buffer[pos + 10] >> 5 & 0x07;
        EIT_section->eit_info[EIT_section->eit_info_num]->free_CA_mode = buffer[pos + 10] >> 2 & 0x01;
        EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length = ((U16)buffer[pos + 10] & 0x0f) | buffer[pos + 11];

        date = MJD2YMD(EIT_section->eit_info[EIT_section->eit_info_num]->start_time_date);
        time = UTC2HM((U32)EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time1 << 8 | EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time2);
        duration = UTC2HM((U32)EIT_section->eit_info[EIT_section->eit_info_num]->duration1 << 8 | EIT_section->eit_info[EIT_section->eit_info_num]->duration2);

        Parser_printf("    event_id: 0x%04x\n", EIT_section->eit_info[EIT_section->eit_info_num]->event_id);
        Parser_printf("      start_time: 0x%04x%04x%02x => %04d/%02d/%02d %02d:%02d:%02d\n", EIT_section->eit_info[EIT_section->eit_info_num]->start_time_date,
                      EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time1, EIT_section->eit_info[EIT_section->eit_info_num]->start_time_time1,
                      date.year, date.month, date.day, time.hour, time.minute, time.second);
        Parser_printf("      duration: 0x%04x%02x => %02d:%02d:%02d\n", EIT_section->eit_info[EIT_section->eit_info_num]->duration1, EIT_section->eit_info[EIT_section->eit_info_num]->duration2,
                      duration.hour, duration.minute, duration.second);
        Parser_printf("      running_status: 0x%x\n", EIT_section->eit_info[EIT_section->eit_info_num]->running_status);
        Parser_printf("      free_CA_mode: 0x%x\n", EIT_section->eit_info[EIT_section->eit_info_num]->free_CA_mode);
        Parser_printf("      descriptors_loop_length: 0x%02x\n", EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length);

        if (EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length != 0)
        {
            //printf("length is 0x%x\n", EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length);
            int loop = 0;
            for ( ; loop < EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length; )
            {
                switch (buffer[pos + 12])
                {
                case 0x4d:	// short_event_descriptor
                {
                    EIT_section->eit_info[EIT_section->eit_info_num]->short_event_descriptor[EIT_section->eit_info[EIT_section->eit_info_num]->short_event_descriptor_num] =
                        (st_short_event_descriptor *)malloc(sizeof(st_short_event_descriptor));
                    loop += parse_short_event_descriptor(EIT_section->eit_info[EIT_section->eit_info_num]->short_event_descriptor[EIT_section->eit_info[EIT_section->eit_info_num]->short_event_descriptor_num], buffer + pos + 12 + loop);
                    EIT_section->eit_info[EIT_section->eit_info_num]->short_event_descriptor_num++;

                    break;
                }
                case 0x4e:  // extended_event_descriptor
                {
                    EIT_section->eit_info[EIT_section->eit_info_num]->extended_event_descriptor[EIT_section->eit_info[EIT_section->eit_info_num]->extended_event_descriptor_num] =
                        (st_extended_event_descriptor *)malloc(sizeof(st_extended_event_descriptor));
                    loop += parse_extended_event_descriptor(EIT_section->eit_info[EIT_section->eit_info_num]->extended_event_descriptor[EIT_section->eit_info[EIT_section->eit_info_num]->extended_event_descriptor_num], buffer + pos + 12 + loop);
                    EIT_section->eit_info[EIT_section->eit_info_num]->extended_event_descriptor_num++;
                    break;
                }
                default:
                    loop += *(buffer + pos + 12 + loop + 1);
                    break;
                }
            }
        }

        pos += 12 + EIT_section->eit_info[EIT_section->eit_info_num]->descriptors_loop_length;
        EIT_section->eit_info_num++;
    }

    Parser_printf("  CRC_32: 0x%08x\n", EIT_section->CRC_32);
    Parser_printf("\n");

    return ;
}

/************************************************************************
* Descriptors Parser
************************************************************************/
int parse_local_time_offset_descriptor(st_local_time_offset_descriptor *local_time_offset, const U8 *buffer)
{
    int loop;

    memset(local_time_offset, 0, sizeof(st_local_time_offset_descriptor_info));
    local_time_offset->descriptor_tag = buffer[0];
    local_time_offset->descriptor_length = buffer[1];
    //printf("length is %d\n\n\n", buffer[1]);

    if (local_time_offset->descriptor_length != 0)
    {
        loop = 0;

        for ( ; loop < local_time_offset->descriptor_length; )
        {
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num] =
                (st_local_time_offset_descriptor_info *)malloc(sizeof(st_local_time_offset_descriptor_info));

            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->country_code1 =
                ((U16)buffer[loop + 2] << 8) | buffer[loop + 3];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->country_code2 =
                buffer[loop + 4];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->country_region_id =
                (buffer[loop + 5] & 0x3f) >> 2;
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->local_time_offset_polarity =
                buffer[loop + 5] & 0x01;
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->local_time_offset =
                ((U16)buffer[loop + 6] << 8) | buffer[loop + 7];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->time_of_change_date =
                ((U16)buffer[loop + 8] << 8) | buffer[loop + 9];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->time_of_change_time1 =
                ((U16)buffer[loop + 10] << 8) | buffer[loop + 11];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->time_of_change_time2 =
                buffer[loop + 12];
            local_time_offset->local_time_offset_descriptor_info[local_time_offset->local_time_offset_descriptor_info_num]->next_time_offset =
                ((U16)buffer[loop + 13] << 8) | buffer[loop + 14];

            loop += sizeof(st_local_time_offset_descriptor_info);
            local_time_offset->local_time_offset_descriptor_info_num++;
        }
    }

    return local_time_offset->descriptor_length + 2;
}


int parse_short_event_descriptor(st_short_event_descriptor *short_event, const U8 *buffer)
{
    short_event->descriptor_tag = buffer[0];
    short_event->descriptor_length = buffer[1];
    short_event->ISO_639_2_language_code1 = (U16)buffer[2] << 8 | buffer[3];
    short_event->ISO_639_2_language_code2 = buffer[4];
    short_event->event_name_length = buffer[5];
    memcpy(short_event->event_name_char, buffer + 6, short_event->event_name_length);
    short_event->event_name_char[short_event->event_name_length] = '\0';
    short_event->event_name_char[short_event->event_name_length + 1] = '\0';
    short_event->text_length = buffer[6+short_event->event_name_length];
    memcpy(short_event->text_char, buffer + 6 + short_event->event_name_length, short_event->text_length);
    short_event->text_char[short_event->text_length] = '\0';
    short_event->text_char[short_event->text_length + 1] = '\0';

    Parser_printf("        descriptor_tag: 0x%02x => short_event_descriptor\n", short_event->descriptor_tag);
    Parser_printf("          descriptor_length(byte): %d\n", short_event->descriptor_length);
    Parser_printf("          ISO_639_2_language_code: 0x%06x =>\n", (U32)short_event->ISO_639_2_language_code1 << 8 |
                  short_event->ISO_639_2_language_code2);
    Parser_printf("          event_name_length: 0x%02x(%d)\n", short_event->event_name_length,
                  short_event->event_name_length);
    Parser_printf("          event name: %s\n", short_event->event_name_char);
    Parser_printf("          text_length: 0x%x(%d)\n", short_event->text_length, short_event->text_length);
    Parser_printf("          text: %s\n", short_event->text_char);

    return short_event->descriptor_length + 2;
}

int parse_extended_event_descriptor(st_extended_event_descriptor *extended_event, const U8 *buffer)
{
    memset(extended_event, 0, sizeof(st_extended_event_descriptor));
    extended_event->descriptor_tag = buffer[0];
    extended_event->descriptor_length = buffer[1];
    extended_event->descriptor_number = (buffer[2] & 0xf0) >> 4;
    extended_event->last_descriptor_number = buffer[2] & 0x0f;
    extended_event->ISO_639_2_language_code1 = (U16)buffer[3] << 8 | buffer[4];
    extended_event->ISO_639_2_language_code2 = buffer[5];
    extended_event->length_of_items = buffer[6];

    int pos = 7;
    for ( ; pos < extended_event->descriptor_length; )
    {
        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num] =
            (st_extended_event_descriptor_info *)malloc(sizeof(st_extended_event_descriptor_info));

        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_length = buffer[pos];
        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_char =
            (U8 *)malloc(sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_length);
        memcpy(extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_char,
               buffer + pos + 1, sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_length);
        pos += 1 + extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_description_length;

        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_length = buffer[pos];
        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_char =
            (U8 *)malloc(sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_length);
        memcpy(extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_char,
               buffer + pos + 1, sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_length);
        pos += 1 + extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->item_length;

        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_length = buffer[pos];
        extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_char =
            (U8 *)malloc(sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_length);
        memcpy(extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_char,
               buffer + pos + 1, sizeof(U8) * extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_length);
        pos += 1 + extended_event->extended_event_descriptor[extended_event->extended_event_descriptor_num]->text_length;

        extended_event->extended_event_descriptor_num++;
    }

    return extended_event->descriptor_length + 2;
}

int parse_service_descriptor(st_service_descriptor *service_des, const U8 *buffer)
{
    service_des->descriptor_tag = buffer[0];
    service_des->descriptor_length = buffer[1];
    service_des->service_type = buffer[2];
    service_des->service_provider_length = buffer[3];
    memcpy(service_des->service_provider_char, buffer + 4, service_des->service_provider_length);
    service_des->service_provider_char[service_des->service_provider_length] = '\0';
    service_des->service_name_length = buffer[4+service_des->service_provider_length];
    memcpy(service_des->service_name_char, buffer+5+service_des->service_provider_length, service_des->service_name_length);
    service_des->service_name_char[service_des->service_name_length] = '\0';

    Parser_printf("          descriptor_tag: 0x%02x => service_descriptor\n", service_des->descriptor_tag);
    Parser_printf("          descriptor_length(byte): %d\n", service_des->descriptor_length);
    Parser_printf("            service_type: 0x%02x\n", service_des->service_type);
    Parser_printf("            service_provider_length: %d\n", service_des->service_provider_length);
    Parser_printf("            service_provider_name: %s\n", service_des->service_provider_char);
    Parser_printf("            service_name_length: %d\n", service_des->service_name_length);
    Parser_printf("            service_name: %s\n", service_des->service_name_char);


    return service_des->descriptor_length + 2;
}

int parse_multilingual_service_name_descriptor(st_multilingual_service_name_descriptor *msn_des, const U8 *buffer)
{
    memset(msn_des, 0, sizeof(st_multilingual_service_name_descriptor));
    msn_des->descriptor_tag = buffer[0];
    msn_des->descriptor_length = buffer[1];

    Parser_printf("          descriptor_tag: 0x%2x => multilingual_service_name_descriptor\n", msn_des->descriptor_tag);
    Parser_printf("          descriptor_length(byte): %d\n", msn_des->descriptor_length);

    int pos = 2;

    for (; pos < msn_des->descriptor_length; )
    {
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num] =
            (st_multilingual_service_name_descriptor_info *)malloc(sizeof(st_multilingual_service_name_descriptor_info));

        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->ISO_639_2_language_code1 = (U16)buffer[pos] << 8 | buffer[pos + 1];
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->ISO_639_2_language_code2 = buffer[pos + 2];
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length = buffer[pos + 3];
        memcpy(msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_char, buffer + pos + 4,
               msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length);
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_char[msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length] = '\0';
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_length = buffer[pos + 4 + msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length];
        memcpy(msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_char, buffer+pos+4+msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length, msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_length);
        msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_char[msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_length] = '\0';

        Parser_printf("            ISO_639_2_language_code: 0x%04x%02x =>\n", msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->ISO_639_2_language_code1, msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->ISO_639_2_language_code2);
        Parser_printf("            service_provider_name_length: %d\n", msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length);
        Parser_printf("            service_provider_name: %s\n", msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_char);
        Parser_printf("            service_name_length: %d\n", msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_length);
        Parser_printf("            service_name: %s\n", msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_char);

        pos += 3 + msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_provider_name_length + 1 + msn_des->multi_service_name_des_info[msn_des->multi_service_name_des_info_num]->service_name_length;
        msn_des->multi_service_name_des_info_num++;
    }

    return msn_des->descriptor_length + 2;
}

U8 GetPATDataFromFile(const char* filename)
{
    FILE *fp;
    U8 buf[TS_PACKET_SIZE];
    int result;
    int count = 0;  // the position of first 0x47

    CSOS_WaitSemaphore(g_Parser_Access);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("open ts file failed!\n");
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    // find syn char "0x47"
    char c;
    while (!feof(fp))
    {
        fread(&c, sizeof(unsigned char), 1, fp);
        if (c != 0x47)
        {
            count++;
            continue;
        }
        else
        {
            break;
        }
    }
    //printf("the first syn byte 0x47 at %d\n", count);

    fseek(fp, count, SEEK_SET);

    stTS_PAT pat;
    section_num_save_init();
    section_parser_init(PAT_PID, PAT_TABLE_ID);
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                ParsePATSection(&pat, mpBuff);
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }
    
    fclose(fp);

    CSOS_SignalSemaphore(g_Parser_Access);

    return CS_PARSER_NO_ERROR;
}

U8 GetPMTDataFromFile(const char* filename)
{
    FILE *fp;
    U8 buf[TS_PACKET_SIZE];
    int result;
    int count = 0;  // the position of first 0x47
    U32 i;
    CSOS_WaitSemaphore(g_Parser_Access);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("open ts file failed!\n");
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    // find syn char "0x47"
    char c;
    while (!feof(fp))
    {
        fread(&c, sizeof(unsigned char), 1, fp);
        if (c != 0x47)
        {
            count++;
            continue;
        }
        else
        {
            break;
        }
    }

    //printf("the first syn byte 0x47 at %d\n", count);

    for (i=0; i<total_program_num; i++)
    {
        //stTS_PMT pmt;
        fseek(fp, count, SEEK_SET);
        section_num_save_init();
        section_parser_init(pmt_pid[i].PMT_PID, PMT_TABLE_ID);
        while (!feof(fp))
        {
            fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
            if (section_parser(buf, TS_PACKET_SIZE) == 1)
            {
                result = check_and_save_section(mpBuff[6], mpBuff[7]);
                if (result == 0)
                {
                    save_section[saved_section_num++] = mpBuff[6];
                    mPos = 0;
                    continue;
                }
                else if (result == 1)
                    continue;
                else
                    break;
            }
        }
    }

#if 0
    printf("saved section is :%d\n\n", g_saved_section_count);
    int j,k;
    for (j=0;j<g_saved_section_count;j++)
    {
        for (k=0;k<g_save_section[j].section_data.section_length;k++)
            printf("0x%x,",*(g_save_section[j].section_data.section_data+k));

        printf("\n");
    }
#endif

    fclose(fp);
    CSOS_SignalSemaphore(g_Parser_Access);

    return CS_PARSER_NO_ERROR;
}

U8 GetPMTDataFromFileByServiceID(const char* filename, U16 service_id)
{
    FILE *fp;
    U8 buf[TS_PACKET_SIZE];
    int result;
    int count = 0;  // the position of first 0x47
    U32 i;
    U32 tmp_pid = 0;
    
    CSOS_WaitSemaphore(g_Parser_Access);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        Parser_printf("open ts file failed!\n");
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    // find syn char "0x47"
    char c;
    while (!feof(fp))
    {
        fread(&c, sizeof(unsigned char), 1, fp);
        if (c != 0x47)
        {
            count++;
            continue;
        }
        else
        {
            break;
        }
    }

    //stTS_PMT pmt;
    fseek(fp, count, SEEK_SET);
    section_num_save_init();

    if (total_program_num == 0)
    {
        fclose(fp);
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }
    
    for (i=0; i<total_program_num; i++)
    {
        if (pmt_pid[i].program_number == service_id)
        {
            tmp_pid = pmt_pid[i].PMT_PID;
        }
    }

    if (tmp_pid == 0)
    {
        fclose(fp);
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }
    else
        section_parser_init(tmp_pid, PMT_TABLE_ID);
    
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }

    fclose(fp);

    CSOS_SignalSemaphore(g_Parser_Access);

    return CS_PARSER_NO_ERROR;
}


U8 GetSDTDataFromFile(const char* filename)
{
    FILE *fp;
    U8 buf[TS_PACKET_SIZE];
    int result;
    int count = 0;  // the position of first 0x47

    CSOS_WaitSemaphore(g_Parser_Access);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        Parser_printf("open ts file failed!\n");
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    // find syn char "0x47"
    char c;
    while (!feof(fp))
    {
        fread(&c, sizeof(unsigned char), 1, fp);
        if (c != 0x47)
        {
            count++;
            continue;
        }
        else
        {
            break;
        }
    }

    //stTS_SDT sdt;
    fseek(fp, count, SEEK_SET);
    section_num_save_init();
    section_parser_init(SDT_PID, SDT_TABLE_ID_ACTUAL);
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }
    
    fclose(fp);

    CSOS_SignalSemaphore(g_Parser_Access);

    return CS_PARSER_NO_ERROR;
}

U8 GetTDTTOTEITDataFromFile(const char* filename)
{
    FILE *fp;
    U8 buf[TS_PACKET_SIZE];
    int result;
    int count = 0;  // the position of first 0x47

    CSOS_WaitSemaphore(g_Parser_Access);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        Parser_printf("open ts file failed!\n");
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    // find syn char "0x47"
    char c;
    while (!feof(fp))
    {
        fread(&c, sizeof(unsigned char), 1, fp);
        if (c != 0x47)
        {
            count++;
            continue;
        }
        else
        {
            break;
        }
    }

    //stTS_EIT eit;
    fseek(fp, count, SEEK_SET);
    section_num_save_init();
    section_parser_init(EIT_PID, EIT_TABLE_ID_ACTUAL);
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }

    //stTS_TDT tdt;
    fseek(fp, count, SEEK_SET);
    section_num_save_init();
    section_parser_init(TDT_PID, TDT_TABLE_ID);
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }

    //stTS_TOT tot;
    fseek(fp, count, SEEK_SET);
    section_num_save_init();
    section_parser_init(TOT_PID, TOT_TABLE_ID);
    while (!feof(fp))
    {
        fread(buf, sizeof(U8), TS_PACKET_SIZE, fp);
        if (section_parser(buf, TS_PACKET_SIZE) == 1)
        {
            result = check_and_save_section(mpBuff[6], mpBuff[7]);
            if (result == 0)
            {
                save_section[saved_section_num++] = mpBuff[6];
                mPos = 0;
                continue;
            }
            else if (result == 1)
                continue;
            else
                break;
        }
    }


#if 0
    printf("saved section is :%d\n\n", g_saved_section_count);
    int j,k;
    for (j=0;j<g_saved_section_count;j++)
    {
        for (k=0;k<g_save_section[j].section_data.section_length;k++)
            printf("0x%x,",*(g_save_section[j].section_data.section_data+k));

        printf("\n");
    }
#endif

    fclose(fp);

    CSOS_SignalSemaphore(g_Parser_Access);

    return CS_PARSER_NO_ERROR;
}


//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    U8 *buf;
    U32 length;
}stMSG;

tCS_SI_Report CS_Parser_Clear_PMT_Info(void)
{
    U8	index;

    CSOS_WaitSemaphore(g_sem_PMT_Access);
    for (index = 0; index < kCS_SI_MAX_NO_OF_PROGRAM_DESCRIPTORS_PER_SERVICE; index++)
    {
        if (g_Current_PMT.Prog_Descriptors[index]!=NULL)
        {
            CSOS_DeallocateMemory(NULL, g_Current_PMT.Prog_Descriptors[index]);
            g_Current_PMT.Prog_Descriptors[index] = NULL;
        }
    }

    for (index = 0; index < kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE; index++)
    {
        if (g_Current_PMT.ESDescriptors[index]!=NULL)
        {
            CSOS_DeallocateMemory(NULL, g_Current_PMT.ESDescriptors[index]);
            g_Current_PMT.ESDescriptors[index] = NULL;
        }
    }

    memset(&g_Current_PMT, 0, sizeof(tCS_SI_PMT_Info));
    g_Current_PMT.IsValid = FALSE;
    g_SectionNumberMask_PMT.toFilter = 0;
    g_SectionNumberMask_PMT.filtered = 0;

    CSOS_SignalSemaphore(g_sem_PMT_Access);

    return(eCS_SI_OK);
}

typedef enum
{
    eCS_SI_PARSE_NO_DESCRIPTOR,
    eCS_SI_PARSE_PAT_DESCRIPTOR,
    eCS_SI_PARSE_PMT_DESCRIPTOR,
    eCS_SI_PARSE_CAT_DESCRIPTOR,
    eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR,
    eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR,
    eCS_SI_PARSE_NIT_ACTUAL_DESCRIPTOR,
    eCS_SI_PARSE_NIT_OTHER_DESCRIPTOR,
    eCS_SI_PARSE_ES_DESCRIPTOR,
    eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR,
    eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR
}tCS_Parser_DescriptorToParse;

U16 CS_Parser_parseDescriptors(U8 *ptrDescriptor, tCS_Parser_DescriptorToParse eDescType)
{
    U8 *ptrLoop;
    //BOOL StringIsEmpty;
    //U16 noOfBytesToCopy;
    U8 descriptor_length;
    //U8 service_type;
    //U8 service_provider_name_length, service_name_length;
    U8 noOfBytesParsed;
    //U8 i;

    descriptor_length = *(ptrDescriptor + 1);
    ptrLoop = ptrDescriptor+2;

    switch ( *ptrDescriptor )
    {
    case VIDEO_STREAM_DESCRIPTOR:
    {
    }
    break;

    case AUDIO_STREAM_DESCRIPTOR:
    {
    }
    break;

    case CA_DESCRIPTOR: 				/* 0x09 	*/
    {
        U16  CA_system_ID;

        switch (eDescType)
        {
        case eCS_SI_PARSE_ES_DESCRIPTOR:
        case eCS_SI_PARSE_PMT_DESCRIPTOR:
        {
            //static int ca_count =0;
            //int i =0;
            g_Current_PMT.Scramble = 1;

            CA_system_ID = ((*ptrLoop)<<8)|*(ptrLoop+1);

        }
        break;
        default:
            break;
        }
    }break;

    case ISO_639_LANGUAGE_DESCRIPTOR:		/* 0x0A 	*/
    {
        switch (eDescType)
        {
        case eCS_SI_PARSE_ES_DESCRIPTOR:
        {
            tCS_SI_ESDescriptor	* ptrESDesc;

            if (g_Current_PMT.ESNumber == 0)
                break;

            ptrESDesc = g_Current_PMT.ESDescriptors[g_Current_PMT.ESNumber-1];

            /* Add By River 11.22.2007 */
            if ( ptrESDesc == NULL ) break;
            if ( ptrESDesc->ESType == eCS_SI_ES_TYPE_UNKNOWN )
            {
                ptrESDesc->ESType = eCS_SI_ES_TYPE_AUDIO_AC3;
            }

            noOfBytesParsed = 0;
            /*while(noOfBytesParsed < descriptor_length)      目前只取第一个语言*/
            if ((descriptor_length!=0)&&(ptrESDesc!=NULL))
            {

                memcpy(ptrESDesc->Language, ptrLoop, kCS_SI_TRIGRAM_MAX_LENGTH);
                ptrESDesc->Language[kCS_SI_TRIGRAM_MAX_LENGTH] = '\0';

                /*noOfBytesParsed += 4;*/
                /*ptrLoop += 4;*/

            }	/* end While */

        }
        break;

        default:
            break;
        }

    }
    break;

    case NETWORK_NAME_DESCRIPTOR :		/*	0x40	*/
    {
    }
    break;

    case SERVICE_LIST_DESCRIPTOR :							/*	0x41	*/
#if 0
        {
            switch (eDescType)
            {
            case eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR:
            case eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR:

            {
                tCS_SI_TS_Info * NIT_TS_Info;
                U8		ts_desc_num;
                U32		temp_value;
                U8		prog_index;


                if (eDescType == eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Actual.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Actual.TSInfo[Current_NIT_Actual.TSNumber -1];
                }
                else if (eDescType == eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Other.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Other.TSInfo[Current_NIT_Other.TSNumber-1];
                }
                else
                {
                    break;
                }

                ts_desc_num = NIT_TS_Info->TSDescNumber;

                if ((NIT_TS_Info->TSDescriptors[ts_desc_num] =
                            (tCS_SI_TSDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_TSDescriptor))) == NULL)
                {
                    break;
                }

                NIT_TS_Info->TSDescNumber++;
                NIT_TS_Info->TSDescriptors[ts_desc_num]->TSDescType = SERVICE_LIST_DESCRIPTOR;

                noOfBytesParsed = 0;
                prog_index = 0;

                while (noOfBytesParsed < descriptor_length)
                {
                    if (prog_index > kCS_SI_MAX_NO_OF_SERVICE_PER_TS)
                        break;

                    temp_value = ((*ptrLoop) << 8) + (*(ptrLoop+1));
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.Services[prog_index].ServiceID = temp_value;
                    prog_index++;

                    noOfBytesParsed+=2;
                    temp_value =  (*(ptrLoop + 2))&0xff;
                    switch (temp_value &0xff)
                    {
                    case 0x1:
                        NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.Services[prog_index].ServiceType =  eCS_SI_TV_SERVICE;
                        break;
                    case 0x2:
                        NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.Services[prog_index].ServiceType =  eCS_SI_RADIO_SERVICE;
                        break;
                    default:
                        NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.Services[prog_index].ServiceType =  eCS_SI_UNKOWN_SERVICE;
                        break;
                    }

                    noOfBytesParsed+=1;
                    ptrLoop+=3;
                }
                NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.ServiceNumber = prog_index;


            }

            default:
                break;
            }
        }
#endif
        break;

    case STUFFING_DESCRIPTOR :								/*	0x42	*/
    {
    }
    break;

    case SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR : 			/*	0x43	*/
    {
#if 0
        switch ( eDescriptorToParse)
        {
        case ePI_PARSE_NIT_DESCRIPTOR:
        {
            pstDeliveryInfoTab = (tPI_SatelliteMedium *)tag;


            /*	read the Transponder freq (8 Digits) in GHz, which is in
             * BCD format like XXX.XXXXX	Bytes 2, 3, 4 & 5*/
            TemporaryValue = ((*(ptrDescriptor + 2) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 2) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 3) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 3) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 4) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 4) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 5) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 5) & 0xF);
            /* frequency in Khz */
            pstDeliveryInfoTab->usFrequencyKhz = TemporaryValue * 10;

            /* read the orbital_position (4Digits) which is in BCD format XXX.X  Bytes 6 & 7*/
            TemporaryValue = ((*(ptrDescriptor + 6) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 6) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 7) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 7) & 0xF);
            /* Orbital position */


            /* read the polarisation	   Byte 8	2 bits		(b6-5) */
            TemporaryValue = ((*(ptrDescriptor + 8) >> 5) & 0x3);

            switch (TemporaryValue & 0x3)
            {
            case 0x0:
                pstDeliveryInfoTab->usPolarization =  eDBASE_LINEAR_HORIZONTAL;
                break;
            case 0x1:
                pstDeliveryInfoTab->usPolarization = eDBASE_LINEAR_VERTICAL;
                break;
            case 0x2: //left
                pstDeliveryInfoTab->usPolarization = eDBASE_LINEAR_HORIZONTAL;
                break;
            case 0x3:// right
                pstDeliveryInfoTab->usPolarization = eDBASE_LINEAR_VERTICAL;
                break;
            default:
                pstDeliveryInfoTab->usPolarization = eDBASE_LINEAR_HORIZONTAL;
                break;
            }
            /* read the modulation		   Byte 8	5 bits		(b4-0) */
            TemporaryValue = (*(ptrDescriptor + 8) & 0x1F);
            switch (TemporaryValue & 0x1F)
            {
            case 0x0:
                break;
            case 0x1:
                break;
            default:
                break;
            }

            /* read the symbol_rate (7Digits- MSymbols/S), which is in BCD format XXX.XXXX	Bytes 9, 10, 11, 12 (4 MSB)*/
            TemporaryValue = ((*(ptrDescriptor + 9) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 9) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 10) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 10) & 0xF);
            TemporaryValue = TemporaryValue * 100 + ((*(ptrDescriptor + 11) & 0xF0) >> 4) * 10 + (*(ptrDescriptor + 11) & 0xF);
            TemporaryValue = TemporaryValue * 10 + ((*(ptrDescriptor + 12) & 0xF0) >> 4);
            pstDeliveryInfoTab->usSymbolRateHz = TemporaryValue * 100;

            /* read the Fec inner		Byte 12 	4 bits		(b3-0) */
            TemporaryValue = (*(ptrDescriptor + 12) & 0xF);
            switch (TemporaryValue & 0xF)
            {
            case 0x0:
                break;
            case 0x1:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_1_2;
                break;
            case 0x2:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_2_3;
                break;
            case 0x3:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_3_4;
                break;
            case 0x4:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_5_6;
                break;
            case 0x5:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_7_8;
                break;
            case 0xF:
                pstDeliveryInfoTab->usFecRate = eDBASE_FEC_AUTO;
                break;
            default:
                break;
            }
        }
        break; /* end of case PI_PARSE_NIT_DESCRIPTOR */

        }
#endif
    }
    break;

    case CABLE_DELIVERY_SYSTEM_DESCRIPTOR : 				/*	0x44	*/

        break;

    case BOUQUET_NAME_DESCRIPTOR :							/*	0x47	*/
    {
    }
    break;

    case SERVICE_DESCRIPTOR :								/*	0x48	*/
#if 0
        {
            switch (eDescType)
            {
            case eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR:
            case eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR:
            {
                tCS_SI_ServiceDescriptor * SDT_Info;
                U8                      index;

                if (eDescType == eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR)
                {
                    if (Current_SDT_Actual.ServiceNumber == 0)
                        break;

                    SDT_Info = Current_SDT_Actual.ServicesInfo[Current_SDT_Actual.ServiceNumber-1];
                }
                else if (eDescType == eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR)
                {
                    if (Current_SDT_Other.ServiceNumber == 0)
                        break;

                    SDT_Info = Current_SDT_Other.ServicesInfo[Current_SDT_Other.ServiceNumber-1];
                }
                else
                {
                    break;
                }

                if (SDT_Info == NULL)
                {
                    break;
                }

                service_type =	*ptrLoop;

                switch (service_type)
                {
                case 0x1:
                    SDT_Info->ServiceType = eCS_SI_TV_SERVICE;
                    break;
                case 0x2:
                    SDT_Info->ServiceType = eCS_SI_RADIO_SERVICE;
                    break;

                default:
                    SDT_Info->ServiceType = eCS_SI_UNKOWN_SERVICE;
                    break;
                }
                service_provider_name_length = *(ptrLoop + 1);
                service_name_length = *(ptrLoop + 2 + service_provider_name_length);


                StringIsEmpty = TRUE;
                if ( service_name_length != 0 )
                {
                    for (i=0 ; i < service_name_length && StringIsEmpty ; i++ )
                    {
                        if ( ((char*)(ptrLoop + 3 + service_provider_name_length))[i] !=' ' )
                        {
                            StringIsEmpty = FALSE;
                        }
                    }
                }

                if (GetFreeServiceName(SDT_Info, &index) == eCS_SI_OK)
                {

                    memset(&(SDT_Info->Multi_ServiceName[index]), 0, sizeof(tCS_DB_ServiceName_t));
                    strcpy(SDT_Info->Multi_ServiceName[index].Language_Trigram, "def");
                    if ( !StringIsEmpty )
                    {
                        noOfBytesToCopy = service_name_length > kCS_DB_MAX_SERVICE_NAME_LENGTH \
                                          ? kCS_DB_MAX_SERVICE_NAME_LENGTH : service_name_length;

                        memcpy(SDT_Info->Multi_ServiceName[index].ServiceName_Text, ptrLoop + 3 + service_provider_name_length,noOfBytesToCopy);

                    }
                    else
                    {
                        strcpy(SDT_Info->Multi_ServiceName[index].ServiceName_Text, "No Name");
                    }

                    SDT_Info->Multi_ServiceName[index].ServiceName_Text[kCS_DB_MAX_SERVICE_NAME_LENGTH-1] = '\0';
                }

            }
            break;

            default:

        break;
            }
        }
#endif
        break;

    case COUNTRY_AVAILABILITY_DESCRIPTOR :					/*	0x49	*/
    {
    }
    break;

    case LINKAGE_DESCRIPTOR :								/*	0x4a	*/
    {
    }
    break;

    case NVOD_REFERENCE_DESCRIPTOR :						/*	0x4b	*/
    {
    }
    break;

    case TIME_SHIFTED_SERVICE_DESCRIPTOR :					/*	0x4c	*/
    {
    }
    break;

    case SHORT_EVENT_DESCRIPTOR :							/*	0x4d	*/
    {
    }
    break;

    case EXTENDED_EVENT_DESCRIPTOR :						/*	0x4e	*/
    {
    }
    break;

    case TIME_SHIFTED_EVENT_DESCRIPTOR :					/*	0x4f	*/
    {
    }
    break;

    case COMPONENT_DESCRIPTOR : 							/*	0x50	*/
    {
    }
    break;

    case MOSAIC_DESCRIPTOR :								/*	0x51	*/
    {
    }
    break;

    case STREAM_IDENTIFIER_DESCRIPTOR : 					/*	0x52	*/
    {

    }
    break;

    case CA_IDENTIFIER_DESCRIPTOR : 						/*	0x53	*/
    {
    }
    break;

    case CONTENT_DESCRIPTOR :								/*	0x54	*/
    {
    }
    break;

    case PARENTAL_RATING_DESCRIPTOR :						/*	0x55	*/
    {
    }
    break;

    case TELETEXT_DESCRIPTOR :								/*	0x56	*/
    {
        switch ( eDescType)
        {
        case eCS_SI_PARSE_ES_DESCRIPTOR:
        {
            U8 teletext_type;
            tCS_SI_ESDescriptor	* ptrESDesc;

            if (g_Current_PMT.ESNumber == 0)
                break;

            ptrESDesc = g_Current_PMT.ESDescriptors[g_Current_PMT.ESNumber -1];

            if (ptrESDesc == NULL)
                break;

            noOfBytesParsed = 0;
            teletext_type = eCS_SI_Teletext_Initial_Teletext_Page;
            ptrESDesc->ESType = eCS_SI_ES_TYPE_TELETEXT;
            while (noOfBytesParsed < descriptor_length)
            {
                teletext_type=(*(ptrLoop+3)&0xF8)>>3;

                if (teletext_type == eCS_SI_Teletext_Subtitle_Page)
                {
                    ptrESDesc->ESType = eCS_SI_ES_TYPE_SUBTITLE;
                    ptrESDesc->uStreamSpecific.ESSubtitleDesc.Subtitle_Type = eCS_SI_TELETEXT_SUBTITLE;
                    memcpy(ptrESDesc->Language, ptrLoop, kCS_SI_TRIGRAM_MAX_LENGTH);
                    ptrESDesc->Language[kCS_SI_TRIGRAM_MAX_LENGTH] = '\0';

                    ptrESDesc->uStreamSpecific.ESSubtitleDesc.uSubtitleSpecific.TXT_Subtitle.MagazineID
                    = *(ptrLoop + 3) & 0x7;
                    ptrESDesc->uStreamSpecific.ESSubtitleDesc.uSubtitleSpecific.TXT_Subtitle.PageID
                    = *(ptrLoop + 4);
                }
                else
                {
                    ptrESDesc->ESType = eCS_SI_ES_TYPE_TELETEXT;

                    memcpy(ptrESDesc->Language, ptrLoop, kCS_SI_TRIGRAM_MAX_LENGTH);
                    ptrESDesc->Language[kCS_SI_TRIGRAM_MAX_LENGTH] = '\0';

                    ptrESDesc->uStreamSpecific.ESTeletextDesc.MagazineID
                    = *(ptrLoop + 3) & 0x7;

                    ptrESDesc->uStreamSpecific.ESTeletextDesc.PageID
                    = *(ptrLoop + 4);
                }

                noOfBytesParsed += 5;
                ptrLoop += 5;

                if ( noOfBytesParsed != descriptor_length )
                {
                    U8	es_index;

                    es_index = g_Current_PMT.ESNumber;
                    if (es_index < kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE)
                    {
                        if ((g_Current_PMT.ESDescriptors[es_index] =
                                    (tCS_SI_ESDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_ESDescriptor))) == NULL)
                        {
                            break;
                        }
                        else
                        {
                            g_Current_PMT.ESNumber++;
                            memset(g_Current_PMT.ESDescriptors[es_index],0,sizeof(tCS_SI_ESDescriptor));
                            *(g_Current_PMT.ESDescriptors[es_index]) = *ptrESDesc;
                            ptrESDesc = g_Current_PMT.ESDescriptors[es_index];
                        }
                    }
                    else
                    {
                        break;
                    }

                }	/* end of case PI_PARSE_ES_DESCRIPTOR */

            }	/* end While */

        }
        break; /* end of case PI_PARSE_ES_DESCRIPTOR */

        default:

            break; /* end of default */
        }
    }
    break;

    case TELEPHONE_DESCRIPTOR : 							/*	0x57	*/
    {
    }
    break;

    case LOCAL_TIME_OFFSET_DESCRIPTOR : 					/*	0x58	*/
    {
    }
    break;
    case SUBTITLING_DESCRIPTOR :							/*	0x59	*/
    {
        switch ( eDescType)
        {
        case eCS_SI_PARSE_ES_DESCRIPTOR:
        {
            tCS_SI_ESDescriptor	* ptrESDesc;

            if (g_Current_PMT.ESNumber == 0)
                break;

            ptrESDesc = g_Current_PMT.ESDescriptors[g_Current_PMT.ESNumber -1];

            if (ptrESDesc == NULL)
                break;

            noOfBytesParsed = 0;
            ptrESDesc->ESType = eCS_SI_ES_TYPE_SUBTITLE;
            ptrESDesc->uStreamSpecific.ESSubtitleDesc.Subtitle_Type = eCS_SI_DVB_SUBTITLE;
            while (noOfBytesParsed < descriptor_length)
            {

                memcpy(ptrESDesc->Language, ptrLoop, kCS_SI_TRIGRAM_MAX_LENGTH);
                ptrESDesc->Language[kCS_SI_TRIGRAM_MAX_LENGTH] = '\0';

                ptrESDesc->uStreamSpecific.ESSubtitleDesc.uSubtitleSpecific.DVB_Subtitle.CompositionPageID
                = (*(ptrLoop + 4) << 8) + *(ptrLoop + 5);

                ptrESDesc->uStreamSpecific.ESSubtitleDesc.uSubtitleSpecific.DVB_Subtitle.AncillaryPageID
                = (*(ptrLoop + 6) << 8) + *(ptrLoop + 7);

                ptrESDesc->ESType = eCS_SI_ES_TYPE_SUBTITLE;

                noOfBytesParsed += 8;
                ptrLoop += 8;

                if ( noOfBytesParsed != descriptor_length )
                {
                    U8	es_index;

                    es_index = g_Current_PMT.ESNumber;
                    if (es_index < kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE)
                    {
                        if ((g_Current_PMT.ESDescriptors[es_index] =
                                    (tCS_SI_ESDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_ESDescriptor))) == NULL)
                        {
                            break;
                        }
                        else
                        {
                            g_Current_PMT.ESNumber++;
                            memset(g_Current_PMT.ESDescriptors[es_index],0,sizeof(tCS_SI_ESDescriptor));
                            *(g_Current_PMT.ESDescriptors[es_index]) = *ptrESDesc;
                            ptrESDesc = g_Current_PMT.ESDescriptors[es_index];
                        }
                    }
                    else
                    {
                        break;
                    }

                }	/* end of case PI_PARSE_ES_DESCRIPTOR */

            }	/* end While */
        }
        break; /* end of case PI_PARSE_ES_DESCRIPTOR */

        default:
            break; /* end of default */
        }
    }
    break;

    case TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR : 				/*	0x44	*/
#if 0
        {
            switch ( eDescType)
            {
            case eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR:
            case eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR:
            {
                tCS_SI_TS_Info * NIT_TS_Info;
                U8		ts_desc_num;
                U32		temp_value;

                if (eDescType == eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Actual.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Actual.TSInfo[Current_NIT_Actual.TSNumber -1];
                }
                else if (eDescType == eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Other.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Other.TSInfo[Current_NIT_Other.TSNumber-1];
                }
                else
                {
                    break;
                }

                ts_desc_num = NIT_TS_Info->TSDescNumber;

                if ((NIT_TS_Info->TSDescriptors[ts_desc_num] =
                            (tCS_SI_TSDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_TSDescriptor))) == NULL)
                {
                    break;
                }

                NIT_TS_Info->TSDescNumber++;
                NIT_TS_Info->TSDescriptors[ts_desc_num]->TSDescType = TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR;

                temp_value = ((*ptrLoop) << 24) + (*(ptrLoop + 1) << 16) + (*(ptrLoop + 2) << 8) + (*(ptrLoop + 3));
                NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.Terrestrial.Frequency10hz = temp_value;

                temp_value = ((*(ptrLoop + 4)) >> 5) &0x7;

                switch (temp_value & 0x7)
                {
                case 0x0:
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.Terrestrial.Bandwidth =  8;
                    break;
                case 0x1:
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.Terrestrial.Bandwidth =  7;
                    break;
                default:
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.Terrestrial.Bandwidth =  8;
                    break;
                }

                /* 目前不需要其他信息*/
            }
            break; /* end of case PI_PARSE_NIT_DESCRIPTOR */
            default:
                break;

            }
        }
#endif
        break;

    case MULTILINGUAL_NETWORK_NAME_DESCRIPTOR : 			/*	0x5b	*/
    {
    }
    break;

    case MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR : 			/*	0x5c	*/
    {
    }
    break;

    case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR : 			/*	0x5d	*/
#if 0
        {
            switch (eDescType)
            {
            case eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR:
            case eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR:
            {
                tCS_SI_ServiceDescriptor * SDT_Info;
                U8                          index;
                //U32                     country_code =0;

                if (eDescType == eCS_SI_PARSE_SDT_ACTUAL_DESCRIPTOR)
                {
                    if (Current_SDT_Actual.ServiceNumber == 0)
                        break;

                    SDT_Info = Current_SDT_Actual.ServicesInfo[Current_SDT_Actual.ServiceNumber-1];
                }
                else if (eDescType == eCS_SI_PARSE_SDT_OTHER_DESCRIPTOR)
                {
                    if (Current_SDT_Other.ServiceNumber == 0)
                        break;

                    SDT_Info = Current_SDT_Other.ServicesInfo[Current_SDT_Other.ServiceNumber-1];
                }
                else
                {
                    break;
                }

                if (SDT_Info == NULL)
                {
                    break;
                }

                noOfBytesParsed = 0;
                while (noOfBytesParsed < descriptor_length)
                {
                    service_provider_name_length = *(ptrLoop + 3);
                    service_name_length = *(ptrLoop + 4 + service_provider_name_length);

                    if (GetFreeServiceName(SDT_Info, &index) == eCS_SI_OK)
                    {
                        memset(&(SDT_Info->Multi_ServiceName[index]), 0, sizeof(tCS_DB_ServiceName_t));

                        SDT_Info->Multi_ServiceName[index].Language_Trigram[0] = ptrLoop[0];
                        SDT_Info->Multi_ServiceName[index].Language_Trigram[1] = ptrLoop[1];
                        SDT_Info->Multi_ServiceName[index].Language_Trigram[2] = ptrLoop[2];
                        SDT_Info->Multi_ServiceName[index].Language_Trigram[3] = 0;

                        StringIsEmpty = TRUE;
                        if ( service_name_length != 0 )
                        {
                            for (i=0 ; i < service_name_length && StringIsEmpty ; i++ )
                            {
                                if ( ((char*)(ptrLoop + 5 + service_provider_name_length))[i] !=' ' )
                                {
                                    StringIsEmpty = FALSE;
                                }
                            }
                        }

                        if ( !StringIsEmpty )
                        {
                            noOfBytesToCopy = service_name_length > kCS_DB_MAX_SERVICE_NAME_LENGTH \
                                              ? kCS_DB_MAX_SERVICE_NAME_LENGTH : service_name_length;

                            memcpy(SDT_Info->Multi_ServiceName[index].ServiceName_Text, ptrLoop + 5 + service_provider_name_length,noOfBytesToCopy);

                        }
                        else
                        {
                            strcpy(SDT_Info->Multi_ServiceName[index].ServiceName_Text, "No Name");
                        }

                        SDT_Info->Multi_ServiceName[index].ServiceName_Text[kCS_DB_MAX_SERVICE_NAME_LENGTH-1] = '\0';

                    }

                    noOfBytesParsed += service_name_length + service_provider_name_length + 5;
                    ptrLoop += service_name_length + service_provider_name_length + 5;
                }

            }

        break;

            default:

                break;

            }
        }
#endif
        break;

    case MULTILINGUAL_COMPONENT_DESCRIPTOR :				/*	0x5e	*/
    {
    }
    break;

    case PRIVATE_DATA_SPECIFIER_DESCRIPTOR :				/*	0x5f	*/
    {
    }
    break;

    case SERVICE_MOVE_DESCRIPTOR :							/*	0x60	*/
    {
    }
    break;

    case SHORT_SMOOTHING_BUFFER_DESCRIPTOR :				/*	0x61	*/
    {
    }
    break;

    case FREQUENCY_LIST_DESCRIPTOR :						/*	0x62	*/
    {
    }
    break;

    case PARTIAL_TRANSPORT_STREAM_DESCRIPTOR :				/*	0x63	*/
    {
    }
    break;

    case DATA_BROADCAST_DESCRIPTOR :						/*	0x64	*/
    {
    }
    break;

    case CA_SYSTEM_DESCRIPTOR : 							/*	0x65	*/
    {
    }
    break;

    case DATA_BROADCAST_ID_DESCRIPTOR : 					/*	0x66	*/
    {
    }
    break;

    case AUDIO_AC3_DESCRIPTOR : 							/*	0x6a	*/
    {
        switch ( eDescType)
        {
        case eCS_SI_PARSE_ES_DESCRIPTOR:
        {
            tCS_SI_ESDescriptor	* ptrESDesc;

            if (g_Current_PMT.ESNumber == 0)
                break;

            ptrESDesc = g_Current_PMT.ESDescriptors[g_Current_PMT.ESNumber -1];

            if (ptrESDesc == NULL)
                break;

            ptrESDesc->ESType = eCS_SI_ES_TYPE_AUDIO_AC3;
        }
        break;

        default:

            break; /* end of default */

        }

    }
    break;

    case LOGIC_CHANNEL_NUMBER:					/*		0x83		*/
#if 0
        {
            switch (eDescType)
            {
            case eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR:
            case eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR:
            {
                tCS_SI_TS_Info * NIT_TS_Info;
                U8		ts_desc_num;
                U32		temp_value;
                U8		prog_index;


                if (eDescType == eCS_SI_PARSE_NIT_ACTUAL_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Actual.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Actual.TSInfo[Current_NIT_Actual.TSNumber -1];
                }
                else if (eDescType == eCS_SI_PARSE_NIT_OTHER_TS_DESCRIPTOR)
                {
                    if (Current_NIT_Other.TSNumber == 0)
                        break;

                    NIT_TS_Info = Current_NIT_Other.TSInfo[Current_NIT_Other.TSNumber-1];
                }
                else
                {
                    break;
                }

                ts_desc_num = NIT_TS_Info->TSDescNumber;

                if ((NIT_TS_Info->TSDescriptors[ts_desc_num] =
                            (tCS_SI_TSDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_TSDescriptor))) == NULL)
                {
                    break;
                }

                NIT_TS_Info->TSDescNumber++;
                NIT_TS_Info->TSDescriptors[ts_desc_num]->TSDescType = LOGIC_CHANNEL_NUMBER;

                noOfBytesParsed = 0;
                prog_index = 0;

                while ((noOfBytesParsed < descriptor_length)&&(prog_index < kCS_SI_MAX_NO_OF_SERVICE_PER_TS))
                {

                    temp_value = ((*ptrLoop) << 8) + (*(ptrLoop + 1));
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.LCNDesc.ServiceLCN[prog_index].ServiceID = temp_value;

                    noOfBytesParsed+=2;
                    temp_value =  ((*(ptrLoop + 2))>>7)&0x1;
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.LCNDesc.ServiceLCN[prog_index].Visible_Service_Flag = temp_value;

                    temp_value = (((*(ptrLoop + 2))&0x3f) << 8) + (*(ptrLoop + 3));
                    NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.LCNDesc.ServiceLCN[prog_index].LCN =  (temp_value&0x03ff) |0x8000;

                    noOfBytesParsed+=2;
                    ptrLoop+=4;
                    //printf("LOGIC_CHANNEL_NUMBER ServiceID = 0x%x, LCN = 0x%x\n", NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.LCNDesc.ServiceLCN[prog_index].ServiceID, NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.LCNDesc.ServiceLCN[prog_index].LCN);
                    prog_index++;
                }
                NIT_TS_Info->TSDescriptors[ts_desc_num]->uTSDescriptor.SListDesc.ServiceNumber = prog_index;


            }

            default:
                break;
            }
        }
#endif
        break;

    default:
    {
    }
    break;


    }

    return(descriptor_length + 2); /* 2 bytes : ptrDescriptor + desriptor_length */
}

#define kCS_SI_NO_OF_CRC_DATA_BYTES 4

static U32 Parse_SI_SetAllSectionMask(U8 lastSectionNumber)
{
	U8 index = 0;
         U32 mask = 0;

	for(index = 0; index < (lastSectionNumber+1); index++)
	{
		mask |=	(1 << index);
	}
	return(mask);
}

static U32 Parse_SI_SetSectionMask(U8 SectionNumber)
{
	U32 mask = 0;

         if(SectionNumber>31)
                return(FALSE);

	mask |=	(1 << SectionNumber);
	return(mask);
}

static BOOL Parse_SI_Section_Isrequested(tCS_Parser_SectionNumberMask filter_mask, U8 SectionNumber)
{
        U32   temp = 0;

        if(SectionNumber>31)
                return(FALSE);

        temp = (1<<SectionNumber);
        
	if((filter_mask.filtered & temp) == temp)
		return(FALSE);
	else
		return(TRUE);
}


tCS_SI_Report CS_Parse_parsePMTsection(stMSG *msgReceived)
{
    tCS_SI_Report		report = eCS_SI_OK;
    U32 mask;
    //U32 iIndex,size,indexToWrite,lengthToWrite;
    U8 *section,*ptrLoop,*ptrDescriptor;
    U16 program_number,iTsInfoLength;
    U16 PCR_PID,program_info_length,ES_info_length;
    U16 noOfBytesParsed,noOfDataBytes,noOf2LoopBytesParsed;
    U8 version_number,section_number,stream_type;
    U8 current_next_indicator,last_section_number;


    section = msgReceived->buf;


    program_number = (*(section + 3) << 8) + *(section + 4);
#if 0

    if ((program_number != Current_Program.ServiceID))
    {
        return(eCS_SI_ERROR);
    }
#endif

    version_number =  ((*(section + 5) >> 1) & 0x1F);

    Parser_printf("PMT_Version = %d\n", g_Current_PMT.PMT_Version);
    if (g_Current_PMT.PMT_Version != version_number)
    {
        CS_Parser_Clear_PMT_Info();
    }

    CSOS_WaitSemaphore(g_sem_PMT_Access);

    g_Current_PMT.PMT_Version = version_number;

    current_next_indicator = (*(section + 5) & 0x1);

    section_number = *(section + 6);

    last_section_number = *(section + 7);

    PCR_PID = ((*(section + 8) & 0x1F ) << 8) + *(section + 9);

    program_info_length = ((*(section + 10) & 0xF ) << 8) + *(section + 11);

    ptrDescriptor = section + 12;

    g_Current_PMT.PCR_PID = PCR_PID;
    g_Current_PMT.ServiceID = program_number;


    if (g_SectionNumberMask_PMT.toFilter == 0)
    {
        g_SectionNumberMask_PMT.toFilter = Parse_SI_SetAllSectionMask(last_section_number);
    }

    if (!Parse_SI_Section_Isrequested(g_SectionNumberMask_PMT, section_number))
    {
        report = eCS_SI_DISCARD;
        g_SectionNumberMask_PMT.toFilter &= ~(g_SectionNumberMask_PMT.filtered);
        CSOS_SignalSemaphore(g_sem_PMT_Access);

        return(report);
    }

    g_SectionNumberMask_PMT.filtered |= Parse_SI_SetSectionMask(section_number);

    g_SectionNumberMask_PMT.toFilter &= ~(g_SectionNumberMask_PMT.filtered);

    mask = g_SectionNumberMask_PMT.toFilter;


    noOfBytesParsed = noOfDataBytes = 0;

    while ( noOfBytesParsed < program_info_length )
    {
        noOfDataBytes = CS_Parser_parseDescriptors(ptrDescriptor, eCS_SI_PARSE_PMT_DESCRIPTOR);
        ptrDescriptor += noOfDataBytes;
        noOfBytesParsed += noOfDataBytes;
    }

    noOfBytesParsed = 0;
    ptrLoop = section + 12 + program_info_length;

    iTsInfoLength = ((msgReceived->length) - kCS_SI_NO_OF_CRC_DATA_BYTES );

    while (ptrLoop < (section + iTsInfoLength) )
    {
        U8	es_index;

        es_index = g_Current_PMT.ESNumber;

        if (es_index < kCS_SI_MAX_NO_OF_ES_DESCRIPTORS_PER_SERVICE)
        {
            if ((g_Current_PMT.ESDescriptors[es_index] =
                        (tCS_SI_ESDescriptor *)CSOS_AllocateMemory(NULL, sizeof(tCS_SI_ESDescriptor))) == NULL)
            {
                report = eCS_SI_ERROR;
                break;
            }
            else
            {
                g_Current_PMT.ESNumber++;
                memset(g_Current_PMT.ESDescriptors[es_index],0,sizeof(tCS_SI_ESDescriptor));
                stream_type = *ptrLoop;

                ES_info_length = ((*(ptrLoop + 3) & 0xF ) << 8) + *(ptrLoop + 4);

                g_Current_PMT.ESDescriptors[es_index]->ESPID = ((*(ptrLoop + 1) & 0x1F ) << 8) + *(ptrLoop + 2);

                //printf("steam pid = 0x%x, stream type = 0x%x\n", g_Current_PMT.ESDescriptors[es_index]->ESPID, stream_type);

                switch (stream_type)
                {
                case 0x1:
                case 0x2:
                {
                    g_Current_PMT.ESDescriptors[es_index]->ESType = eCS_SI_ES_TYPE_VIDEO;

                }
                break;

                case 0x3:
                case 0x4:
                {
                    g_Current_PMT.ESDescriptors[es_index]->ESType = eCS_SI_ES_TYPE_AUDIO;
                }
                break;

                case 0x5:
                case 0x6:
                {
                    g_Current_PMT.ESDescriptors[es_index]->ESType = eCS_SI_ES_TYPE_UNKNOWN;
                }
                break;
                case 0x1B:
                {
                    g_Current_PMT.ESDescriptors[es_index]->ESType = eCS_SI_ES_TYPE_VIDEO_MPEG4;
                }
                break;
                case 0x81: /* Add By River 11.22.2007 */
                {
                    g_Current_PMT.ESDescriptors[es_index]->ESType = eCS_SI_ES_TYPE_AUDIO_AC3;
                }
                break;
                default:
                {
                    CSOS_DeallocateMemory(NULL, g_Current_PMT.ESDescriptors[es_index]);
                    g_Current_PMT.ESDescriptors[es_index] = NULL;
                    g_Current_PMT.ESNumber--;
                    noOfBytesParsed = ES_info_length + 5;
                    ptrLoop += noOfBytesParsed;
                    continue;

                }
                break;
                }

                ptrDescriptor = ptrLoop + 5;
                noOf2LoopBytesParsed = noOfDataBytes = 0;
                while ( noOf2LoopBytesParsed < ES_info_length )
                {
                    noOfDataBytes = CS_Parser_parseDescriptors(ptrDescriptor, eCS_SI_PARSE_ES_DESCRIPTOR);
                    ptrDescriptor += noOfDataBytes;
                    noOf2LoopBytesParsed += noOfDataBytes;
                }

                noOfBytesParsed = ES_info_length + 5;
                ptrLoop += noOfBytesParsed;
            }
        }
        else
        {
            break;
        }

    }

    CSOS_SignalSemaphore(g_sem_PMT_Access);

    report = eCS_SI_OK;

    return(report);
}

#if 0

U8 GetServiceInfo(U16 service_id, tCS_DB_ServiceData *service_data)
{
    U32 i;
    U16 PMTPid;
    BOOL bServiceExist = FALSE;
    stMSG msg;
    stTS_SDT sdt;
    U32 index;
    BOOL found_vid = FALSE;
    BOOL found_aud = FALSE;

    CSOS_WaitSemaphore(g_Parser_Access);

    printf("total_program_num is %d\n", total_program_num);
    for (i=0; i<total_program_num; i++)
    {
        printf("program %d service id is %d, pmt pid is %d\n", i, pmt_pid[i].program_number, pmt_pid[i].PMT_PID);
        if (pmt_pid[i].program_number == service_id)
        {
            PMTPid = pmt_pid[i].PMT_PID;
            bServiceExist = TRUE;
            break;
        }
    }
    
    if (bServiceExist)
    {
        for (i=0; i<g_saved_section_count; i++)
        {
            if (g_save_section[i].pid == PMTPid)
            {
                msg.buf = g_save_section[i].section_data.section_data;
                msg.length = g_save_section[i].section_data.section_length;
                CS_Parse_parsePMTsection(&msg);
            }

            if (g_save_section[i].table_id == SDT_TABLE_ID_ACTUAL)
            {
                ParseSDTSection(&sdt, g_save_section[i].section_data.section_data);
            }
        }

        service_data->sCS_DB_ServiceID = service_id;
        service_data->sCS_DB_PcrPid = g_Current_PMT.PCR_PID;
        service_data->sCS_DB_ServiceScramble = g_Current_PMT.Scramble;
        service_data->sCS_DB_VideoPid = kDB_DEMUX_INVAILD_PID;
        service_data->sCS_DB_AudioPid = kDB_DEMUX_INVAILD_PID;

        for ( index = 0; index < g_Current_PMT.ESNumber; index++ )
        {
            if ( g_Current_PMT.ESDescriptors[index] != NULL )
            {
                //printf("ts pmt es pid = 0x%x, type = %d\n", g_Current_PMT.ESDescriptors[index]->ESPID, g_Current_PMT.ESDescriptors[index]->ESType);
                switch ( g_Current_PMT.ESDescriptors[index]->ESType )
                {
                case eCS_SI_ES_TYPE_VIDEO:
                    if (!found_vid)
                    {
                        service_data->sCS_DB_VideoPid = g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_VideoType = eCS_DB_VIDEO_MPEG2;
                        found_vid = TRUE;
                    }
                    break;

                case  eCS_SI_ES_TYPE_VIDEO_MPEG4:
                    if (!found_vid)
                    {
                        service_data->sCS_DB_VideoPid = g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_VideoType = eCS_DB_VIDEO_H264;
                        found_vid = TRUE;
                    }
                    break;

                case eCS_SI_ES_TYPE_AUDIO:
                    if (!found_aud)
                    {
                        service_data->sCS_DB_AudioPid= g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_AudioType= eCS_DB_AUDIO_MPEG2;
                        found_aud = TRUE;
                    }
                    break;

                case eCS_SI_ES_TYPE_AUDIO_AC3:
                    if (!found_aud)
                    {
                        service_data->sCS_DB_AudioPid = g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_AudioType = eCS_DB_AUDIO_AC3;
                        found_aud = TRUE;
                    }
                    break;

                 case eCS_SI_ES_TYPE_AUDIO_AAC:
                    if (!found_aud)
                    {
                        service_data->sCS_DB_AudioPid= g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_AudioType= eCS_DB_AUDIO_AAC;
                        found_aud = TRUE;
                    }
                    break;

                 case eCS_SI_ES_TYPE_AUDIO_LATM:
                    if (!found_aud)
                    {
                        service_data->sCS_DB_AudioPid= g_Current_PMT.ESDescriptors[index]->ESPID;
                        service_data->sCS_DB_AudioType= eCS_DB_AUDIO_LATM;
                        found_aud = TRUE;
                    }
                    break;
                default:

                    break;
                }
            }

        }

        if ( service_data->sCS_DB_VideoPid != kDB_DEMUX_INVAILD_PID )
        {
            service_data->sCS_DB_ServiceType = eCS_DB_TV_SERVICE;
        }
        else
        {
            service_data->sCS_DB_ServiceType = eCS_DB_RADIO_SERVICE;
        }

        // get service name
        for (index = 0; index < total_sdt_info_num; index++)
        {
            if (sdt_info[index].service_id == service_id)
            {
                if (sdt_info[index].multi_service_name_des_num != 0 &&
                        sdt_info[index].multi_service_name_descriptor[0]->multi_service_name_des_info_num != 0 &&
                        sdt_info[index].multi_service_name_descriptor[0]->multi_service_name_des_info[0]->service_provider_name_length != 0)
                    memcpy(service_data->sCS_DB_Multi_ServiceName,
                           sdt_info[index].multi_service_name_descriptor[0]->multi_service_name_des_info[0]->service_provider_name_char,
                           sdt_info[index].multi_service_name_descriptor[0]->multi_service_name_des_info[0]->service_provider_name_length);
                else if ((sdt_info[index].service_des_num != 0 && sdt_info[index].service_descriptor[0]->service_name_length != 0))
                    memcpy(service_data->sCS_DB_Multi_ServiceName, sdt_info[index].service_descriptor[0]->service_name_char,
                           sdt_info[index].service_descriptor[0]->service_name_length);
            }
        }
    }
    else
    {
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    CSOS_SignalSemaphore(g_Parser_Access);
    return CS_PARSER_NO_ERROR;
}
#endif

BOOL IsLeapYear(U16 year)
{
    if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

U8 GetLocalTime(void)
{
    int i;
    BOOL bTDTExist = FALSE;
    BOOL bTOTExist = FALSE;
    stTS_TDT tdt;
    stTS_TOT tot;

    CSOS_WaitSemaphore(g_Parser_Access);

    for (i=0; i<g_saved_section_count; i++)
    {
        if (g_save_section[i].table_id == TDT_TABLE_ID)
        {
            ParseTDTSection(&tdt, g_save_section[i].section_data.section_data);
            bTDTExist = TRUE;
        }

        if (!bTDTExist)
        {
            if (g_save_section[i].table_id == TOT_TABLE_ID)
            {
                ParseTOTSection(&tot, g_save_section[i].section_data.section_data);
                bTOTExist = TRUE;
            }
        }
    }

    if (bTDTExist)
    {
        g_local_date = MJD2YMD(tdt.UTC_time_date);
        g_local_time = UTC2HM((U32)tdt.UTC_time_time1 << 8 | tdt.UTC_time_time2);
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_NO_ERROR;
    }
    else if (bTOTExist)
    {
        g_local_date = MJD2YMD(tot.UTC_time_date);
        g_local_time = UTC2HM((U32)tot.UTC_time_time1 << 8 | tot.UTC_time_time2);

        if (tot.local_time_offset_descriptor_num != 0)
        {
            if (tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info_num != 0)
            {
                U8 tmp_hour = ((tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset & 0xf000) >> 12) * 10 +
                              (tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset & 0x0f00 >> 8);
                U8 tmp_minute = ((tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset & 0x00f0) >> 4) * 10 +
                                (tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset & 0x000f);

                if (tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset_polarity == 1)
                {
                    g_local_time.minute += tmp_minute;
                    if (g_local_time.minute >= 60)
                    {
                        g_local_time.minute -= 60;
                        g_local_time.hour++;
                        g_local_time.hour += tmp_hour;

                        if (g_local_time.hour >= 24)
                        {
                            g_local_time.hour -= 24;
                            g_local_date.day++;

                            if ((g_local_date.day > 31) && (g_local_date.month == 1 || g_local_date.month == 3 || g_local_date.month == 5 ||
                                                            g_local_date.month == 7 || g_local_date.month == 8 || g_local_date.month == 10 || g_local_date.month == 12))
                            {
                                g_local_date.day -= 31;
                                g_local_date.month++;
                                if (g_local_date.month > 12)
                                {
                                    g_local_date.month -= 12;
                                    g_local_date.year++;
                                }
                            }

                            if ((g_local_date.day > 30) && (g_local_date.month == 4 || g_local_date.month == 6 || g_local_date.month == 9 || g_local_date.month == 11))
                            {
                                g_local_date.day -= 30;
                                g_local_date.month++;
                            }

                            if (g_local_date.month == 2)
                            {
                                if (IsLeapYear(g_local_date.year))
                                {
                                    if (g_local_date.day > 29)
                                    {
                                        g_local_date.day -= 29;
                                        g_local_date.month++;
                                    }
                                }
                                else
                                {
                                    if (g_local_date.day > 28)
                                    {
                                        g_local_date.day -= 28;
                                        g_local_date.month++;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (tot.local_time_offset_descriptor[0]->local_time_offset_descriptor_info[0]->local_time_offset_polarity == 0)
                {
                    g_local_time.minute -= tmp_minute;
                    if (g_local_time.minute >= 60)
                    {
                        g_local_time.minute += 60;
                        g_local_time.hour--;

                        if (g_local_time.hour >= 24)
                        {
                            g_local_time.hour += 24;
                            g_local_date.day--;
                            g_local_time.hour -= tmp_hour;
                            
                            if ((g_local_date.day > 31) && (g_local_date.month == 1 || g_local_date.month == 3 || g_local_date.month == 5 ||
                                                            g_local_date.month == 7 || g_local_date.month == 8 || g_local_date.month == 10 || g_local_date.month == 12))
                            {
                                g_local_date.day += 31;
                                g_local_date.month--;

                                if (g_local_date.month == 0)
                                {
                                    g_local_date.month = 12;
                                    g_local_date.year--;
                                }
                            }

                            if ((g_local_date.day > 30) && (g_local_date.month == 4 || g_local_date.month == 6 || g_local_date.month == 9 || g_local_date.month == 11))
                            {
                                g_local_date.day += 30;
                                g_local_date.month--;
                            }

                            if (g_local_date.month == 2)
                            {
                                if (IsLeapYear(g_local_date.year))
                                {
                                    if (g_local_date.day > 29)
                                    {
                                        g_local_date.day += 29;
                                        g_local_date.month--;
                                    }
                                }
                                else
                                {
                                    if (g_local_date.day > 28)
                                    {
                                        g_local_date.day += 28;
                                        g_local_date.month--;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_NO_ERROR;
    }
    else
    {
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }
}


/*^^***********************************************************************************************
 * Function Name:
 * Author:
 * Date:
 * Function:
 * Input Param:
 *     duration is A 24-bit field containing the duration of the event in hours, minutes, seconds.
 * Output Param:
 * Return:
 * Modify:
 ***********************************************************************************************^^*/
U8 TimePlus(stDT_Date start_date, stDT_Time start_time, U32 duration, stDT_Date *result_date, stDT_Time *result_time)
{
    U8 tmp_hour = (U8)((duration & 0x00f00000) >> 20) * 10 + (U8)((duration & 0x000f0000) >> 16);
    U8 tmp_minute = (U8)((duration & 0x0000f000) >> 12) * 10 + (U8)((duration & 0x00000f00) >> 8);
    U8 tmp_second = (U8)((duration & 0x000000f0) >> 4) * 10 + (U8)(duration & 0x0000000f);

    if (result_date == NULL || result_time == NULL)
        return CS_PARSER_ERROR;

    memcpy(result_date, &start_date, sizeof(stDT_Date));
    memcpy(result_time, &start_time, sizeof(stDT_Time));

    result_time->second += tmp_second;
    result_time->minute += tmp_minute;
    result_time->hour += tmp_hour;
    
    while (result_time->second >= 60)
    {
        result_time->second -= 60;
        result_time->minute++;
    }

    while (result_time->minute >= 60)
    {
        result_time->minute -= 60;
        result_time->hour++;
    }

    while (result_time->hour >= 24)
    {
        result_time->hour -= 24;
        result_date->day++;
    }
    
    if (result_date->day > 31 && (result_date->month == 1 || result_date->month == 3 || result_date->month == 5 || 
        result_date->month == 7 || result_date->month == 8 || result_date->month == 10 || result_date->month == 12))
    {
        result_date->day -= 31;
        result_date->month++;
        if (result_date->month > 12)
        {
            result_date->month -= 12;
            result_date->year++;
        }
    }
    else if (result_date->day > 30 && (result_date->month == 4 || result_date->month == 6 || result_date->month == 9 ||
        result_date->month == 11))
    {
        result_date->day -= 30;
        result_date->month++;
    }
    else if (result_date->month == 2)
    {
        if (result_date->day > 29 && IsLeapYear(result_date->year))
        {
            result_date->day -= 29;
            result_date->month++;
        }
        else if (result_date->day > 28 && !IsLeapYear(result_date->year))
        {
            result_date->day -= 28;
            result_date->month++;
        }
    }
    
    return CS_PARSER_NO_ERROR;
}

U8 TimeSubtract(stDT_Date start_date, stDT_Time start_time, U32 duration, stDT_Date *result_date, stDT_Time *result_time)
{
    U8 tmp_hour = (U8)((duration & 0x00f00000) >> 20) * 10 + (U8)((duration & 0x000f0000) >> 16);
    U8 tmp_minute = (U8)((duration & 0x0000f000) >> 12) * 10 + (U8)((duration & 0x00000f00) >> 8);
    U8 tmp_second = (U8)((duration & 0x000000f0) >> 4) * 10 + (U8)(duration & 0x0000000f);
    
    if (result_date == NULL || result_time == NULL)
        return CS_PARSER_NO_ERROR;

    memcpy(result_date, &start_date, sizeof(stDT_Date));
    memcpy(result_time, &start_time, sizeof(stDT_Time));

    result_time->second -= tmp_second;
    result_time->minute -= tmp_minute;
    result_time->hour -= tmp_hour;
    
    while (result_time->second >= 60)
    {
        result_time->second -= 60;
        result_time->minute--;
    }
    
    while (result_time->minute >= 60)
    {
        result_time->minute -= 60;
        result_time->hour--;
    }
    
    while (result_time->hour >= 24)
    {
        result_time->hour -= 24;
        result_date->day--;
    }
    
    if (result_date->day > 31 && (result_date->month == 1 || result_date->month == 3 || result_date->month == 5 || 
        result_date->month == 7 || result_date->month == 8 || result_date->month == 10 || result_date->month == 12))
    {
        result_date->day -= 31;
        result_date->month--;
        if (result_date->month > 12)
        {
            result_date->month -= 12;
            result_date->year--;
        }
    }
    else if (result_date->day > 30 && (result_date->month == 4 || result_date->month == 6 || result_date->month == 9 ||
        result_date->month == 11))
    {
        result_date->day -= 30;
        result_date->month--;
    }
    else if (result_date->month == 2)
    {
        if (result_date->day > 29 && IsLeapYear(result_date->year))
        {
            result_date->day -= 29;
            result_date->month--;
        }
        else if (result_date->day > 28 && !IsLeapYear(result_date->year))
        {
            result_date->day -= 28;
            result_date->month--;
        }
    }

    return CS_PARSER_NO_ERROR;
}

S8 CompareTime(stDT_Date date1, stDT_Time time1, stDT_Date date2, stDT_Time time2)
{
    if (date1.year < date2.year)
    {
        return -1;
    }
    else if (date1.year > date2.year)
    {
        return 1;
    }
    else 
    {
        if (date1.month < date2.month)
        {
            return -1;
        }
        else if (date1.month > date2.month)
        {
            return 1;
        }
        else 
        {
            if (date1.day < date2.day)
            {
                return -1;
            }
            else if (date1.day > date2.day)
            {
                return 1;
            }
            else 
            {
                if (time1.hour < time2.hour)
                {
                    return -1;
                }
                else if(time1.hour > time2.hour)
                {
                    return 1;
                }
                else
                {
                    if (time1.minute < time2.minute)
                    {
                        return -1;
                    }
                    else if (time1.minute > time2.minute)
                    {
                        return 1;
                    }
                    else 
                    {
                        if (time1.second < time2.second)
                        {
                            return -1;
                        }
                        else if (time1.second > time2.second)
                        {
                            return 1;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                }
            }
        }
    }
}

#if 0
U8 GetPF(U16 service_id, tCS_EIT_Event_t *eit_event_present, tCS_EIT_Event_t *eit_event_follow)
{
    S8 result1, result2;
    U8 err;
    U32 i;
    U16 PMTPid;
    BOOL bServiceExist = FALSE;
    BOOL bEITExist = FALSE;
    stTS_EIT eit[2];
    U8 eit_num = 0;
    stDT_Date date1_start;
    stDT_Date date2_start;
    stDT_Time time1_start;
    stDT_Time time2_start;
    stDT_Date date1_end;
    stDT_Date date2_end;
    stDT_Time time1_end;
    stDT_Time time2_end;

    if (eit_event_present == NULL || eit_event_follow == NULL)
        return CS_PARSER_ERROR;

    CSOS_WaitSemaphore(g_Parser_Access);

    memset(&date1_start, 0, sizeof(stDT_Date));
    memset(&date2_start, 0, sizeof(stDT_Date));
    memset(&time1_start, 0, sizeof(stDT_Time));
    memset(&time2_start, 0, sizeof(stDT_Time));
    memset(&date1_end, 0, sizeof(stDT_Date));
    memset(&date2_end, 0, sizeof(stDT_Date));
    memset(&time1_end, 0, sizeof(stDT_Time));
    memset(&time2_end, 0, sizeof(stDT_Time));

    for (i=0; i<total_program_num; i++)
    {
        if (pmt_pid[i].program_number == service_id)
        {
            PMTPid = pmt_pid[i].PMT_PID;
            bServiceExist = TRUE;
            break;
        }
    }

    if (bServiceExist)
    {
        //printf("service exist!\n");
        for (i=0; i<g_saved_section_count; i++)
        {
            //printf("0x%x, 0x%x, 0x%x\n", *g_save_section[i].section_data.section_data, g_save_section[i].section_data.section_data[6], g_save_section[i].section_data.section_data[7]);
            if (g_save_section[i].table_id == EIT_TABLE_ID_ACTUAL)
            {
                ParseEITSection(&eit[eit_num], g_save_section[i].section_data.section_data);
                //printf("eit.service_id is 0x%x\n", eit[eit_num].service_id);

                if (eit[eit_num].service_id == service_id)
                {
                    bEITExist= TRUE;
                    eit_num++;
                }
            }
        }

        if (bEITExist)
        {
            //printf("eit found!\n");
            if (GetLocalTime() == CS_PARSER_ERROR)
            {
                printf("GetLocalTime Failed!\n");
                CSOS_SignalSemaphore(g_Parser_Access);
                return CS_PARSER_ERROR;
            }
            else
            {
                //printf("local date is %04d/%02d/%02d\n", g_local_date.year, g_local_date.month, g_local_date.day);
                //printf("local time is %02d:%02d:%02d\n", g_local_time.hour, g_local_time.minute, g_local_time.second);
            }

            if (eit_num == 2)   // pf
            {
                if (eit[0].eit_info_num == 0)
                {
                    printf("eit info num is 0!\n");
                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_ERROR;
                }
                else
                {
                    date1_start = MJD2YMD(eit[0].eit_info[0]->start_time_date);
                    time1_start = UTC2HM((U32)eit[0].eit_info[0]->start_time_time1 << 8 | eit[0].eit_info[0]->start_time_time2);
                    err = TimePlus(date1_start, time1_start, (U32)eit[0].eit_info[0]->duration1 << 8 | eit[0].eit_info[0]->duration2,
                        &date1_end, &time1_end);

                    if (err == CS_PARSER_ERROR)
                    {
                        printf("TimePlus Failed!\n");
                        CSOS_SignalSemaphore(g_Parser_Access);
                        return err;
                    }
#if 0
                    stDT_Date test_date;
                    stDT_Time test_time;
                    U32 test_duration;
                    stDT_Date r_date;
                    stDT_Time r_time;
                    test_date.year = 2000;
                    test_date.month = 2;
                    test_date.day = 29;
                    test_time.hour = 23;
                    test_time.minute = 59;
                    test_time.second= 59;
                    test_duration = 0x00595959;
                    TimeSubtract(test_date, test_time, test_duration, &r_date, &r_time);
                    printf("test result is %04d/%02d/%02d %02d:%02d:%02d!\n", r_date.year, r_date.month, r_date.day,
                        r_time.hour, r_time.minute, r_time.second);
#endif
                }

                /*
                if (eit[1].eit_info_num == 0)
                {
                    printf("eit info num is 0!\n");
                    return CS_PARSER_ERROR;
                }
                else
                {
                    date2_start = MJD2YMD(eit[1].eit_info[0]->start_time_date);
                    time2_start = UTC2HM((U32)eit[1].eit_info[0]->start_time_time1 << 8 | eit[1].eit_info[0]->start_time_time2);
                    err = TimePlus(date2_start, time2_start, (U32)eit[1].eit_info[0]->duration1 << 8 | eit[1].eit_info[0]->duration2,
                        &date2_end, &time2_end);
                    if (err == CS_PARSER_ERROR)
                        return err;
                }
                */
                
                result1 = CompareTime(date1_start, time1_start, g_local_date, g_local_time);
                result2 = CompareTime(date1_end, time1_end, g_local_date, g_local_time);

                if (result1 <= 0 && result2 >= 0)
                {                
                    eit_event_present->event_id = eit[0].eit_info[0]->event_id;
                    eit_event_present->start_date_mjd = eit[0].eit_info[0]->start_time_date;
                    eit_event_present->start_time_utc = eit[0].eit_info[0]->start_time_time1;
                    eit_event_present->duration_utc = eit[0].eit_info[0]->duration1;
                    eit_event_present->free_ca = eit[0].eit_info[0]->free_CA_mode;
                    if (eit[0].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_present->event_name, eit[0].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_present->event_name[eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[0].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_present->short_description, eit[0].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_present->short_description[eit[0].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }
                    }

                    if (eit[0].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_present->extended_description, eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_present->extended_description[eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';
                            }
                        }
                    }

                    eit_event_follow->event_id = eit[1].eit_info[0]->event_id;
                    eit_event_follow->start_date_mjd = eit[1].eit_info[0]->start_time_date;
                    eit_event_follow->start_time_utc = eit[1].eit_info[0]->start_time_time1;
                    eit_event_follow->duration_utc = eit[1].eit_info[0]->duration1;
                    eit_event_follow->free_ca = eit[1].eit_info[0]->free_CA_mode;
                    if (eit[1].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_follow->event_name, eit[1].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_follow->event_name[eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[1].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_follow->short_description, eit[1].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[1].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_follow->short_description[eit[1].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }
                    }

                    if (eit[1].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_follow->extended_description, eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_follow->extended_description[eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';
                            }
                        }
                    }
                }
                else if(result1 >=0 && result2 >= 0)
                {
                    eit_event_follow->event_id = eit[0].eit_info[0]->event_id;
                    eit_event_follow->start_date_mjd = eit[0].eit_info[0]->start_time_date;
                    eit_event_follow->start_time_utc = eit[0].eit_info[0]->start_time_time1;
                    eit_event_follow->duration_utc = eit[0].eit_info[0]->duration1;
                    eit_event_follow->free_ca = eit[0].eit_info[0]->free_CA_mode;
                    if (eit[0].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_follow->event_name, eit[0].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_follow->event_name[eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[0].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_follow->short_description, eit[0].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_follow->short_description[eit[0].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }
                    }

                    if (eit[0].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_follow->extended_description, eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_follow->extended_description[eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';
                            }
                        }
                    }

                    eit_event_present->event_id = eit[1].eit_info[0]->event_id;
                    eit_event_present->start_date_mjd = eit[1].eit_info[0]->start_time_date;
                    eit_event_present->start_time_utc = eit[1].eit_info[0]->start_time_time1;
                    eit_event_present->duration_utc = eit[1].eit_info[0]->duration1;
                    eit_event_present->free_ca = eit[1].eit_info[0]->free_CA_mode;
                    if (eit[1].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_present->event_name, eit[1].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_present->event_name[eit[1].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[1].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_present->short_description, eit[1].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[1].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_present->short_description[eit[1].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }
                    }

                    if (eit[1].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_present->extended_description, eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_present->extended_description[eit[1].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';
                            }
                        }
                    }
                }
                else
                {
                    printf("bad time\n");  
                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_ERROR;
                }
            }
            else if (eit_num == 1)
            {
                if (eit[0].eit_info_num == 0)
                {
                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_ERROR;
                }
                else
                {
                    date1_start = MJD2YMD(eit[0].eit_info[0]->start_time_date);
                    time1_start = UTC2HM((U32)eit[0].eit_info[0]->start_time_time1 << 8 | eit[0].eit_info[0]->start_time_time2);
                    err = TimePlus(date1_start, time1_start, (U32)eit[0].eit_info[0]->duration1 << 8 | eit[0].eit_info[0]->duration2,
                        &date1_end, &time1_end);
                    if (err == CS_PARSER_ERROR)
                    {
                        CSOS_SignalSemaphore(g_Parser_Access);
                        return err;
                    }
                }

                result1 = CompareTime(date1_start, time1_start, g_local_date, g_local_time);
                result2 = CompareTime(date1_end, time1_end, g_local_date, g_local_time);

                if (result1 <= 0 && result2 >= 0)   // present
                {
                    eit_event_present->event_id = eit[0].eit_info[0]->event_id;
                    eit_event_present->start_date_mjd = eit[0].eit_info[0]->start_time_date;
                    eit_event_present->start_time_utc = eit[0].eit_info[0]->start_time_time1;
                    eit_event_present->duration_utc = eit[0].eit_info[0]->duration1;
                    eit_event_present->free_ca = eit[0].eit_info[0]->free_CA_mode;
                    if (eit[0].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_present->event_name, eit[0].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_present->event_name[eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[0].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_present->short_description, eit[0].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_present->short_description[eit[0].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }

                    }

                    if (eit[0].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_present->extended_description, eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_present->extended_description[eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';

                            }
                        }
                    }

                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_GETPF_RETURN_P;
                }
                else if (result1 >=0 && result2 >=0)
                {
                    eit_event_follow->event_id = eit[0].eit_info[0]->event_id;
                    eit_event_follow->start_date_mjd = eit[0].eit_info[0]->start_time_date;
                    eit_event_follow->start_time_utc = eit[0].eit_info[0]->start_time_time1;
                    eit_event_follow->duration_utc = eit[0].eit_info[0]->duration1;
                    eit_event_follow->free_ca = eit[0].eit_info[0]->free_CA_mode;
                    if (eit[0].eit_info[0]->short_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length != 0)
                        {
                            memcpy(eit_event_follow->event_name, eit[0].eit_info[0]->short_event_descriptor[0]->event_name_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length);
                            eit_event_follow->event_name[eit[0].eit_info[0]->short_event_descriptor[0]->event_name_length] = '\0';
                        }

                        if (eit[0].eit_info[0]->short_event_descriptor[0]->text_length != 0)
                        {
                            memcpy(eit_event_follow->short_description, eit[0].eit_info[0]->short_event_descriptor[0]->text_char,
                                   sizeof(U8) * eit[0].eit_info[0]->short_event_descriptor[0]->text_length);
                            eit_event_follow->short_description[eit[0].eit_info[0]->short_event_descriptor[0]->text_length] = '\0';
                        }

                    }

                    if (eit[0].eit_info[0]->extended_event_descriptor_num != 0)
                    {
                        if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor_num != 0)
                        {
                            if (eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length != 0)
                            {
                                memcpy(eit_event_follow->extended_description, eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_char,
                                       sizeof(U8) * eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length);
                                eit_event_follow->extended_description[eit[0].eit_info[0]->extended_event_descriptor[0]->extended_event_descriptor[0]->item_description_length] = '\0';

                            }
                        }
                    }

                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_GETPF_RETURN_F;
                }
                else 
                {
                    printf("eit info num is 0!\n");
                    CSOS_SignalSemaphore(g_Parser_Access);
                    return CS_PARSER_ERROR;
                }
            }
            else
            {
                printf("parse eit section get nothing!\n");
                CSOS_SignalSemaphore(g_Parser_Access);
                return CS_PARSER_ERROR;
            }
        }
        else
        {
            printf("no eit table!\n");
            CSOS_SignalSemaphore(g_Parser_Access);
            return CS_PARSER_ERROR;
        }
    }
    else
    {
        CSOS_SignalSemaphore(g_Parser_Access);
        return CS_PARSER_ERROR;
    }

    CSOS_SignalSemaphore(g_Parser_Access);
    return CS_PARSER_NO_ERROR;
}
#endif

