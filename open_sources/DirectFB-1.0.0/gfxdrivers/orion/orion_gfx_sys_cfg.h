/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : host_cfg.h
// MODULE NAME      : host App and Test Case
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-23
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-23   jiasheng Chen           Original
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Host Sys Basic Infomation Cofiguration
//   exp: Memory Endian Mode 
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/
#ifndef _HOST_SYS_CFG_H_
#define _HOST_SYS_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HOST_MEM_BYTE_LITTLE_ENDIAN
#define HOST_MEM_BYTE_LITTLE_ENDIAN
	#ifdef HOST_MEM_BYTE_BIG_ENDIAN
	#undef HOST_MEM_BYTE_BIG_ENDIAN
	#endif
#endif

/*
#ifndef HOST_MEM_BYTE_BIG_ENDIAN
#define HOST_MEM_BYTE_BIG_ENDIAN
	#ifdef HOST_MEM_BYTE_LITTLE_ENDIAN
	#undef HOST_MEM_BYTE_LITTLE_ENDIAN
	#endif
gfx_aux.c#endif
*/

/*
#ifndef HOST_MEM_NIBBLE_LITTLE_ENDIAN
#define HOST_MEM_NIBBLE_LITTLE_ENDIAN
	#ifdef HOST_MEM_NIBBLE_BIG_ENDIAN
	#undef HOST_MEM_NIBBLE_BIG_ENDIAN
	#endif
#endif
*/
#ifndef HOST_MEM_NIBBLE_BIG_ENDIAN
#define HOST_MEM_NIBBLE_BIG_ENDIAN
	#ifdef HOST_MEM_NIBBLE_LITTLE_ENDIAN
	#undef HOST_MEM_NIBBLE_LITTLE_ENDIAN
	#endif
#endif

#define ORION_130    130
#define ORION_131    131
#define ORION_140    140

#define ORION_VERSION ORION_131 //131, 140 



#ifdef __cplusplus
	}
#endif

#endif
