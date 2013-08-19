/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    parametervalidation.cpp
 * @date    04.07.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <jsonrpc/procedure.h>
#include <iostream>

using namespace jsonrpc;
using namespace std;

int main() {

    Procedure proc1("someprocedure", PARAMS_BY_NAME, JSON_BOOLEAN, "name", JSON_STRING, "ssnr", JSON_INTEGER, NULL);

    //Expected to pass validation
    Json::Value param1;
    param1["name"] = "Peter";
    param1["ssnr"] = 4711;

    if(!proc1.ValdiateParameters(param1))
    {
        cerr << "Vaildation returned false" << endl;
        return -1;
    }

    //Expected to fail validation
    Json::Value param2;
    param2.append("Peter");
    param2.append(4711);

    if(proc1.ValdiateParameters(param2))
    {
        cerr << "Vaildation returned false" << endl;
        return -2;
    }

    //Expected to fail validation
    Json::Value param3;
    param3.append(4711);
    param3.append("Peter");

    if(proc1.ValdiateParameters(param3))
    {
        cerr << "Vaildation returned true" << endl;
        return -3;
    }

    return 0;
}
