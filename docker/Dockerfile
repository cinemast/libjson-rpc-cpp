# THIS DOCKERFILE TRIES TO COMPILE CURL, LIBMICROHTTPD, JSONCPP, ARGTABLE AND LIBJSON-RPC-CPP FOR ANDROID
# Based on Victor Laskin Dockerfile (http://vitiy.info/dockerfile-example-to-compile-libcurl-for-android-inside-docker-container/)

FROM debian:sid

MAINTAINER Péricles Lopes Machado <pericles.raskolnikoff@gmail.com>

ENV ANDROID_SDK_PACKAGE android-sdk_r24.2-linux.tgz
ENV ANDROID_NDK_PACKAGE android-ndk-r10e-linux-x86_64.bin

# Install compilation tools

RUN apt-get update && apt-get install -y \
automake \
build-essential \
wget \
p7zip-full \
bash \
cmake \
git \
libjsonrpccpp-dev \
libjsonrpccpp \
libjsonrpccpp-tools


# Download SDK / NDK

RUN mkdir /Android && cd Android && mkdir output
WORKDIR /Android

RUN wget http://dl.google.com/android/$ANDROID_SDK_PACKAGE
RUN wget http://dl.google.com/android/ndk/$ANDROID_NDK_PACKAGE

# Extracting ndk/sdk

RUN tar -xvzf $ANDROID_SDK_PACKAGE && \
chmod a+x $ANDROID_NDK_PACKAGE && \
7z x $ANDROID_NDK_PACKAGE

# Set ENV variables

ENV ANDROID_HOME /Android/android-sdk-linux
ENV NDK_ROOT /Android/android-ndk-r10e

ENV PATH $PATH:$ANDROID_HOME/tools
ENV PATH $PATH:$ANDROID_HOME/platform-tools

# Make stand alone toolchain (Modify platform / arch here)

RUN mkdir=toolchain-arm && \
bash $NDK_ROOT/build/tools/make-standalone-toolchain.sh \
--verbose --platform=android-14 --install-dir=toolchain-arm \
--arch=arm --toolchain=arm-linux-androideabi-4.9 --system=linux-x86_64

ENV TOOLCHAIN /Android/toolchain-arm
ENV SYSROOT $TOOLCHAIN/sysroot
ENV PATH $PATH:$TOOLCHAIN/bin:$SYSROOT/usr/local/bin

# Configure toolchain path

ENV ARCH armv7

ENV CROSS_COMPILE arm-linux-androideabi
ENV CC arm-linux-androideabi-gcc
ENV CXX arm-linux-androideabi-g++
ENV AR arm-linux-androideabi-ar
ENV AS arm-linux-androideabi-as
ENV LD arm-linux-androideabi-ld
ENV RANLIB arm-linux-androideabi-ranlib
ENV NM arm-linux-androideabi-nm
ENV STRIP arm-linux-androideabi-strip
ENV CHOST arm-linux-androideabi

ENV CPPFLAGS -std=c++11

# Download and extract curl

ENV CFLAGS \
-v -DANDROID --sysroot=$SYSROOT -mandroid -march=$ARCH -mfloat-abi=softfp -mfpu=vfp -mthumb 

ENV CPPFLAGS $CPPFLAGS $CFLAGS \
-Wno-psabi -march=armv7-a \
-mfloat-abi=softfp -mfpu=vfp -ffunction-sections \
-funwind-tables -fstack-protector -fno-short-enums \
-DANDROID -Wa,--noexecstack -fno-builtin-memmove \
-std=gnu++0x -O2 -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -mthumb -std=c++1y -Wall -Wno-psabi -W -D_REENTRANT -fPIE 

ENV LDFLAGS -L${TOOLCHAIN}/include

RUN wget http://curl.haxx.se/download/curl-7.38.0.tar.gz && \
tar -xzf curl-7.38.0.tar.gz
RUN cd curl-7.38.0 && ./configure --host=arm-linux-androideabi \
--enable-shared --disable-static \
--disable-dependency-tracking \
--with-zlib=/Android/zlib --without-ca-bundle --without-ca-path \
--enable-ipv6 --disable-ftp --disable-file --disable-ldap \
--disable-ldaps --disable-rtsp --disable-proxy --disable-dict\
--disable-telnet --disable-tftp --disable-pop3 --disable-imap\
--disable-smtp --disable-gopher --disable-sspi --disable-manual\
--target=arm-linux-androideabi --build=x86_64-unknown-linux-gnu || cat config.log

# Make curl 

RUN cd curl-7.38.0 && \
make \
CC=arm-linux-androideabi-gcc \
CXX=arm-linux-androideabi-g++ && \
ls lib/.libs/ && \
cp lib/.libs/libcurl.so /Android/output && \
ls -hs /Android/output && \
mkdir /output

#clone android-cmake-toolchain
RUN git clone https://github.com/taka-no-me/android-cmake.git

#clone and build jsoncpp project
RUN git clone https://github.com/gogo40/jsoncpp.git

RUN mkdir jsoncpp-dev && \
cd jsoncpp-dev && \
cmake -DCMAKE_TOOLCHAIN_FILE=/Android/android-cmake/android.toolchain.cmake \
-DANDROID_NDK=$NDK_ROOT                       \
-DCMAKE_BUILD_TYPE=Release                     \
-DANDROID_ABI="armeabi-v7a"          \
-DANDROID_NATIVE_API_LEVEL=14  \
-DJSONCPP_WITH_TESTS=NO \
-DBUILD_STATIC_LIBS=OFF -DBUILD_SHARED_LIBS=ON \
/Android/jsoncpp && \
make \
CC=arm-linux-androideabi-gcc \
CXX=arm-linux-androideabi-g++ && \
ls


#get and build argtable
RUN wget http://prdownloads.sourceforge.net/argtable/argtable2-13.tar.gz && \
tar -xzf argtable2-13.tar.gz  

RUN mkdir argtable-dev && \
cd argtable-dev && \
cmake -DCMAKE_TOOLCHAIN_FILE=/Android/android-cmake/android.toolchain.cmake \
-DANDROID_NDK=$NDK_ROOT                       \
-DCMAKE_BUILD_TYPE=Release                     \
-DANDROID_ABI="armeabi-v7a"          \
-DANDROID_NATIVE_API_LEVEL=14  \
/Android/argtable2-13 && \
make \
CC=arm-linux-androideabi-gcc \
CXX=arm-linux-androideabi-g++ && \
ls

#get and build libmicrohttpd
RUN wget http://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.42.tar.gz && \
tar -xzf libmicrohttpd-0.9.42.tar.gz

RUN cd libmicrohttpd-0.9.42 && \
./configure --host=arm-linux-androideabi  \
--enable-shared --disable-static \
--disable-dependency-tracking  \
--target=arm-linux-androideabi \
--build=x86_64-unknown-linux-gnu || cat config.log && \
make \
CC=arm-linux-androideabi-gcc \
CXX=arm-linux-androideabi-g++ && \
ls


RUN cp /Android/jsoncpp-dev/src/lib_json/libjsoncpp.so /Android/output
RUN cp /Android/argtable-dev/src/libargtable2.a /Android/output
RUN cp /Android/libmicrohttpd-0.9.42/src/microhttpd/.libs/libmicrohttpd.so /Android/output

#get and build libjson-rpc-cpp-android
RUN git clone https://github.com/cinemast/libjson-rpc-cpp.git

RUN cd /Android/libjson-rpc-cpp && \
cmake -DCMAKE_TOOLCHAIN_FILE=/Android/android-cmake/android.toolchain.cmake \
-DANDROID_NDK=$NDK_ROOT                       \
-DCMAKE_BUILD_TYPE=Release                     \
-DANDROID_ABI="armeabi-v7a"          \
-DANDROID_NATIVE_API_LEVEL=14  \
-DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON \
-DCURL_INCLUDE_DIR=/Android/curl-7.38.0/include \
-DCURL_LIBRARY=/Android/curl-7.38.0/lib/.libs/libcurl.so \
-DARGTABLE_INCLUDE_DIR=/Android/argtable2-13/src \
-DARGTABLE_LIBRARY=/Android/argtable-dev/src/libargtable2.a \
-DJSONCPP_INCLUDE_DIR=/Android/jsoncpp/include \
-DJSONCPP_LIBRARY=/Android/jsoncpp-dev/src/lib_json/libjsoncpp.so \
-DMHD_INCLUDE_DIR=/Android/libmicrohttpd-0.9.42/src/include \
-DMHD_LIBRARY=/Android/libmicrohttpd-0.9.42/src/microhttpd/.libs/libmicrohttpd.so \
/Android/libjson-rpc-cpp  

RUN \
ls /Android/argtable2-13/src && \
cp -r /Android/curl-7.38.0/include/curl /Android/libjson-rpc-cpp/src && \
cd /Android/libjson-rpc-cpp && \
make \
CC=arm-linux-androideabi-gcc \
CXX=arm-linux-androideabi-g++

RUN cp /Android/libjson-rpc-cpp/lib/*.so /Android/output
RUN cp -r /Android/argtable2-13 /Android/output
RUN cp -r /Android/libmicrohttpd-0.9.42 /Android/output
RUN cp -r /Android/jsoncpp /Android/output
RUN cp -r /Android/curl-7.38.0 /Android/output
RUN cp -r /Android/libjson-rpc-cpp /Android/output

# To get the results run container with output folder
# Example: docker run -v HOSTFOLDER:/output --rm=true IMAGENAME

ENTRYPOINT cp -r /Android/output/* /output

