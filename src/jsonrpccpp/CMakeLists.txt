find_package(JSONCPP REQUIRED)
find_package(Threads REQUIRED)

file(GLOB jsonrpc_source_common common/*.c*)
file(GLOB jsonrpc_source_client client/*.c*)
file(GLOB jsonrpc_source_server server/*.c*)
list(APPEND jsonrpc_source_server server/abstractserver.h)

file(GLOB jsonrpc_header_common common/*.h)
file(GLOB jsonrpc_header_client client/*.h)
file(GLOB jsonrpc_header_server server/*.h)
file(GLOB jsonrpc_header *.h)

set(client_connector_source "")
set(client_connector_header "")
set(client_connector_libs   "")
set(server_connector_source "")
set(server_connector_header "")
set(server_connector_libs   "")

include_directories("${PROJECT_SOURCE_DIR}/src/")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/jsonrpccpp/version.h.in"
  "${PROJECT_BINARY_DIR}/version.h"
)
install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION include/jsonrpccpp) 


IF(HTTP_CLIENT_CURL)
    find_package(CURL REQUIRED)
    list(APPEND client_connector_header "client/connectors/httpclient.h")
    list(APPEND client_connector_source "client/connectors/httpclient.cpp")
    list(APPEND client_connector_libs ${CURL_LIBRARIES})
ENDIF()

IF(HTTP_SERVER_MONGOOSE)
    message(STATUS "Activated Mongoose HTTP Server")
    list(APPEND server_connector_header "server/connectors/mongoose.h")
    list(APPEND server_connector_source "server/connectors/mongoose.c")
    list(APPEND server_connector_header "server/connectors/httpserver.h")
    list(APPEND server_connector_source "server/connectors/httpserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

IF(SOCKET_CLIENT)
    find_package(Threads REQUIRED)
    list(APPEND client_connector_header "client/connectors/socketclient.h")
    list(APPEND client_connector_source "client/connectors/socketclient.cpp")
ENDIF()

IF(SOCKET_SERVER)
    find_package(Threads REQUIRED)
    list(APPEND server_connector_header "server/connectors/socketserver.h")
    list(APPEND server_connector_source "server/connectors/socketserver.cpp")
    list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT})

ENDIF()

IF(SOCKET_CLIENT OR SOCKET_SERVER)
    list(APPEND jsonrpc_helper_header_common "common/helper/socket.h")
    list(APPEND jsonrpc_helper_header_common "common/helper/threads.h")
    list(APPEND jsonrpc_helper_source_common "common/helper/threads.cpp")
ENDIF()

add_library(jsonrpccommon SHARED ${jsonrpc_source_common} ${jsonrpc_helper_source_common})
add_library(jsonrpccommonStatic STATIC ${jsonrpc_source_common} ${jsonrpc_helper_source_common})
target_link_libraries(jsonrpccommon ${JSONCPP_LIBRARIES})
target_link_libraries(jsonrpccommonStatic ${JSONCPP_LIBRARIES})
set_target_properties(jsonrpccommonStatic PROPERTIES OUTPUT_NAME jsonrpccpp-common)
set_target_properties(jsonrpccommon PROPERTIES OUTPUT_NAME jsonrpccpp-common)


add_library(jsonrpcclient SHARED ${jsonrpc_source_client} ${client_connector_source})
add_library(jsonrpcclientStatic STATIC ${jsonrpc_source_client} ${client_connector_source})
target_link_libraries(jsonrpcclient jsonrpccommon ${client_connector_libs})
target_link_libraries(jsonrpcclientStatic jsonrpccommonStatic ${client_connector_libs})
set_target_properties(jsonrpcclientStatic PROPERTIES OUTPUT_NAME jsonrpccpp-client)
set_target_properties(jsonrpcclient PROPERTIES OUTPUT_NAME jsonrpccpp-client)


add_library(jsonrpcserver SHARED ${jsonrpc_source_server} ${server_connector_source})
add_library(jsonrpcserverStatic STATIC ${jsonrpc_source_server} ${server_connector_source})
target_link_libraries(jsonrpcserver jsonrpccommon ${server_connector_libs})
target_link_libraries(jsonrpcserverStatic jsonrpccommonStatic ${server_connector_libs})
set_target_properties(jsonrpcserverStatic PROPERTIES OUTPUT_NAME jsonrpccpp-server)
set_target_properties(jsonrpcserver PROPERTIES OUTPUT_NAME jsonrpccpp-server)


set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set_target_properties(jsonrpccommon jsonrpccommonStatic jsonrpcclient jsonrpcclientStatic jsonrpcserver jsonrpcserverStatic PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}")

install(FILES ${jsonrpc_header}                 DESTINATION include/jsonrpccpp)
install(FILES ${jsonrpc_header_common}          DESTINATION include/jsonrpccpp/common)
install(FILES ${jsonrpc_helper_header_common}   DESTINATION include/jsonrpccpp/common/helper)
install(FILES ${jsonrpc_header_client}          DESTINATION include/jsonrpccpp/client)
install(FILES ${jsonrpc_header_server}          DESTINATION include/jsonrpccpp/server)
install(FILES ${client_connector_header}        DESTINATION include/jsonrpccpp/client/connectors)
install(FILES ${server_connector_header}        DESTINATION include/jsonrpccpp/server/connectors)

install(TARGETS jsonrpccommon jsonrpccommonStatic jsonrpcclient jsonrpcclientStatic jsonrpcserver jsonrpcserverStatic
                      LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
