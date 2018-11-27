find_package(jsoncpp CONFIG REQUIRED)

include_directories(${CMAKE_BINARY_DIR})
set(jsonrpcstub_source
    ${CMAKE_CURRENT_SOURCE_DIR}/client/cppclientstubgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client/jsclientstubgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client/pyclientstubgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/cpphelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/server/cppserverstubgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procedure.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/specificationparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stubgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
)
add_executable(jsonrpcstub ${jsonrpcstub_source} main.cpp)
target_link_libraries(jsonrpcstub jsoncpp_lib)

configure_file(jsonrpcstub.1.in "${CMAKE_BINARY_DIR}/jsonrpcstub.1" @ONLY)
add_custom_target(manpage ALL DEPENDS ${CMAKE_BINARY_DIR}/jsonrpcstub.1)
install(FILES ${CMAKE_BINARY_DIR}/jsonrpcstub.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)
add_dependencies(jsonrpcstub manpage)

install(TARGETS jsonrpcstub jsonrpcstub
    RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
)

#if(COMPILE_TESTS)
#    target_sources(jsonrpccpp-unittest PRIVATE
 #       ${jsonrpcstub_source}
  #      ${CMAKE_CURRENT_SOURCE_DIR}/test/test_stubgenerator.cpp
   # )
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec1.json DESTINATION ${PROJECT_BINARY_DIR})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec2.json DESTINATION ${PROJECT_BINARY_DIR})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec3.json DESTINATION ${PROJECT_BINARY_DIR})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec4.json DESTINATION ${PROJECT_BINARY_DIR})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec5.json DESTINATION ${PROJECT_BINARY_DIR})
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/testspec6.json DESTINATION ${PROJECT_BINARY_DIR})
    #add_test(server ${CMAKE_BINARY_DIR}/jsonrpccpp-unittest "[stubgenerator]" --durations yes)
#endif()
