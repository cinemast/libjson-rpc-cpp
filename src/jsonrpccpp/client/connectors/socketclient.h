/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    socketclient.h
 * @date    12.07.2014
 * @author  Bertrand Cachet <bertrand.cachet@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#ifndef JSONRPC_CPP_SOCKETCLIENT_H
#define JSONRPC_CPP_SOCKETCLIENT_H


#include "../abstractclientconnector.h"
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/common/helper/socket.h>

namespace jsonrpc
{
    class SocketClient : public AbstractClientConnector
    {
        public:
            SocketClient(const std::string& url = "0.0.0.0", const std::string& port = "8080", const int type = SOCK_STREAM) throw (JsonRpcException);
            virtual ~SocketClient();

            void SendRPCMessage(const std::string& message, std::string& result) throw(JsonRpcException);
        private:
            int socket_;
            struct addrinfo* server_info_;
    };
}

#endif /* JSONRPC_CPP_SOCKETCLIENT_H*/
