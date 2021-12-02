#include "linuxos.h"
#include "sys_setup.h"
#include "e2p_data.h"
#include "crc.h"
#include "database.h"
#include "database_priv.h"
#include "sattp.h"
#include "default_data.h"
#include "e2p.h"
#include "cs_app_common.h"
/* By KB Kim 2011.01.18 */
#include "mvosapi.h"
#include "ctype.h"

#define							TEMP_DB

#define 						CH_DB		0
#define							SAT_DB		1
#define 						INDEX_DB	2

CSAPP_Applet_t                  TmpApplet = CSApp_Applet_Desktop;

static  CSOS_Semaphore_t 		*sem_ServiceListAccess = NULL;
static  CSOS_Semaphore_t 		*sem_TPListAccess = NULL;
static  CSOS_Semaphore_t 		*sem_DB_FLASH_Access = NULL;
static 	CSOS_Semaphore_t		*sem_DBU_E2P_Access = NULL;
static	char					*cServiceListName[MV_LIST_NUMBER + MV_MAX_FAV_KIND];

/* By KB Kim 2011.01.19 */
U8                              DbMemoryMapInitial       = 1;

U32 							CH_Flash_map_to_ram_Size = 0;
#ifdef TEMP_DB
U32								Temp_CH_Flash_map_to_ram_Size = 0;
#endif
U32 							INDEX_Flash_map_to_ram_Size = 0;
U32 							SAT_Flash_map_to_ram_Size = 0;
U8 								*CH_Flash_map_to_ram_Base = 0;
#ifdef TEMP_DB
U8 								*Temp_CH_Flash_map_to_ram_Base = 0;
#endif
U8 								*INDEX_Flash_map_to_ram_Base = 0;
U8 								*SAT_Flash_map_to_ram_Base = 0;

U16								*List_ServiceOrder[MV_LIST_NUMBER + MV_MAX_FAV_KIND];
tCS_DB_ServiceListInfo			*pServiceListInfo = 0;
tCS_DBU_UserSetting_Save		UserSetting_Data;
U32								User_Volume;
tCS_DBU_Status					User_Mute;
static tCS_DB_ServiceList		Current_ServiceList;
static tCS_DBU_Service			Last_Service;
static tCS_DBU_Service			Last_TVService;
static tCS_DBU_Service			Last_RadioService;
static char						Pin_code_buffer[5];
static char 					Website[64];
static char 					PlugInAddr[64]; /* By KB Kim for Plugin Setting : 2011.05.07 */
static U16						List_CurrentServiceIndex[MV_LIST_NUMBER][MV_MAX_LIST_NUMBER];
static U8						u8Global_u8SatSearchIndex[MV_SAT_MAX];
static MV_stGB_ListIndex		u16Global_SatListIndex[MV_SAT_MAX*2];
static U8						u8Sort_Mode[MV_LIST_NUMBER][MV_MAX_LIST_NUMBER];

MV_stIndex						*u16ChIndex_Glob=0;
MV_stServiceInfo				*stCHInfo_Glob=0;
MV_stSatInfo					*stSatInfo_Glob=0;
MV_stTPInfo						*stTPInfo_Glob=0;
MV_stFavInfo					*stFavInfo_Glob=0;
#ifdef TEMP_DB
tCS_DB_ServiceListInfo			*Temp_pServiceListInfo = 0;
MV_stIndex						*u16TempChIndex_Glob=0;
MV_stServiceInfo				*stTempCHInfo_Glob=0;
MV_stFavInfo_Temp_Old			*stTempFavInfo_Glob=0;
#endif
MV_stDB_ListIndex				*stSat_ListIndex=0;
BOOL							b8MovingFlag = FALSE;

#ifdef NEW_INSTALL
/* By KB Kim 2011.01.18 */
#define		TEMP_CH_MAX			MV_MAX_SERVICE_COUNT

U16								u16Temp_Search_Count = 0;
/* By KB Kim 2011.01.18 */
MV_stServiceInfo				*stTemp_CHInfo = NULL;
#endif

#ifdef SMART_PHONE
MV_Menu_Item_t	Default_MV_Menu_Item[MENU_ITEM_MAX] = {
					{ MVAPP_ENUM_INSTALLATION, 			MVBMP_MUTE_ICON, 		CSApp_Applet_Install, 		CSAPP_STR_INSTALLATION, 		CSAPP_STR_INSTALLATION},
					{ MVAPP_ENUM_SAT_SETTING, 			MVBMP_PAUSE_ICON, 		CSApp_Applet_Sat_Setting, 	CSAPP_STR_SATELLITE_SETTING, 	CSAPP_STR_SATELLITE_SETTING},
					{ MVAPP_ENUM_TP_SETTING, 			MVBMP_VOLUME_ICON, 		CSApp_Applet_TP_Setting, 	CSAPP_STR_TP_SETTING, 			CSAPP_STR_TP_SETTING},
					{ MVAPP_ENUM_LANG_SETTING, 			MVBMP_ANI_DISC1, 		CSApp_Applet_Language, 		CSAPP_STR_MENU_LANG, 			CSAPP_STR_MENU_LANG},
					{ MVAPP_ENUM_TIME_SETTING, 			MVBMP_ANI_DISC2, 		CSApp_Applet_TimeSetting, 	CSAPP_STR_TIME_SETTING, 		CSAPP_STR_TIME_SETTING},
					{ MVAPP_ENUM_SYSTEM_SETTING, 		MVBMP_ANI_DISC3, 		CSapp_Applet_SystemSetting,	CSAPP_STR_SYSTEM_SETTING, 		CSAPP_STR_SYSTEM_SETTING},
					{ MVAPP_ENUM_AV_SETTING, 			MVBMP_CHLIST_INFO_ICON, CSApp_Applet_AVSetting,		CSAPP_STR_AV_SETTING, 			CSAPP_STR_AV_SETTING},
					{ MVAPP_ENUM_PIN_SETTING, 			MVBMP_CHLIST_INFO_ICON, CSApp_Applet_PinSetting,	CSAPP_STR_PIN_SETTING, 			CSAPP_STR_PIN_SETTING},
					{ MVAPP_ENUM_NETWORK_SETTING, 		MVBMP_CHLIST_INFO_ICON, CSApp_Applet_NetSetting,	CSAPP_STR_NET_SETTING, 			CSAPP_STR_NET_SETTING},
					{ MVAPP_ENUM_SYSTEM_INFORMATION, 	MVBMP_CHLIST_INFO_ICON, CSapp_Applet_SystemInfo,	CSAPP_STR_SYSTEM_INFO, 			CSAPP_STR_SYSTEM_INFO},
					{ MVAPP_ENUM_RECORDED_FILE, 		MVBMP_CHLIST_INFO_ICON, CSApp_Applet_Storage_Info,	CSAPP_STR_STORAGE_INFO, 		CSAPP_STR_STORAGE_INFO},
					{ MVAPP_ENUM_RECORD_CONFIG, 		MVBMP_CHLIST_INFO_ICON, CSApp_Applet_Upgrade,		CSAPP_STR_UPGRADE, 				CSAPP_STR_UPGRADE},
					{ MVAPP_ENUM_FILE_TOOLS, 			MVBMP_MUTE_ICON, 		CSApp_Applet_Backup,		CSAPP_STR_BACKUP, 				CSAPP_STR_BACKUP},
					{ MVAPP_ENUM_USB_REMOVE, 			MVBMP_PAUSE_ICON, 		CSApp_Applet_Restore,		CSAPP_STR_BACKUP_RE, 			CSAPP_STR_BACKUP_RE},
					{ MVAPP_ENUM_STORAGE_INFO, 			MVBMP_VOLUME_ICON, 		CSApp_Applet_PlugIn,		CSAPP_STR_PLUG_IN, 				CSAPP_STR_PLUG_IN},
					{ MVAPP_ENUM_MP3_PLAYER, 			MVBMP_ANI_DISC1, 		CSApp_Applet_Reset,			CSAPP_STR_DEFAULT_FACTORY, 		CSAPP_STR_DEFAULT_FACTORY},
					{ MVAPP_ENUM_UPGRADE, 				MVBMP_ANI_DISC2, 		CSApp_Applet_Calendar,		CSAPP_STR_CALENDAR, 			CSAPP_STR_CALENDAR},
					{ MVAPP_ENUM_CI, 					MVBMP_ANI_DISC3, 		CSApp_Applet_Push,			CSAPP_STR_GAME, 				CSAPP_STR_GAME},
					{ MVAPP_ENUM_CAS, 					MVBMP_CHLIST_INFO_ICON, CSAPP_Applet_KeyEdit,		CSAPP_STR_EDIT, 				CSAPP_STR_EDIT},
					{ MVAPP_ENUM_BACKUP, 				MVBMP_CHLIST_INFO_ICON, CSAPP_Applet_Desk_CH_Edit,	CSAPP_STR_CH_CHANGE, 			CSAPP_STR_CH_CHANGE},
					{ MVAPP_ENUM_RESTORE, 				MVBMP_CHLIST_INFO_ICON, CSApp_Applet_ExtInfo,		CSAPP_STR_EXTEND, 				CSAPP_STR_EXTEND},
					{ MVAPP_ENUM_PLUG_IN, 				MVBMP_CHLIST_INFO_ICON, CSApp_Applet_Timer,			CSAPP_STR_TIMER, 				CSAPP_STR_TIMER},
					{ MVAPP_ENUM_FACTORY_RESET, 		MVBMP_CHLIST_INFO_ICON, CSApp_Applet_Change_Fav,	CSAPP_STR_FAV_KEY, 				CSAPP_STR_FAV_KEY},
					{ MVAPP_ENUM_NULL, 					MVBMP_CHLIST_INFO_ICON, CSApp_Applet_MainMenu,		CSAPP_STR_MAINMENU, 			CSAPP_STR_MAINMENU},
				};

U8		Default_Menu_Item[MAIN_ITEM_MAX][MENU_ITEM_MAX] = {
				{	MVAPP_ENUM_INSTALLATION,
					MVAPP_ENUM_SAT_SETTING,
					MVAPP_ENUM_TP_SETTING,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL },
				{	MVAPP_ENUM_LANG_SETTING,
					MVAPP_ENUM_TIME_SETTING,
					MVAPP_ENUM_SYSTEM_SETTING,
					MVAPP_ENUM_AV_SETTING,
					MVAPP_ENUM_PIN_SETTING,
					MVAPP_ENUM_NETWORK_SETTING,
					MVAPP_ENUM_SYSTEM_INFORMATION,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL },
				{	MVAPP_ENUM_RECORDED_FILE,
					MVAPP_ENUM_RECORD_CONFIG,
					MVAPP_ENUM_FILE_TOOLS,
					MVAPP_ENUM_USB_REMOVE,
					MVAPP_ENUM_STORAGE_INFO,
					MVAPP_ENUM_MP3_PLAYER,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL },
				{	MVAPP_ENUM_UPGRADE,
					MVAPP_ENUM_CI,
					MVAPP_ENUM_CAS,
					MVAPP_ENUM_BACKUP,
					MVAPP_ENUM_RESTORE,
					MVAPP_ENUM_PLUG_IN,
					MVAPP_ENUM_FACTORY_RESET,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL,
					MVAPP_ENUM_NULL }
				};
#endif // #ifdef SMART_PHONE

void Check_SatList(void);

void MV_GetSameTPChannel_By_ChIndex(U16 u16ChannelIndex)
{
	int 			i;
	int				j;
	U16				u16Temp_TPIndex = 0;
	/* By KB Kim 2011.01.20 */
	U16             noOfChannel;

	/* By KB Kim 2011.01.20 */
	noOfChannel = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];
	/* By KB Kim 2011.01.20 */
	for( i = 0 ; i < noOfChannel ; i++ )
	{
		if ( stCHInfo_Glob[i].u16ChIndex == Current_ServiceList.Service_Data[u16ChannelIndex].Service_Index )
		{
			u16Temp_TPIndex = stCHInfo_Glob[i].u16TransponderIndex;
			break;
		}
	}

	/* By KB Kim 2011.01.20 */
	for( i = 0 ; i < noOfChannel ; i++ )
	{
		if ( stCHInfo_Glob[i].u16TransponderIndex == u16Temp_TPIndex )
		{
			for( j = 0 ; j < Current_ServiceList.Service_Number ; j++ )
			{
				if( Current_ServiceList.Service_Data[j].Service_Index == stCHInfo_Glob[i].u16ChIndex )
					break;
			}

			//if ( AddEpgChannel( j , stCHInfo_Glob[i].u16ServiceId ) == eCS_DB_ERROR )
			// 	break;
		}
	}
}

U16	MV_Get_ServiceCount_at_Sat(U8 u8SatIndex)
{
	return u16Global_SatListIndex[u8SatIndex].u16ChCount + u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount;
}

U16	MV_Get_TVServiceCount_at_Sat(U8 u8SatIndex)
{
	return u16Global_SatListIndex[u8SatIndex].u16ChCount;
}

U16	MV_Get_RDServiceCount_at_Sat(U8 u8SatIndex)
{
	return u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount;
}

U16	MV_Get_ServiceCount_at_Favorite(U8 u8TVRadio, U8 u8FavIndex)
{
	return stFavInfo_Glob[u8FavIndex].u8FavCount[u8TVRadio];
}

U8 MV_Get_Searched_SatCount(void)
{
	U8	i;
	U8	count = 0;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
		{
			count++;
		}
	}

	return count;
}

U8 MV_Get_Satindex_by_Seq(U8 u8Num)
{
	U8	i;
	U8	count = 0;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
		{
			count++;
		}

		if ( count == u8Num )
			break;
	}

	return i;
}

U8 MV_Get_Favindex_by_Seq(U8 u8TVRadio, U8 u8Num)
{
	U8	i;
	U8	count = 0;

	for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Favorite(u8TVRadio, i) > 0 )
		{
			if ( count == u8Num )
				break;
			else
				count++;
		}
	}

	// printf("MV_Get_Favindex_by_Seq : %d ==> %d ====\n", u8Num, i);
	return i;
}

void MV_Init_SatList(void)
{
	int i, j;
	int tvcount = 0, rdcount = 0;
	/* By KB Kim 2011.01.20 */
	U16             noOfChannel;

	/* By KB Kim 2011.01.20 */
	noOfChannel = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( u8Global_u8SatSearchIndex[i] == TRUE )
		{
			tvcount = 0;
			rdcount = 0;
			u16Global_SatListIndex[i].u16ChCount = stSat_ListIndex->u16ChCount[i];
			u16Global_SatListIndex[i + MV_SAT_MAX].u16ChCount = stSat_ListIndex->u16ChCount[i + MV_SAT_MAX];

			/* By KB Kim 2011.01.20 */
			for ( j = 0 ; j < noOfChannel ; j++ )
			{
				//printf("MV_Init_SatList :: %d === %d \n", stSat_ListIndex->stSatIndexInfo[j].u8SatIndexIndex, stSat_ListIndex->stSatIndexInfo[j].u16ChIndex);
				if ( stSat_ListIndex->stSatIndexInfo[j].u16SatIndexIndex == i )
				{
					u16Global_SatListIndex[i].stChIndex[tvcount] = stSat_ListIndex->stSatIndexInfo[j].u16ChIndex;
					tvcount++;
				}
				else if ( stSat_ListIndex->stSatIndexInfo[j].u16SatIndexIndex == ( i + MV_SAT_MAX ) )
				{
					u16Global_SatListIndex[i+MV_SAT_MAX].stChIndex[rdcount] = stSat_ListIndex->stSatIndexInfo[j].u16ChIndex;
					rdcount++;
				}

				if ( ( tvcount + rdcount ) == ( u16Global_SatListIndex[i].u16ChCount + u16Global_SatListIndex[i + MV_SAT_MAX].u16ChCount ) )
					break;
			}
		}
	}

	for ( i = 0 ; i < MV_LIST_NUMBER ; i++ )
		for ( j = 0 ; j < MV_MAX_LIST_NUMBER ; j++ )
			u8Sort_Mode[i][j] = eCS_DB_BY_NORMAL;
}

void MV_Store_SatListIndex(void)
{
	int i,j;
	U16 ChCount = 0;

	memcpy ( &stSat_ListIndex->u8SatSearchIndex ,&u8Global_u8SatSearchIndex, MV_SAT_MAX );

	//Check_SatList();

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		stSat_ListIndex->u16ChCount[i] = u16Global_SatListIndex[i].u16ChCount;
		stSat_ListIndex->u16ChCount[i + MV_SAT_MAX] = u16Global_SatListIndex[i + MV_SAT_MAX].u16ChCount;

		for ( j = 0 ; j < stSat_ListIndex->u16ChCount[i] ; j++ )
		{
			stSat_ListIndex->stSatIndexInfo[ChCount].u16ChIndex = u16Global_SatListIndex[i].stChIndex[j];
			stSat_ListIndex->stSatIndexInfo[ChCount].u16SatIndexIndex = i;
			ChCount++;
		}

		for ( j = 0 ; j < stSat_ListIndex->u16ChCount[i + MV_SAT_MAX] ; j++ )
		{
			stSat_ListIndex->stSatIndexInfo[ChCount].u16ChIndex = u16Global_SatListIndex[i + MV_SAT_MAX].stChIndex[j];
			stSat_ListIndex->stSatIndexInfo[ChCount].u16SatIndexIndex = i + MV_SAT_MAX;
			//printf("%d ChCount ::: %d th Sat : RADIO : %d ====> %d\n", ChCount, i, stSat_ListIndex->u16ChCount[i + MV_SAT_MAX], stSat_ListIndex->stSatIndexInfo[ChCount].u16ChIndex);
			ChCount++;
		}
	}
}

void MV_DB_Get_Favorite_Name(char *Fav_name, U8 u8Fav_Index)
{
	sprintf(Fav_name, "%s", stFavInfo_Glob[u8Fav_Index].acFavName);
}

void MV_DB_Set_Favorite_Name(char *Fav_name, U8 u8Fav_Index)
{
	sprintf(stFavInfo_Glob[u8Fav_Index].acFavName, "%s", Fav_name);
}


void	MV_DB_Get_Default_SatTPData()
{
	int			i=0, j=0, k=0;
	int         tpCount;

	//printf("\n\n ########### Default Database Sat_TP %d ##############\n\n", MV_SAT_MAX);

	tpCount = 0;
	while ( i < MV_SAT_MAX )
	{
		//u8Sat_Index = stSat_DefaultList[i].u8SatelliteIndex;
		// for ( k = 0 ; k < MV_SAT_MAX ; k++ )
		k = 0;
		while ((k < MV_SAT_MAX) && (stSat_DefaultList[k].u8SatelliteIndex < MV_SAT_MAX))
		{
			if ( stSat_DefaultList[k].u8SatelliteIndex == i )
			{
				stSatInfo_Glob[i] = stSat_DefaultList[k];
				break;
			}
			k++;
		}

		j = 0;
		while (( j < MAX_TP_COUNT ) && ( stTP_DefaultList[j].u8SatelliteIndex < MV_MAX_SATELLITE_COUNT))
		{
			if ( stTP_DefaultList[j].u8SatelliteIndex == stSatInfo_Glob[i].u8SatelliteIndex )
			{
				stTPInfo_Glob[tpCount] = stTP_DefaultList[j];
				tpCount++;
			}
			j++;
		}
		i++;
	}

	i = 0;
	while( i < MAX_TP_COUNT )
	{
		if ( stTP_DefaultList[i].u8SatelliteIndex == MV_MAX_SATELLITE_COUNT )
		{
			// printf ("tpCount : %d / %d\n", tpCount, i);
			stTPInfo_Glob[tpCount] = stTP_DefaultList[i];
			tpCount++;
			break;
		}
		i++;
	}

	//printf("\n\n ########### Default SAT Database ENDDING ##############\n\n");
}

void	MV_DB_Get_Default_CHData()
{
	int			i=0;

	//printf("\n\n ########### Default Database CH ##############\n\n");

	pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] = 0;
	pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID] = 0;

	i = 0;
	while( i < 8 )
	{
		sprintf(stFavInfo_Glob[i].acFavName, "%s" , acFavName_DefaultList[i]);
		i++;
	}
/*
	while ( i < 1 )
	{
		MV_DB_AddOneService(stCh_DefaultList[i], &IIndex);
		i++;
	}
*/
	//printf("\n\n ########### Default CH Database ENDDING ##############\n\n");
}

void MV_GetSatelliteData(MV_stSatInfo *SatInfo)
{

	int		i=0;

	while ( i < MV_SAT_MAX )
	{
		SatInfo[i] = stSatInfo_Glob[i];
		i++;
	}

	//memcpy(stSatInfo_Glob, SatInfo, sizeof(MV_stSatInfo) * MV_SAT_MAX);
}

void MV_Get_Selected_SatelliteData(MV_stSatInfo *SatInfo)
{

	int		i=0;

	while ( i < MV_SAT_MAX )
	{
		if ( stSatInfo_Glob[i].u16Select == SAT_SELECT )
			SatInfo[i] = stSatInfo_Glob[i];
		i++;
	}

	//memcpy(stSatInfo_Glob, SatInfo, sizeof(MV_stSatInfo) * MV_SAT_MAX);
}

void MV_SetServiceData_By_Chindex(MV_stServiceInfo *ServiceInfo, U16 u16Service_Index)
{
	int 		i;
	memcpy(&stCHInfo_Glob[u16Service_Index], ServiceInfo, sizeof(MV_stServiceInfo));
	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		if ( stCHInfo_Glob[u16Service_Index].u16ChIndex == Current_ServiceList.Service_Data[i].Service_Index )
		{
			Current_ServiceList.Service_Data[i].Lock_Flag = stCHInfo_Glob[u16Service_Index].u8Lock;
			break;
		}
	}
}

void MV_SetSatelliteData(MV_stSatInfo *SatInfo)
{

	int		i=0;

	while ( i < MV_SAT_MAX )
	{
		stSatInfo_Glob[i] = SatInfo[i];
		i++;
	}

	//memcpy(stSatInfo_Glob, SatInfo, sizeof(MV_stSatInfo) * MV_SAT_MAX);
	CS_DB_Save_SAT_Database();
}

void MV_SetSatelliteData_By_SatIndex(MV_stSatInfo SatInfo)
{

	int		i=0;

	while ( i < MV_SAT_MAX )
	{
		if ( stSatInfo_Glob[i].u8SatelliteIndex == SatInfo.u8SatelliteIndex )
			stSatInfo_Glob[i] = SatInfo;
		i++;
	}

	//memcpy(stSatInfo_Glob, SatInfo, sizeof(MV_stSatInfo) * MV_SAT_MAX);
	//CS_DB_Save_SAT_Database();
}

void MV_SetTPData_By_TPIndex(MV_stTPInfo TPInfo, U16 u16TPIndex)
{

	int		i=0;

	while ( i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u16TPIndex == u16TPIndex)
		{
			stTPInfo_Glob[i] = TPInfo;
			break;
		}
		else
			i++;
	}

	//memcpy(stSatInfo_Glob, SatInfo, sizeof(MV_stSatInfo) * MV_SAT_MAX);
	CS_DB_Save_SAT_Database();
}

void MV_GetTPData_By_TPIndex(MV_stTPInfo *TPInfo, U16 u16TPIndex)
{

	int		i=0;

	while ( i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u16TPIndex == u16TPIndex)
		{
			*TPInfo = stTPInfo_Glob[i];
			break;
		}
		else
			i++;
	}
}

void MV_ADD_TPData(MV_stTPInfo TPInfo)
{
	int		i=0;

	while ( i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u8Valid == DB_INVALID || stTPInfo_Glob[i].u8SatelliteIndex == MV_SAT_MAX )
		{
//			TPInfo.u16TPIndex = stTPInfo_Glob[i].u16TPIndex;

			if ( stTPInfo_Glob[i].u8SatelliteIndex == MV_SAT_MAX )
			{
				stTPInfo_Glob[i] = TPInfo;
				// printf("MV_ADD_TPData : %d u16TPIndex[%d] u8SatelliteIndex[%d]\n", i, stTPInfo_Glob[i].u16TPIndex, stTPInfo_Glob[i].u8SatelliteIndex);

				if ((i + 1) < MAX_TP_COUNT)
				{
				stTPInfo_Glob[i+1].u8SatelliteIndex = MV_SAT_MAX;
				stTPInfo_Glob[i+1].u16TPIndex = TPInfo.u16TPIndex + 1;
				memset(stTPInfo_Glob[i+1].acTPName, 0x00, MAX_SERVICE_NAME_LENGTH);
				stTPInfo_Glob[i+1].u16NID = 0xFFFF;
				stTPInfo_Glob[i+1].u16SymbolRate = 0;
				stTPInfo_Glob[i+1].u16TPFrequency = 0;
				stTPInfo_Glob[i+1].u16TSID = 0xFFFF;
				stTPInfo_Glob[i+1].u8Polar_H = P_H;
				stTPInfo_Glob[i+1].u8Unused = 0;
				stTPInfo_Glob[i+1].u8Valid = DB_VALID;
				}
			} else
				stTPInfo_Glob[i] = TPInfo;
			break;
		}
		else
			i++;
	}
}

void MV_SetTPData_ADD_TPIndex(MV_stTPInfo TPInfo)
{
	MV_ADD_TPData(TPInfo);
	CS_DB_Save_SAT_Database();
}

void MV_SetTPData_DEL_TPIndex(MV_stTPInfo TPInfo)
{
	int					i=0;
	tCS_DBU_Service     ServiceTriplet;

	i = MV_DB_GetALLServiceNumber();

	/* By KB Kim 2011.01.21 */
	while ( i > 0 )
	{
		/* By KB Kim 2011.01.21 */
		i--;

		if ( stCHInfo_Glob[i].u16TransponderIndex == TPInfo.u16TPIndex )
		{
			MV_DB_DeleteOneService(stCHInfo_Glob[i].u16ChIndex);
		}
	}

	MV_DB_Set_Replace_Index();

	while ( i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u16TPIndex == TPInfo.u16TPIndex )
		{
#if 1
			memcpy (&stTPInfo_Glob[i], &stTPInfo_Glob[i+1], sizeof(MV_stTPInfo) * ( MAX_TP_COUNT - i - 1));
#else
			stTPInfo_Glob[i].u8SatelliteIndex = MV_SAT_MAX;
			stTPInfo_Glob[i].u16TPFrequency   = 0x0000;
			stTPInfo_Glob[i].u16SymbolRate    = 0x0000;
			stTPInfo_Glob[i].u8Polar_H        = P_H;
			stTPInfo_Glob[i].u8Unused         = 0;
			stTPInfo_Glob[i].u8Valid          = DB_INVALID;
			stTPInfo_Glob[i].u16TSID          = 0x0000;
			stTPInfo_Glob[i].u16NID           = 0x0000;
			memset(stTPInfo_Glob[i].acTPName, 0x00, MAX_SERVICE_NAME_LENGTH);
#ifdef FOR_USA
			stTPInfo_Glob[i].u16OrgNID        = 0x0000;
#endif /* #ifdef FOR_USA */
#endif
			break;
		}
		else
			i++;
	}

	CS_DB_SaveDatabase();

	//CS_DB_LoadDatabase();
	CS_DB_RestoreDatabase ();
	CS_DBU_LoadCurrentService(& ServiceTriplet);
	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList,TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
}

void MV_SetTPData_DEL_ALLTPIndex(MV_stTPInfo TPInfo)
{
	int					i=0;

	while ( i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u16TPIndex == TPInfo.u16TPIndex )
		{
			memcpy (&stTPInfo_Glob[i], &stTPInfo_Glob[i+1], sizeof(MV_stTPInfo) * ( MAX_TP_COUNT - i - 1));
			break;
		}
		else
			i++;
	}

	/* By KB Kim 2011.01.21 */
	i = MV_DB_GetALLServiceNumber();

	while ( i > 0 )
	{
		/* By KB Kim 2011.01.21 */
		i--;

		if ( stCHInfo_Glob[i].u16TransponderIndex == TPInfo.u16TPIndex )
		{
			MV_DB_DeleteOneService(stCHInfo_Glob[i].u16ChIndex);
		}
	}
}

void MV_DEL_ALLTPSave(void)
{
	tCS_DBU_Service     ServiceTriplet;

	MV_DB_Set_Replace_Index();

	CS_DB_SaveDatabase();

	//CS_DB_LoadDatabase();
	Load_CH_UseIndex();
	CS_DBU_LoadCurrentService(& ServiceTriplet);
	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList,TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
}

void MV_SetCHData_DEL_BySatellite(U8 u8Delete_Sat_index)
{
	int					i = 0, j = 0;
	MV_stSatInfo		Temp_SatData;
	tCS_DBU_Service     ServiceTriplet;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( MV_Get_ServiceCount_at_Sat(i) > 0 )
		{
			if ( j == u8Delete_Sat_index )
				break;
			j++;
		}
	}

	//MV_GetSelected_SatData_By_Count(&Temp_SatData, i);
	MV_GetSatelliteData_ByIndex(&Temp_SatData, i);

	printf("\n=== %d =>>> %s , %d ==== \n", i, Temp_SatData.acSatelliteName, Temp_SatData.u8SatelliteIndex );

	/* By KB Kim 2011.01.21 */
	i = MV_DB_GetALLServiceNumber();

	while ( i > 0 )
	{
		/* By KB Kim 2011.01.21 */
		i--;

		if ( MV_DB_Get_SatIndex_By_TPindex(stCHInfo_Glob[i].u16TransponderIndex) == Temp_SatData.u8SatelliteIndex)
		{
			MV_DB_DeleteOneService(stCHInfo_Glob[i].u16ChIndex);
		}
	}

#if 0
	{
		U8		u8FavCount = 0;

		for( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
		{
			/* By KB Kim 2011.01.20 */
			u8FavCount = stFavInfo_Glob[i].u8FavCount[0];
			if (u8FavCount > 0)
			{
				for( j = 0 ; j < u8FavCount; j++ )
				{
					printf("=== %d : %d , %d : %d, %s\n", u8FavCount, i, j, stFavInfo_Glob[i].u16FavChIndex[0][j], stCHInfo_Glob[stFavInfo_Glob[i].u16FavChIndex[0][j]].acServiceName);
				}
			}
		}
	}
#endif

	MV_DB_Set_Replace_Index();

#if 0
	{
		U8		u8FavCount = 0;

		for( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
		{
			/* By KB Kim 2011.01.20 */
			u8FavCount = stFavInfo_Glob[i].u8FavCount[0];
			if (u8FavCount > 0)
			{
				for( j = 0 ; j < u8FavCount; j++ )
				{
					printf("=== %d : %d , %d : %d, %s\n", u8FavCount, i, j, stFavInfo_Glob[i].u16FavChIndex[0][j], stCHInfo_Glob[stFavInfo_Glob[i].u16FavChIndex[0][j]].acServiceName);
				}
			}
		}
	}
#endif

	CS_DB_Save_CH_Database();
	CS_DB_Save_INDEX_Database();

	//CS_DB_LoadDatabase();
	Load_CH_UseIndex();
	CS_DBU_LoadCurrentService(& ServiceTriplet);
	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList,TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
}

void MV_SetCHIndex_DEL_ByFavorite(U8 u8Delete_Fav_index)
{
	tCS_DBU_Service     ServiceTriplet;

	stFavInfo_Glob[u8Delete_Fav_index].u8FavCount[kCS_DB_DEFAULT_TV_LIST_ID] = 0;
	stFavInfo_Glob[u8Delete_Fav_index].u8FavCount[kCS_DB_DEFAULT_RADIO_LIST_ID] = 0;
	memset(stFavInfo_Glob[u8Delete_Fav_index].u16FavChIndex, 0x00, MV_LIST_NUMBER*MV_MAX_FAV_COUNT*2 );

	CS_DB_Save_CH_Database();
	CS_DB_Save_INDEX_Database();
	Load_CH_UseIndex();
	CS_DBU_LoadCurrentService(& ServiceTriplet);
	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList,TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);
}

U8	MV_GetSelected_SatData_Count(void)
{
	U8	u8Count = 0;
	U8	i;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
		if ( stSatInfo_Glob[i].u16Select == SAT_SELECT )
			u8Count++;

	return u8Count;
}

tCS_DB_Error MV_GetSelected_SatData_By_Count(MV_stSatInfo *Temp_SatData, U8 u8ReCount )
{
	U8	u8Count=0;
	U8	i;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( stSatInfo_Glob[i].u16Select == SAT_SELECT )
		{
			if ( u8Count == u8ReCount )
				break;

			u8Count++;
		}
	}
	memcpy (Temp_SatData, &stSatInfo_Glob[i], sizeof(MV_stSatInfo));
	return eCS_DB_OK;
}

U8 MV_GetSelected_SatIndex_By_Count(U8 u8ReCount )
{
	U8	u8Count=0;
	U8	i;

	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		if ( stSatInfo_Glob[i].u16Select == SAT_SELECT )
		{
			if ( u8Count == u8ReCount )
				break;

			u8Count++;
		}
	}

	return i;
}

U8 MV_GetSelected_Index_By_Satindex(U8 u8Index )
{
	U8	u8Count=0;
	U8	i;

	for ( i = 0 ; i < u8Index ; i++ )
		if ( stSatInfo_Glob[i].u16Select == SAT_SELECT )
			u8Count++;

	return u8Count;
}

U16 MV_GetSatelliteData_Num()
{
	/*
	U16				i = 0;
	MV_stSatInfo	MV_CheckSat_Data[MV_MAX_SATELLITE_COUNT];

	memset (&MV_CheckSat_Data, 0, sizeof (MV_stSatInfo) * MV_MAX_SATELLITE_COUNT);
	MV_GetSatelliteData(MV_CheckSat_Data);

	while ( strlen(MV_CheckSat_Data[i].acSatelliteName) > 0 )
	{
		i++;
	}
	*/
	return MV_MAX_SATELLITE_COUNT;
}

void MV_GetSatelliteData_ByIndex(MV_stSatInfo *SatInfo, U8 SatIndex)
{
	int		i=0;

	SatInfo->u8SatelliteIndex = MV_SAT_MAX;

	while ( i < MV_SAT_MAX )
	{
		if ( stSatInfo_Glob[i].u8SatelliteIndex == SatIndex )
		{
#if 1
			*SatInfo = stSatInfo_Glob[i];
#else
			SatInfo->acSatelliteName 		= stSatInfo_Glob[i].acSatelliteName;
			SatInfo->s16Longitude 			= stSatInfo_Glob[i].s16Longitude;
			SatInfo->u16Control12V	 		= stSatInfo_Glob[i].u16Control12V;
			SatInfo->u16DiSEqC 				= stSatInfo_Glob[i].u16DiSEqC;
			SatInfo->u16LNBPower 			= stSatInfo_Glob[i].u16LNBPower;
			SatInfo->u16LocalFrequency 		= stSatInfo_Glob[i].u16LocalFrequency;
			SatInfo->u16LocalFrequency_High = stSatInfo_Glob[i].u16LocalFrequency_High;
			SatInfo->u16Tone22K 			= stSatInfo_Glob[i].u16Tone22K;
			SatInfo->u8LNBType 				= stSatInfo_Glob[i].u8LNBType;
			SatInfo->u8MotorPosition 		= stSatInfo_Glob[i].u8MotorPosition;
			SatInfo->u8SatelliteIndex 		= stSatInfo_Glob[i].u8SatelliteIndex;
#endif
			return;
		} else
			i++;
	}
}

U8 MV_DB_Get_SatIndex_By_Satindex(U8 u8SatIndex)
{
	U8			i=0;

	for ( i = 0 ; i < MV_MAX_SATELLITE_COUNT ; i++ )
	{
		if( stSatInfo_Glob[i].u8SatelliteIndex == u8SatIndex )
		{
			break;
		}
	}

	return i;
}

U8 MV_DB_Get_SatIndex_By_TPindex(U16 u16TPIndex)
{
	U16			j=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u16TPIndex == u16TPIndex )
			break;
	}

	return MV_DB_Get_SatIndex_By_Satindex(stTPInfo_Glob[j].u8SatelliteIndex);
}

U8 MV_DB_Get_TPCount_By_Satindex(U8 u8Satindex)
{
	U16			j=0;
	U8			i=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u8SatelliteIndex == MV_SAT_MAX )
		{
			break;
		}
		else if( stTPInfo_Glob[j].u8SatelliteIndex == u8Satindex )
		{
			i++;
		}
	}

	return i;
}

void MV_DB_Get_TPdata_By_ChNum(MV_stTPInfo *TPInfo, U16 u16Chnumber)
{
	U16			j=0;
	U16			u16TP_Index;

	u16TP_Index = MV_DB_Get_TPIndex_By_Chindex(u16Chnumber);

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u16TPIndex== u16TP_Index )
		{
			*TPInfo = stTPInfo_Glob[j];
			break;
		}
	}
}

void MV_DB_Get_TPdata_By_Satindex_and_TPnumber(MV_stTPInfo *TPInfo, U8 u8Satindex, U8 u8TPnumber)
{
	U16			j=0, i=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u8SatelliteIndex == u8Satindex )
		{
			if (i == u8TPnumber )
			{
				//printf("\n %d : FOUND : i = %d =========>>>>>>>>\n", j, i);
				*TPInfo = stTPInfo_Glob[j];
				//printf(" %d / %d / %d <<<==>>>  %d / %d / %d =========>>>>>>>>\n", stTPInfo_Glob[j].u16TPFrequency, stTPInfo_Glob[j].u8Polar_H, stTPInfo_Glob[j].u16SymbolRate, TPInfo->u16TPFrequency, TPInfo->u8Polar_H, TPInfo->u16SymbolRate);
				break;
			}
			i++;
		}
	}
}

void MV_DB_Set_TPdata_By_Satindex_and_TPnumber(MV_stTPInfo *TPInfo, U8 u8Satindex, U8 u8TPnumber)
{
	U16			j=0, i=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u8SatelliteIndex == u8Satindex )
		{
			if (i == u8TPnumber )
			{
				stTPInfo_Glob[j] = *TPInfo;
				break;
			}
			i++;
		}
	}
}

U16 MV_DB_Get_TPIndex_By_Satindex_and_TPnumber(U8 u8Satindex, U8 u8TPnumber)
{
	U16			j=0, i=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u8SatelliteIndex == u8Satindex )
		{
			if (i == u8TPnumber )
				break;
			i++;
		}
	}
	return stTPInfo_Glob[j].u16TPIndex;
}

U8 MV_DB_Get_TPNumber_By_SatIndex_and_TPIndex(U8 u8Satindex, U16 u16TPIndex)
{
	U16			j=0;
	U8			i=0;

	for ( j = 0 ; j < MAX_TP_COUNT ; j++ )
	{
		if( stTPInfo_Glob[j].u8SatelliteIndex == u8Satindex )
		{
			if (stTPInfo_Glob[j].u16TPIndex == u16TPIndex )
			{
				break;
			}
			i++;
		}
	}
	return i;
}

U8 MV_DB_Get_SatIndex_By_Chindex(U16 u16ChIndex)
{
	U16			j=0;
	/* By KB Kim 2011.01.20 */
	U16         noOfChannel;

	/* By KB Kim 2011.01.20 */
	noOfChannel = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];
	/* By KB Kim 2011.01.20 */
	for ( j = 0 ; j < noOfChannel ; j++ )
		if( stCHInfo_Glob[j].u16ChIndex == u16ChIndex )
			break;

	/* By KB Kim 2011.01.20 */
	if ( j >= noOfChannel )
	{
		/* By KB Kim 2011.01.20 */
		return MV_MAX_SATELLITE_COUNT;
	}
	else
	{
		return MV_DB_Get_SatIndex_By_TPindex(stCHInfo_Glob[j].u16TransponderIndex);
	}
}

BOOL MV_DB_Get_SatData_By_Chindex(MV_stSatInfo *SatInfo, U16 u16ChIndex)
{
	U16			j=0;

	for ( j = 0 ; j < MV_MAX_SERVICE_COUNT ; j++ )
		if( stCHInfo_Glob[j].u16ChIndex == u16ChIndex )
			break;

	if ( j >= MV_MAX_SERVICE_COUNT )
		return 0;
	else
	{
		MV_GetSatelliteData_ByIndex(SatInfo, MV_DB_Get_SatIndex_By_TPindex(stCHInfo_Glob[j].u16TransponderIndex));
	}

	return TRUE;
}


U16 MV_DB_Get_TPIndex_By_Chindex(U16 u16ChIndex)
{
	U16			j=0;

	for ( j = 0 ; j < MV_MAX_SERVICE_COUNT ; j++ )
		if( stCHInfo_Glob[j].u16ChIndex == u16ChIndex )
			break;

	return stCHInfo_Glob[j].u16TransponderIndex;
}

U8 MV_DB_Get_NextSatIndex_In_ChList(U32 u32ChList_Type, U8 u8SatIndex)
{
	U16			j=0;
	U8			u32List_Type = 0;
	U8			u8Find_Satindex = 255;

	// printf("MV_DB_Get_NextSatIndex_In_ChList : u32ChList_Type[%d] u8SatIndex [%d]\n", u32ChList_Type, u8SatIndex);

	switch(u32ChList_Type)
	{
		case eCS_DB_TV_LIST:
			u8SatIndex = 255;  // kb : 100331
		case eCS_DB_FAV_TV_LIST:
		case eCS_DB_SAT_TV_LIST:
		default:
			u32List_Type = kCS_DB_DEFAULT_TV_LIST_ID;
			break;
		case eCS_DB_RADIO_LIST:
			u8SatIndex = 255;  // kb : 100331
		case eCS_DB_FAV_RADIO_LIST:
		case eCS_DB_SAT_RADIO_LIST:
			u32List_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
			break;
	}

	if ( u8SatIndex == 255 )
	{
		for ( j = 0 ; j < MV_MAX_SATELLITE_COUNT ; j++ )
		{
			if ( u32List_Type == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			} else {
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j + MV_SAT_MAX].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			}
		}
	} else {
		for ( j = u8SatIndex+1 ; j < MV_MAX_SATELLITE_COUNT ; j++ )
		{
			if ( u32List_Type == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			} else {
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j + MV_SAT_MAX].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			}
		}
	}

	return u8Find_Satindex;
}

U8 MV_DB_Get_PrevSatIndex_In_ChList(U32 u32ChList_Type, U8 u8SatIndex)
{
	int			j=0;
	U8			u32List_Type = 0;
	U8			u8Find_Satindex = 255;

	switch(u32ChList_Type)
	{
		case eCS_DB_TV_LIST:
		case eCS_DB_FAV_TV_LIST:
		case eCS_DB_SAT_TV_LIST:
		default:
			u32List_Type = kCS_DB_DEFAULT_TV_LIST_ID;
			break;
		case eCS_DB_RADIO_LIST:
		case eCS_DB_FAV_RADIO_LIST:
		case eCS_DB_SAT_RADIO_LIST:
			u32List_Type = kCS_DB_DEFAULT_RADIO_LIST_ID;
			break;
	}

	if ( u8SatIndex > 0 )
	{
		for ( j = u8SatIndex-1 ; j >= 0 ; j-- )
		{
			if ( u32List_Type == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			} else {
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j + MV_SAT_MAX].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			}
		}
	}

	if ( u8Find_Satindex == 255 )
	{
		for ( j = MV_MAX_SATELLITE_COUNT ; j >= 0 ; j-- )
		{
			if ( u32List_Type == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			} else {
				if( u8Global_u8SatSearchIndex[j] == TRUE && u16Global_SatListIndex[j + MV_SAT_MAX].u16ChCount != 0)
				{
					u8Find_Satindex = j;
					break;
				}
			}
		}
	}

	return u8Find_Satindex;
}

BOOL CS_DB_FLASH_CRCCheck_Write(U32 size, U8 *buffer, U8 Kind)
{
	U32			crc_count;
	U8 *		tempbuffer = buffer;
	FILE*		fp_flash = NULL;
	int			retval;
	char 		ShellCommand[64];

	if(buffer == NULL)
		return(FALSE);

	if( access( DB_DIR, 0 ) != 0 )
	{
		memset(ShellCommand, 0x00, 64);
	 	sprintf( ShellCommand, "mkdir %s", DB_DIR );
	 	system( ShellCommand );
	}

	if ( Kind == CH_DB )
		fp_flash = fopen(CH_DB_FILE, "wb");
	else if ( Kind == SAT_DB )
		fp_flash = fopen(SAT_DB_FILE, "wb");
	else if ( Kind == INDEX_DB )
		fp_flash = fopen(INDEX_DB_FILE, "wb");

	if (fp_flash == NULL)
	{
		return(FALSE);
	}

	CS_CRC_32bCalculate(tempbuffer, size-4, &crc_count);
	*((U32 *)(tempbuffer+size-4)) = crc_count;

	retval = fwrite(tempbuffer , size,1,  fp_flash);
	//dprintf(("fwrite actual size = %d\n", retval));
	if (retval != 1)
	{
		dprintf(("Error: write database\n"));
		fclose(fp_flash);
		return -1;
	}

	fclose(fp_flash);
	return(TRUE);
}


BOOL CS_DB_FLASH_CRCCheck_DataIfChanged(U32 size, U8 *buffer, U8 Kind)
{
	U32 			crc_count;
	U32 			crc_read;
	U8 *			tempbuffer = buffer;
	FILE*			fp_flash = NULL;
	int				retval;

	if(tempbuffer == NULL)
		return(TRUE);

	CS_CRC_32bCalculate(tempbuffer, size-4, &crc_count);

	if ( Kind == CH_DB )
		fp_flash = fopen(CH_DB_FILE, "r");
	else if ( Kind == SAT_DB )
		fp_flash = fopen(SAT_DB_FILE, "r");
	else if ( Kind == INDEX_DB )
		fp_flash = fopen(INDEX_DB_FILE, "r");

	if (fp_flash == NULL)
	{
		return(FALSE);
	}

	retval = fseek(fp_flash, size-4, SEEK_SET);

	if (retval != 0)
	{
		dprintf(("Error: Seek database\n"));
		fclose(fp_flash);
		return(TRUE);
	}

	retval = fread(((U8 *)&crc_read), 4, 1, fp_flash);
	if (retval != 1)
	{
		dprintf(("Error: Read database\n"));
		fclose(fp_flash);
		return(TRUE);
	}

	if(crc_read != crc_count)
	{
		fclose(fp_flash);
		return(TRUE);
	}

	fclose(fp_flash);

	return(FALSE);
}


BOOL CS_DB_FLASH_CRCCheck_Read(U32 size, U8 *buffer, U8 Kind)
{
	U32			crc_count;
	U32			crc_read;
	U8 *		tempbuffer = buffer;
	FILE*		fp_flash = NULL;
	int			retval;

	if(tempbuffer == NULL)
		return(FALSE);

	if ( Kind == CH_DB )
		fp_flash = fopen(CH_DB_FILE, "r");
	else if ( Kind == SAT_DB )
		fp_flash = fopen(SAT_DB_FILE, "r");
	else if ( Kind == INDEX_DB )
		fp_flash = fopen(INDEX_DB_FILE, "r");

	if (fp_flash == NULL)
	{
		return(FALSE);
	}

	retval = fread(tempbuffer, size, 1, fp_flash);

	if (retval != 1)
	{
		// dprintf(("\n\n\n#### Kind : %d #### Error: Read database ###########\n\n\n", Kind));
		fclose(fp_flash);
		return(FALSE);
	}

	CS_CRC_32bCalculate(tempbuffer, size-4, &crc_count);
	crc_read = *((U32*)(tempbuffer + size-4));
	if(crc_read != crc_count)
	{
		fclose(fp_flash);
		return(FALSE);
	}

	fclose(fp_flash);
	return(TRUE);
}

BOOL CS_DB_FLASH_Erase(U8 Kind)
{
	FILE*       fp_flash = NULL;

	if ( Kind == CH_DB )
		fp_flash = fopen(CH_DB_FILE, "wb");
	else if ( Kind == SAT_DB )
		fp_flash = fopen(SAT_DB_FILE, "wb");
	else if ( Kind == INDEX_DB )
		fp_flash = fopen(INDEX_DB_FILE, "wb");

	if (fp_flash == NULL)
	{
		return(FALSE);
	}

	fclose(fp_flash);
	return(TRUE);
}

void Check_IndexDB(void)
{
	U16	TotalService, i;

	TotalService = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	dprintf((" TOTAL : %d , TV : %d , Radio : %d \n", TotalService , pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] , pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));

	for( i =0 ; i < TotalService ; i++ )
	{
		if ( u16ChIndex_Glob[i].u16Ch_Kind == kCS_DB_DEFAULT_TV_LIST_ID )
		{
			dprintf(("%d th = TV Index : %d\n", i, u16ChIndex_Glob[i].u16Index_Ch));
		} else {
			dprintf(("%d th = Radio Index : %d\n", i, u16ChIndex_Glob[i].u16Index_Ch));
		}
	}
}

void Check_SatList(void)
{
	int i, j;
	int tvcount = 0, rdcount = 0;

	// printf("################## Check_SatList #######################\n");
	for ( i = 0 ; i < MV_SAT_MAX ; i++ )
	{
		printf("===================== %d =======================\n", i);
		if ( u8Global_u8SatSearchIndex[i] == TRUE )
		{
			tvcount = u16Global_SatListIndex[i].u16ChCount;
			rdcount = u16Global_SatListIndex[i + MV_SAT_MAX].u16ChCount;
			// printf("=============== %d %d ===============\n", tvcount, rdcount);

			for ( j = 0 ; j < tvcount ; j++ )
			{
				printf ("%d TV %d == %d\n", i, j, u16Global_SatListIndex[i].stChIndex[j]);
			}

			for ( j = 0 ; j < rdcount ; j++ )
			{
				printf ("%d %d RADIO %d == %d\n", i, i + MV_SAT_MAX, j, u16Global_SatListIndex[i + MV_SAT_MAX].stChIndex[j]);
			}
		}
	}
}

void Load_CH_UseIndex(void)
{
	U16				i;
	U16				TotalService = 0, u16RadioCount = 0, u16TVCount = 0;

	CS_DB_LoadDatabase();

	TotalService = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	//dprintf(("############# %d : %d , %d #################\n", TotalService, pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));

	for( i =0 ; i < TotalService ; i++ )
	{
		if ( u16ChIndex_Glob[i].u16Ch_Kind == kCS_DB_DEFAULT_TV_LIST_ID )
		{
			u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][u16TVCount] = u16ChIndex_Glob[i].u16Index_Ch;
			u16TVCount++;
		} else {
			u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][u16RadioCount] = u16ChIndex_Glob[i].u16Index_Ch;
			u16RadioCount++;
		}
	}
	//dprintf(("############# TV %d : Radio %d #################\n", u16TVCount, u16RadioCount));
}

void Save_CH_Index_ByUseIndex(void)
{
	U16				i;
	U16				TotalService = 0, u16RadioCount = 0, u16TVCount = 0;

	u16TVCount = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID];
	u16RadioCount = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	//dprintf(("############# %d : %d , %d #################\n", TotalService, pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));

	for ( i = 0 ; i < u16TVCount ; i++ )
	{
		u16ChIndex_Glob[TotalService].u16Ch_Kind = kCS_DB_DEFAULT_TV_LIST_ID;
		u16ChIndex_Glob[TotalService].u16Index_Ch = u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i];
		TotalService++;
	}

	for ( i = 0 ; i < u16RadioCount ; i++ )
	{
		u16ChIndex_Glob[TotalService].u16Ch_Kind = kCS_DB_DEFAULT_RADIO_LIST_ID;
		u16ChIndex_Glob[TotalService].u16Index_Ch = u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i];
		TotalService++;
	}
	//dprintf(("############# TV %d : Radio %d #################\n", u16TVCount, u16RadioCount));
}

/**************************************************************************
*	Call : Sys_setup.c ( CS_DRV_Init ) -> Mwlayers.c ( CS_MW_Init ) -> cs_app_main.c ( MiniGUIMain ) ->
*	Parameter
*		bReset：True , False
*	Return
*		True, False
**************************************************************************/

BOOL MV_DB_Init(BOOL bReset)
{
	U16				i = 0;
	U8				*tempaddress, *sat_tempaddress, *index_tempaddress;
#ifdef TEMP_DB
	U8 				*Temp_tempaddress;
#endif
	tCS_DBU_Service	ServiceTriplet;

	//dprintf(("############# DB INIT START #################\n"));

	if ( bReset )
	{
		First_Tunning_State = TRUE;

		/* By KB Kim 2011.01.19 */
		if (DbMemoryMapInitial)
		{
			sem_ServiceListAccess  = CSOS_CreateSemaphoreFifo (NULL, 1 );
			sem_TPListAccess  = CSOS_CreateSemaphoreFifo (NULL, 1 );
			sem_DB_FLASH_Access  = CSOS_CreateSemaphoreFifo (NULL, 1 );
			sem_DBU_E2P_Access = CSOS_CreateSemaphoreFifo (NULL, 1 );

			CH_Flash_map_to_ram_Size = sizeof(MV_stServiceInfo) * MV_MAX_SERVICE_COUNT
								+ 4
								+ sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT
								+ 4
								+ sizeof(MV_stFavInfo) * MV_MAX_FAV_KIND
								+ 4
								+ sizeof(tCS_DB_ServiceListInfo)
								+ 4;
#ifdef TEMP_DB
			Temp_CH_Flash_map_to_ram_Size = sizeof(MV_stServiceInfo) * MV_MAX_SERVICE_COUNT
								+ 4
								+ sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT
								+ 4
								+ sizeof(MV_stFavInfo_Temp_Old) * MV_MAX_FAV_KIND
								+ 4
								+ sizeof(tCS_DB_ServiceListInfo)
								+ 4;
#endif

			SAT_Flash_map_to_ram_Size = sizeof(MV_stSatInfo) *MV_MAX_SATELLITE_COUNT
								+ 4
								+ sizeof(MV_stTPInfo) *MAX_TP_COUNT
								+ 4;

			INDEX_Flash_map_to_ram_Size = sizeof(MV_stDB_ListIndex) + 4;

			CH_Flash_map_to_ram_Base = (U8 *)CSOS_AllocateMemory(NULL,  CH_Flash_map_to_ram_Size);
#ifdef TEMP_DB
			Temp_CH_Flash_map_to_ram_Base = (U8 *)CSOS_AllocateMemory(NULL,  Temp_CH_Flash_map_to_ram_Size);
#endif
			SAT_Flash_map_to_ram_Base = (U8 *)CSOS_AllocateMemory(NULL,  SAT_Flash_map_to_ram_Size);
			INDEX_Flash_map_to_ram_Base = (U8 *)CSOS_AllocateMemory(NULL,  INDEX_Flash_map_to_ram_Size);

			if(CH_Flash_map_to_ram_Base == NULL)
			{
				dprintf(("database CH malloc memory error\n"));
				return FALSE;
			}
#ifdef TEMP_DB
			if(Temp_CH_Flash_map_to_ram_Base == NULL)
			{
				dprintf(("database Temp CH malloc memory error\n"));
				return FALSE;
			}
#endif
			if(SAT_Flash_map_to_ram_Base == NULL)
			{
				dprintf(("database SAT malloc memory error\n"));
				return FALSE;
			}

			if(INDEX_Flash_map_to_ram_Base == NULL)
			{
				dprintf(("database INDEX malloc memory error\n"));
				return FALSE;
			}

			tempaddress = CH_Flash_map_to_ram_Base;
			memset(CH_Flash_map_to_ram_Base, 0, CH_Flash_map_to_ram_Size);

#ifdef TEMP_DB
			Temp_tempaddress = Temp_CH_Flash_map_to_ram_Base;
			memset(Temp_CH_Flash_map_to_ram_Base, 0, Temp_CH_Flash_map_to_ram_Size);
#endif

			sat_tempaddress = SAT_Flash_map_to_ram_Base;
			memset(SAT_Flash_map_to_ram_Base, 0, SAT_Flash_map_to_ram_Size);

			index_tempaddress = INDEX_Flash_map_to_ram_Base;
			memset(INDEX_Flash_map_to_ram_Base, 0, INDEX_Flash_map_to_ram_Size);

			//dprintf(("############# INIT #################\n"));

			stSatInfo_Glob = (MV_stSatInfo*) sat_tempaddress;
			sat_tempaddress += sizeof(MV_stSatInfo) * MV_MAX_SATELLITE_COUNT;
			sat_tempaddress += 4;

			stTPInfo_Glob = (MV_stTPInfo*) sat_tempaddress;
			sat_tempaddress += sizeof(MV_stTPInfo) * MAX_TP_COUNT;
			sat_tempaddress += 4;

			stCHInfo_Glob = (MV_stServiceInfo*) tempaddress;
			tempaddress += sizeof(MV_stServiceInfo) * MV_MAX_SERVICE_COUNT;
			tempaddress += 4;

			u16ChIndex_Glob = (MV_stIndex*) tempaddress;
			tempaddress += sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT;
			tempaddress += 4;

			stFavInfo_Glob = (MV_stFavInfo*) tempaddress;
			tempaddress += sizeof(MV_stFavInfo) * MV_MAX_FAV_KIND;
			tempaddress += 4;

			pServiceListInfo = (tCS_DB_ServiceListInfo*) tempaddress;
			tempaddress += 4;

#ifdef TEMP_DB

			stTempCHInfo_Glob = (MV_stServiceInfo*) Temp_tempaddress;
			Temp_tempaddress += sizeof(MV_stServiceInfo) *MV_MAX_SERVICE_COUNT;
			Temp_tempaddress += 4;

			u16TempChIndex_Glob = (MV_stIndex*) Temp_tempaddress;
			Temp_tempaddress += sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT;
			Temp_tempaddress += 4;

			stTempFavInfo_Glob = (MV_stFavInfo_Temp_Old*) Temp_tempaddress;
			Temp_tempaddress += sizeof(MV_stFavInfo_Temp_Old) *MV_MAX_FAV_KIND;
			Temp_tempaddress += 4;

			Temp_pServiceListInfo = (tCS_DB_ServiceListInfo*) Temp_tempaddress;
			Temp_tempaddress += 4;

#endif

			stSat_ListIndex = (MV_stDB_ListIndex*) index_tempaddress;
			DbMemoryMapInitial = 0;
		}
		else
		{
			memset(CH_Flash_map_to_ram_Base, 0, CH_Flash_map_to_ram_Size);
			memset(SAT_Flash_map_to_ram_Base, 0, SAT_Flash_map_to_ram_Size);
			memset(INDEX_Flash_map_to_ram_Base, 0, INDEX_Flash_map_to_ram_Size);
		}

		memset(u8Global_u8SatSearchIndex, 0x00, MV_SAT_MAX);
		memset(u16Global_SatListIndex, 0x00, (sizeof(MV_stGB_ListIndex) * MV_SAT_MAX * 2));
/*
		for(i=0; i< (MV_LIST_NUMBER + MV_MAX_FAV_KIND); i++)
		{
			List_ServiceOrder[i] = (U16*) tempaddress;
			tempaddress += 2 * MV_MAX_SERVICE_COUNT;
		}
*/
		//dprintf(("############# LOAD DB #################\n"));

		Load_CH_UseIndex();

		// dprintf(("\n############# CH SIZE : %d #################\n", sizeof(MV_stServiceInfo)));
		// dprintf(("## Sat : %d , TP : %d , CH : %d ##\n", sizeof(MV_stSatInfo)*MV_MAX_SATELLITE_COUNT, sizeof(MV_stTPInfo) *MAX_TP_COUNT, sizeof(MV_stServiceInfo) *MV_MAX_SERVICE_COUNT));
		// dprintf(("## Index : %d , FAV : %d , Info : %d ##\n", sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT, sizeof(MV_stFavInfo) *MV_MAX_FAV_KIND, sizeof(tCS_DB_ServiceListInfo)));

#if 0
		//Check_IndexDB();

		//MV_DB_DeleteOneService(3);

		Check_IndexDB();

		dprintf(("\n###########################################\n"));

		for( i = 0 ; i < 6 ; i++ )
			dprintf((" SAT : %d : %20s\n", stSatInfo_Glob[i].u8SatelliteIndex, stSatInfo_Glob[i].acSatelliteName));

		for( i = 0 ; i < 12 ; i++ )
			dprintf((" TP : %d : %d : %d\n", stTPInfo_Glob[i].u16TPIndex, stTPInfo_Glob[i].u16TPFrequency, stTPInfo_Glob[i].u16SymbolRate));

		for( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
			dprintf((" FAV : %d : %20s\n", i, stFavInfo_Glob[i].acFavName));

		dprintf(("###########################################\n\n"));

#endif
		memcpy(&u8Global_u8SatSearchIndex, &stSat_ListIndex->u8SatSearchIndex, MV_SAT_MAX);

		MV_Init_SatList();

		//Check_SatList();

		for(i = 0 ; i < MV_MAX_LIST_NUMBER ; i++)
		{
			List_CurrentServiceIndex[0][i] = MV_DB_INVALID_SERVICE_INDEX;
		}
		for(i = 0 ; i < MV_MAX_LIST_NUMBER ; i++)
		{
			List_CurrentServiceIndex[1][i] = MV_DB_INVALID_SERVICE_INDEX;
		}

		Current_ServiceList.List_Triplet.sCS_DB_ServiceListType = eCS_DB_INVALID_LIST;
		Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue = 0;
		Current_ServiceList.Service_Data = NULL;
		Current_ServiceList.Service_Number = 0;
		Current_ServiceList.Current_Service = 0;

		CS_DBU_LoadUserSettingDataInHW();
		CS_DBU_LoadVolume();
		CS_DBU_LoadMuteStatus();
		CS_DBU_LoadCurrentService(& ServiceTriplet);

		/* By KB Kim for Multi-Recall list problem : 2011.08.30 */
		// memset(&Last_Service, 0, sizeof(tCS_DBU_Service));
		memset(&stRecall_Service, 0xFF, sizeof(tCS_DBU_Service)*MV_RECALL_MAX_ITEM);

		CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
		CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);

		/* By KB Kim for Multi-Recall list problem : 2011.08.30 */
		CS_DB_SetLastServiceTriplet();

		//printf("sCS_DBU_ServiceIndex = %d\n", ServiceTriplet.sCS_DBU_ServiceIndex);

		return TRUE;
	}else
		return FALSE;
}

void MV_UseDB_AddOneServiceIndex(tCS_DB_ServiceType pMvSrvType, U16 u16Index)
{
	u16ChIndex_Glob[pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]].u16Ch_Kind = pMvSrvType;
	u16ChIndex_Glob[pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]].u16Index_Ch = u16Index;

	if ( pMvSrvType == kCS_DB_DEFAULT_TV_LIST_ID )
	{
		//dprintf(("TV ADD Service #### %d : %d ########\n", pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], u16Index));
		u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID]] = u16Index;
	}
	else
	{
		//dprintf(("RD ADD Service #### %d : %d ########\n", pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID], u16Index));
		u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]] = u16Index;
	}
}

void MV_UseDB_DeleteOneServiceIndex(U8 pMvSrvType, U16 u16Index)
{
	int			i;

	for( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i++ )
	{
		if ( u16ChIndex_Glob[i].u16Ch_Kind == pMvSrvType && u16ChIndex_Glob[i].u16Index_Ch == u16Index )
		{
			memcpy(&u16ChIndex_Glob[i], &u16ChIndex_Glob[i + 1], sizeof(MV_stIndex)*(MV_MAX_SERVICE_COUNT - i - 1));
			break;
		}
	}

	if ( pMvSrvType == kCS_DB_DEFAULT_TV_LIST_ID )
	{
		for( i = 0 ; i < pServiceListInfo->u16DB_List_ServiceNum[pMvSrvType] ; i++ )
		{
			if ( u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i] == u16Index )
			{
				memcpy(&u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i], &u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i + 1], sizeof(U16)*(MV_MAX_SERVICE_COUNT - i - 1));
				break;
			}
		}
	}
	else
	{
		for( i = 0 ; i < pServiceListInfo->u16DB_List_ServiceNum[pMvSrvType] ; i++ )
		{
			if ( u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i] == u16Index )
			{
				memcpy(&u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i], &u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i + 1], sizeof(U16)*(MV_MAX_SERVICE_COUNT - i - 1));
				break;
			}
		}
	}
}

void MV_UseDB_DeleteOneServiceSatIndex(U8 pMvSrvType, U8 u8SatIndex, U16 u16Index)
{
	int		i;

	if ( pMvSrvType == kCS_DB_DEFAULT_TV_LIST_ID )
	{
		for ( i = 0 ; i < u16Global_SatListIndex[u8SatIndex].u16ChCount ; i++ )
		{
			if ( u16Global_SatListIndex[u8SatIndex].stChIndex[i] == u16Index )
			{
				memcpy(&u16Global_SatListIndex[u8SatIndex].stChIndex[i], &u16Global_SatListIndex[u8SatIndex].stChIndex[i+1], sizeof(U16)*(u16Global_SatListIndex[u8SatIndex].u16ChCount - i - 1));
				break;
			}
		}
	}
	else
	{
		for ( i = 0 ; i < u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount ; i++ )
		{
			if ( u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[i] == u16Index )
			{
				memcpy(&u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[i], &u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[i+1], sizeof(U16)*(u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount - i - 1));
				break;
			}
		}
	}
}

tCS_DB_Error MV_UserDB_FindIndexNoBySrvIndex(U16 pServiceIndex, U16 *ServIdx)
{
	U16 		i=0;

	for ( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i++ )
	{
		if ( stCHInfo_Glob[i].u16ChIndex == pServiceIndex  )
		{
			//dprintf(("########## SrvID : %d ====>>>>> %d\n", pServiceIndex, i));
			*ServIdx = i;
			return eCS_DB_OK;
		}
	}

	return eCS_DB_ERROR;
}

U8 MV_DB_CheckFavoriteServiceBySrvIndex(U8 u8TVRadio, U16 pServiceIndex , U8 u8favoriteindex)
{
	U8	i;
	/* By KB Kim 2011.01.20 */
	U8  favCount;

	/* By KB Kim 2011.01.20 */
	favCount = stFavInfo_Glob[u8favoriteindex].u8FavCount[u8TVRadio];
	if (favCount > 0)
	{
		for ( i = 0 ; i < favCount ; i++ )
		{
			if(stFavInfo_Glob[u8favoriteindex].u16FavChIndex[u8TVRadio][i] == pServiceIndex)
			return i;
		}
	}

	return MV_MAX_FAV_COUNT;
}

U8 MV_DB_FindFavoriteServiceBySrvIndex(U8 u8TVRadio, U16 u16ChIndex)
{
	U8	i, j;
	/* By KB Kim 2011.01.20 */
	U8  favCount;

	for ( j = 0 ; j < MV_MAX_FAV_KIND ; j++ )
	{
		/* By KB Kim 2011.01.20 */
		favCount = stFavInfo_Glob[j].u8FavCount[u8TVRadio];
		if(favCount > 0)
		{
			for ( i = 0 ; i < favCount ; i++ )
			{
				if(stFavInfo_Glob[j].u16FavChIndex[u8TVRadio][i] == u16ChIndex)
				{
					//printf("MV_DB_FindFavoriteServiceBySrvIndex : %d / %d\n", i, stFavInfo_Glob[j].u8FavCount[u8TVRadio]);
					return j;
				}
			}
		}
	}

	return MV_MAX_FAV_KIND;
}

#if 0 /* By KB Kim 2011.01.20 */
tCS_DB_Error MV_DB_AddOneService(MV_stServiceInfo pMvServiceData, U16 *pServiceIndex)
{
	U16			ServIdx = 0;
	U16			TotalService = 0;
	BOOL 		iffound = FALSE;
	U8			u8SatIndex = 0;

	tCS_DB_ServcieAddedMode servicestatus = eCS_DB_ADDSERVICE;

	/* By KB Kim : 2011.01.19 */
	u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(pMvServiceData.u16TransponderIndex);
	if( MAX_TP_COUNT <= pMvServiceData.u16TransponderIndex || MV_MAX_SATELLITE_COUNT <= u8SatIndex)
	{
		return eCS_DB_ERROR;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	TotalService = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	if(MV_MAX_SERVICE_COUNT <= TotalService)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	/* By KB Kim : 2011.01.20 */
	if (TotalService > 0)
	{
		for(ServIdx = 0; ServIdx <= TotalService; ServIdx++)
	{
		if(( pMvServiceData.u16TransponderIndex == stCHInfo_Glob[ServIdx].u16TransponderIndex))
		{
			if((pMvServiceData.u16ServiceId == stCHInfo_Glob[ServIdx].u16ServiceId)
				&&(stTPInfo_Glob[pMvServiceData.u16TransponderIndex].u16TSID == stTPInfo_Glob[stCHInfo_Glob[ServIdx].u16TransponderIndex].u16TSID)
				&&(stTPInfo_Glob[pMvServiceData.u16TransponderIndex].u16NID == stTPInfo_Glob[stCHInfo_Glob[ServIdx].u16TransponderIndex].u16NID)
				&&(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_VALID))
			{
				/* Modify Service : By KB Kim 2011.01.19 */
					printf("MV_DB_AddOneService : Fav flag[%d]\n", stCHInfo_Glob[ServIdx].u8Favor);
				pMvServiceData.u16ChIndex = stCHInfo_Glob[ServIdx].u16ChIndex;
				pMvServiceData.u8Lock = stCHInfo_Glob[ServIdx].u8Lock;
				pMvServiceData.u8Scramble = stCHInfo_Glob[ServIdx].u8Scramble;
				pMvServiceData.u8Watch = stCHInfo_Glob[ServIdx].u8Watch;
				pMvServiceData.u8Erase = stCHInfo_Glob[ServIdx].u8Erase;
				pMvServiceData.u8Favor = stCHInfo_Glob[ServIdx].u8Favor;
				pMvServiceData.u8Audio_ch = stCHInfo_Glob[ServIdx].u8Audio_ch;
				pMvServiceData.u8Audio_Mode = stCHInfo_Glob[ServIdx].u8Audio_Mode;
				pMvServiceData.u8AudioVolume = stCHInfo_Glob[ServIdx].u8AudioVolume;
				stCHInfo_Glob[ServIdx] = pMvServiceData;
				stCHInfo_Glob[ServIdx].u8Valid = eCS_DB_VALID;

				servicestatus = eCS_DB_DISCARDSERVICE;
				*pServiceIndex = ServIdx;
			}
		}
	}
	}

	if ( servicestatus == eCS_DB_ADDSERVICE )
	{
		/* By KB Kim : 2011.01.19 */
		// u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(pMvServiceData.u16TransponderIndex);
		u8Global_u8SatSearchIndex[u8SatIndex] = TRUE;

		if(pMvServiceData.u8TvRadio ==  eCS_DB_TV_SERVICE || pMvServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE )
		{
			iffound = FALSE;
			for(ServIdx = 0; ServIdx < MV_MAX_SERVICE_COUNT; ServIdx++)
			{
				if(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_INVALID)
				{
					iffound = TRUE;
					//pServiceListInfo->sCS_DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] = ServIdx;
					MV_UseDB_AddOneServiceIndex( kCS_DB_DEFAULT_TV_LIST_ID, ServIdx);
					break;
				}
			}

			pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID]++;
			u16Global_SatListIndex[u8SatIndex].stChIndex[u16Global_SatListIndex[u8SatIndex].u16ChCount] = ServIdx;
			//printf("TV :: %d :: ", pMvServiceData.u16TransponderIndex);
			//printf("%d : %d ===> %d\n", u8SatIndex, u16Global_SatListIndex[u8SatIndex].u16ChCount, u16Global_SatListIndex[u8SatIndex].stChIndex[u16Global_SatListIndex[u8SatIndex].u16ChCount]);
			u16Global_SatListIndex[u8SatIndex].u16ChCount++;
		}
		else
		{
			iffound = FALSE;
			for(ServIdx = 0; ServIdx < MV_MAX_SERVICE_COUNT; ServIdx++)
			{
				if(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_INVALID)
				{
					iffound = TRUE;
					//pServiceListInfo->sCS_DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID] = ServIdx;
					MV_UseDB_AddOneServiceIndex( kCS_DB_DEFAULT_RADIO_LIST_ID,  ServIdx);
					break;
				}
			}
			pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]++;
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount] = ServIdx;
			//printf("RADIO :: %d :: ", pMvServiceData.u16TransponderIndex );
			//printf("%d : %d ===> %d\n", u8SatIndex, u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount, u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount]);
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount++;
		}

		if(iffound)
		{
#if 0
			if ( pMvServiceData.u8TvRadio ==  eCS_DB_TV_SERVICE )
				dprintf(("%5s ADD Service #### %d : %s ,%d ########\n", "TV", ServIdx, pMvServiceData.acServiceName, pMvServiceData.u16VideoPid));
			else
				dprintf(("%5s ADD Service #### %d : %s ,%d ########\n", "RADIO", ServIdx, pMvServiceData.acServiceName, pMvServiceData.u16VideoPid));
#endif
			pMvServiceData.u16ChIndex = ServIdx;
			stCHInfo_Glob[ServIdx] = pMvServiceData;
			//stCHInfo_Glob[ServIdx].u8Lock = eCS_DB_NOT_LOCKED;
			stCHInfo_Glob[ServIdx].u8Valid = eCS_DB_VALID;
			stCHInfo_Glob[ServIdx].u8Audio_Mode = MV_DB_STEREO;
			printf("MV_DB_AddOneService : Fav %d flag[%d / %d]\n", ServIdx, stCHInfo_Glob[ServIdx].u8Favor, pMvServiceData.u8Favor);
			*pServiceIndex = ServIdx;
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return eCS_DB_OK;
}
#else
tCS_DB_Error MV_DB_AddOneService(MV_stServiceInfo pMvServiceData, U16 *pServiceIndex)
{
	U16			ServIdx = 0;
	U16			TotalService = 0;
	U8			u8SatIndex = 0;

	tCS_DB_ServcieAddedMode servicestatus = eCS_DB_ADDSERVICE;

	/* By KB Kim : 2011.01.19 */
	u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(pMvServiceData.u16TransponderIndex);
	if( (MAX_TP_COUNT <= pMvServiceData.u16TransponderIndex) || (MV_MAX_SATELLITE_COUNT <= u8SatIndex))
	{
		return eCS_DB_ERROR;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	TotalService = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	if(MV_MAX_SERVICE_COUNT <= TotalService)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	/* By KB Kim : 2011.01.20 */
	if (TotalService > 0)
	{
		// for(ServIdx = 0; ServIdx <= TotalService; ServIdx++)
		for(ServIdx = 0; ServIdx < TotalService; ServIdx++)
		{
			if(( pMvServiceData.u16TransponderIndex == stCHInfo_Glob[ServIdx].u16TransponderIndex))
			{
				if((pMvServiceData.u16ServiceId == stCHInfo_Glob[ServIdx].u16ServiceId)
					&&(stTPInfo_Glob[pMvServiceData.u16TransponderIndex].u16TSID == stTPInfo_Glob[stCHInfo_Glob[ServIdx].u16TransponderIndex].u16TSID)
					&&(stTPInfo_Glob[pMvServiceData.u16TransponderIndex].u16NID == stTPInfo_Glob[stCHInfo_Glob[ServIdx].u16TransponderIndex].u16NID)
					&&(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_VALID))
				{
					/* Modify Service : By KB Kim 2011.01.19 */
					pMvServiceData.u16ChIndex = stCHInfo_Glob[ServIdx].u16ChIndex;
					pMvServiceData.u8Lock = stCHInfo_Glob[ServIdx].u8Lock;
					pMvServiceData.u8Scramble = stCHInfo_Glob[ServIdx].u8Scramble;
					pMvServiceData.u8Watch = stCHInfo_Glob[ServIdx].u8Watch;
					pMvServiceData.u8Erase = stCHInfo_Glob[ServIdx].u8Erase;
					pMvServiceData.u8Favor = stCHInfo_Glob[ServIdx].u8Favor;
					pMvServiceData.u8Audio_ch = stCHInfo_Glob[ServIdx].u8Audio_ch;
					pMvServiceData.u8Audio_Mode = stCHInfo_Glob[ServIdx].u8Audio_Mode;
					pMvServiceData.u8AudioVolume = stCHInfo_Glob[ServIdx].u8AudioVolume;
					stCHInfo_Glob[ServIdx] = pMvServiceData;
					stCHInfo_Glob[ServIdx].u8Valid = eCS_DB_VALID;

					servicestatus = eCS_DB_DISCARDSERVICE;
					*pServiceIndex = ServIdx;
				}
			}
		}
	}

	if ( servicestatus == eCS_DB_ADDSERVICE )
	{
		/* By KB Kim : 2011.01.19 */
		// u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(pMvServiceData.u16TransponderIndex);
		u8Global_u8SatSearchIndex[u8SatIndex] = TRUE;

		ServIdx = TotalService;
		pMvServiceData.u16ChIndex = ServIdx;
		stCHInfo_Glob[ServIdx] = pMvServiceData;
		//stCHInfo_Glob[ServIdx].u8Lock = eCS_DB_NOT_LOCKED;
		stCHInfo_Glob[ServIdx].u8Valid = eCS_DB_VALID;
		stCHInfo_Glob[ServIdx].u8Audio_Mode = MV_DB_STEREO;

		if(pMvServiceData.u8TvRadio ==  eCS_DB_TV_SERVICE || pMvServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE )
		{
			MV_UseDB_AddOneServiceIndex( kCS_DB_DEFAULT_TV_LIST_ID, ServIdx);
			pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID]++;
			u16Global_SatListIndex[u8SatIndex].stChIndex[u16Global_SatListIndex[u8SatIndex].u16ChCount] = ServIdx;
			//printf("TV :: %d :: ", pMvServiceData.u16TransponderIndex);
			//printf("%d : %d ===> %d\n", u8SatIndex, u16Global_SatListIndex[u8SatIndex].u16ChCount, u16Global_SatListIndex[u8SatIndex].stChIndex[u16Global_SatListIndex[u8SatIndex].u16ChCount]);
			//printf("MV_DB_AddOneService : TV [%d / %d]\n", u16Global_SatListIndex[u8SatIndex].u16ChCount, ServIdx);
			u16Global_SatListIndex[u8SatIndex].u16ChCount++;
		}
		else
		{
			MV_UseDB_AddOneServiceIndex( kCS_DB_DEFAULT_RADIO_LIST_ID,  ServIdx);
			pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]++;
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount] = ServIdx;
			//printf("RADIO :: %d :: ", pMvServiceData.u16TransponderIndex );
			//printf("%d : %d ===> %d\n", u8SatIndex, u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount, u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount]);
			//printf("MV_DB_AddOneService : Radio [%d / %d]\n", u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount, ServIdx);
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount++;
		}
		*pServiceIndex = ServIdx;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return eCS_DB_OK;
}
#endif

void MV_DB_Set_Replace_Index(void)
{
	int 				i, j, k;
	U16					u16Replace_Index;
	U8					u8SatIndex;
	U8                  u8ChTypeTv;

	for( i = 0 ; i < MV_DB_GetALLServiceNumber() ; i++ )
	{
		if ( stCHInfo_Glob[i].u16ChIndex != i )
		{
			u16Replace_Index = stCHInfo_Glob[i].u16ChIndex;
			u8SatIndex = MV_DB_Get_SatIndex_By_Chindex(u16Replace_Index);
			stCHInfo_Glob[i].u16ChIndex = i;
			if((stCHInfo_Glob[i].u8TvRadio == eCS_DB_TV_SERVICE) || (stCHInfo_Glob[i].u8TvRadio  == eCS_DB_HDTV_SERVICE))
			{
				u8ChTypeTv = kCS_DB_DEFAULT_TV_LIST_ID;
			}
			else
			{
				u8ChTypeTv = kCS_DB_DEFAULT_RADIO_LIST_ID;
			}

		/********* Replace Ch Index DB Index ****************************/
			for( k = 0 ; k < MV_DB_GetALLServiceNumber() ; k++)
			{
				if ( u16ChIndex_Glob[k].u16Index_Ch == u16Replace_Index )
				{
					u16ChIndex_Glob[k].u16Index_Ch = stCHInfo_Glob[i].u16ChIndex;
					break;
				}
			}

		/********* Replace Favorite Index ****************************/
			for( k = 0 ; k < MV_MAX_FAV_KIND ; k++)
			{
				for( j = 0 ; j < MV_MAX_FAV_COUNT ; j++ )
				{
					if ( stFavInfo_Glob[k].u16FavChIndex[u8ChTypeTv/*u16ChIndex_Glob[i].u16Ch_Kind*/][j] == u16Replace_Index )
					{
						stFavInfo_Glob[k].u16FavChIndex[u8ChTypeTv /*u16ChIndex_Glob[i].u16Ch_Kind*/][j] = stCHInfo_Glob[i].u16ChIndex;
						break;
					}
				}
			}

		/********* Replace All Ch_Index ****************************/
			if ( u8ChTypeTv == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				for( k = 0 ; k < pServiceListInfo->u16DB_List_ServiceNum[u16ChIndex_Glob[i].u16Ch_Kind] ; k++ )
				{
					if ( u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][k] == u16Replace_Index )
					{
						u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][k] = stCHInfo_Glob[i].u16ChIndex;
						break;
					}
				}
			}
			else
			{
				for( k = 0 ; k < pServiceListInfo->u16DB_List_ServiceNum[u16ChIndex_Glob[i].u16Ch_Kind] ; k++ )
				{
					if ( u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][k] == u16Replace_Index )
					{
						u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][k] = stCHInfo_Glob[i].u16ChIndex;
						break;
					}
				}
			}

		/********* Replace Satellite Ch_Index ****************************/
			if ( u8ChTypeTv == kCS_DB_DEFAULT_TV_LIST_ID )
			{
				for ( k = 0 ; k < u16Global_SatListIndex[u8SatIndex].u16ChCount ; k++ )
				{
					if ( u16Global_SatListIndex[u8SatIndex].stChIndex[k] == u16Replace_Index )
					{
						u16Global_SatListIndex[u8SatIndex].stChIndex[k] = stCHInfo_Glob[i].u16ChIndex;

						//printf("==== %d : %d : %s : %d : %s\n", k, u8SatIndex, stSatInfo_Glob[u8SatIndex].acSatelliteName, stCHInfo_Glob[i].u16ChIndex, stCHInfo_Glob[i].acServiceName);
						break;
					}
				}
			}
			else
			{
				for ( k = 0 ; k < u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount ; k++ )
				{
					if ( u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[k] == u16Replace_Index )
					{
						u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].stChIndex[k] = stCHInfo_Glob[i].u16ChIndex;
						break;
					}
				}
			}
		}
	}
}

#if 1
tCS_DB_Error MV_DB_DeleteOneService(U16 pServiceIndex)
{
	tCS_DB_ServiceType	ServiceType;
	U8 					i;
	U16					ServIdx = 0;
	tCS_DB_Error		tCheckError = eCS_DB_OK;
	U8					u8SatIndex = 0;
	U16					u16ChIndex = 0;

	/* By KB Kim 2011.01.20 */
	U8                  u8ChTypeTv;
	U8                  u8FavCount = 0;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return eCS_DB_ERROR;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	tCheckError = MV_UserDB_FindIndexNoBySrvIndex(pServiceIndex, &ServIdx);

	//printf("\n\n===== %d , %d, %s \n", tCheckError, ServIdx, stCHInfo_Glob[ServIdx].acServiceName);

	if(0 == (pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]))
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		//printf("====== 1 Error \n");
		return eCS_DB_ERROR;
	}

	if(eCS_DB_INVALID == stCHInfo_Glob[ServIdx].u8Valid)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		//printf("====== 2 Error \n");
		return eCS_DB_ERROR;
	}

	/* By KB Kim 2011.01.20 */
	ServiceType = (tCS_DB_ServiceType)stCHInfo_Glob[ServIdx].u8TvRadio;
	if((ServiceType == eCS_DB_TV_SERVICE) || (ServiceType == eCS_DB_HDTV_SERVICE))
	{
		u8ChTypeTv = kCS_DB_DEFAULT_TV_LIST_ID;
	}
	else
	{
		u8ChTypeTv = kCS_DB_DEFAULT_RADIO_LIST_ID;
	}

	for( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
	{
		int  	j=0;

		/* By KB Kim 2011.01.20 */
		u8FavCount = stFavInfo_Glob[i].u8FavCount[u8ChTypeTv];
		if (u8FavCount > 0)
		{
			for( j = 0 ; j < u8FavCount ; j++ )
			{
				if ( stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j] == pServiceIndex )
				{
					//printf("== %d , %d , %d : %d , %d : %d , Name : %s - %s \n", i, j, ServIdx, pServiceIndex, stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j], stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j+1], stCHInfo_Glob[stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j]].acServiceName, stCHInfo_Glob[stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j+1]].acServiceName);
					/* For FavList Delete Problem By KB Kim 2012.04.21 */
					// memcpy(&stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j], &stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j+1], MV_MAX_FAV_COUNT-j-1);
					memcpy(&stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j], &stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j+1], sizeof(U16)*(MV_MAX_FAV_COUNT-j-1));
					stFavInfo_Glob[i].u8FavCount[u8ChTypeTv]--;
					//stFavInfo_Glob[i].u16FavChIndex[u8ChTypeTv][j] = 0xFFFF;
					break;
				}
			}
		}
	}

	stCHInfo_Glob[ServIdx].u8Valid= eCS_DB_INVALID;
	u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(stCHInfo_Glob[ServIdx].u16TransponderIndex);
	//printf("== Channel : %d :: %s ::: %d =======> %d\n", stCHInfo_Glob[ServIdx].u16ChIndex, stCHInfo_Glob[ServIdx].acServiceName, stCHInfo_Glob[ServIdx].u16TransponderIndex, u8SatIndex );
	u16ChIndex = stCHInfo_Glob[ServIdx].u16ChIndex;

	memcpy( &stCHInfo_Glob[ServIdx], &stCHInfo_Glob[ServIdx+1], sizeof(MV_stServiceInfo) * ( MV_MAX_SERVICE_COUNT - (ServIdx+1) ));

	if ( tCheckError == eCS_DB_OK )
	{
		if(pServiceListInfo->u16DB_List_ServiceNum[u8ChTypeTv] > 0)
		{
			MV_UseDB_DeleteOneServiceIndex(u8ChTypeTv, u16ChIndex);
			pServiceListInfo->u16DB_List_ServiceNum[u8ChTypeTv]--;
			//printf("%d -> %d :: pServiceListInfo->u16DB_List_ServiceNum : %d \n", ServiceType, u8ChTypeTv, pServiceListInfo->u16DB_List_ServiceNum[u8ChTypeTv]);
		}
		else
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			//printf("ERROR %d -> %d : if(pServiceListInfo->u16DB_List_ServiceNum[u8ChTypeTv] > 0) Error \n", ServiceType, u8ChTypeTv);
			return eCS_DB_ERROR;
		}
	}
	else
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		//printf("ERROR %d -> %d : if ( tCheckError == eCS_DB_OK ) Error \n", ServiceType, u8ChTypeTv);
		return eCS_DB_ERROR;
	}

	if( u8ChTypeTv == kCS_DB_DEFAULT_TV_LIST_ID )
	{
		if(u16Global_SatListIndex[u8SatIndex].u16ChCount > 0)
		{
			MV_UseDB_DeleteOneServiceSatIndex(kCS_DB_DEFAULT_TV_LIST_ID, u8SatIndex, u16ChIndex);
			u16Global_SatListIndex[u8SatIndex].u16ChCount--;
			//printf("TV Sat %d %s : %d -> %d :: Count : %d \n", u8SatIndex, stSatInfo_Glob[u8SatIndex].acSatelliteName, ServiceType, u8ChTypeTv, u16Global_SatListIndex[u8SatIndex].u16ChCount);
		}
		else
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			//printf("ERROR %d -> %d : if(u16Global_SatListIndex[u8SatIndex].u16ChCount > 0) Error \n", ServiceType, u8ChTypeTv);
			return eCS_DB_ERROR;
		}
	}
	else
	{
		if(u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount > 0)
		{
			MV_UseDB_DeleteOneServiceSatIndex(kCS_DB_DEFAULT_RADIO_LIST_ID, u8SatIndex, u16ChIndex);
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount--;
			//printf("RADIO Sat %d %s : %d -> %d :: Count : %d \n", u8SatIndex, stSatInfo_Glob[u8SatIndex].acSatelliteName, ServiceType, u8ChTypeTv, u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount);
		}
		else
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			//printf("ERROR %d -> %d : if(u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount > 0) Error \n", ServiceType, u8ChTypeTv);
			return eCS_DB_ERROR;
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}
#else
tCS_DB_Error MV_DB_DeleteOneService(U16 pServiceIndex)
{
	tCS_DB_ServiceType	ServiceType;
	U8 					i;
	U16					ServIdx = 0;
	tCS_DB_Error		tCheckError = eCS_DB_OK;
	U8					u8SatIndex = 0;
	U16					u16ChIndex = 0;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return eCS_DB_ERROR;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	tCheckError = MV_UserDB_FindIndexNoBySrvIndex(pServiceIndex, &ServIdx);

	//dprintf(("===== %d , %d\n", tCheckError, ServIdx));

	if(0 == pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID])
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	if(eCS_DB_INVALID == stCHInfo_Glob[ServIdx].u8Valid)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	ServiceType = (tCS_DB_ServiceType)stCHInfo_Glob[ServIdx].u8TvRadio;

	for(i=0; i<MV_MAX_FAV_KIND; i++)
	{
		int  	j=0;

		for( j = 0 ; j < MV_MAX_FAV_COUNT ; j++ )
		{
			if ( stFavInfo_Glob[i].u16FavChIndex[ServiceType][j] == pServiceIndex )
			{
				memcpy(&stFavInfo_Glob[i].u16FavChIndex[j], &stFavInfo_Glob[i].u16FavChIndex[ServiceType][j+1], sizeof(U16)*(MV_MAX_FAV_COUNT -j-1));
				stFavInfo_Glob[i].u8FavCount[ServiceType]--;
				break;
			}
		}
	}

	stCHInfo_Glob[ServIdx].u8Valid= eCS_DB_INVALID;
	u8SatIndex = MV_DB_Get_SatIndex_By_TPindex(stCHInfo_Glob[ServIdx].u16TransponderIndex);
	u16ChIndex = stCHInfo_Glob[ServIdx].u16ChIndex;

	if ( tCheckError == eCS_DB_OK )
	{
		if(ServiceType == eCS_DB_TV_SERVICE || ServiceType == eCS_DB_HDTV_SERVICE )
		{
			if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] > 0)
			{
				MV_UseDB_DeleteOneServiceIndex(kCS_DB_DEFAULT_TV_LIST_ID, u16ChIndex);
				pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID]--;
				//pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID]--;
			}
			else
			{
				CSOS_SignalSemaphore(sem_ServiceListAccess);
				return eCS_DB_ERROR;
			}
		}
		else
		{
			if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID] > 0)
			{
				MV_UseDB_DeleteOneServiceIndex(kCS_DB_DEFAULT_RADIO_LIST_ID, u16ChIndex);
				pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]--;
				//pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]--;
			}
			else
			{
				CSOS_SignalSemaphore(sem_ServiceListAccess);
				return eCS_DB_ERROR;
			}
		}
	}
	else
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	if(ServiceType == eCS_DB_TV_SERVICE || ServiceType == eCS_DB_HDTV_SERVICE )
	{
		if(u16Global_SatListIndex[u8SatIndex].u16ChCount > 0)
		{
			MV_UseDB_DeleteOneServiceSatIndex(kCS_DB_DEFAULT_TV_LIST_ID, u8SatIndex, u16ChIndex);
			u16Global_SatListIndex[u8SatIndex].u16ChCount--;
		}
		else
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			return eCS_DB_ERROR;
		}
	}
	else
	{
		if(u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount > 0)
		{
			MV_UseDB_DeleteOneServiceSatIndex(kCS_DB_DEFAULT_RADIO_LIST_ID, u8SatIndex, u16ChIndex);
			u16Global_SatListIndex[u8SatIndex + MV_SAT_MAX].u16ChCount--;
		}
		else
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			return eCS_DB_ERROR;
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}
#endif

tCS_DB_Error MV_DB_DeleteAllService(void)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	memset(stCHInfo_Glob, 0, sizeof(MV_stServiceInfo) * MV_MAX_SERVICE_COUNT
						+ 4 + sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT
						+ 4 + sizeof(MV_stFavInfo) * MV_MAX_FAV_KIND);

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}

U16 MV_DB_GetALLServiceNumber(void)
{
	U16	num;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	num = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];
	//printf("tv num = %d, radio num = %d\n", pServiceListInfo->sCS_DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST], pServiceListInfo->sCS_DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST]);
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(num);
}

U16 CS_DB_GetListServiceNumber(tCS_DB_ServiceListTriplet  ListTriplet)
{
	U16	num = 0;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	switch(ListTriplet.sCS_DB_ServiceListType)
	{
		case eCS_DB_TV_LIST :
			num = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID];
			break;
		case eCS_DB_RADIO_LIST :
			num = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];
			break;
		case eCS_DB_FAV_TV_LIST :
			num = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u8FavCount[kCS_DB_DEFAULT_TV_LIST_ID];
			break;
		case eCS_DB_FAV_RADIO_LIST :
			num = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u8FavCount[kCS_DB_DEFAULT_RADIO_LIST_ID];
			break;
		case eCS_DB_SAT_TV_LIST:
			num = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue].u16ChCount;
			break;
		case eCS_DB_SAT_RADIO_LIST:
			num = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue + MV_SAT_MAX].u16ChCount;
			break;
		case eCS_DB_INVALID_LIST :
			num = 0;
			break;
		default:
			num = 0;
			break;
	}
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(num);
}

U16 MV_DB_Get_ServiceAllList_Index(tCS_DB_ServiceListType chlist_type, U16 u16ChIndex)
{
	int			i;
	U16			u16Return_Value;

	switch(chlist_type)
	{
		case eCS_DB_TV_LIST :
			for ( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i++ )
			{
				if ( u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i] == u16ChIndex )
					break;
			}
			u16Return_Value = i;
			break;
		case eCS_DB_RADIO_LIST :
			for ( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i++ )
			{
				if ( u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i] == u16ChIndex )
					break;
			}
			u16Return_Value = i;
			break;
		default:
			u16Return_Value = 0;
			break;
	}
	return u16Return_Value;
}

tCS_DB_Error MV_DB_GetServiceDataByIndex(MV_stServiceInfo *pServiceData, U32 pServiceIndex)
{
	tCS_DB_Error  status = eCS_DB_ERROR;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	if( pServiceData == NULL )
	{
		status = eCS_DB_ERROR;
	}
	else if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
	{
		status = eCS_DB_OK;
		*pServiceData = stCHInfo_Glob[pServiceIndex];
		//memcpy(pServiceData,&stCHInfo_Glob[pServiceIndex],sizeof(MV_stServiceInfo));
	}

	return(status);
}

tCS_DB_Error MV_DB_RenameServiceName(char * serviceName,U16 pServiceIndex)
{
	tCS_DB_Error  	status = eCS_DB_ERROR;
	int				i;

	if( serviceName == NULL )
	{
		return(status);

	}

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	for ( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i++ )
	{
		if ( stCHInfo_Glob[i].u16ChIndex == pServiceIndex )
		{
			if(stCHInfo_Glob[i].u8Valid== eCS_DB_VALID)
			{
				status = eCS_DB_OK;
				memcpy(stCHInfo_Glob[i].acServiceName, serviceName, MAX_SERVICE_NAME_LENGTH);
				stCHInfo_Glob[i].acServiceName[(MAX_SERVICE_NAME_LENGTH - 1)] = '\0';
			}
			break;
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}

#if 0
tCS_DB_Error MV_DB_ModifyServiceLockStatus(tCS_DB_LockStatus lock , U16 pServiceIndex)
{
	tCS_DB_Error  	status = eCS_DB_ERROR;
	int				i;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	for ( i = 0 ; i < MV_MAX_SERVICE_COUNT ; i ++ )
	{
		if (stCHInfo_Glob[i].u16ChIndex == pServiceIndex)
		{
			if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
			{
				status = eCS_DB_OK;
				stCHInfo_Glob[pServiceIndex].u8Lock= lock;
			}
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}
#else

tCS_DB_Error MV_DB_ModifyServiceLockStatus(tCS_DB_LockStatus flag , U16 index_inlist)
{
	tCS_DB_Error    status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((index_inlist >=  Current_ServiceList.Service_Number)
		||(Current_ServiceList.Service_Number == 0)
		||(Current_ServiceList.Service_Data == NULL))
	{
		status = eCS_DB_ERROR;
	}
	else
	{
		Current_ServiceList.Service_Data[index_inlist].Lock_Flag = flag;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}

#endif

tCS_DB_Error MV_DB_ModifyServiceAudio(U16 AudioPid, tCS_DB_AudioType AudioType, U16 pServiceIndex)
{
	tCS_DB_Error  status = eCS_DB_ERROR;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
	{
		status = eCS_DB_OK;
		stCHInfo_Glob[pServiceIndex].u16AudioPid = AudioPid;

		if( AudioType >= eCS_DB_AUDIO_UNKNOWN )
			stCHInfo_Glob[pServiceIndex].u8Audio_Type= eCS_DB_AUDIO_MPEG2;
		else
			stCHInfo_Glob[pServiceIndex].u8Audio_Type = AudioType;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(status);
}


tCS_DB_Error MV_DB_ModifyAudioMode(MV_DB_AudioMode AudioMode, U16 pServiceIndex)
{
	tCS_DB_Error  status = eCS_DB_ERROR;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
	{
		status = eCS_DB_OK;
		stCHInfo_Glob[pServiceIndex].u8Audio_Mode= AudioMode;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(status);
}


tCS_DB_Error MV_DB_ModifyServiceVideoPid(U16 VideoPid, U16 pServiceIndex)
{
	tCS_DB_Error  status = eCS_DB_ERROR;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
	{
		status = eCS_DB_OK;
		stCHInfo_Glob[pServiceIndex].u16VideoPid= VideoPid;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(status);
}


tCS_DB_Error MV_DB_ModifyServicePcrPid(U16 PcrPid, U16 pServiceIndex)
{
	tCS_DB_Error  status = eCS_DB_ERROR;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return(status);
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(stCHInfo_Glob[pServiceIndex].u8Valid== eCS_DB_VALID)
	{

		status = eCS_DB_OK;
		stCHInfo_Glob[pServiceIndex].u16PCRPid= PcrPid;

	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(status);
}

void MV_Get_ReCall_List(tCS_DBU_Service *stTemp_Recall)
{
	memcpy(stTemp_Recall, &stRecall_Service, sizeof(tCS_DBU_Service)*MV_RECALL_MAX_ITEM);
}

#if 0 /* By KB Kim for Multi-Recall list problem : 2011.08.30 */
tCS_DB_Error MV_Set_ReCall_List(tCS_DBU_Service stTemp_Recall)
{
	int 			i;
	tCS_DBU_Service	Temp_Struct[MV_RECALL_MAX_ITEM];

	MV_Get_ReCall_List(Temp_Struct);

	for ( i = 0 ; i < MV_RECALL_MAX_ITEM ; i++ )
	{
		if ( memcmp ( &stRecall_Service[i], &stTemp_Recall, sizeof(tCS_DBU_Service) ) == 0 )
		{
			memcpy(&stRecall_Service[1], &Temp_Struct[0], sizeof(tCS_DBU_Service)*( i ));
			stRecall_Service[0] = stTemp_Recall;
			return eCS_DB_ERROR;
		}
	}

	memcpy(&stRecall_Service[1], &Temp_Struct[0], sizeof(tCS_DBU_Service)*( MV_RECALL_MAX_ITEM -1 ));
	stRecall_Service[0] = stTemp_Recall;
	return	eCS_DB_OK;
}
#else

tCS_DB_Error MV_Set_ReCall_List(tCS_DBU_Service curentService)
{
	int 			i;
	int             j;
	tCS_DBU_Service	Temp_Struct[MV_RECALL_MAX_ITEM];
	tCS_DBU_Service lastService;

	lastService = CS_DB_GetLastServiceTriplet();
	MV_Get_ReCall_List(Temp_Struct);

	if (memcmp(&lastService, &curentService, sizeof(tCS_DBU_Service)) == 0)
	{
	}
	else
	{
		stRecall_Service[0] = lastService;
		j = 1;
	}

	for ( i = 0 ; ((j < MV_RECALL_MAX_ITEM) && (i < MV_RECALL_MAX_ITEM)); i++ )
	{
		if ((memcmp (&Temp_Struct[i], &curentService, sizeof(tCS_DBU_Service)) != 0) &&
		    (memcmp (&Temp_Struct[i], &lastService, sizeof(tCS_DBU_Service)) != 0))
		{
			// printf("MV_Set_ReCall_List : %d/%d\n", i, j);
			stRecall_Service[j] = Temp_Struct[i];
			j++;
		}
	}

	if (j < MV_RECALL_MAX_ITEM)
	{
		memset(&stRecall_Service[j], 0xFF, sizeof(tCS_DBU_Service)*(MV_RECALL_MAX_ITEM - j));
	}
	return	eCS_DB_OK;
}
#endif

#if 0 /* By KB Kim for Multi-Recall list problem : 2011.08.30 */
tCS_DB_Error MV_Reset_ReCall_List(tCS_DBU_Service stTemp_Recall)
{
	int 			i;
	tCS_DBU_Service	Temp_Struct[MV_RECALL_MAX_ITEM];

	MV_Get_ReCall_List(Temp_Struct);

	for ( i = 0 ; i < MV_RECALL_MAX_ITEM ; i++ )
	{
		if ( memcmp ( &stRecall_Service[i], &stTemp_Recall, sizeof(tCS_DBU_Service) ) == 0 )
		{
			memcpy(&stRecall_Service[i], &Temp_Struct[i+1], sizeof(tCS_DBU_Service)*( MV_RECALL_MAX_ITEM - i ));
			memset(&stRecall_Service[MV_RECALL_MAX_ITEM - 1], 0xFF, sizeof(tCS_DBU_Service));
			return eCS_DB_ERROR;
		}
	}
	return	eCS_DB_OK;
}
#else
tCS_DB_Error MV_Reset_ReCall_List(void)
{
	memset(&stRecall_Service, 0xFF, sizeof(tCS_DBU_Service) * MV_RECALL_MAX_ITEM);
	return	eCS_DB_OK;
}
#endif
tCS_DB_Error CS_DB_SetCurrentList(tCS_DB_ServiceListTriplet  ListTriplet, BOOL Force)
{
	U16							index = 0;
	U16							*serviceorder = NULL;
	U16  						usr_lcn = 0;
	U16							u16Check_Count = 0;
	U8							u8Sorttype = eCS_DB_BY_NORMAL;
	MV_stServiceInfo			service_data;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((ListTriplet.sCS_DB_ServiceListType != Current_ServiceList.List_Triplet.sCS_DB_ServiceListType)
		||(ListTriplet.sCS_DB_ServiceListTypeValue != Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue)
		||Force)
	{
		switch(ListTriplet.sCS_DB_ServiceListType)
		{
			case eCS_DB_TV_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][0];
				Current_ServiceList.Service_Number = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID];
				serviceorder = &u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST];
				break;
			case eCS_DB_RADIO_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][0];
				Current_ServiceList.Service_Number = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];
				serviceorder = &u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST];
				break;
			case eCS_DB_FAV_TV_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST_NUM + ListTriplet.sCS_DB_ServiceListTypeValue];
				Current_ServiceList.Service_Number = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u8FavCount[kCS_DB_DEFAULT_TV_LIST_ID];
				serviceorder = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u16FavChIndex[kCS_DB_DEFAULT_TV_LIST_ID];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST_NUM + ListTriplet.sCS_DB_ServiceListTypeValue];
				break;
			case eCS_DB_FAV_RADIO_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST_NUM + ListTriplet.sCS_DB_ServiceListTypeValue];
				Current_ServiceList.Service_Number = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u8FavCount[kCS_DB_DEFAULT_RADIO_LIST_ID];
				serviceorder = stFavInfo_Glob[ListTriplet.sCS_DB_ServiceListTypeValue].u16FavChIndex[kCS_DB_DEFAULT_RADIO_LIST_ID];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST_NUM + ListTriplet.sCS_DB_ServiceListTypeValue];
				break;
			case eCS_DB_SAT_TV_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][MV_FIXED_LIST_NUMBER + ListTriplet.sCS_DB_ServiceListTypeValue];
				Current_ServiceList.Service_Number = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue].u16ChCount;
				serviceorder = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue].stChIndex; //[ListTriplet.sCS_DB_ServiceListTypeValue];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_FIXED_LIST_NUMBER + ListTriplet.sCS_DB_ServiceListTypeValue];
				break;
			case eCS_DB_SAT_RADIO_LIST :
				Current_ServiceList.Current_Service = List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_FIXED_LIST_NUMBER + ListTriplet.sCS_DB_ServiceListTypeValue];
				Current_ServiceList.Service_Number = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue + MV_SAT_MAX].u16ChCount;
				serviceorder = u16Global_SatListIndex[ListTriplet.sCS_DB_ServiceListTypeValue + MV_SAT_MAX].stChIndex; //[ListTriplet.sCS_DB_ServiceListTypeValue];
				u8Sorttype = u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_FIXED_LIST_NUMBER + ListTriplet.sCS_DB_ServiceListTypeValue];
				break;
			case eCS_DB_INVALID_LIST :
				CSOS_SignalSemaphore(sem_ServiceListAccess);
				return(eCS_DB_ERROR);
				break;

		}

		if(Current_ServiceList.Service_Data!= NULL)
		{
			CSOS_DeallocateMemory(NULL, Current_ServiceList.Service_Data);
			Current_ServiceList.Service_Data = NULL;
		}

		Current_ServiceList.List_Triplet = ListTriplet;

		if(Current_ServiceList.Service_Number == 0)
		{
			Current_ServiceList.Current_Service = 0;

			CSOS_SignalSemaphore(sem_ServiceListAccess);
			return(eCS_DB_ERROR);
		}

		Current_ServiceList.Service_Data = (tCS_DB_ServiceManageData *)CSOS_AllocateMemory(NULL, Current_ServiceList.Service_Number * sizeof(tCS_DB_ServiceManageData));
		//memset( &Current_ServiceList.Service_Data, 0x00, Current_ServiceList.Service_Number * sizeof(tCS_DB_ServiceManageData));

		if(Current_ServiceList.Service_Data == NULL)
		{
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			return(eCS_DB_ERROR);
		}

		if(Current_ServiceList.Current_Service >= Current_ServiceList.Service_Number)
			Current_ServiceList.Current_Service = 0;

		if(Current_ServiceList.Current_Service >= MV_MAX_SERVICE_COUNT)
		{
			Current_ServiceList.Current_Service = 0;
		}

		usr_lcn = kCS_DB_MIN_USER_LCN;

		for(index = 0; index < Current_ServiceList.Service_Number; index++)
		{
			u16Check_Count = 0;

			switch(ListTriplet.sCS_DB_ServiceListType)
			{
				case eCS_DB_TV_LIST:
				case eCS_DB_RADIO_LIST:
				case eCS_DB_FAV_TV_LIST:
				case eCS_DB_FAV_RADIO_LIST:
				case eCS_DB_SAT_TV_LIST:
				case eCS_DB_SAT_RADIO_LIST:
					Current_ServiceList.Service_Data[index].Service_Index = serviceorder[index];
					MV_DB_GetServiceDataByIndex(&service_data, Current_ServiceList.Service_Data[index].Service_Index);
					Current_ServiceList.Service_Data[index].Lock_Flag = service_data.u8Lock;
					Current_ServiceList.Service_Data[index].Delete_Flag = eDBASE_NOT_DELETE;
					Current_ServiceList.Service_Data[index].Move_Flag = eDBASE_NOT_SELECT;
					Current_ServiceList.Service_Data[index].Select_Flag = eDBASE_NOT_SELECT;
					break;
				default:
					break;
			}
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	//printf("\n\n\n ======== u8Sorttype : %d ===========\n\n\n", u8Sorttype);
	if ( u8Sorttype != eCS_DB_BY_NORMAL )
		CS_DB_SortCurrentServiceList(u8Sorttype);

	return(eCS_DB_OK);
}

void  CS_DB_GetCurrentListTriplet(tCS_DB_ServiceListTriplet  * ListTriplet)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(ListTriplet != NULL)
	{
		memcpy(ListTriplet, &(Current_ServiceList.List_Triplet), sizeof(tCS_DB_ServiceListTriplet));
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

}

tCS_DB_Error CS_DB_GetCurrentList_ServiceData(tCS_DB_ServiceManageData* service_data, U16 index_inlist)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((index_inlist >= Current_ServiceList.Service_Number)
                ||(Current_ServiceList.Service_Data == NULL)
                ||(service_data == NULL))
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return(eCS_DB_ERROR);
	}

	*service_data = Current_ServiceList.Service_Data[index_inlist];
	//memcpy(service_data, &(Current_ServiceList.Service_Data[index_inlist]), sizeof(tCS_DB_ServiceManageData));

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(eCS_DB_OK);
}

U16 CS_DB_GetCurrent_Service_By_ServiceIndex(U16 index_inlist)
{
	U16					i;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		if ( Current_ServiceList.Service_Data[i].Service_Index == index_inlist )
			break;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return i;
}

U16 CS_DB_GetCurrent_ServiceIndex_By_Index(U16 index_inlist)
{
	U16					i;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	i = Current_ServiceList.Service_Data[index_inlist].Service_Index;

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return i;
}

U16  CS_DB_GetCurrentList_ServiceNum(void)
{
	U16	num;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	num = Current_ServiceList.Service_Number;
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(num);
}

tCS_DB_Error CS_DB_SetCurrentService_OrderIndex(U16 index_inlist)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	// printf("CS_DB_SetCurrentService_OrderIndex ====== %d : %d , %d ======\n", Current_ServiceList.List_Triplet.sCS_DB_ServiceListType, index_inlist, Current_ServiceList.Service_Number);

	if( index_inlist < Current_ServiceList.Service_Number)
	{
		Current_ServiceList.Current_Service = index_inlist;

		switch(Current_ServiceList.List_Triplet.sCS_DB_ServiceListType)
		{
			case eCS_DB_TV_LIST :
				List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][0] = index_inlist;
				break;
			case eCS_DB_RADIO_LIST :
			 	List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][0] = index_inlist;
				break;
			case eCS_DB_FAV_TV_LIST :
				List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST_NUM + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = index_inlist;
				break;
			case eCS_DB_FAV_RADIO_LIST :
				List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST_NUM + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = index_inlist;
				break;
			case eCS_DB_SAT_TV_LIST :
				List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID][MV_FIXED_LIST_NUMBER + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = index_inlist;
				break;
			case eCS_DB_SAT_RADIO_LIST :
				List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_FIXED_LIST_NUMBER + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = index_inlist;
				break;
			case eCS_DB_INVALID_LIST :
				CSOS_SignalSemaphore(sem_ServiceListAccess);
				return(eCS_DB_ERROR);
				break;

		}
	}
	else
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return(eCS_DB_ERROR);
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(eCS_DB_OK);
}

U16  CS_DB_GetCurrentService_OrderIndex(void)
{
	U16	current_service;
	CSOS_WaitSemaphore(sem_ServiceListAccess);
	current_service = Current_ServiceList.Current_Service;
	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(current_service);
}

#if 0
tCS_DB_Error CS_DB_SetCurrentServiceIndex(U16   index_inHW)
{
	U16 i = 0;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	for(i=0;( i<Current_ServiceList.Service_Number); i++)
	{
		if(Current_ServiceList.Service_Data[i].Service_Index == index_inHW)
		{
			switch(Current_ServiceList.List_Triplet.sCS_DB_ServiceListType)
			{
				case eCS_DB_TV_LIST :
					List_CurrentServiceIndex[kCS_DB_DEFAULT_TV_LIST_ID] = i;
					break;
				case eCS_DB_RADIO_LIST :
				 	List_CurrentServiceIndex[kCS_DB_DEFAULT_RADIO_LIST_ID] = i;
					break;
				case eCS_DB_FAV_LIST :
					List_CurrentServiceIndex[MV_LIST_NUMBER + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = i;
					break;
				case eCS_DB_INVALID_LIST :
					CSOS_SignalSemaphore(sem_ServiceListAccess);
					return(eCS_DB_ERROR);
					break;
			}
			Current_ServiceList.Current_Service = i;
			break;
		}

	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(eCS_DB_OK);

}
#endif

U16   CS_DB_GetCurrentServiceIndex(void)
{
	U16	index;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	if((Current_ServiceList.Current_Service >=  Current_ServiceList.Service_Number)
		||(Current_ServiceList.Service_Number == 0)
		||(Current_ServiceList.Service_Data == NULL))
	{
		index = MV_DB_INVALID_SERVICE_INDEX;
	}
	else
	{
		index = Current_ServiceList.Service_Data[Current_ServiceList.Current_Service].Service_Index;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(index);
}

void MV_DB_Set_Service_Lock(void)
{
	int 	i,j;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		for ( j = 0 ; j < MV_MAX_SERVICE_COUNT ; j++ )
		{
			if ( stCHInfo_Glob[j].u16ChIndex == Current_ServiceList.Service_Data[i].Service_Index )
			{
				stCHInfo_Glob[j].u8Lock = Current_ServiceList.Service_Data[i].Lock_Flag;
				break;
			}
		}
	}
	CSOS_SignalSemaphore(sem_ServiceListAccess);
}

void MV_DB_Set_Service_Move(void)
{
	int				i;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
    //printf("\nMV_DB_Set_Service_Move :: sCS_DB_ServiceListType : %d =======\n\n", Current_ServiceList.List_Triplet.sCS_DB_ServiceListType);
	switch ( Current_ServiceList.List_Triplet.sCS_DB_ServiceListType )
	{
		case eCS_DB_TV_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
				u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i] = Current_ServiceList.Service_Data[i].Service_Index;

			Save_CH_Index_ByUseIndex();
			break;
		case eCS_DB_RADIO_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
				u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i] = Current_ServiceList.Service_Data[i].Service_Index;

			Save_CH_Index_ByUseIndex();
			break;
		case eCS_DB_FAV_TV_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
				stFavInfo_Glob[Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue].u16FavChIndex[kCS_DB_DEFAULT_TV_LIST_ID][i] = Current_ServiceList.Service_Data[i].Service_Index;
			break;
		case eCS_DB_FAV_RADIO_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
				stFavInfo_Glob[Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue].u16FavChIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][i] = Current_ServiceList.Service_Data[i].Service_Index;
			break;
		case eCS_DB_SAT_TV_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
			{
				u16Global_SatListIndex[Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue].stChIndex[i] = Current_ServiceList.Service_Data[i].Service_Index;
				MV_Store_SatListIndex();
			}
			break;
		case eCS_DB_SAT_RADIO_LIST:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
			{
				u16Global_SatListIndex[MV_SAT_MAX + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue].stChIndex[i] = Current_ServiceList.Service_Data[i].Service_Index;
				MV_Store_SatListIndex();
			}
			break;
		default:
			for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
				u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][i] = Current_ServiceList.Service_Data[i].Service_Index;
			break;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
}

BOOL MV_DB_Get_Moving_Flag(void)
{
	return b8MovingFlag;
}

void MV_DB_Set_Moving_Flag(BOOL b8flag)
{
	b8MovingFlag = b8flag;
}

#ifdef ONE_ITEM_MOVE
void MV_DB_ChangeService_OrderIndex(U32 u32Key, U16 index, U8 u8Num_Per_Page)
{
	tCS_DB_ServiceManageData 	Temp_SerMngData;
	tCS_DB_ServiceManageData	*Temp_SerList;

	Temp_SerList = (tCS_DB_ServiceManageData *)CSOS_AllocateMemory(NULL,  Current_ServiceList.Service_Number);
	memset(Temp_SerList, 0x00, Current_ServiceList.Service_Number);

	if ( u32Key == 0x32 ) {  //CSAPP_KEY_UP

		Temp_SerMngData = Current_ServiceList.Service_Data[index];
		if ( index == 0 )
		{
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[1], Current_ServiceList.Service_Number - 1);
			memcpy(&Current_ServiceList.Service_Data[0], Temp_SerList, Current_ServiceList.Service_Number - 1);
			Current_ServiceList.Service_Data[Current_ServiceList.Service_Number - 1] = Temp_SerMngData;
		} else {
			Current_ServiceList.Service_Data[index] = Current_ServiceList.Service_Data[index-1];
			Current_ServiceList.Service_Data[index-1] = Temp_SerMngData;
		}

	} else if ( u32Key == 0x33 ) {// CSAPP_KEY_DOWN

		Temp_SerMngData = Current_ServiceList.Service_Data[index];
		if ( index == Current_ServiceList.Service_Number - 1 )
		{
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[0], Current_ServiceList.Service_Number - 1);
			memcpy(&Current_ServiceList.Service_Data[1], Temp_SerList, Current_ServiceList.Service_Number - 1);
			Current_ServiceList.Service_Data[0] = Temp_SerMngData;
		} else {
			Current_ServiceList.Service_Data[index] = Current_ServiceList.Service_Data[index+1];
			Current_ServiceList.Service_Data[index+1] = Temp_SerMngData;
		}

	} else if ( u32Key == 0x34 ) { // CSAPP_KEY_LEFT

		Temp_SerMngData = Current_ServiceList.Service_Data[index];

		if ( (int)(index - u8Num_Per_Page) < 0 )
		{
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[0], Current_ServiceList.Service_Number - 1);
			memcpy(&Current_ServiceList.Service_Data[1], Temp_SerList, Current_ServiceList.Service_Number - 1);
			Current_ServiceList.Service_Data[0] = Temp_SerMngData;
		} else {
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[index - u8Num_Per_Page], ( u8Num_Per_Page * sizeof(tCS_DB_ServiceManageData)));
			memcpy(&Current_ServiceList.Service_Data[index - u8Num_Per_Page + 1], Temp_SerList, ( u8Num_Per_Page * sizeof(tCS_DB_ServiceManageData)));
			Current_ServiceList.Service_Data[index - u8Num_Per_Page] = Temp_SerMngData;
		}

	} else if ( u32Key == 0x35 ) { // CSAPP_KEY_RIGHT

		Temp_SerMngData = Current_ServiceList.Service_Data[index];

		if ( index + u8Num_Per_Page > Current_ServiceList.Service_Number - 1 )
		{
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[0], Current_ServiceList.Service_Number - 1);
			memcpy(&Current_ServiceList.Service_Data[1], Temp_SerList, Current_ServiceList.Service_Number - 1);
			Current_ServiceList.Service_Data[0] = Temp_SerMngData;
		} else {
			memcpy(Temp_SerList, &Current_ServiceList.Service_Data[index + 1], ( u8Num_Per_Page * sizeof(tCS_DB_ServiceManageData)));
			memcpy(&Current_ServiceList.Service_Data[index], Temp_SerList, ( u8Num_Per_Page * sizeof(tCS_DB_ServiceManageData)));
			Current_ServiceList.Service_Data[index + u8Num_Per_Page] = Temp_SerMngData;
		}

	}

	CSOS_DeallocateMemory(NULL, Temp_SerList);
}
#else  // #ifdef ONE_ITEM_MOVE

#if 0
U16 MV_DB_ChangeService_OrderIndex(U16	u16NowIndex)
{
	tCS_DB_ServiceManageData 	Temp_SerMngData;
	tCS_DB_ServiceManageData	*Temp_SerList;
	int 						i;

	Temp_SerList = (tCS_DB_ServiceManageData *)CSOS_AllocateMemory(NULL, sizeof(tCS_DB_ServiceManageData) * (Current_ServiceList.Service_Number + 1));

	for ( i = Current_ServiceList.Service_Number ; i >= 0 ; i-- )
	{
		if ( Current_ServiceList.Service_Data[i].Move_Flag == eDBASE_SELECT )
		{
			memset(Temp_SerList, 0x00, sizeof(tCS_DB_ServiceManageData) * (Current_ServiceList.Service_Number + 1));

			b8MovingFlag = TRUE;
			Current_ServiceList.Service_Data[i].Move_Flag = eDBASE_NOT_SELECT;

			Temp_SerMngData = Current_ServiceList.Service_Data[i];

			if ( i > u16NowIndex )
			{
				if ( i == u16NowIndex + 1 )
					continue;

				if ( i == u16NowIndex + 2 )
				{
					Current_ServiceList.Service_Data[i] = Current_ServiceList.Service_Data[u16NowIndex + 1];
					Current_ServiceList.Service_Data[u16NowIndex + 1] = Temp_SerMngData;
				} else {

					memcpy(Temp_SerList, &Current_ServiceList.Service_Data[u16NowIndex+1], sizeof(tCS_DB_ServiceManageData) * (( i - u16NowIndex ) - 1));
					memcpy(&Current_ServiceList.Service_Data[u16NowIndex+2], Temp_SerList, sizeof(tCS_DB_ServiceManageData) * (( i - u16NowIndex ) - 1));
					Current_ServiceList.Service_Data[u16NowIndex + 1] = Temp_SerMngData;
				}
				i++;
			} else {
				memcpy(Temp_SerList, &Current_ServiceList.Service_Data[i +1], sizeof(tCS_DB_ServiceManageData) * (u16NowIndex - i));
				memcpy(&Current_ServiceList.Service_Data[i], Temp_SerList, sizeof(tCS_DB_ServiceManageData) * (u16NowIndex - i));
				u16NowIndex--;
				Current_ServiceList.Service_Data[u16NowIndex + 1] = Temp_SerMngData;
			}
		}
	}
	CSOS_DeallocateMemory(NULL, Temp_SerList);
	return u16NowIndex;
}
#else // #if 0
U16 MV_DB_ChangeService_OrderIndex(U16	u16NowIndex)
{
	tCS_DB_ServiceManageData 	Temp_SerMngData;
	tCS_DB_ServiceManageData	*Temp_SerList;
	int 						i;

	// printf("=== MV_DB_ChangeService_OrderIndex START ====\n");
	Temp_SerList = (tCS_DB_ServiceManageData *)CSOS_AllocateMemory(NULL, sizeof(tCS_DB_ServiceManageData) * (Current_ServiceList.Service_Number + 1));

	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		if ( Current_ServiceList.Service_Data[i].Move_Flag == eDBASE_SELECT )
		{
			memset(Temp_SerList, 0x00, sizeof(tCS_DB_ServiceManageData) * (Current_ServiceList.Service_Number + 1));

			b8MovingFlag = TRUE;
			Current_ServiceList.Service_Data[i].Move_Flag = eDBASE_NOT_SELECT;

			Temp_SerMngData = Current_ServiceList.Service_Data[i];

			if ( i > u16NowIndex )
			{
				memcpy(Temp_SerList, &Current_ServiceList.Service_Data[u16NowIndex], sizeof(tCS_DB_ServiceManageData) * ( i - u16NowIndex ));
				memcpy(&Current_ServiceList.Service_Data[u16NowIndex+1], Temp_SerList, sizeof(tCS_DB_ServiceManageData) * ( i - u16NowIndex ));
				u16NowIndex++;
				Current_ServiceList.Service_Data[u16NowIndex - 1] = Temp_SerMngData;
			} else {
				if ( i == u16NowIndex - 1 || i == u16NowIndex )
					continue;

				if ( i == u16NowIndex - 2 )
				{
					Current_ServiceList.Service_Data[i] = Current_ServiceList.Service_Data[u16NowIndex - 1];
					Current_ServiceList.Service_Data[u16NowIndex - 1] = Temp_SerMngData;
				} else {

					memcpy(Temp_SerList, &Current_ServiceList.Service_Data[i+1], sizeof(tCS_DB_ServiceManageData) * (( u16NowIndex - i ) - 1));
					memcpy(&Current_ServiceList.Service_Data[i], Temp_SerList, sizeof(tCS_DB_ServiceManageData) * (( u16NowIndex - i) - 1));
					Current_ServiceList.Service_Data[u16NowIndex - 1] = Temp_SerMngData;
				}
				i--;
			}
		}
	}
	CSOS_DeallocateMemory(NULL, Temp_SerList);
	return u16NowIndex;
}
#endif

#endif  // #ifdef ONE_ITEM_MOVE

U16 CS_DB_SetNextService_OrderIndex(void)
{
	U16	index;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	Current_ServiceList.Current_Service++;
	if ( Current_ServiceList.Current_Service >=  Current_ServiceList.Service_Number)
	{
		Current_ServiceList.Current_Service = 0;
	}
	index = Current_ServiceList.Current_Service;
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(index);
}

U16 CS_DB_SetPreviousService_OrderIndex(void)
{
	U16	index;

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	if ( Current_ServiceList.Current_Service== 0  )
	{
		Current_ServiceList.Current_Service = Current_ServiceList.Service_Number;//RealNumberOfServices;
	}

	Current_ServiceList.Current_Service--;
	index = Current_ServiceList.Current_Service;
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return(index);
}

void CS_DB_Set_TVLastServiceTriplet(void)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	memcpy(&(Last_TVService.sCS_DBU_ServiceList),&(Current_ServiceList.List_Triplet),sizeof(tCS_DB_ServiceListTriplet));
	Last_TVService.sCS_DBU_ServiceIndex = Current_ServiceList.Current_Service;

	CSOS_SignalSemaphore(sem_ServiceListAccess);
}

tCS_DBU_Service CS_DB_Get_TVLastServiceTriplet(void)
{
	return(Last_TVService);
}

void CS_DB_Set_RadioLastServiceTriplet(void)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	memcpy(&(Last_RadioService.sCS_DBU_ServiceList),&(Current_ServiceList.List_Triplet),sizeof(tCS_DB_ServiceListTriplet));
	Last_RadioService.sCS_DBU_ServiceIndex = Current_ServiceList.Current_Service;

	CSOS_SignalSemaphore(sem_ServiceListAccess);
}

tCS_DBU_Service CS_DB_Get_RadioLastServiceTriplet(void)
{
	return(Last_RadioService);
}

void CS_DB_SetLastServiceTriplet(void)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);
	// printf("CS_DB_SetLastServiceTriplet %d\n", Current_ServiceList.Current_Service);
	memcpy(&(Last_Service.sCS_DBU_ServiceList),&(Current_ServiceList.List_Triplet),sizeof(tCS_DB_ServiceListTriplet));
	Last_Service.sCS_DBU_ServiceIndex = Current_ServiceList.Current_Service;

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	/* For Recall list problem By KB Kim :2011.08.30 */
	// MV_Set_ReCall_List(Last_Service);
}

tCS_DBU_Service CS_DB_GetLastServiceTriplet(void)
{
    // printf("CS_DB_GetLastServiceTriplet %d\n", Last_Service.sCS_DBU_ServiceIndex);
	return(Last_Service);
}

tCS_DB_Error CS_DB_ModifyCurrentService_DeleteFlag(tCS_DB_DeleteStatus flag , U16 index_inlist)
{
	tCS_DB_Error    status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((index_inlist >=  Current_ServiceList.Service_Number)
		||(Current_ServiceList.Service_Number == 0)
		||(Current_ServiceList.Service_Data == NULL))
	{
		status = eCS_DB_ERROR;
	}
	else
	{
		Current_ServiceList.Service_Data[index_inlist].Delete_Flag = flag;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}

tCS_DB_Error CS_DB_ModifyCurrentService_MoveFlag(tCS_DB_DeleteStatus flag , U16 index_inlist)
{
	tCS_DB_Error    status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((index_inlist >=  Current_ServiceList.Service_Number)
		||(Current_ServiceList.Service_Number == 0)
		||(Current_ServiceList.Service_Data == NULL))
	{
		status = eCS_DB_ERROR;
	}
	else
	{
		Current_ServiceList.Service_Data[index_inlist].Move_Flag = flag;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}

tCS_DB_Error CS_DB_ModifyCurrentService_SelectFlag(tCS_DB_DeleteStatus flag , U16 index_inlist)
{
	tCS_DB_Error    status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if((index_inlist >=  Current_ServiceList.Service_Number)
		||(Current_ServiceList.Service_Number == 0)
		||(Current_ServiceList.Service_Data == NULL))
	{
		status = eCS_DB_ERROR;
	}
	else
	{
		Current_ServiceList.Service_Data[index_inlist].Select_Flag = flag;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return(status);
}

int  cs_db_CompareFreq_Low_High(const void* First, const void* Second)
{

	S32 							first_freq, second_freq;
	tCS_DB_ServiceManageData		* element;
	MV_stServiceInfo              		Service_Data;
	MV_stTPInfo         				TP_Data;

	element = (tCS_DB_ServiceManageData*)First;
	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
		{
			MV_DB_GetTPDataByIndex(&TP_Data, Service_Data.u16TransponderIndex);
			first_freq = TP_Data.u16TPFrequency;
		}
		else
		{
			return 0;
		}


		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			if(MV_DB_GetServiceDataByIndex(&Service_Data,element->Service_Index) == eCS_DB_OK)
			{
				MV_DB_GetTPDataByIndex(&TP_Data, Service_Data.u16TransponderIndex);
				second_freq = TP_Data.u16TPFrequency;
			}
			else
			{
				return 0;
			}

			return ( first_freq - second_freq );
		}
	}

	return 0;
}

int  cs_db_CompareName_A_Z(const void* First, const void* Second)
{
	char							first_name[MAX_SERVICE_NAME_LENGTH];
	char							second_name[MAX_SERVICE_NAME_LENGTH];
	tCS_DB_ServiceManageData		* element;
	MV_stServiceInfo              	Service_Data;
	int 							i;

	/*----------------------------------------------*/
	/* Search the ServiceData structure of the two services	*/
	/*----------------------------------------------*/

	element = (tCS_DB_ServiceManageData*)First;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex( &Service_Data, element->Service_Index ) == eCS_DB_OK)
		{
			if(Service_Data.acServiceName[0] == 0x15)
				memcpy(first_name, Service_Data.acServiceName+1, MAX_SERVICE_NAME_LENGTH);
			else
				memcpy(first_name, Service_Data.acServiceName, MAX_SERVICE_NAME_LENGTH);

			first_name[MAX_SERVICE_NAME_LENGTH -1] = '\0';
		}
	}
	else
		return 0;

	element = (tCS_DB_ServiceManageData*)Second;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex( &Service_Data, element->Service_Index ) == eCS_DB_OK)
		{
			if(Service_Data.acServiceName[0] == 0x15)
				memcpy(second_name, Service_Data.acServiceName+1, MAX_SERVICE_NAME_LENGTH);
			else
				memcpy(second_name, Service_Data.acServiceName, MAX_SERVICE_NAME_LENGTH);

			second_name[MAX_SERVICE_NAME_LENGTH -1] = '\0';
		}
	}
	else
		return 0;

	for (i=0; (i < kCS_DB_ALPHABETICAL_SORT_PRECISION) && (first_name[i] != '\0') ; i++)
	{
		first_name[i] = (char)tolower(first_name[i]);
	}
	for (i=0; (i < kCS_DB_ALPHABETICAL_SORT_PRECISION) && (second_name[i] != '\0') ; i++)
	{
		second_name[i] = (char)tolower(second_name[i]);
	}

	//printf("first_name = %s, second_name = %s\n", first_name, second_name);

	return strncmp( first_name, second_name, kCS_DB_ALPHABETICAL_SORT_PRECISION);
}

int  cs_db_CompareName_Z_A(const void* First, const void* Second)
{
	char							first_name[MAX_SERVICE_NAME_LENGTH];
	char							second_name[MAX_SERVICE_NAME_LENGTH];
	tCS_DB_ServiceManageData		* element;
	MV_stServiceInfo              		Service_Data;
	int							i;

	/*----------------------------------------------*/
	/* Search the ServiceData structure of the two services	*/
	/*----------------------------------------------*/

	element = (tCS_DB_ServiceManageData*)First;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex( &Service_Data, element->Service_Index ) == eCS_DB_OK)
		{
			if(Service_Data.acServiceName[0] == 0x15)
				memcpy(first_name, Service_Data.acServiceName+1, MAX_SERVICE_NAME_LENGTH);
			else
				memcpy(first_name, Service_Data.acServiceName, MAX_SERVICE_NAME_LENGTH);

			first_name[MAX_SERVICE_NAME_LENGTH -1] = '\0';
		}
	}
	else
		return 0;

	element = (tCS_DB_ServiceManageData*)Second;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex( &Service_Data, element->Service_Index ) == eCS_DB_OK)
		{
			if(Service_Data.acServiceName[0] == 0x15)
				memcpy(second_name, Service_Data.acServiceName+1, MAX_SERVICE_NAME_LENGTH);
			else
				memcpy(second_name, Service_Data.acServiceName, MAX_SERVICE_NAME_LENGTH);

			second_name[MAX_SERVICE_NAME_LENGTH -1] = '\0';
		}
	}
	else
		return 0;

	for (i=0; (i < kCS_DB_ALPHABETICAL_SORT_PRECISION) && (first_name[i] != '\0') ; i++)
	{
		first_name[i] = (char)tolower(first_name[i]);
	}
	for (i=0; (i < kCS_DB_ALPHABETICAL_SORT_PRECISION) && (second_name[i] != '\0') ; i++)
	{
		second_name[i] = (char)tolower(second_name[i]);
	}

	//printf("first_name = %s, second_name = %s\n", first_name, second_name);

	return strncmp( second_name, first_name, kCS_DB_ALPHABETICAL_SORT_PRECISION);
}

int cs_db_CompareFTA_First(const void* First, const void* Second)
{

	U8  							first_scramble, second_scramble;
	tCS_DB_ServiceManageData 	* element;
	MV_stServiceInfo              		Service_Data;



	element = (tCS_DB_ServiceManageData*)First;
	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
		{
			first_scramble = Service_Data.u8Scramble;
		}
		else
		{
			return 0;
		}


		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
			{
				second_scramble = Service_Data.u8Scramble;
			}
			else
			{
				return 0;
			}
			return ( first_scramble - second_scramble );
		}
	}
	return 0;
}

int cs_db_CompareCAS_First(const void* First, const void* Second)
{

	U8  							first_scramble, second_scramble;
	tCS_DB_ServiceManageData 		* element;
	MV_stServiceInfo              	Service_Data;

	element = (tCS_DB_ServiceManageData*)First;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
		{
			first_scramble = Service_Data.u8Scramble;
		}
		else
		{
			return 0;
		}

		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
			{
				second_scramble = Service_Data.u8Scramble;
			}
			else
			{
				return 0;
			}
			return (  second_scramble - first_scramble );
		}
	}
	return 0;
}

int cs_db_CompareHD_First(const void* First, const void* Second)
{

	U8  							first_HD, second_HD;
	tCS_DB_ServiceManageData 		* element;
	MV_stServiceInfo              	Service_Data;

	element = (tCS_DB_ServiceManageData*)First;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
		{
			first_HD = Service_Data.u8TvRadio;
		}
		else
		{
			return 0;
		}

		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
			{
				second_HD = Service_Data.u8TvRadio;
			}
			else
			{
				return 0;
			}
			return (  second_HD - first_HD );
		}
	}
	return 0;
}

int cs_db_CompareSD_First(const void* First, const void* Second)
{

	U8  							first_HD, second_HD;
	tCS_DB_ServiceManageData 		* element;
	MV_stServiceInfo              	Service_Data;

	element = (tCS_DB_ServiceManageData*)First;

	if(element != NULL)
	{
		if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
		{
			first_HD = Service_Data.u8TvRadio;
		}
		else
		{
			return 0;
		}

		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			if(MV_DB_GetServiceDataByIndex(&Service_Data, element->Service_Index) == eCS_DB_OK)
			{
				second_HD = Service_Data.u8TvRadio;
			}
			else
			{
				return 0;
			}
			return ( first_HD - second_HD);
		}
	}
	return 0;
}

int  cs_db_CompareLCN_Low_High(const void* First, const void* Second)
{

	S32							first_LCN, second_LCN;
	tCS_DB_ServiceManageData        * element;

	element = (tCS_DB_ServiceManageData*)First;
	if(element != NULL)
	{
		first_LCN = element->LCN;

		element = (tCS_DB_ServiceManageData*)Second;
		if(element != NULL)
		{
			second_LCN = element->LCN;
			return ( first_LCN - second_LCN );
		}
	}
	return 0;
}


tCS_DB_Error CS_DB_SortCurrentServiceList(tCS_DB_SortType SortType)
{
	tCS_DB_ServiceManageData		* sort_list;
	U16	size;

	//CSOS_WaitSemaphore(sem_ServiceListAccess);

	sort_list = Current_ServiceList.Service_Data;
	size	= Current_ServiceList.Service_Number;

	if((sort_list == NULL)||(size == 0))
		return eCS_DB_ERROR;

	switch ( SortType )
	{

		case eCS_DB_BY_NAME_A_Z :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareName_A_Z );
			break;

		case eCS_DB_BY_NAME_Z_A :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareName_Z_A );
			break;

		case eCS_DB_BY_FTA_CAS :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareFTA_First );
			break;

		case eCS_DB_BY_CSA_FTA :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareCAS_First );
			break;

		case eCS_DB_BY_SD_HD :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareSD_First );
			break;

		case eCS_DB_BY_HD_SD :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareHD_First );
			break;

		case eCS_DB_BY_NORMAL:
			CS_DB_SetCurrentList(Current_ServiceList.List_Triplet, TRUE);
			break;

		default:
			break;
/*
		case eCS_DB_BY_FREQUENCY_LOW_HIGH :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareFreq_Low_High );
			break;

		case eCS_DB_BY_FTA_FIRST:
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareFTA_First );
			break;

		case eCS_DB_BY_LCN_LOW_HIGH :
			qsort((void*)sort_list, (size_t)size, (size_t)sizeof(tCS_DB_ServiceManageData), cs_db_CompareLCN_Low_High );
			break;
*/
	}

	switch(Current_ServiceList.List_Triplet.sCS_DB_ServiceListType)
	{
		case eCS_DB_TV_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST] = SortType;
			break;
		case eCS_DB_RADIO_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST] = SortType;
			break;
		case eCS_DB_FAV_TV_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_DEFAILT_LIST_NUM + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = SortType;
			break;
		case eCS_DB_FAV_RADIO_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_DEFAILT_LIST_NUM + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = SortType;
			break;
		case eCS_DB_SAT_TV_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_TV_LIST_ID][MV_FIXED_LIST_NUMBER + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = SortType;
			break;
		case eCS_DB_SAT_RADIO_LIST :
			u8Sort_Mode[kCS_DB_DEFAULT_RADIO_LIST_ID][MV_FIXED_LIST_NUMBER + Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue] = SortType;
			break;
		case eCS_DB_INVALID_LIST :
			break;
	}
	//CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_CurrentList_MoveService(U16 from_inlist, U16 to_inlist)
{
	tCS_DB_ServiceManageData		* sort_list;
	tCS_DB_ServiceManageData            temp_from;
	U16	size;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	sort_list = Current_ServiceList.Service_Data;
	size	= Current_ServiceList.Service_Number;

	if((sort_list == NULL)||(size == 0))
		return eCS_DB_ERROR;

	if((from_inlist>=Current_ServiceList.Service_Number)||(to_inlist>=Current_ServiceList.Service_Number))
		return eCS_DB_ERROR;

	if(from_inlist == to_inlist)
		return eCS_DB_OK;

	temp_from = sort_list[from_inlist];

	if(from_inlist < to_inlist)
	{
		memmove(&(sort_list[from_inlist]), &(sort_list[from_inlist+1]), (to_inlist -from_inlist)*sizeof(tCS_DB_ServiceManageData) );
	}
	else if(from_inlist > to_inlist)
	{
		memmove(&(sort_list[to_inlist+1]), &(sort_list[to_inlist]), (from_inlist -to_inlist)*sizeof(tCS_DB_ServiceManageData) );
	}

	sort_list[to_inlist] = temp_from;

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}

tCS_DB_Error cs_db_SaveCurrentList_ServiceOrder(void)
{
	U16 			index = 0;
	tCS_DB_Error error = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	switch(Current_ServiceList.List_Triplet.sCS_DB_ServiceListType)
	{
		case eCS_DB_TV_LIST :
			if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] ==  Current_ServiceList.Service_Number)
			{
				for(index = 0; index < Current_ServiceList.Service_Number; index++)
				{
					u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][index] = Current_ServiceList.Service_Data[index].Service_Index;
				}
			}
			else
			{
				error = eCS_DB_ERROR;
			}
			break;

		case eCS_DB_RADIO_LIST:
			if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID] == Current_ServiceList.Service_Number)
			{
				for(index = 0; index < Current_ServiceList.Service_Number; index++)
				{
					u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][index] = Current_ServiceList.Service_Data[index].Service_Index;
				}
			}
			else
			{
				error = eCS_DB_ERROR;
			}
			break;

		case eCS_DB_FAV_TV_LIST :
			if(Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue < MV_MAX_FAV_KIND )
			{
				if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] == Current_ServiceList.Service_Number)
				{
					for(index = 0; index < Current_ServiceList.Service_Number; index++)
					{
						stFavInfo_Glob[Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue].u16FavChIndex[kCS_DB_DEFAULT_TV_LIST_ID][index] = Current_ServiceList.Service_Data[index].Service_Index;
					}
				}
				else
				{
					error = eCS_DB_ERROR;
				}
			}
			else
			{
				error = eCS_DB_ERROR;
			}
			break;
		case eCS_DB_INVALID_LIST:
		default:
			error = eCS_DB_ERROR;
			break;

	}
	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return (error);
}

U16	CS_DB_Get_Sub_Count(void)
{
	int 		i;
	U16			u16Count = 0;

	for( i = 0 ; i < Current_ServiceList.Current_Service ; i++ )
	{
		if(Current_ServiceList.Service_Data[i].Delete_Flag == eDBASE_DELETE)
			u16Count++;
	}

	return u16Count;
}

tCS_DB_Error CS_DB_SaveListModifications (void)
{
	U16  					i;
	tCS_DB_Error 			error = eCS_DB_OK;
	U8						u8TvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	//tCS_DB_ServiceList	* service_list;

	error = cs_db_SaveCurrentList_ServiceOrder();

	if ( Current_ServiceList.List_Triplet.sCS_DB_ServiceListType == eCS_DB_FAV_TV_LIST )
		u8TvRadio = kCS_DB_DEFAULT_TV_LIST_ID;
	else if ( Current_ServiceList.List_Triplet.sCS_DB_ServiceListType == eCS_DB_FAV_RADIO_LIST )
		u8TvRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;

	if(Current_ServiceList.List_Triplet.sCS_DB_ServiceListType == eCS_DB_FAV_TV_LIST || Current_ServiceList.List_Triplet.sCS_DB_ServiceListType == eCS_DB_FAV_RADIO_LIST)
	{
		for( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
		{
			if(Current_ServiceList.Service_Data[i].Delete_Flag == eDBASE_DELETE)
			{
				// printf("\n ====== %d : %d ====== \n", Current_ServiceList.Service_Number, Current_ServiceList.Service_Data[i].Service_Index);
				CS_DB_RemoveFavoriteService(Current_ServiceList.Service_Data[i].Service_Index, u8TvRadio, Current_ServiceList.List_Triplet.sCS_DB_ServiceListTypeValue);
			}
		}

	}
	else
	{
		for( i = 0 ; i < Current_ServiceList.Service_Number ; i++)
		{
			if(Current_ServiceList.Service_Data[i].Delete_Flag == eDBASE_DELETE)
			{
				//printf("CS_DB_SaveListModifications :: %d - %d ##################\n", i, Current_ServiceList.Service_Data[i].Service_Index);
				MV_DB_DeleteOneService(Current_ServiceList.Service_Data[i].Service_Index);
			}
		}
	}

	return(error);
}

tCS_DB_Error MV_DB_AddFavoriteService_Select_Clear(void)
{
	int 	i;

	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
		if ( Current_ServiceList.Service_Data[i].Select_Flag == eDBASE_SELECT )
			Current_ServiceList.Service_Data[i].Select_Flag = eDBASE_NOT_SELECT;

	return eCS_DB_OK;
}

tCS_DB_Error MV_DB_AddFavoriteService_Select(U8 favoriteindex)
{
	tCS_DB_ServiceManageData 	Temp_SerMngData;
	int 						i;
	U8							u8TVRadio = 0;

	switch( Current_ServiceList.List_Triplet.sCS_DB_ServiceListType )
	{
		case eCS_DB_TV_LIST:
		case eCS_DB_SAT_TV_LIST:
		case eCS_DB_FAV_TV_LIST:
		default:
			u8TVRadio = kCS_DB_DEFAULT_TV_LIST_ID;
			break;

		case eCS_DB_RADIO_LIST:
		case eCS_DB_SAT_RADIO_LIST:
		case eCS_DB_FAV_RADIO_LIST:
			u8TVRadio = kCS_DB_DEFAULT_RADIO_LIST_ID;
			break;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);
	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		if ( Current_ServiceList.Service_Data[i].Select_Flag == eDBASE_SELECT )
		{
			Current_ServiceList.Service_Data[i].Select_Flag = eDBASE_NOT_SELECT;

			Temp_SerMngData = Current_ServiceList.Service_Data[i];

			/* By KB Kim 2011.01.20 */
			// if (MV_DB_CheckFavoriteServiceBySrvIndex( stCHInfo_Glob[Temp_SerMngData.Service_Index].u8TvRadio - 1, Temp_SerMngData.Service_Index, favoriteindex ) < MV_MAX_FAV_COUNT )
			if (MV_DB_CheckFavoriteServiceBySrvIndex(u8TVRadio, Temp_SerMngData.Service_Index, favoriteindex ) < MV_MAX_FAV_COUNT )
			{
				continue;
			}

			stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TVRadio][stFavInfo_Glob[favoriteindex].u8FavCount[u8TVRadio]] = Temp_SerMngData.Service_Index;
			stFavInfo_Glob[favoriteindex].u8FavCount[u8TVRadio]++;
		}
	}
	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}

tCS_DB_Error MV_DB_AddFavoriteService(U8 favoriteindex, U8 u8TvRadio, U16 u16ServiceIndex)
{
	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if ( MV_DB_CheckFavoriteServiceBySrvIndex( u8TvRadio, u16ServiceIndex, favoriteindex ) < MV_MAX_FAV_COUNT )
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	if ( u8TvRadio == kCS_DB_DEFAULT_TV_LIST_ID )
	{
		// printf("\n====== MV_DB_AddFavoriteService : %d, %d\n", favoriteindex, u16ServiceIndex);
		stFavInfo_Glob[favoriteindex].u16FavChIndex[kCS_DB_DEFAULT_TV_LIST_ID][stFavInfo_Glob[favoriteindex].u8FavCount[kCS_DB_DEFAULT_TV_LIST_ID]] = u16ServiceIndex;
		stFavInfo_Glob[favoriteindex].u8FavCount[kCS_DB_DEFAULT_TV_LIST_ID]++;
	}
	else if ( u8TvRadio == kCS_DB_DEFAULT_RADIO_LIST_ID )
	{
		stFavInfo_Glob[favoriteindex].u16FavChIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][stFavInfo_Glob[favoriteindex].u8FavCount[kCS_DB_DEFAULT_RADIO_LIST_ID]] = u16ServiceIndex;
		stFavInfo_Glob[favoriteindex].u8FavCount[kCS_DB_DEFAULT_RADIO_LIST_ID]++;
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_RemoveFavoriteService(U16 pServiceIndex, U8 u8TvRadio, U8 favoriteindex)
{
	U16 index = 0;

	if(pServiceIndex >= MV_MAX_SERVICE_COUNT)
	{
		return eCS_DB_ERROR;
	}

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	if(eCS_DB_INVALID == stCHInfo_Glob[pServiceIndex].u8Valid)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

	if(pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] > 0)
	{
		index = MV_DB_CheckFavoriteServiceBySrvIndex( u8TvRadio, pServiceIndex, favoriteindex );

		if (  index < MV_MAX_FAV_COUNT )
		{
			/* By KB Kim 2011.01.20 */
			// if( index != (MV_MAX_SERVICE_COUNT-1) )
			{
				// printf("\nCS_DB_RemoveFavoriteService : Index : %d\n", index);
				/* By KB Kim 2011.01.20 */
				// memcpy(&stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index], &stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index+1], 2*(MV_MAX_FAV_COUNT -index-1));
				/* For FavList Delete Problem By KB Kim 2012.04.21 */
				// memcpy(&stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index], &stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index+1], (MV_MAX_FAV_COUNT - index - 1));
				memcpy(&stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index], &stFavInfo_Glob[favoriteindex].u16FavChIndex[u8TvRadio][index+1], sizeof(U16)*(MV_MAX_FAV_COUNT - index - 1));
				stFavInfo_Glob[favoriteindex].u8FavCount[u8TvRadio]--;
				/* By KB Kim 2011.01.20 */
				//pServiceListInfo->u16DB_List_ServiceNum[u8TvRadio]--;  // Disable comment : if Favorite Channel list delete then all satellite channel count discrease
			}
		}
	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_GetSlistName(char *name, U8 ListIndex)
{
	if(name == NULL)
	{
		return eCS_DB_ERROR;
	}
	else if(ListIndex >= (MV_LIST_NUMBER + MV_MAX_FAV_KIND))
	{
		return eCS_DB_ERROR;
	}
	else
	{
		CSOS_WaitSemaphore(sem_ServiceListAccess);
		strcpy(name, cServiceListName[ListIndex]);
		CSOS_SignalSemaphore(sem_ServiceListAccess);
	}

	return eCS_DB_OK;
}

U16 CS_DB_Add_TP(void)
{
	U32		TP_Index;
	/* For Blind Scan By KB Kim 2011.02.26 */
	// U16     index;

	for ( TP_Index = 0 ; TP_Index < MAX_TP_COUNT ; TP_Index++ )
	{
		if ( stTPInfo_Glob[TP_Index].u8Valid == DB_INVALID || stTPInfo_Glob[TP_Index].u8SatelliteIndex == MV_SAT_MAX )
		{
			return stTPInfo_Glob[TP_Index].u16TPIndex;
		}
	}

	return stTPInfo_Glob[TP_Index].u16TPIndex;
}

U16 CS_DB_Get_AllTPCount(void)
{
	U32		i;

	for ( i = 0 ; i < MAX_TP_COUNT ; i++ )
		if ( stTPInfo_Glob[i].u8SatelliteIndex == MV_SAT_MAX )
			break;

	return i;
}


tCS_DB_Error compare_add_tp ( MV_stTPInfo *TPInfo )
{
	U16 	i;
	/* Add Problem for low symbol by KB Kim 2011.03.04 */
	U16     tpRange;

	/* Add Problem for low symbol by KB Kim 2011.03.04 */
	tpRange = (TPInfo->u16SymbolRate + 1000) / 2000;
	tpRange += 1;

	if (tpRange > 10)
	{
		tpRange = 10;
	}

	for( i = 0 ; i < MAX_TP_COUNT ; i++ )
	{
		/* Add Problem for low symbol by KB Kim 2011.03.04 */
		if ( ( TPInfo->u16TPFrequency - tpRange < stTPInfo_Glob[i].u16TPFrequency )
			&& ( TPInfo->u16TPFrequency + tpRange > stTPInfo_Glob[i].u16TPFrequency )
			&& ( TPInfo->u8SatelliteIndex == stTPInfo_Glob[i].u8SatelliteIndex ) )
		{
			if ( ( TPInfo->u16SymbolRate - 10 < stTPInfo_Glob[i].u16SymbolRate )
			&& ( TPInfo->u16SymbolRate + 10 > stTPInfo_Glob[i].u16SymbolRate )
			&& ( TPInfo->u8SatelliteIndex == stTPInfo_Glob[i].u8SatelliteIndex ) )
			{
				if ( TPInfo->u8Polar_H == stTPInfo_Glob[i].u8Polar_H )
					return FALSE;
			}
		}
	}

	return TRUE;
}

/* For Blind Scan By KB Kim 2011.02.26 */
U16 GetTpIndexByData (MV_stTPInfo *tPInfo)
{
	U16 	i;
	U16     tpRange;

	tpRange = (tPInfo->u16SymbolRate + 1000) / 2000;
	tpRange += 1;

	if (tpRange > 10)
	{
		tpRange = 10;
	}

	for( i = 0 ; i < MAX_TP_COUNT ; i++ )
	{
		if ( ( (tPInfo->u16TPFrequency - tpRange) < stTPInfo_Glob[i].u16TPFrequency )
			&& ( (tPInfo->u16TPFrequency + tpRange) > stTPInfo_Glob[i].u16TPFrequency )
			&& ( tPInfo->u8SatelliteIndex == stTPInfo_Glob[i].u8SatelliteIndex ) )
		{
			if (((tPInfo->u16SymbolRate - 10) < stTPInfo_Glob[i].u16SymbolRate )
			&& ( (tPInfo->u16SymbolRate + 10) > stTPInfo_Glob[i].u16SymbolRate )
			&& ( tPInfo->u8SatelliteIndex == stTPInfo_Glob[i].u8SatelliteIndex ) )
			{
				if ( tPInfo->u8Polar_H == stTPInfo_Glob[i].u8Polar_H )
				{
					tPInfo->u16TPIndex = stTPInfo_Glob[i].u16TPIndex;
					return stTPInfo_Glob[i].u16TPIndex;
				}
			}
		}
	}

	return MAX_TP_COUNT;
}

/* For Blind Scan By KB Kim 2011.02.26 */
tCS_DB_Error MV_UdateBlindTpData(MV_stTPInfo *tPInfo)
{
	U16 tpIndex;
	MV_stTPInfo tmpTpInfo;

	tpIndex = GetTpIndexByData (tPInfo);
	if (tpIndex >= MAX_TP_COUNT)
	{
		tpIndex = CS_DB_Add_TP();
		tPInfo->u16TPIndex = tpIndex;
		tmpTpInfo = *tPInfo;
		MV_ADD_TPData(tmpTpInfo);
	}

	if (tpIndex >= MAX_TP_COUNT)
	{
		return eCS_DB_ERROR;
	}

	return eCS_DB_OK;
}

tCS_DB_Error MV_DB_GetTPDataByIndex(MV_stTPInfo * pTPData, U16 pTPIndex)
{
	tCS_DB_Error  	status = eCS_DB_ERROR;
	U16				i = 0;

	CSOS_WaitSemaphore(sem_TPListAccess);

	while ( stTPInfo_Glob[i].u8SatelliteIndex != MV_SAT_MAX && i < MAX_TP_COUNT )
	{
		if ( stTPInfo_Glob[i].u16TPIndex == pTPIndex )
		{
			status = eCS_DB_OK;
			/* By KB Kim 2011.01.18 */
			*pTPData = stTPInfo_Glob[i];
			break;
		}
		else
			i++;
	}

	CSOS_SignalSemaphore(sem_TPListAccess);

	return(status);
}

tCS_DB_Error CS_DB_LoadDatabase(void)
{
	tCS_DB_Error		Checker = eCS_DB_OK;
	int					i, j;

	CSOS_WaitSemaphore(sem_DB_FLASH_Access);
	if (! CS_DB_FLASH_CRCCheck_Read( SAT_Flash_map_to_ram_Size, SAT_Flash_map_to_ram_Base, SAT_DB))
	{
		memset(SAT_Flash_map_to_ram_Base,0,SAT_Flash_map_to_ram_Size);

		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
		MV_DB_Get_Default_SatTPData();
		CS_DB_Save_SAT_Database();
		Checker = eCS_DB_ERROR;
	} else
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	CSOS_WaitSemaphore(sem_DB_FLASH_Access);
	if(! CS_DB_FLASH_CRCCheck_Read( INDEX_Flash_map_to_ram_Size, INDEX_Flash_map_to_ram_Base, INDEX_DB))
	{
		memset(INDEX_Flash_map_to_ram_Base,0,INDEX_Flash_map_to_ram_Size);
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
		CS_DB_Save_INDEX_Database();
		Checker = eCS_DB_ERROR;
	} else
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);

#ifndef TEMP_DB
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);
	if(! CS_DB_FLASH_CRCCheck_Read( CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB))
	{
		memset(CH_Flash_map_to_ram_Base,0,CH_Flash_map_to_ram_Size);

		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
		MV_DB_Get_Default_CHData();
		CS_DB_Save_CH_Database();
		Checker = eCS_DB_ERROR;
	} else
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
#else // #ifndef TEMP_DB
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);
	if(! CS_DB_FLASH_CRCCheck_Read( CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB))
	{
		printf("===== CH DB Error\n");
		memset(CH_Flash_map_to_ram_Base,0,CH_Flash_map_to_ram_Size);

		if(! CS_DB_FLASH_CRCCheck_Read( Temp_CH_Flash_map_to_ram_Size, Temp_CH_Flash_map_to_ram_Base, CH_DB))
		{
			printf("===== TEMP CH DB Error\n");
			CSOS_SignalSemaphore(sem_DB_FLASH_Access);
			MV_DB_Get_Default_CHData();
			CS_DB_Save_CH_Database();
			Checker = eCS_DB_ERROR;
		} else {

			memcpy(&u16ChIndex_Glob[0], &u16TempChIndex_Glob[0], sizeof(MV_stIndex) * MV_MAX_SERVICE_COUNT);
			memcpy(&stCHInfo_Glob[0], &stTempCHInfo_Glob[0], sizeof(MV_stServiceInfo) * MV_MAX_SERVICE_COUNT);
//			memcpy(&pServiceListInfo, &Temp_pServiceListInfo, sizeof(tCS_DB_ServiceListInfo));

			pServiceListInfo->u16DB_List_ServiceNum[0] = Temp_pServiceListInfo->u16DB_List_ServiceNum[0];
			pServiceListInfo->u16DB_List_ServiceNum[1] = Temp_pServiceListInfo->u16DB_List_ServiceNum[1];

			CSOS_SignalSemaphore(sem_DB_FLASH_Access);

			for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
			{
				strcpy( stFavInfo_Glob[i].acFavName , stTempFavInfo_Glob[i].acFavName );
				stFavInfo_Glob[i].u8FavCount[0] = stTempFavInfo_Glob[i].u8FavCount[0];
				stFavInfo_Glob[i].u8FavCount[1] = stTempFavInfo_Glob[i].u8FavCount[1];
				for ( j = 0 ; j < stTempFavInfo_Glob[i].u8FavCount[0] ; j++ )
				{
					stFavInfo_Glob[i].u16FavChIndex[0][j] = stTempFavInfo_Glob[i].u16FavChIndex[0][j];
				}

				for ( j = 0 ; j < stTempFavInfo_Glob[i].u8FavCount[1] ; j++ )
				{
					stFavInfo_Glob[i].u16FavChIndex[1][j] = stTempFavInfo_Glob[i].u16FavChIndex[1][j];
				}
//				printf("%d ==== Success : %s -> %s , %d , %d ======\n", i, stTempFavInfo_Glob[i].acFavName, stFavInfo_Glob[i].acFavName, stFavInfo_Glob[i].u8FavCount[0], stFavInfo_Glob[i].u8FavCount[1]);
			}

			CS_DB_Save_CH_Database();

#if 0
			dprintf(("############# %d , %d #################\n", pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));
/*
			pServiceListInfo->u16DB_List_ServiceNum[0] = 0;
			pServiceListInfo->u16DB_List_ServiceNum[1] = 0;
*/
			dprintf(("############# %d , %d #################\n", pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));

			printf("===== RE-Contect CH DB \n");
			CSOS_WaitSemaphore(sem_DB_FLASH_Access);
			if(! CS_DB_FLASH_CRCCheck_Read( CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB))
			{
				memset(CH_Flash_map_to_ram_Base,0,CH_Flash_map_to_ram_Size);

				CSOS_SignalSemaphore(sem_DB_FLASH_Access);
				MV_DB_Get_Default_CHData();
				CS_DB_Save_CH_Database();
				Checker = eCS_DB_ERROR;
			} else {
				CSOS_SignalSemaphore(sem_DB_FLASH_Access);
			}

			dprintf(("############# %d , %d #################\n", pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID], pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID]));

			for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
			{
				printf("%d ==== Success : %d , %d ======\n", i, stFavInfo_Glob[i].u8FavCount[0], stFavInfo_Glob[i].u8FavCount[1]);
			}
#endif
		}
	} else
	{
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
#if 0
		for ( i = 0 ; i < MV_MAX_FAV_KIND ; i++ )
		{
			printf("%d ==== OK Success : %s : %d , %d ======\n", i, stFavInfo_Glob[i].acFavName, stFavInfo_Glob[i].u8FavCount[0], stFavInfo_Glob[i].u8FavCount[1]);
		}
#endif
	}

#endif // #ifndef TEMP_DB
/*
	printf("===== RE-Contect CH DB \n");
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);
	if(! CS_DB_FLASH_CRCCheck_Read( CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB))
	{
		memset(CH_Flash_map_to_ram_Base,0,CH_Flash_map_to_ram_Size);

		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
		MV_DB_Get_Default_CHData();
		CS_DB_Save_CH_Database();
		Checker = eCS_DB_ERROR;
	} else
		CSOS_SignalSemaphore(sem_DB_FLASH_Access);
*/
	//CSOS_SignalSemaphore(sem_DB_FLASH_Access);
	return Checker;
}


tCS_DB_Error CS_DB_SaveDatabase(void)
{
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);

	MV_Store_SatListIndex();

	CS_DB_FLASH_CRCCheck_Write(CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB);
	CS_DB_FLASH_CRCCheck_Write(SAT_Flash_map_to_ram_Size, SAT_Flash_map_to_ram_Base, SAT_DB);
	CS_DB_FLASH_CRCCheck_Write(INDEX_Flash_map_to_ram_Size, INDEX_Flash_map_to_ram_Base, INDEX_DB);

	CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_Save_CH_Database(void)
{
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);

	CS_DB_FLASH_CRCCheck_Write(CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB);

	CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_Save_SAT_Database(void)
{
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);

	CS_DB_FLASH_CRCCheck_Write(SAT_Flash_map_to_ram_Size, SAT_Flash_map_to_ram_Base, SAT_DB);

	CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_Save_INDEX_Database(void)
{
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);

	MV_Store_SatListIndex();
	CS_DB_FLASH_CRCCheck_Write(INDEX_Flash_map_to_ram_Size, INDEX_Flash_map_to_ram_Base, INDEX_DB);

	CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	return eCS_DB_OK;
}

BOOL  CS_DB_CheckIfChanged(void)
{
	if(CS_DB_FLASH_CRCCheck_DataIfChanged(CH_Flash_map_to_ram_Size, CH_Flash_map_to_ram_Base, CH_DB) ||
		CS_DB_FLASH_CRCCheck_DataIfChanged(SAT_Flash_map_to_ram_Size, SAT_Flash_map_to_ram_Base, SAT_DB) ||
		CS_DB_FLASH_CRCCheck_DataIfChanged(INDEX_Flash_map_to_ram_Size, INDEX_Flash_map_to_ram_Base, INDEX_DB) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


tCS_DB_Error CS_DB_EraseDatabase (void)
{
	CSOS_WaitSemaphore(sem_DB_FLASH_Access);

	CS_DB_FLASH_Erase(CH_DB);
	CS_DB_FLASH_Erase(SAT_DB);
	CS_DB_FLASH_Erase(INDEX_DB);

	CSOS_SignalSemaphore(sem_DB_FLASH_Access);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_ResetDatabase (void)
{
	tCS_DBU_Service	ServiceTriplet;
	char			acTemp_Str[50];
	char 			ShellCommand[256];

  	memset(acTemp_Str, 0x00, 50);
	sprintf(acTemp_Str, "rm %s", CH_DB_FILE);
	system(acTemp_Str);

	sprintf(acTemp_Str, "rm %s", SAT_DB_FILE);
	system(acTemp_Str);

	sprintf(acTemp_Str, "rm %s", INDEX_DB_FILE);
	system(acTemp_Str);

	sprintf(ShellCommand, "cp %s/* %s/", DEFAULT_DIR, DB_DIR);
	system( ShellCommand );

	MV_DB_Init(TRUE);

	CS_DBU_InitDefaultUserData();
	CS_DBU_LoadUserSettingDataInHW();
	CS_DBU_LoadVolume();
	CS_DBU_LoadMuteStatus();
	CS_DBU_LoadCurrentService(&ServiceTriplet);

	memset(&Last_Service, 0 , sizeof(tCS_DBU_Service));

	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_ResetDB (void)
{
	char			acTemp_Str[50];
	char 			ShellCommand[256];

  	memset(acTemp_Str, 0x00, 50);
	sprintf(acTemp_Str, "rm %s", CH_DB_FILE);
	system(acTemp_Str);

	sprintf(acTemp_Str, "rm %s", SAT_DB_FILE);
	system(acTemp_Str);

	sprintf(acTemp_Str, "rm %s", INDEX_DB_FILE);
	system(acTemp_Str);

	sprintf(ShellCommand, "cp %s/* %s/", DEFAULT_DIR, DB_DIR);
	system( ShellCommand );

	return eCS_DB_OK;
}


tCS_DB_Error CS_DB_RestoreDatabase (void)
{
	tCS_DBU_Service	ServiceTriplet;

	MV_DB_Init(TRUE);
	CS_DBU_LoadCurrentService(&ServiceTriplet);

	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_Restore_CH_UseIndex (void)
{
	tCS_DBU_Service	ServiceTriplet;

	Load_CH_UseIndex();
	CS_DBU_LoadCurrentService(&ServiceTriplet);

	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);

	return eCS_DB_OK;
}

tCS_DB_Error CS_DB_All_Ch_Delete (void)
{
	tCS_DBU_Service	ServiceTriplet;
	char			acTemp_Str[50];

  	memset(acTemp_Str, 0x00, 50);
	sprintf(acTemp_Str, "rm %s", CH_DB_FILE);
	system(acTemp_Str);

	sprintf(acTemp_Str, "rm %s", INDEX_DB_FILE);
	system(acTemp_Str);

	MV_DB_Init(TRUE);
	CS_DBU_LoadCurrentService(&ServiceTriplet);

	memset(&Last_Service, 0 , sizeof(tCS_DBU_Service));

	CS_DB_SetCurrentList( ServiceTriplet.sCS_DBU_ServiceList, TRUE);
	CS_DB_SetCurrentService_OrderIndex(ServiceTriplet.sCS_DBU_ServiceIndex);

	return eCS_DB_OK;
}

tCS_DB_Error	CS_DBU_InitDefaultUserData(void)
{
	tCS_DBU_Service	ServiceTriplet;

	// printf("CS_DBU_InitDefaultUserData\n");
	CS_DBU_SetScartMode(kCS_DBU_DEFAULT_ScartMode);
	CS_DBU_SetSpdifStatus(kCS_DBU_DEFAULT_SpdifStatus);
	CS_DBU_SetVideoOutput(kCS_DBU_DEFAULT_VideoOutput);
	CS_DBU_SetVideoDefinition(kCS_DBU_DEFAULT_VideoDefinition);
	CS_DBU_SetVideoEncodingMode(kCS_DBU_DEFAULT_VideoEncodingMode);
	CS_DBU_SetVideoAspectRatio(kCS_DBU_DEFAULT_VideoAspectRatio);
	CS_DBU_SetAspectRatioMode(kCS_DBU_DEFAULT_AspectRatioMode);
	CS_DBU_SetAudioLanguage(kCS_DBU_DEFAULT_AudioLanguage);
	CS_DBU_SetSubtitleLanguage(kCS_DBU_DEFAULT_SubtitleLanguage);
	CS_DBU_SetMenuLanguage(kCS_DBU_DEFAULT_MenuLanguage);
	CS_DBU_SetTimeOffset(kCS_DBU_DEFAULT_TimeOffset);
	CS_DBU_SetTimeMode(kCS_DBU_DEFAULT_TimeMode);
	CS_DBU_SetTimeRegion(kCS_DBU_DEFAULT_TimeRegion);
	CS_DBU_SetPinCode(kCS_DBU_DEFAULT_PinCode);
	CS_DBU_SetBootLockStatus(kCS_DBU_DEFAULT_BootLockStatus);
	CS_DBU_SetInstallLockStatus(kCS_DBU_DEFAULT_InstallLockStatus);
	CS_DBU_SetEditLockStatus(kCS_DBU_DEFAULT_EditLockStatus);
	CS_DBU_SetServicesLockStatus(kCS_DBU_DEFAULT_ServicesLockStatus);
	CS_DBU_SetParentalLockStatus(kCS_DBU_DEFAULT_ParentalLockStatus);
	CS_DBU_SetScreenTransparent(kCS_DBU_DEFAULT_ScreenTransparent);
	CS_DBU_SetBannerKeepTime(kCS_DBU_DEFAULT_BannerKeepTime);
	CS_DBU_SetLCNMode(kCS_DBU_DEFAULT_LcnMode);
	CS_DBU_SetVolume(kCS_DBU_DEFAULT_Volume);
	CS_DBU_SetLNB_Power(kCS_DBU_DEFAULT_LNBPower);
	CS_DBU_SetCH_Change_Type(kCS_DBU_DEFAULT_CH_Change);
	CS_DBU_SetCH_List_Type(kCS_DBU_DEFAULT_CH_List);
	CS_DBU_SetRecall_Type(kCS_DBU_DEFAULT_Recall);
	CS_DBU_SetPower_Type(kCS_DBU_DEFAULT_Power);
	CS_DBU_SetLED_Type(kCS_DBU_DEFAULT_LED);
	CS_DBU_SetMuteStatus(kCS_DBU_DEFAULT_MuteStatus);
	CS_DBU_SetDHCP_Type(kCS_DBU_DEFAULT_DHCP);
	CS_DBU_SetDNS_Type(kCS_DBU_DEFAULT_DNS_Type);
	CS_DBU_SetAntenna_Type(kCS_DBU_DEFAULT_Antena);
	CS_DBU_SetLocal_Longitude(kCS_DBU_DEFAULT_Local_Logitude);
	CS_DBU_SetLocal_Latitude(kCS_DBU_DEFAULT_Local_Latitude);
	CS_DBU_SetFont_Type(kCS_DBU_DEFAULT_Font_Type);
	CS_DBU_SetFont_Size(kCS_DBU_DEFAULT_Font_Size);
	CS_DBU_SetPower_Off_Mode(kCS_DBU_DEFAULT_Power_Off_Mode);
	CS_DBU_Set_Push_Game_Level(kCS_DBU_DEFAULT_PushGame_Lavel);
	CS_DBU_Set_Motor_Limit(kCS_DBU_DEFAULT_Motor_Limit);
	CS_DBU_Set_Webaddr(MERIH_VIDEO_URL);
	CS_DBU_Set_PlugInAddr(MERIH_VIDEO_URL);	/* By KB Kim for Plugin Setting : 2011.05.07 */
	CS_DBU_Set_Skin(kCS_DBU_DEFAULT_Skin);
	CS_DBU_Set_Sleep(kCS_DBU_DEFAULT_Sleep);
	CS_DBU_SetANI_Type(kCS_DBU_DEFAULT_Ani);
	/* For Heart bit control By KB Kim 2011.03.11 */
	CS_DBU_SetHeartBit(kCS_DBU_DEFAULT_HeartBit);
	CS_DBU_Set_Use_SubTitle(kCS_DBU_DEFAULT_UseSubTitle);
	CS_DBU_Set_Fixed_Font_Size(kCS_DBU_DEFAULT_FixFontSize);

	ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
	ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
	ServiceTriplet.sCS_DBU_ServiceIndex = 0;

	CS_DBU_SaveUserSettingDataInHW();
	CS_DBU_SaveVolume();
	CS_DBU_SaveMuteStatus();
	CS_DBU_SaveCurrentService(ServiceTriplet);

	return(eCS_DB_OK);
}


tCS_DB_Error	CS_DBU_SaveUserSettingDataInHW(void)
{
	strncpy(UserSetting_Data.sCS_DBU_CheckString, EPP_CHECK_STR, 4);

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_USER_SETTING_DATA_BASE, (U8 *)(&UserSetting_Data), kCS_E2P_USER_SETTING_DATA_SIZE);
	// printf ("CS_DBU_SaveUserSettingDataInHW : %d\n", kCS_E2P_USER_SETTING_DATA_SIZE);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DB_Error	CS_DBU_LoadUserSettingDataInHW(void)
{
	CS_ErrorCode_t  err;
	U8              data;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	// printf("CS_DBU_LoadUserSettingDataInHW !!!!!!!\n");
	err = E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_USER_SETTING_DATA_BASE, (U8 *)(&UserSetting_Data), kCS_E2P_USER_SETTING_DATA_SIZE);
	// printf("CS_DBU_LoadUserSettingDataInHW : Error Status [%d]\n", err);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	/* YXSDF - 22.12.2012 - EEPROM dan okunan temanin yuklenmesi icin*/
	/*if (err == CSAPI_SUCCEED) {
		MV_Resource_Change_Cfg_File(UserSetting_Data.sCS_DBU_Skin);
		//printf("Yuklenen Tema: %d\n", UserSetting_Data.sCS_DBU_Skin + 1);
	}*/ //v37de disable ettim... DREJO...

	// printf("CS_DBU_LoadUserSettingDataInHW sCS_DBU_CheckString = %s, %s %d\n", UserSetting_Data.sCS_DBU_CheckString, EPP_CHECK_STR, UserSetting_Data.sCS_DBU_VideoDefinition);
	if((err != CS_NO_ERROR)||(strncmp(UserSetting_Data.sCS_DBU_CheckString, EPP_CHECK_STR, 4)))
	{
		CS_DBU_InitDefaultUserData();
	}

	return(eCS_DB_OK);
}


BOOL CS_DBU_CheckIfUserSettingDataChanged(void)
{
	tCS_DBU_UserSetting_Save		Dataread;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_USER_SETTING_DATA_BASE, (U8 *)(&Dataread), kCS_E2P_USER_SETTING_DATA_SIZE);

	if(memcmp((void*)&Dataread, (void*)&UserSetting_Data, sizeof(tCS_DBU_UserSetting_Save)) != 0)
	{
		CSOS_SignalSemaphore(sem_DBU_E2P_Access);
		return TRUE;

	}
	else
	{
		CSOS_SignalSemaphore(sem_DBU_E2P_Access);
		return FALSE;
	}

}


tCS_DB_Error  CS_DBU_SetScartMode(tCS_DBU_ScartMode ScartMode)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_ScartMode = ScartMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_ScartMode	CS_DBU_GetScartMode(void)
{
	tCS_DBU_ScartMode 	returnvalue = kCS_DBU_DEFAULT_ScartMode;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_ScartMode <= eCS_DBU_SCARTRGB)
		returnvalue = (tCS_DBU_ScartMode)UserSetting_Data.sCS_DBU_ScartMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error  CS_DBU_SetSpdifStatus(tCS_DBU_Status SpdifStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_SpdifStatus = SpdifStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DBU_Status	CS_DBU_GetSpdifStatus (void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_SpdifStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_SpdifStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_SpdifStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error CS_DBU_SetVideoOutput( tCS_DBU_VideoOutput VideoOutputType )
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_VideoOutPut = VideoOutputType;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_VideoOutput  CS_DBU_GetVideoOutput( void )
{
	tCS_DBU_VideoOutput 	returnvalue = kCS_DBU_DEFAULT_VideoOutput;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	/* By KB Kim 2010.08.31 for RGB Control */
	if(UserSetting_Data.sCS_DBU_VideoOutPut < eCS_DBU_OUTPUT_MAX)
	{
		returnvalue = (tCS_DBU_VideoOutput)UserSetting_Data.sCS_DBU_VideoOutPut;
	}
	else
	{
		/* For Backward compatibility */
		returnvalue = eCS_DBU_OUTPUT_YPBPR;
	}

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error CS_DBU_SetVideoDefinition ( tCS_DBU_VideoDefinition VideoDefinition )
{
	// printf("CS_DBU_SetVideoDefinition : VideoDefinition = %d\n", VideoDefinition);
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_VideoDefinition = VideoDefinition;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_VideoDefinition CS_DBU_GetVideoDefinition ( void )
{
	tCS_DBU_VideoDefinition 	returnvalue = kCS_DBU_DEFAULT_VideoDefinition;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_VideoDefinition <= eCS_DBU_DEFINITION_AUTOMATIC)
		returnvalue = (tCS_DBU_VideoDefinition)UserSetting_Data.sCS_DBU_VideoDefinition;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}



tCS_DB_Error  CS_DBU_SetVideoEncodingMode(tCS_DBU_VideoEncodingMode pMode)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_VideoEncodingMode = pMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}




tCS_DBU_VideoEncodingMode	CS_DBU_GetVideoEncodingMode(void)
{
	tCS_DBU_VideoEncodingMode 	returnvalue = kCS_DBU_DEFAULT_VideoEncodingMode;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_VideoEncodingMode <= eCS_DBU_AUTOMATIC)
		returnvalue = (tCS_DBU_VideoEncodingMode)UserSetting_Data.sCS_DBU_VideoEncodingMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error  CS_DBU_SetVideoAspectRatio(tCS_DBU_VideoAspectRatio pMode)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_VideoAspectRatio = pMode;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_VideoAspectRatio	CS_DBU_GetVideoAspectRatio(void)
{
	tCS_DBU_VideoAspectRatio 	returnvalue = kCS_DBU_DEFAULT_VideoAspectRatio;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_VideoAspectRatio <= eCS_DBU_ASPECT_RATIO_AUTOMATIC)
		returnvalue = (tCS_DBU_VideoAspectRatio)UserSetting_Data.sCS_DBU_VideoAspectRatio;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error  CS_DBU_SetAspectRatioMode (tCS_DBU_AspectRatioMode pMode)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_AspectRatioMode = pMode;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_AspectRatioMode CS_DBU_GetAspectRatioMode (void)
{
	tCS_DBU_AspectRatioMode 	returnvalue = kCS_DBU_DEFAULT_AspectRatioMode;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_AspectRatioMode <= eCS_DBU_ARM_PANSCAN)
		returnvalue = (tCS_DBU_AspectRatioMode)UserSetting_Data.sCS_DBU_AspectRatioMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetDefaultAudioType(tCS_DBU_AudioType audio_type)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_DefaultAudioType = audio_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(eCS_DB_OK);
}

tCS_DBU_AudioType CS_DBU_GetDefaultAudioType(void)
{
	tCS_DBU_AudioType   returnvalue = kCS_DBU_DEFAULT_AudioType;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_DefaultAudioType <= eCS_DBU_AUDIO_AC3)
		returnvalue = (tCS_DBU_AudioType)UserSetting_Data.sCS_DBU_DefaultAudioType;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetAudioLanguage(U32 u32AudioLang)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_AudioLanguage = u32AudioLang;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32 CS_DBU_GetAudioLanguage(void)
{
    U32 u32AudioLang;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	u32AudioLang = UserSetting_Data.sCS_DBU_AudioLanguage;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(u32AudioLang);
}

tCS_DB_Error  CS_DBU_SetSubtitleLanguage(U32 u32SubLang)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_SubtitleLanguage = u32SubLang;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32 CS_DBU_GetSubtitleLanguage(void)
{
    U32 u32SubLang;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	u32SubLang = UserSetting_Data.sCS_DBU_SubtitleLanguage;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(u32SubLang);
}

tCS_DB_Error  CS_DBU_SetMenuLanguage(U32 u32MenuLang)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_MenuLanguage = u32MenuLang;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(status);
}


U32 CS_DBU_GetMenuLanguage(void)
{
	U32 u32MenuLang;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

    u32MenuLang = UserSetting_Data.sCS_DBU_MenuLanguage;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(u32MenuLang);
}



tCS_DB_Error  CS_DBU_SetTimeOffset(U32 OffsetUTC)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if((OffsetUTC &0xffff)<= 0x1200)
		UserSetting_Data.sCS_DBU_TimeOffset = OffsetUTC;
	else
		status = eCS_DB_ERROR;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}


U32  CS_DBU_GetTimeOffset (void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_TimeOffset;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if((UserSetting_Data.sCS_DBU_TimeOffset &0xffff)<= 0x1200)
		returnvalue = UserSetting_Data.sCS_DBU_TimeOffset;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error  CS_DBU_SetTimeMode(tCS_DBU_TimeMode TimeMode)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_TimeMode = TimeMode;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_TimeMode  CS_DBU_GetTimeMode(void)
{
	tCS_DBU_TimeMode 	returnvalue = kCS_DBU_DEFAULT_TimeMode;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_TimeMode <= eCS_DBU_TIME_INTERNET)
		returnvalue = (tCS_DBU_TimeMode)UserSetting_Data.sCS_DBU_TimeMode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetTimeRegion(U32 region_index)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_TimeRegion = region_index;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

U32  CS_DBU_GetTimeRegion(void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_TimeRegion;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_TimeRegion;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetPinCode(char * PinCode)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memcpy(UserSetting_Data.sCS_DBU_PinCode, PinCode, 4);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(status);
}

char * CS_DBU_GetPinCode(void)
{

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memcpy(Pin_code_buffer, UserSetting_Data.sCS_DBU_PinCode, 4);
         Pin_code_buffer[4] = 0;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Pin_code_buffer);
}

tCS_DB_Error  CS_DBU_Set_Webaddr(char * Webaddr)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memset(UserSetting_Data.sCS_DBU_WebSite_Address, 0x00, 64);
	memcpy(UserSetting_Data.sCS_DBU_WebSite_Address, Webaddr, strlen(Webaddr));

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(status);
}

char * CS_DBU_Get_Webaddr(void)
{

	memset( Website, 0x00, 64);
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memcpy(Website, UserSetting_Data.sCS_DBU_WebSite_Address, 64);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Website);
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
tCS_DB_Error  CS_DBU_Set_PlugInAddr(char *plugInAddr)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memset(UserSetting_Data.sCS_DBU_PluginSite_Address, 0x00, 64);
	memcpy(UserSetting_Data.sCS_DBU_PluginSite_Address, plugInAddr, strlen(plugInAddr));

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(status);
}

/* By KB Kim for Plugin Setting : 2011.05.07 */
char * CS_DBU_Get_PlugInAddr(void)
{
	memset( PlugInAddr, 0x00, 64);
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	memcpy(PlugInAddr, UserSetting_Data.sCS_DBU_PluginSite_Address, 64);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(PlugInAddr);
}

tCS_DB_Error  CS_DBU_SetInstallLockStatus(tCS_DBU_Status LockStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_InstallLockStatus = LockStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DBU_Status	CS_DBU_GetInstallLockStatus (void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_InstallLockStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_InstallLockStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_InstallLockStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetBootLockStatus(tCS_DBU_Status LockStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_BootLockStatus = LockStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DBU_Status	CS_DBU_GetBootLockStatus (void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_BootLockStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_InstallLockStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_BootLockStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetEditLockStatus(tCS_DBU_Status LockStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_EditLockStatus = LockStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}


tCS_DBU_Status	CS_DBU_GetEditLockStatus(void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_EditLockStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_EditLockStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_EditLockStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error  CS_DBU_SetServicesLockStatus(tCS_DBU_Status LockStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_ServicesLockStatus = LockStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}



tCS_DBU_Status	CS_DBU_GetServicesLockStatus(void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_ServicesLockStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_ServicesLockStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_ServicesLockStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}



tCS_DB_Error  CS_DBU_SetParentalLockStatus(tCS_DBU_Status LockStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_ParentalLockStatus = LockStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}



tCS_DBU_Status	CS_DBU_GetParentalLockStatus(void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_ParentalLockStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_ParentalLockStatus <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)UserSetting_Data.sCS_DBU_ParentalLockStatus;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetParentalRate(U32 age)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	UserSetting_Data.sCS_DBU_ParentalRate = age;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}



U32	CS_DBU_GetParentalRate(void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_ParentalRate;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_ParentalRate;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}




tCS_DB_Error  CS_DBU_SetScreenTransparent(U32 trans)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(trans <= kCS_DBU_MAX_ScreenTransparent)
		UserSetting_Data.sCS_DBU_ScreenTransparent = trans;
	else
		status = eCS_DB_ERROR;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}


U32 CS_DBU_GetScreenTransparent(void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_ScreenTransparent;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_ScreenTransparent <= kCS_DBU_MAX_ScreenTransparent)
		returnvalue = UserSetting_Data.sCS_DBU_ScreenTransparent;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetLNB_Power(U32 lnb_power)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_LNB_Power = lnb_power;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetLNB_Power(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_LNB_Power;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetCH_Change_Type(U32 change_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_CH_Change = change_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetCH_Change_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_CH_Change;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetCH_List_Type(U32 list_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_CH_List_Type = list_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetCH_List_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_CH_List_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetRecall_Type(U32 recall_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Recall = recall_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetRecall_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Recall;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetPower_Type(U32 Power_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Power = Power_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetPower_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Power;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetLED_Type(U32 LED_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_LED = LED_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetLED_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_LED;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetANI_Type(U8 Ani_type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Ani = Ani_type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U8	CS_DBU_GetANI_Type(void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Ani;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

/* For Heart bit control By KB Kim 2011.03.11 */
tCS_DB_Error  CS_DBU_SetHeartBit(U8 enable)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_HeartBit = enable;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

/* For Heart bit control By KB Kim 2011.03.11 */
U8	CS_DBU_GetHeartBit (void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_HeartBit;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetDNS_Type(U32 DNS_Status)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_DNS_Type = DNS_Status;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetPower_Off_Mode(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Power_Off_Mode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetPower_Off_Mode(U32 Power_Off_Mode)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Power_Off_Mode = Power_Off_Mode;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetDNS_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_DNS_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetDHCP_Type(U32 DHCP_Status)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_DHCP = DHCP_Status;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetDHCP_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_DHCP;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetAntenna_Type(U32 Antena_Type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Antenna_Type = Antena_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetAntenna_Type(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Antenna_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_SetLocal_Longitude(U32 Local_Longitude)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Local_Longitude = Local_Longitude;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetLocal_Longitude(void)
{
	U32 	Local_Longitude = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	Local_Longitude = UserSetting_Data.sCS_DBU_Local_Longitude;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Local_Longitude);
}

tCS_DB_Error  CS_DBU_SetLocal_Latitude(U32 Local_latitude)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Local_Latitude = Local_latitude;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetLocal_Latitude(void)
{
	U32 	Local_latitude = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	Local_latitude = UserSetting_Data.sCS_DBU_Local_Latitude;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Local_latitude);
}

tCS_DB_Error  CS_DBU_SetFont_Type(U32 Font_Type)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Font_Type = Font_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetFont_Type(void)
{
	U32 	Font_Type = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	Font_Type = UserSetting_Data.sCS_DBU_Font_Type;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Font_Type);
}

tCS_DB_Error  CS_DBU_SetFont_Size(U32 Font_Size)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Font_Size = Font_Size;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetFont_Size(void)
{
	U32 	Font_Size = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	Font_Size = UserSetting_Data.sCS_DBU_Font_Size;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	if ( Font_Size < FONT_SIZE_MIN )
		Font_Size = FONT_SIZE_MIN;
	else if ( Font_Size > FONT_SIZE_MAX )
		Font_Size = FONT_SIZE_MAX;

	return(Font_Size);
}

tCS_DB_Error  CS_DBU_SetFont_Kind(U32 Font_Kind)
{
	tCS_DB_Error  status = eCS_DB_OK;

	// printf("\n======== CS_DBU_SetFont_Kind : %d ==========\n", Font_Kind);
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Font_Kind = Font_Kind;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_GetFont_Kind(void)
{
	U32 	Font_Kind = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	Font_Kind = UserSetting_Data.sCS_DBU_Font_Kind;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(Font_Kind);
}

tCS_DB_Error  CS_DBU_SetBannerKeepTime(U32 banner_time)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(banner_time <= kCS_DBU_MAX_BannerKeepTime)
		UserSetting_Data.sCS_DBU_BannerKeepTime = banner_time;
	else
		status = eCS_DB_ERROR;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}



U32	CS_DBU_GetBannerKeepTime(void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_BannerKeepTime;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(UserSetting_Data.sCS_DBU_BannerKeepTime <= kCS_DBU_MAX_BannerKeepTime)
		returnvalue = UserSetting_Data.sCS_DBU_BannerKeepTime;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}


tCS_DB_Error CS_DBU_SetLCNMode(tCS_DB_LCNMode LCNmode)
{
        tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(LCNmode <= eCS_DB_Operator_Defined)
		UserSetting_Data.sCS_DBU_LCNMode = LCNmode;
	else
		status = eCS_DB_ERROR;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return eCS_DB_OK;
}



tCS_DB_LCNMode CS_DBU_GetLCNMode(void)
{
	tCS_DB_LCNMode mode;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	mode = UserSetting_Data.sCS_DBU_LCNMode;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(mode);
}

tCS_DB_Error  CS_DBU_SetVolume(U32 Volume)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(Volume <= kCS_DBU_MAX_VOLUME)
		User_Volume = Volume;
	else
		status = eCS_DB_ERROR;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32 CS_DBU_GetVolume(void)
{
	U32 	returnvalue = kCS_DBU_DEFAULT_Volume;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(User_Volume <= kCS_DBU_MAX_VOLUME)
		returnvalue = User_Volume;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_LoadVolume(void)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	/*从E2P中载入*/
	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_VOLUME_BASE, (U8 *)(&User_Volume), kCS_E2P_VOLUME_SIZE);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DB_Error CS_DBU_SaveVolume(void)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	/*将数据保存到E2P中*/
	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_VOLUME_BASE, (U8 *)(&User_Volume), kCS_E2P_VOLUME_SIZE);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DB_Error  CS_DBU_SetMuteStatus(tCS_DBU_Status MuteStatus)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);
	User_Mute = MuteStatus;
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DBU_Status	CS_DBU_GetMuteStatus(void)
{
	tCS_DBU_Status 	returnvalue = kCS_DBU_DEFAULT_MuteStatus;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	if(User_Mute <= eCS_DBU_ON)
		returnvalue = (tCS_DBU_Status)User_Mute;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_LoadMuteStatus(void)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	/*从E2P中载入*/
	E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_MUTE_STATUS_BASE, (U8 *)(&User_Mute), kCS_E2P_MUTE_STATUS_SIZE);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DB_Error CS_DBU_SaveMuteStatus(void)
{
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	/*将数据保存到E2P中*/
	E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_MUTE_STATUS_BASE, (U8 *)(&User_Mute), kCS_E2P_MUTE_STATUS_SIZE);

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

tCS_DB_Error  CS_DBU_SaveCurrentService(tCS_DBU_Service ServiceTriplet)
{
	CS_ErrorCode_t  error;
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	error = E2P_Write(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_CURRENT_SERVICE_BASE, (U8 *)(&ServiceTriplet), kCS_E2P_CURRENT_SERVICE_SIZE);
	//printf("==== %d , %d , %d , %d ====\n", ServiceTriplet.sCS_DBU_ServiceIndex , ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet.sCS_DBU_ServiceList.sCS_DB_ServiceListType );
	//printf("E2P_Write error = 0x%x\n", error);
	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	/* For Recall List problem By KB Kim 2011.08.30 */
	// MV_Reset_ReCall_List(ServiceTriplet);
	MV_Set_ReCall_List(ServiceTriplet);
	return(eCS_DB_OK);
}

tCS_DB_Error CS_DBU_LoadCurrentService(tCS_DBU_Service *ServiceTriplet)
{
	CS_ErrorCode_t  error;
	tCS_DBU_Service     service;
	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	error = E2P_Read(CS_SYS_GetHandle(CS_SYS_E2P_HANDLE), kCS_E2P_CURRENT_SERVICE_BASE, (U8 *)(&service), kCS_E2P_CURRENT_SERVICE_SIZE);
	//printf("E2P_Read error = 0x%x\n", error);
	//if((service.sCS_DBU_ServiceList.sCS_DB_ServiceListType > eCS_DB_FAV_TV_LIST) || (error!=CS_NO_ERROR))
	if(error!=CS_NO_ERROR)
	{
		service.sCS_DBU_ServiceList.sCS_DB_ServiceListType = eCS_DB_TV_LIST;
		service.sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue = 0;
		service.sCS_DBU_ServiceIndex = 0;
	}

	memcpy(ServiceTriplet, &service, sizeof(tCS_DBU_Service));

	// printf("==== %d , %d , %d , %d ====\n", ServiceTriplet->sCS_DBU_ServiceIndex , ServiceTriplet->sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet->sCS_DBU_ServiceList.sCS_DB_ServiceListTypeValue, ServiceTriplet->sCS_DBU_ServiceList.sCS_DB_ServiceListType );

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(eCS_DB_OK);
}

U32	CS_DBU_Get_Push_Game_Level(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Push_Game_Level;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Push_Game_Level(U32 Game_Level)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Push_Game_Level = Game_Level;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U32	CS_DBU_Get_Motor_Limit(void)
{
	U32 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Motor_Limit;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Motor_Limit(U32 u32OnOff)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Motor_Limit = u32OnOff;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U8	CS_DBU_Get_Skin(void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Skin;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Skin(U8 u8Skin_Kind)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Skin = u8Skin_Kind;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U8	CS_DBU_Get_Slepp(void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Sleep;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Sleep(U8 u8Sleep_Kind)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Sleep = u8Sleep_Kind;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U8	CS_DBU_Get_Use_SubTitle(void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Use_Subtitle;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Use_SubTitle(U8 u8Subtitle)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Use_Subtitle = u8Subtitle;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U8	CS_DBU_Get_Fixed_Font_Size(void)
{
	U8 	returnvalue = 0;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	returnvalue = UserSetting_Data.sCS_DBU_Fix_Fontsize;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);

	return(returnvalue);
}

tCS_DB_Error  CS_DBU_Set_Fixed_Font_Size(U8 u8FontSize)
{
	tCS_DB_Error  status = eCS_DB_OK;

	CSOS_WaitSemaphore(sem_DBU_E2P_Access);

	UserSetting_Data.sCS_DBU_Fix_Fontsize = u8FontSize;

	CSOS_SignalSemaphore(sem_DBU_E2P_Access);
	return(status);
}

U16 MV_Get_Find_List(U16 *Find_List, char *Find_String)
{
	int					i = 0, j = 0, k = 0;
	MV_stServiceInfo	Service_Data;
	char				first_name[MAX_SERVICE_NAME_LENGTH];
	char				second_name[MAX_SERVICE_NAME_LENGTH];

	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		memset( first_name, 0x00, MAX_SERVICE_NAME_LENGTH );
		memset( second_name, 0x00, MAX_SERVICE_NAME_LENGTH );

		MV_DB_GetServiceDataByIndex(&Service_Data, Current_ServiceList.Service_Data[i].Service_Index);

		for (k = 0 ; ( k < MAX_SERVICE_NAME_LENGTH ) && ( Service_Data.acServiceName[k] != '\0' ) ; k++)
		{
			first_name[k] = (char)tolower(Service_Data.acServiceName[k]);
		}
		for (k = 0 ; ( k < MAX_SERVICE_NAME_LENGTH ) && ( Find_String[k] != '\0' ) ; k++)
		{
			second_name[k] = (char)tolower(Find_String[k]);
		}

		//printf("=== %s : %s =====\n", first_name, second_name);
		if ( strstr(first_name, second_name) != NULL )
		{
			Find_List[j] = Current_ServiceList.Service_Data[i].Service_Index;
			j++;
		}
	}

	return j;
}

tCS_DB_Error MV_Get_Find_Current_Service_Order(U16 u16Find_Ch_Index, U16 *u16Return_service)
{
	U16					i = 0;
	tCS_DB_Error		ret = eCS_DB_OK;

	for ( i = 0 ; i < Current_ServiceList.Service_Number ; i++ )
	{
		if ( Current_ServiceList.Service_Data[i].Service_Index == u16Find_Ch_Index )
			break;
	}

	if ( i == Current_ServiceList.Service_Number )
	{
		// printf("MV_Get_Find_Current_Service_Order :: Error : %d => %d \n", Current_ServiceList.Service_Number, i );
		ret = eCS_DB_ERROR;
		*u16Return_service = i ;
	}
	else
	{
		// printf("MV_Get_Find_Current_Service_Order :: OK : %d => %d \n", Current_ServiceList.Service_Number, i );
		ret = eCS_DB_OK;
		*u16Return_service = i ;
	}

	return ret;
}

U16 MV_DB_GetServiceindex_ByOrderIndex(tCS_DB_ServiceListType tcs_Orderindex, U32 u32ListType, U16 u16Serviceindex)
{
	U16		u16OrginalIndex = 0;

	CSOS_WaitSemaphore(sem_ServiceListAccess);

	//printf("CS_DB_SetCurrentService_OrderIndex ====== %d : %d , %d ======\n", tcs_Orderindex, u32ListType, u16Serviceindex);

	switch(tcs_Orderindex)
	{
		case eCS_DB_TV_LIST :
			u16OrginalIndex = u16MV_stUseIndex[kCS_DB_DEFAULT_TV_LIST_ID][u16Serviceindex];
			break;
		case eCS_DB_RADIO_LIST :
		 	u16OrginalIndex = u16MV_stUseIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][u16Serviceindex];
			break;
		case eCS_DB_FAV_TV_LIST :
			u16OrginalIndex = stFavInfo_Glob[u32ListType].u16FavChIndex[kCS_DB_DEFAULT_TV_LIST_ID][u16Serviceindex];
			break;
		case eCS_DB_FAV_RADIO_LIST :
			u16OrginalIndex = stFavInfo_Glob[u32ListType].u16FavChIndex[kCS_DB_DEFAULT_RADIO_LIST_ID][u16Serviceindex];
			break;
		case eCS_DB_SAT_TV_LIST :
			u16OrginalIndex = u16Global_SatListIndex[u32ListType].stChIndex[u16Serviceindex];
			break;
		case eCS_DB_SAT_RADIO_LIST :
			u16OrginalIndex = u16Global_SatListIndex[u32ListType + MV_SAT_MAX].stChIndex[u16Serviceindex];
			break;
		case eCS_DB_INVALID_LIST :
			CSOS_SignalSemaphore(sem_ServiceListAccess);
			return(0xFFFF);
			break;

	}

	CSOS_SignalSemaphore(sem_ServiceListAccess);
	return u16OrginalIndex;
}

#ifdef NEW_INSTALL
/* By KB Kim 2011.01.18 */
void MV_DB_Reset_Temp_AddService(void)
{
	u16Temp_Search_Count = 0;
	if (stTemp_CHInfo != NULL)
	{
		// printf("MV_DB_Reset_Temp_AddService Free Temp Add\n");
		OsMemoryFree(stTemp_CHInfo);
		stTemp_CHInfo = NULL;
	}
}

/* By KB Kim 2011.01.18 */
U32 MV_DB_Temp_Init_AddService(void)
{
	U32 memSize;

	MV_DB_Reset_Temp_AddService();
	memSize = (U32)sizeof(MV_stServiceInfo) * TEMP_CH_MAX;
	stTemp_CHInfo = (MV_stServiceInfo *)OsMemoryAllocate(memSize);
	if (stTemp_CHInfo == NULL)
	{
		return 0;
	}
	// printf("MV_DB_Temp_Init_AddService Alloc Temp Add\n");
	memset(stTemp_CHInfo, 0x00, memSize);

	return memSize;
}

U16	MV_DB_Temp_AddService_Count(void)
{
	return u16Temp_Search_Count;
}

#ifdef SPECIAL_ADD_CH_TO_DB
void MV_Add_Ch_to_WebDB( U16 u16Ch_ServiceId, char *Ch_Name, U16 u16Ch_Index )
{
	MV_stSatInfo 		Temp_SatInfo;
	MV_stTPInfo 		Temp_TPInfo;
	char				Temp_Ch_Name[64];
	char				Temp_Sat_Name[64];
	char				system_command[512];
//	char 				tempSection [DHCP_FILE_MAX_COL + 2];
	int					count, name_count;
	FILE				*fp;
	int					System_Result = 1;

	//printf("==== NAME : %s\n", Ch_Name );
	memset(system_command, 0x00, 512);
	memset(Temp_Ch_Name, 0x00, 64);
	memset(Temp_Sat_Name, 0x00, 64);
	MV_DB_Get_SatData_By_Chindex(&Temp_SatInfo, u16Ch_Index);
	MV_DB_Get_TPdata_By_ChNum(&Temp_TPInfo, u16Ch_Index);

	for ( count = 0 , name_count = 0 ; count < (int)strlen(Ch_Name) ; count++ )
	{
		if ( Ch_Name[count] == ' ' )
		{
			Temp_Ch_Name[name_count++] = '%';
			Temp_Ch_Name[name_count++] = '2';
			Temp_Ch_Name[name_count++] = '0';
		} else if ( Ch_Name[count] == '"' )
		{
			Temp_Ch_Name[name_count++] = '%';
			Temp_Ch_Name[name_count++] = '2';
			Temp_Ch_Name[name_count++] = '0';
		} else {
			Temp_Ch_Name[name_count++] = Ch_Name[count];
		}
	}

	for ( count = 0 , name_count = 0 ; count < (int)strlen(Temp_SatInfo.acSatelliteName) ; count++ )
	{
		if ( Temp_SatInfo.acSatelliteName[count] == ' ' )
		{
			Temp_Sat_Name[name_count++] = '%';
			Temp_Sat_Name[name_count++] = '2';
			Temp_Sat_Name[name_count++] = '0';
		} else {
			Temp_Sat_Name[name_count++] = Temp_SatInfo.acSatelliteName[count];
		}
	}

	sprintf(system_command, "wget -q -O /tmp/chsaved.txt \"http://www.merihvideo.com.tr/chipbox/chansave.php?ch_servis=%d&ch_name=%s&sat_name=%s&sat_long=%d&tp_freq=%d&tp_sym=%d&tp_pol=%d\"",
															u16Ch_ServiceId,
															Temp_Ch_Name,
															Temp_Sat_Name,
															Temp_SatInfo.s16Longitude,
															Temp_TPInfo.u16TPFrequency,
															Temp_TPInfo.u16SymbolRate,
															Temp_TPInfo.u8Polar_H );
	count = 0;
	while( System_Result && count < 10 )
	{
		System_Result = system(system_command);

		if ( System_Result != 0 )
		{
			printf("\n%d : %s \n", count, system_command );
			printf("======= %s : %s ========\n", Ch_Name, Temp_Ch_Name);
		}

		count++;
	}

#if 1
	usleep(200*1000);
#else
	count = 0;
	while(count < 100)
	{
		usleep(100*1000);
		//printf("While Loop %d \n", count);
		if (!(fp = fopen("/tmp/chsaved.txt", "r")))
		{
			count++;
	        continue;
		}
/*
		memset(tempSection, 0x00, 16);
		fgets(tempSection, DHCP_FILE_MAX_COL, fp);

		if ( strstr( tempSection, "OK" ) != NULL )
		{
			fclose(fp);
			break;
		}
*/		else {
			fclose(fp);
			usleep(200*1000);
			system("rm /tmp/chsaved.txt");
			break;
		}
	}
#endif
	printf("%d : %03d : %s \n", count, strlen(system_command), system_command );
}
#endif

/* For First search channel By KB Kim 2011.01.19 */
void MV_DB_Add_Temp_Service(U16 *firstChIndex)
{
	U16		i = 0;
	U16		serviceIndex;
	U16     firstIndex;

	firstIndex = MV_DB_INVALID_SERVICE_INDEX;
	/* By KB Kim 2011.01.18 */
	if (stTemp_CHInfo == NULL)
	{
		return;
	}

	//printf("\n ==== %d Count Save ======\n", u16Temp_Search_Count);
	for ( i = 0 ; i < u16Temp_Search_Count ; i++ )
	{
		//printf("1 %d => %s\n", i, stTemp_CHInfo[i].acServiceName);
		if (MV_DB_AddOneService(stTemp_CHInfo[i], &serviceIndex) != eCS_DB_OK)
		{
			break;
		}

		if ((firstIndex == MV_DB_INVALID_SERVICE_INDEX) &&
			(stTemp_CHInfo[i].u8TvRadio == eCS_DB_TV_SERVICE || stTemp_CHInfo[i].u8TvRadio == eCS_DB_HDTV_SERVICE ))
		{
			firstIndex = serviceIndex;
		}

		//printf("2 %d => %s\n", i, stTemp_CHInfo[i].acServiceName);
#ifdef SPECIAL_ADD_CH_TO_DB
		MV_Add_Ch_to_WebDB( stTemp_CHInfo[i].u16ServiceId, stTemp_CHInfo[i].acServiceName, serviceIndex );
#endif
	}

	*firstChIndex = firstIndex;
	/* By KB Kim 2011.01.18 */
	MV_DB_Reset_Temp_AddService();
}

tCS_DB_Error MV_DB_Temp_AddOneService(MV_stServiceInfo pMvServiceData, U16 *pServiceIndex)
{
	U16			TotalService = 0;

	/* By KB Kim 2011.01.18 */
	if (stTemp_CHInfo == NULL)
	{
		return eCS_DB_ERROR;
	}

	if( MAX_TP_COUNT <= pMvServiceData.u16TransponderIndex || MV_MAX_SATELLITE_COUNT <= MV_DB_Get_SatIndex_By_TPindex(pMvServiceData.u16TransponderIndex) )
	{
		return eCS_DB_ERROR;
	}

	TotalService = pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_TV_LIST_ID] + pServiceListInfo->u16DB_List_ServiceNum[kCS_DB_DEFAULT_RADIO_LIST_ID];

	if(MV_MAX_SERVICE_COUNT <= TotalService)
	{
		CSOS_SignalSemaphore(sem_ServiceListAccess);
		return eCS_DB_ERROR;
	}

#if 0  /* No need this By KB Kim 2011.01.20 */
	if(pMvServiceData.u8TvRadio == eCS_DB_TV_SERVICE || pMvServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE )
	{
		iffound = FALSE;
		for(ServIdx = 0; ServIdx < MV_MAX_SERVICE_COUNT; ServIdx++)
		{
			if(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_INVALID)
			{
				iffound = TRUE;
				break;
			}
		}
	}
	else
	{
		iffound = FALSE;
		for(ServIdx = 0; ServIdx < MV_MAX_SERVICE_COUNT; ServIdx++)
		{
			if(stCHInfo_Glob[ServIdx].u8Valid == eCS_DB_INVALID)
			{
				iffound = TRUE;
				break;
			}
		}
	}

	if(iffound)
	{
		pMvServiceData.u16ChIndex = ServIdx;
		*pServiceIndex = ServIdx;
	}
#endif

	stTemp_CHInfo[u16Temp_Search_Count] = pMvServiceData;
	u16Temp_Search_Count++;
	pMvServiceData.u16ChIndex = TotalService;
	*pServiceIndex = TotalService;

	return eCS_DB_OK;
}
#endif  // #ifdef NEW_INSTALL

void MV_DB_Output_File_Test(void)
{
	/* By KB Kim 2011.01.20 */
	U16 			i;
	U16             totalChannel;
	U16             fileCount;
	MV_stSatInfo	stSatTemp;
	MV_stTPInfo		stTPTemp;
	char            fileName[128];
	FILE* 			fp = NULL;

	memset(fileName, 0x00, 128);
	totalChannel = MV_DB_GetALLServiceNumber();
	//system("rm /tmp/??.txt");
	if (totalChannel > 0)
	{
		for ( i = 0 ; i < totalChannel ; i++ )
		{
			/* By KB Kim 2011.01.20 */
			if ((i % 500) == 0)
			{
				fileCount = (i / 500) + 1;
				sprintf(fileName, "/tmp/%02d.txt", fileCount);
				//printf("MV_DB_Output_File_Test : fileName[%s]\n", fileName);
				if (fp != NULL)
				{
					fclose(fp);
					fp = NULL;
				}
				fp = fopen(fileName, "wt");
				if (fp == NULL)
	                        {
					return;
				}
			}
		MV_DB_Get_SatData_By_Chindex(&stSatTemp, stCHInfo_Glob[i].u16ChIndex);
		MV_DB_Get_TPdata_By_ChNum(&stTPTemp, stCHInfo_Glob[i].u16ChIndex);
		/*                   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    */
			fprintf(fp, "%s;%d;%d;%d;%d;%d;%d;%d;%d;%s;%d;%d;%d;%d;%d;%d;%d;%d;%d;\n",
		/*  1 */		stSatTemp.acSatelliteName,
		/*  2 */		stTPTemp.u16SymbolRate,
		/*  3 */		stTPTemp.u8Polar_H,
		/*  4 */		stTPTemp.u16TPFrequency,
		/*  5 */		stCHInfo_Glob[i].u16ChIndex,
		/*  6 */		stCHInfo_Glob[i].u16ServiceId,
		/*  7 */		stCHInfo_Glob[i].u16PCRPid,
		/*  8 */		stCHInfo_Glob[i].u16VideoPid,
		/*  9 */		stCHInfo_Glob[i].u16AudioPid,
		/* 10 */		stCHInfo_Glob[i].acServiceName,
		/* 11 */		stCHInfo_Glob[i].u8Lock,
		/* 12 */		stCHInfo_Glob[i].u8Scramble,
		/* 13 */		stCHInfo_Glob[i].u8Audio_ch,
		/* 14 */		stCHInfo_Glob[i].u8Audio_Mode,
		/* 15 */		stCHInfo_Glob[i].u8TvRadio,
		/* 16 */		stCHInfo_Glob[i].u8Audio_Type,
		/* 17 */		stCHInfo_Glob[i].u8VideoType,
		/* 18 */		stCHInfo_Glob[i].u8AudioVolume,
		/* 19 */		stCHInfo_Glob[i].u8AC3Flag
						);
		}

		if (fp != NULL)
		{
		fclose(fp);
			fp = NULL;
		}
	}
}

#ifdef SMART_PHONE
void Mv_Default_Menu_Item( MV_Menu_Item_t *MV_Menu_Item)
{
	memcpy(MV_Menu_Item, &Default_MV_Menu_Item, sizeof(MV_Menu_Item_t) * ( MENU_ITEM_MAX + 2 ));
}
#endif

