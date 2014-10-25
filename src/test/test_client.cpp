/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_client.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#define BOOST_TEST_DYN_LINK
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

bool check_exception3(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_RPC_INVALID_REQUEST;
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

BOOST_AUTO_TEST_CASE(test_client_errorresponse)
{
    MockClientConnector c;
    Client client(c);

    c.SetResponse("{\"jsonrpc\":\"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception3);
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
    c.SetResponse("23");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
}

BOOST_AUTO_TEST_CASE(test_client_batchcall_success)
{
    MockClientConnector c;
    Client client(c);

    BatchCall bc;
    BOOST_CHECK_EQUAL(bc.addCall("abc", Json::nullValue, false),1);
    BOOST_CHECK_EQUAL(bc.addCall("def", Json::nullValue, true), -1);
    BOOST_CHECK_EQUAL(bc.addCall("abc", Json::nullValue, false),2);

    c.SetResponse("[{\"jsonrpc\":\"2.0\", \"id\": 1, \"result\": 23},{\"jsonrpc\":\"2.0\", \"id\": 2, \"result\": 24}]");

    BatchResponse response = client.CallProcedures(bc);

    BOOST_CHECK_EQUAL(response.hasErrors(), false);
    BOOST_CHECK_EQUAL(response.getResult(1).asInt(), 23);
    BOOST_CHECK_EQUAL(response.getResult(2).asInt(), 24);
    BOOST_CHECK_EQUAL(response.getResult(3).isNull(),true);

    Json::Value request = c.GetJsonRequest();
    BOOST_CHECK_EQUAL(request.size(), 3);
    BOOST_CHECK_EQUAL(request[0]["method"].asString(), "abc");
    BOOST_CHECK_EQUAL(request[0]["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(request[1]["method"].asString(), "def");
    BOOST_CHECK_EQUAL(request[1]["id"].isNull(), true);
    BOOST_CHECK_EQUAL(request[2]["id"].asInt(), 2);

    bc.toString(false);
}

BOOST_AUTO_TEST_CASE(test_client_batchcall_error)
{
    MockClientConnector c;
    Client client(c);

    BatchCall bc;
    BOOST_CHECK_EQUAL(bc.addCall("abc", Json::nullValue, false),1);
    BOOST_CHECK_EQUAL(bc.addCall("def", Json::nullValue, false),2);
    BOOST_CHECK_EQUAL(bc.addCall("abc", Json::nullValue, false),3);

    c.SetResponse("[{\"jsonrpc\":\"2.0\", \"id\": 1, \"result\": 23},{\"jsonrpc\":\"2.0\", \"id\": 2, \"error\": {\"code\": -32001, \"message\": \"error1\"}},{\"jsonrpc\":\"2.0\", \"id\": null, \"error\": {\"code\": -32002, \"message\": \"error2\"}}]");

    BatchResponse response = client.CallProcedures(bc);

    BOOST_CHECK_EQUAL(response.hasErrors(), true);
    BOOST_CHECK_EQUAL(response.getResult(1).asInt(), 23);
    BOOST_CHECK_EQUAL(response.getResult(2).isNull(), true);
    BOOST_CHECK_EQUAL(response.getResult(3).isNull(),true);
}

BOOST_AUTO_TEST_SUITE_END()
