/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    windowstcpsocketserver.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_WINDOWSTCPSOCKETSERVERCONNECTOR_H_
#define JSONRPC_CPP_WINDOWSTCPSOCKETSERVERCONNECTOR_H_

#include <stdarg.h>
#include <stdint.h>
#include <winsock2.h>

#include "tcpsocketserverprivate.h"

namespace jsonrpc
{
	/**
	 * This class provides an embedded Unix Domain Socket Server,to handle incoming Requests.
	 */
	class WindowsTcpSocketServer: public TcpSocketServerPrivate
	{
		public:
			/**
			 * @brief UnixDomainSocketServer, constructor for the included UnixDomainSocketServer
			 * @param socket_path, a string containing the path to the unix socket
			 */
			WindowsTcpSocketServer(const std::string& ipToBind, const unsigned int &port);
			~WindowsTcpSocketServer();

			bool StartListening();
			bool StopListening();

			bool SendResponse(const std::string& response, void* addInfo = NULL);

		private:
			bool running;
			std::string ipToBind;
			unsigned int port;
			SOCKET socket_fd;
			SOCKADDR_IN address;

			DWORD listenning_thread;

			static DWORD WINAPI LaunchLoop(LPVOID lp_data);
			void ListenLoop();
			struct GenerateResponseParameters
			{
				WindowsTcpSocketServer *instance;
				SOCKET connection_fd;
			};
			static DWORD WINAPI GenerateResponse(LPVOID lp_data);
			bool WriteToSocket(SOCKET fd, const std::string& toSend);
			bool WaitClientClose(SOCKET fd, const int &timeout = 100);
			int CloseByReset(SOCKET fd);
			int CleanClose(SOCKET fd);
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_WINDOWSTCPSOCKETSERVERCONNECTOR_H_ */

