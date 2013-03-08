/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.cpp
 * @date    08.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "server.h"

using namespace jsonrpc;

static int cnt = 0;

void sayHello(const Json::Value& request, Json::Value& response)
{
    response = "Hello: " + request["name"].asString() + "!";
}
void getCounterValue(const Json::Value& request, Json::Value& response)
{
    response = cnt;
}
void add(const Json::Value& request, Json::Value& response)
{
    response = request["value1"].asInt() + request["value2"].asInt();

}
void sub(const Json::Value& request, Json::Value& response)
{
    response = request["value1"].asInt() - request["value2"].asInt();

}
void initCounter(const Json::Value& request)
{
    cnt = request["value"].asInt();
}
void incrementCounter(const Json::Value& request)
{
    cnt++;
}

jsonrpc::Server* getTestServer()
{
    methods_t methods;
    notifications_t notifications;

    methods["sayHello"] = sayHello;
    methods["getCounterValue"] = getCounterValue;
    methods["add"] = add;
    methods["sub"] = sub;

    notifications["initCounter"] = initCounter;
    notifications["incrementCounter"] = incrementCounter;

    return new Server("instance", "out/test/procedures.json", methods, notifications, new HttpServer(8080));
}

jsonrpc::Client* getTestClient()
{
    return new Client(new HttpClient("http://localhost:8080"), true);
}


