#include "curlclient.h"
#include <catch.hpp>
#include <iostream>
#include "../../exception.h"
#include "../microhttpd/microhttpserver.h"
#include "../test_clientconnectionhandler.h"

#define TEST_MODULE "[connector-curl]"

#define TEST_PORT 8484
#define CLIENT_URL "http://localhost:8484"

using namespace jsonrpc;
using namespace std;

struct F {
  TestClienctConnectionHandler handler;
  MicroHttpServer server;
  CurlClient client;

  F() : server(TEST_PORT, {handler}), client(CLIENT_URL, 1000, false) {
    handler.response = "mockresponse";
    REQUIRE(server.StartListening() == true);
  }
};

class FailingHttpServer : public MicroHttpServer {
 public:
  FailingHttpServer(int port, ConnectionHandlers handlers) : MicroHttpServer(port, handlers) {}

 protected:
  virtual bool SendResponse(const std::string &response, struct mhd_coninfo *client_connection) {
    client_connection->code = 500;
    return MicroHttpServer::SendResponse(response, client_connection);
  }
};

TEST_CASE_METHOD(F, "curl_post_success", TEST_MODULE) {
  string response = client.SendRPCMessage("this is a request");

  REQUIRE(response == handler.response);
  REQUIRE("this is a request" == handler.request);
}

TEST_CASE("curl_invalid_url", TEST_MODULE) {
  CurlClient client("http://someinvalidurl/asdf", 100);
  try {
    client.SendRPCMessage("this is a request");
    FAIL("No invalid url exception has been thrown");
  } catch (JsonRpcException e) {
    REQUIRE(e.GetCode() == -32003);
    string message = e.what();
    REQUIRE((message == "JsonRpcException -32003: libcurl error 6, Couldn't resolve host name" ||
             message == "JsonRpcException -32003: libcurl error 28, Timeout was reached") == true);
  }
}

TEST_CASE_METHOD(F, "curl_timeout", TEST_MODULE) {
  CurlClient client(CLIENT_URL, 10);
  handler.timeout = 12;
  try {
    client.SendRPCMessage("this is a request");
    FAIL("No timeout exception has been thrown");
  } catch (JsonRpcException e) {
    REQUIRE(e.GetCode() == -32003);
    string message = e.what();
    REQUIRE(message == "JsonRpcException -32003: libcurl error 28, Timeout was reached");
  }
}

TEST_CASE_METHOD(F, "curl_tls_invalid", TEST_MODULE) {
  if (!MHD_is_feature_supported(MHD_FEATURE_SSL)) {
    WARN("HTTPS won't be tested due to missing TLS support in libmicrohttpd");
  } else {
    CurlClient client("https://localhost:8484");
    server.StopListening();
    REQUIRE(server.EnableTLS("server.pem", "server.key") == true);
    REQUIRE(server.StartListening() == true);
    try {
      client.SendRPCMessage("request");
      FAIL("No tls exception has been thrown");
    } catch (JsonRpcException e) {
      REQUIRE(e.GetCode() == -32003);
      string message = e.what();
      REQUIRE(message ==
              "JsonRpcException -32003: libcurl error 60, Peer certificate cannot be authenticated with "
              "given CA certificates");
    }
  }
}

TEST_CASE_METHOD(F, "curl_tls_valid", TEST_MODULE) {
  if (!MHD_is_feature_supported(MHD_FEATURE_SSL)) {
    WARN("HTTPS won't be tested due to missing TLS support in libmicrohttpd");
  } else {
    CurlClient client("https://localhost:8484", 0, false);
    server.StopListening();
    REQUIRE(server.EnableTLS("server.pem", "server.key") == true);
    REQUIRE(server.StartListening() == true);
    try {
      REQUIRE(client.SendRPCMessage("request") == "mockresponse");
      REQUIRE(handler.request == "request");
    } catch (JsonRpcException e) {
      FAIL("No tls exception expected");
    }
  }
}

TEST_CASE("non_404_status_code", TEST_MODULE) {
  TestClienctConnectionHandler handler;
  handler.response = "testresponse";
  FailingHttpServer server(TEST_PORT, {handler});
  CurlClient client(CLIENT_URL);

  server.StartListening();

  try {
    string result = client.SendRPCMessage("testrequest");
    FAIL("Exception expected, due to 500 status code return");
  } catch (JsonRpcException e) {
    REQUIRE(e.GetCode() ==  -32003);
    string message = e.what();
    REQUIRE(message == "JsonRpcException -32003: Received HTTP status code 500: testresponse");
  }

  server.StopListening();
}