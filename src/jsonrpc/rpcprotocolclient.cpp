/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    responsehandler.cpp
 * @date    13.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolclient.h"
#include "json/writer.h"

namespace jsonrpc
{
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
        authenticator(NULL), id(1)
    {
    }

    RpcProtocolClient::RpcProtocolClient(AbstractAuthenticator &authenticator) :
        authenticator(&authenticator), id(1)
    {
    }

    void RpcProtocolClient::BuildRequest(const std::string &method, const Json::Value &parameter, std::string &result, bool isNotification)
    {
        Json::Value request;
        Json::FastWriter writer;
        this->BuildRequest(method,parameter,request, isNotification);
        result = writer.write(request);
    }

    std::string RpcProtocolClient::BuildBatchRequest(batchProcedureCall_t &requests, bool isNotification)
    {
        std::string result;
        this->BuildBatchRequest(requests, result, isNotification);
        return result;
    }

    void RpcProtocolClient::BuildBatchRequest(batchProcedureCall_t &requests, std::string &result, bool isNotification)
    {
        Json::Value singlerequest;
        Json::Value res;
        Json::FastWriter writer;
        int i=0;
        for(batchProcedureCall_t::iterator it = requests.begin(); it != requests.end(); it++)
        {
            this->BuildRequest(it->first, it->second, singlerequest, isNotification);
            res[i] = singlerequest;
            i++;
        }
        result = writer.write(res);
    }

    Json::Value RpcProtocolClient::HandleResponse(const std::string &response) throw(JsonRpcException)
    {
        Json::Value result;
        this->HandleResponse(response, result);
        return result;
    }

    void RpcProtocolClient::HandleResponse(const std::string &response, Json::Value& result) throw(JsonRpcException)
    {
        Json::Reader reader;
        Json::Value value;
        if(reader.parse(response, value))
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
                throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE, " " + response);
            }
        }
        else
        {
            throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " " + response);
        }
    }

    void RpcProtocolClient::resetId()
    {
        this->id = 1;
    }

    void RpcProtocolClient::BuildRequest(const std::string &method, const Json::Value &parameter, Json::Value &result, bool isNotification)
    {
        result[KEY_PROTOCOL_VERSION] = "2.0";
        result[KEY_PROCEDURE_NAME] = method;
        result[KEY_PARAMETER] = parameter;

        if(!isNotification)
        {
            result[KEY_ID] = id++;
        }
    }
}
