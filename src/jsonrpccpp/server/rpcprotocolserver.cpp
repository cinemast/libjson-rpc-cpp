/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserver.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolserver.h"
#include <jsonrpccpp/common/errors.h>
#include <iostream>

using namespace std;
using namespace jsonrpc;

RpcProtocolServer::RpcProtocolServer(AbstractRequestHandler &requestHandler, vector<Procedure> &procedures) :
    requestHandler(requestHandler)
{
    for (unsigned int i=0; i < procedures.size(); i++)
    {
        this->procedures[procedures.at(i).GetProcedureName()] = new Procedure(procedures.at(i));
    }
}
RpcProtocolServer::RpcProtocolServer(AbstractRequestHandler &requestHandler) :
    requestHandler(requestHandler)
{
}
RpcProtocolServer::~RpcProtocolServer()
{
    for (map<string, Procedure*>::iterator it = procedures.begin(); it != procedures.end(); it++)
    {
        delete it->second;
    }
}

void RpcProtocolServer::HandleRequest       (const string& request, string& retValue)
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
    retValue = w.write(response);
}
void RpcProtocolServer::HandleSingleRequest (Json::Value &req, Json::Value& response)
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
void RpcProtocolServer::HandleBatchRequest  (Json::Value &req, Json::Value& response)
{
    if (req.size() == 0)
        response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_INVALID_REQUEST);
    else
    {
        for (unsigned int i = 0; i < req.size(); i++)
        {
            Json::Value result;
            this->HandleSingleRequest(req[i], result);
            response.append(result);
        }
    }
}
int  RpcProtocolServer::ValidateRequest     (const Json::Value& request)
{
    int error = 0;
    Procedure* proc;
    if (!request.isObject() || !(request.isMember(KEY_REQUEST_METHODNAME)
                                 && request[KEY_REQUEST_METHODNAME].isString()
                                 && request.isMember(KEY_REQUEST_VERSION)
                                 && request[KEY_REQUEST_VERSION].isString()
                                 && request[KEY_REQUEST_VERSION].asString() == "2.0"
                                 && request.isMember(KEY_REQUEST_PARAMETERS)
                                 && (request[KEY_REQUEST_PARAMETERS].isObject() || request[KEY_REQUEST_PARAMETERS].isArray() || request[KEY_REQUEST_PARAMETERS].isNull())) || (request.isMember("id") && !request["id"].isInt()))
    {
        error = Errors::ERROR_RPC_INVALID_REQUEST;
    }
    else
    {
        map<string, Procedure*>::iterator it = this->procedures.find(request[KEY_REQUEST_METHODNAME].asString());
        if (it != this->procedures.end())
        {
            proc = it->second;
            if(request.isMember(KEY_REQUEST_ID) && proc->GetProcedureType() == RPC_NOTIFICATION)
            {
                error = Errors::ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
            }
            else if(!request.isMember(KEY_REQUEST_ID) && proc->GetProcedureType() == RPC_METHOD)
            {
                error = Errors::ERROR_SERVER_PROCEDURE_IS_METHOD;
            }
            else if (!proc->ValdiateParameters(request[KEY_REQUEST_PARAMETERS]))
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
void RpcProtocolServer::ProcessRequest      (const Json::Value& request, Json::Value& response)
{
    Procedure* method = this->procedures[request[KEY_REQUEST_METHODNAME].asString()];
    Json::Value result;

    if (method->GetProcedureType() == RPC_METHOD)
    {
        requestHandler.handleMethodCall(*method, request[KEY_REQUEST_PARAMETERS],
                                        result);
        response[KEY_REQUEST_VERSION] = JSON_RPC_VERSION;
        response[KEY_RESPONSE_RESULT] = result;
        response[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
    }
    else
    {
        requestHandler.handleNotificationCall(*method, request[KEY_REQUEST_PARAMETERS]);
        response = Json::Value::null;
    }
}
void RpcProtocolServer::AddProcedure        (Procedure &procedure)
{
    this->procedures[procedure.GetProcedureName()] = new Procedure(procedure);
}

void RpcProtocolServer::AddProcedure(Procedure *procedure)
{
    this->procedures[procedure->GetProcedureName()] = procedure;
}
