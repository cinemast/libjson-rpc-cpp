#pragma once

#include "../jsonparser.h"
#include "../connector/abstractserverconnector.h"

namespace jsonrpc {

class MockServerConnector : public AbstractServerConnector {
public:
  MockServerConnector(ConnectionHandlers handlers);

  virtual bool StartListening();
  virtual bool StopListening();

  bool SetRequest(const std::string &request);
  Json::Value GetJsonRequest();

  std::string GetResponse();
  Json::Value GetJsonResponse();

private:
  std::string request;
  std::string response;
};

} // namespace jsonrpc