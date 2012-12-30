file(GLOB jsonrpc_source *.cpp)
file(GLOB jsonrpc_header *.h)


add_library(jsonrpc SHARED ${jsonrpc_source})

install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc) 
install(TARGETS jsonrpc LIBRARY DESTINATION lib)
