file(GLOB_RECURSE jsonrpcstub_source *.cpp)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

find_package(Argtable REQUIRED)
    
IF(MSVC)
    find_package(JSONCPP REQUIRED)
    find_package(Threads REQUIRED)

    include_directories(${JSONCPP_INCLUDE_DIRS})
    include_directories(${CURL_INCLUDE_DIRS})
    include_directories(${ARGTABLE_INCLUDE_DIRS})
ENDIF(MSVC)

add_executable(jsonrpcstub ${jsonrpcstub_source})
target_link_libraries(jsonrpcstub jsonrpccommon ${ARGTABLE_LIBRARIES})

#Generate manpage
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_custom_command(OUTPUT jsonrpcstub.1.gz
                   COMMAND gzip -c ${CMAKE_SOURCE_DIR}/doc/manpage > ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz
                   MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/doc/manpage
                   COMMENT "Generating man pages"
                   VERBATIM)

    add_custom_target(manpage ALL DEPENDS jsonrpcstub.1.gz)
    install(FILES ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz DESTINATION share/man/man1/)
    add_dependencies(jsonrpcstub manpage)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(TARGETS jsonrpcstub LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
