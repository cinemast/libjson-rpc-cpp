[![Build Status](https://travis-ci.org/cinemast/libjson-rpc-cpp.png?branch=master)](https://travis-ci.org/cinemast/libjson-rpc-cpp)

libjson-rpc-cpp
===============

This C++ library provides a json-rpc (remote procedure call) framework for Linux and MacOS (or any other UNIX derivate).
It is fully JSON-RPC 2.0 compatible ([JSON-RPC 2.0](http://www.jsonrpc.org/specification)) and provides additional features, such as generic authentication mechanisms.

**Notice: This framework is currently in a beta phase. Bug Reports are very welcome!**

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/artwork/logo.png?raw=true)

5 good reasons for using libjson-rpc-cpp in your next RPC project:
---------
- Full JSON-RPC 2.0 Support (batch requests, parameter type and name checking, ...).
- jsonrpcstub: Generates automatically C++ stub-classes for your json-rpc client AND server.
- Embedded HTTP server to provide simple interfaces for your json-rpc server application.
- Embedded HTTP client to connect easily via HTTP protocol.
- Super liberal [MIT-License](http://en.wikipedia.org/wiki/MIT_License). 

**Other good reasons to use libjson-rpc-cpp**
- Easy to use.
- Method parameter names and checks can be easily configured through a simple json configuration file.
- Very verbose error reporting.
- Easy to use [cmake](http://www.cmake.org) cross platform build system.
- Clean and simple architecture.
- Tested under MacOS X (10.7,10.8), Linux (Debian 8 64-bit).
- Tested under RaspberryPi (raspbian). This library offers great opportunities to remotely control your raspberry pi.
- Automated testing using `make test`
- Useful Examples provided. e.g. XBMC Remote using json-rpc client part and stub generator.
 
Build the framework
------------------

You will need [Git](http://git-scm.com/downloads) and [CMake](http://www.cmake.org/cmake/resources/software.html). You can click on the links to download the latest versions. [libcurl](http://curl.haxx.se/libcurl/) is also required but should already be installed on most systems.
CMake must be Version 2.6 or later.

Under Debian based systems type:

```sh
sudo apt-get install libcurl4-openssl-dev cmake
```

Open a terminal and copy the following commands:

```sh
git clone git://github.com/cinemast/libjson-rpc-cpp.git
mkdir -p libjson-rpc-cpp/build
cd libjson-rpc-cpp/build
cmake .. && make
sudo make install 	#Not required, but makes it easier to use
sudo ldconfig		#only required for linux
```
That's it!

If you are not happy with it, simply uninstall it from your system using (inside the build the directory):
```sh
sudo make uninstall
```
**Build options:**
- `-DCOMPILE_TESTS=NO` disables unit test suite.
- `-DCOMPILE_STUBGEN=NO` disables building the stubgenerator.
- `-DCOMPILE_EXAMPLES=NO` disables examples.
- `-DHTTP_SERVER_MONGOOSE=NO` disable the embedded mongoose webserver.
- `-DHTTP_CLIENT_CURL=NO` disable the curl client.
- `-DSOCKET_SERVER=YES` enable the socket server.
- `-DSOCKET_CLIENT=YES` enable the socket client.

Simple Example
---------------
This example will show the most simple way to create a rpc server and client. If you only need the server, ignore step 4. If you only need the client, ignore step 3. You can find all resources of this sample in the `src/examples` directory of this repository.

### Step 1: Writing the specification file ###

```json
[
	{
		"name": "sayHello",
		"params": { 
			"name": "Peter"
		},
		"returns" : "Hello Peter"
	},
	{
		"name" : "notifyServer",
		"params": null
	}
]
```

The type of a return value or parameter is defined by the literal assigned to it. In this example you can see how to specify methods and notifications.

### Step 2: Generate the stubs for client and server ###

Call jsonrpcstub:
```sh
jsonrpcstub spec.json --cpp-server=AbstractStubServer --cpp-client=StubClient
```

This generates the abstract server (`--server`) and the C++ client (`--cpp`) stub. `spec.json` is the specification file we have defined in Step 1. `--class` defines the class name of the stub. This can also be a namespaced classname like `myapplication::MyStub`.


### Step 3: implement the abstract server stub ###

Extend the abstract server stub and implement all pure virtual (abstract) methods defined in `spec.json`.

```cpp
#include "abstractsubserver.h"
#include <jsonrpccpp/server/connectors/httpserver.h>

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractStubServer
{
    public:
        MyStubServer(AbstractServerConnector &connector);

        virtual void notifyServer();
        virtual std::string sayHello(const std::string& name);
};

MyStubServer::MyStubServer(AbstractServerConnector &connector) :
    AbstractStubServer(connector)
{
}
void MyStubServer::notifyServer()
{
    cout << "Server got notified" << endl;
}
string MyStubServer::sayHello(const string &name)
{
    return "Hello " + name;
}

int main()
{
    HttpServer httpserver(8383);
    MyStubServer s(httpserver);
    s.StartListening();
    getchar();
    s.StopListening();
    return 0;
}
```

In the main function the concrete server is instantiated and started. That is all for the server. Any JSON-RPC 2.0 compliant client can now connect to your server.

Compile the server with:

```sh
g++ main.cpp -ljsonrpccppserver -o sampleserver
```

### Step 4: Create the client application
```cpp
#include <iostream>

#include "stubclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace jsonrpc;
using namespace std;

int main()
{
    HttpClient httpclient("http://localhost:8383");
    StubClient c(httpclient);
    try
    {
        cout << c.sayHello("Peter Knafl") << endl;
        c.notifyServer();
    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }
}
```

Compile the client with:

```sh
g++ main.cpp -ljsonrpccppclient -o sampleclient
```

References
-------------
- [NASA Ames Research Center](http://www.nasa.gov/centers/ames/home/): used it to obtain aircraft state information from an aircraft simulator.
- [LaseShark 3D Printer](https://github.com/macpod/lasershark_3dp): used to control the firmware of the 3D printer.
- [cpp-ethereum](https://github.com/ethereum/cpp-ethereum): a distributed computing framework.
- [mage-sdk-cpp](https://github.com/mage/mage-sdk-cpp): a game engine.
- [bitcodin](http://www.bitmovin.net): a scaleable cloud based video transcoding platform.

If you use this library and find it useful, I would be very pleased if you let me know about it.

Roadmap for next release
------------------------
- JSON RPC 1 legacy support
- libmicrohttpd server connector (to replace mongoose, because of license issues)
- Generate client stubs for other languages.

Changes in v0.3
---------------
- Split up server and client into separate libraries
- Lot's of refactorings in the build system and stubgenerator.
- Added namespace/package support for generated stub classes.
- libjson-cpp is no longer directly embedded.
- Simplified spec format: a procedure specification without `return` field is a notification.
- Introduced a boost-test based unit testing suite, which makes testing more flexible.
- Added CMake options to enable/disable Stubgenerator, Examples, Connectors and Testsuite.
- Removed Autotools support (because of all the changes in this release).
- Bugfix: renamed .so files to avoid collisions with makerbot's libjsonrpc.
- Bugfix: Invalid Batchcalls in Client and Server caused runtime exceptions.


Changes in v0.2.1
-----------------
- Added support for positional parameters. (see at [example specification](https://github.com/cinemast/libjson-rpc-cpp/blob/master/src/example/spec.json) how to declare them)

Changes in v0.2
---------------
- Minor bugfixes.
- Refactored architecture.
- stub generator for client and server.
- removed mandatory configuration files (making it more compatible for embedded use cases).
- Introduced SpecificationWriter to generate Specifications from RPC-Server definitions.
- Introduced SpecificationParser to parse a Specification file and generate Methods for the RPC-Server.
- Updated JsonCPP library
- Update Mongoose library
- Enable SSL Support (provided by mongoose)
- Introduced automated testing after build phase (using `make test`)
- Embedding dependent libraries (to avoid naming conflicts)

Known issues
-------------
- Due to a bug in gcc 4.6.2 this project is not compiling under Mac OS gcc. Use clang++ instead. [See here](http://stackoverflow.com/questions/8887864/template-base-constructor-call-in-member-initialization-list-error)

Licsense
--------

This framework is licensed under [MIT](http://en.wikipedia.org/wiki/MIT_License).

```
Copyright (C) 2011-2014 Peter Spiess-Knafl

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
OR OTHER DEALINGS IN THE SOFTWARE.
```

Used Libraries
---------------

- [jsoncpp](http://jsoncpp.sourceforge.net) (licensed under MIT)
jsoncpp is a very easy to use and powerful json library. 
It is used for all the JSON parsing and generation inside this library.
- [mongoose](https://github.com/valenok/mongoose) (licensed under MIT)
mongoose is a http server that can be easily embedded into other applications. 
It is used here for the HttpConnector to provide HTTP json-rpc Requests.
- [curl](http://curl.haxx.se)
lib curl is used for the HttpClient connections.

Thanks go to **Baptiste Lepilleur** and **Sergey Lyubka** for providing jsoncpp and mongoose.
