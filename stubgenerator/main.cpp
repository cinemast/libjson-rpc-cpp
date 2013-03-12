/**
 * @file main.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief This application generates stubs for all specified methods
 * and notifications of a given file
 */

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <jsonrpc/rpc.h>
#include <jsonrpc/procedure.h>

#include "template.h"

using namespace std;
using namespace jsonrpc;

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

std::string generateMethod(Procedure& proc)
{
    string tmp = TEMPLATE_METHOD;

    //set methodname
    replace_all(tmp, "<methodname>", proc.GetProcedureName());

    //build parameterlist
    stringstream param_string;
    stringstream assignment_string;

    parameterlist_t list = proc.GetParameters();
    for (parameterlist_t::iterator it = list.begin(); it != list.end();)
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
        assignment_string << "p[\"" << it->first << "\"] = " << it->first
                << "; " << endl;

        if (++it != list.end())
        {
            param_string << ", ";
        }
    }

    replace_all(tmp, "<parameters>", param_string.str());
    replace_all(tmp, "<parameter_assign>", assignment_string.str());

    if (proc.GetProcedureType() == RPC_METHOD)
    {
        //TODO: add return type parsing
        replace_all(tmp, "<return_type>", "Json::Value");
        replace_all(tmp, "<return_statement>",
                "return this->client->CallMethod(\"" + proc.GetProcedureName() + "\",p);");
    }
    else
    {
        replace_all(tmp, "<return_type>", "void");
        replace_all(tmp, "<return_statement>", "this->client->CallMethod(\"" + proc.GetProcedureName() + "\",p);");
    }

    return tmp;
}

std::string generateStub(const string& stubname, const string& configfile)
{
    string tmp = TEMPLATE_STUB;
    replace_all(tmp, "<stubname>", stubname);

    string stub_upper = stubname;
    std::transform(stub_upper.begin(), stub_upper.end(), stub_upper.begin(),
            ::toupper);
    replace_all(tmp, "<STUBNAME>", stub_upper);

    //generate procedures
    stringstream procedure_string;
    procedurelist_t* procedures = SpecificationParser::GetProcedures(configfile);
    procedurelist_t::iterator it;
    for (it = procedures->begin(); it != procedures->end(); it++)
    {
        procedure_string << generateMethod(*(it->second)) << endl;
    }

    replace_all(tmp, "<methods>", procedure_string.str());
    return tmp;
}

int main(int argc, char** argv)
{
    try
    {
        if (argc < 3)
        {
            cerr << "call stub jsonrpc generator with: " << endl
                    << "\tjsonrpcstub <StubName> <Json-Specification>" << endl;
            return -1;
        }
        else
        {
            ofstream myfile;
            string filename = argv[1];
            filename = filename + ".h";
            myfile.open(filename.c_str());
            myfile << generateStub(argv[1], argv[2]);
            myfile.close();

            cout << "Stub generated into " << filename << endl;
        }
    }
    catch (Exception& e)
    {
        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}
