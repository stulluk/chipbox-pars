#######################
# Make all components
########################
# Add for make system version 0 support
CS_STB_MAKE_VERSION :=0
SUBDIRS :=  bzip2-1.0.4 DirectFB-1.0.0 libminigui-1.3.3 busybox-1.20.2 freetype-2.2.1 jpeg-6b libpng-1.2.16 zlib-1.2.3  mtd-utils-1.0.0 hdparm-7.7
LIB_NAMES := busybox bzip2 zlib freetype png jpeg directfb minigui mtdutils hdparm
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
 
MAKEME = cd $(2); MakeSubDir $(1); cd ..;
LOOPMAKEFUNC = $(MAKEFUNC) $(foreach dir,$(SUBDIRS),$(call MAKEME,$(1),$(dir)))
 

all: 
	./config.sh $(LIB_NAMES)
jpeg:
	./config.sh jpeg
bzip2:
	./config.sh bzip2
directfb:
	./config.sh zlib jpeg png freetype directfb
minigui:
	./config.sh zlib jpeg png minigui
busybox:
	./config.sh busybox
zlib:
	./config.sh zlib
freetype:
	./config.sh freetype
png:
	./config.sh png
mtdutils:
	./config.sh zlib mtdutils
hdparm:
	./config.sh hdparm

clean:distclean
distclean:
	rm -rf bin man share inc libs rootfs
	$(call LOOPMAKEFUNC,distclean)

.PHONY: distclean all jpeg bzip2 zlib minigui busybox freetype directfb mtdutils

