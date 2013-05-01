/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    serverstubgenerator.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "serverstubgenerator.h"

using namespace std;
using namespace jsonrpc;


ServerStubGenerator::ServerStubGenerator(const std::string &stubname, const std::string &filename) :
    StubGenerator(stubname + "Server", filename)
{
}

