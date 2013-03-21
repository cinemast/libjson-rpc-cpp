file(GLOB jsonrpc_source *.cpp)
file(GLOB jsonrpc_header *.h)

if(USE_HTTP)
  file(GLOB http_connector_headers connectors/http*.h)
  file(GLOB http_connector_sources connectors/http*.cpp)
  
  set(jsonrpc_source ${jsonrpc_source} ${http_connector_sources})
  set(connector_header ${connector_header} ${http_connector_headers})
endif()

add_library(jsonrpc SHARED ${jsonrpc_source})
add_library(jsonrpcStatic STATIC ${jsonrpc_source})

set_target_properties(jsonrpcStatic PROPERTIES OUTPUT_NAME jsonrpc)

target_link_libraries(jsonrpc ${DEP_LIBS})
target_link_libraries(jsonrpcStatic ${DEP_LIBS})

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(FILES ${connector_header} DESTINATION include/jsonrpc/connectors) 

install(TARGETS jsonrpc jsonrpcStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)

