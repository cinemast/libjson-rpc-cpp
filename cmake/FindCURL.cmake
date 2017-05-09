# Find CURL
#
# Find the curl includes and library
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  CURL_INCLUDE_DIR, where to find header, etc.
#  CURL_LIBRARY, the libraries needed to use curl.
#  CURL_FOUND, If false, do not try to use curl.

# only look in default directories
find_path(
	CURL_INCLUDE_DIR
	NAMES curl/curl.h
	DOC "curl include dir"
)

find_library(
	CURL_LIBRARY
	# names from cmake's FindCURL
	NAMES curl curllib libcurl_imp curllib_static libcurl
	DOC "curl library"
)

add_library(CURL::libcurl UNKNOWN IMPORTED)
set_target_properties(
	CURL::libcurl
	PROPERTIES
	IMPORTED_LOCATION "${CURL_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}"
)

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	find_library(
		CURL_LIBRARY_DEBUG
		NAMES curld libcurld
		DOC "curl debug library"
	)

	set_target_properties(
		CURL::libcurl
		PROPERTIES
		IMPORTED_LOCATION_DEBUG "${CURL_LIBRARY_DEBUG}"
	)
	set(CURL_LIBRARY optimized ${CURL_LIBRARY} debug ${CURL_LIBRARY_DEBUG})
endif()

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CURL DEFAULT_MSG
	CURL_INCLUDE_DIR CURL_LIBRARY)
mark_as_advanced (CURL_INCLUDE_DIR CURL_LIBRARY)
