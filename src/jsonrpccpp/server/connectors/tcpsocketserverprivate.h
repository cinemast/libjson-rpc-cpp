/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpsocketserverprivate.h
 * @date    17.07.2015
 * @author  Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPSOCKETSERVERPRIVATECONNECTOR_H_
#define JSONRPC_CPP_TCPSOCKETSERVERPRIVATECONNECTOR_H_

#include "tcpsocketserverprivate.h"

#include "../abstractserverconnector.h"

namespace jsonrpc
{
	/**
	 * This interface allows to abstract the real TcpSocketServer, either it be Windows, Linux or else.
	 */
	class TcpSocketServerPrivate : public AbstractServerConnector
	{
	};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_TCPSOCKETSERVERPRIVATECONNECTOR_H_ */
