cmake_minimum_required(VERSION 3.0)

project(libjson-rpc-cpp)

set(MAJOR_VERSION 2)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)
set(SO_VERSION    2)
set(VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(COVERAGE "generate coverage reports" OFF)
option(COMPILE_TESTS "compile with testing support" ON)
option(JSONRPC_SERVER "jsonrpc server" ON)
option(JSONRPC_CLIENT "jsonrpc client" ON)
option(CONNECTOR_MICROHTTPD "libmicrohttpd based server connector" ON)
option(CONNECTOR_CURL "libcurl based client connector" ON)
option(STUBGENERATOR "stubgenerator for genearting client and server stubs" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -Wno-format-extra-args -Wformat-security -Wformat-nonliteral -Wformat=2 -Wextra -Wnon-virtual-dtor")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

add_subdirectory(src)

if(COMPILE_TESTS)
  enable_testing()
  set(TEST_LIBRARIES "")
  add_subdirectory(src/test)
endif()

if (JSONRPC_CLIENT)
  add_subdirectory(src/client)
endif()

if (JSONRPC_SERVER)
  add_subdirectory(src/server)
endif()

add_subdirectory(src/connector)

if(CONNECTOR_MICROHTTPD)
  add_subdirectory(src/connector/microhttpd)
endif()

if(CONNECTOR_CURL)
  add_subdirectory(src/connector/curl)
endif()


if(STUBGENERATOR)
  add_subdirectory(src/stubgenerator)
endif()