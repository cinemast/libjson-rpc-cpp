configure_file(version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)

install(FILES 
    constants.h 
    exception.h 
    jsonparser.h 
    specification.h 
    ${CMAKE_BINARY_DIR}/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp)