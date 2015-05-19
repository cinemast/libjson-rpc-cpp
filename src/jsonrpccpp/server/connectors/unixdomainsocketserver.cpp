/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    unixdomainsocketserver.cpp
 * @date    07.05.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "unixdomainsocketserver.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <jsonrpccpp/common/specificationparser.h>
#include <cstdio>
#include <unistd.h>

using namespace jsonrpc;
using namespace std;

#define BUFFER_SIZE 64
#define PATH_MAX 108
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR

UnixDomainSocketServer::UnixDomainSocketServer(const std::string &socket_path) :
    AbstractServerConnector(),
    socket_path(socket_path.substr(0, PATH_MAX)),
    running(false)
{
}

bool UnixDomainSocketServer::StartListening()
{
    if(!this->running)
    {
        //Create and bind socket here.
    	//Then launch the listenning loop.
    	this->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(this->socket_fd < 0)
		{
			cerr << "socket() failed" << endl;
			return false;
		}

		unlink(this->socket_path.c_str());

		/* start with a clean address structure */
		memset(&(this->address), 0, sizeof(struct sockaddr_un));

		this->address.sun_family = AF_UNIX;
		snprintf(this->address.sun_path, PATH_MAX, this->socket_path.c_str());

		if(bind(this->socket_fd, reinterpret_cast<struct sockaddr *>(&(this->address)), sizeof(struct sockaddr_un)) != 0)
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
		int ret = pthread_create(&(this->listenning_thread), NULL, UnixDomainSocketServer::LaunchLoop, this);
		if(ret != 0) {
			pthread_detach(this->listenning_thread);
		}
		this->running = static_cast<bool>(ret==0);
    }
    return this->running;
}

bool UnixDomainSocketServer::StopListening()
{
    if(this->running)
    {
       	close(this->socket_fd);
       	unlink(this->socket_path.c_str());
        this->running = false;
    }
    return true;
}

bool UnixDomainSocketServer::SendResponse(const string& response, void* addInfo)
{
	int connection_fd = reinterpret_cast<intptr_t>(addInfo);

	string temp = response;
	if(temp.find(DELIMITER_CHAR) == string::npos) {
		temp.append(1, DELIMITER_CHAR);
	}
	if(DELIMITER_CHAR != '\n') {
		char eot = DELIMITER_CHAR;
		string toSend = temp.substr(0, toSend.find_last_of('\n'));
		toSend += eot;
		write(connection_fd, toSend.c_str(), toSend.size());
	}
	else {
		write(connection_fd, temp.c_str(), temp.size());
	}
	close(connection_fd);
	return true;
}

void* UnixDomainSocketServer::LaunchLoop(void *p_data) {
	UnixDomainSocketServer *instance = reinterpret_cast<UnixDomainSocketServer*>(p_data);;
	instance->ListenLoop();
}

void UnixDomainSocketServer::ListenLoop() {
	int connection_fd;
	socklen_t address_length = sizeof(this->address);
	while((connection_fd = accept(this->socket_fd, (struct sockaddr *) &(this->address),  &address_length)) > -1)
	{
		pthread_t client_thread;
		struct GenerateResponseParameters *params = new struct GenerateResponseParameters();
		params->instance = this;
		params->connection_fd = connection_fd;
		int ret = pthread_create(&client_thread, NULL, UnixDomainSocketServer::GenerateResponse, params);
		if(ret != 0) {
			pthread_detach(client_thread);
			delete params;
			params = NULL;
		}
	}
}

void* UnixDomainSocketServer::GenerateResponse(void *p_data) {
	struct GenerateResponseParameters* params = reinterpret_cast<struct GenerateResponseParameters*>(p_data);
	UnixDomainSocketServer *instance = params->instance;
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
}
