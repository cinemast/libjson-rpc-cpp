/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    mockclientconnectionhandler.cpp
 * @date    10/29/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "mockclientconnectionhandler.h"
#include <unistd.h>

using namespace std;
using namespace jsonrpc;

MockClientConnectionHandler::MockClientConnectionHandler() :
    response(""),
    request(""),
    timeout(0)
{

}

void MockClientConnectionHandler::HandleRequest(const std::string &request, std::string &retValue)
{
    usleep(timeout*1000);
    this->request = request;
    retValue = response;
}
