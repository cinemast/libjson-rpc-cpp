/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    mockserverconnector.cpp
 * @date    10/10/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "mockserverconnector.h"

using namespace jsonrpc;
using namespace std;


MockServerConnector::MockServerConnector()
{
}

bool MockServerConnector::StartListening()
{
    return true;
}

bool MockServerConnector::StopListening()
{
    return true;
}

bool MockServerConnector::SendResponse(const string &response, void *addInfo)
{
    (void)addInfo;
    this->response = response;
    return true;
}

bool MockServerConnector::SetRequest(const string &request)
{
    this->request = request;
    return this->OnRequest(this->request, NULL);
}

Json::Value MockServerConnector::GetJsonRequest()
{
    Json::Reader reader;
    Json::Value result;
    if (reader.parse(request, result))
        return result;
    else
        return Json::nullValue;
}

string MockServerConnector::GetResponse()
{
    return this->response;
}

Json::Value MockServerConnector::GetJsonResponse()
{
    Json::Reader reader;
    Json::Value result;
    if (reader.parse(response, result))
        return result;
    else
        return Json::nullValue;
}
