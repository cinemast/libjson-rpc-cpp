/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    linuxtcpsocketclient.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_LINUXTCPSOCKETCLIENT_H_
#define JSONRPC_CPP_LINUXTCPSOCKETCLIENT_H_

#include <jsonrpccpp/common/exception.h>
#include <string>
#include "tcpsocketclientprivate.h"

namespace jsonrpc
{
        /**
         * This class is the Linux/UNIX implementation of TCPSocketClient.
         * It uses the POSIX socket API to performs its job.
         */
	class LinuxTcpSocketClient : public TcpSocketClientPrivate
	{
		public:
                        /**
                         * @brief LinuxTcpSocketClient, constructor of the Linux/UNIX implementation of class TcpSocketClient
                         * @param hostToConnect The hostname or the ipv4 address on which the client should try to connect
                         * @param port The port on which the client should try to connect
                         */
			LinuxTcpSocketClient(const std::string& hostToConnect, const unsigned int &port);
                        /**
                         * @brief ~LinuxTcpSocketClient, the destructor of LinuxTcpSocketClient
                         */
			virtual ~LinuxTcpSocketClient();
                        /**
                         * @brief The real implementation of TcpSocketClient::SendRPCMessage method.
                         * @param message The message to send
                         * @param result The result of the call returned by the server
                         * @throw JsonRpcException Thrown when an issue is encounter with socket manipulation (see message of exception for more information about what happened).
                         */
			virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);

		private:
			std::string hostToConnect;    /*!< The hostname or the ipv4 address on which the client should try to connect*/
			unsigned int port;          /*!< The port on which the client should try to connect*/
                        int Connect() throw (JsonRpcException);
                        int Connect(const std::string& ip, const int& port) throw (JsonRpcException);
                        bool IsIpv4Address(const std::string& ip);
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_LINUXTCPSOCKETCLIENT_H_ */
