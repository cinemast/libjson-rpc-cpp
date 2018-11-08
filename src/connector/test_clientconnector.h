#pragma once

#include <string>
#include "iclientconnector.h"

class TestClientConnector : public jsonrpc::IClientConnector {
 public:
  virtual std::string SendRPCMessage(const std::string& message) {
      this->request = message;
      return this->response;
  }

  std::string response;
  std::string request;
};