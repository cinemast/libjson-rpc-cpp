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

# boost stuff

# left for backwards compatbility
if (WIN32)
	set(BOOST_ROOT "C:/boost_1_57_0")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	set(Boost_USE_MULTITHREADED ON)
# TODO hanlde other msvc versions or it will fail find them
	set(Boost_COMPILER -vc120)
# use static boost libraries *.lib
	set(Boost_USE_STATIC_LIBS ON)

endif()

# TODO: add? appriopriate commands if it's found
find_package(Boost COMPONENTS unit_test_framework)


