
message(STATUS "Found Bosst Unit Suite -> Enabling test cases")
set(CTEST_OUTPUT_ON_FAILURE TRUE)

file(GLOB_RECURSE test_source *.cpp)
file(GLOB test_specs *.json)

file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${test_specs} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${test_specs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(testlibs ${Boost_LIBRARIES} jsonrpccommon jsonrpcclient jsonrpcserver)

IF(HTTP_CLIENT AND HTTP_SERVER)
    add_definitions(-DHTTP_TESTING)
ENDIF()

IF(SOCKET_SERVER AND SOCKET_CLIENT)
    add_definitions(-DTCPSOCKET_TESTING)
ENDIF()

IF(COMPILE_STUBGEN)
    add_definitions(-DSTUBGEN_TESTING)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
IF(WIN32)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF()


add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${testlibs})
IF(COMPILE_STUBGEN)
    target_link_libraries(unit_testsuite stubgen)
ENDIF()

ADD_TEST(client ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=client)
ADD_TEST(server ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=server)
ADD_TEST(NAME common WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=common)
IF(HTTP_CLIENT AND HTTP_SERVER)
    ADD_TEST(NAME connector_http WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=connector_http)
ENDIF()
IF(COMPILE_STUBGEN)
    ADD_TEST(NAME stubgen WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=stubgenerator)
ENDIF()

ADD_TEST(NAME all WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)
