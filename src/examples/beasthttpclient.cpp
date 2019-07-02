/**
 * @file beastsimpleclient.cpp
 * @date 02.02.2019
 * @author Javier Gutierrez (Github: mr_j0nes)
 * @brief This is a simple client example.
 */

#include <iostream>
#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/beasthttpclient.h>

using namespace jsonrpc;
using namespace std;

int main() {
  BeastHttpClient client("localhost", 8383);
  Client c(client);

  Json::Value params;
  params["name"] = "Peter";

  try {
    cout << c.CallMethod("sayHello", params) << endl;
  } catch (JsonRpcException &e) {
    cerr << e.what() << endl;
  }
}
