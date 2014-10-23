/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserverv1.h
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_RPCPROTOCOLSERVERV1_H
#define JSONRPC_CPP_RPCPROTOCOLSERVERV1_H

#include "abstractprotocolhandler.h"

namespace jsonrpc {

    class RpcProtocolServerV1 : public AbstractProtocolHandler
    {
        public:
            RpcProtocolServerV1(IProcedureInvokationHandler &handler);

            virtual void HandleRequest(const std::string& request, std::string& retValue);
    };

} // namespace jsonrpc

#endif // JSONRPC_CPP_RPCPROTOCOLSERVERV1_H
