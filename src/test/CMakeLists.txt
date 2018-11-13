if (COVERAGE)
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_EXCLUDES '/usr/*' '*/v1/*' '*/catch.hpp' '*/test_*' '*_test.cpp')
  SETUP_TARGET_FOR_COVERAGE(
     NAME coverage
     EXECUTABLE ctest .
     DEPENDENCIES build
  )
endif()

find_path(CATCH_INCLUDE_DIR NAMES catch.hpp)
include_directories(${CATCH_INCLUDE_DIR})

add_executable(jsonrpccpp-unittest unittests.cpp)

find_package(jsoncpp CONFIG REQUIRED)
target_link_libraries(jsonrpccpp-unittest PRIVATE jsoncpp_lib)

if (JSONRPC_CLIENT)
  target_link_libraries(jsonrpccpp-unittest PRIVATE jsonrpccpp-client)
endif()

if (JSONRPC_SERVER)
  target_link_libraries(jsonrpccpp-unittest PRIVATE jsonrpccpp-server)
endif()

if(${CONNECTOR_MICROHTTPD} OR ${CONNECTOR_CURL})
  find_package(CURL REQUIRED)
  include_directories(${MHD_INCLUDE_DIR})
  target_link_libraries(jsonrpccpp-unittest PRIVATE jsonrpccpp-connector-curl jsonrpccpp-connector-microhttpd ${CURL_LIBRARIES} ${MHD_LIBRARIES})
endif()