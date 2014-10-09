/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_client.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#define BOOST_TEST_MODULE

#include <boost/test/unit_test.hpp>
#include <jsonrpccpp/client.h>
#include "mockclientconnector.h"

using namespace jsonrpc;
using namespace std;

bool check_exception1(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_RPC_JSON_PARSE_ERROR;
}

bool check_exception2(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_CLIENT_INVALID_RESPONSE;
}

BOOST_AUTO_TEST_SUITE(client)

BOOST_AUTO_TEST_CASE(test_client_method_success)
{
    MockClientConnector c;
    Client client(c);

    Json::Value params;
    params.append(23);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"result\": 23}");

    Json::Value response = client.CallMethod("abcd", params);
    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(v["id"].asInt(), 1);

    BOOST_CHECK_EQUAL(response.asInt(),23);
}

BOOST_AUTO_TEST_CASE(test_client_notification_success)
{
    MockClientConnector c;
    Client client(c);

    Json::Value params;
    params.append(23);

    client.CallNotification("abcd", params);

    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(v.isMember("id"), false);
}

BOOST_AUTO_TEST_CASE(test_client_invalidjson)
{
    MockClientConnector c;
    Client client(c);
    c.SetResponse("{\"method\":234");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception1);
}

BOOST_AUTO_TEST_CASE(test_client_invalidresponse)
{
    MockClientConnector c;
    Client client(c);

    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"resulto\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id2\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"1.0\", \"id\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("[]");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
}


BOOST_AUTO_TEST_SUITE_END()
