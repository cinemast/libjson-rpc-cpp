/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    concurrency.cpp
 * @date    12.08.2014
 * @author  Emilien Kenler <ekenler@wizcorp.jp>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>

#include "server.h"
#include <jsonrpc/procedure.h>
#include <jsonrpc/specificationwriter.h>
#include <thread>

using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{

    TestServer* server = new TestServer();
    HttpClient *httpClient = new HttpClient("http://localhost:8080");
    Client* client = new Client(httpClient);

    cout << SpecificationWriter::toString(server->GetProtocolHanlder()->GetProcedures()) << endl;

    try {
        server->StartListening();

        int error = 0;

        std::thread t1 = std::thread([client,&error]{
            Json::Value v;
            v["name"] = "Peter";
            Json::Value result = client->CallMethod("sayHello", v);

            if(result.asString() != "Hello: Peter!") {
                cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter!\"" << endl;
                error = -1;
            }
        });

        std::thread t2 = std::thread([client,&error]{
            Json::Value v;
            v["name"] = "Peter Spiess-Knafl";
            Json::Value result = client->CallMethod("sayHello", v);

            if(result.asString() != "Hello: Peter Spiess-Knafl!") {
                cerr << "sayHello returned " << result.asString() << " but should be \"Hello: Peter Spiess-Knafl!\"" << endl;
                error = -2;
            }
        });

        t1.join();
        cout << "t1 joined" << endl;
        t2.join();
        cout << "t2 joined" << endl;

        delete server;
        delete client;
        delete httpClient;

        if (error < 0) {
            return error;
        }

        cout << argv[0] << " passed" << endl;

        return 0;

    } catch(jsonrpc::JsonRpcException e) {

        cerr << "Exception occured: " << e.what() << endl;
        delete server;
        delete client;
        delete httpClient;
        return -999;
    }
}
