# This Makefile.am is in the public domain
if HAVE_CURL
curltests = testcurl
if HAVE_ZZUF
if HAVE_SOCAT
zzuftests = testzzuf
endif
endif
endif

SUBDIRS = include platform microhttpd $(curltests) $(zzuftests) .

if BUILD_EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = \
 datadir/cert-and-key.pem \
 datadir/cert-and-key-for-wireshark.pem 
