#!/bin/bash
set -e

build_configuration() {
    echo "Building configuration $1"
    mkdir -p build
    cd build
    cmake $1 ..
    make
    make test
    mkdir -p root
    make DESTDIR=root install
    cd ..
}

cleanup() {
    cd build
    make DESTDIR=root uninstall
    make clean
    rm sampleclient
    rm sampleserver
    cd ..
#    rm -rf build
}

build_configuration "-DCMAKE_BUILD_TYPE=Debug -DSOCKET_SERVER=YES -DSOCKET_CLIENT=YES -DHTTP_SERVER=YES -DHTTP_CLIENT=YES -DCOMPILE_STUBGEN=YES"

echo "Compiling examples"
cd build
g++ ../src/examples/simpleclient.cpp -Iroot/usr/local/include -Lroot/usr/local/lib -ljsonrpccpp-client -ljsoncpp -ljsonrpccpp-common -lcurl -o sampleclient 
g++ ../src/examples/simpleserver.cpp -Iroot/usr/local/include -Lroot/usr/local/lib -ljsonrpccpp-server -ljsoncpp -ljsonrpccpp-common -lmicrohttpd -o sampleserver
test -f sampleclient
test -f sampleserver

mkdir -p reports

echo "Generating valgrind report"
valgrind --leak-check=full --xml=yes --xml-file=reports/valgrind.xml ./bin/unit_testsuite --log_format=XML --log_sink=reports/tests.xml --log_level=all --report_level=no 

echo "Generating coverage report"
gcovr -x -r .. > reports/coverage.xml
gcovr -r .. --html --html-details -o reports/coverage.html

echo "Generating cppcheck report"
cppcheck --enable=all --xml ../src 2> reports/cppcheck.xml

cd ..
echo "Cleanup that mess"
cleanup

echo "Integration successful"
