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
    rm -f sampleclient
    rm -f sampleserver
    cd ..
    rm -rf build
}
rm -rf reports
mkdir -p reports
build_configuration "-DCMAKE_BUILD_TYPE=Debug -DHTTP_SERVER=YES -DHTTP_CLIENT=YES -DCOMPILE_STUBGEN=YES -DCOMPILE_EXAMPLES=YES -DCOMPILE_TESTS=YES -DUNIX_DOMAIN_SOCKET_SERVER=YES -DUNIX_DOMAIN_SOCKET_CLIENT=YES"
echo "Compiling examples"
cd build
g++ ../src/examples/simpleclient.cpp -Iroot/usr/local/include -Lroot/usr/local/lib -ljsonrpccpp-client -ljsoncpp -ljsonrpccpp-common -lcurl -o sampleclient 
g++ ../src/examples/simpleserver.cpp -Iroot/usr/local/include -Lroot/usr/local/lib -ljsonrpccpp-server -ljsoncpp -ljsonrpccpp-common -lmicrohttpd -o sampleserver
test -f sampleclient
test -f sampleserver

echo "Generating valgrind report"
valgrind --leak-check=full --xml=yes --xml-file=../reports/valgrind.xml ./bin/unit_testsuite --reporter=junit --out=../reports/tests.xml


echo "Generating coverage report"
gcovr -x -r .. > ../reports/coverage.xml
gcovr -r .. --html --html-details -o ../reports/coverage.html

echo "Generating cppcheck report"
cppcheck --enable=all --xml ../src --xml-version=2 2> ../reports/cppcheck.xml

cd ..
echo "Cleanup that mess"
cleanup

build_configuration "-DCMAKE_BUILD_TYPE=Debug -DHTTP_SERVER=NO -DHTTP_CLIENT=NO -DCOMPILE_STUBGEN=YES -DCOMPILE_EXAMPLES=YES -DCOMPILE_TESTS=YES -DUNIX_DOMAIN_SOCKET_SERVER=NO -DUNIX_DOMAIN_SOCKET_CLIENT=NO"
cleanup

echo "Integration successful"
