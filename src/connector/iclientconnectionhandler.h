#pragma once

#include <string>

namespace jsonrpc {
  class IClientConnectionHandler {
   public:
    virtual ~IClientConnectionHandler() {}
    virtual bool HandleRequest(const std::string& request, std::string& response) = 0;
  };
}