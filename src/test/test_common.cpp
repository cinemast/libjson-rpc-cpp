/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_common.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <boost/test/unit_test.hpp>
#include <jsonrpccpp/common/procedure.h>
#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/common/specificationparser.h>
#include <jsonrpccpp/common/specificationwriter.h>

using namespace jsonrpc;
using namespace std;

BOOST_AUTO_TEST_SUITE(common)

BOOST_AUTO_TEST_CASE(test_procedure_parametervalidation)
{
    Procedure proc1("someprocedure", PARAMS_BY_NAME, JSON_BOOLEAN, "name", JSON_STRING, "ssnr", JSON_INTEGER, NULL);

    //Expected to pass validation
    Json::Value param1;
    param1["name"] = "Peter";
    param1["ssnr"] = 4711;
    BOOST_CHECK_EQUAL(proc1.ValidateNamedParameters(param1), true);

    //Expected to fail validation
    Json::Value param2;
    param2.append("Peter");
    param2.append(4711);
    BOOST_CHECK_EQUAL(proc1.ValidateNamedParameters(param2), false);

    //Expected to fail validation
    Json::Value param3;
    param3.append(4711);
    param3.append("Peter");
    BOOST_CHECK_EQUAL(proc1.ValidateNamedParameters(param3), false);

    Procedure proc2("someprocedure", PARAMS_BY_NAME, JSON_BOOLEAN, "bool", JSON_BOOLEAN, "object", JSON_OBJECT, "array", JSON_ARRAY, "real", JSON_REAL, "int", JSON_INTEGER, NULL);
    Json::Value param4;
    Json::Value array;
    array.append(0);
    param4["bool"] = true;
    param4["object"] = param1;
    param4["array"] = array;
    param4["real"] = 0.332;
    param4["int"] = 3;

    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), true);

    param4["bool"] = "String";
    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), false);
    param4["bool"] = true;

    param4["object"] = "String";
    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), false);
    param4["object"] = param1;

    param4["real"] = "String";
    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), false);
    param4["real"] = 0.322;

    param4["array"] = "String";
    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), false);
    param4["array"] = array;

    param4["int"] = "String";
    BOOST_CHECK_EQUAL(proc2.ValidateNamedParameters(param4), false);
}

BOOST_AUTO_TEST_CASE(test_exception)
{
    JsonRpcException ex(Errors::ERROR_CLIENT_CONNECTOR);
    BOOST_CHECK_EQUAL(ex.what(), "Exception -32003 : Client connector error");
    BOOST_CHECK_EQUAL(ex.GetMessage(), "Client connector error");
    BOOST_CHECK_EQUAL(ex.GetCode(), -32003);

    JsonRpcException ex2(Errors::ERROR_CLIENT_CONNECTOR, "addInfo");
    BOOST_CHECK_EQUAL(ex2.what(), "Exception -32003 : Client connector error: addInfo");

    JsonRpcException ex3("addInfo");
    BOOST_CHECK_EQUAL(ex3.what(), "addInfo");
    BOOST_CHECK_EQUAL(ex3.GetMessage(), "addInfo");
    BOOST_CHECK_EQUAL(ex3.GetCode(), 0);

    Json::Value data;
    data.append(13);
    data.append(41);
    JsonRpcException ex4(Errors::ERROR_RPC_INTERNAL_ERROR, "internal error", data);
    BOOST_CHECK_EQUAL(ex4.GetData().size(), 2);
    BOOST_CHECK_EQUAL(ex4.GetData()[0].asInt(), 13);
    BOOST_CHECK_EQUAL(ex4.GetData()[1].asInt(), 41);

}

bool check_exception1(JsonRpcException const & ex)
{
    return ex.GetCode() == Errors::ERROR_RPC_JSON_PARSE_ERROR;
}

bool check_exception2(JsonRpcException const & ex)
{
    return ex.GetCode() == Errors::ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX;
}

BOOST_AUTO_TEST_CASE(test_specificationparser_errors)
{
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromFile("testspec1.json"), JsonRpcException, check_exception1);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromFile("testspec2.json"), JsonRpcException, check_exception2);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromFile("testspec3.json"), JsonRpcException, check_exception2);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromFile("testspec4.json"), JsonRpcException, check_exception2);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromString("{}"), JsonRpcException, check_exception2);

    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromString("[{\"name\":\"proc1\"},{\"name\":\"proc1\"}]"), JsonRpcException, check_exception2);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromString("[{\"name\":\"proc1\", \"params\": {\"param1\": null}}]"), JsonRpcException, check_exception2);
    BOOST_CHECK_EXCEPTION(SpecificationParser::GetProceduresFromString("[{\"name\":\"proc1\", \"params\": 23}]"), JsonRpcException, check_exception2);
}

BOOST_AUTO_TEST_CASE(test_specificationparser_success)
{
    std::vector<Procedure> procs = SpecificationParser::GetProceduresFromFile("testspec5.json");
    BOOST_REQUIRE_EQUAL(procs.size(), 4);

    BOOST_CHECK_EQUAL(procs[0].GetProcedureName(), "testmethod");
    BOOST_CHECK_EQUAL(procs[0].GetReturnType(), JSON_STRING);
    BOOST_CHECK_EQUAL(procs[0].GetProcedureType(), RPC_METHOD);
    BOOST_CHECK_EQUAL(procs[0].GetParameterDeclarationType(), PARAMS_BY_NAME);

    BOOST_CHECK_EQUAL(procs[2].GetProcedureName(), "testmethod2");
    BOOST_CHECK_EQUAL(procs[2].GetReturnType(), JSON_REAL);
    BOOST_CHECK_EQUAL(procs[2].GetProcedureType(), RPC_METHOD);
    BOOST_CHECK_EQUAL(procs[2].GetParameterDeclarationType(), PARAMS_BY_NAME);

    BOOST_CHECK_EQUAL(procs[1].GetProcedureName(), "testnotification");
    BOOST_CHECK_EQUAL(procs[1].GetProcedureType(), RPC_NOTIFICATION);
    BOOST_CHECK_EQUAL(procs[1].GetParameterDeclarationType(), PARAMS_BY_NAME);

    BOOST_CHECK_EQUAL(procs[3].GetProcedureName(), "testnotification2");
    BOOST_CHECK_EQUAL(procs[3].GetProcedureType(), RPC_NOTIFICATION);
    BOOST_CHECK_EQUAL(procs[3].GetParameterDeclarationType(), PARAMS_BY_NAME);
}

BOOST_AUTO_TEST_CASE(test_specificationwriter)
{
    vector<Procedure> procedures;

    procedures.push_back(Procedure("testmethod1", PARAMS_BY_NAME, JSON_INTEGER, "param1", JSON_INTEGER, "param2", JSON_REAL, NULL));
    procedures.push_back(Procedure("testmethod2", PARAMS_BY_POSITION, JSON_INTEGER, "param1", JSON_OBJECT, "param2", JSON_ARRAY, NULL));

    procedures.push_back(Procedure("testnotification1", PARAMS_BY_NAME, "param1", JSON_BOOLEAN, "param2", JSON_STRING, NULL));
    procedures.push_back(Procedure("testnotification2", PARAMS_BY_POSITION, "param1", JSON_INTEGER, "param2", JSON_STRING, NULL));

    procedures.push_back(Procedure("testnotification3", PARAMS_BY_POSITION, NULL));

    Json::Value result = SpecificationWriter::toJsonValue(procedures);

    BOOST_REQUIRE_EQUAL(result.isArray(), true);
    BOOST_REQUIRE_EQUAL(result.size(), procedures.size());

    BOOST_CHECK_EQUAL(result[0]["name"].asString(), "testmethod1");
    BOOST_CHECK_EQUAL(result[1]["name"].asString(), "testmethod2");
    BOOST_CHECK_EQUAL(result[2]["name"].asString(), "testnotification1");
    BOOST_CHECK_EQUAL(result[3]["name"].asString(), "testnotification2");
    BOOST_CHECK_EQUAL(result[4]["name"].asString(), "testnotification3");

    BOOST_REQUIRE_EQUAL(result[0]["params"].isObject(), true);
    BOOST_CHECK_EQUAL(result[0]["params"]["param1"].isInt(), true);
    BOOST_CHECK_EQUAL(result[0]["params"]["param2"].isDouble(), true);

    BOOST_REQUIRE_EQUAL(result[1]["params"].isArray(), true);
    BOOST_CHECK_EQUAL(result[1]["params"][0].isObject(), true);
    BOOST_CHECK_EQUAL(result[1]["params"][1].isArray(), true);

    BOOST_REQUIRE_EQUAL(result[2]["params"].isObject(), true);
    BOOST_CHECK_EQUAL(result[2]["params"]["param1"].isBool(), true);
    BOOST_CHECK_EQUAL(result[2]["params"]["param2"].isString(), true);

    BOOST_REQUIRE_EQUAL(result[3]["params"].isArray(), true);
    BOOST_CHECK_EQUAL(result[3]["params"][0].isInt(), true);
    BOOST_CHECK_EQUAL(result[3]["params"][1].isString(), true);

    BOOST_CHECK_EQUAL(result[4].isMember("params"), false);

    BOOST_CHECK_EQUAL(result[0]["returns"].isInt(), true);
    BOOST_CHECK_EQUAL(result[1]["returns"].isInt(), true);

    BOOST_CHECK_EQUAL(SpecificationWriter::toFile("testspec.json", procedures), true);
    BOOST_CHECK_EQUAL(SpecificationWriter::toFile("/a/b/c/testspec.json", procedures), false);
}

BOOST_AUTO_TEST_SUITE_END()
