/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubserver.cpp
 * @date    02.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#include <iostream>

#include "gen/abstractsubserver.h"
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <stdio.h>

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractStubServer
{
    public:
        MyStubServer(AbstractServerConnector &connector);

        virtual void notifyServer();
        virtual std::string sayHello(const std::string& name);
        virtual int addNumbers(const int& param1, const int& param2);
        virtual double addNumbers2(const double &param1, const double &param2);
        virtual bool isEqual(const std::string& str1, const std::string &str2);
        virtual Json::Value buildObject(const std::string &name, const int &age);
};

MyStubServer::MyStubServer(AbstractServerConnector &connector) :
    AbstractStubServer(connector)
{
}

void MyStubServer::notifyServer()
{
    cout << "Server got notified" << endl;
}

string MyStubServer::sayHello(const string &name)
{
    return "Hello " + name;
}

int MyStubServer::addNumbers(const int &param1, const int &param2)
{
    return param1 + param2;
}

double MyStubServer::addNumbers2(const double &param1, const double &param2)
{
    return param1 + param2;
}

bool MyStubServer::isEqual(const string &str1, const string &str2)
{
    return str1 == str2;
}

Json::Value MyStubServer::buildObject(const string &name, const int &age)
{
    Json::Value result;
    result["name"] = name;
    result["year"] = age;
    return result;
}

int main()
{
    HttpServer httpserver(8383);
    MyStubServer s(httpserver);
    s.StartListening();

    getchar();

    s.StopListening();

    return 0;
}
