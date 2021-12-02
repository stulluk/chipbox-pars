/****************************************************************
*
* FILENAME
*	table_def.h
*
* PURPOSE 
*	Table Process API Function Define
*	
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2008.07.10           KB Kim
*
****************************************************************/
#ifndef __TABLE_DEF_H
#define __TABLE_DEF_H
/****************************************************************
 *                       Include files                          *
 ****************************************************************/

/****************************************************************
 *                          define                              *
 ****************************************************************/
// #define USA_SI_TABLE_SUPPORT
#define MAX_NUMBER_OF_DEMUX              1

#define MAX_NAME_LEN                     30
#define MAX_NUMBER_OF_ES                 50
#define INVALID_U_INT                    0xFFFFFFFF

#define TABLE_INFO_NUMBER                32
#define MAX_PMT_LENGTH                   1024

#define INVALID_VERSION_NUMBER           32
#define VERSION_NUMBER_MASK              0x3E


/****************************************************************
 *	Type Define                                                 *
 ****************************************************************/
typedef enum
{
	DATA_SERVICE = 0,
	TV_SERVICE,
	RADIO_SERVICE,
	HDTV_SERVICE,
	SERVICE_MAX
} SiServiceType;

typedef enum
{
    AUDIO_TYPE_MPEG = 0,
    AUDIO_TYPE_AC3,
    AUDIO_TYPE_AAC,
    AUDIO_TYPE_LATM,
    AUDIO_TYPE_UNKNOWN,
	AUDIO_TYPE_MAX
} SiAudioType;

typedef enum
{
    TELETEXT_RESERVED = 0,
    TELETEXT_INITIAL_PAGE,
    TELETEXT_SUB_TITLE,
    TELETEXT_ADDITIONAL_INFO,
    TELETEXT_SCHEDULED,
    TELETEXT_FOR_HEARING_IMPAIRED,
    TELETEXT_UNKNOWN,
	TELETEXT_MAX
} SiTeletextType;

typedef enum
{
    TELETEXT_SUBTITLE = 0,
    DVB_SUBTITLE,
    UNKNOWN_SUBTITLE,
	SUBTITLE_MAX
} SiSubTitleType;

typedef enum
{
	VIDEO_TYPE_MPEG2 = 0,
	VIDEO_TYPE_H264,
	VIDEO_TYPE_UNKNOWN,
	VIDEO_TYPE_MAX
} SiVideoType;

typedef enum
{
	SI_ES_VIDEO = 0,
	SI_ES_AUDIO,
	SI_ES_TELETEXT,
	SI_ES_SUBTITLE,
	SI_ES_UNKNOWN,
	SI_ES_MAX
}SiEsType;

typedef enum
{
	SI_TP_SATELLITE = 0,
	SI_TP_CABLE,
	SI_TP_TERRESTIAL,
	SI_TP_MAX
}SiNitTpType;

typedef struct SiSubtutleData_s
{
	U16					CompositionPage;
	U16					AncillaryPage;
} SiSubtitleData_t;

typedef struct SiTeletextData_s
{
	U16					MagazineNumber;
	U16					PageNumber;
} SiTeletextData_t;

typedef struct StreamSpecific_s
{
	U8                      EsSubType;
	SiSubtitleData_t		SubtitleData;
	SiTeletextData_t		TeletextData;
} StreamSpecific_t;

typedef struct SiEsData_s
{
	U16	           EsPid;
	SiEsType       EsType;
	char           EsLang[4];
	StreamSpecific_t EsSubData;
} SiEsData_t;

typedef struct SiProgramData_s
{
	U16							ServideId ;
	U16							PmtPid;
	U16							PcrPid;
#ifdef USA_SI_TABLE_SUPPORT
	U16							LCN;
#endif
	U8							ChannelScramble;
	U8                          NumberOfEs;
	SiServiceType				ServiceType;
	SiEsData_t                  EsData[MAX_NUMBER_OF_ES];
	char						ChannelName[MAX_NAME_LEN];
	struct SiProgramData_s     *Next_p;
} SiProgramData_t;

typedef struct SiService_s
{
	U16                       TsId;
	U16                       ONId;
	U8                        NumberOfProgram;
	char                      ServiceName[MAX_NAME_LEN];
	SiProgramData_t          *ProgramData;
} SiService_t;

typedef struct SiSatTpData_s
{
	U32							Frequency ;
	U32							SymbolRate;
	U16							Orbit;
	U8                          EastNotWest;
	U8							Polarization;
} SiSatTpData_t;

typedef struct SiCableTpData_s
{
	U32							Frequency ;
	U32							SymbolRate;
	U8							Modulation;
} SiCableTpData_t;

typedef struct SiTerrTpData_s
{
	U32							Frequency ;
	U8							BandWidth;
	U8							Constellation;
	U8							TxMode;
} SiTerrTpData_t;

typedef struct SiNitTpData_s
{
	SiNitTpType              TpType;
	U16                      TsId;
	U16                      OnId;
	SiSatTpData_t	         SatTpData;
	SiCableTpData_t	         CableTpData;
	SiTerrTpData_t	         TerrTpData;
	struct SiNitTpData_s  	*Next_p;
} SiNitTpData_t;

typedef struct SiNetworkData_s
{
	U16              NetworkId;
	U16              NumberOfTp;
	char             NetworkName[MAX_NAME_LEN];
	SiNitTpData_t	*NitTpData;
} SiNetworkData_t;

typedef void (*DemuxCallback_f)(U8 tableInfoId, U8 *data, U32 size);

typedef void (*SearchResult_f)(SiService_t *serviceData);
typedef void (*LiveSearchResult_f)(SiProgramData_t *progData, U8 *pmtData, U32 length);
typedef void (*NitResult_f)(SiNetworkData_t *networkData);

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *	Function Prototypes                                         *
 ****************************************************************/

#endif /* #define __TABLE_DEF_H */
