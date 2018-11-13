/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    mockserverconnector.cpp
 * @date    10/10/2014
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "test_mockserverconnector.h"

using namespace jsonrpc;
using namespace std;

MockServerConnector::MockServerConnector(ConnectionHandlers handlers) : AbstractServerConnector(handlers) {}

bool MockServerConnector::StartListening() { return true; }

bool MockServerConnector::StopListening() { return true; }

bool MockServerConnector::SetRequest(const string &request) {
  this->request = request;
  this->response = this->ProcessRequest(request);
  return true;
}

Json::Value MockServerConnector::GetJsonRequest() {
  Json::Reader reader;
  Json::Value result;
  if (reader.parse(request, result))
    return result;
  else
    return Json::nullValue;
}

string MockServerConnector::GetResponse() { return this->response; }

Json::Value MockServerConnector::GetJsonResponse() {
  Json::Reader reader;
  Json::Value result;
  if (reader.parse(response, result))
    return result;
  else
    return Json::nullValue;
}
