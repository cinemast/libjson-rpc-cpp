set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out/test)
file(COPY procedures.json DESTINATION ${CMAKE_BINARY_DIR}/out/test)

if(USE_HTTP)
  set(COMMON_SOURCES server.cpp)

  add_executable(helloworld helloworld.cpp ${COMMON_SOURCES})
  target_link_libraries(helloworld jsonrpc)

  add_executable(remotecounter remotecounter.cpp ${COMMON_SOURCES})
  target_link_libraries(remotecounter jsonrpc)

  add_executable(remotecalculator remotecalculator.cpp ${COMMON_SOURCES})
  target_link_libraries(remotecalculator jsonrpc)

  add_executable(errorhandling errorhandling.cpp ${COMMON_SOURCES})
  target_link_libraries(errorhandling jsonrpc)

  add_executable(jsonrpcprotocol jsonrpcprotocol.cpp ${COMMON_SOURCES})
  target_link_libraries(jsonrpcprotocol jsonrpc)
endif()
