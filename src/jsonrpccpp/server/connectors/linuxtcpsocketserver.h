/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    linuxtcpsocketserver.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_LINUXTCPSOCKETSERVERCONNECTOR_H_
#define JSONRPC_CPP_LINUXTCPSOCKETSERVERCONNECTOR_H_

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "tcpsocketserverprivate.h"

namespace jsonrpc
{
	/**
	 * This class provides an embedded Unix Domain Socket Server,to handle incoming Requests.
	 */
	class LinuxTcpSocketServer: public TcpSocketServerPrivate
	{
		public:
			/**
			 * @brief UnixDomainSocketServer, constructor for the included UnixDomainSocketServer
			 * @param socket_path, a string containing the path to the unix socket
			 */
			LinuxTcpSocketServer(const std::string& ipToBind, const unsigned int &port);

			bool StartListening();
			bool StopListening();

			bool SendResponse(const std::string& response, void* addInfo = NULL);

		private:
			bool running;
			std::string ipToBind;
			unsigned int port;
			int socket_fd;
			struct sockaddr_in address;

			pthread_t listenning_thread;

			static void* LaunchLoop(void *p_data);
			void ListenLoop();
			struct GenerateResponseParameters {
				LinuxTcpSocketServer *instance;
				int connection_fd;
			};
			static void* GenerateResponse(void *p_data);
			bool WriteToSocket(int fd, const std::string& toSend);
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_LINUXTCPSOCKETSERVERCONNECTOR_H_ */

