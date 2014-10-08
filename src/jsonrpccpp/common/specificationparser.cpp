/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationparser.cpp
 * @date    12.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "specificationparser.h"
#include <jsoncpp/json/reader.h>
#include <fstream>

using namespace std;
using namespace jsonrpc;


vector<Procedure>   SpecificationParser::GetProceduresFromFile  (const string &filename) throw(JsonRpcException)
{
    string content;
    GetFileContent(filename, content);
    return GetProceduresFromString(content);
}
vector<Procedure>   SpecificationParser::GetProceduresFromString(const string &content) throw(JsonRpcException)
{

    Json::Reader reader;
    Json::Value val;
    if(!reader.parse(content,val)) {
        throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " specification file contains syntax errors");
    }

    vector<Procedure> result;
    for (unsigned int i = 0; i < val.size(); i++)
    {
        Procedure proc;
        GetProcedure(val[i], proc);
        result.push_back(proc);
    }
    return result;
}
void                SpecificationParser::GetProcedure           (Json::Value &signature, Procedure &result)
{
    if (signature.isMember(KEY_SPEC_PROCEDURE_NAME) && signature.isMember(KEY_SPEC_PROCEDURE_PARAMETERS))
    {
        if (signature[KEY_SPEC_PROCEDURE_NAME].isString() &&
                (signature[KEY_SPEC_PROCEDURE_PARAMETERS].isObject() ||
                 signature[KEY_SPEC_PROCEDURE_PARAMETERS].isNull()   ||
                 signature[KEY_SPEC_PROCEDURE_PARAMETERS].isArray()))
        {
            result.SetProcedureName(signature[KEY_SPEC_PROCEDURE_NAME].asString());
            if (signature.isMember(KEY_SPEC_RETURN_TYPE))
            {
                result.SetProcedureType(RPC_METHOD);
                result.SetReturnType(toJsonType(signature[KEY_SPEC_RETURN_TYPE]));
            }
            else
            {
                result.SetProcedureType(RPC_NOTIFICATION);
            }

            if (signature[KEY_SPEC_PROCEDURE_PARAMETERS].isArray())
            {
                GetPositionalParameters(signature, result);
            }
            else if (signature[KEY_SPEC_PROCEDURE_PARAMETERS].isObject())
            {
                GetNamedParameters(signature, result);
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
                               "procedure declaration does not contain name or paramters: "
                               + signature.toStyledString());
    }
}
void                SpecificationParser::GetFileContent         (const std::string &filename, std::string& target)
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
jsontype_t          SpecificationParser::toJsonType             (Json::Value &val)
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
void                SpecificationParser::GetPositionalParameters(Json::Value &val, Procedure &result)
{
    //Positional parameters
    for (unsigned int i=0; i < val[KEY_SPEC_PROCEDURE_PARAMETERS].size(); i++)
    {
        stringstream paramname;
        paramname << "param" << (i+1);
        result.AddParameter(paramname.str(), toJsonType(val[KEY_SPEC_PROCEDURE_PARAMETERS][i]));
    }
}
void                SpecificationParser::GetNamedParameters(Json::Value &val, Procedure &result)
{
    vector<string> parameters = val[KEY_SPEC_PROCEDURE_PARAMETERS].getMemberNames();
    for (unsigned int i = 0; i < parameters.size(); i++)
    {
        result.AddParameter(parameters.at(i), toJsonType(val[KEY_SPEC_PROCEDURE_PARAMETERS][parameters.at(i)]));
    }
}
