/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserverv2.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolserverv2.h"
#include <jsonrpccpp/common/errors.h>
#include <iostream>

using namespace std;
using namespace jsonrpc;

RpcProtocolServerV2::RpcProtocolServerV2(IProcedureInvokationHandler &handler) :
    AbstractProtocolHandler(handler)
{
}

void RpcProtocolServerV2::HandleRequest       (const string& request, string& retValue)
{
    Json::Reader reader;
    Json::Value req;
    Json::Value response, resp;
    Json::FastWriter w;

    if (reader.parse(request, req, false))
    {
        //It could be a Batch Request
        if (req.isArray())
        {
            this->HandleBatchRequest(req, response);
        } //It could be a simple Request
        else if (req.isObject())
        {
            this->HandleSingleRequest(req, response);
        }
        else
        {
            response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_INVALID_REQUEST);
        }
    }
    else
    {
        response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_JSON_PARSE_ERROR);
    }

    if (response != Json::nullValue)
        retValue = w.write(response);
}
void RpcProtocolServerV2::HandleSingleRequest (Json::Value &req, Json::Value& response)
{
    int error = this->ValidateRequest(req);
    if (error == 0)
    {
        try
        {
            this->ProcessRequest(req, response);
        }
        catch (const JsonRpcException & exc)
        {
            response = Errors::GetErrorBlock(req, exc);
        }
    }
    else
    {
        response = Errors::GetErrorBlock(req, error);
    }
}
void RpcProtocolServerV2::HandleBatchRequest  (Json::Value &req, Json::Value& response)
{
    if (req.size() == 0)
        response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_INVALID_REQUEST);
    else
    {
        for (unsigned int i = 0; i < req.size(); i++)
        {
            Json::Value result;
            this->HandleSingleRequest(req[i], result);
            if (result != Json::nullValue)
                response.append(result);
        }
    }
}
int  RpcProtocolServerV2::ValidateRequest     (const Json::Value& request)
{
    int error = 0;
    Procedure proc;
    if (!this->ValidateRequestFields(request))
    {
        error = Errors::ERROR_RPC_INVALID_REQUEST;
    }
    else
    {
        map<string, Procedure>::iterator it = this->procedures.find(request[KEY_REQUEST_METHODNAME].asString());
        if (it != this->procedures.end())
        {
            proc = it->second;
            if(request.isMember(KEY_REQUEST_ID) && proc.GetProcedureType() == RPC_NOTIFICATION)
            {
                error = Errors::ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
            }
            else if(!request.isMember(KEY_REQUEST_ID) && proc.GetProcedureType() == RPC_METHOD)
            {
                error = Errors::ERROR_SERVER_PROCEDURE_IS_METHOD;
            }
            else if (!proc.ValdiateParameters(request[KEY_REQUEST_PARAMETERS]))
            {
                error = Errors::ERROR_RPC_INVALID_PARAMS;
            }
        }
        else
        {
            error = Errors::ERROR_RPC_METHOD_NOT_FOUND;
        }
    }
    return error;
}
bool RpcProtocolServerV2::ValidateRequestFields(const Json::Value &request)
{
    if (!request.isObject())
        return false;
    if (!(request.isMember(KEY_REQUEST_METHODNAME) && request[KEY_REQUEST_METHODNAME].isString()))
        return false;
    if (!(request.isMember(KEY_REQUEST_VERSION) && request[KEY_REQUEST_VERSION].isString() && request[KEY_REQUEST_VERSION].asString() == JSON_RPC_VERSION))
        return false;
    if (request.isMember(KEY_REQUEST_ID) && !(request[KEY_REQUEST_ID].isInt() || request[KEY_REQUEST_ID].isString() || request[KEY_REQUEST_ID].isNull()))
        return false;
    if (request.isMember(KEY_REQUEST_PARAMETERS) && !(request[KEY_REQUEST_PARAMETERS].isObject() || request[KEY_REQUEST_PARAMETERS].isArray() || request[KEY_REQUEST_ID].isNull()))
        return false;
    return true;
}
void RpcProtocolServerV2::ProcessRequest      (const Json::Value& request, Json::Value& response)
{
    Procedure& method = this->procedures[request[KEY_REQUEST_METHODNAME].asString()];
    Json::Value result;

    if (method.GetProcedureType() == RPC_METHOD)
    {
        handler.HandleMethodCall(method, request[KEY_REQUEST_PARAMETERS],
                                        result);
        response[KEY_REQUEST_VERSION] = JSON_RPC_VERSION;
        response[KEY_RESPONSE_RESULT] = result;
        response[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
    }
    else
    {
        handler.HandleNotificationCall(method, request[KEY_REQUEST_PARAMETERS]);
        response = Json::Value::null;
    }
}
