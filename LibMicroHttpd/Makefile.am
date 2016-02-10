# This Makefile.am is in the public domain
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = contrib src m4 .
W32COMMON = w32/common/gen_dll_res.ps1 w32/common/microhttpd_dll_res_vc.rc.in w32/common/microhttpd_dll_res_vc.rc \
  w32/common/MHD_config.h w32/common/vs_dirs.props w32/common/common-build-settings.vcxproj \
  w32/common/libmicrohttpd-build-settings.vcxproj w32/common/libmicrohttpd-files.vcxproj w32/common/libmicrohttpd-filters.vcxproj \
  w32/common/hellobrowser-build-settings.vcxproj w32/common/hellobrowser-files.vcxproj w32/common/hellobrowser-filters.vcxproj
W32VS2013 = w32/VS2013/libmicrohttpd.vcxproj w32/VS2013/libmicrohttpd.vcxproj.filters \
  w32/VS2013/hellobrowser.vcxproj w32/VS2013/hellobrowser.vcxproj.filters \
  w32/VS2013/libmicrohttpd.sln 
W32VS2015 = w32/VS2015/libmicrohttpd.vcxproj w32/VS2015/libmicrohttpd.vcxproj.filters \
  w32/VS2015/hellobrowser.vcxproj w32/VS2015/hellobrowser.vcxproj.filters \
  w32/VS2015/libmicrohttpd.sln 
EXTRA_DIST = acinclude.m4 libmicrohttpd.pc.in \
  $(W32COMMON) $(W32VS2013) $(W32VS2015)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmicrohttpd.pc 

if BUILD_DOC
SUBDIRS += doc
endif
