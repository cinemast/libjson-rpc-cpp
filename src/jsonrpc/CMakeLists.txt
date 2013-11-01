file(GLOB_RECURSE jsonrpc_source *.c*)
file(GLOB jsonrpc_header *.h)
file(GLOB connector_header connectors/*.h)
file(GLOB jsoncpp_header json/*.h)

add_library(jsonrpc SHARED ${jsonrpc_source})
add_library(jsonrpcStatic STATIC ${jsonrpc_source})

set_target_properties(jsonrpcStatic PROPERTIES OUTPUT_NAME jsonrpc)

set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}) 
set_target_properties(jsonrpc jsonrpcStatic PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${VERSION_MAJOR}")

target_link_libraries(jsonrpc pthread dl ${CURL_LIBRARIES})
target_link_libraries(jsonrpcStatic pthread dl ${CURL_LIBRARIES})

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(FILES ${connector_header} DESTINATION include/jsonrpc/connectors)
install(FILES ${jsoncpp_header} DESTINATION include/jsonrpc/json) 

install(TARGETS jsonrpc jsonrpcStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)

