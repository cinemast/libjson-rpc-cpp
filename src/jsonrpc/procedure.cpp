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
    Procedure::Procedure(const string name, jsontype_t returntype, ...)
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
    }

    Procedure::Procedure(const string name, ...)
    {
        va_list parameters;
        va_start(parameters, name);
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
    }

    Procedure::~Procedure()
    {
        this->parameters.clear();
    }

    bool Procedure::ValdiateParameters(const Json::Value& parameters)
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
        return ok;
    }

    parameterlist_t& Procedure::GetParameters()
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

    jsontype_t Procedure::GetReturnType() const
    {
        return this->returntype;
    }

    void Procedure::AddParameter(const string& name, jsontype_t type)
    {
        this->parameters[name] = type;
    }

} /* namespace jsonrpc */

