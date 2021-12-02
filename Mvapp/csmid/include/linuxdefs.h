/*******************************************************************************

File name   : linuxdefs.h

Description : Linux Basic Define 
Copyright (C) 2007 Celestial

References  :

create 	2007.09.26			: Pinanhai - weiyu.Luo  :
 
*******************************************************************************/

/* Define to prevent recursive inclusion */
#ifndef __LINUXDEF_H
#define __LINUXDEF_H


/* Includes ---------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <minigui/common.h>

/* Exported Constants ------------------------------------------------------ */

/* Common driver error constants */
#define CS_DRIVER_ID   0				/* : LS : IS NOT A driver */
#define CS_DRIVER_BASE (CS_DRIVER_ID << 16)

enum
{
	CS_NO_ERROR = CS_DRIVER_BASE,
	CS_ERROR_BAD_PARAMETER,             /* Bad parameter passed       */
	CS_ERROR_NO_MEMORY,                 /* Memory allocation failed   */
	CS_ERROR_UNKNOWN_DEVICE,            /* Unknown device name        */
	CS_ERROR_ALREADY_INITIALIZED,       /* Device already initialized */
	CS_ERROR_NO_FREE_HANDLES,           /* Cannot open device again   */
	CS_ERROR_OPEN_HANDLE,               /* At least one open handle   */
	CS_ERROR_INVALID_HANDLE,            /* Handle is not valid        */
	CS_ERROR_FEATURE_NOT_SUPPORTED,     /* Feature unavailable        */
	CS_ERROR_INTERRUPT_INSTALL,         /* Interrupt install failed   */
	CS_ERROR_INTERRUPT_UNINSTALL,       /* Interrupt uninstall failed */
	CS_ERROR_INTERRUPT_ENABLE,
	CS_ERROR_INTERRUPT_DISABLE,
	CS_ERROR_TIMEOUT,                   /* Timeout occured            */
	CS_ERROR_FAILD_GET_SEMAPHORE,
	CS_ERROR_FAILD_LOCK_MUTEX,
	CS_ERROR_DEVICE_BUSY,               /* Device is currently busy   */
	CS_ERROR_MESSAGE_SEND,
	CS_ERROR_MESSAGE_CLOSE,
	CS_ERROR_FAIL_SETREG,
	CS_ERROR_ABORT,
	CS_ERROR_END
		
};

typedef int			            INT;
//typedef unsigned int			UINT;
typedef unsigned long		    ULONG;
typedef unsigned short int		USHORT;	

//typedef unsigned char			BYTE;
typedef unsigned char			UINT8;
typedef unsigned short			UINT16;
typedef unsigned int			UINT32;
typedef signed char			    INT8;
typedef signed short			INT16;
typedef signed int			    INT32;
typedef signed				    SIGN;
typedef unsigned			    UNSIGN;

typedef unsigned char			INTU8;
typedef unsigned char 	*		PINTU8;

typedef signed char			    INT8S;
typedef signed char 	*		PINT8S;
 
typedef unsigned short			INT16U;
typedef unsigned short 	*		PINT16U;

//typedef signed short			INT16S;
typedef signed short 	*		PINT16S;

typedef unsigned int			INT32U;
typedef unsigned int 	*		PINT32U;
 
//typedef signed int			    INT32S;
typedef signed int 	*		    PINT32S;

typedef unsigned char			U8;
typedef unsigned char 	*		PU8;

typedef signed char			    S8;
typedef signed char 	*		PS8;
 
typedef unsigned short			U16;
typedef unsigned short	*		PU16;

typedef signed short			S16;
typedef signed short	*		PS16;

typedef unsigned int			U32;
typedef unsigned int 	*		PU32;
 
typedef signed int			    S32;
typedef signed int 	*		    PS32;

typedef float				    FP32;
typedef float 		*		    PFP32;
 
typedef double				    FP64;
typedef double 		*		    PFP64;

//typedef void				    VOID;
typedef void 		*		    PVOID;

//typedef S32				        BOOL;

//#define UBYTE				    INT8U

#define TRUE				    1
#define FALSE				    0

/* device access type */
/*
typedef	*(volatile unsigned int *) VPint32_t;
typedef	*(volatile unsigned short *) VPint16_t;
typedef	*(volatile unsigned char *) VPint8_t;
*/

/* Function return error code */
typedef U32 CS_ErrorCode_t;


/* General purpose string type */
typedef char	*CS_String_t;

/* Revision structure */
typedef const char *CS_Revision_t;

/* Device name type */
#define CS_MAX_DEVICE_NAME 24  /* 16->24 :  15 characters plus '\0' */
typedef char CS_DeviceName_t[CS_MAX_DEVICE_NAME];

/* Generic partition type */


/* Exported Variables ------------------------------------------------------ */


/* Exported Macros --------------------------------------------------------- */


/* Exported Functions ------------------------------------------------------ */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef __STDDEFS_H */

/* End of stddefs.h  ------------------------------------------------------- */


