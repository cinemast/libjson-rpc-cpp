/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    requesthandler.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "requesthandler.h"
#include "errors.h"

using namespace std;

namespace jsonrpc
{
    RequestHandler::RequestHandler(procedurelist_t *procedures, AbstractAuthenticator *auth) :
        procedures(procedures),
        authManager(auth)
    {
    }

    RequestHandler::~RequestHandler()
    {
        for (map<string, Procedure*>::iterator it = this->procedures->begin();
             it != this->procedures->end(); it++)
        {
            delete it->second;
        }
        if (this->authManager != NULL)
        {
            delete this->authManager;
        }

        delete this->procedures;
    }

    void RequestHandler::HandleRequest(const std::string& request,
                                       std::string& retValue)
    {
        Json::Reader reader;
        Json::Value req;
        Json::Value response, resp;
        Json::FastWriter w;
        int error;

        if (reader.parse(request, req, false))
        {
            //It could be a Batch Request
            if (req.isArray())
            {
                for (unsigned int i = 0; i < req.size(); i++)
                {
                    error = this->ValidateRequest(req[i]);
                    if (error == 0)
                    {
                        this->ProcessRequest(req[i], resp);
                        if (!resp.isNull())
                        {
                            response[i] = resp;
                        }
                    }
                    else
                    {
                        response[i] = Errors::GetErrorBlock(req[i],error);
                    }
                }
                //It could be a simple Request
            }
            else if (req.isObject())
            {
                error = this->ValidateRequest(req);
                if (error == 0)
                {
                    this->ProcessRequest(req, response);
                }
                else
                {
                    response = Errors::GetErrorBlock(req, error);
                }
            }
        }
        else
        {
            response = Errors::GetErrorBlock(Json::nullValue, Errors::ERROR_RPC_JSON_PARSE_ERROR);
        }
        retValue = w.write(response);
    }

    int RequestHandler::ValidateRequest(const Json::Value& request)
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
                error = proc->ValdiateParameters(request[KEY_REQUEST_PARAMETERS]);
                if (error == 0)
                {
                    if (request.isMember(KEY_REQUEST_ID))
                    {
                        if (proc->GetProcedureType() == RPC_NOTIFICATION)
                        {
                            error = Errors::ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
                        }
                    }
                    else if (proc->GetProcedureType() == RPC_METHOD)
                    {
                        error = Errors::ERROR_SERVER_PROCEDURE_IS_METHOD;
                    }
                    if (this->authManager != NULL)
                    {
                        error = this->authManager->CheckPermission(
                                    request[KEY_AUTHENTICATION],
                                    proc->GetProcedureName());
                    }
                }
            }
            else
            {
                error = Errors::ERROR_RPC_METHOD_NOT_FOUND;
            }
        }
        return error;
    }

    void RequestHandler::ProcessRequest(const Json::Value& request,
                                        Json::Value& response)
    {
        Procedure* method = (*this->procedures)[request[KEY_REQUEST_METHODNAME].asString()];
        Json::Value result;
        if (method->GetProcedureType() == RPC_METHOD)
        {
            (*method->GetMethodPointer())(request[KEY_REQUEST_PARAMETERS],
                                          result);
            //cout << "got result" << endl;
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
            (*method->GetNotificationPointer())(
                        request[KEY_REQUEST_PARAMETERS]);
            response = Json::Value::null;
        }
    }
} /* namespace jsonrpc */

