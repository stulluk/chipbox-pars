#######################
# Make all components
########################
# Add for make system version 0 support
# Author xiaodong.fan@celestialsemi.com

include ./archconfig.mk
CS_STB_MAKE_VERSION :=0
INSTALL_DIR=
SUBDIRS = mvipc 
MAKEFUNC = @MakeSubDir() \
        { \
				if [ -f makefile -o -f Makefile ]; then \
                        echo ""; \
                        make $(1); \
                        if [ "$$?" != "0" ]; then \
                            exit 1; \
                        fi; \
                fi; \
        }; \
 
MAKEME = if test -d $(2); then cd $(2);  MakeSubDir $(1); cd ..; else echo "**************************"; echo "\nThere is no dir: $(2)\n";echo "***************************\n"; fi; 
LOOPMAKEFUNC = $(MAKEFUNC) $(foreach dir,$(SUBDIRS),$(call MAKEME,$(1),$(dir)))

INSTALL = mkdir -p $(INSTALL_DIR)/$(1); cp -r $(1)/include $(INSTALL_DIR)/$(1);
LOOPINSTALL = $(foreach dir,$(SUBDIRS),$(call INSTALL,$(dir)))

all:
	$(call LOOPMAKEFUNC,)
	@echo "#define $(CHIP_ARCH)" > ./include/.temp_mvapi.h
	@grep "ARCH_" ./include/mvapi.h -v >> ./include/.temp_mvapi.h
	@mv -f ./include/.temp_mvapi.h  ./include/mvapi.h
	@cp ./mvipc/include/*.h ../oscam/chipbox/
	@cp ./lib/*.* ../oscam/chipbox/
	 
install:
	mkdir -p $(INSTALL_DIR)
	@$(call LOOPINSTALL)
	cp -r  lib $(INSTALL_DIR)
	cp -r  include $(INSTALL_DIR)
	@echo "#define $(CHIP_ARCH)" > $(INSTALL_DIR)/include/.temp_mvapi.h
	@grep "ARCH_" $(INSTALL_DIR)/include/mvapi.h -v >> $(INSTALL_DIR)/include/.temp_mvapi.h
	@mv -f $(INSTALL_DIR)/include/.temp_mvapi.h  $(INSTALL_DIR)/include/mvapi.h

csm1200:
	@grep "CHIP_ARCH" ./archconfig.mk -v > .temp_arch.mk
	@echo "CHIP_ARCH = ARCH_CSM1200" >> .temp_arch.mk
	@mv -f ./.temp_arch.mk ./archconfig.mk
	@echo "\n**********Configured for CSM1200************"
	@echo "Executing \"make\" to build mvapi for CSM1200\n"
csm1201:
	@grep "CHIP_ARCH" ./archconfig.mk -v > .temp_arch.mk
	@echo "CHIP_ARCH = ARCH_CSM1201" >> .temp_arch.mk
	@mv -f ./.temp_arch.mk ./archconfig.mk
	@echo "\n**********Configured for CSM1201************"
	@echo "Executing \"make\" to build mvapi for CSM1201\n"
distclean:clean
clean:
	$(call LOOPMAKEFUNC,clean)
static:
	$(call LOOPMAKEFUNC,static)
	@echo "#define $(CHIP_ARCH)" > ./include/.temp_mvapi.h
	@grep "ARCH_" ./include/mvapi.h -v >> ./include/.temp_mvapi.h
	@mv -f ./include/.temp_mvapi.h  ./include/mvapi.h
	@cp ./mvipc/include/*.h ../oscam/chipbox/
	@cp ./lib/*.* ../oscam/chipbox/
	
dynamic:
	$(call LOOPMAKEFUNC,dynamic)
	@echo "#define $(CHIP_ARCH)" > ./include/.temp_mvapi.h
	@grep "ARCH_" ./include/mvapi.h -v >> ./include/.temp_mvapi.h
	@mv -f ./include/.temp_mvapi.h  ./include/mvapi.h
	@cp ./mvipc/include/*.h ../oscam/chipbox/
	@cp ./lib/*.* ../oscam/chipbox/

.PHONY: clean all install

