#ifndef _HDMITX_H_
#define _HDMITX_H_


#define SUPPORT_EDID
#define SUPPORT_HDCP
//#define SUPPORT_INPUTRGB
//#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
#define SUPPORT_SYNCEMB

//#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
#define SUPPORT_INPUTYUV
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include "rtdefs.h"
#include "typedef.h"
#include "cat6611_sys.h"
#include "cat6611_drv.h"
#include "edid.h"



//////////////////////////////////////////////////////////////////////
// Function Prototype
//////////////////////////////////////////////////////////////////////


// dump
#ifdef DEBUG
void DumpCat6611Reg() ;
#endif
// I2C

//////////////////////////////////////////////////////////////////////////////
// main.c
//////////////////////////////////////////////////////////////////////////////

#endif // _HDMITX_H_

