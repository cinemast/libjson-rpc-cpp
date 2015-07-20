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
	class WindowsTcpSocketClient : public TcpSocketClientPrivate
	{
		public:
			WindowsTcpSocketClient(const std::string& ipToConnect, const unsigned int &port);
			virtual ~WindowsTcpSocketClient();
			virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);

		private:
			std::string ipToConnect;
			unsigned int port;
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_WINDOWSTCPSOCKETCLIENT_H_ */
