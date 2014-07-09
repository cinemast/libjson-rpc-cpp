set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out/test)


if (HTTP_CONNECTOR)
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

	add_executable(specification specification.cpp ${COMMON_SOURCES})
	target_link_libraries(specification jsonrpc)

	ADD_TEST(helloworld ${TEST_BINARIES}/helloworld)
	ADD_TEST(remotecounter ${TEST_BINARIES}/remotecounter)
	ADD_TEST(remotecalculator ${TEST_BINARIES}/remotecalculator)
	ADD_TEST(errorhandling ${TEST_BINARIES}/errorhandling)
	ADD_TEST(jsonrpcprotocol ${TEST_BINARIES}/jsonrpcprotocol)
	ADD_TEST(specification ${TEST_BINARIES}/specification)
endif()


add_executable(parametervalidation parametervalidation.cpp)
target_link_libraries(parametervalidation jsonrpc)


ADD_TEST(parametervalidation ${TEST_BINARIES}/parametervalidation)