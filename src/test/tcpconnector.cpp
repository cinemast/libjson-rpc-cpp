/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpconnector.cpp
 * @date    07.10.2014
 * @author  Bertrand Cachet <bertrand.cachet@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <jsonrpc/rpc.h>

#include <stdio.h>
#include <string>
#include <iostream>

#include "server.h"
#include <jsonrpc/procedure.h>
#include <jsonrpc/specificationwriter.h>


using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{
#if defined(_WIN32) && !defined(__INTIME__)
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif

    SocketServer server_connector = SocketServer("8888", SOCK_STREAM, 2);
    TestServer server = TestServer(&server_connector);
    SocketClient client_connector = SocketClient("127.0.0.1", "8888");
    SocketClient client_connector2 = SocketClient("127.0.0.1", "8888");
    Client client = Client(&client_connector);
    Client client2 = Client(&client_connector2);
  int status = 0;

    cout << SpecificationWriter::toString(server.GetProtocolHanlder()->GetProcedures()) << endl;

    try {
        server.StartListening();

        Json::Value v;
        v["name"] = "Peter";
        Json::Value result = client.CallMethod("sayHello", v);
        Json::Value result2 = client2.CallMethod("sayHello", v);

        if((result.asString() != "Hello: Peter!")
           && (result2.asString() != "Hello: Peter!")
           ) {
            cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter!\"" << endl;
            status = -1;
            goto cleanup;
        }

        v["name"] = "Peter Spiess-Knafl";
        result = client.CallMethod("sayHello", v);
        if(result.asString() != "Hello: Peter Spiess-Knafl!") {
            cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter Spiess-Knafl!\"" << endl;
            status = -2;
            goto cleanup;
        }

        server.StopListening();

        cout << argv[0] << " passed" << endl;
    } catch(jsonrpc::JsonRpcException e) {

        cerr << "Exception occured: " << e.what() << endl;
        status = -999;
        goto cleanup;
    }
cleanup:
#if defined(_WIN32) && !defined(__INTIME__)
  WSACleanup();
#endif
  return status;
}
