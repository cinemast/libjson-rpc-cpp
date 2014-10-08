file(GLOB_RECURSE test_source *.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${Boost_LIBRARIES} jsonrpccommon jsonrpcclient jsonrpcserver)

ADD_TEST(helloworld ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_helloworld)
ADD_TEST(connector ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_connector)
ADD_TEST(remotecalculator ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_remotecalculator)
ADD_TEST(remotecounter ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_remotecounter)
ADD_TEST(clientexception ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_clientexception)
ADD_TEST(jsonrpcprotocol ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_jsonrpcprotocol)
ADD_TEST(parametervalidation ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=test_parametervalidation)

ADD_TEST(all ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)
