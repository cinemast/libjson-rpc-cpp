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

int main(int argc, char** argv)
{
    
    TestServer t;

    string spec = SpecificationWriter::toString(t.GetProtocolHanlder()->GetProcedures());
    cout << spec << endl;

    procedurelist_t* procs = SpecificationParser::GetProceduresFromString(spec);

    if(procs->size() != 6)
    {
        cerr << "Not ally procedures restored" << endl;
        return -1;
    }

    Procedure* proc = (*procs)["sayHello"];
    if(proc->GetProcedureName() != "sayHello" || proc->GetReturnType() != JSON_STRING || proc->GetParameters().size() != 1 || proc->GetParameters()["name"] != JSON_STRING)
    {
        cerr << "Method sayHello failed verification" << endl;
    }

    proc = (*procs)["getCounterValue"];
    if(proc->GetProcedureName() != "getCounterValue" || proc->GetReturnType() != JSON_INTEGER || proc->GetParameters().size() != 0)
    {
        cerr << "Method getCounterValue failed verification" << endl;
    }

    proc = (*procs)["add"];
    if(proc->GetProcedureName() != "add" || proc->GetReturnType() != JSON_INTEGER || proc->GetParameters().size() != 2 || proc->GetParameters()["value1"] != JSON_INTEGER || proc->GetParameters()["value2"] != JSON_INTEGER)
    {
        cerr << "Method add failed verification" << endl;
    }

    proc = (*procs)["sub"];
    if(proc->GetProcedureName() != "sub" || proc->GetReturnType() != JSON_INTEGER || proc->GetParameters().size() != 2 || proc->GetParameters()["value1"] != JSON_INTEGER || proc->GetParameters()["value2"] != JSON_INTEGER)
    {
        cerr << "Method sub failed verification" << endl;
    }

    proc = (*procs)["initCounter"];
    if(proc->GetProcedureName() != "initCounter" || proc->GetReturnType() != RPC_NOTIFICATION || proc->GetParameters().size() != 1 || proc->GetParameters()["value"] != JSON_INTEGER)
    {
        cerr << "Method initCounter failed verification" << endl;
    }

    proc = (*procs)["incrementCounter"];
    if(proc->GetProcedureName() != "incrementCounter" || proc->GetReturnType() != RPC_NOTIFICATION || proc->GetParameters().size() != 0)
    {
        cerr << "Method incrementCounter failed verification" << endl;
    }

    cout << argv[0] << " passed" << endl;

    return 0;
}
