file(COPY spec.json ssl_cert.pem DESTINATION ${CMAKE_BINARY_DIR}/out)

add_executable(simpleserversample simpleserver.cpp)
target_link_libraries(simpleserversample jsonrpc)

add_executable(simpleclientsample simpleclient.cpp)
target_link_libraries(simpleclientsample jsonrpc)

add_executable(stubclientsample stubclient.cpp)
target_link_libraries(stubclientsample jsonrpc)

add_executable(stubserversample stubserver.cpp)
target_link_libraries(stubserversample jsonrpc)


add_executable(xbmcremote xbmcremote.cpp)
target_link_libraries(xbmcremote jsonrpc)
