# - Try to find CURL
# Once done this will define
#
#  CURL_FOUND - system has CURL
#  CURL_INCLUDE_DIRS - the CURL include directory
#  CURL_LIBRARY - Link these to use CURL

FIND_LIBRARY (CURL_LIBRARIES NAMES curl libcurl
    PATHS
    /usr/lib
    /usr/local/lib
    ${CMAKE_SOURCE_DIR}/win32-deps/lib
)

FIND_PATH (CURL_INCLUDE_DIRS curl.h
    PATHS
    /usr/include
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/win32-deps/include
    PATH_SUFFIXES curl
)

IF(CURL_INCLUDE_DIRS AND CURL_LIBRARIES)
    SET(CURL_FOUND TRUE)
ENDIF(CURL_INCLUDE_DIRS AND CURL_LIBRARIES)

IF(CURL_FOUND)
    IF(NOT CURL_FIND_QUIETLY)
        MESSAGE(STATUS "Found libcurl: ${CURL_LIBRARIES}")
    ENDIF(NOT CURL_FIND_QUIETLY)
ELSE(CURL_FOUND)
    IF(CURL_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find libcurl")
    ENDIF(CURL_FIND_REQUIRED)
ENDIF(CURL_FOUND)

MARK_AS_ADVANCED(
    CURL_LIBRARIES
    CURL_INCLUDE_DIRS
)
