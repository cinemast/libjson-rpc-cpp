#pragma once

#include "../jsonparser.h"
#include "../connector/iclientconnector.h"
#include "batchcall.h"
#include "batchresponse.h"


#include <vector>
#include <map>

namespace jsonrpc
{
    class RpcProtocolClient;

    typedef enum {JSONRPC_CLIENT_V1, JSONRPC_CLIENT_V2} clientVersion_t;

    class Client
    {
        public:
            Client(IClientConnector &connector, clientVersion_t version = JSONRPC_CLIENT_V2, bool omitEndingLineFeed = false);
            virtual ~Client();

            void        CallMethod          (const std::string &name, const Json::Value &parameter, Json::Value& result) ;
            Json::Value CallMethod          (const std::string &name, const Json::Value &parameter) ;

            void           CallProcedures      (const BatchCall &calls, BatchResponse &response) ;
            BatchResponse  CallProcedures      (const BatchCall &calls) ;

            void        CallNotification    (const std::string& name, const Json::Value& parameter) ;

        private:
           IClientConnector  &connector;
           RpcProtocolClient *protocol;

    };

} /* namespace jsonrpc */
