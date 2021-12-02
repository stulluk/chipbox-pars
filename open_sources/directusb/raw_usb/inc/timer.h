/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include "types.h"

#define CFG_PCLK_FREQ		55000000
#define CFG_HZ                  CFG_PCLK_FREQ

#define CFG_TIMERBASE           0x101E2000	/* Timer 0 and 1 base */
#define TIMER_LOAD_VAL 		0xffffffff

#define CFG_TIMER_INTERVAL	10000
#define CFG_TIMER_RELOAD	(CFG_TIMER_INTERVAL >> 4)	/* Divide by 16 */
#define CFG_TIMER_CTRL          0x84	/* Enable, Clock / 16 */

int timer_init(void);
void reset_timer(void);
ulong get_timer(ulong base);
void udelay(unsigned long usec);

#endif

/* EOF */
