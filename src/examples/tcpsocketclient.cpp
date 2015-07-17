/**
 * @file tcpsocketclient.cpp
 * @date 17.07.2015
 * @author Alexandre Poirot <alexandre.poirot@legrand.fr>
 * @brief tcpsocketclient.cpp
 */

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/tcpsocketclient.h>
#include <iostream>

using namespace jsonrpc;
using namespace std;

int main()
{
	TcpSocketClient client("127.0.0.1", 6543);
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
