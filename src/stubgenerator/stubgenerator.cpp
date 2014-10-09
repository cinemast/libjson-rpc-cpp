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
#include <jsonrpccpp/common/specificationparser.h>

#include "stubgenerator.h"

using namespace std;
using namespace jsonrpc;

StubGenerator::StubGenerator    (const string &stubname, std::vector<Procedure> &procedures, CodeGenerator &cg) :
    stubname                    (stubname),
    procedures                  (procedures),
    cg                          (cg)
{
}

StubGenerator::~StubGenerator   ()
{
}

string    StubGenerator::replaceAll                       (const string &text, const string &fnd, const string &rep)
{
    string result = text;
    replaceAll(result, fnd, rep);
    return result;
}

void StubGenerator::replaceAll(string &result, const string &find, const string &replace)
{
    size_t pos = result.find(find);
    while (pos != string::npos)
    {
        result.replace(pos, find.length(), replace);
        pos = result.find(find, pos + replace.length());
    }
}

string  StubGenerator::getStubName                      ()
{
    return this->stubname;
}
