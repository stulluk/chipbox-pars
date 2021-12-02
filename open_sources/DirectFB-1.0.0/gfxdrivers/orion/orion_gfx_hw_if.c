/*////////////////////////////////////////////////////////////////////////
// Copyright (C) 2006 Celestial Semiconductor Inc.
// All rights reserved
// ---------------------------------------------------------------------------
// FILE NAME        : orion_gfx_hw_if.c
// MODULE NAME      : host if for RTL and FPGA Simulation
// AUTHOR           : Jiasheng Chen
// ---------------------------------------------------------------------------
// [RELEASE HISTORY]                           Last Modified : 06-11-24
// VERSION  DATE       AUTHOR                  DESCRIPTION
// 0.1      06-11-24   jiasheng Chen           Original
// 0.2      07-05-14   Jiasheng Chen           Modified to support DirectFB 
// ---------------------------------------------------------------------------
// [DESCRIPTION]
//	host if for RTL and FPGA Simulation
// ---------------------------------------------------------------------------
// $Id: 
///////////////////////////////////////////////////////////////////////*/

#include "orion_gfx_hw_if.h"
#include "orion_gfx_loglib.h"
//#define TRACE_ORIONG_GFX_REG

static U32 sOrionGfxRegBaseAddr = 0;
void SetOrionGfxRegBaseAddr( U32 BaseAddr)
{
	sOrionGfxRegBaseAddr= BaseAddr;
	PRINT_D1("ORION_GFX: Set RegBaseAddr = %08x\n", sOrionGfxRegBaseAddr);
}

U32 GetOrionGfxRegBaseAddr()
{
	return sOrionGfxRegBaseAddr;
}
//Reset
void GfxHWReset()
{
	return;
}
//Register
U32  GfxRegRead( U32 RegAddr)
{
	U32 RegData = 0;
	RegData = *((volatile U32*)(RegAddr + sOrionGfxRegBaseAddr));
	#ifdef TRACE_ORIONG_GFX_REG
	PRINT_D2_L2("ORION_GFX_REG_READ : RegAddr(%08x) = %08x\n", RegAddr, RegData);
	#endif
	return RegData;
}
void GfxRegWrite( U32 RegAddr, U32 WriteData)
{
	*((volatile U32*)(RegAddr + sOrionGfxRegBaseAddr)) = (WriteData);
	#ifdef TRACE_ORIONG_GFX_REG
	PRINT_D2_L2("ORION_GFX_REG_WRITE: RegAddr(%08x) = %08x\n", RegAddr, WriteData);
	#endif
}
//Interrupt
I32  IsGfxGenInterrupt()
{
	return 1;
}

