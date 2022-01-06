INCFLAGS        = -I $(APP_WORK)/csapi/include/
INCFLAGS        += -I ../include/ -I ../../csmid/include/ -I ../../system/include/
DEFINES         = -D_LINUX_
CFLAGS          = -mcpu=arm9 -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -O2 -ffunction-sections -fdata-sections -fno-exceptions
LIBS            = -lcsdemux -lcsaud -lcsvid -lcsdirectfb -lcstuner -lpthread -lrt -lminigui

all:
	arm-linux-gcc $(CFLAGS) -o mvmid  mvos.c -Wl,--start-group $(LIBS)  -Wl,--end-group $(INCFLAGS)
