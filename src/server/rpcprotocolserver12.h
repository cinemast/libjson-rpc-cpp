#pragma once

#include "abstractprotocolhandler.h"
#include "rpcprotocolserverv1.h"
#include "rpcprotocolserverv2.h"

namespace jsonrpc {

    class RpcProtocolServer12 : public AbstractProtocolHandler
    {
        public:
            RpcProtocolServer12(IProcedureInvokationHandler &handler);
            virtual ~RpcProtocolServer12();
            
            void AddProcedure(const Procedure &procedure);

            bool ValidateRequestFields(const Json::Value &request);
            void HandleJsonRequest(const Json::Value &request, Json::Value &response);
            void WrapResult(const Json::Value& request, Json::Value& response, Json::Value& retValue);
            void WrapError(const Json::Value& request, int code, const std::string &message, Json::Value& result);
            procedure_t GetRequestType(const Json::Value& request);

        private:
            RpcProtocolServerV1 rpc1;
            RpcProtocolServerV2 rpc2;

            AbstractProtocolHandler& GetHandler(const Json::Value& request);
    };

} // namespace jsonrpc