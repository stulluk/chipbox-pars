/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_type_def.c
// MODULE NAME      : basic Logsystem Libary
// AUTHOR           : Jiasheng Chen
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-10-25
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.2      06-10-25   jiasheng Chen           Original
// 0.2.1    07-05-14   Jiasheng Chen           Modified to support DirectFB
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// define basic type used for different platform migration
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#ifndef _ORION_GFX_TYPE_DEF_
#define _ORION_GFX_TYPE_DEF_

typedef unsigned int        UINT32;
typedef unsigned char       UINT8;
typedef unsigned char       BYTE;
typedef unsigned int        DWORD;
typedef int                 BOOL;

typedef unsigned char           U8;
typedef signed char             I8;
typedef unsigned short         U16;
typedef signed short           I16;
typedef unsigned int           U32;
typedef signed int             I32;

#ifdef WIN32
typedef unsigned __int64       U64;
typedef   signed __int64       I64;
#else //GCC
typedef unsigned long long int U64;
typedef long long int          I64;
#endif
//For Boolean Type please using I32 as Standard C

#endif
