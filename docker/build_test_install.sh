set -v
set -e

CLIENT_LIBS="-ljsoncpp -lcurl -ljsonrpccpp-common -ljsonrpccpp-client"
SERVER_LIBS="-ljsoncpp -lmicrohttpd -ljsonrpccpp-common -ljsonrpccpp-server"
mkdir -p build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release \
	-DBUILD_STATIC_LIBS=ON -DTCP_SOCKET_SERVER=YES -DTCP_SOCKET_CLIENT=YES \
	-DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
make test
make install
ldconfig
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
