/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    mockclientconnectionhandler.cpp
 * @date    10/29/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "mockclientconnectionhandler.h"
#if _MSC_VER
#include <chrono>
#include <thread>
#else
#include <unistd.h>
#endif

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
#if _MSC_VER
    std::this_thread::sleep_for(std::chrono::microseconds(timeout*1000));
#else
    usleep(timeout*1000);
#endif
    this->request = request;
    retValue = response;
}
