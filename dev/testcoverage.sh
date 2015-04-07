#!/bin/bash
cd ../build

echo "Building debug build"
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4

echo "Executing Test Suite"
bin/unit_testsuite

echo "Generate HTML report"
mkdir -p reports
gcovr -r .. -d -e "build" -e "src/test" --html --html-details -o reports/coverage.html
xdg-open reports/coverage.html
cd ../dev

