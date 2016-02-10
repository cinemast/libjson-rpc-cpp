/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.h
 * @date    10.01.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_SERVER_H_
#define JSONRPC_CPP_SERVER_H_

#include "iserverconnector.h"
#include <jsonrpccpp/client/client.h>
#include <jsonrpccpp/client/batchcall.h>
#include <jsonrpccpp/client/batchresponse.h>
#include <jsonrpccpp/common/jsonparser.h>

#include <vector>
#include <map>

namespace jsonrpc
{

    class Server
    {
        public:
            Server(IServerConnector &connector, clientVersion_t version = JSONRPC_CLIENT_V2);
            virtual ~Server();

            void        CallMethod          (const std::string &name, const Json::Value &parameter, Json::Value& result, unsigned int uiConnectionId = 0) throw (JsonRpcException);
            Json::Value CallMethod          (const std::string &name, const Json::Value &parameter, unsigned int uiConnectionId = 0) throw (JsonRpcException);

            void           CallProcedures      (const BatchCall &calls, BatchResponse &response, unsigned int uiConnectionId = 0) throw (JsonRpcException);
            BatchResponse  CallProcedures      (const BatchCall &calls, unsigned int uiConnectionId = 0) throw (JsonRpcException);

            void        CallNotification    (const std::string& name, const Json::Value& parameter, unsigned int uiConnectionId = 0) throw (JsonRpcException);

        private:
           IServerConnector  &connector;
           RpcProtocolClient *protocol;

    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_SERVER_H_ */
