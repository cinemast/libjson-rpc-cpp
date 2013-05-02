/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubgenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <fstream>
#include <algorithm>
#include <jsonrpc/specificationparser.h>

#include "stubgenerator.h"

using namespace std;
using namespace jsonrpc;

StubGenerator::StubGenerator(const string &stubname, const string &filename) :
    stubname(stubname)
{
    this->procedures = SpecificationParser::GetProcedures(filename);
}

StubGenerator::~StubGenerator()
{
    delete this->procedures;
}

void StubGenerator::replaceAll(string &text, const string &fnd, const string &rep)
{
    size_t pos = text.find(fnd);
    while (pos != string::npos)
    {
        text.replace(pos, fnd.length(), rep);
        pos = text.find(fnd, pos + rep.length());
    }
}

void StubGenerator::generateStubToFile(const string &path)
{
    ofstream stream;
    string filename = this->stubname + ".h";
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    string completepath = path + "/" + filename;
    stream.open(completepath.c_str(), ios_base::out);
    stream << this->generateStub();
    stream.close();
}

string StubGenerator::toCppType(jsontype_t type, bool isConst, bool isReference)
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

string StubGenerator::toCppConversion(jsontype_t type)
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

string StubGenerator::toString(jsontype_t type)
{
    string result;
    switch(type)
    {
        case JSON_BOOLEAN:
            result = "jsonrpc::JSON_BOOLEAN";
            break;
        case JSON_INTEGER:
            result = "jsonrpc::JSON_INTEGER";
            break;
        case JSON_REAL:
            result = "jsonrpc::JSON_REAL";
            break;
        case JSON_STRING:
            result = "jsonrpc::JSON_STRING";
            break;
        case JSON_OBJECT:
            result = "jsonrpc::JSON_OBJECT";
            break;
        case JSON_ARRAY:
            result = "jsonrpc::JSON_ARRAY";
            break;
    }
    return result;
}

string StubGenerator::generateParameterDeclarationList(Procedure &proc)
{
    stringstream param_string;
    parameterlist_t list = proc.GetParameters();
    for (parameterlist_t::iterator it = list.begin(); it != list.end();)
    {
        param_string << toCppType(it->second, true, true) << " " << it->first;
        if (++it != list.end())
        {
            param_string << ", ";
        }
    }
    return param_string.str();
}
