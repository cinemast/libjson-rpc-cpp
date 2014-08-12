#include "socketclient.h"

#include <string>
#include <stdlib.h>
#include <string.h>


namespace jsonrpc {

  SocketClient::SocketClient(const std::string& url, const std::string& port, const int type) throw (JsonRpcException) :
      AbstractClientConnector(),
      socket_(-1),
      server_info_(NULL)
  {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    CHECK(getaddrinfo(url.c_str(), port.c_str(), &hints, &server_info_));
    return;
error:
    if (server_info_ != NULL)
      freeaddrinfo(server_info_);
    server_info_ = NULL;
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR);
  }

  SocketClient::~SocketClient()
  {
    shutdown(socket_, BOTH_DIRECTION);
    closesocket(socket_);
    freeaddrinfo(server_info_);
  }

  void SocketClient::SendMessage(const std::string& message, std::string& result) throw (JsonRpcException) {
    int read_size;
    const int MAX_SIZE = 5000;
    char server_message[MAX_SIZE];
    struct addrinfo *ptr;
    if (socket_ == -1) {
      for (ptr = server_info_; ptr != NULL; ptr = ptr->ai_next) {
        socket_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        CHECK_SOCKET(socket_);
        if (connect(socket_, ptr->ai_addr, (int)ptr->ai_addrlen) == 0) {
          break;
        }
        closesocket(socket_);
      }
    }
    memset(server_message, 0, MAX_SIZE);
    CHECK_STATUS(send(socket_, message.c_str(), message.length(), 0));
    CHECK_READSIZE(read_size = recv(socket_, server_message, MAX_SIZE, 0));
    server_message[read_size] = '\0';
    result.assign(server_message);
    return;
error:
    if (socket_ != -1) {
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
    }
    socket_ = -1;
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR);
  }
}
