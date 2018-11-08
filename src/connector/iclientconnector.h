#pragma once

#include <string>

namespace jsonrpc {
  class IClientConnector {
   public:
    virtual ~IClientConnector() {}
    virtual std::string SendRPCMessage(const std::string& message) = 0;
  };
}
