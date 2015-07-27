/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    linuxtcpsocketclient.cpp
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "linuxtcpsocketclient.h"
#include <string.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <errno.h>
#include <cstring>

#define BUFFER_SIZE 64
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR

using namespace jsonrpc;
using namespace std;

LinuxTcpSocketClient::LinuxTcpSocketClient(const std::string& ipToConnect, const unsigned int &port) :
	TcpSocketClientPrivate(),
	ipToConnect(ipToConnect),
	port(port)
{
}

LinuxTcpSocketClient::~LinuxTcpSocketClient()
{
}

void LinuxTcpSocketClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
	sockaddr_in address;
	int socket_fd, nbytes;
	char buffer[BUFFER_SIZE];
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		string message = "socket() failed";
                int err = errno;
                switch(err) {
                    case EACCES:
                    case EAFNOSUPPORT:
                    case EINVAL:
                    case EMFILE:
                    case ENOBUFS:
                    case ENOMEM:
                    case EPROTONOSUPPORT:
                        message = strerror(err);
                        break;
                }
                cerr << message << endl;
                throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
	}

	memset(&address, 0, sizeof(sockaddr_in));

	address.sin_family = AF_INET;
	inet_aton(this->ipToConnect.c_str(), &(address.sin_addr));
	address.sin_port = htons(this->port);

	if(connect(socket_fd, (struct sockaddr *) &address,  sizeof(sockaddr_in)) != 0) {
		string message = "connect() failed";
                int err = errno;
		switch(err) {
                    case EACCES:
                    case EPERM:
                    case EADDRINUSE:
                    case EAFNOSUPPORT:
                    case EAGAIN:
                    case EALREADY:
                    case EBADF:
                    case ECONNREFUSED:
                    case EFAULT:
                    case EINPROGRESS:
                    case EINTR:
                    case EISCONN:
                    case ENETUNREACH:
                    case ENOTSOCK:
                    case ETIMEDOUT:
                        message = strerror(err);
                        break;
                }
                cerr << message << endl;
                throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
	}

	bool fullyWritten = false;
	string toSend = message;
	do {
		ssize_t byteWritten = send(socket_fd, toSend.c_str(), toSend.size(), 0);
		if(byteWritten == -1) {
                    string message = "send() failed";
                    int err = errno;
                    switch(err) {
                        case EACCES:
                        case EWOULDBLOCK:
                        case EBADF:
                        case ECONNRESET:
                        case EDESTADDRREQ:
                        case EFAULT:
                        case EINTR:
                        case EINVAL:
                        case EISCONN:
                        case EMSGSIZE:
                        case ENOBUFS:
                        case ENOMEM:
                        case ENOTCONN:
                        case ENOTSOCK:
                        case EOPNOTSUPP:
                        case EPIPE:
                            message = strerror(err);
                            break;
                    }
                    close(socket_fd);
                    cerr << message << endl;
                    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
                }
                else if(byteWritten < toSend.size()) {
			int len = toSend.size() - byteWritten;
			toSend = toSend.substr(byteWritten + sizeof(char), len);
		}
		else
			fullyWritten = true;
	} while(!fullyWritten);

	do {
		nbytes = recv(socket_fd, buffer, BUFFER_SIZE, 0);
                if(nbytes == -1) {
                    string message = "recv() failed";
                    int err = errno;
                    switch(err) {
                        case EWOULDBLOCK:
                        case EBADF:
                        case ECONNRESET:
                        case EFAULT:
                        case EINTR:
                        case EINVAL:
                        case ENOMEM:
                        case ENOTCONN:
                        case ENOTSOCK:
                            message = strerror(err);
                            break;
                    }
                    close(socket_fd);
                    cerr << message << endl;
                    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
                }
                else {
                    string tmp;
                    tmp.append(buffer, nbytes);
                    result.append(buffer,nbytes);
                }
	} while(result.find(DELIMITER_CHAR) == string::npos);

	close(socket_fd);
}
