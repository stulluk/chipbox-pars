ECOS_GLOBAL_CFLAGS = -mcpu=arm9 -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority
ECOS_GLOBAL_LDFLAGS = --no-target-default-spec -Wl,--gc-sections -Wl,-static -g -O2 -nostdlib
ECOS_COMMAND_PREFIX = arm-elf-
