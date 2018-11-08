#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include "iclientconnectionhandler.h"

namespace jsonrpc {

  typedef std::initializer_list<std::reference_wrapper<IClientConnectionHandler>> ConnectionHandlers;

  class AbstractServerConnector {
   public:
    AbstractServerConnector(ConnectionHandlers handlers) {
      for (auto handler : handlers) {
        connectionHandlers.push_back(handler);
      }
    }
    virtual ~AbstractServerConnector() {}

    virtual bool StartListening() = 0;
    virtual bool StopListening() = 0;

    virtual std::string ProcessRequest(const std::string& request) {
      std::string result = "";
      for (auto handler : connectionHandlers) {
        if (handler.get().HandleRequest(request, result)) {
          return result;
        }
      }
      return result;
    }

   private:
    std::vector<std::reference_wrapper<IClientConnectionHandler>> connectionHandlers;
  };
}