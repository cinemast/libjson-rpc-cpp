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

#include <jsonrpc/rpc.h>

//Methods
    class TestServer : public jsonrpc::AbstractServer<TestServer>
    {
        public:
            TestServer();
            TestServer(jsonrpc::AbstractServerConnector* connection);

            void sayHello(const Json::Value& request, Json::Value& response);
            void getCounterValue(const Json::Value& request, Json::Value& response);
            void add(const Json::Value& request, Json::Value& response);
            void sub(const Json::Value& request, Json::Value& response);

            //Notifications
            void initCounter(const Json::Value& request);
            void incrementCounter(const Json::Value& request);
        private:
            int cnt;
            void initialize();

    };

#endif // SERVER_H
