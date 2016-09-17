# Set necessary compile and link flags


if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wformat -Wno-format-extra-args -Wformat-security -Wformat-nonliteral -Wformat=2 -Wextra -Wnon-virtual-dtor -fprofile-arcs -ftest-coverage -fPIC -O0")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
# TODO figure clang stuff to enable test-coverage
# Instrument Program flow should be set to Yes
# http://stackoverflow.com/questions/7949781/undefined-symbols-for-architecture-i386-upgrading-project-to-ios-5
	set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wnon-virtual-dtor -fPIC -O0")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        # Suppress warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4290")
endif()