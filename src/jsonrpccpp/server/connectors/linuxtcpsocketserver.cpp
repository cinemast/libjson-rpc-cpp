/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    linuxtcpsocketserver.cpp
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "linuxtcpsocketserver.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <sstream>
#include <iostream>
#include <string>

#include <jsonrpccpp/common/specificationparser.h>

#include <errno.h>

using namespace jsonrpc;
using namespace std;

#define BUFFER_SIZE 64
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR

LinuxTcpSocketServer::LinuxTcpSocketServer(const std::string& ipToBind, const unsigned int &port) :
	TcpSocketServerPrivate(),
	ipToBind(ipToBind),
	port(port),
	running(false)
{
}

bool LinuxTcpSocketServer::StartListening()
{
	if(!this->running)
	{
		//Create and bind socket here.
		//Then launch the listenning loop.
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(this->socket_fd < 0)
		{
			cerr << "socket() failed" << endl;
			return false;
		}

		fcntl(this->socket_fd, F_SETFL, FNDELAY);

		/* start with a clean address structure */
		memset(&(this->address), 0, sizeof(struct sockaddr_in));

		this->address.sin_family = AF_INET;
		inet_aton(this->ipToBind.c_str(), &(this->address.sin_addr));
		this->address.sin_port = htons(this->port);

		if(bind(this->socket_fd, reinterpret_cast<struct sockaddr *>(&(this->address)), sizeof(struct sockaddr_in)) != 0)
		{
			cerr << "bind() failed" << endl;
			return false;
		}

		if(listen(this->socket_fd, 5) != 0)
		{
			cerr << "listen() failed" << endl;
			return false;
		}
		//Launch listening loop there
		this->running = true;
		int ret = pthread_create(&(this->listenning_thread), NULL, LinuxTcpSocketServer::LaunchLoop, this);
		if(ret != 0) {
			pthread_detach(this->listenning_thread);
		}
		this->running = static_cast<bool>(ret==0);
	}
	return this->running;
}

bool LinuxTcpSocketServer::StopListening()
{
	if(this->running)
	{
		this->running = false;
		pthread_join(this->listenning_thread, NULL);
		close(this->socket_fd);
	}
	return !(this->running);
}

bool LinuxTcpSocketServer::SendResponse(const string& response, void* addInfo)
{
	bool result = false;
	int connection_fd = reinterpret_cast<intptr_t>(addInfo);

	string temp = response;
	if(temp.find(DELIMITER_CHAR) == string::npos) {
		temp.append(1, DELIMITER_CHAR);
	}
	if(DELIMITER_CHAR != '\n') {
		char eot = DELIMITER_CHAR;
		string toSend = temp.substr(0, toSend.find_last_of('\n'));
		toSend += eot;
		result = this->WriteToSocket(connection_fd, toSend);
	}
	else {
		result = this->WriteToSocket(connection_fd, temp);
	}
	close(connection_fd);
	return result;
}

void* LinuxTcpSocketServer::LaunchLoop(void *p_data) {
	pthread_detach(pthread_self());
	LinuxTcpSocketServer *instance = reinterpret_cast<LinuxTcpSocketServer*>(p_data);;
	instance->ListenLoop();
	return NULL;
}

void LinuxTcpSocketServer::ListenLoop() {
	int connection_fd;
	socklen_t address_length = sizeof(this->address);
	while(this->running) {
		if((connection_fd = accept(this->socket_fd, reinterpret_cast<struct sockaddr *>(&(this->address)),  &address_length)) > 0)
		{
			pthread_t client_thread;
			struct GenerateResponseParameters *params = new struct GenerateResponseParameters();
			params->instance = this;
			params->connection_fd = connection_fd;
			int ret = pthread_create(&client_thread, NULL, LinuxTcpSocketServer::GenerateResponse, params);
			if(ret != 0) {
				pthread_detach(client_thread);
				delete params;
				params = NULL;
			}
		}
		else {
			usleep(2500);
		}
	}
}

void* LinuxTcpSocketServer::GenerateResponse(void *p_data) {
	pthread_detach(pthread_self());
	struct GenerateResponseParameters* params = reinterpret_cast<struct GenerateResponseParameters*>(p_data);
	LinuxTcpSocketServer *instance = params->instance;
	int connection_fd = params->connection_fd;
	delete params;
	params = NULL;
	int nbytes;
	char buffer[BUFFER_SIZE];
	string request;
	do { //The client sends its json formatted request and a delimiter request.
		nbytes = read(connection_fd, buffer, BUFFER_SIZE);
		request.append(buffer,nbytes);
	} while(request.find(DELIMITER_CHAR) == string::npos);
	instance->OnRequest(request, reinterpret_cast<void*>(connection_fd));
	return NULL;
}


bool LinuxTcpSocketServer::WriteToSocket(int fd, const string& toWrite) {
	bool fullyWritten = false;
	bool errorOccured = false;
	string toSend = toWrite;
	do {
		ssize_t byteWritten = write(fd, toSend.c_str(), toSend.size());
		if(byteWritten < 0)
			errorOccured = true;
		else if(byteWritten < toSend.size()) {
			int len = toSend.size() - byteWritten;
			toSend = toSend.substr(byteWritten + sizeof(char), len);
		}
		else
			fullyWritten = true;
	} while(!fullyWritten && !errorOccured);

	return fullyWritten && !errorOccured;
}
