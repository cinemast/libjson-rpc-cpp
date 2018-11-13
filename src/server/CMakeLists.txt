find_package(jsoncpp CONFIG REQUIRED)

add_library(jsonrpccpp-server 
    abstractprotocolhandler.h abstractprotocolhandler.cpp
    requesthandlerfactory.h requesthandlerfactory.cpp
    abstractserver.h
    procedure.h procedure.cpp
    rpcprotocolserverv1.h rpcprotocolserverv1.cpp
    rpcprotocolserverv2.h rpcprotocolserverv2.cpp
    rpcprotocolserver12.h rpcprotocolserver12.cpp
)
 target_link_libraries(jsonrpccpp-server PRIVATE jsoncpp_lib)

 set_target_properties(jsonrpccpp-server PROPERTIES
  PUBLIC_HEADER "rpcprotocolserverv1.h rpcprotocolserverv2.h rpcprotocolserver12.h procedure.h abstractserver.h abstractprotocolhandler.h"
  SOVERSION ${SO_VERSION}
  VERSION ${VERSION_STRING}
)

configure_file(jsonrpccpp-server.pc.in ${CMAKE_BINARY_DIR}/jsonrpccpp-server.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/jsonrpccpp-server.pc 
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(TARGETS jsonrpccpp-server 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/server
)

if (COMPILE_TESTS)
    target_sources(jsonrpccpp-unittest PRIVATE 
         ${CMAKE_CURRENT_SOURCE_DIR}/test_server.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/test_mockserverconnector.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/test_mockclientconnectionhandler.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/test_serverapp.cpp
     )
     add_test(server ${BMAKE_BINARY_DIR}/jsonrpccpp-unittest "[server]" --durations yes)
endif()