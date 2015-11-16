# Find ZMQ
#
# Find the zeromq includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  ZMQ_INCLUDE_DIRS, where to find header, etc.
#  ZMQ_LIBRARIES, the libraries needed to use curl.
#  ZMQ_FOUND, If false, do not try to use curl.

# only look in default directories
find_path(
    ZMQ_INCLUDE_DIR 
	NAMES zmq.h zmq.hpp
	DOC "zeromq include dir"
)

find_library(
    ZMQ_LIBRARY
	# names from cmake's FindCURL
	NAMES zmq
	DOC "zeromq library"
)

set(ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})
set(ZMQ_LIBRARIES ${ZMQ_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZMQ DEFAULT_MSG
    ZMQ_INCLUDE_DIR ZMQ_LIBRARY)
mark_as_advanced (ZMQ_INCLUDE_DIR ZMQ_LIBRARY)

