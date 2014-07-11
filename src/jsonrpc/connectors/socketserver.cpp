#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <vector>

#include "socketserver.h"

int set_socket_receive_timeout_ms(const int socketfd, const int timeout_ms) {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms*1000;
    return setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval));
}

namespace jsonrpc {
    void CloseAllConnections(std::vector<SocketServer::Connection>& clients) {
        for(std::vector<SocketServer::Connection>::const_iterator it = clients.cbegin(); it != clients.cend(); ++it ) {
            close(it->socket);
            pthread_join(it->thread, NULL);
        }
        clients.clear();
    }
    
    void CloseFinishedConnections(std::vector<SocketServer::Connection>& clients) {
        std::vector<SocketServer::Connection>::const_iterator it = clients.cbegin();
        while (it != clients.cend()) {
            if (it->finished) {
                close(it->socket);
                pthread_join(it->thread, NULL);
                it = clients.erase(it);
            }
            else
                ++it;
        }
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
    StopListening();
    if (host_info_ != NULL)
      freeaddrinfo(host_info_);
    host_info_ = NULL;

  }

  bool SocketServer::StartListening()
  {
    CreateSocket();
    return pthread_create(&server_thread_, NULL, HandleConnections, this) == 0;
  }

  bool SocketServer::StopListening()
  {
    shutdown_ = true;
    CloseSocket();
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
    std::vector<SocketServer::Connection> clients;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(sockaddr_storage);
    server->shutdown_ = false;
    while (!server->shutdown_) {
      Connection client;
      if ((client.socket = accept(server->socket_, (struct sockaddr*)&client_addr, &addr_size)) > 0) {
        client.pserver = server;
//        set_socket_receive_timeout_ms(client.socket, 500);
        CloseFinishedConnections(clients);
        if (clients.size() >= server->poolSize_) {
            close(clients.front().socket);
            pthread_join(clients.front().thread, NULL);
            clients.erase(clients.begin());
        }
        clients.push_back(client);
        pthread_create(&clients.back().thread, NULL, ConnectionHandler, &clients.back());
      }
    }
    CloseAllConnections(clients);
    close(server->socket_);
    return 0;
  }

  void* SocketServer::ConnectionHandler(void* data) {
    Connection* connection = (Connection*)data;
    const int MAX_SIZE = 5000;
    char client_message[MAX_SIZE];
    int read_size;
    connection->finished = false;
    while((read_size = read(connection->socket , client_message , MAX_SIZE)) > 0) {
      client_message[read_size] = '\0';
      std::string request(client_message);
      connection->pserver->OnRequest(request, connection);
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
    if (socket_ != -1)
      close(socket_);
    socket_ = -1;
  }

};
