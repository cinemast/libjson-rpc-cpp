/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_tcpsocket.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <boost/test/unit_test.hpp>

#ifdef TCPSOCKET_TESTING
#define BOOST_TEST_MODULE connector_tcpsocket

#include <jsonrpccpp/server/connectors/socketserver.h>
#include <jsonrpccpp/client/connectors/socketclient.h>
#include "mockclientconnectionhandler.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT "8282"

BOOST_AUTO_TEST_SUITE(connector_tcpsocket)

struct F {
    SocketServer server;
    SocketClient client;
    MockClientConnectionHandler handler;

    F() :
        server(TEST_PORT),
        client("localhost", TEST_PORT)
    {
        server.SetHandler(&handler);
        server.StartListening();
    }
    ~F()
    {
        server.StopListening();
    }
};

BOOST_FIXTURE_TEST_CASE(test_tcpsocket_success, F)
{
    handler.response = "exampleresponse";
    string result;
    client.SendRPCMessage("examplerequest", result);
    BOOST_CHECK_EQUAL(handler.request, "examplerequest");
    BOOST_CHECK_EQUAL(result, "exampleresponse");
}


BOOST_AUTO_TEST_SUITE_END()


#endif
