/**
 * @file filedescriptorclient.cpp
 * @date 27.10.2016
 * @author Jean-Daniel Michaud <jean.daniel,michaud@gmail.com>
 * @brief An example implementation of a client based on plain old file descriptors
 */

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/filedescriptorclient.h>
#include <iostream>
#include <unistd.h>

using namespace jsonrpc;
using namespace std;

int main()
{
  FileDescriptorClient client(STDIN_FILENO, STDOUT_FILENO);
  Client c(client);

  Json::Value params;
  params["name"] = "Peter";

  try
  {
    cerr << "client:" << c.CallMethod("sayHello", params) << endl;
  }
  catch (JsonRpcException& e)
  {
    cerr << e.what() << endl;
  }
}
