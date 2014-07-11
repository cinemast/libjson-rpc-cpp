#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <vector>

#include "socketserver.h"

namespace jsonrpc {

    void CloseConnection(const SocketServer::Connection* connection) {
      shutdown(connection->socket, SHUT_RDWR);
      close(connection->socket);
      pthread_join(connection->thread, NULL);
      delete connection;
    }

    void CloseAllConnections(std::vector<SocketServer::Connection*>& clients) {
        for(std::vector<SocketServer::Connection*>::iterator it = clients.begin(); it != clients.end(); ++it ) {
          CloseConnection(*it);
        }
        clients.clear();
    }

    void CloseFinishedConnections(std::vector<SocketServer::Connection*>& clients) {
        std::vector<SocketServer::Connection*>::iterator it = clients.begin();
        while (it != clients.end()) {
            if ((*it)->finished) {
                CloseConnection(*it);
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
    CHECK(getaddrinfo(NULL, port.c_str(), &hint, &host_info_));
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
    return pthread_create(&server_thread_, NULL, HandleConnections, this) == 0;
  }

  bool SocketServer::StopListening()
  {
    shutdown_ = true;
    CloseSocket();
    pthread_join(server_thread_, NULL);
    return true;
  }

  bool SocketServer::SendResponse( const std::string& response,
                                void* addInfo)
  {
    Connection* connection = (Connection*)addInfo;
    return write(connection->socket, response.c_str(), response.length()) == response.length();
  }

  void* SocketServer::HandleConnections(void* data) {
    SocketServer* server = (SocketServer*) data;
    std::vector<SocketServer::Connection*> clients;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(sockaddr_storage);
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    int client_socket;

    while (!server->shutdown_) {
      if ((client_socket = accept(server->socket_, (struct sockaddr*)&client_addr, &addr_size)) > 0) {
        clients.push_back(new Connection());
        clients.back()->socket = client_socket;
        clients.back()->pserver = server;
        clients.back()->plock_server = &lock;
        CloseFinishedConnections(clients);
        if (clients.size() > server->poolSize_) {
          CloseOldestConnection(clients);
        }
        pthread_create(&clients.back()->thread, NULL, ConnectionHandler, clients.back());
      }
    }
    CloseAllConnections(clients);
    pthread_mutex_destroy(&lock);
    return 0;
  }

  void* SocketServer::ConnectionHandler(void* data) {
    Connection* connection = (Connection*)data;
    const int MAX_SIZE = 5000;
    char client_message[MAX_SIZE];
    int read_size;
      std::string request;
    connection->finished = false;
    while((read_size = read(connection->socket , client_message , MAX_SIZE)) > 0) {
      client_message[read_size] = '\0';
      request.assign(client_message);
      pthread_mutex_lock(connection->plock_server);
      try {
        connection->pserver->OnRequest(request, connection);
      } catch (...) {
      }
      pthread_mutex_unlock(connection->plock_server);
      memset(client_message, 0, MAX_SIZE);
    }
    connection->finished = true;
    return 0;
  }

  void SocketServer::CreateSocket() {
    int yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK(socket_);
    CHECK(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)));
    CHECK(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK(listen(socket_, poolSize_));
    return;
error:
    CloseSocket();
  }

  void SocketServer::CloseSocket() {
    if (socket_ != -1) {
      shutdown(socket_, SHUT_RDWR);
      close(socket_);
    }
    socket_ = -1;
  }

};
