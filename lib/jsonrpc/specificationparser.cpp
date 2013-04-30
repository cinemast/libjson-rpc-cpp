/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationparser.cpp
 * @date    12.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "specificationparser.h"
#include <json/reader.h>
#include <fstream>

using namespace std;

namespace jsonrpc {

    procedurelist_t* SpecificationParser::GetProcedures(const std::string &filename, methodpointer_t &methods, notificationpointer_t &notifications) throw (JsonRpcException)
    {
        string content;
        GetFileContent(filename,content);

        Json::Reader reader;
        Json::Value val;
        if(!reader.parse(content,val)) {
            throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " specification file contains syntax errors");
        }

        procedurelist_t* procedures = new procedurelist_t();
        Procedure* proc;
        methodpointer_t::const_iterator it_methods;
        notificationpointer_t::const_iterator it_notifications;
        for (unsigned int i = 0; i < val.size(); i++)
        {
            proc = new Procedure(val[i]);
            (*procedures)[proc->GetProcedureName()] = proc;
            if (proc->GetProcedureType() == RPC_METHOD)
            {
                it_methods = methods.find(proc->GetProcedureName());
                if (it_methods != methods.end())
                {
                    proc->SetMethodPointer(it_methods->second);
                }
                else
                {
                    throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_POINTER_IS_NULL, proc->GetProcedureName());
                }
            }
            else
            {
                it_notifications = notifications.find(proc->GetProcedureName());
                if (it_notifications != notifications.end())
                {
                    proc->SetNotificationPointer(it_notifications->second);
                }
                else
                {
                    throw JsonRpcException(Errors::ERROR_SERVER_PROCEDURE_POINTER_IS_NULL, proc->GetProcedureName());
                }
            }
        }
        return procedures;
    }

    procedurelist_t *SpecificationParser::GetProcedures(const string &filename) throw(JsonRpcException)
    {
        string content;
        GetFileContent(filename,content);

        Json::Reader reader;
        Json::Value val;
        if(!reader.parse(content,val)) {
            throw JsonRpcException(Errors::ERROR_RPC_JSON_PARSE_ERROR, " specification file contains syntax errors");
        }

        procedurelist_t* procedures = new procedurelist_t();
        Procedure* proc;
        methodpointer_t::const_iterator it_methods;
        notificationpointer_t::const_iterator it_notifications;
        for (unsigned int i = 0; i < val.size(); i++)
        {
            proc = new Procedure(val[i]);
            (*procedures)[proc->GetProcedureName()] = proc;
        }
        return procedures;
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
}
