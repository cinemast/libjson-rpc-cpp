
message(STATUS "Found Bosst Unit Suite -> Enabling test cases")
set(CTEST_OUTPUT_ON_FAILURE TRUE)

file(GLOB_RECURSE test_source *.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${Boost_LIBRARIES} jsonrpccommon jsonrpcclient jsonrpcserver)


ADD_TEST(client ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=client)
ADD_TEST(server ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=server)
ADD_TEST(common ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=common)
ADD_TEST(stubgen ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=stubgen)
ADD_TEST(integration ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=integration)


ADD_TEST(all ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)
