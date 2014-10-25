/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.cpp
 * @date    08.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "server.h"
#include <iostream>

using namespace std;
using namespace jsonrpc;

TestServer::TestServer(AbstractServerConnector &connector, requesthandler_t type) :
    AbstractServer<TestServer>(connector, type),
    cnt(-1)
{
    this->bindAndAddMethod(new Procedure("sayHello", PARAMS_BY_NAME, JSON_STRING, "name", JSON_STRING, NULL), &TestServer::sayHello);
    this->bindAndAddMethod(new Procedure("getCounterValue", PARAMS_BY_NAME, JSON_INTEGER, NULL), &TestServer::getCounterValue);
    this->bindAndAddMethod(new Procedure("add", PARAMS_BY_NAME, JSON_INTEGER, "value1", JSON_INTEGER, "value2", JSON_INTEGER, NULL), &TestServer::add);
    this->bindAndAddMethod(new Procedure("sub", PARAMS_BY_POSITION, JSON_INTEGER, "value1", JSON_INTEGER, "value2", JSON_INTEGER, NULL), &TestServer::sub);
    this->bindAndAddMethod(new Procedure("exceptionMethod", PARAMS_BY_POSITION, JSON_NULL, NULL), &TestServer::exceptionMethod);

    this->bindAndAddNotification(new Procedure("initCounter", PARAMS_BY_NAME, "value", JSON_INTEGER, NULL), &TestServer::initCounter);
    this->bindAndAddNotification(new Procedure("incrementCounter", PARAMS_BY_NAME, "value", JSON_INTEGER, NULL), &TestServer::incrementCounter);
    this->bindAndAddNotification(new Procedure("initZero", PARAMS_BY_POSITION, NULL), &TestServer::initZero);
}

void TestServer::sayHello(const Json::Value &request, Json::Value& response)
{
    response = "Hello: " + request["name"].asString() + "!";
}

void TestServer::getCounterValue(const Json::Value &request, Json::Value &response)
{
    response = cnt;
}

void TestServer::add(const Json::Value &request, Json::Value &response)
{
    response = request["value1"].asInt() + request["value2"].asInt();
}

void TestServer::sub(const Json::Value &request, Json::Value &response)
{
    response = request[0].asInt() - request[1].asInt();
}

void TestServer::exceptionMethod(const Json::Value &request, Json::Value &response)
{
    throw JsonRpcException(-32099, "User exception");
}

void TestServer::initCounter(const Json::Value &request)
{
    cnt= request["value"].asInt();
}

void TestServer::incrementCounter(const Json::Value &request)
{
    cnt+= request["value"].asInt();
}

void TestServer::initZero(const Json::Value &request)
{
    cnt = 0;
}

int TestServer::getCnt()
{
    return cnt;
}

