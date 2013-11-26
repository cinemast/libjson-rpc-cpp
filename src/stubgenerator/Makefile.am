SUBDIRS = .

AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src 
   
appldflags = -rdynamic

appldadd = \
  @CURL_LIBS@ \
  ../libjsonrpccpp.la

bin_PROGRAMS = jsonrpcstub

jsonrpcstub_LDADD = $(appldadd)
jsonrpcstub_LDFLAGS = $(appldflags)
jsonrpcstub_SOURCES = \
	serverstubgenerator.h \
	stubgenerator.h \
	clientstubgenerator.h \
	servertemplate.h \
	stubhelper.h \
	clienttemplate.h \
	serverstubgenerator.cpp \
	clientstubgenerator.cpp \
	stubgenerator.cpp \
	main.cpp






