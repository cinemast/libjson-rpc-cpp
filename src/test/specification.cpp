/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specification.cpp
 * @date    02.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <jsonrpc/rpc.h>
#include "server.h"

using namespace std;
using namespace jsonrpc;

int main()
{
    
    TestServer t;

    string spec = SpecificationWriter::toString(t.GetProtocolHanlder()->GetProcedures());
    cout << spec << endl;


    return 0;
}
