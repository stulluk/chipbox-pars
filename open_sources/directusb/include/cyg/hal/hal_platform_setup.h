#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H
//=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
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

//     Only used by "vectors.S"         

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/orion.h>          // Platform specific hardware definitions

#define DEBUG

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

// We need this here - can't rely on a translation table until MMU has
// been initialized
        .macro RAW_LED_MACRO x
#ifdef DEBUG
        ldr     r0,=(ORION_UART0_BASE+_UART_THR)
        mov     r1,#(\x + 0x41)
        str     r1,[r0]
#endif
        .endm

// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1

#ifdef DEBUG
        // Init UART for debug tracing
        ldr     r4,=ORION_UART0_BASE
        ldrb    r2,=(_UART_LCR_VAL|_UART_LCR_BKSE)
        strb    r2,[r4,#_UART_LCR]
        ldrb    r2,=CYG_DEVICE_SERIAL_BAUD_LSB
        strb    r2,[r4,#_UART_DLL]
        ldrb    r2,=CYG_DEVICE_SERIAL_BAUD_MSB
        strb    r2,[r4,#_UART_DLM]
        ldrb    r2,=_UART_LCR_VAL
        strb    r2,[r4,#_UART_LCR]
        ldrb    r2,=_UART_FCR_VAL
        strb    r2,[r4,#_UART_FCR]
        ldrb    r2,=_UART_MCR_VAL
        strb    r2,[r4,#_UART_MCR]		
#endif
        RAW_LED_MACRO 0
        // Disable and clear caches
        
        mrc  p15,0,r0,c1,c0,0
        bic  r0,r0,#0x1000              // disable ICache
        bic  r0,r0,#0x0007              // disable DCache
                                        // MMU and alignment faults
        mcr  p15,0,r0,c1,c0,0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mov  r0,#0
        mcr  p15,0,r0,c7,c6,0           // clear data cache
        mcr  p15,0,r0,c7,c5,0           // clear instruction cache
        RAW_LED_MACRO 1
		
		// Enable DDR
      mov r0, #0x10000000
      orr r0, r0, #0x110000

      adr r1, DRAM_Config_Table
      adr r2, DRAM_Config_Table_End
1:
      ldr r3, [r1], #4
      str r3, [r0], #4
      cmp r1, r2
      bcc 1b
		
		RAW_LED_MACRO 2

		// Check if we are already running in RAM
		ldr		r0,=(ORION_REMAP_CR)
		ldr		r1, [r0]
		cmp		r1, #(ORION_REMAP_CR_VAL)
		beq		_copy_done
		
		        // Jump to ROM, ROM is not accessible now, so we have to use a few words of RAM
		ldr		r2,=(CYGMEM_REGION_ram_SIZE+CYGMEM_REGION_ram-16)
		adr		r1, _do_remap
		ldmia	r1, {r3-r6}
		stmia	r2, {r3-r6}
		
		adr		lr, _remap_done
		ldr		r1, =(CYGMEM_REGION_rom)
        add     lr, lr, r1
		ldr		r1, =(ORION_REMAP_CR_VAL)
        mov     pc,r2

_do_remap:
		str		r1, [r0]
		mov		pc, lr
		nop
		nop		
_ddrcaddr:
		.word	0x10110000
_ddrparam:

		.word	0x3a1b69ab
		.word	0x213b218a		// 32M*16
		.word	0x05050505
		.word	0x05050505		
		.word	0x11542020	
DRAM_Config_Table:
        .word          0x3a1b69ab
        .word          0x213b218a
        .word          0x05050505
        .word          0x05050505
        .word          0x11762020 
DRAM_Config_Table_End:

_remap_done:
        RAW_LED_MACRO 3
		
#if defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
        ldr     r0,=__rom_vectors_lma   // Relocate FLASH/ROM to SDRAM
        ldr     r1,=__rom_vectors_vma   // ram base & length
        ldr     r2,=__ram_data_end
20:     ldr     r3,[r0],#4
        str     r3,[r1],#4
        cmp     r1,r2
        bne     20b
        ldr     r0,=30f
        nop
        mov     pc,r0
	nop
30:	nop
#endif
_copy_done:
        RAW_LED_MACRO 4

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=ORION_SDRAM_PHYS_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init

        RAW_LED_MACRO 8

        // Enable MMU
        ldr     r2,=10f
        ldr     r1,=MMU_Control_Init|MMU_Control_M
        mcr     MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2    // Change address spaces
        nop
        nop
        nop
10:

        RAW_LED_MACRO 9
        .endm
        
#else // defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
#ifdef DEBUG
#define CYGHWR_LED_MACRO                                \
        ldr     r0,=(ORION_UART0_BASE+_UART_THR);    \
        mov     r1,#((\x) + 0x61);                      \
        str     r1,[r0];
#endif

#endif // CYGONCE_HAL_PLATFORM_SETUP_H
