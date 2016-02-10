PC_DEBUGRELEASE=${1:-Debug}
PC_TARGET=${2:-REC2}
rootfs=$3

echo "preconfiguring LibJsonRpcCpp"

# amalgamate libjsoncpp
echo "preparing LibJsonCpp..."
cd LibJsonCpp
python amalgamate.py
cd ..

# compile microhttpd
echo "preparing LibMicroHttpd..."
cd LibMicroHttpd
echo "configuring..."
./configure
echo "making..."
make
cd ..

# run cmake for LibJsonRpcCpp
cmake .
