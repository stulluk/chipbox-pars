#Makefile for CSAPI 
#Copyright @ Celestial 2007

MODULE_NAME = e2p

include ../../../config.mk
# Installed library (module)
SLIBRARY	    	= lib$(MODULE_NAME).a
#DLIBRARY        	= lib$(MODULE_NAME).so  
SUBDIRS         	=

# Test source and its related module
TESTDIR         	= test
TEST_DEPEND_MODULE	= 

# Sources & Objects
SOURCES			=  $(foreach SUBDIRS,$(SUBDIRS),$(wildcard $(SUBDIRS)/*.c))
HEADFILES       =  $(foreach SUBDIRS,$(SUBDIRS),$(wildcard $(SUBDIRS)/*.h))
OBJECTS			=  $(SOURCES:.c=.o)
TESTOBJECTS     =  $(TESTSOURCES:.c=.o)
MODULE_HEAD     =  $(wildcard ../../../include/$(MODULE_NAME).h)

#Test Source & related head files and libs
TESTSOURCES		=  $(foreach TESTDIR,$(TESTDIR),$(wildcard $(TESTDIR)/*.c))
TESTHEADFILES   =  $(foreach TESTDIR,$(TESTDIR),$(wildcard $(TESTDIR)/*.h))
TESTRELATEDLIBS =  $(addprefix -l, $(TEST_DEPEND_MODULE))
TESTRELATEDHEAD =  $(wildcard ../include/$(TEST_DEPEND_MODULE).h)

# defined flags
INCFLAGS    	+= -I ../../include -I $(APP_WORK)/csapi/include -I ../../../csmid/include
DEFINES	    	+= -D_LINUX_
CFLAGS	    	+= -Wall -O2 
SHARED_FLAGS    += 
STATIC_FLAGS  	+= 
LIBS             = ../../lib

.PRECIOUS: $(LIBRARY)
.SUFFIXES: .c .cc .cpp .cxx .yy.cpp .h

# define target and dependences

.c.o:
	@echo  CC   $< ...
	@$(CC) $(CFLAGS) -c  $(DEFINES) $(INCFLAGS) $< -o $@

.cc.o: 
	@echo  CC  $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

.cpp.o:
	@echo  CPP  $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

.cxx.o: 
	@echo  CXX  $< ...
	@$(CXX) $(CFLAGS) -c $(DEFINES) $(INCFLAGS) $< -o $@

# Make installation
all:  $(SLIBRARY)

$(SLIBRARY): $(OBJECTS) 
	@echo LD Static Library $^ ...
	@$(AR) $(STATIC_FLAGS) $(LIBS)/$(SLIBRARY) $^  
 
$(DLIBRARY): $(OBJECTS) 
	@echo LD Dynamic Library $^ ...
	@$(CC) $(CFLAGS) $(SHARED_FLAGS) $^ -o $(LIBS)/$(DLIBRARY)  

ifneq ($(strip $(TESTSOURCES)),) 
$(MODULE_NAME)_test: $(TESTOBJECTS)  
	@echo Link $^ ...
	@$(CC)  $(CFLAGS) $(DEFINES) $(INCFLAGS) -L$(LIBS) -Wl,--start-group -l$(MODULE_NAME) $(TESTRELATEDLIBS) -Wl,--end-group $^ \
		-o $@.elf 
else
$(MODULE_NAME)_test:
endif

$(OBJECTS):  $(HEADFILES) $(MODULE_HEAD)

$(TESTOBJECTS):$(SLIBRARY) $(DLIBRARY) $(HEADFILES) $(MODULE_HEAD) $(TESTRELATEDHEAD) 
 
clean: 
	@$(RM) *.elf *.o  *.a $(LIBS)/$(DLIBRARY) $(LIBS)/$(SLIBRARY)  $(TESTOBJECTS) $(OBJECTS) 	
