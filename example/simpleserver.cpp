/**
 * @file main.cpp
 * @date 01.08.2011
 * @author Peter Spiess-Knafl
 * @brief main.cpp
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <jsonrpc/rpc.h>

using namespace jsonrpc;
using namespace std;

//method
void sayHello(const Json::Value& request, Json::Value& response)
{
    cout << "Requested say Hello" << endl;
    response = "Hello: " + request["name"].asString();
}

//notification
void notifyServer(const Json::Value& request)
{
    cout << "server received some Notification" << endl;
}

int main(int argc, char** argv)
{

    if(argc < 2)
    {
        cerr << "Missing method specification" << endl;
        cerr << "e.g. " << argv[0] << " " << "procedures.json" << endl;
        return -1;
    }
    else
    {
        methods_t procedurePointers;
        notifications_t notPointers;

        procedurePointers["sayHello"] = &sayHello;
        notPointers["notifyServer"] = &notifyServer;

        try
        {

            Server serv("samplejsonrpcserver", argv[1],
                    procedurePointers, notPointers, new HttpServer(8080));
            if (serv.StartListening())
            {
                cout << "Server started successfully" << endl;
                getchar();
                serv.StopListening();
            }
            else
            {
                cout << "Error starting Server" << endl;
            }
        }
        catch (jsonrpc::Exception& e)
        {
            cerr << e.what() << endl;
        }
    }
    //curl --data "{\"jsonrpc\":\"2.0\",\"method\":\"sayHello\",\"id\":1,\"params\":{\"name\":\"peter\"}}" localhost:8080
    //curl --data "{\"jsonrpc\":\"2.0\",\"method\":\"notifyServer\", \"params\": null}" localhost:8080
}
