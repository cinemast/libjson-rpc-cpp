set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out/test)


if (HTTP_CONNECTOR)
	set(COMMON_SOURCES server.cpp)

	if (NOT("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
		include(CheckCXXCompilerFlag)
		CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
		CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
		if(COMPILER_SUPPORTS_CXX11)
			ADD_DEFINITIONS(-std=c++11)
		elseif(COMPILER_SUPPORTS_CXX0X)
			ADD_DEFINITIONS(-std=c++0x)
		endif()

		if (COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X)
			if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
				ADD_DEFINITIONS(-stdlib=libc++)
			endif()

			add_executable(concurrency concurrency.cpp ${COMMON_SOURCES})
			target_link_libraries(concurrency jsonrpc)

			ADD_TEST(concurrency ${TEST_BINARIES}/concurrency)
		endif()
	endif()

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

if (SOCKET_CONNECTOR)
	set(COMMON_SOURCES server.cpp)
	add_executable(tcpconnector tcpconnector.cpp ${COMMON_SOURCES})
	target_link_libraries(tcpconnector jsonrpc)
  ADD_TEST(tcpconnector ${TEST_BINARIES}/tcpconnector)
endif()

add_executable(parametervalidation parametervalidation.cpp)
target_link_libraries(parametervalidation jsonrpc)


ADD_TEST(parametervalidation ${TEST_BINARIES}/parametervalidation)
