# setup common headers and sources
file(GLOB jsonrpc_header *.h)
file(GLOB jsonrpc_header_common common/*.h)
file(GLOB jsonrpc_source_common common/*.c*)

# setup server headers and sources
file(GLOB jsonrpc_install_header_server 
	server/requesthandlerfactory.h
	server/abstractserver.h
	server/abstractserverconnector.h
	server/iprocedureinvokationhandler.h
	server/iclientconnectionhandler.h
)
file(GLOB jsonrpc_header_server server/*.h)
file(GLOB jsonrpc_source_server server/*.c*)

# setup client headers and sources
file(GLOB jsonrpc_install_header_client
	client/batchcall.h
	client/batchresponse.h
	client/client.h
	client/iclientconnector.h
)
file(GLOB jsonrpc_header_client client/*.h)
file(GLOB jsonrpc_source_client client/*.c*)

# setup connector variables defaults
set(client_connector_source "")
set(client_connector_header "")
set(client_connector_libs   "")
set(server_connector_source "")
set(server_connector_header "")
set(server_connector_libs   "")

# setup sources for http connectors
if (HTTP_CLIENT)
	list(APPEND client_connector_header "client/connectors/httpclient.h")
	list(APPEND client_connector_source "client/connectors/httpclient.cpp")
	list(APPEND client_connector_libs ${CURL_LIBRARIES})
endif()

if (HTTP_SERVER)
	list(APPEND server_connector_header "server/connectors/httpserver.h")
	list(APPEND server_connector_source "server/connectors/httpserver.cpp")
	list(APPEND server_connector_libs ${CMAKE_THREAD_LIBS_INIT} ${MHD_LIBRARIES})  
endif()

# configure a header file to pass some of the CMake settings to the source code
# TODO: move it to custom build step?
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/gen/jsonrpccpp/common")
configure_file("${CMAKE_SOURCE_DIR}/src/jsonrpccpp/version.h.in" "${CMAKE_BINARY_DIR}/gen/jsonrpccpp/version.h")
configure_file("${PROJECT_SOURCE_DIR}/src/jsonrpccpp/common/jsonparser.h.in" "${PROJECT_BINARY_DIR}/gen/jsonrpccpp/common/jsonparser.h")

install(FILES "${CMAKE_BINARY_DIR}/gen/jsonrpccpp/version.h" DESTINATION include/jsonrpccpp)
install(FILES "${PROJECT_BINARY_DIR}/gen/jsonrpccpp/common/jsonparser.h" DESTINATION include/jsonrpccpp/common)

# include required directories
include_directories(..)
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${MHD_INCLUDE_DIRS})

# setup shared common library
add_library(jsonrpccommon SHARED ${jsonrpc_source_common} ${jsonrpc_header} ${jsonrpc_helper_source_common})
target_link_libraries(jsonrpccommon ${JSONCPP_LIBRARIES})
set_target_properties(jsonrpccommon PROPERTIES OUTPUT_NAME jsonrpccpp-common)

# setup static common library
add_library(jsonrpccommonStatic STATIC ${jsonrpc_source_common} ${jsonrpc_header} ${jsonrpc_helper_source_common})
target_link_libraries(jsonrpccommonStatic ${JSONCPP_LIBRARIES})
set_target_properties(jsonrpccommonStatic PROPERTIES OUTPUT_NAME jsonrpccpp-common)

# setup shared client library
add_library(jsonrpcclient SHARED ${jsonrpc_source_client} ${jsonrpc_header} ${jsonrpc_header_client} ${client_connector_source})
add_dependencies(jsonrpcclient jsonrpccommonStatic)
target_link_libraries(jsonrpcclient jsonrpccommon ${client_connector_libs})
set_target_properties(jsonrpcclient PROPERTIES OUTPUT_NAME jsonrpccpp-client)

# setup static client library
add_library(jsonrpcclientStatic STATIC ${jsonrpc_source_client} ${jsonrpc_header} ${jsonrpc_header_client} ${client_connector_source})
target_link_libraries(jsonrpcclientStatic jsonrpccommonStatic ${client_connector_libs})
set_target_properties(jsonrpcclientStatic PROPERTIES OUTPUT_NAME jsonrpccpp-client)

# setup shared server library
add_library(jsonrpcserver SHARED ${jsonrpc_source_server} ${jsonrpc_header} ${jsonrpc_header_server} ${server_connector_source})
add_dependencies(jsonrpcserver jsonrpccommonStatic)
target_link_libraries(jsonrpcserver jsonrpccommon ${server_connector_libs})
set_target_properties(jsonrpcserver PROPERTIES OUTPUT_NAME jsonrpccpp-server)

# setup static server library
add_library(jsonrpcserverStatic STATIC ${jsonrpc_source_server} ${jsonrpc_header} ${jsonrpc_header_server} ${server_connector_source})
target_link_libraries(jsonrpcserverStatic jsonrpccommonStatic ${server_connector_libs})
set_target_properties(jsonrpcserverStatic PROPERTIES OUTPUT_NAME jsonrpccpp-server)

# setup version
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set_target_properties(
	jsonrpccommon jsonrpccommonStatic jsonrpcclient jsonrpcclientStatic jsonrpcserver jsonrpcserverStatic 
	PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${SO_VERSION}"
)

# install libraries
install(FILES ${jsonrpc_header}                 DESTINATION include/jsonrpccpp)
install(FILES ${jsonrpc_header_common}          DESTINATION include/jsonrpccpp/common)
install(FILES ${jsonrpc_helper_header_common}   DESTINATION include/jsonrpccpp/common/helper)
install(FILES ${jsonrpc_install_header_client}  DESTINATION include/jsonrpccpp/client)
install(FILES ${jsonrpc_install_header_server}  DESTINATION include/jsonrpccpp/server)
install(FILES ${client_connector_header}        DESTINATION include/jsonrpccpp/client/connectors)
install(FILES ${server_connector_header}        DESTINATION include/jsonrpccpp/server/connectors)

if (WIN32)
	file(GLOB win32_libs ${CMAKE_SOURCE_DIR}/win32-deps/lib/*.dll)
	file(COPY ${win32_libs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	install(FILES ${win32_libs} DESTINATION bin)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/win32-deps/include DESTINATION .)
endif()

install(TARGETS jsonrpccommon jsonrpccommonStatic jsonrpcclient jsonrpcclientStatic jsonrpcserver jsonrpcserverStatic
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)

