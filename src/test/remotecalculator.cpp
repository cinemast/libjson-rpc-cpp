/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    remotecalculator.cpp
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
        v["value1"] = 1432;
        v["value2"] = 556;

        Json::Value result = client->CallMethod("add", v);
        if(result.asInt() != 1988) {
            cerr << "add returned " << result.asInt() << " but should be 1988" << endl;
            return -1;
        }

        v["value1"] = -1432;
        v["value2"] = 556;

        result = client->CallMethod("add", v);
        if(result.asInt() != -876) {
            cerr << "add returned " << result.asInt() << " but should be -876" << endl;
            return -2;
        }

        v["value1"] = -1432;
        v["value2"] = 556;

        result = client->CallMethod("sub", v);
        if(result.asInt() != -1988) {
            cerr << "sub returned " << result.asInt() << " but should be -1988" << endl;
            return -3;
        }

        v["value1"] = 1432;
        v["value2"] = 556;

        result = client->CallMethod("sub", v);
        if(result.asInt() != 876) {
            cerr << "sub returned " << result.asInt() << " but should be 876" << endl;
            return -4;
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
