if BUILD_EXAMPLES
  EXAMPLES_DIR = example
endif

SUBDIRS = . stubgenerator test $(EXAMPLES_DIR)

AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src
     
lib_LTLIBRARIES = libjsonrpccpp.la

version_Current=0
version_Age=0
version_Revision=2

libjsonrpccpp_la_LDFLAGS = \
  -version-info ${version_Current}:${version_Revision}:${version_Age} \
  @CURL_LIBS@
  
libjsonrpccpp_la_SOURCES = \
  jsonrpc/json/json_internalarray.inl \
  jsonrpc/json/json_valueiterator.inl \
  jsonrpc/json/json_internalmap.inl \
  jsonrpc/json/json_value.cpp \
  jsonrpc/json/json_writer.cpp \
  jsonrpc/json/json_reader.cpp \
  jsonrpc/connectors/httpserver.cpp \
  jsonrpc/connectors/httpclient.cpp \
  jsonrpc/connectors/mongoose.c \
  jsonrpc/errors.cpp \
  jsonrpc/rpcprotocolclient.cpp \
  jsonrpc/specificationwriter.cpp \
  jsonrpc/server.cpp \
  jsonrpc/exception.cpp \
  jsonrpc/procedure.cpp \
  jsonrpc/rpcprotocolserver.cpp \
  jsonrpc/client.cpp \
  jsonrpc/clientconnector.cpp \
  jsonrpc/serverconnector.cpp \
  jsonrpc/specificationparser.cpp


# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_include_HEADERS = \
  jsonrpc/json/json_batchallocator.h \
  jsonrpc/json/writer.h \
  jsonrpc/json/config.h \
  jsonrpc/json/autolink.h \
  jsonrpc/json/json.h \
  jsonrpc/json/value.h \
  jsonrpc/json/features.h \
  jsonrpc/json/forwards.h \
  jsonrpc/json/reader.h \
  jsonrpc/clientconnector.h \
  jsonrpc/rpc.h \
  jsonrpc/rpcprotocolclient.h \
  jsonrpc/specificationwriter.h \
  jsonrpc/procedure.h \
  jsonrpc/errors.h \
  jsonrpc/version.h \
  jsonrpc/abstractauthenticator.h \
  jsonrpc/specification.h \
  jsonrpc/rpcprotocolserver.h \
  jsonrpc/client.h \
  jsonrpc/abstractrequesthandler.h \
  jsonrpc/specificationparser.h \
  jsonrpc/exception.h \
  jsonrpc/serverconnector.h \
  jsonrpc/server.h \
  jsonrpc/connectors/httpserver.h \
  jsonrpc/connectors/httpclient.h \
  jsonrpc/connectors/mongoose.h
  
   






