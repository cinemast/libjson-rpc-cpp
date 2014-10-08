/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    responsehandler.cpp
 * @date    13.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolclient.h"
#include <jsoncpp/json/writer.h>

using namespace jsonrpc;

const std::string RpcProtocolClient::KEY_PROTOCOL_VERSION = "jsonrpc";
const std::string RpcProtocolClient::KEY_PROCEDURE_NAME   = "method";
const std::string RpcProtocolClient::KEY_ID               = "id";
const std::string RpcProtocolClient::KEY_PARAMETER        = "params";
const std::string RpcProtocolClient::KEY_AUTH             = "auth";
const std::string RpcProtocolClient::KEY_RESULT           = "result";
const std::string RpcProtocolClient::KEY_ERROR            = "error";
const std::string RpcProtocolClient::KEY_ERROR_CODE       = "code";
const std::string RpcProtocolClient::KEY_ERROR_MESSAGE    = "message";

RpcProtocolClient::RpcProtocolClient() :
    id(1)
{
}

void        RpcProtocolClient::BuildRequest         (const std::string &method, const Json::Value &parameter, std::string &result, bool isNotification)
{
    Json::Value request;
    Json::FastWriter writer;
    this->BuildRequest(1, method,parameter,request, isNotification);
    result = writer.write(request);
}
Json::Value RpcProtocolClient::HandleResponse       (const std::string &response) throw(JsonRpcException)
{
    Json::Value result;
    this->HandleResponse(response, result);
    return result;
}
void        RpcProtocolClient::HandleResponse       (const std::string &response, Json::Value& result) throw(JsonRpcException)
{
    Json::Reader reader;
    Json::Value value;
    if(reader.parse(response, value))
    {
        this->HandleResponse(value, result);
    }
    else
    {
        throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " " + response);
    }
}

int RpcProtocolClient::HandleResponse(const Json::Value &value, Json::Value &result) throw(JsonRpcException)
{
    if(value.isMember(KEY_ID) && value.isMember(KEY_PROTOCOL_VERSION) && (value.isMember(KEY_RESULT) || value.isMember(KEY_ERROR)))
    {
        if(value.isMember(KEY_RESULT))
        {
            result = value[KEY_RESULT];
        }
        else
        {
            throw JsonRpcException(value[KEY_ERROR][KEY_ERROR_CODE].asInt(), value[KEY_ERROR][KEY_ERROR_MESSAGE].asString());
        }
    }
    else
    {
        throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, " " + value.toStyledString());
    }
    return value[KEY_ID].asInt();
}
void        RpcProtocolClient::resetId              ()
{
    this->id = 1;
}
void        RpcProtocolClient::BuildRequest         (int id, const std::string &method, const Json::Value &parameter, Json::Value &result, bool isNotification)
{
    result[KEY_PROTOCOL_VERSION] = "2.0";
    result[KEY_PROCEDURE_NAME] = method;
    result[KEY_PARAMETER] = parameter;
    if(!isNotification)
    {
        result[KEY_ID] = id;
    }
}
