# Config.mk 
# defined some generic variables

SHELL	= /bin/bash
DIR		= $(shell pwd)

# compile and bin utilies
CC		= arm-linux-gcc -D_FILE_OFFSET_BITS=64 
AR		= arm-linux-ar   
LD		= arm-linux-ld   
RUN		= arm-linux-run  
DB		= arm-linux-gdb  
LINK    = arm-linux-gcc  
CPP     = arm-linux-g++
CXX     = arm-linux-g++
RANLIB  = arm-linux-ranlib
AS      = arm-linux-as
STRIP   = arm-linux-strip

# shell commands
CP 		= cp
MV		= mv

# generic flags 
INCFLAGS    	= -I $(APP_WORK)/csapi/include -I $(APP_WORK)/open_sources/inc -I $(APP_WORK)/open_sources/inc/directfb -I $(APP_WORK)/open_sources/inc/directfb/direct -I $(APP_WORK)/open_sources/inc/directfb/fusion
DEFINES	    	= -DSUPPORT_CI
CFLAGS	    	= -mcpu=arm9 -W -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -O2 #-ffunction-sections -fdata-sections -fno-exceptions  
SHARED_FLAGS   	= -fPIC -shared
STATIC_FLAGS  	= crs

SW_ARCH = CS_ARCH_CSM1201
