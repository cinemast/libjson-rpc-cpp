/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_unixdomainsocket.cpp
 * @date    6/8/2015
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef UNIXDOMAINSOCKET_TESTING
#include <catch.hpp>
#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>
#include "mockclientconnectionhandler.h"

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#define TEST_MODULE "[connector_unixdomainsocket]"

#define SOCKET_PATH "/tmp/jsonrpccpp-socket"

namespace testunixdomainsocketserver
{
    struct F {
            UnixDomainSocketServer server;
            UnixDomainSocketClient client;
            MockClientConnectionHandler handler;

            F() :
                server(SOCKET_PATH),
                client(SOCKET_PATH)
            {
                server.SetHandler(&handler);
                REQUIRE(server.StartListening());
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
}
using namespace testunixdomainsocketserver;

/*
TEST_CASE_METHOD(F, "test_unixdomainsocket_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}*/

TEST_CASE("test_unixdomainsocket_server_multiplestart", TEST_MODULE)
{
    UnixDomainSocketServer server(SOCKET_PATH);
    CHECK(server.StartListening() == true);

    UnixDomainSocketServer server2(SOCKET_PATH);
    CHECK(server2.StartListening() == false);

    CHECK(server.StopListening() == true);
}
/*
TEST_CASE_METHOD(F, "test_unixdomainsocket_client_timeout", TEST_MODULE)
{
    handler.timeout = 20;
    client.SetTimeout(10);
    string result;
    CHECK_EXCEPTION_TYPE(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
    handler.timeout = 0;
    client.SetTimeout(10000);
    handler.response = "asdf";
    client.SendRPCMessage("", result);
    CHECK(result == "asdf");
    server.StopListening();
    CHECK_EXCEPTION_TYPE(client.SendRPCMessage("Test", result), JsonRpcException, check_exception1);
}

TEST_CASE("test_unixdomainsocket_client_headers", TEST_MODULE)
{
    TestunixdomainsocketServer server(TEST_PORT);
    unixdomainsocketClient client(CLIENT_URL);

    REQUIRE(server.StartListening() == true);
    client.AddHeader("X-Auth", "1234");
    server.SetResponse("asdf");
    string result;
    client.SendRPCMessage("", result);
    CHECK(result == "asdf");
    CHECK(server.GetHeader("X-Auth") == "1234");

    client.RemoveHeader("X-Auth");
    client.SendRPCMessage("", result);
    CHECK(server.GetHeader("X-Auth") == "");

    server.StopListening();
}

TEST_CASE_METHOD(F, "test_unixdomainsocket_get", TEST_MODULE)
{
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, CLIENT_URL);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    CURLcode code = curl_easy_perform(curl);
    REQUIRE(code == CURLE_OK);

    long unixdomainsocket_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &unixdomainsocket_code);
    CHECK(unixdomainsocket_code == 405);

    curl_easy_cleanup(curl);
}

TEST_CASE_METHOD(F, "test_unixdomainsocket_get_options", TEST_MODULE)
{
    CURL* curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, CLIENT_URL);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    CURLcode code = curl_easy_perform(curl);
    REQUIRE(code == CURLE_OK);

    long unixdomainsocket_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &unixdomainsocket_code);
    CHECK(unixdomainsocket_code == 200);  // No error when server asked for OPTIONS.

    curl_easy_cleanup(curl);
}

TEST_CASE("test_unixdomainsocket_server_endpoints", TEST_MODULE)
{
    MockClientConnectionHandler handler1;
    MockClientConnectionHandler handler2;

    handler1.response = "response1";
    handler2.response = "response2";

    unixdomainsocketServer server(TEST_PORT);
    server.SetUrlHandler("/handler1", &handler1);
    server.SetUrlHandler("/handler2", &handler2);

    REQUIRE(server.StartListening() == true);
    unixdomainsocketClient client1("unixdomainsocket://localhost:8383/handler1");
    unixdomainsocketClient client2("unixdomainsocket://localhost:8383/handler2");
    unixdomainsocketClient client3("unixdomainsocket://localhost:8383/handler3");

    string response;
    client1.SendRPCMessage("test", response);
    CHECK(response == "response1");
    client2.SendRPCMessage("test", response);
    CHECK(response == "response2");

    CHECK_EXCEPTION_TYPE(client3.SendRPCMessage("test", response), JsonRpcException, check_exception2);

    client3.SetUrl("unixdomainsocket://localhost:8383/handler2");
    client3.SendRPCMessage("test", response);
    CHECK(response == "response2");

    server.StopListening();
}


TEST_CASE_METHOD(F, "test_unixdomainsocket_server_longpost", TEST_MODULE)
{
    int mb = 5;
    unsigned long size = mb * 1024*1024;
    char* str = (char*) malloc(size * sizeof(char));
    if (str == NULL)
    {
        FAIL("Could not allocate enough memory for test");
    }
    for (unsigned long i=0; i < size; i++)
    {
        str[i] = (char)('a'+(i%26));
    }
    str[size-1] = '\0';

    handler.response = str;
    string response;
    client.SetTimeout(-1);
    client.SendRPCMessage(str, response);

    CHECK(handler.request == str);
    CHECK(response == handler.response);
    CHECK(response.size() == size-1);

    free(str);
}

TEST_CASE("test_unixdomainsocket_server_ssl", TEST_MODULE)
{
    unixdomainsocketServer server(TEST_PORT, "/a/b/c", "/d/e/f");
    CHECK(server.StartListening() == false);

    unixdomainsocketServer server2(TEST_PORT, "server.pem", "server.key");
    CHECK(server2.StartListening() == true);
    server2.StopListening();
}*/

#endif
