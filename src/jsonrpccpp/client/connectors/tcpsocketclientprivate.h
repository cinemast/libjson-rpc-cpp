/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpsocketclientprivate.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPSOCKETCLIENTPRIVATE_H_
#define JSONRPC_CPP_TCPSOCKETCLIENTPRIVATE_H_

#include "../iclientconnector.h"
#include <jsonrpccpp/common/exception.h>

namespace jsonrpc
{
        /**
         * This class is an interface to the real implementation of TcpSocketClient. Kind of a strategy design pattern.
         */
	class TcpSocketClientPrivate : public IClientConnector
	{
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_TCPSOCKETCLIENTPRIVATE_H_ */
