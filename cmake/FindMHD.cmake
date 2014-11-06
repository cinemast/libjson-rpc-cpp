# - Try to find MHD
# Once done this will define
#
#  MHD_FOUND - system has MHD
#  MHD_INCLUDE_DIRS - the MHD include directory
#  MHD_LIBRARY - Link these to use MHD

FIND_LIBRARY (MHD_LIBRARIES NAMES microhttpd microhttpd-10
    PATHS
    /usr/lib
    /usr/local/lib
    ${CMAKE_SOURCE_DIR}/win32-deps/lib
)

FIND_PATH (MHD_INCLUDE_DIRS microhttpd.h
    PATHS
    /usr/include
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/win32-deps/include
)

IF(MHD_INCLUDE_DIRS AND MHD_LIBRARIES)
    SET(MHD_FOUND TRUE)
ENDIF(MHD_INCLUDE_DIRS AND MHD_LIBRARIES)

IF(MHD_FOUND)
    IF(NOT MHD_FIND_QUIETLY)
        MESSAGE(STATUS "Found libmicrohttpd: ${MHD_LIBRARIES}")
    ENDIF(NOT MHD_FIND_QUIETLY)
ELSE(MHD_FOUND)
    IF(MHD_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find libmicrohttpd")
    ENDIF(MHD_FIND_REQUIRED)
ENDIF(MHD_FOUND)

MARK_AS_ADVANCED(
    MHD_LIBRARIES
    MHD_INCLUDE_DIRS
)
