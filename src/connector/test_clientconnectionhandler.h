#pragma once

#include <chrono>
#include <thread>
#include "iclientconnectionhandler.h"

class TestClienctConnectionHandler : public jsonrpc::IClientConnectionHandler {
 public:
  TestClienctConnectionHandler() : response(""), request(""), success(true), timeout(0) {}
  TestClienctConnectionHandler(const std::string& response, bool success = true)
      : response(response), request(""), success(success), timeout(0) {}

  virtual void HandleRequest(const std::string& request, std::string& response) {
    std::this_thread::sleep_for(std::chrono::microseconds(timeout * 1000));
    this->request = request;
    response = this->response;
  }

  std::string response;
  std::string request;
  bool success;
  long timeout;
};