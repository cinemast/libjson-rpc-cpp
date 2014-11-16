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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

using namespace jsonrpc;
using namespace std;

BOOST_AUTO_TEST_SUITE(common)

void test(const Procedure& proc)
{

}

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

    Procedure proc2("someprocedure", PARAMS_BY_NAME, JSON_BOOLEAN, "bool", JSON_BOOLEAN, "object", JSON_OBJECT, "array", JSON_ARRAY, "real", JSON_REAL, "null", JSON_NULL, NULL);
    Json::Value param4;
    Json::Value array;
    array.append(0);
    param4["bool"] = true;
    param4["object"] = param1;
    param4["array"] = array;
    param4["real"] = 0.332;
    param4["null"] = Json::nullValue;

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

    param4["null"] = "String";
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
}

BOOST_AUTO_TEST_SUITE_END()
