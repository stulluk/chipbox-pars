## Makefile.am for DirectFB/src/core/gfxcards/orion

INCLUDES = \
	-I$(top_srcdir)/include	\
	-I$(top_srcdir)/lib	\
	-I$(top_builddir)/lib	\
	-I$(top_srcdir)/systems	\
	-I$(top_srcdir)/src

orion_LTLIBRARIES = libdirectfb_orion.la

if BUILD_STATIC
orion_DATA = $(orion_LTLIBRARIES:.la=.o)
endif

oriondir = $(MODULEDIR)/gfxdrivers

libdirectfb_orion_la_SOURCES =	\
  orion_gfx.c \
  orion_gfx.h \
  orion_gfx_2dlib.c \
  orion_gfx_2dlib.h \
  orion_gfx_api.c \
  orion_gfx_api.h \
  orion_gfx_cmd_reg_def.h \
  orion_gfx_file_func.c \
  orion_gfx_file_func.h \
  orion_gfx_hw_if.c \
  orion_gfx_hw_if.h \
  orion_gfx_loglib.c \
  orion_gfx_loglib.h \
  orion_gfx_sys_cfg.h \
  orion_gfx_test_vect.h \
  orion_gfx_type_def.h

libdirectfb_orion_la_LDFLAGS = \
	-export-dynamic		\
	-avoid-version		\
	$(DFB_LDFLAGS)

libdirectfb_orion_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make
