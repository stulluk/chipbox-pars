
CC = arm-elf-gcc
LD = arm-elf-ld

SOURCES := tt_fs.c
INCLUDE := 
OBJS := $(SOURCES:.c=.o)
TARGETS := tt_fs

CFLAGS := -Wpointer-arith -Wstrict-prototypes -Winline -Wundef  -ffunction-sections -fdata-sections -Wall -g  -O0 -mcpu=arm9

LDFLAGS := -nostartfiles -nostdlib --gc-sections -static  
ECOS_INCLUDE := ./include
ECOS_LIB := ./lib


$(TARGETS): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -I./ -I$(ECOS_INCLUDE)  -L$(ECOS_LIB) -Ttarget.ld $(OBJS) -o $(TARGETS).elf
	arm-elf-objcopy -O binary $(TARGETS).elf  $(TARGETS).bin

$(OBJS):%.o:%.c $(INCLUDE)
	$(CC) $(CFLAGS) -I./  -I$(ECOS_INCLUDE) -o $@ -c $<

# Clean .o and obj
#-------------------
clean: 
	-$(RM) *.o *.elf *.bin

