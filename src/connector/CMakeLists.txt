install(FILES iclientconnectionhandler.h abstractserverconnector.h iclientconnector.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonrpccpp/connector
)

if(COMPILE_TESTS)
  target_sources(jsonrpccpp-unittest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abstractserverconnector_test.cpp)
  add_test(connector-abstractserverconnector ${CMAKE_BINARY_DIR}/jsonrpccpp-unittest --durations yes "abstractserverconnector")
endif()

