/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserver.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolserver.h"
#include "errors.h"
#include "server.h"

#include <iostream>

using namespace std;

namespace jsonrpc
{
    RpcProtocolServer::RpcProtocolServer(AbstractRequestHandler* server, procedurelist_t *procedures, AbstractAuthenticator* auth) :
        procedures(procedures),
        authManager(auth),
        server(server)
    {
    }

    RpcProtocolServer::RpcProtocolServer(AbstractRequestHandler* server, AbstractAuthenticator* auth) :
        procedures(new procedurelist_t()),
        authManager(auth),
        server(server)
    {
    }

    RpcProtocolServer::~RpcProtocolServer()
    {
        for (map<string, Procedure*>::iterator it = this->procedures->begin();
             it != this->procedures->end(); it++)
        {
            delete it->second;
        }
        this->SetAuthenticator(NULL);
        this->procedures->clear();
        delete this->procedures;
    }

    void RpcProtocolServer::HandleRequest(const std::string& request,
                                          std::string& retValue)
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
        }
        else
        {
            response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_JSON_PARSE_ERROR);
        }
        retValue = w.write(response);
    }

    void RpcProtocolServer::SetAuthenticator(AbstractAuthenticator *auth)
    {
        if(this->authManager != NULL)
        {
            delete this->authManager;
        }
        this->authManager = auth;
    }

    void RpcProtocolServer::HandleSingleRequest(Json::Value &req, Json::Value& response)
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

    void RpcProtocolServer::HandleBatchRequest(Json::Value &req, Json::Value& response)
    {
        for (unsigned int i = 0; i < req.size(); i++)
        {
            this->HandleSingleRequest(req[i], response[i]);
        }
    }

    int RpcProtocolServer::ValidateRequest(const Json::Value& request)
    {
        int error = 0;
        Procedure* proc;
        if (!(request.isMember(KEY_REQUEST_METHODNAME)
              && request.isMember(KEY_REQUEST_VERSION)
              && request.isMember(KEY_REQUEST_PARAMETERS)))
        {
            error = Errors::ERROR_RPC_INVALID_REQUEST;
        }
        else
        {
            map<string, Procedure*>::iterator it = procedures->find(request[KEY_REQUEST_METHODNAME].asString());
            if (it != this->procedures->end())
            {
                proc = (*this->procedures)[request[KEY_REQUEST_METHODNAME].asString()];
                if(request.isMember(KEY_REQUEST_ID) && proc->GetProcedureType() == RPC_NOTIFICATION)
                {
                    error = Errors::ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
                }
                else if(!request.isMember(KEY_REQUEST_ID) && proc->GetProcedureType() == RPC_METHOD)
                {
                    error = Errors::ERROR_SERVER_PROCEDURE_IS_METHOD;
                }
                else if (proc->ValdiateParameters(request[KEY_REQUEST_PARAMETERS]))
                {
                    if (this->authManager != NULL)
                    {
                        error = this->authManager->CheckPermission(
                                    request[KEY_AUTHENTICATION],
                                    proc->GetProcedureName());
                    }
                }
                else
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

    void RpcProtocolServer::ProcessRequest(const Json::Value& request,
                                           Json::Value& response)
    {
        Procedure* method = (*this->procedures)[request[KEY_REQUEST_METHODNAME].asString()];
        Json::Value result;

        if (method->GetProcedureType() == RPC_METHOD)
        {
            server->handleMethodCall(method, request[KEY_REQUEST_PARAMETERS],
                                     result);
            response[KEY_REQUEST_VERSION] = JSON_RPC_VERSION;
            response[KEY_RESPONSE_RESULT] = result;
            response[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
            if (this->authManager != NULL)
            {
                this->authManager->ProcessAuthentication(
                            request[KEY_AUTHENTICATION],
                            response[KEY_AUTHENTICATION]);
            }
        }
        else
        {
            server->handleNotificationCall(method, request[KEY_REQUEST_PARAMETERS]);
            response = Json::Value::null;
        }
    }

    void RpcProtocolServer::AddProcedure(Procedure *procedure)
    {
        (*this->procedures)[procedure->GetProcedureName()] = procedure;
    }

    procedurelist_t& RpcProtocolServer::GetProcedures()
    {
        return *(this->procedures);
    }

} /* namespace jsonrpc */

