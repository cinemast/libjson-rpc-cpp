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

install(TARGETS jsonrpccpp-client 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/client
)

