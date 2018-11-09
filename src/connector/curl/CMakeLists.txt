find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
add_library(jsonrpccpp-connector-curl curlclient.h curlclient.cpp)
target_link_libraries(jsonrpccpp-connector-curl ${CURL_LIBRARIES})

set_target_properties(jsonrpccpp-connector-curl PROPERTIES
  PUBLIC_HEADER "curlclient.h"
  SOVERSION ${SO_VERSION}
  VERSION ${VERSION_STRING}
)

configure_file(jsonrpccpp-connector-curl.pc.in ${CMAKE_BINARY_DIR}/jsonrpccpp-connector-curl.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/jsonrpccpp-connector-curl.pc 
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(TARGETS jsonrpccpp-connector-curl 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/connector
)

if(COMPILE_TESTS)
  if (NOT TARGET jsonrpccpp-connector-microhttpd)
    add_subdirectory(../microhttpd ${CMAKE_BINARY_DIR}/src/connector/microhttpd)
  endif()
  file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${PROJECT_BINARY_DIR})
  target_sources(jsonrpccpp-unittest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/curlclient_test.cpp)
  add_test(connector-curl ${CMAKE_BINARY_DIR}/jsonrpccpp-unittest "[connector-curl]" --durations yes)
endif()