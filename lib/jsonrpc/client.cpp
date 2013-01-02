/**
 * @file client.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "client.h"

namespace jsonrpc
{
    
    Client::Client(ClientConnector* connector) : connector(connector)
    {
    }
    
    Client::Client(ClientConnector* connector,
            const std::string& serverSpecification) : connector(connector)
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
    
    void Client::CallNotification(const std::string& name, const Json::Value& paramter)
    {
    }

} /* namespace jsonrpc */
