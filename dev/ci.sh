#!/bin/bash
set -e

build_configuration() {
    echo "Building configuration $1"
    mkdir -p build
    cd build
    cmake $1 ..
    make
    make test
    sudo make install
    sudo ldconfig
}

cleanup() {
    sudo make uninstall
    cd ..
    rm -rf build
}

build_configuration "-DSOCKET_SERVER=NO -DSOCKET_CLIENT=NO -DHTTP_SERVER=NO -DHTTP_CLIENT=NO -DCOMPILE_STUBGEN=NO"
cleanup

build_configuration "-DSOCKET_SERVER=YES -DSOCKET_CLIENT=YES -DHTTP_SERVER=NO -DHTTP_CLIENT=NO -DCOMPILE_STUBGEN=NO"
cleanup

build_configuration "-DSOCKET_SERVER=YES -DSOCKET_CLIENT=YES -DHTTP_SERVER=YES -DHTTP_CLIENT=YES -DCOMPILE_STUBGEN=YES"
g++ ../src/examples/simpleclient.cpp -ljsonrpccpp-client -ljsoncpp -ljsonrpccpp-common -lcurl -o sampleclient 
g++ ../src/examples/simpleserver.cpp -ljsonrpccpp-server -ljsoncpp -ljsonrpccpp-common -lmicrohttpd -o sampleserver
test -f sampleclient
test -f sampleserver
cleanup

echo "Integration successful"
