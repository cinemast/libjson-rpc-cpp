/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file
 * @date    7/10/2015
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_ZMQCLIENT_H
#define JSONRPC_ZMQCLIENT_H

#include "../iclientconnector.h"

namespace jsonrpc {

    class ZMQClient : public IClientConnector
    {
        public:
            ZMQClient(const std::string& url);

            virtual void SendRPCMessage(const std::string& message, std::string& result) throw(JsonRpcException);

    };

} // namespace jsonrpc

#endif // JSONRPC_ZMQCLIENT_H
