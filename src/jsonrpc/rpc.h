/**
 * @file json-rpc.h
 * @date 02.08.2011
 * @author Peter Spiess-Knafl
 * @brief json-rpc.h
 *
 * This file is meant to include all necessary .h files of this framework.
 */

#ifndef JSONRPCCPP_H_
#define JSONRPCCPP_H_

#ifdef _WIN32
  // We want to use WinSock2 so we don't want to include WinSock when include Windows.h
  #define _WINSOCKAPI_
  #include <windows.h>
  #ifdef __INTIME__
    // INtime require that windows.h is included before iwin32.h and rt.h
    // as rt.h is included by some standard library includes we ensure this
    // order is followed
    #include <iwin32.h>
  #endif
#endif

#include "server.h"
#include "client.h"

//For error handling and catching Exceptions.
#include "exception.h"

#ifdef HTTP_CONNECTOR
  #include "connectors/httpserver.h"
  #include "connectors/httpclient.h"
#endif

#ifdef SOCKET_CONNECTOR
  #include "connectors/socketserver.h"
  #include "connectors/socketclient.h"
#endif


#include "specificationparser.h"
#include "specificationwriter.h"

#endif /* JSONRPCCPP_H_ */
