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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include <cstring>

#define BUFFER_SIZE 64
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR

using namespace jsonrpc;
using namespace std;

LinuxTcpSocketClient::LinuxTcpSocketClient(const std::string& hostToConnect, const unsigned int &port) :
	TcpSocketClientPrivate(),
	hostToConnect(hostToConnect),
	port(port)
{
}

LinuxTcpSocketClient::~LinuxTcpSocketClient()
{
}

void LinuxTcpSocketClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
	int socket_fd = this->Connect();
	char buffer[BUFFER_SIZE];

	bool fullyWritten = false;
	string toSend = message;
	do
	{
		ssize_t byteWritten = send(socket_fd, toSend.c_str(), toSend.size(), 0);
		if(byteWritten == -1)
		{
			string message = "send() failed";
			int err = errno;
			switch(err)
			{
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
			throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
		}
		else if(static_cast<size_t>(byteWritten) < toSend.size())
		{
			int len = toSend.size() - byteWritten;
			toSend = toSend.substr(byteWritten + sizeof(char), len);
		}
		else
			fullyWritten = true;
	} while(!fullyWritten);

	do
	{
		int nbytes = recv(socket_fd, buffer, BUFFER_SIZE, 0);
		if(nbytes == -1)
		{
			string message = "recv() failed";
			int err = errno;
			switch(err)
			{
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
			throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
		}
		else
		{
			string tmp;
			tmp.append(buffer, nbytes);
			result.append(buffer,nbytes);
		}
	} while(result.find(DELIMITER_CHAR) == string::npos);

	close(socket_fd);
}

int LinuxTcpSocketClient::Connect() throw (JsonRpcException)
{
	if(this->IsIpv4Address(this->hostToConnect))
	{
		return this->Connect(this->hostToConnect, this->port);
	}
	else //We were given a hostname
	{
		struct addrinfo *result = NULL;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
        char port[6];
		snprintf(port, 6, "%d", this->port);
		int retval = getaddrinfo(this->hostToConnect.c_str(), port, &hints, &result);
		if(retval != 0)
			throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "Could not resolve hostname.");
		bool foundValidIp = false;
		int socket_fd;
		for(struct addrinfo *temp = result; (temp != NULL) && !foundValidIp; temp = temp->ai_next)
		{
			if(temp->ai_family == AF_INET)
			{
				try
				{
					sockaddr_in* sock = reinterpret_cast<sockaddr_in*>(temp->ai_addr);
					socket_fd = this->Connect(inet_ntoa(sock->sin_addr), ntohs(sock->sin_port));
					foundValidIp = true;
				}
				catch(const JsonRpcException& e)
				{
					foundValidIp = false;
					socket_fd = -1;
				}
				catch(void* p)
				{
					foundValidIp = false;
					socket_fd = -1;
				}
			}
		}

		if(!foundValidIp)
			throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "Hostname resolved but connection was refused on the given port.");

		return socket_fd;
	}
}

int LinuxTcpSocketClient::Connect(const string& ip, const int& port) throw (JsonRpcException)
{
	sockaddr_in address;
	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		string message = "socket() failed";
		int err = errno;
		switch(err)
		{
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
		throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
	}
	memset(&address, 0, sizeof(sockaddr_in));

	address.sin_family = AF_INET;
	inet_aton(ip.c_str(), &(address.sin_addr));
	address.sin_port = htons(port);

	if(connect(socket_fd, (struct sockaddr *) &address,  sizeof(sockaddr_in)) != 0)
	{
		string message = "connect() failed";
		int err = errno;
		switch(err)
		{
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
		throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, message);
	}
	return socket_fd;
}

bool LinuxTcpSocketClient::IsIpv4Address(const std::string& ip)
{
	struct in_addr addr;
	return (inet_aton(ip.c_str(), &addr) != 0);
}
