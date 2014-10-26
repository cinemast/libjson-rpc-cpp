cmake_minimum_required(VERSION 2.6)
project(libjson-rpc-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall --coverage")
SET(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -Wall --coverage")

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 3)
SET(PATCH_VERSION 2)
SET(SO_VERSION    0)

#possible modules that can be enabled/disabled
SET(HTTP_SERVER_MONGOOSE YES CACHE BOOL "Include HTTP server using mongoose")
SET(HTTP_CLIENT_CURL YES CACHE BOOL "Include HTTP client support using curl")
SET(SOCKET_SERVER NO CACHE BOOL "Include Server Socket support")
SET(SOCKET_CLIENT NO CACHE BOOL "Include Client Socket support")

SET(COMPILE_TESTS YES CACHE BOOL "Compile test framework")
SET(COMPILE_STUBGEN YES CACHE BOOL "Compile the stubgenerator")
SET(COMPILE_EXAMPLES YES CACHE BOOL "Compile example programs")

#the core framework
add_subdirectory(src/jsonrpccpp)

#the test suite
find_package(Boost COMPONENTS unit_test_framework)
IF(Boost_FOUND AND COMPILE_TESTS)
    ENABLE_TESTING()
    add_subdirectory(src/test)
ENDIF()

#the stubgenerator
IF(COMPILE_STUBGEN)
    add_subdirectory(src/stubgenerator)
ENDIF()

#the examples
IF(COMPILE_EXAMPLES AND HTTP_SERVER_MONGOOSE AND HTTP_CLIENT_CURL)
    add_subdirectory(src/examples)
ENDIF()

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
