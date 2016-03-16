# all dependencies that are not directly included in the libjson-rpc-cpp distribution are defined here!
# default search directory for dependencies is ${CMAKE_SOURCE_DIR}/win32-deps (for backwards compatibility)
# if your dependencies directory is different, please run cmake with CMAKE_PREFIX_PATH option eg:
#
# cmake -DCMAKE_PREFIX_PATH=path_to_your_dependencies .

# set default dependencies search path
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_SOURCE_DIR}/win32-deps")

# find JSONCPP
# TODO: handle windows debug libraries!
# TODO: fix FindJSONCPP file!
find_package(Jsoncpp)
message(STATUS "Jsoncpp header: ${JSONCPP_INCLUDE_DIRS}")
message(STATUS "Jsoncpp lib   : ${JSONCPP_LIBRARIES}")
message(STATUS "Jsoncpp prefix: ${JSONCPP_INCLUDE_PREFIX}")

# find Threads!
find_package(Threads)
message(STATUS "Threads: ${CMAKE_THREADS_LIBS_INIT}")

# find Argtable
if(${COMPILE_STUBGEN})
	find_package(Argtable REQUIRED)
	message(STATUS "Argtable header: ${ARGTABLE_INCLUDE_DIRS}")
	message(STATUS "Argtable lib   : ${ARGTABLE_LIBRARIES}")
endif()

if(${HTTP_CLIENT})
# find CURL
	find_package(CURL REQUIRED)
	message(STATUS "CURL header: ${CURL_INCLUDE_DIRS}")
	message(STATUS "CURL lib   : ${CURL_LIBRARIES}")
endif()

if (${HTTP_SERVER})
# find libmicrohttpd
	find_package(MHD REQUIRED)
	message(STATUS "MHD header: ${MHD_INCLUDE_DIRS}")
	message(STATUS "MHD lib   : ${MHD_LIBRARIES}")
endif()

# find doxygen
find_package(Doxygen)

if (${COMPILE_TESTS})

	find_package(Catch)
	if(NOT CATCH_FOUND)
		message("Could not find catch, downloading it now")
    	# Includes Catch in the project:
	   	add_subdirectory(${CMAKE_SOURCE_DIR}/src/catch)
	   	include_directories(${CATCH_INCLUDE_DIR} ${COMMON_INCLUDES})
	else()
    	INCLUDE_DIRECTORIES(${CATCH_INCLUDE_DIRS})
	endif()
endif()
