/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    CPPClientStubGenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "cppclientstubgenerator.h"
#include "../helper/cpphelper.h"

#define TEMPLATE_CPPCLIENT_SIGCLASS "class <stubname> : public jsonrpc::Client"

#define TEMPLATE_CPPCLIENT_SIGCONSTRUCTOR "<stubname>(jsonrpc::AbstractClientConnector &conn) : jsonrpc::Client(conn) {}"

#define TEMPLATE_CPPCLIENT_SIGMETHOD "<returntype> <methodname>(<parameters>) throw (jsonrpc::JsonRpcException)"

#define TEMPLATE_NAMED_ASSIGNMENT "p[\"<paramname>\"] = <paramname>;"
#define TEMPLATE_POSITION_ASSIGNMENT "p.append(<paramname>);"

#define TEMPLATE_METHODCALL "Json::Value result = this->client->CallMethod(\"<name>\",p);"
#define TEMPLATE_NOTIFICATIONCALL "this->client->CallNotification(\"<name>\",p);"

#define TEMPLATE_RETURNCHECK "if (result<cast>)"
#define TEMPLATE_RETURN "return result<cast>;"

using namespace std;
using namespace jsonrpc;


CPPClientStubGenerator::CPPClientStubGenerator(const string &stubname, std::vector<Procedure> &procedures, CodeGenerator &cg) :
    StubGenerator(stubname, procedures, cg)
{
}

void CPPClientStubGenerator::generateStub()
{
    CPPHelper::prolog(cg, this->stubname);

    cg.writeLine("#include <jsonrpccpp/client.h>");
    cg.writeNewLine();

    cg.writeLine(replaceAll(TEMPLATE_CPPCLIENT_SIGCLASS, "<stubname>", this->stubname));
    cg.writeLine("{");
    cg.increaseIndentation();
    cg.writeLine("public:");
    cg.increaseIndentation();

    cg.writeLine(replaceAll(TEMPLATE_CPPCLIENT_SIGCONSTRUCTOR, "<stubname>", this->stubname));

    for (unsigned int i=0; i < procedures.size(); i++)
    {
        this->generateMethod(procedures[i]);
    }

    cg.decreaseIndentation();
    cg.decreaseIndentation();
    cg.writeLine("};");

    CPPHelper::epilog(cg, this->stubname);
}

void CPPClientStubGenerator::generateMethod(Procedure &proc)
{
    string procsignature = TEMPLATE_CPPCLIENT_SIGMETHOD;
    string returntype = CPPHelper::toCppType(proc.GetReturnType());
    if (proc.GetProcedureType() == RPC_NOTIFICATION)
        returntype = "void";

    replaceAll(procsignature, "<returntype>", returntype);
    replaceAll(procsignature, "<methodname>", proc.GetProcedureName());
    replaceAll(procsignature, "<parameters>", CPPHelper::generateParameterDeclarationList(proc));

    cg.writeLine(procsignature);
    cg.writeLine("{");
    cg.increaseIndentation();
    cg.writeLine("Json::Value p;");

    generateAssignments(proc);
    generateProcCall(proc);

    cg.decreaseIndentation();
    cg.writeLine("}");


}

void CPPClientStubGenerator::generateAssignments(Procedure &proc)
{
    string assignment;
    parameterNameList_t list = proc.GetParameters();
    if(list.size() > 0)
    {
        for (parameterNameList_t::iterator it = list.begin(); it != list.end(); it++)
        {

            if(proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
            {
                assignment = TEMPLATE_NAMED_ASSIGNMENT;
            }
            else
            {
                assignment = TEMPLATE_POSITION_ASSIGNMENT;
            }
            replaceAll(assignment, "<paramname>", it->first);
            cg.writeLine(assignment);
        }
    }
    else
    {
        cg.writeLine("p = Json::nullValue;");
    }

}

void CPPClientStubGenerator::generateProcCall(Procedure &proc)
{
    string call;
    if (proc.GetProcedureType() == RPC_METHOD)
    {
        call = TEMPLATE_METHODCALL;
        replaceAll(call, "<name>", proc.GetProcedureName());
        call = TEMPLATE_RETURNCHECK;
        replaceAll(call,"<cast>", CPPHelper::isCppConversion(proc.GetReturnType()));
        cg.writeLine(call);
        cg.increaseIndentation();
        call = TEMPLATE_RETURN;
        replaceAll(call,"<cast>", CPPHelper::toCppConversion(proc.GetReturnType()));
        cg.writeLine(call);
        cg.decreaseIndentation();
        cg.writeLine("else");
        cg.increaseIndentation();
        cg.writeLine("throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());");
        cg.decreaseIndentation();
    }
    else
    {
        call = TEMPLATE_NOTIFICATIONCALL;
        replaceAll(call, "<name>", proc.GetProcedureName());
        cg.writeLine(call);
    }

}
