/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_integration.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/


#ifdef INTEGRATION_TESTING
#include <boost/test/unit_test.hpp>

#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include "gen/abstractsubserver.h"
#include "gen/stubclient.h"

using namespace jsonrpc;
using namespace std;

#define TEST_PORT 8383
#define CLIENT_URL "http://localhost:8383"

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

BOOST_AUTO_TEST_SUITE(integration)

BOOST_AUTO_TEST_CASE(test_integration1)
{
    HttpServer sconn(TEST_PORT);
    HttpClient cconn(CLIENT_URL);
    StubServer server(sconn);
    server.StartListening();
    StubClient client(cconn);

    BOOST_CHECK_EQUAL(client.addNumbers(3,4), 7);
    BOOST_CHECK_EQUAL(client.addNumbers2(3.2,4.2), 7.4);
    BOOST_CHECK_EQUAL(client.sayHello("Test"), "Hello Test");
    BOOST_CHECK_EQUAL(client.methodWithoutParameters(), "foo");
    BOOST_CHECK_EQUAL(client.isEqual("str1", "str1"), true);
    BOOST_CHECK_EQUAL(client.isEqual("str1", "str2"), false);

    Json::Value result = client.buildObject("Test", 33);
    BOOST_CHECK_EQUAL(result["name"].asString(), "Test");
    BOOST_CHECK_EQUAL(result["age"].asInt(), 33);

    server.StopListening();
}

BOOST_AUTO_TEST_SUITE_END()

#endif
