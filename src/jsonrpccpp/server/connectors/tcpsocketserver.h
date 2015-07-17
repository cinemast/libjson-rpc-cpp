/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpsocketserver.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPSOCKETSERVERCONNECTOR_H_
#define JSONRPC_CPP_TCPSOCKETSERVERCONNECTOR_H_

#include "tcpsocketserverprivate.h"

#include "../abstractserverconnector.h"

namespace jsonrpc
{
	/**
	 * This class provides an embedded Tcp Socket Server,to handle incoming Requests.
	 */
	class TcpSocketServer: public AbstractServerConnector
	{
		public:
			/**
			 * @brief TcpSocketServer, constructor for the included TcpSocketServer
			 * @param ipToBind, a string containing the ip of the interface to bind to.
			 * @param port, the port that the server will listen to.
			 */
			TcpSocketServer(const std::string& ipToBind, const unsigned int &port);
			/**
			 * @brief ~TcpSocketServer, destructor for the included TcpSocketServer
			 */
			~TcpSocketServer();
			bool StartListening();
			bool StopListening();

			bool SendResponse(const std::string& response, void* addInfo = NULL);

		private:
			TcpSocketServerPrivate *realSocket;
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_TCPSOCKETSERVERCONNECTOR_H_ */

