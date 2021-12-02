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
#include "mvosapi.h"
#include "mvmiscapi.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
#ifdef DEBUG_ON
	// #define SCART_DEBUG_ON
#endif

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
#define SB_GPIO_PORT     0
#define SB_GPIO_BIT      3  /* Changed From JT LPP Board By KB Kim 2010.11.15 */
#define ASPEC_GPIO_PORT  1
#define ASPEC_GPIO_BIT   47
#define FB_GPIO_PORT     1
#define FB_GPIO_BIT      48

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/
U32 ScartSbGpioId    = 0;
U32 ScartAspecGpioId = 0;
U32 ScartFbGpioId    = 0;
U8  CurrentSb;
U8  CurrentAspec;
U8  CurrentFb;

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

/****************************************************************
 *                         Functions                            *
 ****************************************************************/
/*
 *  Name : ScartAspecChange
 *  Description
 *     Scart Fast Blank ON  / OFF to control RGB.
 *  INPUT Arg
 *     U8 onFB                    : FB control (1 : FB ON(RGB mode), 0 : FB Off)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : FB Set Error
 */
BOOL ScartFbOnOff(U8 onFB)
{
	if (ScartFbGpioId == 0)
	{
		return TRUE;
	}

	if (CurrentFb != onFB)
	{
		if (GpioPortWrite(ScartFbGpioId, onFB) != OS_NO_ERROR)
		{
#ifdef SCART_DEBUG_ON
			OsDebugPrtf("ScartInit Error : Cannot Write ScartFbGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
			return TRUE;
		}
		CurrentFb = onFB;
	}

	return FALSE;
}

/*
 *  Name : ScartAspecChange
 *  Description
 *     Scart Slow Blank change 4:3 and 16:9.
 *  INPUT Arg
 *     U8 on16_9                  : Aspec value (1 : 16:9, 0 : 4:3)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Aspec Set Error
 */
BOOL ScartAspecChange(U8 on16_9)
{
	if (ScartAspecGpioId == 0)
	{
		return TRUE;
	}

	if (CurrentAspec != on16_9)
	{
		if (GpioPortWrite(ScartAspecGpioId, on16_9) != OS_NO_ERROR)
		{
#ifdef SCART_DEBUG_ON
			OsDebugPrtf("ScartInit Error : Cannot Write ScartAspecGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
			return TRUE;
		}
		CurrentAspec = on16_9;
	}

	return FALSE;
}

/*
 *  Name : ScartSbOnOff
 *  Description
 *     Scart Slow Blank ON & Off Toggle.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Slow Blank Set Error
 */
BOOL ScartSbOnOff(void)
{
	if (ScartSbGpioId == 0)
	{
		return TRUE;
	}

	if (GpioPortWrite(ScartSbGpioId, CurrentSb) != OS_NO_ERROR)
	{
#ifdef SCART_DEBUG_ON
		OsDebugPrtf("ScartInit Error : Cannot Write ScartSbGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
		return TRUE;
	}
	CurrentSb = 1 - CurrentSb;

	return FALSE;
}

/* For Scart OFF in Sleep mode by KB Kim 2012.04.07 */
/*
 *  Name : ScartSbControl
 *  Description
 *     Scart Slow Blank ON & Off control.
 *  INPUT Arg
 *     U8 onScart                 : Scart Control (1 : ON, 0 : OFF)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Slow Blank Set Error
 */
BOOL ScartSbControl(U8 onScart)
{
	if (onScart != 0)
	{
		CurrentSb = 0; /* SB On */
	}
	else
	{
		CurrentSb = 1; /* SB Off */
	}

	return ScartSbOnOff();
}

/*
 *  Name : ScartInit
 *  Description
 *     Init Scart Driver.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Init Error
 */
BOOL ScartInit(void)
{
	GpioOpenParam_t openParam;
	BOOL            result;

#ifdef SCART_DEBUG_ON
	OsDebugPrtf("ScartInit : Start\n");
#endif // #ifdef SCART_DEBUG_ON
	openParam.Mode       = GPIO_WRITE_MODE;
	openParam.Value      = 1;
	
	openParam.PortNumber = SB_GPIO_PORT;
	openParam.BitNumber  = SB_GPIO_BIT;
	
	result = GpioPortOpen(&ScartSbGpioId, &openParam);
	if((result != OS_NO_ERROR) || (ScartSbGpioId == 0))
	{
		/* GPIO Device Open Error */
#ifdef SCART_DEBUG_ON
		OsDebugPrtf("ScartInit Error : Cannot open ScartSbGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
		// return TRUE;
	}

	openParam.PortNumber = ASPEC_GPIO_PORT;
	openParam.BitNumber  = ASPEC_GPIO_BIT;
	
	result = GpioPortOpen(&ScartAspecGpioId, &openParam);
	if((result != OS_NO_ERROR) || (ScartAspecGpioId == 0))
	{
		/* GPIO Device Open Error */
#ifdef SCART_DEBUG_ON
		OsDebugPrtf("ScartInit Error : Cannot open ScartAspecGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
		return TRUE;
	}

	openParam.PortNumber = FB_GPIO_PORT;
	openParam.BitNumber  = FB_GPIO_BIT;
	
	result = GpioPortOpen(&ScartFbGpioId, &openParam);
	if((result != OS_NO_ERROR) || (ScartFbGpioId == 0))
	{
		/* GPIO Device Open Error */
#ifdef SCART_DEBUG_ON
		OsDebugPrtf("ScartInit Error : Cannot open ScartFbGpioId GPIO Device\n");
#endif // #ifdef SCART_DEBUG_ON
		return TRUE;
	}

	CurrentSb    = 0; /* SB On */
	CurrentAspec = 0xFF;
	CurrentFb    = 0xFF;
	
	ScartSbOnOff(); /* Set SB On */
	
	return FALSE;
}

/*
 *  Name : ScartInit
 *  Description
 *     Terminate Scart Driver.
 *  INPUT Arg
 *     NONE
 *  OUTPUT Arg
 *     NONE
 *  RETURN : void
 *     NONE
 */
void ScartTerm(void)
{
	if (ScartSbGpioId)
	{
		GpioPortClose(ScartSbGpioId);
	}
	if (ScartAspecGpioId)
	{
		GpioPortClose(ScartAspecGpioId);
	}
	if (ScartFbGpioId)
	{
		GpioPortClose(ScartFbGpioId);
	}
}

