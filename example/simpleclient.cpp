/**
 * @file simpleclient.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include <jsonrpc/rpc.h>
#include <iostream>

using namespace jsonrpc;
using namespace std;

int main()
{
    Client c(new HttpClient("http://localhost:8080"),true);
    Json::Value params;
    params["name"] = "peter";

    cout << c.CallMethod("sayHello", params);


    return 0;
}
