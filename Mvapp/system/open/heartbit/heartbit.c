/****************************************************************
*
* FILENAME
*	scart.c
*
* PURPOSE 
*	Scart control driver source
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.31           KB
*
****************************************************************/

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linuxdefs.h"
#include "database.h"
#include "mvosapi.h"
#include "mvmiscapi.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#ifdef DEBUG_ON
	// #define HEART_DEBUG_ON
#endif

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
#define HEART_GPIO_PORT  0
#define HEART_GPIO_BIT   1

#define HEART_DURATION   500

//#define W_D_TEST_ON

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/
U32 HeartGpioId     = 0;
U32 HeartBitTaskId  = 0;
// U8  HeartBitOn      = 0;

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/
extern BOOL FbStartWatchdog(U8 time);
extern BOOL FbStopWatchdog(void);
extern BOOL FbSendKick(void);

/****************************************************************
 *                         Functions                            *
 ****************************************************************/
/*
 *  Name : HeartBitTask
 *  Description
 *     Heart Bit LED Blink Task.
 *  INPUT Arg
 *     U8 onFB                    : FB control (1 : FB ON(RGB mode), 0 : FB Off)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : FB Set Error
 */
void HeartBitTask(void *param)
{
	U32 heartBitId;
	U32 wait;
	U32 nextTime;
	U32 currentTime;
	U8  bitOn;
	U8  bitVal;

	heartBitId  = *(U32 *)param;
	wait        =  HEART_DURATION; /* 500 msec */
	currentTime =  OsTimeNowMilli();
	nextTime    =  currentTime + HEART_DURATION;
	bitOn       = 0;

	#ifdef W_D_TEST_ON
	FbStartWatchdog(3); /*  For Test By KB Kim */
	#endif

	while(1)
	{
		OsWaitMillisecond(wait);
		currentTime =  OsTimeNowMilli();
		nextTime    = nextTime + HEART_DURATION;
		wait        = OsTimeDiff(nextTime, currentTime);
		if (wait > (HEART_DURATION + 100))
		{
			nextTime = currentTime + HEART_DURATION;
			wait = HEART_DURATION;
		}
		
#ifdef W_D_TEST_ON
		FbSendKick(); /*  For Test By KB Kim */
#endif
	
		if (HeartGpioId != 0)
		{
			bitVal = bitOn & CS_DBU_GetHeartBit() & (U8)CS_DBU_GetPower_Off_Mode();
			
			if (GpioPortWrite(HeartGpioId, bitVal) == OS_NO_ERROR)
			{
				bitOn = 1- bitOn;
			}
#ifdef HEART_DEBUG_ON
			else
			{
				OsDebugPrtf("HeartBitTask Error : Cannot Write HeartGpioId GPIO Device\n");
			}
#endif // #ifdef HEART_DEBUG_ON
		}
	}
}

/*
 *  Name : HeartBitInit
 *  Description
 *     Init Heart Bit Driver.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Init Error
 */
BOOL HeartBitInit(void)
{
	GpioOpenParam_t openParam;
	BOOL            result;

	openParam.Mode       = GPIO_WRITE_MODE;
	openParam.Value      = 1;
	openParam.PortNumber = HEART_GPIO_PORT;
	openParam.BitNumber  = HEART_GPIO_BIT;
	
	result = GpioPortOpen(&HeartGpioId, &openParam);
	if((result != OS_NO_ERROR) || (HeartGpioId == 0))
	{
		/* GPIO Device Open Error */
#ifdef HEART_DEBUG_ON
		OsDebugPrtf("HeartBitInit Error : Cannot open HeartGpioId GPIO Device\n");
#endif // #ifdef HEART_DEBUG_ON
		return TRUE;
	}

	// HeartBitOn = 1;

	result = OsCreateTask(&HeartBitTaskId,
		                  "HeartBitTask",
		                  HeartBitTask,
		                  (void *)&HeartGpioId,
		                  10,
		                  1024);
	if ((result != OS_NO_ERROR) || (HeartBitTaskId == 0))
	{
		/* We want to continue without Skew */
#ifdef HEART_DEBUG_ON
		OsDebugPrtf("HeartBitInit Error : Cannot Create HeartBitTask\n");
#endif // #ifdef HEART_DEBUG_ON
	}

	return FALSE;
}

/*
 *  Name : HeartBitTerm
 *  Description
 *     Terminate Heart Bit Driver.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void
 *     NONE
 */
void HeartBitTerm(void)
{
	if (HeartBitTaskId)
	{
		OsDeleteTask(HeartBitTaskId);
	}
	
	if (HeartGpioId)
	{
		GpioPortClose(HeartGpioId);
	}
}

