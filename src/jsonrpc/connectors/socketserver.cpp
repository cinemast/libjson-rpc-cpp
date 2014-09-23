#include "socketserver.h"

#include <cstring>
#include <vector>

namespace jsonrpc {

    void CloseConnection(const SocketServer::Connection* connection) {
      shutdown(connection->socket, BOTH_DIRECTION);
      closesocket(connection->socket);
      threadJoin(connection->thread);
    }

    void CloseAllConnections(std::vector<SocketServer::Connection*>& clients) {
      for(std::vector<SocketServer::Connection*>::iterator it = clients.begin(); it != clients.end(); ++it ) {
        CloseConnection(*it);
        delete *it;
      }
      clients.clear();
    }

    void CloseFinishedConnections(std::vector<SocketServer::Connection*>& clients) {
      std::vector<SocketServer::Connection*>::iterator it = clients.begin();
      while (it != clients.end()) {
          if ((*it)->finished) {
            CloseConnection(*it);
            delete *it;
            it = clients.erase(it);
          }
          else
            ++it;
      }
    }

    void CloseOldestConnection(std::vector<SocketServer::Connection*>& clients) {
      CloseConnection(clients.front());
      clients.erase(clients.begin());
    }


  SocketServer::SocketServer(const std::string& port, const int type, const int pool) throw (JsonRpcException) :
    AbstractServerConnector(),
    host_info_(NULL),
    shutdown_(false),
    poolSize_(pool)
  {
    struct addrinfo hint;
    memset((char*)&hint, 0, sizeof(addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    int status = getaddrinfo(NULL, port.c_str(), &hint, &host_info_);
    CHECK(status);
    return;
error:
    throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR);
  }

  SocketServer::~SocketServer()
  {
    if (host_info_ != NULL)
      freeaddrinfo(host_info_);
    host_info_ = NULL;

  }

  bool SocketServer::StartListening()
  {
    CreateSocket();
    shutdown_ = false;
    return threadCreate(&server_thread_, (ThreadStartRoutine)HandleConnections, this) == 0;
  }

  bool SocketServer::StopListening()
  {
    shutdown_ = true;
    CloseSocket();
    threadJoin(server_thread_);
    return true;
  }

  bool SocketServer::SendResponse( const std::string& response,
                                void* addInfo)
  {
    Connection* connection = (Connection*)addInfo;
    return send(connection->socket, response.c_str(), response.length(), 0) == response.length();
  }

 THREAD_ROUTINE_RETURN SocketServer::HandleConnections(void* data)
 {
    SocketServer* server = (SocketServer*) data;
    std::vector<SocketServer::Connection*> clients;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(sockaddr_storage);
    MutexHandle lock;
    mutexCreate(&lock);
    int client_socket;

    while (!server->shutdown_) {
      if ((client_socket = accept(server->socket_, (struct sockaddr*)&client_addr, &addr_size)) > 0) {
        clients.push_back(new jsonrpc::SocketServer::Connection());
        clients.back()->socket = client_socket;
        clients.back()->pserver = server;
        clients.back()->plock_server = &lock;
        CloseFinishedConnections(clients);
        if (clients.size() > server->poolSize_) {
          CloseOldestConnection(clients);
        }
        threadCreate(&clients.back()->thread, (ThreadStartRoutine)ConnectionHandler, clients.back());
      }
    }
    CloseAllConnections(clients);
    mutexDestroy(&lock);
    return 0;
  }

  THREAD_ROUTINE_RETURN SocketServer::ConnectionHandler(void* data)
  {
    Connection* connection = (Connection*)data;
    const int MAX_SIZE = 5000;
    char client_message[MAX_SIZE];
    int read_size;
    std::string request;
    connection->finished = false;
    while((read_size = recv(connection->socket , client_message , MAX_SIZE, 0)) > 0) {
      client_message[read_size] = '\0';
      request.assign(client_message);
      mutexLock(connection->plock_server);
      try {
        connection->pserver->OnRequest(request, connection);
      } catch (...) {
      }
      mutexUnlock(connection->plock_server);
      memset(client_message, 0, MAX_SIZE);
    }
    connection->finished = true;
    return 0;
  }

  void SocketServer::CreateSocket() throw (JsonRpcException) {
    char yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK_SOCKET(socket_);
    CHECK_STATUS(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)));
    CHECK_STATUS(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK_STATUS(listen(socket_, poolSize_));
    return;
error:
    CloseSocket();
    throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR);
  }

  void SocketServer::CloseSocket() {
    if (socket_ != -1) {
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
    }
    socket_ = -1;
  }

};
