cmake_minimum_required(VERSION 2.6)
project(libjson-rpc-cpp)
cmake_policy(SET CMP0007 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wnon-virtual-dtor -fprofile-arcs -ftest-coverage -fPIC -O0")

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 4)
SET(PATCH_VERSION 2)
SET(SO_VERSION    0)

#possible modules that can be enabled/disabled
SET(HTTP_SERVER YES CACHE BOOL "Include HTTP server using libmicrohttpd")
SET(HTTP_CLIENT YES CACHE BOOL "Include HTTP client support using curl")

SET(COMPILE_TESTS YES CACHE BOOL "Compile test framework")
SET(COMPILE_STUBGEN YES CACHE BOOL "Compile the stubgenerator")
SET(COMPILE_EXAMPLES YES CACHE BOOL "Compile example programs")

# TODO that's very ugly
# include XXX_INCLUDE_DIRS instead!
include_directories(${CMAKE_PREFIX_PATH}/include)

IF(WIN32)
# TODO that's even more ugly
# maybe use cmake external dependencies instead
	include_directories(${CMAKE_SOURCE_DIR}/win32-deps/include)
	SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dist)
        SET(BOOST_ROOT "C:/boost_1_57_0")
ENDIF()

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

# TODO hanlde other msvc versions or it will fail find them
  SET(Boost_COMPILER -vc120)
# use static boost libraries *.lib
  SET(Boost_USE_STATIC_LIBS ON)

ENDIF()

#the core framework
add_subdirectory(src/jsonrpccpp)

include_directories("${PROJECT_SOURCE_DIR}/src/")
include_directories("${PROJECT_BINARY_DIR}/gen/")
include_directories("${PROJECT_BINARY_DIR}/gen/jsonrpccpp/common")

#the stubgenerator
IF(COMPILE_STUBGEN)
    add_subdirectory(src/stubgenerator)
ENDIF()

#the examples
IF(COMPILE_EXAMPLES AND HTTP_SERVER AND HTTP_CLIENT)
    add_subdirectory(src/examples)
ENDIF()

#the test suite
find_package(Boost COMPONENTS unit_test_framework)
IF(Boost_FOUND AND COMPILE_TESTS)
    ENABLE_TESTING()
    add_subdirectory(src/test)
ENDIF()

find_package(Doxygen)
if(DOXYGEN_FOUND)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
    message(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")
    configure_file("${CMAKE_SOURCE_DIR}/doc/doxyfile.in" "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc COMMENT "Generating API documentation")
endif(DOXYGEN_FOUND)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

#packaging stuff
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libjson-rpc-cpp")
SET(CPACK_PACKAGE_VENDOR "Peter Spiess-Knafl <psk@autistici.org>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")

IF(WIN32 AND NOT UNIX)
  SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} libjson-rpc-cpp")
  SET(CPACK_NSIS_HELP_LINK "http://github.com/cinemast/libjson-rpc-cpp")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://github.com/cinemast/libjson-rpc-cpp")
  SET(CPACK_NSIS_CONTACT "psk@autistici.org")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)
