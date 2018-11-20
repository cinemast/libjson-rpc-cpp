#pragma once

#include "../connector/iclientconnectionhandler.h"
#include "../exception.h"
#include "iprocedureinvokationhandler.h"
#include "procedure.h"

#include <map>
#include <string>

#define KEY_REQUEST_METHODNAME  "method"
#define KEY_REQUEST_ID          "id"
#define KEY_REQUEST_PARAMETERS  "params"
#define KEY_RESPONSE_ERROR      "error"
#define KEY_RESPONSE_RESULT     "result"

namespace jsonrpc {

    class AbstractProtocolHandler : public IClientConnectionHandler
    {
        public:
            AbstractProtocolHandler(IProcedureInvokationHandler &handler);
            virtual ~AbstractProtocolHandler();

            virtual void HandleRequest(const std::string& request, std::string& retValue);
            virtual void AddProcedure(const Procedure& procedure);

            virtual void HandleJsonRequest(const Json::Value& request, Json::Value& response) = 0;
            virtual bool ValidateRequestFields(const Json::Value &val) = 0;
            virtual void WrapResult(const Json::Value& request, Json::Value& response, Json::Value& retValue) = 0;
            virtual void WrapError(const Json::Value& request, int code, const std::string &message, Json::Value& result) = 0;
            virtual procedure_t GetRequestType(const Json::Value& request) = 0;

        protected:
            IProcedureInvokationHandler &handler;
            std::map<std::string, Procedure> procedures;

            void ProcessRequest(const Json::Value &request, Json::Value &retValue);
            ExceptionCode ValidateRequest(const Json::Value &request);

            static std::string GetErrorMessage(ExceptionCode code);
    };

} // namespace jsonrpc
