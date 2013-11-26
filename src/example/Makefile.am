SUBDIRS = .

AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src 
   
appldflags = -rdynamic

appldadd = \
  @CURL_LIBS@ \
  ../libjsonrpccpp.la

bin_PROGRAMS = \
  simpleserversample \
  simpleclientsample \
  stubclientsample \
  stubserversample \
  xbmcremote

simpleserversample_LDADD = $(appldadd)
simpleserversample_LDFLAGS = $(appldflags)
simpleserversample_SOURCES = simpleserver.cpp

simpleclientsample_LDADD = $(appldadd)
simpleclientsample_LDFLAGS = $(appldflags)
simpleclientsample_SOURCES = simpleclient.cpp

stubclientsample_LDADD = $(appldadd)
stubclientsample_LDFLAGS = $(appldflags)
stubclientsample_SOURCES = stubclient.cpp mystubclient.h

stubserversample_LDADD = $(appldadd)
stubserversample_LDFLAGS = $(appldflags)
stubserversample_SOURCES = stubserver.cpp abstractmystubserver.h

xbmcremote_LDADD = $(appldadd)
xbmcremote_LDFLAGS = $(appldflags)
xbmcremote_SOURCES = xbmcremote.cpp XbmcRemoteClient.h


EXTRA_DIST = xbmc_remote.json





