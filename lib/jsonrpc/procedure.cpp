/**
 * @file procedure.cpp
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "procedure.h"
#include "exception.h"
#include "errors.h"

#include <vector>

using namespace std;

namespace jsonrpc
{
    
    Procedure::~Procedure()
    {
        this->parameters.clear();
    }

    Procedure::Procedure(const std::string& name,
            const procedure_t procedure_type, const parameterlist_t& parameters)
            : procedureName(name), procedureType(procedure_type), parameters(
                    parameters)
    {
        this->procedurePointer.np = NULL;
        this->procedurePointer.rp = NULL;
    }

    Procedure::Procedure(const Json::Value& signature)
    {
        if ((signature.isMember(KEY_METHOD_NAME)
                || signature.isMember(KEY_NOTIFICATION_NAME))
                && signature.isMember(KEY_PROCEDURE_PARAMETERS))
        {
            std::string procedure_name;
            if (signature.isMember(KEY_METHOD_NAME))
            {
                procedure_name = KEY_METHOD_NAME;
                this->procedureType = RPC_METHOD;
            }
            else
            {
                procedure_name = KEY_NOTIFICATION_NAME;
                this->procedureType = RPC_NOTIFICATION;
            }
            if (signature[procedure_name].isString()
                    && (signature[KEY_PROCEDURE_PARAMETERS].isObject() || signature[KEY_PROCEDURE_PARAMETERS].isNull()))
            {
                this->procedureName = signature[procedure_name].asString();
                vector<string> parameters =
                        signature[KEY_PROCEDURE_PARAMETERS].getMemberNames();
                for (unsigned int i = 0; i < parameters.size(); i++)
                {
                    switch (signature[KEY_PROCEDURE_PARAMETERS][parameters.at(i)].type())
                    {
                        case Json::uintValue:
                        case Json::intValue:
                            this->parameters[parameters.at(i)] = JSON_INTEGER;
                            break;

                        case Json::realValue:
                            this->parameters[parameters.at(i)] = JSON_REAL;
                            break;
                        case Json::stringValue:
                            this->parameters[parameters.at(i)] = JSON_STRING;
                            break;
                        case Json::booleanValue:
                            this->parameters[parameters.at(i)] = JSON_BOOLEAN;
                            break;
                        case Json::arrayValue:
                            this->parameters[parameters.at(i)] = JSON_ARRAY;
                            break;
                        case Json::objectValue:
                            this->parameters[parameters.at(i)] = JSON_OBJECT;
                            break;
                        default:
                            throw Exception(ERROR_PROCEDURE_PARSE_ERROR,
                                    "Unknown parameter in "
                                            + signature.toStyledString());
                    }
                }
                this->procedurePointer.np = NULL;
                this->procedurePointer.rp = NULL;
            }
            else
            {
                throw Exception(ERROR_PROCEDURE_PARSE_ERROR,
                        "Invalid signature types in fileds: "
                                + signature.toStyledString());
            }
        }
        else
        {
            throw Exception(ERROR_PROCEDURE_PARSE_ERROR,
                    "procedure declaration does not contain method/notification name or paramters: "
                            + signature.toStyledString());
        }
    }

    int Procedure::ValdiateParameters(const Json::Value& parameters)
    {
        map<string, jsontype_t>::iterator it = this->parameters.begin();
        bool ok = true;
        while (ok == true && it != this->parameters.end())
        {
            if (!parameters.isMember(it->first.c_str()))
            {
                ok = false;
            }
            else
            {
                switch (it->second)
                {
                    case JSON_STRING:
                        if (!parameters[it->first].isString())
                            ok = false;
                        break;
                    case JSON_BOOLEAN:
                        if (!parameters[it->first].isBool())
                            ok = false;
                        break;
                    case JSON_INTEGER:
                        if (!parameters[it->first].isInt())
                            ok = false;
                        break;
                    case JSON_REAL:
                        if (!parameters[it->first].isDouble())
                            ok = false;
                        break;
                    case JSON_OBJECT:
                        if (!parameters[it->first].isObject())
                            ok = false;
                        break;
                    case JSON_ARRAY:
                        if (!parameters[it->first].isArray())
                            ok = false;
                        break;
                }
            }
            it++;
        }
        if (ok == true)
        {
            return ERROR_NO;
        }
        else
        {
            return ERROR_INVALID_PARAMS;
        }
    }

    const parameterlist_t& Procedure::GetParameters() const
    {
        return this->parameters;
    }

    procedure_t Procedure::GetProcedureType() const
    {
        return this->procedureType;
    }

    const std::string& jsonrpc::Procedure::GetProcedureName() const
    {
        return this->procedureName;
    }

    pRequest_t jsonrpc::Procedure::GetMethodPointer()
    {
        if (this->procedureType == RPC_METHOD)
        {
            return (pRequest_t) this->procedurePointer.rp;
        }
        else
        {
            return NULL;
        }
    }

    pNotification_t jsonrpc::Procedure::GetNotificationPointer()
    {
        if (this->procedureType == RPC_NOTIFICATION)
        {
            return (pNotification_t) this->procedurePointer.np;
        }
        else
        {
            return NULL;
        }
    }

    bool Procedure::SetMethodPointer(pRequest_t rp)
    {
        if (this->procedureType == RPC_METHOD)
        {
            this->procedurePointer.rp = rp;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Procedure::SetNotificationPointer(pNotification_t np)
    {
        if (this->procedureType == RPC_NOTIFICATION)
        {
            this->procedurePointer.np = np;
            return true;
        }
        else
        {
            return false;
        }
    }

} /* namespace jsonrpc */

