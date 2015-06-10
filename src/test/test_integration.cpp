/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_integration.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/


#ifdef STUBGEN_TESTING
#include <catch.hpp>

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "gen/abstractstubserver.h"
#include "gen/stubclient.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 8383
#define CLIENT_URL "http://localhost:8383"
#define TEST_PATH "/tmp/jsonrpccppintegrationtest"

#define TEST_MODULE "[integration]"

class StubServer : public AbstractStubServer {
    public:
        StubServer(AbstractServerConnector &connector) : AbstractStubServer(connector) {}
        virtual void notifyServer() {

        }

        virtual std::string sayHello(const std::string& name)
        {
            return string("Hello ") + name;
        }

        virtual int addNumbers(int param1, int param2)
        {
            return param1+param2;
        }

        virtual double addNumbers2(double param1, double param2)
        {
            return param1 + param2;
        }

        virtual bool isEqual(const std::string& str1, const std::string &str2)
        {
            return str1 == str2;
        }

        virtual Json::Value buildObject(const std::string &name, int age)
        {
            Json::Value result;
            result["name"] = name;
            result["age"] = age;
            return result;
        }

        virtual std::string methodWithoutParameters()
        {
            return "foo";
        }
};

#ifdef HTTP_TESTING

TEST_CASE("test_integration_http", TEST_MODULE)
{
    HttpServer sconn(TEST_PORT);
    HttpClient cconn(CLIENT_URL);
    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    CHECK(client.addNumbers(3,4) == 7);
    CHECK(client.addNumbers2(3.2,4.2) == 7.4);
    CHECK(client.sayHello("Test") == "Hello Test");
    CHECK(client.methodWithoutParameters() == "foo");
    CHECK(client.isEqual("str1", "str1") == true);
    CHECK(client.isEqual("str1", "str2") == false);

    Json::Value result = client.buildObject("Test", 33);
    CHECK(result["name"].asString() == "Test");
    CHECK(result["age"].asInt() == 33);

    server.StopListening();
}

#endif
#ifdef UNIXDOMAINSOCKET_TESTING

#include <jsonrpccpp/server/connectors/unixdomainsocketserver.h>
#include <jsonrpccpp/client/connectors/unixdomainsocketclient.h>

TEST_CASE("test_integration_unixdomain", TEST_MODULE)
{
    UnixDomainSocketServer sconn(TEST_PATH);
    UnixDomainSocketClient cconn(TEST_PATH);

    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    CHECK(client.addNumbers(3,4) == 7);
    CHECK(client.addNumbers2(3.2,4.2) == 7.4);
    CHECK(client.sayHello("Test") == "Hello Test");
    CHECK(client.methodWithoutParameters() == "foo");
    CHECK(client.isEqual("str1", "str1") == true);
    CHECK(client.isEqual("str1", "str2") == false);

    Json::Value result = client.buildObject("Test", 33);
    CHECK(result["name"].asString() == "Test");
    CHECK(result["age"].asInt() == 33);

    server.StopListening();
}
#endif

#endif
