#!/bin/bash
set -e
mkdir -p build
cd build
cmake ..
make
make test
sudo make install
sudo ldconfig
g++ ../src/examples/simpleclient.cpp -ljsonrpccpp-client -ljsoncpp -ljsonrpccpp-common -lcurl -o sampleclient 
g++ ../src/examples/simpleserver.cpp -ljsonrpccpp-server -ljsoncpp -ljsonrpccpp-common -o sampleserver
test -f sampleclient
test -f sampleserver
sudo make uninstall
echo "Integration successful"
