# - Try to find Hiredis
# Once done this will define
#
#  HIREDIS_FOUND - system has HIREDIS
#  HIREDIS_INCLUDE_DIRS - the HIREDIS include directory
#  HIREDIS_LIBRARY - Link these to use HIREDIS

find_path(
    HIREDIS_INCLUDE_DIR
    NAMES Hiredis Hiredis.h Hiredis/Hiredis.h
    DOC "Hiredis include dir"
)

find_library(
    HIREDIS_LIBRARY
    NAMES Hiredis libHiredis
    DOC "Hiredis library"
)

set(HIREDIS_INCLUDE_DIRS ${HIREDIS_INCLUDE_DIR})
set(HIREDIS_LIBRARIES ${HIREDIS_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hiredis DEFAULT_MSG HIREDIS_INCLUDE_DIR HIREDIS_LIBRARY)
mark_as_advanced(HIREDIS_INCLUDE_DIR HIREDIS_LIBRARY)

