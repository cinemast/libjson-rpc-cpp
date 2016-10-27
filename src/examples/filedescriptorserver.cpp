/**
 * @file filedescriptorserver.cpp
 * @date 11.05.2015
 * @author Alexandre Poirot
 * @brief An example implementation of a server based on plain old file descriptors
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/filedescriptorserver.h>
#include <unistd.h>


#define DELIMITER_CHAR char(0x0A)

using namespace jsonrpc;
using namespace std;

class SampleServer : public AbstractServer<SampleServer>
{
public:
  SampleServer(FileDescriptorServer &server) :
    AbstractServer<SampleServer>(server)
  {
    this->bindAndAddMethod(Procedure("sayHello", PARAMS_BY_NAME, JSON_STRING, "name", JSON_STRING, NULL), &SampleServer::sayHello);
    this->bindAndAddNotification(Procedure("notifyServer", PARAMS_BY_NAME, NULL), &SampleServer::notifyServer);
  }

  //method
  void sayHello(const Json::Value& request, Json::Value& response)
  {
    response = "Hello: " + request["name"].asString();
  }

  //notification
  void notifyServer(const Json::Value& request)
  {
    (void)request;
    exit(0);
  }
};

int main()
{
  try
  {
    FileDescriptorServer server(STDIN_FILENO, STDOUT_FILENO);
    SampleServer serv(server);

    if (serv.StartListening())
    {
      cerr << "Server started successfully" << endl;
      server.Wait();
    }
    else
    {
      cerr << "Error starting Server" << endl;
    }
  }
  catch (jsonrpc::JsonRpcException& e)
  {
    cerr << e.what() << endl;
  }
}
