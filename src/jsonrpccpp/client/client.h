/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.h
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_CLIENT_H_
#define JSONRPC_CPP_CLIENT_H_

#include "abstractclientconnector.h"
#include "rpcprotocolclient.h"
#include "batchcall.h"
#include <jsoncpp/json/json.h>

#include <vector>
#include <map>

namespace jsonrpc
{
    class Client
    {
        public:
            Client(AbstractClientConnector &connector);

            void        CallMethod          (const std::string &name, const Json::Value &paramter, Json::Value& result) throw (JsonRpcException);
            Json::Value CallMethod          (const std::string &name, const Json::Value &paramter) throw (JsonRpcException);

            void                    CallProcedures      (const BatchCall &calls, batchProcedureResponse &response) throw (JsonRpcException);
            batchProcedureResponse  CallProcedures      (const BatchCall &calls) throw (JsonRpcException);

            void        CallNotification    (const std::string& name, const Json::Value& paramter) throw (JsonRpcException);
            void        CallNotifications   (const batchProcedureCall_t &calls) throw (JsonRpcException);

        private:
           AbstractClientConnector &connector;
           RpcProtocolClient protocol;

    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_CLIENT_H_ */
