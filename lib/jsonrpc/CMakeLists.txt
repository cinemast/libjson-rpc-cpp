file(GLOB_RECURSE jsonrpc_source *.cpp)
file(GLOB jsonrpc_header *.h)
file(GLOB connector_header connectors/*.h)

find_package(CURL REQUIRED)

add_library(jsonrpc SHARED ${jsonrpc_source})

target_link_libraries(jsonrpc json mongoose ${CURL_LIBRARIES})

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(FILES ${connector_header} DESTINATION include/jsonrpc/connectors) 

install(TARGETS jsonrpc LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)

