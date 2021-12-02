#ifndef __DATABASE_H
#define __DATABASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mvapp_def.h"

#define UCChar unsigned short

#define NOT_TUNNING									0
#define	RE_TUNNING									1

#define	DB_VALID									0
#define	DB_INVALID									1

#define	SAT_UNSELECT								0
#define	SAT_SELECT									1

#define	KIND_OFF									0
#define	KIND_ON										1

#define FONT_SIZE_MIN								20
#define FONT_SIZE_MAX								27

#define	LONGITUDE_NEW								0
#define LONGITUDE_SIRIUS_2_3						50
#define LONGITUDE_EUTELSAT_W3A						70
#define LONGITUDE_EUROBIRD_9						90
#define LONGITUDE_EUTELSAT_W1						100
#define LONGITUDE_HOTBIRD_6_7A_8					130
#define LONGITUDE_EUTELSAT_W2						160
#define LONGITUDE_ASTRA_1F_1G_1H_1KR_1L				192
#define LONGITUDE_EUTELSAT_W6						215
#define LONGITUDE_ASTRA_1D_3A						235
#define LONGITUDE_BADR_2_3_4_EUROBIRD_2				260
#define LONGITUDE_BADR_C							261
#define LONGITUDE_ASTRA_2A_B_D						282
#define LONGITUDE_ARABSAT_2B						305
#define LONGITUDE_INTELSAT_802_EUROBIRD_3			330
#define LONGITUDE_EUTELSAT_SESAT_W4					360
#define LONGITUDE_PAKSAT_1C							380
#define LONGITUDE_HELLAS_SAT_2___Europe				390
#define LONGITUDE_HELLAS_SAT_2___South_Africa		391
#define LONGITUDE_EXPRESS_AM1						400
#define LONGITUDE_TURKSAT_1C__2A_					420
#define LONGITUDE_INTELSAT_12						450
#define LONGITUDE_YAMAL_202C						490
#define LONGITUDE_EXPRESS_AM22_SESAT_2				531
#define LONGITUDE_INSAT_3EC							550
#define LONGITUDE_BONUM_1							560
#define LONGITUDE_NSS_703							570
#define LONGITUDE_INTELSAT_904						600
#define LONGITUDE_INTELSAT_902						620
#define LONGITUDE_INTELSAT_906						640
#define LONGITUDE_INTELSAT_704C						660
#define LONGITUDE_INTELSAT_7_10						685
#define LONGITUDE_INTELSAT_7_10C 					686
#define LONGITUDE_EUTELSAT_W5						705
#define LONGITUDE_INTELSAT_4						720
#define LONGITUDE_INSAT_3C							740
#define LONGITUDE_EDUSAT__INSAT_4CR					741
#define LONGITUDE_ABS_1								750
#define LONGITUDE_TELSTAR_10						765
#define LONGITUDE_THAICOM_2_5						785
#define LONGITUDE_THAICOM_2_5C						786
#define LONGITUDE_EXPRESS_AM2						800
#define LONGITUDE_INSAT3B_4A						830
#define LONGITUDE_INSAT_2E_3B_4AC					831
#define LONGITUDE_INTELSAT_709						850
#define LONGITUDE_CHINASTAR_1						875
#define LONGITUDE_ST_1								880
#define LONGITUDE_YAMAL_201							900
#define LONGITUDE_MEASAT_1_3						915
#define LONGITUDE_MEASAT_1_3C						916
#define LONGITUDE_INSAT_3A_4BC						935
#define LONGITUDE_INSAT_4B							936
#define LONGITUDE_NSS_6								950
#define LONGITUDE_ASIASAT_2C						1005
#define LONGITUDE_ASIASAT_2							1006
#define LONGITUDE_EXPESS_A2_KAZSAT_1				1030
#define LONGITUDE_ASIASAT_3SC						1055
#define LONGITUDE_ASIASAT_3S						1056
#define LONGITUDE_CAKRAWARTA_1						1077
#define LONGITUDE_TELKOM_1C							1080
#define LONGITUDE_NSS_11							1082
#define LONGITUDE_BSAT_1A_2A						1100
#define LONGITUDE_JCSAT_110							1101
#define LONGITUDE_SINOSAT_1							1105
#define LONGITUDE_PALAPA_C2C						1129
#define LONGITUDE_PALAPA_C2							1130
#define LONGITUDE_KOREASAT_2						1131
#define LONGITUDE_CHINASAT_6B						1155
#define LONGITUDE_KOREASAT_3						1160
#define LONGITUDE_THAICOM_1A						1200
#define LONGITUDE_ASIASAT_4							1220
#define LONGITUDE_JCSAT_4A							1240
#define LONGITUDE_SINOSAT_3							1250
#define LONGITUDE_JCSAT_3							1280
#define LONGITUDE_APSTAR_6C							1340
#define LONGITUDE_APSTAR_6							1341
#define LONGITUDE_TELSTAR_18						1380
#define LONGITUDE_TELSTAR_18C						1381
#define LONGITUDE_EXPRESS_AM3						1400
#define LONGITUDE_SUPERBIRD_C						1440
#define LONGITUDE_AGILA_2							1460
#define LONGITUDE_MEASAT_2							1480
#define LONGITUDE_OPTUS_B3							1520
#define LONGITUDE_JCSAT_2A							1540
#define LONGITUDE_OPTUS_C1							1560
#define LONGITUDE_OPTUS_D1							1600
#define LONGITUDE_INTELSAT_8						1660
#define LONGITUDE_INTELSAT_2						1690
#define LONGITUDE_INTELSAT_701						1800
#define LONGITUDE_NSS_5								1830
#define LONGITUDE_ECHOSTAR_1__2						2120
#define LONGITUDE_AMC_8C							2210
#define LONGITUDE_AMC_7C							2230
#define LONGITUDE_AMC_10C							2250
#define LONGITUDE_GALAXY_15C						2270
#define LONGITUDE_AMC_11C							2290
#define LONGITUDE_GALAXY_27							2310
#define LONGITUDE_ECHOSTAR_5						2311
#define LONGITUDE_GALAXY_13_HORIZONS_1				2330
#define LONGITUDE_GALAXY_14C						2350
#define LONGITUDE_GALAXY_10R						2370
#define LONGITUDE_ECHOSTAR_9						2390
#define LONGITUDE_GALAXY_23							2391
#define LONGITUDE_ECHOSTAR_7__AMC16					2410
#define LONGITUDE_DIRECTV_7S						2412
#define LONGITUDE_SATMEX_5							2432
#define LONGITUDE_SOLIDARIDAD_2						2451
#define LONGITUDE_SATMEX_6							2470
#define LONGITUDE_ANIK_F2							2489
#define LONGITUDE_ECHOSTAR_8__10					2500
#define LONGITUDE_DIRECTV_5							2502
#define LONGITUDE_ANIK_F1__F1R_						2527
#define LONGITUDE_AMC_15							2550
#define LONGITUDE_AMC_1								2570
#define LONGITUDE_AMC_4								2590
#define LONGITUDE_DIRECTV_1R_4_8					2591
#define LONGITUDE_GALAXY_16							2610
#define LONGITUDE_GALAXY_25							2630
#define LONGITUDE_GALAXY_3C							2650
#define LONGITUDE_GALAXY_26							2670
#define LONGITUDE_BRASILSAT_4C						2680
#define LONGITUDE_GALAXY_11							2690
#define LONGITUDE_NIMIQ_1							2691
#define LONGITUDE_GALAXY_28							2710
#define LONGITUDE_AMC_3								2730
#define LONGITUDE_AMC_2								2750
#define LONGITUDE_BRASILSAT_B3C						2760
#define LONGITUDE_AMC_9								2770
#define LONGITUDE_NIMIQ_2							2780
#define LONGITUDE_AMC_5								2810
#define LONGITUDE_SBS_6								2860
#define LONGITUDE_AMC_6								2880
#define LONGITUDE_NAHUEL_1A							2882
#define LONGITUDE_BRASILSAT_B1C						2900
#define LONGITUDE_BRASILSAT_B2C						2950
#define LONGITUDE_ESTRELA_DO_SUL_1					2970
#define LONGITUDE_ECHOSTAR_3__RAINBOW_1				2985
#define LONGITUDE_AMAZONAS							2990
#define LONGITUDE_INTELSAT_9						3020
#define LONGITUDE_INTELSAT_805C						3045
#define LONGITUDE_INTELSAT_707						3070
#define LONGITUDE_INTELSAT_705C						3100
#define LONGITUDE_INTELSAT_1R						3150
#define LONGITUDE_INTELSAT_1RC						3151
#define LONGITUDE_INTELSAT_3R_6B					3170
#define LONGITUDE_NSS_806C							3195
#define LONGITUDE_NSS_10							3226
#define LONGITUDE_INTELSAT_903						3255
#define LONGITUDE_INTELSAT_801						3285
#define LONGITUDE_HISPASAT_1C_1D					3300
#define LONGITUDE_INTELSAT_907						3325
#define LONGITUDE_INTELSAT_905						3355
#define LONGITUDE_NSS_7								3380
#define LONGITUDE_NSS_7C							3381
#define LONGITUDE_INTELSAT_901						3420
#define LONGITUDE_TELSTAR_12						3450
#define LONGITUDE_ATLANTIC_BIRD_1					3475
#define LONGITUDE_EXPRESS_A3						3490
#define LONGITUDE_ATLANTIC_BIRD_2					3520
#define LONGITUDE_NILESAT_101_102__ATLANTIC_BIRD_4	3530
#define LONGITUDE_ATLANTIC_BIRD_3					3550
#define LONGITUDE_ATLANTIC_BIRD_3C					3551
#define LONGITUDE_AMOS_1_2							3560
#define LONGITUDE_INTELSAT_10_02					3590
#define LONGITUDE_THOR_2_3							3592

typedef enum 
{
	MV_SAT_TURKSAT_1C__2A_ = 0,
	MV_SAT_HOTBIRD_6_7A_8,
	MV_SAT_EUTELSAT_W3A,
	MV_SAT_SIRIUS_2_3,
	MV_SAT_THOR_2_3,
	MV_SAT_ASTRA_1F_1G_1H_1KR_1L,
	MV_SAT_NILESAT_101_102__ATLANTIC_BIRD_4,
	MV_SAT_EUROBIRD_9,
	MV_SAT_EUTELSAT_W1,
	MV_SAT_EUTELSAT_W2,
	MV_SAT_EUTELSAT_W6,
	MV_SAT_ASTRA_1D_3A,
	MV_SAT_BADR_2_3_4_EUROBIRD_2,
	MV_SAT_BADR_C,
	MV_SAT_ASTRA_2A_B_D,
	MV_SAT_ARABSAT_2B,
	MV_SAT_INTELSAT_802_EUROBIRD_3,
	MV_SAT_EUTELSAT_SESAT_W4,
	MV_SAT_PAKSAT_1C,
	MV_SAT_HELLAS_SAT_2___Europe,
	MV_SAT_HELLAS_SAT_2___South_Africa,
	MV_SAT_EXPRESS_AM1,
	MV_SAT_INTELSAT_12,
	MV_SAT_YAMAL_202C,
	MV_SAT_EXPRESS_AM22_SESAT_2,
	MV_SAT_INSAT_3EC,
	MV_SAT_BONUM_1,
	MV_SAT_NSS_703,
	MV_SAT_INTELSAT_904,
	MV_SAT_INTELSAT_902,
	MV_SAT_INTELSAT_906,
	MV_SAT_INTELSAT_704C,
	MV_SAT_INTELSAT_7_10,
	MV_SAT_INTELSAT_7_10C,
	MV_SAT_EUTELSAT_W5,
	MV_SAT_INTELSAT_4,
	MV_SAT_INSAT_3C,
	MV_SAT_EDUSAT__INSAT_4CR,
	MV_SAT_ABS_1,
	MV_SAT_TELSTAR_10,
	MV_SAT_THAICOM_2_5,
	MV_SAT_THAICOM_2_5C,
	MV_SAT_EXPRESS_AM2,
	MV_SAT_INSAT3B_4A,
	MV_SAT_INSAT_2E_3B_4AC,
	MV_SAT_INTELSAT_709,
	MV_SAT_CHINASTAR_1,
	MV_SAT_ST_1,
	MV_SAT_YAMAL_201,
	MV_SAT_MEASAT_1_3,
	MV_SAT_MEASAT_1_3C,
	MV_SAT_INSAT_3A_4BC,
	MV_SAT_INSAT_4B,
	MV_SAT_NSS_6,
	MV_SAT_ASIASAT_2C,
	MV_SAT_ASIASAT_2,
	MV_SAT_EXPESS_A2_KAZSAT_1,
	MV_SAT_ASIASAT_3SC,
	MV_SAT_ASIASAT_3S,
	MV_SAT_CAKRAWARTA_1,
	MV_SAT_TELKOM_1C,
	MV_SAT_NSS_11,
	MV_SAT_BSAT_1A_2A,
	MV_SAT_JCSAT_110,
	MV_SAT_SINOSAT_1,
	MV_SAT_PALAPA_C2C,
	MV_SAT_PALAPA_C2,
	MV_SAT_KOREASAT_2,
	MV_SAT_CHINASAT_6B,
	MV_SAT_KOREASAT_3,
	MV_SAT_THAICOM_1A,
	MV_SAT_ASIASAT_4,
	MV_SAT_JCSAT_4A,
	MV_SAT_SINOSAT_3,
	MV_SAT_JCSAT_3,
	MV_SAT_APSTAR_6C,
	MV_SAT_APSTAR_6,
	MV_SAT_TELSTAR_18,
	MV_SAT_TELSTAR_18C,
	MV_SAT_EXPRESS_AM3,
	MV_SAT_SUPERBIRD_C,
	MV_SAT_AGILA_2,
	MV_SAT_MEASAT_2,
	MV_SAT_OPTUS_B3,
	MV_SAT_JCSAT_2A,
	MV_SAT_OPTUS_C1,
	MV_SAT_OPTUS_D1,
	MV_SAT_INTELSAT_8,
	MV_SAT_INTELSAT_2,
	MV_SAT_INTELSAT_701,
	MV_SAT_NSS_5,
	MV_SAT_ECHOSTAR_1__2,
	MV_SAT_AMC_8C,
	MV_SAT_AMC_7C,
	MV_SAT_AMC_10C,
	MV_SAT_GALAXY_15C,
	MV_SAT_AMC_11C,
	MV_SAT_GALAXY_27,
	MV_SAT_ECHOSTAR_5,
	MV_SAT_GALAXY_13_HORIZONS_1,
	MV_SAT_GALAXY_14C,
	MV_SAT_GALAXY_10R,
	MV_SAT_ECHOSTAR_9,
	MV_SAT_GALAXY_23,
	MV_SAT_ECHOSTAR_7__AMC16,
	MV_SAT_DIRECTV_7S,
	MV_SAT_SATMEX_5,
	MV_SAT_SOLIDARIDAD_2,
	MV_SAT_SATMEX_6,
	MV_SAT_ANIK_F2,
	MV_SAT_ECHOSTAR_8__10,
	MV_SAT_DIRECTV_5,
	MV_SAT_ANIK_F1__F1R_,
	MV_SAT_AMC_15,
	MV_SAT_AMC_1,
	MV_SAT_AMC_4,
	MV_SAT_DIRECTV_1R_4_8,
	MV_SAT_GALAXY_16,
	MV_SAT_GALAXY_25,
	MV_SAT_GALAXY_3C,
	MV_SAT_GALAXY_26,
	MV_SAT_BRASILSAT_4C,
	MV_SAT_GALAXY_11,
	MV_SAT_NIMIQ_1,
	MV_SAT_GALAXY_28,
	MV_SAT_AMC_3,
	MV_SAT_AMC_2,
	MV_SAT_BRASILSAT_B3C,
	MV_SAT_AMC_9,
	MV_SAT_NIMIQ_2,
	MV_SAT_AMC_5,
	MV_SAT_SBS_6,
	MV_SAT_AMC_6,
	MV_SAT_NAHUEL_1A,
	MV_SAT_BRASILSAT_B1C,
	MV_SAT_BRASILSAT_B2C,
	MV_SAT_ESTRELA_DO_SUL_1,
	MV_SAT_ECHOSTAR_3__RAINBOW_1,
	MV_SAT_AMAZONAS,
	MV_SAT_INTELSAT_9,
	MV_SAT_INTELSAT_805C,
	MV_SAT_INTELSAT_707,
	MV_SAT_INTELSAT_705C,
	MV_SAT_INTELSAT_1R,
	MV_SAT_INTELSAT_1RC,
	MV_SAT_INTELSAT_3R_6B,
	MV_SAT_NSS_806C,
	MV_SAT_NSS_10,
	MV_SAT_INTELSAT_903,
	MV_SAT_INTELSAT_801,
	MV_SAT_HISPASAT_1C_1D,
	MV_SAT_INTELSAT_907,
	MV_SAT_INTELSAT_905,
	MV_SAT_NSS_7,
	MV_SAT_NSS_7C,
	MV_SAT_INTELSAT_901,
	MV_SAT_TELSTAR_12,
	MV_SAT_ATLANTIC_BIRD_1,
	MV_SAT_EXPRESS_A3,
	MV_SAT_ATLANTIC_BIRD_2,
	MV_SAT_ATLANTIC_BIRD_3,
	MV_SAT_ATLANTIC_BIRD_3C,
	MV_SAT_AMOS_1_2,
	MV_SAT_INTELSAT_10_02,
	MV_NEW_SAT0,
	MV_NEW_SAT1,
	MV_NEW_SAT2,
	MV_NEW_SAT3,
	MV_NEW_SAT4,
	MV_NEW_SAT5,
	MV_NEW_SAT6,
	MV_NEW_SAT7,
	MV_NEW_SAT8,
	MV_NEW_SAT9,
	MV_SAT_MAX
} MV_DEFAULT_SAT;

typedef enum
{
	CSAPP_LANG_ENGLISH=0,
	CSAPP_LANG_TURKISH,
	CSAPP_LANG_GERMAN,
	CSAPP_LANG_FRENCH,
	CSAPP_LANG_MAX_NUM
} CSAPP_Lang_Type;

#define		DB_DIR										"/application/data"
#define 	BACKUP_DIR									"/application/backup"
#define 	DEFAULT_DIR									"/usr/work0/default"
#define		USB_BACKUP_DIR								"/mnt/usb/disk1/BACKUP"
#define		SAT_DB_FILE									"/application/data/sattp"
#define		INDEX_DB_FILE								"/application/data/indexdb"
#define		CH_DB_FILE									"/application/data/database"
#define		FONT_FILE									"/usr/local/etc/font.txt"
#define		USB_ROOT									"/mnt/usb/disk1"
#define 	MERIH_VIDEO_URL								"www.merihvideo.com.tr"

#define		EPP_CHECK_STR								"0015"

#define		MV_MAX_SERVICE_COUNT		 				20000

#define 	MV_MAX_SATELLITE_COUNT						MV_SAT_MAX
#define 	MV_MAX_TP_COUNT								25
#define		MV_MAX_AUDIO_COUNT							8
#define		MV_MAX_FAV_KIND								8
#define		MV_MAX_FAV_COUNT							255
#define		MV_LIST_NUMBER								2
#define 	MV_FIXED_LIST_NUMBER						MV_DEFAILT_LIST_NUM + MV_MAX_FAV_KIND
#define 	MV_MAX_LIST_NUMBER							( MV_MAX_FAV_KIND + ( MV_MAX_SATELLITE_COUNT * 2 ) + 1 )
#define		MV_DEFAILT_LIST								0
#define		MV_DEFAILT_LIST_NUM							1
#define		MAX_SERVICE_NAME_LENGTH 					16
#define 	MV_DB_INVALID_SERVICE_INDEX					MV_MAX_SERVICE_COUNT
#define 	MV_RECALL_MAX_ITEM							10

#define 	kCS_DB_DEFAULT_TV_LIST_ID      				0
#define 	kCS_DB_DEFAULT_RADIO_LIST_ID    			1
#define 	kCS_DB_ALPHABETICAL_SORT_PRECISION	    	11
#define     kCS_DB_MIN_USER_LCN                         350

#define 	kCS_DBU_MAX_ScreenTransparent				255
#define 	kCS_DBU_MAX_BannerKeepTime					10
#define 	kCS_DBU_MAX_VOLUME							50

#define		kCS_DBU_DEFAULT_ScartMode					eCS_DBU_SCARTCVBS
#define 	kCS_DBU_DEFAULT_SpdifStatus					eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_VideoOutput					eCS_DBU_OUTPUT_YPBPR
#define 	kCS_DBU_DEFAULT_VideoDefinition				eCS_DBU_DEFINITION_1080I /* was eCS_DBU_DEFINITION_720P : By KB Kim 2011.01.19 */
#define 	kCS_DBU_DEFAULT_VideoEncodingMode			eCS_DBU_PAL
#define 	kCS_DBU_DEFAULT_VideoAspectRatio			eCS_DBU_ASPECT_RATIO_16_9 /* was eCS_DBU_ASPECT_RATIO_AUTOMATIC : By KB Kim 2011.01.20 */
#define 	kCS_DBU_DEFAULT_AspectRatioMode				eCS_DBU_ARM_COMBINED
#define 	kCS_DBU_DEFAULT_AudioType		         	eCS_DBU_AUDIO_AUTO
#define 	kCS_DBU_DEFAULT_AudioLanguage				CSAPP_LANG_TURKISH
#define 	kCS_DBU_DEFAULT_SubtitleLanguage			CSAPP_LANG_TURKISH
#define 	kCS_DBU_DEFAULT_MenuLanguage				CSAPP_LANG_TURKISH
#define 	kCS_DBU_DEFAULT_TimeOffset					0x0200
#define 	kCS_DBU_DEFAULT_TimeMode					eCS_DBU_TIME_INTERNET   /* By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_TimeRegion					1
#define 	kCS_DBU_DEFAULT_PinCode						"0000"
#define 	kCS_DBU_DEFAULT_BootLockStatus				eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_InstallLockStatus			eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_EditLockStatus				eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_ServicesLockStatus			eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_ParentalLockStatus			eCS_DBU_ON
#define 	kCS_DBU_DEFAULT_ParentalRate		        0
#define 	kCS_DBU_DEFAULT_ScreenTransparent			235  /* was 255 : By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_BannerKeepTime				2    /* was 5 : By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_LcnMode		                eCS_DB_Appearing_Order
#define 	kCS_DBU_DEFAULT_LNBPower	                eCS_DBU_ON
#define 	kCS_DBU_DEFAULT_CH_Change	                1    /* Freez mode By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_CH_List	                	1    /* was 0 : By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_Recall		                0
#define 	kCS_DBU_DEFAULT_Power		                0
#define 	kCS_DBU_DEFAULT_LED		                	1   /* was 0 : By request 2011.04.12 */
#define 	kCS_DBU_DEFAULT_Volume						30
#define 	kCS_DBU_DEFAULT_MuteStatus					eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_DHCP						eCS_DBU_OFF
#define 	kCS_DBU_DEFAULT_DNS_Type					eCS_DBU_ON
#define 	kCS_DBU_DEFAULT_Antena						1   /* was 0 : By KB Kim 2011.01.19 */
#define 	kCS_DBU_DEFAULT_Local_Logitude              325
#define 	kCS_DBU_DEFAULT_Local_Latitude              395
#define		kCS_DBU_DEFAULT_Font_Type					0
#define		kCS_DBU_DEFAULT_Font_Size					25
#define		kCS_DBU_DEFAULT_Font_Kind					0
#define		kCS_DBU_DEFAULT_Power_Off_Mode				1
#define		kCS_DBU_DEFAULT_PushGame_Lavel				0
#define		kCS_DBU_DEFAULT_Motor_Limit					0
#define		kCS_DBU_DEFAULT_Skin						2   /* was 0 : By KB Kim 2011.01.19 */
#define		kCS_DBU_DEFAULT_Sleep						0
#define		kCS_DBU_DEFAULT_Ani							1   /* was 0 : By KB Kim 2011.01.19 */

/* For Heart bit control By KB Kim 2011.03.11 */
#define		kCS_DBU_DEFAULT_HeartBit					1
#define		kCS_DBU_DEFAULT_UseSubTitle					1
#define		kCS_DBU_DEFAULT_FixFontSize					5

enum
{
	MV_POWER_OFF = 0,
	MV_POWER_ON
};

typedef  enum
{
	eCS_DB_OK,
	eCS_DB_ERROR
} tCS_DB_Error;				/*数据库模块错误类型*/

typedef enum
{
	eCS_DB_INVALID_SERVICE = 0,
	eCS_DB_TV_SERVICE,
	eCS_DB_RADIO_SERVICE,
	eCS_DB_HDTV_SERVICE,
	eCS_DB_DATA_SERVICE		
} tCS_DB_ServiceType;		/*节目类型*/

typedef enum
{
	eCS_DB_TV_LIST =0,
	eCS_DB_RADIO_LIST,
	eCS_DB_FAV_TV_LIST,
	eCS_DB_FAV_RADIO_LIST,
	eCS_DB_SAT_TV_LIST,
	eCS_DB_SAT_RADIO_LIST,
	eCS_DB_INVALID_LIST		
} tCS_DB_ServiceListType;		/*节目列表类型*/

typedef enum
{
	eCS_DB_NOT_LOCKED = 0,
	eCS_DB_LOCKED
} tCS_DB_LockStatus;					/*加锁状态*/

typedef enum
{
	eDBASE_DELETE = 0,	
	eDBASE_NOT_DELETE
} tCS_DB_DeleteStatus;				/*删除状态*/

typedef enum
{
	eDBASE_NOT_SELECT = 0,	
	eDBASE_SELECT
} tCS_DB_SelectStatus;	

typedef	enum
{
	eCS_DB_Appearing_Order = 0,		/*普通模式，节目按搜索顺序排列*/
	eCS_DB_Operator_Defined			/*节目按运营商定义顺序排列*/
} tCS_DB_LCNMode;					/*LCN模式*/				/*音频声道模式*/

typedef enum
{
	MV_DB_STEREO = 0,
	MV_DB_MONO,	
	MV_DB_LEFT,
	MV_DB_RIGHT
} MV_DB_AudioMode;					/*音频声道模式*/

typedef enum
{
    eCS_DB_AUDIO_MPEG2 = 0, /* Modify By River 06.12.2008 eCS_DB_AUDIO_PCM */
    eCS_DB_AUDIO_AC3,
    eCS_DB_AUDIO_AAC,       /* Add  By River 06.12.2008 */
    eCS_DB_AUDIO_LATM,      /* Add  By River 06.12.2008 */
    eCS_DB_AUDIO_UNKNOWN    /* Add  By River 06.12.2008 */
} tCS_DB_AudioType;

typedef enum
{
	eCS_DB_VIDEO_MPEG2 = 0,
	eCS_DB_VIDEO_H264
} tCS_DB_VideoType;


typedef enum
{
	eCS_DB_BY_NAME_A_Z,
	eCS_DB_BY_NAME_Z_A,
	eCS_DB_BY_FTA_CAS,
	eCS_DB_BY_CSA_FTA,
	eCS_DB_BY_SD_HD,
	eCS_DB_BY_HD_SD,
	eCS_DB_BY_NORMAL
} tCS_DB_SortType;


typedef	enum
{
	eCS_DBU_SCARTCVBS,	/*CVBS信号选择*/
	eCS_DBU_SCARTRGB		/*RGB信号选择*/
} tCS_DBU_ScartMode;

typedef	enum
{
	eCS_DBU_OFF,
	eCS_DBU_ON
} tCS_DBU_Status;		/*打开/关闭状态，适用于所有0，1状态的数据*/

#if 0 /* By KB Kim 2010.08.31 */
typedef	enum
{
	eCS_DBU_OUTPUT_CVBS = 0,
	eCS_DBU_OUTPUT_YC,
	eCS_DBU_OUTPUT_YCBCR,
	eCS_DBU_OUTPUT_YPBPR,
	eCS_DBU_OUTPUT_RGB,
	eCS_DBU_OUTPUT_HDMI
} tCS_DBU_VideoOutput;
#else
typedef	enum
{
	eCS_DBU_OUTPUT_RGB = 0,
	eCS_DBU_OUTPUT_YPBPR,
	// eCS_DBU_OUTPUT_YC,
	eCS_DBU_OUTPUT_MAX
} tCS_DBU_VideoOutput;
#endif

typedef	enum
{
	eCS_DBU_DEFINITION_480I = 0,
	eCS_DBU_DEFINITION_576I,
	eCS_DBU_DEFINITION_576P,
	eCS_DBU_DEFINITION_720P,
	eCS_DBU_DEFINITION_1080I,
	//eCS_DBU_DEFINITION_1080P,
	eCS_DBU_DEFINITION_AUTOMATIC
} tCS_DBU_VideoDefinition; 	/*图像清晰度*/

typedef enum
{
    eCS_DBU_AUDIO_AUTO = 0,
    eCS_DBU_AUDIO_MP2,
    eCS_DBU_AUDIO_AC3
} tCS_DBU_AudioType;

typedef	enum
{
	eCS_DBU_NTSC = 0,
	eCS_DBU_PAL,
	eCS_DBU_SECAM,
	eCS_DBU_AUTOMATIC
} tCS_DBU_VideoEncodingMode; 	/*视频制式*/

typedef	enum
{
	eCS_DBU_ASPECT_RATIO_4_3	 = 0,
	eCS_DBU_ASPECT_RATIO_16_9,
	eCS_DBU_ASPECT_RATIO_AUTOMATIC
} tCS_DBU_VideoAspectRatio;		/*视频屏宽比*/

typedef enum
{
	eCS_DBU_ARM_COMBINED = 0,	
	eCS_DBU_ARM_LETTER_BOX,
	eCS_DBU_ARM_PANSCAN
} tCS_DBU_AspectRatioMode;		/*视频屏宽比显示模式*/

typedef	enum
{
	eCS_DBU_TIME_AUTOMATIC = 0, 	/*从TDT/TOT中读取时间*/
	eCS_DBU_TIME_MANUAL,			/*用户自己输入时间*/
	eCS_DBU_TIME_INTERNET
} tCS_DBU_TimeMode;					/*时间设置模式*/

typedef struct
{
	tCS_DB_ServiceListType 	sCS_DB_ServiceListType;		/*列表类型*/
	U32						sCS_DB_ServiceListTypeValue;/*列表索引值*/
} tCS_DB_ServiceListTriplet;								/*节目列表关键字数据*/

typedef enum
{
	EN_SORT_BY_SID,										/**< by service ID */
	EN_SORT_BY_NAME_AZ,									/**< by name from a to z */
	EN_SORT_BY_NAME_ZA,									/**< by name from z to a */
	EN_SORT_BY_CHNO,									/**< by channel number and service ID */
	EN_SORT_BY_FAV,	    								/**< by favorite first */
	EN_SORT_BY_NON_FAV,									/**< by non-favorite first */
	EN_SORT_BY_FAVEX,	    							/**< by favorite extention first */
	EN_SORT_BY_NON_FAVEX,								/**< by non-favorite extention first */
	EN_SORT_BY_FREE,   									/**< by free service */
	EN_SORT_BY_CA,	    								/**< by scramble service */
	EN_SORT_BY_CA_SECA,
	EN_SORT_BY_CA_VIACCESS,
	EN_SORT_BY_CA_IRDETO,
	EN_SORT_BY_CA_VIDEOGUARD,
	EN_SORT_BY_CA_CONAX,
	EN_SORT_BY_CA_CRYPTO,
	EN_SORT_BY_CA_POWERVU,
	EN_SORT_BY_CA_BETA,
	EN_SORT_BY_CA_NAGRA,
#ifdef FOR_USA
	EN_SORT_BY_LCN
#endif
} EN_SORT_TYPE;

typedef struct 
{
	U16		u16ChIndex;										/** Original Channel Index */
	U16    	u16ServiceId;									/**< The service id come form SI table to check reduce service. */ 
	U16    	u16PCRPid ;										/**< the PCR PID */ 
	U16    	u16VideoPid ;									/**< the Video PID */ 
	U16    	u16AudioPid ;									/**< the Audio PIDs */ 
	U16    	u16TransponderIndex ;							/**< This is transponder index and a relation key. */
	U16    	u16EitHeaderIdx; 								/**< This is a index point to EIT header. */
	U16    	bitTTXRes			: 2; 
	U16    	bitTTXSubtitle		: 1;  						/**< the TTX SubTitle */ 
	U16    	u16TTXPid			:13;						/**< the TTX PID */ 
	U16    	u16PMTPid;
	U16    	u16Reserved;
	char   	acServiceName[MAX_SERVICE_NAME_LENGTH];			/**< the service name */ 
/********************************************************************************/
	U8     	u8Lock				: 1;						/**< parental lock	:: 0 : unlock , 1 : lock */
	U8     	u8Scramble			: 1;						/**< scrambled indicator :: 0 : not scrambled , 1 : scrambled */
	U8     	u8Watch				: 1;						/**< user watch service program :: 1 : user watch service program */
	U8     	u8Erase				: 1;						/**< service deleted flag or stamping flag :: 1 : service is deleted(or stamp) in SDRAM database */
	U8     	u8Favor				: 1;						/**< Favor service flag :: 1 : this service is favor service */
	U8     	u8Audio_ch			: 1;						/**< audio channel :: 0 : channel_1  , 1 : channel_2 */
	U8     	u8Audio_Mode		: 2;						/**<  audio mode	
																	00 : stereo
																	01 : mono
																	10 : both left
																	11 : both right	 */
/********************************************************************************/
	U8    	u8TvRadio			: 2;						/**<  TV / Radio / Data	
																	00 : Data
																	01 : TV
																	10 : Radio
																	11 : HD TV	 */
	U8		u8Audio_Type		: 3;						/**< MPEG2 / AC3 / AAC / LATM 
																	000 : MPEG2
																	001 : AC3
																	010 : AAC
																	011 : LATM
																	100 : Unknown */
	U8		u8VideoType			: 1;						/**< 0 : MPEG2 , 1 : H.254 */
	U8    	u8Ch_Unused1		: 1;						/**< Temp  */ 
	U8    	u8Ch_Unused2		: 1;						/**< Temp  */ 
/********************************************************************************/
	U8     	u8CASystemID[5];
/********************************************************************************/
	U8     	u8AudioVolume 		: 6;						/* Store audio volume step by service */
	U8     	u8AC3Flag			: 1;
	U8     	u8Valid				: 1;
/********************************************************************************/
//Used when defined ENABLE_LCN_SUPPORT <-- START
#ifdef FOR_USA
	U16    	b1VisibleService	: 1;
	U16    	b1LCNEnable			: 1;
	U16    	u16LCN				:14;
	U16    	u16ONID; 										/* Original Network ID */
#endif
//Used when defined ENABLE_LCN_SUPPORT <-- END

} MV_stServiceInfo;

typedef struct 
{
	U8		u8SatelliteIndex;
	char	acSatelliteName[MAX_SERVICE_NAME_LENGTH] ;		/**< the satellite name */
	U16		u16LocalFrequency ;								/**< This is local frequency. The unit is MHz.
																The normal value for C band is 5150 and for Ku band is 11300.*/
	S16		s16Longitude;									/**< This is satellite longitude.  
			     													The value needs divide 10 to convert to actual value.
		      	     													Example: The decimal value 1055 is 105.5 degree */
	U16		u16Control12V		:1;							/**< 12V control on(TRUE) /off(FALSE) */
	U16		u16Tone22K			:1;							/**< 22K tone on(TRUE) /off(FALSE) */
	U16		u16DiSEqC			:10;						/**< DiSEqC off or port 1,2, ...
			    													The value 0 indicate the DiSEqC is off */
	U16		u16Select			:1;							/**< Select satellite for Multi Scan  0 : Unselect , 1 : Select .. Default Value : 0*/
			/* For Tone Burst Control : By KB Kim 2011.02.28 */
	U16		u16ToneBurst		:2;							/**< ToneBurstControl  0 : OFF , 1 : Sat A, 2 : Sat B, default : 0*/
//	U16		u16Unused2			:1;
	U16		u16LNBPower			:1;							/**< LNB_POWER_SYSTEM / LNB_POWER_OFF / LNB_POWER_13V18V 
															/ LNB_POWER_13V / LNB_POWER_18V */ 
	U16		u16LocalFrequency_High ;						/**< Second local frequency. The unit is MHz.*/
	U8		u8LNBType;   									/**< EN_LNB_TYPE_C_BAND / EN_LNB_TYPE_KU_BAND / EN_LNB_TYPE_UNIVERSAL */ 
	U8		u8MotorPosition; 								/**< DiSEqC 1.2  Motor Position , OFF(0)/01/02/....... */
} MV_stSatInfo;

typedef struct 
{
	U16		u16TPIndex ;									/**< This is satellite index and a relation key. */
	char	acTPName[MAX_SERVICE_NAME_LENGTH];				/**< the transponder name */ 
	U8		u8SatelliteIndex ;								/**< This is satellite index and a relation key. */
//	U16		u16LocalFrequency; 								/**< This is local frequency. The unit is MHz.
// 	                  													The normal value for C band is 5150 and for Ku band is 11300.*/
	U16		u16TPFrequency;       							/**< This is transponder frequency. The unit is MHz. */
	U16		u16SymbolRate;									/**< This is transponder symbol rate. The unit is Kbaud. */
//	BOOL	b8Control12V;										/**< 12V control on(TRUE) / OFF(FALSE) */
//	BOOL	b8Tone22K;										/**< 22K tone ON(TRUE) / OFF(FALSE) */
//	U8     	u8DiSEqC;										/**< DiSEqC OFF or port 1,2, ...
//																The value 0 indicate the DiSEqC is off */
	U8		u8Polar_H			: 1;						/**< Horizontal(TRUE) / Vertical(FALSE) */
	U8		u8Valid				: 1;
	U8		u8Unused			: 7;						/**< Fit to 4 bytes alignment. */
	U16    	u16NID; 										/* Network ID */
	U16    	u16TSID; 										/* Transport Stream ID */ 
#ifdef FOR_USA	
	U16		u16OrgNID; 										/**< Original Network ID*/
#endif	 
} MV_stTPInfo;

typedef struct 
{
	U16		u16FavChIndex[MV_LIST_NUMBER][MV_MAX_FAV_COUNT];				/**< This is Fav Ch index. */
	U8		u8FavCount[MV_LIST_NUMBER];
	char	acFavName[MAX_SERVICE_NAME_LENGTH];				/**< the Favorite name */ 
} MV_stFavInfo;

typedef struct 
{
	U16		u16FavChIndex[MV_LIST_NUMBER][100];				/**< This is Fav Ch index. */
	U8		u8FavCount[MV_LIST_NUMBER];
	char	acFavName[MAX_SERVICE_NAME_LENGTH];				/**< the Favorite name */ 
} MV_stFavInfo_Temp_Old;

typedef struct 
{
	U16		u16SatIndexIndex;								/**< Sat Category Index size is MV_SAT_MAX */
	U16		u16ChIndex;										/**< Channel index in Sat Category */
} MV_stSat_IndexInfo;

typedef struct 
{
	U8						u8SatSearchIndex[MV_SAT_MAX];
	U16						u16ChCount[MV_SAT_MAX*2];	/**< This is Ch index. */
	MV_stSat_IndexInfo		stSatIndexInfo[MV_MAX_SERVICE_COUNT];	/**< TV or Radio */ 
} MV_stDB_ListIndex;

typedef struct 
{
	U16		u16ChCount;	/**< This is Ch index. */
	U16		stChIndex[MV_MAX_SERVICE_COUNT];	/**< TV or Radio */ 
} MV_stGB_ListIndex;

typedef struct 
{
	U8		u8SatIndex;										/**< Sat Category Index */
	U16		u8ChIndex[MV_MAX_SERVICE_COUNT];				/**< Channel Index in Sat Category Index */
} MV_stSat_ChIndexInfo;

typedef struct
{
	U16		u16Index_Ch		:14;							/**< This is Ch index. */
	U16		u16Ch_Kind		:2;								/**< TV or Radio */ 
} MV_stIndex;

typedef struct
{
	U32 		Service_Index 			:16;
	U32 		Delete_Flag   			:1;
	U32 		Move_Flag   			:1;
	U32 		Select_Flag   			:1;
	U32			Lock_Flag				:1;
	U32 		LCN              		:12;
} tCS_DB_ServiceManageData;

typedef struct
{
	tCS_DB_ServiceListTriplet		List_Triplet;
	tCS_DB_ServiceManageData		*Service_Data;
	U16								Service_Number;
	U8								u8SortMode;
	U16 							Current_Service;
} tCS_DB_ServiceList;


typedef struct
{
	U16 	u16DB_List_ServiceNum[MV_LIST_NUMBER];	/*列表中的节目总数*/
} tCS_DB_ServiceListInfo;			/*节目列表的相关信息*/

typedef	struct
{
	char	sCS_DBU_CheckString[4];
	U8		sCS_DBU_ScartMode;										/*SCART输出信号格式*/
	U8		sCS_DBU_SpdifStatus;									/*SPDIF输出状态*/	
	U8 		sCS_DBU_VideoOutPut;									/*视频输出端口*/
	U8 		sCS_DBU_VideoDefinition;								/*视频清晰度*/
	U32		sCS_DBU_VideoEncodingMode;								/*视频制式*/
	U32		sCS_DBU_VideoAspectRatio;								/*视频屏宽比*/
	U32		sCS_DBU_AspectRatioMode;								/*视频屏宽比显示模式*/
	U32		sCS_DBU_DefaultAudioType;								/*默认音频类型*/
	U32		sCS_DBU_AudioLanguage;									/* Audio Language */
	U32		sCS_DBU_SubtitleLanguage;								/* SubTitle Language */
	U32		sCS_DBU_MenuLanguage;									/* OSD Language */
	U32		sCS_DBU_TimeOffset;										/*时间偏移,时间格式第31位: 1表示－，0表示+, 低16位是4数字4位BCD码*/
	U32		sCS_DBU_TimeMode;										/*时间设置模式*/
	U32		sCS_DBU_TimeRegion;										/*时间区域索引值*/
	char	sCS_DBU_PinCode[4];										/*密码*/
	U8		sCS_DBU_InstallLockStatus;								/* Menu Lock status */
	U8		sCS_DBU_BootLockStatus;									/* Boot Lock status */
	U8		sCS_DBU_EditLockStatus;									/* Channel Edit Lock status */
	U8		sCS_DBU_ServicesLockStatus;								/* Channel Lock status */
	U8		sCS_DBU_ParentalLockStatus;								/* All Lock status */
	U32		sCS_DBU_ParentalRate;									/* Parental Menu Lock status */
	U32		sCS_DBU_ScreenTransparent;								/* OSD Transparenst Level */
	U32		sCS_DBU_BannerKeepTime;									/* Time of Banner Display */
	U32		sCS_DBU_LCNMode;										/* Enable or Disable Local Channel Number Mode for USA */
	U32		sCS_DBU_LNB_Power;										/* Enable or Disable LNB Power */
	U32		sCS_DBU_CH_Change;										/* Channel change Type : Pause Screen or Black Screen */
	U32		sCS_DBU_CH_List_Type;									/* Channel List Type : Extend List or Normal List */
	U32		sCS_DBU_Recall;											/* Recall Function Option : Single Recall & Multi recall by List */
	U32		sCS_DBU_Power;											/* Power Option : Sleep mode & Real Power off */
	U32		sCS_DBU_LED;											/* Front Display Time LED : Off & On */
	U32		sCS_DBU_DHCP;											/* Enable or Disable DHCP */
	U32		sCS_DBU_DNS_Type;										/* Enable or Disable Auto upgrade DNS1 */
	U32		sCS_DBU_Antenna_Type;									/* Antenna Connection Type : Direct, Diseqc 1.0,1.1,1.2 , Usals, Unicable */
	U32		sCS_DBU_Local_Longitude;								/* Local Longitude Value */
	U32		sCS_DBU_Local_Latitude;									/* Local Latitude Value */
	U32		sCS_DBU_Font_Type;										/* Font Type Value */
	U32		sCS_DBU_Font_Size;										/* Font Size Value */	
	U32		sCS_DBU_Font_Kind;										/* Font Kind Value */	
	U32		sCS_DBU_Power_Off_Mode;									/* Power On Off Mode */	
	U32		sCS_DBU_Push_Game_Level;								/* Game - PushPush Level Save */
	U32		sCS_DBU_Motor_Limit;									/* Disecq Motor Limit Setting On & Off */
	char	sCS_DBU_WebSite_Address[64];							/* Download S/W for upgrade Website Address */
	char	sCS_DBU_PluginSite_Address[64];							/* Download S/W for upgrade Website Address */
	U8		sCS_DBU_Skin;											/* OSD Skin Select */
	U8		sCS_DBU_Sleep;											/* SleepTime 15 min * 8 = 2 hour */
	U8		sCS_DBU_Ani;											/* Menu Animation On & Off */
	U8      sCS_DBU_HeartBit;                                       /* Heart Bit On & Off */
	U8      sCS_DBU_Use_Subtitle;                                   /* Subtitle Auto display On & Off */
	U8      sCS_DBU_Fix_Fontsize;                                   /* Fixed Font Size 0 ~ 5 */
}tCS_DBU_UserSetting_Save; 	/*用户设置数据部分，保持32位对齐，方便e2p读写*/

typedef struct
{
	tCS_DB_ServiceListTriplet 	sCS_DBU_ServiceList;				/*节目所在列表信息*/
	U32          				sCS_DBU_ServiceIndex;				/*列表中该节目索引号*/	
} tCS_DBU_Service;													/*节目索引信息关键字,32位对齐，方便E2P读写*/

#ifdef SMART_PHONE
enum
{
/* 00 */	MVAPP_ENUM_INSTALLATION = 0,
/* 01 */	MVAPP_ENUM_SAT_SETTING,
/* 02 */	MVAPP_ENUM_TP_SETTING,
/* 03 */	MVAPP_ENUM_LANG_SETTING,
/* 04 */	MVAPP_ENUM_TIME_SETTING,
/* 05 */	MVAPP_ENUM_SYSTEM_SETTING,
/* 06 */	MVAPP_ENUM_AV_SETTING,
/* 07 */	MVAPP_ENUM_PIN_SETTING,
/* 08 */	MVAPP_ENUM_NETWORK_SETTING,
/* 09 */	MVAPP_ENUM_SYSTEM_INFORMATION,
/* 10 */	MVAPP_ENUM_RECORDED_FILE,
/* 11 */	MVAPP_ENUM_RECORD_CONFIG,
/* 12 */	MVAPP_ENUM_FILE_TOOLS,
/* 13 */	MVAPP_ENUM_USB_REMOVE,
/* 14 */	MVAPP_ENUM_STORAGE_INFO,
/* 15 */	MVAPP_ENUM_MP3_PLAYER,
/* 16 */	MVAPP_ENUM_UPGRADE,
/* 17 */	MVAPP_ENUM_CI,
/* 18 */	MVAPP_ENUM_CAS,
/* 19 */	MVAPP_ENUM_BACKUP,
/* 20 */	MVAPP_ENUM_RESTORE,
/* 21 */	MVAPP_ENUM_PLUG_IN,
/* 22 */	MVAPP_ENUM_FACTORY_RESET,
/* 23 */	MVAPP_ENUM_NULL,
/* 24 */	MVAPP_ENUM_MAX
} EN_MENU_LIST;

#define MENU_ITEM_MAX			MVAPP_ENUM_MAX
#define	SERVICES_NUM_WIDTH		6
#define SERVICES_NUM_HEIGHT		4

typedef struct
{
	U8			u8Item_Index;
	U16			u16Image_Index;
	U16			u16Menu_Link;
	U16			u16Name_Index;
	U16			u16exp_Index;
} MV_Menu_Item_t;

#endif // #ifdef SMART_PHONE

tCS_DBU_Service		stRecall_Service[MV_RECALL_MAX_ITEM];
U16					u16MV_stUseIndex[MV_LIST_NUMBER][MV_MAX_SERVICE_COUNT];
char				acFont_Name[10][10];
U8					u8Font_Kind;
BOOL				First_Tunning_State;

U16 CS_DB_GetListServiceNumber(tCS_DB_ServiceListTriplet  ListTriplet);
U8 MV_DB_Get_SatIndex_By_Satindex(U8 u8SatIndex);
U8 MV_DB_Get_SatIndex_By_TPindex(U16 u16TPIndex);
U8 MV_DB_Get_SatIndex_By_Chindex(U16 u16ChIndex);
tCS_DB_Error CS_DB_SetCurrentList(tCS_DB_ServiceListTriplet  ListTriplet, BOOL Force);
void  CS_DB_GetCurrentListTriplet(tCS_DB_ServiceListTriplet  * ListTriplet);
tCS_DB_Error CS_DB_GetCurrentList_ServiceData(tCS_DB_ServiceManageData* service_data, U16 index_inlist);
U16 CS_DB_GetCurrent_ServiceIndex_By_Index(U16 index_inlist);
U16  CS_DB_GetCurrentList_ServiceNum(void);
tCS_DB_Error CS_DB_SetCurrentService_OrderIndex(U16 index_inlist);
U16  CS_DB_GetCurrentService_OrderIndex(void);
U16   CS_DB_GetCurrentServiceIndex(void);
U16 CS_DB_SetNextService_OrderIndex(void);
U16 CS_DB_SetPreviousService_OrderIndex(void);
void CS_DB_SetLastServiceTriplet(void);
tCS_DBU_Service CS_DB_GetLastServiceTriplet(void);

tCS_DB_Error CS_DB_ModifyCurrentService_DeleteFlag(tCS_DB_DeleteStatus flag , U16 index_inlist);
tCS_DB_Error CS_DB_SortCurrentServiceList(tCS_DB_SortType SortType);
tCS_DB_Error CS_DB_CurrentList_MoveService(U16 from_inlist, U16 to_inlist);
tCS_DB_Error CS_DB_SaveListModifications (void);
tCS_DB_Error CS_DB_RemoveFavoriteService(U16 pServiceIndex, U8 u8TvRadio, U8 favoriteindex);
tCS_DB_Error CS_DB_GetSlistName(char *name, U8 ListIndex);
U32 CS_DB_GetCurrentTerRegionIndex(void);
tCS_DB_Error CS_DB_SetLCNMode(tCS_DB_LCNMode LCNmode);
tCS_DB_LCNMode CS_DB_GetLCNMode(void);
tCS_DB_Error CS_DB_DeleteTerTP (U16 pTPIndexToDeleted);
U32	CS_DB_GetCurrentTerTPNumber(void);
tCS_DB_Error CS_DB_LoadDatabase(void);
tCS_DB_Error CS_DB_SaveDatabase(void);
BOOL  CS_DB_CheckIfChanged(void);
tCS_DB_Error CS_DB_EraseDatabase (void);
tCS_DB_Error CS_DB_ResetDatabase (void);
tCS_DB_Error CS_DB_ResetDB (void);
tCS_DB_Error	CS_DBU_InitDefaultUserData(void);
tCS_DB_Error	CS_DBU_SaveUserSettingDataInHW(void);
tCS_DB_Error	CS_DBU_LoadUserSettingDataInHW(void);
BOOL CS_DBU_CheckIfUserSettingDataChanged(void);
tCS_DB_Error  CS_DBU_SetScartMode(tCS_DBU_ScartMode ScartMode);
tCS_DBU_ScartMode	CS_DBU_GetScartMode(void);
tCS_DB_Error  CS_DBU_SetSpdifStatus(tCS_DBU_Status SpdifStatus);
tCS_DBU_Status	CS_DBU_GetSpdifStatus (void);
tCS_DB_Error CS_DBU_SetVideoOutput( tCS_DBU_VideoOutput VideoOutputType );
tCS_DBU_VideoOutput  CS_DBU_GetVideoOutput( void );
tCS_DB_Error CS_DBU_SetVideoDefinition ( tCS_DBU_VideoDefinition VideoDefinition );
tCS_DBU_VideoDefinition CS_DBU_GetVideoDefinition ( void );
tCS_DB_Error  CS_DBU_SetVideoEncodingMode(tCS_DBU_VideoEncodingMode pMode);
tCS_DBU_VideoEncodingMode	CS_DBU_GetVideoEncodingMode(void);
tCS_DB_Error  CS_DBU_SetVideoAspectRatio(tCS_DBU_VideoAspectRatio pMode);
tCS_DBU_VideoAspectRatio	CS_DBU_GetVideoAspectRatio(void);
tCS_DB_Error  CS_DBU_SetAspectRatioMode (tCS_DBU_AspectRatioMode pMode);
tCS_DBU_AspectRatioMode CS_DBU_GetAspectRatioMode (void);
//tCS_DBU_LanguageStructure*CS_DBU_GetAvailableServiceLanguage(U16*totalnumber);
//tCS_DBU_LanguageStructure* CS_DBU_GetAvailableMenuLanguage(U16 *totalnumber);
tCS_DB_Error  CS_DBU_SetDefaultAudioType(tCS_DBU_AudioType audio_type);
tCS_DBU_AudioType CS_DBU_GetDefaultAudioType(void);
tCS_DB_Error  CS_DBU_SetAudioLanguage(U32 u32AudioLang);
U32 CS_DBU_GetAudioLanguage(void);
tCS_DB_Error  CS_DBU_SetSubtitleLanguage(U32 u32SubLang);
U32 CS_DBU_GetSubtitleLanguage(void);
tCS_DB_Error  CS_DBU_SetMenuLanguage(U32 u32MenuLang);
U32 CS_DBU_GetMenuLanguage(void);
tCS_DB_Error  CS_DBU_SetTimeOffset(U32 OffsetUTC);
U32  CS_DBU_GetTimeOffset (void);
tCS_DB_Error  CS_DBU_SetTimeMode(tCS_DBU_TimeMode TimeMode);
tCS_DBU_TimeMode  CS_DBU_GetTimeMode(void);
tCS_DB_Error  CS_DBU_SetTimeRegion(U32 region_index);
U32  CS_DBU_GetTimeRegion(void);
tCS_DB_Error  CS_DBU_SetPinCode(char * PinCode);
char * CS_DBU_GetPinCode(void);
tCS_DB_Error  CS_DBU_Set_Webaddr(char * Webaddr);
char * CS_DBU_Get_Webaddr(void);

/* By KB Kim for Plugin Setting : 2011.05.07 */
tCS_DB_Error  CS_DBU_Set_PlugInAddr(char *plugInAddr);
char * CS_DBU_Get_PlugInAddr(void);

tCS_DB_Error  CS_DBU_SetInstallLockStatus(tCS_DBU_Status LockStatus);
tCS_DBU_Status	CS_DBU_GetInstallLockStatus (void);
tCS_DB_Error  CS_DBU_SetBootLockStatus(tCS_DBU_Status LockStatus);
tCS_DBU_Status	CS_DBU_GetBootLockStatus (void);
tCS_DB_Error  CS_DBU_SetEditLockStatus(tCS_DBU_Status LockStatus);
tCS_DBU_Status	CS_DBU_GetEditLockStatus(void);
tCS_DB_Error  CS_DBU_SetServicesLockStatus(tCS_DBU_Status LockStatus);
tCS_DBU_Status	CS_DBU_GetServicesLockStatus(void);
tCS_DB_Error  CS_DBU_SetParentalLockStatus(tCS_DBU_Status LockStatus);
tCS_DBU_Status	CS_DBU_GetParentalLockStatus(void);
tCS_DB_Error  CS_DBU_SetParentalRate(U32 age);
U32	CS_DBU_GetParentalRate(void);
tCS_DB_Error  CS_DBU_SetScreenTransparent(U32  trans);
U32  CS_DBU_GetScreenTransparent(void);
tCS_DB_Error  CS_DBU_SetBannerKeepTime(U32 banner_time);
U32 	CS_DBU_GetBannerKeepTime(void);
tCS_DB_Error  CS_DBU_SetLNB_Power(U32 lnb_power);
U32 	CS_DBU_GetLNB_Power(void);
tCS_DB_Error  CS_DBU_SetCH_Change_Type(U32 change_type);
U32	CS_DBU_GetCH_Change_Type(void);
tCS_DB_Error  CS_DBU_SetCH_List_Type(U32 list_type);
U32	CS_DBU_GetCH_List_Type(void);
tCS_DB_Error  CS_DBU_SetDNS_Type(U32 DNS_Status);
U32	CS_DBU_GetDNS_Type(void);
tCS_DB_Error  CS_DBU_SetDHCP_Type(U32 DHCP_Status);
U32	CS_DBU_GetDHCP_Type(void);
tCS_DB_Error CS_DBU_SetLCNMode(tCS_DB_LCNMode LCNmode);
tCS_DB_LCNMode CS_DBU_GetLCNMode(void);

tCS_DB_Error  CS_DBU_SetVolume(U32  Volume);
U32  CS_DBU_GetVolume(void);
tCS_DB_Error  CS_DBU_LoadVolume(void);
tCS_DB_Error CS_DBU_SaveVolume(void);

tCS_DB_Error  CS_DBU_SetMuteStatus(tCS_DBU_Status MuteStatus);
tCS_DBU_Status	CS_DBU_GetMuteStatus(void);
tCS_DB_Error CS_DBU_SaveMuteStatus(void);
tCS_DB_Error  CS_DBU_LoadMuteStatus(void);

tCS_DB_Error  CS_DBU_SaveCurrentService(tCS_DBU_Service ServiceTriplet);
tCS_DB_Error CS_DBU_LoadCurrentService(tCS_DBU_Service *ServiceTriplet);

BOOL	CS_DB_FLASH_CRCCheck_Write(U32 size, U8 *buffer, U8 Kind);
BOOL	CS_DB_FLASH_CRCCheck_DataIfChanged(U32 size, U8 *buffer, U8 Kind);
BOOL CS_DB_FLASH_CRCCheck_Read(U32 size, U8 *buffer, U8 Kind);
tCS_DB_Error MV_DB_AddOneService(MV_stServiceInfo pMvServiceData, U16 *pServiceIndex);
U16 MV_DB_Get_ServiceAllList_Index(tCS_DB_ServiceListType chlist_type, U16 u16ChIndex);
tCS_DB_Error MV_DB_GetServiceDataByIndex(MV_stServiceInfo *pServiceData, U32 pServiceIndex);
tCS_DB_Error MV_DB_DeleteOneService(U16 pServiceIndex);
void MV_UseDB_AddOneServiceIndex(tCS_DB_ServiceType pMvSrvType, U16 u16Index);
void MV_UseDB_DeleteOneServiceIndex(U8 pMvSrvType, U16 u16Index);
tCS_DB_Error MV_UserDB_FindIndexNoByIndex(U8 pMvSrvType, U16 pServiceIndex, U16 *ServIdx);
U16 MV_DB_GetALLServiceNumber(void);
tCS_DB_Error MV_DB_DeleteAllService(void);
tCS_DB_Error MV_DB_RenameServiceName(char * serviceName,U16 pServiceIndex);
tCS_DB_Error MV_DB_ModifyServiceAudio(U16 AudioPid, tCS_DB_AudioType AudioType, U16 pServiceIndex);
tCS_DB_Error MV_DB_ModifyAudioMode(MV_DB_AudioMode AudioMode, U16 pServiceIndex);
tCS_DB_Error MV_DB_ModifyServiceVideoPid(U16 VideoPid, U16 pServiceIndex);
tCS_DB_Error MV_DB_ModifyServicePcrPid(U16 PcrPid, U16 pServiceIndex);
U8 MV_DB_CheckFavoriteServiceBySrvIndex(U8 u8TVRadio, U16 pServiceIndex , U8 u8favoriteindex);
U8 MV_DB_FindFavoriteServiceBySrvIndex(U8 u8TVRadio, U16 u16ChIndex);
void MV_GetSatelliteData(MV_stSatInfo *SatInfo);
void MV_GetSatelliteData_ByIndex(MV_stSatInfo *SatInfo, U8 SatIndex);
void MV_DB_Get_TPdata_By_Satindex_and_TPnumber(MV_stTPInfo *TPInfo, U8 u8Satindex, U8 u8TPnumber);
U8 MV_DB_Get_TPCount_By_Satindex(U8 u8Satindex);
tCS_DB_Error MV_DB_GetTPDataByIndex(MV_stTPInfo * pTPData, U16 pTPIndex);
tCS_DB_Error CS_DB_Save_CH_Database(void);
tCS_DB_Error CS_DB_Save_SAT_Database(void);
tCS_DB_Error CS_DB_Save_INDEX_Database(void);
void MV_DB_Get_Default_CHData(void);
U16 MV_GetSatelliteData_Num(void);
U8 MV_DB_Get_NextSatIndex_In_ChList(U32 u32ChList_Type, U8 u8SatIndex);
U8 MV_DB_Get_PrevSatIndex_In_ChList(U32 u32ChList_Type, U8 u8SatIndex);
U16 CS_DB_Add_TP(void);
void MV_SetSatelliteData_By_SatIndex(MV_stSatInfo SatInfo);
U8	MV_GetSelected_SatData_Count(void);
tCS_DB_Error MV_GetSelected_SatData_By_Count(MV_stSatInfo *Temp_SatData, U8 u8ReCount );
BOOL MV_DB_Get_SatData_By_Chindex(MV_stSatInfo *SatInfo, U16 u16ChIndex);
void MV_DB_Get_TPdata_By_ChNum(MV_stTPInfo *TPInfo, U16 u16Chnumber);
U16 MV_DB_Get_TPIndex_By_Chindex(U16 u16ChIndex);
void Load_CH_UseIndex(void);
void MV_DB_Get_Favorite_Name(char *Fav_name, U8 u8Fav_Index);
void MV_DB_Set_Favorite_Name(char *Fav_name, U8 u8Fav_Index);
#if 0
tCS_DB_Error MV_DB_ModifyServiceLockStatus(tCS_DB_LockStatus lock , U16 pServiceIndex);
#else
tCS_DB_Error MV_DB_ModifyServiceLockStatus(tCS_DB_LockStatus flag , U16 index_inlist);
#endif
U8 MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(U8 u8Satindex, U16 u16TPIndex);
void MV_SetTPData_By_TPIndex(MV_stTPInfo TPInfo, U16 u16TPIndex);
void MV_GetTPData_By_TPIndex(MV_stTPInfo *TPInfo, U16 u16TPIndex);
tCS_DB_Error compare_add_tp ( MV_stTPInfo *TPInfo );

/* For Blind Scan By KB Kim 2011.02.26 */
U16 GetTpIndexByData (MV_stTPInfo *tPInfo);
tCS_DB_Error MV_UdateBlindTpData(MV_stTPInfo *tPInfo);

void MV_SetSatelliteData(MV_stSatInfo *SatInfo);
U16 CS_DB_Get_AllTPCount(void);
void MV_ADD_TPData(MV_stTPInfo TPInfo);
void MV_SetTPData_ADD_TPIndex(MV_stTPInfo TPInfo);
#ifdef ONE_ITEM_MOVE
void MV_DB_ChangeService_OrderIndex(U32 u32Key, U16 index, U8 u8Num_Per_Page);
#else // #ifdef ONE_ITEM_MOVE
U16 MV_DB_ChangeService_OrderIndex(U16	u16NowIndex);
#endif  // #ifdef ONE_ITEM_MOVE
tCS_DB_Error CS_DB_ModifyCurrentService_MoveFlag(tCS_DB_DeleteStatus flag , U16 index_inlist);
U16 CS_DB_GetCurrent_Service_By_ServiceIndex(U16 index_inlist);
U16 MV_DB_Get_TPIndex_By_Satindex_and_TPnumber(U8 u8Satindex, U8 u8TPnumber);
U8 MV_Get_Satindex_by_Seq(U8 u8Num);
U16	MV_Get_ServiceCount_at_Favorite(U8 u8TVRadio, U8 u8FavIndex);
U16	MV_Get_ServiceCount_at_Sat(U8 u8SatIndex);
tCS_DB_Error MV_DB_AddFavoriteService_Select(U8 favoriteindex);
U8 MV_Get_Searched_SatCount(void);
U8 MV_Get_Favindex_by_Seq(U8 u8TVRadio, U8 u8Num);
tCS_DB_Error CS_DB_ModifyCurrentService_SelectFlag(tCS_DB_DeleteStatus flag , U16 index_inlist);
tCS_DB_Error MV_DB_AddFavoriteService_Select_Clear(void);
BOOL MV_DB_Get_Moving_Flag(void);
void MV_DB_Set_Moving_Flag(BOOL b8flag);
void MV_DB_Set_Service_Move(void);
void MV_DB_Set_Service_Lock(void);

void MV_DB_Set_TPdata_By_Satindex_and_TPnumber(MV_stTPInfo *TPInfo, U8 u8Satindex, U8 u8TPnumber);
void MV_SetTPData_DEL_TPIndex(MV_stTPInfo TPInfo);
tCS_DB_Error  CS_DBU_SetAntenna_Type(U32 Antena_Type);
U32	CS_DBU_GetAntenna_Type(void);
tCS_DB_Error  CS_DBU_SetLocal_Longitude(U32 Local_Longitude);
U32	CS_DBU_GetLocal_Longitude(void);
tCS_DB_Error  CS_DBU_SetLocal_Latitude(U32 Local_latitude);
U32	CS_DBU_GetLocal_Latitude(void);
tCS_DB_Error  CS_DBU_SetRecall_Type(U32 recall_type);
U32	CS_DBU_GetRecall_Type(void);
tCS_DB_Error  CS_DBU_SetPower_Type(U32 Power_type);
U32	CS_DBU_GetPower_Type(void);
U32	CS_DBU_GetLED_Type(void);
tCS_DB_Error  CS_DBU_SetLED_Type(U32 LED_type);
tCS_DB_Error CS_DBU_SetANI_Type(U8 Ani_type);
U8	CS_DBU_GetANI_Type(void);

/* For Heart bit control By KB Kim 2011.03.11 */
tCS_DB_Error  CS_DBU_SetHeartBit(U8 enable);
U8	CS_DBU_GetHeartBit (void);

U32	CS_DBU_GetPower_Off_Mode(void);
tCS_DB_Error  CS_DBU_SetPower_Off_Mode(U32 Power_Off_Mode);
void MV_DB_Set_Replace_Index(void);
tCS_DB_Error CS_DB_RestoreDatabase (void);
void MV_Get_ReCall_List(tCS_DBU_Service *stTemp_Recall);
tCS_DB_Error MV_Set_ReCall_List(tCS_DBU_Service stTemp_Recall);
tCS_DB_Error MV_Reset_ReCall_List(void); /* By KB Kim for Multi-Recall list problem : 2011.08.30 */
U16 MV_DB_GetServiceindex_ByOrderIndex(tCS_DB_ServiceListType tcs_Orderindex, U32 u32ListType, U16 u16Serviceindex);
tCS_DB_Error MV_DB_AddFavoriteService(U8 favoriteindex, U8 u8TvRadio, U16 u16ServiceIndex);
void MV_SetServiceData_By_Chindex(MV_stServiceInfo *ServiceInfo, U16 u16Service_Index);
U16 MV_Get_Find_List(U16 *Find_List, char *Find_String);
tCS_DB_Error MV_Get_Find_Current_Service_Order(U16 u16Find_Ch_Index, U16 *u16Return_service);
U8 MV_GetSelected_SatIndex_By_Count(U8 u8ReCount );
U8 MV_GetSelected_Index_By_Satindex(U8 u8Index );
void MV_DEL_ALLTPSave(void);
void MV_SetTPData_DEL_ALLTPIndex(MV_stTPInfo TPInfo);
U16	MV_Get_TVServiceCount_at_Sat(U8 u8SatIndex);
U16	MV_Get_RDServiceCount_at_Sat(U8 u8SatIndex);
tCS_DB_Error CS_DB_All_Ch_Delete (void);
void MV_SetCHData_DEL_BySatellite(U8 u8Delete_Sat_index);
void CS_DB_Set_TVLastServiceTriplet(void);
tCS_DBU_Service CS_DB_Get_TVLastServiceTriplet(void);
void CS_DB_Set_RadioLastServiceTriplet(void);
tCS_DBU_Service CS_DB_Get_RadioLastServiceTriplet(void);
tCS_DB_Error  CS_DBU_SetFont_Type(U32 Font_Type);
U32	CS_DBU_GetFont_Type(void);
tCS_DB_Error  CS_DBU_SetFont_Size(U32 Font_Size);
U32	CS_DBU_GetFont_Size(void);
tCS_DB_Error  CS_DBU_SetFont_Kind(U32 Font_Kind);
U32	CS_DBU_GetFont_Kind(void);
U8	CS_DBU_Get_Skin(void);
tCS_DB_Error  CS_DBU_Set_Skin(U8 u8Skin_Kind);
U8	CS_DBU_Get_Slepp(void);
tCS_DB_Error  CS_DBU_Set_Sleep(U8 u8Sleep_Kind);
U32	CS_DBU_Get_Push_Game_Level(void);
tCS_DB_Error  CS_DBU_Set_Push_Game_Level(U32 Game_Level);
U32	CS_DBU_Get_Motor_Limit(void);
tCS_DB_Error  CS_DBU_Set_Motor_Limit(U32 u32OnOff);
U8	CS_DBU_Get_Use_SubTitle(void);
tCS_DB_Error  CS_DBU_Set_Use_SubTitle(U8 u8Subtitle);
U8	CS_DBU_Get_Fixed_Font_Size(void);
tCS_DB_Error  CS_DBU_Set_Fixed_Font_Size(U8 u8FontSize);

/* For First search channel By KB Kim 2011.01.19 */
void MV_DB_Add_Temp_Service(U16 *firstChIndex);
tCS_DB_Error MV_DB_Temp_AddOneService(MV_stServiceInfo pMvServiceData, U16 *pServiceIndex);
U32 MV_DB_Temp_Init_AddService(void);
void MV_DB_Reset_Temp_AddService(void);
U16	MV_DB_Temp_AddService_Count(void);
U16	CS_DB_Get_Sub_Count(void);
tCS_DB_Error CS_DB_Restore_CH_UseIndex (void);

#ifdef SMART_PHONE
void Mv_Default_Menu_Item( MV_Menu_Item_t *MV_Menu_Item);
#endif // #ifdef SMART_PHONE

#ifdef __cplusplus
}
#endif

#endif 


