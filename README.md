libjson-rpc-cpp
===============

This C++ library provides a json-rpc (remote procedure call) framework for Windows, Linux and MacOS (or any other UNIX derivate).
It is fully JSON-RPC 2.0 compatible ([JSON-RPC 2.0](http://www.jsonrpc.org/specification)) and provides additional features, such as generic authentication mechanisms.

![libjson-rpc-cpp logo](https://github.com/cinemast/libjson-rpc-cpp/blob/master/artwork/logo.png?raw=true)

Features
---------
- Full JSON-RPC 2.0 Support (batch requests, parameter type and name checking, ...).
- Authentication support: to restrict certain methods to certain user/groups only.
- Easy to use.
- Method parameter names and checks can be easily configured through a simple json configuration file.
- Very verbose error reporting.
- Embedded HTTP server to provide simple interfaces for your json-rpc server application.
- Using [cmake](http://www.cmake.org) cross platform build system.
- Tested under Windows7, MacOS X (10.7) and Linux (Ubuntu 12.10).
- Super liberal [MIT-License](http://en.wikipedia.org/wiki/MIT_License). 
 
Compile & Run
-------------

....

Examples
--------

....

Licsense
--------

This framework is licensed under [MIT](http://en.wikipedia.org/wiki/MIT_License).

Used Libraries
---------------

- [jsoncpp](http://jsoncpp.sourceforge.net) (licensed under MIT)
jsoncpp is a very easy to use and powerful json library. 
It is used for all the JSON parsing and generation inside this library.
- [mongoose](https://github.com/valenok/mongoose) (licensed under MIT)
mongoose is a http server that can be easily embedded into other applications. 
It is used here for the HttpConnector to provide HTTP json-rpc Requests.

Thank you Baptiste Lepilleur and Sergey Lyubka for providing these libraries.
