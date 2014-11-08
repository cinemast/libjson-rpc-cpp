[![Build Status](https://travis-ci.org/cinemast/libjson-rpc-cpp.png?branch=master)](https://travis-ci.org/cinemast/libjson-rpc-cpp)

libjson-rpc-cpp
===============

This framework provides cross platform JSON-RPC (remote procedure call) support for C++.
It is fully JSON-RPC [2.0 & 1.0 compatible](http://www.jsonrpc.org/specification).

**Notice: This framework is currently in a beta phase. Bug reports as well as contributions are very welcome! Heavy API/ABI changes might occur regularly but you can always find stable versions in the [Releases section](https://github.com/cinemast/libjson-rpc-cpp/releases)**

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/artwork/logo.png?raw=true)

**5 good reasons for using libjson-rpc-cpp in your next RPC project**
- Full JSON-RPC 2.0 & 1.0 Client and Server Support.
- jsonrpcstub - a tool that generates stub-classes for your JSON-RPC client AND server applications.
- Ready to use HTTP server and client to provide simple interfaces for your JSON-RPC application.
- Cross platform build support and [precompiled binaries for WIN32]().
- Super liberal [MIT-License](http://en.wikipedia.org/wiki/MIT_License). 

**Other good reasons to use libjson-rpc-cpp**
- Easy to use [cmake](http://www.cmake.org) cross platform build system.
- Clean and simple architecture, which makes it easy to extend.
- Tested under MacOS X (10.9), Linux (Debian 8 64-bit), Windows 7 x64 and Raspbian Wheezy (armhf).
- Automated testing using `make test`.
- Useful Examples provided. e.g. XBMC Remote using json-rpc client part and stub generator.
- The stubgenerator currently supports C++ and JavaScript.

Overview
=========
![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/develop/dev/artwork/overview.png?raw=true)

Build the framework
===================

You will need [Git](http://git-scm.com/downloads) and [CMake](http://www.cmake.org/cmake/resources/software.html). You can click on the links to download the latest versions.

Installing the dependencies
---------------------------
**Debian based systems**
```sh
sudo apt-get install libcurl4-openssl-dev libmicrohttpd-dev libjsoncpp-dev libargtable2-dev cmake
```

**Arch Linux based systems**
For Arch Linux ther is a [PKGBUILD provided in the AUR](https://aur.archlinux.org/packages/libjson-rpc-cpp/). 
This already takes care of dependencies and the framework itself.

```sh
aura -As libjson-rpc-cpp
```

**Mac OS X**
You need [Brew](http://brew.sh) installed and type the following commands
```sh
brew install argtable cmake jsoncpp libmicrohttpd
```

**Windows**
- Download the precompiled dependencies form [here](https://spiessknafl.at/libjson-rpc-cpp/win32-deps.zip).
- Extract it into the cloned repository, so that there is a `win32-deps` folder in the root project directory.

Build and install this framework
--------------------------------

Open a terminal and copy the following commands:

```sh
git clone git://github.com/cinemast/libjson-rpc-cpp.git
mkdir -p libjson-rpc-cpp/build
cd libjson-rpc-cpp/build
cmake .. && make
sudo make install
sudo ldconfig          #only required for linux
```
That's it!

If you are not happy with it, simply uninstall it from your system using (inside the build the directory):
```sh
sudo make uninstall
```

**Build options:**

Default configuration should be fine for most systems, but here are available compilation flags:

- `-DCOMPILE_TESTS=NO` disables unit test suite.
- `-DCOMPILE_STUBGEN=NO` disables building the stubgenerator.
- `-DCOMPILE_EXAMPLES=NO` disables examples.
- `-DHTTP_SERVER_MONGOOSE=NO` disable the embedded mongoose webserver.
- `-DHTTP_CLIENT_CURL=NO` disable the curl client.
- `-DSOCKET_SERVER=YES` enable the socket server.
- `-DSOCKET_CLIENT=YES` enable the socket client.

Simple Example
==============
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

This generates a serverstub and a clientstub class.


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
g++ main.cpp -ljsonrpccpp-server -ljsoncpp -ljsonrpccpp-common -o sampleserver
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
g++ main.cpp -ljsonrpccpp-client -ljsoncpp -ljsonrpccpp-common -lcurl -o sampleclient
```

References
==========
- [NASA Ames Research Center](http://www.nasa.gov/centers/ames/home/): use it to obtain aircraft state information from an aircraft simulator.
- [LaseShark 3D Printer](https://github.com/macpod/lasershark_3dp): used to control the firmware of the 3D printer.
- [cpp-ethereum](https://github.com/ethereum/cpp-ethereum): a distributed computing framework.
- [mage-sdk-cpp](https://github.com/mage/mage-sdk-cpp): a game engine.
- [bitcodin](http://www.bitmovin.net): a scaleable cloud based video transcoding platform.
- [wgslib](http://wgslib.com/): A web geostatistics library.
- [bitcoin-api-cpp](https://github.com/minium/bitcoin-api-cpp): a C++ interface to bitcoin.

If you use this library and find it useful, I would be very pleased if you let me know about it.

Developer Information
======================

Roadmap for next release
------------------------
- Official win32/64 build support.
- Review socket connectors
- libmicrohttpd server connector (to replace mongoose, because of license issues)
- Generate client stubs for other languages.

Changelogs
----------
Changelogs can be found [here](https://github.com/cinemast/libjson-rpc-cpp/blob/master/CHANGELOG.md).

Licsense
--------
This framework is licensed under [MIT](http://en.wikipedia.org/wiki/MIT_License). 
All of this libraries dependencies are licensed under MIT compatible licenses.

Dependencies
------------
- [jsoncpp](http://jsoncpp.sourceforge.net) (licensed under MIT)
jsoncpp is a very easy to use and powerful json library. 
It is used for all the JSON parsing and generation inside this library.
- [libmicrohttpd]http://www.gnu.org/software/libmicrohttpd/) (licensed under LGPL)
small gnu http server implementation.
- [curl](http://curl.haxx.se)
lib curl is used for the HttpClient connections.
- [argtable2](http://argtable.sourceforge.net/) (licensed under LGPL)
libargtable2 is used for handling commandline parameters of the jsonrpcstub tool.

Run the tests
-------------
For running the tests, the boost-test framework is required:

**Debian based systems**
```sh
cd build
sudo apt-get install libboost-test-dev
cmake .. && make test
```

**Mac OS X**
You need [Brew](http://brew.sh) installed and type the following commands
```sh
cd build
brew install boost
cmake .. && make test
```

Testcoverage can be retrieved by invoking the [dev/ci.sh script](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/testcoverage.sh).

