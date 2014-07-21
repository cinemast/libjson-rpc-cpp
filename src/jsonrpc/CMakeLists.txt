file(GLOB jsonrpc_source *.cpp json/*.c*)
file(GLOB jsonrpc_header *.h)


set(JSONRPC_DEPENDENCIES ${CMAKE_THREAD_LIBS_INIT})

if(HTTP_CONNECTOR)
	file(GLOB connector_mongoose_source connectors/mongoose*.c*)
	file(GLOB connector_mongoose_header connectors/mongoose*.h)
	file(GLOB connector_http_source connectors/http*.c*)
	file(GLOB connector_http_header connectors/http*.h)
	set(JSONRPC_DEPENDENCIES ${JSONRPC_DEPENDENCIES} ${CURL_LIBRARIES} dl)
endif()

file(GLOB jsoncpp_header json/*.h)

if(SOCKET_CONNECTOR)
	file(GLOB connector_socket_source connectors/socket*.c*)
	file(GLOB connector_socket_header connectors/socket*.h)
  set(connector_socket_header ${connector_socket_header} connectors/threads.h)
  set(connector_socket_source ${connector_socket_source} connectors/threads.cpp)
	if (MSVC)
		set(JSONRPC_DEPENDENCIES ${JSONRPC_DEPENDENCIES} ${Socket_LIBRARIES})
	endif()
endif()

set(connectors_source ${connector_http_source} ${connector_mongoose_source} ${connector_socket_source})

add_library(jsonrpc SHARED ${jsonrpc_source} ${connectors_source})
add_library(jsonrpcStatic STATIC ${jsonrpc_source} ${connectors_source})

set_target_properties(jsonrpcStatic PROPERTIES OUTPUT_NAME jsonrpc)

set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}) 
set_target_properties(jsonrpc jsonrpcStatic PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${VERSION_MAJOR}")

target_link_libraries(jsonrpc ${JSONRPC_DEPENDENCIES})
target_link_libraries(jsonrpcStatic ${JSONRPC_DEPENDENCIES})



install(FILES ${jsonrpc_header} DESTINATION include/jsonrpc)
install(FILES ${jsoncpp_header} DESTINATION include/jsonrpc/json)
install(FILES ${connector_http_header} ${connector_mongoose_header} DESTINATION include/jsonrpc/connectors)
install(FILES ${connector_socket_header} DESTINATION include/jsonrpc/connectors)

install(TARGETS jsonrpc jsonrpcStatic
					LIBRARY DESTINATION lib
					ARCHIVE DESTINATION lib
					RUNTIME DESTINATION bin)

