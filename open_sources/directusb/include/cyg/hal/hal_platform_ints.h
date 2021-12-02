#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####

#include <cyg/hal/orion.h>

// These are interrupts on Virgo SoC

#define CYGNUM_HAL_INTERRUPT_WDT               	0
#define CYGNUM_HAL_INTERRUPT_SW               	1
#define CYGNUM_HAL_INTERRUPT_COMMRX             2
#define CYGNUM_HAL_INTERRUPT_COMMTX             3
#define CYGNUM_HAL_INTERRUPT_TIMER_0            4
#define CYGNUM_HAL_INTERRUPT_TIMER_1            4
#define CYGNUM_HAL_INTERRUPT_TIMER_2            5
#define CYGNUM_HAL_INTERRUPT_TIMER_3            5
#define CYGNUM_HAL_INTERRUPT_GPIO0           	6
#define CYGNUM_HAL_INTERRUPT_GPIO1              7
#define CYGNUM_HAL_INTERRUPT_IRDA             	9
//#define CYGNUM_HAL_INTERRUPT_RTC             	10
#define CYGNUM_HAL_INTERRUPT_RTC             	4	//DXL: use Timer-0 for task scheduling now. TODO: use RTC instead
#define CYGNUM_HAL_INTERRUPT_UART_0             12
#define CYGNUM_HAL_INTERRUPT_UART_1          	13
#define CYGNUM_HAL_INTERRUPT_SMI_0          	14
#define CYGNUM_HAL_INTERRUPT_SMI_1          	15
#define CYGNUM_HAL_INTERRUPT_MP_TSD             16
#define CYGNUM_HAL_INTERRUPT_DMAC             	17
#define CYGNUM_HAL_INTERRUPT_GFX         		19
#define CYGNUM_HAL_INTERRUPT_MP_VIDEO           20
#define CYGNUM_HAL_INTERRUPT_MP_AUDIO           21
#define CYGNUM_HAL_INTERRUPT_EXT_0         		28
#define CYGNUM_HAL_INTERRUPT_EXT_1         		29
#define CYGNUM_HAL_INTERRUPT_EXT_2         		30
#define CYGNUM_HAL_INTERRUPT_EXT_3         		31

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX            (CYGNUM_HAL_INTERRUPT_EXT_3)

#define CYGNUM_HAL_ISR_COUNT          (CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_ISR_MIN+1)

//----------------------------------------------------------------------------
// Reset.

// Writing a bad value to the watchdog reload register causes a reset.
#define HAL_PLATFORM_RESET() \
    HAL_WRITE_UINT32(ORION_WDOG_RELOAD, 0)

#define HAL_PLATFORM_RESET_ENTRY ORION_FLASH_PHYS_BASE

#endif // CYGONCE_HAL_PLATFORM_INTS_H
