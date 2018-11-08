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

enable_testing()
add_executable(jsonrpccpp-unittest unittests.cpp)

if(${CONNECTOR_MICROHTTPD} OR ${CONNECTOR_CURL})
  find_package(CURL REQUIRED)
  include_directories(${MHD_INCLUDE_DIR})
  target_link_libraries(jsonrpccpp-unittest jsonrpccpp-connector-curl jsonrpccpp-connector-microhttpd ${CURL_LIBRARIES} ${MHD_LIBRARIES})
endif()