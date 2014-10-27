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
- Due to a bug in gcc 4.6.2 this project is not compiling under Mac OS gcc. Use clang++ instead. [See here](http://stackoverflow.com/questions/8887864/template-base-constructor-call-in-member-initialization-list-error)
