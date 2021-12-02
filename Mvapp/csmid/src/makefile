INCFLAGS        = -I $(APP_WORK)/csapi/include/ 
INCFLAGS        += -I ../include/
DEFINES         = -D_LINUX_
CFLAGS          = -mcpu=arm9 -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -O2 -ffunction-sections -fdata-sections -fno-exceptions
LIBS            = -lcsdemux -lcsaud -lcsvid -lcsdirectfb -lcstuner -lpthread -lrt -lminigui
all:
	arm-linux-gcc $(CFLAGS) -o csmid  linuxos.c csmid_test.c  -L$(APP_WORK)/csapi/lib -Wl,--start-group $(LIBS)  -Wl,--end-group $(INCFLAGS)

