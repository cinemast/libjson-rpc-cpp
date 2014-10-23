/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    requesthandlerfactory.cpp
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "requesthandlerfactory.h"
#include "rpcprotocolserverv1.h"
#include "rpcprotocolserverv2.h"

using namespace jsonrpc;

AbstractProtocolHandler *RequestHandlerFactory::createProtocolHandler(requesthandler_t type, IProcedureInvokationHandler &handler)
{
    AbstractProtocolHandler* result = NULL;
    switch (type) {
        case JSONRPC_V1:
            result = new RpcProtocolServerV1(handler);
            break;
        case JSONRPC_V2:
            result = new RpcProtocolServerV2(handler);
            break;
        case JSONRPC_V1V2:
            break;
    }
    return result;
}
