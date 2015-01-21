/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_http.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef HTTP_TESTING
#include <boost/test/unit_test.hpp>

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <curl/curl.h>

#include "mockclientconnectionhandler.h"
#include "testhttpserver.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 8383
#define CLIENT_URL "http://localhost:8383"

BOOST_AUTO_TEST_SUITE(connector_http)

struct F {
    HttpServer server;
    HttpClient client;
    MockClientConnectionHandler handler;

    F() :
        server(TEST_PORT),
        client(CLIENT_URL)
    {
        server.SetHandler(&handler);
        server.StartListening();
    }
    ~F()
    {
        server.StopListening();
    }
};

bool check_exception1(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
}

bool check_exception2(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_RPC_INTERNAL_ERROR;
}

BOOST_FIXTURE_TEST_CASE(test_http_success, F)
{
    handler.response = "exampleresponse";
    string result;
    client.SendRPCMessage("examplerequest", result);

    BOOST_CHECK_EQUAL(handler.request, "examplerequest");
    BOOST_CHECK_EQUAL(result, "exampleresponse");
}

#ifndef WIN32
BOOST_AUTO_TEST_CASE(test_http_server_multiplestart)
{
    HttpServer server(TEST_PORT);
    BOOST_CHECK_EQUAL(server.StartListening(), true);

    HttpServer server2(TEST_PORT);
    BOOST_CHECK_EQUAL(server2.StartListening(), false);

    BOOST_CHECK_EQUAL(server.StopListening(), true);
}
#endif

BOOST_FIXTURE_TEST_CASE(test_http_client_timeout, F)
{
    handler.timeout = 20;
    client.SetTimeout(10);
    string result;
    BOOST_CHECK_EXCEPTION(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
    handler.timeout = 0;
    client.SetTimeout(10000);
    handler.response = "asdf";
    client.SendRPCMessage("", result);
    BOOST_CHECK_EQUAL(result, "asdf");
    server.StopListening();
    BOOST_CHECK_EXCEPTION(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
}

BOOST_AUTO_TEST_CASE(test_http_client_headers)
{
    TestHttpServer server(TEST_PORT);
    HttpClient client(CLIENT_URL);
    BOOST_REQUIRE_EQUAL(server.StartListening(),true);
    client.AddHeader("X-Auth", "1234");
    server.SetResponse("asdf");
    string result;
    client.SendRPCMessage("", result);
    BOOST_CHECK_EQUAL(result, "asdf");
    BOOST_CHECK_EQUAL(server.GetHeader("X-Auth"), "1234");

    client.RemoveHeader("X-Auth");
    client.SendRPCMessage("", result);
    BOOST_CHECK_EQUAL(server.GetHeader("X-Auth"), "");

    server.StopListening();
}

BOOST_FIXTURE_TEST_CASE(test_http_get,F)
{
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, CLIENT_URL);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    CURLcode code = curl_easy_perform(curl);
    BOOST_REQUIRE_EQUAL(code, CURLE_OK);

    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    BOOST_CHECK_EQUAL(http_code, 405);

    curl_easy_cleanup(curl);
}

BOOST_FIXTURE_TEST_CASE(test_http_get_options, F)
{
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, CLIENT_URL);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    CURLcode code = curl_easy_perform(curl);
    BOOST_REQUIRE_EQUAL(code, CURLE_OK);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    BOOST_CHECK_EQUAL(http_code, 200);  // No error when server asked for OPTIONS.

    curl_easy_cleanup(curl);
}

BOOST_AUTO_TEST_CASE(test_http_server_endpoints)
{
    MockClientConnectionHandler handler1;
    MockClientConnectionHandler handler2;

    handler1.response = "response1";
    handler2.response = "response2";

    HttpServer server(TEST_PORT);
    server.SetUrlHandler("/handler1", &handler1);
    server.SetUrlHandler("/handler2", &handler2);

    BOOST_REQUIRE_EQUAL(server.StartListening(), true);
    HttpClient client1("http://localhost:8383/handler1");
    HttpClient client2("http://localhost:8383/handler2");
    HttpClient client3("http://localhost:8383/handler3");

    string response;
    client1.SendRPCMessage("test", response);
    BOOST_CHECK_EQUAL(response, "response1");
    client2.SendRPCMessage("test", response);
    BOOST_CHECK_EQUAL(response, "response2");

    BOOST_CHECK_EXCEPTION(client3.SendRPCMessage("test", response), JsonRpcException, check_exception2);

    client3.SetUrl("http://localhost:8383/handler2");
    client3.SendRPCMessage("test", response);
    BOOST_CHECK_EQUAL(response, "response2");

    server.StopListening();
}

BOOST_FIXTURE_TEST_CASE(test_http_server_longpost, F)
{
    int mb = 5;
    unsigned long size = mb * 1024*1024;
    char* str = (char*) malloc(size * sizeof(char));
    BOOST_REQUIRE(str != NULL);
    for (unsigned long i=0; i < size; i++)
    {
        str[i] = (char)('a'+(i%26));
    }
    str[size-1] = '\0';

    handler.response = str;
    string response;
    client.SetTimeout(-1);
    client.SendRPCMessage(str, response);

    BOOST_CHECK_EQUAL(handler.request, str);
    BOOST_CHECK_EQUAL(response, handler.response);
    BOOST_CHECK_EQUAL(response.size(), size-1);

    free(str);
}

BOOST_AUTO_TEST_CASE(test_http_server_ssl)
{
    HttpServer server(TEST_PORT, "/a/b/c", "/d/e/f");
    BOOST_CHECK_EQUAL(server.StartListening(), false);

    HttpServer server2(TEST_PORT, "server.pem", "server.key");
    BOOST_CHECK_EQUAL(server2.StartListening(), true);
    server2.StopListening();
}

BOOST_AUTO_TEST_SUITE_END()


#endif
