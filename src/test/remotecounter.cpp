/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    remotecounter.cpp
 * @date    08.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>

#include "server.h"

using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{

    TestServer* server = new TestServer();
    HttpClient *httpClient = new HttpClient("http://localhost:8080");
    Client* client = new Client(httpClient);

    try {
        server->StartListening();

        Json::Value v;
        v["value"] = 3;
        client->CallNotification("initCounter", v);

        Json::Value result = client->CallMethod("getCounterValue", Json::nullValue);
        if(result.asInt() != 3) {
            cerr << "getCounterValue returned " << result.asInt() << " but should be 3" << endl;
            return -1;
        }

        client->CallNotification("incrementCounter", v);
        result = client->CallMethod("getCounterValue", Json::nullValue);
        if(result.asInt() != 4) {
            cerr << "getCounterValue returned " << result.asInt() << " but should be 4" << endl;
            return -2;
        }

        delete server;
        delete client;
        delete httpClient;

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
