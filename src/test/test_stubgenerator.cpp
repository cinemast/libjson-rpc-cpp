/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_connector_http.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifdef STUBGEN_TESTING
#include <boost/test/unit_test.hpp>

#include <jsonrpccpp/common/specificationparser.h>
#include <stubgenerator/server/cppserverstubgenerator.h>
#include <stubgenerator/client/cppclientstubgenerator.h>
#include <stubgenerator/client/jsclientstubgenerator.h>
#include <stubgenerator/helper/cpphelper.h>
#include <stubgenerator/stubgeneratorfactory.h>

#include <sstream>

using namespace jsonrpc;
using namespace std;

BOOST_AUTO_TEST_SUITE(stubgenerator)

BOOST_AUTO_TEST_CASE(test_stubgen_cppclient)
{
    stringstream stream;
    vector<Procedure> procedures = SpecificationParser::GetProceduresFromFile("testspec6.json");
    CPPClientStubGenerator stubgen("ns1::ns2::TestStubClient", procedures, stream);
    stubgen.generateStub();
    string result = stream.str();

    BOOST_CHECK_NE(result.find("#ifndef JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBCLIENT_H_"), string::npos);
    BOOST_CHECK_NE(result.find("#define JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBCLIENT_H_"), string::npos);
    BOOST_CHECK_NE(result.find("namespace ns1"), string::npos);
    BOOST_CHECK_NE(result.find("namespace ns2"), string::npos);
    BOOST_CHECK_NE(result.find("class TestStubClient : public jsonrpc::Client"), string::npos);
    BOOST_CHECK_NE(result.find("std::string test_method(const std::string& name) throw (jsonrpc::JsonRpcException)"), string::npos);
    BOOST_CHECK_NE(result.find("void test_notification(const std::string& name) throw (jsonrpc::JsonRpcException)"), string::npos);
    BOOST_CHECK_NE(result.find("double test_method2(const Json::Value& object, const Json::Value& values) throw (jsonrpc::JsonRpcException)"), string::npos);
    BOOST_CHECK_NE(result.find("void test_notification2(const Json::Value& object, const Json::Value& values) throw (jsonrpc::JsonRpcException)"), string::npos);
    BOOST_CHECK_NE(result.find("#endif //JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBCLIENT_H_"), string::npos);

    BOOST_CHECK_EQUAL(CPPHelper::class2Filename("ns1::ns2::TestClass"), "testclass.h");
}

BOOST_AUTO_TEST_CASE(test_stubgen_cppserver)
{
    stringstream stream;
    vector<Procedure> procedures = SpecificationParser::GetProceduresFromFile("testspec6.json");
    CPPServerStubGenerator stubgen("ns1::ns2::TestStubServer", procedures, stream);
    stubgen.generateStub();
    string result = stream.str();

    BOOST_CHECK_NE(result.find("#ifndef JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBSERVER_H_"), string::npos);
    BOOST_CHECK_NE(result.find("#define JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBSERVER_H_"), string::npos);
    BOOST_CHECK_NE(result.find("namespace ns1"), string::npos);
    BOOST_CHECK_NE(result.find("namespace ns2"), string::npos);
    BOOST_CHECK_NE(result.find("class TestStubServer : public jsonrpc::AbstractServer<TestStubServer>"), string::npos);
    BOOST_CHECK_NE(result.find("virtual std::string test_method(const std::string& name) = 0;"), string::npos);
    BOOST_CHECK_NE(result.find("virtual void test_notification(const std::string& name) = 0;"), string::npos);
    BOOST_CHECK_NE(result.find("virtual double test_method2(const Json::Value& object, const Json::Value& values) = 0;"), string::npos);
    BOOST_CHECK_NE(result.find("virtual void test_notification2(const Json::Value& object, const Json::Value& values) = 0;"), string::npos);
    BOOST_CHECK_NE(result.find("this->bindAndAddMethod(jsonrpc::Procedure(\"test.method\", jsonrpc::PARAMS_BY_NAME, jsonrpc::JSON_STRING, \"name\",jsonrpc::JSON_STRING, NULL), &ns1::ns2::TestStubServer::test_methodI);"), string::npos);
    BOOST_CHECK_NE(result.find("#endif //JSONRPC_CPP_STUB_NS1_NS2_TESTSTUBSERVER_H_"), string::npos);
}

BOOST_AUTO_TEST_CASE(test_stubgen_jsclient)
{
    stringstream stream;
    vector<Procedure> procedures = SpecificationParser::GetProceduresFromFile("testspec6.json");
    JSClientStubGenerator stubgen("TestStubClient", procedures, stream);
    stubgen.generateStub();
    string result = stream.str();

    BOOST_CHECK_NE(result.find("function TestStubClient(url) {"), string::npos);
    BOOST_CHECK_NE(result.find("TestStubClient.prototype.test_method = function(name, callbackSuccess, callbackError)"), string::npos);
    BOOST_CHECK_NE(result.find("TestStubClient.prototype.test_notification = function(name, callbackSuccess, callbackError)"), string::npos);
    BOOST_CHECK_NE(result.find("TestStubClient.prototype.test_method2 = function(object, values, callbackSuccess, callbackError)"), string::npos);
    BOOST_CHECK_NE(result.find("TestStubClient.prototype.test_notification2 = function(object, values, callbackSuccess, callbackError)"), string::npos);

    BOOST_CHECK_EQUAL(JSClientStubGenerator::class2Filename("TestClass"), "testclass.js");
}

BOOST_AUTO_TEST_CASE(test_stubgen_indentation)
{
    stringstream stream;
    CodeGenerator cg(stream);
    cg.setIndentSymbol("   ");
    cg.increaseIndentation();
    cg.write("abc");
    BOOST_CHECK_EQUAL(stream.str(), "   abc");

    stringstream stream2;
    CodeGenerator cg2(stream2);
    cg2.setIndentSymbol("\t");
    cg2.increaseIndentation();
    cg2.write("abc");
    BOOST_CHECK_EQUAL(stream2.str(), "\tabc");
}

BOOST_AUTO_TEST_CASE(test_stubgen_factory_help)
{
    vector<StubGenerator*> stubgens;
    vector<Procedure> procedures;
    const char* argv[2] = {"jsonrpcstub","-h"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(2, (char**)argv, procedures, stubgens), true);
    BOOST_CHECK_EQUAL(stubgens.empty(), true);
    BOOST_CHECK_EQUAL(procedures.empty(), true);
}

BOOST_AUTO_TEST_CASE(test_stubgen_factory_version)
{
    vector<StubGenerator*> stubgens;
    vector<Procedure> procedures;
    const char* argv[2] = {"jsonrpcstub","--version"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(2, (char**)argv, procedures, stubgens), true);
    BOOST_CHECK_EQUAL(stubgens.empty(), true);
    BOOST_CHECK_EQUAL(procedures.empty(), true);
}

BOOST_AUTO_TEST_CASE(test_stubgen_factory_error)
{
    vector<StubGenerator*> stubgens;
    vector<Procedure> procedures;
    const char* argv[2] = {"jsonrpcstub","--cpp-client=TestClient"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(2, (char**)argv, procedures, stubgens), false);
    BOOST_CHECK_EQUAL(stubgens.empty(), true);
    BOOST_CHECK_EQUAL(procedures.empty(), true);

    vector<StubGenerator*> stubgens2;
    vector<Procedure> procedures2;
    const char* argv2[2] = {"jsonrpcstub","--cpxp-client=TestClient"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(2, (char**)argv2, procedures2, stubgens2), false);
    BOOST_CHECK_EQUAL(stubgens2.empty(), true);
    BOOST_CHECK_EQUAL(procedures2.empty(), true);

    vector<StubGenerator*> stubgens3;
    vector<Procedure> procedures3;
    const char* argv3[3] = {"jsonrpcstub", "testspec1.json", "--cpp-client=TestClient"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(3, (char**)argv3, procedures3, stubgens3), false);
    BOOST_CHECK_EQUAL(stubgens3.empty(), true);
    BOOST_CHECK_EQUAL(procedures3.empty(), true);
}

BOOST_AUTO_TEST_CASE(test_stubgen_factory_success)
{
    vector<StubGenerator*> stubgens;
    vector<Procedure> procedures;
    const char* argv[5] = {"jsonrpcstub", "testspec6.json", "--js-client=TestClient", "--cpp-client=TestClient", "--cpp-server=TestServer"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(5, (char**)argv, procedures, stubgens), true);
    BOOST_CHECK_EQUAL(stubgens.size(), 3);
    BOOST_CHECK_EQUAL(procedures.size(), 7);

    StubGeneratorFactory::deleteStubGenerators(stubgens);
}

BOOST_AUTO_TEST_CASE(test_stubgen_factory_fileoverride)
{
    vector<StubGenerator*> stubgens;
    vector<Procedure> procedures;
    const char* argv[9] = {"jsonrpcstub", "testspec6.json", "--js-client=TestClient", "--cpp-client=TestClient", "--cpp-server=TestServer", "--cpp-client-file=client.h", "--cpp-server-file=server.h", "--js-client-file=client.js", "-v"};

    BOOST_CHECK_EQUAL(StubGeneratorFactory::createStubGenerators(9, (char**)argv, procedures, stubgens), true);
    BOOST_CHECK_EQUAL(stubgens.size(), 3);
    BOOST_CHECK_EQUAL(procedures.size(), 7);
    StubGeneratorFactory::deleteStubGenerators(stubgens);
}

BOOST_AUTO_TEST_SUITE_END()
#endif
