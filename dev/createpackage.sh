#!/bin/bash

	echo "Bundling for $1"
	mkdir -p ../dist/libjson-rpc-cpp
	mkdir -p ../dist/libjson-rpc-cpp/doc
	cd ../doc
	doxygen libjson-rpc-cpp.doxyfile
	mv html/* ../dist/libjson-rpc-cpp/doc
	#cmake -DCMAKE_INSTALL_PREFIX=../dist/libjson-rpc-cpp
	#make install
	#cd ../dist
	zip -r libjson-rpc-cpp-$1.zip libjson-rpc-cpp
	rm -r libjson-rpc-cpp
	cd ../dev
