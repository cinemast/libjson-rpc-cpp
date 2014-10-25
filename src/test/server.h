/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file server.h
 * @date    08.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <jsonrpccpp/server.h>
#include <jsonrpccpp/client.h>

//Methods

class TestServer : public jsonrpc::AbstractServer<TestServer>
{
    public:
        TestServer(jsonrpc::AbstractServerConnector &connector, jsonrpc::requesthandler_t type = jsonrpc::JSONRPC_V2);

        void sayHello(const Json::Value& request, Json::Value& response);
        void getCounterValue(const Json::Value& request, Json::Value& response);
        void add(const Json::Value& request, Json::Value& response);
        void sub(const Json::Value& request, Json::Value& response);

        void exceptionMethod(const Json::Value& request, Json::Value& response);

        //Notifications
        void initCounter(const Json::Value& request);
        void incrementCounter(const Json::Value& request);
        void initZero(const Json::Value& request);

        int getCnt();

    private:
        int cnt;
};

#endif // SERVER_H
