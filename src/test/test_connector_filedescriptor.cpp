/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_unixdomainsocket.cpp
 * @date    26/10/2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef FILEDESCRIPTOR_TESTING
#include <string.h>
#include <catch.hpp>
#include <unistd.h>
#include <jsonrpccpp/server/connectors/filedescriptorserver.h>
#include <jsonrpccpp/client/connectors/filedescriptorclient.h>
#include "mockclientconnectionhandler.h"

#include <iostream>

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
                pipe(c2sfd);
                pipe(s2cfd);
                server = new FileDescriptorServer(c2sfd[0], s2cfd[1]);
                client = new FileDescriptorClient(s2cfd[0], c2sfd[1]);
                server->SetHandler(&handler);
                REQUIRE(server->StartListening());
            }
            ~F()
            {
                server->StopListening();
                delete server;
                delete client;
                close(c2sfd[0]);
                close(c2sfd[1]);
                close(s2cfd[0]);
                close(s2cfd[1]);
            }

            int c2sfd[2]; // Client to server fd
            int s2cfd[2]; // Server to client fd
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

    client->SendRPCMessage(request, result);

    CHECK(handler.request == request);
    CHECK(result == expectedResult);
}


TEST_CASE("test_filedescriptor_server_multiplestart", TEST_MODULE)
{
    char input_filename[38];
    strncpy(input_filename, "/tmp/test_filedescriptor_input.XXXXXX", 37);
    char output_filename[39];
    strncpy(output_filename, "/tmp/test_filedescriptor_output.XXXXXX", 38);
    int inputfd = mkstemp(input_filename);
    int outputfd = mkstemp(output_filename);
    FileDescriptorServer server(inputfd, outputfd);
    CHECK(server.StartListening() == true);
    CHECK(server.StartListening() == false);

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
