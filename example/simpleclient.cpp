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
    HttpClient connection("http://localhost:8080");
    Client c(connection);

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
