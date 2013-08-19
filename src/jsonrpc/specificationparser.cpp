/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationparser.cpp
 * @date    12.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "specificationparser.h"
#include "json/reader.h"
#include <fstream>

using namespace std;

namespace jsonrpc {

     procedurelist_t *SpecificationParser::GetProceduresFromFile(const string &filename) throw(JsonRpcException)
     {
         string content;
         GetFileContent(filename, content);
         return GetProceduresFromString(content);
     }

    procedurelist_t *SpecificationParser::GetProceduresFromString(const string &content) throw(JsonRpcException)
    {

        Json::Reader reader;
        Json::Value val;
        if(!reader.parse(content,val)) {
            throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " specification file contains syntax errors");
        }

        procedurelist_t* procedures = new procedurelist_t();
        Procedure* proc;
        for (unsigned int i = 0; i < val.size(); i++)
        {
            proc = GetProcedure(val[i]);
            (*procedures)[proc->GetProcedureName()] = proc;
        }
        return procedures;
    }

    Procedure* SpecificationParser::GetProcedure(Json::Value &signature)
    {
        procedure_t procedureType;
        std::string name_key;
        std::string procedureName;
        Procedure* result;

        if ((signature.isMember(KEY_METHOD_NAME)
             || signature.isMember(KEY_NOTIFICATION_NAME))
                && signature.isMember(KEY_PROCEDURE_PARAMETERS))
        {
            if (signature.isMember(KEY_METHOD_NAME))
            {
                name_key = KEY_METHOD_NAME;
                procedureType = RPC_METHOD;
            }
            else
            {
                name_key = KEY_NOTIFICATION_NAME;
                procedureType = RPC_NOTIFICATION;

            }
            if (signature[name_key].isString()
                    && (signature[KEY_PROCEDURE_PARAMETERS].isObject() || signature[KEY_PROCEDURE_PARAMETERS].isNull() || signature[KEY_PROCEDURE_PARAMETERS].isArray()))
            {
                procedureName = signature[name_key].asString();

                if(signature[KEY_PROCEDURE_PARAMETERS].isObject() || signature[KEY_PROCEDURE_PARAMETERS].isNull())
                {
                    if(procedureType == RPC_NOTIFICATION)
                    {
                        result = new Procedure(procedureName, PARAMS_BY_NAME, NULL);
                    }
                    else
                    {
                        jsontype_t returntype = JSON_OBJECT;
                        if(signature.isMember(KEY_RETURN_TYPE))
                        {
                            returntype = toJsonType(signature[KEY_RETURN_TYPE]);
                        }
                        result = new Procedure(procedureName, PARAMS_BY_NAME, returntype, NULL);
                    }
                    //Named parameters
                    vector<string> parameters =
                            signature[KEY_PROCEDURE_PARAMETERS].getMemberNames();

                    for (unsigned int i = 0; i < parameters.size(); i++)
                    {
                        result->AddParameter(parameters.at(i), toJsonType(signature[KEY_PROCEDURE_PARAMETERS][parameters.at(i)]));
                    }

                }
                else if(signature[KEY_PROCEDURE_PARAMETERS].isArray())
                {
                    if(procedureType == RPC_NOTIFICATION)
                    {
                        result = new Procedure(procedureName, PARAMS_BY_POSITION, NULL);
                    }
                    else
                    {
                        jsontype_t returntype = JSON_OBJECT;
                        if(signature.isMember(KEY_RETURN_TYPE))
                        {
                            returntype = toJsonType(signature[KEY_RETURN_TYPE]);
                        }
                        result = new Procedure(procedureName, PARAMS_BY_POSITION, returntype, NULL);
                    }
                    //Positional parameters
                    for (unsigned int i=0; i < signature[KEY_PROCEDURE_PARAMETERS].size(); i++)
                    {
                        stringstream paramname;
                        paramname << "param" << (i+1);
                        result->AddParameter(paramname.str(), toJsonType(signature[KEY_PROCEDURE_PARAMETERS][i]));
                    }
                }
            }
            else
            {
                throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX,
                                       "Invalid signature types in fileds: "
                                       + signature.toStyledString());
            }
        }
        else
        {
            throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX,
                                   "procedure declaration does not contain method/notification name or paramters: "
                                   + signature.toStyledString());
        }
        return result;
    }

    void SpecificationParser::GetFileContent(const std::string &filename, std::string& target)
    {
        ifstream config(filename.c_str());

        if (config)
        {
            config.open(filename.c_str(), ios::in);
            target.assign((std::istreambuf_iterator<char>(config)),
                          (std::istreambuf_iterator<char>()));
        }
        else
        {
            throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_SPECIFICATION_NOT_FOUND, filename);
        }
    }

    jsontype_t SpecificationParser::toJsonType(Json::Value &val)
    {
        jsontype_t result;
        switch(val.type())
        {
            case Json::uintValue:
            case Json::intValue:
                result =  JSON_INTEGER;
                break;
            case Json::realValue:
                result = JSON_REAL;
                break;
            case Json::stringValue:
                result = JSON_STRING;
                break;
            case Json::booleanValue:
                result = JSON_BOOLEAN;
                break;
            case Json::arrayValue:
                result = JSON_ARRAY;
                break;
            case Json::objectValue:
                result = JSON_OBJECT;
                break;
            case Json::nullValue:
                result = JSON_NULL;
            default:
                throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX,"Unknown parameter in "
                                       + val.toStyledString());
        }
        return result;
    }
}
