# This Makefile.am is in the public domain
AM_CPPFLAGS = \
  -I$(top_srcdir)/src/include

AM_CFLAGS = $(HIDDEN_VISIBILITY_CFLAGS)

if USE_COVERAGE
  AM_CFLAGS += --coverage
endif

if HAVE_W32
noinst_LTLIBRARIES = \
  libplatform_interface.la
libplatform_interface_la_CPPFLAGS = \
  $(AM_CPPFLAGS) \
  -DBUILDING_MHD_LIB=1
libplatform_interface_la_SOURCES = \
  w32functions.c
endif

