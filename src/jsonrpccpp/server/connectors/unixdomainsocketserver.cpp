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
#include <sys/types.h>
#include <jsonrpccpp/common/specificationparser.h>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <string>

using namespace jsonrpc;
using namespace std;

#define BUFFER_SIZE 1024
#define PATH_MAX 108
#ifndef DELIMITER_CHAR
    #define DELIMITER_CHAR char(0x0A)
#endif

UnixDomainSocketServer::UnixDomainSocketServer(const string &socket_path) :
    running(false),
    socket_path(socket_path.substr(0, PATH_MAX))
{
}

bool UnixDomainSocketServer::StartListening()
{
	if(!this->running)
	{
        if (access(this->socket_path.c_str(), F_OK) != -1)
            return false;

        this->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

        fcntl(this->socket_fd, F_SETFL, FNDELAY);

		memset(&(this->address), 0, sizeof(struct sockaddr_un));

		this->address.sun_family = AF_UNIX;
		snprintf(this->address.sun_path, PATH_MAX, this->socket_path.c_str());

        bind(this->socket_fd, reinterpret_cast<struct sockaddr *>(&(this->address)), sizeof(struct sockaddr_un));

        listen(this->socket_fd, 5);

		//Launch listening loop there
        this->running = true;
        return (pthread_create(&(this->listenning_thread), NULL, UnixDomainSocketServer::LaunchLoop, this) == 0);
	}
    return false;
}

bool UnixDomainSocketServer::StopListening()
{
	if(this->running)
	{
        this->running = false;
        pthread_join(this->listenning_thread, NULL);
		close(this->socket_fd);
		unlink(this->socket_path.c_str());
        return true;
	}
    return false;
}

bool UnixDomainSocketServer::SendResponse(const string& response, void* addInfo)
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

void* UnixDomainSocketServer::LaunchLoop(void *p_data)
{
	UnixDomainSocketServer *instance = reinterpret_cast<UnixDomainSocketServer*>(p_data);
	instance->ListenLoop();
    return NULL;
}

void UnixDomainSocketServer::ListenLoop()
{
	int connection_fd;
	socklen_t address_length = sizeof(this->address);
    while(this->running)
	{
        connection_fd = accept(this->socket_fd, (struct sockaddr *) &(this->address),  &address_length);
        if (connection_fd > 0)
        {
            pthread_t client_thread;
            struct ClientConnection *params = new struct ClientConnection();
            params->instance = this;
            params->connection_fd = connection_fd;
            pthread_create(&client_thread, NULL, UnixDomainSocketServer::GenerateResponse, params);
        }
        else
        {
            usleep(25000);
        }

	}
}

void* UnixDomainSocketServer::GenerateResponse(void *p_data)
{
	pthread_detach(pthread_self());
    struct ClientConnection* params = reinterpret_cast<struct ClientConnection*>(p_data);
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

    instance->OnRequest(request.substr(0, request.size()-1), reinterpret_cast<void*>(connection_fd));
    pthread_exit(NULL);
    return NULL;
}


bool UnixDomainSocketServer::WriteToSocket(int fd, const string& toWrite)
{
	bool fullyWritten = false;
	bool errorOccured = false;
	string toSend = toWrite;
	do {
        ssize_t byteWritten = write(fd, toSend.c_str(), toSend.size());
		if(byteWritten < 0)
			errorOccured = true;
        else if(byteWritten < (ssize_t) toSend.size()) {
			int len = toSend.size() - byteWritten;
			toSend = toSend.substr(byteWritten + sizeof(char), len);
		}
		else
			fullyWritten = true;
	} while(!fullyWritten && !errorOccured);

	return fullyWritten && !errorOccured;
}
