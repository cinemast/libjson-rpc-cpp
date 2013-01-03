/**
 * @file simpleclient.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include <jsonrpc/rpc.h>
#include <iostream>
#include "FooBar.h"

using namespace jsonrpc;
using namespace std;

int main()
{
    FooBar stub(new HttpClient("localhost:8080"));

    cout << stub.sayHello("Peter Spiess-Knafl").asString() << endl;


    return 0;
}
