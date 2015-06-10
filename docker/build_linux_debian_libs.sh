#!/bin/bash

docker build -t debian/libjson-rpc-cpp .
docker run -v $PWD/output:/output --rm=true  debian/libjson-rpc-cpp

