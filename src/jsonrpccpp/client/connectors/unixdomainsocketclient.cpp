/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    unixdomainsocketclient.cpp
 * @date    11.05.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "unixdomainsocketclient.h"
#include <string>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <iostream>

#define BUFFER_SIZE 64
#define PATH_MAX 108
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR

using namespace jsonrpc;
using namespace std;

UnixDomainSocketClient::UnixDomainSocketClient(const std::string& path)
    : path(path)
{
}

UnixDomainSocketClient::~UnixDomainSocketClient()
{
}

void UnixDomainSocketClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
	sockaddr_un address;
	int socket_fd, nbytes;
	char buffer[BUFFER_SIZE];
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		cerr << "Socket failed" << endl;
		throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
	}

	memset(&address, 0, sizeof(sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, PATH_MAX, this->path.c_str());

	if(connect(socket_fd, (struct sockaddr *) &address,  sizeof(sockaddr_un)) != 0) {
		cerr << "connect failed" << endl;
		throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
	}
	write(socket_fd, message.c_str(), message.size());

	do {
		nbytes = read(socket_fd, buffer, BUFFER_SIZE);
		string tmp;
		tmp.append(buffer, nbytes);
		result.append(buffer,nbytes);

	} while(result.find(DELIMITER_CHAR) == string::npos);

	close(socket_fd);
}
