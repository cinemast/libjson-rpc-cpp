/**
 * @file main.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <jsonrpc/rpc.h>
#include <jsonrpc/procedure.h>

using namespace std;
using namespace jsonrpc;

#define STUB_PATH  "jsonrpcstub.tpl"
#define METHOD_PATH  "method.tpl"

std::string readfile(const std::string& filename)
{
    ifstream config(filename.c_str());
    string value;
    if (config)
    {
        value.assign((std::istreambuf_iterator<char>(config)),
                (std::istreambuf_iterator<char>()));
    }

    return value;
}

void replace_all(string& text, const string& fnd, const string& rep)
{
    size_t pos = text.find(fnd);
    while (pos != string::npos)
    {
        text.replace(pos, fnd.length(), rep);
        pos = text.find(fnd, pos + rep.length());
    }
}

std::string generateStub(const string& stubname)
{
    string tmp = readfile(STUB_PATH);
    replace_all(tmp, "<stubname>", stubname);

    string stub_upper = stubname;
    std::transform(stub_upper.begin(), stub_upper.end(), stub_upper.begin(),
            ::toupper);
    replace_all(tmp, "<STUBNAME>", stub_upper);
    cout << tmp << endl;
}

std::string generateMethod(Procedure& proc)
{
    string tmp = readfile(METHOD_PATH);

    //set methodname
    replace_all(tmp, "<methodname>", proc.GetProcedureName());

    //build parameterlist
    stringstream param_string, assignment_string;

    parameterlist_t list = proc.GetParameters();

    for (parameterlist_t::iterator it = list.begin(); it != list.end(); it++)
    {
        string type;
        switch (it->second)
        {
            case JSON_BOOLEAN:
                type = "bool";
                break;

            case JSON_INTEGER:
                type = "int";
                break;
            case JSON_REAL:
                type = "double";
                break;
            case JSON_STRING:
                type = "const std::string&";
                break;
            default:
                type = "const Json::Value&";
                break;
        }
        param_string << type << " " << it->first;
        assignment_string << "p[\"" << it->first << "\"] = it->first;" << endl;
        if (it != list.end())
        {
            param_string << ", ";
        }
    }

    replace_all(tmp, "<parameters>", param_string.str());
    replace_all(tmp, "<parameter_assign>", assignment_string.str());

    return tmp;
}

int main(int argc, char** argv)
{
    vector<Procedure*> methods = Server::ParseProcedures("res/procedures.json");

    cout << generateMethod(*methods[0]) << endl;
    //generateStub("FooBar");

    return 0;
}
