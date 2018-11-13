#pragma once

#include "abstractserver.h"

//Methods
namespace jsonrpc
{
    class TestServer : public jsonrpc::AbstractServer<TestServer>
    {
        public:
            TestServer(jsonrpc::serverVersion_t type = jsonrpc::JSONRPC_SERVER_V2);

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

            virtual bool bindAndAddMethod(const Procedure& proc, methodPointer_t pointer);
            virtual bool bindAndAddNotification(const Procedure& proc, notificationPointer_t pointer);
        private:
            int cnt;
    };
}