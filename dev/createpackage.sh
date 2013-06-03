#!/bin/bash

	echo "Bundling for $1"
	mkdir -p ../dist/libjson-rpc-cpp
	mkdir -p ../dist/libjson-rpc-cpp/doc
	cd ../dist/doc
	doxygen ../../doc/libjson-rpc-cpp.doxyfile
	cd ../../
	cd build
	cmake -DCMAKE_INSTALL_PREFIX=../dist/libjson-rpc-cpp
	make install
	cd ../dist
	zip -r libjson-rpc-cpp-$1.zip json-rpc-cpp
	rm -r libjson-rpc-cpp
	cd ../dev
