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
   // template<typename S>
    class Server
    {
        public:
            //typedef void(S::methodPointer_t)(Json::Value &parameter, Json::Value &result);
            Server(AbstractServerConnector* connector);

            Server(const std::string& configfile, methodpointer_t& methods, notificationpointer_t& notifications, AbstractServerConnector* connector);
            virtual ~Server();

            /**
             * @brief StartListening starts the AbstractServerConnector to listen for incoming requests.
             * @return
             */
            bool StartListening();

            /**
             * @brief StopListening stops the AbstractServerConnector, no more requests will be answered.
             * @return
             */
            bool StopListening();

            /**
             * @brief set an authenticator to be used by this server. The authenticator will be deleted automatically.
             * @param auth
             */
            void setAuthenticator(AbstractAuthenticator* auth);

            void registerMethod(Procedure* proc, pMethod_t);

        private:
            AbstractServerConnector* connection;
            RpcProtocolServer handler;
    };

} /* namespace jsonrpc */
#endif /* SERVER_H_ */
