/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_unixdomainsocket.cpp
 * @date    26/10/2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef FILEDESCRIPTOR_TESTING
#include <catch.hpp>
#include <unistd.h>
#include <jsonrpccpp/server/connectors/filedescriptorserver.h>
#include <jsonrpccpp/client/connectors/filedescriptorclient.h>
#include "mockclientconnectionhandler.h"

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#ifndef DELIMITER_CHAR
    #define DELIMITER_CHAR char(0x0A)
#endif

#define TEST_MODULE "[connector_filedescriptor]"

namespace testfiledescriptorserver
{
    struct F {
            FileDescriptorServer *server;
            FileDescriptorClient *client;
            MockClientConnectionHandler handler;

            F()
            {
                int pipefd[2];
                pipe(pipefd);
                c2sfd = pipefd[0];
                s2cfd = pipefd[1];
                server = new FileDescriptorServer(c2sfd, s2cfd);
                client = new FileDescriptorServer(s2cfd, c2sfd);
                server.SetHandler(&handler);
                REQUIRE(server.StartListening());
            }
            ~F()
            {
                server.StopListening();
                delete server;
                delete client;
                close(c2sfd);
                close(s2cfd);
            }

            int c2sfd; // Client to server fd
            int s2cfd; // Server to client fd
    };

    bool check_exception1(JsonRpcException const&ex)
    {
        return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
    }
}
using namespace testfiledescriptorserver;

TEST_CASE_METHOD(F, "test_filedescriptor_success", TEST_MODULE)
{
    handler.response = "exampleresponse";
    handler.timeout = 100;
    string result;
    string request = "examplerequest";
    request.push_back(DELIMITER_CHAR);
    string expectedResult = "exampleresponse";
    expectedResult.push_back(DELIMITER_CHAR);

    client.SendRPCMessage(request, result);

    CHECK(handler.request == request);
    CHECK(result == expectedResult);
}


TEST_CASE("test_filedescriptor_server_multiplestart", TEST_MODULE)
{
    int inputfd = mkstemp('/tmp/test_filedescriptor_input.XXXXXX');
    int outputfd = mkstemp('/tmp/test_filedescriptor_output.XXXXXX');
    FileDescriptorServer server(inputfd, outputfd);
    CHECK(server.StartListening() == true);
    CHECK(server.StartListening() == false);

    FileDescriptorServer server2(inputfd, outputfd);
    CHECK(server2.StartListening() == false);
    CHECK(server2.StopListening() == false);

    CHECK(server.StopListening() == true);

    close(inputfd);
    close(outputfd);
}

TEST_CASE("test_filedescriptor_client_invalid", TEST_MODULE)
{
    FileDescriptorClient client(2, 6);
    string result;
    CHECK_EXCEPTION_TYPE(client.SendRPCMessage("foobar", result), JsonRpcException, check_exception1);
}

#endif // FILEDESCRIPTOR_TESTING
