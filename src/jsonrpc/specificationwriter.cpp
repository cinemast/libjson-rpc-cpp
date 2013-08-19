/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationwriter.cpp
 * @date    30.04.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "specificationwriter.h"
#include <fstream>
#include "json/json.h"
#include "json/writer.h"
#include <iostream>

using namespace std;

namespace jsonrpc
{
    Json::Value SpecificationWriter::toJsonValue(procedurelist_t& procedures)
    {
        procedurelist_t::iterator it;
        Json::Value result;
        Json::Value row;
        int i=0;
        for(it = procedures.begin(); it != procedures.end(); it++)
        {
            Procedure* proc = it->second;
            procedureToJsonValue(proc, row);
            result[i] = row;
            i++;
            row.clear();
        }
        return result;
    }

    std::string SpecificationWriter::toString(procedurelist_t& procedures)
    {
        Json::StyledWriter wr;
        return wr.write(toJsonValue(procedures));
    }

    void SpecificationWriter::toFile(const std::string &filename, procedurelist_t& procedures)
    {
        ofstream file;
        file.open(filename.c_str(), ios_base::out);
        file << toString(procedures);
        file.close();
    }

    Json::Value SpecificationWriter::toJsonLiteral(jsontype_t type)
    {
        Json::Value literal;
        switch(type)
        {
            case JSON_BOOLEAN:
                literal = true;
                break;
            case JSON_STRING:
                literal = "somestring";
                break;
            case JSON_REAL:
                literal = 1.0;
                break;
            case JSON_ARRAY:
                literal = Json::arrayValue;
                break;
            case JSON_OBJECT:
                literal["objectkey"] = "objectvalue";
                break;
            case JSON_INTEGER:
                literal = 1;
                break;
            case JSON_NULL:
                literal = Json::nullValue;
                break;
        }
        return literal;
    }

    void SpecificationWriter::procedureToJsonValue(Procedure *procedure, Json::Value &target)
    {
        if(procedure->GetProcedureType() == RPC_METHOD)
        {
            target[KEY_METHOD_NAME] = procedure->GetProcedureName();
            target[KEY_RETURN_TYPE] = toJsonLiteral(procedure->GetReturnType());
        }
        else
        {
            target[KEY_NOTIFICATION_NAME] = procedure->GetProcedureName();
        }
        int i=0;

        for(parameterNameList_t::const_iterator it = procedure->GetParameters().begin(); it != procedure->GetParameters().end(); it++)
        {
            if(procedure->GetParameterDeclarationType() == PARAMS_BY_NAME)
            {
                target[KEY_PROCEDURE_PARAMETERS][it->first] = toJsonLiteral(it->second);
            }
            else
            {
                target[KEY_PROCEDURE_PARAMETERS].append(toJsonLiteral(it->second));
            }
            i++;
        }

        if(i == 0)
        {
            target[KEY_PROCEDURE_PARAMETERS] = Json::nullValue;
        }
    }
}
