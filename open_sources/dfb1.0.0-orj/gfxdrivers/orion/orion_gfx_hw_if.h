/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_hw_if.h
// MODULE NAME      : Gfx Engine Host Interface
// AUTHOR           : Jiasheng Chen
// AUTHOR'S EMAIL   : jschen@celestialsemi.com
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-01
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-01   jiasheng Chen           Original
// 0.2      07-05-14   Jiasheng Chen           Modified to support DirectFB 
// ---------------------------------------------------------------------------
// [DESCRIPTION]
// Gfx host Reg Operation Inteface and Gfx Reset I/F
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#ifndef _ORION_GFX_HW_IF_H_
#define _ORION_GFX_HW_IF_H_
#include "orion_gfx_type_def.h"
#include "orion_gfx_cmd_reg_def.h" //Register and Commad Que Const Define
//Reset
void GfxHWReset();
//Register
U32  GfxRegRead( U32 RegAddr);
void GfxRegWrite( U32 RegAddr, U32 WriteData);
//Interrupt
I32  IsGfxGenInterrupt();
void SetOrionGfxRegBaseAddr( U32 BaseAddr);
#endif
