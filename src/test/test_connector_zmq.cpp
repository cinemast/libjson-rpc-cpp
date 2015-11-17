/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_zmq.cpp
 * @date    17/11/2015
 * @author  Vladimir Badaev <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef ZMQCONNECTORS_TESTING
#include <unistd.h>

#include <catch.hpp>
#include <jsonrpccpp/server/connectors/zmqserver.h>
#include <jsonrpccpp/client/connectors/zmqclient.h>
#include "mockclientconnectionhandler.h"

#include "checkexception.h"


using namespace jsonrpc;
using namespace std;

#define TEST_MODULE "[connector_zmq]"

#define IPC_SOCKET_PATH "/tmp/jsonrpccpp-zmq-socket"
#define IPC_ENDPOINT "ipc://" IPC_SOCKET_PATH
#define TCP_ENDPOINT "tcp://127.0.0.1:9999"

namespace testzmqserver
{
    struct F {
            ZMQServer server;
            ZMQClient ipc_client;
            ZMQClient tcp_client;
            MockClientConnectionHandler handler;

            F(unsigned int th=0) :
                server(vector<string>({ IPC_ENDPOINT, TCP_ENDPOINT }), th),
                ipc_client(IPC_ENDPOINT),
                tcp_client(TCP_ENDPOINT)
            {
                server.SetHandler(&handler);
                REQUIRE(server.StartListening());
            }
            virtual ~F()
            {
                server.StopListening();
                unlink(IPC_SOCKET_PATH);
            }
    };

    struct FTH : public F {
        FTH() : F(2) {};
        virtual ~FTH() {};
    };


    bool check_exception1(JsonRpcException const&ex)
    {
        return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
    }
};
using namespace testzmqserver;


TEST_CASE_METHOD(F, "test_zmq_ipc_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    ipc_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}

TEST_CASE_METHOD(F, "test_zmq_tcp_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    tcp_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}

TEST_CASE_METHOD(FTH, "test_zmq_threaded_ipc_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    ipc_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}

TEST_CASE_METHOD(FTH, "test_zmq_threaded_tcp_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    string result;
    tcp_client.SendRPCMessage("examplerequest", result);

    CHECK(handler.request == "examplerequest");
    CHECK(result == "exampleresponse");
}

#if 0
TEST_CASE("test_zmq_server_multiplestart", TEST_MODULE)
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
