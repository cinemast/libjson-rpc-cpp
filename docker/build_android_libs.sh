#!/bin/bash

docker build -t android/libjson-rpc-cpp .
docker run -v $PWD/output:/output --rm=true  android/libjson-rpc-cpp

