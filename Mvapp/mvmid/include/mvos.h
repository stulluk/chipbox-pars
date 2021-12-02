/****************************************************************
*
* FILENAME
*	mvos.h
*
* PURPOSE 
*	Header for OS Functions
*
* AUTHOR
*	KB Kim
*
* HISTORY
*  Status                            Date              Author
*  Create                         2009.12.26           KB
*
****************************************************************/
#ifndef MV_OS_H
#define MV_OS_H

/****************************************************************
 *                       Include files                          *
 ****************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "linuxdefs.h"

/****************************************************************
*	                    Define Values                           *
*****************************************************************/
//#define DEBUG_ON

#define OS_RETURN_ERROR   1
#define OS_NO_ERROR       0

#define TIMEOUT_IMMEDIATE      0
#define TIMEOUT_FOREVER        0xFFFFFFFF
#define NULL_ID                0xFFFFFFFF

#define MAX_MESSAGE_QUEUE_NAME 50

#ifdef DEBUG_ON
	// #define OsDebugPrintf    printf
	#define OsDebugPrtf      printf
#endif // #ifdef DEBUG_ON

/****************************************************************
 *                       Type define                            *
 ****************************************************************/
#if 0

#ifndef U32
typedef unsigned int U32;
#endif
#ifndef U16
typedef unsigned short U16;
#endif
#ifndef U8
typedef unsigned char U8;
#endif
#ifndef S32
typedef int S32;
#endif
#ifndef S16
typedef short S16;
#endif
#ifndef S8
typedef signed char S8;
#endif
#ifndef F32
typedef float F32;
#endif
#ifndef F64
typedef double F64;
#endif
#ifndef BOOL
typedef U8 BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#endif
/****************************************************************
 *                      Global Variable                         *
 ****************************************************************/

/****************************************************************
 *                      Extern Variable                         *
 ****************************************************************/

/****************************************************************
 *                     Function Prototype                       *
 ****************************************************************/

#endif // #ifndef MV_OS_H

