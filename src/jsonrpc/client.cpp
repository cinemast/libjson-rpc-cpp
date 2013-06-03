/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.cpp
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "client.h"
#include "rpcprotocolclient.h"
#include "exception.h"


namespace jsonrpc
{
    Client::Client(AbstractClientConnector* connector)
            : connector(connector)
    {
    }

    Client::~Client()
    {
        delete this->connector;
    }

    void Client::CallMethod(const std::string &name, const Json::Value &paramter, Json::Value& result) throw(JsonRpcException)
    {
        std::string request, response;
        protocol.BuildRequest(name, paramter, request, false);
        connector->SendMessage(request, response);
        protocol.HandleResponse(response, result);
    }
    
    Json::Value Client::CallMethod(const std::string& name,
                                   const Json::Value& parameter) throw(JsonRpcException)
    {
        Json::Value result;
        this->CallMethod(name, parameter, result);
        return result;
    }
    
    void Client::CallNotification(const std::string& name, const Json::Value& parameter) throw(JsonRpcException)
    {
        std::string request, response;
        protocol.BuildRequest(name, parameter, request, true);
        connector->SendMessage(request, response);
    }

} /* namespace jsonrpc */
