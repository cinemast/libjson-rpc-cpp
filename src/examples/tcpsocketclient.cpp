/**
 * @file tcpsocketclient.cpp
 * @date 17.07.2015
 * @author Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @brief tcpsocketclient.cpp
 */

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/tcpsocketclient.h>
#include <iostream>
#include <cstdlib>

using namespace jsonrpc;
using namespace std;

int main(int argc, char** argv)
{
        string ip;
        unsigned int port;

        if(argc == 3) {
            ip = string(argv[1]);
            port = atoi(argv[2]);
            cout << "Params are :" << endl;
            cout << "\t ip: " << ip << endl;
            cout << "\t port: " << port << endl;
        }
        else {
            ip = "127.0.0.1";
            port = 6543;
        }
        
	TcpSocketClient client(ip, port);
	Client c(client);

	Json::Value params;
	params["name"] = "Peter";

	try
	{
		cout << c.CallMethod("sayHello", params) << endl;
	}
	catch (JsonRpcException& e)
	{
		cerr << e.what() << endl;
	}


}