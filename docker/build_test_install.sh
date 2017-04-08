set -v

CLIENT_LIBS="-ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client"
SERVER_LIBS="-ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server"
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_LIBS=ON \
-DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
make test
make install
ldconfig
ls -l /usr/local/lib
cd ../src/examples
g++ simpleclient.cpp $CLIENT_LIBS -o simpleclient
g++ simpleserver.cpp $SERVER_LIBS -o simpleserver

mkdir gen && cd gen
jsonrpcstub ../spec.json --cpp-server=AbstractStubServer --cpp-client=StubClient
cd ..
g++ stubclient.cpp $CLIENT_LIBS -o stubclient
g++ stubserver.cpp $SERVER_LIBS -o stubserver

test -f simpleclient
test -f simpleserver
test -f stubserver
test -f stubclient
