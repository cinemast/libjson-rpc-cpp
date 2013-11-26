SUBDIRS = .

AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src
     
  
appldflags = -rdynamic

appldadd = \
  @CURL_LIBS@ \
  ../libjsonrpccpp.la

TESTS = helloworld remotecounter remotecalculator errorhandling jsonrpcprotocol specification parametervalidation

check_PROGRAMS  = $(TESTS)

appcommonsrc = server.cpp server.h

helloworld_LDADD = $(appldadd)
helloworld_LDFLAGS = $(appldflags)
helloworld_SOURCES = helloworld.cpp $(appcommonsrc)

remotecounter_LDADD = $(appldadd)
remotecounter_LDFLAGS = $(appldflags)
remotecounter_SOURCES = remotecounter.cpp $(appcommonsrc)

remotecalculator_LDADD = $(appldadd)
remotecalculator_LDFLAGS = $(appldflags)
remotecalculator_SOURCES = remotecalculator.cpp $(appcommonsrc)

errorhandling_LDADD = $(appldadd)
errorhandling_LDFLAGS = $(appldflags)
errorhandling_SOURCES = errorhandling.cpp $(appcommonsrc)

jsonrpcprotocol_LDADD = $(appldadd)
jsonrpcprotocol_LDFLAGS = $(appldflags)
jsonrpcprotocol_SOURCES = jsonrpcprotocol.cpp $(appcommonsrc)

specification_LDADD = $(appldadd)
specification_LDFLAGS = $(appldflags)
specification_SOURCES = specification.cpp $(appcommonsrc)

parametervalidation_LDADD = $(appldadd)
parametervalidation_LDFLAGS = $(appldflags)
parametervalidation_SOURCES = parametervalidation.cpp

DISTCLEANFILES = Makefile.in





