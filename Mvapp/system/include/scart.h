/****************************************************************
*
* FILENAME
*	scart.h
*
* PURPOSE 
*	Common Header file for Scart Control APIs
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2010.08.31           KB
*
****************************************************************/
#ifndef SCART_DRIVER_H
#define SCART_DRIVER_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include "mvosapi.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/

/****************************************************************
 *                       Type define                            *
 ****************************************************************/

/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
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
extern BOOL ScartFbOnOff(U8 onFB);

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
extern BOOL ScartAspecChange(U8 on16_9);

/*
 *  Name : ScartSbOnOff
 *  Description
 *     Scart Slow Blank ON & Off.
 *  INPUT Arg
 *     U8 sbOn                    : Slow Balnk ON  value (1 : ON, 0 : Off)
 *  OUTPUT Arg
 *     NONE
 *  RETURN : BOOL
 *     FALSE   : No Error
 *     TRUE    : Slow Blank Set Error
 */
extern BOOL ScartSbOnOff(void);

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
extern BOOL ScartSbControl(U8 onScart);

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
extern BOOL ScartInit(void);

#endif // #ifdef SCART_DRIVER_H
