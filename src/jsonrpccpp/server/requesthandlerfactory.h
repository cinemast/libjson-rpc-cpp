/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    requesthandlerfactory.h
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_REQUESTHANDLERFACTORY_H
#define JSONRPC_REQUESTHANDLERFACTORY_H

#include "abstractprotocolhandler.h"

namespace jsonrpc {

    typedef enum {JSONRPC_V1, JSONRPC_V2, JSONRPC_V1V2} requesthandler_t;

    class RequestHandlerFactory
    {
        public:
            static IProtocolHandler* createProtocolHandler(requesthandler_t type, IProcedureInvokationHandler& handler);
    };

} // namespace jsonrpc

#endif // JSONRPC_REQUESTHANDLERFACTORY_H
