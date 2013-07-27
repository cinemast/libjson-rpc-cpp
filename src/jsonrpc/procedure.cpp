/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    procedure.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "procedure.h"
#include "exception.h"
#include "errors.h"

#include <vector>
#include <cstdarg>
#include <string>
using namespace std;

namespace jsonrpc
{
    Procedure::Procedure(const string name, parameterDeclaration_t paramType, jsontype_t returntype, ...)
    {
        va_list parameters;
        va_start(parameters, returntype);
        const char* paramname = va_arg(parameters, const char*);
        jsontype_t type;
        while(paramname != NULL) {
            type = (jsontype_t)va_arg(parameters, int);     //Needs to be tested
            this->AddParameter(paramname, type);
            paramname = va_arg(parameters, const char*);
        }
        va_end(parameters);
        this->procedureName = name;
        this->returntype = returntype;
        this->procedureType = RPC_METHOD;
        this->paramDeclaration = paramType;
    }

    Procedure::Procedure(const string name, parameterDeclaration_t paramType, ...)
    {
        va_list parameters;
        va_start(parameters, paramType);
        const char* paramname = va_arg(parameters, const char*);
        jsontype_t type;
        while(paramname != NULL) {
            type = (jsontype_t)va_arg(parameters, int);     //Needs to be tested
            this->AddParameter(paramname, type);
            paramname = va_arg(parameters, const char*);
        }
        va_end(parameters);
        this->procedureName = name;
        this->procedureType = RPC_NOTIFICATION;
        this->paramDeclaration = paramType;
    }

    Procedure::~Procedure()
    {
        this->parametersName.clear();
    }

    bool Procedure::ValdiateParameters(const Json::Value& parameters)
    {
        if(parameters.isArray() && this->paramDeclaration == PARAMS_BY_POSITION)
        {
            return this->ValidatePositionalParameters(parameters);
        }
        else if(parameters.isObject() && this->paramDeclaration == PARAMS_BY_NAME)
        {
            return this->ValidateNamedParameters(parameters);
        }
        else
        {
            return false;
        }
    }
    parameterNameList_t& Procedure::GetParameters()
    {
        return this->parametersName;
    }

    procedure_t Procedure::GetProcedureType() const
    {
        return this->procedureType;
    }

    const std::string& jsonrpc::Procedure::GetProcedureName() const
    {
        return this->procedureName;
    }

    jsontype_t Procedure::GetReturnType() const
    {
        return this->returntype;
    }

    void Procedure::AddParameter(const string& name, jsontype_t type)
    {
        this->parametersName[name] = type;
        this->parametersPosition.push_back(type);
    }

    parameterDeclaration_t Procedure::GetParameterDeclarationType()
    {
        return this->paramDeclaration;
    }

    bool Procedure::ValidateNamedParameters(const Json::Value &parameters)
    {
        map<string, jsontype_t>::iterator it = this->parametersName.begin();
        bool ok = true;
        while (ok == true && it != this->parametersName.end())
        {
            if (!parameters.isMember(it->first.c_str()))
            {
                ok = false;
            }
            else
            {
                ok = this->ValidateSingleParameter(it->second, parameters[it->first]);
            }
            it++;
        }
        return ok;
    }

    bool Procedure::ValidatePositionalParameters(const Json::Value &parameters)
    {
        bool ok = true;

        if(parameters.size() != this->parametersPosition.size())
        {
            return false;
        }

        for(unsigned int i=0; ok && i < this->parametersPosition.size(); i++)
        {
            ok = this->ValidateSingleParameter(this->parametersPosition.at(i), parameters[i]);
        }
        return ok;
    }

    bool Procedure::ValidateSingleParameter(jsontype_t expectedType, const Json::Value &value)
    {
        bool ok = true;
        switch (expectedType)
        {
            case JSON_STRING:
                if (!value.isString())
                    ok = false;
                break;
            case JSON_BOOLEAN:
                if (!value.isBool())
                    ok = false;
                break;
            case JSON_INTEGER:
                if (!value.isInt())
                    ok = false;
                break;
            case JSON_REAL:
                if (!value.isDouble())
                    ok = false;
                break;
            case JSON_OBJECT:
                if (!value.isObject())
                    ok = false;
                break;
            case JSON_ARRAY:
                if (!value.isArray())
                    ok = false;
                break;
            case JSON_NULL:
                if (!value.isNull())
                    ok = false;
                break;
        }
        return ok;
    }

} /* namespace jsonrpc */

