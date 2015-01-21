/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_client.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

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

struct F {
    MockClientConnector c;
    Client client;
    Json::Value params;

    F() : client(c, JSONRPC_CLIENT_V2)
    {
    }
};

struct F1 {
    MockClientConnector c;
    Client client;
    Json::Value params;

    F1() : client(c, JSONRPC_CLIENT_V1)
    {
    }
};


BOOST_FIXTURE_TEST_CASE(test_client_v2_method_success, F)
{
    params.append(23);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"result\": 23}");
    Json::Value response = client.CallMethod("abcd", params);
    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(v["id"].asInt(), 1);
}

BOOST_FIXTURE_TEST_CASE(test_client_v2_notification_success, F)
{
    params.append(23);
    client.CallNotification("abcd", params);
    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(v.isMember("id"), false);
}

BOOST_FIXTURE_TEST_CASE(test_client_v2_errorresponse, F)
{
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception3);
}

BOOST_FIXTURE_TEST_CASE(test_client_v2_invalidjson, F)
{
    c.SetResponse("{\"method\":234");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception1);
}

BOOST_FIXTURE_TEST_CASE(test_client_v2_invalidresponse, F)
{
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"resulto\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id2\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"1.0\", \"id\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"result\": 23, \"error\": {}}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"jsonrpc\":\"2.0\", \"id\": 1, \"error\": {}}");
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

BOOST_FIXTURE_TEST_CASE(test_client_v2_batchcall_success, F)
{
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

BOOST_FIXTURE_TEST_CASE(test_client_v2_batchcall_error, F)
{
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
    BOOST_CHECK_EQUAL(response.getErrorMessage(2), "error1");
    BOOST_CHECK_EQUAL(response.getErrorMessage(3), "");

    c.SetResponse("{}");
    BOOST_CHECK_EXCEPTION(client.CallProcedures(bc), JsonRpcException, check_exception2);

    c.SetResponse("[1,2,3]");
    BOOST_CHECK_EXCEPTION(client.CallProcedures(bc), JsonRpcException, check_exception2);

    c.SetResponse("[[],[],[]]");
    BOOST_CHECK_EXCEPTION(client.CallProcedures(bc), JsonRpcException, check_exception2);
}

BOOST_FIXTURE_TEST_CASE(test_client_v1_method_success, F1)
{
    params.append(23);
    c.SetResponse("{\"id\": 1, \"result\": 23, \"error\": null}");

    Json::Value response = client.CallMethod("abcd", params);
    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v.isMember("jsonrpc"), false);
    BOOST_CHECK_EQUAL(v["id"].asInt(), 1);

    BOOST_CHECK_EQUAL(response.asInt(),23);
}

BOOST_FIXTURE_TEST_CASE(test_client_v1_notification_success, F1)
{
    params.append(23);

    client.CallNotification("abcd", params);

    Json::Value v = c.GetJsonRequest();

    BOOST_CHECK_EQUAL(v["method"].asString(), "abcd");
    BOOST_CHECK_EQUAL(v["params"][0].asInt(), 23);
    BOOST_CHECK_EQUAL(v.isMember("id"), true);
    BOOST_CHECK_EQUAL(v["id"], Json::nullValue);
}

BOOST_FIXTURE_TEST_CASE(test_client_v1_errorresponse, F1)
{
    c.SetResponse("{\"result\": null, \"error\": {\"code\": -32600, \"message\": \"Invalid Request\"}, \"id\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception3);

    c.SetResponse("{\"result\": null, \"error\": {\"code\": -32600}, \"id\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception3);
}

BOOST_FIXTURE_TEST_CASE(test_client_v1_invalidresponse, F1)
{
    c.SetResponse("{\"id\": 1, \"resulto\": 23, \"error\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"result\": 23}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"error\": null}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"result\": 23, \"error\": {}}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{\"id\": 1, \"result\": null, \"error\": {}}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("{}");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("[]");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
    c.SetResponse("23");
    BOOST_CHECK_EXCEPTION(client.CallMethod("abcd", Json::nullValue), JsonRpcException, check_exception2);
}

BOOST_AUTO_TEST_SUITE_END()
