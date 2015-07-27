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
                switch(errno) {
                    case EACCES:
                        message = strerror(EACCES);
                        break;
                    case EAFNOSUPPORT:
                        message = strerror(EAFNOSUPPORT);
                        break;
                    case EINVAL:
                        message = strerror(EINVAL);
                        break;
                    case EMFILE:
                        message = strerror(EMFILE);
                        break;
                    case ENOBUFS:
                        message = strerror(ENOBUFS);
                        break;
                    case ENOMEM:
                        message = strerror(ENOMEM);
                        break;
                    case EPROTONOSUPPORT:
                        message = strerror(EPROTONOSUPPORT);
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
		switch(errno) {
                    case EACCES:
                        message = strerror(EACCES);
                        break;
                    case EPERM:
                        message = strerror(EPERM);
                        break;
                    case EADDRINUSE:
                        message = strerror(EADDRINUSE);
                        break;
                    case EAFNOSUPPORT:
                        message = strerror(EAFNOSUPPORT);
                        break;
                    case EAGAIN:
                        message = strerror(EAGAIN);
                        break;
                    case EALREADY:
                        message = strerror(EALREADY);
                        break;
                    case EBADF:
                        message = strerror(EBADF);
                        break;
                    case ECONNREFUSED:
                        message = strerror(ECONNREFUSED);
                        break;
                    case EFAULT:
                        message = strerror(EFAULT);
                        break;
                    case EINPROGRESS:
                        message = strerror(EINPROGRESS);
                        break;
                    case EINTR:
                        message = strerror(EINTR);
                        break;
                    case EISCONN:
                        message = strerror(EISCONN);
                        break;
                    case ENETUNREACH:
                        message = strerror(ENETUNREACH);
                        break;
                    case ENOTSOCK:
                        message = strerror(ENOTSOCK);
                        break;
                    case ETIMEDOUT:
                        message = strerror(ETIMEDOUT);
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
                    switch(errno) {
                        case EACCES:
                            message = strerror(EACCES);
                            break;
                        case EWOULDBLOCK:
                            message = strerror(EWOULDBLOCK);
                            break;
                        case EBADF:
                            message = strerror(EBADF);
                            break;
                        case ECONNRESET:
                            message = strerror(ECONNRESET);
                            break;
                        case EDESTADDRREQ:
                            message = strerror(EDESTADDRREQ);
                            break;
                        case EFAULT:
                            message = strerror(EFAULT);
                            break;
                        case EINTR:
                            message = strerror(EINTR);
                            break;
                        case EINVAL:
                            message = strerror(EINVAL);
                            break;
                        case EISCONN:
                            message = strerror(EISCONN);
                            break;
                        case EMSGSIZE:
                            message = strerror(EMSGSIZE);
                            break;
                        case ENOBUFS:
                            message = strerror(ENOBUFS);
                            break;
                        case ENOMEM:
                            message = strerror(ENOMEM);
                            break;
                        case ENOTCONN:
                            message = strerror(ENOTCONN);
                            break;
                        case ENOTSOCK:
                            message = strerror(ENOTSOCK);
                            break;
                        case EOPNOTSUPP:
                            message = strerror(EOPNOTSUPP);
                            break;
                        case EPIPE:
                            message = strerror(EPIPE);
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
                    switch(errno) {
                        case EWOULDBLOCK:
                            message = strerror(EWOULDBLOCK);
                            break;
                        case EBADF:
                            message = strerror(EBADF);
                            break;
                        case ECONNRESET:
                            message = strerror(ECONNRESET);
                            break;
                        case EFAULT:
                            message = strerror(EFAULT);
                            break;
                        case EINTR:
                            message = strerror(EINTR);
                            break;
                        case EINVAL:
                            message = strerror(EINVAL);
                            break;
                        case ENOMEM:
                            message = strerror(ENOMEM);
                            break;
                        case ENOTCONN:
                            message = strerror(ENOTCONN);
                            break;
                        case ENOTSOCK:
                            message = strerror(ENOTSOCK);
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
