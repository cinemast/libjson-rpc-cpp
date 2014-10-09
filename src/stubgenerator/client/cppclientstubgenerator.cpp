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

#include <sstream>
#include <algorithm>

#define TEMPLATE_CLIENT_METHOD "\
        <return_type> <methodname>(<parameters>) throw (jsonrpc::JsonRpcException)\n\
        {\n\
            Json::Value p;\n\
            <parameter_assign>\n\
            <return_statement>\n\
        }\n\
"

#define TEMPLATE_CPPCLIENT_SIGCLASS "class <stubname> : public jsonrpc::Client"

#define TEMPLATE_CPPCLIENT_SIGCONSTRUCTOR "<stubname>(jsonrpc::AbstractClientConnector &conn) : jsonrpc::Client(conn) {}"

#define TEMPLATE_CPPCLIENT_SIGMETHOD "<returntype> <methodname>(<parameters>) throw (jsonrpc::JsonRpcException)"

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

    cg.decreaseIndentation();
    cg.writeLine("}");


}
