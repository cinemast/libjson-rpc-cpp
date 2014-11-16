cmake_minimum_required(VERSION 2.6)
project(libjson-rpc-cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wnon-virtual-dtor -fprofile-arcs -ftest-coverage -fPIC -O0")

SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 4)
SET(PATCH_VERSION 0)
SET(SO_VERSION    0)

#possible modules that can be enabled/disabled
SET(HTTP_SERVER YES CACHE BOOL "Include HTTP server using libmicrohttpd")
SET(HTTP_CLIENT YES CACHE BOOL "Include HTTP client support using curl")

SET(COMPILE_TESTS YES CACHE BOOL "Compile test framework")
SET(COMPILE_STUBGEN YES CACHE BOOL "Compile the stubgenerator")
SET(COMPILE_EXAMPLES YES CACHE BOOL "Compile example programs")

IF(WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/win32-deps/include)
	SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dist)
ENDIF()

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
IF(COMPILE_EXAMPLES AND HTTP_SERVER AND HTTP_CLIENT)
    add_subdirectory(src/examples)
ENDIF()

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
