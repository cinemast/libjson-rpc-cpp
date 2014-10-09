/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    CPPServerStubGenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "cppserverstubgenerator.h"
#include "../helper/cpphelper.h"

#include <jsonrpccpp/common/specificationwriter.h>
#include <sstream>
#include <algorithm>

#define TEMPLATE_CPPSERVER_METHODBINDING       "this->bindAndAddMethod(new jsonrpc::Procedure(\"<procedurename>\", <paramtype>, <returntype>, <parameterlist> NULL), &<stubname>::<procedurename>I);"
#define TEMPLATE_CPPSERVER_NOTIFICATIONBINDING "this->bindAndAddNotification(new jsonrpc::Procedure(\"<procedurename>\", <paramtype>, <parameterlist> NULL), &<stubname>::<procedurename>I);"

#define TEMPLATE_CPPSERVER_SIGCLASS "class <stubname> : public jsonrpc::AbstractServer<<stubname>>"
#define TEMPLATE_CPPSERVER_SIGCONSTRUCTOR "<stubname>(jsonrpc::AbstractServerConnector &conn) : jsonrpc::AbstractServer<<stubname>>(conn)"

#define TEMPLATE_CPPSERVER_SIGMETHOD "inline virtual void <procedurename>I(const Json::Value &request, Json::Value &response)"
#define TEMPLATE_CPPSERVER_SIGNOTIFICATION "inline virtual void <procedurename>I(const Json::Value &request)"

#define TEMPLATE_SERVER_ABSTRACTDEFINITION "virtual <returntype> <procedurename>(<parameterlist>) = 0;"

using namespace std;
using namespace jsonrpc;


CPPServerStubGenerator::CPPServerStubGenerator(const std::string &stubname, vector<Procedure> &procedures, CodeGenerator &cg) :
    StubGenerator(stubname, procedures, cg)
{
}

void CPPServerStubGenerator::generateStub()
{
    CPPHelper::prolog(cg, this->stubname);

    cg.writeLine("#include <jsonrpccpp/server.h>");
    cg.writeNewLine();

    cg.writeLine(replaceAll(TEMPLATE_CPPSERVER_SIGCLASS, "<stubname>", this->stubname));
    cg.writeLine("{");
    cg.increaseIndentation();
    cg.writeLine("public:");
    cg.increaseIndentation();

    cg.writeLine(replaceAll(TEMPLATE_CPPSERVER_SIGCONSTRUCTOR, "<stubname>", this->stubname));
    cg.writeLine("{");
    this->generateBindings();
    cg.writeLine("}");

    cg.writeNewLine();

    this->generateProcedureDefinitions();
}


void CPPServerStubGenerator::generateBindings()
{
    string tmp;
    this->cg.increaseIndentation();
    for(vector<Procedure>::iterator it = this->procedures.begin(); it != this->procedures.end(); it++)
    {
        Procedure &proc = *it;
        if(proc.GetProcedureType() == RPC_METHOD)
        {
            tmp = TEMPLATE_CPPSERVER_METHODBINDING;
        }
        else
        {
            tmp = TEMPLATE_CPPSERVER_NOTIFICATIONBINDING;
        }
        replaceAll(tmp, "<procedurename>", CPPHelper::normalizeString(proc.GetProcedureName()));
        replaceAll(tmp, "<returntype>", CPPHelper::toString(proc.GetReturnType()));
        replaceAll(tmp, "<parameterlist>", generateBindingParameterlist(proc));
        replaceAll(tmp, "<stubname>", this->stubname);

        if(proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
        {
            replaceAll(tmp, "<paramtype>", "jsonrpc::PARAMS_BY_NAME");
        }
        else
        {
            replaceAll(tmp, "<paramtype>", "jsonrpc::PARAMS_BY_POSITION");
        }

        this->cg.writeLine(tmp);
    }
    this->cg.decreaseIndentation();
}

string CPPServerStubGenerator::generateProcedureDefinitions()
{
    stringstream result;
    string tmp;
    for(vector<Procedure>::iterator it = this->procedures.begin(); it != this->procedures.end(); it++)
    {
        Procedure &proc = *it;
        if(proc.GetProcedureType() == RPC_METHOD)
        {
            this->cg.writeLine(replaceAll(TEMPLATE_CPPSERVER_SIGMETHOD, "<procedurename>", proc.GetProcedureName()));
        }
        else
        {
            this->cg.writeLine(replaceAll(TEMPLATE_CPPSERVER_SIGNOTIFICATION, "<procedurename>", proc.GetProcedureName()));
        }
        this->cg.writeLine("{");
        this->cg.increaseIndentation();
        this->generateParameterMapping(proc);
        this->cg.decreaseIndentation();
        this->cg.writeLine("}");
        result << tmp << endl;
    }
    return result.str();
}

string CPPServerStubGenerator::generateAbstractDefinitions()
{
    stringstream result;
    string tmp;

    for(vector<Procedure>::iterator it = this->procedures.begin(); it != this->procedures.end(); it++)
    {
        Procedure& proc = *it;
        tmp = TEMPLATE_SERVER_ABSTRACTDEFINITION;
        string returntype ="void";
        if(proc.GetProcedureType() == RPC_METHOD)
        {
            returntype = CPPHelper::toCppType(proc.GetReturnType());
        }
        replaceAll(tmp, "<returntype>", returntype);
        replaceAll(tmp, "<procedurename>", proc.GetProcedureName());
        replaceAll(tmp, "<parameterlist>", CPPHelper::generateParameterDeclarationList(proc));
        result << tmp;
    }
    return result.str();
}

string CPPServerStubGenerator::generateBindingParameterlist(Procedure &proc)
{
    stringstream parameter;
    const parameterNameList_t& list = proc.GetParameters();

    for(parameterNameList_t::const_iterator it2 = list.begin(); it2 != list.end(); it2++)
    {
        parameter << "\"" << it2->first << "\"," << CPPHelper::toString(it2->second) << ",";
    }
    return parameter.str();
}

void CPPServerStubGenerator::generateParameterMapping(Procedure &proc)
{
    string tmp;
    const parameterNameList_t& params = proc.GetParameters();
    int i=0;
    for(parameterNameList_t::const_iterator it2 = params.begin(); it2 != params.end(); it2++)
    {
        if(proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
        {
            tmp = "request[\"" + it2->first  + "\"]" + CPPHelper::toCppConversion(it2->second);
        }
        else
        {
            stringstream tmp2;
            tmp2 << "request["<< i << "u]" << CPPHelper::toCppConversion(it2->second);
            tmp = tmp2.str();
        }
        this->cg.writeLine(tmp);
        if(it2 != --params.end())
        {
            this->cg.write(", ");
        }
        i++;
    }
}

