file(GLOB_RECURSE jsonrpc_source *.c*)
file(GLOB jsonrpc_header *.h)
file(GLOB connector_header connectors/*.h)

add_library(jsonrpc SHARED ${jsonrpc_source})
add_library(jsonrpcStatic STATIC ${jsonrpc_source})

set_target_properties(jsonrpcStatic PROPERTIES OUTPUT_NAME jsonrpc)

target_link_libraries(jsonrpc ${CURL_LIBRARIES})
target_link_libraries(jsonrpcStatic ${CURL_LIBRARIES})

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(FILES ${connector_header} DESTINATION include/jsonrpc/connectors) 

install(TARGETS jsonrpc jsonrpcStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)

