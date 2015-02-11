# Set necessary compile and link flags

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	
else ()
	set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wnon-virtual-dtor -fprofile-arcs -ftest-coverage -fPIC -O0")
endif()
