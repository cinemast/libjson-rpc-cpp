/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserverv2.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_RPCPROTOCOLSERVERV2_H_
#define JSONRPC_CPP_RPCPROTOCOLSERVERV2_H_

#include <string>
#include <vector>
#include <map>

#include <jsonrpccpp/common/specificationparser.h>
#include "abstractprotocolhandler.h"

#define KEY_REQUEST_METHODNAME  "method"
#define KEY_REQUEST_VERSION     "jsonrpc"
#define KEY_REQUEST_ID          "id"
#define KEY_REQUEST_PARAMETERS  "params"
#define KEY_RESPONSE_ERROR      "error"
#define KEY_RESPONSE_RESULT     "result"
#define KEY_AUTHENTICATION      "auth"
#define JSON_RPC_VERSION        "2.0"

namespace jsonrpc
{
    class RpcProtocolServerV2 : public AbstractProtocolHandler
    {
        public:
            RpcProtocolServerV2(IProcedureInvokationHandler &handler);

            virtual void HandleRequest(const std::string& request, std::string& retValue);

        private:

            void HandleSingleRequest(Json::Value& request, Json::Value& response);
            void HandleBatchRequest(Json::Value& requests, Json::Value& response);

            int ValidateRequest(const Json::Value &val);
            bool ValidateRequestFields(const Json::Value &val);

            /**
             * @pre the request must be a valid request
             * @param request - the request Object compliant to Json-RPC 2.0
             * @param retValue - a reference to an object which will hold the returnValue afterwards.
             *
             * after calling this method, the requested Method will be executed.
             * It is important, that this method only gets called once per request.
             */
            void ProcessRequest(const Json::Value &request,
                    Json::Value &retValue);
    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_RPCPROTOCOLSERVERV2_H_ */
