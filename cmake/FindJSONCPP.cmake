# - Try to find JSONCPP
# Once done this will define
#
#  JSONCPP_FOUND - system has JSONCPP
#  JSONCPP_INCLUDE_DIRS - the JSONCPP include directory
#  JSONCPP_LIBRARY - Link these to use JSONCPP

FIND_LIBRARY (JSONCPP_LIBRARIES NAMES jsoncpp
    PATHS
    /usr/lib
    /usr/local/lib
    ${CMAKE_SOURCE_DIR}/win32-deps/lib
)

FIND_PATH (JSONCPP_INCLUDE_DIRS json.h
    PATHS
    /usr/include
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/win32-deps/include
    PATH_SUFFIXES jsoncpp/json json
)

IF(JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES)
    SET(JSONCPP_FOUND TRUE)
ENDIF(JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES)

IF(JSONCPP_FOUND)
    if(EXISTS "${JSONCPP_INCLUDE_DIRS}/version.h")
        MESSAGE(STATUS "Found New libjsoncpp: ${JSONCPP_LIBRARIES}")
        set(JSONCPP_INCLUDE_PREFIX "json")
    else()
        MESSAGE(STATUS "Found old libjsoncpp: ${JSONCPP_LIBRARIES}")
        set(JSONCPP_INCLUDE_PREFIX "jsoncpp/json")
    endif()
ELSE(JSONCPP_FOUND)
    IF(JSONCPP_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find libjsoncpp")
    ENDIF(JSONCPP_FIND_REQUIRED)
ENDIF(JSONCPP_FOUND)

MARK_AS_ADVANCED(
    JSONCPP_LIBRARIES
    JSONCPP_INCLUDE_DIRS
)
