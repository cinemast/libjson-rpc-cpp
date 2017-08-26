# This file is only used for development for convinience functions as
# quick builds and tests

.PHONY: build

build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=Yes .. && make -j$(nproc)

build-docker:
	cd docker && make all

coverage: test
	mkdir -p reports
	gcovr -r . -d -e "build" -e "src/test" -e "src/examples" -e "src/stubgenerator/main.cpp" --html --html-details -o reports/coverage.html

formate:
	find . -name "*.h" -o -name "*.cpp" -exec clang-format -i {} \;

test: build
	cd build &&	./bin/unit_testsuite

clean:
	rm -rf build
