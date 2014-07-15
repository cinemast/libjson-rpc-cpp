/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    errorhandling.cpp
 * @date    09.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <iostream>
#include "server.h"

using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{
    TestServer* server = new TestServer();
    TestServer* server2 = new TestServer();

    //Server Startup and shutdown tests
    if(!server->StartListening())
    {
        cerr << "Could not start server for the first time" << endl;
        return -1;
    }

    if(!server->StartListening())
    {
        cerr << "Server could not be started a second time" << endl;
        return -2;
    }


    if(!server->StopListening())
    {
        cerr << "Server could not be stopped" << endl;
        return -3;
    }



    if(!server->StopListening())
    {
        cerr << "Server could not be stopped a second time" << endl;
        return -4;
    }

    server->StartListening();
    if(server2->StartListening())
    {
        cerr << "The second server should not get started on the same port" << endl;
        return -5;
    }

    server->StopListening();

    //Client startup and shutdown tests
    HttpClient *httpClient = new HttpClient("http://localhost:8080");
    Client* client = new Client(httpClient);

    int error = 0;
    try
    {
        client->CallNotification("initCounter", Json::nullValue);
    }
    catch(JsonRpcException e)
    {
        error = e.GetCode();
    }

    delete client;
    delete httpClient;

    if(error != -32003)
    {
        return -6;
    }

    return 0;
}
