
set(CTEST_OUTPUT_ON_FAILURE TRUE)

file(GLOB_RECURSE test_source *.cpp)
file(GLOB test_specs *.json)

file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/src/examples/server.key ${CMAKE_SOURCE_DIR}/src/examples/server.pem DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${test_specs} DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${test_specs} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

include_directories(..)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

if (HTTP_CLIENT AND HTTP_SERVER)
        add_definitions(-DHTTP_TESTING)
else()
        list(REMOVE_ITEM test_source "${CMAKE_CURRENT_SOURCE_DIR}/testhttpserver.cpp")
endif()

if (COMPILE_STUBGEN)
	add_definitions(-DSTUBGEN_TESTING)
endif()

if (HTTP_CLIENT AND HTTP_SERVER AND COMPILE_STUBGEN)
	add_definitions(-DINTEGRATION_TESTING)
endif()

# TODO: not sure if this is necessary
if (WIN32)
	link_directories(${Boost_LIBRARY_DIRS})
endif ()

add_executable(unit_testsuite ${test_source})
target_link_libraries(unit_testsuite ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
target_link_libraries(unit_testsuite jsonrpccommon)
target_link_libraries(unit_testsuite jsonrpcserver)
target_link_libraries(unit_testsuite jsonrpcclient)

if (COMPILE_STUBGEN)
	target_link_libraries(unit_testsuite stubgen)
endif()

add_test(client ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=client)
add_test(server ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=server)
add_test(NAME common WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=common)

if (HTTP_CLIENT AND HTTP_SERVER)
	add_test(NAME connector_http WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=connector_http)
endif()

if (COMPILE_STUBGEN)
	add_test(NAME stubgen WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=stubgenerator)
endif()

if (HTTP_CLIENT AND HTTP_SERVER AND COMPILE_STUBGEN)
	add_test(NAME integration WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite --run_test=integration)
endif()

add_test(NAME all WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unit_testsuite)

