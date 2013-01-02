/**
 * @file client.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "client.h"

namespace jsonrpc
{
    size_t Client::id = 1;

    Client::Client(ClientConnector* connector)
            : connector(connector)
    {
    }
    
    Client::Client(ClientConnector* connector,
            const std::string& serverSpecification)
            : connector(connector)
    {
    }

    Client::~Client()
    {
        delete this->connector;
    }
    
    Json::Value Client::CallMethod(const std::string& name,
            const Json::Value& paramter)
    {
        return Json::nullValue;
    }
    
    void Client::CallNotification(const std::string& name,
            const Json::Value& paramter)
    {
    }

    std::vector<Json::Value> Client::BatchCallMethod(
            std::map<std::string, Json::Value> methodcalls)
    {
        //return Json::nullValue;
    }

    void Client::BatchCallNotification(
            std::map<std::string, Json::Value> methodcalls)
    {

    }

} /* namespace jsonrpc */
