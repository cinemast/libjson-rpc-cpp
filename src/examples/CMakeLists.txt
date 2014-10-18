file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)

add_custom_command(OUTPUT abstractstubserver.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/spec.json --cpp-server=AbstractStubServer --cpp-server-file=${CMAKE_BINARY_DIR}/gen/abstractsubserver.h
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Server Stubfiles"
                   VERBATIM)

add_custom_command(OUTPUT stubclient.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/spec.json --cpp-client=StubClient --cpp-client-file=${CMAKE_BINARY_DIR}/gen/stubclient.h
                   MAIN_DEPENDENCY spec.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client Stubfile"
                   VERBATIM)

add_custom_command(OUTPUT xbmcremote.h
                   COMMAND jsonrpcstub ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xbmc_remote.json --cpp-client=XbmcRemoteClient --cpp-client-file=${CMAKE_BINARY_DIR}/gen/xbmcremote.h
                   MAIN_DEPENDENCY xbmc_remote.json
                   DEPENDS jsonrpcstub
                   COMMENT "Generating Client XBMC Stubfile"
                   VERBATIM)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

add_executable(simpleserversample simpleserver.cpp)
target_link_libraries(simpleserversample jsonrpcserver)

add_executable(simpleclientsample simpleclient.cpp)
target_link_libraries(simpleclientsample jsonrpcclient)

IF(COMPILE_STUBGEN)
    add_executable(stubclientsample stubclient.cpp stubclient.h)
    target_link_libraries(stubclientsample jsonrpcclient)

    add_executable(stubserversample stubserver.cpp abstractstubserver.h)
    target_link_libraries(stubserversample jsonrpcserver)

    add_executable(xbmcremote xbmcremote.cpp xbmcremote.h)
    target_link_libraries(xbmcremote jsonrpcclient)
ENDIF()
