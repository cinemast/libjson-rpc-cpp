/**
 * @file client.h
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "clientconnector.h"
#include <json/json.h>

namespace jsonrpc
{
    
    class Client
    {
        public:
            Client(ClientConnector* connector);
            Client(ClientConnector* connector, const std::string& serverSpecification );
            virtual ~Client();

            Json::Value CallMethod(const std::string& name, const Json::Value& paramter);
            void CallNotification(const std::string& name, const Json::Value& paramter);



        private:
           ClientConnector* connector;
    };

} /* namespace jsonrpc */
#endif /* CLIENT_H_ */
