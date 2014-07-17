#include <string>
#include <cstring>
#include <unistd.h>

#include "socketclient.h"


namespace jsonrpc {

  SocketClient::SocketClient(const std::string& url, const std::string& port, const int type) throw (JsonRpcException) :
      AbstractClientConnector()
  {
    int p = std::stoi(port);
    int family = AF_INET;
    server_info_.sin_port = htons(p);
    server_info_.sin_family = family;
    server_info_.sin_addr.s_addr = inet_addr(url.c_str());
    if (server_info_.sin_addr.s_addr == -1) {
      struct hostent *he;
      struct in_addr **addr_list;

      CHECK_PTR((he = gethostbyname(url.c_str())));

      addr_list = (struct in_addr **) he->h_addr_list;

      for(int i = 0; addr_list[i] != NULL; i++) {
        server_info_.sin_addr = *addr_list[i];
        break;
      }
    }

    socket_ = socket(family, type, 0);
    CHECK(socket_);
    return;

error:
    if (socket_ != -1)
      close(socket_);
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR);
  }

  SocketClient::~SocketClient()
  {
      close(socket_);
  }

  void SocketClient::SendMessage(const std::string& message, std::string& result) throw (JsonRpcException) {
    int read_size;
    const int MAX_SIZE = 5000;
    char server_message[MAX_SIZE];
    memset(server_message, 0, MAX_SIZE);
    connect(socket_, (struct sockaddr *)&server_info_ , sizeof(server_info_));
    CHECK(write(socket_, message.c_str(), message.length()));
    CHECK((read_size = read(socket_, server_message, MAX_SIZE)));
    server_message[read_size] = '\0';
    result.assign(server_message);
    return;
error:
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR);
  }

}
