file(GLOB jsonrpc_source *.cpp)
file(GLOB jsonrpc_header *.h)


add_library(jsonrpc SHARED ${jsonrpc_source})

target_link_libraries(jsonrpc json)

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(TARGETS jsonrpc LIBRARY DESTINATION lib)
