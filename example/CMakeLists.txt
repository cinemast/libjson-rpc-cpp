file(COPY res DESTINATION ${CMAKE_BINARY_DIR}/out)

add_executable(jsonrpcsample main.cpp)
target_link_libraries(jsonrpcsample jsonrpc)