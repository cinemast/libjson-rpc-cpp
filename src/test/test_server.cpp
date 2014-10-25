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

BOOST_AUTO_TEST_CASE(test_server_v2_method_success)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asString(),"Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"add\",\"params\":{\"value1\":5,\"value2\":7}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),12);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);


    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);


    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": null, \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].isNull(), true);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);


    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": \"1\", \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asString(), "1");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);
}

BOOST_AUTO_TEST_CASE(test_server_v2_notification_success)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 33);
    BOOST_CHECK_EQUAL(c.GetResponse(), "");

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"incrementCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 66);
    BOOST_CHECK_EQUAL(c.GetResponse(), "");

}

BOOST_AUTO_TEST_CASE(test_server_v2_invalidjson)
{
    MockServerConnector c;
    TestServer server(c);

    c.SetRequest("{\"jsonrpc\":\"2.");

    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32700);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_v2_invalidrequest)
{
    MockServerConnector c;
    TestServer server(c);

    //wrong rpc version
    c.SetRequest("{\"jsonrpc\":\"1.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //wrong rpc version type
    c.SetRequest("{\"jsonrpc\":2.0, \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //no method name
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1,\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //wrong method name type
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": {}, \"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //invalid param structure
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":1}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 3.2, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
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

BOOST_AUTO_TEST_CASE(test_server_v2_method_error)
{
    MockServerConnector c;
    TestServer server(c);

    //invalid methodname
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello2\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32601);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //call notification as procedure
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"initCounter\",\"params\":{\"value\":3}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32605);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //call procedure as notification
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

BOOST_AUTO_TEST_CASE(test_server_v2_params_error)
{
    MockServerConnector c;
    TestServer server(c);

    //invalid param type
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":23}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //invalid param name
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name2\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //invalid parameter passing mode (array instead of object)
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":[\"Peter\"]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);

    //missing parameter field
    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\"}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("result"),false);
}

BOOST_AUTO_TEST_CASE(test_server_v2_batchcall_success)
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

BOOST_AUTO_TEST_CASE(test_server_v2_batchcall_error)
{
    MockServerConnector c;
    TestServer server(c);

    //success and error responses
    c.SetRequest("[{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}},{},{\"jsonrpc\":\"2.0\", \"id\": 3, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter3\"}}]");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().size(), 3);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["result"].asString(), "Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[1]["error"]["code"].asInt(), -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[1]["id"].isNull(), true);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[2]["result"].asString(), "Hello: Peter3!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[2]["id"].asInt(), 3);

    //only invalid requests
    c.SetRequest("[1,2,3]");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().size(), 3);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[0]["error"]["code"].asInt(), -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[1]["error"]["code"].asInt(), -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()[2]["error"]["code"].asInt(), -32600);
}

BOOST_AUTO_TEST_CASE(test_server_v1_method_success)
{
    MockServerConnector c;
    TestServer server(c, JSONRPC_V1);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\",\"params\":[5,7]}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(), -2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("jsonrpc"), false);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), true);
    BOOST_CHECK_EQUAL(c.GetJsonRequest()["error"], Json::nullValue);

    c.SetRequest("{\"id\": \"1\", \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asString(), "1");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("jsonrpc"), false);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), true);
    BOOST_CHECK_EQUAL(c.GetJsonRequest()["error"], Json::nullValue);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": \"1\", \"method\": \"sub\",\"params\":[5,7]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(),-2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asString(), "1");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("jsonrpc"), false);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), true);
    BOOST_CHECK_EQUAL(c.GetJsonRequest()["error"], Json::nullValue);
}

BOOST_AUTO_TEST_CASE(test_server_v1_notification_success)
{
    MockServerConnector c;
    TestServer server(c, JSONRPC_V1);

    c.SetRequest("{\"id\": null, \"method\": \"initZero\", \"params\": null}");
    BOOST_CHECK_EQUAL(server.getCnt(), 0);
    BOOST_CHECK_EQUAL(c.GetResponse(), "");
}

BOOST_AUTO_TEST_CASE(test_server_v1_method_invalid_request)
{
    MockServerConnector c;
    TestServer server(c, JSONRPC_V1);

    c.SetRequest("{\"method\": \"sub\", \"params\": []}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"],Json::nullValue);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\", \"params\": {\"foo\": true}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"],Json::nullValue);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\", \"params\": true}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"],Json::nullValue);

    c.SetRequest("{\"id\": 1, \"params\": []}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"], Json::nullValue);

    c.SetRequest("{}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"], Json::nullValue);

    c.SetRequest("[]");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"], Json::nullValue);

    c.SetRequest("23");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32600);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"], Json::nullValue);
}

BOOST_AUTO_TEST_CASE(test_server_v1_method_error)
{
    MockServerConnector c;
    TestServer server(c, JSONRPC_V1);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\", \"params\": [33]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"],Json::nullValue);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\", \"params\": [33, \"foo\"]}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["error"]["code"], -32602);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"],Json::nullValue);
}

BOOST_AUTO_TEST_CASE(test_server_hybrid)
{
    MockServerConnector c;
    TestServer server(c, JSONRPC_V1V2);

    c.SetRequest("{\"id\": 1, \"method\": \"sub\",\"params\":[5,7]}}");
    cout << c.GetJsonResponse() << endl;
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asInt(), -2);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("jsonrpc"), false);
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), true);
    BOOST_CHECK_EQUAL(c.GetJsonRequest()["error"], Json::nullValue);

    c.SetRequest("{\"id\": null, \"method\": \"initZero\", \"params\": null}");
    BOOST_CHECK_EQUAL(server.getCnt(), 0);
    BOOST_CHECK_EQUAL(c.GetResponse(), "");

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"id\": 1, \"method\": \"sayHello\",\"params\":{\"name\":\"Peter\"}}");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["result"].asString(),"Hello: Peter!");
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["id"].asInt(), 1);
    BOOST_CHECK_EQUAL(c.GetJsonResponse()["jsonrpc"].asString(), "2.0");
    BOOST_CHECK_EQUAL(c.GetJsonResponse().isMember("error"), false);

    c.SetRequest("{\"jsonrpc\":\"2.0\", \"method\": \"initCounter\",\"params\":{\"value\": 33}}");
    BOOST_CHECK_EQUAL(server.getCnt(), 33);
    BOOST_CHECK_EQUAL(c.GetResponse(), "");
}

BOOST_AUTO_TEST_SUITE_END()
