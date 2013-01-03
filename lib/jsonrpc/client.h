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

#include <vector>
#include <map>

namespace jsonrpc
{
    
    typedef std::map<std::string, Json::Value> batchProcedureCall_t;

    class Client
    {
        public:
            Client(ClientConnector* connector, bool validateResponse);
            Client(ClientConnector* connector, const std::string& serverSpecification, bool validateResponse);
            virtual ~Client();

            Json::Value CallMethod(const std::string& name, const Json::Value& paramter);
            void CallNotification(const std::string& name, const Json::Value& paramter);

            /*
            std::vector<Json::Value> BatchCallMethod(std::map<std::string,Json::Value> methodcalls);
            void BatchCallNotification(std::map<std::string,Json::Value> methodcalls);
            */

        private:
           ClientConnector* connector;
           bool validateResponse;

           Json::Value BuildRequestObject(const std::string& name, const Json::Value& parameters, int id) const;
    };

} /* namespace jsonrpc */
#endif /* CLIENT_H_ */
