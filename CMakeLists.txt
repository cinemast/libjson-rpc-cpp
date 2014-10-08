LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

cmake_minimum_required(VERSION 2.6)
project(libjson-rpc-cpp)

include_directories(${CMAKE_SOURCE_DIR}/src/jsonrpccpp)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall --coverage")
SET(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -Wall --coverage")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lgcov")

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 3)
SET(PATCH_VERSION 0)

#possible modules that can be enabled/disabled
SET(HTTP_SERVER_MONGOOSE YES CACHE BOOL "Include HTTP server using mongoose")
SET(HTTP_CLIENT_CURL YES CACHE BOOL "Include HTTP client support using curl")
SET(SOCKET_SERVER NO CACHE BOOL "Include Win32 Socket support")
SET(SOCKET_CLIENT NO CACHE BOOL "Include Win32 Socket support")

SET(COMPILE_TESTS YES CACHE BOOL "Compile test framework")

#the core framework
add_subdirectory(src/jsonrpccpp)

find_package(Boost COMPONENTS unit_test_framework)
IF(Boost_FOUND)
    IF (COMPILE_TESTS)
        message(STATUS "Found Bosst Unit Suite -> Enabling test cases")
        ENABLE_TESTING()
        set(CTEST_OUTPUT_ON_FAILURE TRUE)
        add_subdirectory(src/test)
    ENDIF()
ENDIF()
