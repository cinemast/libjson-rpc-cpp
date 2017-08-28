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
if(HUNTER_ENABLED)
	hunter_add_package(jsoncpp)
	find_package(jsoncpp CONFIG REQUIRED)
	set(JSONCPP_INCLUDE_PREFIX "json")
else()
	find_package(Jsoncpp REQUIRED)
	message(STATUS "Jsoncpp header: ${JSONCPP_INCLUDE_DIR}")
	message(STATUS "Jsoncpp lib   : ${JSONCPP_LIBRARY}")
	message(STATUS "Jsoncpp prefix: ${JSONCPP_INCLUDE_PREFIX}")
endif()

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
	if(HUNTER_ENABLED)
		hunter_add_package(CURL)
		find_package(CURL CONFIG REQUIRED)
	else()
		find_package(CURL REQUIRED)
		message(STATUS "CURL header: ${CURL_INCLUDE_DIR}")
		message(STATUS "CURL lib   : ${CURL_LIBRARY}")
	endif()
endif()

# find libmicrohttpd
if (${HTTP_SERVER})

    find_package(MHD REQUIRED)
    message(STATUS "MHD header: ${MHD_INCLUDE_DIRS}")
    message(STATUS "MHD lib   : ${MHD_LIBRARIES}")
endif()

# find hiredis
if (${REDIS_SERVER} OR ${REDIS_CLIENT})

    find_package(Hiredis REQUIRED)
    message(STATUS "Hiredis header: ${HIREDIS_INCLUDE_DIRS}")
    message(STATUS "Hiredis lib   : ${HIREDIS_LIBRARIES}")
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
        include_directories(${CATCH_INCLUDE_DIRS})
    endif()
endif()
