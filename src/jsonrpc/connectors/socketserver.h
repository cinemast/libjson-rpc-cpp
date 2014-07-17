/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    socketserver.h
 * @date    12.07.2014
 * @author  Bertrand Cachet <bertrand.cachet@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#ifndef JSONRPC_SOCKET_SERVER_H_
#define JSONRPC_SOCKET_SERVER_H_

#include <string>
#include <exception>

#include "../serverconnector.h"
#include "../exception.h"
#include "socket.h"

namespace jsonrpc {
    
  class SocketServer : public AbstractServerConnector {
   public:
      SocketServer(const std::string& port = "8080", const int type = SOCK_STREAM, const int pool = 1) throw (JsonRpcException);
      ~SocketServer();

      bool StartListening();
      bool StopListening();

      bool SendResponse(const std::string& response, void* addInfo = NULL);
      
      struct Connection {
          int socket;
          pthread_t thread;
          SocketServer* pserver;
          bool finished;
      };
    private:

      pthread_t server_thread_;

      int socket_;
      struct addrinfo* host_info_;
      bool shutdown_;
      int poolSize_;

      void CreateSocket();
      void CloseSocket();
//      void CloseAllConnections();
//      void CloseOldConnections();

      static void *ConnectionHandler(void *connection);
      static void *HandleConnections(void* server);

  };

} // namespace jsonrpc

#endif // JSONRPC_SOCKET_SERVER_H_
