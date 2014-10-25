#!/bin/bash
cd ../build

echo "Clear Coverage Data"
lcov --zerocounters --directory ../ -output-file coverage.info

echo "Executing Test Suite"
bin/unit_testsuite

echo "Grabbing Coverage"
lcov --capture --directory ../ --output-file coverage.info
lcov --remove coverage.info "/usr/*" -o coverage.info
lcov --remove coverage.info "*test*" -o coverage.info
#lcov --remove coverage.info "*mongoose.c" -o coverage.info

echo "Generate HTML report"
genhtml coverage.info --output-directory coverage
xdg-open coverage/index.html
cd ../dev

