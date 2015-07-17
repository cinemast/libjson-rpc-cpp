/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    unixdomainsocketclient.h
 * @date    11.05.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPSOCKETCLIENT_H_
#define JSONRPC_CPP_TCPSOCKETCLIENT_H_

#include "../iclientconnector.h"
#include <jsonrpccpp/common/exception.h>
#include <string>
#include "tcpsocketclientprivate.h"

namespace jsonrpc
{
	class TcpSocketClient : public IClientConnector
	{
		public:
			TcpSocketClient(const std::string& ipToConnect, const unsigned int &port);
			virtual ~TcpSocketClient();
			virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);

		private:
			TcpSocketClientPrivate *realSocket;
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_TCPSOCKETCLIENT_H_ */
