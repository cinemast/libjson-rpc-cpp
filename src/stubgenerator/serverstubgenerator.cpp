/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    serverstubgenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "serverstubgenerator.h"
#include "servertemplate.h"

#include <jsonrpc/specificationwriter.h>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace jsonrpc;


ServerStubGenerator::ServerStubGenerator(const std::string &stubname, const std::string &filename) :
    StubGenerator("Abstract"+stubname + "Server", filename)
{
}

string ServerStubGenerator::generateStub()
{
    string stub = TEMPLATE_SERVER_STUB;
    replaceAll(stub, "<stubname>", this->stubname);
    replaceAll(stub, "<procedurebindings>", this->generateBindings());
    replaceAll(stub, "<proceduredefinitions>", this->generateProcedureDefinitions());
    replaceAll(stub, "<abstractdefinitions>", this->generateAbstractDefinitions());

    string stub_upper = this->stubname;
    std::transform(stub_upper.begin(), stub_upper.end(), stub_upper.begin(),
                   ::toupper);
    replaceAll(stub, "<STUBNAME>", stub_upper);

    return stub;
}

string ServerStubGenerator::generateBindings()
{
    stringstream result;
    string tmp;
    Procedure* proc;

    for(procedurelist_t::iterator it = this->procedures->begin(); it != this->procedures->end(); it++)
    {
        proc = it->second;
        if(proc->GetProcedureType() == RPC_METHOD)
        {
            tmp = TEMPLATE_SERVER_METHODBINDING;
        }
        else
        {
            tmp = TEMPLATE_SERVER_NOTIFICATIONBINDING;
        }
        replaceAll(tmp, "<rpcprocedurename>", proc->GetProcedureName());
        replaceAll(tmp, "<procedurename>", normalizeString(proc->GetProcedureName()));
        replaceAll(tmp, "<returntype>", toString(proc->GetReturnType()));
        replaceAll(tmp, "<parameterlist>", generateBindingParameterlist(proc));
        replaceAll(tmp, "<stubname>", this->stubname);

        if(proc->GetParameterDeclarationType() == PARAMS_BY_NAME)
        {
            replaceAll(tmp, "<paramtype>", "jsonrpc::PARAMS_BY_NAME");
        }
        else
        {
            replaceAll(tmp, "<paramtype>", "jsonrpc::PARAMS_BY_POSITION");
        }

        result << tmp << endl;
    }
    return result.str();
}

string ServerStubGenerator::generateProcedureDefinitions()
{
    stringstream result;
    string tmp;
    Procedure* proc;
    for(procedurelist_t::iterator it = this->procedures->begin(); it != this->procedures->end(); it++)
    {
        proc = it->second;
        if(proc->GetProcedureType() == RPC_METHOD)
        {
            tmp = TEMPLATE_SERVER_METHODDEFINITION;
        }
        else
        {
            tmp = TEMPLATE_SERVER_NOTIFICAITONDEFINITION;
        }
        replaceAll(tmp,"<procedurename>", normalizeString(proc->GetProcedureName()));
        replaceAll(tmp,"<parametermapping>", this->generateParameterMapping(proc));
        result << tmp << endl;
    }
    return result.str();
}

string ServerStubGenerator::generateAbstractDefinitions()
{
    stringstream result;
    string tmp;
    Procedure* proc;

    for(procedurelist_t::iterator it = this->procedures->begin(); it != this->procedures->end(); it++)
    {
        proc = it->second;
        tmp = TEMPLATE_SERVER_ABSTRACTDEFINITION;
        string returntype ="void";
        if(proc->GetProcedureType() == RPC_METHOD)
        {
            returntype = toCppType(proc->GetReturnType());
        }
        replaceAll(tmp, "<returntype>", returntype);
        replaceAll(tmp, "<procedurename>", normalizeString(proc->GetProcedureName()));
        replaceAll(tmp, "<parameterlist>", generateParameterDeclarationList(*proc));
        result << tmp;
    }
    return result.str();
}

string ServerStubGenerator::generateBindingParameterlist(Procedure *proc)
{
    stringstream parameter;
    parameterNameList_t& list = proc->GetParameters();

    for(parameterNameList_t::iterator it2 = list.begin(); it2 != list.end(); it2++)
    {
        parameter << "\"" << it2->first << "\"," << toString(it2->second) << ",";
    }
    return parameter.str();
}

string ServerStubGenerator::generateParameterMapping(Procedure *proc)
{
    stringstream parameter;
    string tmp;
    parameterNameList_t& params = proc->GetParameters();
    int i=0;
    for(parameterNameList_t::iterator it2 = params.begin(); it2 != params.end(); it2++)
    {
        if(proc->GetParameterDeclarationType() == PARAMS_BY_NAME)
        {
            tmp = "request[\"" + it2->first  + "\"]" + toCppConversion(it2->second);
        }
        else
        {
            stringstream tmp2;
            tmp2 << "request["<< i << "u]" << toCppConversion(it2->second);
            tmp = tmp2.str();
        }
        parameter << tmp;
        if(it2 != --params.end())
        {
            parameter << ", ";
        }
        i++;
    }
    return parameter.str();
}

