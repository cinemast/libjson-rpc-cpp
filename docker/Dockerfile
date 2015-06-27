# THIS DOCKERFILE DOWNLOADS AND COMPILES CURL, LIBMICROHTTPD, JSONCPP, ARGTABLE AND LIBJSON-RPC-CPP FOR LINUX/DEBIAN

# 2015, author: Péricles Lopes Machado (gogo40) <pericles.raskolnikoff@gmail.com>
# Based on Victor Laskin Dockerfile (http://vitiy.info/dockerfile-example-to-compile-libcurl-for-android-inside-docker-container/)

FROM debian:sid

MAINTAINER Péricles Lopes Machado <pericles.raskolnikoff@gmail.com>

# Create output directories

# Directory to export generated files
RUN mkdir /output 

# Directory with generated files
RUN mkdir /build && mkdir /build/include

# Install compilation tools

RUN apt-get update 

RUN apt-get install -y \
wget \
build-essential \
cmake \
libjsoncpp-dev \
libargtable2-dev \
libcurl4-openssl-dev \
libmicrohttpd-dev \
git 

# Clone and build libjson-rpc-cpp

RUN git clone https://github.com/cinemast/libjson-rpc-cpp.git

RUN cd /libjson-rpc-cpp && \
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON  \
/libjson-rpc-cpp  

RUN cd /libjson-rpc-cpp && \
make -j $(nproc) && \
make install 

# Copy to output generated files

RUN cp -r /libjson-rpc-cpp/lib /build
RUN cp -r /usr/local/include/jsonrpccpp /build/include/jsonrpccpp

# To get the results run container with output folder
# Example: docker run -v HOSTFOLDER:/output --rm=true IMAGENAME

ENTRYPOINT cp -r /build/* /output


