#ifndef JSONRPC_CONNECTORS_SOCKET_H_
#define JSONRPC_CONNECTORS_SOCKET_H_

#define CHECK(status) if ((status) != 0) goto error;
#define CHECK_READSIZE(read) if((read) < 0) goto error;

#if defined(_WIN32) && !defined(__INTIME__)
  #include <WinSock2.h>
  #include <ws2tcpip.h>
  #include <io.h>
  static const int BOTH_DIRECTION=SD_BOTH;
  #define CHECK_STATUS(s) if ((s) == SOCKET_ERROR) goto error;
  #define CHECK_SOCKET(s) if ((s) == INVALID_SOCKET) goto error;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  static const int BOTH_DIRECTION=SHUT_RDWR;
  #define CHECK_STATUS(s) if ((s) < 0) goto error;
  #define CHECK_SOCKET(s) if ((s) == -1) goto error;
#ifndef __INTIME__
  #define closesocket(s) close((s))
#endif
#endif

#endif // JSONRPC_CONNECTORS_SOCKET_H_
