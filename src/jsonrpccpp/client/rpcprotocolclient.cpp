/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    responsehandler.cpp
 * @date    13.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolclient.h"

#ifdef __linux
#include <jsoncpp/json/writer.h>
#else
#include <json/writer.h>
#endif


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

RpcProtocolClient::RpcProtocolClient(clientVersion_t version) :
    version(version)
{
}

void        RpcProtocolClient::BuildRequest         (const std::string &method, const Json::Value &parameter, std::string &result, bool isNotification)
{
    Json::Value request;
    Json::FastWriter writer;
    this->BuildRequest(1, method,parameter,request, isNotification);
    result = writer.write(request);
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
    if(this->ValidateResponse(value))
    {
        if (this->HasError(value))
        {
            this->throwErrorException(value);
        }
        else
        {
            result = value[KEY_RESULT];
        }
    }
    else
    {
        throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, " " + value.toStyledString());
    }
    return value[KEY_ID].asInt();
}

void        RpcProtocolClient::BuildRequest         (int id, const std::string &method, const Json::Value &parameter, Json::Value &result, bool isNotification)
{
    if (this->version == JSONRPC_CLIENT_V2)
        result[KEY_PROTOCOL_VERSION] = "2.0";
    result[KEY_PROCEDURE_NAME] = method;
    result[KEY_PARAMETER] = parameter;
    if (!isNotification)
        result[KEY_ID] = id;
    else if (this->version == JSONRPC_CLIENT_V1)
        result[KEY_ID] = Json::nullValue;
}

void RpcProtocolClient::throwErrorException(const Json::Value &response)
{
    if (response[KEY_ERROR].isMember(KEY_ERROR_MESSAGE) && response[KEY_ERROR][KEY_ERROR_MESSAGE].isString())
        throw JsonRpcException(response[KEY_ERROR][KEY_ERROR_CODE].asInt(), response[KEY_ERROR][KEY_ERROR_MESSAGE].asString());
    throw JsonRpcException(response[KEY_ERROR][KEY_ERROR_CODE].asInt());
}

bool RpcProtocolClient::ValidateResponse(const Json::Value& response)
{
    if (!response.isObject() || !response.isMember(KEY_ID))
        return false;

    if (this->version == JSONRPC_CLIENT_V1)
    {
        if (!response.isMember(KEY_RESULT) || !response.isMember(KEY_ERROR))
            return false;
        if(!response[KEY_RESULT].isNull() && !response[KEY_ERROR].isNull())
            return false;
        if (!response[KEY_ERROR].isNull() && !(response[KEY_ERROR].isObject() && response[KEY_ERROR].isMember(KEY_ERROR_CODE) && response[KEY_ERROR][KEY_ERROR_CODE].isInt()))
            return false;
    }
    else if (this->version == JSONRPC_CLIENT_V2)
    {
        if (!response.isMember(KEY_PROTOCOL_VERSION) || response[KEY_PROTOCOL_VERSION] != "2.0")
            return false;
        if (response.isMember(KEY_RESULT) && response.isMember(KEY_ERROR))
            return false;
        if (!response.isMember(KEY_RESULT) && !response.isMember(KEY_ERROR))
            return false;
        if (response.isMember(KEY_ERROR) && !(response[KEY_ERROR].isObject() && response[KEY_ERROR].isMember(KEY_ERROR_CODE) && response[KEY_ERROR][KEY_ERROR_CODE].isInt()))
            return false;
    }

    return true;
}

bool RpcProtocolClient::HasError(const Json::Value& response)
{
    if (this->version == JSONRPC_CLIENT_V1 && !response[KEY_ERROR].isNull())
        return true;
    else if (this->version == JSONRPC_CLIENT_V2 && response.isMember(KEY_ERROR))
        return true;
    return false;
}
