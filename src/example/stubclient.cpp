/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubclient.cpp
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <jsonrpc/rpc.h>
#include <iostream>

#include "mystubclient.h"

using namespace jsonrpc;
using namespace std;

int main()
{
    HttpClient* httpClient = new HttpClient("http://localhost:8080");
    MyStubClient c(httpClient);

    try
    {
        cout << c.sayHello("Peter Knafl") << endl;
        c.notifyServer();

        cout << " 3 + 5 = " << c.addNumbers(3,5) << endl;

    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }

    delete httpClient;
}

