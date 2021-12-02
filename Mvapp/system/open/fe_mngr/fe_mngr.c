#include "linuxos.h"
//#include "dvbt_tuner.h"
#include "fe_mngr.h"
#include "tunerinit.h"
#include "sattp.h"
#include "mv_motor.h"

#define 	FE_TASK_PRIORITY	10
#define 	FE_TASK_STACK_SIZE 	1024*4

#if 0 // by kb : 20100403
CSOS_TaskFlag_t 				FE_TASK_FLAG;
U8								FE_TASK_STACK[FE_TASK_STACK_SIZE];
CSOS_Task_Handle_t 				FE_TASK_HANDLE;
CSOS_TaskDesc_t 				*FE_TASK_DESC;
BOOL							FE_TaskStart = TRUE;

CSOS_MessageQueue_t				*CS_FE_MsgQid;
#endif

CSOS_Semaphore_t 				*sem_FrontendAccess = NULL;

static TunerSearchParam_t 		Current_TP_Data;

tCS_FE_State					Current_FE_state = eCS_FE_STATE_UNLOCKED;

tCS_FE_NotificationFunction		FE_NotifyFunction[kCS_FE_MAX_NO_OF_NOTIFY_CLIENTS];

tCS_FE_Error CS_FE_Register_Tuner_Notify(U8 *ClientId, tCS_FE_NotificationFunction NotifyFunction)
{
	tCS_FE_Error eError = eCS_FE_NO_ERROR;
	U8	index = 0;
	BOOL    found = FALSE;
	
 	if((ClientId != NULL)&&(NotifyFunction != NULL))
	{
		CSOS_WaitSemaphore(sem_FrontendAccess);
		
		for(index = 0; index< kCS_FE_MAX_NO_OF_NOTIFY_CLIENTS; index++)
		{
			if(FE_NotifyFunction[index] == NULL)
			{
				found = TRUE;
				FE_NotifyFunction[index] = NotifyFunction;
				*ClientId = index;
				break;
			}
		}

		CSOS_SignalSemaphore(sem_FrontendAccess);

		if(found)
			eError = eCS_FE_NO_ERROR;
		else
			eError = eCS_FE_ERROR;
	}
	else
	{
		eError = eCS_FE_ERROR;
	}
	
	return(eError);

}

tCS_FE_Error CS_FE_Unregister_Tuner_Notify(U8 ClientId)
{
	tCS_FE_Error eError = eCS_FE_NO_ERROR;
	
	if(ClientId < kCS_FE_MAX_NO_OF_NOTIFY_CLIENTS)
	{
		CSOS_WaitSemaphore(sem_FrontendAccess);

		if(FE_NotifyFunction[ClientId] != NULL)
		{
			FE_NotifyFunction[ClientId] = NULL;
		}

		CSOS_SignalSemaphore(sem_FrontendAccess);

		eError = eCS_FE_NO_ERROR;
	}
	else
	{
		eError = eCS_FE_ERROR;
	}
	
	return(eError);

}


void FE_Notify(tCS_FE_Notification notification)
{
	U8	notify_index = 0;
	
	for(notify_index = 0; notify_index< kCS_FE_MAX_NO_OF_NOTIFY_CLIENTS; notify_index++)
	{
		if(FE_NotifyFunction[notify_index] != NULL) 
		{
			(FE_NotifyFunction[notify_index])(notification);
		}
	}
	return;
}

#if 0
void FE_MessageHandle(void *pParam)
{
	tCS_FE_Msg_t	*msgReceived;

	while(FE_TaskStart)
	{
		msgReceived = NULL;
		msgReceived = (tCS_FE_Msg_t *)CSOS_ReceiveMessage(CS_FE_MsgQid);

		if( msgReceived != NULL )
		{
			//printf("NotifyEvent = %d, Current_FE_state = %d\n", msgReceived->NotifyEvent, Current_FE_state);
			switch(msgReceived->NotifyEvent)
			{
				case CS_TUNER_EV_LOCKED:
					{
						switch(Current_FE_state)
						{	
							case eCS_FE_STATE_UNLOCKED:
							case eCS_FE_STATE_SCANNING:
								{
									Current_FE_state = eCS_FE_STATE_LOCKED;	
									FE_Notify(eCS_FE_LOCKED);
								}
								break;
							default:
								break;
						}
					}
					break;

				case CS_TUNER_EV_UNLOCKED:
					{
						switch(Current_FE_state)
						{	
							case eCS_FE_STATE_LOCKED:
								{
									Current_FE_state = eCS_FE_STATE_UNLOCKED;	
									FE_Notify(eCS_FE_SIGNAL_LOST);
								}
								break;

							case eCS_FE_STATE_SCANNING:
								{
									Current_FE_state = eCS_FE_STATE_UNLOCKED;	
									FE_Notify(eCS_FE_UNLOCKED);
								}
								break;
							default:
								break;
						}
					}
					break;

				default:
					break;

			}

			CSOS_ReleaseMessagebuffer(CS_FE_MsgQid, (void *)msgReceived);

		}

	}
}

void TUNER_Notify_to_FE(tCS_DVBT_TUNER_EventType_t event, tCS_TUNER_Type_t tunertype)
{
	tCS_FE_Msg_t * mess;

	mess = (tCS_FE_Msg_t *) CSOS_AllocateMemory(NULL, sizeof(tCS_FE_Msg_t));

	if (mess != NULL)
	{
		mess->NotifyEvent = event;
		mess->TunerType = tunertype;
		CSOS_SendMessage(CS_FE_MsgQid, (tCS_FE_Msg_t *)mess, sizeof(tCS_FE_Msg_t), 0);
		CSOS_DeallocateMemory(NULL, mess);
	}
    return;
}
#endif

BOOL CS_FE_Init (void)
{
#if 0
	tCS_TUNER_InitParams initparam;
#endif
	
	Current_FE_state = eCS_FE_STATE_UNLOCKED;

    // FE_TaskStart = TRUE;

    memset(&Current_TP_Data, 0, sizeof(tCS_FE_TerScanParams));

	//initparam.NotifyFunction = TUNER_Notify_to_FE;

#if 0
	CS_DVBT_TUNER_Init(initparam);
#endif

	// TunerInitial();
	// printf("tunerHandleId = %d\n", Tuner_HandleId[0]);

	sem_FrontendAccess  = CSOS_CreateSemaphoreFifo ( NULL, 1 );

#if 0 // by kb : 20100403
	if((CS_FE_MsgQid = CSOS_CreateMessageQueue("/FE_MsgQid",sizeof(tCS_FE_Msg_t), 20 )) == NULL)
	{
		printf("create FE_MsgQid error\n");
		return(FALSE);
	}
        
	if (CSOS_CreateTask(FE_MessageHandle,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						FE_TASK_STACK_SIZE,				/* size of stack in bytes */
						FE_TASK_STACK, 				/* pointer to stack base */
						NULL,
						&FE_TASK_HANDLE,			/* return thread handle */
						&FE_TASK_DESC, 			/* space to store thread data */ 
						FE_TASK_PRIORITY,
						"FE_MessageHandle", 				/* name of thread */
						FE_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the FE_MessageHandle \n" );
		return(FALSE);
	}

	CSOS_StartTask(FE_TASK_HANDLE);
#endif // #if 0
	
	return(TRUE);
}

/* For Blind Scan By KB Kim 2011.02.26 */
U16 GetBlindTpResult(U8 currentPol, TunerBlindTpData_t *tpData)
{
	U16 totalTP;

	totalTP = TunerGetBlindTpData(Tuner_HandleId[0], currentPol, tpData);

	return totalTP;
}

/* For Blind Scan By KB Kim 2011.02.26 */
U8 MV_FE_GetBlindProcess(void)
{
	return TunerGetBlindProcess(Tuner_HandleId[0]);
}

tCS_FE_Error MV_FE_SetBlindProcess (U8 process)
{
	TunerSetBlindProcess(Tuner_HandleId[0], process);

	return eCS_FE_NO_ERROR;
}


tCS_FE_Error CS_FE_StartBlindScan (MV_BlindScanParams blindParam)
{
	TunerSearchParam_t 	scanParams;
	MV_stSatInfo		satData;
	U32                 connection;
	U16                 lnbFreq;
	U16                 centerFreq;
	U8					u8Temp_LNB_Type = 0;
	
	MV_GetSatelliteData_ByIndex(&satData, blindParam.SatIndex);

	if (satData.u8LNBType < EN_LNB_TYPE_UNIVERSAL_5150_5750)
	{
		u8Temp_LNB_Type = 1;
	}
	else if ( (EN_LNB_TYPE_UNIVERSAL_5150_5750 <= satData.u8LNBType) &&  (satData.u8LNBType < EN_LNB_TYPE_UNIVERSAL_9750_10600) )
	{
		u8Temp_LNB_Type = 2;
	}
	else
	{
		switch(satData.u8LNBType)
		{				
			case EN_LNB_TYPE_DIGITURK1:
				u8Temp_LNB_Type = 3;
				break;
			case EN_LNB_TYPE_DIGITURK2:
				u8Temp_LNB_Type = 4;
				break;
			case EN_LNB_TYPE_DIGITURK3:
				u8Temp_LNB_Type = 5;
				break;
			case EN_LNB_TYPE_DIGITURK4:
				u8Temp_LNB_Type = 6;
				break;
			default :
				u8Temp_LNB_Type = 0;
				break;
		}
	}

	if (u8Temp_LNB_Type == 2)
	{
		if (blindParam.LnbHi)
		{
			return eCS_FE_ERROR;
		}
		if (blindParam.PolarH)
		{
			lnbFreq = satData.u16LocalFrequency_High;
		}
		else
		{
			lnbFreq = satData.u16LocalFrequency;
		}
		scanParams.StartFrequency = lnbFreq - TUNER_MAX_FREQ;
		scanParams.StopFrequency  = lnbFreq - TUNER_MIN_FREQ;
	}
	else if (u8Temp_LNB_Type == 0)
	{
		centerFreq = (satData.u16LocalFrequency + TUNER_MIN_FREQ + satData.u16LocalFrequency_High + TUNER_MAX_FREQ) / 2;
		if (blindParam.LnbHi)
		{
			scanParams.StartFrequency = centerFreq;
			scanParams.StopFrequency  = satData.u16LocalFrequency_High + TUNER_MAX_FREQ;
		}
		else
		{
			scanParams.StartFrequency = satData.u16LocalFrequency + TUNER_MIN_FREQ;
			scanParams.StopFrequency  = centerFreq;
		}
	}
	else
	{
		if (blindParam.LnbHi)
		{
			return eCS_FE_ERROR;
		}

		if ((satData.u16LocalFrequency < 9750) && (satData.u16LocalFrequency > TUNER_MAX_FREQ))
		{
			/* C - Band LNB */
			scanParams.StartFrequency = satData.u16LocalFrequency - TUNER_MAX_FREQ;
			scanParams.StopFrequency  = satData.u16LocalFrequency - TUNER_MIN_FREQ;
		}
		else
		{
			scanParams.StartFrequency = satData.u16LocalFrequency + TUNER_MIN_FREQ;
			scanParams.StopFrequency  = satData.u16LocalFrequency + TUNER_MAX_FREQ;
		}
	}
	connection = CS_DBU_GetAntenna_Type() & 0xFF; 
	CSOS_WaitSemaphore(sem_FrontendAccess);
	scanParams.SearchMode           = 0;
	scanParams.Symbolrate			= 0;
	scanParams.TpFrequency			= scanParams.StartFrequency + 10;
	scanParams.HorVer				= blindParam.PolarH;
	scanParams.TpNumber				= 0xFFFF;
	scanParams.TunerNumber			= 0;
	scanParams.SignalType			= STREAM_AUTO;
	scanParams.RollOff             	= ROLL_OFF_35;
	scanParams.FecCode             	= DVB_FEC_AUTO;
	if ( CS_DBU_GetLNB_Power() == eCS_DBU_ON )
		scanParams.Power            = 1;
	else
		scanParams.Power            = 0;
	scanParams.On22khz             	= satData.u16Tone22K;
	printf("Blind Scan Min [%d], Max [%d], Tp[%d], Local[%d]\n", scanParams.StartFrequency, scanParams.StopFrequency, scanParams.TpFrequency, satData.u16LocalFrequency);

	/* For Motor Control By KB Kim 2011.05.22 */
	scanParams.ConnectionMode   = (U8)CS_DBU_GetAntenna_Type();
	scanParams.DiseqcPort       = satData.u16DiSEqC;
	
	scanParams.ToneBurst           	= satData.u16ToneBurst;
	scanParams.Sw12V               	= 0;
	scanParams.SkewValue           	= 0xFF;
	scanParams.LnbType             	= u8Temp_LNB_Type;
	scanParams.LnbLocalHi          	= satData.u16LocalFrequency_High;
	scanParams.LnbLocalLow         	= satData.u16LocalFrequency;

	/* For Motor Control By KB Kim 2011.05.22 */
	scanParams.MotorPosition       	= satData.u8MotorPosition;
	if ((scanParams.ConnectionMode == SCAN_MODE_USALS) || (scanParams.ConnectionMode == SCAN_MODE_DISECQ_MOTOR))
	{
		scanParams.MotorAngle       = TunerGetMotorAngle((int)satData.s16Longitude, (int)CS_DBU_GetLocal_Longitude(), (int)CS_DBU_GetLocal_Latitude());
	}
	else
	{
		scanParams.MotorAngle       = 0;
	}
	scanParams.UnicableSatPosition 	= 0;
	scanParams.UniCableUBand       	= 0;
	scanParams.UniCableBandFreq    	= 0;
#ifdef USA_SUPPORT /* USA Support Version */
	scanParams.LagacySwitch       	= 0;
#endif

	TunerSearchStart(Tuner_HandleId[0], &scanParams);

	CSOS_SignalSemaphore(sem_FrontendAccess);

	return eCS_FE_NO_ERROR;
}

tCS_FE_Error CS_FE_StartScan ( MV_ScanParams scanparam, unsigned char mode)
{
	tCS_FE_Error 		eError = eCS_FE_NO_ERROR;
	TunerSearchParam_t 	ScanParams;
	MV_stSatInfo		Temp_SatData;
	MV_stSatInfo		Current_SatData;

#if 0
	if( mTerCHFreqInRange(scanparam.u16TPFrequency, Current_TP_Data.TpFrequency) /*&& CS_DVBT_TUNER_IsLocked()*/ )
	{
		printf("\n============= mTerCHFreqInRange ===\n");
		Current_FE_state = eCS_FE_STATE_SCANNING;	
		FE_Notify(eCS_FE_LOCKED);
	}
	else
#endif
	{
		U8					u8Temp_LNB_Type = 0;
		tCS_DBU_Service 	Last_ServiceTriplet;

		Last_ServiceTriplet = CS_DB_GetLastServiceTriplet();

		if ( First_Tunning_State == FALSE )
			MV_DB_Get_SatData_By_Chindex(&Current_SatData, Last_ServiceTriplet.sCS_DBU_ServiceIndex);
		else
		{
			First_Tunning_State = FALSE;
			MV_DB_Get_SatData_By_Chindex(&Current_SatData, CS_DB_GetCurrentService_OrderIndex());
		}
		MV_GetSatelliteData_ByIndex(&Temp_SatData, MV_DB_Get_SatIndex_By_TPindex(scanparam.u16Tpnumber));

#if 0 /* For Motor Control By KB Kim 2011.05.22 */
		if ( CS_DBU_GetAntenna_Type() == SCAN_MODE_DISECQ_MOTOR && Temp_SatData.u8MotorPosition != 0 )
		{
			Motor_Moving_Start((U16)Current_SatData.s16Longitude, (U16)Temp_SatData.s16Longitude);
			DVB_MotorControl(EN_MOTOR_CMD_HALT, 0);
			DVB_MotorControl(EN_MOTOR_CMD_GOTO_POSITION, Temp_SatData.u8MotorPosition);
		}
		else if ( CS_DBU_GetAntenna_Type() == SCAN_MODE_USALS )
		{
			Motor_Moving_Start((U16)Current_SatData.s16Longitude, (U16)Temp_SatData.s16Longitude);
			DVB_MotorGotoX (Temp_SatData.s16Longitude, 0, (U16)CS_DBU_GetLocal_Latitude(), 0, (U16)CS_DBU_GetLocal_Longitude(), 0);
		}
#endif

		//printf("\nTP : %d -> Sat_Name : %s ==> LNB : %d ==========\n", scanparam.u16Tpnumber, Temp_SatData.acSatelliteName, Temp_SatData.u8LNBType);

		if (Temp_SatData.u8LNBType < EN_LNB_TYPE_UNIVERSAL_5150_5750) {
			u8Temp_LNB_Type = 1;
		} else if ( (EN_LNB_TYPE_UNIVERSAL_5150_5750 <= Temp_SatData.u8LNBType) &&  (Temp_SatData.u8LNBType < EN_LNB_TYPE_UNIVERSAL_9750_10600) ) {
			u8Temp_LNB_Type = 2;
		} else {
			switch(Temp_SatData.u8LNBType)
			{				
				case EN_LNB_TYPE_DIGITURK1:
					u8Temp_LNB_Type = 3;
					break;
				case EN_LNB_TYPE_DIGITURK2:
					u8Temp_LNB_Type = 4;
					break;
				case EN_LNB_TYPE_DIGITURK3:
					u8Temp_LNB_Type = 5;
					break;
				case EN_LNB_TYPE_DIGITURK4:
					u8Temp_LNB_Type = 6;
					break;
				default :
					u8Temp_LNB_Type = 0;
					break;
			}
		}
		
		CSOS_WaitSemaphore(sem_FrontendAccess);

		Current_FE_state = eCS_FE_STATE_SCANNING;
		// FE_Notify(eCS_FE_LOCKED);

		ScanParams.SearchMode           = mode;
		ScanParams.Symbolrate			= scanparam.u16SymbolRate;
		ScanParams.TpFrequency			= scanparam.u16TPFrequency;
		ScanParams.HorVer				= scanparam.u8Polar_H;
		ScanParams.TpNumber				= scanparam.u16Tpnumber;
		ScanParams.TunerNumber			= 0;
		ScanParams.SignalType			= STREAM_AUTO;
		ScanParams.RollOff             	= ROLL_OFF_35;
		ScanParams.FecCode             	= DVB_FEC_AUTO;
		if ( CS_DBU_GetLNB_Power() == eCS_DBU_ON )
			ScanParams.Power               	= 1;
		else
			ScanParams.Power               	= 0;
		ScanParams.On22khz             	= Temp_SatData.u16Tone22K;
		ScanParams.DiseqcPort          	= Temp_SatData.u16DiSEqC; /* Port A */
		ScanParams.ToneBurst           	= TONEBURST_NONE;
		ScanParams.Sw12V               	= 0;
		ScanParams.SkewValue           	= 0xFF;
		ScanParams.LnbType             	= u8Temp_LNB_Type;
		ScanParams.LnbLocalHi          	= Temp_SatData.u16LocalFrequency_High;
		ScanParams.LnbLocalLow         	= Temp_SatData.u16LocalFrequency;

		/* For Motor Control By KB Kim 2011.05.22 */
		ScanParams.ConnectionMode      	= (U8)CS_DBU_GetAntenna_Type();
		ScanParams.MotorPosition       	= Temp_SatData.u8MotorPosition;
		if ((ScanParams.ConnectionMode == SCAN_MODE_USALS) || (ScanParams.ConnectionMode == SCAN_MODE_DISECQ_MOTOR))
		{
			ScanParams.MotorAngle       = TunerGetMotorAngle((int)Temp_SatData.s16Longitude, (int)CS_DBU_GetLocal_Longitude(), (int)CS_DBU_GetLocal_Latitude());
		}
		else
		{
			ScanParams.MotorAngle       = 0;
		}
		
		ScanParams.UnicableSatPosition 	= 0;
		ScanParams.UniCableUBand       	= 0;
		ScanParams.UniCableBandFreq    	= 0;
#ifdef USA_SUPPORT /* USA Support Version */
		ScanParams.LagacySwitch       	= 0;
#endif

		// printf("\n\nCS_FE_StartScan : FrequencyKHz = %d, Pola = %d , POWER = %d \n\n\n", ScanParams.TpFrequency, ScanParams.HorVer, ScanParams.Power);
		//printf("FrequencyKHz = %d, Priority = %d\n", ScanParams.TpFrequency, ScanParams.HorVer);
		TunerSearchStart(Tuner_HandleId[0], &ScanParams);
		//CS_DVBT_TUNER_SetFrequency( ScanParams );
		//printf("TunerSearchStart END\n");
		Current_TP_Data.TpFrequency		= scanparam.u16TPFrequency;
		Current_TP_Data.Symbolrate		= scanparam.u16SymbolRate;
		Current_TP_Data.HorVer			= scanparam.u8Polar_H;

		CSOS_SignalSemaphore(sem_FrontendAccess);
	}

	return(eError);
}

tCS_FE_Error CS_FE_StartScanByLocal ( MV_ScanParams scanparam, MV_stSatInfo	*Temp_SatData , unsigned char mode)
{
	tCS_FE_Error 		eError = eCS_FE_NO_ERROR;
	TunerSearchParam_t 	ScanParams;

	{
		U8		u8Temp_LNB_Type = 0;

		//printf("\n Sat_Name : %s ==> LNB : %d ==========\n", Temp_SatData->acSatelliteName, Temp_SatData->u8LNBType);

		if (Temp_SatData->u8LNBType < EN_LNB_TYPE_UNIVERSAL_5150_5750) {
			u8Temp_LNB_Type = 1;
		} else if ( (EN_LNB_TYPE_UNIVERSAL_5150_5750 <= Temp_SatData->u8LNBType) &&  (Temp_SatData->u8LNBType < EN_LNB_TYPE_UNIVERSAL_9750_10600) ) {
			u8Temp_LNB_Type = 2;
		} else {
			switch(Temp_SatData->u8LNBType)
			{				
				case EN_LNB_TYPE_DIGITURK1:
					u8Temp_LNB_Type = 3;
					break;
				case EN_LNB_TYPE_DIGITURK2:
					u8Temp_LNB_Type = 4;
					break;
				case EN_LNB_TYPE_DIGITURK3:
					u8Temp_LNB_Type = 5;
					break;
				case EN_LNB_TYPE_DIGITURK4:
					u8Temp_LNB_Type = 6;
					break;
				default :
					u8Temp_LNB_Type = 0;
					break;
			}
		}
		
		CSOS_WaitSemaphore(sem_FrontendAccess);

		Current_FE_state = eCS_FE_STATE_SCANNING;
		// FE_Notify(eCS_FE_LOCKED);

		ScanParams.SearchMode           = mode;
		ScanParams.Symbolrate			= scanparam.u16SymbolRate;
		ScanParams.TpFrequency			= scanparam.u16TPFrequency;
		ScanParams.HorVer				= scanparam.u8Polar_H;
		ScanParams.TpNumber				= scanparam.u16Tpnumber;
		ScanParams.TunerNumber			= 0;
		ScanParams.SignalType			= STREAM_AUTO;
		ScanParams.RollOff             	= ROLL_OFF_35;
		ScanParams.FecCode             	= DVB_FEC_AUTO;
		if ( CS_DBU_GetLNB_Power() == eCS_DBU_ON )
			ScanParams.Power               	= 1;
		else
			ScanParams.Power               	= 0;
		ScanParams.On22khz             	= Temp_SatData->u16Tone22K;
		ScanParams.DiseqcPort          	= Temp_SatData->u16DiSEqC; /* Port A */
		ScanParams.ToneBurst           	= TONEBURST_NONE;
		ScanParams.Sw12V               	= 0;
		ScanParams.SkewValue           	= 0xFF;
		ScanParams.LnbType             	= u8Temp_LNB_Type;
		ScanParams.LnbLocalHi          	= Temp_SatData->u16LocalFrequency_High;
		ScanParams.LnbLocalLow         	= Temp_SatData->u16LocalFrequency;

		/* For Motor Control By KB Kim 2011.05.22 */
		ScanParams.ConnectionMode      	= (U8)CS_DBU_GetAntenna_Type();
		ScanParams.MotorPosition       	= Temp_SatData->u8MotorPosition;
		if ((ScanParams.ConnectionMode == SCAN_MODE_USALS) || (ScanParams.ConnectionMode == SCAN_MODE_DISECQ_MOTOR))
		{
			ScanParams.MotorAngle       = TunerGetMotorAngle((int)Temp_SatData->s16Longitude, (int)CS_DBU_GetLocal_Longitude(), (int)CS_DBU_GetLocal_Latitude());
		}
		else
		{
			ScanParams.MotorAngle       = 0;
		}
		ScanParams.UnicableSatPosition 	= 0;
		ScanParams.UniCableUBand       	= 0;
		ScanParams.UniCableBandFreq    	= 0;
#ifdef USA_SUPPORT /* USA Support Version */
		ScanParams.LagacySwitch       	= 0;
#endif

		// printf("\n\nCS_FE_StartScanByLocal : FrequencyKHz = %d, Priority = %d , POWER = %d \n\n\n", ScanParams.TpFrequency, ScanParams.HorVer, ScanParams.Power);
		TunerSearchStart(Tuner_HandleId[0], &ScanParams);
		//CS_DVBT_TUNER_SetFrequency( ScanParams );
		//printf("TunerSearchStart END\n");
		Current_TP_Data.TpFrequency		= scanparam.u16TPFrequency;
		Current_TP_Data.Symbolrate		= scanparam.u16SymbolRate;
		Current_TP_Data.HorVer			= scanparam.u8Polar_H;

		CSOS_SignalSemaphore(sem_FrontendAccess);
	}

	return(eError);
}

tCS_FE_Error CS_FE_StopScan (void)
{

	tCS_FE_Error eError = eCS_FE_NO_ERROR;
	CSOS_WaitSemaphore(sem_FrontendAccess);
	
	Current_FE_state = eCS_FE_STATE_NONE;

	CSOS_SignalSemaphore(sem_FrontendAccess);
	
   	return(eError);
}

/* By KB Kim 2011.01.13 */
tCS_FE_Error CS_FE_StopSearch (void)
{

	tCS_FE_Error eError = eCS_FE_NO_ERROR;
	CSOS_WaitSemaphore(sem_FrontendAccess);
	
	Current_FE_state = eCS_FE_STATE_NONE;

	TunerSearchStop(Tuner_HandleId[0]);
	
	CSOS_SignalSemaphore(sem_FrontendAccess);
	
   	return(eError);
}

tCS_FE_Error CS_FE_GetInfo (tCS_FE_TerTunerInfo *FEInfo)
{
	tCS_DVBT_TUNER_TunerInfo tunerInfo;

    memset(&tunerInfo, 0, sizeof(tCS_DVBT_TUNER_TunerInfo));

	//CS_DVBT_TUNER_GetTunerInfo(&tunerInfo);

	FEInfo->Ter_FrequencyKHz 		=	tunerInfo.ScanData.FrequencyKHz;
	FEInfo->Ter_BW				=	tunerInfo.ScanData.ChanBW;
	FEInfo->Ter_HPLockStatus		=	tunerInfo.Signal_HPLock;
	FEInfo->Ter_LPLockStatus		=	tunerInfo.Signal_LPLock;	
	FEInfo->Ter_Signal_HPquality	        =	tunerInfo.Signal_HPQuality;
	FEInfo->Ter_Signal_HPlevel		=	tunerInfo.Signal_HPLevel;
	FEInfo->Ter_Signal_LPquality	        =	tunerInfo.Signal_LPQuality;
	FEInfo->Ter_Signal_LPlevel		=	tunerInfo.Signal_LPLevel;
	FEInfo->Ter_Offset				=	((S8)tunerInfo.Freq_Offset);

	switch(tunerInfo.FECHP)
	{
		case CS_DVBT_TUNER_FEC_NONE:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_NONE;
		}
		break;
		case CS_DVBT_TUNER_FEC_1_2:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_1_2;
		}
		break;
		case CS_DVBT_TUNER_FEC_2_3:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_2_3;
		}
		break;
		case CS_DVBT_TUNER_FEC_3_4:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_3_4;
		}
		break;
		case CS_DVBT_TUNER_FEC_5_6:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_5_6;
		}
		break;
		case CS_DVBT_TUNER_FEC_7_8:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_7_8;
		}
		break;
		
		default:
		{
			FEInfo->Ter_FECHP = kCS_FE_FEC_NONE;
		}
		break;
	}

	switch(tunerInfo.FECLP)
	{
		case CS_DVBT_TUNER_FEC_NONE:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_NONE;
		}
		break;
		case CS_DVBT_TUNER_FEC_1_2:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_1_2;
		}
		break;
		case CS_DVBT_TUNER_FEC_2_3:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_2_3;
		}
		break;
		case CS_DVBT_TUNER_FEC_3_4:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_3_4;
		}
		break;
		case CS_DVBT_TUNER_FEC_5_6:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_5_6;
		}
		break;
		case CS_DVBT_TUNER_FEC_7_8:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_7_8;
		}
		break;		
		default:
		{
			FEInfo->Ter_FECLP = kCS_FE_FEC_NONE;
		}
		break;
	}

	switch(tunerInfo.Constellation)
	{
		case CS_DVBT_TUNER_MOD_NONE:
		{
			FEInfo->Ter_Constellation = kCS_FE_MODULATION_NONE;
		}
		break;
		case CS_DVBT_TUNER_MOD_QPSK:
		{
			FEInfo->Ter_Constellation = kCS_FE_MODULATION_QPSK;
		}
		break;
		case CS_DVBT_TUNER_MOD_16QAM:
		{
			FEInfo->Ter_Constellation = kCS_FE_MODULATION_16QAM;
		}
		break;	
		case CS_DVBT_TUNER_MOD_64QAM:
		{
			FEInfo->Ter_Constellation = kCS_FE_MODULATION_64QAM;
		}
		break;
		default:
		{
			FEInfo->Ter_Constellation = kCS_FE_MODULATION_NONE;
		}
		break;
	}

	switch(tunerInfo.ScanData.Priority)
	{
		case CS_DVBT_PARITY_AUTO:
		{
			FEInfo->Ter_FECParity = kCS_FE_HIGH_PARITY;
		}
		break;
		case CS_DVBT_PARITY_HIGH:
		{
			FEInfo->Ter_FECParity = kCS_FE_HIGH_PARITY;
		}
		break;
		case CS_DVBT_PARITY_LOW:
		{
			FEInfo->Ter_FECParity = kCS_FE_LOW_PARITY;
		}
		break;		
		default:
		{
			FEInfo->Ter_FECParity = kCS_FE_HIGH_PARITY;
		}
		break;
	}

	switch(tunerInfo.Force)
	{
		case CS_DVBT_FORCE_NONE:
		{
			FEInfo->Ter_Force = 0;
		}
		break;
		case CS_DVBT_FORCE:
		{
			FEInfo->Ter_Force = 1;
		}
		break;	
		default:
		{
			FEInfo->Ter_Force = 0;
		}
		break;
	}

	switch(tunerInfo.Mode)
	{
		case CS_DVBT_TUNER_MODE_2K:
		{
			FEInfo->Ter_Mode	= kCS_FE_TM_2K;
		}
		break;
		case CS_DVBT_TUNER_MODE_8K:
		{
			FEInfo->Ter_Mode	= kCS_FE_TM_8K;
		}
		break;	
		default:
		{
			FEInfo->Ter_Mode = kCS_FE_TM_2K;
		}
		break;
	}

	switch(tunerInfo.GuardInterval)
	{
		case CS_DVBT_TUNER_GUARD_1_32:
		{
			FEInfo->Ter_GuardInterval = kCS_FE_GI_32;
		}
		break;
		case CS_DVBT_TUNER_GUARD_1_16:
		{
			FEInfo->Ter_GuardInterval 	= kCS_FE_GI_16;
		}
		break;
		case CS_DVBT_TUNER_GUARD_1_8:
		{
			FEInfo->Ter_GuardInterval 	= kCS_FE_GI_8;
		}
		break;	
		case CS_DVBT_TUNER_GUARD_1_4:
		{
			FEInfo->Ter_GuardInterval 	= kCS_FE_GI_4;
		}
		break;	
		default:
		{
			FEInfo->Ter_GuardInterval 	= kCS_FE_GI_32;
		}
		break;
	}

	switch(tunerInfo.HierarchicalInfo)
	{
		case CS_DVBT_TUNER_HIER_NONE:
		{
			FEInfo->Ter_Hierarchical	= kCS_FE_HI_NONE;
		}
		break;
		case CS_DVBT_TUNER_HIER_1:
		{
			FEInfo->Ter_Hierarchical	= kCS_FE_HI_ALPHA1;
		}
		break;
		case CS_DVBT_TUNER_HIER_2:
		{
			FEInfo->Ter_Hierarchical	= kCS_FE_HI_ALPHA2;
		}
		break;	
		case CS_DVBT_TUNER_HIER_4:
		{
			FEInfo->Ter_Hierarchical	= kCS_FE_HI_ALPHA4;
		}
		break;	
		default:
		{
			FEInfo->Ter_Hierarchical	= kCS_FE_HI_NONE;
		}
		break;
	}

	return(eCS_FE_NO_ERROR);
	
}

void MV_FE_SetTuning_TP(MV_stSatInfo *Temp_SatData, U8 u8SatIndex, U8 u8TPIndex)
{
	MV_ScanParams 			FE_ScanData;
	MV_stTPInfo				DB_TPData;

	CS_FE_StopScan();

	MV_DB_Get_TPdata_By_Satindex_and_TPnumber(&DB_TPData, u8SatIndex, u8TPIndex);

	FE_ScanData.u16Tpnumber		= DB_TPData.u16TPIndex;
	FE_ScanData.u16SymbolRate	= DB_TPData.u16SymbolRate;
	FE_ScanData.u16TPFrequency	= DB_TPData.u16TPFrequency;
	FE_ScanData.u8Polar_H		= DB_TPData.u8Polar_H;
	
	CS_FE_StartScanByLocal(FE_ScanData, Temp_SatData, 1);
}


