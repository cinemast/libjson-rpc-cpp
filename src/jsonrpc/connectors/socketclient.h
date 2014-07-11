/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    socketclient.h
 * @date    12.07.2014
 * @author  Bertrand Cachet <bertrand.cachet@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#ifndef JSONRPC_CONNECTORS_SOCKETCLIENT_H
#define JSONRPC_CONNECTORS_SOCKETCLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../clientconnector.h"
#include "../exception.h"
#include "socket.h"

namespace jsonrpc
{
  class SocketClient : public AbstractClientConnector
  {
    public:
      SocketClient(const std::string& url = "localhost", const std::string& port = "8080", const int type = SOCK_STREAM) throw (JsonRpcException);
      virtual ~SocketClient();

      virtual void SendMessage(const std::string& message, std::string& result) throw (JsonRpcException);
    private:
      int socket_;
      struct sockaddr_in server_info_;
  };
} // namespace jsonrpc

#endif /* JSONRPC_CONNECTORS_SOCKETCLIENT_H*/
