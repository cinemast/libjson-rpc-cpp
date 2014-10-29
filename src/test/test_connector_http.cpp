/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_common.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <boost/test/unit_test.hpp>

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "mockclientconnectionhandler.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 8383
#define CLIENT_URL "http://localhost:8383"

BOOST_AUTO_TEST_SUITE(connector_http)

bool check_exception1(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR ;
}

BOOST_AUTO_TEST_CASE(test_http_success)
{
    HttpServer server(TEST_PORT);
    HttpClient client(CLIENT_URL);
    MockClientConnectionHandler handler;
    handler.response = "exampleresponse";
    server.SetHandler(&handler);
    server.StartListening();

    string result;
    client.SendRPCMessage("examplerequest", result);

    BOOST_CHECK_EQUAL(handler.request, "examplerequest");
    BOOST_CHECK_EQUAL(result, "exampleresponse");

    server.StopListening();
}

BOOST_AUTO_TEST_CASE(test_http_server_multiplestart)
{
    HttpServer server(TEST_PORT);
    BOOST_CHECK_EQUAL(server.StartListening(), true);

    HttpServer server2(TEST_PORT);
    BOOST_CHECK_EQUAL(server2.StartListening(), false);

    BOOST_CHECK_EQUAL(server.StopListening(), true);
}

BOOST_AUTO_TEST_CASE(test_http_client_timeout)
{
    MockClientConnectionHandler handler;
    HttpServer server(TEST_PORT);
    server.SetHandler(&handler);
    server.StartListening();
    handler.timeout = 20;
    HttpClient client(CLIENT_URL);
    client.SetTimeout(10);
    string result;
    BOOST_CHECK_EXCEPTION(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
    handler.timeout = 0;
    handler.response = "asdf";
    client.SendRPCMessage("", result);
    BOOST_CHECK_EQUAL(result, "asdf");
    server.StopListening();
    BOOST_CHECK_EXCEPTION(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
}

BOOST_AUTO_TEST_SUITE_END()
