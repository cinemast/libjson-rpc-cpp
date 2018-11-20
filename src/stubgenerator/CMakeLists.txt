find_package(jsoncpp CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(argtable REQUIRED)

include_directories(${CMAKE_BINARY_DIR})
file(GLOB_RECURSE jsonrpcstub_source *.cpp)
add_executable(jsonrpcstub ${jsonrpcstub_source})
target_link_libraries(jsonrpcstub jsoncpp_lib ${ARGTABLE_LIBRARIES})

configure_file(jsonrpcstub.1.in "${CMAKE_BINARY_DIR}/jsonrpcstub.1" @ONLY)
add_custom_target(manpage ALL DEPENDS ${CMAKE_BINARY_DIR}/jsonrpcstub.1)
install(FILES ${CMAKE_BINARY_DIR}/jsonrpcstub.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)
add_dependencies(jsonrpcstub manpage)

install(TARGETS jsonrpcstub jsonrpcstub
    RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
)
