/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    helloworld.cpp
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
    Server* server;
    Client* client;

    try {

        server = getTestServer();
        client = getTestClient();

        server->StartListening();

        Json::Value v;
        v["name"] = "Peter";
        Json::Value result = client->CallMethod("sayHello", v);

        if(result.asString() != "Hello: Peter!") {
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

        cout << argv[0] << " passed" << endl;

        return 0;

    } catch(jsonrpc::Exception e) {

        cerr << "Exception occured: " << e.what() << endl;
        delete server;
        delete client;
        return -999;
    }
}
