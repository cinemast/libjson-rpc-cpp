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
		cerr << "Socket failed" << endl;
		throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
	}

	memset(&address, 0, sizeof(sockaddr_in));

	address.sin_family = AF_INET;
	inet_aton(this->ipToConnect.c_str(), &(address.sin_addr));
	address.sin_port = htons(this->port);

	if(connect(socket_fd, (struct sockaddr *) &address,  sizeof(sockaddr_in)) != 0) {
		cerr << "connect failed" << endl;
		throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
	}

	bool fullyWritten = false;
	string toSend = message;
	do {
		ssize_t byteWritten = send(socket_fd, toSend.c_str(), toSend.size(), 0);
		if(byteWritten < toSend.size()) {
			int len = toSend.size() - byteWritten;
			toSend = toSend.substr(byteWritten + sizeof(char), len);
		}
		else
			fullyWritten = true;
	} while(!fullyWritten);

	do {
		nbytes = recv(socket_fd, buffer, BUFFER_SIZE, 0);
		string tmp;
		tmp.append(buffer, nbytes);
		result.append(buffer,nbytes);

	} while(result.find(DELIMITER_CHAR) == string::npos);

	close(socket_fd);
}
