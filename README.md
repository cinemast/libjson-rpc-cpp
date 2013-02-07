libjson-rpc-cpp
===============

This C++ library provides a json-rpc (remote procedure call) framework for Windows, Linux and MacOS (or any other UNIX derivate).
It is fully JSON-RPC 2.0 compatible ([JSON-RPC 2.0](http://www.jsonrpc.org/specification)) and provides additional features, such as generic authentication mechanisms.

**Notice: This framework is currently in a beta phase. Bug Reports are very welcome!**

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/dev/artwork/logo.png?raw=true)

5 good reasons why you should use libjson-rpc-cpp in your next project:
---------
- Full JSON-RPC 2.0 Support (batch requests, parameter type and name checking, ...).
- Embedded HTTP server to provide simple interfaces for your json-rpc server application.
- Embedded HTTP client to connect easily via HTTP protocol.
- jsonrpcstub: Generates automatically C++ classes for your json-rpc client.
- Super liberal [MIT-License](http://en.wikipedia.org/wiki/MIT_License). 

**Other good reasons to use libjson-rpc-cpp**
- Easy to use.
- Method parameter names and checks can be easily configured through a simple json configuration file.
- Very verbose error reporting.
- Easy to use [cmake](http://www.cmake.org) cross platform build system.
- Clean and simple architecture.
- Authentication Interface: to restrict certain methods to certain user/groups only.
- Tested under Windows7, MacOS X (10.7) and Linux (Ubuntu 12.10).

 
Build the library
-------------

You will need [Git](http://git-scm.com/downloads) and [CMake](http://www.cmake.org/cmake/resources/software.html). You can click on the links to download the latest versions. 
CMake must be Version 2.6 or later.

Open a terminal and copy the following commands:

```sh
git clone git://github.com/cinemast/libjson-rpc-cpp.git
cd libjson-rpc-cpp/build
cmake .. && make
#Not required, but makes it easier to use
sudo make install
sudo ldconfig
```

That's all you need.

To uninstall the lib, type (inside the build directory):

```sh
sudo make uninstall
```

Examples
--------
- [A simple Server Example](https://github.com/cinemast/libjson-rpc-cpp/wiki/A-simple-Server-Example)
- [A simple Client Example](https://github.com/cinemast/libjson-rpc-cpp/wiki/A-simple-Client-example)

Roadmap
--------
- Provide .deb package
- Automated testing after build phase (maybe ctest)
- Get available at ubuntu mirrors.
- Documentation for extending libjson-rpc-cpp (implementing more connectors, authentication manager)
- Tutorial for AJAX-Example (it already works)

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
