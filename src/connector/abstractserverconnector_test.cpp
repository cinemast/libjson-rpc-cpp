#include <catch.hpp>
#include <string>

#include "abstractserverconnector.h"
#include "test_clientconnectionhandler.h"

using namespace jsonrpc;
using namespace std;

#define TEST_MODULE "abstractserverconnector"

class TestServerConnector : public AbstractServerConnector {
 public:
  TestServerConnector(ConnectionHandlers handlers) : AbstractServerConnector(handlers) {}
  virtual bool StartListening() { return true; }
  virtual bool StopListening() { return true; }
};

TEST_CASE("test_single_conectionhandler_valid", TEST_MODULE) {
  TestClienctConnectionHandler handler("testresponse");
  TestServerConnector server({handler});

  REQUIRE(server.ProcessRequest("request1") == "testresponse");
  REQUIRE(handler.request == "request1");
}

TEST_CASE("test_multiple_connectionhandler_invalid_valid", TEST_MODULE) {
  TestClienctConnectionHandler handler("response1", false);
  TestClienctConnectionHandler handler2("response2");
  TestServerConnector server({handler, handler2});

  REQUIRE(server.ProcessRequest("request1") == "response2");
  REQUIRE(handler.request == "request1");
  REQUIRE(handler2.request == "request1");
}

TEST_CASE("test_multiple_connectionhandler_valid_invalid", TEST_MODULE) {
  TestClienctConnectionHandler handler("response1");
  TestClienctConnectionHandler handler2("response2", false);
  TestServerConnector server({handler, handler2});

  REQUIRE(server.ProcessRequest("request1") == "response1");
  REQUIRE(handler.request == "request1");
  REQUIRE(handler2.request == "");
}

TEST_CASE("test_multiple_connectionhandler_invalid_invalid", TEST_MODULE) {
  TestClienctConnectionHandler handler("response1", false);
  TestClienctConnectionHandler handler2("response2", false);
  TestServerConnector server({handler, handler2});

  REQUIRE(server.ProcessRequest("request1") == "response2");
  REQUIRE(handler.request == "request1");
  REQUIRE(handler2.request == "request1");
}