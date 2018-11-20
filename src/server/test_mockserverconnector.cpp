#include "test_mockserverconnector.h"

using namespace jsonrpc;
using namespace std;

MockServerConnector::MockServerConnector(IClientConnectionHandler& handler) : handler(handler) {}

bool MockServerConnector::SetRequest(const string &request) {
  this->request = request;
  this->response = "";
  this->handler.HandleRequest(request, response);
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
