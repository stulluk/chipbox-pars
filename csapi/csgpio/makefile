#Makefile for CSAPI 
#Copyright @ Celestial 2007

MODULE_NAME = csgpio

include ../config.mk
# Installed library (module)
SLIBRARY	    	= lib$(MODULE_NAME).a
DLIBRARY        	= lib$(MODULE_NAME).so  
SUBDIRS         	= src include

# Test source and its related module
TESTDIR         	= test
TEST_DEPEND_MODULE	= pthread 
TEST_RELATED_LIBS_DIR   =

# Debug Macro
DEBUG_MACRO = #CSGPIO_DEBUG


# Sources & Objects
SOURCES			=  $(foreach SUBDIRS,$(SUBDIRS),$(wildcard $(SUBDIRS)/*.c))
HEADFILES       =  $(foreach SUBDIRS,$(SUBDIRS),$(wildcard $(SUBDIRS)/*.h))
OBJECTS			=  $(SOURCES:.c=.o)
TESTOBJECTS     =  $(TESTSOURCES:.c=.o)
MODULE_HEAD     =  $(wildcard ../include/$(MODULE_NAME).h)
MAKE_CONFIG     =  $(wildcard ../*.mk)

#Test Source & related head files and libs
TESTSOURCES		=  $(foreach TESTDIR,$(TESTDIR),$(wildcard $(TESTDIR)/*.c))
TESTHEADFILES   =  $(foreach TESTDIR,$(TESTDIR),$(wildcard $(TESTDIR)/*.h))
TESTRELATEDLIBS =  $(addprefix -l, $(TEST_DEPEND_MODULE))
TESTRELATEDHEAD =  $(wildcard ../include/$(TEST_DEPEND_MODULE).h)

# defined flags
INCFLAGS    	+= -I ./include 
DEFINES	    	+= -D_LINUX_ 
CFLAGS	    	+= -W -Wall   
SHARED_FLAGS    += 
STATIC_FLAGS  	+= 
LIBS_DIR        = ../lib

LIBS_DIR_PREFIX =  $(addprefix -L, $(LIBS_DIR))
TESTRELATED_LIBS_DIR_PREFIX =  $(addprefix -L, $(TEST_RELATED_LIBS_DIR))
ifneq ($(strip $(DEBUG_MACRO)),) 
DEFINES	    	+= -D${DEBUG_MACRO}
CFLAGS			+= -g
else
CFLAGS			+= -O2
endif


.PRECIOUS: $(LIBRARY)
.SUFFIXES: .c .cc .cpp .cxx .yy.cpp .h

# define target and dependences

.c.o:
	@echo  CC     $< ...
	@$(CC) $(CFLAGS) -c  $(DEFINES) $(INCFLAGS) $< -o $@

.cc.o: 
	@echo  CC     $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

.cpp.o:
	@echo  CPP    $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

.cxx.o: 
	@echo  CXX    $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

# Make installation
all:  static dynamic test 

ifneq ($(strip $(DEBUG_MACRO)),) 	
static: $(SLIBRARY)
dynamic: $(DLIBRARY)
else
static: $(SLIBRARY)
dynamic: $(DLIBRARY)
	@$(STRIP) $(LIBS_DIR)/$(DLIBRARY)  
endif

test: $(MODULE_NAME)_test
$(SLIBRARY): $(OBJECTS) 
	@echo  LD     Static Library $^ ...
	@$(AR) $(STATIC_FLAGS) $(LIBS_DIR)/$(SLIBRARY) $^  
 
$(DLIBRARY): $(OBJECTS) 
	@echo  LD     Dynamic Library $^ ...
	@$(CC) $(CFLAGS) $(SHARED_FLAGS) $^ -o $(LIBS_DIR)/$(DLIBRARY)  

ifneq ($(strip $(TESTSOURCES)),) 
$(MODULE_NAME)_test: $(TESTOBJECTS)  
	@echo Link     $^ ...
	@$(CC)  $(CFLAGS) $(DEFINES) $(INCFLAGS) $(TESTRELATED_LIBS_DIR_PREFIX) $(LIBS_DIR_PREFIX)  $^ -o $@.elf -Wl,--start-group -l$(MODULE_NAME) $(TESTRELATEDLIBS) -Wl,--end-group
else
$(MODULE_NAME)_test:
endif

$(OBJECTS):  $(HEADFILES) $(MODULE_HEAD) $(MAKE_CONFIG)

$(TESTOBJECTS):  $(OBJECTS) $(HEADFILES) $(MODULE_HEAD) $(TESTRELATEDHEAD) 
 
clean: 
	@$(RM) -f *.elf *.o *.so *.a $(LIBS_DIR)/$(DLIBRARY) $(LIBS_DIR)/$(SLIBRARY)  $(TESTOBJECTS) $(OBJECTS) 	
