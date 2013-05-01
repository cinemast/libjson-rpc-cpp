/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    clientstubgenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "clientstubgenerator.h"
#include "clienttemplate.h"

#include <sstream>

using namespace std;
using namespace jsonrpc;

ClientStubGenerator::ClientStubGenerator(const string &stubname, const string &filename) :
    StubGenerator(stubname+"Client", filename)
{
}

std::string ClientStubGenerator::generateStub()
{
    string tmp = TEMPLATE_STUB;
    replaceAll(tmp, "<stubname>", stubname);

    string stub_upper = stubname;
    std::transform(stub_upper.begin(), stub_upper.end(), stub_upper.begin(),
            ::toupper);
    replaceAll(tmp, "<STUBNAME>", stub_upper);

    //generate procedures
    stringstream procedure_string;
    procedurelist_t::iterator it;
    for (it = procedures->begin(); it != procedures->end(); it++)
    {
        procedure_string << generateMethod(*(it->second)) << endl;
    }

    replaceAll(tmp, "<methods>", procedure_string.str());
    return tmp;
}

string ClientStubGenerator::generateMethod(Procedure &proc)
{
    string tmp = TEMPLATE_METHOD;

    //set methodname
    replaceAll(tmp, "<methodname>", proc.GetProcedureName());

    //build parameterlist
    stringstream param_string;
    stringstream assignment_string;

    parameterlist_t list = proc.GetParameters();
    for (parameterlist_t::iterator it = list.begin(); it != list.end();)
    {

        param_string << toCppType(it->second, true, true) << " " << it->first;
        assignment_string << "p[\"" << it->first << "\"] = " << it->first
                << "; " << endl;

        if (++it != list.end())
        {
            param_string << ", ";
        }
    }

    replaceAll(tmp, "<parameters>", param_string.str());
    replaceAll(tmp, "<parameter_assign>", assignment_string.str());

    if (proc.GetProcedureType() == RPC_METHOD)
    {
        //TODO: add return type parsing
        replaceAll(tmp, "<return_type>", toCppType(proc.GetReturnType()));
        replaceAll(tmp, "<return_statement>",
                   "return this->client->CallMethod(\"" + proc.GetProcedureName() + "\",p)"+ toCppConversion(proc.GetReturnType()) +";");
    }
    else
    {
        replaceAll(tmp, "<return_type>", "void");
        replaceAll(tmp, "<return_statement>", "this->client->CallNotification(\"" + proc.GetProcedureName() + "\",p);");
    }

    return tmp;
}

string ClientStubGenerator::toCppType(jsontype_t type, bool isConst, bool isReference)
{
    string result;
    switch(type)
    {
        case JSON_BOOLEAN:
            result = "bool";
            break;
        case JSON_INTEGER:
            result = "int";
            break;
        case JSON_REAL:
            result = "double";
            break;
        case JSON_STRING:
            result = "std::string";
            break;
        default:
            result = "Json::Value";
            break;
    }
    if(isConst)
    {
        result = "const " + result;
    }
    if(isReference)
    {
        result = result + "&";
    }
    return result;
}

string ClientStubGenerator::toCppConversion(jsontype_t type)
{
    string result;
    switch(type)
    {
        case JSON_BOOLEAN:
            result = ".asBool()";
            break;
        case JSON_INTEGER:
            result = ".asInt()";
            break;
        case JSON_REAL:
            result = ".asDouble()";
            break;
        case JSON_STRING:
            result = ".asString()";
            break;
        default:
            result = "";
            break;
    }
    return result;
}
