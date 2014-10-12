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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

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
}

BOOST_AUTO_TEST_SUITE_END()
