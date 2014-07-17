/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpconnector.cpp
 * @date    07.10.2014
 * @author  Bertrand Cachet <bertrand.cachet@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>

#include "server.h"
#include <jsonrpc/procedure.h>
#include <jsonrpc/specificationwriter.h>
#include <jsonrpc/connectors/socketserver.h>
#include <jsonrpc/connectors/socketclient.h>

using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{
    SocketServer* connector = new SocketServer("8080", SOCK_STREAM, 2);
    TestServer* server = new TestServer(connector);
    Client* client = new Client(new SocketClient("localhost", "8080"));
    Client* client2 = new Client(new SocketClient("localhost", "8080"));

    cout << SpecificationWriter::toString(server->GetProtocolHanlder()->GetProcedures()) << endl;

    try {
        server->StartListening();

        Json::Value v;
        v["name"] = "Peter";
        Json::Value result = client->CallMethod("sayHello", v);
        Json::Value result2 = client2->CallMethod("sayHello", v);

        if((result.asString() != "Hello: Peter!") && (result2.asString() != "Hello: Peter!")) {
            cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter!\"" << endl;
            return -1;
        }

        v["name"] = "Peter Spiess-Knafl";
        result = client->CallMethod("sayHello", v);
        if(result.asString() != "Hello: Peter Spiess-Knafl!") {
            cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter Spiess-Knafl!\"" << endl;
            return -2;
        }

        delete server;
        delete client;
        delete client2;

        cout << argv[0] << " passed" << endl;

        return 0;

    } catch(jsonrpc::JsonRpcException e) {

        cerr << "Exception occured: " << e.what() << endl;
        delete server;
        delete client;
        return -999;
    }
}
