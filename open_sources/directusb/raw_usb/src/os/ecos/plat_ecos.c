
#include <types.h>

s32 init_regs(void)
{
	return 0;
}

s32 deinit_regs(void)
{
	return 0;
}

u32 read_regs32(u32 address, u32 *val)
{
	volatile u32 tmp = 0;

	tmp = *((volatile u32 *) (address));

	if ((void*)0 != val) *val = tmp;

	return tmp;
}

u16 read_regs16(u32 address, u16 *val)
{
	volatile u16 tmp = 0;

	tmp = *((volatile u16 *) (address));

	if ((void*)0 != val) *val = tmp;

	return tmp;
}

u8 read_regs8(u32 address, u8 *val)
{
	volatile u8 tmp = 0;

	tmp = *((volatile u8 *) (address));

	if ((void*)0 != val) *val = tmp;

	return tmp;
}

s32 write_regs32(u32 address, u32 val)
{
	*((volatile u32 *) (address)) = val;

	return 0;
}

s32 write_regs16(u32 address, u16 val)
{
	*((volatile u16 *) (address)) = val;

	return 0;
}

s32 write_regs8(u32 address, u8 val)
{
	*((volatile u8 *) (address)) = val;

	return 0;
}

