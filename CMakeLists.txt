cmake_minimum_required(VERSION 2.8)
project(libjson-rpc-cpp)

# list command no longer ignores empty elements 
cmake_policy(SET CMP0007 NEW)
cmake_policy(SET CMP0012 NEW)

if (${CMAKE_MAJOR_VERSION} GREATER 2)
	# old policy do not use MACOSX_RPATH
	cmake_policy(SET CMP0042 OLD)
endif()

set(MAJOR_VERSION 0)
set(MINOR_VERSION 4)
set(PATCH_VERSION 2)
set(SO_VERSION    0)

# defaults for modules that can be enabled/disabled
set(HTTP_SERVER YES CACHE BOOL "Include HTTP server using libmicrohttpd")
set(HTTP_CLIENT YES CACHE BOOL "Include HTTP client support using curl")
set(COMPILE_TESTS YES CACHE BOOL "Compile test framework")
set(COMPILE_STUBGEN YES CACHE BOOL "Compile the stubgenerator")
set(COMPILE_EXAMPLES YES CACHE BOOL "Compile example programs")

# print actual settings
message(STATUS "HTTP_SERVER: ${HTTP_SERVER}")
message(STATUS "HTTP_CLIENT: ${HTTP_CLIENT}")
message(STATUS "COMPILE_TESTS: ${COMPILE_TESTS}")
message(STATUS "COMPILE_STUBGEN: ${COMPILE_STUBGEN}")
message(STATUS "COMPILE_EXAMPLES: ${COMPILE_EXAMPLES}")

# setup directory where we should look for cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# setup compiler settings && dependencies
include(CMakeCompilerSettings)
include(CMakeDependencies)

# setup output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if (WIN32)
	set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dist)
endif()

# setup global include_directories (for backwards compatibility)
include_directories("${CMAKE_BINARY_DIR}/gen/")
include_directories("${CMAKE_BINARY_DIR}/gen/jsonrpccpp/common")

# the core framework
add_subdirectory(src/jsonrpccpp)

#the stubgenerator
if (COMPILE_STUBGEN)
	add_subdirectory(src/stubgenerator)
endif ()

# setup examples
if (COMPILE_EXAMPLES AND HTTP_SERVER AND HTTP_CLIENT)
	add_subdirectory(src/examples)
endif()

# setup test suite
if (Boost_FOUND AND COMPILE_TESTS)
	enable_testing()
	add_subdirectory(src/test)
endif()

# create documentation
if (DOXYGEN_FOUND)
	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
	message(STATUS "Found doxygen: ${DOXYGEN_EXECUTABLE}")
	configure_file("${CMAKE_SOURCE_DIR}/doc/doxyfile.in" "${CMAKE_BINARY_DIR}/Doxyfile" @ONLY)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc COMMENT "Generating API documentation")
endif(DOXYGEN_FOUND)

# setup uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY
)

add_custom_target(
	uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

# packaging stuff
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libjson-rpc-cpp")
set(CPACK_PACKAGE_VENDOR "Peter Spiess-Knafl <psk@autistici.org>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")

if (WIN32 AND NOT UNIX)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} libjson-rpc-cpp")
	set(CPACK_NSIS_HELP_LINK "http://github.com/cinemast/libjson-rpc-cpp")
	set(CPACK_NSIS_URL_INFO_ABOUT "http://github.com/cinemast/libjson-rpc-cpp")
	set(CPACK_NSIS_CONTACT "psk@autistici.org")
	set(CPACK_NSIS_MODIFY_PATH ON)
endif(WIN32 AND NOT UNIX)
include(CPack)

