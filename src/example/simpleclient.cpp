/**
 * @file simpleclient.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief This is a simple client example.
 */

#include <jsonrpc/rpc.h>
#include <iostream>

using namespace jsonrpc;
using namespace std;

int main()
{
    HttpClient* httpClient = new HttpClient("http://localhost:8080");
    Client c(httpClient);

    Json::Value params;
    params["name"] = "Peter";

    try
    {
        cout << c.CallMethod("sayHello", params) << endl;
    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }

    delete httpClient;
}
