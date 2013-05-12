/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.h
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef CLIENT_H_
#define CLIENT_H_

#include "clientconnector.h"
#include "rpcprotocolclient.h"
#include "json/json.h"

#include <vector>
#include <map>

namespace jsonrpc
{
    class Client
    {
        public:
            Client(AbstractClientConnector* connector);
            ~Client();


            void CallMethod(const std::string &name, const Json::Value &paramter, Json::Value& result) throw (JsonRpcException);
            Json::Value CallMethod(const std::string& name, const Json::Value& paramter) throw (JsonRpcException);
            void CallNotification(const std::string& name, const Json::Value& paramter) throw (JsonRpcException);

        private:
           AbstractClientConnector* connector;
           RpcProtocolClient protocol;

    };

} /* namespace jsonrpc */
#endif /* CLIENT_H_ */
