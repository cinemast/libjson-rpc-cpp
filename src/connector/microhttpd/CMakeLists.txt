set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(MHD REQUIRED)
include_directories(${MHD_INCLUDE_DIR})

add_library(jsonrpccpp-connector-microhttpd microhttpserver.h microhttpserver.cpp)
target_link_libraries(jsonrpccpp-connector-microhttpd ${MHD_LIBRARIES})

set_target_properties(jsonrpccpp-connector-microhttpd PROPERTIES
  PUBLIC_HEADER "microhttpserver.h"
  SOVERSION ${SO_VERSION}
  VERSION ${VERSION_STRING}
)

configure_file(jsonrpccpp-connector-microhttpd.pc.in ${CMAKE_BINARY_DIR}/jsonrpccpp-connector-microhttpd.pc @ONLY)

install(TARGETS jsonrpccpp-connector-microhttpd 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/connector
)

install(FILES ${CMAKE_BINARY_DIR}/jsonrpccpp-connector-microhttpd.pc 
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

if(COMPILE_TESTS)
  file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${PROJECT_BINARY_DIR})
  target_sources(jsonrpccpp-unittest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/microhttpserver_test.cpp)
  add_test(connector-microhttpd ${CMAKE_BINARY_DIR}/jsonrpccpp-connector-microhttpd-test "[connector-microhttpd]" --durations yes)
endif()