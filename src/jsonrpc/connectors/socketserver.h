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

#include "threads.h"
#include "socket.h"

#include <string>
#include <exception>

#include "../serverconnector.h"
#include "../exception.h"


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
          ThreadHandle thread;
          MutexHandle *plock_server;
          SocketServer* pserver;
          bool finished;
      };
    private:
      ThreadHandle server_thread_;

      int socket_;
      struct addrinfo* host_info_;
      bool shutdown_;
      unsigned int poolSize_;

      void CreateSocket() throw (JsonRpcException);
      void CloseSocket();


      static THREAD_ROUTINE_RETURN ConnectionHandler(void* connection);
      static THREAD_ROUTINE_RETURN HandleConnections(void* server);


  };

} // namespace jsonrpc

#endif // JSONRPC_SOCKET_SERVER_H_
