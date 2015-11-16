/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_zeromq.cpp
 * @date    17/11/2015
 * @author  Vladimir Badaev <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef ZEROMQ_TESTING
#include <catch.hpp>
#include <jsonrpccpp/server/connectors/zeromqclient.h>
#include <jsonrpccpp/client/connectors/zeromqserver.h>
#include "mockclientconnectionhandler.h"

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#define TEST_MODULE "[connector_zeromq]"

#define IPC_SOCKET_PATH "/tmp/jsonrpccpp-socket"
#define IPC_ENDPOINT "ipc://" IPC_SOCKET_PATH
#define TCP_ENDPOINT "tcp://127.0.0.1:9999"

namespace testzeromqserver
{
    struct F {
            ZeroMQServer server;
            ZeroMQClient ipc_client;
            ZeroMQClient tcp_client;
            MockClientConnectionHandler handler;

            F() :
                server(vector<string>({ IPC_ENDPOINT, TCP_ENDPOINT }),
                ipc_client(IPC_ENDPOINT),
                tcp_client(TCP_ENDPOINT)
            {
                server.SetHandler(&handler);
                REQUIRE(server.StartListening());
            }
            ~F()
            {
                server.StopListening();
                unlink(IPC_SOCKET_PATH);
            }
    };

    bool check_exception1(JsonRpcException const&ex)
    {
        return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
    }
}
using namespace testzeromqserver;


TEST_CASE_METHOD(F, "test_zeromq_ipc_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    ipc_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}

TEST_CASE_METHOD(F, "test_zeromq_tcp_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    tcp_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}


#if 0
TEST_CASE("test_zeromq_server_multiplestart", TEST_MODULE)
{
    UnixDomainSocketServer server(SOCKET_PATH);
    CHECK(server.StartListening() == true);
    CHECK(server.StartListening() == false);

    UnixDomainSocketServer server2(SOCKET_PATH);
    CHECK(server2.StartListening() == false);
    CHECK(server2.StopListening() == false);

    CHECK(server.StopListening() == true);

    unlink(SOCKET_PATH);
}

TEST_CASE("test_unixdomainsocket_client_invalid", TEST_MODULE)
{
    UnixDomainSocketClient client("tmp/someinvalidpath");
    string result;
    CHECK_EXCEPTION_TYPE(client.SendRPCMessage("foobar", result), JsonRpcException, check_exception1);
}
#endif
#endif
