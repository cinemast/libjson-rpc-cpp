/**
 * @file requesthandler.cpp
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "requesthandler.h"
#include "errors.h"

using namespace std;

namespace jsonrpc
{
    //TODO: Error Handling
    void createErrorBlock(int errorCode, const Json::Value& request,
            Json::Value& errorBlock)
    {
        errorBlock[KEY_REQUEST_VERSION] = JSON_RPC_VERSION;
        if (!request.isNull() && request.isMember(KEY_REQUEST_ID))
        {
            errorBlock[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
        }
        else if (request.isNull())
        {
            errorBlock[KEY_REQUEST_ID] = Json::Value::null;
        }
        errorBlock[KEY_RESPONSE_ERROR] = Errors::GetInstance()->GetErrorBlock(
                errorCode);
    }

    RequestHandler::RequestHandler(const std::string& instanceName)
            : instanceName(instanceName), authManager(NULL)
    {
    }

    RequestHandler::~RequestHandler()
    {
        for (map<string, Procedure*>::iterator it = this->procedures.begin();
                it != this->procedures.end(); it++)
        {
            delete it->second;
        }
        if (this->authManager != NULL)
        {
            delete this->authManager;
        }
    }

    void RequestHandler::AddObserver(observerFunction fp, observer_t t)
    {
        switch (t)
        {
            case ON_REQUEST:
                this->requestObservers.push_back(fp);
                break;
            case ON_RESPONSE:
                this->responseObservers.push_back(fp);
                break;
            case ON_REQUEST_RESPONSE:
                this->requestObservers.push_back(fp);
                this->responseObservers.push_back(fp);
                break;
        }
    }

    void RequestHandler::RemoveObserver(observerFunction fp)
    {
        bool found = false;
        unsigned int i = 0;
        while (found == false && i < this->requestObservers.size())
        {
            if (this->requestObservers.at(i) == fp)
            {
                found = true;
                this->requestObservers.erase(
                        this->requestObservers.begin() + i);
            }
            else
            {
                i++;
            }
        }
        found = false;
        while (found == false && i < this->responseObservers.size())
        {
            if (this->responseObservers.at(i) == fp)
            {
                found = true;
                this->responseObservers.erase(
                        this->responseObservers.begin() + i);
            }
            else
            {
                i++;
            }
        }
    }

    bool RequestHandler::AddProcedure(Procedure* procedure)
    {
        if (procedure != NULL)
        {
            this->procedures[procedure->GetProcedureName()] = procedure;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool RequestHandler::RemoveProcedure(const std::string& procedure)
    {
        map<string, Procedure*>::iterator it = this->procedures.find(procedure);
        if (it != this->procedures.end())
        {
            this->procedures.erase(it);
            return true;
        }
        else
        {
            return false;
        }
    }

    const Authenticator& RequestHandler::GetAuthManager() const
    {
        return *this->authManager;
    }

    const std::string& RequestHandler::GetInstanceName() const
    {
        return this->instanceName;
    }

    const std::vector<observerFunction>& RequestHandler::GetResponseObservers() const
    {
        return this->responseObservers;
    }

    const std::vector<observerFunction>& RequestHandler::GetRequestObservers() const
    {
        return this->requestObservers;
    }

    const procedurelist_t& RequestHandler::GetProcedures() const
    {
        return this->procedures;
    }

    void RequestHandler::SetAuthManager(Authenticator* authManager)
    {
        this->authManager = authManager;
    }

    void RequestHandler::SetProcedures(const procedurelist_t& procedures)
    {
        this->procedures = procedures;
    }

    void RequestHandler::HandleRequest(const std::string& request,
            std::string& retValue)
    {
        Json::Reader reader;
        Json::Value req;
        Json::Value response, resp;
        Json::FastWriter w;
        int error;

        //  cout << "Request was: " << request << endl;

        if (reader.parse(request, req))
        {
            this->NotifyObservers(this->requestObservers, req);
            //It could be a Batch Request
            if (req.isArray())
            {
                for (unsigned int i = 0; i < req.size(); i++)
                {
                    error = this->ValidateRequest(req[i]);
                    if (error == ERROR_NO)
                    {
                        this->ProcessRequest(req[i], resp);
                        if (!resp.isNull())
                        {
                            response[i] = resp;
                        }
                    }
                    else
                    {
                        createErrorBlock(error, req[i], resp);
                        response[i] = resp;
                    }
                }
                //It could be a simple Request
            }
            else if (req.isObject())
            {
                //cout << "validating request " << endl;
                error = this->ValidateRequest(req);
                if (error == ERROR_NO)
                {
                    //cout << "processing request" << endl;
                    this->ProcessRequest(req, response);
                }
                else
                {
                    createErrorBlock(error, req, response);
                }
            }
        }
        else
        {
            createErrorBlock(ERROR_JSON_PARSE_ERROR, Json::Value::null,
                    response);
        }
        retValue = w.write(response);
        this->NotifyObservers(this->requestObservers, req);
    }

    int RequestHandler::ValidateRequest(const Json::Value& request)
    {
        int error = ERROR_NO;
        Procedure* proc;
        if (!(request.isMember(KEY_REQUEST_METHODNAME)
                && request.isMember(KEY_REQUEST_VERSION)
                && request.isMember(KEY_REQUEST_PARAMETERS)))
        {
            error = ERROR_INVALID_JSON_REQUEST;
        }
        else
        {
            map<string, Procedure*>::iterator it = procedures.find(
                    request[KEY_REQUEST_METHODNAME].asString());

            if (it != this->procedures.end())
            {
                proc =
                        this->procedures[request[KEY_REQUEST_METHODNAME].asString()];
                error = proc->ValdiateParameters(
                        request[KEY_REQUEST_PARAMETERS]);
                if (error == ERROR_NO)
                {
                    if (proc->GetMethodPointer() == NULL
                            && proc->GetNotificationPointer() == NULL)
                    {
                        error = ERROR_PROCEDURE_POINTER_IS_NULL;
                    }
                    else if (request.isMember(KEY_REQUEST_ID))
                    {
                        if (proc->GetProcedureType() == RPC_NOTIFICATION)
                        {
                            error = ERROR_PROCEDURE_IS_NOTIFICATION;
                        }
                    }
                    else if (proc->GetProcedureType() == RPC_METHOD)
                    {
                        error = ERROR_PROCEDURE_IS_METHOD;
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
                error = ERROR_METHOD_NOT_FOUND;
            }
        }
        return error;
    }

    void RequestHandler::ProcessRequest(const Json::Value& request,
            Json::Value& retValue)
    {
    }

    void RequestHandler::NotifyObservers(
            const std::vector<observerFunction>& observerGroup,
            const Json::Value& request)
    {
    }

} /* namespace jsonrpc */

