/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_beast.cpp
 * @date    30/06/2019
 * @author  Javier Gutierrez
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef BEASTHTTP_TESTING
#include "mockclientconnectionhandler.h"
#include <catch2/catch.hpp>
#include <jsonrpccpp/client/connectors/beasthttpclient.h>
#include <jsonrpccpp/server/connectors/beasthttpserver.h>

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif

#define TEST_MODULE "[connector_beasthttp]"

#define IP "127.0.0.1"
#define PORT 50024

namespace testbeasthttpserver {
struct F {
  BeastHttpServer server;
  BeastHttpClient client;
  MockClientConnectionHandler handler;

  F() : server(IP, PORT), client(IP, PORT) {
    server.SetHandler(&handler);
    REQUIRE(server.StartListening());
  }
  ~F() { server.StopListening(); }
};

bool check_exception1(JsonRpcException const &ex) {
  return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
}
} // namespace testbeasthttpserver
using namespace testbeasthttpserver;

TEST_CASE_METHOD(F, "test_beasthttp_success", TEST_MODULE) {
  handler.response = "exampleresponse";
  handler.timeout = 100;
  string result;
  string request = "examplerequest";
  string expectedResult = "exampleresponse";

  client.SendRPCMessage(request, result);

  CHECK(handler.request == request);
  CHECK(result == expectedResult);
}

TEST_CASE("test_beasthttp_server_multiplestart", TEST_MODULE) {
  BeastHttpServer server(IP, PORT);
  CHECK(server.StartListening() == true);
  CHECK(server.StartListening() == false);

  BeastHttpServer server2(IP, PORT);
  CHECK(server2.StartListening() == false);
  CHECK(server2.StopListening() == false);

  CHECK(server.StopListening() == true);
}

TEST_CASE("test_beasthttp_client_invalid", TEST_MODULE) {
  BeastHttpClient client("127.0.0.1", 40000); // If this test fails, check that
                                              // port 40000 is really unused. If
                                              // it is used, change this port
                                              // value to an unused port,
                                              // recompile tests and run tests
                                              // again.
  string result;
  CHECK_EXCEPTION_TYPE(client.SendRPCMessage("foobar", result),
                       JsonRpcException, check_exception1);
}

#endif
