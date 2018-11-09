find_package(jsoncpp CONFIG REQUIRED)

add_library(jsonrpccpp-client 
    batchcall.h batchcall.cpp 
    batchresponse.h batchresponse.cpp 
    client.h client.cpp
    rpcprotocolclient.h rpcprotocolclient.cpp
)
 target_link_libraries(jsonrpccpp-client PRIVATE jsoncpp_lib)

 set_target_properties(jsonrpccpp-client PROPERTIES
  PUBLIC_HEADER "batchcall.h batchresponse.h client.h rpcprotocolclient.h"
  SOVERSION ${SO_VERSION}
  VERSION ${VERSION_STRING}
)

configure_file(jsonrpccpp-client.pc.in ${CMAKE_BINARY_DIR}/jsonrpccpp-client.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/jsonrpccpp-client.pc 
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(TARGETS jsonrpccpp-client 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/client
)

if (COMPILE_TESTS)
    target_sources(jsonrpccpp-unittest PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/test_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mockclientconnector.cpp
    )
    add_test(client ${BMAKE_BINARY_DIR}/jsonrpccpp-unittest "[client]" --durations yes)
endif()