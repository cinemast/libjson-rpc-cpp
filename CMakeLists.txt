cmake_minimum_required(VERSION 2.6)
project(jsonrpccpp)

include_directories(lib)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out/lib)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out/bin)


add_subdirectory(lib)
#add_subdirectory(sample)


