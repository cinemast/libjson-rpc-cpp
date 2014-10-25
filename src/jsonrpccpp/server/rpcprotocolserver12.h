/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserver12.h
 * @date    10/25/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_RPCPROTOCOLSERVER12_H
#define JSONRPC_RPCPROTOCOLSERVER12_H

#include "abstractprotocolhandler.h"
#include "rpcprotocolserverv1.h"
#include "rpcprotocolserverv2.h"

namespace jsonrpc {

    class RpcProtocolServer12 : public AbstractProtocolHandler
    {
        public:
            RpcProtocolServer12(IProcedureInvokationHandler &handler);

            virtual void AddProcedure(Procedure& procedure);

            void HandleJsonRequest(const Json::Value& request, Json::Value& response);
            bool ValidateRequestFields(const Json::Value &val);
            void WrapResult(const Json::Value& request, Json::Value& response, Json::Value& retValue);
            void WrapError(const Json::Value& request, int code, const std::string &message, Json::Value& result);
            procedure_t GetRequestType(const Json::Value& request);

        private:
            RpcProtocolServerV1 rpc1;
            RpcProtocolServerV2 rpc2;

            AbstractProtocolHandler& GetHandler(const Json::Value& request);
    };

} // namespace jsonrpc

#endif // JSONRPC_RPCPROTOCOLSERVER12_H
