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


TEST_CASE_METHOD(F, "test_unixdomainsocket_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}


TEST_CASE("test_unixdomainsocket_server_multiplestart", TEST_MODULE)
{
    UnixDomainSocketServer server(SOCKET_PATH);
    CHECK(server.StartListening() == true);

    UnixDomainSocketServer server2(SOCKET_PATH);
    CHECK(server2.StartListening() == false);
    CHECK(server2.StopListening() == false);

    CHECK(server.StopListening() == true);

}

#endif
