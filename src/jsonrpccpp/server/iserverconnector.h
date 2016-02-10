/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    iserverconnector.h
 * @date    10.01.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ISERVERCONNECTOR_H_
#define JSONRPC_CPP_ISERVERCONNECTOR_H_

#include <string>
#include <jsonrpccpp/common/exception.h>
#include <cstdio>

namespace jsonrpc
{
    class IServerConnector
    {
        public:
            virtual ~IServerConnector()
            {
            }

            virtual void SendRPCMessage(const std::string& message, std::string& result, unsigned int uiConnectionId = 0) throw(JsonRpcException) = 0;
    };

} /* namespace jsonrpc */

#endif /* JSONRPC_CPP_ISERVERCONNECTOR_H_ */
