# - Try to find ARGTABLE
# Once done this will define
#
#  ARGTABLE_FOUND - system has ARGTABLE
#  ARGTABLE_INCLUDE_DIRS - the ARGTABLE include directory
#  ARGTABLE_LIBRARIES - Link these to use ARGTABLE

find_path (
    ARGTABLE_INCLUDE_DIR
    NAMES argtable2.h
    DOC "argtable include dir"
)

find_library (
    ARGTABLE_LIBRARY
    NAMES argtable2
    DOC "argtable library"
)

set(ARGTABLE_INCLUDE_DIRS ${ARGTABLE_INCLUDE_DIR})
set(ARGTABLE_LIBRARIES ${ARGTABLE_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(argtable DEFAULT_MSG ARGTABLE_INCLUDE_DIR ARGTABLE_LIBRARY)
mark_as_advanced (ARGTABLE_INCLUDE_DIR ARGTABLE_LIBRARY)

