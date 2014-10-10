/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_server.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <boost/test/unit_test.hpp>
#include "server.h"
#include "mockserverconnector.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

using namespace jsonrpc;
using namespace std;

BOOST_AUTO_TEST_SUITE(server)

BOOST_AUTO_TEST_CASE(test_server_method_success)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asString(),"Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"add\",\"params\":{\"value1\":5,\"value2\":7}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),12);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");


    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 33);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"incrementCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 66);

}

BOOST_AUTO_TEST_SUITE_END()
