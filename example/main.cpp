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

void sayHello(const Json::Value& request, Json::Value& response) {
	response = "Hello: " + request["name"].asString();
}

void notifyServer(const Json::Value& request) {
	cout << "server received some Notification" << endl;
}

int main() {
	methods_t procedurePointers;
	notifications_t notPointers;

	procedurePointers["sayHello"] = &sayHello;
	notPointers["notifyServer"] = &notifyServer;



	Server serv("A Server Instancename", "res/procedures.json", procedurePointers, notPointers, new HttpServer(8080,"./res"));
	if(serv.StartListening()) {
		cout << "Server started successfully" << endl;
		getchar();
		serv.StopListening();
	} else {
		cout << "Error starting Server" << endl;
	}


	//"{\"jsonrpc\":2.0,\"method\":\"sayHello\",\"id\":1,\"params\":{\"name\":\"peter\"}}"

}
