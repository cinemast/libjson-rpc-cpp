file(GLOB_RECURSE jsonrpcstub_source *.cpp)

add_executable(jsonrpcstub ${jsonrpcstub_source})
target_link_libraries(jsonrpcstub jsonrpc)

file(COPY jsonrpcstub.tpl method.tpl DESTINATION ${CMAKE_BINARY_DIR}/out)