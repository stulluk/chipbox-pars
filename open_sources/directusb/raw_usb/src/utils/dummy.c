
/* 
 * WARNING:
 * 
 *     You should implement the below functions according to the platform you used.
 *
 */

#include <types.h>

// void udelay(int us)
// {
// 
// #include <cyg/hal/hal_arch.h>
// #include <cyg/hal/hal_intr.h>
// #include <cyg/hal/hal_if.h>
// 
// 	CYGACC_CALL_IF_DELAY_US(us);
// }

void enable_interrupts(void)
{
	cyg_interrupt_enable();

	return;
}

int disable_interrupts(void)
{
	cyg_interrupt_disable();

	return 0;
}

void free_usb_bus(void)
{
	write_regs16(0x101e4008, read_regs16(0x101e4008, (void*)0) & ~0x08);
	write_regs16(0x101e4004, read_regs16(0x101e4004, (void*)0) | 0x08);
	write_regs16(0x101e4000, read_regs16(0x101e4000, (void*)0) | 0x08);
	write_regs16(0x10171400, read_regs16(0x10171400, (void*)0) & 0x09);
}

void get_usb_bus(void)
{
	write_regs16(0x101e4008, read_regs16(0x101e4008, (void*)0) & ~0x08);
	write_regs16(0x101e4004, read_regs16(0x101e4004, (void*)0) |  0x08);
	write_regs16(0x101e4000, read_regs16(0x101e4000, (void*)0) & ~0x08);
	write_regs16(0x10171400, read_regs16(0x10171400, (void*)0) |  0x02);
}

