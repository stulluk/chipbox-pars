
#include "linuxos.h"

/* 2010.09.04 By KB KIm For New SI */
// #include "demux.h"
#include "tableApi.h"

#include "db_builder.h"
#include "date_time.h"

#define kNbSecondPerMinute 				60
#define kNbSecondPerHour 					3600
#define kNbMinutePerHour 					60
#define kNbHourPerDay					24

CSOS_Semaphore_t 	* sem_DateTimeAccess = NULL;
CSOS_Semaphore_t 	* sem_DT_Notify_Access = NULL;
tCS_DT_Info 	      sCS_DT_Info;


tCS_DT_Notify		  NotifyDTTab[kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS];
tCS_DT_OsTimerNotify  NotifyOsTimer = NULL;

CS_OS_Clock_t Job_Begin_OsTime = 0;

/*Task params*/
#define DT_IDLE_TASK_PRIORITY	20
#define DT_IDLE_STACK_SIZE 		1024*4
CSOS_TaskFlag_t DT_IDLE_TASK_FLAG;

U8	DT_IDLE_STACK[DT_IDLE_STACK_SIZE];
CSOS_Task_Handle_t  DT_IDLE_TASK_HANDLE;
CSOS_TaskDesc_t    *DT_IDLE_TASK_DESC;

/* 2010.09.04 By KB KIm For New SI */
U8 TdtTotTableInfoId;

extern U16 CS_MW_GetTimeMode(void);

tCS_DT_Time CS_DT_UTCtoHM(U16 utc)
{
	tCS_DT_Time time;

	time.hour    	= (U8) ( ( ((utc >> 12)& 0x0F) * 10 ) + ( (utc >> 8) &0x0F ) );
	time.minute 	= (U8) ( ( ((utc >> 4) & 0x0F) * 10 ) + ( utc&0x0F ) );

	return time;

}

tCS_DT_Date CS_DT_MJDtoYMD(U16 mjd)
{
	S32 year_x, month_x;
	S32 m_day, m_month,m_year;
	U8 k;
	tCS_DT_Date date;

	U32   cal_mjd;


	cal_mjd = mjd +0xa000;

	year_x 	 = (int)( (cal_mjd*100 - 1507820) / 36525 );
	month_x  = (int)( (cal_mjd*10000 - 149561000 - (int)((year_x * 36525)/100)*10000) / 306001 );
	m_day = cal_mjd - 14956 - (int) ((year_x * 36525)/100) - (int) ((month_x * 306001)/10000);

	if (month_x == 14 || month_x == 15)
		k = 1;
	else
		k = 0;

	m_year = (int) (year_x + k);
	m_month  = (int) (month_x - 1 - (k * 12));  

	if( m_month > 12 )
	{
		m_year++; 
		m_month -= 12;
	}
	/* Validate results before accessing array */


	m_year += 1900;

	date.year = m_year;
	date.month = m_month;
	date.day = m_day;

	return date;


}

U16 CS_DT_HMtoUTC(tCS_DT_Time hm)
{
	U16   utc;
	U8  a,b,c,d;
	
	a=(hm.minute%10)&0x0f;
	b=(hm.minute/10)&0x0f;
	c=(hm.hour%10)&0x0f;
	d=(hm.hour/10)&0x0f;

	utc=(d<<12)|(c<<8)|(b<<4)|a;
	return utc;
}

U16 CS_DT_YMDtoMJD(tCS_DT_Date ymd)
{
	
    	S32 L,MJD;

    	if( (ymd.month ==1) || (ymd.month==2) )
        		L=1;
    	else
        		L=0;

    	MJD = 14956 + ymd.day+ ((ymd.year -L-1900)*36525)/100 + ((ymd.month+1+L*12)*306001)/10000;

        MJD-= 0xa000;

    	return ((U16)MJD);
}


U32 CS_DT_UTC_Add(U16 l_utc, U16 r_utc)
{
	U32	result;
	U32	hours, minutes,overflow = 0;

	hours 	= ( ( ( ( l_utc & 0xF000 ) >> 12 ) * 10 )
				+ ( ( l_utc & 0x0F00 ) >> 8 )
				+ ( ( ( r_utc & 0xF000 ) >> 12 ) * 10 )
				+ ( ( r_utc & 0x0F00 ) >> 8 ) );
	minutes 	= ( ( ( ( l_utc & 0x00F0 ) >> 4 ) * 10 )
			  	+ ( l_utc & 0x000F )
			  	+ ( ( ( r_utc & 0x00F0 ) >> 4 ) * 10 )
			  	+ ( r_utc & 0x000F) );

	
	while(minutes >= 60) 
	{
		hours++;
		minutes -= 60;
	}

	while(hours >= 24) 
	{
		overflow++;
		hours -= 24;
	}      

	/*
	** Hours digits.
	*/
	result	 = ( U32 ) ( overflow << 24 );
	result 	|= ( U32 ) ( ( ( hours / 10 ) & 0xF ) << 12 );
	result 	|= ( U32 ) ( ( ( hours % 10 ) & 0xF ) << 8 );

	/*
	** Minutes digits.
	*/
	result 	|= ( U32 ) ( ( ( minutes / 10 ) & 0xF ) << 4 );
	result 	|= ( U32 ) ((  minutes % 10 ) & 0xF );	

	return result;
}

U32 CS_DT_UTC_Subtract(U16 l_utc, U16 r_utc)
{
	S32	hours;
	S32	minutes;
	S32	underflow = 0;
	U32	result;

	hours 	= ( ( ( ( l_utc & 0xF000 ) >> 12 ) * 10 )
				+ ( ( l_utc & 0x0F00 ) >> 8 )
				- ( ( ( r_utc & 0xF000 ) >> 12 ) * 10 )
				- ( ( r_utc & 0x0F00 ) >> 8 ) );
	minutes 	= ( ( ( ( l_utc & 0x00F0 ) >> 4 ) * 10 )
			  	+ ( l_utc & 0x000F)
			 	- ( ( ( r_utc & 0x00F0 ) >> 4 ) * 10 )
			  	- ( r_utc & 0x000F ) );

	while(minutes < 0) 
	{
		hours--;
		minutes += 60;
	}
	
	while(hours < 0) 
	{
		underflow--;
		hours += 24;
	}

	/*
	** Hours digits.
	*/
	result = ( U32 ) ( abs(underflow) << 24 );
	result |= ( U32 ) ( ( ( hours / 10 ) & 0xF ) << 12 );
	result |= ( U32 ) ( ( ( hours % 10 ) & 0xF ) << 8);
	/*
	** Minutes digits.
	*/
	result |= ( U32 ) ( ( ( minutes / 10 ) & 0xF ) << 4 );
	result |= ( U32 ) ( ( minutes % 10 ) & 0xF);

	return result;
}

tCS_DT_Error CS_DT_Register_DateTime_Notify(U8 *ClientId, tCS_DT_UpdateEvent UpdateEvent, tCS_DT_UpdateCallback NotifyFunction)
{
	tCS_DT_Error eError = eCS_DT_NO_ERROR;
	U8	index = 0;
	BOOL    found = FALSE;
	
 	if((ClientId != NULL)&&(NotifyFunction != NULL))
	{
		CSOS_WaitSemaphore(sem_DT_Notify_Access);
		
		for(index = 0; index< kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS; index++)
		{
			if(NotifyDTTab[index].NotifyFunction == NULL)
			{
				found = TRUE;
				NotifyDTTab[index].NotifyFunction = NotifyFunction;
				NotifyDTTab[index].UpdateEvent = UpdateEvent;
				*ClientId = index;
				break;
			}
		}

		CSOS_SignalSemaphore(sem_DT_Notify_Access);

		if(found)
			eError = eCS_DT_NO_ERROR;
		else
			eError = eCS_DT_ERROR;
	}
	else
	{
		eError = eCS_DT_ERROR;
	}
	
	return(eError);

}

tCS_DT_Error CS_DT_Unregister_DateTime_Notify(U8 ClientId)
{
	tCS_DT_Error eError = eCS_DT_NO_ERROR;
	
	if(ClientId < kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS)
	{
		CSOS_WaitSemaphore(sem_DT_Notify_Access);

		if(NotifyDTTab[ClientId].NotifyFunction != NULL)
		{
			NotifyDTTab[ClientId].NotifyFunction = NULL;
			NotifyDTTab[ClientId].UpdateEvent = eCS_DT_NONE;
		}

		CSOS_SignalSemaphore(sem_DT_Notify_Access);

		eError = eCS_DT_NO_ERROR;
	}
	else
	{
		eError = eCS_DT_ERROR;
	}
	
	return(eError);

}

void DT_Notify(tCS_DT_UpdateEvent event, U16 local_mjd, U16 local_utc)
{
	U16	index;
	
	for(index= 0; index < kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS; index++ )
	{
		if(NotifyDTTab[index].NotifyFunction != NULL)
		{
			if( NotifyDTTab[index].UpdateEvent>= event )
			{
				(NotifyDTTab[index].NotifyFunction)(NotifyDTTab[index].UpdateEvent, local_mjd, local_utc);
			}				
		}
	}
	
	return; 
}

void DT_CorrectTimeByTDTTOT(U16  date_mjd, U16  time_utc)
{
	U32				result;
	tCS_DT_Info		*DThandle;
	BOOL			iftimechange = FALSE;
	struct timespec time_value;
	tCS_DT_Time		time_HM;
	tCS_DT_Date		date_YMD;
	struct tm		Temp_Time;

	CSOS_WaitSemaphore( sem_DateTimeAccess );
	DThandle = &sCS_DT_Info;

	if ( (date_mjd != DThandle->CS_DT_TDT_TOT_mjd ) || (time_utc != DThandle->CS_DT_TDT_TOT_utc) ) 
	{

		DThandle->CS_DT_TDT_TOT_mjd  = date_mjd;
		DThandle->CS_DT_TDT_TOT_utc = time_utc;

		// if(DThandle->CS_DT_AutoTimeEnable && CS_MW_GetTimeMode() != eCS_DBU_TIME_INTERNET )
		if(CS_MW_GetTimeMode() == eCS_DBU_TIME_AUTOMATIC )
		{
			iftimechange = TRUE;
			DThandle->CS_DT_Local_mjd= date_mjd;
			DThandle->CS_DT_Local_utc= time_utc;

			switch (DThandle->CS_DT_OffsetPolarity) 
			{
				case eCS_DT_OFFSET_POSITIVE:
					{
						result = CS_DT_UTC_Add(time_utc, DThandle->CS_DT_Offset_utc);
						DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
						result = result >> 24;
						if ( result >0) 
						{
							DThandle->CS_DT_Local_mjd += result;
						}
					}
					break;

				case eCS_DT_OFFSET_NEGATIVE:
					{
						result = CS_DT_UTC_Subtract(time_utc, DThandle->CS_DT_Offset_utc);
						DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
						result = result >> 24;
						if ( result >0) 
						{
							DThandle->CS_DT_Local_mjd -= result;
						}
					}
					break;

				default:
					{
						DThandle->CS_DT_Local_mjd = DThandle->CS_DT_TDT_TOT_mjd;
						DThandle->CS_DT_Local_utc = DThandle->CS_DT_TDT_TOT_utc;
					}
					break;
			}

			if (CS_MW_GetTimeRegion())
			{
				result = CS_DT_UTC_Add(DThandle->CS_DT_Local_utc , 0x0100);
				DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
				if ( result >> 24) 
				{
					DThandle->CS_DT_Local_mjd += result >> 24;
				}
			}
	

#if	1 /* For System Time Setting */
			date_YMD = CS_DT_MJDtoYMD(DThandle->CS_DT_Local_mjd);
			time_HM = CS_DT_UTCtoHM(DThandle->CS_DT_Local_utc);

			Temp_Time.tm_sec = 0;
			Temp_Time.tm_min = time_HM.minute;
			Temp_Time.tm_hour = time_HM.hour;
			Temp_Time.tm_mday = date_YMD.day;
			Temp_Time.tm_mon = date_YMD.month - 1;
			Temp_Time.tm_year = date_YMD.year - 1900;

			time_value.tv_sec = (time_t)mktime(&Temp_Time);
			time_value.tv_nsec = 0;

			// printf("set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);

			if (clock_settime(CLOCK_REALTIME, &time_value) < 0) {
				// printf("ERROR set time to %lu.%.9lu\n", time_value.tv_sec, time_value.tv_nsec);
				printf("clock_settime :: Error\n");
			}
#endif

			//SYS_SetBootFileDateTime(DThandle->DAT_Local_time,DThandle->DAT_Local_mjd);
			
		}

	}

	if(!DThandle->CS_DT_FoundTOTAndTDT)
	{
		DThandle->CS_DT_FoundTOTAndTDT = TRUE;
	}

	CSOS_SignalSemaphore( sem_DateTimeAccess);

	if(iftimechange)
	{
		DThandle->CS_DT_OSTime_LastUpdate = CS_OS_time_now();
		DT_Notify(eCS_DT_MINUTE, DThandle->CS_DT_Local_mjd, DThandle->CS_DT_Local_utc);

	}
}

/* 2010.09.04 By KB KIm For New SI */
/*
void DT_Filter_CallBack(void* userparam,
							DB_FilterHandle handle,
							BOOL timeout,
							U8 const* data,
							U32 size)
							*/
void DT_Filter_CallBack(U8 tableInfoId, U8 *data, U32 size)
{
	U8  *buffer;
	U16  date_mjd;
	U16  time_utc;
	U16  offset_utc = 0;
	tCS_DT_OffsetPolarity	polarity = eCS_DT_OFFSET_NEGATIVE;	
	U8 	Table_ID;

	// printf("TdtTotTableInfoId = %d, tableInfoId = %d, TableId : 0x%02X\n", TdtTotTableInfoId, tableInfoId, data[0]);
	if ((TdtTotTableInfoId < TABLE_INFO_NUMBER) && (tableInfoId != TdtTotTableInfoId))
	{
		return;
	}

	buffer = data;
	Table_ID = buffer[0];

	date_mjd = (U16 )((buffer[3] << 8) | (buffer[4]));
	time_utc = (U16)((buffer[5] << 8) | buffer[6] );

	//printf("Table_ID = 0x%x, time_utc = 0x%x\n", Table_ID, time_utc);

	if(date_mjd>=0xa000)
		date_mjd -= 0xa000;
	else
		date_mjd = 0;

	CSOS_WaitSemaphore( sem_DateTimeAccess );

#if 0 /* For EPG Time problem */
	if((Table_ID == SI_TOT_TABLE_ID) && (sCS_DT_Info.CS_DT_AutoOffsetEnable))
	{
		U32      region_id = 0;
		U16      descriptor_loop_length = ((buffer[8]&0x0f)<<4)|(buffer[9]);
		U16      NoOfLoopBytesParsed = 0;
		U8       descriptor_length = 0;
		U8       descriptor_tag = 0;

		region_id = CS_DBU_GetTimeRegion();
		//printf("kCS_SI_TBID_TOT region_id = %d, descriptor_loop_length = %d\n", region_id, descriptor_loop_length);

		buffer = data+10;

		while(NoOfLoopBytesParsed<descriptor_loop_length)
		{
			descriptor_tag = buffer[0];
			descriptor_length = buffer[1];

			if(descriptor_tag == LOCAL_TIME_OFFSET_DESCRIPTOR)
			{
				U8      total_loop = descriptor_length/13;  /*one loop length is 13*/
				U8      i;
				U8      desc_region;
				U8     *ptr_buffer = buffer+2;
				//printf("total_loop = %d\n", total_loop);
				for(i = 0; i<total_loop; i++)
				{
					desc_region = (ptr_buffer[3+i*13]&0xfc)>>2;

					//printf("desc_region = %d\n", desc_region);
					if(desc_region == region_id)
					{
						polarity = (ptr_buffer[3+i*13]&0x01);
						if(polarity == 0)
						{
							polarity = eCS_DT_OFFSET_POSITIVE;
						}
						else
						{
							polarity = eCS_DT_OFFSET_NEGATIVE;
						}

						offset_utc = (ptr_buffer[4+i*13]<<8)|(ptr_buffer[5+i*13]);
						//printf("region_id = %d, offset_utc = 0x%x\n", region_id, offset_utc);
						break;
					}
				}

				break;
			}

			NoOfLoopBytesParsed+=descriptor_length+2;
			buffer+=descriptor_length+2;
		}

		//offset_utc = 0;
		//polarity = eCS_DT_OFFSET_NEGATIVE;

		if((offset_utc != sCS_DT_Info.CS_DT_Offset_utc) ||(polarity != sCS_DT_Info.CS_DT_OffsetPolarity))
		{

			sCS_DT_Info.CS_DT_Offset_utc = offset_utc;
			sCS_DT_Info.CS_DT_OffsetPolarity = polarity;
			
		}
	}
#endif

	CSOS_SignalSemaphore( sem_DateTimeAccess);
	DT_CorrectTimeByTDTTOT(date_mjd, time_utc);	

	return;
}


void DT_UpdateTimeByOS(void)
{
	
	CS_OS_Clock_t nbOSTickPassed;
	CS_OS_Clock_t nbOSTickPerMinute = CS_OS_ConvertMstoTick(1000) * kNbSecondPerMinute;
	tCS_DT_Info *DThandle;
	S32  nbMinutePassed, nbMinute = 0, nbHour = 0;	
	tCS_DT_Time PassedTime_hms;
	U16 PassedTime_utc;
	U32  result;
	tCS_DT_UpdateEvent event = eCS_DT_NONE;

	CSOS_WaitSemaphore( sem_DateTimeAccess );
	DThandle = &sCS_DT_Info;
	
	nbOSTickPassed = CS_OS_time_minus( CS_OS_time_now() , DThandle->CS_DT_OSTime_LastUpdate);
	nbMinutePassed = nbOSTickPassed / nbOSTickPerMinute;
	DThandle->CS_DT_OSTime_LastUpdate = CS_OS_time_plus(DThandle->CS_DT_OSTime_LastUpdate, nbMinutePassed * nbOSTickPerMinute);
	
	nbHour = nbMinutePassed / kNbMinutePerHour;
	nbMinute = nbMinutePassed - kNbMinutePerHour * nbHour;

	PassedTime_hms.hour = nbHour;
	PassedTime_hms.minute = nbMinute;

	if(nbMinute > 0)
	{
		event = eCS_DT_MINUTE;
	}

	if(nbHour > 0)
	{
		event = eCS_DT_HOUR;
	}

	PassedTime_utc = CS_DT_HMtoUTC(PassedTime_hms);

	result = CS_DT_UTC_Add(DThandle->CS_DT_Local_utc, PassedTime_utc);     
	DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
	result = result >> 24;
	if ( result >0) 
	{
		DThandle->CS_DT_Local_mjd += result;
		event = eCS_DT_DAY;
	}
    
	CSOS_SignalSemaphore( sem_DateTimeAccess);

	if(event > eCS_DT_NONE)
	{
		DT_Notify(event, DThandle->CS_DT_Local_mjd, DThandle->CS_DT_Local_utc);
	}
	
}


void DT_IdleTask(void *pvParam)
{		
	CS_OS_Clock_t	nbOSTickNow;
	BOOL			job_time_reached = FALSE;

	while(TRUE)
	{	
		CSOS_DelayTaskMs(1000 * kNbSecondPerMinute/2);
		DT_UpdateTimeByOS();		

		CSOS_WaitSemaphore( sem_DateTimeAccess );

		if(Job_Begin_OsTime>0)
		{
			nbOSTickNow = CS_OS_time_now();
			if(nbOSTickNow >= Job_Begin_OsTime)
			{
				Job_Begin_OsTime = 0;
				job_time_reached = TRUE;
			}
			else
			{
				job_time_reached = FALSE;
			}
		}
		CSOS_SignalSemaphore( sem_DateTimeAccess);

		if((NotifyOsTimer != NULL)&&(job_time_reached))
		{
			NotifyOsTimer();

			NotifyOsTimer = NULL;
		}
	}		
}



BOOL CS_DT_Init(tCS_DT_Date  start_date, tCS_DT_Time start_time)
{
		
	U8 			index;
	
	
	sem_DateTimeAccess = CSOS_CreateSemaphoreFifo (NULL, 1 );
	sem_DT_Notify_Access = CSOS_CreateSemaphoreFifo (NULL, 1 );

	
	for(index=0;index<kCS_DT_MAX_NO_OF_NOTIFY_CLIENTS;index++)
	{
		NotifyDTTab[index].NotifyFunction = NULL;
		NotifyDTTab[index].UpdateEvent = eCS_DT_NONE;
	}

	sCS_DT_Info.CS_DT_TDT_TOT_mjd = 0;
	sCS_DT_Info.CS_DT_TDT_TOT_utc = 0;
	sCS_DT_Info.CS_DT_Local_mjd = CS_DT_YMDtoMJD(start_date);
	sCS_DT_Info.CS_DT_Local_utc = CS_DT_HMtoUTC(start_time);	
	sCS_DT_Info.CS_DT_Offset_utc= 0;
	sCS_DT_Info.CS_DT_OffsetPolarity = eCS_DT_OFFSET_NEGATIVE;	
	sCS_DT_Info.CS_DT_OSTime_LastUpdate = CS_OS_time_now();
	sCS_DT_Info.CS_DT_AutoTimeEnable = TRUE;
	sCS_DT_Info.CS_DT_AutoOffsetEnable = FALSE;
	sCS_DT_Info.CS_DT_FoundTOTAndTDT= FALSE;

	if (CSOS_CreateTask(DT_IdleTask,					/* thread entry point */
						NULL, 						/* entry point argument */
						NULL,
						DT_IDLE_STACK_SIZE,				/* size of stack in bytes */
						DT_IDLE_STACK, 				/* pointer to stack base */
						NULL,
						&DT_IDLE_TASK_HANDLE,			/* return thread handle */
						&DT_IDLE_TASK_DESC, 			/* space to store thread data */ 
						DT_IDLE_TASK_PRIORITY,
						"date_time_task", 					/* name of thread */
						DT_IDLE_TASK_FLAG) != CS_NO_ERROR)
	{
		printf ( "Failed to create the ssd task \n" );
		return FALSE;
	}	
	else
	{
		CSOS_StartTask(DT_IDLE_TASK_HANDLE);
	}

	/* 2010.09.04 By KB KIm For New SI */
	TdtTotTableInfoId = TABLE_INFO_NUMBER;
	
	return TRUE;
}

/* 2010.09.04 By KB KIm For New SI */
tCS_DT_Error	CS_DT_Stop(void)
{
	if (TdtTotTableInfoId < TABLE_INFO_NUMBER)
	{
		SiStopLiveSection(TdtTotTableInfoId);
		TdtTotTableInfoId = TABLE_INFO_NUMBER;
	}

	return eCS_DT_NO_ERROR;
}

tCS_DT_Error	CS_DT_Start(void)
{
	BOOL error;
	U8   match[8];
	U8   mask[8];

	CS_DT_Stop();

	memset(match,0x00,8);
	memset(mask,0x00,8);

	match[0] = SI_TDT_TABLE_ID;
	mask[0]  = 0xFC;
	
	error = SiStartLiveSection(&TdtTotTableInfoId,
		                        TABLE_TDT_TOT_PID,
		                        SI_TDT_TABLE_ID,
		                        0,
		                        0,
		                        1,
		                        0,
		                        match,
		                        mask,
		                        NULL,
		                        DT_Filter_CallBack);
	
	if ((error) || (TdtTotTableInfoId >= TABLE_INFO_NUMBER))
	{
		return eCS_DT_ERROR;
	}

	return eCS_DT_NO_ERROR;
}

void CS_DT_ManualSetUTCOffset(U16  utc_offset, tCS_DT_OffsetPolarity utc_offset_type)
{
	U32            	result;
	tCS_DT_Info *DThandle;
	BOOL	need_notify = FALSE;

	CSOS_WaitSemaphore( sem_DateTimeAccess );
	DThandle = &sCS_DT_Info;

	// if(((utc_offset != DThandle->CS_DT_Offset_utc)||(utc_offset_type != DThandle->CS_DT_OffsetPolarity))&& (!DThandle->CS_DT_AutoOffsetEnable) )
	if(((utc_offset != DThandle->CS_DT_Offset_utc)||(utc_offset_type != DThandle->CS_DT_OffsetPolarity)))
	{
		DThandle->CS_DT_Offset_utc = utc_offset;
		DThandle->CS_DT_OffsetPolarity = utc_offset_type;	

		// if(DThandle->CS_DT_AutoTimeEnable && DThandle->CS_DT_FoundTOTAndTDT)
		{

			DThandle->CS_DT_Local_mjd = DThandle->CS_DT_TDT_TOT_mjd;
			switch (DThandle->CS_DT_OffsetPolarity) 
			{
				case eCS_DT_OFFSET_POSITIVE:
				{
					result = CS_DT_UTC_Add(DThandle->CS_DT_TDT_TOT_utc, DThandle->CS_DT_Offset_utc);
					DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
					result = result >> 24;
					if ( result >0) 
					{
						DThandle->CS_DT_Local_mjd += result;
					}
				}break;

				case eCS_DT_OFFSET_NEGATIVE:
				{
					result = CS_DT_UTC_Subtract(DThandle->CS_DT_TDT_TOT_utc, DThandle->CS_DT_Offset_utc);
					DThandle->CS_DT_Local_utc = (U16)(result&0x0000FFFF);
					result = result >> 24;
					if ( result >0) 
					{
						DThandle->CS_DT_Local_mjd -= result;
					}
				}break;

				default:
				{
					DThandle->CS_DT_Local_utc= DThandle->CS_DT_TDT_TOT_utc;
					DThandle->CS_DT_Local_mjd= DThandle->CS_DT_TDT_TOT_mjd;
					
				}break;
			}

			DThandle->CS_DT_OSTime_LastUpdate = CS_OS_time_now();
			need_notify = TRUE;
		}
	}

	CSOS_SignalSemaphore( sem_DateTimeAccess);

	if(need_notify == TRUE)
		DT_Notify(eCS_DT_MANUAL, DThandle->CS_DT_Local_mjd, DThandle->CS_DT_Local_utc);
}

void CS_DT_GetUTCOffset(U16  *utc_offset, tCS_DT_OffsetPolarity *utc_offset_type)
{

	CSOS_WaitSemaphore( sem_DateTimeAccess );
	
	if (utc_offset != NULL) 
	{
		*utc_offset = sCS_DT_Info.CS_DT_Offset_utc;
	}

	if (utc_offset_type != NULL) 
	{
		*utc_offset_type = sCS_DT_Info.CS_DT_OffsetPolarity;
	}
	CSOS_SignalSemaphore( sem_DateTimeAccess);
}

void CS_DT_ManualSetDateAndTime(U16 mjd, U16 utc)
{
	tCS_DT_Info *DThandle;
	BOOL		need_notify = FALSE;

	CSOS_WaitSemaphore( sem_DateTimeAccess );
	DThandle = &sCS_DT_Info;

	if ( (mjd != DThandle->CS_DT_Local_mjd) || (utc != DThandle->CS_DT_Local_utc) ) 
	{
		if(!DThandle->CS_DT_AutoTimeEnable)
		{
			DThandle->CS_DT_Local_mjd = mjd;
			DThandle->CS_DT_Local_utc = utc;
			
			//SYS_SetBootFileDateTime(DThandle->DAT_Local_time,DThandle->DAT_Local_mjd);

			DThandle->CS_DT_OSTime_LastUpdate = CS_OS_time_now();
			need_notify = TRUE;			
		}

	}

	CSOS_SignalSemaphore( sem_DateTimeAccess);
	
	if(need_notify == TRUE)
		DT_Notify(eCS_DT_MANUAL, DThandle->CS_DT_Local_mjd, DThandle->CS_DT_Local_utc);
}


U16 CS_DT_Get_TDTTOT_UTC( void )
{
	U16 utc = 0;
	CSOS_WaitSemaphore( sem_DateTimeAccess );
	utc =  sCS_DT_Info.CS_DT_TDT_TOT_utc;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
	return utc;
}

U16 CS_DT_Get_TDTTOT_MJD( void )
{
	U16 mjd = 0;
	CSOS_WaitSemaphore( sem_DateTimeAccess );		
	mjd = sCS_DT_Info.CS_DT_TDT_TOT_mjd;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
	return mjd;
}

U16 CS_DT_GetLocalUTC( void )
{
	U16 utc = 0;
	CSOS_WaitSemaphore( sem_DateTimeAccess );
	utc =  sCS_DT_Info.CS_DT_Local_utc;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
	return utc;
}

U16 CS_DT_GetLocalMJD( void )
{
	U16 mjd = 0;
	CSOS_WaitSemaphore( sem_DateTimeAccess );
	mjd =  sCS_DT_Info.CS_DT_Local_mjd;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
	return mjd;
}

void  CS_DT_EnableAutomaticTime(BOOL  enable)
{
	CSOS_WaitSemaphore( sem_DateTimeAccess );
	sCS_DT_Info.CS_DT_AutoTimeEnable = enable;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
}

void  CS_DT_EnableAutomaticOffset(BOOL  enable)
{
	CSOS_WaitSemaphore( sem_DateTimeAccess );
	sCS_DT_Info.CS_DT_AutoOffsetEnable = enable;
	CSOS_SignalSemaphore( sem_DateTimeAccess);
}


U8  CS_DT_CalculateWeekDay(U16  mjd)
{
        U32     temp;

        temp = mjd + 0xa000;
        
	return ( ( temp + 2 ) % 7+1);
}

void  CS_DT_Caculate_EPG_Localtime(U16 *date_mjd,U16 *time_utc)
{
	U16	mjd_offset;
	U16	utc_offset;
	U32  result = 0;
	tCS_DT_OffsetPolarity 	offset_type;
	tCS_DT_Info 	Current_DTInfo;
	
	CSOS_WaitSemaphore( sem_DateTimeAccess );		
	Current_DTInfo = sCS_DT_Info;
	CSOS_SignalSemaphore( sem_DateTimeAccess);

#if 0
	if(Current_DTInfo.CS_DT_AutoTimeEnable)
	{		
		offset_type = Current_DTInfo.CS_DT_OffsetPolarity;
		mjd_offset = 0 ;
		utc_offset  = Current_DTInfo.CS_DT_Offset_utc;
	}
	else
	{
		if(!Current_DTInfo.CS_DT_FoundTOTAndTDT)
		{
			mjd_offset = result = 0;
			offset_type = eCS_DT_OFFSET_NEGATIVE;
		}
		else
		if(Current_DTInfo.CS_DT_TDT_TOT_mjd > Current_DTInfo.CS_DT_Local_mjd)
		{

			mjd_offset = Current_DTInfo.CS_DT_TDT_TOT_mjd - Current_DTInfo.CS_DT_Local_mjd; 		
			offset_type = eCS_DT_OFFSET_NEGATIVE;
			result =  CS_DT_UTC_Subtract(Current_DTInfo.CS_DT_TDT_TOT_utc, Current_DTInfo.CS_DT_Local_utc);
			if (result >> 24) 
				(mjd_offset)-=result >> 24;
		}
		else
		if(Current_DTInfo.CS_DT_TDT_TOT_mjd < Current_DTInfo.CS_DT_Local_mjd)
		{
			mjd_offset = Current_DTInfo.CS_DT_Local_mjd - Current_DTInfo.CS_DT_TDT_TOT_mjd; 	
			offset_type = eCS_DT_OFFSET_POSITIVE;
			result =  CS_DT_UTC_Subtract(Current_DTInfo.CS_DT_Local_utc, Current_DTInfo.CS_DT_TDT_TOT_utc);
			if (result >> 24) 
				(mjd_offset)-=result >> 24;
		}
		else
		{
			mjd_offset = 0;
			if (Current_DTInfo.CS_DT_TDT_TOT_utc > Current_DTInfo.CS_DT_Local_utc)
			{
				offset_type = eCS_DT_OFFSET_NEGATIVE;
				result	 = CS_DT_UTC_Subtract( Current_DTInfo.CS_DT_TDT_TOT_utc , Current_DTInfo.CS_DT_Local_utc);
			}
			else 
			{
				offset_type = eCS_DT_OFFSET_POSITIVE;
				result	 = CS_DT_UTC_Subtract(Current_DTInfo.CS_DT_Local_utc, Current_DTInfo.CS_DT_TDT_TOT_utc);
			}
		}
		utc_offset = (U16)result;	
	}
#else
	offset_type = Current_DTInfo.CS_DT_OffsetPolarity;
	mjd_offset = 0 ;
	utc_offset  = Current_DTInfo.CS_DT_Offset_utc;
#endif

	switch (offset_type) 
	{
		case eCS_DT_OFFSET_POSITIVE:
		{
			*date_mjd += mjd_offset;			
			result = CS_DT_UTC_Add(*time_utc , utc_offset);
			if ( result >> 24) 
			{
				(*date_mjd)+=result >> 24;
			}
		}
		break;

		case eCS_DT_OFFSET_NEGATIVE:
		{
			*date_mjd -= mjd_offset;	
			result = CS_DT_UTC_Subtract(*time_utc, utc_offset);			
			if ( result >> 24) 
			{
				(*date_mjd)-=result >> 24;
			}
		}
		break;			
	}
	*time_utc  = (U16)(result&0x0000FFFF);

	if (CS_MW_GetTimeRegion())
	{
		result = CS_DT_UTC_Add(*time_utc , 0x0100);
		*time_utc  = (U16)(result&0x0000FFFF);
		if ( result >> 24) 
		{
			(*date_mjd)+=result >> 24;
		}
	}
	
        if(*date_mjd>=0xa000)
            *date_mjd -= 0xa000;
        else
            *date_mjd = 0;
	
}

void CS_DT_SetOsTimerJob(U32 minute, tCS_DT_OsTimerNotify NotifyFunction)
{
    CS_OS_Clock_t nbOSTickNow;
    CS_OS_Clock_t nbOSTickPerMinute = CS_OS_ConvertMstoTick(1000) * kNbSecondPerMinute;

    CSOS_WaitSemaphore( sem_DateTimeAccess );

    nbOSTickNow = CS_OS_time_now();
    Job_Begin_OsTime = CS_OS_time_plus(nbOSTickNow, minute * nbOSTickPerMinute);
    NotifyOsTimer = NotifyFunction;

    CSOS_SignalSemaphore( sem_DateTimeAccess);
    
}

void CS_DT_ClearOsTimerJob(void)
{

    CSOS_WaitSemaphore( sem_DateTimeAccess );

    Job_Begin_OsTime = 0;
    NotifyOsTimer = NULL;

    CSOS_SignalSemaphore( sem_DateTimeAccess);
}

BOOL CS_DT_GetTDTTOTFoundStatus(void)
{
    BOOL status;
    
    CSOS_WaitSemaphore( sem_DateTimeAccess );
    
    status = sCS_DT_Info.CS_DT_FoundTOTAndTDT;
        
    CSOS_SignalSemaphore( sem_DateTimeAccess);

    return(status);
}


