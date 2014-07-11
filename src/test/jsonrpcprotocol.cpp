/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    jsonrpcprotocol.cpp
 * @date    09.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <iostream>
#include "server.h"

using namespace jsonrpc;
using namespace std;

int validateRequest(Client* client, const std::string& method, Json::Value& param, bool isMethod)
{
    int error = 0;

    try
    {
        if(isMethod)
        {
            client->CallMethod(method, param);
        }
        else
        {
            client->CallNotification(method, param);
        }
    }
    catch (JsonRpcException e)
    {
        error = e.GetCode();
    }
    return error;
}

int main(int argc, char** argv)
{

    TestServer* server = new TestServer();
    HttpClient *httpClient = new HttpClient("http://localhost:8080");
    Client* client = new Client(httpClient);
    server->StartListening();

    Json::Value v;
    v["name"] = "Peter";
    if(validateRequest(client,"sayHello", v, true) != 0)
    {
        cerr << "Error field != 0" << endl;
        return -1;
    }

    if(validateRequest(client,"nonexistingMethod", v, true) != -32601)
    {
        cerr << "Wrong error code for nonexisting Method" << endl;
        return -2;
    }

    v.removeMember("name");
    v["aaawrwer"] = "asökfjölwer";
    if(validateRequest(client,"sayHello", v, true) != -32602)
    {
        cerr << "Wrong error code for invalid parameter" << endl;
        return -3;
    }

    if(validateRequest(client,"incrementCounter", v, true) != -32605)
    {
        cerr << "Wrong error code for method called as notification " << validateRequest(client,"sayHello", v, false) << endl;
        return -4;
    }


    delete server;
    delete client;
    delete httpClient;

    return 0;
}
