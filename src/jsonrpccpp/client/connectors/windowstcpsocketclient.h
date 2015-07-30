/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    windowstcpsocketclient.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_WINDOWSTCPSOCKETCLIENT_H_
#define JSONRPC_CPP_WINDOWSTCPSOCKETCLIENT_H_

#include <jsonrpccpp/common/exception.h>
#include <string>
#include "tcpsocketclientprivate.h"

namespace jsonrpc
{
        /**
         * This class is the windows implementation of TCPSocketClient.
         * It uses the Winsock2 API to performs its job.
         */
	class WindowsTcpSocketClient : public TcpSocketClientPrivate
	{
		public:
                        /**
                         * @brief WindowsTcpSocketClient, constructor of the Windows implementation of class TcpSocketClient
                         * @param ipToConnect The ipv4 address on which the client should try to connect
                         * @param port The port on which the client should try to connect
                         */
			WindowsTcpSocketClient(const std::string& ipToConnect, const unsigned int &port);
                        /**
                         * @brief ~WindowsTcpSocketClient, the destructor of WindowsTcpSocketClient
                         */
			virtual ~WindowsTcpSocketClient();
                        /**
                         * @brief The real implementation of TcpSocketClient::SendRPCMessage method.
                         * @param message The message to send
                         * @param result The result of the call returned by the server
                         * @throw JsonRpcException Thrown when an issue is encounter with socket manipulation (see message of exception for more information about what happened).
                         */
			virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);

		private:
			std::string ipToConnect;    /*!< The ipv4 address on which the client should try to connect*/
			unsigned int port;          /*!< The port on which the client should try to connect*/
                        /**
                         * @brief A method to produce human readable messages from Winsock2 error values.
                         * @param e A Winsock2 error value
                         * @return The message matching the error value
                         */
			static std::string GetErrorMessage(const int &e);
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_WINDOWSTCPSOCKETCLIENT_H_ */
