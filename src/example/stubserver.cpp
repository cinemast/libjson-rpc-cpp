/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubserver.cpp
 * @date    02.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#include <jsonrpc/rpc.h>
#include <iostream>
#include <jsonrpc/connectors/httpserver.h>
#include "abstractmystubserver.h"

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractMyStubServer
{
    public:
        MyStubServer();

        virtual void notifyServer();
        virtual std::string sayHello(const std::string& name);
        virtual int addNumbers(const int& param1, const int& param2);
};

MyStubServer::MyStubServer() :
    AbstractMyStubServer(new HttpServer(8080))
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

int main()
{
    MyStubServer s;
    s.StartListening();

    getchar();

    s.StopListening();

    return 0;
}
