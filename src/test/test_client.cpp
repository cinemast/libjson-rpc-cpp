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
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "server.h"

using namespace jsonrpc;
using namespace std;

bool check_exception1(JsonRpcException const&ex)
{
    return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
}

BOOST_AUTO_TEST_CASE(test_client_httpclient_error)
{
    HttpClient c("absdfasöj");
    string result;
    BOOST_CHECK_EXCEPTION(c.SendRPCMessage("foo", result), JsonRpcException, check_exception1);
}

BOOST_AUTO_TEST_CASE(test_client_httpclient_success)
{
    HttpClient c("http://www.google.at");
    string result;
    c.SendRPCMessage("foo", result);
    BOOST_CHECK_EQUAL(result.substr(0, 1), "<");

    c.SetUrl("http://docs.google.com");
    c.SendRPCMessage("foo", result);
    BOOST_CHECK_EQUAL(result.substr(0, 1), "<");
}

BOOST_AUTO_TEST_CASE(test_client_clientprotocol_batchrequest)
{



    //client.BuildBatchRequest()
}
