# This file is only used for development for convinience functions as
# quick builds and tests

build:
	mkdir -p build
	cd build && cmake .. && make -j$(nproc)

build-docker:
	cd docker && make all

test: build
	cd build &&	./bin/unit_testsuite

clean:
	rm -rf build
