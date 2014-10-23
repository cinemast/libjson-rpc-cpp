/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    iclientconnectionhandler.h
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ICLIENTCONNECTIONHANDLER_H
#define JSONRPC_CPP_ICLIENTCONNECTIONHANDLER_H

#include <string>

namespace jsonrpc
{
    class IClientConnectionHandler {
        public:
            virtual ~IClientConnectionHandler() {}

            virtual void HandleRequest(const std::string& request, std::string& retValue) = 0;
    };
}

#endif // JSONRPC_CPP_ICLIENTCONNECTIONHANDLER_H
