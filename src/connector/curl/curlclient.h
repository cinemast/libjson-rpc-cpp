#pragma once

#include <map>
#include "../iclientconnector.h"

namespace jsonrpc {
  class CurlClient : public IClientConnector {
   public:
    /**
     * @param timeout - timeout in milliseconds, if zero, no timeout is set
     */
    CurlClient(const std::string &url, long timeout = 0, bool verifyTLS = true);
    virtual std::string SendRPCMessage(const std::string &message);

    void AddHeader(const std::string &attr, const std::string &val);

   private:
    std::map<std::string, std::string> headers;
    std::string url;
    long timeout;
    bool verifyTLS;
  };
}
