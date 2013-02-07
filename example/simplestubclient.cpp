#include <iostream>
#include <cstdlib>

#include <jsonrpc/rpc.h>
#include "MyStubName.h"

using namespace std;
using namespace jsonrpc;

int main(int argc, char** argv)
{

    try
    {
        MyStubName stub(new HttpClient("http://localhost:8080"));

        cout << stub.sayHello("Peter") << endl;
        stub.notifyServer();
    }
    catch (jsonrpc::Exception e)
    {
        cerr << e.what() << endl;
        return -1;
    }

    return 0;
}
