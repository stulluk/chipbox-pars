#ifndef CYGONCE_ORION_H
#define CYGONCE_ORION_H

//=============================================================================
//
//      orion.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
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

#include <pkgconf/hal_arm_arm9_orion.h>

#define ORION_BASE         ORION_REGS_PHYS_BASE

#define ORION_SDRAM_PHYS_BASE 0x00000000
#define ORION_FLASH_PHYS_BASE 0x34000000
#define ORION_REGS_PHYS_BASE  0x10100000
#define ORION_MEDIA_PHYS_BASE 0x40000000

#define CYGNUM_HAL_ARM_ORION_PERIPHERAL_CLOCK	55000000	// Virgo changed to 27MHz clock input.
// #define CYGHWR_NET_DRIVER_ETH0
//-----------------------------------------------------------------------------
// Serial
#define ORION_UART0_BASE            (ORION_BASE+0xf1000)
#define ORION_UART1_BASE            (ORION_BASE+0xf2000)

#define _UART_RBR         		0x0000
#define _UART_THR         		0x0000
#define _UART_DLL         		0x0000
#define _UART_IER              	0x0004
#define _UART_DLM              	0x0004
#define _UART_FCR              	0x0008
#define _UART_IIR              	0x0008
#define _UART_LCR              	0x000c
#define _UART_MCR              	0x0010
#define _UART_LSR              	0x0014
#define _UART_MSR              	0x0018
#define _UART_SCR              	0x001c

#define _UART_LCR_BKSE			0x80
#define _UART_LCR_VAL     		0x03	// 8-N-1
#define _UART_MCR_VAL          	0x03	// DTR+RTS
#define	_UART_FCR_VAL			0x07	// FIFO, Soft-Reset

#define _UART_LSR_THRE			0x20	/* Xmit holding register empty */
#define _UART_LSR_DR			0x01	/* Data ready */

#define CYG_DEVICE_SERIAL_BAUD_DIV (CYGNUM_HAL_ARM_ORION_PERIPHERAL_CLOCK/CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD/16)
#define CYG_DEVICE_SERIAL_BAUD_LSB (CYG_DEVICE_SERIAL_BAUD_DIV&0xff)
#define CYG_DEVICE_SERIAL_BAUD_MSB ((CYG_DEVICE_SERIAL_BAUD_DIV>>8)&0xff)

//Remap registers
#define	ORION_REMAP_CR			(ORION_BASE+0xe0008)
#define ORION_REMAP_CR_VAL		1

//Timer
#define ORION_TIMER0_BASE		(ORION_BASE+0xe2000)	
#define ORION_TIMER1_BASE		(ORION_BASE+0xe2014)	// Share IRQ with Timer 0
#define ORION_TIMER2_BASE		(ORION_BASE+0xe3000)
#define ORION_TIMER3_BASE		(ORION_BASE+0xe3014)	// Share IRQ with Timer 2

#define _TIMER_LOADCOUNT 		0
#define _TIMER_COUNT			4
#define _TIMER_CONTROL   		8
#define _TIMER_EOI       		0x0c

//RTC
#define ORION_RTC_BASE			(ORION_BASE+0xe8000)
#define _RTC_LOADCOUNT     		 0x8
#define _RTC_COUNT         		 0x0
#define _RTC_CONTROL       		 0x0c
#define _RTC_EOI           		 0x18
#define _RTC_STAT         		 0x10
#define _RTC_CMR           		 0x04

#define ORION_SMI0_BASE			(ORION_BASE+0xf0000)
#define ORION_GPIO0_BASE		(ORION_BASE+0xe4000)
#define ORION_GPIO1_BASE		(ORION_BASE+0xe5000)

// Interrupt Controller
#define ORION_ICTL_BASE			(ORION_BASE+0x40000)
#define _ICTL_IRQ_ENABLE		0
#define _ICTL_IRQ_MASK			8
#define _ICTL_IRQ_STATUS		0x28
#define _ICTL_IRQ_FORCE         0x10



//-----------------------------------------------------------------------------
// Watchdog controller
#define	ORION_WDOG_BASE				(ORION_BASE+0xe1000)
#define ORION_WDOG_CR               (ORION_WDOG_BASE+0x00)
#define ORION_WDOG_COUNT            (VIROG_WDOG_BASE+0x04)
#define ORION_WDOG_RELOAD           (ORION_WDOG_BASE+0x08)

#endif // CYGONCE_ORION_H
