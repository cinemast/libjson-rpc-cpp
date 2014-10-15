# - Try to find CURL
# Once done this will define
#
#  CURL_FOUND - system has CURL
#  CURL_INCLUDES - the CURL include directory
#  CURL_LIBRARY - Link these to use CURL

FIND_LIBRARY (CURL_LIBRARIES NAMES curl
    PATHS
    ENV LD_LIBRARY_PATH
    ENV LIBRARY_PATH
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /opt/local/lib
	 ${CURL_ROOT}/lib
	${CMAKE_SOURCE_DIR}/win32-deps/lib
    )

FIND_PATH (CURL_INCLUDE_DIRS curl.h
    ENV CPATH
	${CURL_ROOT}/include
    /usr/include
    /usr/local/include
    /opt/local/include
	${CMAKE_SOURCE_DIR}/win32-deps/include
    PATH_SUFFIXES curl
    )

IF(CURL_INCLUDE_DIRS AND CURL_LIBRARIES)
    SET(CURL_FOUND TRUE)
ENDIF(CURL_INCLUDE_DIRS AND CURL_LIBRARIES)

IF(CURL_FOUND)
  IF(NOT CURL_FIND_QUIETLY)
    MESSAGE(STATUS "Found CURL: ${CURL_LIBRARIES}")
  ENDIF(NOT CURL_FIND_QUIETLY)
ELSE(CURL_FOUND)
  IF(CURL_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find CURL")
  ENDIF(CURL_FIND_REQUIRED)
ENDIF(CURL_FOUND)

MARK_AS_ADVANCED(
  CURL_LIBRARIES
  CURL_INCLUDE_DIRS
  )

