/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubgenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <fstream>
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
