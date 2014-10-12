file(COPY spec.json ssl_cert.pem DESTINATION ${CMAKE_BINARY_DIR}/out)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gen)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

add_custom_command(OUTPUT abstractstubserver.h
                   COMMAND jsonrpcstub ARGS -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json -s ${CMAKE_CURRENT_SOURCE_DIR}/gen/abstractsubserver.h -c AbstractStubServer
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Server Stubfiles"
                   VERBATIM)

add_custom_command(OUTPUT stubclient.h
                   COMMAND jsonrpcstub ARGS -i ${CMAKE_CURRENT_SOURCE_DIR}/spec.json --cpp=${CMAKE_CURRENT_SOURCE_DIR}/gen/stubclient.h -c StubClient
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client Stubfile"
                   VERBATIM)

add_custom_command(OUTPUT xbmcremote.h
                   COMMAND jsonrpcstub ARGS -i ${CMAKE_CURRENT_SOURCE_DIR}/xbmc_remote.json --cpp=${CMAKE_CURRENT_SOURCE_DIR}/gen/xbmcremote.h -c XbmcRemoteClient
                   MAIN_DEPENDENCY xbmc_remote.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client XBMC Stubfile"
                   VERBATIM)


add_executable(simpleserversample simpleserver.cpp)
target_link_libraries(simpleserversample jsonrpcserver)

add_executable(simpleclientsample simpleclient.cpp)
target_link_libraries(simpleclientsample jsonrpcclient)

add_executable(stubclientsample stubclient.cpp stubclient.h)
target_link_libraries(stubclientsample jsonrpcclient)

add_executable(stubserversample stubserver.cpp abstractstubserver.h)
target_link_libraries(stubserversample jsonrpcserver)

add_executable(xbmcremote xbmcremote.cpp xbmcremote.h)
target_link_libraries(xbmcremote jsonrpcclient)
