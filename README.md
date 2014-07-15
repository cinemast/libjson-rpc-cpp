[![Build Status](https://travis-ci.org/cinemast/libjson-rpc-cpp.png?branch=master)](https://travis-ci.org/cinemast/libjson-rpc-cpp)

libjson-rpc-cpp
===============

This C++ library provides a json-rpc (remote procedure call) framework for Linux and MacOS (or any other UNIX derivate).
It is fully JSON-RPC 2.0 compatible ([JSON-RPC 2.0](http://www.jsonrpc.org/specification)) and provides additional features, such as generic authentication mechanisms.

**Notice: This framework is currently in a beta phase. Bug Reports are very welcome!**

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/artwork/logo.png?raw=true)

5 good reasons why you should use libjson-rpc-cpp in your next project:
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
- Authentication Interface: to restrict certain methods to certain user/groups only.
- Tested under MacOS X (10.7,10.8), Linux (Ubuntu 12.10 64bit, LinuxMint 14 64bit).
- Tested under RaspberryPi (raspbian). This library offers great opportunities to remotely control your raspberry pi.
- Automated testing using `make test`
- Useful Examples provided. g.g. XBMC Remote using json-rpc client part and stub generator.
 
Build the library
-------------

You will need [Git](http://git-scm.com/downloads) and [CMake](http://www.cmake.org/cmake/resources/software.html). You can click on the links to download the latest versions. [libcurl](http://curl.haxx.se/libcurl/) is also required but should already be installed on most systems.
CMake must be Version 2.6 or later.

Under Debian based systems type:

```sh
sudo apt-get install libcurl4-openssl-dev cmake
```

Open a terminal and copy the following commands:

```sh
git clone git://github.com/cinemast/libjson-rpc-cpp.git
cd libjson-rpc-cpp/build
cmake .. && make
#Not required, but makes it easier to use
sudo make install
#only required for linux
sudo ldconfig	
```
To uninstall the library and the stubgenerator, type:

```sh
cd build && sudo make uninstall
```

That's all you need.


Under CentOS based (automake) systems type:

```sh
sudo yum install automake autoconf libtool libcurl-devel
```

Open a terminal and copy the following commands:

```sh
git clone git://github.com/cinemast/libjson-rpc-cpp.git
cd libjson-rpc-cpp/
autoreconf -if
cd build
../configure
make
#Not required, but makes it easier to use
sudo make install
#only required for linux
sudo ldconfig	
```
To build RPM package for Fedora/Redhat/CentOS systems type:

```sh
sudo yum install rpm-build
cd libjson-rpc-cpp/build
../configure
make rpm
```
Your packages will be available under libjson-rpc-cpp/build/RPMBUILD/RPMS folder

Simple Example
--------
This example will show the most simple way to create a rpc server and client. If you only need the server, ignore step 4. If you only need the client, ignore step 3. You can find all resources of this sample in the `src/example` directory of this repository.

### Step 1: Writing the specification file ###

```json
[
	{
		"method": "sayHello",
		"params": { 
			"name": "Peter"
		},
		"returns" : "Hello Peter"
	},
	{
		"notification" : "notifyServer",
		"params": null
	}
]
```

The type of a return value or parameter is defined by the literal assigned to it. In this example you can see how to specify methods and notifications.

### Step 2: Generate the stubs for client and server ###

Call jsonrpcstub:
```sh
jsonrpcstub -s -c -o /Users/cinemast/Desktop spec.json MyStub
```

This generates the server (-s) and the client (-c) stub to the specified output directory (-o). `spec.json` is the specification file. `MyStub` defines the name of the stub classes.

You should see the following on your command line: 

```sh
Client Stub genearted to: /Users/cinemast/Desktop/MyStubClient.h
Server Stub genearted to: /Users/cinemast/Desktop/AbstractMyStubServer.h
```

### Step 3: implement the abstract server stub ###

Extend the abstract server stub and implement all pure virtual (abstract) methods defined in `spec.json`.

```cpp
#include <jsonrpc/rpc.h>
#include <iostream>

#include "abstractmystubserver.h"

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractMyStubServer
{
    public:
        MyStubServer();

        virtual void notifyServer();
        virtual std::string sayHello(const std::string& name);
};

MyStubServer::MyStubServer() :
    AbstractMyStubServer(new HttpServer(8080))
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
    MyStubServer s;
    s.StartListening();
    getchar();
    s.StopListening();
    return 0;
}
```

In the main function the concrete server is instantiated and started. That is all for the server. Any JSON-RPC 2.0 compliant client can now connect to your server.

Compile the server with:

```sh
g++ main.cpp -ljsonrpc -o sampleserver
```

### Step 4: Create the client application
```cpp
#include <jsonrpc/rpc.h>
#include <iostream>

#include "mystubclient.h"

using namespace jsonrpc;
using namespace std;

int main()
{
    HttpClient* httpClient = new HttpClient("http://localhost:8080");
    MyStubClient c(httpClient);
    try
    {
        cout << c.sayHello("Peter Knafl") << endl;
        c.notifyServer();
    }
    catch (JsonRpcException e)
    {
        cerr << e.what() << endl;
    }
    delete httpClient;
    return 0;
}
```

Compile the client with:

```sh
g++ main.cpp -ljsonrpc -o sampleclient
```

References
-------------
- [NASA Ames Research Center](http://www.nasa.gov/centers/ames/home/): used it to obtain aircraft state information from an aircraft simulator.
- [LaseShark 3D Printer](https://github.com/macpod/lasershark_3dp): used to control the firmware of the 3D printer.
- [cpp-ethereum](https://github.com/ethereum/cpp-ethereum): a distributed computing framework.
- [mage-sdk-cpp](https://github.com/mage/mage-sdk-cpp): a game engine.
- [bitcodin](http://www.bitmovin.net): a scaleable cloud based video transcoding platform.

If you use this library and find it useful, I would be very pleased if you let me know about it.

Roadmap for v0.3
--------
- Provide .deb package
- Documentation for extending libjson-rpc-cpp (implementing more connectors, authentication manager)

Changes in v0.2.1
---------

- Added support for positional parameters. (see at [example specification](https://github.com/cinemast/libjson-rpc-cpp/blob/master/src/example/spec.json) how to declare them)

Changes in v0.2
---------
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
Copyright (C) 2011-2013 Peter Spiess-Knafl

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
