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
}

BOOST_AUTO_TEST_CASE(test_server_notification_success)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 33);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"incrementCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 66);
}

BOOST_AUTO_TEST_CASE(test_server_invalidjson)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.");

    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32700);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_invalidrequest)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"1.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":2.0, \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1,\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\"}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":1}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": \"1\", \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": 3,\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("[]");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("23");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_method_error)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello2\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32601);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"initCounter\",\"params\":{\"value\":3}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32605);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32604);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"sub\",\"params\":{\"value1\":3, \"value\": 4}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32604);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"add\",\"params\":[3,4]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32604);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_params_error)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":23}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name2\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":[\"Peter\"]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_batchcall_success)
{
    MockServerConnector c;
    TestServer server(c);

    //Simple Batchcall with only methods
    c.SetRequest("[{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}},{\"jsonrpc\":\"2.0\", \"id\": 2, \"method\": \"add\",\"params\":{\"value1\":23,\"value2\": 33}}]");

    BOOST_CHECK_EQUAL(c.GetJsonResponse().size(), 2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["result"].asString(), "Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[1]["result"].asInt(), 56);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[1]["id"].asInt(), 2);

    //Batchcall containing methods and notifications
    c.SetRequest("[{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}},{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\":23}}]");

    BOOST_CHECK_EQUAL(c.GetJsonResponse().size(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["result"].asString(), "Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(server.getCnt(), 23);

    //Batchcall containing only notifications
    c.SetRequest("[{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\":23}},{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\":23}}]");

    BOOST_CHECK_EQUAL(c.GetResponse(), "");

}

BOOST_AUTO_TEST_CASE(test_server_batchcall_error)
{
    //same ids in batchcall
}

BOOST_AUTO_TEST_SUITE_END()
