#pragma once

#include "../jsonparser.h"
#include "../connector/iclientconnectionhandler.h"

namespace jsonrpc {

class MockServerConnector {
public:
  MockServerConnector(IClientConnectionHandler& handler);

  bool SetRequest(const std::string &request);
  Json::Value GetJsonRequest();

  std::string GetResponse();
  Json::Value GetJsonResponse();

private:
  IClientConnectionHandler& handler;
  std::string request;
  std::string response;
};

} // namespace jsonrpc