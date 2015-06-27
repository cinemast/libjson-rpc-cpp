Changes in v0.6.0
-----------------
- NEW: pkg-config files for all shared libraries
- NEW: UNIX Socket client + server connector
- NEW: multiarch support
- Change: unit testing framework to catch
- Change: allow disabling shared library build
- Change: split out shared/static library for stubgenerator

Changes in v0.5.0
-----------------
- Added `--version` option to jsonrpcstub.
- Added msvc support.
- Added data field support for JsonRpcException.
- Added contributions guide: https://github.com/cinemast/libjson-rpc-cpp#contributions
- HttpClient uses Http Keep-Alive, which improves performance drastically.
- Added multiarch support.
- Fixed building tests with examples disabled.
- Made static library build optional (via `BUILD_STATIC_LIBS`).
- Fixed unnecessary rebuilds of stubs on each `make` call.

Changes in v0.4.2
-----------------
- Fix of spelling mistakes.
- Use CMAKE versioning in manpage.
- Improving include scheme of jsoncpp.
- Bugfix in HttpServer with Threading option in SSL startup.

Changes in v0.4.1
-----------------
- Added coverity scan support
- [Added API compatibility report](http://upstream.rosalinux.ru/versions/libjson-rpc-cpp.html)
- Improved manpage
- Extended Stubgenerator for protocol switches (JSON-RPC 1.0 & 2.0)

Changes in v0.4
---------------
- Memory leak fixes
- Switched Http Server to libmicrohttpd
- Added full WIN32 build support
- Improved test coverage (100% line coverage)
- Added JavaScript client stub support
- Removed TCP Client/Server implementation due to security and codestyle problems.
- Finally removed dirty pointer stuff in bindAndAddX() methods.
- Using call by value in generated stubs for primitive data types.

Changes in v0.3.2
-----------------
- Bugfixes
- Additional testcases for client + server -> higher testcoverage
- JSON-RPC 1 Client + Server support
- Refactorings in server for JSON-RPC 1 support
- Hiding irrelevant API headers from installation
- Renamed AbstractClientConnector to IClientConnector (please regenearte your client stubs after upgrading)
- Reactivated dev/testcoverage.sh to measure testcoverage.

Changes in v0.3.1
-----------------
- Bugfixes
- Changed SOVERSION
- Added experimental Javascript client to stubgenerator
- Adapted HTTP Server to enable CORS.

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
- Under Ubuntu 12.04 SSL support is not enabled in the libmicrohttpd-dev package that is provided by ubuntu. Look at the [.travis.yml](https://github.com/cinemast/libjson-rpc-cpp/blob/develop/.travis.yml) file to see how I cope with this.
