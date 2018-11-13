#include "test_mockclientconnectionhandler.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace jsonrpc;

MockClientConnectionHandler::MockClientConnectionHandler()
    : response(""), request(""), timeout(0) {}

std::string MockClientConnectionHandler::HandleRequest(const std::string &request) {
  std::this_thread::sleep_for(std::chrono::microseconds(timeout * 1000));
  this->request = request;
  return response;
}
