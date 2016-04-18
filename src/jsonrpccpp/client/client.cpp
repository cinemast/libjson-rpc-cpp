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

using namespace jsonrpc;

Client::Client(IClientConnector &connector, clientVersion_t version) :
    connector(connector)
{
    this->protocol = new RpcProtocolClient(version);
}

Client::~Client()
{
    delete this->protocol;
}

void Client::CallMethod(const std::string &name, const Json::Value &parameter, Json::Value& result) throw(JsonRpcException)
{
    std::string request, response;
    protocol->BuildRequest(name, parameter, request, false);
    connector.SendRPCMessage(request, response);
    protocol->HandleResponse(response, result);
}

void Client::CallProcedures(const BatchCall &calls, BatchResponse &result) throw(JsonRpcException)
{
    std::string request, response;
    request = calls.toString();
    connector.SendRPCMessage(request, response);
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
                if(tmpresult[i].isMember("id") && tmpresult[i]["id"].isIntegral())
                    id = tmpresult[i]["id"].asInt();
                result.addResponse(id, tmpresult[i]["error"], true);
            }
        }
        else
            throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, "Object in Array expected.");
    }
}

BatchResponse Client::CallProcedures(const BatchCall &calls) throw(JsonRpcException)
{
    BatchResponse result;
    this->CallProcedures(calls, result);
    return result;
}

Json::Value Client::CallMethod(const std::string& name, const Json::Value& parameter) throw(JsonRpcException)
{
    Json::Value result;
    this->CallMethod(name, parameter, result);
    return result;
}

void Client::CallNotification(const std::string& name, const Json::Value& parameter) throw(JsonRpcException)
{
    std::string request, response;
    protocol->BuildRequest(name, parameter, request, true);
    connector.SendRPCMessage(request, response);
}
