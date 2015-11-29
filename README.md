[![Build Status](https://travis-ci.org/cinemast/libjson-rpc-cpp.png?branch=master)](https://travis-ci.org/cinemast/libjson-rpc-cpp) [![Coverage Status](https://coveralls.io/repos/cinemast/libjson-rpc-cpp/badge.png?branch=develop)](https://coveralls.io/r/cinemast/libjson-rpc-cpp?branch=develop) [![Coverity Status](https://scan.coverity.com/projects/3169/badge.svg?flat=1)](https://scan.coverity.com/projects/3169)

libjson-rpc-cpp
===============

This framework provides cross platform JSON-RPC (remote procedure call) support for C++.
It is fully JSON-RPC [2.0 & 1.0 compatible](http://www.jsonrpc.org/specification).

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/artwork/logo.png?raw=true)

**5 good reasons for using libjson-rpc-cpp in your next RPC project**
- Full JSON-RPC 2.0 & 1.0 Client and Server Support.
- jsonrpcstub - a tool that generates stub-classes for your JSON-RPC client AND server applications.
- Ready to use HTTP server and client to provide simple interfaces for your JSON-RPC application.
- Cross platform build support and [precompiled binaries for WIN32](http://spiessknafl.at/libjson-rpc-cpp).
- Super liberal [MIT-License](http://en.wikipedia.org/wiki/MIT_License). 

**Other good reasons to use libjson-rpc-cpp**
- Easy to use [cmake](http://www.cmake.org) cross platform build system.
- Clean and simple architecture, which makes it easy to extend.
- Tested under MacOS X (10.9), GNU/Linux (Debian 8 64-bit), Windows 7 (MinGW32) and Raspbian Wheezy (armhf).
- Automated testing using `make test`.
- Useful Examples provided. e.g. XBMC Remote using json-rpc client part and stub generator.
- The stubgenerator currently supports C++ and JavaScript.

Overview
=========
![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/develop/dev/artwork/overview.png?raw=true)

Install the framework
=====================

**Debian**

```sh
sudo apt-get install libjsonrpccpp-dev libjsonrpccpp-tools
```

**Arch Linux**

For Arch Linux there is a [PKGBUILD provided in the AUR](https://aur.archlinux.org/packages/libjson-rpc-cpp/).

```sh
sudo aura -A libjson-rpc-cpp
```

**Gentoo Linux**

```sh
sudo emerge dev-cpp/libjson-rpc-cpp
```

**Mac OS X**

For OS X a [Brew](http://brew.sh) package is available:
```sh
brew install libjson-rpc-cpp
```

**Windows**

There is a ready to use compiled package [here](http://spiessknafl.at/libjson-rpc-cpp).
Just download execute the installer EXE.

Build from source
=================
Install the dependencies
------------------------
- [libcurl](http://curl.haxx.se/)
- [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)
- [libjsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [libargtable](http://argtable.sourceforge.net/)
- [cmake](http://www.cmake.org/)

**UNIX**

For Debian and Arch GNU/Linux based systems, all dependencies are available via the package manager.
For OS X all dependencies are available in [Brew](http://brew.sh)

**Windows**

- Download the precompiled dependencies form [here](https://spiessknafl.at/libjson-rpc-cpp/libjson-rpc-cpp_win32-deps.zip).
- Extract it into the cloned repository, so that there is a `win32-deps` folder in the root project directory.

Build
-----

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
- `-DHTTP_SERVER=NO` disable the libmicrohttpd webserver.
- `-DHTTP_CLIENT=NO` disable the curl client.
- `-DUNIX_DOMAIN_SOCKET_SERVER=NO` disable the unix domain socket server connector.
- `-DUNIX_DOMAIN_SOCKET_CLIENT=NO` disable the unix domain socket client connector.

Using the framework
===================
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
		"name" : "notifyServer"
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
#include "abstractstubserver.h"
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
g++ main.cpp -ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server -o sampleserver
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
        cout << c.sayHello("Peter") << endl;
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
g++ main.cpp -ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client -o sampleclient
```

References
==========
- [NASA Ames Research Center](http://www.nasa.gov/centers/ames/home/): use it to obtain aircraft state information from an aircraft simulator.
- [LaseShark 3D Printer](https://github.com/macpod/lasershark_3dp): used to control the firmware of the 3D printer.
- [cpp-ethereum](https://github.com/ethereum/cpp-ethereum): a distributed computing framework.
- [mage-sdk-cpp](https://github.com/mage/mage-sdk-cpp): a game engine.
- [bitcodin](http://www.bitmovin.net): a scalable cloud based video transcoding platform.
- [wgslib](http://wgslib.com/): A web geostatistics library.
- [bitcoin-api-cpp](https://github.com/minium/bitcoin-api-cpp): a C++ interface to bitcoin.
- [NIT DASH Content Server](http://www.nit.eu/offer/research-projects-products/334-http2dash): Dynamic Adaptive Streaming over HTTP server.

If you use this library and find it useful, I would be very pleased if you let me know about it.

Developer Information
=====================
Contributions
-------------
Contributions of any kind are always very welcome. 
Here are some suggestions:
- Bugreports
- Bugfixes
- Extending documentation (especially doxygen)
- Suggestion of new features
- New features:
  - Adding new connectors.
  - Adding new languages to the stubgenerator.

**Guidelines / Conventions**

We do not want to prevent you from contributing by having too strict guidelines.
If you have ideas for improvement, just do it your way, rather than doing it not at all.

Anyway here is a list of how we would prefer your contributions:
- Issues:
  - Use the issue tracker on github to report bugs or improvements.
  - Please avoid sending me mails directly, as this is not visible to others.
  - Please close issues on your own if you think a problem has been dealt with.
- Code contributions:
  - Please raise a pull-request against the develop branch.
  - If you add features, please keep the test-coverage at 100% and document them (doxygen, manpage, etc.).
  - If you fix a bug, please refer the issue in the [commit message](https://help.github.com/articles/closing-issues-via-commit-messages/).
  - Please make sure that the travis-ci build passes (you will get notified if you raise a pull-request).
  - Add yourself to the AUTHORS.md.
  - Use 4 spaces instead of tabs.

Mailing list
------------
[libjsonrpccpp-devel@lists.sourceforge.net](https://lists.sourceforge.net/lists/listinfo/libjsonrpccpp-devel)

Roadmap for next release
------------------------
- Generate client stubs for other languages.
- Extend doxygen documentation.

Changelogs
----------
Changelogs can be found [here](https://github.com/cinemast/libjson-rpc-cpp/blob/master/CHANGELOG.md).

API compatibility
-----------------
We do our best to keep the API/ABI stable, to prevent problems when updating this framework.
A compatiblity report can be found [here](http://upstream.rosalinux.ru/versions/libjson-rpc-cpp.html).

License
-------
This framework is licensed under [MIT](http://en.wikipedia.org/wiki/MIT_License). 
All of this libraries dependencies are licensed under MIT compatible licenses.

Documentation
-------------

The documentation for this library can be generated using doxygen.
If it is installed on your system, you can simply type:

```sh
cd build
make doc
```

This generates the Latex and HTML documentation into `build/doc`

Run the tests
-------------
Simply run: 

```sh
make test
```

Testcoverage can be retrieved by invoking the [dev/testcoverage.sh script](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/testcoverage.sh) inside the `dev` folder.
