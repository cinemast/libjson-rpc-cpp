# -*- cmake -*-
# - Find JSONCpp
# Find the JSONCpp includes and library
# This module defines
#  JSONCPP_INCLUDE_DIR, where to find json.h, etc.
#  JSONCPP_LIBRARIES, the libraries needed to use jsoncpp.
#  JSONCPP_FOUND, If false, do not try to use jsoncpp.
#  also defined, but not for general use are
#  JSONCPP_LIBRARY, where to find the jsoncpp library.

FIND_PATH(JSONCPP_INCLUDE_DIR json.h
/usr/include/jsoncpp/json
/usr/include/jsoncpp
/usr/local/include/jsoncpp/json
/usr/local/include/jsoncpp
)


FIND_LIBRARY(JSONCPP_LIBRARY NAMES jsoncpp HINTS /usr/lib /usr/local/lib)

IF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
    SET(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
    SET(JSONCPP_FOUND "YES")
ELSE (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
  SET(JSONCPP_FOUND "NO")
ENDIF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)


IF (JSONCPP_FOUND)
   IF (NOT JSONCPP_FIND_QUIETLY)
      MESSAGE(STATUS "Found JSONCpp: ${JSONCPP_LIBRARIES}")
   ENDIF (NOT JSONCPP_FIND_QUIETLY)
ELSE (JSONCPP_FOUND)
   IF (JSONCPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find JSONCPP library include: ${JSONCPP_INCLUDE_DIR}, lib: ${JSONCPP_LIBRARIES}")
   ENDIF (JSONCPP_FIND_REQUIRED)
ENDIF (JSONCPP_FOUND)

# Deprecated declarations.
SET (NATIVE_JSONCPP_INCLUDE_PATH ${JSONCPP_INCLUDE_DIR} )
GET_FILENAME_COMPONENT (NATIVE_JSONCPP_LIB_PATH ${JSONCPP_LIBRARY} PATH)

MARK_AS_ADVANCED(
  JSONCPP_LIBRARY
  JSONCPP_INCLUDE_DIR
  )

