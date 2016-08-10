/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubclient.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <iostream>

#include "gen/stubclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace jsonrpc;
using namespace std;

int main()
{

    Json::Value a = 3;
    Json::Value b = "3";

    std::map<Json::Value, Json::Value> responses;

    responses[a] = b;
    responses[b] = "asölfj";

    cout << responses[b] << endl;

    if (a == b)
    {
        cout << a.toStyledString() << " == " << b.toStyledString() << endl;
    }
    else
    {
        cout << a.toStyledString() << " != " << b.toStyledString() << endl;
    }

    HttpClient httpclient("http://localhost:8383");
    //StubClient c(httpclient, JSONRPC_CLIENT_V1); //json-rpc 1.0
    StubClient c(httpclient, JSONRPC_CLIENT_V2);   //json-rpc 2.0


    try
    {
        cout << c.sayHello("Peter Knafl") << endl;
        c.notifyServer();

        cout << " 3 + 5 = " << c.addNumbers(3,5) << endl;
        cout << " 3.2 + 5.3 = " << c.addNumbers2(3.2,5.3) << endl;
        cout << "Compare: " << c.isEqual("Peter", "peter") << endl;
        cout << "Build object: " << c.buildObject("Peter", 1990) << endl;

    }
    catch (JsonRpcException& e)
    {
        cerr << e.what() << endl;
    }
}

