# Config.mk 
# defined some generic variables

SHELL	= /bin/bash
DIR		= $(shell pwd)
 
# compile and bin utilies
CC		= arm-linux-gcc  
AR		= arm-linux-ar   
LD		= arm-linux-ld   
RUN		= arm-linux-run  
DB		= arm-linux-gdb  
LINK    = arm-linux-gcc  
CPP     = arm-linux-g++
CXX     = arm-linux-g++
RANLIB  = arm-linux-ranlib
AS      = arm-linux-as
STRIP	= arm-linux-strip
NM		= arm-linux-nm
# shell commands
CP 		= cp
MV		= mv

# generic flags 
INCFLAGS    	= -I ./include -I../include
DEFINES	    	=
CFLAGS	    	= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -march=armv5te -W -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -O2 #-ffunction-sections -fdata-sections -fno-exceptions  
SHARED_FLAGS   	= -fPIC -shared
STATIC_FLAGS  	= crs

include $(DIR)/../archconfig.mk
CFLAGS +=-D${CHIP_ARCH}

