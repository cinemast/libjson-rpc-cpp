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
	class LinuxTcpSocketClient : public TcpSocketClientPrivate
	{
		public:
			LinuxTcpSocketClient(const std::string& ipToConnect, const unsigned int &port);
			virtual ~LinuxTcpSocketClient();
			virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);

		private:
			std::string ipToConnect;
			unsigned int port;
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_LINUXTCPSOCKETCLIENT_H_ */
