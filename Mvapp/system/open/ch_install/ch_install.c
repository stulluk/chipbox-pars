#include "linuxos.h"

#include "database.h"

/* 2010.09.04 By KB KIm For New SI */
#include "demux.h"
#include "tableApi.h"
#include "mvosapi.h"
/* For Blind Scan By KB Kim 2011.02.26 */
#include "dvbtuner.h"

#include "fe_mngr.h"
#include "db_builder.h"
#include "ch_install.h"
#include "av_zapping.h"
#include "userdefine.h"

//#define CH_INSTALL_DEBUG_ON
/* For NIT search  By KB Kim 2011.01.09 */
#define SI_MINIMUM_SAT_TP_FREQUENCY        3000             
#define SI_MINIMUM_QPSK_SYMBOL_RATE        1000             

static U8                       NitModeOn;
static U8                       FtaModeOn;
/* For Blind Scan By KB Kim 2011.02.26 */
static U8                       BlindModeOn;

/* For TP NIT Search by KB Kim 11 Jan 2011 */
static U8                       TpSearchModeOn;

static U16						Current_TP_Num = 0;
static U16						u16Current_TP = 0;
static U8						CS_INSTALL_FE_Client = 0;
static U16						NumberOfTPInstalled = 0;
static U8						u8Now_Scan_Sat = 0;
static U8						u8Scan_Sat_Total = 0;
static U16						u16Scan_TP_Total = 0;
static st_INSTALL_TPList		CS_INSTALL_TPList[512];
static tCS_INSTALL_State		CS_INSTALL_State = eCS_INSTALL_STATE_NONE;

static CSOS_Semaphore_t			*sem_InstallAccess = NULL;

static tMV_Display_ServiceInfo	Backup_Current_Service[500];

/* By KB Kim 2011.01.18 */
static U16  					FirstInstallService = MV_DB_INVALID_SERVICE_INDEX;

tCS_INSTALL_NotificationFunction	INSTALL_NotifyFunction[kCS_INSTALL_MAX_NO_OF_NOTIFY_CLIENTS];

BOOL CS_INSTALL_Read_TPdata(U8 *u8Sat_Index, U8 u8Sat_count, U8 u8SelectTP)
{
	U8	TP_Count = 0;
	U8	i = 0, j = 0;

	u8Now_Scan_Sat = 0;
	u16Scan_TP_Total = 0;
	u8Scan_Sat_Total = u8Sat_count;
	/* For Blind Scan By KB Kim 2011.02.26 */
	memset(CS_INSTALL_TPList, 0x00, sizeof(CS_INSTALL_TPList));

	if ( u8Sat_count != 0 )
	{
		/*
		if ( u8Sat_count == 1 )
			printf("################# Single-Sat Search Start #####################\n");
		else
			printf("################# Multi-Sat Search Start #####################\n");
			*/
		
		for ( j = 0 ; j < u8Sat_count ; j++ )
		{
			TP_Count = MV_DB_Get_TPCount_By_Satindex(u8Sat_Index[j]);

			if ( TP_Count > 0 )
			{
				for ( i = 0 ; i < TP_Count ; i++ )
				{
					CS_INSTALL_TPList[j].u16TP_Index[i] = MV_DB_Get_TPIndex_By_Satindex_and_TPnumber(u8Sat_Index[j], i);
					//printf("==================> sat : %d , TP Index : %d =========", u8Sat_Index[j], CS_INSTALL_TPList[j].u16TP_Index[i]);
				}
			}
			CS_INSTALL_TPList[j].TP_Num = TP_Count;
			
			u16Scan_TP_Total += TP_Count;
		}

		if ( u16Scan_TP_Total == 0 )
			return FALSE;
		
	}
	else 
	{
		u16Scan_TP_Total = 1;
		u8Scan_Sat_Total = 1;
		CS_INSTALL_TPList[0].TP_Num = 1;
		CS_INSTALL_TPList[0].u16TP_Index[0] = MV_DB_Get_TPIndex_By_Satindex_and_TPnumber(u8Sat_Index[0], u8SelectTP);
	}
		
	return TRUE;
}

void MV_SetTPData_ADD_Scan_TPData(MV_stTPInfo TPInfo)
{
	CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num] = TPInfo.u16TPIndex;
	CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num++;
	u16Scan_TP_Total++;

#if 0
	printf("=============================================\n");
	printf("u8Now_Scan_Sat ID : %d\n", u8Now_Scan_Sat);
	printf("u16Scan_TP_Total : %d\n", u16Scan_TP_Total);
	printf("Current_TP_Num : %d , Added TP Index : %d\n", Current_TP_Num, TPInfo.u16TPIndex);
	printf("=============================================\n");
#endif
}

/* For Blind Scan By KB Kim 2011.02.26 */
void MvAddBlindScanTpData(MV_stTPInfo TPInfo)
{
	CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num] = TPInfo.u16TPIndex;
	CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num++;
	u16Scan_TP_Total++;
}

/* For Blind Scan By KB Kim 2011.02.26 */
void MV_INSTALL_Init_Blind_TPdata(U8 u8Sat_count)
{
	u8Now_Scan_Sat = 0;
	u16Scan_TP_Total = 0;
	u8Scan_Sat_Total = u8Sat_count;
	memset(CS_INSTALL_TPList, 0x00, sizeof(CS_INSTALL_TPList));
}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MV_INSTALL_Add_Blind_TPData(U8 *u8Sat_Index, U8 u8Sat_count, U8 polH)
{
	TunerBlindTpData_t  tpData[256];
	MV_stTPInfo		    addTpData;
	U16                 totalTP;
	U16                 count;
	
	totalTP = GetBlindTpResult (polH, tpData);

	if (totalTP > 0)
	{
		u8Now_Scan_Sat = u8Sat_count;
		Current_TP_Num = CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num;
		printf("MV_INSTALL_Add_Blind_TPData : Sat[%d - %d] TP[%d / %d]\n",u8Sat_count, u8Sat_Index[u8Sat_count], Current_TP_Num, totalTP);
		
		for (count = 0; count < totalTP; count++)
		{
			addTpData.u8SatelliteIndex = u8Sat_Index[u8Sat_count];
			addTpData.u16TPFrequency   = (U16)(tpData[count].Frequency  & 0xFFFF);
			addTpData.u16SymbolRate    = (U16)(tpData[count].SymbolRate & 0xFFFF);
			addTpData.u8Polar_H        = polH;
			addTpData.u8Unused         = 0;
			addTpData.u8Valid          = DB_VALID;
			addTpData.u16TSID          = 0;
			addTpData.u16NID           = 0;
#ifdef FOR_USA	
			addTpData.u16OrgNID        = 0;
#endif /* #ifdef FOR_USA */
			if ((addTpData.u16TPFrequency >= SI_MINIMUM_SAT_TP_FREQUENCY) &&
				(addTpData.u16SymbolRate  >= SI_MINIMUM_QPSK_SYMBOL_RATE))
			{
				if (MV_UdateBlindTpData(&addTpData) == eCS_DB_OK)
				{
					MV_SetTPData_ADD_Scan_TPData(addTpData);
				}
			}
		}
	}

	return FALSE;
}

tMV_Display_ServiceInfo * MV_INSTALL_GetServiceList(void)
{
	return(Backup_Current_Service);
}

U16 CS_INSTALL_Get_Num_Of_TP_BySat(U8 *u8Search_Sat_Index, U16 u16num_installed)
{
	U16		i = 0, j = 0;
	U16		u16Num = 0;
	U16		u16Temp_Num = 0;

	u16Num = u16num_installed;
	
	while(u8Search_Sat_Index[i] != 0xFF )
	{		
		u16Temp_Num += MV_DB_Get_TPCount_By_Satindex(u8Search_Sat_Index[i]);
		if ( u16Num < u16Temp_Num )
		{
			if ( i == 0 )
				return u16Num;
			else
			{
				for ( j = 0 ; j < i ; j++ )
					u16Num -= MV_DB_Get_TPCount_By_Satindex(u8Search_Sat_Index[j]);
				
				return u16Num;
			}
		}
		else
			i++;
	}
	return (0xFF);
}

#if 0 /* For TP Data Sync By KB KIm 2011.03.03 */
BOOL CS_INSTALL_Get_TPdata(MV_stTPInfo 	*Temp_TPDatas, U8 u8Sat_Index, U8 u8Start)
{
	U8	TP_Count = 0;
	U8	i = 0;
	
	TP_Count = MV_DB_Get_TPCount_By_Satindex(u8Sat_Index);

	if ( TP_Count > 0 )
	{
		for ( i = 0 ; i < 5 ; i++ )
		{
			if ( ( i + u8Start ) > TP_Count )
				break;
			else
				MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&Temp_TPDatas[i], u8Sat_Index, ( i + u8Start ));
		}
	} else 
		return FALSE;
		
	return TRUE;
}
#else
BOOL CS_INSTALL_Get_TPdata(MV_stTPInfo 	*Temp_TPDatas)
{
	U16	tpCount = 0;
	U16 tpNumber;
	U16 tpIndex;
	U16	i = 0;
	
	tpCount = CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num;
	tpNumber = Current_TP_Num;

	if ( tpCount > 0 )
	{
		for ( i = 0 ; i < 5 ; i++ )
		{
			if (tpNumber >= tpCount)
			{
				return TRUE;
			}
			
			tpIndex = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[tpNumber];
			MV_GetTPData_By_TPIndex(&Temp_TPDatas[i], tpIndex);
			tpNumber++;
		}
	}
	else
	{
		return FALSE;
	}
		
	return TRUE;
}
#endif

BOOL CS_INSTALL_Register_Notify(U8 *ClientId, tCS_INSTALL_NotificationFunction NotifyFunction)
{
	U8		index = 0;
	BOOL	found = FALSE;
	
 	if((ClientId != NULL)&&(NotifyFunction != NULL))
	{
		CSOS_WaitSemaphore(sem_InstallAccess);
		
		for(index = 0; index< kCS_INSTALL_MAX_NO_OF_NOTIFY_CLIENTS; index++)
		{
			if(INSTALL_NotifyFunction[index] == NULL)
			{
				found = TRUE;
				INSTALL_NotifyFunction[index] = NotifyFunction;
				*ClientId = index;
				break;
			}
		}

		CSOS_SignalSemaphore(sem_InstallAccess);
	}
	
	return(found);

}

BOOL CS_INSTALL_Unregister_Notify(U8 ClientId)
{
	BOOL NoError = TRUE;
	
	if(ClientId < kCS_INSTALL_MAX_NO_OF_NOTIFY_CLIENTS)
	{
		CSOS_WaitSemaphore(sem_InstallAccess);

		if(INSTALL_NotifyFunction[ClientId] != NULL)
		{
			INSTALL_NotifyFunction[ClientId] = NULL;
		}

		CSOS_SignalSemaphore(sem_InstallAccess);
	}
	else
	{
		NoError = FALSE;
	}
	
	return(NoError);

}

void INSTALL_Notify(tCS_INSTALL_Notification notification)
{
	U8	notify_index = 0;
	
	for(notify_index = 0; notify_index< kCS_SI_MAX_NO_OF_NOTIFY_CLIENTS; notify_index++)
	{
		if(INSTALL_NotifyFunction[notify_index] != NULL) 
		{
			(INSTALL_NotifyFunction[notify_index])(notification);
		}

	}

	return;
}

/* For handling error to get next TP by KB Kim 2011.01.13 */
BOOL GetInstallNextTune(MV_stTPInfo *dbData)
{
	if (dbData == NULL)
	{
		return TRUE;
	}
	
	while(u8Now_Scan_Sat < u8Scan_Sat_Total )
	{
		if((CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num > 0) && (Current_TP_Num < CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num))
		{
			u16Current_TP = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
			if ( MV_DB_GetTPDataByIndex(dbData, u16Current_TP) == eCS_DB_OK )
			{
				return FALSE;
			}

			Current_TP_Num++;
		}
		else
		{
			u8Now_Scan_Sat++;
			Current_TP_Num = 0;
		}
	}

	return TRUE;
}

void INSTALL_NextTune(void)
{
	MV_ScanParams 			FE_ScanData;
	MV_stTPInfo				DB_TPData;

	// printf("\n########  START INSTALL_NextTune() ##############\n");

	CS_INSTALL_State = eCS_INSTALL_STATE_NONE;
	CS_FE_StopScan();

	/* By KB Kim 2011.01.15 */
	// INSTALL_Notify(eCS_INSTALL_TPINFO);

	if(MV_DB_GetALLServiceNumber() < MV_MAX_SERVICE_COUNT)
	{
		// printf("========= Current_TP_Num : %d , CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num : %d ===== \n", Current_TP_Num, CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num);
		
/* For handling error to get next TP by KB Kim 2011.01.13 */
#if 0		
		if((CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num == 0) || (Current_TP_Num >= CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num))
		{
			u8Now_Scan_Sat++;
			Current_TP_Num = 0;
			
			if ( u8Now_Scan_Sat >= u8Scan_Sat_Total )
			{
				INSTALL_Notify(eCS_INSTALL_COMPLETE);
				return;
			}
		} 
		
		u16Current_TP = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
		//printf("=== %d / %d ==>> %d : %d / %d =====\n", u8Now_Scan_Sat, u8Scan_Sat_Total, Current_TP_Num, u16Current_TP, CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num);

		if ( MV_DB_GetTPDataByIndex(&DB_TPData, u16Current_TP) == eCS_DB_ERROR )
		{
			/* Error in TP list stop here */
			INSTALL_Notify(eCS_INSTALL_COMPLETE);
			return;
		}
#else
		if (GetInstallNextTune(&DB_TPData))
		{
			/* Search finished */
			// printf("GetInstallNextTune : End\n");
			INSTALL_Notify(eCS_INSTALL_COMPLETE);
			return;
		}
#endif		
		/* By KB Kim 2011.01.15 */
		INSTALL_Notify(eCS_INSTALL_TPINFO);

		FE_ScanData.u16Tpnumber		= u16Current_TP;
		FE_ScanData.u16SymbolRate	= DB_TPData.u16SymbolRate;
		FE_ScanData.u16TPFrequency	= DB_TPData.u16TPFrequency;
		FE_ScanData.u8Polar_H		= DB_TPData.u8Polar_H;

		//printf("=== %d - %d th : %d , %d ==\n", Current_TP_Num, CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num], FE_ScanData.u16TPFrequency, FE_ScanData.u16SymbolRate);

		CS_INSTALL_State = eCS_INSTALL_STATE_SCANNING;

		DB_DemuxResetChannel();

		CS_FE_StartScan(FE_ScanData, 0);

	}
	else
	{
		// printf("\n\n=============== OVER MAX SERVICE ================\n\n");
		INSTALL_Notify(eCS_INSTALL_MAX_SERVICE_NUMBER_REACHED);
	}

	return;

}

/* For NIT search  By KB Kim 2011.01.09 */
void UpdateTpData(SiNitTpData_t *tpData, U8 satIndex, S16 orbit)
{
	S16				streamOrbit;
	MV_stTPInfo		addTpData;
	static int		count_num = 0;
	
#ifdef CH_INSTALL_DEBUG_ON
	printf("========>  TP for TS_ID[0x%04X] , ON_ID[0x%04X]\n",
		tpData->TsId, tpData->OnId);
#endif
	switch(tpData->TpType)
	{
	case SI_TP_SATELLITE  :
		streamOrbit = (S16)tpData->SatTpData.Orbit;
		if ((tpData->SatTpData.EastNotWest == 0) && (streamOrbit < 1800))
		{
			streamOrbit = 3600 - streamOrbit;
		}

		addTpData.u8SatelliteIndex = satIndex;
		addTpData.u16TPFrequency   = tpData->SatTpData.Frequency;
		addTpData.u16SymbolRate    = tpData->SatTpData.SymbolRate;
		addTpData.u8Polar_H        = 1 - tpData->SatTpData.Polarization;
		addTpData.u8Unused         = 0;
		addTpData.u8Valid          = DB_VALID;
		addTpData.u16TSID          = tpData->TsId;
		addTpData.u16NID           = tpData->OnId;
#ifdef FOR_USA	
		addTpData.u16OrgNID        = tpData->OnId;
#endif /* #ifdef FOR_USA */

//		printf("COUNT %d >>  %d / %d / %d ==========\n", count_num++, tpData->SatTpData.Frequency, tpData->SatTpData.Polarization, tpData->SatTpData.SymbolRate);
		if ((tpData->SatTpData.Frequency >= SI_MINIMUM_SAT_TP_FREQUENCY) &&
			(tpData->SatTpData.SymbolRate >= SI_MINIMUM_QPSK_SYMBOL_RATE) &&
			(orbit == streamOrbit) && compare_add_tp (&addTpData))
		{
			addTpData.u16TPIndex = CS_DB_Add_TP();
			MV_ADD_TPData(addTpData);
			// MV_SetTPData_ADD_TPIndex(addTpData);
			MV_SetTPData_ADD_Scan_TPData(addTpData);
#ifdef CH_INSTALL_DEBUG_ON
			printf ("  - Frequency(Add): %d MHz\n", tpData->SatTpData.Frequency);
			printf ("  - Symbolrate    : %d KHz\n", tpData->SatTpData.SymbolRate);
			printf ("  - Polirization  : %d\n", (U8)tpData->SatTpData.Polarization);
			printf ("  - Orbit         : %d %04d %04d\n", (U8)tpData->SatTpData.EastNotWest, tpData->SatTpData.Orbit, streamOrbit);
#endif
		}
#ifdef CH_INSTALL_DEBUG_ON
		else
		{
			printf ("  - Frequency     : %d MHz\n", tpData->SatTpData.Frequency);
			printf ("  - Symbolrate    : %d KHz\n", tpData->SatTpData.SymbolRate);
			printf ("  - Polirization  : %d\n", (U8)tpData->SatTpData.Polarization);
			printf ("  - Orbit         : %d %04d %04d\n", (U8)tpData->SatTpData.EastNotWest, tpData->SatTpData.Orbit, streamOrbit);
		}
#endif
		
		break;
	case SI_TP_CABLE      :
		OsDebugPrintf("  - Frequency     : %d MHz\n", tpData->CableTpData.Frequency);
		OsDebugPrintf("  - Symbolrate    : %d KHz\n", tpData->CableTpData.SymbolRate);
		if (tpData->CableTpData.Modulation < 6)
		{
			OsDebugPrintf("  - Modulation  : %d\n", (U8)tpData->CableTpData.Modulation);
		}
		else
	{
			OsDebugPrintf("  - Modulation  : Reserved\n");
	}
		break;
	case SI_TP_TERRESTIAL :
		OsDebugPrintf("  - Frequency     : %d MHz\n", tpData->TerrTpData.Frequency);
		OsDebugPrintf("  - Band Width    : %d MHz\n", (8 - tpData->TerrTpData.BandWidth));
		if (tpData->TerrTpData.Constellation < 3)
	{
			OsDebugPrintf("  - Constellation : %d\n", (U8)tpData->TerrTpData.Constellation);
		}
		else
			{
			OsDebugPrintf("  - Constellation : Reserved\n");
		}
		if (tpData->TerrTpData.TxMode < 3)
				{
			OsDebugPrintf("  - Tx Mode       : %d\n", (U8)tpData->TerrTpData.TxMode);
		}
		else
					{
			OsDebugPrintf("  - Tx Mode       : Reserved\n");
		}
		break;
	default :
		break;
	}

}
                                    
void InstallNitCallBack(SiNetworkData_t *nitData)
{
	U16            tpCount;
	U16            tpIndex;
	SiNitTpData_t *tpData;
	MV_stTPInfo    tPInfo;
	MV_stSatInfo   satInfo;

	if (nitData != NULL)
	{
		if (nitData->NumberOfTp == 0)
		{
#ifdef CH_INSTALL_DEBUG_ON
			printf ("---------> InstallNitCallBack : NIT but no TP Data\n");
#endif
			return;
		}
		
		/* For TP NIT Search by KB Kim 11 Jan 2011 */
		TpSearchModeOn = 0; /* Need to clear after get NIT data for TP search mode */
		
		tpIndex = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
		if (MV_DB_GetTPDataByIndex(&tPInfo, tpIndex) == eCS_DB_OK)
		{
			MV_GetSatelliteData_ByIndex(&satInfo, tPInfo.u8SatelliteIndex);
			if (satInfo.u8SatelliteIndex == MV_SAT_MAX)
			{
#ifdef CH_INSTALL_DEBUG_ON
				printf ("---------> InstallNitCallBack : Cannot find current Sat data\n");
#endif

				return;
			}
		}
		else
		{
#ifdef CH_INSTALL_DEBUG_ON
			printf ("---------> InstallNitCallBack : Cannot find current TP data\n");
#endif
			return;
		}
		
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> InstallNitCallBack : %d TP Found from [%d/%d 0x%04X : %s]\n", nitData->NumberOfTp, tPInfo.u16TPFrequency, tPInfo.u16SymbolRate, nitData->NetworkId, nitData->NetworkName);
#endif
		tpData = nitData->NitTpData;
		tpCount = 0;

		while((nitData != NULL) && (tpCount < nitData->NumberOfTp))
		{
			UpdateTpData(tpData, tPInfo.u8SatelliteIndex, satInfo.s16Longitude);
			tpData = tpData->Next_p;
			tpCount++;
		}                                     
	}
	else
	{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> InstallNitCallBack : No NIt Data Found\n");
#endif
	}
}

U8 GetProgramData(SiProgramData_t *programData, MV_stServiceInfo *dBServiceData)
			{
	U8                esCount;
	U8                videoFound;
	U8                audioFound;
	U8                validProgram;
	U32               nameLength;
                            
	validProgram = 0;
            	        
	if ((programData == NULL) || (dBServiceData == NULL))
					{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> GetProgramData Error : No program Data or dbbuffer\n");
#endif
		return validProgram;
	}
            	        
	if (programData->NumberOfEs == 0)
						{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> GetProgramData Error : No ES !!\n");
#endif
		return validProgram;
										}
                                                
	if (FtaModeOn && programData->ChannelScramble)
	{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> GetProgramData Error : FTA only but Scrambled channel!!\n");
#endif
		return validProgram;
	}

	esCount    = 0;
	videoFound = 0;
	audioFound = 0;
	
	while (((!videoFound) || (!audioFound)) &&  (esCount < programData->NumberOfEs))
	{
		if (programData->EsData[esCount].EsType == SI_ES_VIDEO)
		{
			if (!videoFound)
			{
				videoFound = 1;
				validProgram = 1;
				dBServiceData->u16VideoPid		= programData->EsData[esCount].EsPid;
				dBServiceData->u8VideoType		= programData->EsData[esCount].EsSubData.EsSubType;
				dBServiceData->u16EitHeaderIdx	= dBServiceData->u8VideoType;
			}
		}
		else if (programData->EsData[esCount].EsType == SI_ES_AUDIO)
		{
			if (!audioFound)
			{
				audioFound = 1;
				validProgram = 1;
				dBServiceData->u16AudioPid		= programData->EsData[esCount].EsPid;
				dBServiceData->u8Audio_Type		= programData->EsData[esCount].EsSubData.EsSubType;
			}
		}
		esCount++;
	}
	
	if (validProgram)
	{
		if (videoFound)
		{
			if (programData->ServiceType == HDTV_SERVICE)
			{
#ifdef CH_INSTALL_DEBUG_ON
				printf ("---------> GetProgramData : HD ");
#endif
				dBServiceData->u8TvRadio = (U8)HDTV_SERVICE;
			}
			else
			{
#ifdef CH_INSTALL_DEBUG_ON
				printf ("---------> GetProgramData : TV ");
#endif
				dBServiceData->u8TvRadio = (U8)TV_SERVICE;
			}
		}
		else if (audioFound)
		{
#ifdef CH_INSTALL_DEBUG_ON
			printf ("---------> GetProgramData : RADIO ");
#endif
			dBServiceData->u8TvRadio = (U8)RADIO_SERVICE;
			dBServiceData->u16VideoPid = 0x1FFF;
		}

		memset(dBServiceData->acServiceName, 0x00, MAX_SERVICE_NAME_LENGTH);
		if (programData->ChannelName[0] == 0x00)
		{
			sprintf(dBServiceData->acServiceName, "Channel %d", programData->ServideId);
		}
		else
		{
			nameLength = (U32)strlen(programData->ChannelName);
			if (nameLength >= MAX_SERVICE_NAME_LENGTH)
			{
				nameLength = MAX_SERVICE_NAME_LENGTH - 1;
			}
			memcpy(dBServiceData->acServiceName, programData->ChannelName, nameLength);
		}
		dBServiceData->u16TransponderIndex = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
		dBServiceData->u16ServiceId		   = programData->ServideId;
		dBServiceData->u16PMTPid		   = programData->PmtPid;
		dBServiceData->u16PCRPid		   = programData->PcrPid;
		dBServiceData->u8Scramble		   = programData->ChannelScramble;
		dBServiceData->u8Lock			   = 0; //UnLock
		dBServiceData->u8Watch			   = 1;
		dBServiceData->u8AC3Flag		   = 0;
		dBServiceData->u8AudioVolume	   = 32;
		dBServiceData->u8Audio_ch		   = 0;
		dBServiceData->u8Audio_Mode		   = 0;
		dBServiceData->u8Erase			   = 0;
		dBServiceData->u8Favor			   = 0;
		dBServiceData->u8Valid			   = 0;
#ifdef FOR_USA
		dBServiceData->u16ONID			   = 0;
		dBServiceData->u16LCN			   = programData->LCN;
#endif
#ifdef CH_INSTALL_DEBUG_ON
		printf ("[0x%04X : %s] Program Found\n", dBServiceData->u16ServiceId, dBServiceData->acServiceName);
#endif
	}

	return validProgram;
}

void InstallSiTableCallBack(SiService_t *serviceData)
{
	SiProgramData_t  *programData;
	MV_stServiceInfo  dBServiceData;
	MV_stTPInfo       tPInfo;
	U16               serviceIndex;
	U16               tpIndex;
	U16               updateCount;
	U8                nameLength;
	U8                programCount;

	if(CS_INSTALL_State != eCS_INSTALL_STATE_SCANNING)
	{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> InstallSiTableCallBack Error : Not a Search Mode\n");
#endif
		return;
	}

	updateCount = 0;

	tpIndex = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
	
	if ((serviceData != NULL) && (MV_DB_GetTPDataByIndex(&tPInfo, tpIndex) == eCS_DB_OK))
	{
		memset( &Backup_Current_Service, 0x00, sizeof(tMV_Display_ServiceInfo)*500 );
		tPInfo.u16TSID   = serviceData->TsId;
#ifdef FOR_USA	
		tPInfo.u16OrgNID = serviceData->ONId;
#endif	 
		tPInfo.u16NID    = serviceData->ONId;
		nameLength = MAX_SERVICE_NAME_LENGTH - 1;
		memcpy(tPInfo.acTPName, serviceData->ServiceName, nameLength);
		tPInfo.acTPName[nameLength] = 0x00;
		
		if (serviceData->NumberOfProgram > 0)
		{
			programCount = 0;
			updateCount = 0;
			programData = serviceData->ProgramData;

			while ((programData != NULL) && (programCount < serviceData->NumberOfProgram))
			{
				if (GetProgramData(programData, &dBServiceData))
				{
#ifdef NEW_INSTALL
					if (MV_DB_Temp_AddOneService(dBServiceData, &serviceIndex) == eCS_DB_OK)
					{
						strcpy(Backup_Current_Service[updateCount].acServiceNames ,dBServiceData.acServiceName);
						Backup_Current_Service[updateCount].u8ServiceType     = dBServiceData.u8TvRadio;
						Backup_Current_Service[updateCount].u8ServiceScramble = dBServiceData.u8Scramble;
						updateCount++;

						if( (FirstInstallService == MV_DB_INVALID_SERVICE_INDEX) && ( dBServiceData.u8TvRadio == eCS_DB_TV_SERVICE || dBServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE ) )
							FirstInstallService = serviceIndex;
					}
#else
					if (MV_DB_AddOneService(dBServiceData, &serviceIndex) == eCS_DB_OK)
					{
						strcpy(Backup_Current_Service[updateCount].acServiceNames ,dBServiceData.acServiceName);
						Backup_Current_Service[updateCount].u8ServiceType     = dBServiceData.u8TvRadio;
						Backup_Current_Service[updateCount].u8ServiceScramble = dBServiceData.u8Scramble;
						updateCount++;

						if( (FirstInstallService == MV_DB_INVALID_SERVICE_INDEX) && ( dBServiceData.u8TvRadio == eCS_DB_TV_SERVICE || dBServiceData.u8TvRadio == eCS_DB_HDTV_SERVICE ) )
							FirstInstallService = serviceIndex;
					}

#endif
#ifdef CH_INSTALL_DEBUG_ON
					else
					{
						printf ("---------> InstallSiTableCallBack : Same Channel!!\n");
					}
#endif
				}
				
				programData = programData->Next_p;
				programCount++;
			}
		}
	}

	if( updateCount > 0 )
	{
		INSTALL_Notify(eCS_INSTALL_SERVICEINFO);
	}

	Current_TP_Num++;

	if ( u8Now_Scan_Sat == u8Scan_Sat_Total )
	{
		if (Current_TP_Num < CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num)
		{
			INSTALL_NextTune();
		}
		else
		{
			INSTALL_Notify(eCS_INSTALL_COMPLETE);
		}
	}
	else
	{
		INSTALL_NextTune();
	}
}

void InstallClearSearchMode (void)
{
	NitModeOn   = 0; /* Default : NIT Mode Off */
	FtaModeOn   = 0; /* Default : FTA Mode Off */
	/* For Blind Scan By KB Kim 2011.02.26 */
	BlindModeOn = 0;

	/* For TP NIT Search by KB Kim 11 Jan 2011 */
	TpSearchModeOn = 0;
}

BOOL CS_INSTALL_Init(void)
{
	sem_InstallAccess  = CSOS_CreateSemaphoreFifo ( NULL, 1 );

	InstallClearSearchMode();
	SiRegisterNitCallBack(InstallNitCallBack);
	SiRegisterSearchCallBack(InstallSiTableCallBack);

	return(TRUE);
}

void INSTALL_TPLocked (void)
{
	if(CS_INSTALL_State == eCS_INSTALL_STATE_SCANNING)
	{
		/* For TP NIT Search by KB Kim 11 Jan 2011 */
		SiStartSearchChannel(0/* tuner : 0 */, NitModeOn, TpSearchModeOn, 0 /* channel Mode (Main, Pip, Rec.. */);
	}
}

void INSTALL_TPUnlocked(void)
{
	if(CS_INSTALL_State == eCS_INSTALL_STATE_SCANNING)
	{
		SiStopSearchChannel();
		Current_TP_Num++;
		INSTALL_NextTune();
	}
}

/* For Blind Scan By KB Kim 2011.02.26 */
void INSTALL_BlindCallback(tCS_FE_Notification Notification)
{
	switch (Notification)
	{
		case eCS_FE_LOCKED:
			printf("INSTALL_BlindCallback : eCS_FE_LOCKED ======================== \n");
			BroadcastMessage( MSG_INSTALL_BLIND_UPDATED, eCS_FE_LOCKED, 0);
			break;

		case eCS_FE_UNLOCKED:
		case eCS_FE_SIGNAL_LOST:
			//printf("=========== eCS_FE_UNLOCKED =============\n");
			BroadcastMessage( MSG_INSTALL_BLIND_UPDATED, eCS_FE_UNLOCKED, 0);
			break;

		default:
			break;
	}

	return;
}

void  INSTALL_FECallback(tCS_FE_Notification Notification)
{
	
	tCS_FE_TerTunerInfo		FETunerInfo;

	if(CS_INSTALL_State != eCS_INSTALL_STATE_SCANNING)
	{
		//printf("INSTALL_FECallback bot Not eCS_INSTALL_STATE_SCANNING\n");
		return;
	}
		
	CS_FE_GetInfo(&FETunerInfo);

	switch (Notification)
	{
		case eCS_FE_LOCKED:
			//printf("eCS_FE_LOCKED ======================== \n");
			BroadcastMessage( MSG_SCANINFO_UPDATE, eCS_FE_LOCKED, 0);
			INSTALL_TPLocked ();
			//INSTALL_TPUnlocked();
			break;

		case eCS_FE_UNLOCKED:
		case eCS_FE_SIGNAL_LOST:
			//printf("=========== eCS_FE_UNLOCKED =============\n");
			BroadcastMessage( MSG_SCANINFO_UPDATE, eCS_FE_UNLOCKED, 0);
			/* For Blind Scan By KB Kim 2011.02.26 */
			// INSTALL_TPUnlocked();
			break;

		default:
			break;
	}

	return;
}

void CS_INSTALL_SetSearchMode(U8 mode)
{
	/* For Blind Scan By KB Kim 2011.02.26 */
	BlindModeOn = (mode & 0x04) >> 2;
	if (BlindModeOn)
	{
		FtaModeOn      = mode & 0x01;
		FtaModeOn      = 0;
		TpSearchModeOn = 0;
	}
	else
	{
		NitModeOn      = mode & 0x01;
		FtaModeOn      = (mode & 0x02) >> 1;
		/* For TP NIT Search by KB Kim 11 Jan 2011 */
		TpSearchModeOn = (mode & 0x80) >> 7;
	}
}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvGetLnbMode(U8 *u8Sat_Index, U8 u8Sat_count, U8 *blinfLnbUniversal)
{
	MV_stSatInfo satInfo;
	U8           count;

	if (u8Sat_count > 0)
	{
		CSOS_WaitSemaphore(sem_InstallAccess);
		
		for (count = 0; count < u8Sat_count; count++)
		{
			MV_GetSatelliteData_ByIndex(&satInfo, u8Sat_Index[count]);
			if (satInfo.u8SatelliteIndex < MV_SAT_MAX)
			{
				if ((satInfo.u8LNBType < EN_LNB_TYPE_UNIVERSAL_9750_10600) ||
					(satInfo.u8LNBType > EN_LNB_TYPE_UNIVERSAL_9750_10750))
				{
					/* Normal LNB */
					blinfLnbUniversal[count] = 0;
				}
				else
				{
					/* Universal LNB */
					blinfLnbUniversal[count] = 1;
				}
#ifdef CH_INSTALL_DEBUG_ON
				printf ("---------> MvGetLnbMOde : LNB Mode [%d] %s - %d\n", count, satInfo.acSatelliteName, blinfLnbUniversal[count]);
#endif
			}
		}
		
		CSOS_SignalSemaphore(sem_InstallAccess);
		
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/* For Blind Scan By KB Kim 2011.02.26 */
void MvInstallSetBlind(U8 mode)
{
	CS_INSTALL_SetSearchMode(mode);
	Current_TP_Num      = 0;
	NumberOfTPInstalled = 0;
#ifdef NEW_INSTALL
	MV_DB_Temp_Init_AddService();
#endif

}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvInstallStartBlind(MV_BlindScanParams blindParam)
{
	MV_stSatInfo satInfo;
	
	CSOS_WaitSemaphore(sem_InstallAccess);
	
	CS_INSTALL_State = eCS_INSTALL_STATE_NONE;
	MV_GetSatelliteData_ByIndex(&satInfo, blindParam.SatIndex);
	if (satInfo.u8SatelliteIndex == MV_SAT_MAX)
	{
#ifdef CH_INSTALL_DEBUG_ON
		printf ("---------> InstallNitCallBack : Cannot find current Sat data\n");
#endif

		CSOS_SignalSemaphore(sem_InstallAccess);
		return TRUE;
	}

	if (blindParam.LnbHi)
	{
		if ((satInfo.u8LNBType < EN_LNB_TYPE_UNIVERSAL_9750_10600) ||
			(satInfo.u8LNBType > EN_LNB_TYPE_UNIVERSAL_9750_10750))
		{
			/* LNB is not Universal */
			CSOS_SignalSemaphore(sem_InstallAccess);
			return TRUE;
		}
	}
	
	CS_FE_StopSearch();
	CS_FE_Register_Tuner_Notify(&CS_INSTALL_FE_Client, INSTALL_BlindCallback);
	CS_INSTALL_State = eCS_INSTALL_STATE_BLIND_SCANNING;
	CSOS_SignalSemaphore(sem_InstallAccess);
	
	if (CS_FE_StartBlindScan (blindParam) != eCS_FE_NO_ERROR)
	{
		return TRUE;
	}

	return FALSE;

}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvInstallStopBlind(void)
{
	CSOS_WaitSemaphore(sem_InstallAccess);

	CS_DB_Save_SAT_Database();

	CS_INSTALL_State = eCS_INSTALL_STATE_NONE;
	InstallClearSearchMode();

	/* By KB Kim 2011.01.21 */
	CS_FE_StopSearch();
	CS_FE_Unregister_Tuner_Notify(CS_INSTALL_FE_Client);

	CSOS_SignalSemaphore(sem_InstallAccess);
	return(TRUE);
}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvBlindStartInstallation(void)
{
	CSOS_WaitSemaphore(sem_InstallAccess);

	/* By KB Kim 2011.01.18 */
	FirstInstallService = MV_DB_INVALID_SERVICE_INDEX;
	// NumberOfTPInstalled =0; /* ??? */
	CS_INSTALL_State = eCS_INSTALL_STATE_SCANNING;

	/* By KB Kim 2011.01.13 */
	CS_FE_StopSearch();
	CS_FE_Register_Tuner_Notify(&CS_INSTALL_FE_Client, INSTALL_FECallback);

	CSOS_SignalSemaphore(sem_InstallAccess);
	
	INSTALL_NextTune();

	return(TRUE);
}

/* For Blind Scan By KB Kim 2011.02.26 */
BOOL MvBlindStopInstallation(void)
{
	CSOS_WaitSemaphore(sem_InstallAccess);

	CS_INSTALL_State = eCS_INSTALL_STATE_NONE;

	SiStopSearchChannel();
	InstallClearSearchMode();

	NumberOfTPInstalled = 0;
	/* By KB Kim 2011.01.21 */
	CS_FE_StopSearch();
	CS_FE_Unregister_Tuner_Notify(CS_INSTALL_FE_Client);

	CSOS_SignalSemaphore(sem_InstallAccess);
	return(TRUE);
}

BOOL CS_INSTALL_StartInstallation(U8 mode)
{
	CSOS_WaitSemaphore(sem_InstallAccess);

	/* By KB Kim 2011.01.18 */
	FirstInstallService = MV_DB_INVALID_SERVICE_INDEX;
	NumberOfTPInstalled =0;
	CS_INSTALL_State = eCS_INSTALL_STATE_SCANNING;

	CS_INSTALL_SetSearchMode(mode);

	/* By KB Kim 2011.01.13 */
	CS_FE_StopSearch();
	CS_FE_Register_Tuner_Notify(&CS_INSTALL_FE_Client, INSTALL_FECallback);

	Current_TP_Num = 0;

	/*  First Sat can have no TP for multi-Sat search. Need to Handle by INSTALL_NextTune
	if(Current_TP_Num >= CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num)
	{
		CSOS_SignalSemaphore(sem_InstallAccess);
		return(FALSE);
	}
	*/

	CSOS_SignalSemaphore(sem_InstallAccess);
	
#ifdef NEW_INSTALL
	MV_DB_Temp_Init_AddService();
#endif

	INSTALL_NextTune();

	return(TRUE);
}

BOOL CS_INSTALL_StopInstallation(void)
{
	CSOS_WaitSemaphore(sem_InstallAccess);

	if (NitModeOn)
	{
		CS_DB_Save_SAT_Database();
	}

	CS_INSTALL_State = eCS_INSTALL_STATE_NONE;

	SiStopSearchChannel();
	InstallClearSearchMode();

	u8Now_Scan_Sat = u8Scan_Sat_Total;
	Current_TP_Num = CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num + 1;

	NumberOfTPInstalled = 0;
	/* By KB Kim 2011.01.21 */
	CS_FE_StopSearch();
	CS_FE_Unregister_Tuner_Notify(CS_INSTALL_FE_Client);

	CSOS_SignalSemaphore(sem_InstallAccess);
	return(TRUE);
}

U16  CS_INSTALL_GetTPListNum(void)
{
	U16	num;
	CSOS_WaitSemaphore(sem_InstallAccess);
	//num = CS_INSTALL_TPList[u8Now_Scan_Sat].TP_Num;
	num = u16Scan_TP_Total;
	CSOS_SignalSemaphore(sem_InstallAccess);
	return(num);
}

U16  CS_INSTALL_GetNumOfTPInstalled(void)
{
	U16	num = 0;
	U8	i;
	
	CSOS_WaitSemaphore(sem_InstallAccess);

	if ( u8Now_Scan_Sat > 0 )
	{
		for( i = 0 ; i < u8Now_Scan_Sat ; i++ )
		{
			num += CS_INSTALL_TPList[i].TP_Num;
		}
		num += Current_TP_Num;
	}	
	else
		num = Current_TP_Num;
	CSOS_SignalSemaphore(sem_InstallAccess);
    return(num);
}

U16  CS_INSTALL_GetIndexOfTPInstalled(void)
{
	U16	num = 0;
	CSOS_WaitSemaphore(sem_InstallAccess);
	num = CS_INSTALL_TPList[u8Now_Scan_Sat].u16TP_Index[Current_TP_Num];
	CSOS_SignalSemaphore(sem_InstallAccess);
    return(num);
}

U16 CS_INSTALL_GetCurrentTPIndex(void)
{
	U16	Index = 0;
	CSOS_WaitSemaphore(sem_InstallAccess);
	Index = u16Current_TP;
	CSOS_SignalSemaphore(sem_InstallAccess);
    return(Index);
}
/*
BOOL INSTALL_CheckTPExist(tCS_INSTALL_TPData TPData)
{
	tCS_INSTALL_TPData *Current_TPData;
	BOOL found  = FALSE;
			
	Current_TPData = CS_INSTALL_TPList.pTPListHead;
		
	while (Current_TPData != NULL)
	{
#ifdef MV_PROJECT
		if(Current_TPData->u16TP_Index == TPData.u16TP_Index)
#else // #ifdef MV_PROJECT
		if(Current_TPData->TerTP_Index == TPData.TerTP_Index)
#endif // #ifdef MV_PROJECT
		{
			found = TRUE;
                           break;
		}

		Current_TPData = Current_TPData->pNext_TP;
	}
		
	return(found);
}


BOOL CS_INSTALL_AppendTP(tCS_INSTALL_TPData TPData)
{
	tCS_INSTALL_TPData *pTemp = NULL;
	tCS_INSTALL_TPData *pNewTPData = NULL;

	CSOS_WaitSemaphore(sem_InstallAccess);

	if(!INSTALL_CheckTPExist(TPData))
	{
		if ((pNewTPData = (tCS_INSTALL_TPData *)CSOS_AllocateMemory(NULL, sizeof(tCS_INSTALL_TPData))) == NULL)
		{
			CSOS_SignalSemaphore(sem_InstallAccess);
			return(FALSE);
		}

		memcpy(pNewTPData,&TPData,sizeof(tCS_INSTALL_TPData));
		pNewTPData->pNext_TP = NULL;
		
		
		if( CS_INSTALL_TPList.pTPListHead == NULL)
		{
			CS_INSTALL_TPList.pTPListHead = pNewTPData;
		}
		else if(CS_INSTALL_TPList.pCurrent_TP != NULL)
		{
			pTemp = CS_INSTALL_TPList.pCurrent_TP->pNext_TP;
			CS_INSTALL_TPList.pCurrent_TP->pNext_TP = pNewTPData;
			pNewTPData->pNext_TP= pTemp;
		}
		else
		{
			CSOS_DeallocateMemory(NULL, pNewTPData);
			pNewTPData = NULL;
			CSOS_SignalSemaphore(sem_InstallAccess);
			return(FALSE);
		}

		CS_INSTALL_TPList.pCurrent_TP = pNewTPData;

		CS_INSTALL_TPList.TP_Num++;

	}

	CSOS_SignalSemaphore(sem_InstallAccess);

   	 return(TRUE);
}

st_INSTALL_TPList * CS_INSTALL_GetCurrentTPData(void)
{
	return(CS_INSTALL_TPList.pCurrent_TP);
}
*/

void CS_INSTALL_Get_AddTP_Data(U16 *u16Temp_TpIndex, U16 num_installed)
{
	int				i = 0, j = 0;

	// printf("===== num_installed : %d , u16Scan_TP_Total: %d\n", num_installed, u16Scan_TP_Total);
	
	for ( i = num_installed ; i < u16Scan_TP_Total ; i++ )
	{
		if ( j < 5 )
		{
			u16Temp_TpIndex[j] = CS_INSTALL_TPList[0].u16TP_Index[i];
		} else
			break;
		
		j++;
	}
}

void CS_INSTALL_GetSignalInfo (U8 *Level, U8 * Quality, BOOL* lock)
{
	tCS_FE_TerTunerInfo	FE_Info;

	CS_FE_GetInfo(&FE_Info);

	if(FE_Info.Ter_FECParity == kCS_FE_LOW_PARITY)
	{
		*Quality	= FE_Info.Ter_Signal_LPquality;
		*Level	= FE_Info.Ter_Signal_LPlevel;
		
		if (FE_Info.Ter_LPLockStatus == 1)
			* lock = TRUE;
		else
			* lock = FALSE;
	}
	else
	{
		*Quality	= FE_Info.Ter_Signal_HPquality;
		*Level	= FE_Info.Ter_Signal_HPlevel;
		if (FE_Info.Ter_HPLockStatus == 1)
			* lock = TRUE;
		else
			* lock = FALSE;
	}

   	return ;
}

U16 CS_INSTALL_GetFirstInstallService(void)
{
	return(FirstInstallService);
}


