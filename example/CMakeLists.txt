file(COPY res DESTINATION ${CMAKE_BINARY_DIR}/out)

add_executable(jsonrpcserversample simpleserver.cpp)
target_link_libraries(jsonrpcserversample jsonrpc)

add_executable(jsonrpcclientsample simpleclient.cpp)
target_link_libraries(jsonrpcclientsample jsonrpc)

add_executable(jsonrpcstubclient stubclient.cpp)
target_link_libraries(jsonrpcstubclient jsonrpc)
