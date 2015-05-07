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
		//cout << "(" << pthread_self() << ") " << "Will launch the thread for the loop" << endl;
		int ret = pthread_create(&(this->listenning_thread), NULL, UnixDomainSocketServer::LaunchLoop, this);
		if(ret != 0) {
			pthread_detach(this->listenning_thread);
		}
		this->running = static_cast<bool>(ret==0);
    }
    //cout << "(" << pthread_self() << ") " << "Returning " << this->running << endl;
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
	//cout << "(" << pthread_self() << ") " << "Entered in send response" << endl;
	int connection_fd = reinterpret_cast<intptr_t>(addInfo);

	//cout << "(" << pthread_self() << ") " << "Will write response #" << response << "#";
	char eot = 0x04;
	string toSend = response.substr(0, toSend.find_last_of('\n'));
	toSend += eot;
	write(connection_fd, toSend.c_str(), toSend.size());

	//cout << "(" << pthread_self() << ") " << "Will close client socket" << endl;
	close(connection_fd);
	return true;
}

void* UnixDomainSocketServer::LaunchLoop(void *p_data) {
	UnixDomainSocketServer *instance = reinterpret_cast<UnixDomainSocketServer*>(p_data);;
	instance->ListenLoop();
}

void UnixDomainSocketServer::ListenLoop() {
	int connection_fd;
	socklen_t address_length;
	//cout << "(" << pthread_self() << ") " << "Will launch the loop" << endl;
	while((connection_fd = accept(this->socket_fd, (struct sockaddr *) &(this->address),  &address_length)) > -1)
	{
		//cout << "(" << pthread_self() << ") " << "Client connected" << endl;
		pthread_t client_thread;
		struct GenerateResponseParameters *params = new struct GenerateResponseParameters();
		params->instance = this;
		params->connection_fd = connection_fd;
		//cout << "(" << pthread_self() << ") " << "Will launch the thread to handle the request" << endl;
		//cout << "Sending instance=" << this << endl;
		//cout << "Sending connection_fd=" << connection_fd << endl;
		int ret = pthread_create(&client_thread, NULL, UnixDomainSocketServer::GenerateResponse, params);
		if(ret != 0) {
			pthread_detach(client_thread);
			delete params;
			params = NULL;
		}
	}
}

void* UnixDomainSocketServer::GenerateResponse(void *p_data) {
	//cout << "(" << pthread_self() << ") " << "Entrered in generate response" << endl;
	struct GenerateResponseParameters* params = reinterpret_cast<struct GenerateResponseParameters*>(p_data);
	UnixDomainSocketServer *instance = params->instance;
	//cout << "Got instance=" << instance << endl;
	int connection_fd = params->connection_fd;
	//cout << "Got connection_fd=" << connection_fd << endl;
	//cout << "(" << pthread_self() << ") " << "Obtained params" << endl;
	delete params;
	params = NULL;
	int nbytes;
	char buffer[BUFFER_SIZE];
	//cout << "(" << pthread_self() << ") " << "Will read the request" << endl;
	string request;
	do { //The client sends its json formatted request and an \0.
		//cout << "(" << pthread_self() << ") " << "Current request state: #" << request << "#";
		nbytes = read(connection_fd, buffer, BUFFER_SIZE);
		//cout << "(" << pthread_self() << ") " << "Received part of request: #" << buffer << "#";
		if(request.empty())
			request = string(buffer);
		else
			request.append(buffer,nbytes);
	} while(request.find(char(0x04)) == string::npos);

	//cout << "(" << pthread_self() << ") " << "Received request #" << request << "#" << endl;

	//cout << "(" << pthread_self() << ") " << "Will launch OnRequest" << endl;
	instance->OnRequest(request, reinterpret_cast<void*>(connection_fd));
}
