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
    Client c(new HttpClient("http://localhost:8081"), true);

    Json::Value params;
    params["name"] = "Peter";

    try
    {
        cout << c.CallMethod("sayHello", params) << endl;
    }
    catch (Exception e)
    {
        cerr << e.what() << endl;
    }


}
