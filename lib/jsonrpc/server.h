/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.h
 * @date    30.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>
#include <vector>

#include "rpcprotocolserver.h"
#include "serverconnector.h"

namespace jsonrpc
{
    class Server
    {
        public:
            Server(const std::string& configfile, methodpointer_t& methods, notificationpointer_t& notifications, AbstractServerConnector* connector, AbstractAuthenticator* auth = NULL);
            virtual ~Server();

            bool StartListening();
            bool StopListening();

        private:
            AbstractServerConnector* connection;
            RpcProtocolServer handler;
    };

} /* namespace jsonrpc */
#endif /* SERVER_H_ */
