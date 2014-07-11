#ifndef JSONRPC_CONNECTORS_SOCKET_H_
#define JSONRPC_CONNECTORS_SOCKET_H_

#define CHECK(status) if ((status) < 0) goto error;
#define CHECK_PTR(ptr) if ((ptr) == NULL) goto error;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>



#endif // JSONRPC_CONNECTORS_SOCKET_H_
