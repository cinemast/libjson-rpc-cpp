cmake_minimum_required(VERSION 2.6)
project(libjson-rpc-cpp)


find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/src/deps)
include_directories(${CMAKE_SOURCE_DIR}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out)


add_subdirectory(src/deps/json)
add_subdirectory(src/deps/mongoose)
add_subdirectory(src/jsonrpc)
add_subdirectory(src/test)
add_subdirectory(src/stubgenerator)
add_subdirectory(src/example)


# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/dev/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 1)
SET(PATCH_VERSION 0)

IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "libjsonrpccpp-dev")
SET(CPACK_SET_DESTDIR "on")


SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
SET(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET(CPACK_GENERATOR "PackageMaker")
	SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(CPACK_GENERATOR "DEB")
	SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	SET(CPACK_GENERATOR "NSIS")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

SET(CPACK_PACKAGE_DESCRIPTION "jsonrpc framework for C++, including server and client support with stubgenerator.")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "jsonrpc framework for C++")
SET(CPACK_PACKAGE_VENDOR "Peter Spiess-Knafl")
SET(CPACK_PACKAGE_CONTACT "Peter Spiess-Knafl <peter.knafl@gmail.com>")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4-nss-dev")

SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

SET(CPACK_COMPONENTS_ALL jsonrpc jsoncpp mongoose jsonrpcstub)
INCLUDE(CPack)

ENDIF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

ENABLE_TESTING()

set(TEST_BINARIES ${CMAKE_BINARY_DIR}/out/test)
set(CTEST_OUTPUT_ON_FAILURE TRUE)

ADD_TEST(helloworld ${TEST_BINARIES}/helloworld)
ADD_TEST(remotecounter ${TEST_BINARIES}/remotecounter)
ADD_TEST(remotecalculator ${TEST_BINARIES}/remotecalculator)
ADD_TEST(errorhandling ${TEST_BINARIES}/errorhandling)
ADD_TEST(jsonrpcprotocol ${TEST_BINARIES}/jsonrpcprotocol)





