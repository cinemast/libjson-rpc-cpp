/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    windowstcpsocketserver.cpp
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "windowstcpsocketserver.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <windows.h>

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

WindowsTcpSocketServer::WindowsTcpSocketServer(const std::string& ipToBind, const unsigned int &port) :
	TcpSocketServerPrivate(),
	ipToBind(ipToBind),
	port(port),
	running(false)
{
    //if(nbInstances == 0) {
        WSADATA WSAData;
        WSAStartup(MAKEWORD(2,0), &WSAData);
    /*}
    nbInstances++;//*/
}

WindowsTcpSocketServer::~WindowsTcpSocketServer() {
    /*nbInstances--;
    if(nbInstances == 0) {
        WSACleanup();
    }//*/
}

bool WindowsTcpSocketServer::StartListening()
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

		ioctlsocket(this->socket_fd, FIONBIO, 1); //Set non blocking

		/* start with a clean address structure */
		memset(&(this->address), 0, sizeof(struct SOCKADDR_IN));

		this->address.sin_family = AF_INET;
		this->address.sin_addr.s_addr = inet_addr(this->ipToBind.c_str());
		this->address.sin_port = htons(this->port);

		if(bind(this->socket_fd, reinterpret_cast<SOCKADDR*>(&(this->address)), sizeof(struct SOCKADDR_IN)) != 0)
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
		HANDLE ret = CreateThread(NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(WindowsTcpSocketServer::LaunchLoop), this, 0, &(this->listenning_thread));
		if(ret == NULL) {
			CloseHandle(this->listenning_thread);
		}
		this->running = static_cast<bool>(ret!=NULL);
	}
	return this->running;
}

bool WindowsTcpSocketServer::StopListening()
{
	if(this->running)
	{
		this->running = false;
		WaitForSingleObject(this->listenning_thread, INFINITE);
		closesocket(this->socket_fd);
	}
	return !(this->running);
}

bool WindowsTcpSocketServer::SendResponse(const string& response, void* addInfo)
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
	closesocket(connection_fd);
	return result;
}

void* WindowsTcpSocketServer::LaunchLoop(void *p_data) {
	CloseHandle(GetCurrentThreadId());
	WindowsTcpSocketServer *instance = reinterpret_cast<WindowsTcpSocketServer*>(p_data);;
	instance->ListenLoop();
	return NULL;
}

void WindowsTcpSocketServer::ListenLoop() {
	int connection_fd;
	unsigned int address_length = sizeof(this->address);
	while(this->running) {
		if((connection_fd = accept(this->socket_fd, reinterpret_cast<SOCKADDR*>(&(this->address)),  &address_length)) > 0)
		{
			DWORD client_thread;
			struct GenerateResponseParameters *params = new struct GenerateResponseParameters();
			params->instance = this;
			params->connection_fd = connection_fd;
                        HANDLE ret = CreateThread(NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(WindowsTcpSocketServer::GenerateResponse), params, 0, &client_thread);
                        if(ret == NULL) {
                                CloseHandle(client_thread);
                                delete params;
				params = NULL;
                        }
		}
		else {
			Sleep(2.5);
		}
	}
}

void* WindowsTcpSocketServer::GenerateResponse(void *p_data) {
	CloseHandle(GetCurrentThreadId());
	struct GenerateResponseParameters* params = reinterpret_cast<struct GenerateResponseParameters*>(p_data);
	WindowsTcpSocketServer *instance = params->instance;
	int connection_fd = params->connection_fd;
	delete params;
	params = NULL;
	int nbytes;
	char buffer[BUFFER_SIZE];
	string request;
	do { //The client sends its json formatted request and a delimiter request.
		nbytes = recv(connection_fd, buffer, BUFFER_SIZE, 0);
		request.append(buffer,nbytes);
	} while(request.find(DELIMITER_CHAR) == string::npos);
	instance->OnRequest(request, reinterpret_cast<void*>(connection_fd));
	return NULL;
}


bool WindowsTcpSocketServer::WriteToSocket(int fd, const string& toWrite) {
	bool fullyWritten = false;
	bool errorOccured = false;
	string toSend = toWrite;
	do {
		ssize_t byteWritten = send(fd, toSend.c_str(), toSend.size(), 0);
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
