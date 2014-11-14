
message(STATUS "Found Bosst Unit Suite -> Enabling test cases")
set(CTEST_OUTPUT_ON_FAILURE TRUE)

file(GLOB_RECURSE test_source *.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

IF(HTTP_CLIENT AND HTTP_SERVER)
    add_definitions(-DHTTP_TESTING)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${Boost_LIBRARIES} jsonrpccommon jsonrpcclient jsonrpcserver)


ADD_TEST(client ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=client)
ADD_TEST(server ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=server)
ADD_TEST(common ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=common)
IF(HTTP_CLIENT AND HTTP_SERVER)
    ADD_TEST(NAME connector_http WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=connector_http )
ENDIF()


ADD_TEST(NAME all WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)
