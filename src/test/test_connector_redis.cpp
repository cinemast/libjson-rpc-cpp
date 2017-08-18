/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_redis.cpp
 * @date    13.08.2017
 * @author  Jacques Software <software@jacques.com.au>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef REDIS_TESTING
#include <catch.hpp>
#include <jsonrpccpp/client/connectors/redisclient.h>

#include "checkexception.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 6379
#define CLIENT_HOST "127.0.0.1"
#define TEST_QUEUE "mytest"

#define TEST_MODULE "[connector_redis]"

namespace testredisserver
{
    struct F {

            F()
            {
            }
            ~F()
            {
            }
    };

    bool check_exception1(JsonRpcException const&ex)
    {
        return ex.GetCode() == Errors::ERROR_CLIENT_CONNECTOR;
    }

    bool check_exception2(JsonRpcException const&ex)
    {
        return ex.GetCode() == Errors::ERROR_RPC_INTERNAL_ERROR;
    }
}
using namespace testredisserver;

TEST_CASE_METHOD(F, "test_redis_client_init", TEST_MODULE)
{
    RedisClient client(CLIENT_HOST, TEST_PORT, TEST_QUEUE);
}

TEST_CASE_METHOD(F, "test_redis_client_send", TEST_MODULE)
{
    RedisClient client(CLIENT_HOST, TEST_PORT, TEST_QUEUE);

    string result;
    CHECK_EXCEPTION_TYPE(client.SendRPCMessage("test", result), JsonRpcException, check_exception2);
}

#endif
