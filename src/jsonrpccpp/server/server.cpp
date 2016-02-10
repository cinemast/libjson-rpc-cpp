/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.cpp
 * @date    10.01.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "server.h"
#include <jsonrpccpp/common/rpcprotocolclient.h>
#include <cstdio>

using namespace jsonrpc;

Server::Server(IServerConnector &connector, clientVersion_t version) :
    connector(connector)
{
    this->protocol = new RpcProtocolClient(version);
}

Server::~Server()
{
    delete this->protocol;
}

void Server::CallMethod(const std::string &name, const Json::Value &parameter, Json::Value& result, unsigned int uiConnectionId /*= 0*/) throw(JsonRpcException)
{
    std::string request, response;
    protocol->BuildRequest(name, parameter, request, false);
    connector.SendRPCMessage(request, response, uiConnectionId);
    protocol->HandleResponse(response, result);
}

void Server::CallProcedures(const BatchCall &calls, BatchResponse &result, unsigned int uiConnectionId /*= 0*/) throw(JsonRpcException)
{
    std::string request, response;
    request = calls.toString();
    connector.SendRPCMessage(request, response, uiConnectionId);
    Json::Reader reader;
    Json::Value tmpresult;

    if (!reader.parse(response, tmpresult) || !tmpresult.isArray())
    {
        throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, "Array expected.");
    }

    for (unsigned int i=0; i < tmpresult.size(); i++)
    {
        if (tmpresult[i].isObject()) {
            Json::Value singleResult;
            try {
                int id = this->protocol->HandleResponse(tmpresult[i], singleResult);
                result.addResponse(id, singleResult, false);
            }
            catch (JsonRpcException& ex) {
                int id = -1;
                if(tmpresult[i].isMember("id") && tmpresult[i]["id"].isInt())
                    id = tmpresult[i]["id"].asInt();
                result.addResponse(id, tmpresult[i]["error"], true);
            }
        }
        else
            throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, "Object in Array expected.");
    }
}

BatchResponse Server::CallProcedures(const BatchCall &calls, unsigned int uiConnectionId /*= 0*/) throw(JsonRpcException)
{
    BatchResponse result;
    this->CallProcedures(calls, result, uiConnectionId);
    return result;
}

Json::Value Server::CallMethod(const std::string& name, const Json::Value& parameter, unsigned int uiConnectionId /*= 0*/) throw(JsonRpcException)
{
    Json::Value result;
    this->CallMethod(name, parameter, result, uiConnectionId);
    return result;
}

void Server::CallNotification(const std::string& name, const Json::Value& parameter, unsigned int uiConnectionId /*= 0*/) throw(JsonRpcException)
{
    std::string request;
    std::string response;
    protocol->BuildRequest(name, parameter, request, true);
    connector.SendRPCMessage(request, response, uiConnectionId);
}
