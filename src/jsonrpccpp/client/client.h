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

#include "iclientconnector.h"
#include "batchcall.h"
#include "batchresponse.h"

#ifdef __linux
#include <jsoncpp/json/json.h>
#else
#include <json/json.h>
#endif

#include <vector>
#include <map>

namespace jsonrpc
{
    class RpcProtocolClient;

    typedef enum {JSONRPC_CLIENT_V1, JSONRPC_CLIENT_V2} clientVersion_t;

    class Client
    {
        public:
            Client(IClientConnector &connector, clientVersion_t version = JSONRPC_CLIENT_V2);
            virtual ~Client();

            void        CallMethod          (const std::string &name, const Json::Value &paramter, Json::Value& result) throw (JsonRpcException);
            Json::Value CallMethod          (const std::string &name, const Json::Value &paramter) throw (JsonRpcException);

            void           CallProcedures      (const BatchCall &calls, BatchResponse &response) throw (JsonRpcException);
            BatchResponse  CallProcedures      (const BatchCall &calls) throw (JsonRpcException);

            void        CallNotification    (const std::string& name, const Json::Value& paramter) throw (JsonRpcException);

        private:
           IClientConnector  &connector;
           RpcProtocolClient *protocol;

    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_CLIENT_H_ */
